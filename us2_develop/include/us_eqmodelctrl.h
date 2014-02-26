#ifndef US_EQMODELCTRL_H
#define US_EQMODELCTRL_H

#include <qlabel.h>
#include <vector>
#include <algorithm>
#include <qframe.h>
#include <qinputdialog.h>
#include <qpushbutton.h>

#include "us.h"
#include "us_util.h"
#include "us_math.h"
#include "us_selectmodel.h"
#include "us_selectmodel3.h"
#include "us_selectmodel10.h"
#include "us_selectmodel13.h"
#include "us_showdetails.h"
#include "us_infopanel.h"
#include "us_extinction.h"
#include "us_db_tbl_buffer.h"
#include "us_db_tbl_vbar.h"
#include "us_rundetails.h"

extern vector <QString> modelString;


struct EquilRun_FitProfile
{
//
// variable:    the center of the fitting vectors initialization
// variable_index: serves as a linked list to index the position in the Jacobian matrix
// variable_range: The range between which the fitting vectors of dud will be inititlized
// variable_fit: boolean to control wether a parameter is fitted or fixed
// variable_bound: boolean to control wether the variable is constrained between bounds or allowed to float outside
//
   vector <float> mw;
   vector <unsigned int> mw_index;
   vector <float> mw_range;
   vector <bool> mw_fit;
   vector <bool> mw_bound;
   vector <float> vbar20;
   vector <unsigned int> vbar20_index;
   vector <float> vbar20_range;
   vector <bool> vbar20_fit;
   vector <bool> vbar20_bound;
   vector <float> virial;
   vector <unsigned int> virial_index;
   vector <float> virial_range;
   vector <bool> virial_fit;
   vector <bool> virial_bound;
   unsigned int components;
   unsigned int association_constants;
   unsigned long runs;
   float runs_percent;
   float runs_expected;
   float runs_variance;
   float eq[4];
   unsigned int eq_index[4];
   float eq_range[4];
   bool eq_fit[4];
   bool eq_bound[4];
   float stoich1;
   float stoich2;
   float stoich3;
   float stoich4;
};

struct EquilScan_FitProfile
{
   bool FitScanFlag;
   bool autoExcluded;
   unsigned int points;
   vector <float> x;
   vector <float> y;
   unsigned int pos;      // runs statistics
   unsigned int neg;      // runs statistics
   unsigned int runs;   // the number of runs in the residuals
   unsigned int start_index;
   unsigned int stop_index;
   bool limits_modified;
   vector <float> amplitude;               // one for each component
   vector <unsigned int> amplitude_index;   // one for each component
   vector <float> amplitude_range;         // one for each component
   vector <bool> amplitude_fit;            // one for each component
   vector <bool> amplitude_bound;         // one for each component
   vector <float> extinction;               // one for each component
   float pathlength;
   float baseline;
   float meniscus;
   unsigned int baseline_index;
   float baseline_range;
   bool baseline_fit;
   bool baseline_bound;
   float density;
   unsigned int wavelength;
   unsigned int rpm;
   float temperature;
   QString run_id;
   QString description;
   unsigned int cell;
   unsigned int lambda;
   unsigned int channel;
   int rotor;
   unsigned int centerpiece;
   vector <float> integral;
};

class US_EqModelControl : public QFrame
{
   Q_OBJECT
   
   public:
      US_EqModelControl(vector <struct EquilScan_FitProfile> *, struct EquilRun_FitProfile *, vector <struct runinfo> *,
      int, bool *temp_model_widget, uint *, QWidget *p=0, const char *name = 0);
      
      ~US_EqModelControl();
      
      vector <struct EquilScan_FitProfile> *scanfit_vector;
      struct EquilRun_FitProfile *runfit;
      vector <struct runinfo> *run_information;
      int xpos, ypos, border, buttonh, model;
      struct XY_data extinction;
      US_Config *USglobal;
      QString projectName;
      uint current_component, *selected_scan;
      uint scan_range_start, scan_range_stop, pathlength_range_start, pathlength_range_stop;
      uint ext_range_start, ext_range_stop;
      bool *model_widget;
      int eqconst1, eqconst2, eqconst3, eqconst4;   // toggle between association const, 
      US_FitParameterLegend *legend1;               // dissociation constant, and log of absorbance
      US_FitParameterLegend *legend2;
      US_FitParameterLegend *legend3;
      US_FitParameter *fp_mw;
      US_FitParameter *fp_vbar;
      US_FitParameter *fp_virial;
      US_FitParameter *fp_eqconst1;
      US_FitParameter *fp_eqconst2;
      US_FitParameter *fp_eqconst3;
      US_FitParameter *fp_eqconst4;
      US_FitParameter *fp_baseline;
      US_FitParameter *fp_amplitude;
      US_Vbar_DB *vbar_dlg;
      US_Buffer_DB *buffer_dlg;
      QLabel *lbl_banner1;
      QLabel *lbl_banner2;
      QLabel *lbl_component1;
      QLabel *lbl_component2;
      QLabel *lbl_virial;
      QLabel *lbl_description1;
      QLabel *lbl_description2;
      QLabel *lbl_run_id1;
      QLabel *lbl_run_id2;
      QLabel *lbl_temperature1;
      QLabel *lbl_temperature2;
      QLabel *lbl_speed1;
      QLabel *lbl_speed2;
      QLabel *lbl_density_apply1;
      QLabel *lbl_density_apply2;
      QLabel *lbl_pathlength_apply1;
      QLabel *lbl_pathlength_apply2;
      QLabel *lbl_extinction_apply1;
      QLabel *lbl_extinction_apply2;
      QLabel *lbl_wavelength1;
      QLabel *lbl_wavelength2;
      QLabel *lbl_pathlength;
      QLabel *lbl_sigma1;
      QLabel *lbl_sigma2;
      QLabel *lbl_baseline;
      QLabel *lbl_amplitude;
      QLabel *lbl_include;
      QLabel *lbl_active_scan;
      QLabel *lbl_mw;
      QLineEdit *le_scan_range_start;
      QLineEdit *le_scan_range_stop;
      QLineEdit *le_ext_range_start;
      QLineEdit *le_ext_range_stop;
      QLineEdit *le_pathlength_range_start;
      QLineEdit *le_pathlength_range_stop;
      QLineEdit *le_density;
      QLineEdit *le_pathlength;
      QLineEdit *le_extinction;
      QwtCounter *cnt_component1;
      QwtCounter *cnt_component2;
      QwtCounter *cnt_active_scan;
      QPushButton *pb_density_apply;
      QPushButton *pb_extinction_apply;
      QPushButton *pb_pathlength_apply;
      QPushButton *pb_vbar;
      QPushButton *pb_eqconst1;
      QPushButton *pb_eqconst2;
      QPushButton *pb_eqconst3;
      QPushButton *pb_eqconst4;
      QPushButton *pb_density;
      QPushButton *pb_extinction;
      QPushButton *pb_close;
      QPushButton *pb_help;
      QCheckBox *cb_include;

   public slots:
      void update_component1(double);
      void update_component2(double);
      void update_component_labels();
            
      void mw_float();
      void mw_float(bool);
      void mw_constrained();
      void get_mw_value(const QString &);
      void get_mw_range(const QString &);
      
      void vbar_float();
      void vbar_float(bool);
      void vbar_constrained();
      void get_vbar_value(const QString &);
      void get_vbar_range(const QString &);
      void read_vbar();
      void update_vbar_lbl(float, float);
      
      void virial_float();
      void virial_float(bool);
      void virial_constrained();
      void get_virial_value(const QString &);
      void get_virial_range(const QString &);

      void eqconst1_float();
      void eqconst1_float(bool);
      void eqconst1_constrained();
      void get_eqconst1_value(const QString &);
      void get_eqconst1_range(const QString &);

      void eqconst2_float();
      void eqconst2_float(bool);
      void eqconst2_constrained();
      void get_eqconst2_value(const QString &);
      void get_eqconst2_range(const QString &);

      void eqconst3_float();
      void eqconst3_float(bool);
      void eqconst3_constrained();
      void get_eqconst3_value(const QString &);
      void get_eqconst3_range(const QString &);

      void eqconst4_float();
      void eqconst4_float(bool);
      void eqconst4_constrained();
      void get_eqconst4_value(const QString &);
      void get_eqconst4_range(const QString &);

      void baseline_float();
      void baseline_float(bool);
      void baseline_constrained();
      void get_baseline_value(const QString &);
      void get_baseline_range(const QString &);

      void amplitude_float();
      void amplitude_float(bool);
      void amplitude_constrained();
      void get_amplitude_value(const QString &);
      void get_amplitude_range(const QString &);

      void setup_model();
      void apply_density_range();
      void apply_extinction_range();
      void apply_pathlength_range();
      void update_projectName(const QString &);
      void update_sigma(const QString &);
      void update_scan_range_start(const QString &);
      void update_scan_range_stop(const QString &);
      void update_ext_range_start(const QString &);
      void update_ext_range_stop(const QString &);
      void update_pathlength_range_start(const QString &);
      void update_pathlength_range_stop(const QString &);
      void update_scan_number(int);
      void update_scan_info(int);
      void update_eqconst1();
      void update_eqconst2();
      void update_eqconst3();
      void update_eqconst4();
      void update_density1();
      void update_density2(const QString &);
      void update_density_label(float, float);
      void update_extinction1();
      void update_extinction2(const QString &);
      void update_extinction3(double *, double *, unsigned int);
      void update_extinction4();
      void update_pathlength2(const QString &);
      void update_include();
      void change_include();
      float calc_density_tb(float, float);
      void quit();
      void help();

   protected slots:
      void setup_GUI();
      void closeEvent(QCloseEvent *);

   signals:
      void ControlWindowClosed();
};

#endif

