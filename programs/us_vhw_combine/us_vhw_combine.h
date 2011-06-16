#ifndef US_VHW_COMBO_H
#define US_VHW_COMBO_H

#include "us_editor.h"
#include "us_math2.h"
#include "us_run_details2.h"
#include "us_buffer_gui.h"
#include "qwt_plot_marker.h"

#define PA_POINTS 20     // plateau average points to left and right

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug() //!< debug-level-conditioned qDebug()
#endif

class US_vHW_Combine : public US_Widgets
{
   Q_OBJECT

   public:
      US_vHW_Combine();

   private:

      // Distribution description object
      class DistrDesc
      {
         public:
            QString            runID;    // Run ID
            QString            triple;   // Triple
            QVector< double >  dsedcs;   // Distribution sedimentation coeffs.
            QVector< double >  bfracs;   // Boundary fractions
            QVector< double >  esedcs;   // Envelope sedimentation coeffs.
            QVector< double >  efreqs;   // Envelope frequencies
            QwtSymbol          symbol;   // Curve symbol
            QColor             color;    // Curve color
      };

      QList< DistrDesc >  distros;     // All distributions
      QList< DistrDesc >  pdistrs;     // Plotted distributions

      QList< int >        symbols;     // Possible symbol shapes
      QList< QColor >     colors;      // Possible colors

      QStringList    distIDs;          // All distribs. IDs
      QStringList    pdisIDs;          // Plotted distribs. IDs

      QLineEdit*     le_runid;
      QLineEdit*     le_distname;

      QListWidget*   lw_runids;
      QListWidget*   lw_triples;

      QCheckBox*     ck_distrib;
      QCheckBox*     ck_envelope;

      US_Help        showHelp;
 
      QwtPlot*       data_plot1;
      US_PlotPicker* gpick;

      QString        runID;
      QString        triple;

      int            dbg_level;

   private slots:

      void load           ( void );
      void save           ( void );
      void reset_data     ( void );
      void reset_plot     ( void );
      void plot_data      ( void );
      void plot_distr     ( DistrDesc, QString );
      void runid_select   ( int );
      void triple_select  ( int );
      void setSymbol      ( DistrDesc&, int );
      int  envel_data     ( DistrDesc& );
      void possibleSymbols( void );
      QString expandedTriple ( QString );
      QString collapsedTriple( QString );

      void help(      void )
      { showHelp.show_help( "vhw_combine.html" ); };

};
#endif
