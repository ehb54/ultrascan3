#ifndef US_CSV_LOADER_H
#define US_CSV_LOADER_H

#include "us_widgets.h"
#include "us_widgets_dialog.h"
#include <QTableWidget>
#include <QSortFilterProxyModel>
#include <QTableWidgetItem>
#include <QTableView>
#include <QStandardItem>
#include <QStandardItemModel>


class CSVTableView : public QTableView {

   Q_OBJECT
   public:
      CSVTableView(QWidget *parent=nullptr);

   signals:
      void row_column_deleted();

   protected:
      void contextMenuEvent(QContextMenuEvent *event) override;

   private slots:
      void delete_rows();
      void delete_columns();
};

class CSVSortFilterProxyModel: public QSortFilterProxyModel {
   public:
      using QSortFilterProxyModel::QSortFilterProxyModel;
      bool lessThan(const QModelIndex &, const QModelIndex &) const override;
};

class US_GUI_EXTERN US_CSV_Loader : public US_WidgetsDialog {

   Q_OBJECT
   public:
      US_CSV_Loader(QWidget* parent=0);
      bool set_filepath(QString&, bool);
      void setMessage(const QString&);
      void setEditable(bool);
      bool data(QVector<QVector<double>>&, QStringList&);
      bool dataFileInfo(QFileInfo&);

   private:
      enum DELIMITER {TAB, COMMA, SEMICOLON, SPACE, OTHER, NONE};
      bool editable;
      QFileInfo infile;
      DELIMITER  delimiter;
      QPushButton* pb_open;
      QPushButton* pb_ok;
      QPushButton* pb_cancel;
      QPushButton* pb_add_header;
      QPushButton* pb_save_csv;
      QPushButton* pb_reset;
      QPushButton* pb_show_red;
      QRadioButton* rb_tab;
      QRadioButton* rb_comma;
      QRadioButton* rb_semicolon;
      QRadioButton* rb_space;
      QRadioButton* rb_other;
      QButtonGroup* bg_delimiter;
      QLineEdit* le_other;
      QLineEdit* le_filename;
      QLineEdit* le_msg;
      QString str_delimiter;
      QString curr_dir;
      QStringList file_lines;
      CSVTableView* tv_data;
      QStandardItemModel* model;
      CSVSortFilterProxyModel* proxy;

      bool parse_file(QString&);
      QStringList gen_alpha_list(int);
      bool check_table();
      void get_sorted(QVector<QVector<double>>&, QStringList&);
      bool write_csv(const QString&, const QString&, QString&);

   private slots:
      void open();
      void ok();
      void cancel();
      void save_csv_clicked();
      void reset();
      void fill_table(int);
      void new_delimiter(const QString &);
      void add_header();
      void item_changed(QStandardItem *);
      void relabel();
      void row_column_deleted();
      void show_red();
};

#endif // US_CSV_LOADER_H
