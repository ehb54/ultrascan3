//! \file us_sim_input_loader.h
#ifndef US_SIM_INPUT_LOADER_H
#define US_SIM_INPUT_LOADER_H

#include <QString>

#include "ius_db2.h"
#include "us_buffer.h"
#include "us_extern.h"
#include "us_model.h"
#include "us_rotor.h"

//! \brief Finds and loads the model, buffer and rotor a simulator is given on
//! its command line, from the database or the local data folders, without
//! any dialog.
//!
//! Each argument is matched as the selection dialogs match it: a readable
//! file path is loaded as such; otherwise the argument is looked for among
//! the investigator's records. Pass an open connection to search the
//! database, or none to search the local data folders.
//!
//! Where it differs from the dialogs: a database buffer may also be given
//! by GUID; local rotors are listed from every lab's own folder (the dialog
//! listed the current lab's rotors for each lab) and a lab without rotor
//! files is skipped; and a path that is not a readable file, a rotor or
//! calibration that cannot be read, are reported rather than ignored.
class US_UTIL_EXTERN US_SimInputLoader
{
   public:
      //! \brief Load a model given as a file path, a model GUID, or (from the
      //! database) a model ID.
      static bool model( const QString& spec, IUS_DB2* db, int investigator, US_Model& model, QString& error );

      //! \brief Load a buffer given as a file path, a buffer GUID, or (from
      //! the database) a buffer ID.
      static bool buffer( const QString& spec, IUS_DB2* db, int investigator, US_Buffer& buffer, QString& error );

      //! \brief The rotor and calibration a rotor argument names.
      struct RotorChoice
      {
            //! "c1:c2" gives only the two coefficients, no rotor.
            bool                       coefficientsOnly = false;
            US_Rotor::Rotor            rotor;
            US_Rotor::RotorCalibration calibration;
            double                     coeff1 = 0.0;
            double                     coeff2 = 0.0;
      };

      //! \brief Find a rotor given as "c1:c2" coefficients, or as a rotor ID,
      //! a rotor name or a lab ID, as the rotor dialog's list entries
      //! "ID:name:labID" are matched. Its first calibration profile is used,
      //! and a rotor or calibration read from the database is saved to disk,
      //! as the dialog does.
      static bool rotor( const QString& spec, IUS_DB2* db, RotorChoice& choice, QString& error );

      //! \brief Whether a rotor list entry "ID:name:labID" matches a rotor
      //! argument, as the rotor dialog decides it.
      static bool rotorEntryMatches( const QString& entry, const QString& spec );

      //! \brief Parse "c1:c2" rotor coefficients; false when the argument is
      //! not two colon-separated parts.
      static bool rotorCoefficients( const QString& spec, double& coeff1, double& coeff2 );
};

#endif
