//! \file us_sim_cli.h
#ifndef US_SIM_CLI_H
#define US_SIM_CLI_H

#include <QString>
#include <QStringList>

#include "us_extern.h"

//! \brief A simulator's parsed and validated command-line options.
//! An empty string, a -1 index, or a zero cell, channel or seed means the
//! option was not given.
class US_UTIL_EXTERN US_SimCliArgs
{
   public:
      bool        given     = false;    //!< Any option other than --no-db was given
      bool        no_db     = false;    //!< Use only locally available data
      bool        errors_cl = false;    //!< Report errors to the console, never the GUI
      bool        close     = false;    //!< Close if no errors occurred
      bool        start     = false;    //!< Start simulating; implied by --save
      bool        movie     = false;    //!< Show the simulation as a movie (us_astfem_sim)
      bool        timecorr  = false;    //!< Use time correction (us_astfem_sim)
      QString     model;                //!< Model to load (us_astfem_sim)
      QStringList models;               //!< Model files, one per wavelength (us_mwl_species_sim)
      QString     buffer;               //!< Buffer to load
      QString     simparams;            //!< Simulation parameters file
      QString     rotor;                //!< Rotor to load
      int         centerpiece     = -1; //!< Centerpiece-list index
      int         centerpiece_row = -1; //!< Row within the centerpiece
      int         cell            = 0;  //!< Output cell, 1 to 8 (us_astfem_sim)
      char        channel         = 0;  //!< Output channel letter (us_astfem_sim)
      QString     run_type;             //!< Two-character optical data type
      QString     description;          //!< Description stored with the data
      quint32     noise_seed = 0;       //!< Noise generator seed
      QString     save;                 //!< Output directory, with forward slashes
      QString     guid_seed;            //!< Seed for reproducible GUIDs
      QString     edit_stamp;           //!< yyMMddhhmm for edit filenames
};

//! \brief The simulators' command-line options.
//! Each option is declared once, with its help text and validator, in a table
//! that parse() registers, parses and validates in one pass.
class US_UTIL_EXTERN US_SimCli
{
   public:
      //! \brief The simulators; each option names the ones that accept it.
      enum Tool
      {
         Astfem     = 0x1, //!< us_astfem_sim
         MwlSpecies = 0x2  //!< us_mwl_species_sim
      };

      //! \brief Supported optical data types.
      //! RI and WI would need conversion to intensity; WA and WI are wavelength scans.
      static QStringList           runTypes();

      //! \brief The error parse() reports for an invalid option value.
      //! Tests compare against this rather than repeating the text.
      //! \param option The option name, without the leading dashes.
      //! \param value  The value as given.
      static QString               invalidValueMessage( const QString& option, const QString& value );

      //! \brief Error formats shared with the tests; fill them with QString::arg().
      //! A --cell beyond the loaded rotor's holes: %1 cell, %2 holes.
      static constexpr const char* cellBeyondRotorMessage    = "--cell %1 exceeds the rotor's %2 holes";
      //! A --channel whose centerpiece row differs from --centerpiece-channel's:
      //! %1 channel letter, %2 its row, %3 the selected row.
      static constexpr const char* channelRowMismatchMessage = "--channel %1 is centerpiece row %2, but "
                                                               "--centerpiece-channel selects row %3";
      //! Simulation parameters with other than one speed step: %1 file, %2 step count.
      static constexpr const char* speedStepCountMessage     = "%1 has %2 speed steps; only one can be simulated";

      //! \brief Parse a simulator's command line.
      //! \param tool      The simulator being run.
      //! \param arguments The command line, program name first.
      //! \param args      Filled with the parsed options.
      //! \param output    Help, version or error text to print before exiting.
      //! \return -1 to continue; otherwise the status to exit with immediately,
      //!         after printing output to stdout for 0 or stderr otherwise.
      static int                   parse( Tool tool, const QStringList& arguments, US_SimCliArgs& args, QString& output );
};

#endif
