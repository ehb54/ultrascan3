//! \file us_tmst_viewer.cpp

#include "us_tmst_viewer.h"

#include "us_constants.h"
#include "us_db2.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "us_investigator.h"
#include "us_license.h"
#include "us_license_t.h"
#include "us_passwd.h"
#include "us_select_item.h"
#include "us_settings.h"
#include "us_tmst_plot.h"
#include "us_util.h"

#ifdef WIN32
#include <float.h>
#define isnan _isnan
#endif

#ifndef DbgLv
#define DbgLv(a) \
  if (dbg_level >= a) qDebug()
#endif

int main(int argc, char* argv[]) {
  QApplication application(argc, argv);

#include "main1.inc"

  // License is OK.  Start up.

  US_TmstViewer ww;
  ww.show();                  //!< \memberof QWidget
  return application.exec();  //!< \memberof QApplication
}

// Constructor of TimeState Viewer
US_TmstViewer::US_TmstViewer() : US_Widgets() {
  setWindowTitle(tr("Time State Viewer"));
  setPalette(US_GuiSettings::frameColor());

  QGridLayout* settings = new QGridLayout;

  dbg_level = US_Settings::us_debug();
  haveTmst = false;

  // Load controls
  QLabel* lb_loadtmst = us_banner(tr("Select a Time State Record"));
  QLabel* lb_smry = us_banner(tr("Time State Summary"));

  pb_loadtmst = us_pushbutton(tr("Select Time State"));
  pb_showtmst = us_pushbutton(tr("Show Time State"));

  QLabel* lb_dir = us_label(tr("Directory"), -1);
  le_dir = us_lineedit("", -1, true);

  QLabel* lb_runID = us_label(tr("Run ID:"), -1);
  le_runID = us_lineedit("", -1, false);

  QGridLayout* src_box1 = us_checkbox(tr("tmp"), ck_src_tmp, true);
  QGridLayout* src_box2 = us_checkbox(tr("imports"), ck_src_imp, true);
  QGridLayout* src_box3 = us_checkbox(tr("results"), ck_src_res, true);
  QGridLayout* src_box4 = us_checkbox(tr("database"), ck_src_db, true);

  // Status and standard pushbuttons
  QLabel* lb_status = us_banner(tr("Status"));
  le_status = us_lineedit(tr("(no data loaded)"), -1, true);
  QPalette stpal;
  stpal.setColor(QPalette::Text, Qt::white);
  stpal.setColor(QPalette::Base, Qt::blue);
  le_status->setPalette(stpal);

  QPushButton* pb_help = us_pushbutton(tr("Help"));
  QPushButton* pb_close = us_pushbutton(tr("Close"));

  // Signals and Slots
  connect(pb_loadtmst, SIGNAL(clicked()), this, SLOT(loadTimeState()));
  connect(pb_showtmst, SIGNAL(clicked()), this, SLOT(showTimeState()));
  connect(pb_help, SIGNAL(clicked()), this, SLOT(help()));
  connect(pb_close, SIGNAL(clicked()), this, SLOT(close()));

  // Do the layout
  int row = 0;
  settings->addWidget(lb_loadtmst, row++, 0, 1, 8);
  settings->addWidget(pb_loadtmst, row, 0, 1, 4);
  settings->addWidget(pb_showtmst, row++, 4, 1, 4);
  settings->addLayout(src_box1, row, 0, 1, 2);
  settings->addLayout(src_box2, row, 2, 1, 2);
  settings->addLayout(src_box3, row, 4, 1, 2);
  settings->addLayout(src_box4, row++, 6, 1, 2);
  settings->addWidget(lb_smry, row++, 0, 1, 8);
  settings->addWidget(lb_dir, row++, 0, 1, 8);
  settings->addWidget(le_dir, row++, 0, 1, 8);
  settings->addWidget(lb_runID, row, 0, 1, 2);
  settings->addWidget(le_runID, row++, 2, 1, 6);
  settings->addWidget(lb_status, row++, 0, 1, 8);
  settings->addWidget(le_status, row++, 0, 1, 8);
  settings->addWidget(pb_help, row, 0, 1, 4);
  settings->addWidget(pb_close, row++, 4, 1, 4);

  // Now let's assemble the page
  QVBoxLayout* main = new QVBoxLayout(this);
  main->setSpacing(2);
  main->setContentsMargins(2, 2, 2, 2);

  main->addLayout(settings);

  adjustSize();
  enableControls();
}

// Enable the common dialog controls based on the presence of data
void US_TmstViewer::enableControls(void) {
  // Enable and disable controls now
  pb_showtmst->setEnabled(haveTmst);
}

// Load Time State selection
void US_TmstViewer::loadTimeState() {
  // Build the list of timestate items
  tmstdata.clear();
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  le_status->setText(tr("Building TimeState list ..."));
  qApp->processEvents();

  if (ck_src_tmp->isChecked()) {  // Files from */ultrascan/tmp
    tmst_items("tmp");
  }
  if (ck_src_imp->isChecked()) {  // Files from */ultrascan/imports/*
    tmst_items("imports");
  }
  if (ck_src_res->isChecked()) {  // Files from */ultrascan/results/*
    tmst_items("results");
  }
  if (ck_src_db->isChecked()) {  // Entries from the database (copied to
                                 // */ultrascan/tmp)
    tmst_items("database");
  }

  le_status->setText(tr("TimeState list built."));
  qApp->processEvents();
  QApplication::restoreOverrideCursor();

  // Build title and headers for selection dialog
  QString dtitl = tr("Select Time State Record");
  QStringList hdrs;
  hdrs << "Run/Label" << "Source" << "Date" << "Cksum/Size";
  selx = -1;

  // Open a dialog to select a timestate
  US_SelectItem* lddiag = new US_SelectItem(tmstdata, hdrs, dtitl, &selx, -3);
  if (lddiag->exec() == QDialog::Rejected) {
    DbgLv(1) << "RDr:  rtn fr SelectItem dialog:  REJECT";
    return;
  }

  qApp->processEvents();
  DbgLv(1) << "RDr:  rtn fr SelectItem dialog  selx" << selx;
  if (selx < 0) {
    DbgLv(1) << "RDr:  rtn fr SelectItem dialog:  SELX<0";
    return;
  }
  DbgLv(1) << "RDr:   run/label" << tmstdata[selx][0];

  // Parse the selected timestate record
  QString tsdesc = tmstdata[selx][0];
  QString tssrc = tmstdata[selx][1];
  QString tspath = tmstdata[selx][4];
  QString tsdir = QString(tspath).section("/", 0, -2);
  haveTmst = QFile(tspath).exists();

  if (tssrc == "results" || tssrc == "imports") {
    tsdir = QString(tspath).section("/", 0, -3);
  } else if (tssrc == "database") {
    tsdir = US_Settings::tmpDir();
    haveTmst = true;
  }

  le_status->setText(tr("TimeState selected."));
  le_dir->setText(tsdir);
  le_runID->setText(tsdesc);

  // Ok to enable some buttons now
  enableControls();
}

// Show Time State
void US_TmstViewer::showTimeState() {
  QString tspath = tmstdata[selx][4];
  QString tssrc = tmstdata[selx][1];

  if (tssrc ==
      "database") {  // For database, first download record to local disk
    QString tmstID = tspath;
    int idTmst = tmstID.toInt();
    bool needTmst = false;
    QString fname;
    QString xdefs;
    QString cksumd;
    US_Passwd pw;
    US_DB2 db(pw.getPasswd());
    QString tsdesc = tmstdata[selx][0];
    tspath = US_Settings::tmpDir() + "/" + tsdesc + ".time_state.tmst";
    US_TimeState::dbExamine(&db, &idTmst, 0, &fname, &xdefs, &cksumd, 0);
    DbgLv(1) << "sTS: db : fname" << fname << "cksumd" << cksumd << "tspath"
             << tspath << "idTmst" << idTmst;
    if (QFile(tspath).exists()) {  // File exists, so check if it matches DB
      QString cksumf = US_Util::md5sum_file(tspath);
      DbgLv(1) << "sTS: db :   cksumf" << cksumf;
      if (cksumf != cksumd) {  // Non-matching:  need to download new version
        DbgLv(1) << "sTS: db :    OVERWRITE";
        needTmst = true;
      } else
        DbgLv(1) << "sTS: db :    DB-COPY_FILE_EXISTS";
    } else {  // File non-existent, so need to simply download
      DbgLv(1) << "sTS: db :    NEW DOWNLOAD";
      needTmst = true;
    }

    if (needTmst) {
      // Download the .tmst file
      US_TimeState::dbDownload(&db, idTmst, tspath);
      // And write the xdefs sibling file
      QString xfpath = QString(tspath).replace(".tmst", ".xml");
      QFile fileo(xfpath);
      if (fileo.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream tso(&fileo);
        tso << xdefs;
        tso.flush();
        fileo.close();
      }
    }
  }
  DbgLv(1) << "sTS: tspath" << tspath;

  // Open the dialog to display the timestate
  US_TmstPlot* tsdiag = new US_TmstPlot(this, tspath);

  DbgLv(1) << "sTS: tsdiag exec()";
  tsdiag->exec();
  DbgLv(1) << "sTS: tsdiag DONE";
}

// Add to the list of timestate record items
void US_TmstViewer::tmst_items(const QString source) {
  QStringList tmste;
  QStringList tsfilt("*.tmst");

  if (source == "database") {  // Read all database "timestate" table rows
    US_Passwd pw;
    US_DB2 db(pw.getPasswd());

    if (db.lastErrno() != US_DB2::OK) {
      qDebug() << "DB CONNECT ERROR: " << db.lastError();
      return;
    }

    QString invID = QString::number(US_Settings::us_inv_ID());
    QStringList qry;
    qry << "get_experiment_desc" << invID;
    db.query(qry);
    int numrows = db.numRows();
    DbgLv(1) << "items:  DB query" << qry << "DB numrows" << numrows << "DB err"
             << db.lastErrno() << db.lastError();
    QStringList expIds;

    while (db.next()) {  // Accumulate a list of experiment IDs of user's
                         // timestates
      expIds << db.value(0).toString();
    }

    for (int ii = 0; ii < expIds.count();
         ii++) {  // Issue a query for each experiment ID of timestates
      qry.clear();
      qry << "get_experiment_timestate" << expIds[ii];
      db.query(qry);

      while (db.next()) {
        QString tmstID = db.value(0).toString();
        QString fname = db.value(1).toString();
        QString cksum = db.value(3).toString() + " " + db.value(4).toString();
        QString descr = QString(fname).section(".", 0, -3);
        QString udate =
            US_Util::toUTCDatetimeText(
                db.value(5).toDateTime().toString(Qt::ISODate), true)
                .section(" ", 0, 0)
                .simplified();

        // Add a database entry to the list
        tmste.clear();
        tmste << descr << source << udate << cksum << tmstID;
        DbgLv(1) << "items:  DB tmste" << tmste;

        tmstdata << tmste;
      }
    }
  }

  else {  // Read all "*.tmst" local files from the specified source
    QStringList fdirs;
    QString bdir;

    if (source == "tmp") {  // From the single */ultrascan/tmp directory
      bdir = US_Settings::tmpDir();
      fdirs << "";
    } else if (source ==
               "imports") {  // From all */ultrascan/imports/* directories
      bdir = US_Settings::importDir();
      fdirs = QDir(bdir).entryList(QDir::AllDirs | QDir::NoDotAndDotDot,
                                   QDir::Name);
    } else if (source ==
               "results") {  // From all */ultrascan/results/* directories
      bdir = US_Settings::resultDir();
      fdirs = QDir(bdir).entryList(QDir::AllDirs | QDir::NoDotAndDotDot,
                                   QDir::Name);
    }
    DbgLv(1) << "items:  source" << source << "bdir" << bdir;

    if (bdir.endsWith("/")) bdir = bdir.left(bdir.length() - 1);

    for (int ii = 0; ii < fdirs.size();
         ii++) {  // Get the list of .tmst files in the directory
      QString sdir = bdir + "/" + fdirs.at(ii);
      QDir dirdir(sdir);
      QStringList tfiles =
          QDir(sdir).entryList(tsfilt, QDir::Files, QDir::Name);
      if (sdir.endsWith("/")) sdir = sdir.left(sdir.length() - 1);

      for (int jj = 0; jj < tfiles.size();
           jj++) {  // Build an entry from a file in the directory
        QString tmstfn = tfiles[jj];
        QString tmstfp = sdir + "/" + tmstfn;
        QFile filei(tmstfp);
        QString descr = QString(tmstfn).section(".", 0, -3);
        QString icksum = US_Util::md5sum_file(tmstfp);
        QString fdate =
            US_Util::toUTCDatetimeText(
                QFileInfo(tmstfp).lastModified().toUTC().toString(Qt::ISODate),
                true)
                .section(" ", 0, 0)
                .simplified();

        // Add a file's timestate entry to the list
        tmste.clear();
        tmste << descr << source << fdate << icksum << tmstfp;

        tmstdata << tmste;
      }
    }
  }
}
