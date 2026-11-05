#include "us_hydrodyn_grid_volume.h"

#include <algorithm>
#include <cmath>

namespace somo_volume {

namespace {

// Offsets of every voxel within `radius` of the origin, as flat (dx,dy,dz) triples.
struct Ball {
    std::vector<int> dx, dy, dz;
};

Ball make_ball(double radius, double grid) {
    Ball b;
    const int n = (int) std::floor(radius / grid);
    const double r2 = radius * radius;
    for (int i = -n; i <= n; ++i) {
        for (int j = -n; j <= n; ++j) {
            for (int k = -n; k <= n; ++k) {
                const double d2 = (i * i + j * j + k * k) * grid * grid;
                if (d2 <= r2) {
                    b.dx.push_back(i);
                    b.dy.push_back(j);
                    b.dz.push_back(k);
                }
            }
        }
    }
    return b;
}

} // namespace

Result grid_volume(const std::vector<Sphere>& atoms, const Options& opt) {
    Result out;
    out.probe = opt.probe;
    out.grid = opt.grid;
    if (atoms.empty() || opt.grid <= 0 || opt.probe < 0) {
        return out;
    }

    // Bounding box: every atom's blocked sphere is r+probe, and E can extend at most that far.
    // One extra probe of slack keeps the dilation from clipping at the wall.
    double rmax = 0;
    for (const Sphere& a : atoms) {
        rmax = std::max(rmax, a.r);
    }
    const double pad = rmax + 2.0 * opt.probe + 2.0 * opt.grid;
    double lo[3] = {atoms[0].x, atoms[0].y, atoms[0].z};
    double hi[3] = {atoms[0].x, atoms[0].y, atoms[0].z};
    for (const Sphere& a : atoms) {
        const double c[3] = {a.x, a.y, a.z};
        for (int d = 0; d < 3; ++d) {
            lo[d] = std::min(lo[d], c[d]);
            hi[d] = std::max(hi[d], c[d]);
        }
    }
    for (int d = 0; d < 3; ++d) {
        lo[d] -= pad;
        hi[d] += pad;
    }

    const long nx = (long) std::ceil((hi[0] - lo[0]) / opt.grid) + 1;
    const long ny = (long) std::ceil((hi[1] - lo[1]) / opt.grid) + 1;
    const long nz = (long) std::ceil((hi[2] - lo[2]) / opt.grid) + 1;
    const long n = nx * ny * nz;
    out.total = n;
    if (n <= 0 || n > opt.max_voxels) {
        return out;                       // caller must check ok
    }

    // ---- 1. blocked = within (r_i + probe) of some atom. accessible A = !blocked.
    std::vector<uint8_t> blocked((size_t) n, 0);
    for (const Sphere& a : atoms) {
        const double rr = a.r + opt.probe;
        const double rr2 = rr * rr;
        const long i0 = std::max(0L, (long) std::floor((a.x - rr - lo[0]) / opt.grid));
        const long i1 = std::min(nx - 1, (long) std::ceil((a.x + rr - lo[0]) / opt.grid));
        const long j0 = std::max(0L, (long) std::floor((a.y - rr - lo[1]) / opt.grid));
        const long j1 = std::min(ny - 1, (long) std::ceil((a.y + rr - lo[1]) / opt.grid));
        const long k0 = std::max(0L, (long) std::floor((a.z - rr - lo[2]) / opt.grid));
        const long k1 = std::min(nz - 1, (long) std::ceil((a.z + rr - lo[2]) / opt.grid));
        for (long i = i0; i <= i1; ++i) {
            const double ddx = lo[0] + i * opt.grid - a.x;
            for (long j = j0; j <= j1; ++j) {
                const double ddy = lo[1] + j * opt.grid - a.y;
                const double xy = ddx * ddx + ddy * ddy;
                if (xy > rr2) {
                    continue;
                }
                for (long k = k0; k <= k1; ++k) {
                    const double ddz = lo[2] + k * opt.grid - a.z;
                    if (xy + ddz * ddz <= rr2) {
                        blocked[(size_t) ((i * ny + j) * nz + k)] = 1;
                    }
                }
            }
        }
    }

    // probe == 0: E is exactly the blocked set (the vdW union). No dilation to do.
    if (opt.probe <= 0.0) {
        long cnt = 0;
        for (long idx = 0; idx < n; ++idx) {
            cnt += blocked[(size_t) idx];
        }
        out.voxels = cnt;
        out.volume = cnt * opt.grid * opt.grid * opt.grid;
        out.ok = true;
        return out;
    }

    // ---- 2. Reclaim: E = blocked minus everything within `probe` of an accessible voxel.
    // Only accessible voxels on the A/blocked boundary can be the nearest A voxel to a blocked
    // one, so dilating the boundary is equivalent to dilating all of A -- and it is a surface
    // rather than a volume, which is what makes this affordable at a 0.25 A grid.
    const Ball ball = make_ball(opt.probe, opt.grid);
    std::vector<uint8_t> reclaimed((size_t) n, 0);
    const size_t nball = ball.dx.size();

    for (long i = 0; i < nx; ++i) {
        for (long j = 0; j < ny; ++j) {
            for (long k = 0; k < nz; ++k) {
                const size_t idx = (size_t) ((i * ny + j) * nz + k);
                if (blocked[idx]) {
                    continue;                       // not accessible
                }
                // boundary test: 6-neighbourhood touches a blocked voxel. A voxel on the box
                // wall is treated as boundary so the outside world still dilates inward.
                bool boundary =
                    (i == 0 || j == 0 || k == 0 || i == nx - 1 || j == ny - 1 || k == nz - 1);
                if (!boundary) {
                    boundary =
                        blocked[idx - ny * nz] || blocked[idx + ny * nz] ||
                        blocked[idx - nz]      || blocked[idx + nz]      ||
                        blocked[idx - 1]       || blocked[idx + 1];
                }
                if (!boundary) {
                    continue;
                }
                for (size_t b = 0; b < nball; ++b) {
                    const long ii = i + ball.dx[b];
                    const long jj = j + ball.dy[b];
                    const long kk = k + ball.dz[b];
                    if (ii < 0 || jj < 0 || kk < 0 || ii >= nx || jj >= ny || kk >= nz) {
                        continue;
                    }
                    reclaimed[(size_t) ((ii * ny + jj) * nz + kk)] = 1;
                }
            }
        }
    }

    long cnt = 0;
    for (long idx = 0; idx < n; ++idx) {
        if (blocked[(size_t) idx] && !reclaimed[(size_t) idx]) {
            ++cnt;
        }
    }
    out.voxels = cnt;
    out.volume = cnt * opt.grid * opt.grid * opt.grid;
    out.ok = true;
    return out;
}

} // namespace somo_volume
