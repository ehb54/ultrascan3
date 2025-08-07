//! \file us_mlplot.h
#ifndef US_MLINES_PLOT_H
#define US_MLINES_PLOT_H

#include "qwt_color_map.h"
#include "us_extern.h"
#include "us_help.h"
#include "us_model.h"
#include "us_noise.h"
#include "us_pcsa_modelrec.h"
#include "us_plot.h"
#include "us_resids_bitmap.h"
#include "us_widgets_dialog.h"

//! \brief A class to provide a window with a model lines plot

class US_MLinesPlot : public US_WidgetsDialog {
  Q_OBJECT

 public:
  //! \brief Model Lines Plot constructor
  //! \param ylo   y low value
  //! \param yhi   y high value
  //! \param xlo   x low value
  //! \param xhi   x high value
  //! \param typ   curve type flag (0-3 -> SL,IS,DS,HL)
  //! \param nkp   number of k points (variations)
  //! \param nlp   number of line curve points (model solutes)
  //! \param bmx   best model index
  US_MLinesPlot(double&, double&, double&, double&, int&, int&, int&, int&);

  void setModel(US_Model*, QVector<US_ModelRecord>&);
  void plot_data(void);

 private:
  double& ymin;
  double& ymax;
  double& xmin;
  double& xmax;

  int& ctype;
  int& nkpts;
  int& nlpts;
  int& bmndx;

  US_Plot* plotLayout1;

  QwtPlot* data_plot1;

  QHBoxLayout* mainLayout;
  QVBoxLayout* leftLayout;
  QVBoxLayout* rightLayout;
  QGridLayout* pltctrlsLayout;
  QHBoxLayout* buttonsLayout;

  QLabel* lb_ltypeh;
  QLabel* lb_counth;
  QLabel* lb_rmsdhd;
  QLabel* lb_rmsdb;
  QLabel* lb_rmsdw;
  QLabel* lb_neline;
  QLabel* lb_nsline;
  QLabel* lb_nvline;

  QwtCounter* ct_neline;
  QwtCounter* ct_nsline;
  QwtCounter* ct_nvline;

  QLineEdit* le_mtype;
  QLineEdit* le_nlines;
  QLineEdit* le_npoints;
  QLineEdit* le_varcount;
  QLineEdit* le_rmsdb;
  QLineEdit* le_rmsdw;
  QLineEdit* le_rmsde;
  QLineEdit* le_rmsds;
  QLineEdit* le_rmsdv;
  QLineEdit* le_colmap;

  QPushButton* pb_colmap;

  QVector<US_ModelRecord> mrecs;
  QVector<QColor> cs_colors;
  QwtLinearColorMap* colormap;
  QString cmapname;

  US_Model* model;

  int dbg_level;
  int nmodel;
  int neline;
  int nsline;
  int nvline;

  double rmsd_best;
  double rmsd_worst;
  double rmsd_elite;
  double rmsd_solut;
  double rmsd_visib;

 protected:
  US_Help showHelp;

 private slots:
  void close_all(void);
  void updateElite(double);
  void updateSolutes(double);
  void updateVisible(double);
  void defaultColorMap(void);
  void selectColorMap(void);
  void showColorItems(bool);
  QColor positionColor(double);
  QwtLinearColorMap* reverseColorMap(void);

  void help(void) { showHelp.show_help("pcsa_mlines.html"); };
};
#endif
