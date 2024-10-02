//! \file us_plot_control_fem.h
#ifndef US_PLOT_CONTROL_H
#define US_PLOT_CONTROL_H

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_resplot_fem.h"
#include "us_plot3d.h"
#include "us_model.h"
#include "us_plot.h"
#include "us_help.h"

//! \brief A class to provide a window with enhanced plot controls

class US_PlotControlFem : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! \brief US_PlotControlFem constructor
      //! \param p       Pointer to the parent of this widget
      //! \param amodel  Pointer to the model to be plotted
      US_PlotControlFem( QWidget* p = 0, US_Model* = 0 );

      //! \brief Public function to return US_PlotControlFem's caller (e.g., US_Fematch)
      //! \returns Pointer to the QWidget that created this plot control
      QWidget* caller( void );

      //! \brief Force (re-)plot of currently specified 3-D plot
      void do_3dplot( void );
      void do_3dplot_auto( void );
      
      //! \brief Return 3-d plot data widget pointer
      QGLWidget* data_3dplot( void );

      //! \brief Return 3-d plot main widget pointer
      US_Plot3D* widget_3dplot( void );

   private:
      QVBoxLayout*  mainLayout;
      QGridLayout*  controlsLayout;
      QHBoxLayout*  buttonsLayout;

      QwtCounter*   ct_rxscale;
      QwtCounter*   ct_ryscale;

      QVector< QCheckBox* >    xCheck;
      QVector< QCheckBox* >    yCheck;

      QWidget*                 wparent;
      QPointer< US_Plot3D    > plot3d_w;
      US_Model*                model;

      double zscale;
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
      void plot3_btn(  void );
      void plot3_btn_auto(  void );
      void close_all(  void );
      void plot_close( void );
      int  dimensionType( QVector< QCheckBox* >& );

      void help      ( void )
      { showHelp.show_help( "fe_match_controls.html" ); };
};
#endif

