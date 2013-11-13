include( ../../gui.pri )

QT           += xml svg

TARGET        = us_pcsa

win32 {
LIBS         += -lpsapi
CONFIG       += console
}
mac {
CONFIG       += console
}

HEADERS       = us_pcsa.h                \
                us_plot_control_pc.h     \
                us_resplot_pc.h          \
                us_mlplot.h              \
                us_analysis_control_pc.h \
                us_adv_analysis_pc.h     \
                us_pcsa_process.h        \
                us_model_record.h        \
                us_rpscan.h              \
                us_worker_pc.h

SOURCES       = us_pcsa.cpp                \
                us_plot_control_pc.cpp     \
                us_resplot_pc.cpp          \
                us_mlplot.cpp              \
                us_analysis_control_pc.cpp \
                us_adv_analysis_pc.cpp     \
                us_pcsa_process.cpp        \
                us_model_record.cpp        \
                us_rpscan.cpp              \
                us_worker_pc.cpp

