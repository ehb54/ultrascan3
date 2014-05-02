//! \file us_mwl_pltctrl.h
#ifndef US_MWL_PLTCNTRL_H
#define US_MWL_PLTCNTRL_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_plot3d_xyz.h"
#include "us_plot.h"
#include "us_help.h"

//! \brief A class to provide a window with enhanced plot controls

class US_MwlPlotControl : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! \brief US_MwlPlotControl constructor
      //! \param p  Pointer to the parent of this widget
      //! \param d  Pointer to the xyz data to be plotted
      US_MwlPlotControl( QWidget* p = 0, QVector< QVector3D >* = 0 );

      //! \brief Function to return US_MwlPlotControl's caller
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

      QwtCounter*   ct_gridreso;
      QwtCounter*   ct_peaksmoo;
      QwtCounter*   ct_peakwidth;
      QwtCounter*   ct_zscalefac;
      QwtCounter*   ct_rxscale;
      QwtCounter*   ct_ryscale;

      QCheckBox*    ck_yrevrs;

      QVector< QCheckBox* >    xCheck;
      QVector< QCheckBox* >    yCheck;

      QWidget*                 wparent;
      QPointer< US_Plot3Dxyz > plot3d_w;
      QVector< QVector3D >*    xyzdat;

      double zscale;
      double rxscale;
      double ryscale;
      double gridres;
      double pksmooth;
      double pkwidth;

   protected:
      US_Help       showHelp;

   private slots:
      void xmwtCheck( bool );
      void ymwtCheck( bool );
      void xsedCheck( bool );
      void ysedCheck( bool );
      void xdifCheck( bool );
      void ydifCheck( bool );
      void xfcoCheck( bool );
      void yfcoCheck( bool );
      void xfraCheck( bool );
      void yfraCheck( bool );
      void xvbaCheck( bool );
      void yvbaCheck( bool );
      void checkSet(  bool, bool, int );
      void zscal_value( double );
      void gridr_value( double );
      void peaks_value( double );
      void peakw_value( double );
      void plot3_btn( void );
      void close_all( void );
      int  dimensionType( QVector< QCheckBox* >& );

      void help     ( void )
      { showHelp.show_help( "mwlr_view_pltctrl.html" ); };
};
#endif

