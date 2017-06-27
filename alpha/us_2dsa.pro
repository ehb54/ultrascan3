include( ../../gui.pri )

QT           += xml svg

TARGET        = us_2dsa

win32:LIBS   += -lpsapi

HEADERS       = us_2dsa.h                \
                us_plot_control_2d.h     \
                us_resplot_2d.h          \
                us_analysis_control_2d.h \
                us_adv_analysis_2d.h     \
                us_2dsa_process.h        \
                us_worker_2d.h           \
                us_show_norm.h           \
                us_worker_calcnorm.h

SOURCES       = us_2dsa.cpp              \
                us_plot_control_2d.cpp   \
                us_resplot_2d.cpp        \
                us_analysis_control_2d.cpp  \
                us_adv_analysis_2d.cpp   \
                us_2dsa_process.cpp      \
                us_worker_2d.cpp         \
                us_show_norm.cpp         \
                us_worker_calcnorm.cpp

