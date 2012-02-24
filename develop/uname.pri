# uname.pri
#
# This is designed to be included in a .pro file
# It provides the variable UNAME for all Ultrascan .pro files,
# issues a message, and DEFINES a qmake variable in DEFINE
# that matches the system

unix {
UNAME = $$system(uname -a)
DEFINES += UNIX
}
macx {
UNAME = $$system(uname -a)
DEFINES += MACX
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

win32 {
  DEFINES += WIN32 
  message ("Configuring for the Microsoft Windows Platform...")
}

macx:contains (UNAME, i386) {
  DEFINES += MAC
  message ("Configuring for the Macintosh Intel 386 Platform...")
}
macx:contains (UNAME, powerpc) {
  DEFINES += MAC
  message ("Configuring for the Macintosh PowerPC Platform...")
}

macx {
   QMAKE_CFLAGS   += -arch ppc -arch i386
   QMAKE_LFLAGS   += -arch ppc -arch i386
   CFLAGS   += -arch ppc -arch i386
   LDFLAGS   += -arch ppc -arch i386
   QWTDIR        = /usr/local/qt3/qwt-4.2.0
   QWT3DDIR      = /usr/local/qt3/qwtplot3d
}

