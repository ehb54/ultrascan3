include( ../../gui.pri )

QT           += xml svg opengl sql printsupport
QT           += datavisualization 

TARGET        = us_query_rmsd

HEADERS       = us_query_rmsd.h                     \
                ../us_fematch/us_adv_dmgamc.h       \
                ../us_fematch/us_advanced_fem.h     \
                ../us_fematch/us_dmga_mc_stats.h    \
                ../us_fematch/us_fematch.h


SOURCES       = us_query_rmsd.cpp                     \
                ../us_fematch/us_adv_dmgamc.cpp       \
                ../us_fematch/us_advanced_fem.cpp     \
                ../us_fematch/us_dmga_mc_stats.cpp    \
                ../us_fematch/us_fematch.cpp
