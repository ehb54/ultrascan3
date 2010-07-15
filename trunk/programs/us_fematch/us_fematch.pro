include( ../../gui.pri )

QT           += xml svg

TARGET        = us_fematch

HEADERS       = us_fematch.h us_resids_bitmap.h us_plot_control.h \
                us_resplot.h us_plot3d.h

SOURCES       = us_fematch.cpp us_resids_bitmap.cpp us_plot_control.cpp \
                us_resplot.cpp us_plot3d.cpp

RESOURCES     = images.qrc

