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
    emit row_deleted();
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
    emit column_deleted();
}

US_CSV_Loader::US_CSV_Loader(const QString& filePath, const QString& note,
                             bool editable,QWidget* parent) : US_WidgetsDialog(parent, 0)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
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
    QApplication::restoreOverrideCursor();
}

void US_CSV_Loader::set_UI() {
    setWindowTitle( tr( "Load CSV Files" ) );
    setPalette( US_GuiSettings::frameColor() );
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
    QRect geometry = QApplication::primaryScreen()->geometry();
    int w = geometry.width() * 0.6;
    int h = geometry.height() * 0.6;
    int x = (geometry.width() - w) / 2;
    int y = (geometry.height() - h) / 2;
    resize(w, h);
    move(x, y);

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
    connect(tv_data, &CSVTableView::column_deleted, this, &US_CSV_Loader::column_deleted);
    connect(tv_data, &CSVTableView::row_deleted, this, &US_CSV_Loader::row_deleted);
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

void US_CSV_Loader::row_deleted() {
    check_header();
    check_table();
}

void US_CSV_Loader::column_deleted() {
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

US_CSV_Data US_CSV_Loader::data() {
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
    csv_data.clear();
    if (! check_table()) return;
    QString error;
    make_csv_data(error);
    accept();
}

void US_CSV_Loader::save_csv_clicked() {
    csv_data.clear();
    if (file_lines.size() == 0) return;
    if(! check_table()) return;
    QString error;
    if (! make_csv_data(error)) {
        QMessageBox::warning(this, "Error!", "Error in making the CSV data!\n\n" + error);
        return;
    }

    QString delimiter_str;
    QString user_delimiter = le_other->text().trimmed();
    int state = QMessageBox::question(this, "Set Delimiter", "Do you want to save it with a different delimiter?");
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
    csv_data.setFilePath(file_path);

    if (csv_data.writeFile(delimiter_str)) {
        QMessageBox::warning(this, "", tr("CSV File Saved!\n%1").arg(file_path));
    } else {
        QMessageBox::warning(this, "Error!", tr("Error in saving the CSV file!\n%1").arg(csv_data.error()));
        csv_data.clear();
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
            QString line = ts.readLine().trimmed();
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
                error_msg = tr("Cannot open non-text files!\n\n%1").arg(filepath);
                return false;
            }
            if (! line.isEmpty() ) {
                file_lines.append(line);
            }
        }
        if (file_lines.size() == 0) {
            error_msg = tr("File is empty!\n\n%1").arg(filepath);
            return false;
        }
        return true;
    } else {
        error_msg = tr("Cannot open the file\n\n%1!").arg(filepath);
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
    QApplication::setOverrideCursor(Qt::WaitCursor);

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
    bool is_header = false;
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
                    is_header = true;
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

    if (! is_header) {
        QStringList headers = gen_alpha_list(n_columns);
        for (int ii = 0; ii < n_columns; ii++) {
            model->item(0, ii)->setData(headers.at(ii), Qt::DisplayRole);
            model->item(0, ii)->setData(true, Qt::UserRole);
            model->item(0, ii)->setData(font, Qt::FontRole);
            model->item(0, ii)->setBackground(Qt::green);
            model->item(0, ii)->setEditable(m_editable);
        }
    }

    // model->sort(0, Qt::DescendingOrder);
    proxy->setSourceModel(model);
    tv_data->setModel(proxy);
    relabel();
    check_table();
    tv_data->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    tv_data->resizeColumnsToContents();
    // qDebug() << QDateTime::currentMSecsSinceEpoch() << " fill_table";
    connect(model, &QStandardItemModel::itemChanged, this, &US_CSV_Loader::item_changed);
    tv_data->sortByColumn(0, Qt::AscendingOrder);
    QApplication::restoreOverrideCursor();
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

void US_CSV_Loader::check_header() {
    if (file_lines.size() == 0) return;
    model->disconnect(this);
    int nc = model->columnCount();
    for (int ii = 0; ii < nc; ii++) {
        QString val = model->item(1, ii)->data(Qt::DisplayRole).toString();
        if (! val.isEmpty()) {
            model->item(0, ii)->setBackground(Qt::green);
            model->item(0, ii)->setData(true, Qt::UserRole);
        }
        else {
            model->item(0, ii)->setBackground(Qt::red);
            model->item(0, ii)->setData(false, Qt::UserRole);
        }
    }
    connect(model, &QStandardItemModel::itemChanged, this, &US_CSV_Loader::item_changed);
}


bool US_CSV_Loader::make_csv_data(QString&error) {
    csv_data.clear();
    error.clear();
    QStringList headers;
    QVector < QVector < double > > data;
    int nrows = tv_data->model()->rowCount();
    int ncols = tv_data->model()->columnCount();
    for (int jj = 0; jj < ncols; jj++) {
        QVector<double> column;
        for (int ii = 0; ii < nrows; ii++) {
            QModelIndex index = tv_data->model()->index(ii, jj);
            QVariant item = tv_data->model()->itemData(index).value(Qt::DisplayRole);
            if (ii == 0) {
                headers << item.toString();
            } else {
                column << item.toDouble();
            }
        }
        data << column;
    }
    if ( csv_data.setData(infile.fileName(), headers, data) ) {
        return true;
    } else {
        error = csv_data.error();
        csv_data.clear();
        return false;
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
