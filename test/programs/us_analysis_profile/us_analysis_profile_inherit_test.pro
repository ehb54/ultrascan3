# Build after utils, qwtplot3d, gui and programlibs/analysis, using #660's archive.
ROOT = $$clean_path($$PWD/../../..)
include($$ROOT/local.pri)
TEMPLATE = app
TARGET = test_us_analysis_profile_inherit_gui
CONFIG += console testcase c++17
CONFIG -= app_bundle
QT += testlib widgets xml sql svg opengl printsupport
 greaterThan(QT_MAJOR_VERSION, 5): QT += openglwidgets
SOURCES = test_us_analysis_profile_inherit_gui.cpp
INCLUDEPATH += $$ROOT/test/common $$ROOT/utils $$ROOT/gui $$ROOT/programs \
               $$ROOT/programs/us_analysis_profile $$ROOT/qwtplot3d/include $$QWTPATH/include $$QWTPATH/src $$MYSQLPATH
LIBS += -L$$ROOT/lib -lus_programs_analysis -lus_gui -lus_utils $$QWTLIB -lcrypto -lmysqlclient -larchive
