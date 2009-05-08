#ifndef US_COFDISTRO_H
#define US_COFDISTRO_H

#include "us_dtactrl.h"
#include "us_extern.h"
#include "us_resplot.h"

#include <qlistbox.h>
#include <qcheckbox.h>
#include <qwt_symbol.h>
#include <cerrno>

class  US_EXTERN US_CofDistro_W : public Data_Control_W
{
   Q_OBJECT
   
   public:
   
      US_CofDistro_W(QWidget *p = 0, const char *name = 0);
      US_CofDistro_W(double, double, QString, unsigned int, unsigned int, QWidget *p=0, const char *name=0);
      ~US_CofDistro_W();
      
      
   private:
   
      int plot2;
      QLabel *lbl_s_max;
      QLabel *lbl_s_min;
      QLabel *lbl_ff0_min;
      QLabel *lbl_ff0_max;
      QLabel *lbl_resolution;
      QLabel *lbl_iteration;
      QLabel *lbl_shape_distro;
      QPushButton *pb_fit;
      QPushButton *pb_abort;
      QPushButton *pb_distro;
      QCheckBox *cb_par_c;
      QListBox *lb_shape_distro;
      QwtCounter *cnt_s_max;
      QwtCounter *cnt_s_min;
      QwtCounter *cnt_ff0_min;
      QwtCounter *cnt_ff0_max;
      QwtCounter *cnt_resolution;
      QwtCounter *cnt_par_c;
      QLabel *lbl_info1, *lbl_info2;
      US_ResidualPlot *resplot;
      US_Pixmap *pm;
      float rmsd;
      bool fit_status, fit_par_c;
      double s_min, s_max, par_a, par_b, par_c, ff0_min, ff0_max;
      unsigned int resolution, shape_distro;

#ifdef WIN32
  #pragma warning( disable: 4251 )
#endif

      vector <double> s_distribution;
      vector <double> D_distribution;
      vector <double> frequency;
      vector <double> s20w;
      vector <double> D20w;
      vector <double> mw;
      vector <double> ff0;
#ifdef WIN32
  #pragma warning( default: 4251 )
#endif

      struct mfem_data residuals;
      unsigned int iteration;
      
   private slots:

      void setup_GUI();
      float linesearch();
      void update_resolution(double);
      void update_s_max(double);
      void update_s_min(double);
      void update_ff0_min(double);
      void update_ff0_max(double);
      void update_par_c(double);
      void update_shape_distro(int);
      void set_par_c();
      void setupGUI();
      void enableButtons();
      void update_distribution();
      void clear_data(struct mfem_data *);
      void fit();
      float calc_residuals();
      float calc_testParameter(float);
      void second_plot(int);
      void write_cofdistro();
      void write_res();
      void calc_distros();
      void set_abc();

// re-implemented Functions:

      void view();
      void help();
      void save();
      void second_plot();
};

#endif

