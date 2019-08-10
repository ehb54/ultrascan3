#ifndef US_DENS_MATCH_H
#define US_DENS_MATCH_H

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
   QList< S_Solute >   in_distro;      // Raw input distribution
   QList< S_Solute >   nm_distro;      // Normalized concentration distro
   QList< S_Solute >   bf_distro;      // Boundary fractions distro
   QString             run_name;
   QString             analys_name;
   QString             method;
   QString             editGUID;
   QString             solutionGUID;
   QString             label;
   int                 distro_type;
   int                 plot_x;
   int                 solutionID;
   double              d2opct;
} DisSys;

//! \brief Less-than function for sorting distributions
bool distro_lessthan( const S_Solute&, const S_Solute& );

//! Class for displaying models in pseudo-3D
class US_Density_Match : public US_Widgets
{
   Q_OBJECT

   public:
      //! \brief Pseudo-3D Combination constructor
      US_Density_Match();

   private:

      enum attr_type { ATTR_S, ATTR_K, ATTR_W, ATTR_V, ATTR_D, ATTR_F, ATTR_R };

      QLabel*       lb_plt_smin;
      QLabel*       lb_plt_smax;
      QLabel*       lb_plt_kmin;
      QLabel*       lb_plt_kmax;
      QLabel*       lb_tolerance;
      QLabel*       lb_division;

      QTextEdit*    te_distr_info;

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
      QwtCounter*   ct_tolerance;
      QwtCounter*   ct_division;
      QwtCounter*   ct_smoothing;
      QwtCounter*   ct_boundaryPct;
      QwtCounter*   ct_boundaryPos;

      QwtPlot*      data_plot;

      QwtPlotPicker* pick;

      US_Disk_DB_Controls* dkdb_cntrls;

      QPushButton*  pb_pltall;
      QPushButton*  pb_stopplt;
      QPushButton*  pb_refresh;
      QPushButton*  pb_reset;
      QPushButton*  pb_prefilt;
      QPushButton*  pb_lddistr;
      QPushButton*  pb_help;
      QPushButton*  pb_close;
      QPushButton*  pb_rmvdist;

      QCheckBox*    ck_autosxy;
      QCheckBox*    ck_autoscz;
      QCheckBox*    ck_plot_sk;
      QCheckBox*    ck_plot_wk;
      QCheckBox*    ck_plot_sv;
      QCheckBox*    ck_plot_wv;
      QCheckBox*    ck_savepl;
      QCheckBox*    ck_locsave;

      QRadioButton* rb_x_mass;
      QRadioButton* rb_x_ff0;
      QRadioButton* rb_x_rh;
      QRadioButton* rb_x_vbar;
      QRadioButton* rb_x_s;

      QButtonGroup* bg_x_axis;

      QList< DisSys > alldis;

      double        resolu;
      double        plt_smin;
      double        plt_smax;
      double        plt_kmin;
      double        plt_kmax;
      double        plt_zmin;
      double        plt_zmax;
      double        s_range;
      double        k_range;
      double        xreso;
      double        yreso;
      double        zfloor;

      int           curr_distr;
      int           patm_id;
      int           patm_dlay;
      int           dbg_level;
      int           plot_x;

      bool          auto_sxy;
      bool          auto_scz;
      bool          cont_loop;
      bool          need_save;
      bool          runsel;
      bool          latest;

      QString       xa_title;
      QString       ya_title;
      QString       mfilter;

      QStringList   pfilts;

   private slots:

      void update_resolu(     double );
      void update_xreso(      double );
      void update_yreso(      double );
      void update_curr_distr( double );
      void update_plot_smin(  double );
      void update_plot_smax(  double );
      void update_plot_kmin(  double );
      void update_plot_kmax(  double );
      void plot_data(      int );
      void plot_data(      void );
      void select_autosxy( void );
      void select_autoscz( void );
      void update_disk_db( bool );
      void update_divis  ( double );
      void select_prefilt( void );
      void load_distro(    void );
      void load_distro(    US_Model, QString );
      void plotall(     void );
      void reset(       void );
      void set_limits(  void );
      void sort_distro( QList< S_Solute >&, bool );
      void remove_distro( void );
      void select_x_axis( int  );
      void build_bf_distro( void );
      QString anno_title  ( int );

      void help       ( void )
      { showHelp.show_help( "pseudo3d_combine.html" ); };
};
#endif
