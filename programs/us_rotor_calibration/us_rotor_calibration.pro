include( ../../gui.pri )

TARGET        = us_rotor_calibration
QT           += xml
CONFIG		+= debug

HEADERS       = us_rotor_calibration.h \
                us_get_dbexp.h

SOURCES       = us_rotor_calibration.cpp \
                us_get_dbexp.cpp
