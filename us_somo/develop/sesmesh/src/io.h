// sesmesh: mesh output.  Each writer throws std::runtime_error if a file cannot be written.
#pragma once

#include <string>

#include "mesh.h"

namespace sesmesh {

// MSMS: base.vert (vertices, outward normals, nearest atom, patch type) and base.face.
void write_msms( const std::string& base, const std::string& name, const Mesh& m, int natoms, double probe, double h );

// msroll: base.c3p (vertices with normals, edges, triangles with their edges, component 1) and
// base.c3v (areas and volume; rcoal reads the area and volume of component 1 from its line 13).
void write_c3p( const std::string& base, const std::string& name, const Mesh& m, int natoms, double probe,
                double area, double volume );

// BEST: base.best, N, area and volume, then the nine coordinates of each triangle.
void write_best( const std::string& base, const Mesh& m, double area, double volume );

// ZENO: base.bod, UNITS, TEMP, VISCOSITY, MASS (if mass > 0) and one TRIANGLE line per triangle.
void write_bod( const std::string& base, const Mesh& m, double mass, double temp, double visc );

}  // namespace sesmesh
