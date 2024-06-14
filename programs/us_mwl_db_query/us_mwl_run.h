//! \file us_mwl_run.h
#ifndef US_MWL_RUN_H
#define US_MWL_RUN_H

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_db2.h"

//! \class US_MwlRun
//! This class creates a dialog with all the local MWL runs,
//! dates last updated, and files-in-directory counts displayed for the
//! current user. When the user clicks the select button, the highlighted
//! runID's full directory path is passed back to the calling program.
class US_MwlRun : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! \brief  Generic constructor for the US_MwlRun dialog.
      //! \param runID    A reference for returning selected runID's directory.
      //! \param isRawMwl Flag of whether Raw MWL files are desired.
      US_MwlRun( QString&, bool = true  );

      //! \class RunInfo
      //! This class is used to store the information to display
      class RunInfo
      {
         public:
         QString  runID;      //!< The run directory
         QString  date;       //!< The date the experiment was last updated
         int      nfiles;     //!< Number of MWL files in directory
      };

   private:
      QString&           runID;
      bool               isRawMwl;

      QLineEdit*         le_search;
      QTableWidget*      tw;
      QList< RunInfo >   runInfo;
      QString            resdir;

   private slots:
      void load_files    ( void );
      void select        ( void );
      void populate_list ( void );
      void limit_data    ( const QString& );
};
#endif
