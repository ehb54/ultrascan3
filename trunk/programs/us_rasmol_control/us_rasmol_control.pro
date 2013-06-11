include( ../../gui.pri )

TARGET        = us_rasmol_control

HEADERS       = us_rasmol_control.h

SOURCES       = us_rasmol_control.cpp

unix {
HEADERS      += us_x11_utils.h
SOURCES      += us_x11_utils.c
}

macx {
INCLUDES     += /usr/X11/include
LIBS         += -L/usr/X11/lib -lXau -lX11
}

win32 {
HEADERS      += us_win_utils.h
SOURCES      += us_win_utils.c
}

