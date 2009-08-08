#ifndef US_EDVABS_H
#define US_EDVABS_H

#include <QtGui>

#include "qwt_plot_marker.h"

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_plot.h"
#include "us_dataIO.h"

class US_EXTERN US_Edvabs : public US_Widgets
{
	Q_OBJECT

	public:
		US_Edvabs();

	private:

      enum { MENISCUS, RANGE, PLATEAU, BASELINE, FINISHED } step;

      rawData          data;
      double           meniscus;
      double           meniscus_left;
      double           range_left;
      double           range_right;
      QList< double >  plateau;
      double           baseline;
      QList< int >     includes;

      US_Help          showHelp;
      
      QString          workingDir;
      QString          runID;
      QStringList      files;
      QStringList      tripples;
                      
      QwtPlot*         data_plot;
      QwtPlotCurve*    raw_curve;
      QwtPlotCurve*    fit_curve;
      QwtPlotCurve*    v_line;
      QwtPlotCurve*    minimum_curve;
      QwtPlotGrid*     grid;
      QwtPlotMarker*   marker;
      US_PlotPicker*   pick;
                      
      QLineEdit*       le_info;
      QLineEdit*       le_meniscus;
      QLineEdit*       le_dataRange;
      QLineEdit*       le_plateau;
      QLineEdit*       le_baseline;
                    
      QPushButton*     pb_details;
      QPushButton*     pb_exclude;
      QPushButton*     pb_excludeRange;
      QPushButton*     pb_exclusion;
      QPushButton*     pb_include;
      QPushButton*     pb_edit1;
      QPushButton*     pb_meniscus;
      QPushButton*     pb_dataRange;
      QPushButton*     pb_plateau;
      QPushButton*     pb_baseline;
      QPushButton*     pb_noise;
      QPushButton*     pb_subtract;
      QPushButton*     pb_spikes;
      QPushButton*     pb_invert;
      QPushButton*     pb_write;
                    
      QComboBox*       cb_tripple;
      QComboBox*       cb_cell;
      QComboBox*       cb_channel;
      QComboBox*       cb_wavelength;
                    
      QwtCounter*      ct_from;
      QwtCounter*      ct_to;
      QwtCounter*      ct_noise;
	
      void set_pbColors   ( QPushButton* );
      void draw_vline     ( double );
      int  index          ( scan*, double );
      void next_step      ( void );
      void replot         ( void );
      void plot_current   ( void );
      void plot_all       ( void );
      void plot_range     ( void );
      void plot_last      ( void );
      void init_includes  ( void );
      void reset_excludes ( void );

	private slots:

      void load         ( void );
      void mouse        ( const QwtDoublePoint& );
      void set_meniscus ( void );
      void set_dataRange( void );
      void set_plateau  ( void );
      void set_baseline ( void );
      void focus_from   ( double );
      void focus_to     ( double );
      void focus        ( int, int );
      void exclude_one  ( void );
      void exclude_range( void );
      void exclusion    ( void );
      void edit_scan    ( void );
      void include      ( void );

      void reset        ( void );
		void help         ( void )
      { showHelp.show_help( "manual/edit_velocity.html" ); };
};
#endif
