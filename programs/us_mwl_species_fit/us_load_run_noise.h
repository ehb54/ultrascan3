//! \file us_load_run_noise.h
#ifndef US_LOADRUNNOISE_H
#define US_LOADRUNNOISE_H

#include <QtCore>
#include <QtGui>
#include "us_extern.h"
#include "us_dataIO.h"
#include "us_model.h"
#include "us_db2.h"
#include "us_noise.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug() //!< debug-level-conditioned qDebug()
#endif

//! \brief Load the latest noises for a run and set up for edit loads

//! \class US_LoadRunNoise
//!  Provides a function to count noises available on disk or in the
//!  database that match a run ID. Once internally loaded, the noises can be
//!  fetched for each editedData in the run.

class US_LoadRunNoise : public QObject
{
  Q_OBJECT

  public:
    //! \brief Simple constructor.
    //! \param parent A pointer to the parent QObject.  This normally can be 
    //!               left as NULL.
    US_LoadRunNoise( QObject* = 0 );

    //! \brief Count available noise records and set up for load dialog.
    //! \param dbload   Flag for noise source is DB: true=db; false=local.
    //! \param runID    Run ID for which to determine latest noises.
    //! \param edIDsP   Pointer for return of edit GUIDs for the run.
    //! \param noiInfP  Pointer for return of available noises summary strings.
    //! \param neditsP  Pointer for return of latest-edits-in-run count.
    //! \return         Count of total noises available for given run.
    int count_noise( const bool, const QString, QStringList*, QStringList*,
                     int* );

    //! \brief Return noises for a given edit GUID.
    //! \param edGUID   Edit GUID for which to return noise(s).
    //! \param ti_noise Returned TI noise for edit.
    //! \param ri_noise Returned RI noise for edit.
    //! \return         Noise type flag: 0,1,2,3 -> none,TI,RI,TI+RI.
    int get_noises( const QString, US_Noise&, US_Noise& );

  private:
    // Scan database for edits and noises related to the run
    int scan_db  ( const QString );

    // Scan local disk for edits and noises related to the run
    int scan_disk( const QString );

    int         dbg_level;

    US_DB2*     dbP;               // Database pointer (or NULL)

    QStringList edGUIDs;           // Edits-in-run GUIDs
    QStringList noiInfos;          // Noise info strings for each edit

    QMap< QString, int > edIdMap;  // Map of DB ID's to edit GUIDs
    QMap< QString, int > noIdMap;  // Map of DB ID's to noise GUIDs

};
#endif

