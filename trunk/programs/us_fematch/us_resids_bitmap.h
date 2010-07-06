//! \file us_resids_bitmap.h
#ifndef US_RESIDS_BITMAP_H
#define US_RESIDS_BITMAP_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_dataIO2.h"
#include "us_plot.h"
#include "us_help.h"

//! \brief A class to provide a window with a residuals bitmap

class US_EXTERN US_ResidsBitmap : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! \param resids A reference to a vector of scans,points residual values
      US_ResidsBitmap( QVector< QVector< double > >& );

   private:
      QLabel*       lb_bitmap;
};
#endif
