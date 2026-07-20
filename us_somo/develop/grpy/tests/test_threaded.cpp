// Prove the tiled Cholesky is thread-safe and scales: same result serial vs
// threaded, with timing. A std::thread pool stands in for the Qt/QThreadPool
// backend (structurally identical Parallel implementation).
#include "linalg.hpp"
#include <chrono>
#include <cstdio>
#include <random>
#include <thread>
#include <atomic>

// Minimal work-stealing-free pool: splits [0,n) across hardware threads.
struct StdThreads : la::Parallel {
    unsigned nthreads;
    explicit StdThreads(unsigned n = std::thread::hardware_concurrency()) : nthreads(n ? n : 4) {}
    void for_range(int n, const std::function<void(int)>& f) override {
        if (n <= 1 || nthreads <= 1) { for (int i = 0; i < n; ++i) f(i); return; }
        std::atomic<int> next{0};
        auto worker = [&] { int i; while ((i = next.fetch_add(1)) < n) f(i); };
        std::vector<std::thread> ts;
        for (unsigned t = 0; t < nthreads; ++t) ts.emplace_back(worker);
        for (auto& t : ts) t.join();
    }
};

int main() {
    int n = 2000, b = 256;
    std::mt19937 rng(1);
    std::uniform_real_distribution<double> u(-1, 1);
    Eigen::MatrixXd R(n, n);
    for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) R(i, j) = u(rng);
    Eigen::MatrixXd A = R.transpose() * R + n * Eigen::MatrixXd::Identity(n, n);
    Eigen::MatrixXd B = Eigen::MatrixXd::Random(n, 11);

    auto fill = [&](la::TiledUpperSPD<double>& M) {
        for (int c = 0; c < n; ++c) for (int r = 0; r <= c; ++r) M.at(r, c) = A(r, c);
    };
    auto run = [&](la::Parallel& par, const char* name) {
        la::TiledUpperSPD<double> M(n, b); fill(M);
        auto t0 = std::chrono::steady_clock::now();
        M.factor(par);
        Eigen::MatrixXd X = M.solve(B);
        double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
        double err = (A * X - B).norm() / B.norm();
        std::printf("  %-8s factor+solve %.0f ms   residual=%.2e\n", name, ms, err);
        return X;
    };

    std::printf("n=%d tile=%d, upper storage = %.1f MB (full would be %.1f MB)\n",
                n, b, la::TiledUpperSPD<double>(n, b).bytes() / 1e6, (double)n * n * 8 / 1e6);
    la::Serial ser; StdThreads thr;
    std::printf("threads available: %u\n", thr.nthreads);
    Eigen::MatrixXd Xs = run(ser, "serial");
    Eigen::MatrixXd Xt = run(thr, "threaded");
    double diff = (Xs - Xt).norm() / Xs.norm();
    std::printf("serial vs threaded result diff = %.2e -> %s\n", diff, diff < 1e-12 ? "MATCH" : "DIFFER");
    return diff < 1e-12 ? 0 : 1;
}
