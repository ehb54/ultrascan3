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
      int           currentTriple;

      QLineEdit*    le_dir;

      QComboBox*    cb_triple;                // cell, channel, wavelength
      QLabel*       lb_triple;

      QwtCounter*   ct_from;
      QwtCounter*   ct_to;

      QwtCounter*   ct_tolerance;

      QPushButton*  pb_write;
      QPushButton*  pb_writeAll;
      QPushButton*  pb_exclude;
      QPushButton*  pb_include;
      QPushButton*  pb_details;

      QList< US_DataIO::beckmanRaw >  legacyData;     // legacy data from file
      QList< US_DataIO::beckmanRaw* > ccwLegacyData;  // legacy data with this cell/channel/wl
      US_DataIO::rawData              newRawData;     // filtered legacy data in new raw format
      QList< US_DataIO::rawData >     allData;        // all the data, separated by c/c/w

      QList< int >  includes;                         // list of points to include in plot
      QwtPlot*      data_plot;
      QwtPlotGrid*  grid;

      QLabel*       lb_progress;
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
      void details         ( void );
      void reset           ( void );
      void resetAll        ( void );
      void read            ( void );
      void setCcwTriples   ( void );
      void setCcrTriples   ( void );
      void convert         ( bool showProgressBar = false );
      void changeTriple    ( int  );
      void focus_from      ( double );
      void focus_to        ( double );
      void focus           ( int, int );
      void exclude_scans   ( void );
      void include         ( void );
      void reset_scan_ctrls( void );
      void write           ( void );
      int  writeAll        ( void );
      void help            ( void )
        { showHelp.show_help( "manual/us_convert.html" ); };
};
#endif
