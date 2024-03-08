include( ../../gui.pri )

TARGET        = us_pseudo_absorbance
QT           += xml printsupport

HEADERS       =  us_pseudo_absorbance.h \
                 us_convert_scan.h      \
                 us_add_refScan.h       \
                 us_remove_ri.h         \
                 us_refScan_dataIO.h    \
                 us_pabs_common.h       \


SOURCES       =  us_pseudo_absorbance.cpp \
                 us_convert_scan.cpp      \
                 us_add_refScan.cpp       \
                 us_remove_ri.cpp         \
                 us_refScan_dataIO.cpp    \
                 us_pabs_common.cpp       \

