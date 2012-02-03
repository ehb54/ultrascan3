#ifndef US_COMBMODEL_H
#define US_COMBMODEL_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets.h"
#include "us_db2.h"
#include "us_help.h"
#include "us_model.h"
#include "us_settings.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

class US_CombineModels : public US_Widgets
{
   Q_OBJECT

   public:
      US_CombineModels();

   private:

      QList< US_Model >   models;      // List of selected models

      QStringList         mdescs;      // List of descriptions of models

      US_Help       showHelp;

      US_DB2*       dbP;

      QPushButton*  pb_add;
      QPushButton*  pb_reset;
      QPushButton*  pb_help;
      QPushButton*  pb_close;
      QPushButton*  pb_save;

      US_Disk_DB_Controls* dkdb_cntrls;

      QListWidget*  lw_models;

      int           ntrows;
      int           dbg_level;

      bool          rbtn_click;
      bool          changed;

      QString       mfilter;
      QString       run_name;
      QString       cmodel_name;


   private slots:

      void add_models    ( void );
      void reset         ( void );
      void save          ( void );
      void update_disk_db( bool );

      void help          ( void )
      { showHelp.show_help( "combine_models.html" ); };

   protected:
      bool eventFilter( QObject*, QEvent* );
};
#endif
