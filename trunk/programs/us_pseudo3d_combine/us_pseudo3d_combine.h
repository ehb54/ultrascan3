#ifndef US_PSE3D_COMBINE_H
#define US_PSE3D_COMBINE_H

#include <QtGui>

#include "qwt_plot_marker.h"
#include "qwt_plot_spectrogram.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_zoomer.h"
#include "qwt_plot_panner.h"
#include "qwt_scale_widget.h"
#include "qwt_scale_draw.h"
#include "qwt_color_map.h"

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_editor.h"
#include "us_model_loader.h"
#include "us_plot.h"
#include "us_colorgradIO.h"
#include "us_spectrodata.h"

typedef struct distro_sys
{
   QList< Solute > sk_distro;
   QList< Solute > wk_distro;
   QList< Solute > sv_distro;
   QList< Solute > wv_distro;
   QList< Solute > sk_distro_zp;
   QList< Solute > wk_distro_zp;
   QList< Solute > sv_distro_zp;
   QList< Solute > wv_distro_zp;
   QwtLinearColorMap* colormap;
   QString         run_name;
   QString         analys_name;
   QString         method;
   QString         cmapname;
   QString         editGUID;
   int             distro_type;
   bool            monte_carlo;
} DisSys;

bool distro_lessthan( const Solute&, const Solute& );

class US_Pseudo3D_Combine : public US_Widgets
{
   Q_OBJECT

   public:
      US_Pseudo3D_Combine();

   private:
      QLabel*       lb_plt_smin;
      QLabel*       lb_plt_smax;
      QLabel*       lb_plt_fmin;
      QLabel*       lb_plt_fmax;

      QTextEdit*    te_distr_info;

      QLineEdit*    le_cmap_name;
      QLineEdit*    le_prefilt;

      US_Help       showHelp;
 
      QwtCounter*   ct_resolu;
      QwtCounter*   ct_xreso;
      QwtCounter*   ct_yreso;
      QwtCounter*   ct_zfloor;
      QwtCounter*   ct_plt_fmin;     
      QwtCounter*   ct_plt_fmax;     
      QwtCounter*   ct_plt_smin;     
      QwtCounter*   ct_plt_smax;     
      QwtCounter*   ct_plt_dlay;     
      QwtCounter*   ct_curr_distr;

      QwtPlot*      data_plot;

      QwtPlotPicker* pick;

      QwtLinearColorMap* colormap;

      US_Disk_DB_Controls* dkdb_cntrls;

      QPushButton*  pb_pltall;
      QPushButton*  pb_stopplt;
      QPushButton*  pb_refresh;
      QPushButton*  pb_reset;
      QPushButton*  pb_prefilt;
      QPushButton*  pb_lddistr;
      QPushButton*  pb_ldcolor;
      QPushButton*  pb_help;
      QPushButton*  pb_close;
      QPushButton*  pb_rmvdist;

      QCheckBox*    cb_autosxy;
      QCheckBox*    cb_autoscz;
      QCheckBox*    cb_conloop;
      QCheckBox*    cb_plot_sk;
      QCheckBox*    cb_plot_wk;
      QCheckBox*    cb_plot_sv;
      QCheckBox*    cb_plot_wv;
      QCheckBox*    cb_zpcent;

      QList< DisSys > system;

      double        resolu;
      double        plt_smin;
      double        plt_smax;
      double        plt_fmin;
      double        plt_fmax;
      double        plt_zmin;
      double        plt_zmax;
      double        plt_zmin_zp;
      double        plt_zmax_zp;
      double        plt_zmin_co;
      double        plt_zmax_co;
      double        k_range;
      double        s_range;
      double        xreso;
      double        yreso;
      double        zfloor;

      int           curr_distr;
      int           init_solutes;
      int           mc_iters;
      int           patm_id;
      int           patm_dlay;
      int           dbg_level;
      int           plot_xy;

      bool          cnst_vbar;
      bool          auto_sxy;
      bool          auto_scz;
      bool          cont_loop;
      bool          plot_s;
      bool          plot_k;
      bool          looping;
      bool          need_save;
      bool          runsel;
      bool          latest;
      bool          zpcent;

      QString       xa_title_s;
      QString       xa_title_w;
      QString       xa_title;
      QString       ya_title_k;
      QString       ya_title_v;
      QString       ya_title;
      QString       cmapname;
      QString       mfilter;

      QStringList   pfilts;

   private slots:

      void update_resolu(     double );
      void update_xreso(      double );
      void update_yreso(      double );
      void update_zfloor(     double );
      void update_curr_distr( double );
      void update_plot_fmin(  double );
      void update_plot_fmax(  double );
      void update_plot_smin(  double );
      void update_plot_smax(  double );
      void plot_data(      int );
      void plot_data(      void );
      void select_autosxy( void );
      void select_autoscz( void );
      void select_conloop( void );
      void select_plot_sk( void );
      void select_plot_wk( void );
      void select_plot_sv( void );
      void select_plot_wv( void );
      void update_disk_db( bool );
      void select_prefilt( void );
      void load_distro(    void );
      void load_distro(    US_Model, QString );
      void load_color(     void );
      void plotall(     void );
      void stop(        void );
      void reset(       void );
      void set_limits(  void );
      void sort_distro( QList< Solute >&, bool );
      void remove_distro( void );

      void help       ( void )
      { showHelp.show_help( "pseudo3d_combine.html" ); };

   protected:
      virtual void timerEvent( QTimerEvent *e );
};
#endif
