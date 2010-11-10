include( ../../gui.pri )

QT           += xml svg

TARGET        = us_2dsa

HEADERS       = us_2dsa.h                \
                us_plot_control.h        \
                us_resplot.h             \
                us_analysis_control.h    \
                us_2dsa_process.h        \

SOURCES       = us_2dsa.cpp              \
                us_plot_control.cpp      \
                us_resplot.cpp           \
                us_analysis_control.cpp  \
                us_2dsa_process.cpp

