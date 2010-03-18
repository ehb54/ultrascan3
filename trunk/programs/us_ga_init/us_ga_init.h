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

typedef struct distro_sys
{
   QList< Solute > s_distro;
   QList< Solute > mw_distro;
   QwtLinearColorMap* colormap;
   QString         run_name;
   QString         cell;
   QString         wavelength;
   QString         method;
   QString         cmapname;
   int             distro_type;
   bool            monte_carlo;
} DisSys;

bool distro_lessthan( const Solute&, const Solute& );

class US_EXTERN US_GA_Init : public US_Widgets
{
   Q_OBJECT

   public:
      US_GA_Init();

   private:

      QLabel*       lb_info1;
      QLabel*       lb_nisols;
      QLabel*       lb_dafmin;
      QLabel*       lb_dafmax;
      QLabel*       lb_wsbuck;
      QLabel*       lb_wfbuck;
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

      QTextEdit*    te_sbin_data;
      QTextEdit*    te_pctl_help;

      US_Help       showHelp;
 
      QwtCounter*   ct_nisols;
      QwtCounter*   ct_dafmin;
      QwtCounter*   ct_dafmax;
      QwtCounter*   ct_wsbuck;
      QwtCounter*   ct_wfbuck;
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
      QwtLinearColorMap*  colormap;
      US_PlotPicker*      pick;

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

      QList< Solute > s_distro;
      QList< Solute > mw_distro;

      QwtDoublePoint  p1;
      QwtDoublePoint  p2;

      double        dafmin;
      double        dafmax;
      double        wsbuck;
      double        wfbuck;
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
      int           init_solutes;
      int           mc_iters;
      int           patm_id;
      int           patm_dlay;
      int           plot_dim;
      int           distro_type;

      bool          minmax;
      bool          zoom;
      bool          auto_lim;
      bool          monte_carlo;

      QString       run_name;
      QString       cell;
      QString       wavelength;
      QString       method;
      QString       xa_title_s;
      QString       xa_title_mw;
      QString       xa_title;
      QString       cmapname;

   private slots:

      void update_nisols( double );
      void update_dafmin( double );
      void update_dafmax( double );
      void update_wsbuck( double );
      void update_wfbuck( double );
      void update_resolu( double );
      void update_xreso(  double );
      void update_yreso(  double );
      void update_zfloor( double );
      void update_plfmin( double );
      void update_plfmax( double );
      void update_plsmin( double );
      void update_plsmax( double );
      void plot_1dim( void );
      void plot_2dim( void );
      void plot_3dim( void );
      void select_autolim( void );
      void select_plot1d(  void );
      void select_plot2d(  void );
      void select_plot3d(  void );
      void load_distro( void );
      void load_distro( const QString& );
      void load_color(  void );
      void mandrawsb(   void );
      void shrinksb(    void );
      void autassignsb( void );
      void resetsb(     void );
      void save(        void );
      void reset(       void );
      void set_limits(  void );
      void sort_distro( QList< Solute >&, bool );
      void highlight_solute( int );
      void getMouseDown( const QwtDoublePoint& );
      void getMouseUp(   const QwtDoublePoint& );

      void help     ( void )
      { showHelp.show_help( "ga_initialize.html" ); };
};
#endif
