#ifndef US_CONVERT_H
#define US_CONVERT_H

#include <QtGui>

#include "qwt_plot_marker.h"

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_plot.h"
#include "us_dataIO.h"

class US_Convert : public US_Widgets
{
	Q_OBJECT

	public:
		US_Convert();

	private:

      US_Help        showHelp;
      US_PlotPicker* pick;

      QString       runType;
      QStringList   triples;

      QLineEdit*    le_dir;

      QComboBox*    cb_triple;                // cell, channel, wavelength

      QwtCounter*   ct_from;
      QwtCounter*   ct_to;

      QPushButton*  pb_write;
      QPushButton*  pb_writeAll;
      QPushButton*  pb_exclude;
      QPushButton*  pb_excludeRange;
      QPushButton*  pb_include;

      QList< beckmanRaw > legacyData;          // legacy data from file
      QList< beckmanRaw* > ccwLegacyData;      // legacy data with this cell/channel/wl
      rawData     newRawData;                  // filtered legacy data in new raw format

      QList< int >  includes;                  // list of points to include in plot
      QwtPlot*      data_plot;
      QwtPlotGrid*  grid;
      QProgressBar* progress;

      int  write           ( const QString& );
      void setInterpolated ( unsigned char*, int );
      void plot_current    ( void );
      void init_includes   ( void );
      void plot_all        ( void );
      void replot          ( void );
      void set_colors      ( const QList< int >& );
   
	private slots:
      void load            ( void );
      void reset           ( void );
      void read            ( void );
      void convert         ( void );
      void changeCcw       ( int  );
      void focus_from      ( double );
      void focus_to        ( double );
      void focus           ( int, int );
      void exclude_one     ( void );
      void exclude_range   ( void );
      void include         ( void );
      void reset_scan_ctrls( void );
      void write           ( void );
      void writeAll        ( void );
		  void help            ( void )
        { showHelp.show_help( "manual/us_convert.html" ); };
};
#endif
