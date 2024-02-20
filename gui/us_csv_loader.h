#ifndef US_CSV_LOADER_H
#define US_CSV_LOADER_H


// #include "us_widgets.h"
#include "us_widgets_dialog.h"

class US_GUI_EXTERN US_CSV_Loader : public US_WidgetsDialog
{
   Q_OBJECT
   public:
      US_CSV_Loader(QWidget* parent);

   private:
      QPushButton* pb_open;
      QPushButton* pb_ok;
      QPushButton* pb_cancel;
      QRadioButton* rb_tab;
      QRadioButton* rb_comma;
      QRadioButton* rb_semicolon;
      QRadioButton* rb_space;
      QRadioButton* rb_other;
      QRadioButton* rb_string;
      QRadioButton* rb_numeric;
      QCheckBox* cb_header;
      QCheckBox* cb_samesize;
      QLineEdit* le_other;
      QLineEdit* le_filename;
      QTableWidget* tw_data;


};

#endif // US_CSV_LOADER_H
