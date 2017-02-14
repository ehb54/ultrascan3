#ifndef US_RI_NOISE_H
#define US_RI_NOISE_H

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_dataIO.h"
#include "us_plot.h"

#include <qwt_counter.h>

class US_RiNoise : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      US_RiNoise( const US_DataIO::RawData&,
                  const QList< int >&,
                  double,
                  double,
                  const QString&,
                  int&,
                  QList< double >& );

      static void calc_residuals( const US_DataIO::RawData&,
                                  const QList< int >&,
                                  double,
                                  double,
                                  int,
                                  QList< double >& );

   private:
      const US_DataIO::RawData&  data;
      const QList< int >&        includes;
      double                     range_left;
      double                     range_right;
      int&                       order;
      QList< double >&           residuals;

      QTextEdit*       te_details;

      QwtPlot*         data_plot;
      QwtPlotCurve*    curve;
      QwtCounter*      ct_order;

   private slots:
      void draw_fit( double );
};
#endif
