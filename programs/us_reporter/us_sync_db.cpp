//! \file us_sync_db.cpp

#include "us_sync_db.h"

#include "us_constants.h"
#include "us_db2.h"
#include "us_gui_settings.h"
#include "us_investigator.h"
#include "us_matrix.h"
#include "us_passwd.h"
#include "us_settings.h"
#include "us_util.h"

// Main constructor

US_SyncWithDB::US_SyncWithDB() : US_WidgetsDialog(0, 0) {
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle(tr("Synchronize Reports with Database"));
  setPalette(US_GuiSettings::frameColor());
  setMinimumSize(300, 160);
  resize(400, 200);

  // Main layout
  QVBoxLayout* main = new QVBoxLayout(this);
  main->setContentsMargins(2, 2, 2, 2);
  main->setSpacing(2);

  // Top layout: buttons and fields above list widget
  QGridLayout* top = new QGridLayout;
  int row = 0;

  // Investigator
  // Only enable the investigator button for privileged users
  pb_invest = us_pushbutton(tr("Select Investigator"));

  int invlev = US_Settings::us_inv_level();
  pb_invest->setEnabled(invlev > 0);
  connect(pb_invest, SIGNAL(clicked()), SLOT(get_person()));
  top->addWidget(pb_invest, row, 0);

  QString name =
      (invlev > 0) ? QString::number(US_Settings::us_inv_ID()) + ": " : "";

  le_invest = us_lineedit(name + US_Settings::us_inv_name(), -1, true);
  top->addWidget(le_invest, row++, 1);

  // Instructions text
  te_desc = us_textedit();
  us_setReadOnly(te_desc, true);
  QString desc =
      tr("<b>Note:</b> Proceeding may result in local reports<br/>"
         "being replaced from the database.<ul>"
         "<li><b>Cancel</b>   to abort synchronizing from the DB.</li>"
         "<li><b>Download</b> to proceed with DB synchronization.</li>"
         "<li><b>New Only</b> to only download new DB records.</li></ul>");
  te_desc->setHtml(desc);
  top->addWidget(te_desc, row, 0, 4, 2);

  main->addLayout(top);

  // Button Row
  QHBoxLayout* buttons = new QHBoxLayout;
  QPushButton* pb_cancel = us_pushbutton(tr("Cancel"));
  QPushButton* pb_accept = us_pushbutton(tr("Download"));
  QPushButton* pb_newonly = us_pushbutton(tr("New Only"));

  connect(pb_cancel, SIGNAL(clicked()), SLOT(cancelled()));
  connect(pb_accept, SIGNAL(clicked()), SLOT(accepted()));
  connect(pb_newonly, SIGNAL(clicked()), SLOT(downnew()));

  buttons->addWidget(pb_cancel);
  buttons->addWidget(pb_accept);
  buttons->addWidget(pb_newonly);

  main->addLayout(buttons);
}

// Investigator button clicked:  get investigator from dialog
void US_SyncWithDB::get_person() {
  int invID = US_Settings::us_inv_ID();
  US_Investigator* dialog = new US_Investigator(true, invID);

  dialog->exec();
}

// Cancel button
void US_SyncWithDB::cancelled() {
  qDebug() << "CANCELED";
  reject();
  close();
}

// Download button:  download all DB records and return accepted
void US_SyncWithDB::accepted() {
  qDebug() << "DOWNLOAD";
  newonly = false;
  scan_db_reports();  // Download reports from the database

  accept();  // Signal that selection was accepted
  close();
}

// New Only button:  download only new DB records and return accepted
void US_SyncWithDB::downnew() {
  qDebug() << "NEWONLY";
  newonly = true;
  scan_db_reports();  // Download new reports from the database

  accept();  // Signal that selection was accepted
  close();
}

// Scan database for reports
void US_SyncWithDB::scan_db_reports() {
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

  QString desc =
      tr("<b>Note:</b> Proceeding may result in local reports<br/>"
         "being replaced from the database.<ul>"
         "<li><b>Cancel</b>   to abort synchronizing from the DB.</li>"
         "<li><b>Download</b> to proceed with DB synchronization.</li>"
         "<li><b>New Only</b> to only download new DB records.</li></ul>"
         "<b>Downloading report records from the database...</b>");
  te_desc->setHtml(desc);
  qApp->processEvents();
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

  QStringList query;
  QString invID = QString::number(US_Settings::us_inv_ID());

  setWindowTitle(tr("Download Report Data from the Database"));

  query.clear();
  query << "count_reports" << invID;
  int nreports = db.functionQuery(query);
  qDebug() << "Reports count" << nreports;
  desc = desc + "<br/>&nbsp;&nbsp;&nbsp;";
  int nbdchr = desc.length();
  desc = desc.left(nbdchr) + tr("Total DB run reports is %1").arg(nreports);
  te_desc->setHtml(desc);
  qApp->processEvents();

  // Determine runIDs of existing local results
  QString resdir = US_Settings::resultDir() + "/";
  QDir dirres(resdir);
  QStringList resruns =
      dirres.entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name);

  query.clear();
  query << "get_report_desc" << invID;
  db.query(query);
  QStringList runids;
  int kreports = 0;
  int jreports = 0;
  nrunrpl = nrunadd = ndocrpl = ndocadd = 0;
  QString rptdir = US_Settings::reportDir() + "/";
  QDir dirrpt(rptdir);

  while (db.next()) {
    QString runid = db.value(4).toString();
    jreports++;
    if (resruns.contains(runid)) {
      runids << runid;
      qDebug() << "  report id" << db.value(0).toString() << "runID" << runid;
      kreports++;
    }
  }
  qDebug() << "Report descs count" << kreports;

  desc =
      desc.left(nbdchr) + tr("DB reports for local runIDs is %1").arg(kreports);
  te_desc->setHtml(desc);
  qApp->processEvents();

  for (int ii = 0; ii < kreports; ii++) {
    QString runid = runids[ii];
    QString rundir = rptdir + runid;
    QString runresd = resdir + runid;

    US_Report freport;
    freport.readDB(runid, &db);

    int ntriples = freport.triples.count();
    qDebug() << "  Report" << ii << "triples count" << ntriples << "runID"
             << runid;

    if (ntriples > 0) {
      if (QDir(rundir).exists()) {
        if (newonly) continue;
        nrunrpl++;
      }

      else {
        nrunadd++;
        dirrpt.mkdir(runid);
      }
    }

    int kdocadd = 0;

    for (int jj = 0; jj < ntriples; jj++) {
      US_Report::ReportTriple* tripl = &freport.triples[jj];
      int ndocs = tripl->docs.count();
      qDebug() << "    Triple" << jj << "docs count" << ndocs;

      desc =
          desc.left(nbdchr) + tr("Run %1, Triple %2: comparing %3 documents...")
                                  .arg(ii + 1)
                                  .arg(jj + 1)
                                  .arg(ndocs);
      te_desc->setHtml(desc);
      qApp->processEvents();

      for (int kk = 0; kk < ndocs; kk++) {
        US_Report::ReportDocument* doc = &tripl->docs[kk];
        QString fname = doc->filename;
        qDebug() << "      Doc" << kk << "filename" << fname << "ID"
                 << doc->documentID;
        QString fpath = rundir + "/" + fname;
        if (fname.endsWith(".csv")) fpath = runresd + "/" + fname;

        QFile dfile(fpath);

        if (dfile.exists()) {
          if (newonly) continue;
          ndocrpl++;
        }

        else {
          ndocadd++;
        }

        int st = doc->readDB(rundir, &db);

        if (st != US_DB2::OK) {
          qDebug() << "ReportDoc read ERROR: status" << st << fpath;
        }
      }
    }

    if (newonly && kdocadd != ndocadd) nrunrpl++;
  }
  QApplication::restoreOverrideCursor();

  QMessageBox::information(this, tr("DB Reports Downloaded"),
                           tr("Run IDs: %1 updated, %2 added.\n"
                              "Documents: %3 replaced, %4 added.")
                               .arg(nrunrpl)
                               .arg(nrunadd)
                               .arg(ndocrpl)
                               .arg(ndocadd));
  qDebug() << "nrunrpl nrunadd ndocrpl ndocadd" << nrunrpl << nrunadd << ndocrpl
           << ndocadd;
}
