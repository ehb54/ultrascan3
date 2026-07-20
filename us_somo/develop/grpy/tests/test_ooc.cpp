// Out-of-core (mmap-backed) tiled Cholesky: identical result to in-core, and the
// factorization runs against a disk-backed file (RAM stays bounded when the matrix
// exceeds RAM -- best measured on a large-memory box; here we prove correctness +
// that storage is genuinely the file, not the heap).
#include "linalg.hpp"
#include "parallel_std.hpp"
#include <cstdio>
#include <random>
#include <sys/stat.h>

int main() {
    int n = 900, b = 128;
    std::mt19937 rng(5);
    std::uniform_real_distribution<double> u(-1, 1);
    Eigen::MatrixXd R(n, n);
    for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) R(i, j) = u(rng);
    Eigen::MatrixXd A = R.transpose() * R + n * Eigen::MatrixXd::Identity(n, n);
    Eigen::MatrixXd B = Eigen::MatrixXd::Random(n, 11);
    la::StdThreads par(8);

    auto fill = [&](la::TiledUpperSPD<double>& M) {
        for (int c = 0; c < n; ++c) for (int r = 0; r <= c; ++r) M.at(r, c) = A(r, c);
    };
    // in-core
    la::TiledUpperSPD<double> Mi(n, b);
    fill(Mi); Mi.factor(par);
    Eigen::MatrixXd Xi = Mi.solve(B);

    // out-of-core: backing file
    const char* path = "/tmp/grpy_ooc_test.bin";
    Eigen::MatrixXd Xo;
    size_t fsize = 0;
    {
        la::TiledUpperSPD<double> Mo(n, b, path);
        printf("out_of_core=%d  backing file exists during use: ", (int)Mo.out_of_core());
        struct stat st; fsize = (stat(path, &st) == 0) ? (size_t)st.st_size : 0;
        printf("%s (%.1f MB, expected %.1f)\n", fsize ? "yes" : "NO",
               fsize / 1e6, Mo.bytes() / 1e6);
        fill(Mo); Mo.factor(par);
        Xo = Mo.solve(B);
    }  // destructor munmaps + unlinks
    struct stat st; bool cleaned = (stat(path, &st) != 0);

    double diff = (Xi - Xo).norm() / Xi.norm();
    double resid = (A * Xo - B).norm() / B.norm();
    printf("in-core vs out-of-core result diff = %.2e  residual=%.2e  file cleaned=%s\n",
           diff, resid, cleaned ? "yes" : "no");
    bool ok = diff < 1e-12 && resid < 1e-10 && fsize > 0 && cleaned;
    printf("%s\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
