include( ../../gui.pri )

QT           += opengl sql xml network
QT           += datavisualization
QT           += printsupport

TARGET        = us_xpn_viewer


HEADERS       = us_xpn_viewer_gui.h    \
                us_xpn_run_auc.h   \
                us_xpn_run_raw.h  \
                ../us_mwl_species_fit/us_mwl_species_fit.h \
                ../us_mwl_species_fit/us_mwl_sf_plot3d.h \
                ../us_mwl_species_fit/us_load_run_noise.h
                
SOURCES       = us_xpn_viewer_main.cpp  \
                us_xpn_viewer_gui.cpp  \
                us_xpn_run_auc.cpp \
                us_xpn_run_raw.cpp \
                ../us_mwl_species_fit/us_mwl_species_fit.cpp \
                ../us_mwl_species_fit/us_mwl_sf_plot3d.cpp \
                ../us_mwl_species_fit/us_load_run_noise.cpp
                
                                

