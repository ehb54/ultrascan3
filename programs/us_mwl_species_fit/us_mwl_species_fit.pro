include( ../../gui.pri )

QT += datavisualization

#TEMPLATE = app

TARGET        = us_mwl_species_fit

HEADERS       = us_mwl_species_fit.h   \
                us_customformatter.h \
                us_load_run_noise.h \
                us_mwl_sf_plot3d.h

SOURCES       = us_mwl_species_fit.cpp  \
                us_customformatter.cpp \
                us_load_run_noise.cpp \
                us_mwl_sf_plot3d.cpp

