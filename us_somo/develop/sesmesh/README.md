# sesmesh

`sesmesh` triangulates the solvent-excluded surface (SES, the molecular surface of Richards and
Connolly) of a set of spheres. It makes closed surface meshes of macromolecules for
boundary-element hydrodynamics ([BEST](https://github.com/ehb54/best-hydro), through its mesh
coarsener rcoal) and for ZENO.

- It writes msroll's `.c3p`/`.c3v` format, which rcoal reads, and MSMS's `.vert`/`.face`.
- It is a free replacement for msroll, which is commercial, and MSMS, whose binary is free only
  for academic use.
- It is written from the published methods in C++17, needs no libraries beyond the C++
  standard library, runs in parallel on `std::thread`s, and is MIT-licensed.

## Building

```bash
make                   # bin/sesmesh
make CXX=clang++       # another compiler
make test              # unit tests
```

sesmesh needs only a C++17 compiler and its standard library. It runs in parallel on
`std::thread`s, one per hardware thread unless `-t` says otherwise.

## Running

```
sesmesh [options] input.xyzr
  -p probe     probe radius in A (default 1.5)
  -g h         grid spacing in A (default 0.2)
  -n N         choose the grid spacing for a mesh of about N triangles (instead of -g)
  -x h2,h3,..  also mesh at these spacings and extrapolate area and volume to h = 0 in h^2
  -r           raw marching-tetrahedra mesh: skip the clean-up
  -c frac      clean-up: collapse edges shorter than frac times h (default 0.3)
  -o base      output base name (default: the input's name without .xyzr)
  -f list      msms, c3p, best, bod or none (default msms,c3p)
  -M mass      mass in Da for the .bod file
  -T temp      temperature in C for the .bod file (default 20)
  -V visc      solvent viscosity in cP for the .bod file (default 1.002)
  -k           keep internal cavities (default: fill them)
  -t n         threads
  -v           time each stage
```

The input has one sphere per line, `x y z r`. Further columns are ignored, and `#` starts a
comment. To reproduce msroll's surface, use the radii msroll would use.

Examples:
- `sesmesh -p 1.5 -n 25000 -x 0.3,0.25,0.2 protein.xyzr` writes `protein.c3p`/`.c3v` and
  `protein.vert`/`.face` with about 25,000 triangles, for rcoal. The area and volume in
  `protein.c3v` are extrapolated from four spacings.
- `sesmesh -p 1.5 -g 0.2 -f bod -M 14305 protein.xyzr` writes a fine mesh for ZENO.

With `-n`, sesmesh also meshes at a finer spacing and warns if the topology differs: a coarse
grid can open a narrow channel or close a thin neck.

## Output

- `base.vert`, `base.face`: MSMS format (rcoal 3.0 reads it with `-s`). In the `.vert` file,
  each vertex has its outward normal, the input number of the nearest atom, and the type of
  patch the vertex is on: 1 contact, 2 reentrant, 3 toroidal.
- `base.c3p`, `base.c3v`: msroll's format, which rcoal reads.
  - `.c3p` holds the vertices with normals, the edges, and the triangles with their edges, all
    in component 1.
  - Line 13 of `.c3v` holds the area and volume that rcoal passes on to BEST. These are the
    h² extrapolated values when `-x` is given, otherwise the mesh's own.

- `base.best`: BEST's triangle format. The first line is the number of triangles, then the
  area and volume (as for `.c3v`), then one line with the nine coordinates of each triangle.
- `base.bod`: ZENO's format: `UNITS A`, `TEMP`, `VISCOSITY`, `MASS` (with `-M`), then one
  `TRIANGLE` line per triangle.

Triangles run counter-clockwise seen from outside. Coordinates are written with 8 decimals.

## Method

1. **Exact surface function.** Let U be the union of the balls of radius r + p (the
   solvent-accessible solid). Then f(x) = p − dist(x, ∂U) inside U and p + dist(x, U)
   outside.
   - f is negative inside the SES and positive outside it, and its zero set is exactly the
     SES, cusps included.
   - The point of ∂U nearest to x lies on one of three features, each not inside any other
     ball:
     - a sphere patch (the radial projection of x),
     - a circle where two spheres meet (the nearest point of that circle),
     - a point where three spheres meet.
   - The exposed arcs of every circle are computed once; their ends are the exposed triple
     points. A spatial hash finds the spheres near a point.
2. **Narrow band.** The grid is cut into bricks of 8×8×8 nodes. f is 1-Lipschitz, so one
   value at a brick's centre shows whether the whole brick is clearly outside, clearly inside,
   or near the surface. Only bricks near the surface are evaluated node by node.
3. **Cavities.** A flood fill from the edge of the grid marks the outside that solvent can
   reach. It moves along the same 14 neighbour directions as the tetrahedron edges. Outside
   regions it does not reach are internal cavities, and they are counted as inside.
4. **Marching tetrahedra.** Each grid cube is split into the six Freudenthal tetrahedra
   around its main diagonal. This split matches between neighbouring cubes, so the cut is a
   closed, oriented 2-manifold. No triangle leaves its tetrahedron, so the mesh cannot
   intersect itself.
   - Each vertex is a root of the exact f on its edge (false position with the Illinois
     step), so it lies on the true surface.
   - A vertex is kept at least 10⁻⁴ of its edge from either end, so that a surface through a
     grid node gives no zero-area triangles.
   - The grid is shifted by irrational fractions of h, so that its nodes do not line up with
     the sphere centres.
5. **Clean-up** (skipped with `-r`). Marching tetrahedra leaves about a quarter of its
   triangles with an angle under 20°: needles where the surface passes close to a grid node,
   and caps. Three operations remove them, repeated a few times, and none moves a vertex off
   the exact surface:
   - **Edge collapse.** An edge shorter than 0.3 h becomes one vertex, at either end or at the
     midpoint projected onto the surface, whichever leaves the best angles.
     - The projection is one Newton step on f, exact while the nearest feature stays the same.
     - The link condition keeps the mesh a manifold of the same topology.
     - A collapse is made only if no triangle turns over and the smallest angle around the
       edge improves or stays above 20°.
   - **Edge flip.** The shared edge of two nearly coplanar triangles is swapped for the other
     diagonal when that raises their smallest angle.
   - **Tangential smoothing.** A vertex moves halfway to its neighbours' centroid within the
     tangent plane and is projected back onto the surface. The move is kept only if it raises
     the smallest angle around the vertex.
   - Flips and smoothing work only near triangles with an angle under 30°. A degenerate cap
     (an angle near 180°) can also lose either short edge at its obtuse corner, whatever its
     length.
   - All three run in parallel:
     - Collapses and flips run over spatial cells coloured like a 3-D checkerboard. An
       operation runs in its cell only if every vertex it touches is in that cell.
     - Smoothing moves one colour class of vertices at a time, vertices that share no
       triangle.
6. **Area and volume** come from the mesh. Their error falls as h², so meshes at a few
   spacings extrapolate to the smooth surface's values (`-x`). The extrapolation uses the
   meshes before clean-up, whose error follows h² most closely.

## Accuracy

`make test` runs the unit tests:
- f against the closed form for one sphere, and against a brute-force distance to a dense
  sampling of ∂U for random clusters;
- meshes of one, two and three spheres against the closed-form area and volume (h = 0.2 to
  0.1 Å, extrapolated in h²). One case pinches to a cusp;
- cavity filling;
- closed-manifold checks on random clusters;
- the clean-up: every cleaned mesh keeps its topology, its area and volume are no further from
  the closed form than the raw mesh's, and no angle is under 10°;
- the writers: what they write reads back.

The extrapolated area and volume of the analytic cases agree with the closed forms to about
10⁻⁵. In the cusp case the area agrees to 7×10⁻⁴: near a conical tip the error goes as
h² log h.

Lysozyme (PDB 1AKI) with msroll's 1001 spheres and a 1.5 Å probe, compared with msroll's
analytic SES (area 5633.206 Å², volume 23368.748 Å³). Every mesh from h = 0.5 to 0.05 Å is
one closed manifold with Euler characteristic 2. Before clean-up:

| h (Å) | triangles | area | volume | wall time (64 threads) |
|---|---|---|---|---|
| 0.5 | 204,532 | −0.136% | −0.026% | 0.5 s |
| 0.25 | 821,140 | −0.035% | −0.007% | 0.7 s |
| 0.2 | 1,283,832 | −0.024% | −0.004% | 0.9 s |
| 0.1 | 5,139,668 | −0.006% | −0.001% | 2.5 s |
| 0.05 | 20,562,584 | −0.002% | −0.0003% | 9.3 s |
| h² fit, 0.2/0.15/0.1 | | −0.0003% | −0.00002% | |

The clean-up removes about a third of the triangles, and area and volume change by about
10⁻⁴ or less. Before it, a quarter of the triangles have an angle under 20°:

| h (Å) | triangles | smallest angle | under 20° | area | volume | clean-up time (16 threads) |
|---|---|---|---|---|---|---|
| 0.5 | 131,448 | 18.7° | 2 | −0.163% | −0.022% | 0.3 s |
| 0.25 | 527,536 | 21.8° | 0 | −0.043% | −0.006% | 1.1 s |
| 0.2 | 824,900 | 22.7° | 0 | −0.028% | −0.004% | 1.4 s |

ZENO 5.3.1 (10⁷ walks and interior samples) was run on the full meshes of three programs,
without coarsening, and each program's results were extrapolated in 1/N:

| mesher | meshes | triangles | D (10⁻⁶ cm²/s) | [η] (cm³/g) |
|---|---|---|---|---|
| sesmesh, before clean-up | 13 | 35k–3.3M | 1.11515 ± 0.00009 | 3.1777 ± 0.0005 |
| NanoShaper | 6 | 38k–1.08M | 1.11528 ± 0.00012 | 3.1768 ± 0.0007 |
| msroll | 12 | 17k–43k | 1.11544 ± 0.00028 | 3.1757 ± 0.0033 |

They agree within 0.03% in D and 0.06% in [η]. ZENO on the cleaned 0.5–0.2 Å meshes gives D
1.11495–1.11532 and [η] 3.1775–3.1815, within its statistical error (0.02% and 0.055%) of
these limits. msroll's error is larger because it wrote no lysozyme mesh finer than 43k
triangles.

### Five structures, four meshers

The structures are crambin (1CRN), lysozyme (1AKI), pepsin (4PEP, with a deep cleft), the
hemoglobin tetramer (1A3N, with internal cavities) and the B-DNA dodecamer 1BNA (with its
major and minor grooves).
- **Inputs:** protein or DNA atoms only (no water), with msroll's `best.radii` spheres and a
  1.5 Å probe. The DNA atoms get msroll's nucleotide radii once renamed to the older PDB names
  its atom table uses (residues A, C, G, T; `C1*`, `O1P`, `C5M` and so on).
- **Cavities:** filled. For msroll and NanoShaper, the outer surface is the largest component.
- **Area and volume:** sesmesh's h² extrapolation is compared with msroll's analytic SES:

| structure | atoms | msroll area (Å²) | sesmesh | msroll volume (Å³) | sesmesh |
|---|---|---|---|---|---|
| 1CRN | 327 | 2667.783 | −0.0002% | 8184.656 | −0.0003% |
| 1AKI | 1001 | 5633.206 | −0.0005% | 23368.748 | +0.0001% |
| 4PEP | 2423 | 11209.040 | −0.0006% | 56016.768 | −0.0001% |
| 1A3N | 4370 | 19509.094 | −0.004% | 102582.214 | −0.0000% |
| 1BNA | 486 | 4047.108 | −0.005% | 11326.144 | +0.0000% |

ZENO 5.3.1 was run on 107 full meshes, with no coarsening:
- msroll at finenesses 0.60 to 0.36, as far as it ran without crashing;
- NanoShaper at grid scales 1.5–6, plus 8 for crambin and DNA;
- sesmesh at h = 1.0 to 0.25;
- EDTSurf (VCMC) at scales 2–5.

Each mesher's results were extrapolated in 1/N and are shown against the mean of the sesmesh
and NanoShaper limits:

| structure | sesmesh D, [η] | NanoShaper | msroll | EDTSurf |
|---|---|---|---|---|
| 1CRN | −0.01%, +0.02% | +0.01%, −0.02% | −0.06%, +0.04% | +1.85%, −5.35% |
| 1AKI | +0.00%, −0.00% | −0.00%, +0.00% | +0.01%, −0.07% | +1.44%, −4.17% |
| 4PEP | +0.01%, −0.05% | −0.01%, +0.05% | −0.02%, −0.03% | +1.16%, −3.35% |
| 1A3N | +0.01%, −0.03% | −0.01%, +0.03% | −0.05%, +0.06% | +1.00%, −2.91% |
| 1BNA | −0.01%, +0.03% | +0.01%, −0.03% | −0.11%, +0.23% | +1.76%, −4.97% |

- **sesmesh and NanoShaper** agree within ZENO's statistical error, and so do their finest
  single meshes (within 0.02% in D and 0.06% in [η]).
- **msroll** extrapolates to the same values for the proteins. Its finest meshes (at most 120k
  triangles) are off by +0.1–0.2% in D and −0.3–0.6% in [η].
- **msroll on DNA** extrapolates less well: its meshes stop at 28k triangles, and its limit
  moves by about 0.1% with the meshes chosen for the fit. Its analytic area and volume agree
  with sesmesh's within 0.005%, so the surfaces are the same.
- **EDTSurf** is biased and does not converge: about +1–2% in D and −3–5% in [η].

## Speed

Wall time on a 64-core Linux host that other jobs were also using. sesmesh and NanoShaper ran
on 16 threads; msroll and EDTSurf use one. Grid spacing is 0.5 and 0.25 Å for sesmesh and
NanoShaper (grid scale 2 and 4), and scale 2 and 4 for EDTSurf; msroll used fineness 0.60 and
0.44. NanoShaper runs in Docker, so its times include about 0.7 s to start the container.

| structure | sesmesh cleaned | sesmesh raw | NanoShaper | msroll | EDTSurf |
|---|---|---|---|---|---|
| 1CRN | 0.34 s / 0.92 s | 0.15 / 0.37 | 0.79 / 1.15 | 0.20 / 0.33 | 0.09 / 0.65 |
| 1AKI | 0.66 / 1.97 | 0.24 / 0.71 | 0.96 / 1.43 | 0.97 / 2.02 | 0.20 / 1.56 |
| 4PEP | 1.25 / 3.28 | 0.45 / 1.27 | 1.23 / 2.74 | 3.84 / 9.81 | 0.50 / 3.57 |
| 1A3N | 1.60 / 5.26 | 0.70 / 1.92 | 1.62 / 3.48 | 13.4 / 46.7 | 0.80 / 6.66 |
| 1BNA | 0.52 / 1.22 | 0.24 / 0.57 | 0.82 / 1.35 | 0.39 / 0.86 | 0.12 / 0.91 |

Mesh sizes at the finer setting for 1A3N:

| sesmesh cleaned | sesmesh raw | NanoShaper | msroll | EDTSurf |
|---|---|---|---|---|
| 1.81M triangles | 2.81M | 0.95M | 0.12M | 0.50M |

On one thread sesmesh is slower: 1A3N at 0.25 Å takes 19 s raw and 40 s cleaned, against 5.2 s
for NanoShaper. Its time goes into evaluating the exact surface function, about 12 million
times.

## Limitations

- **Fine-scale topology.** Thin sheets of solid between nearly touching probe positions can
  have pinholes much smaller than the grid. Whether a grid resolves them depends on h, so the
  Euler characteristic of a large protein can change with h: pepsin's is −2 down to 0.3 Å and
  −8 to −14 below that. Area, volume and hydrodynamics are not affected measurably. `-n` warns
  when its grid and a finer one disagree.
- **One thread.** On one thread sesmesh is slower than NanoShaper (see Speed).
- **rcoal.**
  - rcoal 2.0 as distributed reads cleaned meshes. However, some of its outputs have
    non-manifold or misoriented edges.
  - With the fixes in ehb54/best-hydro#1 and ehb54/best-hydro#2 its outputs are closed. It
    coarsens a cleaned mesh by at most about 12 times, so its outputs follow the input. For
    lysozyme with SOMO's targets (2,152–3,588 triangles):

    | `-n` | rcoal outputs |
    |---|---|
    | 12,000 | 1,944–2,748 |
    | 25,000 | 2,674–3,566 |
    | 40,000 | 3,448–4,404 |

  - BEST through rcoal, extrapolated linearly in 1/N:
    - At a 30,000-triangle input, sesmesh agrees with msroll at the same input size within
      0.03% in D and [η].
    - The extrapolation over rcoal's narrow range of N carries a systematic of a few tenths of
      a percent in D, and up to about 1% in [η]. It depends on the input mesh, for msroll's
      meshes as well. ZENO on a full mesh does not have it.

## References

- M. L. Connolly, J. Appl. Cryst. 16, 548–558 (1983): the solvent-excluded surface.
- W. E. Lorensen and H. E. Cline, Computer Graphics 21(4), 163–169 (1987): marching cubes. This
  code uses marching tetrahedra on the Freudenthal (Kuhn) subdivision instead.
- D. Xu and Y. Zhang, PLoS ONE 4, e8140 (2009): computing the SES as a distance field on a
  grid.

## Licence

MIT; see `LICENSE`.
