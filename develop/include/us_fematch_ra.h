#ifndef US_FEMATCH_RA_H
#define US_FEMATCH_RA_H

#include "us_dtactrl.h"
#include "us_extern.h"
#include "us_resplot.h"
#include "us_util.h"
#include "us_plotdata.h"
#include "us_3dsolutes.h"
#include "us_astfem_rsa.h"
#include "us_ga_initialize.h"
#include "us_montecarlostats.h"
#include "../3dplot/mesh2mainwindow.h"

#include <qbuttongroup.h>

#ifndef WIN32
#include <unistd.h>
#endif
#include <math.h>
#include <float.h>
#include <cerrno>

using namespace std;

class Parameter
{
   public:
      double x;
      double y;
      Parameter() {};
      ~Parameter() {};
      bool operator==(const Parameter& objIn)
      {
         return (x == objIn.x);
      }
      bool operator!=(const Parameter& objIn)
      {
         return (x != objIn.x);
      }
      bool operator < (const Parameter& objIn) const
      {
         if (x < objIn.x)
         {
            return (true);
         }
         else
         {
            return(false);
         }
      }
};

struct par
{
   QString name;
   double mode;
   double mean;
   double median;
   list <Parameter> parameter_list;
};

class  US_EXTERN US_FeMatchRa_W : public Data_Control_W
{
   Q_OBJECT

   public:

      US_FeMatchRa_W(QWidget *p = 0, const char *name = 0);
      ~US_FeMatchRa_W();

   private:

      unsigned int monte_carlo_iterations, simpoints, current_parameter, current_model;
      SA2d_control_variables sa2d_ctrl_vars;
      int mesh, moving_grid, plotmode;
      float band_volume;
      
#ifdef WIN32
        #pragma warning ( disable: 4251 )
#endif

      vector <double> ri_noise, ti_noise;
      vector <struct mfem_data> simdata;
      vector <par> ga_param; // struct
      vector <ModelSystem> msv;

#ifdef WIN32
        #pragma warning ( default: 4251 )
#endif

      QString analysis_type;

      QLabel *lbl_variance;
      QLabel *lbl_variance2;
      QLabel *lbl_bandVolume;
      QLabel *lbl_simpoints;
      QLabel *lbl_parameter;

      QPushButton *pb_loadConstraints;
      QPushButton *pb_model;

      QwtCounter *cnt_simpoints;
      QwtCounter *cnt_band_volume;
      QwtCounter *cnt_model;
      QwtCounter *cnt_parameter;

      QComboBox *cmb_radialGrid;
      QComboBox *cmb_timeGrid;

      QButtonGroup *bg_plotmode;

      QCheckBox  *cb_mode;
      QCheckBox  *cb_median;
      QCheckBox  *cb_mean;
      QCheckBox  *cb_current_model;
      
      US_ResidualPlot *resplot;
      US_Pixmap *pm;
      float rmsd;
      int model;
      bool stopFlag, movieFlag;
      struct mfem_data residuals;
      ModelSystem ms;
      struct SimulationParameters sp;
      US_Astfem_RSA *astfem_rsa;
      US_ModelEditor *component_dialog;

   private slots:

      void setup_GUI();
      void enableButtons();
      void clear_data(struct mfem_data *);
      float fit();
      void write_res();
      void load_model();
      void clearDisplay();
      void printError(const int &);
      void update_radialGrid(int);
      void update_timeGrid(int);
      void update_simpoints(double val);
      void update_band_volume(double val);
      void update_model(double val);
      void update_parameter(double val);
      void assign_parameters();
      void assign_model();
      void show_model();
      void show_parameter();
      void select_plotmode(int);
      void reduce(list <Parameter> *, double * /*mode*/, double * /*mean*/, double * /*median*/);
      //void update_plot();

// re-implemented Functions:

      void view();
      void help();
      void save();
      void second_plot();
};

#endif

