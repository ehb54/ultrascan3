#ifndef US_DTACTRL_H
#define US_DTACTRL_H

#include <qprinter.h>
#include <qprogbar.h>
#include <qmessagebox.h>
#include <qsqldatabase.h>
#include <qsqlquery.h>
#include <qsqlcursor.h>
#include <qwt_plot_printfilter.h>
#include "us_rundetails.h"
#include "us_vhwsim.h"
#include "us.h"
#include "us_util.h"
#include "us_meniscus_dlg.h"
#include "us_selectplot.h"
#include "us_extern.h"
#include "us_math.h"
#include "us_db_tbl_buffer.h"
#include "us_db_tbl_vbar.h"
#include "us_printfilter.h"
#include "us_data_io.h"

extern int global_Xpos;
extern int global_Ypos;

struct currentRun
{
   unsigned int scans[8][3];

#ifdef WIN32
    #pragma warning( disable: 4251 )
#endif

   vector < vector < vector <float> > > temperature;   // temperature of each scan
   vector < vector < vector <unsigned int> > > time;   // time stamp of each scan
   vector < vector < vector <float> > > omega_s_t;      // omega-square-t of each scan
   vector < vector < vector <float> > > plateau;      // plateau value of each scan
   vector < vector < vector <unsigned int> > > rpm;   // Rotor speed in rotation per minute

#ifdef WIN32
    #pragma warning( default: 4251 )
#endif
};

class US_EXTERN Data_Control_W : public QFrame
{
   Q_OBJECT

   public:

      Data_Control_W(const int temp_run_type, QWidget *p=0, const char *name = 0);
      ~Data_Control_W();
      struct BufferData Buffer;
      struct peptideDetails Vbar_info;
      bool start;
      bool vbar_from_seq;
      bool buffer_from_seq;
      bool dont_plot;
      bool GUI;
      bool plateaus_corrected, rad_corrected;
      unsigned int first_cell, old_local_scans;
      int run_type;      // 0 = no run, 1 = Velocity Run, 2 = Equilibrium Run, 3 = Light Scattering Run

#ifdef WIN32
    #pragma warning( disable: 4251 )
#endif
      
    vector <struct rotorInfo> rotor_list;
      vector <struct centerpieceInfo> cp_list;

#ifdef WIN32
    #pragma warning( default: 4251 )
#endif
      struct runinfo run_inf;
      struct currentRun temp_run;
      unsigned int selected_cell;
      unsigned int selected_lambda;
      unsigned int selected_channel;
      bool has_data[8], baseline_flag;
      QString htmlDir, runID_old;
      US_Config *USglobal;
      US_Vbar_DB *vbar_dlg;
      US_Buffer_DB *buffer_dlg;
      US_Buffer_DB *buffer_db_dlg;
      US_Vbar_DB *vbar_db_dlg;
      QPushButton *pb_load;
      QPushButton *pb_details;
      QPushButton *pb_second_plot;
      QPushButton *pb_save;
      QPushButton *pb_print;
      QPushButton *pb_view;
      QPushButton *pb_help;
      QPushButton *pb_close;
      QPushButton *pb_vbar;
      QPushButton *pb_reset;
      QPushButton *pb_viscosity;
      QPushButton *pb_density;
      QPoint p1;
      QPoint p2;
      QLabel *banner1;
      QLabel *banner2;
      QLabel *lbl_run_id1;
      QLabel *lbl_run_id2;
      QLabel *lbl_cell_info1;
      QLabel *lbl_cell_info2;
   //   QLabel *lbl_cell_descr;
      QTextEdit *lbl_cell_descr;
      QLabel *lbl_temperature1;
      QLabel *lbl_temperature2;
      QLabel *lbl1_excluded;
      QLabel *lbl2_excluded;
      QLineEdit *density_le;
      QLineEdit *viscosity_le;
      QLabel *lbl_vbar;
      QLineEdit *vbar_le;
      QListBox *cell_select;
      QListBox *lambda_select;
      QListBox *channel_select;
      QLabel *smoothing_lbl;
      QwtCounter *smoothing_counter;
      QLabel *range_lbl;
      QwtCounter *range_counter;
      QLabel *position_lbl;
      QwtCounter *position_counter;
      QPushButton *pb_exsingle;
      QwtCounter *cnt_exsingle;
      QPushButton *pb_exrange;
      QwtCounter *cnt_exrange;
      QwtPlot *analysis_plot;
      QwtPlot *edit_plot;
      QLabel *lbl_status;

      QProgressBar *progress;
      RunDetails_F *run_details;
      unsigned int i, j, k, xpos, ypos, buttonw, buttonh, border, step;
      bool print_bw; // true if we are printing in black & white
      bool print_inv; // true if we are printing the inverse colors
      bool already; // checks if the initial analysis has been performed.
      bool show_gui; //determined if certain GUI functions are shown.
      bool vbarDisplay; //flag that controls wether the vbar line edit is updated
      bool densityDisplay; //flag that controls wether the viscosity line edit is updated
      bool viscosityDisplay; //flag that controls wether the density line edit is updated
      int extraCounter; //number that provides enough space to add extra counters
      bool smooth_flag; //flag that keeps track if load_scan was called from smoothig function
      float *rad, **abs;
      double *radius, **absorbance, *temp_radius, *temp_abs;
      double upper_limit, lower_limit, range;
      uint *curve;
      bool *included;
      unsigned int points;
      float density, viscosity;
      float density_tb, density_wt, viscosity_tb, viscosity_wt;
      float buoyancyb, buoyancyw, correction, initial_concentration, weight_average_s;
      float vbar;
      float vbar20;
      unsigned int smooth;
      double omega_s;
      double bd_range;
      double bd_position;
      unsigned int exclude_single;
      unsigned int exclude_range;
      bool reset_flag;
      bool exclude_flag;

      struct correctionTerms corr_inf;
      struct hydrodynamicData hydro_inf;

// Garbage Collection variables:

      bool scan_loaded;      // once a scan is loaded = true
      bool scan_copied;      // once a temporary copy of abs and rad have been made = true
      bool run_loaded;      // once a run was loaded = true
      int old_selected_cell;
      int old_selected_lambda;
      int old_points;
      float Density[8][4];
      float Viscosity[8][4];
      float   Vbar[8][4][3];
      float Vbar20[8][4][3];
      bool buffer_from_db;
      bool vbar_from_db;

      QString fn;

   public slots:


// Garbage Collection functions:

      void cleanup_loaded_scan();      // once a scan is loaded = true
      void cleanup_copied_scan();      // once a temporary copy of abs and rad have been made = true
      void cleanup_loaded_run();      // once a run was loaded = true
      void newMessage(QString, int);
      void newMessage(QString);
      void show_cell(int);
      void show_lambda(int);
      void show_channel(int);
      int load_data();
      int load_data(const QString &);
      int load_data(bool);
      int load_data(const QString &, bool);
      int write_data();
      void details();
      void updateLabels();
      void quit();
      void update_density();
      void update_density(const QString &);
      void update_viscosity();
      void update_viscosity(const QString &);
      void read_vbar();
      void update_vbar();
      void update_vbar(const QString &);
      void update_vbar_lbl(float, float);
      void update_buffer_lbl(float, float);
      void update_smoothing(double);
      void update_single(double);
      void update_range(double);
      void update_boundary_range(double);
      void update_boundary_position(double);
      void ex_single();
      void ex_range();
      void plot_edit();
      void update_screen();
      int load_scan();
      void calc_correction(float);
      void find_plateaus();
      void get_buffer();
      void update_buffer_signal(float, float);
      void update_vbar_signal(float, float);
      void update_mw_signal(float);
      bool createHtmlDir();

// Virtual Functions, need to be defined in derived class:

      virtual int plot_analysis();
      virtual int setups();
      virtual void view();
      virtual void reset();
      virtual void help();
      virtual void save();
      virtual void second_plot();
      virtual void getAnalysisPlotMouseReleased(const QMouseEvent &e);
      virtual void getAnalysisPlotMousePressed(const QMouseEvent &e);
      virtual void print();  // default definition is to print analysis plot
      virtual void updateButtons();

   protected slots:
      virtual void closeEvent(QCloseEvent *);
      virtual void excludeSingleSignal(unsigned int);
      virtual void excludeRangeSignal(unsigned int);

   private slots:
      void create_GUI();

   signals:

      void vbarChanged(float, float);
      void mwChanged(float);
      void bufferChanged(float, float);
      void dataLoaded();
      void excludeUpdated();
      void controls_updated();
      void datasetChanged();

};

#endif

