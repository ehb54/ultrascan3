//! \file us_xpn_run_auc.cpp

#include "us_xpn_run_auc.h"
#include "us_gui_settings.h"
#include "us_settings.h"
#include "us_util.h"

// Primary constructor to establish the dialog
US_XpnRunAuc::US_XpnRunAuc(QString &runID) : US_WidgetsDialog(0, 0), runID(runID) {
   setWindowTitle(tr("US3 Directories with Optima-derived .auc Files"));

   setPalette(US_GuiSettings::frameColor());

   runID = "";
   QVBoxLayout *main = new QVBoxLayout(this);
   main->setSpacing(2);
   main->setContentsMargins(2, 2, 2, 2);

   // Search
   QHBoxLayout *search = new QHBoxLayout;
   QLabel *lb_search = us_label(tr("Search"));
   le_search = us_lineedit("");
   search->addWidget(lb_search);
   search->addWidget(le_search);
   connect(le_search, SIGNAL(textChanged(const QString &)), this, SLOT(limit_data(const QString &)));

   // Load the runInfo structure with current data
   load_runs();

   // Tree
   tw = new QTableWidget(runInfo.size(), 3, this);
   populate_list();

   // Button Row
   QHBoxLayout *buttons = new QHBoxLayout;

   QPushButton *pb_cancel = us_pushbutton(tr("Cancel"));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(reject()));
   buttons->addWidget(pb_cancel);

   QPushButton *pb_accept = us_pushbutton(tr("Select"));
   connect(pb_accept, SIGNAL(clicked()), SLOT(select()));
   buttons->addWidget(pb_accept);

   main->addLayout(search);
   main->addWidget(tw);
   main->addLayout(buttons);
   qDebug() << "gDBr: size" << size();

   resize(600, 300);
   qDebug() << "gDBr: size" << size();
}

// Function to load the runInfo structure with all runID's on local disk
void US_XpnRunAuc::load_runs(void) {
   impdir = US_Settings::importDir(); // Imports directory
   impdir.replace("\\", "/"); // Possible Windows issue

   if (impdir.right(1) != "/")
      impdir = impdir + "/"; // Insure trailing slash

   // Set up to load either from a raw DB file or from openAUC files
   QStringList efilt("*.auc");

   QStringList runids;
   QStringList rdirs = QDir(impdir).entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name);
   qDebug() << "LdDk:  rdirs count" << rdirs.count() << "impdir" << impdir << "efilt" << efilt;

   // Get the list of all Run IDs with data in their work directories
   for (int ii = 0; ii < rdirs.count(); ii++) {
      QString runID = rdirs[ ii ];
      QString wdir = impdir + runID + "/";
      QStringList efiles = QDir(wdir).entryList(efilt, QDir::Files, QDir::Name);
      int nfiles = efiles.count();
      qDebug() << "LdDk:   ii" << ii << "run" << rdirs[ ii ] << "count" << nfiles;

      if (nfiles < 1) // Definitely not Optima
         continue;

      QString rfn = wdir + efiles[ 0 ];
      QString date = US_Util::toUTCDatetimeText(QFileInfo(rfn).lastModified().toUTC().toString(Qt::ISODate), true)
                        .section(" ", 0, 0)
                        .simplified();

      // Look for TMST definition file and test import origin
      QString dfname = runID + ".time_state.xml";
      QString dpath = wdir + dfname;
      QFile dfile(dpath);

      if (!dfile.exists() || !dfile.open(QIODevice::ReadOnly)) {
         qDebug() << "LdDk:    dfname -- NOT exists/opened" << dpath;
         continue; // Skip if TMST def file does not exist or can't be opened
      }
      qDebug() << "LdDk:    dfname -- exists/opened";

      QTextStream fsi(&dfile);
      QString pmatch("import_type=\"Optima\"");
      QString pmatch2("import_type=\"XPN\"");
      QString xmli = fsi.readAll();
      dfile.close();
      qDebug() << "LdDk:     pmatch" << pmatch;

      if (!xmli.contains(pmatch) && !xmli.contains(pmatch2)) {
         continue; // Skip if TMST def has no import_type="Optima"
         qDebug() << "LdDk:      *SKIP* non-Optima";
      }


      // Add an eligible run directory to the list
      //qDebug() << "LdDk:   ii" << ii << "  rfn" << rfn;
      RunInfo rr;
      rr.runID = runID;
      rr.date = date;
      rr.ntriple = nfiles;
      qDebug() << "LdDk:   ii" << ii << "     runID date count" << rr.runID << rr.date << rr.ntriple;

      runInfo << rr;
   }

   if (runInfo.size() < 1) {
      QMessageBox::information(this, tr("Error"), tr("There are no US3 runs on the local Disk to load.\n"));
   }

   return;
}

// Function to pass information back when select button is pressed
void US_XpnRunAuc::select(void) {
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

   runID = impdir + tw->item(ndx, 0)->text();
   qDebug() << "XpnRunAuc:  accept : runID" << runID;
   accept();
}

// Function to populate the data tree
void US_XpnRunAuc::populate_list() {
   QFont tw_font(US_Widgets::fixedFont().family(), US_GuiSettings::fontSize());
   QFontMetrics *fm = new QFontMetrics(tw_font);
   int rowht = fm->height() + 2;
   tw->setFont(tw_font);
   tw->setPalette(US_GuiSettings::editColor());
   tw->setRowCount(runInfo.count());

   QStringList headers;
   headers << tr("Run") << tr("Date") << tr("Triples Count");

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
      QTableWidgetItem *item;
      RunInfo rr = runInfo[ ii ];
      QString tripk = QString().sprintf("%5d", rr.ntriple);

      item = new QTableWidgetItem(rr.runID);
      item->setFlags(item->flags() ^ Qt::ItemIsEditable);
      tw->setItem(ii, 0, item);

      item = new QTableWidgetItem(rr.date);
      item->setFlags(item->flags() ^ Qt::ItemIsEditable);
      tw->setItem(ii, 1, item);

      item = new QTableWidgetItem(tripk);
      item->setFlags(item->flags() ^ Qt::ItemIsEditable);
      tw->setItem(ii, 2, item);
      //qDebug() << "setItems ii" << ii << "ID date runID label"
      // << rr.ID << rr.date << rr.runID << rr.label;

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
void US_XpnRunAuc::limit_data(const QString &sfilt) {
   bool have_search = !sfilt.isEmpty();
   qDebug() << "LimData: sfilt" << sfilt << "have_search" << have_search;
   QFont tw_font(US_Widgets::fixedFont().family(), US_GuiSettings::fontSize());
   QFontMetrics *fm = new QFontMetrics(tw_font);
   int rowht = fm->height() + 2;
   tw->clearContents();
   tw->setSortingEnabled(false);
   qDebug() << "LimData:   runInfo size" << runInfo.size();

   for (int ii = 0; ii < runInfo.size(); ii++) {
      QTableWidgetItem *item;
      RunInfo rr = runInfo[ ii ];
      QString tripk = QString().sprintf("%5d", rr.ntriple);
      qDebug() << "LimData:    ii" << ii << "runID" << rr.runID;

      // Skip item if search text exists and runID does not contain it
      if (have_search && !rr.runID.contains(sfilt, Qt::CaseInsensitive))
         continue;

      item = new QTableWidgetItem(rr.runID);
      item->setFlags(item->flags() ^ Qt::ItemIsEditable);
      tw->setItem(ii, 0, item);

      item = new QTableWidgetItem(rr.date);
      item->setFlags(item->flags() ^ Qt::ItemIsEditable);
      tw->setItem(ii, 1, item);

      item = new QTableWidgetItem(tripk);
      item->setFlags(item->flags() ^ Qt::ItemIsEditable);
      tw->setItem(ii, 2, item);
      qDebug() << "setItems ii" << ii << "date runID dbId" << rr.date << rr.runID << rr.dbId;

      tw->setRowHeight(ii, rowht);
   }

   tw->setSortingEnabled(true);
   tw->sortByColumn(1, Qt::DescendingOrder);
   tw->resizeColumnsToContents();
   tw->adjustSize();
   //   tw->update();
   //   update();
   tw->resize(size().width() - 4, tw->size().height());
   qApp->processEvents();
}
