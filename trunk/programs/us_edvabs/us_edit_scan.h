//! \file us_edit_scan.cpp
#ifndef US_EDIT_SCAN_H
#define US_EDIT_SCAN_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_help.h"
#include "us_dataIO.h"
#include "us_plot.h"

class US_EXTERN US_EditScan : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      US_EditScan( scan&, const double );

   signals:
      void scan_updated( QList< QPointF > );

   private:
      scan             workingScan;
      scan&            originalScan;
      double           invert;
      double*          radii;
      double*          values;
      bool             dragging;
      int              point;
      QList< QPointF > changes;

      QwtPlot*         data_plot;
      QwtPlotCurve*    curve;
      US_PlotPicker*   pick;
      QwtSymbol        fgSym;
      QwtSymbol        bgSym;
      QPen             bgPen;
      QPen             fgPen;
                      
      US_Help          showHelp;

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
