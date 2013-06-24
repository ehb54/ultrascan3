include( ../../gui.pri )

QT           += xml svg

TARGET        = us_pcsa

win32:LIBS   += -lpsapi

HEADERS       = us_pcsa.h                \
                us_plot_control.h        \
                us_resplot.h             \
                us_mlplot.h              \
                us_analysis_control.h    \
                us_pcsa_process.h        \
                us_model_record.h        \
                us_rpscan.h              \
                us_worker.h

SOURCES       = us_pcsa.cpp              \
                us_plot_control.cpp      \
                us_resplot.cpp           \
                us_mlplot.cpp            \
                us_analysis_control.cpp  \
                us_pcsa_process.cpp      \
                us_model_record.cpp      \
                us_rpscan.cpp            \
                us_worker.cpp

