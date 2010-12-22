//! \file us_analysis_base2.h
#ifndef US_ANALYSIS_BASE_H
#define US_ANALYSIS_BASE_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets.h"
#include "us_dataIO2.h"
#include "us_plot.h"
#include "us_math2.h"
#include "us_help.h"
#include "us_db2.h"
#include "us_analyte.h"
#include "us_buffer.h"

#include "qwt_counter.h"

//! \brief A base class for analysis programs.  Other programs will derive from 
//!        this class and override layouts and functions as required for the
//!        specific analysis to be done.

class US_EXTERN US_AnalysisBase2 : public US_Widgets
{
   Q_OBJECT

   public:
      US_AnalysisBase2();

   protected:
      //! A set of edited data for the analysis
      QVector< US_DataIO2::EditedData > dataList;

      //! A set of raw data for the analysis
      QVector< US_DataIO2::RawData    > rawList;

      //! The currently loaded triples in the form cell / channel / wavelength
      QStringList                       triples;

      //! Scans excluded by the user beyond those excluded by US_Edit
      QList< int >                      excludedScans;

      //! Current data values for athe analysis
      QList< QVector< double > >        savedValues;

      //! The solution data corresponding to the current triple
      US_Math2::SolutionData            solution;
      
      bool         dataLoaded;      //!< A flag to indicate data is loaded
      bool         def_local;       //!< Flag if default source is local
      bool         buffLoaded;      //!< Flag to indicate buffer is loaded

      QString      directory;       //!< Data directory of analysis files.
      QString      editID;          //!< Current edit ID.  Ususally a date-time.
      QString      runID;           //!< User specified run ID string
      QString      dfilter;         //!< Data files filter
      QString      investig;        //!< Investigator string

      US_Buffer    buff;            //!< Currently loaded buffer

      //! A class to display help in the US Help viewer
      US_Help      showHelp;

      double       time_correction; //!< Time correction, centrifuge acceler.
      double       density;         //!< Density of the buffer
      double       viscosity;       //!< Viscosity of the buffer
      double       vbar;            //!< Specific volume of the analyte

      // Layouts
      QGridLayout* mainLayout;      //!< Overall Layout
      QBoxLayout*  leftLayout;      //!< Layout of the left column
      QBoxLayout*  rightLayout;     //!< Layout of the right column
      QBoxLayout*  buttonLayout;    //!< Layout of the bottom button row

      QGridLayout* analysisLayout;  //!< Layout for analysis functions
      QGridLayout* runInfoLayout;   //!< Layout for run details
      QGridLayout* parameterLayout; //!< Layout for other analysis controls
      QGridLayout* controlsLayout;  //!< Layout of analysis controls

      US_Plot*     plotLayout1;     //!< Top plot layout
      US_Plot*     plotLayout2;     //!< Bottom plot layout

      // Widgets
      QwtPlot*     data_plot1;      //!< Access to the top qwt plot 
      QwtPlot*     data_plot2;      //!< Access to the bottom qwt plot

      QPushButton* pb_load;         //!< Pushbutton to load data
      QPushButton* pb_details;      //!< Pushbuttion to view run details
      QPushButton* pb_view;         //!< Pushbuttion to view run a data report
      QPushButton* pb_save;         //!< Pushbutton to save the analysis results
      QPushButton* pb_reset;        //!< Pushbutton to reset the screen
      QPushButton* pb_help;         //!< Pushbutton to show help
      QPushButton* pb_close;        //!< Pushbutton to close the application
      QPushButton* pb_exclude;      //!< Pushbutton to exclude specified scans
      QPushButton* pb_reset_exclude;//!< PB to revert the excluded scan range

      QLineEdit*   le_id;           //!< Display the current RunID
      QLineEdit*   le_temp;         //!< Display/change the temperature used
      QTextEdit*   te_desc;         //!< Text edit box for discription
      QListWidget* lw_triples;      //!< List widget containing triples

      QLineEdit*   le_density;      //!< Display/change the density used
      QLineEdit*   le_viscosity;    //!< Display/change the viscosity
      QLineEdit*   le_vbar;         //!< Display/change the specific volume
      QLineEdit*   le_skipped;      //!< Display the number of scans skipped

      QwtCounter*  ct_smoothing;
        //!< Counter for the number of coefficients in a polynomial smoothing function
      QwtCounter*  ct_boundaryPercent;
        //!< Counter for boundary percentage specification
      QwtCounter*  ct_boundaryPos;  //!< Counter for boundary starting point
      QwtCounter*  ct_from;         //!< Counter for specifying start of exclude range
      QwtCounter*  ct_to;           //!< Counter for specifying end of exclude range

      QCheckBox*   ck_edlast;           //!< Checkbox for edit-lastest flag

      US_Disk_DB_Controls* disk_controls; //!< Radiobuttons for disk/db choice

      virtual void update   ( int );    //!< Update the screen for a new dataset
      virtual void data_plot( void );   //!< Update the data plot

      //! Write the specified plot to a file
      static  void write_plot( const QString&, const QwtPlot* ); 

      //! A utility to create a directory
      bool         mkdir     ( const QString&, const QString& );

   protected slots:
      //! Resets the class to a default state.
      virtual void reset        ( void );
    
      //! Update the class to show data for a new triple.
      virtual void new_triple   ( int  );

      //! Load data from a directory on disk.
      virtual void load         ( void );

      //! Return run details in an html formatted string.
      QString      run_details  ( void )                           const;

      //! List solution data in an html formatted string.
      QString      hydrodynamics( void )                           const;

      //! Return smoothing and boundary factors in an html formatted string.
      QString      analysis     ( const QString& )                 const;

      //! Return scan information in an html formatted string.
      QString      scan_info    ( void )                           const;
      
      //! Returns an html string of a two column table row.
      QString      table_row    ( const QString&, const QString& ) const;

      //! Returns an html string of a three column table row.
      QString      table_row    ( const QString&, const QString&, 
                                  const QString& )                 const;
      
      //! Calculate the 11 point average of the last point in the current
      //! dataset boundary.  Assumes that there are at least 5 points
      //! available above the boundary.
      double       calc_baseline( void )                           const;

      //! Exclude scans in the specified range
      virtual void exclude( void );

      //! Restore excluded scans
      virtual void reset_excludes( void );


   private:
      double* x_weights;
      double* y_weights;
      
      double smooth_point( int, int, int, int, int = 0 );

   private slots:
      void details       ( void   );
      void get_vbar      ( void   );
      void update_vbar   ( US_Analyte );
      void get_buffer    ( void   );
      void update_buffer ( double, double );
      void boundary_pct  ( double );
      void boundary_pos  ( double );
      void exclude_from  ( double );
      void exclude_to    ( double );
      void smoothing     ( double );
      void update_disk_db( bool );
      bool solinfo_db    ( US_DataIO2::EditedData*, QString&, QString&,
                           QString&, QString& );
      bool solinfo_disk  ( US_DataIO2::EditedData*, QString&, QString&,
                           QString&, QString& );
      bool bufvals_db    ( QString&, QString&, QString&, QString&, QString& );
      bool bufvals_disk  ( QString&, QString&, QString&, QString&, QString& );
      bool verify_buffer ( void );
      void buffer_text   ( void );
      bool verify_vbar   ( void );
      void vbar_text     ( void );
      void load_progress ( void );
};
#endif
