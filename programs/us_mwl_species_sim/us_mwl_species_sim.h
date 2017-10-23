#ifndef US_MWL_SPECIES_SIM_H
#define US_MWL_SPECIES_SIM_H

#include "us_extern.h"
#include "us_simparms.h"
#include "us_simparams.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_simparms.h"
#include "us_rotor_gui.h"
#include "us_buffer_gui.h"
#include "us_model_loader.h"
#include "us_plot.h"
#include "us_colorgradIO.h"
#include "us_spectrodata.h"

#include "qwt_plot_marker.h"
#include "qwt_plot_spectrogram.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_zoomer.h"
#include "qwt_plot_panner.h"
#include "qwt_scale_widget.h"
#include "qwt_scale_draw.h"
#include "qwt_color_map.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

class US_MwlSpeciesSim : public US_Widgets
{
   Q_OBJECT

   public:
      US_MwlSpeciesSim();

   private:
      int           dbg_level;
      int           nmodels;
      int           tripx;
      int           npoint;
      int           nscan;

      bool          dbload;
      bool          stopFlag;

      double        curr_meniscus;
      double        curr_bottom;

      QString       mfilt;
      QString       mrunid;
      QString       orunid;

      QVector< double >  mtconcs;

      QVector< US_DataIO::RawData > synData;
      QList< US_Model >             models;
      QVector< bool >               have_p1;

      US_DataIO::RawData            rdata0;
      US_SimulationParameters       simparams;
      US_SimulationParameters       working_simparams;
      US_Rotor::Rotor               rotor;
      US_Rotor::RotorCalibration    rotor_calib;
      US_Buffer                     buffer;

      QStringList   pfilts;
      QStringList   mdescs;

      US_Help       showHelp;

      QwtPlot*      data_plot1;

      QPushButton*  pb_prefilt;
      QPushButton*  pb_semodels;
      QPushButton*  pb_defbuff;
      QPushButton*  pb_simparms;
      QPushButton*  pb_selrotor;
      QPushButton*  pb_strtsims;
      QPushButton*  pb_stopsims;
      QPushButton*  pb_savesims;
      QPushButton*  pb_prev;
      QPushButton*  pb_next;
      QPushButton*  pb_help;
      QPushButton*  pb_close;

      QLineEdit*    le_runid;
      QLineEdit*    le_triples;

      QTextEdit*    te_status;

      void data_plot ( void );

   private slots:
      void pre_filt      ( void );
      void select_models ( void );
      void define_buffer ( void );
      void sim_params    ( void );
      void select_rotor  ( void );
      void start_sims    ( void );
      void stop_sims     ( void );
      void save_sims     ( void );
      void prev_plot     ( void );
      void next_plot     ( void );
      void close_all     ( void );
      void init_simparams( void );
      void set_parameters( void );
      void assign_rotor  ( US_Rotor::Rotor&,
                           US_Rotor::RotorCalibration& );
      void change_buffer ( US_Buffer );
      void init_rawdata  ( void );
      void build_rawdata ( void );
      void plot_data1    ( void );
      int  writeTimeState( const QString&, US_SimulationParameters&,
                           US_DataIO::RawData& );
      void help          ( void )
      { showHelp.show_help( "manual/mwl_species_sim.html" ); };
};
#endif

