#ifndef US_ASTFEM_SIM_H
#define US_ASTFEM_SIM_H

#include <iostream>
#include <vector>

#include <qwidget.h>
#include <qframe.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <string.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qprogressbar.h>
#include <qlayout.h>

#include <qlcdnumber.h>
#include <qwt_plot.h>
#include <qwt_counter.h>

#include "us_util.h"
#include "us_femglobal.h"
#include "us_astfem_rsa.h"
#include "us_model_editor.h"
#include "us_simulationparameters.h"
#include "us_clipdata.h"
#include "us_modelselection.h"
#include "us_pixmap.h"

class US_EXTERN US_Astfem_Sim : public QFrame
{
   Q_OBJECT

   public:
      US_Astfem_Sim(QWidget *p = 0, const char *name = 0);
      ~US_Astfem_Sim();
      US_Config *USglobal;
      bool stopFlag;
      bool movieFlag;
      bool time_correctionFlag;
      float total_conc;
      
   private:
      US_Astfem_RSA *astfem_rsa;
      struct ModelSystem system;
      struct SimulationParameters simparams;
      int imagecounter;
      US_Pixmap *pm;
      bool save_movie;
      QString imagedir;

#ifdef WIN32
     #pragma warning ( disable: 4251 )
#endif

      vector <struct mfem_data> astfem_data;
      
#ifdef WIN32
     #pragma warning ( default: 4251 )
#endif
      
      QwtPlot *movie_plot, *scan_plot;
      QPushButton *pb_load_system;
      QPushButton *pb_save_system;
      QPushButton *pb_new_model;
      QPushButton *pb_load_model;
      QPushButton *pb_change_model;
      QPushButton *pb_simulation_parameters;
      QPushButton *pb_start_simulation;
      QPushButton *pb_stop_simulation;
      QPushButton *pb_dcdt_window;
      QPushButton *pb_save_scans;
      QPushButton *pb_help;
      QPushButton *pb_close;
      QLabel *lbl_speed;
      QLabel *lbl_time;
      QLabel *lbl_component;
      QLabel *lbl_progress;
      QLCDNumber *lcd_speed;
      QLCDNumber *lcd_component;
      QLCDNumber *lcd_time;
      QCheckBox *cb_movie;
      QCheckBox *cb_savemovie;
      QCheckBox *cb_time_correction;
      QProgressBar *progress;

   protected slots:
      void closeEvent(QCloseEvent *e);
      
   private slots:
      void setup_GUI();
      void new_model();
      void assign_model();
      void load_model();
      void load_model(const QString &);
      void load_system();
      void load_system(const QString &);
      void save_system();
      void save_system(const QString &);
      void change_model();
      void simulation_parameters();
      void start_simulation();
      void stop_simulation();
      void dcdt_window();
      void save_scans();
      void help();
      void update_movie_plot(vector <double> *, double *);
      void update_movieFlag();
      void update_savemovie();
      void update_time_correctionFlag();
      void update_time(float);
      void update_progress(int);
      void update_speed(unsigned int);
      void printError(const int &);
      void save_xla(const QString &);
      void save_ultrascan(const QString &);
};

#endif
