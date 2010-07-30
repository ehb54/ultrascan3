#ifndef US_LOAD_DB_H
#define US_LOAD_DB_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_dataIO2.h"

class US_EXTERN US_LoadDB : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      US_LoadDB( QString&);
      
   private:
      int          personID;
      QTreeWidget* tree;
      QString&     workingDir;
      QLineEdit*   le_investigator;

      QMap< QString, QTreeWidgetItem* > runIDs;
      QMap< QString, QStringList      > dbIDs;
      QMap< QString, QStringList      > filenames;

      void populate_tree      ( void );       

   private slots:
      void load               ( void );
      void sel_investigator   ( void );
      void assign_investigator( int, const QString&, const QString& );
};
#endif
