//! \file us_loadable_noise.h
#ifndef US_LOADABLENOISE_H
#define US_LOADABLENOISE_H

#include <QtCore>
#if QT_VERSION > 0x050000
#include <QtWidgets>
#else
#include <QtGui>
#endif
#include "us_extern.h"
#include "us_dataIO.h"
#include "us_model.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug() //!< debug-level-conditioned qDebug()
#endif

//! \brief Count loadable noise available and set up for load

/*! \class US_LoadableNoise

  Provides a function to count noises available on disk or in the
  database that match edited data and, possible, a loaded model
*/

class US_GUI_EXTERN US_LoadableNoise : public QObject
{
  Q_OBJECT

  public:
    /*! \brief Simple constructor.
        \param parent A pointer to the parent QObject.  This normally can be 
                      left as NULL.
    */  
    US_LoadableNoise( QObject* = 0 );

    /*! \brief Count available noise records and set up for load dialog
        \param ondisk   Flag for noise source: true=local; false=db.
        \param edata    Pointer to edited data for which to search noise.
        \param model    Optional pointer to loaded model, priority noise.
        \param mieGUIDs List of returned model-in-edit ModelGUID strings.
        \param nieGUIDs List of returned noise-in-edit NoiseGUID strings.
        \return         Count of noises available for given edit.
    */
    int count_noise( bool, US_DataIO::EditedData*, US_Model*,
                     QStringList&, QStringList& );

  private:
    // Build a list of models(GUIDs) for a given edit(GUID)
    int models_in_edit( bool, QString, QStringList& );

    // Build a list of noises(GUIDs) for a given model(GUID)
    int noises_in_model( bool, QString, QStringList& );

    // Build noise,model IDs list for database
    int id_list_db( QString );

    // Build noise,model IDs list for local disk
    int id_list_disk( QString );

    int dbg_level;

    QStringList noiIDs;      // Noise GUIDs
    QStringList noiEdIDs;    // Noise edit GUIDs
    QStringList noiMoIDs;    // Noise model GUIDs
    QStringList noiTypes;    // Noise types
    QStringList modIDs;      // Model GUIDs
    QStringList modEdIDs;    // Model edit GUIDs
    QStringList modDescs;    // Model descriptions
};
#endif

