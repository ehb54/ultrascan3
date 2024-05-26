//! \file us_load_auc.h
#ifndef US_LOAD_AUC_H
#define US_LOAD_AUC_H

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_dataIO.h"
#include "us_db2.h"

//! \brief A dialog class to select and load raw data
class US_GUI_EXTERN US_LoadAUC : public US_WidgetsDialog
{
   Q_OBJECT

   public:

      //! \brief Main contructor for dialog to select raw
      //!        data and load it
      //! \param local  Flag: is data source local disk?
      //! \param rData  A reference for a returned vector of RawData objects
      //! \param trips  A reference for a returned list of triples
      //! \param wdir   A reference for the returned data working directory
      US_LoadAUC( bool, QVector< US_DataIO::RawData >&, QStringList&,
                  QString& );

      US_LoadAUC( bool, QString, QVector< US_DataIO::RawData >&, QStringList&,
                  QString& );

      bool us_auto_mode; 

   signals:
      //! \brief A signal to tell the parent when the disk/db selection
      //!        has changed.
      //! \param DB   True if DB is selected
      void changed( bool DB );

      //! \brief A signal to provide the parent with a progress message
      //!        documenting currently loading triple
      //! \param message Message with currently loading triple
      void progress( const QString message );

   private:
      QVector< US_DataIO::RawData >&   rawList;    // Raw data vector
      QStringList&                     triples;    // Triples list
      QString&                         workingDir; // Base working directory

      class DataDesc   // Description of each data set in the list presented
      {
         public:
            QString label;           // Item label string
            QString runID;           // Run identifier string
            QString tripID;          // Triple identifier string
            QString filename;        // File name
            QString rawGUID;         // Raw data global identifier string
            QString date;            // Date/time last updated
            QString dcheck;          // Data checksum+size
            int     DB_id;           // Database ID number
            int     tripknt;         // Count of triples per run
            int     tripndx;         // Index of triple in run
      };

      QMap< QString, DataDesc >         datamap;  // Map: labels,data-desc-objs

      QStringList                       dlabels;  // Keys for datamap

      US_Disk_DB_Controls*              dkdb_cntrls;

      US_Help      showHelp;

      int          personID;
      bool         sel_run;
      QString      runID_sel;
      QTreeWidget* tree;
      QPushButton* pb_invest;
      QLineEdit*   le_invest;
      QLineEdit*   le_search;
      QTextEdit*   te_notes;

      void populate_tree      ( void );       
      void show_data_info     ( QPoint );
      int  scan_db            ( void );
      int  scan_disk          ( void );
      int  scan_run_db        ( void );
      int  scan_run_disk      ( void );
      void create_descs       ( QStringList&, QStringList&, int );
      void load_db            ( QList< DataDesc >& );
      void load_disk          ( QList< DataDesc >& );

   private slots:
      void    load               ( void );
      void    fill_in            ( void );
      void    expand             ( void );
      void    collapse           ( void );
      void    sel_investigator   ( void );
      void    assign_investigator( int  );
      void    limit_data         ( const QString& );
      void    update_disk_db     ( bool );
      void    help               ( void )
      { showHelp.show_help( "load_auc.html" ); };

   protected:
      //! \brief Event filter to flag whether a mouse click is
      //!        with the right mouse button;
      //! \param obj   Pointer to object on which event occurred.
      //! \param e     Pointer to event to filter.
      //! \return      Flag if event has been completely handled.
      bool eventFilter( QObject*, QEvent* );
};
#endif
