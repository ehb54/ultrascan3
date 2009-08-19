#ifndef US_RI_NOISE_H
#define US_RI_NOISE_H

#include <QtGui>
#include <qwt_counter.h>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_dataIO.h"
#include "us_plot.h"

#include "qwt_counter.h"

class US_EXTERN US_RiNoise : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      US_RiNoise( const rawData&, int&, QList< double >& );

   signals:

   private:
      const rawData&   data;
      int&             order;

      QList< double >& residuals;

      QTextEdit*       te_details;
                      
      QwtCounter*      ct_order;
                      
      QwtPlot*         data_plot;
      QwtPlotCurve*    curve;

   private slots:
      void ok      ( void );
      void draw_fit( double );
};
#endif
