//! \file us_distrib_plot.h
#ifndef US_DISTRIB_PLOT_H
#define US_DISTRIB_PLOT_H

#include "us_dataIO.h"
#include "us_extern.h"
#include "us_help.h"
#include "us_plot.h"
#include "us_widgets_dialog.h"

#ifndef DbgLv
#define DbgLv(a) \
  if (dbg_level >= a) qDebug()  //!< debug-level-conditioned qDebug()
#endif

//! \brief A class to provide a window with the details of a run

class US_DistribPlot : public US_WidgetsDialog {
  Q_OBJECT

 public:
  //! \param divfracs Reference to a vector of division bdoundary fractions
  //! \param divsedcs Reference to a vector of division intercept sedcoeffs
  //! \param tconc    Total concentration
  US_DistribPlot(QVector<double>&, QVector<double>&, const double);

  //! \param plot1File The name of the distribution plot file
  //! \param plot2File The name of the histogram plot file
  void save_plots(QString&, QString&);

 private:
  QVector<double>& bfracs;  // boundary fractions for divisions
  QVector<double>& dsedcs;  // sed.coeff. intercepts for divisions
  double tot_conc;

  enum pType { DISTR, HISTO, ENVEL, COMBO, NONE };

  US_Help showHelp;

  pType plotType;
  pType plotTypeH;

  QLabel* lb_sensitivity;
  QLabel* lb_smoothing;

  QPushButton* pb_histogram;
  QPushButton* pb_envelope;
  QPushButton* pb_plot_type;
  QPushButton* pb_help;
  QPushButton* pb_close;

  QwtCounter* ct_sensitivity;
  QwtCounter* ct_smoothing;

  QwtPlot* data_plot;
  QwtPlotCurve* dcurve;
  QwtPlotCurve* hcurve;
  QwtPlotCurve* ecurve;

  int divsCount;
  int nSensit;
  int nSmooth;
  int dbg_level;

 private slots:
  void type_plot(void);
  void hide_histo(void);
  void hide_envel(void);
  void show_plot(void);
  void plot_distrib(void);
  void plot_histogram(void);
  void plot_envelope(void);
  void plot_combined(void);
  void change_sensit(double);
  void change_smooth(double);
  int histo_data(QVector<double>&, QVector<double>&);
  int envel_data(QVector<double>&, QVector<double>&);
  void save_data_file(QString);
  void save_and_close(void);
  void help(void) { showHelp.show_help("vhw_distrib_plot.html"); };
};
#endif
