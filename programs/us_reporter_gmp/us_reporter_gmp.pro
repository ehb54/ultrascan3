include( ../../gui.pri )

TARGET        = us_reporter_gmp
QT           += xml
QT           += sql
QT           += svg opengl
QT           += printsupport
QT           += datavisualization

HEADERS       = us_reporter_gmp.h                            \
                us_reporter_gmp_duration.h                   \
                ../us_edited_data_loaders.h                  \
                ../us_abde/us_norm_profile.h                 \
                ../us_analysis_profile/us_analysis_profile.h \
                ../us_ddist_combine/us_ddist_combine.h       \
                ../us_ddist_combine/us_select_rundd.h        \
                ../us_integral/us_delete_models.h            \
                ../us_integral/us_integral.h                 \
                ../us_pseudo3d_combine/us_pseudo3d_combine.h \
                ../us_pseudo3d_combine/us_remove_distros.h


SOURCES       = us_reporter_gmp_main.cpp                       \
                us_reporter_gmp.cpp                            \
                ../us_edited_data_loaders.cpp                  \
                ../us_abde/us_norm_profile.cpp                 \
                ../us_analysis_profile/us_analysis_profile.cpp \
                ../us_analysis_profile/us_anapro_utils.cpp     \
                ../us_ddist_combine/us_ddist_combine.cpp       \
                ../us_ddist_combine/us_select_rundd.cpp        \
                ../us_integral/us_delete_models.cpp            \
                ../us_integral/us_integral.cpp                 \
                ../us_pseudo3d_combine/us_pseudo3d_combine.cpp \
                ../us_pseudo3d_combine/us_remove_distros.cpp
