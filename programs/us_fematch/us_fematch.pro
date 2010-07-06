include( ../../gui.pri )

QT           += xml svg

TARGET        = us_fematch

HEADERS       = us_fematch.h us_resids_bitmap.h us_plot_control.h \
                us_noise_loader.h us_resplot.h

SOURCES       = us_fematch.cpp us_resids_bitmap.cpp us_plot_control.cpp \
                us_noise_loader.cpp us_resplot.cpp

