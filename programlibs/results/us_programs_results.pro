# Program sources for combining and plotting results, used by several programs.

include( ../programlib.pri )

TARGET        = us_programs_results
QT           += xml sql svg opengl printsupport

HEADERS      = ../../programs/us_ddist_combine/us_ddist_combine.h       \
               ../../programs/us_ddist_combine/us_select_rundd.h        \
               ../../programs/us_pseudo3d_combine/us_pseudo3d_combine.h \
               ../../programs/us_pseudo3d_combine/us_remove_distros.h   \
               ../../programs/us_integral/us_delete_models.h            \
               ../../programs/us_integral/us_integral.h

SOURCES      = ../../programs/us_ddist_combine/us_ddist_combine.cpp       \
               ../../programs/us_ddist_combine/us_select_rundd.cpp        \
               ../../programs/us_pseudo3d_combine/us_pseudo3d_combine.cpp \
               ../../programs/us_pseudo3d_combine/us_remove_distros.cpp   \
               ../../programs/us_integral/us_delete_models.cpp            \
               ../../programs/us_integral/us_integral.cpp
