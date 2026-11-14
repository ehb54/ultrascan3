//! \file us_resplot_fem.h
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

//! \brief The data a residuals plot dialog shows.
//!
//! The dialog keeps its own copy, so the caller may change or discard its data
//! while the dialog is open; set_inputs() shows new data.
struct US_ResidPlotInputs
{
   US_DataIO::EditedData       edited;      //!< Experimental data
   US_DataIO::RawData          simulated;   //!< Simulated data
   QList< int >                excluded;    //!< Excluded scan indexes
   US_Noise                    ti_noise;    //!< Time-invariant noise
   US_Noise                    ri_noise;    //!< Radially-invariant noise
   QPointer< US_ResidsBitmap > bitmap;      //!< Caller's residuals bitmap to reuse, if any
   QString                     triple_info; //!< Title suffix in Analysis mode
};

//! \brief A class to provide a window with experimental/residuals plots

class US_ResidPlotFem : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! \brief Which program the dialog is shown from; selects titles, sizes,
      //! visibility and residuals-bitmap behavior.
      enum Mode
      {
         Standalone, //!< US_FeMatch
         Analysis,   //!< Autoflow analysis
         Report      //!< GMP report generation (dialog stays hidden)
      };

      //! \param inputs The data to plot
      //! \param mode   The calling program's mode
      //! \param parent The calling widget, which owns the dialog. The dialog
      //!               deletes itself when closed.
      US_ResidPlotFem( const US_ResidPlotInputs& inputs, Mode mode, QWidget* parent = nullptr );

      //! \brief Stop reacting to the bitmap before child widgets are deleted
      ~US_ResidPlotFem() override;

      //! \brief Show new data, keeping the plot options that still apply
      //! \param inputs The data to plot; its bitmap is ignored
      void set_inputs( const US_ResidPlotInputs& inputs );

      //! \brief Force a specific plot in the lower plot
      //! \param plotf Flag plot to force: 1,2 == ti_noise,ri_noise
      void set_plot( int );

      //! \brief Return a QwtPlot pointer for the upper plot
      QwtPlot* rp_data_plot1();

      //! \brief Return a QwtPlot pointer for the lower plot
      QwtPlot* rp_data_plot2();

   private:
      Mode          mode;

      QHBoxLayout*  mainLayout;
      QVBoxLayout*  leftLayout;
      QVBoxLayout*  rightLayout;
      QGridLayout*  datctrlsLayout;
      QGridLayout*  resctrlsLayout;
      QVBoxLayout*  buttonsLayout;

      US_Plot*      plotLayout1;
      US_Plot*      plotLayout2;

      QwtPlot*      data_plot1;
      QwtPlot*      data_plot2;

      bool          have_ed;
      bool          have_sd;
      bool          have_ti;
      bool          have_ri;
      bool          have_bm;
      bool          skip_plot;

      int           dbg_level;

   protected:
      US_ResidPlotInputs          data_in;
      US_DataIO::EditedData*      edata;
      US_DataIO::Scan*            escan;
      QList< int >*               excllist;
      US_DataIO::RawData*         sdata;
      US_Noise*                   ri_noise;
      US_Noise*                   ti_noise;
      QPointer< US_ResidsBitmap > resbmap;

      QString     tripleInfo;

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
      void connect_pboxes( bool );

      void closeEvent      ( QCloseEvent* );

   signals:
      void on_close( void );
};
#endif

