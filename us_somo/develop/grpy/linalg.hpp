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
#include <chrono>
#include <cstdio>
#include <functional>
#include <string>
#include <vector>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace la {

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
        } else {
            path = file; unlink_on_close = true;
            mapbytes = nelem * sizeof(S);
            fd = ::open(file.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0600);
            if (fd < 0 || ::ftruncate(fd, mapbytes) != 0) { std::perror("grpy ooc open"); std::abort(); }
            void* p = ::mmap(nullptr, mapbytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            if (p == MAP_FAILED) { std::perror("grpy ooc mmap"); std::abort(); }
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
                base      += n;
                work_done += n;
                if (prog) {
                    prog((int)(30 + 60 * work_done / work_total), "INVERTING MATRICES");
                    // Aim each chunk at ~80 ms: long enough that the chunking overhead and
                    // the callback stay negligible, short enough to feel responsive. Halve
                    // or double rather than solve for it, so a transient stall (swap, another
                    // process) cannot make one measurement set a wild chunk size.
                    const double ms = std::chrono::duration<double, std::milli>(
                                          std::chrono::steady_clock::now() - t0).count();
                    if      (ms <  40.0 && chunk < (1 << 20)) chunk *= 2;
                    else if (ms > 160.0 && chunk > 1)         chunk /= 2;
                }
            }
            // A column with no trailing update (the last one) still owes a tick.
            if (prog && njobs == 0)
                prog((int)(30 + 60 * work_done / work_total), "INVERTING MATRICES");
        }
    }

    // Solve A X = B for X. A = U^T U: forward solve U^T Y = B, then back solve U X = Y.
    // Serial, but O(nt^2) tile products against the RHS is not instant at large nt, and it
    // runs on the calling (GUI) thread -- so it reports too, over the last 10% of the bar.
    Eigen::Matrix<S, Eigen::Dynamic, Eigen::Dynamic>
    solve(const Eigen::Matrix<S, Eigen::Dynamic, Eigen::Dynamic>& B,
          const Progress& prog = {}) {
        Mat X = B;
        for (int i = 0; i < nt; ++i) {                 // forward: U^T Y = B
            for (int k = 0; k < i; ++k)
                X.middleRows(i * b, rows(i)).noalias() -=
                    tile(k, i).transpose() * X.middleRows(k * b, rows(k));
            auto Xi = X.middleRows(i * b, rows(i));
            tile(i, i).template triangularView<Eigen::Upper>().transpose().solveInPlace(Xi);
            if (prog) prog(90 + 5 * (i + 1) / nt, "SOLVING");
        }
        for (int i = nt - 1; i >= 0; --i) {            // back: U X = Y
            for (int k = i + 1; k < nt; ++k)
                X.middleRows(i * b, rows(i)).noalias() -=
                    tile(i, k) * X.middleRows(k * b, rows(k));
            auto Xi = X.middleRows(i * b, rows(i));
            tile(i, i).template triangularView<Eigen::Upper>().solveInPlace(Xi);
            if (prog) prog(95 + 5 * (nt - i) / nt, "SOLVING");
        }
        return X;
    }
};

}  // namespace la
