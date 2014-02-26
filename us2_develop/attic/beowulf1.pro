TEMPLATE	= app
INSTALL		= install
INCLUDE		= /usr/local/qwt/include
CONFIG		= qt warn_off release
HEADERS		= include/dtactrl.h \
		include/edit.h \
		include/editor.h \
		include/edscan.h \
		include/fefit.h \
		include/femfit_beowulf.h \
		include/finite1.h \
		include/finite2.h \
		include/hydrod.h \
		include/matrix.h \
		include/us_global.h \
		include/us_math.h \
		include/us_rundetails.h \
		include/us_util.h \
		include/ushydro.h
SOURCES		= src/dtactrl.cpp \
		src/edit.cpp \
		src/editor.cpp \
		src/edscan.cpp \
		src/fefit.cpp \
		src/femfit_beowulf.cpp \
		src/finite1.cpp \
		src/finite2.cpp \
		src/hydrod.cpp \
		src/matrix.cpp \
		src/us_global.cpp \
		src/us_math.cpp \
		src/us_rundetails.cpp \
		src/us_util.cpp \
		src/ushydro.cpp
TARGET		= us_beowulf
LIBS 		= -lm -lqwt
