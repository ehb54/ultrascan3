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
                ../us_mwl_species_fit/us_load_run_noise.h \
                ../us_convert/us_convert.h \
                ../us_convert/us_experiment.h \   
                ../us_convert/us_experiment_gui.h \
                ../us_convert/us_convert_gui.h \
                ../us_convert/us_convertio.h \      
                ../us_convert/us_get_run.h \
                ../us_convert/us_intensity.h \
                ../us_convert/us_selectbox.h \
                ../us_convert/us_select_triples.h \
                ../us_analysis_profile/us_analysis_profile.h 
                
SOURCES       = us_xpn_viewer_main.cpp  \
                us_xpn_viewer_gui.cpp  \
                us_xpn_run_auc.cpp \
                us_xpn_run_raw.cpp \
                ../us_mwl_species_fit/us_mwl_species_fit.cpp \
                ../us_mwl_species_fit/us_mwl_sf_plot3d.cpp \
                ../us_mwl_species_fit/us_load_run_noise.cpp \
                ../us_convert/us_convert.cpp \
                ../us_convert/us_experiment.cpp \   
                ../us_convert/us_experiment_gui.cpp \
                ../us_convert/us_convert_gui.cpp \
                ../us_convert/us_convertio.cpp \      
                ../us_convert/us_get_run.cpp \
                ../us_convert/us_intensity.cpp \
                ../us_convert/us_selectbox.cpp \
                ../us_convert/us_select_triples.cpp \
                ../us_analysis_profile/us_analysis_profile.cpp \
                ../us_analysis_profile/us_anapro_utils.cpp
                
                
                
                                

