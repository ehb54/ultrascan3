include( ../../gui.pri )

QT += datavisualization
QT += printsupport
QT           += sql

TARGET        = us_mwl_species_fit

HEADERS       = us_mwl_species_fit.h   \
                us_load_run_noise.h \
                us_mwl_sf_plot3d.h \
                ../us_analysis_profile/us_analysis_profile.h 
                

SOURCES       = us_mwl_species_fit_main.cpp  \
                us_mwl_species_fit.cpp  \
                us_load_run_noise.cpp \
                us_mwl_sf_plot3d.cpp \
                ../us_analysis_profile/us_analysis_profile.cpp \
                ../us_analysis_profile/us_anapro_utils.cpp
                
                

