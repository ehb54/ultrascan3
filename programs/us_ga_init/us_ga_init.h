//! \file us_ga_init.h
#ifndef US_GA_INIT_H
#define US_GA_INIT_H

#include "us_extern.h"
#include "us_widgets.h"
#include "us_editor.h"
#include "us_help.h"
#include "us_plot.h"
#include "us_colorgradIO.h"
#include "us_spectrodata.h"
#include "us_solutedata.h"

#include "qwt_plot_marker.h"
#include "qwt_plot_spectrogram.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_zoomer.h"
#include "qwt_plot_panner.h"
#include "qwt_scale_widget.h"
#include "qwt_scale_draw.h"
#include "qwt_color_map.h"

//! \brief Less-than function for sorting S_Solute values
bool distro_lessthan( const S_Solute&, const S_Solute& );

//! Class for displaying model distributions and selecting buckets
//! for use in Genetic Algorithm analysis
class US_GA_Initialize : public US_Widgets
{
   Q_OBJECT

   public:
      //! Genetic Algorithm Initialize constructor
      US_GA_Initialize();

      enum attr_type { ATTR_S, ATTR_K, ATTR_W, ATTR_V, ATTR_D, ATTR_F };

   private:

      QLabel*       lb_info1;
      QLabel*       lb_nisols;
      QLabel*       lb_wxbuck;
      QLabel*       lb_hybuck;
      QLabel*       lb_info2;
      QLabel*       lb_resolu;
      QLabel*       lb_xreso;
      QLabel*       lb_yreso;
      QLabel*       lb_zfloor;
      QLabel*       lb_autlim;
      QLabel*       lb_plxmin;
      QLabel*       lb_plxmax;
      QLabel*       lb_plymin;
      QLabel*       lb_plymax;

      QListWidget*  lw_sbin_data;
      QTextEdit*    te_pctl_help;
      QTextEdit*    te_status;
      QLineEdit*    le_prefilt;

      US_Help       showHelp;
 
      QwtCounter*   ct_nisols;
      QwtCounter*   ct_wxbuck;
      QwtCounter*   ct_hybuck;
      QwtCounter*   ct_resolu;
      QwtCounter*   ct_xreso;
      QwtCounter*   ct_yreso;
      QwtCounter*   ct_zfloor;
      QwtCounter*   ct_plxmin;     
      QwtCounter*   ct_plxmax;     
      QwtCounter*   ct_plymin;     
      QwtCounter*   ct_plymax;     

      QwtPlot*             data_plot;
      QwtPlotSpectrogram*  d_spectrogram;
      US_SpectrogramData*  spec_dat;
      QwtPlotCurve*        pc1;
      QwtLinearColorMap*   colormap;
      US_PlotPicker*       pick;
      US_Disk_DB_Controls* dkdb_cntrls;

      US_SoluteData*       soludata;

      QPushButton*  pb_prefilt;
      QPushButton*  pb_lddistr;
      QPushButton*  pb_ldcolor;
      QPushButton*  pb_refresh;
      QPushButton*  pb_mandrsb;
      QPushButton*  pb_shrnksb;
      QPushButton*  pb_ckovrlp;
      QPushButton*  pb_autassb;
      QPushButton*  pb_resetsb;
      QPushButton*  pb_loadsb;
      QPushButton*  pb_help;
      QPushButton*  pb_close;
      QPushButton*  pb_save;
      QPushButton*  pb_reset;
      QPushButton*  pb_view;

      QCheckBox*    ck_autlim;
      QCheckBox*    ck_1dplot;
      QCheckBox*    ck_2dplot;
      QCheckBox*    ck_3dplot;

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

      QPen*         pickpen;
      QPen          cbukpen;
      QPen          pbukpen;

      QList< S_Solute >  sk_distro;
      QList< S_Solute >  xy_distro;
      QList< S_Solute >* sdistro;

      QwtDoublePoint  p1;
      QwtDoublePoint  p2;

      double        wxbuck;
      double        hybuck;
      double        resolu;
      double        plxmin;
      double        plxmax;
      double        plymin;
      double        plymax;
      double        k_range;
      double        s_range;
      double        xreso;
      double        yreso;
      double        zfloor;

      int           nisols;
      int           nibuks;
      int           mc_iters;
      int           patm_id;
      int           patm_dlay;
      int           plot_dim;
      int           distro_type;
      int           sxset;
      int           psdsiz;
      int           dbg_level;
      int           plot_xy;
      int           attr_x;
      int           attr_y;
      int           attr_z;

      bool          minmax;
      bool          zoom;
      bool          auto_lim;
      bool          monte_carlo;
      bool          runsel;
      bool          latest;
      bool          manbuks;
      bool          is_saved;

      QString       run_name;
      QString       analys_name;
      QString       method;
      QString       xa_title_s;
      QString       xa_title_w;
      QString       ya_title_k;
      QString       ya_title_v;
      QString       xa_title;
      QString       ya_title;
      QString       cmapname;
      QString       dfilname;
      QString       stcmline;
      QString       stdiline;
      QString       stdfline;
      QString       stnpline;
      QString       stfxline;
      QString       mfilter;
      QString       editGUID;
      QString       binfpath;

      QStringList   pfilts;

   private slots:

      void update_nisols( double );
      void update_wxbuck( double );
      void update_hybuck( double );
      void update_resolu( double );
      void update_xreso(  double );
      void update_yreso(  double );
      void update_zfloor( double );
      void update_plxmin( double );
      void update_plxmax( double );
      void update_plymin( double );
      void update_plymax( double );
      void replot_data( void );
      void plot_1dim(   void );
      void plot_2dim(   void );
      void plot_3dim(   void );
      void select_autolim( void );
      void select_plot1d(  void );
      void select_plot2d(  void );
      void select_plot3d(  void );
      void select_prefilt( void );
      void load_distro (   void );
      void load_color(     void );
      void manDrawSb(      void );
      void checkOverlaps(  void );
      void autoAssignSb(   void );
      void resetSb(        void );
      void save(           void );
      void reset(          void );
      void view(           void );
      void set_limits(     void );
      void sort_distro( QList< S_Solute >&, bool );
      void setBucketPens(  void );
      void highlight_solute( QwtPlotCurve* );
      void highlight_solute( int );
      void getMouseDown(     const QwtDoublePoint& );
      void getMouseUp(       const QwtDoublePoint& );
      void sclick_sbdata(    const QModelIndex& );
      void sclick_sbdata(    const int          );
      void dclick_sbdata(    const QModelIndex& );
      void newrow_sbdata(    int );
      void changeBucketRect( int, QRectF& );
      QwtPlotCurve* drawBucketRect( int, QRectF );
      QwtPlotCurve* drawBucketRect( int, QPointF, QPointF );
      QwtPlotCurve* bucketCurveAt(  int );
      void erase_buckets   ( bool );
      void erase_buckets   ( void );
      void removeSoluteBin ( int );
      void resetPlotAndList( int );
      bool equivalent      ( double, double, double );
      void update_disk_db  ( bool );
      void select_x_axis   ( int );
      void select_y_axis   ( int );
      void build_xy_distro ( void );
      void load_bins       ( void );
      QString anno_title   ( int  );
      QwtLinearColorMap* ColorMapCopy( QwtLinearColorMap* );

      void help     ( void )
      { showHelp.show_help( "ga_initialize.html" ); };
};
#endif
