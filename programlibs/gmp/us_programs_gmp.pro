# Program sources for the GMP workflow: analysis, report and e-signature, used by several programs.

include( ../programlib.pri )

TARGET        = us_programs_gmp
QT           += xml sql svg opengl network printsupport datavisualization

# Generate the header before qmake scans the reporter's includes in a clean tree.
# Refresh it before compilation so an incremental build sees the current revision.
unix {
    revision.target = ../../programs/us/us_revision.h
    revision.commands = cd ../../programs/us && sh revision.sh
    revision.depends = FORCE
    !exists( $$revision.target ): system( $$revision.commands )
    QMAKE_EXTRA_TARGETS += revision
}

HEADERS      = ../../programs/us_esigner_gmp/us_esigner_gmp.h             \
               ../../programs/us_reporter_gmp/us_reporter_gmp.h           \
               ../../programs/us_reporter_gmp/us_reporter_gmp_duration.h  \
               ../../programs/us_autoflow_analysis/us_autoflow_analysis.h

SOURCES      = ../../programs/us_esigner_gmp/us_esigner_gmp.cpp             \
               ../../programs/us_reporter_gmp/us_reporter_gmp.cpp           \
               ../../programs/us_autoflow_analysis/us_autoflow_analysis.cpp
