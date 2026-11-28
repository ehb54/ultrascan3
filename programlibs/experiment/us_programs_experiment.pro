# Program sources for setting up experiments, used by several programs.

include( ../programlib.pri )

TARGET        = us_programs_experiment
QT           += xml sql network printsupport

HEADERS      = ../../programs/us_experiment/us_experiment_gui_optima.h

SOURCES      = ../../programs/us_experiment/us_exp_utils.cpp             \
               ../../programs/us_experiment/us_experiment_gui_optima.cpp \
               ../../programs/us_experiment/us_proto_ranges.cpp
