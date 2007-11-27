unix {
UNAME = $$system(uname -a)
}

unix:contains(UNAME,Linux) {
	DEFINES  += LINUX
	message ("Configuring for the Linux operating system...")
} 
unix:contains(UNAME,FreeBSD) {
	DEFINES  += FREEBSD
	message ("Configuring for the FreeBSD operating system...")
} 
unix:contains(UNAME,NetBSD) {
	DEFINES  += NETBSD
	message ("Configuring for the NetBSD operating system...")
} 
unix:contains(UNAME,OpenBSD) {
	DEFINES  += OPENBSD
	message ("Configuring for the OpenBSD operating system...")
} 
unix:contains(UNAME,SunOS) {
	DEFINES  += SOLARIS
	message ("Configuring for the Sun Solaris operating system...")
} 
unix:contains(UNAME,IRIX) {
	DEFINES  += IRIX
	message ("Configuring for SGI Irix operating system...")
} 
unix:contains (UNAME,IRIX64) {
	DEFINES += IRIX
	UNAME = $$system(uname -ap)
	DESTDIR  = ../lib
	message ("Configuring for SGI Irix 64-bit operating system...")
}
unix:contains(UNAME,Darwin) {
	DEFINES  += OSX
	UNAME = $$system(uname -p)
	message ("Configuring for the Darwin Macintosh OS-X operating system...")
} 
unix:contains (UNAME, pentium4) {
	DEFINES  += INTEL
	UNAME = $$system(uname -p)
	message ("Configuring for the Intel Pentium IV Platform...")
}
unix:contains (UNAME, i686) {
	UNAME = $$system(uname -p)
	DEFINES += INTEL
	message ("Configuring for the Intel i686 Platform...")
}
unix:contains (UNAME, i586) {
	UNAME = $$system(uname -p)
	DEFINES += INTEL
	message ("Configuring for the Intel i586 Platform...")
}
unix:contains (UNAME, i486) {
	UNAME = $$system(uname -p)
	DEFINES += INTEL
	message ("Configuring for the Intel i486 Platform...")
}
unix:contains (UNAME, i386) {
	UNAME = $$system(uname -p)
	DEFINES += INTEL
	message ("Configuring for the Intel i386 Platform...")
}
unix:contains (UNAME, x86_64) {
	DEFINES += OPTERON
	message ("Configuring for the Opteron 64-bit Platform...")
}
unix:contains (UNAME, mips) {
	DEFINES += SGI
	message ("Configuring for the SGI mips Platform...")
}
unix:contains (UNAME, ppc) {
	DEFINES += MAC
	message ("Configuring for the Macintosh PowerPC Platform...")
}
unix:contains (UNAME, powerpc) {
	DEFINES += MAC
	message ("Configuring for the Macintosh PowerPC Platform...")
}
unix:contains (UNAME, sparc) {
	DEFINES += SPARC
	message ("Configuring for the Sun Sparc Platform...")
}
TEMPLATE	= app
unix:CONFIG	+= qt warn thread release
win32:CONFIG	+= qt warn thread release
unix:DEFINES   += UNIX
win32:DEFINES  += WIN32
INCLUDEPATH     = $(QWTDIR)/include $(QWT3DDIR)/include \
						$(ADOLCDIR) \
						$(ADOLCDIR)/drivers \
		  				$(ADOLCDIR)/sparse \
		 				$(ADOLCDIR)/tapedoc
DEPENDPATH      += ../src \
                   ../include
SOURCES		+= main.cpp
HEADERS		= ../include/us_viewmwl.h

unix:contains (UNAME,x86_64) {
	TARGET		= ../../bin64/us_viewmwl
} else {
	TARGET		= ../../bin/us_viewmwl
}

unix:contains(UNAME,x86_64) {
   LIBS     += -L$(QWTDIR)/lib64/ -lqwt -L$(QWT3DDIR)/lib64 -lqwtplot3d -L$(ULTRASCAN)/lib64 -lus
} else {
   win32 {
      LIBS += $(QWTDIR)/lib/qwt.lib ../../lib/us940.lib $(QWT3DDIR)/lib/qwtplot3d.lib
      }
   unix {
      LIBS += -L$(QWTDIR)/lib -lqwt -L$(QWT3DDIR)/lib -lqwtplot3d -L$(ULTRASCAN)/lib -lus
      }
}
 
macx:RC_FILE = ultrascan.icns
MOC_DIR		= ../src/moc
