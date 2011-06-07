#ifndef US_MANAGE_DATA_H
#define US_MANAGE_DATA_H

#include <QtGui>

#include "us_extern.h"
#include "us_data_model.h"
#include "us_data_process.h"
#include "us_data_tree.h"
#include "us_sync_exper.h"
#include "us_widgets.h"
#include "us_db2.h"
#include "us_model.h"
#include "us_buffer.h"
#include "us_analyte.h"
#include "us_help.h"
#include "us_dataIO2.h"
#include "us_settings.h"

class US_ManageData : public US_Widgets
{
   Q_OBJECT

   public:
      US_ManageData();

      enum State { NOSTAT=0,  REC_DB=1,  REC_LO=2, PAR_DB=4, PAR_LO=8,
                   HV_DET=16, IS_CON=32, ALL_OK=64 };

      class DataDesc
      {
         public:
         int       recordID;          // record DB Identifier
         int       recType;           // record type (1-4)=Raw/Edit/Model/Noise
         int       parentID;          // parent's DB Identifier
         int       recState;          // record state flag
         QString   subType;           // sub-type (e.g., TI,RI for noises)
         QString   dataGUID;          // this record data Global Identifier
         QString   parentGUID;        // parent's GUID
         QString   filename;          // file name if on local disk
         QString   contents;          // md5sum() and length of data
         QString   label;             // record identifying label
         QString   description;       // record description string
         QString   lastmodDate;       // last modification date/time
      };

      US_DB2*       db;

   private:

      QTreeWidget*        tw_recs;    // tree widget
      QTreeWidgetItem*    tw_item;    // current tree widget item

      DataDesc            cdesc;      // current record description

      US_DataModel*       da_model;   // underlying data handler
      US_DataProcess*     da_process; // data processing handler
      US_DataTree*        da_tree;    // data tree display handler
      US_SyncExperiment*  syncExper;  // experiment synchronizer

      QProgressBar* progress;

      US_Help       showHelp;

      US_Buffer     buffer;
      US_Analyte    analyte;
 
      QLabel*       lb_status;

      QTextEdit*    te_status;

      QLineEdit*    le_invtor;

      QPushButton*  pb_invtor;
      QPushButton*  pb_reset;
      QPushButton*  pb_scanda;
      QPushButton*  pb_hsedit;
      QPushButton*  pb_hsmodl;
      QPushButton*  pb_hsnois;
      QPushButton*  pb_helpdt;
      QPushButton*  pb_help;
      QPushButton*  pb_close;

      int           personID;
      int           ntrows;
      int           ntcols;
      int           ncrecs;
      int           ncraws;
      int           ncedts;
      int           ncmods;
      int           ncnois;
      int           ndrecs;
      int           ndraws;
      int           ndedts;
      int           ndmods;
      int           ndnois;
      int           nlrecs;
      int           nlraws;
      int           nledts;
      int           nlmods;
      int           nlnois;
      int           kdmy;
      int           dbg_level;

      bool          rbtn_click;

      QString       run_name;
      QString       investig;

   private slots:

      void toggle_edits (  void );
      void toggle_models(  void );
      void toggle_noises(  void );
      void dtree_help(     void );
      void scan_data    (  void );
      void reset(       void );
      void chg_investigator(    void );
      void find_investigator(   QString& );
      void sel_investigator(    void );
      void assign_investigator( int, const QString&, const QString& );
      void clickedItem(      QTreeWidgetItem* );
      QString action_text(   QString, QString );
      void    action_result( int,     QString );
      void reset_hsbuttons( bool, bool, bool, bool );
      void reportDataStatus( void    );

      void help     ( void )
      { showHelp.show_help( "manage_data.html" ); };

      //quint32 crc32( quint32, const unsigned char*, int );
   protected:
      bool eventFilter( QObject*, QEvent* );
};
#endif
