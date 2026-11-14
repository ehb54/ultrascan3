// sesmesh rcoal: the files rcoal reads and writes, as the Fortran reads and writes them.
//
// Copyright (c) 1998-2025 Sergio R. Aragon (Coalesce and rcoal, in ehb54/best-hydro)
// Copyright (c) 2026 Emre H. Brookes (this port, and the fixes of best-hydro PRs #1 and #2)
// MIT licence: see LICENSE.
#pragma once

#include <ostream>
#include <string>
#include <vector>

#include "coalesce.h"

namespace sesmesh::rcoal {

// The output base name: the input name up to its extension (.c3p, or .fac of .face).
std::string base_name( const std::string& input, bool sanner );

// msroll's name.c3p, and from name.c3v the volume and molecular area of component 1.
Surface read_c3p( const std::string& name );

// MSMS's name.face and name.vert, and from name.msms.log the analytical area and the numerical
// volume and area.
Surface read_msms( const std::string& face );

// The summary that scoalesce prints after each triangulation; his: the .his file's version of
// it, whose labels differ slightly.  v3: rcoal 3.0's version number.
void write_summary( std::ostream& os, const Surface& s, const Stats& st, bool his, bool v3 );

// Writes base_NNNNN (BEST's input; NNNNN is the number of triangles, modulo 100000) and
// base_NNNNN.his, as rcoal does, and appends their names to written.  If a triangle has two
// vertices at the same position as written, it writes nothing and returns the number of such
// triangles; otherwise it returns 0.  msg receives rcoal's messages: the alert for triangles
// under 20% of the average area, and the refusal.  nsmall, if not null, receives the number of
// those small triangles.  Throws std::runtime_error if a file cannot be written.
int write_result( const std::string& base, const Surface& s, const Result& r, double edgfrac, bool v3, std::ostream& msg,
                  std::vector<std::string>& written, int* nsmall = nullptr );

}  // namespace sesmesh::rcoal
