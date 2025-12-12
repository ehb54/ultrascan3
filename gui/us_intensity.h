//! \file us_intensity.h
#ifndef US_INTENSITY_H
#define US_INTENSITY_H

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_plot.h"

//! \brief A class to provide a detail plot of the intensity profile

class US_GUI_EXTERN US_Intensity : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! \param runID   The runID of the experiment
      //! \param triple  The triple to place in the name of the saved file
      //! \param data    A reference to a list of doubles to plot 
      //! \param scan    A reference to a list of scan number
      US_Intensity( const QString, const QString, const QVector< double >&,
                                   const QVector< double >& );

   private:
      QwtPlot*     data_plot;
		QLabel*      lbl_average;
		QLineEdit*   le_average;

      const QVector< double >& dataIn;
      const QVector< double >& scanIn;

      void         draw_plot    ( const QVector< double >&,
                                  const QVector< double >& );

};
#endif
