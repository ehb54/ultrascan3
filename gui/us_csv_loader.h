#ifndef US_CSV_LOADER_H
#define US_CSV_LOADER_H


#include "us_widgets.h"
#include "us_widgets_dialog.h"
#include <QTabWidget>

class CustomTableWidget : public QTableWidget {
   public:
   CustomTableWidget(QWidget *parent = nullptr) : QTableWidget(parent) {}

   protected:
   void contextMenuEvent(QContextMenuEvent *event) override {
      QMenu contextMenu(this);

      QAction *ins_row_above = contextMenu.addAction("Insert A Row Above");
      QAction *ins_row_below = contextMenu.addAction("Insert A Row Below");
      QAction *del_cur_row = contextMenu.addAction("Delete Current Row");
      QAction *ins_col_left = contextMenu.addAction("Insert A Column Left");
      QAction *ins_col_right = contextMenu.addAction("Insert A Column Right");
      QAction *del_cur_col = contextMenu.addAction("Delete Current Column");

      QString styleSheet = "QMenu { background-color: rgb(255, 253, 208); }"
                           "QMenu::item { background: transparent; }"
                           "QMenu::item:selected { background-color: transparent; color: red; font-weight: bold; }";


      contextMenu.setStyleSheet(styleSheet);

      // connect(action1, &QAction::triggered, this, &CustomTableWidget::handleOption1);
      // connect(action2, &QAction::triggered, this, &CustomTableWidget::handleOption2);

      contextMenu.exec(event->globalPos());
   }

   private slots:
   // void handleOption1() {
   //    // Implement the action for Option 1
   // }

   // void handleOption2() {
   //    // Implement the action for Option 2
   // }
};

class US_GUI_EXTERN US_CSV_Loader : public US_WidgetsDialog
{
   Q_OBJECT
   public:
      US_CSV_Loader(QWidget* parent);

   private:
      enum DELIMITER {TAB, COMMA, SEMICOLON, SPACE, OTHER};
      bool loaded;
      DELIMITER  delimiter;
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
      CustomTableWidget* tw_data;
      QString str_delimiter;
      QButtonGroup* bg_delimiter;

      QStringList file_lines;
      QStringList make_labels(int);



   private slots:
      void open();
      void ok();
      void cancel();
      void fill_table(int);



};

#endif // US_CSV_LOADER_H
