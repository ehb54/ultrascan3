# Program sources for the GMP workflow window, used by several programs.
#
# us_protocol_dev has classes with the same names, so it links gmp only.

include( ../programlib.pri )

TARGET        = us_programs_comproject
QT           += xml sql svg opengl network printsupport datavisualization

HEADERS      = ../../programs/us_com_project/us_com_project_gui.h

SOURCES      = ../../programs/us_com_project/us_com_project_gui.cpp
