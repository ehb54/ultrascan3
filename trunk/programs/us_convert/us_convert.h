//! \file us_convert.h
#ifndef US_CONVERT_H
#define US_CONVERT_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_plot.h"
#include "us_dataIO2.h"
#include "us_expinfo.h"
#include "us_solution.h"
#include "us_selectbox.h"
//#include "us_solution_gui.h"

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

      //! \brief  Class that contains information about relevant 
      //!         cell/channel/wavelength combinations
      class TripleInfo
      {
         public:
         int              tripleID;           //!< The ID of this c/c/w combination
         QString          tripleDesc;         //!< The description of this triple ( e.g., "2 / A / 260" )
         QString          description;        //!< A text description of this triple
         char             tripleGUID[16];     //!< The GUID of this triple
         QString          tripleFilename;     //!< The filename of this auc file
         bool             excluded;           //!< Whether this triple has been dropped or not
         int              centerpiece;        //!< The ID of the centerpiece that was used
         US_Solution      solution;           //!< The solution information for this triple
         TripleInfo();                        //!< A generic constructor
         void             clear( void );
         void             show( void );       // temporary
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

      US_Disk_DB_Controls* disk_controls;     //!< Radiobuttons for disk/db choice

      QList< double > subsets;                //!< A list of subset boundaries

  signals:

  public slots:

  private:

      enum { SPLIT, REFERENCE, NONE } step;

      aucStatus      saveStatus;
      US_Help        showHelp;
      US_PlotPicker* picker;

      QString       runType;
      QString       oldRunType;

      QLabel*       lb_description;

      QLineEdit*    le_investigator;
      QString       runID;
      QLineEdit*    le_runID;
      QLineEdit*    le_runID2;
      QLineEdit*    le_dir;
      QLineEdit*    le_description;
      QLineEdit*    le_solutionDesc;

      QLabel*       lb_triple;
      QListWidget*  lw_triple;                        // cell, channel, wavelength

      QwtCounter*   ct_from;
      QwtCounter*   ct_to;

      QwtCounter*   ct_tolerance;

      QPushButton*  pb_editRuninfo;
      QPushButton*  pb_import;
      QPushButton*  pb_loadUS3;
      QPushButton*  pb_details;
      QPushButton*  pb_applyAll;
      QPushButton*  pb_solution;
      QPushButton*  pb_exclude;
      QPushButton*  pb_include;
      QPushButton*  pb_define;
      QPushButton*  pb_process;
      QPushButton*  pb_reference;
      QPushButton*  pb_intensity;
      QPushButton*  pb_cancelref;
      QPushButton*  pb_dropScan;
      QPushButton*  pb_saveUS3;

      US_SelectBox*  cb_centerpiece;

      QList< US_DataIO2::BeckmanRawScan > legacyData; // legacy data from file
      QVector< US_DataIO2::RawData >      allData;    // all the data, separated by c/c/w
      QVector< US_DataIO2::RawData >      RIData;     // to save RI data, after converting to Pseudo
      QString       currentDir;
      QString       saveDescription;

      QVector< Excludes > allExcludes;                // excludes for all triples

      QwtPlot*      data_plot;
      QwtPlotGrid*  grid;

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
      QList< TripleInfo >             triples;
      int                             currentTriple;

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
      //! \brief Select the current investigator
      void sel_investigator( void );

      /*! \brief Assign the selected investigator as current
          \param invID  The ID of the selected investigator
          \param lname  The last name of the investigator
          \param fname  The first name of the investigator
      */
      void assign_investigator( int, const QString&, const QString& );

      void import          ( QString dir = "" );
      void reimport        ( void );
      void enableControls  ( void );
      void runIDChanged    ( void );
      void toleranceValueChanged( double );           // signal to notify of change
      void editRuninfo     ( void );
      void loadUS3         ( QString dir = "" );
      void loadUS3Disk     ( void );
      void loadUS3Disk     ( QString );
      void loadUS3DB       ( void );
      void updateExpInfo   ( US_ExpInfo::ExperimentInfo& );
      void cancelExpInfo   ( void );
      void getSolutionInfo ( void );
      void updateSolutionInfo( US_Solution& );
      void cancelSolutionInfo( void );
      void tripleApplyAll    ( void );
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
      void saveUS3         ( void );
      int  saveUS3Disk     ( void );
      void saveUS3DB       ( void );
      void resetAll        ( void );
      void source_changed  ( bool );
      void help            ( void )
        { showHelp.show_help( "manual/us_convert.html" ); };
};
#endif
