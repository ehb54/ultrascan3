#ifndef US_COFS_H
#define US_COFS_H

#include "us_dtactrl.h"
#include "us_extern.h"
//#include "us_mfem.h"
#include "us_resplot.h"
#include <qcheckbox.h>
#include <qwt_symbol.h>
#include <cerrno>

class  US_EXTERN US_CofS_W : public Data_Control_W
{
   Q_OBJECT
   
   public:
   
      US_CofS_W(QWidget *p = 0, const char *name = 0);
      US_CofS_W(double, double, QString, unsigned int, unsigned int, QWidget *p=0, const char *name=0);
      ~US_CofS_W();
      
      
   private:
   
      int plot2;
      QLabel *lbl_top;
      QLabel *lbl_bottom;
      QLabel *lbl_resolution;
      QLabel *lbl_ff0;
      QLabel *lbl_posBaseline;
      QLabel *lbl_negBaseline;
      QLabel *lbl_iteration;
      QPushButton *pb_fit;
      QPushButton *pb_abort;
      QCheckBox *cb_ff0;
      QCheckBox *cb_posBaseline;
      QCheckBox *cb_negBaseline;
      QwtCounter *cnt_top;
      QwtCounter *cnt_bottom;
      QwtCounter *cnt_resolution;
      QwtCounter *cnt_ff0;
      QLabel *lbl_info1, *lbl_info2;
      US_ResidualPlot *resplot;
      US_Pixmap *pm;
      float rmsd;
      bool fit_status, fit_ff0, fit_posBaseline, fit_negBaseline;
      double bottom, top;
      unsigned int resolution;

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif

      vector <double> s_distribution;
      vector <double> D_distribution;
      vector <double> frequency;
      vector <double> s20w;
      vector <double> D20w;
      vector <double> mw;

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif

      struct mfem_data residuals;
      float f_f0;
      unsigned int iteration;
      
   private slots:

      void setup_GUI();
      float linesearch();
      void update_resolution(double);
      void update_top(double);
      void update_bottom(double);
      void update_ff0(double);
      void set_ff0();
      void set_posBaseline();
      void set_negBaseline();
      void cofs_GUI();
      void enableButtons();
      void update_distribution();
      void clear_data(struct mfem_data *);
      void fit();
      float calc_residuals();
      float calc_testParameter(float);
      void second_plot(int);
      void write_cofs();
      void write_res();
      void calc_distros();

// re-implemented Functions:

      void view();
      void help();
      void save();
      void second_plot();
};

#endif

