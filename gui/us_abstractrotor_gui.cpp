//! \file us_abstractrotor_gui.cpp

#include "us_abstractrotor_gui.h"
#include "us_db2.h"
#include "us_gui_settings.h"
#include "us_license.h"
#include "us_license_t.h"
#include "us_passwd.h"
#include "us_settings.h"
#include "us_util.h"
#include "us_widgets.h"

US_AbstractRotorGui::US_AbstractRotorGui(bool signal_wanted, int select_db_disk, US_Rotor::Rotor *dataIn) :
    US_WidgetsDialog(0, 0), currentRotor(dataIn) {
   this->setModal(true);
   this->signal = signal_wanted;

   setAttribute(Qt::WA_DeleteOnClose);

   setupGui(select_db_disk);
}

void US_AbstractRotorGui::setupGui(int select_db_disk) {
   setWindowTitle(tr("Rotor Type Selection"));
   setPalette(US_GuiSettings::frameColor());

   QGridLayout *top = new QGridLayout(this);
   top->setSpacing(2);
   top->setContentsMargins(2, 2, 2, 2);

   QFontMetrics fm(QFont(US_GuiSettings::fontFamily(), US_GuiSettings::fontSize()));

   int row = 0;
   QStringList DB = US_Settings::defaultDB();
   if (DB.isEmpty())
      DB << "Undefined";

   QLabel *lbl_bannerDB = us_banner(tr("Current Database: ") + DB.at(0));
   lbl_bannerDB->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
   top->addWidget(lbl_bannerDB, row++, 0, 1, 2);

   disk_controls = new US_Disk_DB_Controls(select_db_disk);
   connect(disk_controls, SIGNAL(changed(bool)), SLOT(source_changed(bool)));
   top->addLayout(disk_controls, row++, 0, 1, 2);


   QLabel *lbl_lab = us_label(tr(" Please select a Rotor Type: "));
   top->addWidget(lbl_lab, row, 0, 1, 1);
   cb_rotors = us_comboBox();
   connect(
      cb_rotors, SIGNAL(activated(int)), // Only if the user has changed it
      SLOT(showDetails(int)));
   top->addWidget(cb_rotors, row++, 1, 1, 1);

   if (!loadAbstractRotors()) {
      qDebug() << "Problem loading Abstract Rotors...";
   }
   currentARIndex = getIndex();

   te_details = us_textedit();
   te_details->setText(tr("Rotor Type Details:"));
   te_details->setMinimumSize(400, 250);
   te_details->setFont(QFont("monospace", US_GuiSettings::fontSize()));
   top->addWidget(te_details, row++, 0, 1, 2);

   showDetails(cb_rotors->currentIndex());

   QLabel *lbl_name = us_label(tr(" Rotor name: "), -1);
   top->addWidget(lbl_name, row, 0);

   le_name = us_lineedit("", -1);
   le_name->setText("");
   top->addWidget(le_name, row++, 1);

   QLabel *lbl_serialNumber = us_label(tr(" Rotor serial number: "), -1);
   top->addWidget(lbl_serialNumber, row, 0);

   le_serialNumber = us_lineedit("", -1);
   le_serialNumber->setText("");
   top->addWidget(le_serialNumber, row++, 1);

   pb_help = us_pushbutton(tr("Help"));
   connect(pb_help, SIGNAL(clicked()), this, SLOT(help()));
   top->addWidget(pb_help, row, 0);

   pb_reset = us_pushbutton(tr("Reset"));
   connect(pb_reset, SIGNAL(clicked()), this, SLOT(reset()));
   top->addWidget(pb_reset, row++, 1);

   pb_close = us_pushbutton(tr("Cancel"));
   connect(pb_close, SIGNAL(clicked()), this, SLOT(close()));
   top->addWidget(pb_close, row, 0);

   pb_accept = us_pushbutton(tr("Add Rotor"));
   connect(pb_accept, SIGNAL(clicked()), this, SLOT(select()));
   top->addWidget(pb_accept, row, 1);
}

void US_AbstractRotorGui::reset(void) {
   le_name->setText("");
   le_serialNumber->setText("");

   currentARIndex = getIndex();
   showDetails(currentARIndex);
}

void US_AbstractRotorGui::source_changed(bool db) {
   QStringList DB = US_Settings::defaultDB();

   if (db && (DB.size() < 5)) {
      QMessageBox::warning(this, tr("Attention"), tr("There is no default database set."));
   }

   emit use_db(db);
   qApp->processEvents();

   // Clear out abstract rotor information
   abstractRotorList.clear();
   cb_rotors->clear();
   te_details->clear();

   loadAbstractRotors();
   reset();
}

bool US_AbstractRotorGui::loadAbstractRotors(void) {
   // Get ID's for the abstract rotors we have
   if (disk_controls->db()) {
      US_Passwd pw;
      QString masterPW = pw.getPasswd();
      US_DB2 db(masterPW);

      if (db.lastErrno() != US_DB2::OK) {
         connect_error(db.lastError());
         return (false);
      }

      // Get information about all the abstract rotors
      if (US_Rotor::readAbstractRotorsDB(abstractRotorList, &db) == US_Rotor::NOT_FOUND) {
         QMessageBox::warning(this, tr("Database Problem"), tr("Could not read abstract rotor information \n"));
         return (false);
      }
   }

   else {
      // Get information about all the abstract rotors
      // Add function to check the disk in us_rotor.
      if (US_Rotor::readAbstractRotorsDisk(abstractRotorList) == US_Rotor::NOT_FOUND) {
         QMessageBox::warning(this, tr("Local Disk Problem"), tr("Could not read abstract rotor information\n"));
         return (false);
      }
   }

   // We have all the info; make a list of items to put into the list widget
   QStringList arDescriptions;
   for (int i = 0; i < abstractRotorList.size(); i++) {
      US_Rotor::AbstractRotor ar = abstractRotorList[ i ];

      arDescriptions << (QString::number(ar.ID) + ": " + ar.name);
   }

   // Add the rotor descriptions to the list
   cb_rotors->clear();
   cb_rotors->addItems(arDescriptions);

   return (true);
}

// Get the current logical index into the abstractRotorList vector from the rotor cb
int US_AbstractRotorGui::getIndex(void) {
   if (cb_rotors->count() == 0) {
      // Couldn't read anything---combo box is empty
      return 0;
   }

   QString itemText = cb_rotors->currentText();
   QStringList parts = itemText.split(":");
   int arID = parts[ 0 ].toInt(); // the ID in the AbstractRotor class

   // Now find the corresponding record in abstractRotorList
   for (int i = 0; i < abstractRotorList.size(); i++) {
      if (abstractRotorList[ i ].ID == arID)
         return i;
   }

   // If we got here, there is a problem
   return 0;
}

// Function to update the current rotor information and return
// Calling program updates the rotorID, rotorGUID, labID and creates new rotor
void US_AbstractRotorGui::select() {
   // First double-check if the user has filled out name and serial
   if (le_name->text().isEmpty() || le_serialNumber->text().isEmpty()) {
      QMessageBox::warning(
         this, tr("Please provide the missing information:"), tr("Please provide both a rotor name and serial number"));
      return;
   }

   // Verify that we have the currently-selected abstractRotor information
   currentARIndex = getIndex();
   US_Rotor::AbstractRotor ar = abstractRotorList[ currentARIndex ];

   // Now update the current rotor information
   currentRotor->abstractRotorID = ar.ID;
   currentRotor->abstractRotorGUID = ar.GUID;
   currentRotor->name = le_name->text().trimmed();
   currentRotor->serialNumber = le_serialNumber->text();

   accept();
}

void US_AbstractRotorGui::connect_error(const QString &error) {
   QMessageBox::warning(this, tr("Connection Problem"), tr("Could not connect to database \n") + error);
}

// Function to change the current rotor
void US_AbstractRotorGui::showDetails(int) {
   if (abstractRotorList.size() == 0)
      return; // nothing to display

   currentARIndex = getIndex();
   US_Rotor::AbstractRotor ar = abstractRotorList[ currentARIndex ];

   QString text = tr("Details for the ") + ar.name + tr(" Rotor Type:\n\n");
   text += "Database ID:     " + QString::number(ar.ID) + "\n";
   text += "GUID:            " + ar.GUID + "\n";
   text += "Number of Holes: " + QString::number(ar.numHoles) + "\n";
   text += "Maximum Speed:   " + QString::number(ar.maxRPM) + " rpm\n";
   text += "Magnet Offset:   " + QString::number(ar.magnetOffset) + " degrees\n";
   text += "Cell Center:     " + QString::number(ar.cellCenter) + " cm\n";
   text += "Material:        " + ar.material + "\n";
   text += "Manufacturer:    " + ar.manufacturer;
   te_details->clear();
   te_details->setText(text);
}
