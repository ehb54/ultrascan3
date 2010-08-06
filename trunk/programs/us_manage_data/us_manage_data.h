#ifndef US_MANAGE_DATA_H
#define US_MANAGE_DATA_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets.h"
#include "us_db2.h"
#include "us_model.h"
#include "us_help.h"

class US_EXTERN US_ManageData : public US_Widgets
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
         int       recType;           // record type (0-4)=None/Raw/Edit/Model/Noise
         int       parentID;          // parent's DB Identifier
         int       recState;          // record state flag
         QString   subType;           // sub-type (e.g., TI,RI for noises)
         QString   dataGUID;          // this record data Global Identifier
         QString   parentGUID;        // parent's GUID
         QString   filename;          // file name if on local disk
         QString   contents;          // XML contents or head of encoded BLOB
         QString   label;             // record identifying label
         QString   description;       // record description string
         QString   lastmodDate;       // last modification date/time
      };

   private:

      QTableWidget*       tw_recs;    // table widget
      QTableWidgetItem*   tw_item;    // current table widget item

      DataDesc            cdesc;      // current record description
      QVector< DataDesc > ddescs;     // DB descriptions
      QVector< DataDesc > ldescs;     // local-disk descriptions
      QVector< DataDesc > adescs;     // all (merged) descriptions

      QProgressBar* progress;

      US_Help       showHelp;

      US_DB2*       db;
 
      QLabel*       lb_status;

      QTextEdit*    te_status;

      QLineEdit*    le_invtor;

      QPushButton*  pb_invtor;
      QPushButton*  pb_browse;
      QPushButton*  pb_detail;
      QPushButton*  pb_hsedit;
      QPushButton*  pb_hsmodl;
      QPushButton*  pb_hsnois;
      QPushButton*  pb_helptb;
      QPushButton*  pb_reset;
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

      bool          details;
      bool          rbtn_click;

      QString       run_name;
      QString       cell;
      QString       investig;

      QPoint        cur_pos;

   private slots:

      void toggle_edits (  void );
      void toggle_models(  void );
      void toggle_noises(  void );
      void toggle_hidden( QString, bool );
      void table_help(     void );
      void browse_data  (  void );
      void detail_data  (  void );
      void build_table  (  void );
      void reset(       void );
      void chg_investigator(    void );
      void find_investigator(   QString& );
      void sel_investigator(    void );
      void assign_investigator( int, const QString&, const QString& );
      void clickedItem(      QTableWidgetItem* );
      void row_context_menu( QTableWidgetItem* );
      void item_upload(      void );
      void item_download(    void );
      void item_remove_db(   void );
      void item_remove_loc(  void );
      void item_remove_all(  void );
      void item_details(     void );
      void item_show_child(  void );
      void item_hide_child(  void );
      void browse_dbase(     bool );
      void browse_local(     bool );
      void merge_dblocal(    void );
      void sort_descs( QVector< DataDesc >& descs );
      QString sort_string( DataDesc, int );
      QString model_type(   int, int, int );
      QString model_type(   US_Model      );
      QString model_type(   QString       );
      QString record_state( int      );
      bool    review_descs( QStringList&, QVector< DataDesc >& );
      int         index_substring(  QString, int, QStringList& );
      QStringList filter_substring( QString, int, QStringList& );
      QStringList list_orphans    ( QStringList&, QStringList& );
      int         record_state(     DataDesc, int );


      void help     ( void )
      { showHelp.show_help( "manage_data.html" ); };

   protected:
      bool eventFilter( QObject*, QEvent* );
};
#endif
