//! \file us_adv_dmgamc.h
#ifndef US_ADV_DMGA_H
#define US_ADV_DMGA_H

#include <qwt_plot.h>

#include "us_extern.h"
#include "us_help.h"
#include "us_model.h"
#include "us_plot.h"
#include "us_widgets_dialog.h"

//! \brief A class to provide a window for advanced DMGA-MC analysis controls

class US_AdvDmgaMc : public US_WidgetsDialog {
  Q_OBJECT

 public:
  //! \brief US_AdvDmgaMc constructor
  //! \param amodel   Pointer to model
  //! \param aimodels Reference to iteration models vector
  //! \param adv_vals Reference to advanced values map
  //! \param p        Pointer to the parent of this widget
  US_AdvDmgaMc(US_Model*, QVector<US_Model>&, QMap<QString, QString>&,
               QWidget* p = 0);

 private:
  US_Model* model;
  QVector<US_Model>& imodels;
  QMap<QString, QString>& parmap;
  US_Model umodel;

  QStringList ls_params;

  QString lddesc;
  int ncomp;

  QVBoxLayout* mainLayout;

  QGridLayout* paramLayout;
  QGridLayout* lo_mean;
  QGridLayout* lo_median;
  QGridLayout* lo_mode;
  QGridLayout* lo_curmod;

  US_Plot* plotLayout;
  QwtPlot* data_plot;

  QWidget* parentw;

  QPushButton* pb_nextmodel;
  QPushButton* pb_reaction;

  QLabel* lb_ms_iter;

  QwtCounter* ct_modelnbr;

  QLineEdit* le_modtype;
  QLineEdit* le_kdissoc;
  QLineEdit* le_koffrate;
  QLineEdit* le_sedcoeff;
  QLineEdit* le_difcoeff;
  QLineEdit* le_moweight;
  QLineEdit* le_friratio;
  QLineEdit* le_vbar20;
  QLineEdit* le_partconc;
  QLineEdit* le_ms_mean;
  QLineEdit* le_ms_95lo;
  QLineEdit* le_ms_95hi;
  QLineEdit* le_ms_medi;
  QLineEdit* le_ms_mode;
  QLineEdit* le_ms_iter;

  QRadioButton* rb_mean;
  QRadioButton* rb_median;
  QRadioButton* rb_mode;
  QRadioButton* rb_curmod;

  QComboBox* cb_params;

 protected:
  US_Help showHelp;

 private slots:

  void done(void);
  void next_model(void);
  void change_model(double);
  void set_model_type(bool);
  void plot_distrib(void);
  void next_param(void);
  void prev_param(void);
  void simulate(void);
  void help(void) { showHelp.show_help("fem_adv_dmgamc.html"); };
};
#endif
