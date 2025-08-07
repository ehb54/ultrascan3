//! \file us_plot_control_pc.h
#ifndef US_PLOT_CONTROL_H
#define US_PLOT_CONTROL_H

#include "us_extern.h"
#include "us_help.h"
#include "us_model.h"
#include "us_plot.h"
#include "us_plot3d.h"
#include "us_widgets_dialog.h"

//! \brief A class to provide a window with enhanced plot controls

class US_PlotControlPc : public US_WidgetsDialog {
  Q_OBJECT

 public:
  //! \brief US_PlotControlPc constructor
  //! \param p       Pointer to the parent of this widget
  //! \param amodel  Pointer to the model to be plotted
  US_PlotControlPc(QWidget* = 0, US_Model* = 0);

 private:
  QVBoxLayout* mainLayout;
  QGridLayout* controlsLayout;
  QHBoxLayout* buttonsLayout;

  QwtCounter* ct_rxscale;
  QwtCounter* ct_ryscale;

  QVector<QCheckBox*> xCheck;
  QVector<QCheckBox*> yCheck;

  QPointer<US_Plot3D> plot3d_w;
  US_Model* model;

  double zscale;
  double gridres;
  double pksmooth;
  double pkwidth;

 protected:
  US_Help showHelp;

 private slots:
  void xmwtCheck(bool);
  void ymwtCheck(bool);
  void xsedCheck(bool);
  void ysedCheck(bool);
  void xdifCheck(bool);
  void ydifCheck(bool);
  void xfcoCheck(bool);
  void yfcoCheck(bool);
  void xfraCheck(bool);
  void yfraCheck(bool);
  void xvbaCheck(bool);
  void yvbaCheck(bool);
  void checkSet(bool, bool, int);
  void zscal_value(double);
  void gridr_value(double);
  void peaks_value(double);
  void peakw_value(double);
  void plot3_btn(void);
  void close_all(void);
  void plot_close(void);
  int dimensionType(QVector<QCheckBox*>&);

  void help(void) { showHelp.show_help("us_pcsa_controls.html"); };
};
#endif
