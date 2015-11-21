#ifndef US_SELECT_TRIPLES_H
#define US_SELECT_LAMBDAS_H

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_help.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

class US_SelectTriples : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      US_SelectTriples( QStringList& );

   signals:
      void new_triple_list( QStringList );

   private:
      QStringList&   triples;

      QLineEdit*     le_original;
      QLineEdit*     le_selected;

      QListWidget*   lw_original;
      QListWidget*   lw_selected;

      QPushButton*   pb_add;
      QPushButton*   pb_remove;
      QPushButton*   pb_accept;

      int            dbg_level;
      int            nbr_orig;
      int            nbr_select;
      int            nbr_incl;
      int            nbr_excl;

      QStringList    original;
      QStringList    includes;
      QStringList    excludes;
      
      US_Help        showHelp;

   private slots:
      void add_selections( void );
      void rmv_selections( void );
      void cancel        ( void );
      void done          ( void );
      void reset         ( void );
      void help          ( void )
      { showHelp.show_help( "manual/convert-seltrip.html" ); };
};
#endif
