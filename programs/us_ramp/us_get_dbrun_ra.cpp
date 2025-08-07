//! \file us_get_dbrun_ra.cpp

#include "us_get_dbrun_ra.h"
#include "us_db2.h"
#include "us_gui_settings.h"
#include "us_passwd.h"
#include "us_settings.h"

// Primary constructor to establish the dialog
US_GetDBRunRa::US_GetDBRunRa(QString &r) : US_WidgetsDialog(0, 0), runID(r) {
   setWindowTitle(tr("Available US3 Runs..."));
   setPalette(US_GuiSettings::frameColor());

   QVBoxLayout *main = new QVBoxLayout(this);
   main->setSpacing(2);
   main->setContentsMargins(2, 2, 2, 2);

   QFont font(US_GuiSettings::fontFamily(), US_GuiSettings::fontSize());
   QFontMetrics *fm = new QFontMetrics(font);

   // Load the runInfo structure with current data
   if (!loadData()) {
      // Why doesn't this work?
      // hide();
      // close(); // done( -1 );
      // return;
   }

   tw = new QTableWidget(runInfo.size(), 4, this); // rows, columns, parent
   tw->setPalette(US_GuiSettings::editColor());

   QStringList headers;
   headers << "ID"
           << "Date"
           << "RunID"
           << "Label";

   tw->setHorizontalHeaderLabels(headers);
   tw->verticalHeader()->hide();
   tw->setShowGrid(false);
   tw->setSelectionBehavior(QAbstractItemView::SelectRows);
   tw->setMinimumWidth(640);
   tw->setMinimumHeight(480);
   tw->setRowHeight(0, fm->height() + 4);
   tw->setColumnWidth(0, 50);
   tw->setColumnWidth(1, 150);
   tw->setColumnWidth(2, 250);
   tw->setColumnWidth(3, 350);

   // Now load the table, marking each as not-editable
   for (int i = 0; i < runInfo.size(); i++) {
      RunInfo r = runInfo[ i ];

      QTableWidgetItem *item = new QTableWidgetItem(QString::number(r.ID));
      item->setFlags(item->flags() ^ Qt::ItemIsEditable);
      tw->setItem(i, 0, item);

      item = new QTableWidgetItem(r.date);
      item->setFlags(item->flags() ^ Qt::ItemIsEditable);
      tw->setItem(i, 1, item);

      item = new QTableWidgetItem(r.runID);
      item->setFlags(item->flags() ^ Qt::ItemIsEditable);
      tw->setItem(i, 2, item);

      item = new QTableWidgetItem(r.label);
      item->setFlags(item->flags() ^ Qt::ItemIsEditable);
      tw->setItem(i, 3, item);
   }

   // Enable sorting by a particular column
   QHeaderView *qHeader = tw->horizontalHeader();
   connect(qHeader, SIGNAL(sectionClicked(int)), SLOT(columnClicked(int)));

   main->addWidget(tw);

   // Button Row
   QHBoxLayout *buttons = new QHBoxLayout;

   QPushButton *pb_cancel = us_pushbutton(tr("Cancel"));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(reject()));
   buttons->addWidget(pb_cancel);

   QPushButton *pb_delete = us_pushbutton(tr("Delete"));
   connect(pb_delete, SIGNAL(clicked()), SLOT(deleteRun()));
   buttons->addWidget(pb_delete);

   QPushButton *pb_accept = us_pushbutton(tr("Select"));
   connect(pb_accept, SIGNAL(clicked()), SLOT(select()));
   buttons->addWidget(pb_accept);

   main->addLayout(buttons);
}

// Function to load the runInfo structure with all runID's in the DB
bool US_GetDBRunRa::loadData(void) {
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db(masterPW);

   if (db.lastErrno() != US_DB2::OK) {
      QMessageBox::information(this, tr("Error"), tr("Error making the DB connection.\n"));
      return false;
   }

   // Get all the experiment ID's
   QStringList expIDs;
   expIDs.clear();
   QStringList q("get_experiment_desc");
   q << QString::number(US_Settings::us_inv_ID());
   db.query(q);
   while (db.next())
      expIDs << db.value(0).toString();

   // Now get information we want about each experiment
   runInfo.clear();
   foreach (QString expID, expIDs) {
      q.clear();
      q << QString("get_experiment_info") << expID;
      db.query(q);
      db.next();

      RunInfo r;
      r.ID = expID.toInt();
      r.date = db.value(13).toString();
      r.runID = db.value(2).toString();
      r.label = db.value(10).toString();

      runInfo << r;
   }

   if (runInfo.size() < 1) {
      QMessageBox::information(this, tr("Error"), tr("There are no US3 runs in the DB to load.\n"));
      return false;
   }

   return true;
}

// Function to sort rows when column header is clicked
void US_GetDBRunRa::columnClicked(int col) {
   tw->sortItems(col);
}

// Function to pass information back when select button is pressed
void US_GetDBRunRa::select(void) {
   int ndx = tw->currentRow();

   runID = tw->item(ndx, 2)->text();
   accept();
}

// Function to delete the highlighted run when delete button is pressed
void US_GetDBRunRa::deleteRun(void) {
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db(masterPW);

   if (db.lastErrno() != US_DB2::OK) {
      QMessageBox::information(this, tr("Error"), tr("Error making the DB connection.\n"));
      return;
   }

   int status = QMessageBox::information(
      this, tr("Warning"),
      tr("Are you sure you want to delete this run from the DB? ") + tr("This action is not reversible. Proceed? "),
      tr("&OK"), tr("&Cancel"), 0, 0, 1);
   if (status != 0)
      return;

   int ndx = tw->currentRow();
   QString expID = tw->item(ndx, 0)->text();

   // Let's make sure it's not a calibration experiment in use
   QStringList q("count_calibration_experiments ");
   q << expID;
   int count = db.functionQuery(q);

   if (count < 0) {
      qDebug() << "count_calibration_experiments( " << expID << " ) returned a negative count";
      return;
   }

   else if (count > 0) {
      QMessageBox::information(
         this, tr("Error"),
         tr("Cannot delete an experiment that is associated "
            "with a rotor calibration\n"));
      return;
   }

   tw->removeRow(ndx);

   // Delete links between experiment and solutions
   q.clear();
   q << "delete_experiment_solutions" << expID;
   status = db.statusQuery(q);

   // Same with cell table
   q.clear();
   q << "delete_cell_experiments" << expID;
   status = db.statusQuery(q);

   // Now delete the experiment and all existing rawData,
   // because we're starting over
   q.clear();
   q << "delete_experiment" << expID;
   status = db.statusQuery(q);

   if (status != US_DB2::OK) {
      QMessageBox::information(this, tr("Error"), db.lastError() + " (" + status + ")\n");
   }
}
