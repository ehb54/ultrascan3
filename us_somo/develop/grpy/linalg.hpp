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
    void factor(Parallel& par, const Progress& prog = {}) {
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
            par.for_range((int)jobs.size(), [&](int t) {
                int i = jobs[t].first, j = jobs[t].second;
                auto Tij = tile(i, j);
                Tij.noalias() -= tile(k, i).transpose() * tile(k, j);
            });
            if (prog) prog(30 + 60 * (k + 1) / nt, "INVERTING MATRICES");
        }
    }

    // Solve A X = B for X. A = U^T U: forward solve U^T Y = B, then back solve U X = Y.
    Eigen::Matrix<S, Eigen::Dynamic, Eigen::Dynamic>
    solve(const Eigen::Matrix<S, Eigen::Dynamic, Eigen::Dynamic>& B) {
        Mat X = B;
        for (int i = 0; i < nt; ++i) {                 // forward: U^T Y = B
            for (int k = 0; k < i; ++k)
                X.middleRows(i * b, rows(i)).noalias() -=
                    tile(k, i).transpose() * X.middleRows(k * b, rows(k));
            auto Xi = X.middleRows(i * b, rows(i));
            tile(i, i).template triangularView<Eigen::Upper>().transpose().solveInPlace(Xi);
        }
        for (int i = nt - 1; i >= 0; --i) {            // back: U X = Y
            for (int k = i + 1; k < nt; ++k)
                X.middleRows(i * b, rows(i)).noalias() -=
                    tile(i, k) * X.middleRows(k * b, rows(k));
            auto Xi = X.middleRows(i * b, rows(i));
            tile(i, i).template triangularView<Eigen::Upper>().solveInPlace(Xi);
        }
        return X;
    }
};

}  // namespace la
