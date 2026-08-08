// Per-bead solvent exposure, for shell reduction.
//
// WHY. GRPY costs O((11N)^3), so the bead count is worth far more than any cheaper
// approximation of the hydrodynamics themselves. The physical licence for dropping beads
// is that a dense packing is hydrodynamically SCREENED: interior beads sit in essentially
// stagnant fluid and carry almost no force. (The same fact defeats the Kirkwood/
// single-scattering approximation, which assumes every bead carries F/N and lands ~27%
// high against GRPY on these models -- here we exploit screening instead of fighting it,
// by removing the beads rather than trying to down-weight them.)
//
// This generalizes SOMO's existing ASA buried-bead exclusion (grpy_bead_inclusion) from a
// binary include/exclude to a continuous knob, so it can be driven by a convergence test
// that reports its own error -- see grpy_shell.hpp.
//
// Shrake-Rupley: sample K points on each bead's surface (inflated by a probe radius) and
// count the fraction not contained in any other bead.
//
// Cost is negligible in context: measured 0.6-26 ms against 79-14143 ms for the
// corresponding solve (<=1%). This is not a hot spot; do not micro-optimize it at the
// expense of clarity.
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>
#include "grpy_core.hpp"

namespace grpy {
namespace shell {

// Fibonacci sphere -- near-uniform K points on the unit sphere, without the pole
// clustering of a naive lat/long grid.
inline std::vector<std::array<double, 3>> unit_points(int K) {
    std::vector<std::array<double, 3>> p(K);
    const double ga = M_PI * (3.0 - std::sqrt(5.0));       // golden angle
    for (int i = 0; i < K; ++i) {
        double z = 1.0 - 2.0 * (i + 0.5) / K;
        double r = std::sqrt(std::max(0.0, 1.0 - z * z));
        double th = ga * i;
        p[i] = {r * std::cos(th), r * std::sin(th), z};
    }
    return p;
}

// exposure[i] in [0,1] = fraction of bead i's (probe-inflated) surface not buried inside
// another bead. probe > 0 inflates every sphere by the probe radius.
//
// The probe is essential, not cosmetic: at probe 0 almost nothing registers as buried,
// because bare bead spheres nearly always retain some exposed surface. A water-sized
// probe (1.4 A) is what makes burial meaningful.
inline std::vector<double> exposure(const std::vector<core::Bead>& b, int N,
                                    int K = 64, double probe = 1.4) {
    auto pts = unit_points(K);
    std::vector<double> ex(N, 1.0);
    double rmax = 0;
    for (int i = 0; i < N; ++i) rmax = std::max(rmax, b[i].r);
    rmax += probe;

    std::vector<int> nb;
    for (int i = 0; i < N; ++i) {
        const double ai = b[i].r + probe;
        nb.clear();
        for (int j = 0; j < N; ++j) {                      // neighbours that can reach i's surface
            if (j == i) continue;
            double dx = b[i].x - b[j].x, dy = b[i].y - b[j].y, dz = b[i].z - b[j].z;
            double reach = ai + rmax;
            if (dx * dx + dy * dy + dz * dz <= reach * reach) nb.push_back(j);
        }
        int free_pts = 0;
        for (int k = 0; k < K; ++k) {
            double px = b[i].x + ai * pts[k][0];
            double py = b[i].y + ai * pts[k][1];
            double pz = b[i].z + ai * pts[k][2];
            bool buried = false;
            for (int j : nb) {
                double aj = b[j].r + probe;
                double dx = px - b[j].x, dy = py - b[j].y, dz = pz - b[j].z;
                if (dx * dx + dy * dy + dz * dz < aj * aj) { buried = true; break; }
            }
            if (!buried) ++free_pts;
        }
        ex[i] = (double)free_pts / K;
    }
    return ex;
}

// Keep the most-exposed ceil(frac*N) beads.
//
// PREFER THIS over a fixed exposure threshold. A threshold is not comparable across model
// types: at probe 1.4 the fully-buried fraction ranges from ~1% (surface-only models) to
// ~50% (overlapping-bead models), so one threshold yields wildly different bead counts and
// therefore wildly different cost. Ranking and taking a target fraction controls the
// O(N^3) cost directly and behaves the same for every model type.
// Indices of the beads to keep, most-exposed first. The selection lives here rather than in
// reduce_top_frac so a caller can learn WHICH beads were kept, not merely how many -- needed
// to write out the reduced model, and to build the reduced bead list without searching.
inline std::vector<size_t> reduce_top_frac_idx(const std::vector<core::Bead>& b,
                                               const std::vector<double>& ex, double frac) {
    const size_t N = b.size();
    size_t keep = (size_t)std::ceil(frac * N);
    if (keep > N) keep = N;
    if (keep < 4) keep = std::min<size_t>(4, N);
    std::vector<size_t> idx(N);
    for (size_t i = 0; i < N; ++i) idx[i] = i;
    if (keep >= N) return idx;                     // everything, in original order
    // Rank by exposure descending; ties broken by radius then index so the selection is
    // deterministic (exposure is quantized to K sample points, so ties are common).
    std::partial_sort(idx.begin(), idx.begin() + keep, idx.end(),
                      [&](size_t i, size_t j) {
                          if (ex[i] != ex[j]) return ex[i] > ex[j];
                          if (b[i].r != b[j].r) return b[i].r > b[j].r;
                          return i < j;
                      });
    idx.resize(keep);
    return idx;
}

inline std::vector<core::Bead> reduce_top_frac(const std::vector<core::Bead>& b,
                                               const std::vector<double>& ex, double frac) {
    if ((size_t)std::ceil(frac * b.size()) >= b.size()) return b;
    std::vector<size_t> idx = reduce_top_frac_idx(b, ex, frac);
    std::vector<core::Bead> out;
    out.reserve(idx.size());
    for (size_t k : idx) out.push_back(b[k]);
    return out;
}

}  // namespace shell
}  // namespace grpy
