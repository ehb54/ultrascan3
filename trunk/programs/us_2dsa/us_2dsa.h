#ifndef US_2DSA_H
#define US_2DSA_H

#include "us_plot_control.h"
#include "us_analysis_control.h"
#include "us_noise_loader.h"
#include "us_resplot.h"
#include "us_dataIO2.h"
#include "us_db2.h"
#include "us_astfem_rsa.h"
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

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

class US_2dsa : public US_AnalysisBase2
{
   Q_OBJECT

   public:
      US_2dsa();

      void analysis_done( int );

      US_DataIO2::EditedData*     mw_editdata();
      US_DataIO2::RawData*        mw_simdata();
      US_DataIO2::RawData*        mw_resdata();
      US_Model*                   mw_model();
      US_Noise*                   mw_ti_noise();
      US_Noise*                   mw_ri_noise();
      QPointer< QTextEdit >       mw_status_text();

   private:
      QGridLayout*         progressLayout;

      US_Editor*           te_results;

      US_DataIO2::EditedData*     edata;
      US_DataIO2::RawData         sdata;
      US_DataIO2::RawData         rdata;

      QPointer< US_ResidPlot >        resplotd;
      QPointer< US_PlotControl >      eplotcd;
      QPointer< US_AnalysisControl >  analcd;

      US_Model             model;
      US_Noise             ri_noise_in;
      US_Noise             ti_noise_in;

      QList< US_Model >    models;
      QList< US_Noise >    rinoises;
      QList< US_Noise >    tinoises;

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

   private slots:
      void open_resplot( void );
      void open_3dplot(  void );
      void open_fitcntl( void );
      QString distrib_info(   void );
      QString iteration_info( void );
      void data_plot( void );
      void write_report( QTextStream& );
      void write_svg(    const QString, const QwtPlot* );
      void write_png(    const QString, const QWidget* );
      void load     ( void );
      void view     ( void );
      void save     ( void );
      void help     ( void )
      { showHelp.show_help( "manual/2dsa.html" ); };
};
#endif
