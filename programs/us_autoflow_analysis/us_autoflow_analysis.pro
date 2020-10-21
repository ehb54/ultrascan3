include( ../../gui.pri )

TARGET        = us_autoflow_analysis
QT           += xml
QT           += sql
QT           += svg opengl

HEADERS       = us_autoflow_analysis.h \
                ../us_fit_meniscus/us_fit_meniscus.h \
                ../us_fematch/us_fematch.h  \
                ../us_fematch/us_plot_control_fem.h \
                ../us_fematch/us_resplot_fem.h \
                ../us_fematch/us_advanced_fem.h     \
                ../us_fematch/us_adv_dmgamc.h       \
                ../us_fematch/us_dmga_mc_stats.h    \
                ../us_fematch/us_thread_worker.h

SOURCES       = us_autoflow_analysis_main.cpp       \
                us_autoflow_analysis.cpp         \
                ../us_fit_meniscus/us_fit_meniscus.cpp \
                ../us_fematch/us_fematch.cpp  \
                ../us_fematch/us_plot_control_fem.cpp \
                ../us_fematch/us_resplot_fem.cpp      \
                ../us_fematch/us_advanced_fem.cpp     \
                ../us_fematch/us_adv_dmgamc.cpp       \
                ../us_fematch/us_dmga_mc_stats.cpp    \
                ../us_fematch/us_thread_worker.cpp
                
                
               
