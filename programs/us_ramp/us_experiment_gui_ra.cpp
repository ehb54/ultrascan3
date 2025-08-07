//! \file us_experiment_gui_ra.cpp

#include "us_experiment_gui_ra.h"
#include "us_db2.h"
#include "us_gui_settings.h"
#include "us_investigator.h"
#include "us_passwd.h"
#include "us_project_gui.h"
#include "us_rotor_gui.h"
#include "us_settings.h"

US_ExperimentGuiRa::US_ExperimentGuiRa(bool signal_wanted, const US_ExperimentRa &dataIn, int select_db_disk) :
    US_WidgetsDialog(0, 0), expInfo(dataIn) {
   signal = signal_wanted;

   setPalette(US_GuiSettings::frameColor());
   setWindowTitle(tr("Experiment Information"));
   setAttribute(Qt::WA_DeleteOnClose);

   // Set up left panel with experiment information
   QGridLayout *experiment = new QGridLayout;
   int row = 0;

   // Current experiment information
   QLabel *lb_experiment_banner = us_banner(tr("Experiment: "));
   experiment->addWidget(lb_experiment_banner, row++, 0, 1, 2);

   // Show current runID
   QLabel *lb_runID = us_label(tr("Run ID "));
   experiment->addWidget(lb_runID, row++, 0, 1, 2);
   le_runID = us_lineedit("", 0, true);

   experiment->addWidget(le_runID, row++, 0, 1, 2);

   // Experiment label
   QLabel *lb_label = us_label(tr("Label:"));
   experiment->addWidget(lb_label, row++, 0, 1, 2);
   le_label = us_lineedit();
   connect(le_label, SIGNAL(editingFinished()), SLOT(saveLabel()));
   experiment->addWidget(le_label, row++, 0, 1, 2);

   // Project
   QLabel *lb_project = us_label(tr("Project:"));
   experiment->addWidget(lb_project, row, 0);
   QPushButton *pb_project = us_pushbutton(tr("Select Project"));
   connect(pb_project, SIGNAL(clicked()), SLOT(selectProject()));
   pb_project->setEnabled(true);
   experiment->addWidget(pb_project, row++, 1);

   le_project = us_lineedit("", 0, true);
   experiment->addWidget(le_project, row++, 0, 1, 2);

   // Experiment type
   QLabel *lb_expType = us_label(tr("Experiment Type:"));
   experiment->addWidget(lb_expType, row, 0);
   cb_expType = us_expTypeComboBox();
   experiment->addWidget(cb_expType, row++, 1);

   // Optical system
   QLabel *lb_opticalSystem = us_label(tr("Optical System:"));
   experiment->addWidget(lb_opticalSystem, row, 0);
   QLineEdit *le_opticalSystem = us_lineedit("", 0, true);
   experiment->addWidget(le_opticalSystem, row++, 1);

   // The optical system won't change
   if ((expInfo.opticalSystem == "RA") || (expInfo.opticalSystem == "WA"))
      le_opticalSystem->setText("Absorbance");

   else if ((expInfo.opticalSystem == "RI") || (expInfo.opticalSystem == "WI"))
      le_opticalSystem->setText("Intensity");

   else if (expInfo.opticalSystem == "IP")
      le_opticalSystem->setText("Interference");

   else if (expInfo.opticalSystem == "FI")
      le_opticalSystem->setText("Fluorescence");

   else // Unsupported optical system
      le_opticalSystem->setText("Unsupported");

   // Now for predominantly hardware info
   QGridLayout *hardware = new QGridLayout;
   row = 0;

   // Selected hardware information
   QLabel *lb_hardware_banner = us_banner(tr("Hardware: "));
   hardware->addWidget(lb_hardware_banner, row++, 0, 1, 2);

   QPushButton *pb_rotor = us_pushbutton(tr("Select Lab / Rotor / Calibration"));
   connect(pb_rotor, SIGNAL(clicked()), SLOT(selectRotor()));
   pb_rotor->setEnabled(true);
   hardware->addWidget(pb_rotor, row++, 0, 1, 2);

   le_rotorDesc = us_lineedit("", 0, true);
   hardware->addWidget(le_rotorDesc, row++, 0, 1, 2);

   // Rotor speeds
   QLabel *lb_rotorSpeeds = us_label(tr("Unique Rotor Speeds:"));
   hardware->addWidget(lb_rotorSpeeds, row++, 0, 1, 2);
   lw_rotorSpeeds = us_listwidget();
   lw_rotorSpeeds->setMaximumHeight(50);
   lw_rotorSpeeds->setPalette(vlgray);
   hardware->addWidget(lw_rotorSpeeds, row, 0, 2, 2);
   row += 2;

   // instrumentID
   QLabel *lb_instrument = us_label(tr("Instrument:"));
   hardware->addWidget(lb_instrument, row, 0);
   cb_instrument = new US_SelectBoxRa(this);
   connect(cb_instrument, SIGNAL(activated(int)), SLOT(change_instrument(int)));
   hardware->addWidget(cb_instrument, row++, 1);

   // operatorID
   QLabel *lb_operator = us_label(tr("Operator:"));
   hardware->addWidget(lb_operator, row, 0);
   cb_operator = new US_SelectBoxRa(this);
   hardware->addWidget(cb_operator, row++, 1);

   // Run Temperature
   QLabel *lb_runTemp = us_label(tr("Average Run Temperature:"));
   hardware->addWidget(lb_runTemp, row, 0);
   le_runTemp = us_lineedit("", 0, true);
   hardware->addWidget(le_runTemp, row++, 1);

   // Run temperature won't change
   le_runTemp->setText(expInfo.runTemp);

   // Some pushbuttons
   QHBoxLayout *buttons = new QHBoxLayout;

   QPushButton *pb_help = us_pushbutton(tr("Help"));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));
   buttons->addWidget(pb_help);

   QPushButton *pb_cancel = us_pushbutton(tr("Cancel"));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));
   buttons->addWidget(pb_cancel);

   pb_accept = us_pushbutton(tr("Accept"));
   connect(pb_accept, SIGNAL(clicked()), SLOT(accept()));
   buttons->addWidget(pb_accept);

   // Now let's assemble the page
   QGridLayout *main = new QGridLayout(this);
   main->setSpacing(2);
   main->setContentsMargins(2, 2, 2, 2);

   row = 0;

   // Database choices
   QStringList DB = US_Settings::defaultDB();
   if (DB.isEmpty())
      DB << "Undefined";
   QLabel *lb_DB = us_banner(tr("Database: ") + DB.at(0));
   main->addWidget(lb_DB, row++, 0, 1, 2);

   // Investigator
   if (US_Settings::us_inv_level() > 2) {
      QPushButton *pb_investigator = us_pushbutton(tr("Select Investigator"));
      connect(pb_investigator, SIGNAL(clicked()), SLOT(selectInvestigator()));
      main->addWidget(pb_investigator, row, 0);
   }
   else {
      QLabel *lb_investigator = us_label(tr("Investigator:"));
      main->addWidget(lb_investigator, row, 0);
   }

   le_investigator = us_lineedit(tr("Not Selected"));
   le_investigator->setReadOnly(true);
   main->addWidget(le_investigator, row++, 1);

   disk_controls = new US_Disk_DB_Controls(select_db_disk);
   connect(disk_controls, SIGNAL(changed(bool)), SLOT(source_changed(bool)));
   main->addLayout(disk_controls, row++, 0, 1, 2);

   main->addLayout(experiment, row, 0);
   main->addLayout(hardware, row, 1);
   row++; // += 10;

   // Experiment comments
   QLabel *lb_comment = us_label(tr("Comments:"));
   main->addWidget(lb_comment, row++, 0, 1, 2);

   te_comment = us_textedit();
   main->addWidget(te_comment, row, 0, 4, 2);
   te_comment->setMaximumHeight(80);
   te_comment->setReadOnly(false);
   row += 4;

   main->addLayout(buttons, row++, 0, 1, 2);

   // Let's load everything we can
   if (!load()) {
      cancel();
      return;
   }

   reset();
}

void US_ExperimentGuiRa::reset(void) {
   reload();

   le_investigator->clear();
   le_label->clear();
   le_runID->setText(expInfo.runID);
   le_project->clear();
   te_comment->clear();

   pb_accept->setEnabled(false);

   // Update controls to represent selected experiment
   cb_instrument->setLogicalIndex(expInfo.instrumentID);
   cb_operator->setLogicalIndex(expInfo.operatorID);

   le_label->setText(expInfo.label);
   le_project->setText(expInfo.project.projectDesc);
   te_comment->setText(expInfo.comments);

   // Experiment types combo
   cb_expType->setCurrentIndex(0); // default is "velocity"
   for (int i = 0; i < experimentTypes.size(); i++) {
      if (experimentTypes[ i ].toUpper() == expInfo.expType.toUpper()) {
         cb_expType->setCurrentIndex(i);
         break;
      }
   }

   // Display investigator
   expInfo.invID = US_Settings::us_inv_ID();

   if (expInfo.invID > 0 || !disk_controls->db()) {
      le_investigator->setText(QString::number(expInfo.invID) + ": " + US_Settings::us_inv_name());

      if (disk_controls->db()) {
         US_Passwd pw;
         QString masterPW = pw.getPasswd();
         US_DB2 db(masterPW);

         int runIDStatus = US_DB2::NO_EXPERIMENT;
         if (db.lastErrno() == US_DB2::OK)
            runIDStatus = expInfo.checkRunID(&db);

         if (expInfo.expID > 0)
            pb_accept->setEnabled(true);

         else if (runIDStatus != US_DB2::OK) {
            // Then an investigator has been chosen, and
            //  the current runID doesn't exist in the db
            pb_accept->setEnabled(true);
         }
      }

      else {
         // We can always accept in disk mode
         pb_accept->setEnabled(true);
      }

      // However, project needs to be selected, either from db or disk
      if (expInfo.project.projectID == 0 && expInfo.project.projectGUID.isEmpty())
         pb_accept->setEnabled(false);

      // The label can't be empty either
      if (expInfo.label.isEmpty())
         pb_accept->setEnabled(false);
   }

   else
      le_investigator->setText(US_Settings::us_inv_name());
}

// function to load what we can initially
// returns true if successful
bool US_ExperimentGuiRa::load(void) {
   if (expInfo.invID == 0) {
      // Try to get info from settings
      int inv = US_Settings::us_inv_ID();
      if (inv > -1) {
         expInfo.invID = inv;
         getInvestigatorInfo();
      }
   }

   // Load values that were passed in
   if (!expInfo.rotorName.isEmpty())
      le_rotorDesc->setText(expInfo.rotorName + " / " + expInfo.rotorUpdated.toString("yyyy-MM-dd"));

   if (!expInfo.label.isEmpty())
      le_label->setText(expInfo.label);

   if (expInfo.project.projectID > 0)
      le_project->setText(expInfo.project.projectDesc);

   if (!expInfo.comments.isEmpty())
      te_comment->setText(expInfo.comments);

   if (disk_controls->db()) {
      US_Passwd pw;
      QString masterPW = pw.getPasswd();
      US_DB2 db(masterPW);

      if (db.lastErrno() == US_DB2::OK)
         US_Rotor::readLabsDB(labList, &db);

      if (labList.size() > 0 && labList[ 0 ].instruments.size() == 0) { // If empty instrument table, warn and exit
         US_Rotor::Instrument instrument;
         US_Rotor::Operator loperator;
         instrument.ID = 1;
         instrument.name = "XLA #1";
         instrument.serial = "9999";
         loperator.ID = 1;
         loperator.GUID = "";
         loperator.lname = "operator";
         loperator.fname = "some";
         instrument.operators << loperator;
         labList[ 0 ].instruments << instrument;

         QMessageBox::warning(
            this, tr("No Instruments"),
            tr("There is no instrument record in the database.\n"
               "At least one instrument record should be added\n"
               "   through the LIMS interface.\n\n"
               "The current Convert Legacy Data program\n"
               "   WILL NOW BE CLOSED!!!!"));

         cancel();
         exit(99);
      }
   }

   else
      US_Rotor::readLabsDisk(labList);

   lab_changed = true; // so boxes will go through all the reload code 1st time

   return (true);
}

void US_ExperimentGuiRa::reload(void) {
   qDebug() << "ExpG:reload: IN labList size" << labList.size();
   if (lab_changed && labList.size() > 0) {
      // Find labList info for this lab
      currentLab = 0;
      bool found = false;
      for (int i = 0; i < labList.size(); i++) {
         if (labList[ i ].ID == expInfo.labID) {
            found = true;
            currentLab = i;
            break;
         }
      }

      if (!found) {
         // replace with the first one on the list
         expInfo.labID = labList[ 0 ].ID;
         currentLab = 0;
      }

      qDebug() << "ExpG:reload:  call setInstr";
      setInstrumentList();
      qDebug() << "ExpG:reload:  call setOper";
      setOperatorList();

      qDebug() << "ExpG:reload:  call instr load()";
      cb_instrument->load();
      qDebug() << "ExpG:reload:  call oper load()";
      cb_operator->load();
      qDebug() << "ExpG:reload:  retn fr open load()";
   }
   qDebug() << "ExpG:reload: RTN";

   lab_changed = false;
}

void US_ExperimentGuiRa::syncHardware(void) {}

void US_ExperimentGuiRa::selectInvestigator(void) {
   US_Investigator *inv_dialog = new US_Investigator(true, expInfo.invID);

   connect(inv_dialog, SIGNAL(investigator_accepted(int)), SLOT(assignInvestigator(int)));

   inv_dialog->exec();
}

void US_ExperimentGuiRa::assignInvestigator(int invID) {
   expInfo.invID = invID;

   QString number = (invID > 0) ? QString::number(invID) + ": " : "";
   le_investigator->setText(number + US_Settings::us_inv_name());
}

void US_ExperimentGuiRa::getInvestigatorInfo(void) {
   expInfo.invID = US_Settings::us_inv_ID(); // just to be sure
   expInfo.name = US_Settings::us_inv_name();

   if (disk_controls->db()) {
      US_Passwd pw;
      QString masterPW = pw.getPasswd();
      US_DB2 db(masterPW);

      if (db.lastErrno() == US_DB2::OK) {
         QStringList q("get_person_info");
         q << QString::number(expInfo.invID);
         db.query(q);

         if (db.next())
            expInfo.invGUID = db.value(9).toString();
      }
   }
}

void US_ExperimentGuiRa::source_changed(bool db) {
   QStringList DB = US_Settings::defaultDB();

   if (db && (DB.size() < 5)) {
      QMessageBox::warning(this, tr("Attention"), tr("There is no default database set."));
   }

   emit use_db(db);
   qApp->processEvents();

   //load();
   reset();
}

void US_ExperimentGuiRa::update_disk_db(bool db) {
   (db) ? disk_controls->set_db() : disk_controls->set_disk();

   // Pass it on to US_Convert dialog
   emit use_db(db);
}

void US_ExperimentGuiRa::selectProject(void) {
   // Save other elements on the page first
   expInfo.label = le_label->text();
   expInfo.comments = te_comment->toPlainText();
   expInfo.expType = cb_expType->currentText().toLower();

   int dbdisk = (disk_controls->db()) ? US_Disk_DB_Controls::DB : US_Disk_DB_Controls::Disk;

   US_Project project = expInfo.project;

   US_ProjectGui *projInfo = new US_ProjectGui(true, dbdisk, project);

   connect(projInfo, SIGNAL(updateProjectGuiSelection(US_Project &)), SLOT(assignProject(US_Project &)));

   connect(projInfo, SIGNAL(cancelProjectGuiSelection()), SLOT(cancelProject()));

   connect(projInfo, SIGNAL(use_db(bool)), SLOT(update_disk_db(bool)));

   projInfo->exec();
   delete projInfo;
}

void US_ExperimentGuiRa::assignProject(US_Project &project) {
   expInfo.project = project;

   reset();
}

void US_ExperimentGuiRa::cancelProject(void) {
   reset();
}

// Function to update the labe associated with the current experiment
void US_ExperimentGuiRa::saveLabel(void) {
   expInfo.label = le_label->text();
   expInfo.label = expInfo.label.trimmed();

   // Save other elements on the page before reset
   expInfo.comments = te_comment->toPlainText();
   expInfo.expType = cb_expType->currentText().toLower();

   reset(); // To get the pb_accept enable code
}

QComboBox *US_ExperimentGuiRa::us_expTypeComboBox(void) {
   QComboBox *cb = us_comboBox();

   // Experiment types
   experimentTypes.clear();
   experimentTypes << "Velocity"
                   << "Equilibrium"
                   << "Diffusion"
                   << "Buoyancy"
                   << "Calibration"
                   << "Other";

   cb->addItems(experimentTypes);

   return cb;
}

void US_ExperimentGuiRa::setInstrumentList(void) {
   qDebug() << "ExpG: setInstrL: IN labList size" << labList.size() << "currentLab" << currentLab;
   QList<listInfo> options;
   QList<US_Rotor::Instrument> instruments = labList[ currentLab ].instruments;
   qDebug() << "ExpG: setInstrL:  instruments size" << instruments.size();

   foreach (US_Rotor::Instrument instrument, instruments) {
      struct listInfo option;
      option.ID = QString::number(instrument.ID);
      option.text = instrument.name;
      options << option;
   }

   cb_instrument->clear();
   if (options.size() > 0) {
      cb_instrument->addOptions(options);

      // is the instrument ID in the list?
      currentInstrument = 0;
      for (int i = 0; i < options.size(); i++) {
         if (expInfo.instrumentID == options[ i ].ID.toInt()) {
            currentInstrument = i;
            break;
         }
      }

      // Replace instrument ID with one from the list
      expInfo.instrumentID = instruments[ currentInstrument ].ID;
      expInfo.instrumentSerial = instruments[ currentInstrument ].serial;
      qDebug() << "ExpG: setInstrL:  ins ID Ser" << expInfo.instrumentID << expInfo.instrumentSerial;
   }
}

void US_ExperimentGuiRa::setOperatorList(void) {
   QList<listInfo> options;
   QList<US_Rotor::Instrument> instruments = labList[ currentLab ].instruments;
   QList<US_Rotor::Operator> operators = instruments[ currentInstrument ].operators;

   foreach (US_Rotor::Operator oper, operators) {
      struct listInfo option;
      option.ID = QString::number(oper.ID);
      option.text = oper.lname + ", " + oper.fname;
      options << option;
   }

   cb_operator->clear();
   if (options.size() > 0) {
      int currentOperator = 0;

      if (!disk_controls->db()) {
         struct listInfo disk_only;
         disk_only.ID = QString("0");
         disk_only.text = "Local";
         cb_operator->addOption(disk_only);

         expInfo.operatorID = 0;
         expInfo.operatorGUID = "";
      }

      else {
         cb_operator->addOptions(options);

         // is the operator ID in the list?
         for (int i = 0; i < options.size(); i++) {
            if (expInfo.operatorID == options[ i ].ID.toInt()) {
               currentOperator = i;
               break;
            }
         }

         // Replace operator ID with one from the list
         expInfo.operatorID = operators[ currentOperator ].ID;
         expInfo.operatorGUID = operators[ currentOperator ].GUID;
      }
   }
}

// Function to change the current instrument
void US_ExperimentGuiRa::change_instrument(int) {
   // First time through here the combo box might not be displayed yet
   expInfo.instrumentID = (cb_instrument->getLogicalID() == -1) ? expInfo.instrumentID : cb_instrument->getLogicalID();

   // Save other elements on the page too
   expInfo.label = le_label->text();
   expInfo.comments = te_comment->toPlainText();
   expInfo.expType = cb_expType->currentText();

   lab_changed = true;
   reset();
}

void US_ExperimentGuiRa::selectRotor(void) {
   // Save other elements on the page first
   expInfo.label = le_label->text();
   expInfo.comments = te_comment->toPlainText();
   expInfo.expType = cb_expType->currentText().toLower();

   US_Rotor::Rotor rotor;
   rotor.ID = expInfo.rotorID;

   US_Rotor::RotorCalibration calibration;
   calibration.ID = expInfo.calibrationID;

   int dbdisk = (disk_controls->db()) ? US_Disk_DB_Controls::DB : US_Disk_DB_Controls::Disk;

   US_RotorGui *rotorInfo = new US_RotorGui(
      true, // signal_wanted
      dbdisk, rotor, calibration);

   connect(
      rotorInfo, SIGNAL(RotorCalibrationSelected(US_Rotor::Rotor &, US_Rotor::RotorCalibration &)),
      SLOT(assignRotor(US_Rotor::Rotor &, US_Rotor::RotorCalibration &)));

   connect(rotorInfo, SIGNAL(RotorCalibrationCanceled()), SLOT(cancelRotor()));

   connect(rotorInfo, SIGNAL(use_db(bool)), SLOT(update_disk_db(bool)));

   rotorInfo->exec();
}

void US_ExperimentGuiRa::assignRotor(US_Rotor::Rotor &rotor, US_Rotor::RotorCalibration &calibration) {
   expInfo.rotorID = rotor.ID;
   expInfo.rotorGUID = rotor.GUID;
   expInfo.rotorSerial = rotor.serialNumber;
   expInfo.rotorName = rotor.name;
   expInfo.labID = rotor.labID;
   expInfo.calibrationID = calibration.ID;
   expInfo.rotorCoeff1 = calibration.coeff1;
   expInfo.rotorCoeff2 = calibration.coeff2;
   expInfo.rotorUpdated = calibration.lastUpdated;

   le_rotorDesc->setText(rotor.name + " / " + calibration.lastUpdated.toString("yyyy-MM-dd"));

   lab_changed = true;
   setInstrumentList();
   setOperatorList();

   reset();
}

void US_ExperimentGuiRa::cancelRotor(void) {
   reset();
}

void US_ExperimentGuiRa::accept(void) {
   // We can sync with the DB
   expInfo.syncOK = true;

   // Overwrite data directly from the form

   // First get the invID
   expInfo.invID = US_Settings::us_inv_ID();
   expInfo.name = US_Settings::us_inv_name();
   getInvestigatorInfo();

   // Other info on the form
   expInfo.runID = le_runID->text();
   expInfo.instrumentID = cb_instrument->getLogicalID();
   expInfo.operatorID = cb_operator->getLogicalID();
   expInfo.expType = cb_expType->currentText().toLower();
   expInfo.runTemp = le_runTemp->text();
   expInfo.label = le_label->text();
   expInfo.comments = te_comment->toPlainText();

   // Update items from the DB after getting values from the form, if we can
   if (disk_controls->db()) {
      US_Passwd pw;
      QString masterPW = pw.getPasswd();
      US_DB2 db(masterPW);

      if (db.lastErrno() == US_DB2::OK) {
         QStringList q("get_instrument_info");
         q << QString::number(expInfo.instrumentID);
         db.query(q);
         db.next();
         expInfo.instrumentSerial = db.value(1).toString();

         q.clear();
         q << "get_person_info" << QString::number(expInfo.operatorID);
         db.query(q);
         db.next();
         expInfo.operatorGUID = db.value(9).toString();
      }
   }

   emit updateExpInfoSelection(expInfo);
   close();
}

void US_ExperimentGuiRa::cancel(void) {
   //   expInfo.clear();

   emit cancelExpInfoSelection();
   close();
}

void US_ExperimentGuiRa::connect_error(const QString &error) {
   QMessageBox::warning(this, tr("Connection Problem"), tr("Could not connect to database \n") + error);
}
