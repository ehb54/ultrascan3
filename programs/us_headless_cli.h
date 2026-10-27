//! \file us_headless_cli.h
//! \brief Shared command-line support for QApplication-based simulation tools.
//! Parses the options US_SimCli declares, handles input-loading failures, and
//! decides whether to show the GUI or return a status code.
#ifndef US_HEADLESS_CLI_H
#define US_HEADLESS_CLI_H

#include <QApplication>
#include <QDebug>
#include <QRandomGenerator>
#include <QTextStream>

#include "us_sim_cli.h"

//! \brief Parse the simulator's command line, printing help, version or errors.
//! \return true if the caller should return immediately using exit_code;
//! false if it should continue with args.
inline bool parseSimCli( US_SimCli::Tool tool, US_SimCliArgs& args, int& exit_code )
{
   QString output;
   exit_code = US_SimCli::parse( tool, QApplication::arguments(), args, output );
   if ( exit_code < 0 )
   {
      return false;
   }
   QTextStream( exit_code == 0 ? stdout : stderr ) << output << Qt::endl;
   return true;
}

//! \brief Give a started run without --noise-seed a fresh seed, and print it.
//! Unseeded, the generator starts from the same fixed state in every process,
//! so replicate runs would all get the same noise.
inline void chooseNoiseSeedIfUnset( US_SimCliArgs& args )
{
   if ( !args.start || args.noise_seed != 0 )
   {
      return;
   }

   // Zero is reserved: US_Math2::randomize() reads it as "use the clock".
   quint32 seed = 0;
   while ( seed == 0 )
   {
      seed = QRandomGenerator::system()->generate();
   }

   args.noise_seed = seed;
   QTextStream( stdout ) << "Noise seed: " << seed << " (pass --noise-seed " << seed << " to reproduce this run's noise)"
                         << Qt::endl;
}

//! \brief Handle an input-loading failure during a headless run.
//! With --errors-cl, log the failure and exit with status 2. Otherwise, mark
//! the run as requiring the GUI.
inline void reportHeadlessLoadFailure( const QString& what,
                                       const QString& id,
                                       bool           errors_to_cl,
                                       bool&          gui_needed,
                                       bool&          error_occured )
{
   if ( errors_to_cl )
   {
      qDebug() << "Error loading" << what << id;
      exit( 2 );
   }
   gui_needed    = true;
   error_occured = true;
}

//! \brief Show the GUI if the headless run could not finish.
//! If --errors-cl is set, return the initialization status without opening
//! the GUI.
template <typename Widget> inline int showGuiIfNeeded( Widget& w, int init_status, const US_SimCliArgs& args )
{
   if ( init_status != 0 && !args.errors_cl )
   {
      w.show();
      return QApplication::exec();
   }
   return init_status;
}

#endif // US_HEADLESS_CLI_H
