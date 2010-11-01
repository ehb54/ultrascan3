#ifndef US_2DSA_H
#define US_2DSA_H

#include "us_plot_control.h"
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

      US_DataIO2::EditedData*     mw_editdata();
      US_DataIO2::RawData*        mw_simdata();
      US_Model*                   mw_model();
      US_Noise*                   mw_ti_noise();
      US_Noise*                   mw_ri_noise();

   private:
      QGridLayout*         progressLayout;

      US_Editor*           te_results;

      US_DataIO2::EditedData*     edata;
      US_DataIO2::Scan*           dscan;
      US_DataIO2::RawData*        rdata;
      US_DataIO2::RawData*        sdata;

      QPointer< US_ResidPlot >    resplotd;
      QPointer< US_PlotControl >  eplotcd;

      US_Model             model;
      US_Model             model_loaded;
      US_Noise             ri_noise;
      US_Noise             ti_noise;

      QPoint               rbd_pos;
      QPoint               epd_pos;

      QTextEdit*           te_status;
      QProgressBar*        b_progress;

   private slots:
      void open_resplot( void );
      void open_3dplot(  void );
      void data_plot( void );
      void view     ( void );
      void save     ( void );
      void help     ( void )
      { showHelp.show_help( "manual/us_2dsa.html" ); };
};
#endif
