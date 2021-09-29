include( ../../gui.pri )

TARGET        = us_autoflow_reports
QT           += xml
QT           += sql
QT           += printsupport

HEADERS       = us_autoflow_reports.h

SOURCES       = us_autoflow_reports_main.cpp       \
                us_autoflow_reports.cpp  
               
