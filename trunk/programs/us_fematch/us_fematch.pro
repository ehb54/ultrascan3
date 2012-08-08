include( ../../gui.pri )

QT           += xml svg opengl

TARGET        = us_fematch

HEADERS       = us_fematch.h us_plot_control.h us_resplot.h \
                us_advanced.h us_thread_worker.h

SOURCES       = us_fematch.cpp us_plot_control.cpp us_resplot.cpp \
                us_advanced.cpp us_thread_worker.cpp

