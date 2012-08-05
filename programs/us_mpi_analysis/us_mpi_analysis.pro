# US_Nnls
include(../../local.pri)

# Not a gui program

CONFIG      += $${DEBUGORRELEASE} qt thread warn
TEMPLATE     = app
QT          -= gui
QT          += network
DEFINES     += LINUX

TARGET       = us_mpi_analysis
DESTDIR      = .

MOC_DIR      = ./moc
OBJECTS_DIR  = ./obj
revision.target       = us_revision.h
revision.commands     = sh revision.sh
revision.depends      = FORCE
QMAKE_EXTRA_TARGETS  += revision

SOURCES      += us_mpi_analysis.cpp  \
                2dsa_master.cpp      \
                2dsa_worker.cpp      \
                ga_master.cpp        \
                ga_worker.cpp        \
                parallel_masters.cpp \
                us_mpi_parse.cpp

HEADERS      += us_mpi_analysis.h

INCLUDEPATH  += ../../utils /usr/include/mysql
DEPENDPATH   += ../../utils
LIBS         += -lus_utils -L../../lib

# mpi references
DEFINES      += OMPI_SKIP_MPICXX
INCLUDEPATH  += $${MPIPATH}/include
LIBS         += $${MPILIBS}

