#ifndef US_CONVERT_H
#define US_CONVERT_H

#include <QtGui>

#include "qwt_plot_marker.h"

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_plot.h"
#include "us_dataIO.h"
#include "us_expinfo.h"
#include "us_ccwinfo.h"

class US_EXTERN US_Convert : public US_Widgets
{
  Q_OBJECT

  public:
      US_Convert();

  private:

      enum { SPLIT, REFERENCE, NONE } step;

      US_Help        showHelp;
      US_PlotPicker* picker;

      QString       runType;
      QStringList   triples;
      int           currentTriple;

      QString       runID;
      QLineEdit*    le_dir;
      QLineEdit*    le_runID;

      QLabel*       lb_description;
      QLineEdit*    le_description;

      QLabel*       lb_triple;
      QListWidget*  lw_triple;                        // cell, channel, wavelength

      QwtCounter*   ct_from;
      QwtCounter*   ct_to;

      QwtCounter*   ct_tolerance;

      QPushButton*  pb_write;
      QPushButton*  pb_writeAll;
      QPushButton*  pb_exclude;
      QPushButton*  pb_include;
      QPushButton*  pb_details;
      QPushButton*  pb_define;
      QPushButton*  pb_process;
      QPushButton*  pb_reference;
      QPushButton*  pb_cancelref;
      QPushButton*  pb_change_runID;
      QPushButton*  pb_expinfo;
      QPushButton*  pb_ccwinfo;

      QList< US_DataIO::beckmanRaw >  legacyData;     // legacy data from file
      QList< US_DataIO::beckmanRaw* > ccwLegacyData;  // legacy data with this cell/channel/wl
      US_DataIO::rawData              newRawData;     // filtered legacy data in new raw format
      QList< US_DataIO::rawData >     allData;        // all the data, separated by c/c/w
      QList< US_DataIO::rawData >     RIData;         // to save RI data, after converting to RP
      QString       saveDir;
      QString       saveDescription;

      QList< int >  includes;                         // list of points to include in plot
      QwtPlot*      data_plot;
      QwtPlotGrid*  grid;

      QLabel*       lb_progress;
      QProgressBar* progress;

      QList< double > ss_limits;                      // list of subset boundaries
      double        reference_start;                  // boundary of reference scans
      double        reference_end;
      bool          RP_averaged;                      // true if RI averages have been done
      int           RP_reference_triple;              // number of the triple that is the reference
      QList< double > RP_averages;

      int  write           ( const QString& );
      void setInterpolated ( unsigned char*, int );
      void plot_current    ( void );
      void plot_titles     ( void );
      void init_includes   ( void );
      void plot_all        ( void );
      void replot          ( void );
      void set_colors      ( const QList< int >& );
      void RP_calc_avg     ( void );
  
      bool show_plot_progress;
      US_CCWInfo::CCWInfo  CCWData; 
      QList< US_CCWInfo::CCWInfo > allCCWData;
      US_ExpInfo::ExpInfo  ExpData; 
      int writeXmlFile     ( void );

  private slots:
      void load            ( QString dir = "" );
      void details         ( void );
      void change_runID    ( void );
      void reset           ( void );
      void resetAll        ( void );
      void read            ( void );
      void read            ( QString );                // If you know the dirname already
      void setCcwTriples   ( void );
      void setCcrTriples   ( void );
      void convert         ( bool showProgressBar = false );
      void changeTriple    ( QListWidgetItem* );
      void focus_from      ( double );
      void focus_to        ( double );
      void focus           ( int, int );
      void exclude_scans   ( void );
      void include         ( void );
      void reset_scan_ctrls( void );
      void write           ( void );
      int  writeAll        ( void );
      void cClick          ( const QwtDoublePoint& );
      void define_subsets  ( void );
      void process_subsets ( void );
      void cDrag           ( const QwtDoublePoint& );
      void define_reference  ( void );
      void start_reference   ( const QwtDoublePoint& );
      void process_reference ( const QwtDoublePoint& );
      void cancel_reference( void );
      void get_expinfo     ( void );
      void update_expinfo  ( US_ExpInfo::ExpInfo& );
      void cancel_expinfo  ( void );
      void get_ccwinfo     ( void );
      void update_ccwinfo  ( US_CCWInfo::CCWInfo& );
      void cancel_ccwinfo  ( void );
      void draw_vline      ( double );
      void help            ( void )
        { showHelp.show_help( "manual/us_convert.html" ); };
};
#endif
