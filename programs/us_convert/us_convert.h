#ifndef US_CONVERT_H
#define US_CONVERT_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_plot.h"
#include "us_dataIO.h"

class US_EXTERN US_Convert : public US_Widgets
{
  Q_OBJECT

  public:
      US_Convert();

      class TripleInfo
      {
         public:
         int              tripleID;
         int              centerpiece;
         int              bufferID;
         int              analyteID;
         TripleInfo();
      };

      class ExperimentInfo
      {
         public:
         int              invID;
         QString          lastName;
         QString          firstName;
         QString          expType;
         int              rotor;
         QString          date;
         QString          label;
         QString          comments;
         QList< TripleInfo > triples;
         ExperimentInfo();
         void clear( void );
      };

      enum ioError { OK, CANTOPEN, NODATA, NOXML, PARTIAL_XML };

  private:

      enum { SPLIT, REFERENCE, NONE } step;

      US_Help        showHelp;
      US_PlotPicker* picker;

      QString       runType;
      QString       oldRunType;
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

      QPushButton*  pb_writeAll;
      QPushButton*  pb_exclude;
      QPushButton*  pb_include;
      QPushButton*  pb_details;
      QPushButton*  pb_define;
      QPushButton*  pb_process;
      QPushButton*  pb_reference;
      QPushButton*  pb_cancelref;
      QPushButton*  pb_expinfo;
      QPushButton*  pb_tripleinfo;

      QList< US_DataIO::beckmanRaw >  legacyData;     // legacy data from file
      QList< US_DataIO::rawData >     allData;        // all the data, separated by c/c/w
      QList< US_DataIO::rawData >     RIData;         // to save RI data, after converting to RP
      QString       saveDir;
      QString       saveDescription;

      QList< int >  includes;                         // list of points to include in plot
      QwtPlot*      data_plot;
      QwtPlotGrid*  grid;

      QList< double > ss_limits;                      // list of subset boundaries
      double        reference_start;                  // boundary of reference scans
      double        reference_end;
      bool          RP_averaged;                      // true if RI averages have been done
      int           RP_reference_triple;              // number of the triple that is the reference
      QList< double > RP_averages;

      void plot_current    ( void );
      void plot_titles     ( void );
      void init_includes   ( void );
      void plot_all        ( void );
      void replot          ( void );
      void set_colors      ( const QList< int >& );
      void RP_calc_avg     ( void );
  
      bool show_plot_progress;
      ExperimentInfo       ExpData; 

  private slots:
      void load            ( QString dir = "" );
      void reload          ( void );
      bool read            ( void );
      bool read            ( QString dir );
      void setTripleInfo   ( void );
      void details         ( void );
      void reset           ( void );
      void resetAll        ( void );
      bool convert         ( void );
      void changeTriple    ( QListWidgetItem* );
      void focus_from      ( double );
      void focus_to        ( double );
      void focus           ( int, int );
      void exclude_scans   ( void );
      void include         ( void );
      void reset_scan_ctrls( void );
      int  writeAll        ( void );
      void cClick          ( const QwtDoublePoint& );
      void define_subsets  ( void );
      void process_subsets ( void );
      void cDrag           ( const QwtDoublePoint& );
      void define_reference  ( void );
      void start_reference   ( const QwtDoublePoint& );
      void process_reference ( const QwtDoublePoint& );
      void cancel_reference( void );
      void getExpInfo      ( void );
      void updateExpInfo   ( US_Convert::ExperimentInfo& );
      void cancelExpInfo   ( void );
      void getTripleInfo   ( void );
      void updateTripleInfo( US_Convert::TripleInfo& );
      void cancelTripleInfo( void );
      void draw_vline      ( double );
      void help            ( void )
        { showHelp.show_help( "manual/us_convert.html" ); };
};
#endif
