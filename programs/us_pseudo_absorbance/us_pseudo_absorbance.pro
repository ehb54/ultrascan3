include( ../../gui.pri )

TARGET        = us_pseudo_absorbance
QT           += xml printsupport

HEADERS       = \
    ../us_convert/us_select_triples.h \
    us_refScan_dataIO.h \
    us_add_refScan.h \
    us_convert_scan.h \
    us_gen_colormap.h \
    us_pseudo_absorbance.h


SOURCES       =       \
                ../us_convert/us_select_triples.cpp \
                us_refScan_dataIO.cpp \
                us_add_refScan.cpp \
                us_convert_scan.cpp \
                us_gen_colormap.cpp \
                us_pseudo_absorbance.cpp
