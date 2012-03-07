//! \file us_intensity.h
#ifndef US_INTENSITY_H
#define US_INTENSITY_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_plot.h"

//! \brief A class to provide a detail plot of the intensity profile

class US_Intensity : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! \param runID   The runID of the experiment
      //! \param triple  The triple to place in the name of the saved file
      //! \param data    A reference to a list of doubles to plot 
      US_Intensity( const QString, const QString, const QVector< double >& );

   private:
      QwtPlot*     data_plot;
      const QVector< double >& dataIn;

      void         draw_plot    ( const QVector< double >& );

};
#endif
