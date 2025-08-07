//! \file us_dmga_init.cpp

#include "us_dmga_init.h"

#include <QApplication>

#include "us_associations_gui.h"
#include "us_constants.h"
#include "us_constraints_edit.h"
#include "us_gui_settings.h"
#include "us_investigator.h"
#include "us_license.h"
#include "us_license_t.h"
#include "us_math2.h"
#include "us_matrix.h"
#include "us_model.h"
#include "us_model_loader.h"
#include "us_passwd.h"
#include "us_report.h"
#include "us_settings.h"
#include "us_util.h"

#ifndef DbgLv
#define DbgLv(a) \
  if (dbg_level >= a) qDebug()
#endif

// main program
int main(int argc, char* argv[]) {
  QApplication application(argc, argv);

#include "main1.inc"

  // License is OK.  Start up.

  US_DMGA_Init w;
  w.show();                   //!< \memberof QWidget
  return application.exec();  //!< \memberof QApplication
}

// US_DMGA_Init class constructor
US_DMGA_Init::US_DMGA_Init() : US_Widgets() {
  // set up the GUI

  setWindowTitle(tr("Discrete Genetic Algorithm Initialization"));
  setPalette(US_GuiSettings::frameColor());

  dbg_level = US_Settings::us_debug();

  // Grid
  QGridLayout* main = new QGridLayout(this);
  main->setSpacing(2);
  main->setContentsMargins(2, 2, 2, 2);
  int dataloc = US_Settings::default_data_location();

  // Disk/DB
  dkdb_cntrls = new US_Disk_DB_Controls(dataloc);
  // Model and constraints
  QLabel* lb_modcnst = us_banner(tr("Model and Constraints"));
  pb_loadmodel = us_pushbutton(tr("Load Base Model"));
  pb_loadconstr = us_pushbutton(tr("Load Constraints"));
  pb_defmodel = us_pushbutton(tr("Define Base Model"));
  pb_defconstr = us_pushbutton(tr("Define Constraints"));
  pb_savemodel = us_pushbutton(tr("Save Base Model"));
  pb_saveconstr = us_pushbutton(tr("Save Constraints"));
  pb_savemodel->setEnabled(false);
  pb_saveconstr->setEnabled(false);

  // General control
  QLabel* lb_gencntrl = us_banner(tr("General Control"));
  pb_help = us_pushbutton(tr("Help"));
  pb_close = us_pushbutton(tr("Close"));
  le_status = us_lineedit(tr("(no model/constraints loaded)"), 1, true);
  QPalette stpal;
  stpal.setColor(QPalette::Text, Qt::white);
  stpal.setColor(QPalette::Base, Qt::blue);
  le_status->setPalette(stpal);

  // Layout
  int row = 0;
  main->addLayout(dkdb_cntrls, row++, 0, 1, 4);
  main->addWidget(lb_modcnst, row++, 0, 1, 4);
  main->addWidget(pb_loadmodel, row, 0, 1, 2);
  main->addWidget(pb_loadconstr, row++, 2, 1, 2);
  main->addWidget(pb_defmodel, row, 0, 1, 2);
  main->addWidget(pb_defconstr, row++, 2, 1, 2);
  main->addWidget(pb_savemodel, row, 0, 1, 2);
  main->addWidget(pb_saveconstr, row++, 2, 1, 2);

  main->addWidget(lb_gencntrl, row++, 0, 1, 4);
  main->addWidget(le_status, row++, 0, 1, 4);
  main->addWidget(pb_help, row, 0, 1, 2);
  main->addWidget(pb_close, row++, 2, 1, 2);

  // Signals and slots
  connect(dkdb_cntrls, SIGNAL(changed(bool)), SLOT(update_disk_db(bool)));
  connect(pb_loadmodel, SIGNAL(clicked()), SLOT(load_model()));
  connect(pb_loadconstr, SIGNAL(clicked()), SLOT(load_constraints()));
  connect(pb_defmodel, SIGNAL(clicked()), SLOT(define_model()));
  connect(pb_defconstr, SIGNAL(clicked()), SLOT(define_constraints()));
  connect(pb_savemodel, SIGNAL(clicked()), SLOT(save_model()));
  connect(pb_saveconstr, SIGNAL(clicked()), SLOT(save_constraints()));
  connect(pb_help, SIGNAL(clicked()), SLOT(help()));
  connect(pb_close, SIGNAL(clicked()), SLOT(close()));

  resize(500, 200);

  const int sctmms = 1000;  // Delay 1 second to start model scan
  sctm_id = startTimer(sctmms);
  le_status->setText(tr("Scanning models for unassigned edits"));
  qApp->processEvents();
}

// US_DMGA_Init class destructor
US_DMGA_Init::~US_DMGA_Init() {
  constraints.init_constraints();

  attribs.clear();
  flt_attrs.clear();
}

// Load an existing base model
void US_DMGA_Init::load_model(void) {
  DbgLv(1) << "dGA:load_model";
  le_status->setText(tr("Loading a list of base models"));
  qApp->processEvents();
  bool loadDB = dkdb_cntrls->db();
  QString mfilt = "=m";
  QString mdesc = "";
  QString eGUID = "";
  // mfilt="";
  // eGUID="00000000-0000-0000-0000-000000000000";
  US_ModelLoader mldiag(loadDB, mfilt, bmodel, mdesc, eGUID);

  connect(&mldiag, SIGNAL(changed(bool)), SLOT(update_disk_db(bool)));

  if (mldiag.exec() == QDialog::Accepted) {
    le_status->setText(tr("A base model has been loaded."));
    constraints.load_base_model(&bmodel);
    pb_savemodel->setEnabled(true);
    cmodel.components.clear();
  } else {
    le_status->setText(tr("No base model was loaded."));
  }

  qApp->processEvents();
}

// Define a base model
void US_DMGA_Init::define_model(void) {
  DbgLv(1) << "dGA:define_model";
  US_ModelGui* mddiag = new US_ModelGui(bmodel);
  le_status->setText(tr("Editing or creating a base model"));
  qApp->processEvents();

  connect(mddiag, SIGNAL(valueChanged(US_Model)),
          SLOT(new_base_model(US_Model)));

  mddiag->exec();

  if (bmodel.components.size() > 0) {
    le_status->setText(tr("A base model has been created or edited."));
    constraints.load_base_model(&bmodel);
    pb_savemodel->setEnabled(true);
  } else {
    le_status->setText(tr("No base model was defined."));
  }

  qApp->processEvents();
}

// Load an existing constraints model
void US_DMGA_Init::load_constraints(void) {
  DbgLv(1) << "dGA:load_constraints";

  bool loadDB = dkdb_cntrls->db();
  QString mfilt = "=u Constr";
  QString mdesc = "";
  QString eGUID = "";
  le_status->setText(tr("Loading a list of existing constraints models"));
  qApp->processEvents();

  US_ModelLoader mldiag(loadDB, mfilt, cmodel, mdesc, eGUID);

  connect(&mldiag, SIGNAL(changed(bool)), SLOT(update_disk_db(bool)));
  if (mldiag.exec() != QDialog::Accepted) return;

  DbgLv(1) << "dGA:load_constraints model comps" << cmodel.components.size();
  US_ConstraintsEdit cediag(cmodel);

  if (cediag.exec() == QDialog::Accepted) {
    le_status->setText(tr("A constraints model has been loaded."));
    constraints.load_constraints(&cmodel);
    pb_saveconstr->setEnabled(true);
  }

  else {
    le_status->setText(tr("No constraints model was loaded."));
    cmodel.components.clear();
  }

  qApp->processEvents();
}

// Define a constraints model
void US_DMGA_Init::define_constraints(void) {
  DbgLv(1) << "dGA:define_constraints";
  if (cmodel.components.size() ==
      0) {  // If no constraints model, point to the base model
    cmodel = bmodel;
    cmodel.modelGUID = US_Util::new_guid();
  }

  US_ConstraintsEdit cediag(cmodel);

  le_status->setText(tr("Editor to define a constraints model"));
  qApp->processEvents();

  if (cediag.exec() == QDialog::Accepted) {
    le_status->setText(tr("A constraints model has been defined."));
    constraints.load_constraints(&cmodel);
    pb_saveconstr->setEnabled(true);
  }

  else {
    le_status->setText(tr("No constraints model was defined."));
  }

  qApp->processEvents();
}

// Save a defined base model
void US_DMGA_Init::save_model(void) {
  DbgLv(1) << "dGA:save_model";
  QString msg1 =
      tr("A Base Model has been loaded or created."
         " It's description is:") +
      "<br/>&nbsp;&nbsp;<b>" + bmodel.description + "</b>.<br/><br/>" +
      tr("It may be saved as it is or you may modify the description"
         " or other attributes by clicking the <b>Cancel</b> button"
         " here and clicking the <b>Define Base Model</b> in the"
         " main dialog. The model may also be updated or saved within"
         " the resulting dialog.<br/><br/>"
         "&nbsp;&nbsp;Click:<br/>"
         "<b>OK</b><br/>&nbsp;&nbsp;to output the model as is;<br/>"
         "<b>Cancel</b><br/>&nbsp;&nbsp;to abort the model save.<br/>");

  QMessageBox mbox;
  mbox.setWindowTitle(tr("Save Current Base Model"));
  mbox.setText(msg1);
  QPushButton* pb_ok = mbox.addButton(tr("OK"), QMessageBox::YesRole);
  QPushButton* pb_canc = mbox.addButton(tr("Cancel"), QMessageBox::RejectRole);
  mbox.setEscapeButton(pb_canc);
  mbox.setDefaultButton(pb_ok);

  mbox.exec();

  if (mbox.clickedButton() == pb_canc) return;

  // Output the base model
  int code;

  if (dkdb_cntrls->db()) {  // Write to DB and local
    US_Passwd pw;
    US_DB2 db(pw.getPasswd());

    code = bmodel.write(&db);
  } else {  // Write to local disk only
    bool newFile;
    QString modelPath;
    US_Model::model_path(modelPath);
    QString modelGuid = bmodel.modelGUID;

    if (modelGuid.isEmpty()) {
      modelGuid = US_Util::new_guid();
      bmodel.modelGUID = modelGuid;
    }

    QString fnameo = US_Model::get_filename(modelPath, modelGuid, newFile);
    code = bmodel.write(fnameo);
  }

  QString mtitle = tr("Base Model Saving ...");

  if (code == US_DB2::OK) {
    QString destination =
        dkdb_cntrls->db() ? tr("local disk and database.") : tr("local disk.");
    QMessageBox::information(this, mtitle,
                             tr("The file \"") + cmodel.description +
                                 tr("\"\n was successfully saved to ") +
                                 destination);
  } else {
    QMessageBox::warning(this, mtitle,
                         tr("Writing the file \"") + cmodel.description +
                             tr("\"\n resulted in error code %1 .").arg(code));
  }
}

// Save a defined constraints model
void US_DMGA_Init::save_constraints(void) {
  DbgLv(1) << "dGA:save_constraints";
  QString now = QDateTime::currentDateTime().toUTC().toString("yyyyMMdd-hhmm");
  QString ftype = QString("-DMGA_Constraints");
  QString suffix = "";
  QString fext = ".model";
  QString cmfdesc = now + ftype + suffix + fext;
  QString modelGuid = US_Util::new_guid();
  QString modelPath;
  US_Model::model_path(modelPath);

  QString msg1 = tr("A Discrete Model Genetic Algorithm Constraints"
                    " model has been created. It's description is:") +
                 "<br/><b>" + cmfdesc + "</b>.<br/><br/" +
                 tr("Click:<br/><br/>"
                    "  <b>OK</b>     to output the model as is;<br/>"
                    "  <b>Edit</b>   to append custom text to the name;<br/>"
                    "  <b>Cancel</b> to abort the model save.<br/>");

  QMessageBox mbox;
  mbox.setWindowTitle(tr("Save Constraints Model"));
  mbox.setText(msg1);
  QPushButton* pb_ok = mbox.addButton(tr("OK"), QMessageBox::YesRole);
  QPushButton* pb_edit = mbox.addButton(tr("Edit"), QMessageBox::AcceptRole);
  QPushButton* pb_canc = mbox.addButton(tr("Cancel"), QMessageBox::RejectRole);
  mbox.setEscapeButton(pb_canc);
  mbox.setDefaultButton(pb_ok);

  mbox.exec();

  if (mbox.clickedButton() == pb_canc) return;
  if (mbox.clickedButton() ==
      pb_edit) {  // Open another dialog to get a custom name prefix
    bool ok;
    QString msg2 =
        tr("Enter optional suffix text to the"
           " model description.<br/> Use alphumeric"
           " characters, underscores, or hyphens<br/>"
           " (no spaces).  Enter 1 to 20 characters.");
    suffix = QInputDialog::getText(this, tr("Model Description Suffix"), msg2,
                                   QLineEdit::Normal, suffix, &ok);

    if (!ok) return;

    suffix.remove(QRegExp("[^\\w\\d_-]"));

    if (suffix.length() > 20) suffix = suffix.left(20);
    cmfdesc = now + ftype + suffix + fext;
  }

  // Output the constraints model
  cmodel.analysis = US_Model::DMGA_CONSTR;
  cmodel.description = cmfdesc;
  cmodel.modelGUID = modelGuid;
  int code;

  if (dkdb_cntrls->db()) {
    US_Passwd pw;
    US_DB2 db(pw.getPasswd());

    code = cmodel.write(&db);
  } else {
    bool newFile;
    QString fnameo = US_Model::get_filename(modelPath, modelGuid, newFile);
    code = cmodel.write(fnameo);
  }

  QString mtitle = tr("Constraints Model Saving ...");

  if (code == US_DB2::OK) {
    QString destination =
        dkdb_cntrls->db() ? tr("local disk and database.") : tr("local disk.");
    QMessageBox::information(this, mtitle,
                             tr("The file \"") + cmodel.description +
                                 tr("\"\n was successfully saved to ") +
                                 destination);
  } else {
    QMessageBox::warning(this, mtitle,
                         tr("Writing the file \"") + cmodel.description +
                             tr("\"\n resulted in error code %1 .").arg(code));
  }
}

// Slot to handle a dialog change in the disk/db selection
void US_DMGA_Init::update_disk_db(bool isDB) {
  if (isDB)
    dkdb_cntrls->set_db();
  else
    dkdb_cntrls->set_disk();

  sctm_id = startTimer(1000);
  le_status->setText(tr("Scanning models for unassigned edits"));
  qApp->processEvents();
}

// Slot to handle a model changed in the model editor
void US_DMGA_Init::new_base_model(US_Model new_model) { bmodel = new_model; }

// Protected slot to filter timer event and handle model-scan delay
void US_DMGA_Init::timerEvent(QTimerEvent* event) {
  const QString uaeditID("1");
  int tm_id = event->timerId();
  DbgLv(1) << "dGA:tmEv: tm_id" << tm_id << sctm_id;

  if (tm_id !=
      sctm_id) {  // If other than scan delay, pass it on to the normal handler
    QObject::timerEvent(event);
    return;
  }

  // Otherwise, count and display models with unassigned edit
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  QString atypeMan = QString::number(US_Model::MANUAL);
  QString atypeCgr = QString::number(US_Model::CUSTOMGRID);
  QString atypeDga = QString::number(US_Model::DMGA_CONSTR);
  DbgLv(1) << "  Atype Man Cgr Dga" << atypeMan << atypeCgr << atypeDga;
  qApp->processEvents();
  QTime timer;
  QStringList mdlIDs;
  QString invID = QString::number(US_Settings::us_inv_ID());
  timer.start();
  int numodel = 0;
  int ndmodel = 0;
  int nmmodel = 0;
  int ncmodel = 0;
  int nlmodel = 0;
  DbgLv(1) << " Timing(count_models)" << timer.elapsed();
  if (dkdb_cntrls->db()) {  // Scan models in the database
    US_Passwd pw;
    US_DB2 db(pw.getPasswd());
    QStringList qry;
    qry << "count_models_by_editID" << invID << uaeditID;
    numodel = db.functionQuery(qry);

    if (numodel > 0) {  // If there are models unassigned to edits, scan them
      qry.clear();
      qry << "get_model_desc_by_editID" << invID << uaeditID;
      db.query(qry);

      while (db.next()) {
        QString mdlID = db.value(0).toString();
        QString mdesc = db.value(2).toString();
        QString edtGID = db.value(5).toString();
        int edtID = db.value(6).toString().toInt();

        if (!mdesc.contains("Custom")) {
          mdlIDs << mdlID;
          nlmodel++;
        }
        DbgLv(1) << " mdlID" << mdlID << "mdesc" << mdesc << "editID" << edtID
                 << "editGUID" << edtGID;
      }
      DbgLv(1) << " Timing(get_model_desc)" << timer.elapsed();

      // Scan to parse analysis types
      for (int ii = 0; ii < nlmodel; ii++) {
        QString mdlID = mdlIDs[ii];
        qry.clear();
        qry << "get_model_info" << mdlID;
        db.query(qry);
        db.next();
        QString xmlmdl = db.value(2).toString();
        int jj = xmlmdl.indexOf("analysisType=");
        QString atype = xmlmdl.mid(jj, 40).section("\"", 1, 1);
        DbgLv(1) << "   mdlID" << mdlID << "atype" << atype << "jj" << jj;

        if (atype == atypeDga)
          ndmodel++;
        else if (atype == atypeCgr)
          ncmodel++;
        else if (atype == atypeMan)
          nmmodel++;
      }
      DbgLv(1) << " Timing(get_model_info)" << timer.elapsed();
    }
  }

  else {  // Scan local disk models
    const QString uaGUID("00000000-0000-0000-0000-000000000000");
    QString path = US_Settings::dataDir() + "/models";
    QDir dir;
    if (!dir.exists(path)) dir.mkpath(path);
    dir = QDir(path);
    QStringList mfilt("M*.xml");
    QStringList f_names = dir.entryList(mfilt, QDir::Files, QDir::Name);
    QXmlStreamAttributes attr;

    for (int ii = 0; ii < f_names.size(); ii++) {
      QString fname(path + "/" + f_names[ii]);
      QFile m_file(fname);
      if (!m_file.open(QIODevice::ReadOnly | QIODevice::Text)) continue;
      QXmlStreamReader xml(&m_file);

      while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement() && xml.name() == "model") {
          attr = xml.attributes();
          QString edGUID = attr.value("editGUID").toString();

          if (!edGUID.isEmpty() && edGUID != uaGUID) continue;

          numodel++;

          QString atype = attr.value("analysisType").toString();

          if (atype == atypeDga)
            ndmodel++;
          else if (atype == atypeCgr)
            ncmodel++;
          else if (atype == atypeMan)
            nmmodel++;
        }
      }

      m_file.close();
    }
  }

  DbgLv(1) << " numodel" << numodel << "ndmodel ncmodel nmmodel" << ndmodel
           << ncmodel << nmmodel;

  le_status->setText(tr("%1 unassigned-edit models found"
                        " (%2 Manual, %3 DMGA, %4 CGrid)")
                         .arg(numodel)
                         .arg(nmmodel)
                         .arg(ndmodel)
                         .arg(ncmodel));
  QApplication::restoreOverrideCursor();
  QApplication::restoreOverrideCursor();
  qApp->processEvents();

  killTimer(tm_id);
  return;
}
