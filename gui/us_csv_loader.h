#ifndef US_CSV_LOADER_H
#define US_CSV_LOADER_H

#include "us_widgets.h"
#include "us_widgets_dialog.h"
#include <QTableWidget>
#include <QSortFilterProxyModel>
#include <QTableWidgetItem>

class CSVTableWidgetItem : public QTableWidgetItem {

   public:
      CSVTableWidgetItem ( QString value, bool numericPriority );
      void setNumericPriority( bool );
      bool operator < ( const QTableWidgetItem & ) const override;

   private:
      bool m_num_prio;

};

class CSVTableWidget : public QTableWidget {

   Q_OBJECT
   public:
      CSVTableWidget(QWidget *parent=nullptr);
      void add_header();

   signals:
      void new_content();

   protected:
      void contextMenuEvent(QContextMenuEvent *event) override;

   private slots:
      void delete_rows();
      void delete_columns();
};

class US_GUI_EXTERN US_CSV_Loader : public US_WidgetsDialog {

   Q_OBJECT
   public:
      US_CSV_Loader(QWidget* parent=0);
      bool set_filepath(QString&, bool);
      void set_numeric_state(bool, bool);
      void set_msg(QString);
      QVector<QStringList> get_data();
      QFileInfo get_file_info();

   private:
      enum DELIMITER {TAB, COMMA, SEMICOLON, SPACE, OTHER, NONE};
      QFileInfo infile;
      QVector<QStringList> column_list;
      DELIMITER  delimiter;
      QPushButton* pb_open;
      QPushButton* pb_ok;
      QPushButton* pb_cancel;
      QPushButton* pb_add_header;
      QPushButton* pb_save_csv;
      QRadioButton* rb_tab;
      QRadioButton* rb_comma;
      QRadioButton* rb_semicolon;
      QRadioButton* rb_space;
      QRadioButton* rb_other;
      QRadioButton* rb_string;
      QRadioButton* rb_numeric;
      QButtonGroup* bg_delimiter;
      QLineEdit* le_other;
      QLineEdit* le_filename;
      QLineEdit* le_msg;
      QString str_delimiter;
      QString curr_dir;
      QStringList file_lines;
      CSVTableWidget* tw_data;

      bool check_table_size();
      bool check_table_data();
      bool parse_file(QString&);
      QStringList gen_alpha_list(int);
      QStringList make_labels(int);

   private slots:
      void open();
      void ok();
      void cancel();
      void save_csv();
      void fill_table(int);
      void new_delimiter(const QString &);
      void add_header();
      void highlight_header();
};

#endif // US_CSV_LOADER_H
