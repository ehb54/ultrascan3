#ifndef US_VIEWMWL_H
#define US_VIEWMWL_H

#include "us.h"
#include "us_extern.h"
#include "us_util.h"
#include "us_pixmap.h"
#include "us_femglobal.h"
#include "../3dplot/mesh2mainwindow.h"

#include <vector>
#include <string>

#include <qlabel.h>
#include <qgl.h>
#include <qframe.h>
#include <qstring.h>
#include <qcheckbox.h>
#include <qwt_counter.h>
#include <qwt_plot.h>
#include <qlayout.h>
#include <qprogressbar.h>
#include <qstringlist.h>
#include <qlistbox.h>
#include <qfiledialog.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qpixmap.h>

struct radial_scan
{
   float wavelength;
   vector <short int> absorbance;
};

struct time_point
{
   float rotor_speed;                        // rotor speed at this time
   float time;                               // seconds elapsed since the start of the expt.
   float omega_2_t;                            // omega^2*t integral at this time
   float temperature;                        // the temperature at this time
   vector <float> radius;                     // radius values
   vector <struct radial_scan> scan;          // dimension: number of radial points
};

struct channel
{
   QString contents;                           // sample description
   short unsigned int channel_number;         // the channel number
   short int measurement_mode;                // 0 for absorbance, 1 for intensity
   vector <struct time_point> data;            // all data for one channel, dimension: number of time points
};

struct cell
{
   short unsigned int cell_number;            // the cell number
   short unsigned int centerpiece;             // ID for centerpiece
   vector <struct channel> cell_channel;       // all channels from one cell, dimension: number of channels
};

struct experiment
{
   QString experiment_name;                  // identifier/description for the experiment
   vector <struct cell> cell_info;             // dimension: number of cells in the experiment
};


class US_EXTERN US_ViewMWL : public QFrame
{
   Q_OBJECT

   public:
      US_ViewMWL(QWidget *p=0, const char *name = 0);
      ~US_ViewMWL();
      Mesh2MainWindow *mainwindow;

      bool export_mode; // wavelength=true or radius=false
      bool view_mode; // 2D=false or 3D=true
      bool file_format; // ASCII=true, Binary=false
      bool loading; // controls aborting of loading process
      bool show_model; // controls if a loaded model is shown
      int measurement_mode; // Absorbance=0, Intensity=1
      unsigned int current_cell, current_channel, max_time, min_time;
      unsigned int min_radius_element, max_radius_element, min_lambda_element, max_lambda_element;
      float max_lambda, min_lambda, max_radius, min_radius, min_od, max_od;
      unsigned int average;
      struct experiment MWL_experiment;
      struct cell cell_data;
      struct SA2d_control_variables controlvar_3d;
      vector <struct mfem_data> model_vector;
      vector <struct element_3D> abs_val;
      vector <int> lambda_flag;
      float xscaling, yscaling, zscaling;

      bool widget3d_flag, pngs;
      US_Pixmap *pm;
      QProgressBar *progress;
      QwtPlot *plot_2d, *plot_residual;

      QListBox *lb_channel;
      QListBox *lb_cell;

      QLabel *lbl_info;
      QLabel *lbl_status;

      QLabel *lbl_max_lambda;
      QLabel *lbl_max_time;
      QLabel *lbl_max_radius;
      QLabel *lbl_min_lambda;
      QLabel *lbl_min_time;
      QLabel *lbl_min_radius;
      QLabel *lbl_update;
      QLabel *lbl_update2;
      QLabel *lbl_average;
      QLabel *lbl_xscaling;
      QLabel *lbl_yscaling;
      QLabel *lbl_zscaling;

      QCheckBox *cb_set_wavelength;
      QCheckBox *cb_set_radius;
      QCheckBox *cb_set_2d;
      QCheckBox *cb_set_3d;
      QCheckBox *cb_ascii;
      QCheckBox *cb_binary;
      QCheckBox *cb_intensity;
      QCheckBox *cb_absorbance;
      QCheckBox *cb_pngs;
      QCheckBox *cb_model;
      QCheckBox *cb_rev_wl_axis;

      QwtCounter *cnt_max_lambda;
      QwtCounter *cnt_min_lambda;
      QwtCounter *cnt_max_radius;
      QwtCounter *cnt_min_radius;
      QwtCounter *cnt_max_time;
      QwtCounter *cnt_min_time;
      QwtCounter *cnt_average;
      QwtCounter *cnt_xscaling;
      QwtCounter *cnt_yscaling;
      QwtCounter *cnt_zscaling;

      QPushButton *pb_save;
      QPushButton *pb_load;
      QPushButton *pb_export_data;
      QPushButton *pb_print;
      QPushButton *pb_update;
      QPushButton *pb_movie;
      QPushButton *pb_help;
      QPushButton *pb_cancel;
      QPushButton *pb_model;

      US_Config *USglobal;    /*!< A US_Config reference. */

   public slots:

      void load();
      void save();

   private slots:

      void update_max_lambda(double);
      void update_min_lambda(double);
      void update_max_time(double);
      void update_min_time(double);
      void update_max_radius(double);
      void update_min_radius(double);
      void update_average(double);
      void update_xscaling(double);
      void update_yscaling(double);
      void update_zscaling(double);

      void set_radius();
      void set_model();
      void set_wavelength();
      void set_absorbance();
      void set_intensity();
      void set_ascii();
      void set_binary();
      void set_2d();
      void set_3d();
      void set_pngs();
      void setLoading();

      void find_elements();
      void find_minmax_od();
      void cancel();
      void load_model();
      void help();
      void export_data();
      void print();
      void update();
      void update(unsigned int);
      void update(unsigned int, QString);
      void movie();
      void select_cell(int);
      void select_channel(int);

   protected slots:
      void setup_GUI();
      void closeEvent(QCloseEvent *);

};

using namespace Qwt3D;

class ReversedScale : public LinearScale
{
 public:
   Scale* clone() const {return new ReversedScale;}
   QString ticLabel(unsigned int idx) const
   {
      unsigned int s = majors_p.size();
      if ( idx < s )
      {
         return QString::number(majors_p[s - idx - 1]);
      }
      return QString("");
   }
};

#endif
