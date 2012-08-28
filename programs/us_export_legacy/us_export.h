#ifndef US_EXPORT_LEGACY_H
#define US_EXPORT_LEGACY_H

#include <QtGui>

#include "us_widgets.h"
#include "us_plot.h"
#include "us_dataIO2.h"
#include "us_db2.h"
#include "us_editor.h"
#include "us_math2.h"
#include "us_help.h"
#include "us_run_details2.h"
#include <qwt_plot.h>
#include <qwt_plot_marker.h>

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

class US_ExportLegacy : public US_Widgets
{
   Q_OBJECT

   public:
      US_ExportLegacy();

   private:

      QLineEdit*    le_id;
      QLineEdit*    le_temp;

      QTextEdit*    te_desc;
      QTextEdit*    te_stat;

      QPushButton*  pb_load;
      QPushButton*  pb_details;
      QPushButton*  pb_view;
      QPushButton*  pb_save;
      QPushButton*  pb_reset;
      QPushButton*  pb_help;
      QPushButton*  pb_close;

      QListWidget*  lw_triples;

      int           scanCount;
      int           valueCount;
      int           dbg_level;
      int           nexport;

      bool          dataLoaded;

      QString       run_name;
      QString       workingDir;
      QString       runID;

      QStringList   files;

      US_DataIO2::EditedData*  edata;
      US_DataIO2::RawData*     rdata;
      US_DataIO2::Scan*        dscan;

      US_Disk_DB_Controls*     dkdb_cntrls;

   private slots:

      void load       ( void );
      void data_plot  ( void );
      void export_data( void );
      void details    ( void );
      void reset      ( void );
      void update     ( int  );

      void    exp_mosttypes   ( QStringList& );
      void    exp_intensity   ( QStringList& );
      void    exp_interference( QStringList& );
      void    write_report    ( QTextStream& );
      void    update_disk_db  ( bool );
      bool    mkdir           ( const QString&, const QString& );
      void    new_triple      ( int );
      QString indent          ( int ) const;
      void    view_report     ( void );
      QString table_row       ( const QString&, const QString& ) const;
      QString html_header     ( QString, QString, US_DataIO2::EditedData* );
      QString data_details    ( void )                      const;
      void help               ( void )
      { showHelp.show_help( "export_legacy.html" ); };

   protected:
      QStringList                       triples;
      QVector< US_DataIO2::EditedData > dataList;
      QVector< US_DataIO2::RawData    > rawList;
      QVector< int >                    extrips;

      US_Help      showHelp;

      // Layouts
      QBoxLayout*  mainLayout;
      QBoxLayout*  leftLayout;
      QBoxLayout*  rightLayout;
      QBoxLayout*  buttonLayout;

      QGridLayout* analysisLayout;
      QGridLayout* runInfoLayout;

      US_Plot*     plotLayout2;  // Derived from QVBoxLayout

      // Widgets
      QwtPlot*     data_plot2;
};
#endif
