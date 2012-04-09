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
   QList< Solute > s_distro;
   QList< Solute > mw_distro;
   QwtLinearColorMap* colormap;
   QString         run_name;
   QString         analys_name;
   QString         method;
   QString         cmapname;
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

      QCheckBox*    cb_autolim;
      QCheckBox*    cb_conloop;
      QCheckBox*    cb_plot_s;
      QCheckBox*    cb_plot_mw;

      QList< DisSys > system;

      double        resolu;
      double        plt_smin;
      double        plt_smax;
      double        plt_fmin;
      double        plt_fmax;
      double        plt_zmin;
      double        plt_zmax;
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

      bool          cnst_vbar;
      bool          auto_lim;
      bool          cont_loop;
      bool          plot_s;
      bool          looping;
      bool          need_save;
      bool          runsel;
      bool          latest;

      QString       xa_title_s;
      QString       xa_title_mw;
      QString       xa_title;
      QString       ya_title_ff;
      QString       ya_title_vb;
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
      void select_autolim( void );
      void select_conloop( void );
      void select_plot_s(  void );
      void select_plot_mw( void );
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

      void help       ( void )
      { showHelp.show_help( "pseudo3d_combine.html" ); };

   protected:
      virtual void timerEvent( QTimerEvent *e );
};
#endif
