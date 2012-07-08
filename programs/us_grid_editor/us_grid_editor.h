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
#include "us_widgets.h"

struct gridpoint { double s, D, vbar, mw, f, ff0, f0; };

class US_Grid_Editor : public US_Widgets
{
   Q_OBJECT

   public:
      US_Grid_Editor();

   private:

      QLabel*       lbl_info1;
      QLabel*       lbl_info2;
      QLabel*       lbl_xaxis;
      QLabel*       lbl_yaxis;
      QLabel*       lbl_xRes;
      QLabel*       lbl_yRes;
      QLabel*       lbl_xMin;
      QLabel*       lbl_xMax;
      QLabel*       lbl_yMin;
      QLabel*       lbl_yMax;
      QLabel*       lbl_zVal;
      QLabel*       lbl_density;
      QLabel*       lbl_viscosity;

      QLineEdit*    le_density;
      QLineEdit*    le_viscosity;

      US_Help       showHelp;
		QList <gridpoint> grid;
		QVector <double> xData;
		QVector <double> yData;
		gridpoint maxgridpoint;
		gridpoint mingridpoint;
 
      QwtCounter*   ct_xRes;
      QwtCounter*   ct_yRes;
      QwtCounter*   ct_yMin;     
      QwtCounter*   ct_yMax;     
      QwtCounter*   ct_xMin;     
      QwtCounter*   ct_xMax;     
      QwtCounter*   ct_zVal;     

      QwtPlot*            data_plot;
      QwtPlotCurve*       pc1;
      QwtLinearColorMap*  colormap;
      US_PlotPicker*      pick;
      US_Disk_DB_Controls* dkdb_cntrls;

      QPushButton*  pb_help;
      QPushButton*  pb_close;
      QPushButton*  pb_save;
      QPushButton*  pb_reset;

      QRadioButton* rb_x_s;
      QRadioButton* rb_x_mw;
      QRadioButton* rb_y_ff0;
      QRadioButton* rb_y_vbar;
		QButtonGroup* bg_x_axis;
		QButtonGroup* bg_y_axis;

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
		int gridsize;			

   private slots:

      void update_xRes( double );
      void update_yRes( double );
      void update_xMin( double );
      void update_xMax( double );
      void update_yMin( double );
      void update_yMax( double );
      void update_zVal( double );
      void update_density( const QString & );
      void update_viscosity( const QString & );
      void update_plot( void );
      void select_x_axis( int );
      void select_y_axis( int );
      void save( void );
      void reset( void );
      void help( void ) { showHelp.show_help( "create_grid.html" ); };
		void calc_gridpoints( void );
		void set_minmax( const struct gridpoint &);

};

#endif
