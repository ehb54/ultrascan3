// Unit test: tiled SPD Cholesky+solve vs Eigen dense LLT, float and double.
#include "linalg.hpp"
#include <cstdio>
#include <random>

template <typename S>
int check(int n, int b, int nc, double tol) {
    using Mat = Eigen::Matrix<S, Eigen::Dynamic, Eigen::Dynamic>;
    std::mt19937 rng(12345 + n * 31 + b);
    std::uniform_real_distribution<double> u(-1, 1);
    // random SPD: A = R^T R + n*I  (well-conditioned)
    Mat R(n, n);
    for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) R(i, j) = (S)u(rng);
    Mat A = R.transpose() * R + n * Mat::Identity(n, n);
    Mat B(n, nc);
    for (int i = 0; i < n; ++i) for (int j = 0; j < nc; ++j) B(i, j) = (S)u(rng);

    // reference
    Mat Xref = A.llt().solve(B);

    // tiled: fill upper, factor, solve
    la::TiledUpperSPD<S> T(n, b);
    for (int c = 0; c < n; ++c) for (int r = 0; r <= c; ++r) T.at(r, c) = A(r, c);
    la::Serial ser;
    T.factor(ser);
    Mat X = T.solve(B);

    double err = (X - Xref).norm() / Xref.norm();
    bool ok = err < tol;
    std::printf("  n=%-4d b=%-4d nc=%-2d %-7s relerr=%.2e  %s\n",
                n, b, nc, sizeof(S) == 4 ? "float" : "double", err, ok ? "OK" : "FAIL");
    return ok ? 0 : 1;
}

int main() {
    int fails = 0;
    std::puts("double:");
    fails += check<double>(10, 4, 3, 1e-10);
    fails += check<double>(64, 16, 11, 1e-10);
    fails += check<double>(100, 32, 11, 1e-10);   // partial trailing tile
    fails += check<double>(257, 64, 11, 1e-9);    // many tiles, ragged
    fails += check<double>(300, 256, 5, 1e-9);    // 2 tiles
    std::puts("float:");
    fails += check<float>(100, 32, 11, 1e-3);
    fails += check<float>(257, 64, 11, 1e-3);
    std::printf("%s (%d failures)\n", fails ? "FAILURES" : "ALL PASS", fails);
    return fails ? 1 : 0;
}
