// Runs the GRPY program as a subprocess, and presents it as a grpy::SolveFn.
//
// This is the implementation of the injection point in grpy_types.hpp: the solver is a
// separate GPLv3 program (ehb54/grpy-cpp) that SOMO invokes, so nothing GPLv3-derived is
// linked into libus_somo. See ehb54/ultrascan-tickets#1012.
//
// The contract with the program is the one SOMO used before the in-process port: run it
// with `-e <file>`, read the report from stdout, and scrape the carriage-return-separated
// `NN% TASK:` banner for progress. Nothing was added to it -- the extra controls
// (precision, out-of-core, thread count) are environment variables, so the command line is
// still the one the Fortran GRPY published.
//
// Running out of process also restores what the in-process port gave up: a failed or
// oversized solve kills a child rather than the session, and Stop can end a solve that is
// already running instead of waiting for it to finish.
#pragma once
#include <cmath>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <QTextStream>

#include "grpy_types.hpp"

namespace grpy {

// ---------------------------------------------------------------------------------------
// Report parsing. Free functions so they can be tested without a process.
// ---------------------------------------------------------------------------------------

// The value on a GRPY report line: the label is right-justified in 70 columns and followed
// by one ES11.3 field, so the first number after the label is the quantity.
inline bool report_value( const QString& report, const QString& label, double& value,
                          int occurrence = 1, const QString& unit = QString() ) {
   const QStringList lines = report.split( '\n' );
   int seen = 0;
   for ( const QString& line : lines ) {
      if ( !line.contains( label ) ) {
         continue;
      }
      if ( !unit.isEmpty() && !line.contains( unit ) ) {
         continue;
      }
      if ( ++seen != occurrence ) {
         continue;
      }
      const QString tail = line.mid( line.indexOf( label ) + label.length() );
      // The value is written in an ES field, so it always carries an exponent, and the
      // EXPONENT IS WHAT IDENTIFIES IT. Matching a bare number instead picks up digits in
      // the label itself: the sedimentation line reads
      //   Sedimentation coefficient (Mw Dlt (1. - (vbar*rho))/(nA kB T)):  2.493E-02
      // and "the first number after the label" is the 1. in the formula, not the result.
      // Taking the last match is a second guard, for a unit that ends in a digit.
      static const QRegularExpression number( "([-+]?\\d*\\.?\\d+[EeDd][-+]?\\d+)" );
      QRegularExpressionMatchIterator it = number.globalMatch( tail );
      QString last;
      while ( it.hasNext() ) {
         last = it.next().captured( 1 );
      }
      if ( last.isEmpty() ) {
         return false;
      }
      bool ok = false;
      const double v = last.toDouble( &ok );
      if ( !ok ) {
         return false;
      }
      value = v;
      return true;
   }
   return false;
}

// Fill the scalars the shell reduction converges on. The report is the contract, so this
// reads it rather than expecting the program to publish anything extra.
//
// "Translational diffusion coefficient:" appears THREE times: at the coordinate origin, at
// the centre of diffusion, and a third time labelled the same way but carrying the Stokes
// radius in [cm] -- a mislabelling inherited from the Fortran. The observable the ladder
// uses is the centre-of-diffusion value, so match on the unit as well as the label.
inline void parse_report( const QString& report, Results& r ) {
   const QString cm2s = "[cm^2/s]";
   double v = 0;
   if ( report_value( report, "Rotational diffusion coefficient:", v ) ) {
      r.rotational_diffusion = v;
   }
   if ( report_value( report, "Sedimentation coefficient", v ) ) {
      r.sedimentation = v;
   }
   if ( report_value( report, "Translational diffusion coefficient:", v, 1, cm2s ) ) {
      r.translational_diffusion = v;
   }
   if ( report_value( report, "Translational diffusion coefficient:", v, 2, cm2s ) ) {
      r.translational_diffusion_centre = v;
   } else {
      r.translational_diffusion_centre = r.translational_diffusion;
   }
   if ( report_value( report, "High frequency intrinsic viscosity eta oo:", v ) ) {
      r.intrinsic_viscosity_high = v;
   }
   if ( report_value( report, "Zero frequency intrinsic viscosity eta 0:", v ) ) {
      r.intrinsic_viscosity_zero = v;
   }
   r.report = report.toStdString();
}

// Split a chunk of the program's stdout into progress records and report text. The banner
// is carriage-return separated and overwrites itself in a terminal; the report is the part
// that is not a banner record.
//
// `carry` holds a trailing PARTIAL record between calls, and the caller must pass the same
// one every time. Reads off a pipe break wherever the buffer happened to fill, not on record
// boundaries, so a banner split across two reads would otherwise match nothing and have both
// halves appended to the report -- progress text ending up in the .grpy_res file and in what
// parse_report() reads. Holding the tail back until its terminator arrives also repairs a
// "\r\n" straddling the boundary. Call once more with flush = true when the pipe is closed,
// or a final record with no terminator is dropped.
inline QString consume_progress( const QString& chunk, const ProgressFn& progress,
                                 QString& carry, bool flush = false ) {
   static const QRegularExpression banner( "^\\s*(\\d+)%\\s*TASK:\\s*(.*)$" );
   QString text = carry + chunk;
   carry.clear();
   text.replace( "\r\n", "\n" );                  // Windows line endings
   QString report;
   QStringList records = text.split( '\r' );
   if ( !flush && !records.isEmpty() ) {
      carry = records.takeLast();                 // incomplete: wait for the rest
   }
   for ( const QString& record : records ) {
      const QRegularExpressionMatch m = banner.match( record );
      if ( m.hasMatch() ) {
         if ( progress ) {
            progress( m.captured( 1 ).toInt(), m.captured( 2 ).trimmed().toUtf8().constData() );
         }
      } else {
         report += record;
      }
   }
   return report;
}

// ---------------------------------------------------------------------------------------
// Reading the .grpy file SOMO writes (us_hydrodyn_write.cpp). Written against OUR OWN
// writer, not translated from the Fortran reader: header lines carrying the title,
// temperature, solvent viscosity, molecular weight, partial specific volume, density,
// length unit and bead count, then one "x y z radius" record per bead. Fields are taken as
// the first token of a record, which is how the file is written and how GRPY reads it.
// ---------------------------------------------------------------------------------------
struct Input {
   std::vector<Bead> beads;
   PhysParams        params;
};

inline Input read_grpy_input( const QString& path ) {
   QFile f( path );
   if ( !f.open( QIODevice::ReadOnly | QIODevice::Text ) ) {
      throw std::runtime_error(
         QString( "GRPY: cannot read the input file '%1'." ).arg( path ).toStdString() );
   }
   QTextStream ts( &f );
   int         lineno = 0;

   auto field = [ &ts, &lineno, &path ]( const char* what ) -> double {
      if ( ts.atEnd() ) {
         throw std::runtime_error(
            QString( "GRPY: input file '%1' ends before the %2 field." )
            .arg( path ).arg( what ).toStdString() );
      }
      const QString line = ts.readLine();
      ++lineno;
      const QStringList tokens = line.split( QRegularExpression( "[\\s,]+" ), Qt::SkipEmptyParts );
      bool ok = false;
      const double v = tokens.isEmpty() ? 0.0 : tokens[ 0 ].toDouble( &ok );
      if ( !ok ) {
         throw std::runtime_error(
            QString( "GRPY: input file '%1' line %2: expected a number for %3, found '%4'." )
            .arg( path ).arg( lineno ).arg( what ).arg( line ).toStdString() );
      }
      return v;
   };

   Input in;
   if ( ts.atEnd() ) {
      throw std::runtime_error(
         QString( "GRPY: input file '%1' is empty." ).arg( path ).toStdString() );
   }
   ts.readLine();                                 // title, discarded as GRPY discards it
   ++lineno;
   in.params.temperature_C = field( "temperature" );
   in.params.eta           = field( "solvent viscosity" );
   in.params.mw            = field( "molecular weight" );
   in.params.vbar          = field( "partial specific volume" );
   in.params.rho           = field( "solution density" );
   in.params.units         = field( "length unit" );
   in.params.input_label   = "GRPY";
   const double count = field( "bead count" );
   if ( count < 1 || count > 1e8 || count != std::floor( count ) ) {
      throw std::runtime_error(
         QString( "GRPY: input file '%1' declares an implausible bead count (%2)." )
         .arg( path ).arg( count ).toStdString() );
   }

   const int n = (int) count;
   in.beads.reserve( (size_t) n );
   for ( int i = 0; i < n; ++i ) {
      if ( ts.atEnd() ) {
         throw std::runtime_error(
            QString( "GRPY: input file '%1' declares %2 beads but contains only %3." )
            .arg( path ).arg( n ).arg( i ).toStdString() );
      }
      const QString     line   = ts.readLine();
      ++lineno;
      const QStringList tokens = line.split( QRegularExpression( "[\\s,]+" ), Qt::SkipEmptyParts );
      if ( tokens.size() < 4 ) {
         throw std::runtime_error(
            QString( "GRPY: input file '%1' line %2: expected 'x y z radius', found '%3'." )
            .arg( path ).arg( lineno ).arg( line ).toStdString() );
      }
      Bead b{};
      b.x      = tokens[ 0 ].toDouble();
      b.y      = tokens[ 1 ].toDouble();
      b.z      = tokens[ 2 ].toDouble();
      b.radius = tokens[ 3 ].toDouble();
      b.mw     = 0;
      if ( !( b.radius > 0 ) || !std::isfinite( b.radius ) ) {
         throw std::runtime_error(
            QString( "GRPY: input file '%1' line %2: bead %3 has a radius that is not positive (%4)." )
            .arg( path ).arg( lineno ).arg( i + 1 ).arg( b.radius ).toStdString() );
      }
      in.beads.push_back( b );
   }
   return in;
}

// ---------------------------------------------------------------------------------------
// The solver itself.
// ---------------------------------------------------------------------------------------
class ProcessSolver {
public:
   // Removes a rung's temporary input however the run ends. Without this a failed or
   // stopped rung left its file behind, and a ladder that stops early leaks one per rung.
   struct ScopedFile {
      QString path;
      ~ScopedFile() {
         if ( !path.isEmpty() ) {
            QFile::remove( path );
         }
      }
   };

   struct Config {
      QString program;                            // full path to the GRPY binary
      QString working_dir;                        // process working directory
      // The .grpy file SOMO already wrote for the whole model. A rung that uses every bead
      // is run on THIS file rather than on a rewritten copy, so the ordinary unreduced
      // calculation reads exactly the file the user sees, as it always has.
      QString full_input;
      int     full_beads = 0;
      QString rung_dir;                           // where reduced rung files are written
      bool    single     = false;                 // -> GRPY_SINGLE
      QString ooc_dir;                            // -> GRPY_OOC
      int     threads    = 0;                     // -> GRPY_THREADS (0 = the program decides)
   };

   ProcessSolver( const Config& cfg, std::function<bool()> should_stop = {} )
      : cfg_( cfg ), should_stop_( std::move( should_stop ) ) {}

   // Bind this to the injection point the shell reduction calls.
   SolveFn fn() {
      return [ this ]( const std::vector<Bead>& beads, const PhysParams& p,
                       const ProgressFn& progress ) {
         return this->run_one( beads, p, progress );
      };
   }

   Results run_one( const std::vector<Bead>& beads, const PhysParams& p,
                    const ProgressFn& progress ) {
      if ( beads.empty() ) {
         throw std::runtime_error( "GRPY: asked to solve a model with no beads." );
      }
      if ( !QFileInfo( cfg_.program ).isExecutable() ) {
         throw std::runtime_error(
            QString( "GRPY: the GRPY program '%1' is missing or is not executable."
                     " Check the UltraScan installation." )
            .arg( cfg_.program ).toStdString() );
      }

      const bool unreduced = cfg_.full_beads > 0
                             && (int) beads.size() == cfg_.full_beads
                             && !cfg_.full_input.isEmpty()
                             && QFileInfo( cfg_.full_input ).exists();
      ScopedFile rung;
      if ( !unreduced ) {
         rung.path = write_rung_file( beads, p );
      }
      const QString input = unreduced ? cfg_.full_input : rung.path;

      QString stderr_text;
      const QString report = run_program( input, progress, stderr_text );
      if ( !report.contains( "GRPY program" ) ) {
         throw std::runtime_error(
            QString( "GRPY: the GRPY program produced no report.%1" )
            .arg( stderr_text.isEmpty() ? QString() : QString( "\n%1" ).arg( stderr_text ) )
            .toStdString() );
      }

      Results r;
      parse_report( report, r );
      r.mass = p.mw;
      return r;
   }

private:
   Config                cfg_;
   std::function<bool()> should_stop_;
   int                   rung_serial_ = 0;

   // A rung's bead subset, in the GRPY native format SOMO already writes. Coordinates are
   // written at full precision: they came from a file the program itself will read back, so
   // anything shorter would move the model between rungs.
   QString write_rung_file( const std::vector<Bead>& beads, const PhysParams& p ) {
      const QString dir  = cfg_.rung_dir.isEmpty() ? QDir::tempPath() : cfg_.rung_dir;
      const QString path = QDir( dir ).filePath( QString( "somo_grpy_rung_%1_%2.grpy" )
                                                 .arg( QCoreApplication::applicationPid() )
                                                 .arg( ++rung_serial_ ) );
      QFile f( path );
      if ( !f.open( QIODevice::WriteOnly | QIODevice::Text ) ) {
         throw std::runtime_error(
            QString( "GRPY: cannot write the reduced model file '%1'." ).arg( path ).toStdString() );
      }
      double mw = p.mw;
      if ( mw <= 0 ) {
         mw = 0;
         for ( const Bead& b : beads ) {
            mw += b.mw;
         }
      }
      QTextStream ts( &f );
      ts.setRealNumberPrecision( 17 );
      ts << "shell reduction rung\tTitle\n"
         << p.temperature_C << "\tTemperature\n"
         << p.eta << "\tSolvent viscosity\n"
         << mw << "\tMolecular weight\n"
         << p.vbar << "\tSpecific volume of macromolecule\n"
         << p.rho << "\tSolution Density\n"
         << p.units << "\t\t\t\tUnit of length for coordinates and radii, cm\n"
         << (int) beads.size() << "\tNumber of beads\n";
      for ( const Bead& b : beads ) {
         ts << b.x << "\t" << b.y << "\t" << b.z << "\t" << b.radius << "\n";
      }
      f.close();
      return path;
   }

   // One invocation. Progress is delivered as it arrives, so the bar moves during the
   // solve, and the stop predicate is consulted between reads so a running solve can be
   // ended -- which the in-process solver could not do, its factorization having no
   // interior abort.
   QString run_program( const QString& input, const ProgressFn& progress, QString& stderr_text ) {
      QProcess proc;
      if ( !cfg_.working_dir.isEmpty() ) {
         proc.setWorkingDirectory( cfg_.working_dir );
      }
      QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
      if ( cfg_.single ) {
         env.insert( "GRPY_SINGLE", "1" );
      }
      if ( !cfg_.ooc_dir.isEmpty() ) {
         env.insert( "GRPY_OOC", cfg_.ooc_dir );
      }
      if ( cfg_.threads > 0 ) {
         env.insert( "GRPY_THREADS", QString::number( cfg_.threads ) );
      }
      // Ask for the high-precision report. The program's default report is the Fortran's
      // own ES11.3 -- four significant figures -- which is right for a drop-in replacement
      // a human reads, and wrong for a caller that DIFFERENCES successive results.
      //
      // The shell reduction's error estimate is built from gaps between consecutive rungs
      // and from the ratio of two such gaps, so a relative quantisation of ~1e-4 in the
      // values lands directly on the bar. Measured over 266 reduced evaluations by rounding
      // recorded full-precision rung values through %11.3E: the reported estimate moves by
      // a median 2.5% and up to 17.6%, the observed order shifts by up to 0.26, the floor
      // changes which term governs in 14 cases, and 2 stopping decisions flip. Coverage
      // happened to survive on that corpus -- the minimum margin fell only 1.131x to
      // 1.119x -- so this is not a correctness fix, but none of that movement should exist:
      // it is an output format leaking into a numerical result.
      env.insert( "GRPY_HP", "1" );
      proc.setProcessEnvironment( env );

      proc.start( cfg_.program, QStringList() << "-e" << input, QIODevice::ReadOnly );
      if ( !proc.waitForStarted( 30000 ) ) {
         throw std::runtime_error(
            QString( "GRPY: could not start '%1': %2" )
            .arg( cfg_.program ).arg( proc.errorString() ).toStdString() );
      }

      QString report;
      QString carry;
      stderr_text.clear();
      while ( proc.state() != QProcess::NotRunning ) {
         if ( should_stop_ && should_stop_() ) {
            proc.kill();
            proc.waitForFinished( 5000 );
            throw Stopped();
         }
         if ( proc.waitForReadyRead( 100 ) ) {
            report += consume_progress( QString::fromLocal8Bit( proc.readAllStandardOutput() ),
                                        progress, carry );
         }
         // Drained every pass, not once at the end. The channels are separate and each has a
         // finite pipe buffer: a child that writes more to stderr than that buffer holds
         // blocks in write(), which stops it producing stdout, which stops this loop making
         // progress -- and waitForFinished( -1 ) below then waits forever. Reading stderr
         // here keeps the child running whatever it chooses to write.
         stderr_text += QString::fromLocal8Bit( proc.readAllStandardError() );
      }
      proc.waitForFinished( -1 );
      report += consume_progress( QString::fromLocal8Bit( proc.readAllStandardOutput() ),
                                  progress, carry, true );
      stderr_text += QString::fromLocal8Bit( proc.readAllStandardError() );
      stderr_text = stderr_text.trimmed();

      if ( proc.exitStatus() != QProcess::NormalExit ) {
         throw std::runtime_error(
            QString( "GRPY: the GRPY program crashed.%1" )
            .arg( stderr_text.isEmpty() ? QString() : QString( "\n%1" ).arg( stderr_text ) )
            .toStdString() );
      }
      if ( proc.exitCode() != 0 ) {
         throw std::runtime_error(
            QString( "GRPY: the GRPY program failed (exit %1).%2" )
            .arg( proc.exitCode() )
            .arg( stderr_text.isEmpty() ? QString() : QString( "\n%1" ).arg( stderr_text ) )
            .toStdString() );
      }
      return report;
   }
};

}  // namespace grpy
