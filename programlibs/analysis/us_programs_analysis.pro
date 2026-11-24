# Program sources for analyses and their profiles, used by several programs.

include( ../programlib.pri )

TARGET        = us_programs_analysis
QT           += xml sql svg opengl printsupport datavisualization

HEADERS      = ../../programs/us_analysis_profile/us_analysis_profile.h \
               ../../programs/us_2dsa/us_2dsa.h                         \
               ../../programs/us_2dsa/us_2dsa_process.h                 \
               ../../programs/us_2dsa/us_adv_analysis_2d.h              \
               ../../programs/us_2dsa/us_analysis_control_2d.h          \
               ../../programs/us_2dsa/us_plot_control_2d.h              \
               ../../programs/us_2dsa/us_resplot_2d.h                   \
               ../../programs/us_2dsa/us_show_norm.h                    \
               ../../programs/us_2dsa/us_worker_2d.h                    \
               ../../programs/us_2dsa/us_worker_calcnorm.h              \
               ../../programs/us_fit_meniscus/us_fit_meniscus.h         \
               ../../programs/us_fematch/us_adv_dmgamc.h                \
               ../../programs/us_fematch/us_advanced_fem.h              \
               ../../programs/us_fematch/us_dmga_mc_stats.h             \
               ../../programs/us_fematch/us_fematch.h                   \
               ../../programs/us_mwl_species_fit/us_load_run_noise.h    \
               ../../programs/us_mwl_species_fit/us_mwl_sf_plot3d.h     \
               ../../programs/us_mwl_species_fit/us_mwl_species_fit.h   \
               ../../programs/us_mwl_species_sim/us_mwl_species_sim.h

SOURCES      = ../../programs/us_analysis_profile/us_analysis_profile.cpp \
               ../../programs/us_analysis_profile/us_anapro_utils.cpp     \
               ../../programs/us_2dsa/us_2dsa.cpp                         \
               ../../programs/us_2dsa/us_2dsa_process.cpp                 \
               ../../programs/us_2dsa/us_adv_analysis_2d.cpp              \
               ../../programs/us_2dsa/us_analysis_control_2d.cpp          \
               ../../programs/us_2dsa/us_plot_control_2d.cpp              \
               ../../programs/us_2dsa/us_resplot_2d.cpp                   \
               ../../programs/us_2dsa/us_show_norm.cpp                    \
               ../../programs/us_2dsa/us_worker_2d.cpp                    \
               ../../programs/us_2dsa/us_worker_calcnorm.cpp              \
               ../../programs/us_fit_meniscus/us_fit_meniscus.cpp         \
               ../../programs/us_fematch/us_adv_dmgamc.cpp                \
               ../../programs/us_fematch/us_advanced_fem.cpp              \
               ../../programs/us_fematch/us_dmga_mc_stats.cpp             \
               ../../programs/us_fematch/us_fematch.cpp                   \
               ../../programs/us_mwl_species_fit/us_load_run_noise.cpp    \
               ../../programs/us_mwl_species_fit/us_mwl_sf_plot3d.cpp     \
               ../../programs/us_mwl_species_fit/us_mwl_species_fit.cpp   \
               ../../programs/us_mwl_species_sim/us_mwl_species_sim.cpp
