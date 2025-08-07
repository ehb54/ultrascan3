//! \file us_xpn_run_raw.cpp

#include "us_xpn_run_raw.h"

#include "us_gui_settings.h"
#include "us_settings.h"
#include "us_util.h"

// Primary constructor to establish the dialog
US_XpnRunRaw::US_XpnRunRaw(QString& runDesc, QStringList& rdLists)
    : US_WidgetsDialog(0, 0), runDesc(runDesc), rdLists(rdLists) {
  setWindowTitle(tr("Raw Optima postgreSQL database runs"));

  setPalette(US_GuiSettings::frameColor());

  runID = "";
  runDesc = rdLists.count() > 0 ? rdLists[0] : "";
  QVBoxLayout* main = new QVBoxLayout(this);
  main->setSpacing(2);
  main->setContentsMargins(2, 2, 2, 2);

  // Search
  QHBoxLayout* search = new QHBoxLayout;
  QLabel* lb_search = us_label(tr("Search"));
  le_search = us_lineedit("");
  search->addWidget(lb_search);
  search->addWidget(le_search);
  connect(le_search, SIGNAL(textChanged(const QString&)), this,
          SLOT(limit_data(const QString&)));
  qDebug() << "XRR:  runDesc" << runDesc;

  // Load the runInfo structure with current data
  load_runs();
  qDebug() << "XRR:  load_runs() return";

  // Tree
  tw = new QTableWidget(runInfo.size(), 11, this);
  populate_list();
  qDebug() << "XRR:  populate_list() return";

  // Button Row
  QHBoxLayout* buttons = new QHBoxLayout;

  QPushButton* pb_cancel = us_pushbutton(tr("Cancel"));
  connect(pb_cancel, SIGNAL(clicked()), SLOT(reject()));
  buttons->addWidget(pb_cancel);

  QPushButton* pb_accept = us_pushbutton(tr("Select"));
  connect(pb_accept, SIGNAL(clicked()), SLOT(select()));
  buttons->addWidget(pb_accept);

  main->addLayout(search);
  main->addWidget(tw);
  main->addLayout(buttons);
  qDebug() << "XRR: size" << size();

  resize(600, 300);
  qDebug() << "XRR: size" << size();
}

// Function to load the runInfo structure with all runID's on local disk
void US_XpnRunRaw::load_runs(void) {
  // Get the description fields delimiter from first character
  int nruns = rdLists.count();
  QString delim = (nruns > 0) ? QString(rdLists[0]).left(1) : "";
  qDebug() << "XRR:l_r: delim" << delim << "nruns" << nruns;

  // Populate the list of RunInfo objects
  for (int ii = 0; ii < nruns; ii++) {
    QString rDesc = rdLists[ii];
    QStringList lDesc = QString(rDesc).mid(1).split(delim);
    qDebug() << "XRR:l_r:  ii rDesc" << ii << rDesc << "lD count"
             << lDesc.count();

    RunInfo rirec;
    rirec.dbId = lDesc[0].toInt();
    rirec.expId = lDesc[1].toInt();
    rirec.dcount = lDesc[2].toInt();
    rirec.date = lDesc[3];
    rirec.expname = lDesc[4];
    rirec.resname = lDesc[5];
    rirec.abscnf = (lDesc[6] == "1");
    rirec.flscnf = (lDesc[7] == "1");
    rirec.inscnf = (lDesc[8] == "1");
    rirec.wascnf = (lDesc[9] == "1");
    rirec.runID = QString().sprintf("%s-run%d", rirec.expname.toLatin1().data(),
                                    rirec.dbId);
    qDebug() << "XRR:l_r:    runID" << rirec.runID;

    runInfo << rirec;
  }

  if (runInfo.size() < 1) {
    QMessageBox::information(
        this, tr("Error"),
        tr("There are no US3 runs in the PostgreSQL database\n"
           "or connection to the database failed.\n\n"
           "Verify that the Optima is on and connected."));
  }

  return;
}

// Function to pass information back when select button is pressed
void US_XpnRunRaw::select(void) {
  int ndx = tw->currentRow();

  if (ndx < 0) {
    QMessageBox::information(
        this, tr("No Run Selected"),
        tr("You have not selected a run to load."
           " To cancel loading, click on the \"Cancel\" button."
           " Otherwise, make a selection in the list before"
           " clicking on the \"Select\" button"));
    return;
  }

  runID = tw->item(ndx, 2)->text().simplified();
  QString delim = QString(rdLists[0]).left(1);
  qDebug() << "XRR:  select : ndx" << ndx << "runID" << runID << "delim"
           << delim;

  // Search description list and choose the item with matching runID
  for (int ii = 0; ii < rdLists.count(); ii++) {
    QString rDesc = rdLists[ii];
    QString lRunID = QString(rDesc).mid(1).section(delim, 0, 0);
    qDebug() << "XRR:  select :  ii" << ii << "rDesc" << rDesc << "lRunID"
             << lRunID;

    if (lRunID == runID) {
      runDesc = rDesc;
      break;
    }
  }

  qDebug() << "XRR:  accept : runID" << runID << "desc" << runDesc;
  accept();
}

// Function to populate the data tree
void US_XpnRunRaw::populate_list() {
  QFont tw_font(US_Widgets::fixedFont().family(), US_GuiSettings::fontSize());
  QFontMetrics* fm = new QFontMetrics(tw_font);
  int rowht = fm->height() + 2;
  tw->setFont(tw_font);
  tw->setPalette(US_GuiSettings::editColor());
  tw->setRowCount(runInfo.count());

  QStringList headers;
  headers << tr("Run") << tr("Date") << tr("RunId") << tr("ExpId")
          << tr("DataCount") << tr("Experiment") << tr("Researcher") << tr("A?")
          << tr("F?") << tr("I?") << tr("W?");

  tw->setHorizontalHeaderLabels(headers);
  tw->verticalHeader()->hide();
  tw->setShowGrid(false);
  tw->setSelectionBehavior(QAbstractItemView::SelectRows);
  tw->setMinimumWidth(100);
  tw->setMinimumHeight(100);
  tw->setColumnWidth(0, 250);
  tw->setColumnWidth(1, 150);
  tw->setColumnWidth(2, 50);
  tw->setSortingEnabled(false);
  tw->clearContents();

  // Now load the table, marking each as not-editable
  for (int ii = 0; ii < runInfo.size(); ii++) {
    QTableWidgetItem* item;
    RunInfo rr = runInfo[ii];
    QString sRunId = QString().sprintf("%7d", rr.dbId);
    QString sExpId = QString().sprintf("%7d", rr.expId);
    QString sDknt = QString().sprintf("%7d", rr.dcount);

    item = new QTableWidgetItem(rr.runID);
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    tw->setItem(ii, 0, item);

    item = new QTableWidgetItem(rr.date);
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    tw->setItem(ii, 1, item);

    item = new QTableWidgetItem(sRunId);
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    tw->setItem(ii, 2, item);

    item = new QTableWidgetItem(sExpId);
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    tw->setItem(ii, 3, item);

    item = new QTableWidgetItem(sDknt);
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    tw->setItem(ii, 4, item);

    item = new QTableWidgetItem(rr.expname);
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    tw->setItem(ii, 5, item);

    item = new QTableWidgetItem(rr.resname);
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    tw->setItem(ii, 6, item);

    item = new QTableWidgetItem(rr.abscnf ? "  1" : "  0");
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    tw->setItem(ii, 7, item);

    item = new QTableWidgetItem(rr.flscnf ? "  1" : "  0");
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    tw->setItem(ii, 8, item);

    item = new QTableWidgetItem(rr.inscnf ? "  1" : "  0");
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    tw->setItem(ii, 9, item);

    item = new QTableWidgetItem(rr.wascnf ? "  1" : "  0");
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    tw->setItem(ii, 10, item);

    tw->setRowHeight(ii, rowht);
  }

  tw->setSortingEnabled(true);
  tw->sortByColumn(1, Qt::DescendingOrder);
  tw->resizeColumnsToContents();
  tw->adjustSize();
  tw->resize(size().width() - 4, tw->size().height());
  qApp->processEvents();
}

// Function to limit table data shown based on search criteria
void US_XpnRunRaw::limit_data(const QString& sfilt) {
  bool have_search = !sfilt.isEmpty();
  qDebug() << "LimData: sfilt" << sfilt << "have_search" << have_search;
  QFont tw_font(US_Widgets::fixedFont().family(), US_GuiSettings::fontSize());
  QFontMetrics* fm = new QFontMetrics(tw_font);
  int rowht = fm->height() + 2;
  qDebug() << "LimData:   runInfo size" << runInfo.size();
  int krow = 0;
  if (have_search) {
    for (int ii = 0; ii < runInfo.size(); ii++) {
      RunInfo rr = runInfo[ii];
      if (rr.runID.contains(sfilt, Qt::CaseInsensitive)) krow++;
    }
  } else
    krow = runInfo.size();

  qDebug() << "LimData:    krow" << krow;
  tw->setRowCount(krow);
  tw->clearContents();
  tw->setSortingEnabled(false);
  int kk = 0;

  for (int ii = 0; ii < runInfo.size(); ii++) {
    QTableWidgetItem* item;
    RunInfo rr = runInfo[ii];
    QString sRunId = QString().sprintf("%7d", rr.dbId);
    QString sExpId = QString().sprintf("%7d", rr.expId);
    QString sDknt = QString().sprintf("%7d", rr.dcount);
    qDebug() << "LimData:    ii" << ii << "runID" << rr.runID;

    // Skip item if search text exists and runID does not contain it
    if (have_search && !rr.runID.contains(sfilt, Qt::CaseInsensitive)) continue;

    qDebug() << "LimData:     add: runID sRunId" << rr.runID << sRunId;
    item = new QTableWidgetItem(rr.runID);
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    tw->setItem(kk, 0, item);

    item = new QTableWidgetItem(rr.date);
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    tw->setItem(kk, 1, item);

    item = new QTableWidgetItem(sRunId);
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    tw->setItem(kk, 2, item);

    item = new QTableWidgetItem(sExpId);
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    tw->setItem(kk, 3, item);

    item = new QTableWidgetItem(sDknt);
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    tw->setItem(kk, 4, item);

    item = new QTableWidgetItem(rr.date);
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    tw->setItem(kk, 5, item);

    item = new QTableWidgetItem(rr.expname);
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    tw->setItem(kk, 6, item);

    item = new QTableWidgetItem(rr.resname);
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    tw->setItem(kk, 7, item);

    item = new QTableWidgetItem(rr.abscnf ? "  1" : "  0");
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    tw->setItem(kk, 9, item);

    item = new QTableWidgetItem(rr.flscnf ? "  1" : "  0");
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    tw->setItem(kk, 9, item);

    item = new QTableWidgetItem(rr.inscnf ? "  1" : "  0");
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    tw->setItem(kk, 10, item);

    item = new QTableWidgetItem(rr.wascnf ? "  1" : "  0");
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    tw->setItem(kk, 11, item);
    tw->setRowHeight(kk, rowht);
    kk++;
  }

  tw->setSortingEnabled(true);
  tw->sortByColumn(2, Qt::DescendingOrder);
  tw->resizeColumnsToContents();
  tw->adjustSize();
  //   tw->update();
  //   update();
  tw->resize(size().width() - 4, tw->size().height());
  qApp->processEvents();
}
