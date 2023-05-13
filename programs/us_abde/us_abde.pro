include( ../../gui.pri )

TARGET        = us_abde
QT           += xml

HEADERS       = us_abde_main.h \
                ../us_buoyancy/us_buoyancy.h  \
                us_norm_profile.h

SOURCES       = us_abde_main.cpp \
                ../us_buoyancy/us_buoyancy.cpp \
                us_norm_profile.cpp
