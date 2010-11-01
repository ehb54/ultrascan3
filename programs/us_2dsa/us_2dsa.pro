include( ../../gui.pri )

QT           += xml svg

TARGET        = us_2dsa

HEADERS       = us_2dsa.h            \
                us_plot_control.h    \
                us_resplot.h         \
                us_anal_control.h    \
                us_2dsa_process.h    \

SOURCES       = us_2dsa.cpp          \
                us_plot_control.cpp  \
                us_resplot.cpp       \
                us_anal_control.cpp  \
                us_2dsa_process.cpp

