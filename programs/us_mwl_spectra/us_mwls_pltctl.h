//! \file us_mwls_pltctl.h
#ifndef US_MWL_S_PLTCTL_H
#define US_MWL_S_PLTCTL_H

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_plot3d.h"
#include "us_plot.h"
#include "us_help.h"

//! \brief A class to provide a window with enhanced plot controls

class US_MwlSPlotControl : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! \brief US_MwlSPlotControl constructor
      //! \param p  Pointer to the parent of this widget
      //! \param d  Pointer to the xyz data to be plotted
      US_MwlSPlotControl( QWidget* p = 0, QVector< QVector3D >* = 0 );

      //! \brief Function to return US_MwlSPlotControl's caller
      //! \returns Pointer to the QWidget that created this plot control
      QWidget*      caller      ( void );

      //! \brief Force (re-)plot of currently specified 3-D plot
      void          do_3dplot   ( void );

      //! \brief Return 3-d plot data widget pointer
      QGLWidget*    data_3dplot ( void );

      //! \brief Return 3-d plot main widget pointer
      US_Plot3D* widget_3dplot  ( void );

   signals:
      //! \brief Signal emitted when this dialog has been closed.
      void has_closed( void );

   private:
      QVBoxLayout*  mainLayout;
      QGridLayout*  controlsLayout;
      QHBoxLayout*  buttonsLayout;

      QwtCounter*   ct_zscalefac;
      QwtCounter*   ct_rxscale;
      QwtCounter*   ct_ryscale;

      QCheckBox*    ck_yrevrs;

      QWidget*              wparent;
      QPointer< US_Plot3D > plot3d_w;
      QVector< QVector3D >* xyzdat;

      double        zscale;
      double        rxscale;
      double        ryscale;

   protected:
      US_Help       showHelp;

   private slots:
      void plot3_btn( void );
      void close_all( void );

      void help     ( void )
      { showHelp.show_help( "mwl_spectra_pltctl.html" ); };
};
#endif

