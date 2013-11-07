#ifndef US_PCSA_H
#define US_PCSA_H

#include <unistd.h>
#include "us_plot_control.h"
#include "us_analysis_control.h"
#include "us_noise_loader.h"
#include "us_resplot.h"
#include "us_mlplot.h"
#include "us_model_record.h"
#include "us_db2.h"
#include "us_astfem_rsa.h"
#include "us_model.h"
#include "us_noise.h"
//#include "us_model_loader.h"
#include "us_editor.h"
#include "us_math2.h"
#include "us_run_details2.h"
#include "us_buffer_gui.h"
#include "us_analyte.h"
#include "qwt_plot_marker.h"
#include "us_analysis_base2.h"
#ifdef Q_WS_WIN         // Include headers so getpid() works on Windows
#include <windows.h>
#include <psapi.h>
#include <process.h>
#endif

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

class US_pcsa : public US_AnalysisBase2
{
   Q_OBJECT

   public:
      US_pcsa();

      void analysis_done( int );

      US_DataIO::EditedData*      mw_editdata();
      US_DataIO::RawData*         mw_simdata();
      US_DataIO::RawData*         mw_resdata();
      US_Model*                   mw_model();
      US_Noise*                   mw_ti_noise();
      US_Noise*                   mw_ri_noise();
      QPointer< QTextEdit >       mw_status_text();
      QStringList*                mw_model_stats();
      QVector< ModelRecord >*     mw_mrecs();
      QVector< ModelRecord >*     mw_mrecs_mc();
      int*                        mw_base_rss();

   private:
      QGridLayout*         progressLayout;

      US_Editor*           te_results;

      US_SolveSim::DataSet            dset;

      QList< US_SolveSim::DataSet* >  dsets;

      US_DataIO::EditedData*          edata;
      US_DataIO::RawData              sdata;
      US_DataIO::RawData              rdata;

      QPointer< US_ResidPlot >        resplotd;
      QPointer< US_PlotControl >      eplotcd;
      QPointer< US_AnalysisControl >  analcd;
      QPointer< US_MLinesPlot >       mlplotd;

      QVector< ModelRecord >          mrecs;
      QVector< ModelRecord >          mrecs_mc;

      US_Model             model;
      US_Noise             ri_noise_in;
      US_Noise             ti_noise_in;

      QList< US_Model >    models;
      QList< US_Noise >    rinoises;
      QList< US_Noise >    tinoises;

      QStringList          model_stats;

      QPoint               rbd_pos;
      QPoint               epd_pos;
      QPoint               acd_pos;

      QLineEdit*           le_vari;
      QLineEdit*           le_rmsd;

      QTextEdit*           te_status;

      QPushButton*         pb_fitcntl;
      QPushButton*         pb_plt3d;
      QPushButton*         pb_pltres;

      double               rmsd;

      int                  dbg_level;
      int                  mc_iters;
      int                  baserss;

   private slots:
      void open_resplot( void );
      void open_3dplot(  void );
      void open_fitcntl( void );
      QString model_statistics( void );
      QString distrib_info( void );
      void data_plot      ( void );
      void write_report   ( QTextStream& );
      void write_bmap     ( const QString );
      void child_closed   ( QObject* );
      void load     ( void );
      void view     ( void );
      void save     ( void );
      void close    ( void );
      void new_triple( int );
      void help     ( void )
      { showHelp.show_help( "manual/pcsa.html" ); };
};
#endif
