//! \file us_xpn_run_auc.h
#ifndef US_XPN_RUN_AUC_H
#define US_XPN_RUN_AUC_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_db2.h"

//! \class US_XpnRunAuc
//! This class creates a dialog with all the local XPN runs,
//! dates last updated, and triples-in-run counts displayed for the
//! current user. When the user clicks the select button, the highlighted
//! runID's Id string (full directory path or DB identifier) is passed
//! to the calling program.
class US_XpnRunAuc : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! \brief  Generic constructor for the US_XpnRunAuc dialog.
      //! \param runID    A reference for returning selected runID's directory.
      US_XpnRunAuc( QString&  );

      //! \class RunInfo
      //! This class is used to store the information to display
      class RunInfo
      {
         public:
         QString  runID;      //!< The run Id name string
         QString  date;       //!< Date the experiment was last updated
         int      ntriple;    //!< Number of triples for the run
         int      dbId;       //!< Database ID of run (-1 => local)
      };

   private:
      QString&           runID;
      bool               doRawXpn;

      QLineEdit*         le_search;
      QTableWidget*      tw;
      QList< RunInfo >   runInfo;
      QString            impdir;

   private slots:
      void load_runs     ( void );
      void select        ( void );
      void populate_list ( void );
      void limit_data    ( const QString& );
};
#endif
