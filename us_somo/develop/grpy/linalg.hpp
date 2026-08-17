// GRPY C++ port -- Phase 3 linear algebra: tiled symmetric-positive-definite
// Cholesky factor + solve, designed for the memory-bound large-N regime.
//
// Why tiled: the 11N x 11N mobility is the memory wall (thousands of beads maxed a
// 256 GB VM). We store only the UPPER-triangle tiles (~half the memory), factor
// IN PLACE (no second matrix), and template on the scalar type so `float` halves
// storage again for very large systems. The blocked structure also lets the O(N^3)
// trailing update fan out over a thread pool (the `Parallel` abstraction) and emit
// fine-grained progress per block-column -- the two things the SOMO integration needs.
//
// Storage: A = U^T U (U upper). Tiles A(i,j) for 0<=i<=j<nt, packed by column.
#pragma once
#include <Eigen/Dense>
#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace la {

// Raised instead of aborting or returning silently corrupt state. Running in-process means a
// failure here takes the whole of SOMO with it unless the caller is given something to catch:
// the GRPY solve is wrapped at its call site in us_hydrodyn_grpy.cpp.
struct Error : std::runtime_error {
    explicit Error( const std::string& what ) : std::runtime_error( what ) {}
};

// ---- parallel-for abstraction ----------------------------------------------
// Runs f(k) for k in [0,n). The serial backend is used for standalone tests and
// the Qt backend (QThreadPool) for SOMO -- swapped without touching the algorithm.
struct Parallel {
    virtual void for_range(int n, const std::function<void(int)>& f) = 0;
    virtual ~Parallel() = default;
};
struct Serial : Parallel {
    void for_range(int n, const std::function<void(int)>& f) override {
        for (int k = 0; k < n; ++k) f(k);
    }
};

// pct in [0,100], stage label; called from the driving thread between block-columns
using Progress = std::function<void(int pct, const char* stage)>;

// Progress reporting has two independent problems, and one knob cannot solve both.
//
//   FREEZE LENGTH -- with a blocking parallel backend the calling thread is a worker, so
//   the event loop cannot turn until the current slice of work ends. Fixed by sizing that
//   slice: `tune_chunk` below.
//
//   CALL RATE -- the callback itself costs something (a GUI repaint against a large text
//   buffer is not cheap), and that cost is paid per call however short the slices are.
//   Fixed by rate-limiting the calls: `ProgressGate` below.
//
// Conflating them is a trap worth naming: an earlier version timed work+callback together
// and resized the chunk from it, so an expensive callback SHRANK the chunk, which raised
// the call rate and made the overhead worse. Measured on a 512-bead model, a 20 ms callback
// cost 2.79x the whole solve.

// Size a work slice so the event loop gets a turn often enough. Times only the work, never
// the callback -- the callback's cost is the gate's problem, not the chunk's. Halve or
// double rather than solving for the target, so one anomalous measurement (a scheduler
// stall, another process, swap) cannot set a wild chunk size.
template <typename TP>
inline void tune_chunk(int& chunk, const TP& t0, const TP& t1, int chunk_max = (1 << 20)) {
    const double work_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    if      (work_ms <  40.0 && chunk < chunk_max) chunk *= 2;
    else if (work_ms > 160.0 && chunk > 1)         chunk /= 2;
}

// Rate-limit progress calls, adapting to what the callback actually costs: emit only after
// ~10x the last callback's duration has elapsed, and never more often than every ~80 ms.
// A cheap callback is therefore essentially unthrottled; an expensive one throttles itself
// until it is ~10% overhead instead of dominating.
//
// This is what governs cadence where the emission points are fixed and not chunked -- the
// triangular solve ticks once per tile, and nt runs to the hundreds on a large model.
// Frequent repaints and low overhead are not simultaneously achievable when a repaint is
// expensive; this trades update rate for throughput, deliberately.
class ProgressGate {
public:
    void tick(const Progress& prog, int pct, const char* stage) {
        if (!prog) return;
        const auto now = std::chrono::steady_clock::now();
        if (primed_ &&
            std::chrono::duration<double, std::milli>(now - last_).count()
                < std::max(80.0, 10.0 * cb_ms_)) return;
        prog(pct, stage);
        const auto done = std::chrono::steady_clock::now();
        cb_ms_  = std::chrono::duration<double, std::milli>(done - now).count();
        last_   = done;
        primed_ = true;
    }
    // Emit unconditionally -- for the final tick, so the bar always lands on its end value.
    void flush(const Progress& prog, int pct, const char* stage) {
        if (prog) prog(pct, stage);
    }
private:
    std::chrono::steady_clock::time_point last_{};
    double cb_ms_  = 0.0;
    bool   primed_ = false;
};

// ---- tiled upper SPD matrix -------------------------------------------------
// Storage is a single flat buffer of the upper tiles (packed by column). It is a
// plain heap allocation by default, or -- if a backing file is given -- a
// memory-mapped file, so the OS spills cold tiles to disk and RAM stays bounded
// (out-of-core). The factor/solve code is identical either way: tile(i,j) returns
// an Eigen::Map view into the buffer.
template <typename S>
struct TiledUpperSPD {
    using Mat = Eigen::Matrix<S, Eigen::Dynamic, Eigen::Dynamic>;
    using MapT = Eigen::Map<Mat>;
    int n, b, nt;
    std::vector<size_t> off;      // element offset of each upper tile, idx(i,j)
    size_t nelem = 0;             // total elements
    S* data = nullptr;
    // mmap bookkeeping (data owned by mmap when fd>=0, else by ::free)
    int fd = -1; size_t mapbytes = 0; std::string path; bool unlink_on_close = false;

    // In-core (file empty) or out-of-core (file = backing path, created & sized).
    TiledUpperSPD(int n_, int b_ = 256, const std::string& file = "")
        : n(n_), b(b_), nt((n_ + b_ - 1) / b_) {
        off.resize((size_t)nt * (nt + 1) / 2);
        for (int j = 0; j < nt; ++j)
            for (int i = 0; i <= j; ++i) { off[idx(i, j)] = nelem; nelem += (size_t)rows(i) * rows(j); }
        if (file.empty()) {
            data = static_cast<S*>(::calloc(nelem, sizeof(S)));
            // An unchecked calloc here meant every later tile access dereferenced null. The
            // sizes involved are exactly the ones that fail: this is the memory wall the
            // tiling exists for.
            if (!data)
                throw Error("GRPY: could not allocate the " + std::to_string(nelem * sizeof(S))
                            + " byte mobility matrix. Reduce the model size, or enable "
                              "single precision or out-of-core in the GRPY options.");
        } else {
            path = file; unlink_on_close = true;
            mapbytes = nelem * sizeof(S);
            fd = ::open(file.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0600);
            // Previously std::abort(): an unwritable or full out-of-core directory killed
            // SOMO outright and took any unsaved session with it.
            if (fd < 0)
                throw Error("GRPY: cannot create the out-of-core file '" + file
                            + "': " + std::strerror(errno));
            if (::ftruncate(fd, mapbytes) != 0) {
                const std::string e = std::strerror(errno);
                ::close(fd); fd = -1; ::unlink(file.c_str());
                throw Error("GRPY: cannot size the out-of-core file '" + file + "' to "
                            + std::to_string(mapbytes) + " bytes: " + e
                            + ". Check the free space on that filesystem.");
            }
            void* p = ::mmap(nullptr, mapbytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            if (p == MAP_FAILED) {
                const std::string e = std::strerror(errno);
                ::close(fd); fd = -1; ::unlink(file.c_str());
                throw Error("GRPY: cannot map the out-of-core file '" + file + "': " + e);
            }
            data = static_cast<S*>(p);
        }
    }
    ~TiledUpperSPD() {
        if (fd >= 0) { ::munmap(data, mapbytes); ::close(fd); if (unlink_on_close) ::unlink(path.c_str()); }
        else ::free(data);
    }
    TiledUpperSPD(const TiledUpperSPD&) = delete;
    TiledUpperSPD& operator=(const TiledUpperSPD&) = delete;

    int idx(int i, int j) const { return j * (j + 1) / 2 + i; }   // i<=j
    int rows(int i) const { return std::min(b, n - i * b); }
    // Row offset of tile-row i, widened BEFORE the multiply. int*int would overflow int and
    // only then convert to Eigen::Index, which is what CodeQL flags at the middleRows() calls.
    Eigen::Index roff(int i) const { return static_cast<Eigen::Index>(i) * b; }
    MapT tile(int i, int j) { return MapT(data + off[idx(i, j)], rows(i), rows(j)); }

    // element write for assembly (upper only: caller passes r<=c). Column-major.
    S& at(int r, int c) {
        int ti = r / b, tj = c / b, rr = r % b, cc = c % b;
        return data[off[idx(ti, tj)] + (size_t)cc * rows(ti) + rr];
    }
    size_t bytes() const { return nelem * sizeof(S); }
    bool out_of_core() const { return fd >= 0; }

    // Right-looking tiled Cholesky, in place: A -> U (upper), A = U^T U.
    // tile(...) returns Map views into the flat buffer, so writes go straight to it
    // (in-core or mmap'd). solveInPlace avoids aliasing between same-buffer views.
    // The caller's progress callback typically drives a GUI, and with a blocking parallel
    // backend (QtConcurrent::blockingMap) the calling thread IS one of the workers -- so the
    // event loop cannot run for the whole duration of a for_range. Reporting once per tile
    // column therefore froze the UI for the length of one column, and the trailing update
    // costs O((nt-k)^2), making the FIRST columns by far the longest (measured ~5 s each on
    // a 31152-dim factorization). The trailing update is chunked so the callback fires
    // inside it, and the chunk self-tunes to a target wall time so the granularity holds on
    // any machine. With no callback the work runs as one chunk exactly as before.
    void factor(Parallel& par, const Progress& prog = {}) {
        // Cost-weighted denominator: the bar tracks trailing-update work actually done, not
        // the column index. Cost per column is quadratic, so a k-linear bar (what this used
        // to report) crawls at the start and races at the end.
        long long work_total = 0;
        for (int k = 0; k < nt; ++k) {
            long long m = nt - k - 1;
            work_total += m * (m + 1) / 2;
        }
        if (work_total <= 0) work_total = 1;
        long long work_done = 0;
        int chunk = 64;                       // self-tuning; see below
        ProgressGate gate;

        for (int k = 0; k < nt; ++k) {
            // POTRF: diagonal tile A(k,k) = U^T U, store U (upper). Read UPPER (matrixU).
            auto Akk = tile(k, k);
            Eigen::LLT<Eigen::Ref<Mat>, Eigen::Upper> llt(Akk);  // reads upper of A(k,k)
            Mat Ukk = llt.matrixU();                             // U (upper), A(k,k)=U^T U
            Akk = Ukk;
            // TRSM: for j>k, U(k,j) = L(k,k)^{-1} A(k,j)  (independent across j)
            par.for_range(nt - k - 1, [&](int t) {
                int j = k + 1 + t;
                auto Tkj = tile(k, j);
                Ukk.transpose().template triangularView<Eigen::Lower>().solveInPlace(Tkj);
            });
            // trailing update: A(i,j) -= U(k,i)^T U(k,j) for k<i<=j  (independent per (i,j))
            std::vector<std::pair<int,int>> jobs;
            for (int j = k + 1; j < nt; ++j)
                for (int i = k + 1; i <= j; ++i) jobs.push_back({i, j});

            const int njobs = (int) jobs.size();
            // Advance by what was actually done, never by `chunk` -- it is retuned at the
            // bottom of this loop, so using it as the stride would skip jobs when it grew.
            // Retuning WITHIN a column matters: the first columns carry the most work and
            // are where the freeze was worst, so they must not wait for the next column to
            // get a better chunk size.
            for (int base = 0; base < njobs; ) {
                const int n = prog ? std::min(chunk, njobs - base) : (njobs - base);
                const auto t0 = std::chrono::steady_clock::now();
                par.for_range(n, [&](int t) {
                    int i = jobs[base + t].first, j = jobs[base + t].second;
                    auto Tij = tile(i, j);
                    Tij.noalias() -= tile(k, i).transpose() * tile(k, j);
                });
                const auto t1 = std::chrono::steady_clock::now();
                base      += n;
                work_done += n;
                if (prog) {
                    tune_chunk(chunk, t0, t1);
                    gate.tick(prog, (int)(30 + 60 * work_done / work_total),
                              "INVERTING MATRICES");
                }
            }
            // A column with no trailing update (the last one) still owes a tick.
            if (prog && njobs == 0)
                gate.tick(prog, (int)(30 + 60 * work_done / work_total),
                          "INVERTING MATRICES");
        }
    }

    // Solve A X = B for X. A = U^T U: forward solve U^T Y = B, then back solve U X = Y.
    // Serial, but O(nt^2) tile products against the RHS is not instant at large nt, and it
    // runs on the calling (GUI) thread -- so it reports too, over the last 10% of the bar.
    Eigen::Matrix<S, Eigen::Dynamic, Eigen::Dynamic>
    solve(const Eigen::Matrix<S, Eigen::Dynamic, Eigen::Dynamic>& B,
          const Progress& prog = {}) {
        Mat X = B;
        // Gated: one tick per tile is up to 2*nt calls -- several hundred on a large model
        // -- against an operation that is cheap next to the factor. Ungated, the callback
        // cost alone could exceed the solve it is reporting on.
        ProgressGate gate;
        for (int i = 0; i < nt; ++i) {                 // forward: U^T Y = B
            for (int k = 0; k < i; ++k)
                X.middleRows(roff(i), rows(i)).noalias() -=
                    tile(k, i).transpose() * X.middleRows(roff(k), rows(k));
            auto Xi = X.middleRows(roff(i), rows(i));
            tile(i, i).template triangularView<Eigen::Upper>().transpose().solveInPlace(Xi);
            gate.tick(prog, 90 + 5 * (i + 1) / nt, "SOLVING");
        }
        for (int i = nt - 1; i >= 0; --i) {            // back: U X = Y
            for (int k = i + 1; k < nt; ++k)
                X.middleRows(roff(i), rows(i)).noalias() -=
                    tile(i, k) * X.middleRows(roff(k), rows(k));
            auto Xi = X.middleRows(roff(i), rows(i));
            tile(i, i).template triangularView<Eigen::Upper>().solveInPlace(Xi);
            gate.tick(prog, 95 + 5 * (nt - i) / nt, "SOLVING");
        }
        gate.flush(prog, 100, "SOLVING");              // always land on the end value
        return X;
    }
};

}  // namespace la
