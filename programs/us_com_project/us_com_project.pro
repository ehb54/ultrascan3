include( ../../gui.pri )

TARGET        = us_comproject
QT           += opengl
QT           += sql
QT           += xml

HEADERS       = us_com_project_gui.h \
                ../us_experiment/us_experiment_gui_optima.h  \
                ../us_analysis_profile/us_analysis_profile.h \
                ../us_xpn_viewer/us_xpn_viewer_gui.h \
                ../us_xpn_viewer/us_xpn_run_raw.h \
                ../us_xpn_viewer/us_xpn_run_auc.h \
                ../us_convert/us_convert_gui.h \
                ../us_convert/us_convertio.h \
                ../us_convert/us_get_run.h \
                ../us_convert/us_experiment.h \
                ../us_convert/us_experiment_gui.h \
                ../us_convert/us_intensity.h \
                ../us_convert/us_selectbox.h \
                ../us_convert/us_select_triples.h \
                ../us_edit/us_edit.h \
                ../us_edit/us_edit_scan.h \
                ../us_edit/us_exclude_profile.h \
                ../us_edit/us_get_edit.h \
                ../us_edit/us_ri_noise.h \
                ../us_edit/us_select_lambdas.h 
                

SOURCES       = us_com_project_main.cpp \
                us_com_project_gui.cpp \
                ../us_experiment/us_experiment_gui_optima.cpp \
                ../us_experiment/us_exp_utils.cpp \
                ../us_experiment/us_proto_ranges.cpp \
                ../us_analysis_profile/us_analysis_profile.cpp \
                ../us_analysis_profile/us_anapro_utils.cpp     \
                ../us_xpn_viewer/us_xpn_viewer_gui.cpp \
                ../us_xpn_viewer/us_xpn_run_raw.cpp \
                ../us_xpn_viewer/us_xpn_run_auc.cpp \
                ../us_convert/us_convert.cpp         \
                ../us_convert/us_convert_gui.cpp     \
                ../us_convert/us_convertio.cpp       \
                ../us_convert/us_get_run.cpp         \
                ../us_convert/us_experiment.cpp      \
                ../us_convert/us_experiment_gui.cpp  \
                ../us_convert/us_intensity.cpp       \
                ../us_convert/us_selectbox.cpp       \
                ../us_convert/us_select_triples.cpp  \
                ../us_edit/us_edit.cpp \
                ../us_edit/us_edit_scan.cpp \
                ../us_edit/us_exclude_profile.cpp \
                ../us_edit/us_get_edit.cpp \
                ../us_edit/us_ri_noise.cpp \
                ../us_edit/us_select_lambdas.cpp
                
