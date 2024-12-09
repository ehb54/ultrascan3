#include <QtGlobal>
#include <QTimer>
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

CSVTableView::CSVTableView(QWidget *parent) : QTableView(parent) {}

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
   QVector<int> rows;
   QList<QModelIndex> selindex = this->selectedIndexes();
   foreach (QModelIndex midx, selindex) {
      int r = midx.row();
      if (! rows.contains(r)) rows << r;
   }
   std::sort(rows.begin(), rows.end(), [&](auto a, auto b) {return a > b;});

   foreach (int ii, rows) {
      this->model()->removeRow(ii);
   }
   emit row_column_deleted();
}

void CSVTableView::delete_columns() {
   QVector<int> cols;
   QList<QModelIndex> selindex = this->selectedIndexes();
   foreach (QModelIndex midx, selindex) {
      int c = midx.column();
      if (! cols.contains(c)) cols << c;
   }
   std::sort(cols.begin(), cols.end(), [&](auto a, auto b) {return a > b;});

   foreach (int ii, cols) {
      this->model()->removeColumn(ii);
   }
   emit row_column_deleted();
}

int US_CSV_Loader::CSV_Data::columnCount() const {
   return m_header.size();
}

int US_CSV_Loader::CSV_Data::rowCount() const {
   if (m_columns.size() == 0) return 0;
   return m_columns.at(0).size();
}

QStringList US_CSV_Loader::CSV_Data::header() const {
   return m_header;
}

QVector<double> US_CSV_Loader::CSV_Data::columnAt(int id) const {
   QVector<double> col;
   if (id >= 0 && id < m_columns.size()) {
      col = m_columns.at(id);
   }
   return col;
}

bool US_CSV_Loader::CSV_Data::setData(const QString &file_path,
                          const QStringList& header,
                          const QVector<QVector<double>>& columns) {
   m_columns.clear();
   m_header.clear();
   m_path.clear();
   int ncols = header.size();
   if (columns.size() != ncols) {
      return false;
   }
   m_header << header;
   int nrows = -1;
   for (int ii = 0; ii < ncols; ii++) {
      int nr = columns.at(ii).size();
      if (nrows == -1) {
         nrows = nr;
      }
      if (nr == 0 || nr != nrows) {
         m_columns.clear();
         m_header.clear();
         return false;
      }
      m_columns << columns.at(ii);
   }
   m_path = file_path;
   return true;
}

void US_CSV_Loader::CSV_Data::clear() {
   m_header.clear();
   m_columns.clear();
   m_path.clear();
}

QString US_CSV_Loader::CSV_Data::filePath() const {
   return m_path;
}

US_CSV_Loader::US_CSV_Loader(const QString& filePath, const QString& note,
                             bool editable,QWidget* parent) : US_WidgetsDialog(parent, 0)
{

   if (! parse_file(filePath)) {
      // QMessageBox::warning(this, "Error!", "Failed to load the file!\n\n" + error_msg);
      QTimer::singleShot(25, this, [=](){this->done(-2);});
   } else {
      set_UI();
      m_editable = editable;
      le_msg->setText(note);
      delimiter = NONE;
      fill_table(bg_delimiter->checkedId());
      infile = QFileInfo(filePath);
      le_filename->setText("Filename: " + infile.fileName());
   }
}

void US_CSV_Loader::set_UI() {
   setWindowTitle( tr( "Load CSV Files" ) );
   setPalette( US_GuiSettings::frameColor() );
   setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
   setMinimumSize(QSize(550,600));
   // setMaximumSize(QSize(800,800));

   le_filename = us_lineedit("Filename:", 0, true);
   le_filename->setFrame(false);

   rb_tab = new QRadioButton();
   QGridLayout *lyt_tab = us_radiobutton("Tab", rb_tab);

   rb_comma = new QRadioButton();
   QGridLayout *lyt_comma = us_radiobutton("Comma", rb_comma);

   rb_semicolon = new QRadioButton();
   QGridLayout *lyt_semicolon = us_radiobutton("Semicolon", rb_semicolon);

   rb_space = new QRadioButton();
   QGridLayout *lyt_space = us_radiobutton("Space", rb_space);

   rb_other = new QRadioButton();
   QGridLayout *lyt_other = us_radiobutton("Other: ", rb_other);

   bg_delimiter = new QButtonGroup();
   bg_delimiter->addButton(rb_tab, TAB);
   bg_delimiter->addButton(rb_comma, COMMA);
   bg_delimiter->addButton(rb_semicolon, SEMICOLON);
   bg_delimiter->addButton(rb_space, SPACE);
   bg_delimiter->addButton(rb_other, OTHER);
   rb_tab->setChecked(true);
   delimiter = TAB;

   le_other = us_lineedit("");
   le_other->setMaxLength(3);
   le_other->setMinimumWidth(10);
   le_other->setFrame(false);

   QHBoxLayout* lyt_delimiter = new QHBoxLayout();
   lyt_delimiter->setSpacing(0);
   lyt_delimiter->setMargin(0);
   lyt_delimiter->addLayout(lyt_tab);
   lyt_delimiter->addLayout(lyt_comma);
   lyt_delimiter->addLayout(lyt_semicolon);
   lyt_delimiter->addLayout(lyt_space);
   lyt_delimiter->addLayout(lyt_other);
   lyt_delimiter->addSpacing(2);
   lyt_delimiter->addWidget(le_other);

   pb_add_header = us_pushbutton("Add Header");
   pb_cancel = us_pushbutton("Cancel");
   pb_ok = us_pushbutton("Apply", false);
   pb_save_csv = us_pushbutton("Save CSV");
   pb_reset = us_pushbutton("Reset");
   pb_show_red = us_pushbutton("Show Bad Data");

   QString vert_sts = "QScrollBar:vertical {"
                      "border: 2px solid black;"
                      "background: #DCDCDC;"
                      "width: 15px;"
                      "margin: 20px 0px 20px 0;"
                      "}"
                      "QScrollBar::handle:vertical {"
                      "background: black;"
                      "min-height: 20px;"
                      "}"
                      "QScrollBar::add-line:vertical {"
                      "border: 2px solid black;"
                      "background: grey;"
                      "height: 20px;"
                      "   subcontrol-position: bottom;"
                      "   subcontrol-origin: margin;"
                      "}"
                      "QScrollBar::sub-line:vertical {"
                      "border: 2px solid black;"
                      "background: grey;"
                      "height: 20px;"
                      "   subcontrol-position: top;"
                      "   subcontrol-origin: margin;"
                      "}"
                      "QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical {"
                      "   border: 1px solid #DCDCDC;"
                      "   width: 3px;"
                      "   height: 3px;"
                      "   background: #DCDCDC;"
                      "}";

   QString horz_sts = "QScrollBar:horizontal {"
                      "border: 2px solid black;"
                      "background: #DCDCDC;"
                      "height: 15px;"
                      "margin: 0px 20px 0 20px;"
                      "}"
                      "QScrollBar::handle:horizontal {"
                      "background: black;"
                      "min-width: 20px;"
                      "}"
                      "QScrollBar::add-line:horizontal {"
                      "border: 2px solid black;"
                      "background: grey;"
                      "width: 20px;"
                      "   subcontrol-position: right;"
                      "   subcontrol-origin: margin;"
                      "}"
                      "QScrollBar::sub-line:horizontal {"
                      "border: 2px solid black;"
                      "background: grey;"
                      "width: 20px;"
                      "   subcontrol-position: left;"
                      "   subcontrol-origin: margin;"
                      "}"
                      "QScrollBar::left-arrow:horizontal, QScrollBar::right-arrow:horizontal {"
                      "   border: 1px solid #DCDCDC;"
                      "   width: 3px;"
                      "   height: 3px;"
                      "   background: #DCDCDC;"
                      "}";

   tv_data = new CSVTableView();
   tv_data->verticalScrollBar()->setStyleSheet(vert_sts);
   tv_data->horizontalScrollBar()->setStyleSheet(horz_sts);
   tv_data->setSortingEnabled(true);
   model = new QStandardItemModel(1, 1);
   proxy = new CSVSortFilterProxyModel();
   proxy->setSourceModel(model);
   tv_data->setModel(proxy);
   tv_data->setStyleSheet("background-color: white");

   le_msg = us_lineedit("Note: ", 0, true);
   le_msg->setFrame(false);

   QGridLayout* top_lyt = new QGridLayout();
   top_lyt->addWidget(le_filename,       0, 0, 1, 3);
   top_lyt->addLayout(lyt_delimiter,     1, 0, 1, 3);
   top_lyt->addWidget(pb_reset,          2, 0, 1, 1);
   top_lyt->addWidget(pb_show_red,       2, 1, 1, 1);
   top_lyt->addWidget(pb_add_header,     2, 2, 1, 1);
   top_lyt->addWidget(pb_save_csv,       3, 0, 1, 1);
   top_lyt->addWidget(pb_cancel,         3, 1, 1, 1);
   top_lyt->addWidget(pb_ok,             3, 2, 1, 1);
   top_lyt->addWidget(le_msg,            4, 0, 1, 3);
   top_lyt->setMargin(0);
   top_lyt->setHorizontalSpacing(2);
   top_lyt->setVerticalSpacing(2);

   QFrame* top_frm = new QFrame();
   top_frm->setLayout(top_lyt);
   top_frm->setFrameShape(QFrame::WinPanel);
   top_frm->setFrameShadow(QFrame::Raised);
   top_frm->setLineWidth(3);
   top_frm->setMinimumWidth(550);
   top_frm->setMaximumWidth(650);

   QVBoxLayout* main_lyt = new QVBoxLayout();
   main_lyt->addWidget(top_frm, 0, Qt::AlignCenter);
   main_lyt->addWidget(tv_data, 1);
   main_lyt->setMargin(2);
   main_lyt->setSpacing(1);

   setLayout(main_lyt);

   connect(pb_add_header, &QPushButton::clicked, this, &US_CSV_Loader::add_header);
   connect(pb_ok, &QPushButton::clicked, this, &US_CSV_Loader::ok);
   connect(pb_cancel, &QPushButton::clicked, this, &US_CSV_Loader::cancel);
   connect(le_other, &QLineEdit::textChanged, this, &US_CSV_Loader::new_delimiter);
   connect(pb_save_csv, &QPushButton::clicked, this, &US_CSV_Loader::save_csv_clicked);
   connect(pb_reset, &QPushButton::clicked, this, &US_CSV_Loader::reset);
   connect(tv_data, &CSVTableView::row_column_deleted, this, &US_CSV_Loader::row_column_deleted);
   connect(model, &QStandardItemModel::itemChanged, this, &US_CSV_Loader::item_changed);
   connect(pb_show_red, &QPushButton::clicked, this, &US_CSV_Loader::show_red);
#if QT_VERSION >= QT_VERSION_CHECK(5,15,0)
   connect(bg_delimiter, QOverload<int>::of(&QButtonGroup::idClicked), this, &US_CSV_Loader::fill_table);
#else
   connect(bg_delimiter, QOverload<int>::of(&QButtonGroup::buttonClicked), this, &US_CSV_Loader::fill_table);
#endif

}

QString US_CSV_Loader::error_message() {
   return error_msg;
}

void US_CSV_Loader::reset() {
   if (file_lines.isEmpty()) return;
   delimiter = NONE;
   fill_table(bg_delimiter->checkedId());
}

void US_CSV_Loader::show_red() {
   if (file_lines.isEmpty()) return;
   int nrows = model->rowCount();
   int ncols = model->columnCount();
   if (nrows == 0 || ncols == 0) return;
   int II = -1;
   int JJ = -1;
   for (int ii = 0; ii < nrows; ii++) {
      if (II > -1 && JJ > -1) break;
      for (int jj = 0; jj < ncols; jj++) {
         if (! model->item(ii, jj)->data(Qt::UserRole).toBool()) {
            II = ii;
            JJ = jj;
            break;
         }
      }
   }
   if (II == -1 || JJ == -1) return;
   QModelIndex index = tv_data->model()->index(II, JJ);
   tv_data->scrollTo(index);
}

void US_CSV_Loader::row_column_deleted() {
   check_table();
   relabel();
}

void US_CSV_Loader::item_changed(QStandardItem *item) {
   int row = item->row();
   if (row == 0) {
      if (item->data(Qt::DisplayRole).toString().trimmed().isEmpty()) {
         item->setBackground(Qt::red);
         item->setData(false, Qt::UserRole);
      } else {
         item->setBackground(Qt::green);
         item->setData(true, Qt::UserRole);
      }
   } else {
      bool ok;
      item->data(Qt::DisplayRole).toDouble(&ok);
      if (ok) {
         item->setBackground(Qt::white);
         item->setData(true, Qt::UserRole);
      } else {
         item->setBackground(Qt::red);
         item->setData(false, Qt::UserRole);
      }
   }
   check_table();
}

void US_CSV_Loader::add_header() {
   if (file_lines.size() == 0) return;
   model->disconnect(this);
   model->insertRow(0);
   int nr = model->rowCount();
   int nc = model->columnCount();
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
      it->setEditable(m_editable);
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
   connect(model, &QStandardItemModel::itemChanged, this, &US_CSV_Loader::item_changed);
}

US_CSV_Loader::CSV_Data US_CSV_Loader::data() {
   csv_data.clear();
   if ( check_table()) {
      QVector<QVector<double>> columns;
      QStringList header;
      get_sorted(columns, header);
      csv_data.setData(infile.absoluteFilePath(), header, columns);
   }
   return csv_data;
}

void US_CSV_Loader::relabel() {
   QFont font( US_Widgets::fixedFont().family(),
              US_GuiSettings::fontSize() );
   int nrows = model->rowCount();
   for (int ii = 0; ii < nrows; ii++) {
      QStandardItem *it = new QStandardItem();
      it->setData(font, Qt::FontRole);
      if (ii == 0) it->setData("Header", Qt::DisplayRole);
      else it->setData(QString::number(ii), Qt::DisplayRole);
      model->setVerticalHeaderItem(ii, it);
   }
   // int ncols = model->columnCount();
   // if (hlabel.size() > 0) {
   //    for (int ii = 0; ii < ncols; ii++) {
   //       QStandardItem *it = new QStandardItem();
   //       it->setData(font, Qt::FontRole);
   //       if (ii < hlabel.size()) it->setData(hlabel.at(ii), Qt::DisplayRole);
   //       else it->setData(QString::number(ii + 1), Qt::DisplayRole);
   //       model->setHorizontalHeaderItem(ii, it);
   //    }
   // }
}

void US_CSV_Loader::ok() {
   if (! check_table()) return;
   accept();
}

void US_CSV_Loader::save_csv_clicked() {
   if (file_lines.size() == 0) return;
   if(! check_table()) return;

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

   QString file_path = QFileDialog::getSaveFileName(this, "Save CSV File",
                                                    US_Settings::workBaseDir(), "(CSV)(*.csv)");
   if (file_path.isEmpty()) return;
   QString err_msg;
   bool ok = write_csv(file_path, delimiter_str, err_msg);
   if (ok) QMessageBox::warning(this, "", tr("Data Saved!\n\n").arg(file_path));
   else QMessageBox::warning(this, "Error!", err_msg);
}

bool US_CSV_Loader::write_csv(const QString& fpath,
                              const QString& delimiter_str, QString& err_msg) {

   QString file_path = fpath;
   err_msg.clear();
   if (file_path.size() == 0) {
      err_msg.append("Filename is empty!");
      return false;
   }
   if (! file_path.endsWith(".csv", Qt::CaseInsensitive)) {
      file_path += ".csv";
   }
   QStringList headers;
   QVector<QVector<double>> data;
   get_sorted(data, headers);
   int nrows = data.at(0).size();
   int ncols = data.size();
   QFile file(file_path);
   if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QTextStream ts(&file);
      QString item;
      for (int ii = -1 ; ii < nrows; ii++) {
         for (int jj = 0; jj < ncols; jj++) {
            if (ii == -1) {
               item = headers.at(jj);
            } else {
               item = QString::number(data.at(jj).at(ii));
            }
            ts << item.trimmed();
            if (jj < ncols - 1) ts << delimiter_str;
            else ts << "\n";
         }
      }
      file.close();
      return true;
   } else {
      err_msg.append("Error: Couldn't open the file to write!\n");
      err_msg.append(file_path);
      return false;
   }
}

void US_CSV_Loader::cancel() {
   reject();
}

bool US_CSV_Loader::parse_file(const QString& filepath) {
   QFile file(filepath);
   error_msg.clear();
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
            error_msg = tr("The loaded file is not in text format!\n\n%1").arg(filepath);
            return false;
         }
         file_lines.append(line);
      }
      if (file_lines.size() == 0) {
         error_msg = tr("The loaded file is empty!\n\n%1").arg(filepath);
         return false;
      }
      return true;
   } else {
      error_msg = tr("Couldn't open the file\n\n%1!").arg(filepath);
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
   if (file_lines.size() == 0) return;

   QFont font( US_Widgets::fixedFont().family(),
                 US_GuiSettings::fontSize() );
   // QFontMetrics* fm = new QFontMetrics( font );
   // int rowht = fm->height() + 2;

   model->disconnect(this);
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
         it->setEditable(m_editable);
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
   tv_data->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
   tv_data->resizeColumnsToContents();
   // qDebug() << QDateTime::currentMSecsSinceEpoch() << " fill_table";
   connect(model, &QStandardItemModel::itemChanged, this, &US_CSV_Loader::item_changed);
}

bool US_CSV_Loader::check_table() {
   if (file_lines.isEmpty()) {
      pb_ok->setEnabled(false);
      return false;
   }
   bool ready = true;
   int nrows = model->rowCount();
   int ncols = model->columnCount();
   if (nrows == 0 || ncols == 0) ready =  false;
   for (int ii = 0; ii < nrows; ii++) {
      for (int jj = 0; jj < ncols; jj++) {
         ready = ready && model->item(ii, jj)->data(Qt::UserRole).toBool();
         if (! ready) break;
      }
      if (! ready) break;
   }
   pb_ok->setEnabled(ready);
   return ready;
}

void US_CSV_Loader::get_sorted(QVector<QVector<double>>& data, QStringList& headers) {
   headers.clear();
   data.clear();
   int nrows = tv_data->model()->rowCount();
   int ncols = tv_data->model()->columnCount();
   for (int jj = 0; jj < ncols; jj++) {
      QVector<double> rows;
      for (int ii = 0; ii < nrows; ii++) {
         QModelIndex index = tv_data->model()->index(ii, jj);
         QVariant item = tv_data->model()->itemData(index).value(Qt::DisplayRole);
         if (ii == 0) {
            headers << item.toString();
         } else {
            rows << item.toDouble();
         }
      }
      data << rows;
   }
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

bool US_CSV_Loader::ReadCSV(const QString &filePath, CSV_Data& data, QString& error, const QString &delimiter) {
    QFile file(filePath);
    error.clear();
    data.clear();
    if (! file.exists()) {
        error = "File doesn't exist.";
        return false;
    }

    // Read the text file into a QStringList
    QStringList file_lines;
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
                error = tr("File is not a text format!");
                return false;
            }
            file_lines.append(line);
        }
        if (file_lines.size() == 0) {
            error = tr("File is empty!");
            return false;
        } else if (file_lines.size() == 1) {
            error = tr("File has only one line! It must have at least two lines, including a header line and a data line.");
            return false;
        }
    } else {
        error = tr("Couldn't open the file");
        return false;
    }

    // Parse the QStringList of lines
    QStringList delimiters;
    if (delimiter.isEmpty()) {
        delimiters << "\t" << " " << "," << ";";
    } else {
        delimiters << delimiter;
    }

    int n_columns = 0;
    int n_rows = file_lines.size();
    QVector<QStringList> data_list;
    bool split_status = true;
    int II = 0;
    for (int dd = 0; dd < delimiters.size(); dd++) {
        split_status = true;
        data_list.clear();
        for (int ii = 0; ii < n_rows; ii++ ) {
            QString line = file_lines.at(ii);
            QStringList lsp = line.split(delimiters.at(dd));
            if (ii == 0) {
                n_columns = lsp.size();
            }
            if (n_columns != lsp.size()) {
                split_status = false;
                II = ii + 1;
                break;
            }
            data_list << lsp;
        }
    }

    if (! split_status) {
        error = tr("Cannot split the lines with the given separator. Error at the line: %1").arg(II);
        return false;
    }

    // Check data if it include float numbers
    QStringList header;
    bool droplast = true;
    for (int ii = 0; ii < n_columns; ii++ ) {
        QString cell = data_list.at(0).at(ii).trimmed();
        if (! cell.isEmpty()) {
            droplast = false;
        }
        header << cell;
    }
    QVector<QVector<double>> columns;
    for (int jj = 0; jj < n_columns; jj++) {
        QVector<double> column;
        for (int ii = 1; ii < n_rows; ii++) {
            QString cell = data_list.at(ii).at(jj).trimmed();
            if (jj == n_columns - 1 && !cell.isEmpty()) {
                droplast = false;
            }
            bool ok;
            double val = cell.toDouble(&ok);
            if ( ok ) {
                column << val;
            } else {
                error = tr("Cannot convert the line to the floating numbers. Error at the line: %1").arg(ii + 1);
                return false;
            }
        }
        columns << column;
    }

    if (droplast) {
        header.removeLast();
        columns.removeLast();
    }
    data.setData(filePath, header, columns);
    return true;
}
