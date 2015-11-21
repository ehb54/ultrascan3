//! \file us_edit_scan.cpp
#ifndef US_EDIT_SCAN_H
#define US_EDIT_SCAN_H

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_help.h"
#include "us_dataIO.h"
#include "us_plot.h"

class US_EditScan : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      US_EditScan( US_DataIO::Scan&, const QVector< double >&, 
                   double, double, double );

   signals:
      void scan_updated( QList< QPointF > );

   private:
      US_DataIO::Scan    workingScan;
      US_DataIO::Scan&   originalScan;
      QVector< double >  allRadii;

      double           invert;
      double           range_left;
      double           range_right;
      double*          radii;
      double*          values;
      bool             dragging;
      int              point;
      int              offset;
      QList< QPointF > changes;

      QwtPlot*         data_plot;
      QwtPlotCurve*    curve;
      US_PlotPicker*   pick;
      QwtSymbol        fgSym;
      QwtSymbol        bgSym;
      QPen             bgPen;
      QPen             fgPen;
                      
      US_Help          showHelp;

      void             redraw( void );

   private slots:
      void done        ( void );
      void reset       ( void );
      void start_drag  ( QMouseEvent* );
      void end_drag    ( const QwtDoublePoint& );
      void drag        ( const QwtDoublePoint& );

      void help        ( void )
      { showHelp.show_help( "manual/edit_scan.html" ); };
};
#endif
