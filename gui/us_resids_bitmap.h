//! \file us_resids_bitmap.h
#ifndef US_RESIDS_BITMAP_H
#define US_RESIDS_BITMAP_H

#include "us_dataIO.h"
#include "us_extern.h"
#include "us_help.h"
#include "us_plot.h"
#include "us_widgets_dialog.h"

//! \brief A class to provide a window with a residuals bitmap

class US_GUI_EXTERN US_ResidsBitmap : public US_WidgetsDialog {
      Q_OBJECT

   public:
      //! Constructor
      //! \param resids  A reference to a vector of scans,points residual values
      //! \param wparent Pointer to parent widget
      US_ResidsBitmap(QVector<QVector<double>> &, QWidget * = 0);

      //! Re-plot new residuals matrix to existing bitmap
      //! \param resids  A reference to a vector of scans,points residual values
      void replot(QVector<QVector<double>> &);

   private:
      QLabel *lb_bitmap;
};
#endif
