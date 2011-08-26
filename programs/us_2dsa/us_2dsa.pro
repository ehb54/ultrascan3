include( ../../gui.pri )

QT           += xml svg

TARGET        = us_2dsa

win32:LIBS   += -lpsapi

HEADERS       = us_2dsa.h                \
                us_plot_control.h        \
                us_resplot.h             \
                us_analysis_control.h    \
                us_adv_analysis.h        \
                us_2dsa_process.h        \
                us_worker.h

SOURCES       = us_2dsa.cpp              \
                us_plot_control.cpp      \
                us_resplot.cpp           \
                us_analysis_control.cpp  \
                us_adv_analysis.cpp      \
                us_2dsa_process.cpp      \
                us_worker.cpp

