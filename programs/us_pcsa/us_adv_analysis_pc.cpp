//! \file us_adv_analysis_pc.cpp

#include "us_adv_analysis_pc.h"
#include "us_gui_settings.h"
#include "us_mrecs_loader.h"
#include "us_passwd.h"
#include "us_pcsa.h"
#include "us_settings.h"

// constructor:  enhanced plot control widget
US_AdvAnalysisPc::US_AdvAnalysisPc(QVector<US_ModelRecord> *p_mrs, const int nth, US_SolveSim::DataSet *ds0, QWidget *p) :
    US_WidgetsDialog(p, 0) {
   p_mrecs = p_mrs;
   nthr = nth;
   dset0 = ds0;
   parentw = p;
   dbg_level = US_Settings::us_debug();
   store_dir = US_Settings::resultDir() + "/" + dset0->run_data.runID;
   DbgLv(1) << "AA: IN";

   setObjectName("US_AdvAnalysisPc");
   setPalette(US_GuiSettings::frameColor());
   setFont(QFont(US_GuiSettings::fontFamily(), US_GuiSettings::fontSize()));
   setAttribute(Qt::WA_DeleteOnClose, false);
   QFontMetrics fmet(font());

   // lay out the GUI
   setWindowTitle(tr("PCSA Advanced Controls"));

   // Define the layouts
   mainLayout = new QHBoxLayout(this);
   finmodelLayout = new QGridLayout();
   mreclistLayout = new QGridLayout();

   mainLayout->setSpacing(2);
   mainLayout->setContentsMargins(2, 2, 2, 2);

   mainLayout->addLayout(finmodelLayout);
   mainLayout->addLayout(mreclistLayout);
   mainLayout->setStretchFactor(finmodelLayout, 1);
   mainLayout->setStretchFactor(mreclistLayout, 1);
   DbgLv(1) << "AA: define GUI elements";

   // Define banners and labels
   QLabel *lb_fitctrl = us_banner(tr("Fitting Controls:"));
   QLabel *lb_bfmstat = us_banner(tr("Best Final Model Status:"));
   QLabel *lb_mrecctrl = us_banner(tr("Final & Model Records Controls:"));
   QLabel *lb_mrecstat = us_banner(tr("Model Records Status:"));
   QLabel *lb_curvtype = us_label(tr("Curve Type:"));
   QLabel *lb_x_range = us_label(tr("X Range:"));
   QLabel *lb_y_range = us_label(tr("Y Range:"));
   lb_sigmpar1 = us_label(tr("Par 1:"));
   lb_sigmpar2 = us_label(tr("Par 2:"));
   lb_y_strpt = us_label(tr("Line Y Start Point:"));
   lb_y_endpt = us_label(tr("Line Y End Point:"));
   QLabel *lb_crpoints = us_label(tr("Curve Resolution Points:"));
   QLabel *lb_mciters = us_label(tr("Monte Carlo Iterations:"));
   QLabel *lb_progress = us_label(tr("Progress:"));
   QLabel *lb_space1 = us_banner("");
   QLabel *lb_space2 = us_banner("");

   // Define buttons
   pb_loadmrs = us_pushbutton(tr("Load Model Records"));
   pb_storemrs = us_pushbutton(tr("Store Model Records"));
   pb_loadbfm = us_pushbutton(tr("Load Final Model"));
   pb_storebfm = us_pushbutton(tr("Store Final Model"));
   pb_resetbfm = us_pushbutton(tr("Reset Final Model"));
   pb_resetmrs = us_pushbutton(tr("Reset Model Records"));
   pb_buildbfm = us_pushbutton(tr("Build Final Model"));
   pb_mciters = us_pushbutton(tr("Start Monte Carlo"));
   pb_help = us_pushbutton(tr("Help"));
   pb_cancel = us_pushbutton(tr("Cancel"));
   pb_accept = us_pushbutton(tr("Accept"), false);

   // Define counters
   le_x_lower = us_lineedit("1", -1, true);
   le_x_upper = us_lineedit("10", -1, true);
   le_y_lower = us_lineedit("1", -1, false);
   le_y_upper = us_lineedit("4", -1, false);
   le_y_strpt = us_lineedit("1", -1, false);
   le_y_endpt = us_lineedit("4", -1, false);
   le_sigmpar1 = us_lineedit("1", -1, false);
   le_sigmpar2 = us_lineedit("2", -1, false);
   le_crpoints = us_lineedit("100", -1, false);
   le_mciters = us_lineedit("20", -1, false);

   // Define combo box
   cb_curvtype = us_comboBox();
   cb_curvtype->setMaxVisibleItems(3);
   cb_curvtype->addItem(tr("Straight Line"));
   cb_curvtype->addItem(tr("Increasing Sigmoid"));
   cb_curvtype->addItem(tr("Decreasing Sigmoid"));
   cb_curvtype->addItem(tr("Horizontal Line [ C(s) ]"));
   cb_curvtype->addItem(tr("Second-Order Power Law"));

   // Define status text boxes and progress bar
   te_bfmstat = us_textedit();
   te_mrecstat = us_textedit();
   b_progress = us_progressBar(0, 100, 0);
   us_setReadOnly(te_bfmstat, true);
   us_setReadOnly(te_mrecstat, true);
   te_bfmstat->setTextColor(Qt::blue);
   te_mrecstat->setTextColor(Qt::blue);

   // Lay out the left side, of BFM controls and status
   DbgLv(1) << "AA: populate finmodelLayout";
   int row = 0;
   finmodelLayout->addWidget(lb_fitctrl, row++, 0, 1, 6);
   finmodelLayout->addWidget(lb_curvtype, row, 0, 1, 3);
   finmodelLayout->addWidget(cb_curvtype, row++, 3, 1, 3);
   finmodelLayout->addWidget(lb_x_range, row, 0, 1, 3);
   finmodelLayout->addWidget(le_x_lower, row, 3, 1, 1);
   finmodelLayout->addWidget(le_x_upper, row++, 4, 1, 1);
   finmodelLayout->addWidget(lb_y_range, row, 0, 1, 3);
   finmodelLayout->addWidget(le_y_lower, row, 3, 1, 1);
   finmodelLayout->addWidget(le_y_upper, row++, 4, 1, 1);
   finmodelLayout->addWidget(lb_sigmpar1, row, 0, 1, 3);
   finmodelLayout->addWidget(le_sigmpar1, row++, 3, 1, 3);
   finmodelLayout->addWidget(lb_sigmpar2, row, 0, 1, 3);
   finmodelLayout->addWidget(le_sigmpar2, row++, 3, 1, 3);
   finmodelLayout->addWidget(lb_y_strpt, row, 0, 1, 3);
   finmodelLayout->addWidget(le_y_strpt, row++, 3, 1, 3);
   finmodelLayout->addWidget(lb_y_endpt, row, 0, 1, 3);
   finmodelLayout->addWidget(le_y_endpt, row++, 3, 1, 3);
   finmodelLayout->addWidget(lb_crpoints, row, 0, 1, 3);
   finmodelLayout->addWidget(le_crpoints, row++, 3, 1, 3);
   finmodelLayout->addWidget(lb_mciters, row, 0, 1, 3);
   finmodelLayout->addWidget(le_mciters, row++, 3, 1, 3);

   finmodelLayout->addWidget(lb_bfmstat, row++, 0, 1, 6);
   finmodelLayout->addWidget(te_bfmstat, row, 0, 8, 6);
   row += 8;
   finmodelLayout->addWidget(lb_space1, row, 0, 1, 6);

   // Lay out the right side, of Model Record controls and status
   DbgLv(1) << "AA: populate mreclistLayout";
   row = 0;
   mreclistLayout->addWidget(lb_mrecctrl, row++, 0, 1, 6);
   mreclistLayout->addWidget(pb_loadmrs, row, 0, 1, 3);
   mreclistLayout->addWidget(pb_storemrs, row++, 3, 1, 3);
   mreclistLayout->addWidget(pb_loadbfm, row, 0, 1, 3);
   mreclistLayout->addWidget(pb_storebfm, row++, 3, 1, 3);
   mreclistLayout->addWidget(pb_resetbfm, row, 0, 1, 3);
   mreclistLayout->addWidget(pb_resetmrs, row++, 3, 1, 3);
   mreclistLayout->addWidget(pb_buildbfm, row, 0, 1, 3);
   mreclistLayout->addWidget(pb_mciters, row++, 3, 1, 3);

   mreclistLayout->addWidget(lb_mrecstat, row++, 0, 1, 6);
   mreclistLayout->addWidget(te_mrecstat, row, 0, 8, 6);
   mreclistLayout->setRowStretch(row, 2);
   row += 8;
   mreclistLayout->addWidget(lb_progress, row, 0, 1, 1);
   mreclistLayout->addWidget(b_progress, row++, 1, 1, 5);
   mreclistLayout->addWidget(pb_help, row, 0, 1, 2);
   mreclistLayout->addWidget(pb_cancel, row, 2, 1, 2);
   mreclistLayout->addWidget(pb_accept, row++, 4, 1, 2);
   mreclistLayout->addWidget(lb_space2, row, 0, 1, 6);

   cb_curvtype->setEnabled(false);
#if 0
   le_x_lower ->setEnabled( false );
   le_x_upper ->setEnabled( false );
   le_y_lower ->setEnabled( false );
   le_y_upper ->setEnabled( false );
#endif
   int fwidth = fmet.maxWidth();
   int rheight = le_x_lower->height();
   int cminw = fwidth * 4;
   int csizw = cminw + fwidth;
   le_x_lower->setMinimumWidth(cminw);
   le_x_upper->setMinimumWidth(cminw);
   le_y_lower->setMinimumWidth(cminw);
   le_y_upper->setMinimumWidth(cminw);
   le_x_lower->resize(csizw, rheight);
   le_x_upper->resize(csizw, rheight);
   le_y_lower->resize(csizw, rheight);
   le_y_upper->resize(csizw, rheight);

   // Set defaults and status values based on the initial model records
   mrecs_mc.clear();
   mrecs = *p_mrecs;
   nmrecs = mrecs.size();
   mciters = 0;
   kciters = 0;
   bfm0_exists = false;
   mrs0_exists = false;
   bfm_new = false;
   mrs_new = false;
   mc_done = false;
   ctype = CTYPE_IS;
   nisols = 0;
   ncsols = (nmrecs > 0) ? mrecs[ 0 ].csolutes.size() : 0;

   if (ncsols > 0) { // We are starting with models already computed
      mrec = mrecs[ 0 ];

      set_fittings(mrecs);

      DbgLv(1) << "AA: mr p1 p2  m0 p1 p2" << mrec.par1 << mrec.par2 << mrecs[ 0 ].par1 << mrecs[ 0 ].par2
               << "  typ ni nc" << ctype << nisols << ncsols;

      stat_bfm(tr("An initial best final model, with RMSD of %1,\n"
                  "  has been read.")
                  .arg(mrec.rmsd));

      stat_mrecs(tr("An initial model records list, with %1 fits,\n"
                    "  has been read.")
                    .arg(nmrecs));

      mrec0 = mrec; // Save initial model records
      mrecs0 = mrecs;
      bfm0_exists = true;
      mrs0_exists = true;
   }

   else { // We are starting with models not yet computed
      mrec = (nmrecs > 0) ? mrecs[ 0 ] : mrec;
      mrec0 = mrec; // Save initial model records
      mrecs0 = mrecs;
      DbgLv(1) << "AA: nmrecs" << nmrecs << "ncsols" << ncsols;
      stat_bfm(tr("No initial best final model has been read"));
      stat_mrecs(tr("No initial model records list has been read"));
   }
   ctype = mrec.ctype;

   // Define connections
   DbgLv(1) << "AA: connect buttons";
   connect(cb_curvtype, SIGNAL(currentIndexChanged(int)), this, SLOT(curvtypeChanged(int)));

#if 0
   connect( le_x_lower,  SIGNAL( textChanged( const QString& ) ),
            this,        SLOT(   slowerChanged( double ) ) );
   connect( le_x_upper,  SIGNAL( textChanged( const QString& ) ),
            this,        SLOT(   supperChanged( double ) ) );
   connect( le_y_lower,  SIGNAL( textChanged( const QString& ) ),
            this,        SLOT(   klowerChanged( double ) ) );
   connect( le_y_upper,  SIGNAL( textChanged( const QString& ) ),
            this,        SLOT(   kupperChanged( double ) ) );
   connect( le_sigmpar1, SIGNAL( textChanged( const QString& ) ),
            this,        SLOT(   sipar1Changed( double ) ) );
   connect( le_sigmpar2, SIGNAL( textChanged( const QString& ) ),
            this,        SLOT(   sipar2Changed( double ) ) );
   connect( le_crpoints, SIGNAL( textChanged( const QString& ) ),
            this,        SLOT(   pointsChanged( double ) ) );
#endif
   connect(le_mciters, SIGNAL(textChanged(const QString &)), this, SLOT(mciterChanged(double)));

   connect(pb_loadmrs, SIGNAL(clicked()), this, SLOT(load_mrecs()));
   connect(pb_storemrs, SIGNAL(clicked()), this, SLOT(store_mrecs()));
   connect(pb_loadbfm, SIGNAL(clicked()), this, SLOT(load_bfm()));
   connect(pb_storebfm, SIGNAL(clicked()), this, SLOT(store_bfm()));
   connect(pb_resetbfm, SIGNAL(clicked()), this, SLOT(reset_bfm()));
   connect(pb_resetmrs, SIGNAL(clicked()), this, SLOT(reset_mrecs()));
   connect(pb_buildbfm, SIGNAL(clicked()), this, SLOT(build_bfm()));
   connect(pb_mciters, SIGNAL(clicked()), this, SLOT(start_montecarlo()));

   connect(pb_help, SIGNAL(clicked()), this, SLOT(help()));
   connect(pb_cancel, SIGNAL(clicked()), this, SLOT(cancel()));
   connect(pb_accept, SIGNAL(clicked()), this, SLOT(select()));

   curvtypeChanged(1);
   resize(780, 400);
   DbgLv(1) << "Post-resize size" << size();
   qApp->processEvents();
}

// Return state flag from advanced actions and, possibly, MC models
int US_AdvAnalysisPc::advanced_results(QVector<US_ModelRecord> *p_mrecsmc) {
   // Set state flag reflecting new-bfm, new-mrs, montecarlo
   int state = bfm_new ? msk_bfnew : 0;
   state = mrs_new ? (state | msk_mrnew) : state;
   state = mc_done ? (state | msk_mcarl) : state;
   DbgLv(1) << "advanced_results - state=" << state;

   if (p_mrecsmc != 0) {
      if (mc_done) { // If MonteCarlo was done, return its model records
         *p_mrecsmc = mrecs_mc;
      }
      else { // If MonteCarlo not done or overridden, make sure to clear it
         p_mrecsmc->clear();
      }
   }

   return state;
}

// Accept button clicked
void US_AdvAnalysisPc::select() {
   DbgLv(1) << "AA:Accept: mrs_new" << mrs_new << "p_mrecs" << p_mrecs;
   if (mrs_new && p_mrecs != 0) { // If model records are new, return them to the caller
      *p_mrecs = mrecs;
      DbgLv(1) << "AA:Accept: mr mnmx x y" << p_mrecs->at(0).xmin << p_mrecs->at(0).xmax << p_mrecs->at(0).ymin
               << p_mrecs->at(0).ymax;
      DbgLv(1) << "AA:Accept: mr-size mr0-RMSD" << p_mrecs->size() << p_mrecs->at(9).rmsd << "mr0-solsize"
               << p_mrecs->at(0).csolutes.size();
   }

   if (!mc_done) { // Insure there is no montecarlo left over if there shouldn't be
      mrecs_mc.clear();
   }

   accept();
   close();
}

// Cancel button clicked
void US_AdvAnalysisPc::cancel() {
   bfm_new = false;
   mrs_new = false;
   mc_done = false;

   reject();
   close();
}

// Slot to handle a change in curve type
void US_AdvAnalysisPc::curvtypeChanged(int ivalue) {
   DbgLv(1) << "curvtypeChanged" << ivalue;
   ctype = ivalue;
   ctype = (ivalue == 0) ? CTYPE_SL : ctype;
   ctype = (ivalue == 1) ? CTYPE_IS : ctype;
   ctype = (ivalue == 2) ? CTYPE_DS : ctype;
   ctype = (ivalue == 3) ? CTYPE_HL : ctype;
   ctype = (ivalue == 4) ? CTYPE_2O : ctype;
   bool is_sigm = (ctype == CTYPE_IS || ctype == CTYPE_DS);
   bool is_line = !is_sigm;

   lb_sigmpar1->setVisible(is_sigm);
   le_sigmpar1->setVisible(is_sigm);
   lb_sigmpar2->setVisible(is_sigm);
   le_sigmpar2->setVisible(is_sigm);
   lb_y_strpt->setVisible(ctype == CTYPE_SL);
   le_y_strpt->setVisible(ctype == CTYPE_SL);
   lb_y_endpt->setVisible(is_line);
   le_y_endpt->setVisible(is_line);
   if (ctype == CTYPE_HL)
      lb_y_endpt->setText(tr("Line Y End Points:"));
}

#if 0
// Slot to handle a change in S lower bound
void US_AdvAnalysisPc::slowerChanged( double value )
{
DbgLv(1) << "slowerChanged" << value;
}

// Slot to handle a change in S upper bound
void US_AdvAnalysisPc::supperChanged( double value )
{
DbgLv(1) << "supperChanged" << value;
}

// Slot to handle a change in sigmoid par 1
void US_AdvAnalysisPc::sipar1Changed( double value )
{
DbgLv(1) << "sipar1Changed" << value;
}

// Slot to handle a change in sigmoid par 2
void US_AdvAnalysisPc::sipar2Changed( double value )
{
DbgLv(1) << "sipar2Changed" << value;
}

// Slot to handle a change in K(f/f0) lower bound
void US_AdvAnalysisPc::klowerChanged( double value )
{
DbgLv(1) << "klowerChanged" << value;
}

// Slot to handle a change in K(f/f0) upper bound
void US_AdvAnalysisPc::kupperChanged( double value )
{
DbgLv(1) << "kupperChanged" << value;
}

// Slot to handle a change in curve points
void US_AdvAnalysisPc::pointsChanged( double value )
{
DbgLv(1) << "pointsChanged" << value;
}
#endif

// Slot to handle a change in monte carlo iterations
void US_AdvAnalysisPc::mciterChanged(double value) {
   DbgLv(1) << "mciterChanged" << value;
   mciters = ( int ) value;
}

// Slot to load a model records list from database or disk
void US_AdvAnalysisPc::load_mrecs() {
   bool loadDB = dset0->requestID.contains("DB");
   QString mrdesc;
   QString edGUID = dset0->run_data.editGUID;
   QString dsearch = "";
   QString runID = dset0->run_data.runID;
   DbgLv(1) << "load_mrecs  loadDB" << loadDB << "reqID" << dset0->requestID;

   US_MrecsLoader mrldDiag(loadDB, dsearch, mrecs, mrdesc, edGUID, runID);

   if (mrldDiag.exec() != QDialog::Accepted) {
      DbgLv(1) << "mrldDiag.exec==rejected";
      //*DEBUG*
      test_db_mrecs();
      //*DEBUG*
      return;
   }

   // Re-generate curve points for every model record
   nmrecs = mrecs.size();
   double xmin = mrecs[ 0 ].xmin;
   double xmax = mrecs[ 0 ].xmax;
   double ymin = mrecs[ 0 ].ymin;
   double ymax = mrecs[ 0 ].ymax;
   DbgLv(1) << "mrldDiag post-accept xmin xmax ymin ymax" << xmin << xmax << ymin << ymax << "nmrecs" << nmrecs;

   for (int mr = 0; mr < nmrecs; mr++) {
      mrecs[ mr ].xmin = xmin;
      mrecs[ mr ].xmax = xmax;
      mrecs[ mr ].ymin = ymin;
      mrecs[ mr ].ymax = ymax;

      curve_isolutes(mrecs[ mr ]);
   }

   mrec = mrecs[ 0 ];
   ctype = mrec.v_ctype;
   ncsols = mrec.csolutes.size();
   QString sctype = US_ModelRecord::ctype_text(ctype);

   // Build the model that goes along with the BFM
   DbgLv(1) << "pre-bfm-model mr0 rmsd" << mrec.rmsd;
   bfm_model();
   DbgLv(1) << "post-bfm-model mr0 rmsd" << mrec.rmsd << mrecs[ 0 ].rmsd;

   stat_bfm(tr("A new Best Final Model derives from the top spot\n"
               "  of the just-loaded Model Records list.\n"
               "The %1 model has %2 computed solutes\n"
               "  and an RMSD of %3")
               .arg(sctype)
               .arg(ncsols)
               .arg(mrec.rmsd));

   set_fittings(mrecs);
   DbgLv(1) << "post-set-fittings mr0 rmsd" << mrecs[ 0 ].rmsd;

   bfm_new = true;
   mrs_new = true;
   mc_done = false;
   pb_accept->setEnabled(true);
}

// Slot to store a model records list to disk
void US_AdvAnalysisPc::store_mrecs() {
   DbgLv(1) << "store_mrecs";
   // Test and return immediately if valid mrecs still required
   if (mrecs_required("Store Model Records"))
      return;

   // Query and get the file for storing
   QString store_file = store_dir + "/pcsa-mrs-new_mrecs.xml";
   store_file = QFileDialog::getSaveFileName(
      this, tr("Specify XML File Name for Model Records Store"), store_dir,
      tr("Model Records files (*pcsa-mrs-*.xml);;"
         "Any XML files (*.xml);;Any files (*)"));

   if (store_file.isEmpty()) {
      DbgLv(1) << "store_mrecs - FILE NAME EMPTY";
      return;
   }
   else
      DbgLv(1) << "store_mrecs - FILE NAME *NOT* EMPTY" << store_file;

   store_file = store_file.replace("\\", "/");
   QString fdir = store_file.section("/", 0, -2) + "/";
   QString fname = store_file.section("/", -1, -1);

   // Massage the name to be in "mrs_<name>.xml" form
   if (store_file.endsWith(".")) { // Ends with ".":  no ".xml" is to be added
      store_file = store_file.left(store_file.length() - 1);
      fname = fname.left(fname.length() - 1);
   }

   else if (!store_file.endsWith(".xml")) { // If no ".xml" extension, add one
      store_file = store_file + ".xml";
      fname = fname + ".xml";
   }

   if (fname.startsWith(".")) { // Starts with ".":  no "pcsa-mrs-" prefix is to be added
      store_file = fdir + fname.mid(1);
   }

   else if (!fname.startsWith("pcsa-mrs-")) { // If no "pcsa-mrs-" prefix, add one
      store_file = fdir + "pcsa-mrs-" + fname;
   }

   // Open the specified output file
   QFile fileo(store_file);

   if (!fileo.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QMessageBox::critical(this, tr("Open Error"), tr("Cannot open file %1 .").arg(store_file));
      return;
   }

   // Write out the XML file
   ctype = mrecs[ 0 ].v_ctype;
   double xmin = mrecs[ 0 ].xmin;
   double xmax = mrecs[ 0 ].xmax;
   double ymin = mrecs[ 0 ].ymin;
   double ymax = mrecs[ 0 ].ymax;
   int stype = mrecs[ 0 ].stype;
   QXmlStreamWriter xmlo(&fileo);
   QString mrdesc;

   US_ModelRecord::write_modelrecs(xmlo, mrecs, mrdesc, ctype, xmin, xmax, ymin, ymax, stype);
   fileo.close();

   // Report on saved file
   fdir = store_file.section("/", 0, -2) + "/";
   fname = store_file.section("/", -1, -1);
   stat_mrecs(
      tr("Model Records have been stored in file\n"
         "  \"%1\", of directory\n  \"%2\".")
         .arg(fname)
         .arg(fdir),
      true);
}

// Slot to load a best final model from disk
void US_AdvAnalysisPc::load_bfm() {
   DbgLv(1) << "load_bfm";
   //under_construct( "Load Final Model" );
   // Test and return immediately if valid mrecs still required
   if (mrecs_required("Load Final Model"))
      return;

   // Query and get the file for loading
   QString load_file = store_dir + "/pcsa-bfm-old_mrecs.xml";
   load_file = QFileDialog::getOpenFileName(
      this, tr("Select XML File Name for Best Final Model Load"), store_dir,
      tr("Best Final Model files (*pcsa-bfm-*.xml);;"
         "Any XML files (*.xml);;Any files (*)"));

   if (load_file.isEmpty()) {
      return;
   }

   load_file = load_file.replace("\\", "/");
   QString fdir = load_file.section("/", 0, -2) + "/";
   QString fname = load_file.section("/", -1, -1);

   // Open the specified input file
   QFile filei(load_file);

   if (!filei.open(QIODevice::ReadOnly)) {
      QMessageBox::critical(this, tr("Open Error"), tr("Cannot open file %1 .").arg(load_file));
      return;
   }

   // Read in and parse the XML file to generate a new BFM
   int nisols = 0;
   bool is_bfmf = false;
   QString xmlname = "";
   QXmlStreamReader xmli(&filei);

   while (!xmli.atEnd()) {
      xmli.readNext();

      if (xmli.isComment()) { // Verify DOCTYPE PcsaBestFinalModel
         QString comm = xmli.text().toString();
         //DbgLv(1) << "LM:xml: comm" << comm;

         if (comm.contains("PcsaBestFinalModel"))
            is_bfmf = true;

         else {
            QMessageBox::critical(
               this, tr("File Type Error"), tr("File \"%1\" is not a PcsaBestFinalModel XML file.").arg(fname));
            filei.close();
            return;
         }
      }

      xmlname = xmli.name().toString();

      if (xmli.isStartElement()) {
         //DbgLv(1) << "LM:xml: start name" << xmlname;
         QXmlStreamAttributes xattrs = xmli.attributes();

         if (xmlname == "modelrecord") {
            nisols = xattrs.value("curve_points").toString().toInt();
            mrec.taskx = xattrs.value("taskx").toString().toInt();
            mrec.str_y = xattrs.value("start_y").toString().toDouble();
            mrec.end_y = xattrs.value("end_y").toString().toDouble();
            mrec.par1 = xattrs.value("par1").toString().toDouble();
            mrec.par2 = xattrs.value("par2").toString().toDouble();
            mrec.par3 = xattrs.value("par3").toString().toDouble();
            mrec.rmsd = xattrs.value("rmsd").toString().toDouble();
            mrec.ctype = xattrs.value("type").toString().toInt();
            mrec.xmin = xattrs.value("xmin").toString().toDouble();
            mrec.xmax = xattrs.value("xmax").toString().toDouble();
            mrec.ymin = xattrs.value("ymin").toString().toDouble();
            mrec.ymax = xattrs.value("ymax").toString().toDouble();
            mrec.isolutes.resize(nisols);
            mrec.csolutes.clear();
            ncsols = 0;
            //DbgLv(1) << "LM:xml:    nmrecs" << nmrecs << "kisols" << kisols;
         }

         else if (xmlname == "c_solute") {
            US_ZSolute csolute;
            csolute.x = xattrs.value("x").toString().toDouble();
            csolute.y = xattrs.value("y").toString().toDouble();
            csolute.z = xattrs.value("z").toString().toDouble();
            csolute.c = xattrs.value("c").toString().toDouble();
            csolute.x *= 1.e-13;

            mrec.csolutes << csolute;
            ncsols++;
         }
      }
   }
   DbgLv(1) << "LM:xml: End ALL: nmrecs" << nmrecs << "last ncsols" << ncsols;
   filei.close();

   if (xmli.hasError()) {
      QMessageBox::critical(this, tr("XML Invalid"), tr("File \"%1\" is not a valid XML file.").arg(fname));
      return;
   }

   else if (!is_bfmf) {
      QMessageBox::critical(
         this, tr("File Type Error"), tr("File \"%1\" is not a PcsaBestFinalModel XML file.").arg(fname));
      return;
   }

   // Test if new final model is compatible with model records
   if (bfm_incompat(fname))
      return;

   // Re-generate curve points for the model
   curve_isolutes(mrec);

   QString sctype = US_ModelRecord::ctype_text(mrec.ctype);

   // Build the model that goes along with the BFM
   bfm_model();

   // Report on loaded file
   stat_bfm(tr("A new Best Final Model has been loaded from file\n"
               "  \"%1\", of directory\n  \"%2\".\n"
               "The %3 model has %4 computed solutes\n"
               "  and an RMSD of %5")
               .arg(fname)
               .arg(fdir)
               .arg(sctype)
               .arg(ncsols)
               .arg(mrec.rmsd));


   bfm_new = true;
   mc_done = false;
   mrecs[ 0 ] = mrec;
   set_fittings(mrecs);
   pb_accept->setEnabled(true);
}

// Slot to store a best final model to disk
void US_AdvAnalysisPc::store_bfm() {
   DbgLv(1) << "store_bfm";
   // Query and get the file for storing
   QString store_file = store_dir + "/pcsa-bfm-new_bfm.xml";
   store_file = QFileDialog::getSaveFileName(
      this, tr("Specify XML File Name for Best Final Model Store"), store_dir,
      tr("Best Final Model files (*pcsa-bfm-*.xml);;"
         "Any XML files (*.xml);;Any files (*)"));

   if (store_file.isEmpty()) {
      return;
   }

   store_file = store_file.replace("\\", "/");
   QString fdir = store_file.section("/", 0, -2) + "/";
   QString fname = store_file.section("/", -1, -1);

   // Massage the name to be in "pcsa-bfm-<name>.xml" form
   if (store_file.endsWith(".")) { // Ends with ".":  no ".xml" is to be added
      store_file = store_file.left(store_file.length() - 1);
      fname = fname.left(fname.length() - 1);
   }

   else if (!store_file.endsWith(".xml")) { // If no ".xml" extension, add one
      store_file = store_file + ".xml";
      fname = fname + ".xml";
   }

   if (fname.startsWith(".")) { // Starts with ".":  no "pcsa-bfm-" prefix is to be added
      store_file = fdir + fname.mid(1);
   }

   else if (!fname.startsWith("pcsa-bfm-")) { // If no "pcsa-bfm-" prefix, add one
      store_file = fdir + "pcsa-bfm-" + fname;
   }

   // Open the specified output file
   QFile fileo(store_file);

   if (!fileo.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QMessageBox::critical(this, tr("Open Error"), tr("Cannot open file %1 .").arg(store_file));
      return;
   }

   // Write out the XML file
   ctype = mrec.ctype;
   //   int    nisols    = (int)le_crpoints->value();
   int kisols = mrec.isolutes.size();
   int ncsols = mrec.csolutes.size();
   double xmin = mrec.xmin;
   double xmax = mrec.xmax;
   double ymin = mrec.ymin;
   double ymax = mrec.ymax;
   QXmlStreamWriter xmlo;
   xmlo.setDevice(&fileo);
   xmlo.setAutoFormatting(true);
   xmlo.writeStartDocument("1.0");
   xmlo.writeComment("DOCTYPE PcsaBestFinalModel");
   xmlo.writeCharacters("\n");
   xmlo.writeStartElement("modelrecord");
   xmlo.writeAttribute("version", "1.0");
   xmlo.writeAttribute("type", QString::number(ctype));
   xmlo.writeAttribute("xmin", QString::number(xmin));
   xmlo.writeAttribute("xmax", QString::number(xmax));
   xmlo.writeAttribute("ymin", QString::number(ymin));
   xmlo.writeAttribute("ymax", QString::number(ymax));
   xmlo.writeAttribute("curve_points", QString::number(kisols));
   xmlo.writeAttribute("taskx", QString::number(mrec.taskx));
   xmlo.writeAttribute("start_y", QString::number(mrec.str_y));
   xmlo.writeAttribute("end_y", QString::number(mrec.end_y));
   xmlo.writeAttribute("par1", QString::number(mrec.par1));
   xmlo.writeAttribute("par2", QString::number(mrec.par2));
   xmlo.writeAttribute("par3", QString::number(mrec.par3));
   xmlo.writeAttribute("rmsd", QString::number(mrec.rmsd));

   for (int cc = 0; cc < ncsols; cc++) {
      xmlo.writeStartElement("c_solute");
      double sval = mrec.csolutes[ cc ].x * 1.e13;
      xmlo.writeAttribute("x", QString::number(sval));
      xmlo.writeAttribute("y", QString::number(mrec.csolutes[ cc ].y));
      xmlo.writeAttribute("z", QString::number(mrec.csolutes[ cc ].z));
      xmlo.writeAttribute("c", QString::number(mrec.csolutes[ cc ].c));
      xmlo.writeEndElement();
   }

   xmlo.writeEndElement();
   xmlo.writeEndDocument();
   fileo.close();

   // Report on the saved file
   fdir = store_file.section("/", 0, -2) + "/";
   fname = store_file.section("/", -1, -1);

   stat_bfm(
      tr("The Best Final Model has been stored in file\n"
         "  \"%1\", of directory\n  \"%2\".")
         .arg(fname)
         .arg(fdir),
      true);
}

// Slot to reset the best final model to its initial state
void US_AdvAnalysisPc::reset_bfm() {
   DbgLv(1) << "reset_bfm";
   mrec = mrec0;
   mrecs[ 0 ] = mrec0;

   stat_bfm(tr("Best Final Model has been reset to original state."));

   if (bfm0_exists) {
      set_fittings(mrecs);

      stat_bfm(
         tr("An initial best final model, with RMSD of %1,"
            "  has been restored.")
            .arg(mrec.rmsd),
         true);
   }

   else {
      stat_bfm(
         tr("The initial empty best final model\n"
            "  has been restored."),
         true);
   }

   bfm_new = false;
   mc_done = false;
}

// Slot to reset the list of model records to its initial state
void US_AdvAnalysisPc::reset_mrecs() {
   DbgLv(1) << "reset_mrecs";
   mrecs = mrecs0;
   mrec = mrec0;
   nmrecs = mrecs.size();

   stat_mrecs(tr("Model Records have been reset to original state."));

   if (mrs0_exists) {
      stat_mrecs(
         tr("An initial model records list, with %1 fits,\n"
            "  has been restored.")
            .arg(nmrecs),
         true);
   }

   else { // We are starting with models not yet computed
      stat_mrecs(
         tr("The initial empty model records list\n"
            "  has been restored."),
         true);
   }

   mrs_new = false;
   reset_bfm();
   pb_accept->setEnabled(false);
}

// Slot to build a new best final model from specified fitting controls
void US_AdvAnalysisPc::build_bfm() {
   DbgLv(1) << "build_bfm";
   //under_construct( "Build Final Model" );
   // Test and return immediately if valid mrecs still required
   if (mrecs_required("Build Final Model"))
      return;

   QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
   stat_bfm(tr("A new Best Final Model is being built ..."));

   QString sctype = cb_curvtype->currentText();
   int ctypex = cb_curvtype->currentIndex();
   ctype = (ctypex == 0) ? CTYPE_SL : CTYPE_NONE;
   ctype = (ctypex == 1) ? CTYPE_IS : ctype;
   ctype = (ctypex == 2) ? CTYPE_DS : ctype;
   ctype = (ctypex == 3) ? CTYPE_HL : ctype;
   ctype = (ctypex == 4) ? CTYPE_2O : ctype;
   double xmin = le_x_lower->text().toDouble();
   double xmax = le_x_upper->text().toDouble();
   double ymin = le_y_lower->text().toDouble();
   double ymax = le_y_upper->text().toDouble();
   mrec.ctype = ctype;
   mrec.xmin = xmin;
   mrec.xmax = xmax;
   mrec.ymin = ymin;
   mrec.ymax = ymax;

   // Set parameters for the specific curve to use
   if (ctype == CTYPE_SL) {
      mrec.str_y = le_y_strpt->text().toDouble();
      mrec.end_y = le_y_endpt->text().toDouble();
      mrec.par1 = mrec.str_y;
      mrec.par2 = (mrec.str_y - mrec.end_y) / (xmax - xmin);
   }
   else if (ctype == CTYPE_IS || ctype == CTYPE_DS) {
      mrec.str_y = ymin;
      mrec.end_y = ymax;
      mrec.par1 = le_sigmpar1->text().toDouble();
      mrec.par2 = le_sigmpar2->text().toDouble();
   }

   else if (ctype == CTYPE_HL) {
      mrec.end_y = le_y_endpt->text().toDouble();
      mrec.str_y = mrec.end_y;
      mrec.par1 = mrec.str_y;
      mrec.par2 = 0.0;
   }

   else if (ctype == CTYPE_2O) {
      mrec.str_y = ymin;
      mrec.end_y = ymax;
      mrec.par1 = le_sigmpar1->text().toDouble();
      mrec.par2 = le_sigmpar2->text().toDouble();
      if (mrecs.size() > 0)
         mrec.par3 = mrecs[ 0 ].par3;
   }

   // Generate the solute points on the curve
   curve_isolutes(mrec);

   // Compute the simulation and computed-solutes
   QList<US_SolveSim::DataSet *> dsets;
   dsets << dset0;
   US_SolveSim::Simulation sim_vals;
   sim_vals.zsolutes = mrec.isolutes;
   sim_vals.ti_noise = mrec.ti_noise;
   sim_vals.ri_noise = mrec.ri_noise;

   US_SolveSim *solvesim = new US_SolveSim(dsets, 0, false);

   solvesim->calc_residuals(0, 1, sim_vals);

   mrec.variance = sim_vals.variance;
   mrec.rmsd = sqrt(sim_vals.variance);
   mrec.csolutes = sim_vals.zsolutes;

   ncsols = mrec.csolutes.size();

   // Build the model that goes along with the BFM
   bfm_model();

   // Report on built file
   stat_bfm(
      tr("A new Best Final Model has been built\n"
         "  from the specified fitting controls.\n"
         "The %1 model has %2 computed solutes\n"
         "  and an RMSD of %3")
         .arg(sctype)
         .arg(ncsols)
         .arg(mrec.rmsd),
      true);

   stat_mrecs(
      tr("A new best final %1-solute model ( RMSD = %2 )\n"
         "  now occupies the top curve model records list spot.")
         .arg(ncsols)
         .arg(mrec.rmsd),
      true);

   mrecs[ 0 ] = mrec;
   bfm_new = true;
   mrs_new = true;
   mc_done = false;
   QApplication::restoreOverrideCursor();
   qApp->processEvents();
   pb_accept->setEnabled(true);
}

// Slot to start and process monte carlo iterations
void US_AdvAnalysisPc::start_montecarlo() {
   // Test and return immediately if valid mrecs still required
   if (mrecs_required("Start Monte Carlo"))
      return;

   DbgLv(1) << "start_montecarlo";
   wdata = dset0->run_data;
   edata = &wdata;
   mciters = le_mciters->text().toInt();
   stat_bfm(tr("%1 Monte Carlo iterations are being computed...").arg(mciters));
   ksiters = 0;
   kciters = 0;
   int nsol_m = mrec.isolutes.size();
   double par1_m = mrec.par1;
   double par2_m = mrec.par2;
   double ymin_m = mrec.ymin;
   double ymax_m = mrec.ymax;
   int nsol_p = le_crpoints->text().toInt();
   double par1_p = le_sigmpar1->text().toDouble();
   double par2_p = le_sigmpar2->text().toDouble();
   double ymin_p = le_y_lower->text().toDouble();
   double ymax_p = le_y_upper->text().toDouble();
   double difp1 = qAbs((par1_m - par1_p) / par1_m);
   double difp2 = qAbs((par2_m - par2_p) / par2_m);
   double dymin = qAbs((ymin_m - ymin_p) / ymin_m);
   double dymax = qAbs((ymax_m - ymax_p) / ymax_m);

   if (nsol_p != nsol_m || difp1 > 1.0e-4 || difp2 > 1.0e-4 || dymin > 1.0e-4 || dymin > 1.0e-4) {
      mrec.isolutes.resize(nsol_p);
      mrec.par1 = par1_p;
      mrec.par2 = par2_p;
      mrec.ymin = ymin_p;
      mrec.ymax = ymax_p;
      DbgLv(1) << " MC reset curves: p1_m p2_m p1_p p2_p ns_m ns_p" << par1_m << par2_m << par1_p << par2_p << nsol_m
               << nsol_p;
      DbgLv(1) << " MC reset curves:   dp1 dp2 sdp1 sdp2" << difp1 << difp2 << (par1_m - par1_p) << (par2_m - par2_p);
      DbgLv(1) << " MC reset curves:   dyl dyh sdyl sdyh" << dymin << dymax << (ymin_m - ymin_p) << (ymax_m - ymax_p);

      curve_isolutes(mrec);
   }

   mrecs_mc.clear();
   US_ModelRecord mrec_mc = mrec;
   US_SolveSim::DataSet dset = *dset0;
   QList<US_SolveSim::DataSet *> dsets;
   dsets << &dset;

   QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
   b_progress->reset();
   b_progress->setMaximum(mciters);

   if (nthr == 1) { // Monte Carlo with single thread
      for (int iter = 0; iter < mciters; iter++) {
         ksiters++;
         US_SolveSim::Simulation sim_vals;
         sim_vals.zsolutes = mrec.isolutes;
         dsets[ 0 ]->run_data = wdata;

         US_SolveSim *solvesim = new US_SolveSim(dsets, 0, false);

         solvesim->calc_residuals(0, 1, sim_vals);

         kciters = ksiters;
         mrec_mc.variance = sim_vals.variance;
         mrec_mc.rmsd = sqrt(sim_vals.variance);
         mrec_mc.csolutes = sim_vals.zsolutes;
         ncsols = mrec_mc.csolutes.size();
         DbgLv(1) << "  kciters" << kciters << "rmsd" << mrec_mc.rmsd << "ncsols" << ncsols;

         mrecs_mc << mrec_mc;

         // Set up new data modified by a gaussian distribution
         if (iter == 0) {
            set_gaussians(sim_vals);
         }

         // Except on last iteration, we must create new "experiment" data
         if (kciters < mciters) {
            apply_gaussians();
         }

         stat_bfm(
            tr("%1 are completed ( last:  %2-solute,  RMSD=%3 )").arg(kciters).arg(ncsols).arg(mrec_mc.rmsd), true, 1);
         b_progress->setValue(kciters);
         qApp->processEvents();
      }

      // Complete the MonteCarlo process
      b_progress->setValue(mciters);
      qApp->processEvents();

      montecarlo_done();

      QApplication::restoreOverrideCursor();
      qApp->processEvents();
   }

   else { // Monte Carlo in threads
      stat_bfm(tr("%1 Monte Carlo iterations are being"
                  " computed in %2 threads...")
                  .arg(mciters)
                  .arg(nthr));

      // Do the first iteration computation and set gaussians
      US_SolveSim::Simulation sim_vals;
      sim_vals.zsolutes = mrec.isolutes;
      dsets[ 0 ]->run_data = wdata;
      ksiters++;

      US_SolveSim *solvesim = new US_SolveSim(dsets, 0, false);

      solvesim->calc_residuals(0, 1, sim_vals);

      kciters = ksiters;
      mrec_mc.variance = sim_vals.variance;
      mrec_mc.rmsd = sqrt(sim_vals.variance);
      mrec_mc.csolutes = sim_vals.zsolutes;
      ncsols = mrec_mc.csolutes.size();
      DbgLv(1) << "  kciters" << kciters << "rmsd" << mrec_mc.rmsd << "ncsols" << ncsols << "res tskx,thrn" << 1 << 0;
      stat_bfm(tr("%1 are completed ( last:  %2-solute,  RMSD=%3 )").arg(kciters).arg(ncsols).arg(mrec_mc.rmsd), true, 1);
      b_progress->setValue(kciters);
      qApp->processEvents();

      mrecs_mc << mrec_mc;
      set_gaussians(sim_vals);

      WorkPacketPc wtbase;
      wtbase.par1 = mrec.par1;
      wtbase.par2 = mrec.par2;
      wtbase.str_y = mrec.str_y;
      wtbase.end_y = mrec.end_y;
      wtbase.isolutes = mrec.isolutes;
      wtbase.csolutes.clear();
      wtbase.noisf = 0;
      wtbase.dsets = dsets;
      wtbase.depth = 0;
      US_SolveSim::DataSet wkdset = *(dsets[ 0 ]);
      wkdsets.clear();

      for (int jt = 0; jt < nthr; jt++) { // Build up a list of datasets for each thread
         wkdsets << wkdset;
      }

      // Start the next few iterations in available threads
      for (int jt = 0; jt < nthr; jt++) {
         apply_gaussians();

         WorkPacketPc wtask = wtbase;
         US_SolveSim::Simulation sim_vals;
         sim_vals.zsolutes = mrec.isolutes;

         wtask.thrn = jt + 1;
         wtask.taskx = jt + 1;
         wtask.str_y = mrec.str_y;
         wtask.end_y = mrec.end_y;
         wtask.sim_vals = sim_vals;
         wtask.dsets[ 0 ] = &wkdsets[ jt ];

         wtask.dsets[ 0 ]->run_data = wdata;

         WorkerThreadPc *wthrd = new WorkerThreadPc(this);
         connect(wthrd, SIGNAL(work_complete(WorkerThreadPc *)), this, SLOT(process_job(WorkerThreadPc *)));

         wthrd->define_work(wtask);

         wthrd->start();

         ksiters++;
         DbgLv(1) << "    ksiters" << ksiters << "dsets[0]" << wtask.dsets[ 0 ];
      }
   }
}

// Set gaussian distribution: sigmas and iteration 1 simulation data
void US_AdvAnalysisPc::set_gaussians(US_SolveSim::Simulation &sim_vals) {
   bool gausmoo = US_Settings::debug_match("MC-GaussianSmooth");
   int nscans = edata->scanCount();
   int npoints = edata->pointCount();
   DbgLv(1) << "AA: set_gaus: gausmoo" << gausmoo;
   US_DataIO::RawData *sdata = &sim_vals.sim_data;
   US_DataIO::RawData *rdata = &sim_vals.residuals;

   sigmas.clear();

   for (int ss = 0; ss < nscans; ss++) { // Loop to accumulate the residuals from iteration 1
      QVector<double> vv(npoints);

      for (int rr = 0; rr < npoints; rr++) { // Get all residuals points from a scan
         vv[ rr ] = qAbs(rdata->value(ss, rr));
      }

      if (gausmoo) { // Do a 5-point gaussian smooth of each scan's residuals
         US_Math2::gaussian_smoothing(vv, 5);
      }

      // Append residuals to build total sigmas array
      sigmas << vv;
   }

   if (gausmoo) { // Scale the sigmas so they are at the same level as original residuals
      double rmsdi = mrecs_mc[ 0 ].rmsd;
      double rmsds = 0.0;
      int ntpoints = nscans * npoints;

      for (int rr = 0; rr < ntpoints; rr++)
         rmsds += sq(sigmas[ rr ]); // Sum of squares

      rmsds = sqrt(rmsds / ( double ) ntpoints); // Sigmas RMSD
      double sigscl = rmsdi / rmsds; // Sigma scale factor

      for (int rr = 0; rr < ntpoints; rr++)
         sigmas[ rr ] *= sigscl; // Scaled sigmas
      DbgLv(1) << "AA: gausmoo: rmsd-i rmsc-s sigscl" << rmsdi << rmsds << sigscl;
   }

   // Save the simulation data set from iteration 1
   sdata1 = *sdata;
}

// Apply gaussians: add in random variations of sigmas to base simulation
void US_AdvAnalysisPc::apply_gaussians() {
   int nscans = edata->scanCount();
   int npoints = edata->pointCount();
   int kk = 0;

   // Add box-muller portion of each sigma to the base simulation
   for (int ss = 0; ss < nscans; ss++) {
      for (int rr = 0; rr < npoints; rr++) {
         double svari = US_Math2::box_muller(0.0, sigmas[ kk++ ]);
         wdata.setValue(ss, rr, (sdata1.value(ss, rr) + svari));
      }
   }
}

// Process the completion of an MC worker thread
void US_AdvAnalysisPc::process_job(WorkerThreadPc *wthr) {
   kciters++;
   WorkPacketPc wresult;
   wthr->get_result(wresult);

   US_ModelRecord mrec_mc = mrecs_mc[ 0 ];
   mrec_mc.variance = wresult.sim_vals.variance;
   mrec_mc.rmsd = sqrt(mrec_mc.variance);
   mrec_mc.csolutes = wresult.sim_vals.zsolutes;
   ncsols = mrec_mc.csolutes.size();
   DbgLv(1) << "  kciters" << kciters << "rmsd" << mrec_mc.rmsd << "ncsols" << ncsols << "res tskx,thrn"
            << wresult.taskx << wresult.thrn;

   stat_bfm(tr("%1 are completed ( last:  %2-solute,  RMSD=%3 )").arg(kciters).arg(ncsols).arg(mrec_mc.rmsd), true, 1);
   b_progress->setValue(kciters);
   qApp->processEvents();

   mrecs_mc << mrec_mc;

   if (kciters == mciters) { // Complete the MonteCarlo process
      b_progress->setValue(mciters);

      montecarlo_done();

      QApplication::restoreOverrideCursor();
      qApp->processEvents();
   }

   else if (ksiters < mciters) { // Submit a new task
      ksiters++;
      DbgLv(1) << "    ksiters" << ksiters << "   apply_gaussians";

      apply_gaussians();

      WorkPacketPc wtask = wresult;
      wtask.dsets[ 0 ] = &wkdsets[ wresult.thrn - 1 ];
      wtask.dsets[ 0 ]->run_data = wdata;
      wtask.taskx = ksiters;
      DbgLv(1) << "    ksiters" << ksiters << "     wt tskx,thrn" << wtask.taskx << wtask.thrn;

      delete wthr;

      WorkerThreadPc *wthrd = new WorkerThreadPc(this);
      connect(wthrd, SIGNAL(work_complete(WorkerThreadPc *)), this, SLOT(process_job(WorkerThreadPc *)));

      wthrd->define_work(wtask);
      wthrd->start();
      DbgLv(1) << "    ksiters" << ksiters << "      dsets[0]" << wtask.dsets[ 0 ];
   }
}

// Complete model records and the final model after monte carlo completion
void US_AdvAnalysisPc::montecarlo_done(void) {
   DbgLv(1) << "==montecarlo_done()==";
   stat_bfm(tr("Building MC models and final composite..."), true);
#if 0
   int     nccsol   = 0;
   QVector< US_ZSolute > compsols;
   QStringList sortlst;
#endif
#if 1
   QStringList mfnames;
   edata = &dset0->run_data;
   int ctype = mrecs_mc[ 0 ].ctype;
   int stype = mrecs_mc[ 0 ].stype;
   QString tmppath = US_Settings::tmpDir() + "/";
   QString runID = edata->runID;
   QString tripID = edata->cell + edata->channel + edata->wavelength;
   QString dates = "e" + edata->editID + "_a" + QDateTime::currentDateTime().toUTC().toString("yyMMddhhmm");
   QString analysType = "PCSA-SL-MC";
   analysType = (ctype == CTYPE_IS) ? "PCSA-IS-MC" : analysType;
   analysType = (ctype == CTYPE_DS) ? "PCSA-DS-MC" : analysType;
   analysType = (ctype == CTYPE_HL) ? "PCSA-HL-MC" : analysType;
   analysType = (ctype == CTYPE_2O) ? "PCSA-2O-MC" : analysType;
   QString analysID = dates + "_" + analysType + "_local_";
   QString base_mdesc = runID + "." + tripID + "." + analysID;
   model.editGUID = edata->editGUID;
   model.monteCarlo = true;
#endif
   US_Model::SimulationComponent zcomponent;
   zcomponent.vbar20 = dset0->vbar20;

   // Build individual models and append all solutes to one composite
   for (int jmc = 0; jmc < mciters; jmc++) {
      mrec = mrecs_mc[ jmc ];
      QVector<US_ZSolute> csolutes = mrec.csolutes;
      int nsols = csolutes.size();
      model.components.resize(nsols);

      for (int cc = 0; cc < nsols; cc++) {
         // Get component values and sorting string
#if 0
         double  sol_x    = csolutes[ cc ].x;
         double  sol_y    = csolutes[ cc ].y;
         double  sol_z    = csolutes[ cc ].z;
         double  sol_c    = csolutes[ cc ].c;
         QString sol_id   = QString().sprintf( "%.4f:%.4f:%d",
            sol_x * 1.e13, sol_y, nccsol++ );
DbgLv(1) << "MCD: cc" << cc << "sol_id" << sol_id;

         // Save unsorted/summed solute and sorting string
         US_ZSolute compsol;
         compsol.x        = sol_x;
         compsol.y        = sol_y;
         compsol.z        = sol_z;
         compsol.c        = sol_c;
         compsols << compsol;
         sortlst  << sol_id;

         // Build the model component
         model.components[ cc ]                       = zcomponent;
         model.components[ cc ].s                     = sol_x;
         model.components[ cc ].f_f0                  = sol_y;
         model.components[ cc ].vbar20                = sol_z;
         model.components[ cc ].signal_concentration  = sol_c;
         model.components[ cc ].name = QString().sprintf( "SC%04d", cc + 1 );
#endif
#if 1
         model.components[ cc ] = zcomponent;
         US_ZSolute::set_mcomp_values(model.components[ cc ], csolutes[ cc ], stype, true);
#endif
         model.calc_coefficients(model.components[ cc ]);
      }

      // Save the individual MC model
      mrec.model = model;
      mrecs_mc[ jmc ] = mrec;

      // Write the iteration model to a temp file and save its name
      QString iterID = QString().sprintf("mc%04d", jmc + 1);
      model.description = base_mdesc + iterID + ".model";
      QString mfname = tmppath + model.description + ".tmp";
      mfnames << mfname;
      model.write(mfname);
   }

   // Now sort the solute id strings to create sorted composite
#if 0
   qSort( sortlst );
   US_ZSolute pcompsol;
   US_ZSolute ccompsol;
   QString pskmatch  = QString();
   mrec.csolutes.clear();
   double  cnorm     = 1.0 / (double)mciters;
   ncsols            = 0;

   for ( int cc = 0; cc < nccsol; cc++ )
   {
      QString sol_id    = sortlst[ cc ];
      QString skmatch   = sol_id.section( ":", 0, 1 );
      int     ccin      = sol_id.section( ":", 2, 2 ).toInt();
      ccompsol          = compsols[ ccin ];

      if ( skmatch != pskmatch )
      {  // New s,k combination:  output previous component, start new sum
         if ( cc > 0 )
         {
            pcompsol.c    *= cnorm;
            mrec.csolutes << pcompsol;
            ncsols++;
         }

         pcompsol       = ccompsol;
         pskmatch       = skmatch;
DbgLv(1) << "MCD: cc ccin ncsols" << cc << ccin << ncsols;
      }

      else
      {  // Identical s,k to previous:  sum concentration;
         pcompsol.c    += ccompsol.c;
      }
   }

   // Output last component
   pcompsol.c    *= cnorm;
   mrec.csolutes << pcompsol;
   ncsols++;
   US_Model modela;

   model          = mrec.model;           // Model to pass back to main
   model.components.resize( ncsols );
   modela         = model;                // Model for application (corrected)
   double sfactor = 1.0 / dset0->s20w_correction;
   double dfactor = 1.0 / dset0->D20w_correction;

   // Build the model that goes along with the new composite model record
   for ( int cc = 0; cc < ncsols; cc++ )
   {
      model.components[ cc ]                       = zcomponent;
      model.components[ cc ].s                     = mrec.csolutes[ cc ].x;
      model.components[ cc ].f_f0                  = mrec.csolutes[ cc ].y;
      model.components[ cc ].vbar20                = mrec.csolutes[ cc ].z;
      model.components[ cc ].signal_concentration  = mrec.csolutes[ cc ].c;
      model.components[ cc ].name = QString().sprintf( "SC%04d", cc + 1 );
      model.calc_coefficients( model.components[ cc ] );

      modela.components[ cc ]                      = model.components[ cc ];
      modela.components[ cc ].s                   *= sfactor;
      modela.components[ cc ].D                   *= dfactor;
   }
#endif
#if 1
   // Create composite MC file and load the model
   QString cmfname = US_Model::composite_mc_file(mfnames, true);
   model.load(cmfname);
   US_Model modela = model; // Model for application (corrected)
   ncsols = model.components.size();
   double sfactor = 1.0 / dset0->s20w_correction;
   double dfactor = 1.0 / dset0->D20w_correction;

   // Build the apply-model that goes along with the new composite model record
   for (int cc = 0; cc < ncsols; cc++) {
      modela.components[ cc ].s *= sfactor;
      modela.components[ cc ].D *= dfactor;
   }
#endif

   // Do a fit with the composite model and get the RMSD
   US_DataIO::RawData *sdata = &mrec.sim_data;
   US_AstfemMath::initSimData(*sdata, *edata, 0.0);

   US_Astfem_RSA astfem_rsa(modela, dset0->simparams);
   astfem_rsa.calculate(*sdata);

   int nscans = edata->scanCount();
   int npoints = edata->pointCount();
   double varia = 0.0;
   bool have_ti = (mrec.ti_noise.size() > 0);
   bool have_ri = (mrec.ri_noise.size() > 0);
   double tinoi = 0.0;
   double rinoi = 0.0;

   for (int ss = 0; ss < nscans; ss++) {
      rinoi = have_ri ? mrec.ri_noise[ ss ] : 0.0;

      for (int rr = 0; rr < npoints; rr++) {
         tinoi = have_ti ? mrec.ti_noise[ rr ] : 0.0;
         varia += sq((edata->value(ss, rr) - sdata->value(ss, rr) - rinoi - tinoi));
      }
   }

   varia /= ( double ) (nscans * npoints);
   mrec.variance = varia;
   mrec.rmsd = sqrt(varia);
   mrec.model = model;
   mrecs[ 0 ] = mrec;

   // Report MC completion status
   stat_bfm(tr("MC models and final %1-solute composite model are built.").arg(ncsols), true);

   stat_mrecs(
      tr("A new best final %1-solute model ( RMSD = %2 )\n"
         "  now occupies the top curve model records list spot.")
         .arg(ncsols)
         .arg(mrec.rmsd),
      true);

   bfm_new = true;
   mrs_new = true;
   mc_done = true;
   pb_accept->setEnabled(true);
}

// Pop up an under-construction message dialog
void US_AdvAnalysisPc::under_construct(QString proc) {
   QMessageBox::information(
      this, tr("UNDER CONSTRUCTION"), tr("Implementation of <b>%1</b> is not yet complete.").arg(proc));
}

// Re-generate the input solute curve points for a model record
void US_AdvAnalysisPc::curve_isolutes(US_ModelRecord &mrec) {
   int nisols = mrec.isolutes.size();
   int ctype = mrec.ctype;
   int stype = mrec.stype;
   int attr_x = (stype >> 6) & 7;
   int attr_y = (stype >> 3) & 7;
   double xscl = (attr_x == US_ZSolute::ATTR_S) ? 1.0e-13 : 1.0;
   double yscl = (attr_y == US_ZSolute::ATTR_S) ? 1.0e-13 : 1.0;
   double xmin = mrec.xmin;
   double xmax = mrec.xmax;
   double ymin = mrec.ymin;
   double ymax = mrec.ymax;
   double prng = ( double ) (nisols - 1);
   double xrng = xmax - xmin;
   double xinc = xrng / prng;
   double par1 = mrec.par1;
   double par2 = mrec.par2;
   double xoinc = 1.0 / prng;
   double xoff = 0.0;
   DbgLv(1) << "AA:CP: xinc" << xinc << "ctype" << ctype;

   if (ctype == CTYPE_IS) // Increasing Sigmoid
   {
      double ydif = ymax - ymin;
      double p1rt = sqrt(2.0 * par1);

      for (int kk = 0; kk < nisols; kk++) {
         double xval = xmin + xoff * xrng;
         double efac = 0.5 * erf((xoff - par2) / p1rt) + 0.5;
         double yval = ymin + ydif * efac;
         mrec.isolutes[ kk ].x = xval * xscl;
         mrec.isolutes[ kk ].y = yval * yscl;
         xoff += xoinc;
      }
   }

   else if (ctype == CTYPE_SL) // Straight Line
   {
      double yval = mrec.str_y;
      double yinc = (mrec.end_y - mrec.str_y) / prng;
      double xval = xmin;

      for (int kk = 0; kk < nisols; kk++) {
         mrec.isolutes[ kk ].x = xval * xscl;
         mrec.isolutes[ kk ].y = yval * yscl;
         xval += xinc;
         yval += yinc;
      }
      DbgLv(1) << "AA:CP:  ni" << nisols << "last yv" << yval;
   }

   else if (ctype == CTYPE_DS) // Decreasing Sigmoid
   {
      double ydif = ymin - ymax;
      double p1rt = sqrt(2.0 * par1);

      for (int kk = 0; kk < nisols; kk++) {
         double xval = xmin + xoff * xrng;
         double efac = 0.5 * erf((xoff - par2) / p1rt) + 0.5;
         double yval = ymax + ydif * efac;
         mrec.isolutes[ kk ].x = xval * xscl;
         mrec.isolutes[ kk ].y = yval * yscl;
         xoff += xoinc;
      }
   }

   else if (ctype == CTYPE_HL) // Horizontal Line
   {
      double yval = mrec.end_y;
      double xval = xmin;

      for (int kk = 0; kk < nisols; kk++) {
         mrec.isolutes[ kk ].x = xval * xscl;
         mrec.isolutes[ kk ].y = yval * yscl;
         xval += xinc;
      }
      DbgLv(1) << "AA:CP:  ni" << nisols << "last yv" << yval;
   }

   else if (ctype == CTYPE_2O) // 2nd-Order Power Law
   {
      double xval = xmin;
      double aval = par1;
      double cval = par2;
      // y = a * x^b +c
      // x^b = ( y - c ) / a
      // x   = logb( ( y - c ) / a )
      // x   = log10( ( y - c ) / a ) / log10( b )
      // log10( b ) = log10( ( y - c ) / a ) / x
      // b   = [ ( y - c ) / a ] - log10( x )
      //
      //      double bval    = ( ( mrec.end_y - cval ) / qMax( 0.00001, aval ) )
      //                       - log10( xmin );
      double bval = ((mrec.str_y - cval) / qMax(0.00001, aval)) - log10(xmin);

      for (int kk = 0; kk < nisols; kk++) {
         double yval = aval * pow(xval, bval) + cval;
         mrec.isolutes[ kk ].x = xval * xscl;
         mrec.isolutes[ kk ].y = yval * yscl;
         xval += xinc;
      }
   }
   DbgLv(1) << "AA:CP: sol0 x,y" << mrec.isolutes[ 0 ].x << mrec.isolutes[ 0 ].y;
   int nn = nisols - 1;
   DbgLv(1) << "AA:CP: soln x,y" << mrec.isolutes[ nn ].x << mrec.isolutes[ nn ].y;
}

// Generate the model that goes with the BFM record
void US_AdvAnalysisPc::bfm_model(void) {
   US_Model modela;
   US_Model::SimulationComponent zcomponent;
   zcomponent.vbar20 = dset0->vbar20;
   nisols = mrec.isolutes.size();
   ncsols = mrec.csolutes.size();

   model = mrec.model; // Model to pass back to main
   model.components.resize(ncsols);
   modela = model; // Model for application (corrected)
   double sfactor = 1.0 / dset0->s20w_correction;
   double dfactor = 1.0 / dset0->D20w_correction;
   double rmsdsv = mrec.rmsd;

   // Build the model that goes along with the new composite model record
   for (int cc = 0; cc < ncsols; cc++) {
      model.components[ cc ] = zcomponent;
      model.components[ cc ].s = mrec.csolutes[ cc ].x;
      model.components[ cc ].f_f0 = mrec.csolutes[ cc ].y;
      model.components[ cc ].signal_concentration = mrec.csolutes[ cc ].c;
      model.components[ cc ].name = QString().sprintf("SC%04d", cc + 1);

      model.calc_coefficients(model.components[ cc ]);

      modela.components[ cc ] = model.components[ cc ];
      modela.components[ cc ].s *= sfactor;
      modela.components[ cc ].D *= dfactor;
   }

   // Do a fit with the composite model and get the RMSD
   edata = &dset0->run_data;
   US_DataIO::RawData *sdata = &mrec.sim_data;
   US_DataIO::RawData *rdata = &mrec.residuals;
   US_AstfemMath::initSimData(*sdata, *edata, 0.0);
   US_AstfemMath::initSimData(*rdata, *edata, 0.0);

   US_Astfem_RSA astfem_rsa(modela, dset0->simparams);
   astfem_rsa.calculate(*sdata);

   int nscans = edata->scanCount();
   int npoints = edata->pointCount();
   double varia = 0.0;

   for (int ss = 0; ss < nscans; ss++) {
      for (int rr = 0; rr < npoints; rr++) {
         double rval = edata->value(ss, rr) - sdata->value(ss, rr);
         varia += sq(rval);
         rdata->setValue(ss, rr, rval);
      }
   }

   varia /= ( double ) (nscans * npoints);
   double rmsd = sqrt(varia);

   if (rmsd > rmsdsv && rmsd > mrecs[ 3 ].rmsd) { // Computed rmsd too high:  must have skipped noise, so ignore
      mrecs[ 0 ].sim_data = mrec.sim_data;
      mrecs[ 0 ].residuals = mrec.residuals;
      mrec = mrecs[ 0 ];
      model = mrec.model;

      if (!mrec.modelGUID.isEmpty()) { // If possible, load the model referred to in best model record
         US_Passwd pw;
         bool loadDB = dset0->requestID.contains("DB");
         US_DB2 *dbP = loadDB ? new US_DB2(pw.getPasswd()) : NULL;

         model.load(loadDB, mrec.modelGUID, dbP);

         mrec.model = model;
         mrecs[ 0 ] = mrec;
      }
      return;
   }

   mrec.variance = varia;
   mrec.rmsd = sqrt(varia);
   mrec.model = model;
   mrecs[ 0 ] = mrec;
}

// Display status message for model records
void US_AdvAnalysisPc::stat_mrecs(const QString msg, bool append, int line) {
   show_stat(te_mrecstat, msg, append, line);
}

// Display status message for best final model
void US_AdvAnalysisPc::stat_bfm(const QString msg, bool append, int line) {
   show_stat(te_bfmstat, msg, append, line);
}

// Display status message to a text edit with append and line options
void US_AdvAnalysisPc::show_stat(QTextEdit *tedit, const QString msg, bool append, int aft_line) {
   if (append) { // Message gets appended
      QString sttext = tedit->toPlainText();

      if (aft_line > 0) { // Append after the specified line
         QStringList stlines = sttext.split("\n"); // Get the lines
         sttext.clear();

         for (int ii = 0; ii < qMin(aft_line, stlines.size()); ii++) { // Rebuild lines before the specified one
            sttext += stlines[ ii ] + "\n";
         }
      }

      else { // Append to full current text
         sttext += "\n";
      }

      tedit->setText(sttext + msg);
   }

   else { // Message fully replaces content
      tedit->setText(msg);
   }
}

// Set the fitting control counters from model records
void US_AdvAnalysisPc::set_fittings(QVector<US_ModelRecord> &s_mrecs) {
   US_ModelRecord s_mrec = s_mrecs[ 0 ];
   nisols = s_mrec.isolutes.size();
   nmrecs = s_mrecs.size();
   int ctype = s_mrec.ctype;
#if 0
   int    stype = s_mrec.stype;
   int    attr_x  = ( stype >> 6 ) & 7;
   int    attr_y  = ( stype >> 3 ) & 7;
   double xscl    = ( attr_x == US_ZSolute::ATTR_S ) ? 1.0e13 : 1.0;
   double yscl    = ( attr_y == US_ZSolute::ATTR_S ) ? 1.0e13 : 1.0;
#endif
   double xmin = s_mrec.xmin;
   double xmax = s_mrec.xmax;
   double ymin = s_mrec.ymin;
   double ymax = s_mrec.ymax;
   ctype = s_mrec.ctype;
   DbgLv(1) << "AA:SF: ctype x,y min,max" << ctype << xmin << xmax << ymin << ymax;
   le_y_strpt->setText(QString::number(s_mrec.str_y));
   le_y_endpt->setText(QString::number(s_mrec.end_y));
   le_sigmpar1->setText(QString::number(s_mrec.par1));
   le_sigmpar2->setText(QString::number(s_mrec.par2));

#if 0
   for ( int ii = 0; ii < nmrecs; ii++ )
   {
      s_mrec       = s_mrecs[ ii ];
      ymin         = qMin( ymin, s_mrec.str_y );
      ymax         = qMax( ymax, s_mrec.end_y );
      nisols       = s_mrec.isolutes.size();

      for ( int jj = 0; jj < nisols; jj++ )
      {
         double xval  = s_mrec.isolutes[ jj ].x * xscl;
         double yval  = s_mrec.isolutes[ jj ].y * yscl;
         xmin         = qMin( xmin, xval );
         xmax         = qMax( xmax, xval );
         ymin         = qMin( ymin, yval );
         ymax         = qMax( ymax, yval );
if(xval==0.0 || yval==0.0)
DbgLv(1) << "AA:SF:   ii jj ni" << ii << jj << nisols << "x y" << xval << yval;
      }
   }
#endif

   int ctypex = 0;
   ctypex = (ctype == CTYPE_SL) ? 0 : ctypex;
   ctypex = (ctype == CTYPE_IS) ? 1 : ctypex;
   ctypex = (ctype == CTYPE_DS) ? 2 : ctypex;
   ctypex = (ctype == CTYPE_HL) ? 3 : ctypex;
   ctypex = (ctype == CTYPE_2O) ? 4 : ctypex;
   cb_curvtype->setCurrentIndex(ctypex);
   le_x_lower->setText(QString::number(xmin));
   le_x_upper->setText(QString::number(xmax));
   le_y_lower->setText(QString::number(ymin));
   le_y_upper->setText(QString::number(ymax));
   le_crpoints->setText(QString::number(nisols));
   DbgLv(1) << "AA:SF: (2) x,y min,max" << ctype << xmin << xmax << ymin << ymax;
}

// Return a flag and possibly warn if operation requires valid mrecs
bool US_AdvAnalysisPc::mrecs_required(QString oper) {
   // Test the mrecs list for validity
   bool needMrs = (mrecs.size() < 2 || mrecs[ 1 ].csolutes.size() < 1);

   // Output a warning dialog if not
   if (needMrs) {
      QMessageBox::critical(
         this, tr("Invalid Operation"),
         tr("<b>%1</b> is not a valid operation<br/>"
            "&nbsp;&nbsp;when no Model Records have been computed.<br/>"
            "Your options at this point are:"
            "<ul><li>Go back to the main analysis controls window and do"
            " a fit with <b>Start Fit</b>, then return here to retry; or"
            "</li><li>Do a <b>Load Model Records</b> here, and then\n"
            "  retry this operation.</li></ul>")
            .arg(oper));
   }

   return needMrs;
}

// Return a flag and possibly warn if operation incompatible
bool US_AdvAnalysisPc::bfm_incompat(QString fname) {
   // Test the compatibility of a new BFM with existing mrecs
   bool inCompat = false;

   int ftype = mrec.ctype;
   double fxmin = mrec.xmin;
   double fxmax = mrec.xmax;
   double fymin = mrec.ymin;
   double fymax = mrec.ymax;
   int rtype = mrecs[ 1 ].ctype;
   double rxmin = mrecs[ 1 ].xmin;
   double rxmax = mrecs[ 1 ].xmax;
   double rymin = mrecs[ 1 ].ymin;
   double rymax = mrecs[ 1 ].ymax;
   DbgLv(1) << "AA:BI: ftype rtype" << ftype << rtype << "fname" << fname;

   inCompat = (inCompat || (ftype != rtype));
   DbgLv(1) << "AA:BI:  (1)inCompat" << inCompat;
   inCompat = (inCompat || (fxmin > rxmax));
   inCompat = (inCompat || (fxmax < rxmin));
   inCompat = (inCompat || (fymin > rymax));
   inCompat = (inCompat || (fymax < rymin));
   DbgLv(1) << "AA:BI:  (5)inCompat" << inCompat;

   if (inCompat) {
      const char *ctps[] = { "Straight Line", "Increasing Sigmoid", "Decreasing Sigmoid", "Horizontal Line" };
      int ftx = ftype;
      ftx = (ftype == CTYPE_SL) ? 0 : ftx;
      ftx = (ftype == CTYPE_IS) ? 1 : ftx;
      ftx = (ftype == CTYPE_DS) ? 2 : ftx;
      ftx = (ftype == CTYPE_HL) ? 3 : ftx;
      int rtx = rtype;
      rtx = (rtype == CTYPE_SL) ? 0 : rtx;
      rtx = (rtype == CTYPE_IS) ? 1 : rtx;
      rtx = (rtype == CTYPE_DS) ? 2 : rtx;
      rtx = (rtype == CTYPE_HL) ? 3 : rtx;
      QString fpars = QString(ctps[ ftx ])
                      + tr(" ; s %1 to %2 ; f/f0 %3 to %4").arg(fxmin).arg(fxmax).arg(fymin).arg(fymax);
      QString rpars = QString(ctps[ rtx ])
                      + tr(" ; s %1 to %2 ; f/f0 %3 to %4").arg(rxmin).arg(rxmax).arg(rymin).arg(rymax);

      QMessageBox::critical(
         this, tr("Incompatible Final Model"),
         tr("File <b>%1</b> has fitting controls incompatible<br/>"
            "&nbsp;&nbsp;with the existing model records."
            "<ul><li>File:&nbsp;&nbsp;%2.</li>"
            "<li>Records:&nbsp;&nbsp;%3.</li></ul>")
            .arg(fname)
            .arg(fpars)
            .arg(rpars));

      mrec = mrecs[ 0 ];
   }

   return inCompat;
}

// Test procedures for handling pcsa_modelrecs table
void US_AdvAnalysisPc::test_db_mrecs() {
   US_Passwd pw;
   US_DB2 *dbP = new US_DB2(pw.getPasswd());
   int mrID1 = -1;
   QString invID = QString::number(US_Settings::us_inv_ID());
   QStringList qry;

   qry.clear();
   qry << "count_mrecs" << invID;
   qDebug() << "QRY:" << qry;
   int nmrec = dbP->functionQuery(qry);
   qDebug() << " nmrec" << nmrec;
   qDebug() << "  nm stat" << dbP->lastErrno() << dbP->lastError();

   qry.clear();
   qry << "get_mrecs_desc" << invID;
   qDebug() << "QRY:" << qry;
   dbP->query(qry);
   if (dbP->lastErrno() == US_DB2::OK) {
      qDebug() << " numRows" << dbP->numRows();
      qDebug() << "  nR stat" << dbP->lastErrno() << dbP->lastError();
      while (dbP->next()) {
         int mrID = dbP->value(0).toInt();
         mrID1 = (mrID1 < 0) ? mrID : mrID1;
         QString mrGUID = dbP->value(1).toString();
         int edID = dbP->value(2).toInt();
         QString edGUID = dbP->value(3).toString();
         int moID = dbP->value(4).toInt();
         QString moGUID = dbP->value(5).toString();
         QString descr = dbP->value(6).toString();
         QString cksm = dbP->value(7).toString();
         QString size = dbP->value(8).toString();
         QString utime = dbP->value(9).toString();
         qDebug() << "  mrID mrGUID" << mrID << mrGUID;
         qDebug() << "   edID edGUID" << edID << edGUID;
         qDebug() << "   moID moGUID" << moID << moGUID;
         qDebug() << "   desc" << descr;
         qDebug() << "   cksm size" << cksm << size;
         qDebug() << "   utime" << utime;
      }
   }

   qry.clear();
   qry << "get_mrecs_desc" << "0";
   qDebug() << "QRY:" << qry;
   dbP->query(qry);
   if (dbP->lastErrno() == US_DB2::OK) {
      qDebug() << " numRows" << dbP->numRows();
      qDebug() << "  nR stat" << dbP->lastErrno() << dbP->lastError();
      while (dbP->next()) {
         int mrID = dbP->value(0).toInt();
         QString mrGUID = dbP->value(1).toString();
         int edID = dbP->value(2).toInt();
         QString edGUID = dbP->value(3).toString();
         int moID = dbP->value(4).toInt();
         QString moGUID = dbP->value(5).toString();
         QString descr = dbP->value(6).toString();
         QString cksm = dbP->value(7).toString();
         QString size = dbP->value(8).toString();
         QString utime = dbP->value(9).toString();
         qDebug() << "  mrID mrGUID" << mrID << mrGUID;
         qDebug() << "   edID edGUID" << edID << edGUID;
         qDebug() << "   moID moGUID" << moID << moGUID;
         qDebug() << "   desc" << descr;
         qDebug() << "   cksm size" << cksm << size;
         qDebug() << "   utime" << utime;
      }
   }

   else {
      qDebug() << " *ERROR*" << dbP->lastErrno() << dbP->lastError();
   }

   qry.clear();
   QString edGUID = dset0->run_data.editGUID;
   qry << "get_editID" << edGUID;
   qDebug() << "QRY:" << qry;
   dbP->query(qry);
   QString editID = dbP->value(0).toString();
   qDebug() << " editID edGUID" << editID << edGUID;

   qry.clear();
   qry << "count_mrecs_by_editID" << invID << editID;
   qDebug() << "QRY:" << qry;
   int nmredt = dbP->functionQuery(qry);
   qDebug() << " nmredt" << nmredt;
   qDebug() << "  nm stat" << dbP->lastErrno() << dbP->lastError();

   qry.clear();
   qry << "get_mrecs_desc_by_editID" << invID << editID;
   qDebug() << "QRY:" << qry;
   dbP->query(qry);
   if (dbP->lastErrno() == US_DB2::OK) {
      qDebug() << " numRows" << dbP->numRows();
      qDebug() << "  nR stat" << dbP->lastErrno() << dbP->lastError();
      while (dbP->next()) {
         int mrID = dbP->value(0).toInt();
         QString mrGUID = dbP->value(1).toString();
         int edID = dbP->value(2).toInt();
         QString edGUID = dbP->value(3).toString();
         int moID = dbP->value(4).toInt();
         QString moGUID = dbP->value(5).toString();
         QString descr = dbP->value(6).toString();
         QString cksm = dbP->value(7).toString();
         QString size = dbP->value(8).toString();
         QString utime = dbP->value(9).toString();
         qDebug() << "  mrID mrGUID" << mrID << mrGUID;
         qDebug() << "   edID edGUID" << edID << edGUID;
         qDebug() << "   moID moGUID" << moID << moGUID;
         qDebug() << "   desc" << descr;
         qDebug() << "   cksm size" << cksm << size;
         qDebug() << "   utime" << utime;
      }
   }

   else {
      qDebug() << " *ERROR*" << dbP->lastErrno() << dbP->lastError();
   }

   qry.clear();
   qry << "get_mrecs_info" << QString::number(mrID1);
   qDebug() << "QRY:" << qry;
   dbP->query(qry);
   if (dbP->lastErrno() == US_DB2::OK) {
      qDebug() << " numRows" << dbP->numRows();
      qDebug() << "  nR stat" << dbP->lastErrno() << dbP->lastError();
      dbP->next();
      QString mrGUID = dbP->value(0).toString();
      QString xmlstr = dbP->value(1).toString();
      int edID = dbP->value(2).toInt();
      QString edGUID = dbP->value(3).toString();
      int moID = dbP->value(4).toInt();
      QString moGUID = dbP->value(5).toString();
      QString descr = dbP->value(6).toString();
      QString cksm = dbP->value(7).toString();
      int xsize = dbP->value(8).toInt();
      QString utime = dbP->value(9).toString();
      qDebug() << "  mrID mrGUID" << mrID1 << mrGUID;
      qDebug() << "   edID edGUID" << edID << edGUID;
      qDebug() << "   moID moGUID" << moID << moGUID;
      qDebug() << "   desc" << descr;
      qDebug() << "   cksm size" << cksm << xsize << "xml len" << xmlstr.length();
      QString xmls1 = QString(xmlstr).left(1600);
      QString xmls2 = QString(xmlstr).right(1600);
      qDebug() << "==== XML1" << xmls1 << "XML1 ======";
      qDebug() << "==== XML2" << xmls2 << "XML2 ======";
   }

   else {
      qDebug() << " *ERROR*" << dbP->lastErrno() << dbP->lastError();
   }
}
