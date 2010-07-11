//! \file us_plot_control.h
#ifndef US_PLOT_CONTROL_H
#define US_PLOT_CONTROL_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_resplot.h"
#include "us_dataIO2.h"
#include "us_model.h"
#include "us_plot.h"
#include "us_help.h"

//! \brief A class to provide a window with enhanced plot controls

class US_EXTERN US_PlotControl : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! \param resids A reference to a vector of scans,points residual values
      US_PlotControl( QWidget* p = 0, US_Model* = 0 );

   private:
      QVBoxLayout*  mainLayout;
      QGridLayout*  controlsLayout;
      QHBoxLayout*  plotbtnLayout;
      QHBoxLayout*  buttonsLayout;

      QVector< QCheckBox* >    xCheck;
      QVector< QCheckBox* >    yCheck;

      QWidget*                 wparent;
      QPointer< US_ResidPlot > resplotd;
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
      void checkSet(  bool, bool, int );
      void zscal_value( double );
      void gridr_value( double );
      void peaks_value( double );
      void peakw_value( double );
      void rplot_btn( void );
      void plot3_btn( void );
      void close_all( void );
      int  dimensionType( QVector< QCheckBox* >& );

      void help     ( void )
      { showHelp.show_help( "plot_control.html" ); };
};
#endif

