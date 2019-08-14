#ifndef US_DENS_MATCH_H
#define US_DENS_MATCH_H

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_editor.h"
#include "us_model_loader.h"
#include "us_solute.h"
#include "us_spectrodata.h"
#include "us_plot.h"

#include "qwt_plot_marker.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_zoomer.h"
#include "qwt_plot_panner.h"
#include "qwt_scale_widget.h"
#include "qwt_scale_draw.h"

//!< \brief Distribution structure
typedef struct distro_sys
{
   QList< S_Solute >   in_distro;      // Raw input distribution
   QList< S_Solute >   nm_distro;      // Normalized concentration distro
   QList< S_Solute >   bo_distro;      // Boundary distro w/ orig points
   QList< S_Solute >   bf_distro;      // Boundary fractions distro
   QString             run_name;       // Distro run name
   QString             analys_name;    // Distro analysis name
   QString             method;         // Model method (e.g., "2DSA")
   QString             editGUID;       // Associated edit GUID
   QString             solutionGUID;   // Associated solution GUID
   QString             label;          // Distro label (channel description)
   int                 distro_type;    // Distro type flag
   int                 solutionID;     // Associated solution db ID
   double              d2opct;         // D2O percent for distro
   double              bdensity;       // Distro buffer density
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
      QwtCounter*   ct_plt_kmin;     
      QwtCounter*   ct_plt_kmax;     
      QwtCounter*   ct_plt_smin;     
      QwtCounter*   ct_plt_smax;     
      QwtCounter*   ct_plt_dlay;     
      QwtCounter*   ct_division;
      QwtCounter*   ct_smoothing;
      QwtCounter*   ct_boundaryPct;
      QwtCounter*   ct_boundaryPos;

      QwtPlot*      data_plot;

      QwtPlotPicker* pick;

      US_Disk_DB_Controls* dkdb_cntrls;

      QPushButton*  pb_refresh;
      QPushButton*  pb_reset;
      QPushButton*  pb_prefilt;
      QPushButton*  pb_lddistr;
      QPushButton*  pb_help;
      QPushButton*  pb_close;
      QPushButton*  pb_rmvdist;
      QPushButton*  pb_mdlpars;
      QPushButton*  pb_save;

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
      QRadioButton* rb_x_d;

      QButtonGroup* bg_x_axis;

      QVector< DisSys >             alldis;    // All distributions

      QVector< double >             v_bfracs;  // Boundary fraction vector
      QVector< double >             v_vbars;   // Vector of vbars per fraction
      QVector< double >             v_mmass;   // Vector of molar masses per fraction
      QVector< double >             v_hrads;   // Vector of hydro radii per fraction
      QVector< double >             v_frats;   // Vector of frict ratios per fraction
      QVector< QVector< double > >  v_sedcs;   // Vector of sedi coeff vectors per distro
      QVector< QVector< double > >  v_difcs;   // Vector of diff coeff vectors per distro

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

      int           curr_distr;
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
      QStringList   mdescs;

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
      void reset(   void );
      void save(    void );
      void set_limits     (  void );
      void sort_distro    ( QList< S_Solute >&, bool );
      void remove_distro  ( void );
      void set_mparms     ( void );
      void select_x_axis  ( int  );
      void build_bf_distro( int  );
      void build_bf_dists ( void );
      void build_bf_vects ( void );
      int  plot_x_select  ( void );
      QString anno_title  ( int );
      QString ptype_text  ( int );

      void help       ( void )
      { showHelp.show_help( "dens_match.html" ); };
};
#endif
