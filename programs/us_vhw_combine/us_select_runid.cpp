//! \file us_select_runid.cpp

#include "us_select_runid.h"

#include "us_constants.h"
#include "us_db2.h"
#include "us_editor.h"
#include "us_gui_settings.h"
#include "us_investigator.h"
#include "us_matrix.h"
#include "us_passwd.h"
#include "us_report.h"
#include "us_settings.h"
#include "us_util.h"

// Main constructor with flags for edit, latest-edit and local-data

US_SelectRunid::US_SelectRunid(bool dbase, QStringList& runIDs)
    : US_WidgetsDialog(0, 0), runIDs(runIDs) {
  sel_db = dbase;
  dbg_level = US_Settings::us_debug();

  setWindowTitle(tr("Select Run ID for vHW Distributions (%1)")
                     .arg(sel_db ? "DB" : "Local"));
  setPalette(US_GuiSettings::frameColor());
  setMinimumSize(480, 300);
  DbgLv(1) << "SE:sel_db" << sel_db;

  // Main layout
  QVBoxLayout* main = new QVBoxLayout(this);
  main->setContentsMargins(2, 2, 2, 2);
  main->setSpacing(2);

  // Top layout: buttons and fields above list widget
  QGridLayout* top = new QGridLayout;

  dkdb_cntrls = new US_Disk_DB_Controls(sel_db ? US_Disk_DB_Controls::DB
                                               : US_Disk_DB_Controls::Disk);
  pb_invest = us_pushbutton(tr("Select Investigator"));
  QString invnum = QString::number(US_Settings::us_inv_ID()) + ": ";
  QString invusr = US_Settings::us_inv_name();
  le_invest = us_lineedit(invnum + invusr, 0, true);
  pb_invest->setEnabled(sel_db);

  // Search line
  QLabel* lb_filtdata = us_label(tr("Search"));

  le_dfilter = us_lineedit();

  connect(dkdb_cntrls, SIGNAL(changed(bool)), this, SLOT(update_disk_db(bool)));
  connect(pb_invest, SIGNAL(clicked()), SLOT(get_person()));
  connect(le_dfilter, SIGNAL(textChanged(const QString&)),
          SLOT(search(const QString&)));

  int row = 0;
  top->addLayout(dkdb_cntrls, row++, 0, 1, 3);
  top->addWidget(pb_invest, row, 0, 1, 1);
  top->addWidget(le_invest, row++, 1, 1, 2);
  top->addWidget(lb_filtdata, row, 0, 1, 1);
  top->addWidget(le_dfilter, row++, 1, 1, 2);

  main->addLayout(top);

  QFont font(US_GuiSettings::fontFamily(), US_GuiSettings::fontSize());

  // List widget to show data choices
  lw_data = new QListWidget(this);
  lw_data->setFrameStyle(QFrame::NoFrame);
  lw_data->setPalette(US_GuiSettings::editColor());
  lw_data->setFont(font);
  lw_data->setSelectionMode(QAbstractItemView::ExtendedSelection);
  // lw_data->setSelectionMode( QAbstractItemView::SingleSelection );
  connect(lw_data, SIGNAL(itemSelectionChanged()), this,
          SLOT(selectionChanged()));

  main->addWidget(lw_data);

  // Button Row
  QHBoxLayout* buttons = new QHBoxLayout;

  QPushButton* pb_cancel = us_pushbutton(tr("Cancel"));
  QPushButton* pb_accept = us_pushbutton(tr("Accept"));

  connect(pb_cancel, SIGNAL(clicked()), SLOT(cancelled()));
  connect(pb_accept, SIGNAL(clicked()), SLOT(accepted()));

  buttons->addWidget(pb_cancel);
  buttons->addWidget(pb_accept);

  main->addLayout(buttons);

  // Status Row
  QFontMetrics fm(font);
  int fhigh = fm.lineSpacing();
  int fwide = fm.width(QChar('6'));
  int lhigh = fhigh * 4 + 12;
  int lwide = fwide * 32;

  te_status = us_textedit();
  te_status->setMaximumHeight(lhigh);
  te_status->resize(lwide, lhigh);
  us_setReadOnly(te_status, true);
  te_status->setTextColor(Qt::blue);

  main->addWidget(te_status);

  // List from disk or db source
  list_data();
}

void US_SelectRunid::search(const QString& search_string) {
  lw_data->setCurrentItem(NULL);

  for (int ii = 0; ii < lw_data->count(); ii++) {
    QListWidgetItem* lwi = lw_data->item(ii);
    bool hide = !lwi->text().contains(search_string, Qt::CaseInsensitive);
    lwi->setHidden(hide);
  }
}

// List data choices (from db or disk)
void US_SelectRunid::list_data() {
  rlabels.clear();
  mRDates.clear();

  if (sel_db)  // Scan database data
  {
    scan_dbase_runs();
  } else  // Scan local disk data
  {
    scan_local_runs();
  }

  lw_data->clear();

  if (rlabels.size() == 0) {
    QString clabel = tr("No data found.");
    lw_data->addItem(new QListWidgetItem(clabel));
    return;
  }

  DbgLv(1) << "LD:sel_db" << sel_db << "rlsize" << rlabels.size();
  sort_rlabels(rlabels);

  for (int ii = 0; ii < slabels.size();
       ii++) {  // Propagate list widget with labels sorted by date
    QString clabel = slabels.at(ii);

    lw_data->addItem(new QListWidgetItem(clabel));
  }

  count_list = lw_data->count();
  count_seld = lw_data->selectedItems().size();
  te_status->setText(
      tr("The list derives from %1 scanned run IDs.\n"
         "Of these, %2 have associated vHW distribution data.\n"
         "%3 %4 currently selected for combination plot components.\n"
         "List items are in recent-to-older order.")
          .arg(count_allr)
          .arg(count_list)
          .arg(count_seld)
          .arg((count_seld != 1) ? tr("runs are") : tr("run is")));
}

// Cancel button:  no editIDs returned
void US_SelectRunid::cancelled() {
  reject();
  close();
}

// Accept button:  set up to return editID pre-filter information
void US_SelectRunid::accepted() {
  DbgLv(1) << "SE:accepted()";
  QList<QListWidgetItem*> selitems = lw_data->selectedItems();

  if (selitems.size() == 0) {
    QMessageBox::information(
        this, tr("No Data Selected"),
        tr("You have not selected any data.\nSelect or Cancel"));
    return;
  }

  // Get and return runIDs from selected edit items
  for (int ii = 0; ii < selitems.size(); ii++) {
    QListWidgetItem* lwi_data = selitems.at(ii);
    QString clabel = lwi_data->text();
    DbgLv(1) << "SE:  ii clabel" << ii << clabel;

    runIDs << clabel;
  }
  DbgLv(1) << "SE: runID" << runIDs[0];

  accept();  // Signal that selection was accepted
  close();
}

// Scan database for edit sets
void US_SelectRunid::scan_dbase_runs() {
  US_Passwd pw;
  US_DB2 db(pw.getPasswd());
  count_allr = 0;
  count_list = 0;
  count_seld = 0;

  if (db.lastErrno() != US_DB2::OK) {
    QMessageBox::information(
        this, tr("DB Connection Problem"),
        tr("There was an error connecting to the database:\n") +
            db.lastError());
    return;
  }

  QStringList docruns;
  QString runid;

  QStringList query;
  QString invID = QString::number(US_Settings::us_inv_ID());

  query.clear();
  query << "get_report_desc" << invID;
  db.query(query);

  while (db.next()) {  // Build list of runIDs with documents
    runid = db.value(4).toString();
    DbgLv(1) << "ScDB:     report runid" << runid;

    if (!docruns.contains(runid)) {
      docruns << runid;
    }
  }

  query.clear();
  query << "get_experiment_desc" << invID;
  db.query(query);

  while (db.next()) {  // Map dates to runIDs with documents
    runid = db.value(1).toString();
    count_allr++;

    if (docruns.contains(runid)) {
      QString rdate = db.value(5).toDateTime().toString("yyMMddhhmm");
      mRDates[runid] = rdate;  // Save mapping of date to runID
    }
  }

  int nreps = docruns.count();
  DbgLv(1) << "ScDB: nreps" << nreps;
  for (int ii = 0; ii < nreps; ii++) {
    runid = docruns[ii];
    US_Report freport;
    freport.readDB(runid, &db);
    int ntrip = freport.triples.count();
    DbgLv(1) << "ScDB:  ntrip" << ntrip;
    int ndats = 0;

    for (int jj = 0; jj < ntrip; jj++) {
      US_Report::ReportTriple* tripl = &freport.triples[jj];
      int ndocs = tripl->docs.count();

      for (int kk = 0; kk < ndocs; kk++) {
        US_Report::ReportDocument* doc = &tripl->docs[kk];
        QString fname = doc->filename;

        DbgLv(1) << "ScDB:    kk fname" << kk << fname;
        if (fname.contains("distrib.csv")) ndats++;
      }
    }

    DbgLv(1) << "ScDB:   ndats" << ndats;
    if (ndats > 0) {
      count_list++;
      rlabels << runid;
    }
  }
  DbgLv(1) << "ScDB:count_list" << count_list;
}

// Scan local disk for edit sets
void US_SelectRunid::scan_local_runs(void) {
  QString rdir = US_Settings::resultDir();
  QStringList aucdirs =
      QDir(rdir).entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name);
  DbgLv(1) << "ScLo:rdir" << rdir << "aucdir count" << aucdirs.count();

  QStringList aucfilt("*.auc");
  QStringList datfilt;
  count_allr = 0;
  count_list = 0;
  count_seld = 0;

  for (int ii = 0; ii < aucdirs.count(); ii++) {
    QString subdir = rdir + "/" + aucdirs.at(ii);
    QStringList aucfiles =
        QDir(subdir).entryList(aucfilt, QDir::Files, QDir::Name);
    DbgLv(1) << "ScLo:  subdir" << subdir << "aucfiles count"
             << aucfiles.count();

    if (aucfiles.count() < 1) continue;

    QString aucfbase = aucfiles.at(0);
    QString aucfname = subdir + "/" + aucfbase;
    QString runID = aucfbase.section(".", 0, -6);
    QString tripl = aucfbase.section(".", -4, -2).replace(".", "");
    QString fdate = QFileInfo(subdir).lastModified().toString("yyMMddhhmm");
    count_allr++;

    datfilt.clear();
    datfilt << "vHW.*distrib.csv"
            << "vHW.*envelope.csv";
    QStringList datfiles =
        QDir(subdir).entryList(datfilt, QDir::Files, QDir::Name);
    DbgLv(1) << "ScLo:    datfilt0" << datfilt[0];
    DbgLv(1) << "ScLo:    datfiles count" << datfiles.count();

    if (datfiles.count() < 1) continue;

    count_list++;
    rlabels << runID;  // Save selectable run ID

    mRDates[runID] = fdate;  // Save mapping of date to runID
  }
  DbgLv(1) << "ScLo:rlabels count" << count_list << rlabels.count();
}

// Investigator button clicked:  get investigator from dialog
void US_SelectRunid::get_person() {
  int invID = US_Settings::us_inv_ID();
  US_Investigator* dialog = new US_Investigator(true, invID);

  connect(dialog, SIGNAL(investigator_accepted(int)), SLOT(update_person(int)));

  dialog->exec();
}

// Slot to handle accept in investigator dialog
void US_SelectRunid::update_person(int ID) {
  QString number = (ID > 0) ? QString::number(ID) + ": " : "";
  le_invest->setText(number + US_Settings::us_inv_name());

  list_data();
}

// Slot to update disk/db selection
void US_SelectRunid::update_disk_db(bool isDB) {
  emit changed(isDB);

  sel_db = isDB;
  list_data();

  pb_invest->setEnabled(isDB);
  setWindowTitle(tr("Select Run ID for vHW Distributions (%1)")
                     .arg(sel_db ? "DB" : "Local"));
}

// Slot to record a change in list item selection
void US_SelectRunid::selectionChanged() {
  count_seld = lw_data->selectedItems().size();

  te_status->setText(
      tr("The list derives from %1 scanned run IDs.\n"
         "Of these, %2 have associated vHW distribution data.\n"
         "%3 %4 currently selected for combination plot components.\n"
         "List items are in recent-to-older order.")
          .arg(count_allr)
          .arg(count_list)
          .arg(count_seld)
          .arg((count_seld != 1) ? tr("runs are") : tr("run is")));
}

// Sort given runID labels into date order, newest to oldest
void US_SelectRunid::sort_rlabels(const QStringList rlabs) {
  slabels.clear();
  QStringList keys = mRDates.keys();
  QStringList svals;

  for (int ii = 0; ii < keys.count();
       ii++) {  // Create a list of concatenated date+label strings
    QString rlabel = keys[ii];

    if (rlabs.contains(rlabel)) {
      QString rdate = mRDates[rlabel];
      QString sval = rdate + "^" + rlabel;
      svals << sval;
    }
  }

  // Sort combined values into date order
  qSort(svals);

  for (int ii = svals.count() - 1; ii >= 0;
       ii--) {  // Create the sorted labels list (reverse date order)
    QString sval = svals[ii];
    QString rlabel = sval.section("^", 1, -1);
    slabels << rlabel;
  }
}
