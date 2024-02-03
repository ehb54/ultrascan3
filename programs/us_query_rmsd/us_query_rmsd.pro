include( ../../gui.pri )

QT           += xml svg opengl sql printsupport

TARGET        = us_query_rmsd

HEADERS       =  us_query_rmsd.h \
                 ../us_analysis_profile/us_analysis_profile.h \
                 ../us_autoflow_analysis/us_autoflow_analysis.h \
                 ../us_fit_meniscus/us_fit_meniscus.h  \
                 ../us_fematch/us_fematch.h  \
                 ../us_fematch/us_plot_control_fem.h \
                 ../us_fematch/us_resplot_fem.h \
                 ../us_fematch/us_advanced_fem.h     \
                 ../us_fematch/us_adv_dmgamc.h       \
                 ../us_fematch/us_dmga_mc_stats.h    \
                 ../us_fematch/us_thread_worker.h    \
                 ../us_reporter_gmp/us_reporter_gmp.h \
                 ../us_ddist_combine/us_ddist_combine.h \
                 ../us_ddist_combine/us_select_rundd.h  \
                 ../us_pseudo3d_combine/us_pseudo3d_combine.h \
                 ../us_pseudo3d_combine/us_remove_distros.h   \
                 ../us_esigner_gmp/us_esigner_gmp.h \


SOURCES       =  us_query_rmsd.cpp \
                 ../us_analysis_profile/us_analysis_profile.cpp \
                 ../us_analysis_profile/us_anapro_utils.cpp     \
                 ../us_autoflow_analysis/us_autoflow_analysis.cpp \
                 ../us_fit_meniscus/us_fit_meniscus.cpp    \
                 ../us_fematch/us_fematch.cpp  \
                 ../us_fematch/us_plot_control_fem.cpp \
                 ../us_fematch/us_resplot_fem.cpp      \
                 ../us_fematch/us_advanced_fem.cpp     \
                 ../us_fematch/us_adv_dmgamc.cpp       \
                 ../us_fematch/us_dmga_mc_stats.cpp    \
                 ../us_fematch/us_thread_worker.cpp    \
                 ../us_reporter_gmp/us_reporter_gmp.cpp \
                 ../us_ddist_combine/us_ddist_combine.cpp \
                 ../us_ddist_combine/us_select_rundd.cpp  \
                 ../us_pseudo3d_combine/us_pseudo3d_combine.cpp \
                 ../us_pseudo3d_combine/us_remove_distros.cpp  \
                 ../us_esigner_gmp/us_esigner_gmp.cpp \

