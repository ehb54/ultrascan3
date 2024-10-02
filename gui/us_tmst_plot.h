//! \file us_tmst_plot.h
#ifndef US_TMST_PLOT_H
#define US_TMST_PLOT_H

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_plot.h"
#include "us_help.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()  //!< debug-level-conditioned qDebug()
#endif

//! \brief A class to provide a dialog to display Time State plots

class US_GUI_EXTERN US_TmstPlot : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! \brief TimeState Plot constructor
      //! \param parent  A pointer to the parent widget of this one
      //! \param tspath  TimeState (.tmst) full path file name
      US_TmstPlot( QWidget*, const QString );

      QStringList timestamp_data_dkeys ( );
      QVector< QVector< double > > timestamp_data_dvals( );
      QMap< QString, double > timestamp_data_mins ( );
      QMap< QString, double > timestamp_data_maxs ( );
      QMap< QString, double > timestamp_data_avgs ( );
      QMap < QString, QMap< QString, double >>  timestamp_data_avgs_stdd_first_scan ( );

      //! \brief Return a QwtPlot pointer for the upper plot
      QwtPlot* rp_data_plot1( QString );

   private:
      QwtPlot*      data_plot1;             //!< Upper plot (specific key)
      QwtPlot*      data_plot2;             //!< Lower plot (combined)

      QComboBox*    cb_pltkey;              //!< Plot (upper) key options
      QListWidget*  lw_datinfo;             //!< Summary information text
      QCheckBox*    ck_xscans;              //!< X-axis scans only checkbox
      QCheckBox*    ck_sstart;              //!< X start at scan 1 checkbox

      QStringList                   pkeys;  //!< Plot option keys
      QStringList                   dkeys;  //!< Data keys
      QVector< QVector< double > >  dvals;  //!< Data value vectors

      QMap< QString, double >       dmins;  //!< Data minima
      QMap< QString, double >       dmaxs;  //!< Data maxima
      QMap< QString, double >       davgs;  //!< Data averages

      QList< int >  scnxs;                  //!< Non-zero scan indexs in data
      QList< int >  pntxs;                  //!< Current plot point indexs in data

      QString       tmstpath;               //!< Full path to .tmst file
      QString       defvers;                //!< Definitions file version
      QString       imptype;                //!< Import type ("XLA"|"MWRS",...)

      int           dbg_level;              //!< Debug level
      int           nfkeys;                 //!< Number of file field keys
      int           ndkeys;                 //!< Number of data keys
      int           ntimes;                 //!< Number of time values
      int           nscans;                 //!< Number of non-0 scan numbers
      int           nplpts;                 //!< Number of plot points
      int           ssreso;                 //!< Set speed resolution

      bool          x_scans;                //!< Flag: X axis is scan numbers?
      bool          s_start;                //!< Flag: X axis starts at scan 1?

   private slots:
      void plot_data ( void );      //!< Create both plots
      void plot_kdata( void );      //!< Create key-specific plot
      void plot_cdata( void );      //!< Create combined plot
      void prev_plot ( void );      //!< Move to previous key plot
      void next_plot ( void );      //!< Move to next key plot
      void details   ( void );      //!< Display data details
      void read_data ( void );      //!< Read TimeState data
      QColor curve_color( const int );  //!< Curve color by key index
      int  point_indexes( void );   //!< Generate plot point indexes
};
#endif

