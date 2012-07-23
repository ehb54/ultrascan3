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

		int		grid_index;   	// number of total partialGrids, used as the index to identify partialGrids,
										// starts at 1 and aligns with partialGrid
		int		partialGrid;  	// currently active partialGrid
		int		subGrids;		// number of subgrids
      QLabel	*lbl_info1;
      QLabel	*lbl_info2;
      QLabel	*lbl_xaxis;
      QLabel	*lbl_yaxis;
      QLabel	*lbl_xRes;
      QLabel	*lbl_yRes;
      QLabel	*lbl_xMin;
      QLabel	*lbl_xMax;
      QLabel	*lbl_yMin;
      QLabel	*lbl_yMax;
      QLabel	*lbl_zVal;
      QLabel	*lbl_density;
      QLabel	*lbl_viscosity;
      QLabel	*lbl_partialGrid;
      QLabel	*lbl_subGrid;

      QLineEdit    *le_density;
      QLineEdit    *le_viscosity;
      QLineEdit    *le_investigator;

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

		QCheckBox *cb_show_final_grid;
		QCheckBox *cb_show_sub_grid;

      QRadioButton *rb_x_s;
      QRadioButton *rb_x_mw;
      QRadioButton *rb_y_ff0;
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
		int plot_x; // 0 = s, 1 = MW
		int plot_y; // 0 = ff0, 1 = vbar
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
      void update_subGrids( double );
      void update_density( const QString & );
      void update_viscosity( const QString & );
      void update_plot( void );
      void select_x_axis( int );
      void select_y_axis( int );
      void select_plot( int );
      void delete_partialGrid( void );
      void add_partialGrid( void );
      void save( void );
      void reset( void );
      void help( void ) { showHelp.show_help( "grid_editor.html" ); };
		void calc_gridpoints( void );
		void set_minmax( const struct gridpoint &);
		void show_final_grid( bool );
		void show_sub_grid( bool );
      void update_disk_db(  bool );
		void sel_investigator( void );


};

#endif
