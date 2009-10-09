#ifndef US_ANALYSIS_BASE_H
#define US_ANALYSIS_BASE_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets.h"
#include "us_dataIO.h"
#include "us_plot.h"
#include "us_math.h"
#include "us_help.h"

#include "qwt_counter.h"

//! \brief A base class for analysis programs

class US_EXTERN US_AnalysisBase : public US_Widgets
{
   Q_OBJECT

   public:
      US_AnalysisBase();

   protected:
      QList< US_DataIO::editedData > dataList;
      QList< US_DataIO::rawData    > rawList;
      QStringList                    triples;
      QList< int >                   excludedScans;

      QList< QVector< double > >     savedValues;

      struct solution_data           solution;
      
      bool         dataLoaded;
      double       time_correction;
      QString      directory;
      QString      editID;
      QString      runID;

      US_Help      showHelp;

      double       density;
      double       viscosity;
      double       vbar;

      // Layouts
      QBoxLayout*  mainLayout;
      QBoxLayout*  leftLayout;
      QBoxLayout*  rightLayout;
      QBoxLayout*  buttonLayout;

      QGridLayout* analysisLayout;
      QGridLayout* runInfoLayout;
      QGridLayout* parameterLayout;
      QGridLayout* controlsLayout;

      US_Plot*     plotLayout1;  // Derived from QVBoxLayout
      US_Plot*     plotLayout2;

      // Widgets
      QwtPlot*     data_plot1;
      QwtPlot*     data_plot2;

      QPushButton* pb_load;
      QPushButton* pb_details;
      QPushButton* pb_view;
      QPushButton* pb_save;
      QPushButton* pb_reset;
      QPushButton* pb_help;
      QPushButton* pb_close;
      QPushButton* pb_exclude;

      QLineEdit*   le_id;
      QLineEdit*   le_temp;
      QTextEdit*   te_desc;
      QListWidget* lw_triples;

      QLineEdit*   le_density;
      QLineEdit*   le_viscosity;
      QLineEdit*   le_vbar;
      QLineEdit*   le_skipped;

      QwtCounter*  ct_smoothing;
      QwtCounter*  ct_boundaryPercent;
      QwtCounter*  ct_boundaryPos;
      QwtCounter*  ct_from;
      QwtCounter*  ct_to;

      virtual void update   ( int );
      virtual void data_plot( void );
      static  void write_plot( const QString&, const QwtPlot* );
      bool         mkdir     ( const QString&, const QString& );

   protected slots:
      virtual void reset        ( void );
      virtual void new_triple   ( int  );
      virtual void load         ( void );
      QString      run_details  ( void )                           const;
      QString      hydrodynamics( void )                           const;
      QString      analysis     ( const QString& )                 const;
      QString      scan_info    ( void )                           const;
      
      QString      table_row    ( const QString&, const QString& ) const;
      QString      table_row    ( const QString&, const QString&, 
                                  const QString& )                 const;
      double       calc_baseline( void )                           const;

   private:
      double* x_weights;
      double* y_weights;
      
      double smooth_point( int, int, int, int, int = 0 );

   private slots:
      void details      ( void   );
      void get_vbar     ( void   );
      void update_vbar  ( double );
      void get_buffer   ( void   );
      void update_buffer( double, double );
      void boundary_pct ( double );
      void boundary_pos ( double );
      void exclude_from ( double );
      void exclude_to   ( double );
      void exclude      ( void   );
      void smoothing    ( double );
};
#endif
