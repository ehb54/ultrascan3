//! \file us_select_item.h
#ifndef US_FAILED_GMP_RUN_GUI_H
#define US_FAILED_GMP_RUN_GUI_H

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_widgets.h"
#include "us_help.h"



//! \brief A dialog class 
class US_GUI_EXTERN  US_FailedRunGui: public US_Widgets
{
   Q_OBJECT

   public:

      US_FailedRunGui( QMap<QString, QString> );

   private:
      QComboBox*   cb_stage;
      QLineEdit*   le_description;
      QLineEdit*   le_name;

      QPushButton* pb_save;   
      QPushButton* pb_cancel;

      QString      autoflowID;

   private slots:
     void save_new     ( void );
     void cancel       ( void ); 

   signals:
     void failed_status_set();
};
#endif  
