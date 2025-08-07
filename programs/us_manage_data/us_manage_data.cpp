//! \file us_manage_data.cpp

#include "us_manage_data.h"

#include <QApplication>

#include "us_constants.h"
#include "us_data_model.h"
#include "us_data_process.h"
#include "us_data_tree.h"
#include "us_editor.h"
#include "us_gui_settings.h"
#include "us_investigator.h"
#include "us_license.h"
#include "us_license_t.h"
#include "us_matrix.h"
#include "us_passwd.h"
#include "us_select_item.h"
#include "us_settings.h"
#include "us_util.h"

const QColor colorRed(240, 0, 0);
const QColor colorBlue(0, 0, 255);
const QColor colorBrown(120, 60, 0);
const QColor colorGreen(0, 150, 0);
const QColor colorGray(110, 110, 110);
const QColor colorWhite(255, 255, 240);

#define timeFmt QString("hh:mm:ss")
#define nowTime() "T=" + QDateTime::currentDateTime().toString(timeFmt)

// main program
int main(int argc, char* argv[]) {
  QApplication application(argc, argv);

#include "main1.inc"

  // License is OK.  Start up.

  US_ManageData w;
  // w.show();                   //!< \memberof QWidget
  if (w.db != NULL)
    return application.exec();  //!< \memberof QApplication
  else
    return -1;
}

// US_ManageData class constructor
US_ManageData::US_ManageData() : US_Widgets() {
  dbg_level = US_Settings::us_debug();

  // set up the GUI
  setWindowTitle(tr("Manage US DB/Local Data Sets"));
  setPalette(US_GuiSettings::frameColor());
  DbgLv(1) << "GUI setup begun";

  // primary layouts
  QHBoxLayout* mainLayout = new QHBoxLayout(this);
  QVBoxLayout* leftLayout = new QVBoxLayout();
  QVBoxLayout* rghtLayout = new QVBoxLayout();
  QGridLayout* dctlLayout = new QGridLayout();
  QVBoxLayout* smryLayout = new QVBoxLayout();
  QGridLayout* statLayout = new QGridLayout();
  QGridLayout* treeLayout = new QGridLayout();
  mainLayout->setSpacing(2);
  mainLayout->setContentsMargins(2, 2, 2, 2);
  leftLayout->setSpacing(0);
  leftLayout->setContentsMargins(0, 1, 0, 1);
  rghtLayout->setSpacing(0);
  rghtLayout->setContentsMargins(0, 1, 0, 1);
  dctlLayout->setSpacing(1);
  dctlLayout->setContentsMargins(0, 0, 0, 0);
  smryLayout->setSpacing(0);
  smryLayout->setContentsMargins(0, 1, 0, 1);
  statLayout->setSpacing(1);
  statLayout->setContentsMargins(0, 0, 0, 0);
  treeLayout->setSpacing(1);
  treeLayout->setContentsMargins(0, 0, 0, 0);

  // fill in the GUI components
  int row = 0;

  pb_invtor = us_pushbutton(tr("Investigator"));
  dctlLayout->addWidget(pb_invtor, row, 0, 1, 3);
  if (US_Settings::us_inv_level() < 3) pb_invtor->setEnabled(false);

  personID = US_Settings::us_inv_ID();
  QString invn = (personID > 0) ? QString::number(personID) : "";
  le_invtor = us_lineedit(invn + ": " + US_Settings::us_inv_name(), 1, true);
  dctlLayout->addWidget(le_invtor, row++, 3, 1, 5);

  pb_reset = us_pushbutton(tr("Reset"), false);
  dctlLayout->addWidget(pb_reset, row, 0, 1, 4);
  connect(pb_reset, SIGNAL(clicked()), this, SLOT(reset()));

  pb_scanda = us_pushbutton(tr("Scan Data"));
  dctlLayout->addWidget(pb_scanda, row++, 4, 1, 4);
  connect(pb_scanda, SIGNAL(clicked()), this, SLOT(scan_data()));

  pb_hsedit = us_pushbutton(tr("Show All Edits"));
  dctlLayout->addWidget(pb_hsedit, row, 0, 1, 4);
  connect(pb_hsedit, SIGNAL(clicked()), this, SLOT(toggle_edits()));

  pb_hsmodl = us_pushbutton(tr("Show All Models"));
  dctlLayout->addWidget(pb_hsmodl, row++, 4, 1, 4);
  connect(pb_hsmodl, SIGNAL(clicked()), this, SLOT(toggle_models()));

  pb_hsnois = us_pushbutton(tr("Show All Noises"));
  dctlLayout->addWidget(pb_hsnois, row, 0, 1, 4);
  connect(pb_hsnois, SIGNAL(clicked()), this, SLOT(toggle_noises()));

  pb_helpdt = us_pushbutton(tr("Data Tree Help"));
  dctlLayout->addWidget(pb_helpdt, row++, 4, 1, 4);

  pb_help = us_pushbutton(tr("Help"));
  dctlLayout->addWidget(pb_help, row, 0, 1, 4);
  connect(pb_help, SIGNAL(clicked()), this, SLOT(help()));

  pb_close = us_pushbutton(tr("Close"));
  dctlLayout->addWidget(pb_close, row++, 4, 1, 4);
  connect(pb_close, SIGNAL(clicked()), this, SLOT(close()));

  QLabel* lb_runid = us_label(tr("RunID:"));
  cb_runid = us_comboBox();
  cb_runid->addItem(tr("ALL"));
  cb_runid->addItem(tr("Select individual run"));
  dctlLayout->addWidget(lb_runid, row, 0, 1, 2);
  dctlLayout->addWidget(cb_runid, row++, 2, 1, 6);
  connect(cb_runid, SIGNAL(activated(int)), this, SLOT(selected_runID(int)));

  QLabel* lb_triple = us_label(tr("Triple:"));
  cb_triple = us_comboBox();
  cb_triple->addItem("ALL");
  dctlLayout->addWidget(lb_triple, row, 0, 1, 2);
  dctlLayout->addWidget(cb_triple, row++, 2, 1, 6);

  QLabel* lb_source = us_label(tr("Source:"));
  cb_source = us_comboBox();
  cb_source->addItem("ALL");
  cb_source->addItem("DB Only");
  cb_source->addItem("Local Only");
  cb_source->addItem("Exclude Local-Only Trees");
  cb_source->addItem("Exclude DB-Only Trees");
  dctlLayout->addWidget(lb_source, row, 0, 1, 2);
  dctlLayout->addWidget(cb_source, row++, 2, 1, 6);
  connect(cb_source, SIGNAL(currentIndexChanged(QString)), this, SLOT(reset()));

  pb_invtor->setToolTip(
      tr("Use an Investigator dialog to set the database person ID"));
  pb_scanda->setToolTip(
      tr("Scan Database and Local data, with content analysis"));
  pb_helpdt->setToolTip(
      tr("Show a short Help/Legend dialog for notes on the data tree"));
  pb_reset->setToolTip(tr("Reset the data display to its default state"));
  pb_help->setToolTip(
      tr("Display detailed US_ManageData documentation text and images"));
  pb_close->setToolTip(tr("Close the US_ManageData window and exit"));
  cb_runid->setToolTip(
      tr("Select a single run ID with which to populate the data tree "));
  cb_triple->setToolTip(
      tr("Select a single triple of the run to populate the data tree "));
  cb_source->setToolTip(
      tr("Select a filter to limit records based on source (DB/Local) "));

  QLabel* lb_info2 = us_banner(tr("User Data Sets Summary:"));
  dctlLayout->addWidget(lb_info2, row++, 0, 1, 8);

  QPalette pa(le_invtor->palette());
  te_status = us_textedit();
  te_status->setPalette(US_GuiSettings::normalColor());
  te_status->setTextBackgroundColor(pa.color(QPalette::Window));
  te_status->setTextColor(pa.color(QPalette::WindowText));
  te_status->setFont(
      QFont(US_Widgets::fixedFont().family(), US_GuiSettings::fontSize() - 2));
  te_status->setText(
      tr(" 0 Combined Total data sets;\n"
         "   0 Combined RawData    records;\n"
         "   0 Combined EditedData records;\n"
         "   0 Combined Model      records;\n"
         "   0 Combined Noise      records.\n"
         " 0 Database Total data sets;\n"
         "   0 Database RawData    records;\n"
         "   0 Database EditedData records;\n"
         "   0 Database Model      records;\n"
         "   0 Database Noise      records.\n"
         " 0 Local    Total data sets;\n"
         "   0 Local    RawData    records;\n"
         "   0 Local    EditedData records;\n"
         "   0 Local    Model      records;\n"
         "   0 Local    Noise      records.\n"));
  smryLayout->addWidget(te_status);
  QFontMetrics fm(te_status->font());
  DbgLv(1) << "te_status font family" << te_status->font().family();
  int fontw = fm.maxWidth();
  fontw = (fontw > 0) ? fontw : fm.width('W');
  int fonth = fm.lineSpacing();
  int minsw = fontw * 44 + 10;
  int minsh = fonth * 18 + 10;
  DbgLv(1) << "te_status fw fh  mw mh" << fontw << fonth << " " << minsw
           << minsh;
  int maxsw = (minsw * 5) / 4;
  int maxsh = (minsh * 5) / 4;

  if (maxsw > 500 || maxsh > 450) {
    maxsw = qMin(maxsw, 500);
    maxsh = qMin(maxsh, 450);
    minsw = (maxsw * 4) / 5;
    minsh = (maxsh * 4) / 5;
  }

  cb_runid->setMaximumSize((maxsw * 3) / 4, fonth * 2);
  cb_triple->setMaximumSize((maxsw * 3) / 4, fonth * 2);
  cb_source->setMaximumSize((maxsw * 3) / 4, fonth * 2);

  te_status->setMinimumSize(minsw, minsh);
  te_status->setMaximumSize(maxsw, maxsh);
  DbgLv(1) << "te_status minw minh" << minsw << minsh;
  DbgLv(1) << "te_status maxw maxh" << maxsw << maxsh;
  te_status->adjustSize();
  DbgLv(1) << "te_status size" << te_status->size();

  row = 0;
  QLabel* lb_progr = us_label(tr("% Completed:"));
  progress = us_progressBar(0, 100, 0);
  statLayout->addWidget(lb_progr, row, 0, 1, 4);
  statLayout->addWidget(progress, row++, 4, 1, 4);

  lb_status = us_label(tr("Status"));
  statLayout->addWidget(lb_status, row, 0, 1, 8);

  // set up data tree; populate with sample data
  tw_recs = new QTreeWidget();
  tw_recs->setPalette(te_status->palette());
  treeLayout->addWidget(tw_recs);

  QStringList theads;
  theads << "Type" << "Label" << "SubType" << "Source"
         << "Children\nDB, Local" << "Descendants\nDB, Local";
  ntrows = 5;
  ntcols = theads.size();
  tw_recs->setHeaderLabels(theads);
  tw_recs->setFont(
      QFont(US_Widgets::fixedFont().family(), US_GuiSettings::fontSize() - 1));
  tw_recs->setObjectName(QString("tree-widget"));
  tw_recs->setAutoFillBackground(true);
  tw_recs->setSelectionMode(QAbstractItemView::ExtendedSelection);

  connect(tw_recs, SIGNAL(itemPressed(QTreeWidgetItem*, int)), this,
          SLOT(clickedItem(QTreeWidgetItem*)));

  reset_hsbuttons(false, true, true, true);  // hs button labels,tooltips

  // put layouts together for overall layout
  leftLayout->addLayout(dctlLayout);
  leftLayout->addLayout(smryLayout);
  leftLayout->setStretchFactor(smryLayout, 10);
  leftLayout->addLayout(statLayout);
  rghtLayout->addLayout(treeLayout);

  mainLayout->addLayout(leftLayout);
  mainLayout->addLayout(rghtLayout);
  mainLayout->setStretchFactor(leftLayout, 2);
  mainLayout->setStretchFactor(rghtLayout, 8);

  setMinimumWidth((int)(maxsw * 2.6));

  show();  // display main window before password dialog appears

  // insure we can connect to the database
  US_Passwd pw;
  db = new US_DB2(pw.getPasswd());
  if (db->lastErrno() != US_DB2::OK) {
    QMessageBox::information(
        this, tr("DB Connection Problem"),
        tr("There was an error connecting to the database:\n") +
            db->lastError() +
            "\n"
            //+ tr( "Cannot continue.  Closing" ) );
            + tr("Continuing without database."));
    db = NULL;
    // return;
  }
  DbgLv(1) << "db passwd complete";

  connect(pb_invtor, SIGNAL(clicked()), this, SLOT(sel_investigator()));
  DbgLv(1) << "GUI setup complete";

  // Create an object to handle the data itself
  da_model = new US_DataModel(this);

  // Set needed pointers for class interaction in model object
  da_model->setDatabase(db);
  da_model->setProgress(progress, lb_status);

  // Create an object to handle processing the data (upload,download,remove)
  da_process = new US_DataProcess(da_model, this);

  // Create an object to handle the data tree display
  da_tree = new US_DataTree(da_model, tw_recs, this);

  // Set needed pointers to sibling classes in model object
  da_model->setSiblings((QObject*)da_process, (QObject*)da_tree);
  DbgLv(1) << "classes setup complete";

  // Set up initial state of GUI
  connect(pb_helpdt, SIGNAL(clicked()), da_tree, SLOT(dtree_help()));

  reset();
}

// Reset the GUI
void US_ManageData::reset(void) {
  da_model->dummy_data();
  da_tree->build_dtree();

  reportDataStatus();

  int src_flg = cb_source->currentIndex();
  da_model->getRunIDs(runIDs, src_flg);

  cb_runid->clear();
  cb_runid->addItem(tr("ALL"));
  cb_runid->addItem(tr("Select individual run"));
  // cb_runid ->addItems( runIDs );
  cb_triple->clear();
  cb_triple->addItem(tr("ALL"));
}

// toggle edits between hide/show
void US_ManageData::toggle_edits() {
  bool show = pb_hsedit->text().startsWith("Show");  // show or hide?

  da_tree->toggle_expand("Raw", show);  // expand/collapse one level up

  if (show) {  // for show edits, need only relabel edit button
    reset_hsbuttons(show, true, false, false);
  }

  else {  // for hide edits, must toggle labels below; relabel all buttons
    da_tree->toggle_expand("Model", show);
    da_tree->toggle_expand("Noise", show);

    reset_hsbuttons(show, true, true, true);
  }
}

// toggle models between hide/show
void US_ManageData::toggle_models() {
  bool show = pb_hsmodl->text().startsWith("Show");  // show or hide?

  da_tree->toggle_expand("Edited", show);  // expand/collapse one level up

  if (show) {  // for show models, must expand levels above; reset edit,model
               // buttons
    da_tree->toggle_expand("Raw", show);

    reset_hsbuttons(show, false, true, true);
  }

  else {  // for hide models, must collapse below;  reset model,noise buttons
    da_tree->toggle_expand("Noise", false);

    reset_hsbuttons(show, false, true, true);
  }
}

// toggle noises between hide/show
void US_ManageData::toggle_noises() {
  bool show = pb_hsnois->text().startsWith("Show")  // show or hide?
              || pb_hsnois->text().startsWith("Expand");

  da_tree->toggle_expand("Model", show);  // expand/collapse one level up

  if (show) {  // for show noise, must expand above; reset edit,model,noise
               // buttons
    da_tree->toggle_expand("Edited", show);
    da_tree->toggle_expand("Raw", show);

    reset_hsbuttons(show, true, true, true);
  }

  else {  // for hide noise, need only reset noise button
    reset_hsbuttons(show, false, false, true);
  }
}

// change hide/show button text and tooltips for new expand/collapse state
void US_ManageData::reset_hsbuttons(bool show, bool edts, bool mods,
                                    bool nois) {
  QString blabl;
  QString btool;

  if (show) {    // show (expand) is the new state
    if (edts) {  // Hide Edits
      pb_hsedit->setText(tr("Collapse All"));
      pb_hsedit->setToolTip(
          tr("Collapse All, so Edits and all descendants are hidden"));
    }

    if (mods) {  // Hide Models
      pb_hsmodl->setText(tr("Hide All Models"));
      pb_hsmodl->setToolTip(
          tr("Collapse Edits, so Models and their children are hidden"));
    }

    if (nois) {  // Hide Noises
      pb_hsnois->setText(tr("Hide All Noises"));
      pb_hsnois->setToolTip(tr("Collapse Models, so Noises are hidden"));
    }
  }

  else {         // hide (collapse) is the new state
    if (edts) {  // Show Edits
      pb_hsedit->setText(tr("Show All Edits"));
      pb_hsedit->setToolTip(
          tr("Expand Raws, so Edits and parent Raws are shown"));
    }

    if (mods) {  // Show Models
      pb_hsmodl->setText(tr("Show All Models"));
      pb_hsmodl->setToolTip(
          tr("Expand Edits, so Models and their parents are shown"));
    }

    if (nois) {  // Show Noises
      pb_hsnois->setText(tr("Expand All"));
      pb_hsnois->setToolTip(
          tr("Expand All, so Noises and all ancestors are shown"));
    }
  }
}

// Scan the database and local disk for R/E/M/N data sets
void US_ManageData::scan_data() {
  QString rF = cb_runid->currentText();
  QString tF = cb_triple->currentText();
  QString sF = cb_source->currentText();
  DbgLv(1) << "ScnDM:  Start          " << nowTime();
  da_model->setFilters(rF, tF, sF);  // Set any run,triple,source filters

  da_model->scan_data();  // Scan the data
  DbgLv(1) << "ScnDM:  Scan Done      " << nowTime();

  lb_status->setText(tr("Building Data Tree..."));
  qApp->processEvents();

  da_tree->build_dtree();  // Rebuild the data tree with present data
  DbgLv(1) << "ScnDM:  Tree Built     " << nowTime();

  lb_status->setText(tr("Data Tree Build Complete"));
  qApp->processEvents();

  // resize so all of columns are shown
  tw_recs->expandAll();  // expand the entire tree

  for (int jj = 0; jj < ntcols; jj++) {
    tw_recs->resizeColumnToContents(jj);  // resize to fit contents
  }

  tw_recs->collapseAll();  // collapse the entire tree

  this->resize(1000, 500);

  reset_hsbuttons(false, true, true, true);  // hs button labels,tooltips

  // reformat and display report on record counts
  reportDataStatus();

  pb_reset->setEnabled(true);
  DbgLv(1) << "ScnDM:  Scan All Done  " << nowTime();
}

// Open dialog and get investigator when button clicked
void US_ManageData::sel_investigator() {
  US_Investigator* inv_dialog = new US_Investigator(true, personID);

  connect(inv_dialog, SIGNAL(investigator_accepted(int)),
          SLOT(assign_investigator(int)));

  inv_dialog->exec();
}

// Assign an investigator string in proper id:lastname,firstname form
void US_ManageData::assign_investigator(int invID) {
  personID = invID;
  le_invtor->setText(QString::number(personID) + ": " +
                     US_Settings::us_inv_name());

  da_model->setDatabase(db);
}

// Handle a right-mouse click of a row cell
void US_ManageData::clickedItem(QTreeWidgetItem* item) {
  // DbgLv(2) << "TABLE ITEM CLICKED";

  if (QApplication::mouseButtons() ==
      Qt::RightButton) {  // only bring up context menu if right-mouse-button
                          // was clicked
    da_tree->row_context_menu(item);
  }
}

// open a dialog and display data tree help
void US_ManageData::dtree_help() {}

// format an item action text for a message box
QString US_ManageData::action_text(QString exstext, QString acttext) {
  return tr("This item exists on %1.<br>"
            "Are you sure you want to proceed with a %2?<ul>"
            "<li><b>No </b> to cancel the action;</li>"
            "<li><b>Yes</b> to proceed with the action.</li></ul>")
      .arg(exstext)
      .arg(acttext);
}

// report the result of an item action
void US_ManageData::action_result(int stat, QString item_act) {
  if (stat != 999) {  // proceed was selected:  test status of action

    if (stat == 0) {  // action was successful
      // QMessageBox::information( this,
      //       item_act + tr( " Successful!" ),
      //       tr( "The \"%1\" action was successfully performed." )
      //       .arg( item_act ) );
      lb_status->setText(tr("\"%1\" Success!").arg(item_act));
    }

    else {  // action got an error
      QMessageBox::warning(
          this, item_act + tr(" *ERROR*!"),
          tr("The \"%1\" action had an error: %2").arg(item_act).arg(stat));
      lb_status->setText(tr("\"%1\" ERROR!").arg(item_act));
    }
  }

  else {  // cancel was selected:  report it
    // QMessageBox::information( this,
    //       item_act + tr( " Cancelled!" ),
    //       tr( "The \"%1\" action was cancelled." ).arg( item_act ) );
    lb_status->setText(tr("\"%1\" Cancelled!").arg(item_act));
  }
}

// report status of data
void US_ManageData::reportDataStatus() {
  US_DataModel::DataDesc cdesc;

  // count each type of record
  ncrecs = da_model->recCount();
  ndrecs = da_model->recCountDB();
  nlrecs = da_model->recCountLoc();
  ncraws = ncedts = ncmods = ncnois = 0;
  ndraws = ndedts = ndmods = ndnois = 0;
  nlraws = nledts = nlmods = nlnois = 0;

  for (int ii = 0; ii < ncrecs; ii++) {
    cdesc = da_model->row_datadesc(ii);

    if (cdesc.recType == 1) {
      ncraws++;
      ndraws += (cdesc.recState & US_DataModel::REC_DB) != 0 ? 1 : 0;
      nlraws += (cdesc.recState & US_DataModel::REC_LO) != 0 ? 1 : 0;
    }

    else if (cdesc.recType == 2) {
      ncedts++;
      ndedts += (cdesc.recState & US_DataModel::REC_DB) != 0 ? 1 : 0;
      nledts += (cdesc.recState & US_DataModel::REC_LO) != 0 ? 1 : 0;
    }

    else if (cdesc.recType == 3) {
      ncmods++;
      ndmods += (cdesc.recState & US_DataModel::REC_DB) != 0 ? 1 : 0;
      nlmods += (cdesc.recState & US_DataModel::REC_LO) != 0 ? 1 : 0;
    }

    else if (cdesc.recType == 4) {
      ncnois++;
      ndnois += (cdesc.recState & US_DataModel::REC_DB) != 0 ? 1 : 0;
      nlnois += (cdesc.recState & US_DataModel::REC_LO) != 0 ? 1 : 0;
    }
  }

  // In case there was filtering, recompute counts
  ndrecs = ndraws + ndedts + ndmods + ndnois;
  nlrecs = nlraws + nledts + nlmods + nlnois;
  ncrecs = ndrecs + nlrecs;
  ncraws = ndraws + nlraws;
  ncedts = ndedts + nledts;
  ncmods = ndmods + nlmods;
  ncnois = ndnois + nlnois;

  // Reformat and display report on record counts
  te_status->setText(
      QString().sprintf("%5d", ncrecs) + tr(" Combined Total data sets;\n  ") +
      QString().sprintf("%5d", ncraws) +
      tr(" Combined RawData    records;\n  ") +
      QString().sprintf("%5d", ncedts) +
      tr(" Combined EditedData records;\n  ") +
      QString().sprintf("%5d", ncmods) +
      tr(" Combined Model      records;\n  ") +
      QString().sprintf("%5d", ncnois) + tr(" Combined Noise      records.\n") +
      QString().sprintf("%5d", ndrecs) + tr(" Database Total data sets;\n  ") +
      QString().sprintf("%5d", ndraws) +
      tr(" Database RawData    records;\n  ") +
      QString().sprintf("%5d", ndedts) +
      tr(" Database EditedData records;\n  ") +
      QString().sprintf("%5d", ndmods) +
      tr(" Database Model      records;\n  ") +
      QString().sprintf("%5d", ndnois) + tr(" Database Noise      records.\n") +
      QString().sprintf("%5d", nlrecs) + tr(" Local    Total data sets;\n  ") +
      QString().sprintf("%5d", nlraws) +
      tr(" Local    RawData    records;\n  ") +
      QString().sprintf("%5d", nledts) +
      tr(" Local    EditedData records;\n  ") +
      QString().sprintf("%5d", nlmods) +
      tr(" Local    Model      records;\n  ") +
      QString().sprintf("%5d", nlnois) + tr(" Local    Noise      records.\n"));
}

// Slot to handle a newly select runID filter
void US_ManageData::selected_runID(QString selrunID) {
  DbgLv(0) << "selected_runID: selrunID" << selrunID;
  if (selrunID.isEmpty() || selrunID == tr("ALL")) {
    cb_triple->clear();
    cb_triple->addItem(tr("ALL"));
    return;
  }

  if (selrunID.startsWith(
          tr("Select individual"))) {  // Combo box select is "Select individual
                                       // run" so present list dialog
    int selx = -1;
    QStringList hdrs;
    QString dtitl = tr("Select Run ID");
    hdrs << "Run ID";
    runid_data.clear();

    for (int ii = 0; ii < runIDs.count();
         ii++) {  // Build data consisting of a single Run ID column
      runid_data << QStringList(runIDs[ii]);
    }

    US_SelectItem* srdiag =
        new US_SelectItem(runid_data, hdrs, dtitl, &selx, 1);
    if (srdiag->exec() == QDialog::Accepted &&
        selx >= 0) {  // Add selected run to comboBox and set Run ID variable
      QString srunID = runid_data[selx][0];
      cb_runid->disconnect();
      cb_runid->clear();
      cb_runid->addItem(tr("ALL"));
      cb_runid->addItem(tr("Select individual run"));
      cb_runid->addItem(srunID);
      cb_runid->setCurrentIndex(2);
      connect(cb_runid, SIGNAL(activated(int)), this,
              SLOT(selected_runID(int)));
    }
  }

  lb_status->setText(tr("Scanning triples for a run..."));
  qApp->processEvents();
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  da_model->getTriples(triples, selrunID);
  DbgLv(0) << "selected_runID: da_m getTriples size" << triples.size();

  cb_triple->clear();
  cb_triple->addItem("ALL");
  cb_triple->addItems(triples);
  QApplication::restoreOverrideCursor();
  int ntriple = triples.size();

  lb_status->setText(
      (ntriple == 1) ? tr("The run has a single triple")
                     : tr("%1 triples are available to select").arg(ntriple));
}

// Slot to handle a newly select runID filter
void US_ManageData::selected_runID(int index) {
  selected_runID(cb_runid->itemText(index));
}
