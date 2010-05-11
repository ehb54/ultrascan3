//! \file us_run_details.h
#ifndef US_DISTRIB_PLOT_H
#define US_DISTRIB_PLOT_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_dataIO.h"
#include "us_plot.h"
#include "us_help.h"

//! \brief A class to provide a window with the details of a run

class US_EXTERN US_DistribPlot : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! \param divfracs A reference to a list of division bdoundary fractions
      //! \param divsedcs A reference to a list of division intercept sedcoeffs
      US_DistribPlot( const QList< double >&, const QList< double >& );

   private:
      QList< double >  bfracs;    // boundary fractions for divisions
      QList< double >  dsedcs;    // sed.coeff. intercepts for divisions
      
      enum pType { DISTR, HISTO, ENVEL, COMBO, NONE };

      US_Help       showHelp;

      pType         plotType;
      pType         plotTypeH;

      QLabel*       lb_sensitivity;
      QLabel*       lb_smoothing;

      QPushButton*  pb_histogram;
      QPushButton*  pb_envelope;
      QPushButton*  pb_plot_type;
      QPushButton*  pb_help;
      QPushButton*  pb_close;

      QwtCounter*   ct_sensitivity;
      QwtCounter*   ct_smoothing;

      QwtPlot*      data_plot;
      QwtPlotCurve* dcurve;
      QwtPlotCurve* hcurve;
      QwtPlotCurve* ecurve;

      int           divsCount;
      int           nSensit;
      int           nSmooth;

   private slots:
      void type_plot     ( void );
      void hide_histo    ( void );
      void hide_envel    ( void );
      void show_plot     ( void );
      void plot_distrib  ( void );
      void plot_histogram( void );
      void plot_envelope ( void );
      void plot_combined ( void );
      void change_sensit ( double );
      void change_smooth ( double );
      int  histo_data( double**, double** );
      int  envel_data( double**, double** );
      void help     ( void )
      { showHelp.show_help( "vHW_distrib_plot.html" ); };
};
#endif
