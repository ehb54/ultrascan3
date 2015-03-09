//! \file us_vhwc_pltctl.h
#ifndef US_VHW_C_PLTCTL_H
#define US_VHW_C_PLTCTL_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_plot3d_xyz.h"
#include "us_plot.h"
#include "us_help.h"

//! \brief A class to provide a window with enhanced plot controls

class US_VhwCPlotControl : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! \brief US_VhwCPlotControl constructor
      //! \param p  Pointer to the parent of this widget
      //! \param d  Pointer to the xyz data to be plotted
      //! \param a_envel  Flag if data is envelope data
      US_VhwCPlotControl( QWidget* p = 0, QVector< QVector3D >* = 0,
                          bool = false );

      //! \brief Function to return US_VhwCPlotControl's caller
      //! \returns Pointer to the QWidget that created this plot control
      QWidget*      caller       ( void );

      //! \brief Force (re-)plot of currently specified 3-D plot
      void          do_3dplot    ( void );

      //! \brief Return 3-d plot data widget pointer
      QGLWidget*    data_3dplot  ( void );

      //! \brief Return 3-d plot main widget pointer
      US_Plot3Dxyz* widget_3dplot( void );

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
      QwtCounter*   ct_peaksmoo;
      QwtCounter*   ct_peakwid;
      QwtCounter*   ct_gridres;

      QCheckBox*    ck_yrevrs;
      QCheckBox*    ck_contour;

      QWidget*                 wparent;
      QPointer< US_Plot3Dxyz > plot3d_w;
      QVector< QVector3D >*    xyzdat;

      double        zscale;
      double        rxscale;
      double        ryscale;
      double        pksmooth;
      double        pkwidth;

      int           gridres;

      bool          envel;

   protected:
      US_Help       showHelp;

   private slots:
      void ptype_toggle( bool );
      void plot3_btn   ( void );
      void close_all   ( void );

      void help        ( void )
      { showHelp.show_help( "vhw_combine_pltctl.html" ); };
};
#endif

