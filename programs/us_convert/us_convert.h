//! \file us_convert.h
#ifndef US_CONVERT_H
#define US_CONVERT_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_plot.h"
#include "us_dataIO2.h"
#include "us_analyte.h"
#include "us_expinfo.h"
#include "us_solution_gui.h"

class US_EXTERN US_Convert : public US_Widgets
{
  Q_OBJECT

  public:

      //! \brief  Generic constructor for the US_Convert() program.
      US_Convert();

      //! \brief   Some status codes returned by the us_convert program
      enum ioError
      { 
         OK,                                  //!< Ok, no error
         CANTOPEN,                            //!< The file cannot be opened
         DUP_RUNID,                           //!< The given run ID already exists
         NODATA,                              //!< There is no data to write
         NODB,                                //!< Connection to database cannot be made
         NOXML,                               //!< XML data has not been entered
         BADXML,                              //!< XML not formed correctly
         BADGUID,                             //!< GUID read in the XML was not found in the database
         PARTIAL_XML,                         //!< XML data has not been entered for all c/c/w combinations
         NOT_WRITTEN                          //!< data was not written
      };

      //! \brief   Some status codes to keep track of where data has been saved to
      enum aucStatus
      {
         NOT_SAVED,                           //!< The file has not been saved
         EDITING,                             //!< Data is being edited; certain operations not permitted
         HD_ONLY,                             //!< The file has been saved to the HD
         DB_SYNC,                             //!< The HD file has been synchronized with the DB
         BOTH                                 //!< The file has been saved to both HD and DB
      };

      //! \brief Class to contain a list of scans to exclude from a data set
      //!        for a single c/c/w combination
      class Excludes
      {
         public:
         QList< int >  excludes;              //!< list of scan indexes to exclude 
         bool contains ( int x )              //!< function to determine if x is contained in the list
           { return excludes.contains( x ); }
         bool empty    ()                     //!< function to determine if the list is empty
           { return excludes.empty(); }
         void push_back( int x )              //!< function to add x to the end of the list
           { excludes.push_back( x ); }
         Excludes& operator<<( const int x )  //!< function to insert x at the end of the list
            { this->push_back( x ); return *this; }

      };

  private:

      enum { SPLIT, REFERENCE, NONE } step;

      aucStatus      saveStatus;
      US_Help        showHelp;
      US_PlotPicker* picker;

      QString       runType;
      QString       oldRunType;

      QLabel*       lb_description;

      QString       runID;
      QLineEdit*    le_runID;
      QLineEdit*    le_dir;
      QLineEdit*    le_description;
      QLineEdit*    le_bufferInfo;
      QLineEdit*    le_analyteInfo;

      QLabel*       lb_triple;
      QListWidget*  lw_triple;                        // cell, channel, wavelength

      QwtCounter*   ct_from;
      QwtCounter*   ct_to;

      QwtCounter*   ct_tolerance;

      QPushButton*  pb_editRuninfo;
      QPushButton*  pb_load;
      QPushButton*  pb_loadUS3HD;
      QPushButton*  pb_loadUS3DB;
      QPushButton*  pb_details;
      QPushButton*  pb_solution;
      QPushButton*  pb_exclude;
      QPushButton*  pb_include;
      QPushButton*  pb_define;
      QPushButton*  pb_process;
      QPushButton*  pb_reference;
      QPushButton*  pb_intensity;
      QPushButton*  pb_cancelref;
      QPushButton*  pb_dropScan;
      QPushButton*  pb_savetoHD;
      QPushButton*  pb_savetoDB;

      QComboBox*    cb_centerpiece;

      QList< US_DataIO2::BeckmanRawScan > legacyData; // legacy data from file
      QVector< US_DataIO2::RawData >      allData;    // all the data, separated by c/c/w
      QVector< US_DataIO2::RawData >      RIData;     // to save RI data, after converting to Pseudo
      QString       currentDir;
      QString       saveDescription;

      QVector< Excludes > allExcludes;                // excludes for all triples

      QwtPlot*      data_plot;
      QwtPlotGrid*  grid;

      QList< double > ss_limits;                      // list of subset boundaries
      double        reference_start;                  // boundary of reference scans
      double        reference_end;
      bool          Pseudo_averaged;                      // true if RI averages have been done
      int           Pseudo_reference_triple;              // number of the triple that is the reference
      QVector< double > Pseudo_averages;
      bool          isPseudo;                         // Is this RI data pseudo-absorbance?
      bool          toleranceChanged;                 // keep track of whether the tolerance has changed
      double        scanTolerance;                    // remember the scan tolerance value

      bool show_plot_progress;
      US_ExpInfo::ExperimentInfo      ExpData; 
      QList< US_SolutionGui::TripleInfo > triples;
      int                             currentTriple;
      QStringList                     centerpieceTypes;

      void reset           ( void );
      void enableRunIDControl( bool );
      void enableScanControls( void );
      void enableSyncDB    ( void );
      void getExpInfo      ( void );
      void setTripleInfo   ( void );
      int  findTripleIndex ( void );
      void focus           ( int, int );
      void init_excludes   ( void );
      void start_reference   ( const QwtDoublePoint& );
      void process_reference ( const QwtDoublePoint& );
      void PseudoCalcAvg     ( void );
      bool read            ( void );
      bool read            ( QString dir );
      bool convert         ( void );
      void initTriples     ( void );
      bool centerpieceInfo ( void );
      void plot_current    ( void );
      void plot_titles     ( void );
      void plot_all        ( void );
      void replot          ( void );
      void set_colors      ( const QList< int >& );
      void draw_vline      ( double );
      void db_error        ( const QString& );

  private slots:
      void load            ( QString dir = "" );
      void reload          ( void );
      void enableControls  ( void );
      void runIDChanged    ( void );
      void toleranceValueChanged( double );           // signal to notify of change
      void editRuninfo     ( void );
      void loadUS3HD       ( void );
      void loadUS3HD       ( QString );
      void loadUS3DB       ( void );
      void updateExpInfo   ( US_ExpInfo::ExperimentInfo& );
      void cancelExpInfo   ( void );
      void getSolutionInfo ( void );
      void updateSolutionInfo( QList< US_SolutionGui::TripleInfo >&, int& );
      void cancelSolutionInfo( void );
      void runDetails      ( void );
      void changeTriple    ( QListWidgetItem* );
      void getCenterpieceIndex( int );
      void focus_from      ( double );
      void focus_to        ( double );
      void exclude_scans   ( void );
      void include         ( void );
      void define_subsets  ( void );
      void cClick          ( const QwtDoublePoint& );
      void process_subsets ( void );
      void define_reference  ( void );
      void show_intensity  ( void );
      void cancel_reference( void );
      void drop_reference  ( void );
      int  savetoHD        ( void );
      void savetoDB        ( void );
      void resetAll        ( void );
      void help            ( void )
        { showHelp.show_help( "manual/us_convert.html" ); };
};
#endif
