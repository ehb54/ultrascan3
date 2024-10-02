//! \file us_pseudo3d_combine.h
#ifndef US_PSE3D_COMBINE_H
#define US_PSE3D_COMBINE_H

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_editor.h"
#include "us_model_loader.h"
#include "us_plot.h"
#include "us_colorgradIO.h"
#include "us_spectrodata.h"

#include "qwt_plot_marker.h"
#include "qwt_plot_spectrogram.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_zoomer.h"
#include "qwt_plot_panner.h"
#include "qwt_scale_widget.h"
#include "qwt_scale_draw.h"
#include "qwt_color_map.h"

//!< \brief Distribution structure
typedef struct distro_sys
{
   QList< S_Solute >   sk_distro;
   QList< S_Solute >   xy_distro;
   QList< S_Solute >   sk_distro_zp;
   QList< S_Solute >   xy_distro_zp;
   QwtLinearColorMap*  colormap;
   QString             run_name;
   QString             analys_name;
   QString             method;
   QString             cmapname;
   QString             editGUID;
   int                 distro_type;
   int                 plot_x;
   int                 plot_y;
   bool                monte_carlo;
} DisSys;

//! \brief Less-than function for sorting distributions
bool distro_lessthan( const S_Solute&, const S_Solute& );

//! Class for displaying models in pseudo-3D
class US_Pseudo3D_Combine : public US_Widgets
{
   Q_OBJECT

   public:
      //! \brief Pseudo-3D Combination constructor
      US_Pseudo3D_Combine();

      void load_distro_auto( QString, QStringList );
      QwtPlot* rp_data_plot();
      void reset_auto(       void );
      void select_x_axis_auto( int  );
      void select_y_axis_auto( int  );

   private:

      enum attr_type { ATTR_S, ATTR_K, ATTR_W, ATTR_V, ATTR_D, ATTR_F };

      QLabel*       lb_plt_smin;
      QLabel*       lb_plt_smax;
      QLabel*       lb_plt_kmin;
      QLabel*       lb_plt_kmax;

      QTextEdit*    te_distr_info;

      QLineEdit*    le_cmap_name;
      QLineEdit*    le_prefilt;

      US_Help       showHelp;
 
      QwtCounter*   ct_resolu;
      QwtCounter*   ct_xreso;
      QwtCounter*   ct_yreso;
      QwtCounter*   ct_zfloor;
      QwtCounter*   ct_plt_kmin;     
      QwtCounter*   ct_plt_kmax;     
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

      QCheckBox*    ck_autosxy;
      QCheckBox*    ck_autoscz;
      QCheckBox*    ck_conloop;
      QCheckBox*    ck_plot_sk;
      QCheckBox*    ck_plot_wk;
      QCheckBox*    ck_plot_sv;
      QCheckBox*    ck_plot_wv;
      QCheckBox*    ck_zpcent;
      QCheckBox*    ck_savepl;
      QCheckBox*    ck_locsave;

      QRadioButton* rb_x_s;
      QRadioButton* rb_x_ff0;
      QRadioButton* rb_x_mw;
      QRadioButton* rb_x_vbar;
      QRadioButton* rb_x_D;
      QRadioButton* rb_x_f;
      QRadioButton* rb_y_s;
      QRadioButton* rb_y_ff0;
      QRadioButton* rb_y_mw;
      QRadioButton* rb_y_vbar;
      QRadioButton* rb_y_D;
      QRadioButton* rb_y_f;

      QButtonGroup* bg_x_axis;
      QButtonGroup* bg_y_axis;

      QList< DisSys > system;

      double        resolu;
      double        plt_smin;
      double        plt_smax;
      double        plt_kmin;
      double        plt_kmax;
      double        plt_zmin;
      double        plt_zmax;
      double        plt_zmin_zp;
      double        plt_zmax_zp;
      double        plt_zmin_co;
      double        plt_zmax_co;
      double        s_range;
      double        k_range;
      double        xreso;
      double        yreso;
      double        zfloor;

      int           curr_distr;
      int           init_solutes;
      int           mc_iters;
      int           patm_id;
      int           patm_dlay;
      int           dbg_level;
      int           plot_x;
      int           plot_y;

      bool          cnst_vbar;
      bool          auto_sxy;
      bool          auto_scz;
      bool          cont_loop;
      bool          looping;
      bool          need_save;
      bool          runsel;
      bool          latest;
      bool          zpcent;

      QString       xa_title;
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
      void update_plot_smin(  double );
      void update_plot_smax(  double );
      void update_plot_kmin(  double );
      void update_plot_kmax(  double );
      void plot_data(      int );
      void plot_data(      void );
      void select_autosxy( void );
      void select_autoscz( void );
      void select_conloop( void );
      void update_disk_db( bool );
      void select_prefilt( void );

      void load_distro(    void );
      
      
      void load_distro(    US_Model, QString );
      void load_color(     void );
      void plotall(     void );
      void stop(        void );
      void reset(       void );
      void set_limits(  void );
      void sort_distro( QList< S_Solute >&, bool );
      void remove_distro( void );
      void select_x_axis( int  );
      void select_y_axis( int  );
      void build_xy_distro( void );
      QString anno_title  ( int );
      QwtLinearColorMap* ColorMapCopy( QwtLinearColorMap* );

      void help       ( void )
      { showHelp.show_help( "pseudo3d_combine.html" ); };

   protected:
      virtual void timerEvent( QTimerEvent *e );
};
#endif
