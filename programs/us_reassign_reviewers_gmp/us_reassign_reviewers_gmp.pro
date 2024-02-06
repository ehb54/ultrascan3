include( ../../gui.pri )

TARGET        = us_reassign_reviewers_gmp
QT           += xml
QT           += sql
QT           += svg opengl
QT           += printsupport

HEADERS       = ../us_esigner_gmp/us_esigner_gmp.h                  

SOURCES       = us_reassign_reviewers_gmp_main.cpp       \
                ../us_esigner_gmp/us_esigner_gmp.cpp



               
