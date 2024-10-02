include( ../../gui.pri )

QT += datavisualization
QT += printsupport

TARGET        = us_mwl_species_fit

HEADERS       = us_mwl_species_fit.h   \
                us_load_run_noise.h \
                us_mwl_sf_plot3d.h

SOURCES       = us_mwl_species_fit.cpp  \
                us_load_run_noise.cpp \
                us_mwl_sf_plot3d.cpp

