#ifndef US_2DSA_H
#define US_2DSA_H

#include "us_plot_control_2d.h"
#include "us_analysis_control_2d.h"
#include "us_noise_loader.h"
#include "us_resplot_2d.h"
#include "us_dataIO.h"
#include "us_db2.h"
#include "us_solve_sim.h"
#include "us_model.h"
#include "us_noise.h"
#include "us_model_loader.h"
#include "us_editor.h"
#include "us_math2.h"
#include "us_run_details2.h"
#include "us_buffer_gui.h"
#include "us_analyte.h"
#include "qwt_plot_marker.h"
#include "us_analysis_base2.h"
#include "us_show_norm.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

#ifndef SP_SPEEDPROFILE
#define SP_SPEEDPROFILE US_SimulationParameters::SpeedProfile
#endif
#ifndef SS_DATASET
#define SS_DATASET US_SolveSim::DataSet
#endif

class US_2dsa : public US_AnalysisBase2
{
   Q_OBJECT

   public:
      US_2dsa();

      void analysis_done( int );
      void resplot_done( void );
      US_Math_BF::Band_Forming_Gradient* bfg;
      US_DataIO::EditedData*      mw_editdata();
      US_DataIO::RawData*         mw_simdata();
      US_DataIO::RawData*         mw_resdata();
      QList< int >*               mw_excllist();
      US_Model*                   mw_model();
      US_Noise*                   mw_ti_noise();
      US_Noise*                   mw_ri_noise();
      QVector<double>*            mw_Anorm() ;
      QPointer< QTextEdit >       mw_status_text();
      int*                        mw_base_rss();
      QString temp_Id_name() ;
   private:
      QGridLayout*         progressLayout;

      US_Editor*           te_results;

      SS_DATASET           dset;

      QList< SS_DATASET* >       dsets;

      QVector< SP_SPEEDPROFILE > speed_steps;
      QVector< double >          normvA  ;

      US_DataIO::EditedData*     edata;
      US_DataIO::RawData         sdata;
      US_DataIO::RawData         rdata;

      QPointer< US_ResidPlot2D >        resplotd;
      QPointer< US_PlotControl2D >      eplotcd;
      QPointer< US_AnalysisControl2D >  analcd;
      QPointer< US_show_norm >          analcd1;

      US_Model             model;
      US_Noise             ri_noise_in;
      US_Noise             ti_noise_in;

      QList< US_Model >    models;
      QList< US_Noise >    ri_noises;
      QList< US_Noise >    ti_noises;

      QPoint               rbd_pos;
      QPoint               epd_pos;
      QPoint               acd_pos;

      QLineEdit*           le_vari;
      QLineEdit*           le_rmsd;

      QTextEdit*           te_status;

      QPushButton*         pb_fitcntl;
      QPushButton*         pb_plt3d;
      QPushButton*         pb_pltres;
      QPushButton*         pb_shownorm;

      bool                 loadDB;
      bool                 exp_steps;

      double               rmsd;

      int                  dbg_level;
      int                  baserss;

   private slots:
      void open_resplot( void );
      void open_3dplot(  void );
      void open_fitcntl( void );
      QString distrib_info(      void );
      QString iteration_info(    void );
      QString fit_meniscus_data( void );
      void data_plot( void );
      void write_report( QTextStream& );
      void write_bmap( const QString );
      void load     ( void );
      void view     ( void );
      void save     ( void );
      void new_triple( int );
      void close_all( void );
      void help     ( void )
      { showHelp.show_help( "manual/2dsa.html" ); };
};
#endif
