//! \file us_show_norm.h
#ifndef US_SHOW_NORM_H
#define US_SHOW_NORM_H

#include "qwt_color_map.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_marker.h"
#include "qwt_plot_panner.h"
#include "qwt_plot_spectrogram.h"
#include "qwt_plot_zoomer.h"
#include "qwt_scale_draw.h"
#include "qwt_scale_widget.h"
#include "us_2dsa_process.h"
#include "us_colorgradIO.h"
#include "us_extern.h"
#include "us_help.h"
#include "us_model.h"
#include "us_plot.h"
#include "us_spectrodata.h"
#include "us_widgets_dialog.h"

//! \brief Less-than function for sorting distributions
bool distro_lessthan(const S_Solute &, const S_Solute &);

//! \brief A class to provide a window with 2DSA analysis controls
class US_show_norm : public US_WidgetsDialog {
      Q_OBJECT

   public:
      US_show_norm(US_Model *, bool &, QWidget * = 0);

   private:
      bool &cnst_vbar;
      US_Model *model;

      enum attr_type { ATTR_S, ATTR_K, ATTR_W, ATTR_V, ATTR_D, ATTR_F };

      int dbg_level;

      QHBoxLayout *mainLayout;

      QWidget *parentw;

      QPushButton *pb_plot;
      QPushButton *pb_save;

      QLabel *lb_plt_smin;
      QLabel *lb_plt_smax;
      QLabel *lb_plt_kmin;
      QLabel *lb_plt_kmax;

      QTextEdit *te_distr_info;

      QLineEdit *le_cmap_name;
      QLineEdit *le_prefilt;

      QwtCounter *ct_resolu;
      QwtCounter *ct_xreso;
      QwtCounter *ct_yreso;
      QwtCounter *ct_zfloor;
      QwtCounter *ct_plt_kmin;
      QwtCounter *ct_plt_kmax;
      QwtCounter *ct_plt_smin;
      QwtCounter *ct_plt_smax;
      QwtCounter *ct_curr_distr;

      QwtPlot *data_plot;

      QwtPlotPicker *pick;

      QwtLinearColorMap *colormap;

      QPushButton *pb_ldcolor;
      QPushButton *pb_close;
      QPushButton *pb_reset;
      QPushButton *pb_refresh;

      QCheckBox *ck_autosxy;
      QCheckBox *ck_autoscz;
      QCheckBox *ck_conloop;
      QCheckBox *ck_plot_sk;
      QCheckBox *ck_plot_wk;
      QCheckBox *ck_plot_sv;
      QCheckBox *ck_plot_wv;
      QCheckBox *ck_zpcent;
      QCheckBox *ck_savepl;
      QCheckBox *ck_locsave;

      QRadioButton *rb_x_s;
      QRadioButton *rb_x_ff0;
      QRadioButton *rb_x_mw;
      QRadioButton *rb_x_vbar;
      QRadioButton *rb_x_D;
      QRadioButton *rb_x_f;
      QRadioButton *rb_y_s;
      QRadioButton *rb_y_ff0;
      QRadioButton *rb_y_mw;
      QRadioButton *rb_y_vbar;
      QRadioButton *rb_y_D;
      QRadioButton *rb_y_f;
      QButtonGroup *bg_x_axis;
      QButtonGroup *bg_y_axis;

      QList<S_Solute> xy_distro;
      QList<S_Solute> xy_distro_zp;

      double resolu;
      double plt_smin;
      double plt_smax;
      double plt_kmin;
      double plt_kmax;
      double plt_zmin;
      double plt_zmax;
      double plt_zmin_zp;
      double plt_zmax_zp;
      double plt_zmin_co;
      double plt_zmax_co;
      double s_range;
      double k_range;
      double xreso;
      double yreso;
      double zfloor;

      int init_solutes;
      int plot_x;
      int plot_y;

      bool auto_sxy;
      bool auto_scz;
      bool cont_loop;
      bool looping;
      bool need_save;
      bool runsel;
      bool latest;
      bool zpcent;

      QString xa_title;
      QString ya_title;
      QString cmapname;
      QString mfilter;

      QStringList pfilts;

   private slots:
      void select_x_axis(int);
      void select_y_axis(int);
      void sort_distro(QList<S_Solute> &, bool);
      void load_color(void);
      void update_resolu(double);
      void update_xreso(double);
      void update_yreso(double);
      void update_zfloor(double);
      void update_plot_smin(double);
      void update_plot_smax(double);
      void update_plot_kmin(double);
      void update_plot_kmax(double);
      void set_limits(void);
      void reset(void);
      void plot_data(int);
      void plot_data(void);
      void select_autosxy(void);
      void select_autoscz(void);
      void build_xy_distro(void);
      QString anno_title(int);
      QwtLinearColorMap *ColorMapCopy(QwtLinearColorMap *);
};
#endif
