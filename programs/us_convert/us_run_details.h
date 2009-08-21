#ifndef US_RUN_DETAILS_H
#define US_RUN_DETAILS_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_dataIO.h"
#include "us_plot.h"

class US_EXTERN US_RunDetails : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      US_RunDetails( const QList< rawData >&, int,
            const QString&, const QString&, const QStringList& );

   private:
      const QList< rawData >& dataList;
      const QStringList&      triples;

      QListWidget* lw_triples;

      QLabel*      lb_red;
      QLabel*      lb_green;

      QLineEdit*   le_avgTemp;
      QLineEdit*   le_runLen;
      QLineEdit*   le_timeCorr;
      QLineEdit*   le_rotorSpeed;

      QwtPlot*     data_plot;
      QwtPlotGrid* grid;

   private slots:
      void update( int );
};
#endif
