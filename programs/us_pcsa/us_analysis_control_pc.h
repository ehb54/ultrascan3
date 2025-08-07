//! \file us_analysis_control_pc.h
#ifndef US_ANALYSIS_CTL_H
#define US_ANALYSIS_CTL_H

#include "us_extern.h"
#include "us_help.h"
#include "us_mlplot.h"
#include "us_pcsa_process.h"
#include "us_plot.h"
#include "us_widgets_dialog.h"

//! \brief A class to provide a window with 2DSA analysis controls

class US_AnalysisControlPc : public US_WidgetsDialog {
  Q_OBJECT

 public:
  //! \brief US_AnalysisControlPc constructor
  //! \param dsets   Reference to the datasets list
  //! \param p       Pointer to the parent of this widget
  US_AnalysisControlPc(QList<US_SolveSim::DataSet*>&, QWidget* p = 0);

  //! \brief Public function to close an analysis control window
  void close(void);

 public slots:
  void update_progress(double);
  void completed_process(int);
  void progress_message(QString, bool = true);
  void reset_steps(int, int);

 private:
  int dbg_level;
  int ncsteps;
  int nctotal;
  int grtype;
  int bmndx;
  int nypts;
  int nlpts;
  int ctype;
  int ctypex;
  int nlmodl;
  int attr_x;
  int attr_y;
  int attr_z;
  int sol_type;

  double xmin;
  double xmax;
  double ymin;
  double ymax;
  double yinc;
  double varimin;

  bool need_fit;
  bool need_final;
  bool resume;

  QString fitpars;
  QString type_x;
  QString type_y;
  QString type_z;
  QString z_func;

  QHBoxLayout* mainLayout;
  QGridLayout* controlsLayout;
  QGridLayout* optimizeLayout;

  QList<US_SolveSim::DataSet*>& dsets;

  QVector<US_ModelRecord> mrecs;
  QVector<US_ModelRecord> mrecs_mc;

  US_DataIO::EditedData* edata;
  US_DataIO::RawData* sdata;
  US_DataIO::RawData* rdata;
  US_Model* model;
  US_Noise* ri_noise;
  US_Noise* ti_noise;
  US_SimulationParameters* sparms;
  QPointer<QTextEdit> mw_stattext;
  QStringList* mw_modstats;
  QVector<US_ModelRecord>* mw_mrecs;
  QVector<US_ModelRecord>* mw_mrecs_mc;
  int* mw_baserss;
  QVector<int> ctypes;

  QWidget* parentw;
  QWidget* main_widg;
  US_pcsaProcess* processor;
  US_MLinesPlot* mlnplotd;

  QwtCounter* ct_lolimitx;
  QwtCounter* ct_uplimitx;
  QwtCounter* ct_lolimity;
  QwtCounter* ct_uplimity;
  QwtCounter* ct_varcount;
  QwtCounter* ct_gfiters;
  QwtCounter* ct_gfthresh;
  QwtCounter* ct_cresolu;
  QwtCounter* ct_lmmxcall;
  QwtCounter* ct_tralpha;
  QwtCounter* ct_thrdcnt;

  QComboBox* cb_curvtype;
  QComboBox* cb_z_type;

  QCheckBox* ck_lmalpha;
  QCheckBox* ck_fxalpha;
  QCheckBox* ck_tinoise;
  QCheckBox* ck_rinoise;

  QButtonGroup* bg_x_axis;
  QButtonGroup* bg_y_axis;

  QRadioButton* rb_x_s;
  QRadioButton* rb_x_ff0;
  QRadioButton* rb_x_mw;
  QRadioButton* rb_x_vbar;
  QRadioButton* rb_x_D;
  ;
  QRadioButton* rb_y_s;
  QRadioButton* rb_y_ff0;
  QRadioButton* rb_y_mw;
  QRadioButton* rb_y_vbar;
  QRadioButton* rb_y_D;

  QLineEdit* le_minvari;
  QLineEdit* le_minrmsd;
  QLineEdit* le_z_func;

  QTextEdit* te_status;

  QPushButton* pb_pltlines;
  QPushButton* pb_startfit;
  QPushButton* pb_scanregp;
  QPushButton* pb_finalmdl;
  QPushButton* pb_stopfit;
  QPushButton* pb_advanaly;
  QPushButton* pb_plot;
  QPushButton* pb_save;
  QPushButton* pb_help;
  QPushButton* pb_close;

 protected:
  US_Help showHelp;
  QProgressBar* b_progress;

 private slots:
  void optimize_options(void);
  void uncheck_optimize(int);
  void xlim_change(void);
  void ylim_change(void);
  void reso_change(void);
  void type_change(void);
  void set_alpha(void);
  void start(void);
  void fit_final(void);
  void scan_alpha(void);
  void final_only(void);
  void stop_fit(void);
  void plot(void);
  void advanced(void);
  void save(void);
  void close_all(void);
  void compute(void);
  void plot_lines(void);
  void adjust_xyz(const int = 0);
  void recompute_mrec(void);
  void select_x_axis(int);
  void select_y_axis(int);
  void ztype_change(int);
  void closed(QObject*);
  void fitpars_connect(bool);
  QString fitpars_string(void);
  int memory_check(void);
  void set_solute_type(void);

  void help(void) { showHelp.show_help("pcsa_analys.html"); };
};
#endif
