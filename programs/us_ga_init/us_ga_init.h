#ifndef US_GA_INIT_H
#define US_GA_INIT_H

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
#include "us_plot.h"
#include "us_colorgradIO.h"
#include "us_spectrodata.h"
#include "us_solutedata.h"

bool distro_lessthan( const Solute&, const Solute& );

class US_EXTERN US_GA_Initialize : public US_Widgets
{
   Q_OBJECT

   public:
      US_GA_Initialize();

   private:

      QLabel*       lb_info1;
      QLabel*       lb_nisols;
      QLabel*       lb_wsbuck;
      QLabel*       lb_hfbuck;
      QLabel*       lb_info2;
      QLabel*       lb_resolu;
      QLabel*       lb_xreso;
      QLabel*       lb_yreso;
      QLabel*       lb_zfloor;
      QLabel*       lb_autlim;
      QLabel*       lb_plsmin;
      QLabel*       lb_plsmax;
      QLabel*       lb_plfmin;
      QLabel*       lb_plfmax;

      QListWidget*  lw_sbin_data;
      QTextEdit*    te_pctl_help;
      QTextEdit*    te_status;

      US_Help       showHelp;
 
      QwtCounter*   ct_nisols;
      QwtCounter*   ct_wsbuck;
      QwtCounter*   ct_hfbuck;
      QwtCounter*   ct_resolu;
      QwtCounter*   ct_xreso;
      QwtCounter*   ct_yreso;
      QwtCounter*   ct_zfloor;
      QwtCounter*   ct_plfmin;     
      QwtCounter*   ct_plfmax;     
      QwtCounter*   ct_plsmin;     
      QwtCounter*   ct_plsmax;     

      QwtPlot*            data_plot;
      QwtPlotSpectrogram* d_spectrogram;
      QwtPlotCurve*       pc1;
      QwtLinearColorMap*  colormap;
      US_PlotPicker*      pick;

      US_SoluteData*      soludata;

      QPushButton*  pb_lddistr;
      QPushButton*  pb_ldcolor;
      QPushButton*  pb_refresh;
      QPushButton*  pb_mandrsb;
      QPushButton*  pb_shrnksb;
      QPushButton*  pb_autassb;
      QPushButton*  pb_resetsb;
      QPushButton*  pb_help;
      QPushButton*  pb_close;
      QPushButton*  pb_save;
      QPushButton*  pb_reset;

      QCheckBox*    cb_autlim;
      QCheckBox*    cb_1dplot;
      QCheckBox*    cb_2dplot;
      QCheckBox*    cb_3dplot;
      QCheckBox*    cb_plot_s;
      QCheckBox*    cb_plot_mw;

      QPen*         pickpen;
      QPen          cbukpen;
      QPen          pbukpen;

      QList< Solute >  s_distro;
      QList< Solute >  w_distro;
      QList< Solute >* sdistro;

      QwtDoublePoint  p1;
      QwtDoublePoint  p2;

      double        wsbuck;
      double        hfbuck;
      double        resolu;
      double        plsmin;
      double        plsmax;
      double        plfmin;
      double        plfmax;
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

      bool          minmax;
      bool          zoom;
      bool          auto_lim;
      bool          monte_carlo;
      bool          plot_s;
      bool          rbtn_click;

      QString       run_name;
      QString       cell;
      QString       wavelength;
      QString       method;
      QString       xa_title_s;
      QString       xa_title_mw;
      QString       xa_title;
      QString       cmapname;
      QString       dfilname;
      QString       stcmline;
      QString       stdiline;
      QString       stdfline;
      QString       stnpline;

   private slots:

      void update_nisols( double );
      void update_wsbuck( double );
      void update_hfbuck( double );
      void update_resolu( double );
      void update_xreso(  double );
      void update_yreso(  double );
      void update_zfloor( double );
      void update_plfmin( double );
      void update_plfmax( double );
      void update_plsmin( double );
      void update_plsmax( double );
      void replot_data( void );
      void plot_1dim(   void );
      void plot_2dim(   void );
      void plot_3dim(   void );
      void select_autolim( void );
      void select_plot1d(  void );
      void select_plot2d(  void );
      void select_plot3d(  void );
      void select_plot_s(  void );
      void select_plot_mw( void );
      void load_distro( void );
      void load_color(  void );
      void mandrawsb(   void );
      void shrinksb(    void );
      void autassignsb( void );
      void resetsb(     void );
      void save(        void );
      void reset(       void );
      void set_limits(  void );
      void sort_distro( QList< Solute >&, bool );
      void setBucketPens( void );
      void highlight_solute( QwtPlotCurve* );
      void highlight_solute( int );
      void getMouseDown( const QwtDoublePoint& );
      void getMouseUp(   const QwtDoublePoint& );
      void sclick_sbdata( const QModelIndex& );
      void dclick_sbdata( const QModelIndex& );
      void changeBucketRect( int, QRectF& );
      QwtPlotCurve* drawBucketRect( int, QRectF );
      QwtPlotCurve* drawBucketRect( int, QPointF, QPointF );
      QwtPlotCurve* bucketCurveAt( int );
      void erase_buckets( bool );
      void erase_buckets();
      void removeSoluteBin( int );

      void help     ( void )
      { showHelp.show_help( "ga_initialize.html" ); };

   protected:
      bool eventFilter( QObject*, QEvent* );
};
#endif
