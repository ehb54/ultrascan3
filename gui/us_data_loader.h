//! \file us_data_loader.h
#ifndef US_DATA_LOAD_H
#define US_DATA_LOAD_H

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_dataIO.h"
#include "us_db2.h"

//! \brief A dialog class to select and load data (edited or raw)
class US_GUI_EXTERN US_DataLoader : public US_WidgetsDialog
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
      //! \param desc     A concatenation of DataDesc information
      //! \param tfilt    Optional experiment type filter (default=Velocity).

      US_DataLoader( bool, int,
                     QVector< US_DataIO::RawData >&,
                     QVector< US_DataIO::EditedData >&,
                     QStringList&,
                     QString&,
                     QString = "" );
      
      US_DataLoader( bool, int,
                     QVector< US_DataIO::RawData >&,
                     QVector< US_DataIO::EditedData >&,
                     QStringList&,
                     QString&,
		     QMap<QString,QString>&,
                     QString = "");

      bool us_automode;
 
   signals:
      //! \brief A signal to tell the parent when the disk/db selection
      //!        has changed
      //! \param DB True if DB is selected
      void changed( bool DB );

      //! \brief A signal to provide the parent with a progress message
      void progress( const QString );

   private:
      US_Help showHelp;

      class DataDesc   // Description of each data set in the list presented
      {
         public:
         QString runID;           // Run identifier string
         QString tripID;          // Triple identifier string
         QString editID;          // Edit identifier string
         QString label;           // Identifying label for choice list
         QString descript;        // Full descriptive string
         QString filename;        // File name
         QString dataGUID;        // Data Global Identifier
         QString aucGUID;         // AUC GUID (same as dataGUID for raw)
         QString date;            // Date/time last updated
         QString acheck;          // Checksum + size of auc record
         QString echeck;          // Checksum + size of edit record
         QString elabel;          // Experiment (run) label
         int     DB_id;           // Edit database ID number
         int     exp_id;          // Experiment database ID number
         int     auc_id;          // AUC database ID number
         int     tripknt;         // Count of triples per run
         int     tripndx;         // Index of triple in run
         int     editknt;         // Count of edits per triple
         int     editndx;         // Index of edit in triple
         bool    isEdit;          // Flag:  is this edit? False->raw
         bool    isLatest;        // Flag:  are edits latest ones
      };

      QMap< QString, DataDesc > datamap;  // Map of labels,data-desc-objs

      DataDesc       ddesc;       // Current selected data's description object
      QStringList    dlabels;     // Keys for datamap

      QPushButton*   pb_invest;   // Investigator button

      QLineEdit*     le_invest;   // Investigator text entry
      QLineEdit*     le_dfilter;  // Data search filter text entry

      QTextEdit*     te_notes;    // Notes text entry
      QTreeWidget*   tw_data;     // Data list widget

      bool           latest;      // Current use-lastest-edit flag
      bool           sel_run;     // Run-is-selected flag

      QString        dsearch;     // Current data search string
      QString        dinvtext;    // Current data investigator string
      QString        runID_sel;   // Currently selected run

      US_Disk_DB_Controls* disk_controls; //!< Radiobuttons for disk/db choice

      QVector< US_DataIO::RawData    >& rawData;
      QVector< US_DataIO::EditedData >& editedData;
      QStringList&                      triples;
      QString&                          description;
      QString                           etype_filt;
      QMap<QString, QString>            protocol_details;

   private slots:
      bool load_edit      ( void );
      void describe       ( void );
      void scan_dbase_edit( void );
      void scan_local_edit( void );
      void scan_dbase_runs( void );
      void scan_local_runs( void );
      void pare_to_latest ( void );
      void pare_latest_mwl( void );
      void show_data_info ( QPoint );
      void list_data      ( void );
      void list_data_auto      ( void );
      void get_person     ( void );
      void update_disk_db(  bool );
      void update_person  ( int  );
      void search         ( const QString& );
      void cancelled      ( void );
      void selected       ( void );
      void accepted       ( void );
      void experiment_info( QString&, QString&, QString& );
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
