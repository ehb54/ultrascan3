#ifndef US_FIT_MENISCUS_H
#define US_FIT_MENISCUS_H

#include <QtGui>

#include "qwt_plot_marker.h"

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_editor.h"
#include "us_plot.h"

#define DbgLv(a) if(dbg_level>=a)qDebug()  //!< dbg_level-conditioned qDebug()

class US_FitMeniscus : public US_Widgets
{
   Q_OBJECT

   public:
      US_FitMeniscus();

   private:
      QLineEdit*           le_fit;
      QLineEdit*           le_rms_error;
      QLineEdit*           le_status;

      US_Help              showHelp;
      
      US_Editor*           te_data;

      QPushButton*         pb_update;
      QPushButton*         pb_scandb;
      QPushButton*         pb_plot;
      QPushButton*         pb_reset;

      QSpinBox*            sb_order;

      QwtPlot*             meniscus_plot;
      QwtPlotCurve*        raw_curve;
      QwtPlotCurve*        fit_curve;
      QwtPlotCurve*        minimum_curve;
 
      US_Disk_DB_Controls* dkdb_cntrls;

      QString              filedir;
      QString              fname_load;
      QString              fname_edit;

      int                  dbg_level;
   private slots:
      void reset    (      void );
      void plot_data(      void );
      void plot_data(      int );
      void edit_update(    void );
      void scan_dbase(     void );
      void file_loaded(    QString );
      void update_disk_db( bool );
      void update_db_edit( QString, QString, QString& );
      void remove_models(  void );
      void noises_in_edit( QString, QStringList&,
                           QStringList&, QStringList& );

      void help     ( void )
      { showHelp.show_help( "manual/fit_meniscus.html" ); };
};
#endif
