#ifndef US_GRIDCONTROL_H
#define US_GRIDCONTROL_H

#include "us.h"
#include "us_util.h"
#include "us_dtactrl.h"
#include "us_mfem.h"
#include "us_ga_gridcontrol.h"
#include "us_sa2d_gridcontrol.h"
#include "us_sa2dbase.h"
#include "us_cluster.h"
#include "us_gridcontrol_t.h"
#include <qapp.h>

#include <qprogbar.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <qlineedit.h>

class US_EXTERN US_GridControl : public Data_Control_W
{
   Q_OBJECT
   
   public:
   
      US_GridControl(QWidget *p = 0, const char *name = 0);
      US_GridControl(const QString &, const QString &, QWidget *p = 0, const char *name = 0);
      ~US_GridControl();
      US_Cluster *cluster;
      vector <struct mfem_data> experiment;
      struct control_parameters Control_Params;
      vector <struct gene> solutions;
      QListBox *lb_experiments;
      QPushButton *pb_ga_control;
      QPushButton *pb_sa2d_control;
      struct ga_data GA_Params;
      struct sa2d_data SA2D_Params;
      QString analysis_type, email;
      bool data_loaded;
      bool analysis_defined;
      QRadioButton *bt_tinoise;
      QRadioButton *bt_rinoise;
      QLabel *lbl_email;
      QLineEdit *le_email;
      bool fit_tinoise, fit_rinoise;
      float meniscus_range;
      bool fit_meniscus, use_iterative;
      unsigned int meniscus_gridpoints, max_iterations;
      QDateTime timestamp;
      QString timestamp_string;
      float regularization;
      QString gcfile;
      QString gridopt;
      struct US_DatabaseLogin db_login;
      
   private slots:
   
      void setup_GUI();
      void help();
      void add_experiment();
      void delete_experiment(int);
      void write_experiment();
      void write_solutes(const QString &);
      void update_email(const QString &);
      void write_ga_experiment();
      void write_sa2d_experiment();
      void ga_control();
      void sa2d_control();
      void update_tinoise();
      void update_rinoise();
   
   protected:
   
      void closeEvent(QCloseEvent *);
};

#endif

