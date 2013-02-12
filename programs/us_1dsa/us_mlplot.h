//! \file us_mlplot.h
#ifndef US_MLINES_PLOT_H
#define US_MLINES_PLOT_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_dataIO2.h"
#include "us_plot.h"
#include "us_resids_bitmap.h"
#include "us_noise.h"
#include "us_plot.h"
#include "us_model.h"
#include "us_model_record.h"
#include "us_help.h"

//! \brief A class to provide a window with a model lines plot

class US_MLinesPlot : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! \param parent A pointer to the parent widget of this one
      US_MLinesPlot( double&, double&, double&, double&, double&,
                     int&, int&, int&, int& );

      void setModel ( US_Model*, QVector< ModelRecord >& );
      void plot_data( void );

   private:
      double&       fmin;
      double&       fmax;
      double&       finc;
      double&       smin;
      double&       smax;

      int&          nlpts;
      int&          bmndx;
      int&          nkpts;
      int&          ctype;

      US_Plot*      plotLayout1;

      QwtPlot*      data_plot1;

      QHBoxLayout*  mainLayout;
      QVBoxLayout*  leftLayout;
      QVBoxLayout*  rightLayout;
      QGridLayout*  pltctrlsLayout;
      QVBoxLayout*  buttonsLayout;

      QLabel*       lb_lefact;
      QLabel*       lb_mefact;
      QLabel*       lb_hefact;
      QLabel*       lb_mpfact;

      QwtCounter*   ct_lefact;
      QwtCounter*   ct_mefact;
      QwtCounter*   ct_hefact;
      QwtCounter*   ct_mpfact;

      QLineEdit*    le_mtype;
      QLineEdit*    le_nlines;
      QLineEdit*    le_npoints;
      QLineEdit*    le_kincr;

      QVector< ModelRecord > mrecs;

      US_Model*              model;

      int           dbg_level;

      double        le_fact;
      double        me_fact;
      double        he_fact;
      double        mp_fact;
      double        low_elite;
      double        mid_elite;
      double        high_elite;
      double        mid_poor;
      double        best_rmsd;
      double        worst_rmsd;

   private slots:
      void close_all   ( void );
      void updateLeFact( double );
      void updateMeFact( double );
      void updateHeFact( double );
      void updateMpFact( double );
};
#endif

