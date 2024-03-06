#include <QtGlobal>
#include "us_csv_loader.h"
#include "us_gui_settings.h"
#include "us_settings.h"

CustomTableWidget::CustomTableWidget(QWidget *parent) : QTableWidget(parent) {};

void CustomTableWidget::add_header() {
   this->insertRow(0);
}

void CustomTableWidget::contextMenuEvent(QContextMenuEvent *event) {
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

void CustomTableWidget::delete_row() {
   this->removeRow(this->currentRow());
   emit new_content();
}

void CustomTableWidget::delete_column() {
   this->removeColumn(this->currentColumn());
   emit new_content();
}


US_CSV_Loader::US_CSV_Loader(QWidget* parent) : US_WidgetsDialog(parent, 0)
{
   setWindowTitle( tr( "Load CSV File" ) );
   setPalette( US_GuiSettings::frameColor() );
   setMinimumSize(QSize(550,550));
   setMaximumSize(QSize(800,800));
   QLabel *lb_filename = us_label("Filename:");
   lb_filename->setAlignment(Qt::AlignRight);
   le_filename = us_lineedit("");
   pb_open = us_pushbutton("Open");
   QLabel *lb_delimiter = us_label("Separated by:");
   lb_delimiter->setAlignment(Qt::AlignRight);

   rb_tab = new QRadioButton();
   QGridLayout *lyt_tab = us_radiobutton("Tab", rb_tab);

   rb_comma = new QRadioButton();
   QGridLayout *lyt_comma = us_radiobutton("Comma", rb_comma);

   rb_semicolon = new QRadioButton();
   QGridLayout *lyt_semicolon = us_radiobutton("Semicolon", rb_semicolon);

   rb_space = new QRadioButton();
   QGridLayout *lyt_space = us_radiobutton("Space", rb_space);

   rb_other = new QRadioButton();
   QGridLayout *lyt_other = us_radiobutton("Other", rb_other);

   bg_delimiter = new QButtonGroup();
   bg_delimiter->addButton(rb_tab, TAB);
   bg_delimiter->addButton(rb_comma, COMMA);
   bg_delimiter->addButton(rb_semicolon, SEMICOLON);
   bg_delimiter->addButton(rb_space, SPACE);
   bg_delimiter->addButton(rb_other, OTHER);
   rb_tab->setChecked(true);
   delimiter = TAB;

   le_other = us_lineedit("");

   rb_string = new QRadioButton();
   QGridLayout *lyt_string = us_radiobutton("String Data", rb_string);

   rb_numeric = new QRadioButton();
   QGridLayout *lyt_numeric = us_radiobutton("Numeric Data", rb_numeric);

   QLabel* lb_feature = us_label("Features:");
   lb_feature->setAlignment(Qt::AlignRight);

   QButtonGroup* bg_feature = new QButtonGroup();
   bg_feature->addButton(rb_string);
   bg_feature->addButton(rb_numeric);
   rb_numeric->setChecked(true);

   pb_add_header = us_pushbutton("Add Header");
   pb_cancel = us_pushbutton("Cancel");
   pb_ok = us_pushbutton("Ok");

   tw_data = new CustomTableWidget();
   tw_data->setRowCount(20);
   tw_data->setColumnCount(10);
   tw_data-> setHorizontalHeaderLabels(make_labels(10));
   tw_data-> setHorizontalHeaderLabels(make_labels(20));
   tw_data->setStyleSheet("background-color: white");
   QHeaderView *header = tw_data->horizontalHeader();
   header->setSectionResizeMode(QHeaderView::Stretch);

   le_msg = us_lineedit("", 0, true);

   QGridLayout* main_lyt = new QGridLayout();
   main_lyt->addWidget(lb_filename,       0, 0, 1, 1);
   main_lyt->addWidget(le_filename,       0, 1, 1, 4);
   main_lyt->addWidget(pb_open,           0, 5, 1, 2);

   main_lyt->addWidget(lb_delimiter,      1, 0, 1, 1);
   main_lyt->addLayout(lyt_tab,           1, 1, 1, 2);
   main_lyt->addLayout(lyt_space,         1, 3, 1, 2);
   main_lyt->addLayout(lyt_comma,         1, 5, 1, 2);

   main_lyt->addLayout(lyt_semicolon,     2, 1, 1, 2);
   main_lyt->addLayout(lyt_other,         2, 3, 1, 2);
   main_lyt->addWidget(le_other,          2, 5, 1, 2);

   main_lyt->addWidget(lb_feature,        3, 0, 1, 1);
   main_lyt->addLayout(lyt_numeric,       3, 1, 1, 3);
   main_lyt->addLayout(lyt_string,        3, 4, 1, 3);

   main_lyt->addWidget(pb_add_header,     5, 0, 1, 1);
   main_lyt->addWidget(pb_cancel,         5, 3, 1, 2);
   main_lyt->addWidget(pb_ok,             5, 5, 1, 2);

   main_lyt->addWidget(le_msg,            6, 0, 1, 7);
   main_lyt->addWidget(tw_data,           7, 0, 5, 7);

   main_lyt->setSpacing(2);
   main_lyt->setMargin(2);

   setLayout(main_lyt);
   loaded = false;

   connect(pb_open, &QPushButton::clicked, this, &US_CSV_Loader::open);
   connect(pb_add_header, &QPushButton::clicked, this, &US_CSV_Loader::add_header);
   connect(tw_data, &CustomTableWidget::new_content, this, &US_CSV_Loader::highlight_header);
   connect(pb_ok, &QPushButton::clicked, this, &US_CSV_Loader::ok);
   connect(pb_cancel, &QPushButton::clicked, this, &US_CSV_Loader::cancel);
#if QT_VERSION >= QT_VERSION_CHECK(5,15,0)
   connect(bg_delimiter, QOverload<int>::of(&QButtonGroup::idClicked), this, &US_CSV_Loader::fill_table);
#else
   connect(bg_delimiter, QOverload<int>::of(&QButtonGroup::buttonClicked), this, &US_CSV_Loader::fill_table);
#endif

}

void US_CSV_Loader::set_numeric_state(bool state, bool enabled) {
   if (state) rb_numeric->setChecked(true);
   else rb_string->setChecked(true);
   rb_numeric->setEnabled(enabled);
   rb_string->setEnabled(enabled);
}

void US_CSV_Loader::add_header() {
   if (! loaded) {
      return;
   }
   tw_data->add_header();
   qDebug() << tw_data->rowCount();
   qDebug() << tw_data->columnCount();
   QFont tw_font( US_Widgets::fixedFont().family(),
                 US_GuiSettings::fontSize() );
   QStringList headers = gen_alpha_list(tw_data->columnCount());
   for (int ii = 0; ii < tw_data->columnCount(); ii++) {
      QTableWidgetItem *twi = new QTableWidgetItem(headers.at(ii));
      twi->setFont(tw_font);
      tw_data->setItem(0, ii, twi);
   }
   highlight_header();
}

void US_CSV_Loader::set_msg(QString msg) {
   le_msg->setText(msg);
}

QVector<QStringList> US_CSV_Loader::get_data() {
   return column_list;
}

QStringList US_CSV_Loader::make_labels(int number) {
   QStringList labels;
   for (int ii = 0; ii < number; ii++) {
      labels.append(QString::number(ii + 1));
   }
   return labels;
}

void US_CSV_Loader::ok() {
   if(! check_table_size()) return;
   if(! check_table_data()) return;
   column_list.clear();
   int nrows = tw_data->rowCount();
   int ncols = tw_data->columnCount();
   for (int jj = 0; jj < ncols; jj++) {
      QStringList col;
      for (int ii = 0 ; ii < nrows; ii++) {
         col << tw_data->item(ii, jj)->text();
      }
      column_list << col;
   }
   accept();
}

void US_CSV_Loader::cancel() {
   column_list.clear();
   reject();
}

void US_CSV_Loader::open() {
   QString fpath = QFileDialog::getOpenFileName(this, "Open File", US_Settings::workBaseDir(),
                                                   "(TEXT Files) (*)");
   if (fpath.isEmpty()) return;
   infile = QFileInfo(fpath);
   QFile file(infile.absoluteFilePath());
   if(file.open(QIODevice::ReadOnly)) {
      file_lines.clear();
      QTextStream ts(&file);
      bool isAscii = true;
      while (true) {
         if (ts.atEnd()) {
            file.close();
            break;
         }
         QString line = ts.readLine();
         QByteArray byte_arr = line.toUtf8();
         for (char ch : byte_arr) {
            if (ch < 0 || ch > 127) {
               qDebug() << "Non-ASCII character detected. Binary file.";
               file.close();
               isAscii = false;
               break;
            }
         }
         if (!isAscii) {
            file.close();
            file_lines.clear();
            QMessageBox::warning(this, "Error!", "Please load a text file!");
            return;
         }
         file_lines.append(line);
      }
      loaded = false;
      fill_table(bg_delimiter->checkedId());
      le_filename->setText(infile.fileName());
      column_list.clear();
   } else {
      QMessageBox::warning(this, "Error!", "Couldn't open the file!");
      return;
   }
}

void US_CSV_Loader::fill_table(int id) {
   if (delimiter == id && loaded) {
      return;
   } else {
      delimiter = static_cast<DELIMITER>(id);
      loaded = true;
   }

   QFont tw_font( US_Widgets::fixedFont().family(),
                 US_GuiSettings::fontSize() );
   // QFontMetrics* fm = new QFontMetrics( tw_font );
   // int rowht = fm->height() + 2;
   tw_data->clearContents();
   int n_columns = static_cast<int>(-1e99);
   int n_rows = file_lines.size();
   QVector<QStringList> data_list;

   for (int ii = 0; ii < n_rows; ii++ ) {
      QString line = file_lines.at(ii);
      QStringList lsp;
      if (rb_tab->isChecked()) lsp = line.split('\t');
      else if (rb_space->isChecked()) lsp = line.split(u' ');
      else if (rb_comma->isChecked()) lsp = line.split(u',');
      else if (rb_semicolon->isChecked()) lsp = line.split(u';');
      else if (rb_other->isChecked()) {
         if (le_other->text().isEmpty()) lsp << line;
         else lsp = line.split(le_other->text());
      }
      n_columns = qMax(lsp.size(), n_columns);
      data_list << lsp;

   }
   tw_data->setRowCount(n_rows);
   tw_data->setColumnCount(n_columns);
   for (int ii = 0; ii < n_rows; ii++ ) {
      int nd = data_list.at(ii).size();
      for (int jj = 0; jj < n_columns; jj++) {
         QTableWidgetItem *twi;
         if (jj < nd){
            twi = new QTableWidgetItem(data_list.at(ii).at(jj).trimmed());
         } else {
            twi = new QTableWidgetItem("");
         }
         twi->setFont(tw_font);
         tw_data->setItem(ii, jj, twi);
         //         tw_data->setRowHeight(ii, rowht);
      }
   }

   tw_data->setHorizontalHeaderLabels(make_labels(n_columns));
   tw_data->setVerticalHeaderLabels(make_labels(n_rows));
   highlight_header();
   // qDebug() << QDateTime::currentMSecsSinceEpoch() << " fill_table";
}

void US_CSV_Loader::highlight_header() {
   for (int ii = 0; ii < tw_data->rowCount(); ii++) {
      for (int jj = 0; jj < tw_data->columnCount(); jj++) {
         if (ii == 0) tw_data->item(ii, jj)->setBackground(QBrush(Qt::yellow));
         else tw_data->item(ii, jj)->setBackground(QBrush(Qt::white));
      }
   }
}

bool US_CSV_Loader::check_table_size() {
   int nrows = tw_data->rowCount();
   int ncols = tw_data->columnCount();
   int nc_0 = 0;
   for (int ii = 0; ii < nrows; ii++) {
      int nc_r = 0;
      for (int jj = 0; jj < ncols; jj++) {
         if (ii == 0 && !tw_data->item(ii, jj)->text().isEmpty()) {
            nc_0++;
            nc_r++;
         } else if (ii > 0 && !tw_data->item(ii, jj)->text().isEmpty()){
            nc_r++;
         }
      }
      if (nc_0 != nc_r) {
         QMessageBox::warning(this, "Error!",
                              tr("The number of cells doesn't match!\nrow %1").arg(ii + 1));
         return false;
      }
   }
   return true;
}

bool US_CSV_Loader::check_table_data() {
   if (rb_string->isChecked()) return true;
   bool state;
   int nrows = tw_data->rowCount();
   int ncols = tw_data->columnCount();
   for (int ii = 1; ii < nrows; ii++) {
      for (int jj = 0; jj < ncols; jj++) {
         tw_data->item(ii, jj)->text().toDouble(&state);
         if (! state) {
            QMessageBox::warning(this, "Error!", tr("Cell (%1, %2) is not numeric").arg(ii + 1).arg(jj + 1));
            tw_data->setCurrentCell(ii, jj);
            return false;
         }
      }
   }
   return true;
}

QFileInfo US_CSV_Loader::get_file_info() {
   return infile;
}

QStringList US_CSV_Loader::gen_alpha_list (int num) {
   const QString alphabet("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
   QStringList outlist;
   int divisor = alphabet.size();
   int dividend, quotient, remainder;
   for (int ii = 0; ii < num; ii++) {
      QStringList tmplist;
      dividend = ii;
      while (true) {
         quotient = dividend / divisor;
         remainder = dividend % divisor;
         tmplist << alphabet.at(remainder);
         if (quotient == 0) {
            break;
         } else {
            dividend = quotient - 1;
         }
      }
      QString str = "";
      for (int jj = tmplist.size() - 1; jj >= 0; jj--) {
         str += tmplist.at(jj);
      }
      outlist << str;
   }
   return outlist;
}
