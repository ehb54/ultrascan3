include( ../../gui.pri )

TARGET        = us_analysis_profile
QT           += xml
QT           += sql

HEADERS       = us_analysis_profile.h    \
                us_anapro_parms.h

SOURCES       = us_anapro_main.cpp       \
                us_analysis_profile.cpp  \
                us_anapro_utils.cpp      \
                us_anapro_parms.cpp
