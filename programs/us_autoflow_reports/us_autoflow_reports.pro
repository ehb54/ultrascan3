include( ../../gui.pri )

TARGET        = us_autoflow_reports
QT           += xml
QT           += sql
QT           += printsupport

HEADERS       = us_autoflow_reports.h    \
                ../us_analysis_profile/us_analysis_profile.h 
                                

SOURCES       = us_autoflow_reports_main.cpp       \
                us_autoflow_reports.cpp            \
                ../us_analysis_profile/us_analysis_profile.cpp   \
                ../us_analysis_profile/us_anapro_utils.cpp
                

               
