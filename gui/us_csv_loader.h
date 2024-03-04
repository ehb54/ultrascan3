#ifndef US_CSV_LOADER_H
#define US_CSV_LOADER_H


#include "us_widgets.h"
#include "us_widgets_dialog.h"
#include <QTabWidget>

class CustomTableWidget : public QTableWidget {
   Q_OBJECT
   public:
   CustomTableWidget(QWidget *parent = nullptr) : QTableWidget(parent) {}
   void add_header() {
      this->insertRow(0);
   }

   signals:
   void new_content();

   protected:
   void contextMenuEvent(QContextMenuEvent *event) override {
      QMenu contextMenu(this);

      QAction *del_row = contextMenu.addAction("Delete Row");
      QAction *del_col = contextMenu.addAction("Delete Column");

      QString styleSheet = "QMenu { background-color: rgb(255, 253, 208); }"
                           "QMenu::item { background: transparent; }"
                           "QMenu::item:selected { background-color: transparent; color: red; font-weight: bold; }";

      contextMenu.setStyleSheet(styleSheet);

      connect(del_row, &QAction::triggered, this, &CustomTableWidget::delete_row);
      connect(del_col, &QAction::triggered, this, &CustomTableWidget::delete_column);

      contextMenu.exec(event->globalPos());
   }

   private slots:
   void delete_row() {
      this->removeRow(this->currentRow());
      emit new_content();
   }

   void delete_column() {
      this->removeColumn(this->currentColumn());
      emit new_content();
   }
};

class US_GUI_EXTERN US_CSV_Loader : public US_WidgetsDialog
{
   Q_OBJECT
   public:
      US_CSV_Loader(QWidget* parent);
      void set_numeric_state(bool, bool);
      void set_msg(QString&);
      QVector<QStringList> get_data();

   private:
      enum DELIMITER {TAB, COMMA, SEMICOLON, SPACE, OTHER};
      bool loaded;
      QVector<QStringList> column_list;
      DELIMITER  delimiter;
      QPushButton* pb_open;
      QPushButton* pb_ok;
      QPushButton* pb_cancel;
      QPushButton* pb_add_header;
      QRadioButton* rb_tab;
      QRadioButton* rb_comma;
      QRadioButton* rb_semicolon;
      QRadioButton* rb_space;
      QRadioButton* rb_other;
      QRadioButton* rb_string;
      QRadioButton* rb_numeric;
      QLineEdit* le_other;
      QLineEdit* le_filename;
      QLineEdit* le_msg;
      CustomTableWidget* tw_data;
      QString str_delimiter;
      QButtonGroup* bg_delimiter;

      QStringList file_lines;
      QStringList make_labels(int);

      bool check_table_size();
      bool check_table_data();

   private slots:
      void open();
      void ok();
      void cancel();
      void fill_table(int);
      void add_header();
      void highlight_header();



};

#endif // US_CSV_LOADER_H
