//! \file us_auto_noise.h
#ifndef US_AUTO_NOISE_H
#define US_AUTO_NOISE_H

#include "us_dataIO.h"
#include "us_extern.h"
#include "us_model.h"
#include "us_noise.h"

#include <QWidget>

//! \brief The noise that autoflow analysis and the GMP reporter load for a
//! model: the newest time- and radially-invariant noise of the model; if it
//! has none, the noise of other models of the same edit, as the edit's noise
//! list orders them. Noise shorter than the data is padded with zeroes.
class US_GUI_EXTERN US_AutoNoise
{
   public:
      //! \brief Load the noise of model \a triple_information["mID"] into
      //! \a ti_noise and \a ri_noise.
      //! \param parent   Parent of the messages
      //! \param invID    Investigator whose noise and model records are listed
      //! \param edata    The model's edited data
      //! \param model    The loaded model
      //! \return False if the database cannot be reached.
      static bool load( QWidget*                parent,
                        int                     invID,
                        QMap<QString, QString>& triple_information,
                        US_DataIO::EditedData*  edata,
                        US_Model&               model,
                        US_Noise&               ti_noise,
                        US_Noise&               ri_noise );

   private:
      US_AutoNoise( QWidget*               parent,
                    int                    invID,
                    US_DataIO::EditedData* edata,
                    US_Model&              model,
                    US_Noise&              ti_noise,
                    US_Noise&              ri_noise );

      bool                   loadNoises( QMap<QString, QString>& triple_information );
      void                   loadNoises_whenAbsent();
      int                    count_noise_auto( US_DataIO::EditedData*, US_Model*, QStringList&, QStringList& );
      int                    noises_in_model_auto( QString, QStringList& );
      int                    models_in_edit_auto( QString, QStringList& );
      int                    id_list_db_auto( QString );

      QWidget*               parent;
      int                    invID;
      int                    dbg_level;
      US_DataIO::EditedData* edata;
      US_Model&              model;
      US_Noise&              ti_noise;
      US_Noise&              ri_noise;

      QStringList            noiIDs;   //!< Noise GUIDs
      QStringList            noiEdIDs; //!< Noise edit GUIDs
      QStringList            noiMoIDs; //!< Noise model GUIDs
      QStringList            noiTypes; //!< Noise types
      QStringList            modIDs;   //!< Model GUIDs
      QStringList            modEdIDs; //!< Model edit GUIDs
      QStringList            modDescs; //!< Model descriptions
};

#endif
