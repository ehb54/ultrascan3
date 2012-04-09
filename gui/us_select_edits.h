//! \file us_select_edits.h
#ifndef US_SELECT_EDITS_H
#define US_SELECT_EDITS_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_dataIO2.h"
#include "us_db2.h"

//! \brief A dialog class to select run or edit IDs for models,noises pre-filter
class US_GUI_EXTERN US_SelectEdits : public US_WidgetsDialog
{
   Q_OBJECT

   public:

      //! \brief Main constructor for dialog to select edits(s)
      //! \param dbase    Flag: Data source is database
      //! \param runsel   Flag: Select runIDs (false->edits)
      //! \param late     Flag: Present only latest if runsel=false
      //! \param edIDs    A reference to a list of selected editIDs

      US_SelectEdits( bool, bool&, bool&, QStringList& );

   private:
      bool&          sel_run;     // Current select-runid flag
      bool&          sel_late;    // Current use-lastest-edit flag
      QStringList&   editIDs;     // Selected edit IDs (GUIDs for local)

      bool           sel_db;      // Select-database flag

      US_Help showHelp;

      class EditDesc   // description of each edit set in the list presented
      {
         public:
         QString label;           // Identifying label for choice list
         QString editID;          // Edit ID (DB) string
         QString editGUID;        // Edit GUID
         QString date;            // Updated-date string
      };

      QMap< QString, EditDesc > editmap;  // map of labels,data-desc-objs

      EditDesc       edesc;       // Current selected data's description object
      QStringList    elabels;     // Keys for editmap
      QStringList    rlabels;     // RunIDs in editmap

      QRadioButton*  rb_runid;    // RunID radio button
      QRadioButton*  rb_latest;   // Latest-edit radio button
      QRadioButton*  rb_alledit;  // All-edits radio button

      QLineEdit*     le_dfilter;  // Data search filter text entry

      QLabel*        lb_list;     // Label at top of data list

      QListWidget*   lw_data;     // Data list widget

      QString        dsearch;     // Current data search string

   private slots:

      void scan_dbase_edit( void );
      void scan_local_edit( void );
      void build_runids   ( void );
      void pare_to_latest ( void );
      void list_data      ( void );
      void search         ( const QString& );
      void cancelled      ( void );
      void accepted       ( void );
      void help           ( void )
      { showHelp.show_help( "select_edits.html" ); };

};
#endif
