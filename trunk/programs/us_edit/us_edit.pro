include( ../../gui.pri )

TARGET        = us_edit
QT           += xml

HEADERS       = us_edit.h            \
                us_exclude_profile.h \
                us_get_edit.h        \
                us_load_db.h         \
                us_ri_noise.h        \
                us_edit_scan.h

SOURCES       = us_edit.cpp            \
                us_exclude_profile.cpp \
                us_get_edit.cpp        \
                us_load_db.cpp         \
                us_ri_noise.cpp        \
                us_edit_scan.cpp

