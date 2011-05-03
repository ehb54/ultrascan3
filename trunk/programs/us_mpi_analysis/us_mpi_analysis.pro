# US_Nnls
# Not a gui program

CONFIG      += debug qt thread warn
TEMPLATE     = app
QT          -= gui
QT          += network
DEFINES     += LINUX

TARGET       = us_mpi_analysis
DESTDIR      = .

MOC_DIR      = ./moc
OBJECTS_DIR  = ./obj

SOURCES      += us_mpi_analysis.cpp \
                2dsa_master.cpp     \
                2dsa_worker.cpp     \
                ga_master.cpp       \
                ga_worker.cpp       \
                us_mpi_parse.cpp    \
                us_tar.cpp

HEADERS      += us_mpi_analysis.h \
                us_tar.h

INCLUDEPATH  += ../../utils /usr/include/mysql
DEPENDPATH   += ../../utils
LIBS         += -lus_utils -L../../lib

# mpi references
DEFINES      += OMPI_SKIP_MPICXX 
INCLUDEPATH  += /share/apps64/openmpi/include
LIBS         += -L/share/apps64/openmpi/lib -lmpi


