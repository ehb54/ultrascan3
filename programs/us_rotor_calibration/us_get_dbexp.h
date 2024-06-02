//! \file us_get_dbexp.h
#ifndef US_GET_DBEXP_H
#define US_GET_DBEXP_H

#include "us_extern.h"
#include "us_widgets_dialog.h"

/*! \class US_GetDBExp
           This class creates a dialog with all the experimentIDs,
           dates last updated, runIDs and labels belonging to the
           current user displayed. When the user clicks the select 
           button, the highlighted experimentID is passed back to
           the calling program.
*/
class US_GetDBExp : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! \brief  Generic constructor for the US_GetDBExp dialog.
      //! \param  eID Where the selected runID will be stored
      US_GetDBExp( QString& );

      //! \class RunInfo
      //!  Used to store the information to display
      class RunInfo
      {
         public:
         int      ID;             //!< The experiment ID
         QString  date;           //!< The date the experiment was last updated
         QString  runID;          //!< The unique per-investigator runID
         QString  label;          //!< The identifying label
      };
      
   private:
      QTableWidget*      tw;
      QString&           expID;
      QList< RunInfo >   runInfo;

      bool loadData      ( void );

   private slots:
      void columnClicked ( int );
      void select        ( void );
};
#endif
