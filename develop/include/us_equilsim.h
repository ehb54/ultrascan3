#ifndef US_EQUILSIM_H
#define US_EQUILSIM_H

#include <qprinter.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qpixmap.h>
#include <qfiledialog.h>
#include <qtextstream.h>
#include <qfiledialog.h>
#include <qdatetime.h>

#include <qwt_counter.h>
#include <qwt_symbol.h>

#include <vector>

#include "us_util.h"
#include "us_2dplot.h"
#include "us.h"
#include "us_selectmodel.h"
#include "us_selectmodel10.h"
#include "us_selectmodel13.h"
#include "us_selectmodel3.h"
#include "us_math.h"
#include "us_db_tbl_buffer.h"
#include "us_db_tbl_vbar.h"

struct xlaScan
{
#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif

  vector <float> radius, absorbance, sigma;

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif

  QString mode;
   float temperature, omega;
   unsigned int speed, lambda, averages, time;
};


class US_EXTERN US_EquilSim : public QFrame
{
   Q_OBJECT
   
   public:
      US_EquilSim(QWidget *p=0, const char *name="us_equilsim");
      ~US_EquilSim();

      QString fileName;
      unsigned int components;
      int model;
      
#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif

    vector <struct xlaScan> scans;
      vector <float> mw, concentration, vbar20, extinction;
      vector <float> concs, od, bin, frequency;
      vector <unsigned int> speeds;

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif

      unsigned int species, current_species, conc_steps, speed_start;
      unsigned int speed_stop, speed_steps, datapoints, bin_number;
      bool assoc1, assoc2, assoc3, export_format;  // true for ultrascan
      bool model3_vbar_flag; //true for common vbar
      bool plot_widget, histogram_widget, export_flag;
      float stoich1, stoich2, mw_upperLimit, mw_lowerLimit, model3_vbar;
      float equil1, equil2, equil3, baseline, conc_increment, max_od;
      float pathlength, meniscus, delta_r, noise, nonlin_noise, lambda;
      float density, temperature, column, bottom;
      QLabel *lbl_model;
      QLabel *lbl_blank1,*lbl_blank2,*lbl_blank3,*lbl_blank4;
      QLabel *lbl_species;
      QLabel *lbl_mw;
      QLabel *lbl_concentration;
      QLabel *lbl_baseline;
      QLabel *lbl_od;
      QLabel *lbl_od_value;
      QLabel *lbl_conc_increment;
      QLabel *lbl_conc_steps;
      QLabel *lbl_pathlength;
      QLabel *lbl_speed_start;
      QLabel *lbl_speed_stop;
      QLabel *lbl_speed_steps;
      QLabel *lbl_temperature;
      QLabel *lbl_column;
      QLabel *lbl_bottom;
      QLabel *lbl_meniscus;
      QLabel *lbl_meniscus_value;
      QLabel *lbl_max_od;
      QLabel *lbl_datapoints;
      QLabel *lbl_delta_r;
      QLabel *lbl_delta_r_value;
      QLabel *lbl_lambda;
      QLabel *lbl_noise;
      QLabel *lbl_nonlin_noise;
      QLabel *lbl_export;

      QwtCounter *cnt_species;
      QwtCounter *cnt_conc_steps;
      QwtCounter *cnt_speed_start;
      QwtCounter *cnt_speed_stop;
      QwtCounter *cnt_speed_steps;
      QwtCounter *cnt_column;
      QwtCounter *cnt_bottom;
      QwtCounter *cnt_datapoints;
      QwtCounter *cnt_noise;
      QwtCounter *cnt_nonlin_noise;

      QPushButton *pb_model;
      QPushButton *pb_equil1;
      QPushButton *pb_equil2;
      QPushButton *pb_equil3;
      QPushButton *pb_vbar;
      QPushButton *pb_extinction;
      QPushButton *pb_density;
      QPushButton *pb_help;
      QPushButton *pb_plot;
      QPushButton *pb_histogram;
      QPushButton *pb_export;
      QPushButton *pb_close;
      
      QLineEdit *le_mw;
      QLineEdit *le_concentration;
      QLineEdit *le_equil1;
      QLineEdit *le_equil2;
      QLineEdit *le_equil3;
      QLineEdit *le_vbar;
      QLineEdit *le_baseline;
      QLineEdit *le_extinction;
      QLineEdit *le_conc_increment;
      QLineEdit *le_pathlength;
      QLineEdit *le_temperature;
      QLineEdit *le_density;
      QLineEdit *le_max_od;
      QLineEdit *le_lambda;

      QCheckBox *cb_ultrascan;
      QCheckBox *cb_xla;

      US_Config *USglobal;
      US_SelectModel *SelectModel;
      US_2dPlot *data_plot, *histogram_plot;
      US_Buffer_DB *buffer_dlg;

   private slots:

// counters:

      void select_species(double);
      void select_conc_steps(double);
      void select_speed_start(double);
      void select_speed_stop(double);
      void select_speed_steps(double);
      void select_column(double);
      void select_bottom(double);
      void select_datapoints(double);
      void select_noise(double);
      void select_nonlin_noise(double);

// pushbuttons:

      void select_model();
      void change_equil1();
      void change_equil2();
      void change_equil3();
      void change_vbar();
      void change_extinction();
      void change_density();
      void help();
      void plot();
      void histogram();
      void export_data();
      void quit();

// lineedits:

      void update_equil1(const QString &);
      void update_equil2(const QString &);
      void update_equil3(const QString &);
      void update_mw(const QString &);
      void update_concentration(const QString &);
      void update_vbar(const QString &);
      void update_baseline(const QString &);
      void update_extinction(const QString &);

      void update_conc_increment(const QString &);
      void update_pathlength(const QString &);
      void update_temperature(const QString &);
      void update_density(const QString &);
      void update_max_od(const QString &);
      
      void update_lambda(const QString &);

// checkboxes:

      void select_ultrascan();
      void select_xla();
      
// other:

      void update_vbar(float, float);
      void update_e280(float);
      void update_density_label(float, float);
      float calc_density_tb(float);
      void calc_conc(float ***, float *);
      void calc_function(double **, double **, unsigned int, unsigned int, unsigned int, float ***);
      
   protected slots:
      void setup_GUI();
      void closeEvent(QCloseEvent *);
      void plot_closeEvent();
      void histogram_closeEvent();
      float integral_root(float, float, float);
      float find_root(float, float, float);
      double calc_taylor(double, unsigned int);

};

#endif

