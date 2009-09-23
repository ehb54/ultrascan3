#ifndef US_ANALYSIS_BASE_H
#define US_ANALYSIS_BASE_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets.h"
#include "us_dataIO.h"
#include "us_plot.h"
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

      int     load_status;
      QString directory;
      QString editID;
      QString runID;

      US_Help      showHelp;

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

      virtual void load  ( void );
      virtual void update( int );

   protected slots:
      void tbd( void   );
      void tbd( double );

   private slots:
      void details    ( void   );
      void get_vbar   ( void   );
      void update_vbar( double );

};
#endif
