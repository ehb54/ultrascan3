!include ( ../generic.pri ) error( "../generic.pri missing.  Aborting..." )

TARGET = us3_config
win32:RC_FILE = us3_config.rc
macx:ICON = ../../etc/us3_config.icns
