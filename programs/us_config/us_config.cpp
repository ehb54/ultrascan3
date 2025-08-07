//! \file us_config.cpp
#include <QApplication>

#include "us_config.h"
#include "us_constants.h"
#include "us_defines.h"
#include "us_gui_settings.h"
#include "us_license.h"
#include "us_license_t.h"
#include "us_settings.h"

int main(int argc, char *argv[]) {
   QApplication application(argc, argv);

#include "main1.inc"

   // License is OK.  Start up.
   US_Config w;
   w.show(); //!< \memberof QWidget
   return application.exec(); //!< \memberof QApplication
}

///////////////////
US_Config::US_Config(QWidget *parent, Qt::WindowFlags flags) : US_Widgets(true, parent, flags) {
   font = NULL;
   db = NULL;
   colors = NULL;
   admin = NULL;
   chg_dimport = false;
   chg_dtmp = false;

   setWindowTitle("UltraScan Configuration");
   setPalette(US_GuiSettings::frameColor());

   // Directories

   QBoxLayout *topbox = new QVBoxLayout(this);
   topbox->setContentsMargins(2, 2, 2, 2);
   topbox->setSpacing(2);

   QLabel *banner = us_banner(tr("UltraScan %1 Configuration").arg(US_Version));
   topbox->addWidget(banner);

   QLabel *paths = us_banner(tr("Directories"));
   paths->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
   topbox->addWidget(paths);

   int row = 0;
   QGridLayout *directories = new QGridLayout();

   // Calculate width for directory pushbuttons
   QFont *f = new QFont(US_GuiSettings::fontFamily(), US_GuiSettings::fontSize());
   QFontMetrics *fm = new QFontMetrics(*f);
   int w = fm->width(tr("Temporary Directory:")) + 20;

   // Browser
   pb_browser = us_pushbutton(tr("WWW Browser:"));
   pb_browser->setFixedWidth(w);
   directories->addWidget(pb_browser, row, 0);
   connect(pb_browser, SIGNAL(clicked()), this, SLOT(open_browser()));

   // Calculate the minimum width of all the le boxes
   QString browser = US_Settings::browser();
   int le_w = fm->width(browser) + 20;

   le_browser = us_lineedit(browser, 0);
   directories->addWidget(le_browser, row++, 1);

   // Base Work Directory
   pb_workDir = us_pushbutton(tr("Base Work Directory:"));
   pb_workDir->setFixedWidth(w);
   directories->addWidget(pb_workDir, row, 0);
   connect(pb_workDir, SIGNAL(clicked()), this, SLOT(open_workDir()));

   QString workDir = US_Settings::workBaseDir();
   le_w = qMax(fm->width(workDir) + 20, le_w);

   le_workDir = us_lineedit(workDir, 0);
   directories->addWidget(le_workDir, row++, 1);
   connect(le_workDir, SIGNAL(editingFinished()), this, SLOT(update_workDir()));

   // Imports Directory
   pb_importDir = us_pushbutton(tr("Imports Directory:"));
   pb_importDir->setFixedWidth(w);
   directories->addWidget(pb_importDir, row, 0);
   connect(pb_importDir, SIGNAL(clicked()), this, SLOT(open_importDir()));

   QString importDir = US_Settings::importDir();
   le_w = qMax(fm->width(importDir) + 20, le_w);

   le_importDir = us_lineedit(importDir, 0);
   directories->addWidget(le_importDir, row++, 1);
   connect(le_importDir, SIGNAL(editingFinished()), this, SLOT(update_importDir()));

   // Temporary Directory
   pb_tmpDir = us_pushbutton(tr("Temporary Directory:"));
   pb_tmpDir->setFixedWidth(w);
   directories->addWidget(pb_tmpDir, row, 0);
   connect(pb_tmpDir, SIGNAL(clicked()), this, SLOT(open_tmpDir()));

   QString tmpDir = US_Settings::tmpDir();
   le_w = qMax(fm->width(tmpDir) + 20, le_w);

   le_tmpDir = us_lineedit(tmpDir, 0);
   directories->addWidget(le_tmpDir, row++, 1);
   connect(le_tmpDir, SIGNAL(editingFinished()), this, SLOT(update_tmpDir()));

   le_browser->setMinimumWidth(le_w);
   le_workDir->setMinimumWidth(le_w);
   le_importDir->setMinimumWidth(le_w);
   le_tmpDir->setMinimumWidth(le_w);

   topbox->addLayout(directories);

   QLabel *misc = us_banner(tr("Miscellaneous Settings"));
   misc->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
   topbox->addWidget(misc);

   // Misc Settings
   row = 0;
   QGridLayout *otherSettings = new QGridLayout();

   // Disk/DB preference
   QLabel *lb_disk_db = us_label("Default Data Location:");
   otherSettings->addWidget(lb_disk_db, row, 0);

   disk_db_control = new US_Disk_DB_Controls(US_Disk_DB_Controls::Default);
   otherSettings->addLayout(disk_db_control, row++, 1);
   connect(disk_db_control, SIGNAL(changed(bool)), SLOT(set_data_location(bool)));
   // Color Preferences
   QLabel *color = us_label("Color Preferences:");
   otherSettings->addWidget(color, row, 0);

   pb_color = us_pushbutton(tr("Change"));
   otherSettings->addWidget(pb_color, row++, 1);
   connect(pb_color, SIGNAL(clicked()), this, SLOT(update_colors()));

   // Font Preferences
   QLabel *font = us_label("Font Preferences:");
   otherSettings->addWidget(font, row, 0);

   pb_font = us_pushbutton(tr("Change"));
   otherSettings->addWidget(pb_font, row++, 1);
   connect(pb_font, SIGNAL(clicked()), this, SLOT(update_font()));

   // Database Preferences
   QLabel *db = us_label("Database Preferences:");
   otherSettings->addWidget(db, row, 0);

   pb_db = us_pushbutton(tr("Change"));
   otherSettings->addWidget(pb_db, row++, 1);
   connect(pb_db, SIGNAL(clicked()), this, SLOT(update_db()));
   /* For DB change - check autoflow records & disable with message if there are any records */

   /******************************************************************************************/


   // Master Password
   QLabel *password = us_label("Master Password:");
   otherSettings->addWidget(password, row, 0);

   pb_password = us_pushbutton(tr("Change"));
   otherSettings->addWidget(pb_password, row++, 1);
   connect(pb_password, SIGNAL(clicked()), this, SLOT(update_password()));

   // Advanced settings
   QLabel *advanced = us_label("Advanced Settings:");
   otherSettings->addWidget(advanced, row, 0);

   pb_advanced = us_pushbutton(tr("Change"));
   otherSettings->addWidget(pb_advanced, row++, 1);
   connect(pb_advanced, SIGNAL(clicked()), this, SLOT(update_advanced()));

   // Optima Host
   QLabel *lb_xpnhost = us_label("Instrument Preferences:");
   otherSettings->addWidget(lb_xpnhost, row, 0);

   pb_xpnh = us_pushbutton(tr("Change"));
   if (!disk_db_control->db())
      pb_xpnh->setEnabled(false);

   otherSettings->addWidget(pb_xpnh, row++, 1);
   connect(pb_xpnh, SIGNAL(clicked()), this, SLOT(update_xpnhost()));

   topbox->addLayout(otherSettings);

   // Pushbuttons
   pb_help = us_pushbutton(tr("Help"));
   connect(pb_help, SIGNAL(clicked()), this, SLOT(help()));

   pb_save = us_pushbutton(tr("Save"));
   connect(pb_save, SIGNAL(clicked()), this, SLOT(save()));

   pb_cancel = us_pushbutton(tr("Close"));
   connect(pb_cancel, SIGNAL(clicked()), this, SLOT(close()));

   QBoxLayout *buttons = new QHBoxLayout();
   buttons->addWidget(pb_save);
   buttons->addWidget(pb_help);
   buttons->addWidget(pb_cancel);
   topbox->addLayout(buttons);
}

void US_Config::help(void) {
   showhelp.show_help("config.html");
}

void US_Config::save(void) {
   US_Settings::set_browser(le_browser->text());
   US_Settings::set_workBaseDir(le_workDir->text());
   US_Settings::set_importDir(le_importDir->text());
   US_Settings::set_tmpDir(le_tmpDir->text());

   // Ensure work directories are properly created
   QDir dir;
   dir.mkpath(le_workDir->text());
   dir.mkpath(le_importDir->text());
   dir.mkpath(le_tmpDir->text());

   QMessageBox::information(this, tr("Settings Saved"), tr("The settings were successfully saved"));
}

void US_Config::set_data_location(bool) {
   if (disk_db_control->db()) {
      US_Settings::set_default_data_location(US_Disk_DB_Controls::DB);
      pb_xpnh->setEnabled(true);
   }
   else {
      US_Settings::set_default_data_location(US_Disk_DB_Controls::Disk);
      pb_xpnh->setEnabled(false);
   }
}

void US_Config::update_colors(void) {
   colors = new US_Color; // Automatic delete
   colors->show();
}

void US_Config::update_font(void) {
   font = new US_Font; // Automatic delete
   font->show();
}

void US_Config::update_db(void) {
   db = new US_Database; // Automatic delete
   db->show();
}

void US_Config::update_password(void) {
   admin = new US_Admin; // Automatic delete
   admin->show();
}

void US_Config::update_advanced(void) {
   advanced = new US_Advanced;
   advanced->show();
}

void US_Config::update_xpnhost(void) {
   /* Existing: from .conf file */
   // xpnhost  = new US_XpnHost;
   // xpnhost ->setParent(this, Qt::Window);
   // xpnhost->setWindowModality(Qt::WindowModal);
   // xpnhost->setAttribute(Qt::WA_DeleteOnClose);
   // xpnhost ->show();

   /* New: from us-lims DB */
   xpnhost_db = new US_XpnHostDB;
   xpnhost_db->setParent(this, Qt::Window);
   xpnhost_db->setWindowModality(Qt::WindowModal);
   xpnhost_db->setAttribute(Qt::WA_DeleteOnClose);
   xpnhost_db->show();
}

void US_Config::open_browser(void) {
   QString browser = QFileDialog::getOpenFileName(this, tr("Select desired browser application"), le_browser->text());

   if (browser != "")
      le_browser->setText(browser);
}

// Slot to react to file dialog setting of base work directory
void US_Config::open_workDir(void) {
   QString dir = QFileDialog::getExistingDirectory(this, tr("Select desired base work directory"), le_workDir->text());

   if (dir != "") {
      le_workDir->setText(dir);
      update_workDir();
   }
}

// Slot to react to file dialog setting of imports directory
void US_Config::open_importDir(void) {
   QString dir = QFileDialog::getExistingDirectory(this, tr("Select desired imports directory"), le_importDir->text());

   if (dir != "") {
      le_importDir->setText(dir);
      update_importDir();
   }
}

// Slot to react to file dialog setting of tmp directory
void US_Config::open_tmpDir(void) {
   QString dir = QFileDialog::getExistingDirectory(this, tr("Select desired temporary directory"), le_tmpDir->text());

   if (dir != "") {
      le_tmpDir->setText(dir);
      update_tmpDir();
   }
}

// Slot to react to change in work base directory
void US_Config::update_workDir(void) {
   QString work_dir = le_workDir->text();

   if (!chg_dimport) { // If imports directory wasn't changed already, default using work base
      le_importDir->setText(work_dir + "/imports");
   }

   if (!chg_dtmp) { // If tmp directory wasn't changed already, default using work base
      le_tmpDir->setText(work_dir + "/tmp");
   }
}

// Slot to react to change in imports directory
void US_Config::update_importDir(void) {
   // Mark imports directory as having been changed
   chg_dimport = true;
}

// Slot to react to change in tmp directory
void US_Config::update_tmpDir(void) {
   // Mark tmp directory as having been changed
   chg_dtmp = true;
}
