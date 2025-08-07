//! \file us_buoyancy.cpp

#include "us_fds_filemanager.h"

#include <QApplication>
#include <QDomDocument>

#include "us_db2.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "us_license.h"
#include "us_license_t.h"
#include "us_passwd.h"
#include "us_settings.h"
#include "us_util.h"
#if QT_VERSION < 0x050000
#define setSamples(a, b, c) setData(a, b, c)
#define setMinimum(a) setMinValue(a)
#define setMaximum(a) setMaxValue(a)
#endif

#ifndef DbgLv
#define DbgLv(a) \
  if (dbg_level >= a) qDebug()
#endif

//! \brief Main program for US_FDS_FileManager. This program is intended to help
//! the
// user to filter useful files from FDS runs where multiple gains and speeds
// were collected.

int main(int argc, char* argv[]) {
  QApplication application(argc, argv);

#include "main1.inc"

  // License is OK.  Start up.

  US_FDS_FileManager w;
  w.show();                   //!< \memberof QWidget
  return application.exec();  //!< \memberof QApplication
}

// Constructor
US_FDS_FileManager::US_FDS_FileManager() : US_Widgets() {
  setWindowTitle(tr("Fluorescence Data File Manager"));
  setPalette(US_GuiSettings::frameColor());

  QVBoxLayout* top = new QVBoxLayout(this);
  top->setSpacing(2);
  top->setContentsMargins(2, 2, 2, 2);

  // Put the Run Info across the entire window
  QHBoxLayout* runInfo = new QHBoxLayout();
  QLabel* lb_info = us_label(tr("Dataset Info:"), -1);
  runInfo->addWidget(lb_info);

  le_info = us_lineedit("", 1, true);
  le_info->setReadOnly(true);
  runInfo->addWidget(le_info);

  top->addLayout(runInfo);

  QHBoxLayout* main = new QHBoxLayout();
  QVBoxLayout* left = new QVBoxLayout;

  // Start of Grid Layout
  QGridLayout* specs = new QGridLayout;
  int s_row = 0;

  // Row 1
  // Investigator

  QPushButton* pb_load = us_pushbutton(tr("Load Data"));
  connect(pb_load, SIGNAL(clicked()), SLOT(load()));
  specs->addWidget(pb_load, s_row, 0, 1, 1);

  le_directory = us_lineedit("");
  le_directory->setReadOnly(true);
  specs->addWidget(le_directory, s_row++, 1, 1, 3);

  QLabel* lbl_progress = us_label(tr("Progress:"), -1);
  lbl_progress->setAlignment(Qt::AlignLeft);
  specs->addWidget(lbl_progress, s_row, 0, 1, 1);

  progress = us_progressBar(0, 100, 0);
  specs->addWidget(progress, s_row++, 1, 1, 3);

  QLabel* lb_triple = us_label(tr("Cell / Channel / Wavelength:"), -1);
  specs->addWidget(lb_triple, s_row, 0, 1, 2);

  cb_triple = us_comboBox();
  specs->addWidget(cb_triple, s_row++, 2, 1, 2);

  lbl_rpms = us_label(tr("Speed Step (RPM) of triple:"), -1);
  specs->addWidget(lbl_rpms, s_row, 0, 1, 2);

  cb_rpms = us_comboBox();
  specs->addWidget(cb_rpms, s_row++, 2, 1, 2);

  lbl_gains = us_label(tr("Gain Setting:"), -1);
  specs->addWidget(lbl_gains, s_row, 0, 1, 2);

  cb_gains = us_comboBox();
  specs->addWidget(cb_gains, s_row++, 2, 1, 2);

  lbl_scans = us_label(tr("Scans in active Selection:"), -1);
  specs->addWidget(lbl_scans, s_row, 0, 1, 2);

  le_scans = us_lineedit("");
  le_scans->setReadOnly(true);
  specs->addWidget(le_scans, s_row++, 2, 1, 2);

  // Scans focus from:
  lbl_from = us_label(tr("Scan Focus from:"), -1);
  // lbl_from->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
  specs->addWidget(lbl_from, s_row, 0, 1, 2);

  ct_from = us_counter(3, 0.0, 0.0);  // Update range upon load
  ct_from->setSingleStep(1);
  specs->addWidget(ct_from, s_row++, 2, 1, 2);
  connect(ct_from, SIGNAL(valueChanged(double)), SLOT(focus_from(double)));

  // Scan focus to
  lbl_to = us_label(tr("Scan Focus to:"), -1);
  // lbl_to->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
  specs->addWidget(lbl_to, s_row, 0, 1, 2);

  ct_to = us_counter(3, 0.0, 0.0);  // Update range upon load
  ct_to->setSingleStep(1);
  specs->addWidget(ct_to, s_row++, 2, 1, 2);
  connect(ct_to, SIGNAL(valueChanged(double)), SLOT(focus_to(double)));

  // Exclude and Include pushbuttons
  pb_exclude = us_pushbutton(tr("Delete marked Scan(s)"), false);
  specs->addWidget(pb_exclude, s_row, 0, 1, 2);
  connect(pb_exclude, SIGNAL(clicked()), SLOT(exclude_scans()));

  pb_delete_all = us_pushbutton(tr("Delete all Scans"), false);
  specs->addWidget(pb_delete_all, s_row++, 2, 1, 2);
  connect(pb_delete_all, SIGNAL(clicked()), SLOT(delete_all()));

  pb_save_first = us_pushbutton(tr("Save first Scan"), false);
  specs->addWidget(pb_save_first, s_row, 0, 1, 2);
  pb_save_first->setEnabled(false);
  connect(pb_save_first, SIGNAL(clicked()), SLOT(save_first()));

  pb_save_last = us_pushbutton(tr("Save last Scan"), false);
  specs->addWidget(pb_save_last, s_row++, 2, 1, 2);
  pb_save_last->setEnabled(false);
  connect(pb_save_last, SIGNAL(clicked()), SLOT(save_last()));

  pb_save_first_and_last = us_pushbutton(tr("Save first and last Scan"), false);
  specs->addWidget(pb_save_first_and_last, s_row, 0, 1, 2);
  pb_save_first_and_last->setEnabled(false);
  connect(pb_save_first_and_last, SIGNAL(clicked()),
          SLOT(save_first_and_last()));

  pb_delete_triple = us_pushbutton(tr("Delete Triple"), false);
  specs->addWidget(pb_delete_triple, s_row++, 2, 1, 2);
  pb_delete_triple->setEnabled(false);
  connect(pb_delete_triple, SIGNAL(clicked()), SLOT(delete_triple()));

  pb_undo = us_pushbutton(tr("Undo last Delete"), false);
  specs->addWidget(pb_undo, s_row, 0, 1, 2);
  pb_undo->setEnabled(false);
  connect(pb_undo, SIGNAL(clicked()), SLOT(undo()));

  pb_write = us_pushbutton(tr("Export File Selection"), false);
  connect(pb_write, SIGNAL(clicked()), SLOT(write()));
  specs->addWidget(pb_write, s_row++, 2, 1, 2);

  // Modify export prefix:
  lbl_prefix = us_label(tr("Prefix for export Filename:"), -1);
  // lbl_prefix->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
  specs->addWidget(lbl_prefix, s_row, 0, 1, 2);

  ct_prefix = us_counter(3, 0.0, 10000.0);
  ct_prefix->setSingleStep(1);
  specs->addWidget(ct_prefix, s_row++, 2, 1, 2);
  connect(ct_prefix, SIGNAL(valueChanged(double)), SLOT(update_prefix(double)));

  // Button rows
  QBoxLayout* buttons = new QHBoxLayout;

  QPushButton* pb_reset = us_pushbutton(tr("Reset"));
  connect(pb_reset, SIGNAL(clicked()), SLOT(reset()));
  buttons->addWidget(pb_reset);

  QPushButton* pb_help = us_pushbutton(tr("Help"));
  connect(pb_help, SIGNAL(clicked()), SLOT(help()));
  buttons->addWidget(pb_help);

  QPushButton* pb_accept = us_pushbutton(tr("Close"));
  connect(pb_accept, SIGNAL(clicked()), SLOT(close()));
  buttons->addWidget(pb_accept);

  // Plot layout on right side of window
  plot = new US_Plot(data_plot, tr("Fluorescence Data"), tr("Radius (in cm)"),
                     tr("Fluorescence Intensity"));

  data_plot->setMinimumSize(600, 400);
  data_plot->enableAxis(QwtPlot::xBottom, true);
  data_plot->enableAxis(QwtPlot::yLeft, true);

  left->addLayout(specs);
  left->addStretch();
  left->addLayout(buttons);

  main->addLayout(left);
  main->addLayout(plot);
  main->setStretchFactor(plot, 3);
  top->addLayout(main);

  reset();
}

// Load an AUC data set
void US_FDS_FileManager::load(void) {
  reset();
  QFileDialog fd;
  fd.setFileMode(QFileDialog::Directory);
  source_dir = fd.getExistingDirectory(this, tr("Raw Data Directory"),
                                       US_Settings::importDir(),
                                       QFileDialog::DontResolveSymlinks);

  source_dir.replace("\\", "/");

  if (source_dir.isEmpty())  // If no directory chosen, return now
  {
    return;
  } else {
    le_directory->setText(source_dir);
  }

  QDir readDir(source_dir, "*.[F,f][I,i][1-8]", QDir::Name,
               QDir::Files | QDir::Readable);
  readDir.makeAbsolute();
  if (source_dir.right(1) != "/") source_dir += "/";  // Ensure trailing "/"
  files =
      QDir(readDir).entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
  parse_files();
}

// parse all selected files into scaninfo
void US_FDS_FileManager::parse_files(void) {
  cb_triple->disconnect();
  cb_triple->clear();
  scaninfo.clear();
  QString line, str, str1, str2;
  QStringList tokens, triplelist;
  int i;
  triplelist.clear();
  for (i = 0; i < files.size(); i++) {
    tmp_scaninfo.filename = files.at(i);
    if (files.at(i).contains("A", Qt::CaseInsensitive)) {
      tmp_scaninfo.channel = "A";
    } else {
      tmp_scaninfo.channel = "B";
    }
    QFile f(source_dir + tmp_scaninfo.filename);
    if (f.open(QFile::ReadOnly)) {
      QTextStream ts(&f);
      line = ts.readLine();
      tokens = line.split(" ", QString::SkipEmptyParts);
      tmp_scaninfo.date = tokens.at(0);
      tmp_scaninfo.time = tokens.at(1) + " " + tokens.at(2);
      tmp_scaninfo.voltage = tokens.at(4).toInt();
      tmp_scaninfo.gain = tokens.at(6).toInt();
      tmp_scaninfo.range = tokens.at(8).toInt();
      tmp_scaninfo.include = true;
      tokens.clear();
      line = ts.readLine();
      tokens = line.split(" ", QString::SkipEmptyParts);
      tmp_scaninfo.cell = tokens.at(1).toInt();
      tmp_scaninfo.rpm = tokens.at(3).toInt();
      tmp_scaninfo.seconds = tokens.at(4).toDouble();
      tmp_scaninfo.omega_s = tokens.at(4).toDouble();
      tmp_scaninfo.lambda = tokens.at(6).toInt();
      tmp_scaninfo.triple = str1.setNum(tmp_scaninfo.cell) + " / " +
                            tmp_scaninfo.channel + " / " +
                            str2.setNum(tmp_scaninfo.lambda);
      tmp_scaninfo.gainset = "V: " + str1.setNum(tmp_scaninfo.voltage) +
                             ", G: " + str.setNum(tmp_scaninfo.gain) +
                             ", R: " + str2.setNum(tmp_scaninfo.range);
      tmp_scaninfo.include = true;
      tmp_scaninfo.x.clear();
      tmp_scaninfo.y.clear();
      while (!ts.atEnd()) {
        line = ts.readLine();
        tokens.clear();
        tokens = line.split(" ", QString::SkipEmptyParts);
        tmp_scaninfo.x.append(tokens.at(0).toDouble());
        tmp_scaninfo.y.append(tokens.at(1).toDouble());
      }
    }
    triplelist.append(tmp_scaninfo.triple);
    progress->setValue(100 * (i + 1.0) / files.size());
    scaninfo.append(tmp_scaninfo);
  }
  if (scaninfo.size() == 0) {
    QMessageBox::warning(
        this, tr("Attention:"),
        tr("The selected directory does not contain fluorescence files...\n"
           "Please try again."),
        QMessageBox::Ok);
    return;
  }
  triplelist.removeDuplicates();
  triplelist.sort();
  cb_triple->addItems(triplelist);
  connect(cb_triple, SIGNAL(currentIndexChanged(int)),
          SLOT(select_triple(int)));
  select_triple(cb_triple->currentIndex());
}

// Select a new triple
void US_FDS_FileManager::select_triple(int index) {
  cb_rpms->disconnect();
  cb_rpms->clear();
  current_triple = index;
  QStringList rpmlist;
  rpmlist.clear();
  QString str;
  for (int i = 0; i < scaninfo.size(); i++) {
    if (scaninfo[i].triple == cb_triple->currentText() && scaninfo[i].include) {
      rpmlist.append(str.setNum(scaninfo[i].rpm));
    }
  }
  rpmlist.removeDuplicates();
  rpmlist.sort();
  cb_rpms->addItems(rpmlist);
  connect(cb_rpms, SIGNAL(currentIndexChanged(int)), SLOT(select_rpm(int)));
  select_rpm(cb_rpms->currentIndex());
}

// Select a new speed within a triple
void US_FDS_FileManager::select_rpm(int index) {
  cb_gains->disconnect();
  cb_gains->clear();
  current_rpm = index;
  QStringList gainlist;
  gainlist.clear();
  QString str1;
  gainlist.clear();
  for (int i = 0; i < scaninfo.size(); i++) {
    if (scaninfo[i].triple == cb_triple->currentText() &&
        str1.setNum(scaninfo[i].rpm) == cb_rpms->currentText() &&
        scaninfo[i].include) {
      gainlist.append(scaninfo[i].gainset);
    }
  }
  gainlist.removeDuplicates();
  gainlist.sort();
  cb_gains->addItems(gainlist);
  connect(cb_gains, SIGNAL(currentIndexChanged(int)), SLOT(select_gain(int)));
  select_gain(cb_gains->currentIndex());
}

void US_FDS_FileManager::select_gain(int index) {
  QString str, str1, str2, str3;
  current_gain = index;
  ct_from->disconnect();
  ct_to->disconnect();
  to = 0;
  from = 0;
  ct_from->setValue(0.0);
  ct_to->setValue(0.0);
  connect(ct_from, SIGNAL(valueChanged(double)), SLOT(focus_from(double)));
  connect(ct_to, SIGNAL(valueChanged(double)), SLOT(focus_to(double)));
  le_info->setText(
      "Loaded " + str.setNum(scaninfo.size()) + " scans containing " +
      str1.setNum(cb_triple->count()) + " triples. The current triple (" +
      cb_triple->currentText() + ") contains " + str2.setNum(cb_rpms->count()) +
      " speed(s) and " + str3.setNum(cb_gains->count()) + " gain setting(s).");
  plot_scans();
}

// Plot a single scan curve
void US_FDS_FileManager::plot_scans(void) {
  QString str1;
  QList<QwtPlotCurve*> c;
  c.clear();
  scanindex.clear();
  dataPlotClear(data_plot);

  for (int i = 0; i < scaninfo.size(); i++) {
    if (scaninfo[i].triple == cb_triple->currentText() &&
        scaninfo[i].rpm == cb_rpms->currentText().toInt() &&
        scaninfo[i].gainset == cb_gains->currentText() && scaninfo[i].include) {
      scanindex.append(i);
    }
  }

  ct_from->setMaximum(scanindex.size());
  ct_to->setMaximum(scanindex.size());
  int npts;
  QwtPlotCurve* curve;
  c.clear();
  if (scanindex.size() == 1) {
    le_scans->setText("1 scan");
  } else {
    le_scans->setText(str1.setNum(scanindex.size()) + " scans");
  }
  for (int i = 0; i < scanindex.size(); i++) {
    npts = scaninfo[scanindex[i]].x.size();
    double x[npts], y[npts];
    for (int j = 0; j < npts; j++) {
      x[j] = scaninfo[scanindex[i]].x.at(j);
      y[j] = scaninfo[scanindex[i]].y.at(j);
    }
    str1.setNum(i + 1);
    curve = us_curve(data_plot, "Scan " + str1);
    curve->setSamples(x, y, npts);
    c.append(curve);
    if (from == 0 && to == 0) {  // all scans are yellow
      curve->setPen(QPen(Qt::yellow));
    } else if (i >= from - 1 && i <= to - 1) {
      curve->setPen(QPen(Qt::red));
    } else {
      curve->setPen(QPen(Qt::yellow));
    }
    data_plot->setAxisAutoScale(QwtPlot::yLeft);
    data_plot->setAxisAutoScale(QwtPlot::xBottom);
  }
  data_plot->replot();
  if (c.size() > 1) {
    pb_delete_all->setEnabled(true);
    pb_save_first->setEnabled(true);
    pb_save_last->setEnabled(true);
    pb_save_first_and_last->setEnabled(true);
    pb_delete_triple->setEnabled(true);
  } else if (c.size() == 1) {
    pb_delete_all->setEnabled(true);
    pb_save_first->setEnabled(false);
    pb_save_last->setEnabled(false);
    pb_save_first_and_last->setEnabled(false);
    pb_delete_triple->setEnabled(true);
  } else if (c.size() == 0) {
    pb_delete_all->setEnabled(false);
    pb_save_first->setEnabled(false);
    pb_save_last->setEnabled(false);
    pb_save_first_and_last->setEnabled(false);
    pb_delete_triple->setEnabled(true);
  }
  pb_write->setEnabled(true);
}

// Reset parameters to their defaults
void US_FDS_FileManager::reset(void) {
  dataPlotClear(data_plot);

  data_plot->setAxisScale(QwtPlot::xBottom, 5.7, 7.3);
  data_plot->setAxisScale(QwtPlot::yLeft, 0.0, 1.5);

  data_plot->replot();

  ct_from->setMinimum(0);
  ct_from->setMaximum(0);
  ct_from->setValue(0);
  ct_to->setMinimum(0);
  ct_to->setMaximum(0);
  ct_to->setValue(0);
  ct_prefix->setValue(0);

  prefix = "";

  progress->setValue(0);
  cb_triple->clear();
  cb_rpms->clear();
  cb_gains->clear();
  scaninfo.clear();
  scanindex.clear();
  tmp_scanindex.clear();

  pb_delete_all->setEnabled(false);
  pb_save_first->setEnabled(false);
  pb_save_last->setEnabled(false);
  pb_save_first_and_last->setEnabled(false);
  pb_write->setEnabled(false);
  pb_undo->setEnabled(false);

  le_scans->setText("");
  le_directory->setText("");
  le_info->setText("");
}

// undo the latest delete action
void US_FDS_FileManager::undo(void) {
  if (undo_triple == "") {
    for (int i = 0; i < tmp_scanindex.size(); i++) {
      scaninfo[tmp_scanindex[i]].include = true;
    }
    pb_undo->setEnabled(false);
    tmp_scanindex.clear();
    for (int i = 0; i < scanindex.size(); i++) {
      tmp_scanindex.append(scanindex.at(i));
    }
  } else {  // add back entire triple:
    for (int i = 0; i < scaninfo.size(); i++) {
      if (scaninfo.at(i).triple == undo_triple) {
        scaninfo[i].include = true;
      }
    }
  }
  plot_scans();
}

// activate the undo button once a delete action was performed
void US_FDS_FileManager::activate_undo(void) {
  pb_undo->setEnabled(true);
  undo_triple =
      "";  // reset when delete other than delete_triple was performed.
  tmp_scanindex = scanindex;
  plot_scans();
}

void US_FDS_FileManager::activate_undo(QString triple) {
  pb_undo->setEnabled(true);
  undo_triple = triple;
  plot_scans();
}

// delete all scans shown in the plot area
void US_FDS_FileManager::delete_all(void) {
  for (int i = 0; i < scanindex.size(); i++) {
    scaninfo[scanindex[i]].include = false;
  }
  activate_undo();
}

// only save the first scan, delete all others shown (for initial meniscus)
void US_FDS_FileManager::save_first(void) {
  for (int i = 1; i < scanindex.size(); i++) {
    scaninfo[scanindex[i]].include = false;
  }
  activate_undo();
}

// save the last scan only, delete all others shown on screen (last one is at
// equilibrium)
void US_FDS_FileManager::save_last(void) {
  for (int i = 0; i < scanindex.size() - 1; i++) {
    scaninfo[scanindex[i]].include = false;
  }
  activate_undo();
}

// for first speed, the first scan is useful for meniscus, the last is at
// equilibrium
void US_FDS_FileManager::save_first_and_last(void) {
  for (int i = 1; i < scanindex.size() - 1; i++) {
    scaninfo[scanindex[i]].include = false;
  }
  activate_undo();
}

// Delete all scans belonging to a particular triple
void US_FDS_FileManager::delete_triple(void) {
  for (int i = 0; i < scaninfo.size(); i++) {
    if (scaninfo.at(i).triple == cb_triple->currentText()) {
      scaninfo[i].include = false;
    }
  }
  activate_undo(cb_triple->currentText());
}

// exclude only the visually selected (highlighted) scans
void US_FDS_FileManager::exclude_scans(void) {
  for (int i = 0; i < scanindex.size(); i++) {
    if (from == 0 && to == 0) {  // include all scans (should never happen)
      scaninfo[scanindex[i]].include = true;
    } else if (i >= from - 1 && i <= to - 1) {
      scaninfo[scanindex[i]].include = false;
    } else {
      scaninfo[scanindex[i]].include = true;
    }
  }

  from = 0;
  to = 0;

  ct_from->disconnect();
  ct_from->setValue(0.0);
  ct_to->disconnect();
  ct_to->setValue(0.0);

  connect(ct_from, SIGNAL(valueChanged(double)), SLOT(focus_from(double)));
  connect(ct_to, SIGNAL(valueChanged(double)), SLOT(focus_to(double)));
  pb_exclude->setEnabled(false);
  activate_undo();
}

// sets the scan focus for the start of the excluded scan range
void US_FDS_FileManager::focus_from(double scan) {
  from = (int)scan;
  to = (int)ct_to->value();

  if (from > to) {
    ct_to->disconnect();
    ct_to->setValue(scan);
    to = from;
    connect(ct_to, SIGNAL(valueChanged(double)), SLOT(focus_to(double)));
  }
  focus(from, to);
}

// sets the scan focus for the end of the excluded scan range
void US_FDS_FileManager::focus_to(double scan) {
  to = (int)scan;
  from = (int)ct_from->value();

  if (from > to) {
    ct_from->disconnect();
    ct_from->setValue(scan);
    from = to;

    connect(ct_from, SIGNAL(valueChanged(double)), SLOT(focus_from(double)));
  }
  focus(from, to);
}

// function for activating the exclude button
void US_FDS_FileManager::focus(int from, int to) {
  if (from == 0 && to == 0) {
    pb_exclude->setEnabled(false);
  } else {
    pb_exclude->setEnabled(true);
  }
  plot_scans();
}

// function for updating the prefix for export filenames:
void US_FDS_FileManager::update_prefix(double val) {
  QString str;
  prefix = str.setNum(val);
  if (prefix == "0") {
    prefix = "";
  } else {
    prefix = prefix + "_";
  }
}

// write all remaining files to a new directory with successive filenames
void US_FDS_FileManager::write(void) {
  int i, j;
  QFileDialog fd;
  QString target_dir;
  fd.setFileMode(QFileDialog::Directory);
  target_dir = fd.getExistingDirectory(
      this,
      tr("Please select or create an output directory for the ordered "
         "fluorescence data (ANY EXISITNG DATA WILL BE DELETED!!):"),
      US_Settings::importDir(), QFileDialog::DontResolveSymlinks);

  target_dir.replace("\\", "/");

  if (target_dir.isEmpty())  // If no directory chosen, return now
  {
    return;
  } else {
    le_info->setText("Writing all files to " + target_dir);
    QDir target(target_dir);
    target.remove("*");
  }
  if (target_dir.right(1) != "/") target_dir += "/";  // Ensure trailing "/"
  QStringList triples;
  QString filename, str1, str2;
  TripleIndex tmp_tripleCounts;
  triples.clear();
  tripleCounts.clear();
  for (i = 0; i < scaninfo.size(); i++) {
    if (scaninfo[i].include) {  // filter out the triples that are not excluded
      triples << scaninfo[i].triple;
    }
  }
  triples.removeDuplicates();  // get a simple list
  for (i = 0; i < triples.size(); i++) {
    tmp_tripleCounts.name = triples.at(i);
    tmp_tripleCounts.index = 0;
    tripleCounts.append(tmp_tripleCounts);
  }
  progress->setValue(0);
  QFile f;
  for (i = 0; i < scaninfo.size(); i++) {
    if (scaninfo[i].include) {  // increment filename and write to new location
      j = 0;
      while (scaninfo[i].triple != tripleCounts[j].name) j++;
      tripleCounts[j].index++;
      if (tripleCounts[j].index < 10) {
        filename = target_dir + prefix + scaninfo.at(i).channel + "0000" +
                   str1.setNum(tripleCounts[j].index) + ".FI" +
                   str2.setNum(scaninfo.at(i).cell);
      } else if (tripleCounts[j].index > 9 && tripleCounts[j].index < 100) {
        filename = target_dir + prefix + scaninfo.at(i).channel + "000" +
                   str1.setNum(tripleCounts[j].index) + ".FI" +
                   str2.setNum(scaninfo.at(i).cell);
      } else if (tripleCounts[j].index > 99 && tripleCounts[j].index < 1000) {
        filename = target_dir + prefix + scaninfo.at(i).channel + "00" +
                   str1.setNum(tripleCounts[j].index) + ".FI" +
                   str2.setNum(scaninfo.at(i).cell);
      } else if (tripleCounts[j].index > 999 && tripleCounts[j].index < 10000) {
        filename = target_dir + prefix + scaninfo.at(i).channel + "0" +
                   str1.setNum(tripleCounts[j].index) + ".FI" +
                   str2.setNum(scaninfo.at(i).cell);
      } else if (tripleCounts[j].index > 9999 &&
                 tripleCounts[j].index < 100000) {
        filename = target_dir + prefix + scaninfo.at(i).channel +
                   str1.setNum(tripleCounts[j].index) + ".FI" +
                   str2.setNum(scaninfo.at(i).cell);
      }
      if (!f.copy(source_dir + scaninfo.at(i).filename, filename)) {
        str1 = tr("The file:\n\n   ") + filename +
               tr("\n\nalready exists in:\n\n   ") + target_dir + "\n\n" +
               tr("Please use a different directory or prefix, or create a new "
                  "directory and try again.");
        QMessageBox::warning(this, tr("Attention:"), str1, QMessageBox::Ok);
        return;
      }
    }
    progress->setValue(100 * (i + 1) / scaninfo.size());
  }
}
