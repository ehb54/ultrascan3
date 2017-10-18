#ifndef US_MWL_SPECIES_FIT_H
#define US_MWL_SPECIES_FIT_H

#include "us_analysis_base2.h"
#include "us_editor.h"
#include "us_math2.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

class US_MwlSpeciesFit : public US_AnalysisBase2
{
   Q_OBJECT

   public:
      US_MwlSpeciesFit();

   private:
      int           dbg_level;
      int           nspecies;
      int           jspec;
      int           lmbxs;
      int           lmbxe;
      int           trpxs;
      int           trpxe;
      int           radxs;
      int           radxe;
      int           kradii;
      int           kscan;
      int           klambda;

      QVector< double >  radii;
      QVector< double >  spconcs;
      QVector< int >     lambdas;
      QVector< int >     spwavls;
      QVector< int >     nwavls;

      QVector< US_DataIO::RawData > synData;
      QVector< bool >               have_p1;

      QStringList   spfiles;
      QStringList   celchns;
      QList< int >  ftndxs;
      QList< int >  ltndxs;

      US_Editor*    te_results;

      QPushButton*  pb_loadsfit;
      QPushButton*  pb_sfitdata;
      QPushButton*  pb_prev;
      QPushButton*  pb_next;

      void data_plot ( void );

   private slots:
      void write_report   ( QTextStream& );
      void exclude        ( void );
      void load           ( void );
      void view           ( void );
      void save           ( void );
      void loadSpecs      ( void );
      void specFitData    ( void );
      void new_triple     ( const int );
      int  triple_index   ( const int );
      void apply_noise    ( const int, US_Noise&, US_Noise& );
      void plot_data2     ( void );
      void plot_data1     ( void );
      void prev_plot      ( void );
      void next_plot      ( void );
      void help           ( void )
      { showHelp.show_help( "manual/mwl_species_fit.html" ); };
};
#endif
