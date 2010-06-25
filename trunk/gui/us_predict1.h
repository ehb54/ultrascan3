#ifndef US_PREDICT1_H
#define US_PREDICT1_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_plot.h"
#include "us_math2.h"
#include "us_help.h"
#include "us_buffer.h"
#include "us_analyte_gui.h"
#include "us_analyte.h"
#include "us_hydrosim.h"


//! A class to present a graph of frictional ratio as a function of
//! axial ratio.  Allows specification of parameters and calulate
//! resulting molecular characteristics.

class US_EXTERN US_Predict1 : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! \brief Constructor.
      //! \param parm - Reference to molecular characteristics
      //! \param invID - The investigator ID for DB access.  -1 for unpecified.
      //! \param a_data - Analyte data for initialization
      //! \param disk_access - A value to pass to \ref US_Analyte to initial
      //!                      use disk or DB access
      //! \param signal_wanted - A value to indicate if signals are wanted.
      US_Predict1( US_Hydrosim&, 
                   int = -1, 
                   US_Analyte = US_Analyte(),
                   bool = true,
                   bool = false );


   signals:
      //! \brief Return a data set of current analyte values.  This
      //!        also indicates that the class is done and the Hydrosim
      //!        reference has been updated.
      void changed( US_Analyte );

      //! \brief A signal to indicate that calculations have been updated.
      void changed( void );

      //! \brief A signal to indicate that the class is finished and is closing.
      void done   ( void );

   private:
      US_Hydrosim&            allparams;
      int                     investigator;
      US_Analyte              base_analyte;
      bool                    access;
      bool                    signal;

      US_Math2::SolutionData  solution;

      US_Buffer               buffer;
      US_Analyte              analyte;

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

      QLineEdit*     le_mw;
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

      void update       ( void );

   private slots:
      void new_value    ( const QwtDoublePoint&   );
      void mouseU       ( const QwtDoublePoint&   );
      void update_ratio ( void                    );
      void update_mw    ( const QString&          );
      void degC         ( const QString&          ); 
      void density      ( const QString&          );
      void viscosity    ( const QString&          );
      void vbar         ( const QString&          );
      void get_buffer   ( void                    ); 
      void get_peptide  ( void                    ); 
      void complete     ( void                    );
      void debug        ( void                    );

      void update_buffer( const US_Buffer         );
      void update_vbar  ( const US_Analyte        );

      void help         ( void )
      { showHelp.show_help( "manual/predict1.html" ); };
};
#endif
