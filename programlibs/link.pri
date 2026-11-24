# Include file for programs that use the area libraries. Include it after
# gui.pri, with the program's libraries in PROGRAM_LIBS.

# The libraries that each one needs
contains( PROGRAM_LIBS, comproject ) : PROGRAM_LIBS += gmp
contains( PROGRAM_LIBS, gmp )        : PROGRAM_LIBS += experiment analysis results data
contains( PROGRAM_LIBS, experiment ) : PROGRAM_LIBS += analysis

# Each library goes before the ones it needs, and all go before gui and utils
PROGRAM_LIBS_ORDER = data results analysis experiment gmp comproject
for( area, PROGRAM_LIBS_ORDER ) {
  contains( PROGRAM_LIBS, $$area ) {
    LIBS            = -lus_programs_$${area} $$LIBS
    PRE_TARGETDEPS += ../../lib/libus_programs_$${area}.a
  }
}
LIBS          = -L../../lib $$LIBS
