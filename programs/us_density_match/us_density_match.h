//! \file us_density_match.h
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

      QLabel*       lb_division;

      QTextEdit*    te_distr_info;

      QLineEdit*    le_prefilt;

      US_Help       showHelp;
 
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

      QRadioButton* rb_x_mass;
      QRadioButton* rb_x_ff0;
      QRadioButton* rb_x_rh;
      QRadioButton* rb_x_vbar;
      QRadioButton* rb_x_s;
      QRadioButton* rb_x_d;
      QRadioButton* rb_da_n;
      QRadioButton* rb_da_s;
      QRadioButton* rb_da_w;

      QButtonGroup* bg_x_axis;
      QButtonGroup* bg_di_avg;

      QVector< DisSys >             alldis;    // All distributions

      QVector< double >             v_bfracs;  // Boundary fraction vector
      QVector< double >             v_vbars;   // Vector of vbars per fraction
      QVector< double >             v_mmass;   // Vector of molar masses per fraction
      QVector< double >             v_hrads;   // Vector of hydro radii per fraction
      QVector< double >             v_frats;   // Vector of frict ratios per fraction
      QVector< QVector< double > >  v_sedcs;   // Vector of sedi coeff vectors per distro
      QVector< QVector< double > >  v_difcs;   // Vector of diff coeff vectors per distro

      int           dbg_level;
      int           plot_x;
      int           diff_avg;

      QString       xa_title;
      QString       ya_title;
      QString       mfilter;

      QStringList   pfilts;
      QStringList   mdescs;

   private slots:

      void plot_data(      int );
      void plot_data(      void );
      void update_disk_db( bool );
      void update_divis  ( double );
      void select_prefilt( void );
      void load_distro(    void );
      void load_distro(    US_Model, QString );
      void reset(   void );
      void save(    void );
      void sort_distro    ( QList< S_Solute >&, bool );
      void remove_distro  ( void );
      void set_mparms     ( void );
      void select_x_axis  ( int  );
      void select_di_avg  ( int  );
      void build_bf_distro( int  );
      void build_bf_dists ( void );
      void build_bf_vects ( void );
      int  plot_x_select  ( void );
      QString anno_title  ( int );
      QString ptype_text  ( int );
      void models_summary ( void );
      void write_csv( const QString, const QString, QVector< double >&,
                                     const QString, QVector< double >& );

      void help       ( void )
      { showHelp.show_help( "dens_match.html" ); };
};
#endif
