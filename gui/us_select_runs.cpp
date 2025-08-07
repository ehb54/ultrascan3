//! \file us_select_runs.cpp

#include "us_select_runs.h"

#include "us_constants.h"
#include "us_dataIO.h"
#include "us_db2.h"
#include "us_editor.h"
#include "us_gui_settings.h"
#include "us_investigator.h"
#include "us_matrix.h"
#include "us_passwd.h"
#include "us_settings.h"
#include "us_util.h"

// Main constructor with flags for edit, latest-edit and local-data

US_SelectRuns::US_SelectRuns(bool dbase, QStringList& runIDs)
    : US_WidgetsDialog(0, 0), runIDs(runIDs) {
  sel_db = dbase;

  setWindowTitle(tr("Select Run(s) as Models Pre-Filter (%1)")
                     .arg(sel_db ? "DB" : "Local"));
  setPalette(US_GuiSettings::frameColor());
  setMinimumSize(480, 300);

  // Main layout
  QVBoxLayout* main = new QVBoxLayout(this);
  main->setContentsMargins(2, 2, 2, 2);
  main->setSpacing(2);

  // Top layout: buttons and fields above list widget
  QGridLayout* top = new QGridLayout;
  int ddstate = sel_db ? US_Disk_DB_Controls::DB : US_Disk_DB_Controls::Disk;

  dkdb_cntrls = new US_Disk_DB_Controls(ddstate);
  connect(dkdb_cntrls, SIGNAL(changed(bool)), this, SLOT(update_disk_db(bool)));

  pb_invest = us_pushbutton(tr("Select Investigator"));
  QString invnum = QString::number(US_Settings::us_inv_ID()) + ": ";
  QString invusr = US_Settings::us_inv_name();
  le_invest = us_lineedit(invnum + invusr, 0, true);
  connect(pb_invest, SIGNAL(clicked()), SLOT(get_person()));
  if (US_Settings::us_inv_level() < 3) pb_invest->setEnabled(false);

  // Search line
  QLabel* lb_filtdata = us_label(tr("Search"));
  le_dfilter = us_lineedit();

  int row = 0;
  top->addLayout(dkdb_cntrls, row++, 0, 1, 4);
  top->addWidget(pb_invest, row, 0, 1, 2);
  top->addWidget(le_invest, row++, 2, 1, 2);
  top->addWidget(lb_filtdata, row, 0, 1, 1);
  top->addWidget(le_dfilter, row++, 1, 1, 3);

  connect(le_dfilter, SIGNAL(textChanged(const QString&)),
          SLOT(search(const QString&)));

  main->addLayout(top);

  QFont font(US_GuiSettings::fontFamily(), US_GuiSettings::fontSize());

  QStringList headers;
  headers << "Run" << "Date" << "dbID" << "Label";
  tw_data = new QTableWidget(20, 4, this);
  tw_data->setFrameStyle(QFrame::NoFrame);
  tw_data->setPalette(US_GuiSettings::editColor());
  tw_data->setFont(font);
  tw_data->setSelectionMode(QAbstractItemView::ExtendedSelection);
  tw_data->setSelectionBehavior(QAbstractItemView::SelectRows);
  tw_data->setHorizontalHeaderLabels(headers);
  tw_data->verticalHeader()->hide();
  tw_data->setShowGrid(false);
  tw_data->setColumnWidth(0, 250);
  tw_data->setColumnWidth(1, 100);
  tw_data->setColumnWidth(2, 50);
  tw_data->setColumnWidth(3, 350);

  main->addWidget(tw_data);

  // Button Row
  QHBoxLayout* buttons = new QHBoxLayout;

  QPushButton* pb_help = us_pushbutton(tr("Help"));
  connect(pb_help, SIGNAL(clicked()), SLOT(help()));
  buttons->addWidget(pb_help);

  QPushButton* pb_cancel = us_pushbutton(tr("Cancel"));
  connect(pb_cancel, SIGNAL(clicked()), SLOT(cancelled()));
  buttons->addWidget(pb_cancel);

  QPushButton* pb_accept = us_pushbutton(tr("Select PreFilter(s)"));
  connect(pb_accept, SIGNAL(clicked()), SLOT(accepted()));
  buttons->addWidget(pb_accept);

  main->addLayout(buttons);

  // List from disk or db source
  list_data();
  resize(720, 360);
}

void US_SelectRuns::search(const QString& search_string) {
  bool have_search = !search_string.isEmpty();
  QFont tw_font(US_Widgets::fixedFont().family(), US_GuiSettings::fontSize());
  QFontMetrics* fm = new QFontMetrics(tw_font);
  int rowht = fm->height() + 2;
  tw_data->clearContents();
  tw_data->setSortingEnabled(false);

  for (int ii = 0; ii < rlabels.size(); ii++) {
    QTableWidgetItem* twi;
    QString runID = rlabels.at(ii);

    if (have_search && !runID.contains(search_string, Qt::CaseInsensitive))
      continue;

    twi = new QTableWidgetItem(runID);
    twi->setFlags(twi->flags() ^ Qt::ItemIsEditable);
    tw_data->setItem(ii, 0, twi);
    twi = new QTableWidgetItem(runmap[runID].date);
    twi->setFlags(twi->flags() ^ Qt::ItemIsEditable);
    tw_data->setItem(ii, 1, twi);
    twi = new QTableWidgetItem(runmap[runID].DB_id);
    twi->setFlags(twi->flags() ^ Qt::ItemIsEditable);
    tw_data->setItem(ii, 2, twi);
    twi = new QTableWidgetItem(runmap[runID].label);
    twi->setFlags(twi->flags() ^ Qt::ItemIsEditable);
    tw_data->setItem(ii, 3, twi);

    tw_data->setRowHeight(ii, rowht);
  }

  tw_data->setSortingEnabled(true);
  tw_data->sortByColumn(1, Qt::DescendingOrder);
  tw_data->resizeColumnsToContents();
  tw_data->adjustSize();
  int twwid = size().width() - 4;
  int twhgt = qMax(height(), tw_data->size().height());
  tw_data->resize(twwid, twhgt);
  qApp->processEvents();
}

// List data choices (from db or disk)
void US_SelectRuns::list_data() {
  QFont tw_font(US_Widgets::fixedFont().family(), US_GuiSettings::fontSize());
  QFontMetrics* fm = new QFontMetrics(tw_font);
  int rowht = fm->height() + 2;
  runmap.clear();
  rlabels.clear();

  if (sel_db)  // Scan database data
  {
    scan_dbase_run();
  } else  // Scan local disk data
  {
    scan_local_run();
  }

  tw_data->clearContents();

  if (rlabels.size() == 0) {
    QString clabel = tr("No data found.");
    tw_data->setItem(0, 0, new QTableWidgetItem(clabel));
    return;
  }

  tw_data->setSortingEnabled(false);
  tw_data->setRowCount(rlabels.size());

  for (int ii = 0; ii < rlabels.size();
       ii++) {  // Propagate list widget with labels
    QString runID = rlabels.at(ii);

    tw_data->setItem(ii, 0, new QTableWidgetItem(runID));
    tw_data->setItem(ii, 1, new QTableWidgetItem(runmap[runID].date));
    tw_data->setItem(ii, 2, new QTableWidgetItem(runmap[runID].DB_id));
    tw_data->setItem(ii, 3, new QTableWidgetItem(runmap[runID].label));

    tw_data->setRowHeight(ii, rowht);
  }

  tw_data->setSortingEnabled(true);
  tw_data->sortByColumn(1, Qt::DescendingOrder);
  tw_data->resizeColumnsToContents();
  tw_data->adjustSize();
  int twwid = size().width() - 4;
  int twhgt = qMax(height(), tw_data->size().height());
  tw_data->resize(twwid, twhgt);
  qApp->processEvents();
}

// Cancel button:  no editIDs returned
void US_SelectRuns::cancelled() {
  runIDs.clear();
  reject();
  close();
}

// Accept button:  set up to return editID pre-filter information
void US_SelectRuns::accepted() {
  runIDs.clear();

  QList<QTableWidgetItem*> selitems = tw_data->selectedItems();

  if (selitems.size() == 0) {
    QMessageBox::information(
        this, tr("No Data Selected"),
        tr("You have not selected any data.\nSelect or Cancel"));
    return;
  }

  // Get and return runIDs from selected runID items

  for (int ii = 0; ii < selitems.size(); ii++) {  // Loop thru selected runIDs
    QTableWidgetItem* twi = selitems.at(ii);
    if (twi->column() != 0) {
      int row = twi->row();
      twi = tw_data->item(row, 0);
    }
    QString runID = twi->text();
    if (!runIDs.contains(runID)) runIDs << runID;
  }
  qDebug() << "Acpt: runIDs size" << runIDs.size();
  qDebug() << "Acpt: runIDs" << runIDs;
  if (sel_db) {
    US_Passwd pw;
    US_DB2 db(pw.getPasswd());
    QString invID = QString::number(US_Settings::us_inv_ID());
    QStringList query;
    query.clear();
    query << "count_models_by_runID" << invID << runIDs[0];
    int nmodl = db.functionQuery(query);
    qDebug() << "Acpt: runID" << runIDs[0] << "count_models_by_runID" << nmodl;
  }

  accept();  // Signal that selection was accepted
  close();
}

// Scan database for run sets
void US_SelectRuns::scan_dbase_run() {
  // qDebug() << "ScDB:TM:00: " <<
  // QTime::currentTime().toString("hh:mm:ss:zzzz");
  US_Passwd pw;
  US_DB2 db(pw.getPasswd());

  if (db.lastErrno() != US_DB2::OK) {
    QMessageBox::information(
        this, tr("DB Connection Problem"),
        tr("There was an error connecting to the database:\n") +
            db.lastError());
    return;
  }

  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  QStringList query;
  QString invID = QString::number(US_Settings::us_inv_ID());

  // Build a mapping of experiment labels to experiment DB ids
  QMap<QString, QString> explabs;
  query.clear();
  query << "get_experiment_desc" << invID;

  db.query(query);

  while (db.next()) {
    QString expID = db.value(0).toString();
    QString runID = db.value(1).toString();
    QString label = db.value(4).toString();
    QString date = US_Util::toUTCDatetimeText(
                       db.value(5).toDateTime().toString(Qt::ISODate), true)
                       .section(" ", 0, 0);

    rdesc.runID = runID;
    rdesc.label = label;
    rdesc.date = date;
    rdesc.DB_id = expID;
    runmap[runID] = rdesc;
    rlabels << runID;
  }
  qDebug() << "ScDB: runmap size" << runmap.size();
  // qDebug() << "ScDB:TM:88: " <<
  // QTime::currentTime().toString("hh:mm:ss:zzzz");

  build_runids();

  QApplication::restoreOverrideCursor();
  QApplication::restoreOverrideCursor();
  // qDebug() << "ScDB:TM:99: " <<
  // QTime::currentTime().toString("hh:mm:ss:zzzz");
}

// Scan local disk for run sets
void US_SelectRuns::scan_local_run(void) {
  QString rdir = US_Settings::resultDir();
  QStringList aucdirs =
      QDir(rdir).entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name);

  QStringList aucfilt("*.auc");
  QStringList aucfiles;
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

  for (int ii = 0; ii < aucdirs.size(); ii++) {
    QString subdir = rdir + "/" + aucdirs.at(ii);
    QDir dirRun(subdir);
    QStringList aucfiles =
        QDir(subdir).entryList(aucfilt, QDir::Files, QDir::Name);

    if (aucfiles.size() < 1) continue;

    QString aucfbase = aucfiles.at(0);
    QString aucfname = subdir + "/" + aucfbase;
    QString runID = aucfbase.section(".", 0, -6);
    QString subType = aucfbase.section(".", -5, -5);
    QString tripl = aucfbase.section(".", -4, -2);
    QString explabel = runID;
    QString expID = "-1";
    QString date = "";

    QStringList edtfilt;
    edtfilt << runID + "." + subType + ".xml";
    QStringList edtfiles;
    edtfiles = dirRun.entryList(edtfilt, QDir::Files, QDir::Name);

    if (edtfiles.size() != 1) continue;

    QString efpath = subdir + "/" + edtfiles[0];
    QFile fileri(efpath);
    if (fileri.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QXmlStreamReader xml(&fileri);
      QXmlStreamAttributes attr;

      while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
          if (xml.name() == "experiment") {
            attr = xml.attributes();
            expID = attr.value("id").toString();
          }

          else if (xml.name() == "label") {
            xml.readNext();
            explabel = xml.text().toString();
            break;
          }
        }
      }

      fileri.close();

      date = US_Util::toUTCDatetimeText(
                 QFileInfo(fileri).lastModified().toUTC().toString(Qt::ISODate),
                 true)
                 .section("", 0, 0);
    }

    rdesc.runID = runID;
    rdesc.label = explabel;
    rdesc.date = date;
    rdesc.DB_id = expID;

    runmap[runID] = rdesc;
  }

  build_runids();

  QApplication::restoreOverrideCursor();
  QApplication::restoreOverrideCursor();
}

// Build the runID list from full run map
void US_SelectRuns::build_runids(void) {
  rlabels = runmap.keys();

  // Add "unassigned" entry
  QString runID = "Unassigned";
  rdesc.runID = runID;
  rdesc.label = "Manual/Custom/Global";
  rdesc.date = "2011/01/01";
  rdesc.DB_id = "0";
  rlabels << runID;
  runmap[runID] = rdesc;
  qDebug() << "BldR: rlabels size" << rlabels.size();
  qDebug() << "BldR: runmap size" << runmap.size();
}

// Investigator button clicked:  get investigator from dialog
void US_SelectRuns::get_person() {
  int invID = US_Settings::us_inv_ID();
  US_Investigator* dialog = new US_Investigator(true, invID);

  connect(dialog, SIGNAL(investigator_accepted(int)), SLOT(update_person(int)));

  dialog->exec();
}

// Slot to handle accept in investigator dialog
void US_SelectRuns::update_person(int ID) {
  QString number = (ID > 0) ? QString::number(ID) + ": " : "";
  le_invest->setText(number + US_Settings::us_inv_name());

  list_data();
}

// Slot to handle accept in investigator dialog
void US_SelectRuns::update_disk_db(bool isDB) {
  emit dkdb_changed(isDB);

  sel_db = isDB;
  runmap.clear();
  rlabels.clear();

  if (isDB)  // Scan database
  {
    scan_dbase_run();
    setWindowTitle(tr("Select Run(s) as Models Pre-Filter (DB)"));
  } else  // Scan local disk data
  {
    scan_local_run();
    setWindowTitle(tr("Select Run(s) as Models Pre-Filter (Local)"));
  }

  list_data();

  QString sfilt = le_dfilter->text();
  if (!sfilt.isEmpty()) search(sfilt);
}
