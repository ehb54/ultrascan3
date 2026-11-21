include( ../../gui.pri )

TARGET        = us_autoflow_analysis
QT           += xml
QT           += sql
QT           += svg opengl printsupport
QT           += datavisualization

win32:LIBS   += -lpsapi

HEADERS       = us_autoflow_analysis.h                       \
                ../us_edited_data_loaders.h                  \
                ../us_2dsa/us_2dsa.h                         \
                ../us_2dsa/us_2dsa_process.h                 \
                ../us_2dsa/us_adv_analysis_2d.h              \
                ../us_2dsa/us_analysis_control_2d.h          \
                ../us_2dsa/us_plot_control_2d.h              \
                ../us_2dsa/us_resplot_2d.h                   \
                ../us_2dsa/us_show_norm.h                    \
                ../us_2dsa/us_worker_2d.h                    \
                ../us_2dsa/us_worker_calcnorm.h              \
                ../us_abde/us_norm_profile.h                 \
                ../us_analysis_profile/us_analysis_profile.h \
                ../us_edit/us_edit.h                         \
                ../us_edit/us_edit_scan.h                    \
                ../us_edit/us_exclude_profile.h              \
                ../us_edit/us_get_edit.h                     \
                ../us_edit/us_ri_noise.h                     \
                ../us_edit/us_select_lambdas.h               \
                ../us_fit_meniscus/us_fit_meniscus.h         \
                ../us_mwl_species_fit/us_load_run_noise.h    \
                ../us_mwl_species_fit/us_mwl_sf_plot3d.h     \
                ../us_mwl_species_fit/us_mwl_species_fit.h   \
                ../us_mwl_species_sim/us_mwl_species_sim.h


SOURCES       = us_autoflow_analysis_main.cpp                  \
                us_autoflow_analysis.cpp                       \
                ../us_edited_data_loaders.cpp                  \
                ../us_2dsa/us_2dsa.cpp                         \
                ../us_2dsa/us_2dsa_process.cpp                 \
                ../us_2dsa/us_adv_analysis_2d.cpp              \
                ../us_2dsa/us_analysis_control_2d.cpp          \
                ../us_2dsa/us_plot_control_2d.cpp              \
                ../us_2dsa/us_resplot_2d.cpp                   \
                ../us_2dsa/us_show_norm.cpp                    \
                ../us_2dsa/us_worker_2d.cpp                    \
                ../us_2dsa/us_worker_calcnorm.cpp              \
                ../us_abde/us_norm_profile.cpp                 \
                ../us_analysis_profile/us_analysis_profile.cpp \
                ../us_edit/us_edit.cpp                         \
                ../us_edit/us_edit_scan.cpp                    \
                ../us_edit/us_exclude_profile.cpp              \
                ../us_edit/us_get_edit.cpp                     \
                ../us_edit/us_ri_noise.cpp                     \
                ../us_edit/us_select_lambdas.cpp               \
                ../us_fit_meniscus/us_fit_meniscus.cpp         \
                ../us_mwl_species_fit/us_load_run_noise.cpp    \
                ../us_mwl_species_fit/us_mwl_sf_plot3d.cpp     \
                ../us_mwl_species_fit/us_mwl_species_fit.cpp   \
                ../us_mwl_species_sim/us_mwl_species_sim.cpp   \
                ../us_analysis_profile/us_anapro_utils.cpp
