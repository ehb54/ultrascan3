//! \file us_select_runid.h
#ifndef US_SELECT_RUNID_H
#define US_SELECT_RUNID_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_widgets.h"
#include "us_dataIO2.h"
#include "us_db2.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug() //!< debug-level-conditioned qDebug()
#endif

//! \brief A dialog class to select run ID for combo discrete distributions plot
class US_SelectRunid : public US_WidgetsDialog
{
   Q_OBJECT

   public:

      //! \brief Main constructor for dialog to select edits(s)
      //! \param dbase    Flag: Data source is database
      //! \param runids   A reference to the selected runIDs list
      //! \param mdescrs  A reference to the model descriptions for runs

      US_SelectRunid( bool, QStringList&, QStringList& );

   signals:
      //! \brief A signal to tell the parent when the disk/db selection
      //!        has changed
      //! \param isDB  True if DB is selected
      void changed( bool isDB );

   private:
      QStringList&   runIDs;      // Selected run IDs
      QStringList&   mDescrs;     // Selected runs' model descriptions
      bool           sel_db;      // Select-database flag

      QStringList    mRunIDs;     // Model run IDs

      QPushButton*   pb_invest;   // Investigator button

      QLineEdit*     le_invest;   // Investigator id:name text entry
      QLineEdit*     le_dfilter;  // Data search filter text entry

      QTextEdit*     te_status;   // Text box for current list status

      QListWidget*   lw_data;     // Data list widget

      US_Disk_DB_Controls* dkdb_cntrls;  // Radio buttons for disk/db choice

      QString        dsearch;     // Current data search string

      QStringList    rlabels;     // Run ID labels

      int            count_allr;  // Count of all user runs
      int            count_list;  // Count of discrete distribution runs
      int            count_seld;  // Count of runs currently selected;
      int            dbg_level;   // Debug level

   private slots:

      void scan_dbase_runs ( void );
      void scan_local_runs ( void );
      void list_data       ( void );
      void search          ( const QString& );
      void cancelled       ( void );
      void accepted        ( void );
      void get_person      ( void );
      void update_person   ( int  );
      void update_disk_db  ( bool );
      void selectionChanged( void );
};
#endif
