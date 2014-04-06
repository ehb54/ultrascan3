!include ( ../generic.pri ) error( "../generic.pri missing.  Aborting..." )

TARGET = us3_config
win32:RC_FILE = us3_config.rc
macx:ICON = ../../etc/us3_config.icns
#The following line was inserted by qt3to4
QT += qt3support 
ICON = ../../etc/us3_config.icns
