#ifndef US_TABLE_H
#define US_ANALYTE_H

#include <QtGui>

#include "us_widgets_dialog.h"
#include "us_extern.h"

class US_EXTERN US_Table : public US_WidgetsDialog
{
	Q_OBJECT

	public:
		US_Table( QMap< double, double >&, const QString&, 
            QWidget* = 0, Qt::WindowFlags = 0 );

   private:
      QMap< double, double >& values;
      QMap< double, double >  local;
      QListWidget*            lw_table;

      QLineEdit*              le_wl;
      QLineEdit*              le_value;

      void update    ( void );

   private slots:
      void done      ( void );
      void edited    ( void );
      void new_row   ( void );
      void delete_row( QListWidgetItem* );
};
#endif

