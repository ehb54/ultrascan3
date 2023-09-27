#ifndef US_GET_RUN_H
#define US_GET_RUN_H

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_db2.h"

//! \class US_GetRun
//!        This class creates a dialog with all the experimentIDs,
//!        dates last updated, runIDs and labels belonging to the
//!        current user displayed. When the user clicks the select 
//!        button, the highlighted runID is passed back to
//!        the calling program.
class US_GetRun : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! \brief  Generic constructor for the US_GetRun dialog.
      //  \param run   A reference for the returned selected runID
      //  \param inDB  Flag of whether data source is database
      US_GetRun( QString&, bool = true  );

      //! \class RunInfo
      //! This class is used to store the information to display
      class RunInfo
      {
         public:
         int      ID;         //!< The experiment ID
         QString  date;       //!< The date the experiment was last updated
         QString  runID;      //!< The unique per-investigator runID
         QString  label;      //!< The identifying label
      };

   signals:
      //! \brief A signal to tell the parent when the disk/db selection changes.
      //! \param DB  True if DB is now selected
      void dkdb_changed( bool DB );

   private:
      QString&           runID;

      QPushButton*       pb_invest;
      QLineEdit*         le_invest;
      QLineEdit*         le_search;
      QTableWidget*      tw;
      QList< RunInfo >   runInfo;
      US_Help            showHelp;

      int                personID;


      US_Disk_DB_Controls*    dkdb_cntrls;

   private slots:
      void load_db            ( void );
      void load_disk          ( void );
      void sel_investigator   ( void );
      void assign_investigator( int  );
      void select        ( void );
      void deleteRun     ( void );
      bool check_filename_for_autoflow( QString );
      void populate_list ( void );
      void update_disk_db( bool );
      void limit_data    ( const QString& );
      void help( void )
      { showHelp.show_help( "get_run.help" ); };
};
#endif
