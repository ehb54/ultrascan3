//! \file us_xpn_run_raw.h
#ifndef US_XPN_RUN_RAW_H
#define US_XPN_RUN_RAW_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_db2.h"

//! \class US_XpnRunRaw
//! This class creates a dialog with all the local XPN runs,
//! dates last updated, and triples-in-run counts displayed for the
//! current user. When the user clicks the select button, the highlighted
//! runID's Id string (full directory path or DB identifier) is passed
//! to the calling program.
class US_XpnRunRaw : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! \brief  Generic constructor for the US_XpnRunRaw dialog.
      //! \param runDesc  A reference for returning selected runID's description.
      //! \param rdLists  A reference to run description strings list.
      US_XpnRunRaw( QString&, QStringList&  );

      //! \class RunInfo
      //! This class is used to store the information to display
      class RunInfo
      {
         public:
         QString  runID;      //!< The run Id name string
         QString  date;       //!< Date the experiment was last updated
         QString  expname;    //!< Experiment name
         QString  resname;    //!< Researcher name
         int      dbId;       //!< Database (run) ID of run
         int      expId;      //!< Experiment ID of run
         int      dcount;     //!< Data tables count for run
         bool     abscnf;     //!< Absorbance scans present flag
         bool     flscnf;     //!< Fluorescence scans present flag
         bool     inscnf;     //!< Interference scans present flag
         bool     wascnf;     //!< Wavelength scans present flag
      };

   private:
      QString&           runDesc;
      QStringList&       rdLists;

      QString            runID;
      QLineEdit*         le_search;
      QTableWidget*      tw;
      QList< RunInfo >   runInfo;

   private slots:
      void load_runs     ( void );
      void select        ( void );
      void populate_list ( void );
      void limit_data    ( const QString& );
};
#endif
