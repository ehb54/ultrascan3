#include <QtGlobal>
#include "us_csv_loader.h"
#include "us_gui_settings.h"
#include "us_settings.h"

bool CSVSortFilterProxyModel::lessThan(const QModelIndex &left,
                                       const QModelIndex &right) const  {
   if(left.row() == 0 || right.row() == 0) return false;

   QVariant left_data  = sourceModel()->data(left, Qt::DisplayRole);
   QVariant right_data = sourceModel()->data(right, Qt::DisplayRole);

   bool left_ok  = false;
   bool right_ok = false;

   double left_num = left_data.toDouble(&left_ok);
   double right_num = right_data.toDouble(&right_ok);

   if (left_ok && right_ok) return left_num < right_num;
   else if (left_ok) return true;
   else if (right_ok) return false;
   else return left_data.toString() < right_data.toString();

}

CSVTableView::CSVTableView(QWidget *parent) : QTableView(parent) {};

void CSVTableView::add_header() {
   // this->insertRow(0);
}

void CSVTableView::contextMenuEvent(QContextMenuEvent *event) {
   QMenu contextMenu(this);

   QAction *del_rows = contextMenu.addAction("Delete Row(s)");
   QAction *del_cols = contextMenu.addAction("Delete Column(s)");

   QString styleSheet = "QMenu { background-color: rgb(255, 253, 208); }"
                        "QMenu::item { background: transparent; }"
                        "QMenu::item:selected { background-color: transparent; color: red; font-weight: bold; }";

   contextMenu.setStyleSheet(styleSheet);

   connect(del_rows, &QAction::triggered, this, &CSVTableView::delete_rows);
   connect(del_cols, &QAction::triggered, this, &CSVTableView::delete_columns);

   contextMenu.exec(event->globalPos());
}

void CSVTableView::delete_rows() {
   // QVector<int> rows;
   // foreach (QTableWidgetItem *item, this->selectedItems()) {
   //    int r = item->row();
   //    if (! rows.contains(r)) rows << r;
   // }
   // std::sort(rows.begin(), rows.end(), [&](auto a, auto b) {return a > b;});

   // foreach (int ii, rows) {
   //    this->removeRow(ii);
   // }
   // emit new_content();
}

void CSVTableView::delete_columns() {
   // QVector<int> cols;
   // foreach (QTableWidgetItem *item, this->selectedItems()) {
   //    int c = item->column();
   //    if (! cols.contains(c)) cols << c;
   // }
   // std::sort(cols.begin(), cols.end(), [&](auto a, auto b) {return a > b;});

   // foreach (int ii, cols) {
   //    this->removeColumn(ii);
   // }
   // emit new_content();
}


US_CSV_Loader::US_CSV_Loader(QWidget* parent) : US_WidgetsDialog(parent, 0)
{
   setWindowTitle( tr( "Load CSV File" ) );
   setPalette( US_GuiSettings::frameColor() );
   setMinimumSize(QSize(600,600));
   setMaximumSize(QSize(800,800));
   QLabel *lb_filename = us_label("Filename:");
   lb_filename->setAlignment(Qt::AlignRight);
   le_filename = us_lineedit("");
   pb_open = us_pushbutton("Open");
   QLabel *lb_delimiter = us_label("Separated By:");
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
   le_other->setMaxLength(5);

   pb_add_header = us_pushbutton("Add Header");
   pb_cancel = us_pushbutton("Cancel");
   pb_ok = us_pushbutton("Ok");
   pb_save_csv = us_pushbutton("Save CSV");
   pb_reset = us_pushbutton("Reset");

   tv_data = new CSVTableView();
   tv_data->setSortingEnabled(true);
   model = new QStandardItemModel(20, 5);
   proxy = new CSVSortFilterProxyModel();
   proxy->setSourceModel(model);
   tv_data->setModel(proxy);

   for (int ii = 0; ii < model->rowCount(); ii++) {
      for (int jj =0; jj < model->columnCount(); jj++) {
         QStandardItem *it = new QStandardItem();
         it->setData("", Qt::DisplayRole);
         model->setItem(ii, jj, it);
      }
   }
   tv_data->setStyleSheet("background-color: white");

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

   main_lyt->addWidget(pb_save_csv,       3, 1, 1, 2);
   main_lyt->addWidget(pb_reset,          3, 3, 1, 2);
   main_lyt->addWidget(pb_add_header,     3, 5, 1, 2);

   main_lyt->addWidget(pb_cancel,         4, 3, 1, 2);
   main_lyt->addWidget(pb_ok,             4, 5, 1, 2);

   main_lyt->addWidget(le_msg,            5, 0, 1, 7);
   main_lyt->addWidget(tv_data,           6, 0, 5, 7);

   main_lyt->setSpacing(2);
   main_lyt->setMargin(2);

   setLayout(main_lyt);

   connect(pb_open, &QPushButton::clicked, this, &US_CSV_Loader::open);
   connect(pb_add_header, &QPushButton::clicked, this, &US_CSV_Loader::add_header);
   // connect(tv_data, &CSVTableView::new_content, this, &US_CSV_Loader::highlight_header);
   connect(pb_ok, &QPushButton::clicked, this, &US_CSV_Loader::ok);
   connect(pb_cancel, &QPushButton::clicked, this, &US_CSV_Loader::cancel);
   connect(le_other, &QLineEdit::textChanged, this, &US_CSV_Loader::new_delimiter);
   connect(pb_save_csv, &QPushButton::clicked, this, &US_CSV_Loader::save_csv);
#if QT_VERSION >= QT_VERSION_CHECK(5,15,0)
   connect(bg_delimiter, QOverload<int>::of(&QButtonGroup::idClicked), this, &US_CSV_Loader::fill_table);
#else
   connect(bg_delimiter, QOverload<int>::of(&QButtonGroup::buttonClicked), this, &US_CSV_Loader::fill_table);
#endif

}

void US_CSV_Loader::add_header() {
   if (file_lines.size() == 0) return;
   // tv_data->add_header();
   model->insertRow(0);
   int nr = tv_data->model()->rowCount();
   int nc = tv_data->model()->columnCount();
   qDebug() << nr;
   qDebug() << nc;
   QFont font( US_Widgets::fixedFont().family(),
                 US_GuiSettings::fontSize() );
   QStringList headers = gen_alpha_list(nc);
   for (int ii = 0; ii < nc; ii++) {
      QStandardItem *it = new QStandardItem();
      it->setData(headers.at(ii), Qt::DisplayRole);
      it->setData(true, Qt::UserRole);
      it->setData(font, Qt::FontRole);
      it->setBackground(Qt::green);
      model->setItem(0, ii, it);

      bool ok;
      model->item(1, ii)->data(Qt::DisplayRole).toDouble(&ok);
      if (ok) {
         model->item(1, ii)->setBackground(Qt::white);
         model->item(1, ii)->setData(true, Qt::UserRole);
      }
      else {
         model->item(1, ii)->setBackground(Qt::red);
         model->item(1, ii)->setData(false, Qt::UserRole);
      }
   }
   relabel();
   check_table();
}

void US_CSV_Loader::set_msg(QString msg) {
   le_msg->setText(msg);
}

QVector<QVector<double>> US_CSV_Loader::get_data() {
   return data;
}

QStringList US_CSV_Loader::get_header() {
   return header;
}

void US_CSV_Loader::relabel() {
   QFont font( US_Widgets::fixedFont().family(),
              US_GuiSettings::fontSize() );
   int nrows = model->rowCount();
   int ncols = model->columnCount();
   for (int ii = 0; ii < nrows; ii++) {
      QStandardItem *it = new QStandardItem();
      it->setData(font, Qt::FontRole);
      if (ii == 0) it->setData("Header", Qt::DisplayRole);
      else it->setData(QString::number(ii), Qt::DisplayRole);
      model->setVerticalHeaderItem(ii, it);
   }
   if (hlabel.size() > 0) {
      for (int ii = 0; ii < ncols; ii++) {
         QStandardItem *it = new QStandardItem();
         it->setData(font, Qt::FontRole);
         if (ii < hlabel.size()) it->setData(hlabel.at(ii), Qt::DisplayRole);
         else it->setData(QString::number(ii + 1), Qt::DisplayRole);
         model->setHorizontalHeaderItem(ii, it);
      }
   }
}

void US_CSV_Loader::ok() {
   data.clear();
   header.clear();
   int nrows = tv_data->model()->rowCount();
   int ncols = tv_data->model()->columnCount();
   bool ok;
   double cell_d;
   for (int jj = 0; jj < ncols; jj++) {
      QVector<double> col;
      for (int ii = 0 ; ii < nrows; ii++) {
         QVariant cell_v = model->item(ii, jj)->data(Qt::DisplayRole);
         if (ii == 0) {
            header << cell_v.toString();
            continue;
         }
         cell_d = cell_v.toDouble(&ok);
         if (! ok) {
            data.clear();
            QString mesg("Cell(%1,%2) with the value '%3' cannot convert to numeric!");
            QMessageBox::warning(this, "Error!", mesg.arg(ii + 1).arg(jj + 1).arg(cell_v.toString()));
            return;
         }
         col << cell_d;
      }
      data << col;
   }
   accept();
}

void US_CSV_Loader::save_csv() {
   if (file_lines.size() == 0) return;
   if(! check_table()) return;
   // if(! check_table_data()) return;

   QString delimiter_str;
   QString user_delimiter = le_other->text().trimmed();
   int state = QMessageBox::question(this, "Set Delimiter", "Do you want to save as different delimiter?");
   if (state == QMessageBox::Yes) {
      QComboBox* cb_delimiter = us_comboBox();
      cb_delimiter->addItem("Tab");
      cb_delimiter->addItem("Space");
      cb_delimiter->addItem("Comma");
      cb_delimiter->addItem("Semicolon");
      if (! user_delimiter.isEmpty()) {
         cb_delimiter->addItem(tr("Other: %1").arg(user_delimiter));
      }
      QDialog *dialog = new QDialog(this);
      QDialogButtonBox *buttons = new QDialogButtonBox(Qt::Horizontal);
      buttons->addButton(QDialogButtonBox::Ok);
      buttons->addButton(QDialogButtonBox::Cancel);
      QVBoxLayout* lyt = new QVBoxLayout();
      lyt->addWidget(cb_delimiter);
      lyt->addWidget(buttons);
      dialog->setLayout(lyt);
      connect(buttons, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
      connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
      state = dialog->exec();
      if (state == QDialog::Accepted) {
         int id = cb_delimiter->currentIndex();
         if (id == 0) delimiter_str = "\t";
         else if (id == 1) delimiter_str = " ";
         else if (id == 2) delimiter_str = ",";
         else if (id == 3) delimiter_str = ";";
         else if (id == 4) delimiter_str = user_delimiter;
      } else {
         QMessageBox::warning(this, "Warning!", "Nothing Saved!");
         return;
      }
   } else {
      if (rb_tab->isChecked()) delimiter_str = "\t";
      else if (rb_space->isChecked()) delimiter_str = " ";
      else if (rb_comma->isChecked()) delimiter_str = ",";
      else if (rb_semicolon->isChecked()) delimiter_str = ";";
      else if (rb_other->isChecked()) delimiter_str = user_delimiter;
   }

   int nrows = tv_data->model()->rowCount();
   int ncols = tv_data->model()->columnCount();
   QString filePath = QFileDialog::getSaveFileName(this, "Save CSV File", US_Settings::workBaseDir(), "(CSV)(*.csv)");
   if (filePath.size() == 0) return;
   if (! filePath.endsWith(".csv", Qt::CaseInsensitive)) {
      filePath += ".csv";
   }
   QFile file(filePath);
   if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QTextStream ts(&file);
      for (int ii = 0 ; ii < nrows; ii++) {
         for (int jj = 0; jj < ncols; jj++) {
            ts << model->item(ii, jj)->data(Qt::DisplayRole).toString().trimmed();
            if (jj < ncols - 1) ts << delimiter_str;
            else ts << "\n";
         }
      }
      file.close();
      QMessageBox::warning(this, "", tr("Data Saved!\n\n").arg(filePath));
   } else {
      QMessageBox::warning(this, "Error!", "Error: Couldn't open the file for writing.");
   }

}

void US_CSV_Loader::cancel() {
   data.clear();
   header.clear();
   reject();
}

void US_CSV_Loader::open() {
   QString dir = curr_dir.isEmpty() ? US_Settings::workBaseDir() : curr_dir;
   qDebug() << dir;
   QString filepath = QFileDialog::getOpenFileName(this, "Open File", dir, "(TEXT Files) (*)");
   if (filepath.isEmpty()) return;
   parse_file(filepath);
}

bool US_CSV_Loader::parse_file(QString& filepath) {
   QFile file(filepath);
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
               file.close();
               isAscii = false;
               break;
            }
         }
         if (!isAscii) {
            file.close();
            file_lines.clear();
            QMessageBox::warning(this, "Error!", tr("This is not a text file!\n%1").arg(filepath));
            return false;
         }
         file_lines.append(line);
      }
      if (file_lines.size() == 0) {
         QMessageBox::warning(this, "Error!", tr("This is an empty file!\n%1").arg(filepath));
         return false;
      }
      delimiter = NONE;
      infile = QFileInfo(filepath);
      curr_dir = infile.absoluteDir().absolutePath();
      fill_table(bg_delimiter->checkedId());
      le_filename->setText(infile.fileName());
      data.clear();
      header.clear();
      return true;
   } else {
      QMessageBox::warning(this, "Error!", tr("Couldn't open the file!").arg(filepath));
      return false;
   }
}

bool US_CSV_Loader::set_filepath(QString& filepath, bool openEnabled) {
   if (parse_file(filepath)) {
      pb_open->setEnabled(openEnabled);
      return true;
   } else {
      return false;
   }
}

void US_CSV_Loader::new_delimiter(const QString &) {
   if (delimiter == OTHER) {
      delimiter = NONE;
      fill_table(OTHER);
   }
}

void US_CSV_Loader::fill_table(int id) {
   if (delimiter == id) {
      return;
   } else {
      delimiter = static_cast<DELIMITER>(id);
   }

   QFont font( US_Widgets::fixedFont().family(),
                 US_GuiSettings::fontSize() );
   // QFontMetrics* fm = new QFontMetrics( font );
   // int rowht = fm->height() + 2;

   model->clear();

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
   model = new QStandardItemModel (n_rows, n_columns);
   bool droplast = true;
   for (int ii = 0; ii < n_rows; ii++ ) {
      int nd = data_list.at(ii).size();
      for (int jj = 0; jj < n_columns; jj++) {
         QVariant val("");
         if (jj < nd) {
            QString str = data_list.at(ii).at(jj).trimmed();
            val = QVariant(str);
            if (jj == n_columns - 1 && !str.isEmpty()) {
               droplast = false;
            }
         }
         QStandardItem *it = new QStandardItem();
         it->setData(val, Qt::DisplayRole);
         it->setData(font, Qt::FontRole);
         if (ii == 0) {
            if (val.toString().isEmpty()) {
               it->setBackground(Qt::red);
               it->setData(false, Qt::UserRole);
            } else {
               it->setBackground(Qt::green);
               it->setData(true, Qt::UserRole);
            }
         } else {
            bool ok;
            val.toDouble(&ok);
            if (ok) {
               it->setBackground(Qt::white);
               it->setData(true, Qt::UserRole);
            }
            else {
               it->setBackground(Qt::red);
               it->setData(false, Qt::UserRole);
            }
         }
         model->setItem(ii, jj, it);
      }
   }
   if (droplast) {
      model->removeColumn(n_columns - 1);
   }
   // model->sort(0, Qt::DescendingOrder);
   proxy->setSourceModel(model);
   tv_data->setModel(proxy);
   relabel();
   check_table();
   tv_data->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
   tv_data->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
   tv_data->resizeColumnsToContents();
   // qDebug() << QDateTime::currentMSecsSinceEpoch() << " fill_table";
}

bool US_CSV_Loader::check_table() {
   bool ready = true;
   for (int ii = 0; ii < model->rowCount(); ii++) {
      for (int jj = 0; jj < model->columnCount(); jj++) {
         ready = ready && model->item(ii, jj)->data(Qt::UserRole).toBool();
         if (! ready) break;
      }
      if (! ready) break;
   }
   if (ready) pb_ok->setEnabled(true);
   else pb_ok->setEnabled(false);
   return ready;
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
