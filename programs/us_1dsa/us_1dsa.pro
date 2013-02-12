include( ../../gui.pri )

QT           += xml svg

TARGET        = us_1dsa

win32:LIBS   += -lpsapi

HEADERS       = us_1dsa.h                \
                us_plot_control.h        \
                us_resplot.h             \
                us_mlplot.h              \
                us_analysis_control.h    \
                us_1dsa_process.h        \
                us_model_record.h        \
                us_worker.h

SOURCES       = us_1dsa.cpp              \
                us_plot_control.cpp      \
                us_resplot.cpp           \
                us_mlplot.cpp            \
                us_analysis_control.cpp  \
                us_1dsa_process.cpp      \
                us_model_record.cpp      \
                us_worker.cpp

