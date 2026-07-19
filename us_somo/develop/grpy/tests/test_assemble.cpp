// Verify assemble_tiled (upper triangle) == assemble() full matrix upper triangle.
#include "grpy_core.hpp"
#include <cstdio>
#include <random>
using namespace core;

int main() {
    std::mt19937 rng(7);
    std::uniform_real_distribution<double> pos(-30, 30), rad(1.0, 4.0);
    int N = 8;
    std::vector<Bead> b(N);
    for (auto& x : b) x = { pos(rng), pos(rng), pos(rng), rad(rng), 0 };
    // ensure at least one overlapping (Yamakawa) pair: place bead 1 near bead 0
    b[1] = { b[0].x + b[0].r, b[0].y, b[0].z, b[0].r, 0 };

    // reference full matrix (Phase 2 assembly, validated vs Fortran)
    MatrixXd APP, APQ, AQQ; assemble(b, N, APP, APQ, AQQ);
    int n = 11 * N;
    MatrixXd Mfull(n, n);
    Mfull.topLeftCorner(6*N,6*N) = APP;
    Mfull.topRightCorner(6*N,5*N) = APQ;
    Mfull.bottomLeftCorner(5*N,6*N) = APQ.transpose();
    Mfull.bottomRightCorner(5*N,5*N) = AQQ;

    la::TiledUpperSPD<double> Mt(n, 16);
    la::Serial ser; assemble_tiled<double>(b, N, Mt, ser);

    double worst = 0; int nbad = 0;
    for (int r = 0; r < n; ++r)
        for (int c = r; c < n; ++c) {   // upper triangle only
            double a = Mfull(r, c), t = Mt.at(r, c);
            double d = std::max(std::abs(a), std::abs(t));
            double e = d > 1e-12 ? std::abs(a - t) / d : std::abs(a - t);
            if (e > 1e-12) { if (nbad < 8) std::printf("  mismatch (%d,%d) full=%.6e tiled=%.6e\n", r, c, a, t); ++nbad; }
            worst = std::max(worst, e);
        }
    std::printf("upper-triangle worst rel err = %.2e, mismatches = %d  -> %s\n",
                worst, nbad, nbad ? "FAIL" : "PASS");
    return nbad ? 1 : 0;
}
