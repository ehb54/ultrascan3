# Include file for the area libraries: program sources that several programs use.
#
# A source is in one library only, so it is compiled once however many programs
# use it. The libraries are static, so a program gets only the objects it needs.
# programs/CMakeLists.txt has the same libraries; keep the areas and their
# dependencies the same in both (scripts/check-program-libraries.py).

include( ../gui.pri )

TEMPLATE      = lib
CONFIG       += staticlib
DESTDIR       = ../../lib
DEPENDPATH   += ../../programs
INCLUDEPATH  += ../../programs

# A static library links nothing; link.pri says what the programs link. Without
# the .prl file, a program does not link the Qt modules of sources it never uses.
LIBS          =
CONFIG       -= create_prl
