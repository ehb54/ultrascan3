#ifndef US_GRID_EDITOR_H
#define US_GRID_EDITOR_H

#include <QtGui>

#include "qwt_plot_zoomer.h"
#include "qwt_plot_panner.h"
#include "qwt_color_map.h"

#include "us_constants.h"
#include "us_editor.h"
#include "us_extern.h"
#include "us_gui_settings.h"
#include "us_help.h"
#include "us_license.h"
#include "us_license_t.h"
#include "us_math2.h"
#include "us_matrix.h"
#include "us_model_loader.h"
#include "us_passwd.h"
#include "us_plot.h"
#include "us_select_edits.h"
#include "us_settings.h"
#include "us_util.h"
#include "us_widgets.h"
#include "us_investigator.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

struct gridpoint
{
	double s;
	double D;
	double vbar;
   double mw;
   double f;
	double ff0;
	double f0;
	int index;
};

class US_Grid_Editor : public US_Widgets
{
   Q_OBJECT

   public:
      US_Grid_Editor();

   private:

      enum attr_type { ATTR_S, ATTR_K, ATTR_W, ATTR_V, ATTR_D, ATTR_F };

		int		grid_index;   	// number of total partialGrids, used as the index to identify partialGrids,
										// starts at 1 and aligns with partialGrid
		int		partialGrid;  	// currently active partialGrid
		int		subGrids;		// number of subgrids
      QLabel	*lb_info1;
      QLabel	*lb_info2;
      QLabel	*lb_xaxis;
      QLabel	*lb_yaxis;
      QLabel	*lb_fixed;
      QLabel	*lb_xRes;
      QLabel	*lb_yRes;
      QLabel	*lb_xMin;
      QLabel	*lb_xMax;
      QLabel	*lb_yMin;
      QLabel	*lb_yMax;
      QLabel	*lb_zVal;
      QLabel	*lb_density;
      QLabel	*lb_viscosity;
      QLabel	*lb_partialGrid;
      QLabel	*lb_subGrid;

      QLineEdit    *le_density;
      QLineEdit    *le_viscosity;
      QLineEdit    *le_investigator;

      QComboBox    *cb_fixed;

      US_Help       showHelp;
		QList <gridpoint> current_grid;
		QList <gridpoint> final_grid;

		gridpoint maxgridpoint;
		gridpoint mingridpoint;

      QwtCounter   *ct_xRes;
      QwtCounter   *ct_yRes;
      QwtCounter   *ct_yMin;
      QwtCounter   *ct_yMax;
      QwtCounter   *ct_xMin;
      QwtCounter   *ct_xMax;
      QwtCounter   *ct_zVal;
      QwtCounter   *ct_partialGrid;
      QwtCounter   *ct_subGrids;

      QwtPlot            *data_plot1;
      QwtLinearColorMap  *colormap;
      US_PlotPicker      *pick1;
      US_PlotPicker      *pick2;
      US_Disk_DB_Controls* dkdb_cntrls;


      QPushButton  *pb_add_partialGrid;
      QPushButton  *pb_delete_partialGrid;
      QPushButton  *pb_help;
      QPushButton  *pb_close;
      QPushButton  *pb_save;
      QPushButton  *pb_reset;
      QPushButton  *pb_investigator;

		QCheckBox *ck_show_final_grid;
		QCheckBox *ck_show_sub_grid;

      QRadioButton *rb_x_s;
      QRadioButton *rb_x_mw;
      QRadioButton *rb_x_ff0;
      QRadioButton *rb_x_D;
      QRadioButton *rb_x_f; 
      QRadioButton *rb_x_vbar;
      QRadioButton *rb_y_s;
      QRadioButton *rb_y_mw;
      QRadioButton *rb_y_ff0;
      QRadioButton *rb_y_D;
      QRadioButton *rb_y_f; 
      QRadioButton *rb_y_vbar;
      QRadioButton *rb_plot1;
      QRadioButton *rb_plot2;

		QButtonGroup *bg_x_axis;
		QButtonGroup *bg_y_axis;
		QButtonGroup *toggle_plot;

      double        xMin;
      double        xMax;
      double        yMin;
      double        yMax;
      double        xRes;
      double        yRes;
		double		  zVal;
		double		  viscosity;
		double		  density;
		double		  vbar;
		double		  ff0;

      int dbg_level;
		int plot_x; // 0-5 for s,f/f0,mw,vbar,D,f
		int plot_y; // 0-5 for s,f/f0,mw,vbar,D,f
		int plot_z; // 0-5 for s,f/f0,mw,vbar,D,f
		int selected_plot;

   private slots:

      void update_xRes( double );
      void update_yRes( double );
      void update_xMin( double );
      void update_xMax( double );
      void update_yMin( double );
      void update_yMax( double );
      void update_zVal( double );
      void update_partialGrid( double );
      void update_subGrids   ( double );
      void update_density  ( const QString & );
      void update_viscosity( const QString & );
      void update_plot  ( void );
      void select_x_axis( int );
      void select_y_axis( int );
      void select_fixed ( const QString & );
      void select_plot  ( int );
      void delete_partialGrid( void );
      void add_partialGrid   ( void );
      void save ( void );
      void reset( void );
      void help ( void ) { showHelp.show_help( "grid_editor.html" ); };
		void calc_gridpoints  ( void );
		void calc_gridpoints_2( void );
		void set_minmax( const struct gridpoint &);
		void show_final_grid( bool );
		void show_sub_grid( bool );
      void update_disk_db(  bool );
		void sel_investigator( void );
		void print_minmax( void );
		double grid_value( struct gridpoint&, int );
      bool set_comp_skw( struct gridpoint& );
      bool set_comp_skv( struct gridpoint& );
      bool set_comp_skd( struct gridpoint& );
      bool set_comp_skf( struct gridpoint& );
      bool set_comp_swv( struct gridpoint& );
      bool set_comp_swd( struct gridpoint& );
      bool set_comp_swf( struct gridpoint& );
      bool set_comp_svd( struct gridpoint& );
      bool set_comp_svf( struct gridpoint& );
      bool set_comp_sdf( struct gridpoint& );
      bool set_comp_kwv( struct gridpoint& );
      bool set_comp_kwd( struct gridpoint& );
      bool set_comp_kwf( struct gridpoint& );
      bool set_comp_kvd( struct gridpoint& );
      bool set_comp_kvf( struct gridpoint& );
      bool set_comp_kdf( struct gridpoint& );
      bool set_comp_wvd( struct gridpoint& );
      bool set_comp_wvf( struct gridpoint& );
      bool set_comp_wdf( struct gridpoint& );
      bool set_comp_vdf( struct gridpoint& );
      bool check_grid_point( double, struct gridpoint& );
      bool validate_ff0    ( void );
      void clear_grid      ( struct gridpoint& );
      void set_grid_value  ( struct gridpoint&, const int, const double );
      bool complete_comp   ( struct gridpoint& );
};

#endif
