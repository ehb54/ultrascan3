# Program sources for the GMP workflow: analysis, report and e-signature, used by several programs.

include( ../programlib.pri )

TARGET        = us_programs_gmp
QT           += xml sql svg opengl network printsupport datavisualization

# us_reporter_gmp.cpp includes the revision header. programs/us writes it too,
# but is built after this library.
!exists( ../../programs/us/us_revision.h ): system( cd ../../programs/us && sh revision.sh )

HEADERS      = ../../programs/us_esigner_gmp/us_esigner_gmp.h             \
               ../../programs/us_reporter_gmp/us_reporter_gmp.h           \
               ../../programs/us_reporter_gmp/us_reporter_gmp_duration.h  \
               ../../programs/us_autoflow_analysis/us_autoflow_analysis.h

SOURCES      = ../../programs/us_esigner_gmp/us_esigner_gmp.cpp             \
               ../../programs/us_reporter_gmp/us_reporter_gmp.cpp           \
               ../../programs/us_autoflow_analysis/us_autoflow_analysis.cpp
