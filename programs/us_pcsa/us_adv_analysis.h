//! \file us_adv_analysis.h
#ifndef US_ADV_ANALYSIS_H
#define US_ADV_ANALYSIS_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_pcsa_process.h"
#include "us_model_record.h"
#include "us_plot.h"
#include "us_help.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

//! \brief A class to provide a window for advanced analysis controls

class US_AdvAnalysis : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! \brief US_AdvAnalysis constructor
      //! \param sim_par Pointer to simulation parameters
      //! \param         Pointer to the parent of this widget
      US_AdvAnalysis( QVector< ModelRecord >*, const int,
                      US_SolveSim::DataSet*, QWidget* p = 0 );

      //! \brief Return flag of advanced analysis state and possibly MC models
      //! \param p_mrecsmc  Pointer for return of MC model records if appropo
      //! \return           Flag with or'd state (1=new-bfm, 2=new-mrs, 4=mc)
      int advanced_results( QVector< ModelRecord >* );

   private:
      QVector< ModelRecord >*  p_mrecs;
      QVector< ModelRecord >   mrecs0;
      QVector< ModelRecord >   mrecs;
      QVector< ModelRecord >   mrecs_mc;

      ModelRecord              mrec0;
      ModelRecord              mrec;

      US_SolveSim::DataSet*    dset0;

      US_DataIO::EditedData*   edata;
      US_DataIO::EditedData    wdata;
      US_DataIO::RawData       sdata1;

      QVector< double >        sigmas;

      US_Model                 model;

      QVector< US_SolveSim::DataSet >  wkdsets;

      static const int         msk_bfnew = 1;
      static const int         msk_mrnew = 2;
      static const int         msk_mcarl = 4;

      int           nthr;
      int           mciters;
      int           kciters;
      int           ksiters;
      int           nmrecs;
      int           ctype;
      int           nisols;
      int           ncsols;
      int           dbg_level;

      bool          bfm0_exists;
      bool          mrs0_exists;
      bool          bfm_new;
      bool          mrs_new;
      bool          mc_done;
      bool          mc_running;

      QString       store_dir;

      QHBoxLayout*  mainLayout;
      QGridLayout*  finmodelLayout;
      QGridLayout*  mreclistLayout;

      QWidget*      parentw;

      QLabel*       lb_sigmpar1;
      QLabel*       lb_sigmpar2;
      QLabel*       lb_k_lower;
      QLabel*       lb_k_upper;

      QwtCounter*   ct_s_lower;
      QwtCounter*   ct_s_upper;
      QwtCounter*   ct_sigmpar1;
      QwtCounter*   ct_sigmpar2;
      QwtCounter*   ct_k_lower;
      QwtCounter*   ct_k_upper;
      QwtCounter*   ct_mciters;
      QwtCounter*   ct_crpoints;

      QPushButton*  pb_loadmrs;
      QPushButton*  pb_storemrs;
      QPushButton*  pb_loadbfm;
      QPushButton*  pb_storebfm;
      QPushButton*  pb_resetbfm;
      QPushButton*  pb_resetmrs;
      QPushButton*  pb_buildbfm;
      QPushButton*  pb_mciters;
      QPushButton*  pb_help;
      QPushButton*  pb_cancel;
      QPushButton*  pb_accept;

      QComboBox*    cb_curvtype;

      QTextEdit*    te_bfmstat;
      QTextEdit*    te_mrecstat;

   protected:
      US_Help       showHelp;
      QProgressBar* b_progress;

   private slots:
      void curvtypeChanged ( int );
      void slowerChanged   ( double );
      void supperChanged   ( double );
      void sipar1Changed   ( double );
      void sipar2Changed   ( double );
      void klowerChanged   ( double );
      void kupperChanged   ( double );
      void pointsChanged   ( double );
      void mciterChanged   ( double );
      void load_mrecs      ( void );
      void store_mrecs     ( void );
      void load_bfm        ( void );
      void store_bfm       ( void );
      void reset_bfm       ( void );
      void reset_mrecs     ( void );
      void build_bfm       ( void );
      void start_montecarlo( void );
      void set_gaussians   ( US_SolveSim::Simulation& );
      void apply_gaussians ( void );
      void process_job     ( WorkerThread* );
      void montecarlo_done ( void );
      void under_construct ( QString );
      void curve_points    ( int, double, double, double, double,
                             ModelRecord& );
      void bfm_model       ( void );

      void select          ( void );
      void cancel          ( void );
//      void load_model      ( void );

      void help     ( void )
      { showHelp.show_help( "pcsa_advanced.html" ); };
};
#endif


