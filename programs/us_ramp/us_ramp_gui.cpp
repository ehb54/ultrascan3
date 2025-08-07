#include "us_ramp_gui.h"

#include <QApplication>

#include "qwt_scale_engine.h"
#include "us_constants.h"
#include "us_db2.h"
#include "us_experiment_gui_ra.h"
#include "us_get_dbrun_ra.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "us_images.h"
#include "us_intensity_ra.h"
#include "us_investigator.h"
#include "us_license.h"
#include "us_license_t.h"
#include "us_math2.h"
#include "us_passwd.h"
#include "us_plot.h"
#include "us_report.h"
#include "us_rotor_gui.h"
#include "us_run_details2.h"
#include "us_select_triples_ra.h"
#include "us_settings.h"
#include "us_solution_gui.h"
#include "us_util.h"
#if QT_VERSION < 0x050000
#define setSamples(a, b, c) setData(a, b, c)
#endif

#ifdef Q_OS_WIN
#include <float.h>
#ifndef isnan
#define isnan _isnan
#endif
#endif

int main(int argc, char* argv[]) {
  QApplication application(argc, argv);

#include "main1.inc"

  // License is OK.  Start up.

  US_RampGui w;
  w.show();                   //!< \memberof QWidget
  return application.exec();  //!< \memberof QApplication
}

US_RampGui::US_RampGui() : US_Widgets() {
  ExpData.invID = US_Settings::us_inv_ID();

  // Ensure data directories are there
  QDir dir;
  dir.mkpath(US_Settings::workBaseDir());
  dir.mkpath(US_Settings::importDir());
  dir.mkpath(US_Settings::tmpDir());
  dir.mkpath(US_Settings::dataDir());
  dir.mkpath(US_Settings::archiveDir());
  dir.mkpath(US_Settings::resultDir());
  dir.mkpath(US_Settings::reportDir());
  dir.mkpath(US_Settings::etcDir());

  setWindowTitle(tr("Import Ramp Data (Lechner, OpenAUC Multiwavelength)"));
  setPalette(US_GuiSettings::frameColor());

  isMwl = false;
  dbg_level = US_Settings::us_debug();
  DbgLv(0) << "CGui: dbg_level" << dbg_level;

  QGridLayout* settings = new QGridLayout;

  int row = 0;

  // First row
  QStringList DB = US_Settings::defaultDB();
  if (DB.isEmpty()) DB << "Undefined";
  QLabel* lb_DB = us_banner(tr("Database: ") + DB.at(0));
  lb_DB->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

  // Investigator
  bool isadmin = (US_Settings::us_inv_level() > 2);
  QWidget* wg_investigator;
  QPushButton* pb_investigator = us_pushbutton(tr("Select Investigator"));

  if (isadmin) {  // Admin gets investigator button
    wg_investigator = (QWidget*)pb_investigator;
  } else {  // Non-admin gets only investigator label
    QLabel* lb_investigator = us_label(tr("Investigator:"));
    wg_investigator = (QWidget*)lb_investigator;
    pb_investigator->setVisible(false);
  }

  le_investigator = us_lineedit(tr("Not Selected"), 0, true);

  // Radio buttons
  disk_controls = new US_Disk_DB_Controls(US_Disk_DB_Controls::Default);
  save_diskDB = US_Disk_DB_Controls::Default;

  // Display status
  QLabel* lb_status = us_banner(tr("Status:"));
  le_status = us_lineedit(tr("(no data loaded)"), 1, true);
  QPalette stpal;
  stpal.setColor(QPalette::Text, Qt::blue);
  le_status->setPalette(stpal);

  // Display Run ID
  QLabel* lb_runID = us_label(tr("Run ID:"));
  // Add this later, after tabs:settings->addWidget( lb_runID, row, 0 );
  lb_runID->setVisible(false);  // for now

  le_runID = us_lineedit("", 1, true);
  // le_runID ->setMinimumWidth( 280 );
  //  Add this later, after tabs: settings->addWidget( le_runID, row++, 1 );
  le_runID->setVisible(false);  // for now

  // Load the run
  QLabel* lb_run = us_banner(tr("Load the Run"));

  // Pushbuttons to load and reload data
  pb_importlechner = us_pushbutton(tr("Import Lechner Ramp Data from HD"));

  // External program to enter experiment information
  pb_editRuninfo = us_pushbutton(tr("Edit Run Information"));
  pb_editRuninfo->setEnabled(false);

  // load US3 data ( that perhaps has been done offline )
  pb_loadUS3 = us_pushbutton(tr("Load US3 Ramp Data"), true);

  //    // Run details
  //    pb_details        = us_pushbutton( tr( "Run Details" ), false );

  // Load MWL data
  pb_importmwlramp = us_pushbutton(tr("Import MWL Ramp Data from HD"));
  le_lambraw = us_lineedit(tr(""), 0, true);

  //    // Set the wavelength tolerance for c/c/w determination
  //    QLabel* lb_tolerance = us_label(   tr( "Separation Tolerance:" ) );
  //    ct_tolerance      = us_counter ( 2, 0.0, 100.0, 5.0 );
  //    ct_tolerance->setStep( 1 );
  //    //ct_tolerance->setMinimumWidth( 80 );

  // Set up MWL controls
  QFont font(US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() - 1);
  QFontMetrics fmet(font);
  // ct_tolerance->adjustSize();
  // int   fwid    = fmet.maxWidth();
  // int   swid    = fwid * 5;
  static QChar clambda(955);  // Lambda character

  lb_mwlctrl = us_banner(tr("Multi-Wavelength Lambda Controls"));
  lb_lambstrt = us_label(tr("%1 Start:").arg(clambda));
  lb_lambstop = us_label(tr("%1 End:").arg(clambda));
  lb_lambplot = us_label(tr("Plot %1:").arg(clambda));
  cb_lambstrt = us_comboBox();
  cb_lambstop = us_comboBox();
  cb_lambplot = us_comboBox();
  pb_lambprev = us_pushbutton("previous", true, -2);
  pb_lambnext = us_pushbutton("next", true, -2);
  pb_lambprev->setIcon(US_Images::getIcon(US_Images::ARROW_LEFT));
  pb_lambnext->setIcon(US_Images::getIcon(US_Images::ARROW_RIGHT));
  // lb_lambstrt->setMinimumWidth( swid );
  // cb_lambstrt->setMinimumWidth( swid );
  // lb_lambstop->setMinimumWidth( swid );
  // cb_lambstop->setMinimumWidth( swid );
  //  MWL Controls
  QGridLayout* mwl_controls = new QGridLayout();

  QLabel* lb_runinfo = us_banner(tr("Run Information"));

  // Change Run ID
  QLabel* lb_runID2 = us_label(tr("Run ID:"));
  le_runID2 = us_lineedit("", 1);
  // le_runID2 ->setMinimumWidth( 225 );

  // Directory
  QLabel* lb_dir = us_label(tr("Directory:"));
  le_dir = us_lineedit("", 1, true);

  // Description
  lb_description = us_label(tr("Description:"), 0);
  // lb_description ->setMaximumWidth( 175 );
  le_description = us_lineedit("", 1);

  // Cell / Channel / Wavelength
  QGridLayout* ccw = new QGridLayout();

  // Meniscus
  lb_Meniscus = us_label(tr("Meniscus:"), 0);
  le_Meniscus = us_lineedit("", 1, true);

  // Bottom
  lb_Bottom = us_label(tr("Bottom:"), 0);
  le_Bottom = us_lineedit("", 1, true);

  // Radius
  lb_Radius = us_label(tr("Radius:"), 0);
  le_Radius = us_lineedit("", 1, true);

  lb_triple = us_banner(tr("Cell / Channel / Wavelength"), -1);
  lw_triple = us_listwidget();
  //    lw_triple->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
  lw_triple->setMaximumWidth(150);
  QLabel* lb_ccwinfo = us_label(tr("Enter Associated Triple (c/c/w) Info:"));
  lb_ccwinfo->setMaximumHeight(25);

  // Set up centerpiece drop-down
  cb_centerpiece = new US_SelectBoxRa(this);
  centerpieceInfo();
  cb_centerpiece->load();

  // External program to enter solution information
  pb_solution = us_pushbutton(tr("Manage Solutions"), false);
  pb_applyAll = us_pushbutton(tr("Apply to All"), false);
  // Choosing reference channel
  pb_reference = us_pushbutton(tr("Referencing"), false);
  pb_cancelref = us_pushbutton(tr("Undo Referencing"), false);
  // Drop scan
  pb_dropScan = us_pushbutton(tr("Drop Channel?"), false);
  le_dropScan = us_lineedit("Keep Channel", 1);
  QLabel* lb_solution = us_label(tr("Solution:"));
  le_solutionDesc = us_lineedit("", 1, true);

  // Standard pushbuttons
  QPushButton* pb_reset = us_pushbutton(tr("Reset"));
  QPushButton* pb_help = us_pushbutton(tr("Help"));
  pb_saveUS3 = us_pushbutton(tr("Save"), false);
  QPushButton* pb_close = us_pushbutton(tr("Close"));

  // Add widgets to layouts
  settings->addWidget(lb_DB, row++, 0, 1, 4);
  settings->addWidget(wg_investigator, row, 0, 1, 2);
  settings->addWidget(le_investigator, row++, 2, 1, 2);
  settings->addLayout(disk_controls, row++, 0, 1, 4);
  settings->addWidget(lb_run, row++, 0, 1, 4);
  settings->addWidget(pb_importlechner, row, 0, 1, 2);
  settings->addWidget(pb_editRuninfo, row++, 2, 1, 2);
  settings->addWidget(pb_importmwlramp, row, 0, 1, 2);

  settings->addWidget(pb_loadUS3, row++, 2, 1, 2);
  //    settings ->addWidget( pb_details,      row++, 2, 1, 2 );
  //    settings ->addWidget( lb_tolerance,    row,   0, 1, 2 );
  //    settings ->addWidget( ct_tolerance,    row++, 2, 1, 2 );

  ccw->addWidget(lb_runinfo, row++, 0, 1, 12);
  ccw->addWidget(lb_runID2, row, 0, 1, 3);
  ccw->addWidget(le_runID2, row++, 3, 1, 9);
  ccw->addWidget(lb_dir, row, 0, 1, 3);
  ccw->addWidget(le_dir, row++, 3, 1, 9);
  ccw->addWidget(lb_triple, row++, 0, 1, 12);
  ccw->addWidget(lb_description, row, 0, 1, 3);
  ccw->addWidget(le_description, row++, 3, 1, 9);

  ccw->addWidget(lw_triple, row, 0, 8, 4);

  ccw->addWidget(lb_Meniscus, row, 4, 1, 4);
  ccw->addWidget(le_Meniscus, row++, 8, 1, 4);
  ccw->addWidget(lb_Bottom, row, 4, 1, 4);
  ccw->addWidget(le_Bottom, row++, 8, 1, 4);
  ccw->addWidget(lb_Radius, row, 4, 1, 4);
  ccw->addWidget(le_Radius, row++, 8, 1, 4);

  ccw->addWidget(lb_ccwinfo, row++, 4, 1, 8);
  ccw->addWidget(cb_centerpiece, row++, 4, 1, 8);
  ccw->addWidget(pb_solution, row, 4, 1, 4);
  ccw->addWidget(pb_applyAll, row++, 8, 1, 4);
  ccw->addWidget(pb_reference, row, 4, 1, 4);
  ccw->addWidget(pb_cancelref, row++, 8, 1, 4);
  ccw->addWidget(pb_dropScan, row, 4, 1, 4);
  ccw->addWidget(le_dropScan, row++, 8, 1, 4);
  ccw->addWidget(lb_solution, row, 0, 1, 3);
  ccw->addWidget(le_solutionDesc, row++, 3, 1, 9);
  mwl_controls->addWidget(lb_mwlctrl, row++, 0, 1, 4);
  mwl_controls->addWidget(le_lambraw, row++, 0, 1, 4);
  mwl_controls->addWidget(lb_lambstrt, row, 0, 1, 1);
  mwl_controls->addWidget(cb_lambstrt, row, 1, 1, 1);
  mwl_controls->addWidget(lb_lambstop, row, 2, 1, 1);
  mwl_controls->addWidget(cb_lambstop, row++, 3, 1, 1);
  mwl_controls->addWidget(lb_lambplot, row, 0, 1, 1);
  mwl_controls->addWidget(cb_lambplot, row, 1, 1, 1);
  mwl_controls->addWidget(pb_lambprev, row, 2, 1, 1);
  mwl_controls->addWidget(pb_lambnext, row++, 3, 1, 1);
  //    ccw -> addLayout        ( mwl_controls,    row++, 0, 1, 12);
  ccw->addWidget(lb_status, row++, 0, 1, 12);
  ccw->addWidget(le_status, row++, 0, 1, 12);

  settings->addLayout(ccw, row++, 0, 1, 4);

  settings->addWidget(pb_reset, row, 0, 1, 1);
  settings->addWidget(pb_help, row, 1, 1, 1);
  settings->addWidget(pb_saveUS3, row, 2, 1, 1);
  settings->addWidget(pb_close, row++, 3, 1, 1);

  // Plot layout for the right side of window
  QBoxLayout* plot = new US_Plot(data_plot, tr("Ramp Data"), tr("w²t (/ s¯¹)"),
                                 tr("Intensity (/AU)"));

  data_plot->setMinimumSize(500, 300);

  data_plot->enableAxis(QwtPlot::xBottom, true);
  data_plot->enableAxis(QwtPlot::yLeft, true);

  data_plot->setAxisScale(QwtPlot::xBottom, 5.7, 7.3);
  data_plot->setAxisScale(QwtPlot::yLeft, 0.0, 1.5);

  picker = new US_PlotPicker(data_plot);
  picker->setRubberBand(QwtPicker::VLineRubberBand);
  picker->setMousePattern(QwtEventPattern::MouseSelect1, Qt::LeftButton,
                          Qt::ControlModifier);

  QGridLayout* todo = new QGridLayout();

  // Instructions ( missing to do items )
  QLabel* lb_todoinfo = us_banner(tr("Instructions ( to do list )"), 0);
  lw_todoinfo = us_listwidget();
  lw_todoinfo->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
  lw_todoinfo->setMaximumHeight(90);
  lw_todoinfo->setSelectionMode(QAbstractItemView::NoSelection);

  // Scan controls:
  todo->addWidget(lb_todoinfo, row++, 0, 1, 4);
  todo->addWidget(lw_todoinfo, row++, 0, 1, 4);

  // Connect signals and slots
  if (isadmin)
    connect(pb_investigator, SIGNAL(clicked()), SLOT(sel_investigator()));
  //    connect( disk_controls,  SIGNAL( changed       ( bool ) ),
  //                             SLOT  ( source_changed( bool ) ) );
  //    connect( pb_importlechner,      SIGNAL( clicked()     ),
  //                             SLOT(   import()      ) );
  connect(pb_editRuninfo, SIGNAL(clicked()), SLOT(editRuninfo(void)));
  connect(pb_loadUS3, SIGNAL(clicked()), SLOT(loadUS3()));
  //    connect( pb_details,     SIGNAL( clicked()     ),
  //                             SLOT(   runDetails()  ) );
  connect(pb_importmwlramp, SIGNAL(clicked()), SLOT(importMWL(void)));
  //    connect( ct_tolerance,   SIGNAL( valueChanged         ( double ) ),
  //                             SLOT  ( toleranceValueChanged( double ) ) );
  connect(le_description, SIGNAL(textEdited(QString)),
          SLOT(changeDescription()));
  connect(lw_triple, SIGNAL(itemSelectionChanged()), SLOT(changeCellchan()));
  connect(cb_centerpiece, SIGNAL(activated(int)),
          SLOT(getCenterpieceIndex(int)));
  connect(pb_solution, SIGNAL(clicked()), SLOT(getSolutionInfo()));
  connect(pb_applyAll, SIGNAL(clicked()), SLOT(tripleApplyAll()));
  connect(pb_reference, SIGNAL(clicked()), SLOT(define_reference()));
  //    connect( pb_cancelref,   SIGNAL( clicked()          ),
  //                             SLOT(   cancel_reference() ) );
  connect(pb_dropScan, SIGNAL(clicked()), SLOT(drop_reference()));
  connect(pb_reset, SIGNAL(clicked()), SLOT(resetAll()));
  connect(pb_help, SIGNAL(clicked()), SLOT(help()));
  connect(pb_saveUS3, SIGNAL(clicked()), SLOT(saveUS3()));
  connect(pb_close, SIGNAL(clicked()), SLOT(close()));

  // Now let's assemble the page

  QVBoxLayout* left = new QVBoxLayout;

  left->addLayout(settings);

  QVBoxLayout* right = new QVBoxLayout;

  right->addLayout(mwl_controls);
  right->addLayout(plot);
  right->addLayout(todo);

  QHBoxLayout* main = new QHBoxLayout(this);
  main->setSpacing(2);
  main->setContentsMargins(2, 2, 2, 2);

  main->addLayout(left);
  main->addLayout(right);
  main->setStretchFactor(left, 3);
  main->setStretchFactor(right, 5);

  DbgLv(1) << "CGui: GUI setup complete";
  //    reset();
  DbgLv(1) << "CGui: reset complete";
}

// User pressed the import ramp data button KL
void US_RampGui::importMWL(void) {
  // Ask for data directory
  QString dir = QFileDialog::getExistingDirectory(
      this, tr("Raw MWL Data Directory"), US_Settings::importDir(),
      QFileDialog::DontResolveSymlinks);
  qDebug() << dir;
  //    QString dir = "/root/ultrascan/imports/mw_speedramp_test_runID";

  // Restore area beneath dialog
  qApp->processEvents();

  if (dir.isEmpty()) return;

  dir.replace("\\", "/");  // WIN32 issue

  currentDir = QString(dir);
  runID = currentDir.section("/", -1, -1);

  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

  // Read the data
  le_status->setText(tr("Importing MWL Ramp data ..."));
  qApp->processEvents();

  QString runID_xml_mwl;
  ramp.import_data(currentDir, le_status, allData, runID_xml_mwl);

  /////////// runType could be useful to distinguish MWL from Lechner data
  //    runType     = "rampI";

  ExpData.runID = runID_xml_mwl;

  le_runID2->setText(ExpData.runID);
  le_runID->setText(ExpData.runID);
  le_dir->setText(currentDir);

  if (runID != runID_xml_mwl) {  // Output warning when resetting (but only if
                                 // we have data)
    int status = QMessageBox::information(
        this, tr("Information"),
        tr("The runID from xml-file doesn't match directory name."), tr("&OK"),
        tr("&Cancel"), 0, 0, 1);

    if (status != 0) {
      reset();
      QApplication::restoreOverrideCursor();
      return;
    }
  }
  runID = runID_xml_mwl;

  ////////////////////////////////////////////////////////////////////////////
  //  For the future implementation of wl-averaging and reduction of wl-span
  //    //////////////////////////////////////////////////////////////////////
  //    // Define default tolerances before converting
  //    scanTolerance = ( runType == "WA" ) ? 0.1 : 5.0; 	// No idea what
  //    qDebug()<< "scanTolerance" << scanTolerance;	// that's for..
  //    ct_tolerance->setValue( scanTolerance );		// -still not
  //    working?
  //
  //    show_mwl_control( true );
  //    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
  //
  //    // Set initial lambda range; build the output data
  //    le_status->setText( tr( "Duplicating wavelengths ..." ) );
  //    qApp->processEvents();
  //    slambda       = mwl_data.countOf( "slambda" );
  //    elambda       = mwl_data.countOf( "elambda" );
  //    ramp.set_lambdas   ( slambda, elambda );
  //    //////////////////////////////////////////////////////

  init_output_data();

  setTripleInfo();

  // Initialize some lineedits and plot graph for selected cellchan..
  changeCellchan();
  QApplication::restoreOverrideCursor();

  pb_editRuninfo->setEnabled(true);
  saveStatus = NOT_SAVED;
  enableControls();
  le_status->setText(tr("Finished Import of MWL Ramp data"));
}

void US_RampGui::reset(void) {
  lw_triple->clear();

  le_dir->setText("");

  le_description->setText("");
  le_runID->setText("");
  le_runID2->setText("");
  le_Bottom->setText("");
  le_Meniscus->setText("");
  le_Radius->setText("");

  pb_importlechner->setEnabled(true);
  pb_loadUS3->setEnabled(true);
  pb_importmwlramp->setEnabled(true);
  pb_cancelref->setEnabled(false);
  pb_dropScan->setEnabled(false);
  pb_solution->setEnabled(false);
  pb_editRuninfo->setEnabled(false);
  pb_applyAll->setEnabled(false);
  pb_saveUS3->setEnabled(false);
  le_lambraw->clear();

  cb_centerpiece->setEnabled(false);

  // Clear any data structures
  all_chaninfo.clear();
  //    out_chaninfo.clear();
  allData.clear();
  outData.clear();
  ExpData.clear();
  if (isMwl) ramp.clear();
  //    show_plot_progress = true;

  // Erase the todo list
  lw_todoinfo->clear();
  lw_todoinfo->addItem("Load or import some AUC data");

  dataPlotClear(data_plot);
  picker->disconnect();
  data_plot->setAxisScale(QwtPlot::xBottom, 0, 1e10);
  data_plot->setAxisScale(QwtPlot::yLeft, 0.0, 1.5);
  grid = us_grid(data_plot);
  data_plot->replot();

  enableRunIDControl(true);

  toleranceChanged = false;
  saveStatus = NOT_SAVED;
  isPseudo = false;
  isMwl = false;

  pb_reference->setEnabled(false);
  referenceDefined = false;
  DbgLv(1) << "CGui: (1)referDef=" << referenceDefined;

  // Display investigator
  ExpData.invID = US_Settings::us_inv_ID();

  QString number =
      (ExpData.invID > 0) ? QString::number(ExpData.invID) + ": " : "";

  le_investigator->setText(number + US_Settings::us_inv_name());
  show_mwl_control(true);
}

void US_RampGui::resetAll(void) {
  if (allData.size() >
      0) {  // Output warning when resetting (but only if we have data)
    int status = QMessageBox::information(
        this, tr("New Data Warning"),
        tr("This will erase all data currently on the screen, and "
           "reset the program to its starting condition. No hard-drive "
           "data or database information will be affected. Proceed? "),
        tr("&OK"), tr("&Cancel"), 0, 0, 1);

    if (status != 0) return;
  }

  reset();

  le_status->setText(tr("(no data loaded)"));

  runID = "";
  data_plot->setTitle(tr("Ramp Data"));
}

// Function to select the current investigator
void US_RampGui::sel_investigator(void) {
  US_Investigator* inv_dialog = new US_Investigator(true, ExpData.invID);

  connect(inv_dialog, SIGNAL(investigator_accepted(int)),
          SLOT(assign_investigator(int)));

  inv_dialog->exec();
}

// Function to assign the selected investigator as current
void US_RampGui::assign_investigator(int invID) {
  ExpData.invID = invID;

  QString number = (invID > 0) ? QString::number(invID) + ": " : "";

  le_investigator->setText(number + US_Settings::us_inv_name());
}
//
// // Function to change the data source (disk/db)
// void US_ConvertGui::source_changed( bool )
// {
//    QStringList DB = US_Settings::defaultDB();
//
//    if ( DB.size() < 5 )
//    {
//       QMessageBox::warning( this,
//          tr( "Attention" ),
//          tr( "There is no default database set." ) );
//    }
//
//    // Did we switch from disk to db?
//    if ( disk_controls->db() && ! save_diskDB )
//    {
//       // Make sure these are explicitly chosen
//       ExpData.operatorID = 0;
//       ExpData.rotorID    = 0;
//       ExpData.calibrationID = 0;
//    }
//
//    save_diskDB = disk_controls->db();
//
//    // Reinvestigate whether to enable the buttons
//    enableControls();
// }
//
// void US_ConvertGui::update_disk_db( bool db )
// {
//    save_diskDB = disk_controls->db();
//
//    ( db ) ? disk_controls->set_db() : disk_controls->set_disk();
// }
//
// // User changed the dataset separation tolerance
// void US_ConvertGui::toleranceValueChanged( double )
// {
//    toleranceChanged = true;
//    scanTolerance    = ct_tolerance->value();
//    reimport();
// }
//

// Enable the common dialog controls when there is data
void US_RampGui::enableControls(void) {
  if (allData.size() == 0)
    reset();

  else {
    DbgLv(1) << "CGui: enabCtl: have-data";
    // Ok to enable some buttons now
    pb_solution->setEnabled(true);
    cb_centerpiece->setEnabled(true);
    pb_editRuninfo->setEnabled(true);

    pb_dropScan->setEnabled(true);

    //       if ( runType == "RI" )
    pb_reference->setEnabled(true);
    // 	       qDebug()<<"_________________enablecontrols"<<referenceDefined;

    // Disable load buttons if there is data
    pb_importlechner->setEnabled(false);
    pb_loadUS3->setEnabled(false);
    pb_importmwlramp->setEnabled(false);

    // Cell Channel Details
    lb_triple->setText(tr("Cell / Channel"));
    //       ct_tolerance->setMinimumWidth( 160 );
    //       ct_tolerance->setNumButtons  ( 2 );
    //       ct_tolerance->setRange       ( 0.0, 100.0 );
    //       ct_tolerance->setStep        ( 1.0 );
    //       ct_tolerance->setValue       ( scanTolerance );

    // Let's calculate if we're eligible to copy this triple info to all
    // or to save it
    // We have to check against GUID's, because solutions won't have
    // solutionID's yet if they are created as needed offline
    QRegExp rx(
        "^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-"
        "F]{12}$");

    pb_applyAll->setEnabled(false);

    if (all_chaninfo.size() > 1 &&
        rx.exactMatch(all_chaninfo[cellchan].solution.solutionGUID)) {
      pb_applyAll->setEnabled(true);
    }

    enableRunIDControl(saveStatus == NOT_SAVED);
    DbgLv(1) << "CGui: enabCtl: enabRunID complete";
    DbgLv(1) << "CGui:   tLx infsz" << cellchan << all_chaninfo.count();

    enableSaveBtn();
    DbgLv(1) << "CGui: enabCtl: enabSvBtn complete";
  }
}
//
// Enable or disable the runID control
void US_RampGui::enableRunIDControl(bool setEnable) {
  if (setEnable) {
    us_setReadOnly(le_runID2, false);
    connect(le_runID2, SIGNAL(textEdited(QString)), SLOT(runIDChanged()));
  }

  else {
    le_runID2->disconnect();
    us_setReadOnly(le_runID2, true);
  }
}

// Enable the "save" button, if appropriate
// Let's use the same logic to populate the todo list too
void US_RampGui::enableSaveBtn(void) {
  lw_todoinfo->clear();
  int count = 0;
  bool completed = true;
  DbgLv(1) << " enabCtl: tLx infsz" << cellchan << all_chaninfo.count();
  cb_centerpiece->setLogicalIndex(all_chaninfo[cellchan].centerpiece);

  if (allData.size() == 0) {
    count++;
    lw_todoinfo->addItem(QString::number(count) +
                         tr(": Load or import some AUC data"));
    completed = false;
  }

  // Do we have any triples?
  if (all_chaninfo.size() == 0) {
    count++;
    lw_todoinfo->addItem(QString::number(count) +
                         tr(": Load or import some AUC data"));
    completed = false;
  }

  // Is the run info defined?
  QRegExp rx(
      "^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]"
      "{12}$");

  // Not checking operator on disk -- defined as "Local"
  if ((ExpData.rotorID == 0) || (ExpData.calibrationID == 0) ||
      (ExpData.labID == 0) || (ExpData.instrumentID == 0) ||
      (ExpData.label.isEmpty()) ||
      (!rx.exactMatch(ExpData.project.projectGUID))) {
    count++;
    lw_todoinfo->addItem(QString::number(count) + tr(": Edit run information"));
    completed = false;
  }

  // Have we filled out all the c/c/w info?
  // Check GUIDs, because solutionID's may not be present yet.
  foreach (US_Ramp::TripleInfo tripinfo, all_chaninfo) {
    if (!rx.exactMatch(tripinfo.solution.solutionGUID)) {
      count++;
      lw_todoinfo->addItem(QString::number(count) +
                           tr(": Select solution for triple ") +
                           tripinfo.tripleDesc);
      completed = false;
    }
  }

  foreach (US_Ramp::TripleInfo tripinfo, all_chaninfo) {
    if (tripinfo.centerpiece == 0) {
      count++;
      lw_todoinfo->addItem(QString::number(count) +
                           tr(": Select centerpiece for triple ") +
                           tripinfo.tripleDesc);
      completed = false;
    }
  }

  if (disk_controls->db()) {
    // Verify connectivity
    US_Passwd pw;
    QString masterPW = pw.getPasswd();
    US_DB2 db(masterPW);

    if (db.lastErrno() != US_DB2::OK) {
      count++;
      lw_todoinfo->addItem(QString::number(count) +
                           tr(": Verify database connectivity"));
      completed = false;
    }

    // Information is there, but we need to see if the runID exists in the
    // DB. If we didn't load it from there, then we shouldn't be able to sync
    int recStatus = ExpData.checkRunID(&db);

    // if a record is found but saveStatus==BOTH,
    //  then we are editing that record
    if ((recStatus == US_DB2::OK) &&
        (saveStatus != BOTH))  // ||
                               // ( ! ExpData.syncOK ) )
    {
      count++;
      lw_todoinfo->addItem(QString::number(count) +
                           tr(": Select a different runID"));
      completed = false;
    }

    // Not checking operator on disk -- defined as "Local"
    if (ExpData.operatorID == 0) {
      count++;
      lw_todoinfo->addItem(QString::number(count) +
                           tr(": Select operator in run information"));
      completed = false;
    }
  }

  // This can go on the todo list, but should not prevent user from saving
  if ((runType == "rampI") && (!referenceDefined)) {
    count++;
    lw_todoinfo->addItem(QString::number(count) +
                         tr(": Define reference scans"));
  }

  // If we made it here, user can save
  pb_saveUS3->setEnabled(completed);
}

// Process when the user changes the runID
void US_RampGui::runIDChanged(void) {
  // See if we need to update the runID
  QRegExp rx("^[A-Za-z0-9_-]{1,80}$");
  QString new_runID = le_runID2->text();

  if (rx.indexIn(new_runID) >= 0) {
    runID = new_runID;
    if (runID.length() > 50) {
      QMessageBox::warning(this, tr("RunID Name Too Long"),
                           tr("The runID name may be at most\n"
                              "50 characters in length."));
      runID = runID.left(50);
    }
    //       plot_titles();
  }

  // If the runID has changed, a number of other things need to change too,
  // for instance GUID's.
  ExpData.clear();
  foreach (US_Ramp::TripleInfo tripinfo, all_chaninfo) tripinfo.clear();
  le_runID2->setText(runID);
  le_runID->setText(runID);

  saveStatus = NOT_SAVED;

  // Set the directory too
  QDir resultDir(US_Settings::resultDir());
  currentDir = resultDir.absolutePath() + "/" + runID + "/";
  le_dir->setText(currentDir);
}

// Function to generate a new guid for experiment, and associate with DB
void US_RampGui::editRuninfo(void) {
  DbgLv(1) << "CGui: edRuninfo: IN";
  qDebug() << "___________________savestatus" << saveStatus;
  if (saveStatus == NOT_SAVED) {
    ExpData.show();
    // Create a new GUID for the experiment as a whole
    ExpData.expGUID = US_Util::new_guid();
  }

  getExpInfo();
  DbgLv(1) << "CGui: edRuninfo: getExpInfo complete";
  ExpData.show();
}

// Function to load US3 data
void US_RampGui::loadUS3(QString dir) {
  DbgLv(1) << "CGui: ldUS3: IN";
  //    dir = "/root/ultrascan/results/mw_speedramp_test_runID/";
  /*  if ( disk_controls->db() )
 //       loadUS3DB();

    else*/
  if (dir.isEmpty())
    loadUS3Disk();

  else
    loadUS3Disk(dir);

  //    checkTemperature();          // Check to see if temperature varied too
  //    much
  DbgLv(1) << "CGui: ldUS3: RTN";
}

void US_RampGui::loadUS3Disk(void) {
  // Ask for data directory
  QString dir = QFileDialog::getExistingDirectory(
      this, tr("US3 Raw Data Directory"), US_Settings::resultDir(),
      QFileDialog::DontResolveSymlinks);

  // Restore area beneath dialog
  qApp->processEvents();

  if (dir.isEmpty()) return;

  dir.replace("\\", "/");               // WIN32 issue
  if (dir.right(1) != "/") dir += "/";  // Ensure trailing /

  loadUS3Disk(dir);
}

void US_RampGui::loadUS3Disk(QString dir) {
  //    resetAll();
  le_status->setText(tr("Loading data from local disk ..."));
  qApp->processEvents();

  // Check the runID
  QStringList components = dir.split("/", QString::SkipEmptyParts);
  QString new_runID = components.last();

  QRegExp rx("^[A-Za-z0-9_-]{1,80}$");
  if (rx.indexIn(new_runID) < 0) {
    QMessageBox::warning(this, tr("Bad runID Name"),
                         tr("The runID name may consist only of alphanumeric\n"
                            "characters, the underscore, and the hyphen."));
    return;
  }

  // Set the runID and directory
  runID = new_runID;
  le_runID->setText(runID);
  le_runID2->setText(runID);
  le_dir->setText(dir);
  currentDir = QString(dir);

  // Reload the AUC data
  le_status->setText(tr("Loading data from Disk (raw data) ..."));
  qApp->processEvents();
  DbgLv(1) << "CGui: ldUS3Dk: call read";
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  int status = US_Ramp::readUS3Disk(dir, allData, all_chaninfo);
  QApplication::restoreOverrideCursor();
  //    return;

  if (status == US_Ramp::NODATA) {
    QMessageBox::warning(this, tr("No Files Found"),
                         tr("There were no files of the form *.auc\n"
                            "found in the specified directory."));
    return;
  }

  if (status == US_Ramp::NOAUC) {
    QMessageBox::warning(this, tr("UltraScan Error"),
                         tr("Could not read data file.\n"));
    return;
  }

  all_chaninfo[0].show();
  // Initialize export data pointers vector
  init_output_data();
  qDebug() << "____________after_init_out";
  all_chaninfo[0].show();

  // Now try to read the xml file
  DbgLv(1) << "CGui: ldUS3Dk: call rdExp  sz(trinfo)" << all_chaninfo.count();
  le_status->setText(tr("Loading data from Disk (experiment) ..."));
  qApp->processEvents();
  ExpData.clear();

  status = ExpData.readFromDisk(all_chaninfo, runType, runID, dir);
  //    qDebug() << "all_chaninfo_size" << all_chaninfo.size();

  if (status == US_Ramp::CANTOPEN) {
    QString readFile = runID + "." + runType + ".xml";
    QMessageBox::information(
        this, tr("Error"),
        tr("US3 run data ok, but unable to assocate run with DB.\n ") +
            tr("Cannot open read file: ") + dir + readFile);
  }

  else if (status == US_Ramp::BADXML) {
    QString readFile = runID + "." + runType + ".xml";
    QMessageBox::information(
        this, tr("Error"),
        tr("US3 run data ok, but there is an error in association with DB.\n"
           "Improper XML in read file: ") +
            dir + readFile);
  }

  else if (status != US_Ramp::OK) {
    QMessageBox::information(this, tr("Error"), tr("Unknown error: ") + status);
  }

  // Now that we have the experiment, let's read the rest of the
  //  solution and project information
  le_status->setText(tr("Loading data from Disk (project) ..."));
  qApp->processEvents();
  DbgLv(1) << "CGui: ldUS3Dk: call prj-rDk";
  status = ExpData.project.readFromDisk(ExpData.project.projectGUID);

  // Error reporting
  if (status == US_DB2::NO_PROJECT) {
    QMessageBox::information(
        this, tr("Attention"),
        tr("The project was not found.\n"
           "Please select an existing project and try again.\n"));
  }

  else if (status != US_DB2::OK) {
    QMessageBox::information(this, tr("Disk Read Problem"),
                             tr("Could not read data from the disk.\n"
                                "Disk status: ") +
                                 QString::number(status));
  }

  // and clear it out
  if (status != US_DB2::OK) ExpData.project.clear();

  le_status->setText(tr("Project and experiment are loaded."));
  qApp->processEvents();
  QString psolGUID = "";
  DbgLv(1) << "CGui: SOLCHK:loop";
  ///////////////////////////////////////////////////////////////////////////////
  // Now the solutions
  for (int ii = 0; ii < all_chaninfo.size(); ii++) {
    QString csolGUID = all_chaninfo[ii].solution.solutionGUID;
    qDebug() << "___________-sol" << csolGUID;

    if (csolGUID == psolGUID) {
      qDebug() << "bla";
      all_chaninfo[ii].solution = all_chaninfo[ii - 1].solution;
      status = US_DB2::OK;
      continue;
    } else {
      if (csolGUID.isEmpty()) {
        qDebug() << "bla";
        DbgLv(1) << "SOLCHK:  ii csolGUID EMPTY" << ii << csolGUID;
        if (ii > 0) {
          csolGUID = psolGUID;
          all_chaninfo[ii].solution.solutionGUID = csolGUID;
        }
      }
      status = all_chaninfo[ii].solution.readFromDisk(csolGUID);
      qDebug() << "buffer" << all_chaninfo[ii].solution.solutionDesc;
      // 	 qDebug()<<"buffer"<<all_chaninfo[ ii].solution.buffer;
      psolGUID = csolGUID;
      qDebug() << "____status" << status;
    }

    // Error reporting
    if (status == US_DB2::NO_SOLUTION) {
      QMessageBox::information(
          this, tr("Attention"),
          tr("A solution this run refers to was not found,"
             " or could not be read.\n"
             "Please select an existing solution and try again.\n"));
      DbgLv(1) << "SOLERR: ii psolGUID csolGUI" << ii << psolGUID << csolGUID;
    }

    else if (status == US_DB2::NO_BUFFER) {
      QMessageBox::information(
          this, tr("Attention"),
          tr("The buffer this solution refers to was not found.\n"
             "Please restore and try again.\n"));
    }

    else if (status == US_DB2::NO_ANALYTE) {
      QMessageBox::information(
          this, tr("Attention"),
          tr("One of the analytes this solution refers to was not found.\n"
             "Please restore and try again.\n"));
    }

    else if (status != US_DB2::OK) {
      QMessageBox::information(this, tr("Disk Read Problem"),
                               tr("Could not read data from the disk.\n"
                                  "Disk status: ") +
                                   QString::number(status));
    }
    qDebug() << "all_chaninfo[ ii ].solutionDesc0"
             << all_chaninfo[ii].solution.solutionDesc;
    // Just clear it out
    if (status != US_DB2::OK) all_chaninfo[ii].solution.clear();
    qDebug() << "all_chaninfo[ ii ].solutionDesc1"
             << all_chaninfo[ii].solution.solutionDesc;
  }
  DbgLv(1) << "CGui: SOLCHK:loop-END";

  le_status->setText(tr("Solutions are now loaded."));
  qApp->processEvents();
  // /////////////////////////////////////////////////////////////////////////

  referenceDefined = true;
  isPseudo = true;
  DbgLv(1) << "CGui: (5)referDef=" << referenceDefined;
  pb_reference->setEnabled(false);
  pb_cancelref->setEnabled(true);

  if (allData.size() == 0) return;

  le_status->setText(tr("%1 data triples are now loaded.").arg(allData.size()));
  qApp->processEvents();

  DbgLv(1) << "CGui: call setTripleInfo";
  // Update triple information on screen
  cellchan = 0;
  //    qDebug()<<"_________size_allchaninfo ______size_alldata_____cellchan"<<
  //    all_chaninfo.size()<<allData.size()<<cellchan;
  //    qDebug()<<"___meniscus___size_wl____size_alldata[2].intarray0.size"<<allData[cellchan].meniscus<<allData[0].wl_array.size()<<allData[2].intarray.size()<<allData[2].intarray[0].size();

  setTripleInfo();

  DbgLv(1) << "CGui:  setDesc trLx" << cellchan << all_chaninfo.size();
  le_solutionDesc->setText(all_chaninfo[cellchan].solution.solutionDesc);

  // Restore description
  le_description->setText(allData[cellchan].description);
  saveDescription = QString(ExpData.runID);

  // The centerpiece combo box
  cb_centerpiece->setLogicalIndex(all_chaninfo[cellchan].centerpiece);

  // Redo plot
  DbgLv(1) << "CGui: call plot_current";
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  plot_current();
  QApplication::restoreOverrideCursor();

  // Ok to enable some buttons now
  enableControls();
  //    if ( ! disk_controls->db() )
  //       pb_saveUS3  ->setEnabled( true );

  //    pb_details     ->setEnabled( true );
  //    pb_solution    ->setEnabled( true );
  //    cb_centerpiece ->setEnabled( true );
  //    pb_editRuninfo ->setEnabled( true );

  //    // some things one can't do from here
  //    ct_tolerance   ->setEnabled( false );

  enableRunIDControl(false);

  saveStatus = (ExpData.expID == 0) ? HD_ONLY : BOTH;
  pb_editRuninfo->setEnabled(saveStatus == HD_ONLY);

  //    // Read it in ok, so ok to sync with DB
  //    ExpData.syncOK = true;

  //    enableSaveBtn();
  le_status->setText(tr("Local disk data load is complete."));
  qApp->processEvents();
  DbgLv(1) << "CGui: ldUS3Dk: RTN";
}
//
// // Function to load an experiment from the DB
// void US_ConvertGui:: loadUS3DB( void )
// {
// DbgLv(1) << "CGui: ldUS3DB: IN";
//    le_status->setText( tr( "Loading data from DB ..." ) );
//    qApp->processEvents();
//
//    // Verify connectivity
//    US_Passwd pw;
//    QString masterPW = pw.getPasswd();
//    US_DB2 db( masterPW );
//
//    if ( db.lastErrno() != US_DB2::OK )
//    {
//       QMessageBox::information( this,
//           tr( "Error" ),
//           tr( "Error making the DB connection.\n" ) );
//       return;
//    }
//
//    // Present a dialog to ask user which experiment to load
//    QString runID;
// DbgLv(1) << "CGui: ldUS3DB: call GetDBRun";
//    US_GetDBRun dialog( runID );
//
//    if ( dialog.exec() == QDialog::Rejected )
//       return;
//
//    if ( runID == QString( "" ) )
//       return;
//
//    // Restore area beneath dialog
//    qApp->processEvents();
//
//    // Now that we have the runID, let's copy the DB info to HD
//    QDir        readDir( US_Settings::resultDir() );
//    QString     dirname = readDir.absolutePath() + "/" + runID + "/";
//
// DbgLv(1) << "CGui: ldUS3DB: call rdDBExp";
//    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
//    le_status->setText( tr( "Loading data from DB (Experiment) ..." ) );
//    qApp->processEvents();
//    QString status = US_ConvertIO::readDBExperiment( runID, dirname, &db );
//    QApplication::restoreOverrideCursor();
//
//    if ( status  != QString( "" ) )
//    {
//       QMessageBox::information( this, tr( "Error" ), status + "\n" );
//       return;
//    }
//
//    // and load it
// DbgLv(1) << "CGui: ldUS3DB: call ldUS3Dk";
//    le_status->setText( tr( "Loading data from DB (Disk data) ..." ) );
//    qApp->processEvents();
//    loadUS3Disk( dirname );
//
//    saveStatus = BOTH;         // override from loadUS3Disk()
//    ExpData.syncOK = true;     // since we just read it from there
//
//    enableControls();
//
//    le_status->setText( tr( "%1 data triples are now loaded from DB." )
//                        .arg( allData.size() ) );
//    qApp->processEvents();
//
// DbgLv(1) << "CGui: ldUS3DB: RTN";
// }
//
void US_RampGui::getExpInfo(void) {
  DbgLv(1) << "CGui: gExpInf: IN";
  ExpData.runID = le_runID->text();
  ExpData.runTemp = QString::number(allData[0].temperature);

  if (disk_controls->db()) {
    // Then we're working in DB, so verify connectivity
    US_Passwd pw;
    QString masterPW = pw.getPasswd();
    US_DB2 db(masterPW);

    if (db.lastErrno() != US_DB2::OK) {
      QMessageBox::information(this, tr("Error"),
                               tr("Error making the DB connection.\n"));
      return;
    }

    // Check if the run ID already exists in the DB
    int recStatus = ExpData.checkRunID(&db);

    // if saveStatus == BOTH, then we are editing the record from the database
    if ((recStatus == US_DB2::OK) && (saveStatus != BOTH)) {
      QMessageBox::information(
          this, tr("Error"),
          tr("The current runID already exists in the database.\n"
             "To edit that information, load it from the database\n"
             "to start with."));
      return;
    }
  }

  // OK, proceed

  //    // Calculate average temperature
  ///////////////////////////////////////////////////
  //    // ramp: our mwl-machine is not capable
  //    //       of measuring the temperature
  ///////////////////////////////////////////////////
  //    double sum = 0.0;
  //    double count = 0.0;
  //    for ( int i = 0; i < allData.size(); i++ )
  //    {
  //       US_DataIO::RawData raw = allData[ i ];
  //       for ( int j = 0; j < raw.scanData.size(); j++ )
  //          sum += raw.scanData[ j ].temperature;
  //
  //       count += raw.scanData.size();
  //    }

  int dbdisk = (disk_controls->db()) ? US_Disk_DB_Controls::DB
                                     : US_Disk_DB_Controls::Disk;

  US_ExperimentGuiRa* expInfo = new US_ExperimentGuiRa(true,  // signal_wanted
                                                       ExpData, dbdisk);

  connect(expInfo, SIGNAL(updateExpInfoSelection(US_ExperimentRa&)), this,
          SLOT(updateExpInfo(US_ExperimentRa&)));

  connect(expInfo, SIGNAL(cancelExpInfoSelection()), this,
          SLOT(cancelExpInfo()));

  connect(expInfo, SIGNAL(use_db(bool)), SLOT(update_disk_db(bool)));

  expInfo->exec();
  DbgLv(1) << "CGui: gExpInf: RTN";
}

// Updating after user has selected info from experiment dialog
void US_RampGui::updateExpInfo(US_ExperimentRa& d) {
  // Update local copy
  ExpData = d;

  if (this->saveStatus == NOT_SAVED)
    this->saveStatus = EDITING;  // don't delete the data!

  enableControls();
}

void US_RampGui::cancelExpInfo(void) {
  // Don't clear out the data, just don't save anything new

  enableControls();
}

void US_RampGui::getSolutionInfo(void) {
  const int chanID = 1;

  cellchan = lw_triple->currentRow();
  ExpData.runID = le_runID->text();

  int dbdisk = (disk_controls->db()) ? US_Disk_DB_Controls::DB
                                     : US_Disk_DB_Controls::Disk;

  US_Solution solution = all_chaninfo[cellchan].solution;

  US_SolutionGui* solutionInfo = new US_SolutionGui(ExpData.expID,
                                                    chanID,  // channelID
                                                    true,    // signal wanted
                                                    dbdisk,  // data source
                                                    solution);

  connect(solutionInfo, SIGNAL(updateSolutionGuiSelection(US_Solution)), this,
          SLOT(updateSolutionInfo(US_Solution)));

  connect(solutionInfo, SIGNAL(cancelSolutionGuiSelection()), this,
          SLOT(cancelSolutionInfo()));

  connect(solutionInfo, SIGNAL(use_db(bool)), SLOT(update_disk_db(bool)));

  solutionInfo->exec();
}

// Updating after user has selected info from experiment dialog
void US_RampGui::updateSolutionInfo(US_Solution s) {
  cellchan = lw_triple->currentRow();

  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

  // Update local copy
  all_chaninfo[cellchan].solution = s;

  le_solutionDesc->setText(all_chaninfo[cellchan].solution.solutionDesc);

  // Re-plot
  plot_current();

  QApplication::restoreOverrideCursor();

  enableControls();
}

void US_RampGui::cancelSolutionInfo(void) { enableControls(); }

// Function to copy the current triple's data to all triples
void US_RampGui::tripleApplyAll(void) {
  US_Ramp::TripleInfo tripinfo = all_chaninfo[cellchan];

  // Copy selected fields only

  for (int ii = 0; ii < all_chaninfo.size(); ii++) {
    all_chaninfo[ii].centerpiece = tripinfo.centerpiece;
    all_chaninfo[ii].solution = tripinfo.solution;
  }

  le_status->setText(
      tr("The current c/c/w information has been"
         " copied to all."));
  qApp->processEvents();

  plot_current();

  enableControls();
}
//
// void US_ConvertGui::runDetails( void )
// {
//    // Create data structures for US_RunDetails2
//    QStringList tripleDescriptions;
//    QVector< US_DataIO::RawData >  currentData;
//    if ( isMwl )
//    {  // For MWL, only pass the 1st data set of each cell/channel
//       for ( int ii = 0; ii < all_chaninfo.size(); ii++ )
//       {
//          currentData        << *outData[ out_chandatx[ ii ] ];
//          tripleDescriptions << all_chaninfo[ ii ].tripleDesc;
//       }
//    }
//
//    else
//    {  // For most data, pass all (non-excluded) triples
//       for ( int ii = 0; ii < out_tripinfo.size(); ii++ )
//       {
//          currentData        << *outData[ ii ];
//          tripleDescriptions << out_tripinfo[ ii ].tripleDesc;
//       }
//    }
//
//    US_RunDetails2* dialog = new US_RunDetails2( currentData, runID,
//    currentDir,
//                                                 tripleDescriptions );
//    dialog->exec();
//    qApp->processEvents();
//    delete dialog;
// }
//
void US_RampGui::changeDescription(void) {
  if (le_description->text().size() < 1)
    le_description->setText(allData[tripDatax].description);

  else {
    allData[tripDatax].description = le_description->text().trimmed();
  }
}

void US_RampGui::changeCellchan() {
  // If a new triple is selected several widgets get updated:
  cellchan = lw_triple->currentRow();
  le_dir->setText(currentDir);
  le_description->setText(allData[cellchan].description);
  le_solutionDesc->setText(all_chaninfo[cellchan].solution.solutionDesc);
  QString bottom = QString::number(allData[cellchan].bottom);
  QString meniscus = QString::number(allData[cellchan].meniscus);
  QString radius = QString::number(allData[cellchan].radius);
  le_Bottom->setText(bottom);
  le_Meniscus->setText(meniscus);
  le_Radius->setText(radius);
  // The centerpiece combo box
  cb_centerpiece->setLogicalIndex(all_chaninfo[cellchan].centerpiece);

  // chan excluded?
  if (all_chaninfo[cellchan].excluded) {
    le_dropScan->setText("Drop Channel");
  } else if (!all_chaninfo[cellchan].excluded) {
    le_dropScan->setText("Keep Channel");
  }

  // Redo plot
  plot_current();
}
//
// Reset triple controls after a change
void US_RampGui::setTripleInfo(void) {
  exp_lambdas = allData[0].wl_array;
  all_lambdas = allData[0].wl_array;
  // Load them into the list box
  int trListSave = lw_triple->currentRow();

  lw_triple->disconnect();
  lw_triple->clear();

  int nchans = allData.size();
  qDebug() << "__________nchans" << nchans;
  for (int iii = 0; iii < nchans; iii++) {
    lw_triple->addItem(all_chaninfo[iii].tripleDesc);
  }

  cellchan = qMax(0, qMin(trListSave, (nchans - 1)));

  qDebug() << "____triplistx" << cellchan;
  // Get wavelengths for the currently selected cell/channel
  nlambda = allData[cellchan].wl_array.size();
  slambda = exp_lambdas[0];
  elambda = exp_lambdas[nlambda - 1];
  int plambda = cb_lambplot->currentText().toInt();
  lambda = nlambda / 2;

  DbgLv(1) << " sTI:  lambda" << lambda << "nlambda" << nlambda;

  mwl_connect(false);
  cb_lambplot->clear();

  for (int wvx = 0; wvx < nlambda; wvx++) {  // Rebuild the plot lambda list
    int ilamb = exp_lambdas[wvx];
    cb_lambplot->addItem(QString::number(ilamb));

    if (ilamb == plambda) lambda = wvx;
  }

  // Re-do selections for lambda start,stop,plot
  cb_lambstrt->setCurrentIndex(all_lambdas.indexOf(slambda));
  cb_lambstop->setCurrentIndex(all_lambdas.indexOf(elambda));
  cb_lambplot->setCurrentIndex(lambda);

  mwl_connect(true);
  lw_triple->setCurrentRow(cellchan);
  connect(lw_triple, SIGNAL(itemSelectionChanged()), SLOT(changeCellchan()));
}
/////////////////////////////////////////////////////////////////////////
// Makes no sense for MWL-Erlangen due to lack of temperature measurement
// Maybe later..
/////////////////////////////////////////////////////////////////////////
// void US_ConvertGui::checkTemperature( void )
// {
//    // Temperature check
//    double dt = 0.0;
//
//    foreach( US_DataIO::RawData triple, allData )
//    {
//        double temp_spread = triple.temperature_spread();
//        dt = ( temp_spread > dt ) ? temp_spread : dt;
//    }
//
//    if ( dt > US_Settings::tempTolerance() )
//    {
//       QMessageBox::warning( this,
//             tr( "Temperature Problem" ),
//             tr( "The temperature in this run varied over the course\n"
//                 "of the run to a larger extent than allowed by the\n"
//                 "current threshold (" )
//                 + QString::number( US_Settings::tempTolerance(), 'f', 1 )
//                 + " " + DEGC + tr( "). The accuracy of experimental\n"
//                 "results may be affected significantly." ) );
//    }
// }
//
void US_RampGui::getCenterpieceIndex(int) {
  qDebug() << "all_chaninfo.size();_____________" << all_chaninfo.size();
  cellchan = lw_triple->currentRow();
  int cpID = cb_centerpiece->getLogicalID();
  all_chaninfo[cellchan].centerpiece = cpID;
  // DbgLv(1) << "getCenterpieceIndex " << all_chaninfo[cellchan].centerpiece;

  enableSaveBtn();
}
//
// User pressed the Define reference button while analyzing intensity data
void US_RampGui::define_reference(void) {
  // identify reference cellchan
  reference_cellchan = lw_triple->currentRow();

  // reference point by point through all triples, except the reference cellchan
  for (int cc = 0; cc < allData.size(); cc++) {
    if (cc != reference_cellchan) {  // cycle through all wavelengths
      for (int wl = 0; wl < allData[cc].intarray.size();
           wl++) {  // cycle through all measurement points for one wl
        for (int w2t = 0; w2t < allData[cc].intarray[wl].size(); w2t++) {
          double old = allData[cc].intarray[wl][w2t];
          double temp = (allData[reference_cellchan].intarray[wl][w2t] / old);
          allData[cc].intarray[wl][w2t] = log10(temp);
          // 	  old = log10(10);
        }
      }
    }
  }
  referenceDefined = true;
  enableSaveBtn();
}
//////////////////////////////////////////////////////////////////////////////
// Makes not too much sense for Ramp data
// Example code from US_Convert left here, if functionality is needed later
//////////////////////////////////////////////////////////////////
// // Select starting point of reference scan in intensity data
// void US_ConvertGui::start_reference( const QwtDoublePoint& p )
// {
//    reference_start   = p.x();
//
//    draw_vline( reference_start );
//    data_plot->replot();
// }
//
// // Select end point of reference scan in intensity data
// void US_ConvertGui::process_reference( const QwtDoublePoint& p )
// {
//    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
//
//    // Just in case we get a second click message right away
//    if ( fabs( p.x() - reference_start ) < 0.005 ) return;
//
//    reference_end = p.x();
//    draw_vline( reference_end );
//    data_plot->replot();
//    picker        ->disconnect();
//
//    // Double check if min < max
//    if ( reference_start > reference_end )
//    {
//       double temp     = reference_start;
//       reference_start = reference_end;
//       reference_end   = temp;
//    }
//
//    // Calculate the averages for all triples
//    PseudoCalcAvg();
//
//    // Now that we have the averages, let's replot
//    Pseudo_reference_triple = cellchan;
//
//    // Default to displaying the first non-reference triple
//    for ( int trx = 0; trx < outData.size(); trx++ )
//    {
//       if ( trx != Pseudo_reference_triple )
//       {
//          cellchan = trx;
//          break;
//       }
//    }
//
//    lw_triple->setCurrentRow( cellchan );
//    plot_current();
//    QApplication::restoreOverrideCursor();
//
//    pb_reference  ->setEnabled( false );
//    referenceDefined = true;
// DbgLv(1) << "CGui: (6)referDef=" << referenceDefined;
//    enableSaveBtn();
//    le_status->setText( tr( "The reference scans have been defined." ) );
//    qApp->processEvents();
// }
//
// // Process a control-click on the plot window
// void US_ConvertGui::cClick( const QwtDoublePoint& p )
// {
//    switch ( step )
//    {
//       case SPLIT :
//          if ( countSubsets < 4 )
//          {
//              // process equil-abs data
//              draw_vline( p.x() );
//              subsets << p.x();
//              countSubsets++;
//          }
//
//          break;
//
//       case REFERENCE :
//          // process reference scan
//          if ( reference_start == 0.0 )
//             start_reference( p );
//
//          else
//             process_reference( p );
//
//       default :
//          break;
//
//    }
// }
//
// // Reference calculation for pseudo-absorbance
// void US_ConvertGui::PseudoCalcAvg( void )
// {
//    if ( referenceDefined ) return;  // Average calculation has already been
//    done
//
//    if ( isMwl )
//    {  // Do calculations for each wavelength, if MWL
//       PseudoCalcAvgMWL();
//       return;
//    }
//
//    US_DataIO::RawData* referenceData = outData[ tripDatax ];
//    int ref_size = referenceData->xvalues.size();
//
//    for ( int ss = 0; ss < referenceData->scanData.size(); ss++ )
//    {
//       US_DataIO::Scan* scan = &referenceData->scanData[ ss ];
//
//       int    rr     = 0;
//       int    count  = 0;
//       double sum    = 0.0;
//
//       while ( referenceData->radius( rr ) < reference_start  &&
//               rr < ref_size )
//          rr++;
//
//       while ( referenceData->radius( rr ) < reference_end  &&
//               rr < ref_size )
//       {
//          sum += scan->rvalues[ rr ];
//          count++;
//          rr++;
//       }
//
//       if ( count > 0 )
//          ExpData.RIProfile << sum / count;
//
//       else
//          ExpData.RIProfile << 1.0;    // See the log10 function, later
//
//    }
//
//    // Now average around excluded values
//    int lastGood  = 0;
//    int countBad  = 0;
//    for ( int ss = 0; ss < ExpData.RIProfile.size(); ss++ )
//    {
//       // In case there are adjacent excluded scans...
//       if ( allExcludes[ tripDatax ].contains( ss ) )
//          countBad++;
//
//       // Calculate average of before and after for intervening values
//       else if ( countBad > 0 )
//       {
//          double newAvg = ( ExpData.RIProfile[ lastGood ]
//                          + ExpData.RIProfile[ ss ] ) / 2.0;
//
//          for ( int rr = lastGood + 1; rr < ss; rr++ )
//             ExpData.RIProfile[ rr ] = newAvg;
//
//          countBad = 0;
//       }
//
//       // Normal situation -- value is not excluded
//       else
//          lastGood = ss;
//
//    }
//
//    // Now calculate the pseudo-absorbance
//    for ( int trx = 0; trx < outData.size(); trx++ )
//    {
//       US_DataIO::RawData* currentData = outData[ trx ];
//
//       for ( int ss = 0; ss < currentData->scanData.size(); ss++ )
//       {
//          US_DataIO::Scan* scan = &currentData->scanData[ ss ];
//
//          for ( int rr = 0; rr < scan->rvalues.size(); rr++ )
//          {
//             double rvalue = scan->rvalues[ rr ];
//
//             // Protect against possible inf's and nan's, if a reading
//             // evaluates to 0 or wherever log function is undefined or -inf
//             if ( rvalue < 1.0 ) rvalue = 1.0;
//
//             // Check for boundary condition
//             int ndx = ( ss < ExpData.RIProfile.size() )
//                     ? ss : ExpData.RIProfile.size() - 1;
//             scan->rvalues[ rr ] = log10( ExpData.RIProfile[ ndx ] / rvalue );
//          }
//       }
//
//       // Let's mark pseudo-absorbance as different from RI data,
//       //  since it needs some different processing in some places
//       isPseudo = true;
//    }
//
//    // Enable intensity plot
//    referenceDefined = true;
// DbgLv(1) << "CGui: (7)referDef=" << referenceDefined;
//    pb_reference->setEnabled( false );
//    pb_cancelref->setEnabled( true );
// }
//
// // Un-do reference scans apply
// void US_ConvertGui::cancel_reference( void )
// {
//    int wvoff    = 0;
//    int rscans   = ExpData.RI_nscans;
//
//    // Do the inverse operation and retrieve raw intensity data
//    for ( int ii = 0; ii < outData.size(); ii++ )
//    {
//       US_DataIO::RawData* currentData = outData[ ii ];
//
//       if ( isMwl )
//       {  // For MWL, profile is offset by wavelength
//          int iwavl    = out_triples[ ii ].section( " / ", 2, 2 ).toInt();
//          wvoff        = ExpData.RIwvlns.indexOf( iwavl );
//
//          if ( wvoff < 0 )
//          {
//             qDebug() << "Triple " << out_triples[ ii ]
//                << "has NO CORRESPONDING RI PROFILE POINT!!!";
//             wvoff        = 0;
//             QMessageBox::information( this,
//               tr( "Error" ),
//               tr( "Triple %1 has NO CORRESPONDING RI PROFILE POINT!!!" )
//               .arg( out_triples[ ii ] ) );
//             int kwavl    = 99999;
//
//             for ( int jj = 0; jj < ExpData.RI_nwvlns; jj++ )
//             {  // Find index of nearest wavelength
//                int jwavl    = qAbs( ExpData.RIwvlns[ jj ] - iwavl );
//
//                if ( jwavl < kwavl )
//                {
//                   kwavl        = jwavl;
//                   wvoff        = jj;
//                }
//             }
//          }
//       }
//
//       wvoff       *= rscans;
//
//       for ( int jj = 0; jj < currentData->scanData.size(); jj++ )
//       {
//          US_DataIO::Scan* scan  = &currentData->scanData[ jj ];
//          double           rppro = ExpData.RIProfile[ jj + wvoff ];
//
//          for ( int kk = 0; kk < scan->rvalues.size(); kk++ )
//          {
//             double rvalue = scan->rvalues[ kk ];
//
//             scan->rvalues[ kk ] = rppro / pow( 10, rvalue );
//          }
//       }
//    }
//
//    referenceDefined = false;
//    isPseudo         = false;
//    ExpData.RIProfile.clear();
//    reference_start  = 0.0;
//    reference_end    = 0.0;
// DbgLv(1) << "CGui: (8)referDef=" << referenceDefined;
//
//    for ( int ii = 0; ii < all_tripinfo.size(); ii++ )
//       all_tripinfo[ ii ].excluded = false;
//
//    setTripleInfo();
//
//    pb_reference  ->setEnabled( true );
//    pb_cancelref  ->setEnabled( false );
//    cellchan = 0;
//    lw_triple->setCurrentRow( cellchan );
//
//    plot_current();
//
//    enableSaveBtn();
//
//    le_status->setText( tr( "The reference scans have been canceled." ) );
//    qApp->processEvents();
// }
/////////////////////////////////////////////////////////////////////////////////////////////////////

// Drop selected triples
void US_RampGui::drop_reference(void) {
  cellchan = lw_triple->currentRow();
  all_chaninfo[cellchan].excluded = !all_chaninfo[cellchan].excluded;
  if (all_chaninfo[cellchan].excluded) {
    le_dropScan->setText("Drop Channel");
  } else if (!all_chaninfo[cellchan].excluded) {
    le_dropScan->setText("Keep Channel");
  }
}

// Function to save US3 data
void US_RampGui::saveUS3(void) {
  if (disk_controls->db())
    saveUS3DB();  // Save AUCs to disk then DB

  else
    saveUS3Disk();  // Save AUCs to disk
}

// Save to disk (default directory)
int US_RampGui::saveUS3Disk(void) {
  QDir writeDir(US_Settings::resultDir());
  QString dirname = writeDir.absolutePath() + "/" + runID + "/";
  //    if ( saveStatus == NOT_SAVED  &&
  //         writeDir.exists( runID ) )
  //    {
  //         QMessageBox::information( this,
  //            tr( "Error" ),
  //            tr( "The write directory,  " ) + dirname +
  //            tr( " already exists. Please change run ID to a unique value." )
  //            );
  //         return US_Ramp::DUP_RUNID;
  //    }

  if (!writeDir.exists(runID)) {
    if (!writeDir.mkpath(dirname)) {
      QMessageBox::information(
          this, tr("Error"), tr("Cannot write to ") + writeDir.absolutePath());
      return US_Ramp::CANTOPEN;
    }
  }
  qDebug() << "i size_allchaninfo size_alldata" << all_chaninfo.size()
           << allData.size();

  US_Ramp::saveToDisk(outData, all_chaninfo, runID, dirname);

  // Now try to write the xml file
  int status;
  status = ExpData.saveToDisk(outData, all_chaninfo, runType, runID, dirname);

  // How many files should have been written?
  int fileCount = 0;
  for (int i = 0; i < all_chaninfo.size(); i++)
    if (!all_chaninfo[i].excluded) fileCount++;
  DbgLv(1) << "SV:   fileCount" << fileCount;

  // Now try to communicate status
  if (status == US_Ramp::CANTOPEN) {
    QString writeFile = runID + "." + runType + ".xml";
    QMessageBox::information(
        this, tr("Error"),
        tr("Cannot open write file: ") + dirname + writeFile);
  }

  else if (status == US_Ramp::NOXML) {
    // Main xml data is missing
    QMessageBox::information(this, tr("Warning"),
                             tr("The run information file was not written. "
                                "Please click on the "
                                "'Associate Run with DB' button \n\n ") +
                                 QString::number(fileCount) + " " + runID +
                                 tr(" files written."));
    return (status);
  }

  else if (status == US_Ramp::PARTIAL_XML) {
    // xml data is missing for one or more triples
    QMessageBox::information(
        this, tr("Warning"),
        tr("Solution information is incomplete. Please click on the "
           "'Manage Solutions' button for each "
           "cell, channel, and wavelength combination \n\n ") +
            QString::number(fileCount) + " " + runID + tr(" files written."));
    return (status);
  }

  else if (status != US_Ramp::OK) {
    QMessageBox::information(this, tr("Error"), tr("Error: ") + status);
    return (status);
  }
  qDebug() << "_________________savestatus___status" << saveStatus << status;
  // Status is OK
  le_status->setText(
      tr("%1 %2 files were written to disk.").arg(fileCount).arg(runID));
  qApp->processEvents();

  if (saveStatus == NOT_SAVED) saveStatus = HD_ONLY;

  return (US_Ramp::OK);
}

//////////////////////////////// Maybe useful to check db-sync works
// //    // Check to see if we have all the data to write
// //    if ( ! ExpData.syncOK )
// //    {
// //       status = QMessageBox::information( this,
// //                   tr( "Warning" ),
// //                   tr( "The run has not yet been associated with the
// database."
// //                       " Click 'OK' to proceed anyway, or click 'Cancel'"
// //                       " and then click on the 'Edit Run Information'"
// //                       " button to enter this information first.\n" ),
// //                   tr( "&OK" ), tr( "&Cancel" ),
// //                   0, 0, 1 );
// //       if ( status != 0 ) return US_Convert::NOT_WRITTEN;
// //    }
//

//
// //    // If we are here, it's ok to sync with DB
// //    ExpData.syncOK = true;
//
// //    enableRunIDControl( false );
//
// //    if ( isMwl )
// //    {  // Where triples were expanded for MWL, skip plot save
// //       return( US_Convert::OK );
// //    }
///////////////////////////////////////////////////////////////////////////

// Save to Database
void US_RampGui::saveUS3DB(void) {
  //    // Verify connectivity
  //    US_Passwd pw;
  //    QString masterPW = pw.getPasswd();
  //    US_DB2 db( masterPW );
  //
  //    if ( db.lastErrno() != US_DB2::OK )
  //    {
  //       QMessageBox::information( this,
  //              tr( "Error" ),
  //              tr( "Database connectivity error" ) );
  //
  //       return;
  //    }
  // DbgLv(1) << "DBSv:  (1)trip0tripFilename" <<
  // out_tripinfo[0].tripleFilename;
  //
  //    // Ok, let's make sure they know what'll happen
  //    if ( saveStatus == BOTH )
  //    {
  //      int status = QMessageBox::information( this,
  //               tr( "Warning" ),
  //               tr( "This will overwrite the raw data currently in the " ) +
  //               tr( "database, and all existing edit profiles, models "  ) +
  //               tr( "and noise data will be deleted too. Proceed? "      ),
  //               tr( "&OK" ), tr( "&Cancel" ),
  //               0, 0, 1 );
  //      if ( status != 0 ) return;
  //    }
  //
  //    else
  //    {
  //      int status = QMessageBox::information( this,
  //               tr( "Warning" ),
  //               tr( "Once this data is written to the DB you will not "  ) +
  //               tr( "be able to make changes to it without erasing the " ) +
  //               tr( "edit profiles, models and noise files too. Proceed? " ),
  //               tr( "&OK" ), tr( "&Cancel" ),
  //               0, 0, 1 );
  //      if ( status != 0 ) return;
  //    }
  //
  //    // First check some of the data with the DB
  //    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
  //    le_status->setText( tr( "Preparing Save with DB check ..." ) );
  //    qApp->processEvents();
  //
  //    int status = US_ConvertIO::checkDiskData( ExpData, out_tripinfo, &db );
  //
  //    QApplication::restoreOverrideCursor();
  // DbgLv(1) << "DBSv:  (2)trip0tripFilename" <<
  // out_tripinfo[0].tripleFilename;
  //
  //    if ( status == US_DB2::NO_PERSON )  // Investigator or operator doesn't
  //    exist
  //    {
  //       QMessageBox::information( this,
  //             tr( "Error" ),
  //             tr( "This investigator or instrument operator was not found\n"
  //                 "in the database.\n" ) );
  //       return;
  //    }
  //
  //    if ( status == US_DB2::BADGUID )
  //    {
  //       QMessageBox::information( this,
  //             tr( "Error" ),
  //             tr( "Bad GUID format.\n"
  //                 "Please click on Edit Run Information"
  //                 " and re-select hardware.\n" ) );
  //       return;
  //    }
  //
  //    if ( status == US_DB2::NO_ROTOR )
  //    {
  //       QMessageBox::information( this,
  //             tr( "Error" ),
  //             tr( "Don't recognize the rotor configuration.\n"
  //                 "Please click on Edit Run Information"
  //                 " and re-select hardware.\n") );
  //       return;
  //    }
  //
  //    if ( status != US_DB2::OK && status != US_DB2::NO_RAWDATA )
  //    {
  //       // US_DB2::OK means we're updating; US_DB2::NO_RAWDATA means it's new
  //       QMessageBox::information( this,
  //             tr( "Error" ),
  //             db.lastError() + " (" + db.lastErrno() + ")\n" );
  //       return;
  //    }
  //
  //    // Save updated files and prepare to transfer to DB
  //    le_status->setText( tr( "Preparing Save with Disk write ..." ) );
  //    qApp->processEvents();
  //    status = saveUS3Disk();
  //    if ( status != US_Convert::OK )
  //       return;
  // DbgLv(1) << "DBSv:  local files saved";
  //
  //    QString error = QString( "" );
  //
  //    // Get the directory where the auc files are
  //    QDir        resultDir( US_Settings::resultDir() );
  //    QString     dir = resultDir.absolutePath() + "/" + ExpData.runID + "/";
  //
  //    if ( ! resultDir.exists( ExpData.runID ) )
  //    {
  //       QMessageBox::information( this,
  //             tr( "Error" ),
  //             tr( "Cannot read from " ) + dir ); // aucDir.absolutePath() );
  //       return;
  //    }
  //
  //    QStringList nameFilters = QStringList( "*.auc" );
  //
  //    QDir readDir( dir );
  //
  //    QStringList files =  readDir.entryList( nameFilters,
  //          QDir::Files | QDir::Readable, QDir::Name );
  // DbgLv(1) << "DBSv:  files count" << files.size();
  //
  //    if ( files.size() == 0 )
  //    {
  //       QMessageBox::warning( this,
  //             tr( "No Files Found" ),
  //             tr( "There were no files of the form *.auc\n"
  //                 "found in the specified directory." ) );
  //       return;
  //    }
  //
  //    if ( ExpData.checkRunID( &db ) == US_DB2::OK && ( saveStatus != BOTH ) )
  //    {
  //       // Then the user is trying to overwrite a runID that is already in
  //       the DB QMessageBox::warning( this,
  //             tr( "Duplicate runID" ),
  //             tr( "This runID already exists in the database. To edit that "
  //                 "run information, load it from there to begin with.\n" ) );
  //       return;
  //    }
  //
  //    // If saveStatus == BOTH already, then it came from the db to begin with
  //    // and it should be updated. Otherwise, there shouldn't be any database
  //    // records with this runID found
  //    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
  //    le_status->setText( tr( "Saving Experiment to DB ..." ) );
  //    qApp->processEvents();
  //    status = ExpData.saveToDB( ( saveStatus == BOTH ), &db );
  //    QApplication::restoreOverrideCursor();
  //
  //    if ( status == US_DB2::NO_PROJECT )
  //    {
  //       QMessageBox::warning( this,
  //             tr( "Project missing" ),
  //             tr( "The project associated with this experiment could not be "
  //                 "updated or added to the database.\n" ) );
  //       return;
  //    }
  //
  //    else if ( status == US_DB2::DUPFIELD )
  //    {
  //       QMessageBox::warning( this,
  //             tr( "Duplicate runID" ),
  //             tr( "The runID already exists in the database.\n" ) );
  //       return;
  //    }
  //
  //    else if ( status == US_DB2::DBERROR )
  //    {
  //       // This is what happens in the case of RI data, and the xml is bad
  //       QMessageBox::warning( this,
  //          tr( "Bad RI XML" ),
  //          tr( "There was a problem with the xml data"
  //              " read from the database.\n" ) );
  //    }
  //
  //    else if ( status != US_DB2::OK )
  //    {
  //       QMessageBox::warning( this,
  //          tr( "Problem saving experiment information" ),
  //          tr( "MySQL Error : " ) + db.lastError() + " ("
  //          + QString::number( status ) + ")" );
  //       return;
  //    }
  //
  //    // If the data came from the database in the first place,
  //    // then this function erases all the edit profiles, models
  //    // and noise files in the database too. However, if one
  //    // changes most of the things here ( solution, rotor, etc. )
  //    // it would invalidate the data anyway.
  //    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
  //    le_status->setText( tr( "Writing raw data to DB ..." ) );
  //    qApp->processEvents();
  //    QString writeStatus = US_ConvertIO::writeRawDataToDB( ExpData,
  //    out_tripinfo,
  //                                                          dir, &db );
  //    QApplication::restoreOverrideCursor();
  //
  //    if ( ! writeStatus.isEmpty() )
  //    {
  //       QMessageBox::warning( this,
  //             tr( "Problem saving experiment" ),
  //             tr( "Unspecified database error: " ) + writeStatus );
  //       le_status->setText( tr( "*ERROR* Problem saving experiment." ) );
  //       return;
  //    }
  //
  //    le_status->setText( tr( "DB data save complete. Saving reports..." ) );
  //    qApp->processEvents();
  //    saveStatus = BOTH;
  //    enableRunIDControl( false );
  //
  //    saveReportsToDB();
  //
  //    le_status->setText( tr( "DB data and reports save is complete." ) );
  //    qApp->processEvents();
}
//
// void US_ConvertGui::saveReportsToDB( void )
// {
//    if ( isMwl )
//       return;
//
//    // Verify connectivity
//    US_Passwd pw;
//    QString masterPW = pw.getPasswd();
//    US_DB2 db( masterPW );
//
//    if ( db.lastErrno() != US_DB2::OK )
//    {
//       QMessageBox::information( this,
//              tr( "Error" ),
//              tr( "Database connectivity error" ) );
//
//       return;
//    }
//
//    // Get a list of report files produced by us_convert
//    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
//    QString dir = US_Settings::reportDir() + "/" + runID;
//    QDir d( dir, "cnvt*", QDir::Name, QDir::Files | QDir::Readable );
//    d.makeAbsolute();
//    QStringList files = d.entryList( QDir::Files );
//
//    // Add any run details files to it
//    QDir d2( dir, "rundetail*", QDir::Name, QDir::Files | QDir::Readable );
//    d2.makeAbsolute();
//    files << d2.entryList( QDir::Files );
//
//    // Create US_Report object
//    QString now = QDateTime::currentDateTime().toString();
//    US_Report myReport;
//    myReport.reset();
//    myReport.runID = runID;
//    myReport.title = runID + " Report";
//    myReport.html  = "<p>Report created " + now + "</p>";
//
//    // Save all us_convert report files to the DB
//    QString errorMsg = "";
//    foreach( QString file, files )
//    {
//       // Get description info for the correct triple
//       QStringList parts = file.split( "." );
//       QString fileTriple = US_Util::expanded_triple( parts[ 1 ], true );
//
//       // Match the triple to find the correct description in memory
//       QString description = QString( "" );
//       for ( int ii = 0; ii < out_tripinfo.size(); ii++ )
//       {
//          if ( fileTriple == out_tripinfo[ ii ].tripleDesc )
//          {
//             description = outData[ ii ]->description;
//             break;
//          }
//       }
//
//       // Edit data ID is not known yet, so use 1. It goes in the report
//       document
//       //   table itself, so we're not overwriting anything.
//       US_Report::Status status = myReport.saveDocumentFromFile(
//             dir, file, &db, 1, description );
//       if ( status != US_Report::REPORT_OK )
//       {
//          errorMsg += file + " was not saved to report database; error code: "
//                           + QString::number( status ) + "\n";
//          qDebug() << "US_ConvertGui.saveDocumentFromFile error: "
//                   << db.lastError() << db.lastErrno();
//       }
//    }
//    QApplication::restoreOverrideCursor();
//
//    if ( ! errorMsg.isEmpty() )
//    {
//       QMessageBox::warning( this,
//             tr( "Problem saving reports to DB" ),
//             errorMsg );
//    }
//
// }
//
// bool US_ConvertGui::read( void )
// {
//    // Ask for data directory
//    QString dir = QFileDialog::getExistingDirectory( this,
//          tr( "Raw Data Directory" ),
//          US_Settings::importDir(),
//          QFileDialog::DontResolveSymlinks );
//
//    // Restore area beneath dialog
//    qApp->processEvents();
//
//    if ( dir.isEmpty() ) return( false );
//
//    dir.replace( "\\", "/" );  // WIN32 issue
//
//    return( read( dir ) );
// }
//

bool US_RampGui::centerpieceInfo(void) {
  DbgLv(1) << "CGui: centpInfo: db" << disk_controls->db();
  //    if ( disk_controls->db() )
  //       return centerpieceInfoDB();

  //    else
  return centerpieceInfoDisk();
}

// Function to get abstractCenterpiece names from DB
bool US_RampGui::centerpieceInfoDB(void) {
  // Verify connectivity
  US_Passwd pw;
  QString masterPW = pw.getPasswd();
  US_DB2 db(masterPW);

  if (db.lastErrno() != US_DB2::OK) {
    QMessageBox::information(this, tr("Error"),
                             tr("Database connectivity error"));

    return (false);
  }

  QStringList q("get_abstractCenterpiece_names");
  db.query(q);

  QList<listInfo> options;
  while (db.next()) {
    struct listInfo option;
    option.ID = db.value(0).toString();
    option.text = db.value(1).toString();
    options << option;
  }

  cb_centerpiece->clear();
  if (options.size() > 0) cb_centerpiece->addOptions(options);

  DbgLv(1) << "CGui: centpInfoDB RTN";
  return true;
}

// Function to get abstractCenterpiece names from disk
bool US_RampGui::centerpieceInfoDisk(void) {
  // First figure out the xml file name, and try to open it
  QFile f(US_Settings::appBaseDir() + "/etc/abstractCenterpieces.xml");

  if (!f.open(QIODevice::ReadOnly)) return false;

  QList<listInfo> options;
  QXmlStreamReader xml(&f);
  while (!xml.atEnd()) {
    xml.readNext();

    if (xml.isStartElement()) {
      if (xml.name() == "abstractCenterpiece") {
        struct listInfo option;
        QXmlStreamAttributes a = xml.attributes();
        option.ID = a.value("id").toString();
        option.text = a.value("name").toString();
        options << option;
      }
    }
  }

  bool error = xml.hasError();
  f.close();

  if (error) return false;

  cb_centerpiece->clear();
  if (options.size() > 0) {
    // Let's sort them so they come up like the DB
    for (int i = 0; i < options.size() - 1; i++)
      for (int j = i + 1; j < options.size(); j++)
        if (options[i].text > options[j].text) options.swap(i, j);

    cb_centerpiece->addOptions(options);
  }

  DbgLv(1) << "CGui: centpInfoDk RTN";
  return true;
}

void US_RampGui::plot_current(void) {
  cellchan = lw_triple->currentRow();

  US_mwlRamp::RampRawData currentData = allData[cellchan];

  //    if ( currentData.scanData.empty() ) return;
  //
  //    plot_titles();
  // DbgLv(1) << " PlCur: PlTit RTN";
  //
  //    // Plot current data for cell / channel / wavelength triple
  //    plot_all();
  // DbgLv(1) << " PlCur: PlAll RTN";
  //
  //    // Set the Scan spin boxes
  //    enableScanControls();

  dataPlotClear(data_plot);
  grid = us_grid(data_plot);

  int n = currentData.intarray[0].size();
  // int n_wl = currentData.intarray.size();
  //    qDebug() << "_________________________________n_n_wl"<<n<<n_wl;

  QVector<double> w2tv(
      currentData.intarray[0].size());  // initialize plot vectors
  QVector<double> intarrayv(currentData.intarray[0].size());

  double* w2t = w2tv.data();  // pointer to initialized plot vectors
  double* intarray = intarrayv.data();

  for (int ii = 0; ii < n; ii++) {
    w2t[ii] = currentData.w2t[ii];
    intarray[ii] = currentData.intarray[lambda][ii];
  }

  //    for (int i = 0; i < intarrayv.size(); i++)
  //    {
  //      qDebug()<<"w2t int"<<w2tv.at(i)<<intarrayv.at(i);
  //    }

  QwtPlotCurve* c = us_curve(data_plot, "RawData at fist WaveLength");

  c->setSamples(w2t, intarray, (int)n);
  double minw2t = w2t[0];
  double maxw2t = w2t[n - 1];
  // min and max indexes for the meas data change after
  // referencing --> sorting is required!!
  // from http://www.qcustomplot.com/index.php/support/forum/17
  QVector<double>::iterator maxY =
      std::max_element(intarrayv.begin(), intarrayv.end());
  QVector<double>::iterator minY =
      std::min_element(intarrayv.begin(), intarrayv.end());

  double minint = *minY;
  double maxint = *maxY;

  // Reset the scan curves within the new limits
  double padw2t = (maxw2t - minw2t) / 30.0;
  double padint = (maxint - minint) / 10.0;
  qDebug() << minint << maxint << padint;
  qDebug() << minw2t << maxw2t << padw2t << minw2t - padw2t << maxw2t + padw2t;

  qDebug() << QwtPlot::xBottom << QwtPlot::yLeft;
#if QT_VERSION < 0x050000
  data_plot->setAxisScaleEngine(QwtPlot::xBottom, new QwtLog10ScaleEngine);
#else
  data_plot->setAxisScaleEngine(QwtPlot::xBottom, new QwtLogScaleEngine);
#endif
  data_plot->setAxisScale(QwtPlot::xBottom, 100, maxw2t + padw2t);
  data_plot->setAxisScale(QwtPlot::yLeft, minint - padint, maxint + padint);
  //    data_plot->setAxisTitle( 2, "w2t");
  if (referenceDefined) {
    data_plot->setAxisTitle(0, "Absorbance / AU");
  } else {
    data_plot->setAxisTitle(0, "Intensity / AU");
  }
  //    data_plot->setTitle("Speed Ramp Experiment");
  data_plot->replot();
}
////////////////////////////////////////////// maybe useful lateron for Lechner
///format..
// void US_ConvertGui::plot_titles( void )
// {
// DbgLv(1) << "  PlTit: tDx outsz otrisz" << tripDatax << outData.size()
//  << out_tripinfo.size();
//    char chtype[ 3 ] = { 'R', 'A', '\0' };
//
//    strncpy( chtype, outData[ tripDatax ]->type, 2 );
//    QString dataType    = QString( chtype ).left( 2 );
//    QString triple      = out_tripinfo[ tripDatax ].tripleDesc;
//    QStringList parts   = triple.split(" / ");
// DbgLv(1) << "  PlTit: triple" << triple << "parts" << parts;
//
//    QString  cell       = parts[ 0 ];
//    QString  channel    = parts[ 1 ];
//    QString  wavelen    = isMwl ? cb_lambplot->currentText() : parts[ 2 ];
//
//    // Plot Title and legends
//    QString title;
//    QString xLegend     = tr( "Radius (in cm)" );
//    QString yLegend     = tr( "Absorbance" );
//    QString ccwlong     = runID + tr( "\nCell: "       ) + cell
//                                + tr( "  Channel: "    ) + channel
//                                + tr( "  Wavelength: " ) + wavelen;
// DbgLv(1) << "  PlTit: dataType" << dataType;
//
//    if      ( dataType == "RA" )
//    {
//       title = tr( "Radial Absorbance Data\nRun ID: " ) + ccwlong;
//    }
//
//    else if ( dataType == "RI"  &&  isPseudo )
//    {
//       title = tr( "Pseudo Absorbance Data\nRun ID: " ) + ccwlong;
//    }
//
//    else if ( dataType == "RI" )
//    {
//       title = tr( "Radial Intensity Data\nRun ID: " ) + ccwlong;
//       yLegend = tr( "Radial Intensity at " ) + wavelen + " nm";
//    }
//
//    else if ( dataType == "IP" )
//    {
//       title = tr( "Interference Data\nRun ID: " ) + ccwlong;
//       yLegend = tr( "Fringes" );
//    }
//
//    else if ( dataType == "FI" )
//    {
//       title = tr( "Fluorescence Intensity Data\nRun ID: " ) + ccwlong;
//       yLegend = tr( "Fluorescence Intensity" );
//    }
//
//    else if ( dataType == "WA" )
//    {
//       title = tr( "Wavelength Data\nRun ID: " ) + ccwlong;
//       xLegend = tr( "Wavelength" );
//       yLegend = tr( "Value" );
//    }
//
//    else if ( dataType == "WI" )
//    {
//       title = tr( "Wavelength Intensity Data\nRun ID: " ) + ccwlong;
//       xLegend = tr( "Wavelength" );
//       yLegend = tr( "Value" );
//    }
//
//    else
//       title = tr( "File type not recognized" );
//
//    data_plot->setTitle( title );
//    data_plot->setAxisTitle( QwtPlot::yLeft, yLegend );
//    data_plot->setAxisTitle( QwtPlot::xBottom, xLegend );
//
// }
////////////////////////////////////////////////////////////////////

// void US_ConvertGui::db_error( const QString& error )
// {
//    QMessageBox::warning( this, tr( "Database Problem" ),
//          tr( "Database returned the following error: \n" ) + error );
// }
//
// // User changed the Lambda Start value
// void US_ConvertGui::lambdaStartChanged( int value )
// {
// DbgLv(1) << "lambdaStartChanged" << value;
//    slambda       = cb_lambstrt->itemText( value ).toInt();
//    elambda       = cb_lambstop->currentText()    .toInt();
//    cellchan     = lw_triple->currentRow();
//    int currChan  = all_chaninfo[ cellchan ].channelID;
// DbgLv(1) << "lambdaStartChanged" << value << "sl el tLx cCh"
//  << slambda << elambda << cellchan << currChan;
//
//    for ( int trx = 0; trx < all_tripinfo.count(); trx++ )
//    {
//       int iwavl     = all_tripinfo[ trx ]
//                       .tripleDesc.section( " / ", 2, 2 ).toInt();
//
//       if ( all_tripinfo[ trx ].channelID == currChan )
//          all_tripinfo[ trx ].excluded = ( iwavl < slambda );
// DbgLv(1) << "lStChg:  trx iwavl chnID excl" << trx << iwavl
//  << all_tripinfo[trx].channelID << all_tripinfo[trx].excluded;
//    }
//
//    build_output_data();
// DbgLv(1) << "lStChg: BlOutDa RTN";
//    reset_lambdas();
// DbgLv(1) << "lStChg: RsLambd RTN";
// }
//
/////////////////////////////////////////////////////// useful lateron for savin
///only span of wavelengths implementation
// // User changed the Lambda End value
// void US_ConvertGui::lambdaEndChanged( int value )
// {
// DbgLv(1) << "lambdaEndChanged" << value;
//    slambda       = cb_lambstrt->currentText()    .toInt();
//    elambda       = cb_lambstop->itemText( value ).toInt();
//    cellchan     = lw_triple->currentRow();
//    int currChan  = all_chaninfo[ cellchan ].channelID;
// DbgLv(1) << "lEnChg:  val" << value << "sl el tLx cCh"
//  << slambda << elambda << cellchan << currChan;
//
//    for ( int trx = 0; trx < all_tripinfo.count(); trx++ )
//    {
//       int iwavl     = all_tripinfo[ trx ]
//                       .tripleDesc.section( " / ", 2, 2 ).toInt();
//
//       if ( all_tripinfo[ trx ].channelID == currChan  &&  iwavl > elambda )
//          all_tripinfo[ trx ].excluded = ( iwavl > elambda );
//    }
//
//    build_output_data();
// DbgLv(1) << "lEnChg: BlOutDa RTN";
//    reset_lambdas();
// DbgLv(1) << "lEnChg: RsLambd RTN";
// }
////////////////////////////////////////////////////////////

// // User changed the Lambda Plot value
void US_RampGui::lambdaPlotChanged(int value) {
  cellchan = lw_triple->currentRow();
  lambda = cb_lambplot->currentIndex();

  plot_current();
  pb_lambprev->setEnabled(value > 0);
  pb_lambnext->setEnabled((value + 1) < cb_lambplot->count());

  qDebug() << "____________123" << allData[cellchan].intarray[0][1];
}

// User clicked the Previous Lambda Plot button
void US_RampGui::lambdaPrevClicked() {
  int wvx = qMax(0, cb_lambplot->currentIndex() - 1);

  cb_lambplot->setCurrentIndex(wvx);
}

// User clicked the Next Lambda Plot button
void US_RampGui::lambdaNextClicked() {
  int wvxmax = cb_lambplot->count() - 1;
  int wvx = qMin(wvxmax, cb_lambplot->currentIndex() + 1);

  cb_lambplot->setCurrentIndex(wvx);
}

// Show or hide MWL controls
void US_RampGui::show_mwl_control(bool show) {
  lb_mwlctrl->setVisible(show);
  lb_lambstrt->setVisible(show);
  cb_lambstrt->setVisible(show);
  lb_lambstop->setVisible(show);
  cb_lambstop->setVisible(show);
  lb_lambplot->setVisible(show);
  cb_lambplot->setVisible(show);
  pb_lambprev->setVisible(show);
  pb_lambnext->setVisible(show);

  adjustSize();
}

// Turn MWL connections on or off
void US_RampGui::mwl_connect(bool connect_on) {
  if (connect_on) {
    //       connect( cb_lambstrt,  SIGNAL( currentIndexChanged( int    ) ),
    //                this,         SLOT  ( lambdaStartChanged ( int    ) ) );
    //       connect( cb_lambstop,  SIGNAL( currentIndexChanged( int    ) ),
    //                this,         SLOT  ( lambdaEndChanged   ( int    ) ) );
    connect(cb_lambplot, SIGNAL(currentIndexChanged(int)), this,
            SLOT(lambdaPlotChanged(int)));
    connect(pb_lambprev, SIGNAL(clicked()), this, SLOT(lambdaPrevClicked()));
    connect(pb_lambnext, SIGNAL(clicked()), this, SLOT(lambdaNextClicked()));
  }

  else {
    cb_lambstrt->disconnect();
    cb_lambstop->disconnect();
    cb_lambplot->disconnect();
    pb_lambprev->disconnect();
    pb_lambnext->disconnect();
  }
}

/////////////////////////////////////////////////////// useful lateron for savin
///only span of wavelengths implementation
// // Reset with lambda delta,range changes
// void US_ConvertGui::reset_lambdas()
// {
//    if ( ! isMwl )
//       return;
//
//    int plambda   = cb_lambplot->currentText().toInt();
//    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
//
// DbgLv(1) << "rsL: slambda elambda" << slambda << elambda;
//    mwl_data.set_lambdas   ( slambda, elambda, cellchan );
//    mwl_connect( false );
//
//    nlambda       = mwl_data.lambdas( exp_lambdas );
//    int plx       = nlambda / 2;
//    cb_lambplot->clear();
// DbgLv(1) << "rsL:  nlambda" << nlambda;
//
//    for ( int wvx = 0; wvx < nlambda; wvx++ )
//    {  // Add to plot-lambda list and possibly detect old selection in new
//    list
//       int ilamb     = exp_lambdas[ wvx ];
//       cb_lambplot->addItem( QString::number( ilamb ) );
//
//       if ( ilamb == plambda )  plx = wvx;
//    }
//
// DbgLv(1) << "rsL:  plambda" << plambda << "plx" << plx;
//    cb_lambplot->setCurrentIndex( plx );
//
//    // Rebuild list of triples
//    QStringList celchns;
//    QString     pwvln = " / " + QString::number( exp_lambdas[ 0           ] )
//                      + "-"   + QString::number( exp_lambdas[ nlambda - 1 ] )
//                      + " ("  + QString::number( nlambda ) + ")";
//
//    int ncelchn   = mwl_data.cellchannels( celchns );
//    nlambda       = mwl_data.countOf( "lambda" );
// DbgLv(1) << "rsL: ncc nl szcc szci" << ncelchn << nlambda << celchns.count()
//  << all_chaninfo.count();
//
//    for ( int ii = 0; ii < ncelchn; ii++ )
//    {  // Redo internal channel information description field
//       all_chaninfo[ ii ].tripleDesc  = celchns[ ii ] + pwvln;
//    }
//
//    mwl_connect( true );
//    QApplication::restoreOverrideCursor();
//
//    setTripleInfo();
// DbgLv(1) << "rsL: sTrInf RTN";
//
//    init_excludes();
// DbgLv(1) << "rsL: InExcl RTN";
//
//    plot_current();
// DbgLv(1) << "rsL: PlCurr RTN";
// }
//

//
// Do MWL Gui setup after import or load of MWL data
// void US_RampGui::mwl_setup()
// {
//    mwl_connect( false );
//
//    // Propagate initial lists of Lambdas
//    nlamb_i         = all_lambdas.size();
//    int    rlamb_s  = all_lambdas[ 0 ];
//    int    rlamb_e  = all_lambdas[ nlamb_i - 1 ];
//    cb_lambstrt->clear();
//    cb_lambstop->clear();
//
// //    for ( int ii = 0; ii < nlamb_i; ii++ )
// //    {
// //       QString clamb = QString::number( all_lambdas[ ii ] );
// //       cb_lambstrt->addItem( clamb );
// //       cb_lambstop->addItem( clamb );
// //    }
// //
// //    cb_lambstrt->setCurrentIndex( 0 );
// //    cb_lambstop->setCurrentIndex( nlamb_i - 1 );
// //    nlambda         = mwl_data.lambdas( exp_lambdas );
// //    cb_lambplot->clear();
// //
// //    for ( int ii = 0; ii < nlambda; ii++ )
// //    {
// //       QString clamb = QString::number( exp_lambdas[ ii ] );
// //       cb_lambplot->addItem( clamb );
// //    }
//
// //    cb_lambplot->setCurrentIndex( nlambda / 2 );
// //
// //    show_mwl_control( true );
//    mwl_connect( true );
//
// //    setTripleInfo();
// //    le_description -> setText( allData[ 0 ].description );
// //
// //    // Initialize exclude list
// //    init_excludes();
//
//    plot_current();
//
//    saveStatus = NOT_SAVED;
//
// //    // Ok to enable some buttons now
//    enableControls();
//
//    static QChar clambda( 955 );   // Lambda character
//    QString lambmsg = tr( "%1 raw:  %2 %3 to %4" )
//       .arg( nlamb_i ).arg( clambda ).arg( rlamb_s ).arg( rlamb_e );
//    le_lambraw->setText( lambmsg );
//    qApp->processEvents();
//    adjustSize();
// }

// Initialize output data pointers and lists
void US_RampGui::init_output_data() {
  outData.clear();
  all_chaninfo.clear();

  // Set up initial export-data pointers list and all/out lists
  for (int trx = 0; trx < allData.size(); trx++) {
    US_mwlRamp::RampRawData* edata = &allData[trx];
    //       US_Convert::TripleInfo tripinfo;
    US_Ramp::TripleInfo chaninfo;

    outData << edata;

    QString celchn = QString(edata->cell) + " / " + QString(edata->chan);
    chaninfo.tripleID = trx + 1;
    chaninfo.description = edata->description;
    chaninfo.excluded = false;
    chaninfo = chaninfo;
    chaninfo.tripleDesc = celchn;

    all_chaninfo << chaninfo;
  }
}
