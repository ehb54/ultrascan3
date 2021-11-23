include( ../../gui.pri )

TARGET        = us_reporter_gmp
QT           += xml
QT           += sql
QT           += printsupport

HEADERS       = us_reporter_gmp.h \
                ../us_analysis_profile/us_analysis_profile.h                 

SOURCES       = us_reporter_gmp_main.cpp       \
                us_reporter_gmp.cpp            \
                ../us_analysis_profile/us_analysis_profile.cpp   \
                ../us_analysis_profile/us_anapro_utils.cpp
                

               
