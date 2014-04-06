!include ( ../generic.pri ) error( "../generic.pri missing.  Aborting..." )

TARGET = us3_somo
win32:RC_FILE = us3_somo.rc
macx:ICON = ../../etc/us3_somo.icns
#The following line was inserted by qt3to4
QT += qt3support 
