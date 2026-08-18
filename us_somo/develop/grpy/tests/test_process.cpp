// ProcessSolver: the GRPY program run as a subprocess, presented as a grpy::SolveFn.
//
// Driven by a FAKE GRPY program -- a script that replays a real golden report and the real
// progress banner -- so the parsing, the progress scraping and both failure paths are
// tested without needing a built GRPY. The report it replays is the one the production
// Fortran binary produced for the dumbbell example, so the expected values below are that
// program's actual output, not values invented for the test.
//
// Build: see run.sh (needs QtCore).
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>

#include "grpy_process.hpp"

using namespace grpy;

static int fails = 0;

static int chk( const char* what, bool ok ) {
   std::printf( "  %-60s %s\n", what, ok ? "OK" : "FAIL" );
   return ok ? 0 : 1;
}

static bool close_to( double a, double b, double rtol = 1e-12 ) {
   return std::fabs( a - b ) <= rtol * std::fabs( b );
}

// A stand-in for the GRPY program: emits the banner to stdout exactly as GRPY does
// (carriage-return separated, no newline), then the report, then exits with $EXIT.
static QString write_fake_grpy( const QDir& dir, const QString& report_file, int exit_code,
                                bool slow = false ) {
   const QString path = dir.filePath( QString( "fake_grpy_%1.sh" ).arg( exit_code ) );
   QFile f( path );
   f.open( QIODevice::WriteOnly | QIODevice::Text );
   QTextStream ts( &f );
   ts << "#!/bin/sh\n"
      << "printf '\\r  0%% TASK:                  READING DATA'\n"
      << "printf '\\r 50%% TASK:            INVERTING MATRICES'\n";
   if ( slow ) {
      ts << "sleep 30\n";
   }
   ts << "printf '\\r100%% TASK:                      COMPLETE'\n"
      << "cat " << report_file << "\n"
      << "exit " << exit_code << "\n";
   f.close();
   QFile::setPermissions( path, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner );
   return path;
}

int main( int argc, char** argv ) {
   QCoreApplication app( argc, argv );

   const QString here   = QFileInfo( __FILE__ ).absolutePath();
   const QString golden = QDir( here ).filePath( "data/dumbbell_golden.txt" );
   // Its own directory, so a leftover from an earlier run cannot make the cleanup check
   // pass or fail for the wrong reason.
   QDir tmp( QDir( QDir::tempPath() ).filePath(
                QString( "grpy_test_process_%1" ).arg( QCoreApplication::applicationPid() ) ) );
   QDir().mkpath( tmp.path() );

   std::vector<Bead> beads = { { 0, 0, 0, 3.0, 1000.0 }, { 6, 0, 0, 3.0, 1000.0 } };
   PhysParams        phys;
   phys.mw = 2000.0;

   // ---- the parser must survive the HIGH-PRECISION report ----------------------
   // SOMO sets GRPY_HP so the program prints %24.15E instead of the Fortran's %11.3E.
   // That matters because the shell reduction differences successive rungs: at four
   // significant figures the quantisation lands straight on the error bar. The parser
   // finds values by their exponent rather than by column, so it should be indifferent
   // to the field width -- this asserts that, and that a double survives the round trip.
   {
      const double dt  = 5.0158680123456789e-07;
      const double dr  = 2.5544796912345678e+07;
      const QString hp =
         QString( " Rotational diffusion coefficient:                                     %1[s^-1]    \n"
                  " Translational diffusion coefficient:                                  %2[cm^2/s]  \n"
                  " Translational diffusion coefficient:                                  %3[cm^2/s]  \n" )
            .arg( dr, 24, 'E', 15 ).arg( dt, 24, 'E', 15 ).arg( dt, 24, 'E', 15 );
      Results r;
      parse_report( hp, r );
      chk( "high-precision report parses D_r", close_to( r.rotational_diffusion, dr, 1e-14 ) );
      chk( "high-precision report parses D_t", close_to( r.translational_diffusion, dt, 1e-14 ) );
      chk( "high-precision round trip beats 4 significant figures",
           std::fabs( r.translational_diffusion - dt ) / dt < 1e-12 );

      // And the legacy width must still parse, since the program's default is unchanged
      // and an older binary on PATH would still emit it.
      const QString es3 =
         QString( " Translational diffusion coefficient:                                  %1[cm^2/s]  \n" )
            .arg( dt, 11, 'E', 3 );
      Results r3;
      parse_report( es3, r3 );
      chk( "legacy ES11.3 report still parses",
           std::fabs( r3.translational_diffusion - dt ) / dt < 1e-3 );
   }

   // ---- the input reader, on the file SOMO writes -----------------------------
   {
      const QString in_path = QDir( here ).filePath( "data/dumbbell.grpy" );
      Input in = read_grpy_input( in_path );
      fails += chk( "reads the bead count", in.beads.size() == 2 );
      fails += chk( "reads the length unit", in.params.units > 0 );
      fails += chk( "reads a positive radius", in.beads[ 0 ].radius > 0 );

      bool threw = false;
      try {
         read_grpy_input( QDir( here ).filePath( "data/no_such_file.grpy" ) );
      } catch ( const std::exception& ) {
         threw = true;
      }
      fails += chk( "a missing input file is reported", threw );
   }

   // ---- a banner split across two reads ---------------------------------------
   // Reads off a pipe break wherever the buffer happens to fill, not on record boundaries.
   // Before consume_progress held the trailing partial record back, BOTH halves of a split
   // banner were appended to the report -- so progress text reached the .grpy_res file and
   // whatever parse_report() then made of it.
   {
      QString     carry;
      int         pct = -1;
      std::string stage;
      ProgressFn  prog = [ &pct, &stage ]( int p, const char* st ) {
         pct   = p;
         stage = st ? st : "";
      };

      QString report;
      report += consume_progress( QString( "  50% TASK: assembl" ), prog, carry );
      fails += chk( "a half-arrived banner is held back, not reported",
                    report.isEmpty() && pct == -1 );

      report += consume_progress( QString( "ing\rreal report text" ), prog, carry, true );
      fails += chk( "a banner split across reads is still recognised", pct == 50 );
      fails += chk( "the split banner's stage is intact", stage == "assembling" );
      fails += chk( "neither half of a split banner reaches the report",
                    report.indexOf( "TASK:" ) < 0 && report.indexOf( "50%" ) < 0 );
      fails += chk( "report text following it survives",
                    report.indexOf( "real report text" ) >= 0 );
   }

   // ---- a successful run ------------------------------------------------------
   {
      const QString fake = write_fake_grpy( tmp, golden, 0 );
      ProcessSolver::Config cfg;
      cfg.program     = fake;
      cfg.working_dir = tmp.path();
      cfg.rung_dir    = tmp.path();

      int last_pct = -1;
      int ticks    = 0;
      ProcessSolver solver( cfg );
      Results r = solver.run_one( beads, phys,
                                  [ &last_pct, &ticks ]( int pct, const char* ) {
                                     last_pct = pct;
                                     ++ticks;
                                  } );

      fails += chk( "progress was reported", ticks >= 3 );
      fails += chk( "progress ended at 100%", last_pct == 100 );
      fails += chk( "the report was captured", r.report.find( "GRPY program" ) != std::string::npos );
      fails += chk( "the banner is not in the report", r.report.find( "TASK:" ) == std::string::npos );
      fails += chk( "rotational diffusion parsed", close_to( r.rotational_diffusion, 3.514E+06 ) );
      fails += chk( "sedimentation parsed", close_to( r.sedimentation, 2.493E-02 ) );
      fails += chk( "eta_oo parsed", close_to( r.intrinsic_viscosity_high, 2.584E+02 ) );
      fails += chk( "eta_0 parsed", close_to( r.intrinsic_viscosity_zero, 2.837E+02 ) );
      // The first occurrence is at the origin, the second at the centre of diffusion --
      // and a third line carries the Stokes radius under the same label.
      fails += chk( "Dt at the origin parsed", close_to( r.translational_diffusion, 8.251E-07 ) );
      fails += chk( "Dt at the centre parsed (not the origin, not the radius)",
                    close_to( r.translational_diffusion_centre, 6.075E-07 ) );
      fails += chk( "the rung file was cleaned up",
                    tmp.entryList( QStringList() << "somo_grpy_rung_*", QDir::Files ).isEmpty() );
      QFile::remove( fake );
   }

   // ---- a failing run ---------------------------------------------------------
   {
      const QString fake = write_fake_grpy( tmp, golden, 3 );
      ProcessSolver::Config cfg;
      cfg.program  = fake;
      cfg.rung_dir = tmp.path();
      ProcessSolver solver( cfg );
      bool        threw = false;
      std::string message;
      try {
         solver.run_one( beads, phys, {} );
      } catch ( const Stopped& ) {
         message = "reported as a stop";
      } catch ( const std::exception& e ) {
         threw   = true;
         message = e.what();
      }
      fails += chk( "a non-zero exit is a failure", threw );
      fails += chk( "the failure names the exit code",
                    message.find( "exit 3" ) != std::string::npos );
      QFile::remove( fake );
   }

   // ---- a missing program -----------------------------------------------------
   {
      ProcessSolver::Config cfg;
      cfg.program  = tmp.filePath( "definitely_not_here" );
      cfg.rung_dir = tmp.path();
      ProcessSolver solver( cfg );
      bool threw = false;
      try {
         solver.run_one( beads, phys, {} );
      } catch ( const std::exception& ) {
         threw = true;
      }
      fails += chk( "a missing program is reported before running", threw );
   }

   // ---- stop, while a solve is running ----------------------------------------
   {
      const QString fake = write_fake_grpy( tmp, golden, 0, true );   // sleeps 30s
      ProcessSolver::Config cfg;
      cfg.program  = fake;
      cfg.rung_dir = tmp.path();
      ProcessSolver solver( cfg, []() { return true; } );             // stop immediately
      bool stopped = false;
      try {
         solver.run_one( beads, phys, {} );
      } catch ( const Stopped& ) {
         stopped = true;
      } catch ( const std::exception& ) {
      }
      // The in-process solver could not do this: its factorization had no interior abort,
      // so Stop only took effect between models.
      fails += chk( "a running solve is killed on stop", stopped );
      QFile::remove( fake );
   }

   tmp.removeRecursively();
   std::printf( fails ? "FAILURES: %d\n" : "ALL PASS (%d failures)\n", fails );
   return fails ? 1 : 0;
}
