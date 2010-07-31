#ifndef US_GET_EDIT_H
#define US_GET_EDIT_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"

class US_EXTERN US_GetEdit : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      US_GetEdit( int&, QStringList& );
      
   private:
      QListWidget* lw;
      int& index;

   private slots:
      void select( void );
};
#endif
