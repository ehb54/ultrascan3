#ifndef US_DIRHIST_H
#define US_DIRHIST_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <q3table.h>
#include <qdatetime.h>
//Added by qt3to4:
#include <QCloseEvent>

#include "us_util.h"

//standard C and C++ defs:

#include <map>
#include <set>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using namespace std;

class US_EXTERN US_Dirhist : public QDialog
{
   Q_OBJECT

   public:
      friend class US_Hydrodyn;

      US_Dirhist(
                 QStringList                 & history,
                 map < QString, QDateTime >  & last_access,
                 map < QString, QString >    & last_filetype,
                 QString                     & selected,
                 bool                        & is_ok,
                 QWidget                     * p = 0, 
                 const char                  * name = 0
                 );
      ~US_Dirhist();

   private:

      US_Config                  * USglobal;

      QLabel                     * lbl_info;

      Q3Table                     * t_hist;

      QPushButton                * pb_del;

      QPushButton                * pb_help;
      QPushButton                * pb_ok;
      QPushButton                * pb_cancel;

      void                         setupGUI();

      QStringList                * history;
      map < QString, QDateTime > * last_access;
      map < QString, QString >   * last_filetype;
      QString                    * selected;
      bool                       * is_ok;

      bool                         order_ascending;

   private slots:

      void t_selectionChanged();
      void t_sort_column  ( int col );
      void t_doubleClicked( int row, int col, int button, const QPoint & mousePos );

      void del();
      void ok();

      void cancel();
      void help();
   
   protected slots:

      void closeEvent(QCloseEvent *);
};

#endif
