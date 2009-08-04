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
      rawData data;

      enum { MENISCUS, RANGE, PLATAEU, BASELINE, FINISHED } step;

      double meniscus;
      double meniscus_left;
      double range_left;
      double range_right;
      double plateau;
      double baseline;



      US_Help        showHelp;
      US_PlotPicker* pick;
      
      class channels
      {
         public:
            QChar channel;
            QList< double > wavelength;
            
            bool operator== ( const channels& x )
              const { return channel == x.channel; };
      };

      class cell
      {
         public:
            int cellNum;
            QList< channels > channelList;

            bool operator== ( const cell& x )
              const { return cellNum == x.cellNum; };
      };

      QList< cell > cellList;
      QString       workingDir;
      QString       runID;
      QStringList   files;


      QwtPlot*      data_plot;
      QwtPlotCurve* raw_curve;
      QwtPlotCurve* fit_curve;
      QwtPlotCurve* v_line;
      QwtPlotCurve* minimum_curve;
      QwtPlotGrid*  grid;

      QwtPlotMarker* marker;

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
	
      void set_channels   ( void );
      void set_wavelengths( void );
      void plot_current   ( void );
      void set_pbColors   ( QPushButton* );
      void draw_vline     ( double );
      int  index          ( scan*, double );
      void next_step      ( void );

	private slots:

      void load     ( void );
      void mouse    ( const QwtDoublePoint& );

      void reset    ( void );
		void help     ( void )
      { showHelp.show_help( "manual/edit_velocity.html" ); };
};
#endif
