//! \file us_resplot.h
#ifndef US_RESID_PLOT_H
#define US_RESID_PLOT_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_dataIO.h"
#include "us_plot.h"
#include "us_resids_bitmap.h"
#include "us_noise.h"
#include "us_plot.h"
#include "us_help.h"

//! \brief A class to provide a window with experimental/residuals plots

class US_ResidPlot : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! \param parent A pointer to the parent widget of this one
      US_ResidPlot( QWidget* );

   private:
      QHBoxLayout*  mainLayout;
      QVBoxLayout*  leftLayout;
      QVBoxLayout*  rightLayout;
      QGridLayout*  datctrlsLayout;
      QGridLayout*  resctrlsLayout;
      QVBoxLayout*  buttonsLayout;

      US_Plot*      plotLayout1;
      US_Plot*      plotLayout2;

      bool          have_ed;
      bool          have_sd;
      bool          have_ti;
      bool          have_ri;
      bool          have_bm;
      bool          skip_plot;

      int           dbg_level;

   protected:
      QwtPlot*      data_plot1;
      QwtPlot*      data_plot2;

      US_DataIO::EditedData*      edata;
      US_DataIO::Scan*            escan;
      US_DataIO::RawData*         sdata;
      US_Noise*                   ri_noise;
      US_Noise*                   ti_noise;
      QPointer< US_ResidsBitmap > resbmap;

      QCheckBox*  ck_plteda;
      QCheckBox*  ck_subtin;
      QCheckBox*  ck_subrin;
      QCheckBox*  ck_pltsda;
      QCheckBox*  ck_addtin;
      QCheckBox*  ck_addrin;
      QCheckBox*  ck_pltres;
      QCheckBox*  ck_plttin;
      QCheckBox*  ck_pltrin;
      QCheckBox*  ck_pltran;
      QCheckBox*  ck_shorbm;

      QLineEdit*  le_vari;
      QLineEdit*  le_rmsd;

   private slots:
      void pedaCheck( bool );
      void stinCheck( bool );
      void srinCheck( bool );
      void psdaCheck( bool );
      void atinCheck( bool );
      void arinCheck( bool );
      void presCheck( bool );
      void ptinCheck( bool );
      void prinCheck( bool );
      void pranCheck( bool );
      void srbmCheck( bool );
      void close_all  ( void );
      void plot_data(   void );
      void plot_edata(  void );
      void plot_rdata(  void );
      void resids_closed(  void );
};
#endif

