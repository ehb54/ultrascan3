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

      class DistrDesc
      {
         public:
            QString runID;
            QString triple;
            QList< double > dseds;
            QList< double > bfracs;
            QColor     color;
            QwtSymbol* symbol;
      };

      QList< DistrDesc >  distros;
      QList< DistrDesc >  pdistrs;

      QStringList    distIDs;
      QStringList    pdisIDs;

      QLineEdit*     le_runid;
      QLineEdit*     le_distname;

      QListWidget*   lw_runids;
      QListWidget*   lw_triples;

      QCheckBox*     ck_distrib;
      QCheckBox*     ck_envelope;

      US_Help        showHelp;
 
      QwtPlot*       data_plot1;
      QwtPlotCurve*  curve;
      QwtPlotCurve*  dcurve;
      US_PlotPicker* gpick;

      QString        runID;
      QString        triple;

      int            dbg_level;

   private slots:

      void load(      void );
      void save(      void );
      void reset(     void );
      void data_plot( void );

      void help(      void )
      { showHelp.show_help( "vhw_combine.html" ); };

};
#endif
