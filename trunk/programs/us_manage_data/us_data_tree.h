#ifndef US_DATA_TREE_H
#define US_DATA_TREE_H

#include <QtGui>

#include "us_extern.h"
#include "us_sync_exper.h"
#include "us_data_model.h"
#include "us_data_process.h"
#include "us_db2.h"
#include "us_model.h"
#include "us_buffer.h"
#include "us_analyte.h"
#include "us_help.h"
#include "us_dataIO2.h"

class US_DataTree : public QObject
{
   Q_OBJECT

   public:
      US_DataTree( US_DataModel*, QTreeWidget*, QWidget* = 0 );

      void toggle_expand(    QString, bool );
      void build_dtree(      void );

   public slots:
      void dtree_help(       void );
      void item_upload(      void );
      void item_download(    void );
      void item_remove_db(   void );
      void item_remove_loc(  void );
      void item_remove_all(  void );
      void items_remove(     void );
      void item_details(     void );
      void row_context_menu( QTreeWidgetItem* );

   private:

      US_DataModel*    da_model;     // data model object
      QTreeWidget*     tw_recs;      // tree widget
      QPushButton*     pb_hsedit;    // pointer to edits hide/show button
      QPushButton*     pb_hsmodl;    // pointer to models hide/show button
      QPushButton*     pb_hsnois;    // pointer to noises hide/show button
      QWidget*         parentw;      // pointer to parent widget

      US_DataProcess*  da_process;   // data processor object

      QTreeWidgetItem* tw_item;      // current tree widget item

      US_DataModel::DataDesc  cdesc; // current record description

      QList< QTreeWidgetItem* > selitems;  // all selected items

      QLabel*       lb_status;

      QTextEdit*    te_status;

      QLineEdit*    le_invtor;

      QPushButton*  pb_browse;
      QPushButton*  pb_detail;
      QPushButton*  pb_reset;
      QPushButton*  pb_help;
      QPushButton*  pb_close;

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

   private slots:
      void record_type(      int,     QString& );
      QString action_text(   QString, QString  );
      void    action_result( int,     QString  );
      QString record_state(  int               );
};
#endif
