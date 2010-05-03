#ifndef US_PREDICT1_H
#define US_PREDICT1_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_plot.h"
#include "us_math.h"
#include "us_help.h"
#include "us_buffer.h"
#include "us_analyte.h"


class US_EXTERN US_Predict1 : public US_WidgetsDialog
{
	Q_OBJECT

	public:
      class Hydrocomp
      {
         public:
            Hydrocomp();
            double sedcoeff;
            double diffcoeff;
            double f;
            double f_f0;
            double a;
            double b;
            double volume;
      };

      class Hydrosim
      {
         public:
            Hydrosim();
            double mw;
            double density;
            double viscosity;
            double vbar;
            double temperature;
            double axial_ratio;

            Hydrocomp sphere;
            Hydrocomp prolate;
            Hydrocomp oblate;
            Hydrocomp rod;
      };

		US_Predict1( Hydrosim&, int = -1, QWidget* = 0, Qt::WindowFlags = 0 );
      void update( void );

   signals:
      void changed( void );
      void done   ( void );

   private:
      US_Math::SolutionData   d;
      Hydrosim&               allparams;
      
      US_Buffer               buffer;
      US_Analyte::analyteData analyte;
      int                     investigator;

      static const int ARRAYSIZE = 999;
      
      double         oblate [ ARRAYSIZE ];
      double         rod    [ ARRAYSIZE ];
      double         prolate[ ARRAYSIZE ];
      double         ratio_x[ ARRAYSIZE ];

      double         vline_x[ 2 ];
      double         vline_y[ 2 ];

      double         temperature;
      double         mw;
      double         ratio;

      QLineEdit*     le_density;
      QLineEdit*     le_viscosity;
      QLineEdit*     le_vbar;
      QLineEdit*     le_axial;

      QLabel*        lb_info;
      QLabel*        lb_sphere [ 8 ];
      QLabel*        lb_prolate[ 8 ];
      QLabel*        lb_oblate [ 8 ];
      QLabel*        lb_rod    [ 8 ];
                     
      US_Plot*       plotLayout;
      QwtPlot*       plot;
      QwtPlotPicker* pick;

      QwtPlotCurve*  prolate_curve;
      QwtPlotCurve*  oblate_curve;
      QwtPlotCurve*  rod_curve;
      QwtPlotCurve*  vline_curve;

      US_Help        showHelp;

   private slots:
      void new_value    ( const QwtDoublePoint&   );
      void mouseU       ( const QwtDoublePoint&   );
      void update_ratio ( const QString&          );
      void update_mw    ( const QString&          );
      void degC         ( const QString&          ); 
      void density      ( const QString&          );
      void viscosity    ( const QString&          );
      void vbar         ( const QString&          );
      void get_buffer   ( void                    ); 
      void get_peptide  ( void                    ); 
      void complete     ( void                    );

      void update_buffer( const US_Buffer               );
      void update_vbar  ( const US_Analyte::analyteData );

      void help         ( void )
      { showHelp.show_help( "manual/predict1.html" ); };
};
#endif
