//! \file us_data_loader.h
#ifndef US_DATA_LOAD_H
#define US_DATA_LOAD_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_widgets.h"
#include "us_dataIO2.h"
#include "us_db2.h"
#include "us_help.h"

//! \brief A dialog class to select and load data (edited or raw)
class US_EXTERN US_DataLoader : public US_WidgetsDialog
{
   Q_OBJECT

   public:

      //! \brief Main constructor for dialog to select edit and raw
      //!        data and load it
      //! \param late     Flag: present only latest if editsel=true
      //! \param local    Flag: default data source to local disk
      //! \param rData    A reference to a vector of rawData objects
      //! \param eData    A reference to a vector of editedData objects
      //! \param trips    A reference to a list of triples to fill
      //! \param desc     A concatenatination of DataDesc information.

      US_DataLoader( bool, int,
                     QVector< US_DataIO2::RawData >&,
                     QVector< US_DataIO2::EditedData >&,
                     QStringList&,
                     QString& );
 
   signals:
      //! \brief A signal to tell the parent when the disk/db selection
      //!        has changed
      //! \param DB True if DB is selected
      void changed( bool DB );

      //! \brief A signal to provide the parent with a progress message
      void progress( const QString& );

   private:
      US_Help showHelp;

      class DataDesc   // description of each data set in the list presented
      {
         public:
         QString runID;           // run identifier string
         QString tripID;          // triple identifier string
         QString editID;          // edit identifier string
         QString label;           // identifying label for choice list
         QString descript;        // full descriptive string
         QString filename;        // file name
         QString dataGUID;        // data Global Identifier
         QString aucGUID;         // AUC GUID (same as dataGUID for raw)
         QString date;            // date/time last updated
         int     DB_id;           // database ID number
         int     tripknt;         // count of triples per run
         int     tripndx;         // index of triple in run
         int     editknt;         // count of edits per triple
         int     editndx;         // index of edit in triple
         bool    isEdit;          // flag:  is this edit? False->raw
         bool    isLatest;        // flag:  are edits latest ones
      };

      QMap< QString, DataDesc > datamap;  // map of labels,data-desc-objs

      DataDesc       ddesc;       // current selected data's description object
      QStringList    dlabels;     // keys for datamap

      QLineEdit*     le_invest;   // investigator text entry
      QLineEdit*     le_dfilter;  // data search filter text entry

      QTreeWidget*   tw_data;     // data list widget

      bool           latest;      // current use-lastest-edit flag

      QString        dsearch;     // current data search string
      QString        dinvtext;    // current data investigator string

      US_Disk_DB_Controls* disk_controls; //!< Radiobuttons for disk/db choice

      QVector< US_DataIO2::RawData    >& rawData;
      QVector< US_DataIO2::EditedData >& editedData;
      QStringList&                       triples;
      QString&                           description;

      bool load_edit      ( void );
      void describe       ( void );
      void scan_dbase_edit( void );
      void scan_local_edit( void );
      void pare_to_latest ( void );
      void show_data_info ( QPoint );

   private slots:
      void list_data      ( bool = true);
      void get_person     ( void );
      void update_person  ( int, const QString&, const QString& );
      void search         ( const QString& );
      void cancelled      ( void );
      void accepted       ( void );
      void help           ( void )
      { showHelp.show_help( "data_loader.html" ); };

   protected:
      //! \brief Event filter to flag whether a mouse click is
      //!        with the right mouse button
      //! \param obj Pointer to object on which event occurred.
      //! \param e   Pointer to event to filter.
      //! \return    Flag if event has been completely handled.
      bool eventFilter(        QObject*, QEvent* );
};
#endif
