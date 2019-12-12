DEFINES += ADOLC
DEFINES += NO_DB

unix:{
SUBDIRS   = \
            us_saxs_cmds_t
TEMPLATE  = subdirs
}

win32:TEMPLATE=subdirs

SUBDIRS += \
    us_admin \
    us3_config \
    us3_hydrodyn
