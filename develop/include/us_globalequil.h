#ifndef US_GLOBALEQUIL_H
#define US_GLOBALEQUIL_H

#include <qapp.h>
#include <qstyle.h>
#include <qprinter.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qwt_plot.h>
#include <qwt_counter.h>
#include <qwt_symbol.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qpixmap.h>
#include <qfiledialog.h>
#include <qtextstream.h>
#include "us_eqmodelctrl.h"
#include "us_eqfitter.h"
#include "us_util.h"
#include "us_pixmap.h"
#include "us_montecarlo.h"
#include "us.h"
#include "us_math.h"
#include "us_dtactrl.h"
#include "us_db_tbl_buffer.h"
#include "us_db_tbl_vbar.h"
#include "us_selectplot.h"
#include "us_printfilter.h"
#include "us_data_io.h"


class US_EXTERN US_GlobalEquil : public QFrame
{
   Q_OBJECT

   public:

      US_GlobalEquil(QWidget *p=0, const char *name="usglobalequil");
      ~US_GlobalEquil();
      
#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif

    vector <struct rotorInfo> rotor_list;
      vector <struct centerpieceInfo> cp_list;
      bool model_widget;
      bool monte_carlo_widget;
      bool fitting_widget;
      bool has_data[8];
      bool mouse_was_moved;
      bool float_parameters;
      bool scans_have_problems;
      bool show_messages;
      bool scans_initialized;
      QString projectName, htmlDir, fitName;
      US_Pixmap *pm;
      int model;
      unsigned int dataset, scans_in_list, first_fitted_scan, fitpoints, parameters;
      uint selected_scan;
      unsigned int new_start_index, new_stop_index;
      vector <struct EquilScan_FitProfile> scanfit_vector;
      struct EquilRun_FitProfile runfit;
      struct runinfo run_inf;
      float bottom[8], std_deviation;
      float od_limit;   //limit of the max. upper absorbance/fringe value
      vector <QString> filenames;
      vector <struct runinfo> run_information;
      vector <QString> scan_info;
      vector <float> residuals, original_data, yfit;

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif

      US_MonteCarlo *monte_carlo_window;
      struct MonteCarlo mc;

// These parameters are needed to set up the fixed molecular weight distribution model:
      float mw_upperLimit, mw_lowerLimit, model3_vbar;
      bool model3_vbar_flag;

// These parameters are needed for the user-defined equilibrium models:
      float nmer_size1, nmer_size2;

// Thesr parameters are needed for retrieving buffer, vbar info from DB
      struct BufferData Buffer;
      struct peptideDetails Vbar_info;
      US_Buffer *buffer_db_dlg;
      US_Vbar_DB *vbar_db_dlg;
      float Density[8][4];
      float Vbar20[8][4][3];
      bool buffer_from_db;
      bool vbar_from_db;

      QListBox *lb_scans;
      QPushButton *pb_load;
      QPushButton *pb_details;
      QPushButton *pb_unload;
      QPushButton *pb_print;
      QPushButton *pb_diagnostics;
      QPushButton *pb_scan_check;
      QPushButton *pb_reset_limits;
      QPushButton *pb_help;
      QPushButton *pb_histogram;
      QPushButton *pb_close;
      QPushButton *pb_model;
      QPushButton *pb_fitcontrol;
      QPushButton *pb_report;
      QPushButton *pb_monte_carlo;
      QPushButton *pb_loadfit;
      QPushButton *pb_float_all;
      QPushButton *pb_initialize;
      QPushButton *pb_model_control;
      QLabel *lbl_banner1;
      QLabel *lbl_banner2;
      QLabel *lbl_banner3;
      QLabel *lbl_scans;
      QLabel *lbl_projectName;
      QLineEdit *le_projectName;
      QwtPlot *data_plot;
      QwtPlot *histogram_plot;
      QwtCounter *cnt_scans;
      US_Config *USglobal;
      US_EqModelControl *model_control;
      US_ShowDetails *ShowDetails;
      US_SelectModel *SelectModel;
      US_InfoPanel *InfoPanel;
      US_EqFitter *eqFitter;
      RunDetails_F *run_details;
      int xpos, ypos, buttonw, buttonh, border;

   private slots:
      void load();
      int load(const QString &);
      int assign_scanfit(int, int, int, const QString);
      void unload();
      void update_projectName(const QString &);
      void details();
      void print();
      bool createHtmlDir();
      void diagnostics();
      void scan_check();
      void reset_limits();
      void update_limit(float);
      void help();
      void quit();
      void update_model_button();
      void update_pixmap();
      void updateModelWindow();
      void updateViewer();
      void change_FitFlag(int);
//      void change_FitFlag(QListBoxItem *, const QPoint &);
      void show_scan(int);
      void activate_scan(double);   // called from buttonReleased signal of Model COntrol widget
      void call_show_scan();   // called when line edit for max od has return pressed to update scan
                              // acts as an intermediary slot between void and int
      void select_model();
      void fitcontrol();
      void report();
      void monte_carlo();
      void monte_carlo_iterate();

      void loadfit();
      void loadfit(const QString &);
      void float_all();
      void fix_all();
      void histogram();
      void initialize();
      float calc_testParameter(float);
      float linesearch();
      void select_model_control();
      void setup_vectors();
      void select_scan(double);
      void select_scan(int);
      void select_scan(uint);
      void fit_converged();
      void fit_suspended();
      void fit_resumed();
      void writeScanList(const QString &, const int);
      void clean_runInfo();

   protected slots:
      void setup_GUI();
//      virtual void resizeEvent(QResizeEvent *e);
      void getPlotMouseMoved(const QMouseEvent &e);
      void getPlotMouseReleased(const QMouseEvent &e);
      void closeEvent(QCloseEvent *);
};

#endif

