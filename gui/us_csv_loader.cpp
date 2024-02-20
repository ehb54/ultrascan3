#include "us_csv_loader.h"
#include "us_gui_settings.h"
#include "us_settings.h"

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

   cb_header = new QCheckBox();
   QGridLayout *lyt_header = us_checkbox("Header", cb_header);

   cb_samesize = new QCheckBox();
   QGridLayout *lyt_samesize = us_checkbox("Same-size Columns", cb_samesize);

   pb_cancel = us_pushbutton("Cancel");
   pb_ok = us_pushbutton("Ok");

   tw_data = new QTableWidget();
   tw_data->setRowCount(20);
   tw_data->setColumnCount(10);
   tw_data-> setHorizontalHeaderLabels(make_labels(10));
   tw_data-> setHorizontalHeaderLabels(make_labels(20));
   tw_data->setStyleSheet("background-color: white");
   QHeaderView *header = tw_data->horizontalHeader();
   header->setSectionResizeMode(QHeaderView::Stretch);

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

   main_lyt->addLayout(lyt_header,        4, 1, 1, 3);
   main_lyt->addLayout(lyt_samesize,      4, 4, 1, 3);

   main_lyt->addWidget(pb_cancel,         5, 3, 1, 2);
   main_lyt->addWidget(pb_ok,             5, 5, 1, 2);

   main_lyt->addWidget(tw_data,           6, 0, 5, 7);

   main_lyt->setSpacing(2);
   main_lyt->setMargin(2);

   setLayout(main_lyt);
   loaded = false;

   connect(pb_open, SIGNAL(clicked()), this, SLOT(open()));
//   connect(rb_tab, SIGNAL(clicked()), this, SLOT(fill_table()));
//   connect(rb_space, SIGNAL(clicked()), this, SLOT(fill_table()));
//   connect(rb_semicolon, SIGNAL(clicked()), this, SLOT(fill_table()));
//   connect(rb_comma, SIGNAL(clicked()), this, SLOT(fill_table()));
//   connect(rb_other, SIGNAL(clicked()), this, SLOT(fill_table()));
   connect(bg_delimiter, SIGNAL(buttonClicked(int)),
           this, SLOT(fill_table(int)));

}

QStringList US_CSV_Loader::make_labels(int number) {
   QStringList labels;
   for (int ii = 0; ii < number; ii++) {
      labels.append(QString::number(ii + 1));
   }
   return labels;
}

void US_CSV_Loader::ok() {

}

void US_CSV_Loader::cancel() {

}

void US_CSV_Loader::open() {
   QString fpath = QFileDialog::getOpenFileName(this, "Open File", US_Settings::workBaseDir(),
                                                   "(DAT, DSP, CSV) (*.dat *.dsp *.csv)");
   if (fpath.isEmpty()) return;
   QFile file(fpath);
   QFileInfo finfo(fpath);
   if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      file_lines.clear();
      QTextStream ts(&file);
      while (true) {
         if (ts.atEnd()) {
            file.close();
            break;
         }
         file_lines.append(ts.readLine());
      }
      loaded = false;
      fill_table(bg_delimiter->checkedId());
   } else {
      QMessageBox::warning(this, "Error!", "Please load a text file!");
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
   QFontMetrics* fm = new QFontMetrics( tw_font );
   int rowht = fm->height() + 2;
   tw_data->clearContents();
//   int min_nc =  1e99;
   int n_columns = static_cast<int>(-1e99);
//   int min_nr =  1e99;
//   int max_nr = -1e99;

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
      for (int jj = 0; jj < data_list.at(ii).size(); jj++) {
         QTableWidgetItem *twi = new QTableWidgetItem(data_list.at(ii).at(jj));
         twi->setFont(tw_font);
         tw_data->setItem(ii, jj, twi);
         //         tw_data->setRowHeight(ii, rowht);
      }
   }


   tw_data->setHorizontalHeaderLabels(make_labels(n_columns));
   tw_data->setVerticalHeaderLabels(make_labels(n_rows));

   qDebug() << QDateTime::currentMSecsSinceEpoch() << " fill_table";



}
