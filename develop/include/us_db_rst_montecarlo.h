#ifndef US_DB_RST_MONTECARLO_H
#define US_DB_RST_mONTECARLO_H

#include "us_db.h"
#include "us_montecarloreport.h"
#include "us_db_runrequest.h"
#include "us_db_widgets.h"

#include <qprogressdialog.h>

class US_EXTERN US_DB_RST_Montecarlo : public US_DB_Widgets
{
   Q_OBJECT
   
   public:
   
      US_DB_RST_Montecarlo( QWidget* p = 0, const char* name = 0 );
      ~US_DB_RST_Montecarlo(){};

   private:
      QPushButton* pb_investigator;
      QPushButton* pb_runrequest;
      QPushButton* pb_hd;
      QPushButton* pb_db;
      QPushButton* pb_retrieve;
      QPushButton* pb_save;
      QPushButton* pb_display;
      QPushButton* pb_help;
      QPushButton* pb_close;
      QPushButton* pb_delete;
      QPushButton* pb_reset;

      QListBox*    lb_result;

      QLabel*      lbl_blank;
      QLabel*      lbl_instr;
      QLabel*      lbl_item;
      QLabel*      lbl_investigator;
      QLabel*      lbl_runrequest;

      QString  fileName;
      QString  parameterFile;
      QString  projectName;
      
      QString* item_projectName;

      int      investigatorID;
      int      runrequestID;
      int      parameters;
      int      montecarloID;
      int      projectID;
      int*     item_projectID;

      bool     from_HD;
      bool     from_DB;
      bool     query_flag;
      
   private:
      void setup_GUI       ( void );
      void clearTmpDir     ( void );
      bool create_tar      ( const QString&, const QString&, QStringList& );
      void move_file       ( const QString&, const QString& );
      void clear           ( void );
      bool insertData      ( void );

   private slots:
      
      void load_HD         ( void );
      void load_DB         ( void );
      void retrieve        ( void );
      void sel_investigator( void );
      void sel_runrequest  ( void );
      void select_result   ( int  );
      void save_db         ( void );
      void display         ( void );
      void check_permission( void );
      void reset           ( void );
      void help            ( void );
      void quit            ( void );

   public slots:

      void closeEvent             ( QCloseEvent* );
      void update_investigator_lbl( QString, int );
      void update_runrequest_lbl  ( int );
      void delete_db              ( bool );
};

#endif
