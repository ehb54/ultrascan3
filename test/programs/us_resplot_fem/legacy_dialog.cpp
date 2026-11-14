// Test-only compilation seam: bypass only the three application declarations.
// The complete production dialog implementation and its real header/moc are used.
// No source copy, constructor replacement, numerical or ownership changes.
#include "owner_doubles.h"
#define US_FEMATCH_H
#define US_ANALYSIS_AUTO_H
#define US_REPORTER_GMP_H
#define DbgLv(a) if (dbg_level >= (a)) qDebug()
#include "../../../programs/us_fematch/us_resplot_fem.cpp"
