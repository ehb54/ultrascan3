#ifndef US_RI_NOISE_H
#define US_RI_NOISE_H

#include <QtGui>
#include <qwt_counter.h>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_dataIO.h"
#include "us_plot.h"

class US_EXTERN US_RiNoise : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      US_RiNoise( const US_DataIO::rawData&, int&, QList< double >& );

   private:
      const US_DataIO::rawData& data;

      int&             order;

      QList< double >& residuals;
      QTextEdit*       te_details;
                      
      QwtPlot*         data_plot;
      QwtPlotCurve*    curve;
      QwtCounter*      ct_order;

   private slots:
      void draw_fit( double );
};
#endif
