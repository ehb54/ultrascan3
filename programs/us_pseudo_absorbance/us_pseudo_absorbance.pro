include( ../../gui.pri )

TARGET        = us_pseudo_absorbance
QT           += xml printsupport

HEADERS       = \
    add_refScan.h \
    build_colormap.h \
    convert_scan.h \
    refScan_dataIO.h \
    select_program.h \
    switch.h \
    ../us_convert/us_select_triples.h

SOURCES       =       \
                add_refScan.cpp \
                build_colormap.cpp \
                convert_scan.cpp \
                refScan_dataIO.cpp \
                select_program.cpp \
                switch.cpp \
                ../us_convert/us_select_triples.cpp \
                us_pseudo_absorbance_main.cpp
