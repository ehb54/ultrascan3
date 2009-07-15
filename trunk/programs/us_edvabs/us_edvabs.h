#ifndef US_EDVABS_H
#define US_EDVABS_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_plot.h"

class US_EXTERN US_Edvabs : public US_Widgets
{
	Q_OBJECT

	public:
		US_Edvabs();

	private:

      US_Help       showHelp;
      
      QwtPlot*      data_plot;
      QwtPlotCurve* raw_curve;
      QwtPlotCurve* fit_curve;
      QwtPlotCurve* minimum_curve;

      QLineEdit*    le_info;
      QLineEdit*    le_meniscus;
      QLineEdit*    le_dataRange;
      QLineEdit*    le_plateau;
      QLineEdit*    le_baseline;

      QPushButton*  pb_details;
      QPushButton*  pb_exclude;
      QPushButton*  pb_excludeRange;
      QPushButton*  pb_exclusion;
      QPushButton*  pb_edit1;
      QPushButton*  pb_meniscus;
      QPushButton*  pb_dataRange;
      QPushButton*  pb_plateau;
      QPushButton*  pb_baseline;
      QPushButton*  pb_noise;
      QPushButton*  pb_subtract;
      QPushButton*  pb_spikes;
      QPushButton*  pb_invert;
      QPushButton*  pb_write;


      QComboBox*    cb_cell;
      QComboBox*    cb_channel;
      QComboBox*    cb_wavelength;

      QwtCounter*   ct_from;
      QwtCounter*   ct_to;
      QwtCounter*   ct_noise;
	
	public slots:
      //void plot_data( void );
      void load     ( void );

      void reset    ( void );
		void help     ( void )
      { showHelp.show_help( "manual/edit_velocity.html" ); };
};
#endif
