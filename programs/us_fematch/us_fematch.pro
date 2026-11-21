include( ../../gui.pri )

QT           += xml svg opengl sql printsupport
QT           += datavisualization

TARGET        = us_fematch

HEADERS       = us_fematch.h          \
                us_advanced_fem.h     \
                us_adv_dmgamc.h       \
                us_dmga_mc_stats.h

SOURCES       = us_fematch_main.cpp     \
                us_fematch.cpp          \
                us_advanced_fem.cpp     \
                us_adv_dmgamc.cpp       \
                us_dmga_mc_stats.cpp
