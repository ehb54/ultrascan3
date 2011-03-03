#ifndef US_REPORTER_H
#define US_REPORTER_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets.h"
#include "us_db2.h"
#include "us_help.h"
#include "us_settings.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

class US_EXTERN US_Reporter : public US_Widgets
{
   Q_OBJECT

   public:
      US_Reporter();

      class DataDesc
      {
         public:
         int       linen;             // line number
         int       level;             // tree level (0,...)
         int       checkState;        // check state flag (0,1,2)
         int       children;          // number of children
         QString   label;             // entry label
         QString   type;              // type of entry (Run, Data, ...)
         QString   filename;          // file name
         QString   filepath;          // full file path name
         QString   runid;             // run ID
         QString   triple;            // triple (e.g., "2/A/280")
         QString   analysis;          // analysis
         QString   lastmodDate;       // file last modification date/time
      };

   private:

      QTreeWidget*        tw_recs;    // tree widget
      QTreeWidgetItem*    tw_item;    // current tree widget item

      DataDesc            cdesc;      // current record description
      QVector< DataDesc > adescs;     // all descriptions

      QMap< QString, QString > appmap; // applications name,label map
      QMap< QString, QString > extmap; // applications name,label map
      QMap< QString, QString > rptmap; // reports name,label map

      QList< int >        se_rptrows;  // List of selected report rows

      QStringList         sl_runids;   // List of existing report runIDs
      QStringList         se_runids;   // List of selected runIDs
      QStringList         se_reports;  // List of selected reports

      US_Help       showHelp;

      US_DB2*       db;

      QComboBox*    cb_runids;

      QPushButton*  pb_view;
      QPushButton*  pb_save;
      QPushButton*  pb_help;
      QPushButton*  pb_close;

      int           ntrows;
      int           ntcols;
      int           nsrpts;
      int           nsruns;
      int           nstrips;
      int           nshtmls;
      int           nsplots;
      int           dbg_level;

      bool          rbtn_click;
      bool          change_tree;

      QString       run_name;
      QString       investig;
      QString       pagedir;
      QString       pagepath;
      QString       hsclogo;
      QString       becklogo;
      QString       us3logo;


   private slots:

      void sample_tree   ( void );
      void clickedItem   ( QTreeWidgetItem* );
      void changedItem   ( QTreeWidgetItem*, int );
      void row_context   ( QTreeWidgetItem* );
      void build_runids  ( void );
      void new_runid     ( int  );
      void build_descs   ( QString&, int& );
      void build_map     ( QString, QMap< QString, QString >& );
      void build_tree    ( void );
      void count_children( DataDesc*, int&, int& );
      void state_children( DataDesc*, int& );
      void state_parents ( DataDesc*, int& );
      void mark_checked  ( void );
      void view          ( void );
      void save          ( void );
      bool write_report  ( void );
      bool count_reports ( void );
      void item_view     ( void );
      void item_show     ( void );
      void item_data     ( void );
      void load_profile  ( void );
      void save_profile  ( void );

      void help          ( void )
      { showHelp.show_help( "reporter.html" ); };

   protected:
      bool eventFilter( QObject*, QEvent* );
};
#endif
