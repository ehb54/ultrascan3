//! \file us_database.cpp
#include "us_database.h"
#include "us_crypto.h"
#include "us_db2.h"
#include "us_gui_settings.h"
#include "us_help.h"
#include "us_local_server.h"
#include "us_passwd.h"
#include "us_settings.h"

#ifndef DbgLv
#define DbgLv(a) \
   if (dbg_level >= a) \
   qDebug() //!< debug-level-conditioned qDebug()
#endif

US_Database::US_Database(QWidget *w, Qt::WindowFlags flags) : US_Widgets(true, w, flags) {
   dbg_level = US_Settings::us_debug();
   uuid.clear();

   // Frame layout
   setPalette(US_GuiSettings::frameColor());

   setWindowTitle("Database Configuration");
   setAttribute(Qt::WA_DeleteOnClose);

   QByteArray currentHash = US_Settings::UltraScanPW();

   if (currentHash.isEmpty()) {
      QMessageBox::information(this, tr("No master password"), tr("The Master Password has not been set."));
      close();
   }

   // check if DA (com/acad) opened
   QString instance_socket = US_Settings::etcDir() + "/usinstance_";
   QInstances instances(instance_socket);
   qDebug() << "Active instances: " << instances.count();
   if (instances.count()) {
      QMessageBox::information(
         this, tr("Default Database Cannot be Changed!"),
         tr("Database preferences cannot be currently changed since there are Data Acquisition related processes "
            "ongoing. \n\nTo change DB preferences, Data Acquisiiton programs must be closed."));
      close();
   }


   QBoxLayout *topbox = new QVBoxLayout(this);
   topbox->setSpacing(2);

   // Set up the database list window
   QLabel *banner = us_banner(tr("Database List"));
   topbox->addWidget(banner);

   QLabel *banner2 = us_banner(tr("(Doubleclick for details and set the default)"), -1);
   topbox->addWidget(banner2);

   lw_entries = new QListWidget();
   lw_entries->setSortingEnabled(true);
   lw_entries->setPalette(US_GuiSettings::editColor());
   lw_entries->setFont(QFont(US_GuiSettings::fontFamily(), US_GuiSettings::fontSize()));
   topbox->addWidget(lw_entries);

   // Populate db_list
   QStringList DB = US_Settings::defaultDB();
   qDebug() << "USCFG: defDBl" << DB;
   QString defaultDB;
   if (!DB.isEmpty()) {
      defaultDB = US_Settings::defaultDB().at(0);
   }
   DbgLv(1) << "USCFG: defDBd" << defaultDB;
   update_lw(defaultDB);

   connect(lw_entries, SIGNAL(itemDoubleClicked(QListWidgetItem *)), SLOT(select_db(QListWidgetItem *)));
   // Detail info
   QLabel *info = us_banner(tr("Database Detailed  Information"));
   topbox->addWidget(info);

   int row = 0;
   QGridLayout *details = new QGridLayout();

   // Row 1
   QLabel *desc = us_label("Database Description:");
   details->addWidget(desc, row, 0);

   le_description = us_lineedit("", 0);
   details->addWidget(le_description, row++, 1);
   connect(le_description, SIGNAL(editingFinished()), this, SLOT(values_updated()));
   // Row 2
   QLabel *user = us_label("User Name:");
   details->addWidget(user, row, 0);

   le_username = us_lineedit("", 0);
   details->addWidget(le_username, row++, 1);
   connect(le_username, SIGNAL(editingFinished()), this, SLOT(values_updated()));
   // Row 3
   QLabel *password = us_label("DB Password:");
   details->addWidget(password, row, 0);

   le_password = us_lineedit("", 0);
   le_password->setEchoMode(QLineEdit::Password);
   details->addWidget(le_password, row++, 1);
   connect(le_password, SIGNAL(editingFinished()), this, SLOT(values_updated()));
   // Row 4
   QLabel *DBname = us_label("Database Name:");
   details->addWidget(DBname, row, 0);

   le_dbname = us_lineedit("", 0);
   details->addWidget(le_dbname, row++, 1);
   connect(le_dbname, SIGNAL(editingFinished()), this, SLOT(values_updated()));
   // Row 5
   QLabel *host = us_label("Host Address:");
   details->addWidget(host, row, 0);

   le_host = us_lineedit("", 0);
   details->addWidget(le_host, row++, 1);
   connect(le_host, SIGNAL(editingFinished()), this, SLOT(values_updated()));
   // Row 6
   QLabel *investigator = us_label("Investigator Email:");
   details->addWidget(investigator, row, 0);

   le_investigator_email = us_lineedit("", 0);

   // Make the line edit entries wider
   QFontMetrics fm(le_investigator_email->font());
   le_investigator_email->setMinimumWidth(fm.maxWidth() * 10);

   details->addWidget(le_investigator_email, row++, 1);
   connect(le_investigator_email, SIGNAL(editingFinished()), this, SLOT(values_updated()));
   // Row 7
   QLabel *investigator_pw = us_label("Investigator Password:");
   details->addWidget(investigator_pw, row, 0);

   le_investigator_pw = us_lineedit("", 0);
   le_investigator_pw->setEchoMode(QLineEdit::Password);
   details->addWidget(le_investigator_pw, row, 1);
   connect(le_investigator_pw, SIGNAL(editingFinished()), this, SLOT(values_updated()));
   topbox->addLayout(details);

   //Pushbuttons
   row = 0;
   int col = 0;
   QGridLayout *buttons = new QGridLayout();
   pb_paste = us_pushbutton(tr("Paste"));
   connect(pb_paste, SIGNAL(clicked()), this, SLOT(paste_db()));
   buttons->addWidget(pb_paste, row, col++);
   pb_save = us_pushbutton(tr("Save Entry"));
   pb_save->setEnabled(false);
   connect(pb_save, SIGNAL(clicked()), this, SLOT(check_add()));
   buttons->addWidget(pb_save, row, col++);

   pb_delete = us_pushbutton(tr("Delete Current Entry"));
   pb_delete->setEnabled(false);
   connect(pb_delete, SIGNAL(clicked()), this, SLOT(deleteDB()));
   buttons->addWidget(pb_delete, row++, col++);

   pb_testConnect = us_pushbutton(tr("Test Database Connectivity"));
   connect(pb_testConnect, SIGNAL(clicked()), this, SLOT(test_connect()));
   buttons->addWidget(pb_testConnect, row, 0, 1, col);

   QHBoxLayout *std_buttons = new QHBoxLayout;

   pb_reset = us_pushbutton(tr("Reset"));
   connect(pb_reset, SIGNAL(clicked()), this, SLOT(reset()));
   std_buttons->addWidget(pb_reset);

   QPushButton *pb_help = us_pushbutton(tr("Help"));
   connect(pb_help, SIGNAL(clicked()), this, SLOT(help()));
   std_buttons->addWidget(pb_help);

   QPushButton *pb_cancel = us_pushbutton(tr("Close"));
   connect(pb_cancel, SIGNAL(clicked()), this, SLOT(close()));
   std_buttons->addWidget(pb_cancel);

   topbox->addLayout(buttons);
   topbox->addLayout(std_buttons);
   if (!lw_entries->selectedItems().isEmpty()) {
      select_db(lw_entries->selectedItems().at(0));
   }
}

// Query autoflow for # records
int US_Database::get_autoflow_records() {
   // Check DB connection
   US_Passwd pw;
   QString masterpw = pw.getPasswd();
   US_DB2 *db = new US_DB2(masterpw);

   int record_number = 0;

   if (db->lastErrno() != US_DB2::OK) {
      QMessageBox::warning(
         this, tr("Connection Problem"), tr("Read protocol: Could not connect to database \n") + db->lastError());
      return record_number;
   }

   QStringList qry;
   qry << "count_autoflow_records";

   record_number = db->functionQuery(qry);

   return record_number;
}

void US_Database::select_db(QListWidgetItem *entry) {
   // When this is run, we will always have a current dblist

   // Delete trailing (default) if that is present
   QString item = entry->text().remove(" (default)");
   DbgLv(1) << "USCFG: seldb: item" << item;

   // Get the master PW
   US_Passwd pw;
   QString master_pw = pw.getPasswd();
   QString email = dblist.at(0).at(6);
   if (email.isEmpty()) {
      for (const auto &jj: dblist) {
         if (!jj.at(6).isEmpty()) {
            email = jj.at(6);
            DbgLv(1) << "USCFG: seldb: jj email" << jj << email;
            break;
         }
      }
   }


   for (int i = 0; i < dblist.size(); i++) {
      if (item == dblist.at(i).at(0)) {
         if (dblist.at(i).at(6).isEmpty()) {
            //dblist.at( i ).replace( 6, email );
            QStringList db = dblist.at(i);
            db.replace(6, email);
            dblist.replace(i, db);
         }
         DbgLv(1) << "USCFG: def upd  i" << i;
         le_description->setText(item);
         le_username->setText(dblist.at(i).at(1));
         le_dbname->setText(dblist.at(i).at(2));
         le_host->setText(dblist.at(i).at(3));
         le_investigator_email->setText(dblist.at(i).at(6));

         // DB Logon PW
         QString cipher = dblist.at(i).at(4);
         QString iv = dblist.at(i).at(5);
         QString pw_string = US_Crypto::decrypt(cipher, master_pw, iv);
         le_password->setText(pw_string);

         // DB Internal PW
         cipher = dblist.at(i).at(7);
         iv = dblist.at(i).at(8);
         pw_string = US_Crypto::decrypt(cipher, master_pw, iv);
         le_investigator_pw->setText(pw_string);
         values_updated();
         uuid = dblist.at(i).at(9);
         //qDebug() << "USCFG: dbl 0,6,9"
         // << dblist.at(i).at(0) << dblist.at(i).at(6) << dblist.at(i).at(9);

         // Set the default DB and user for that DB
         US_Settings::set_defaultDB(dblist.at(i));

         update_inv();
         update_lw(item);

         QMessageBox::information(this, tr("Database Selected"), tr("The default database has been updated."));
         pb_save->setEnabled(true);
         pb_delete->setEnabled(true);

         break;
      }
   }
}

void US_Database::update_inv() {
   US_Passwd pw;
   US_DB2 db(pw.getPasswd());

   if (db.lastErrno() != US_DB2::OK) {
      //qDebug() << "USCFG: UpdInv: ERROR connect";
      QMessageBox::information(this, tr("Error"), tr("Error making the DB connection.\n"));

      return;
   }

   QStringList q("get_user_info");
   db.query(q);
   db.next();

   int ID = db.value(0).toInt();
   QString fname = db.value(1).toString();
   QString lname = db.value(2).toString();
   int level = db.value(5).toInt();
   //qDebug() << "USCFG: UpdInv: ID,name,lev" << ID << fname << lname << level;
   //if(ID<1) return;

   US_Settings::set_us_inv_name(lname + ", " + fname);
   US_Settings::set_us_inv_ID(ID);
   US_Settings::set_us_inv_level(level);
}

QString US_Database::validate_value(QLineEdit *line_edit, const QString &property) {
   QString value = line_edit->text();
   if (!property.contains("password")) {
      value = value.trimmed();
   }
   if (value.isEmpty()) {
      // create a red border
      line_edit->setStyleSheet("border: 2px solid red");
      return tr(qUtf8Printable("Please enter a " + property + "."));
   }
   if (value != line_edit->text()) {
      line_edit->setText(value);
   }
   // reset the style sheet of the button
   line_edit->setStyleSheet("");
   return "";
}

void US_Database::check_add() {
   QStringList problems;
   // Check that all fields have at least something
   QString status = validate_value(le_description, "description");
   if (!status.isEmpty()) {
      problems << status;
   }
   status = validate_value(le_username, "username");
   if (!status.isEmpty()) {
      problems << status;
   }
   status = validate_value(le_password, "password");
   if (!status.isEmpty()) {
      problems << status;
   }
   status = validate_value(le_dbname, "database name");
   if (!status.isEmpty()) {
      problems << status;
   }
   status = validate_value(le_host, "host address");
   if (!status.isEmpty()) {
      problems << status;
   }
   status = validate_value(le_investigator_email, "investigator email");
   if (!status.isEmpty()) {
      problems << status;
   }
   status = validate_value(le_investigator_pw, "investigator password");
   if (!status.isEmpty()) {
      problems << status;
   }
   if (!problems.isEmpty()) {
      QString text = "Please correct the following problems before saving:\n";
      text += problems.join("\n");
      QMessageBox::information(this, tr("Attention"), tr(qUtf8Printable(text)));
      return;
   }

   if (uuid.isEmpty()) {
      if (!test_connect()) {
         return;
      }
   }

   // Get the master PW
   US_Passwd pw;
   QString master_pw = pw.getPasswd();

   // Encrypt the DB and investigator passwords with the master password
   QString password = le_password->text();
   QStringList pw_list = US_Crypto::encrypt(password, master_pw);

   password = le_investigator_pw->text();
   QStringList inv_pw = US_Crypto::encrypt(password, master_pw);

   // Save the DB entry
   dblist = US_Settings::databases();
   bool updated = false;

   for (int i = 0; i < dblist.size(); i++) {
      QStringList db = dblist.at(i);
      if (db.at(0) == le_description->text()) {
         db.replace(1, le_username->text());
         db.replace(2, le_dbname->text());
         db.replace(3, le_host->text());
         db.replace(4, pw_list.at(0)); // Encrypted password
         db.replace(5, pw_list.at(1)); // Initialization vector
         db.replace(6, le_investigator_email->text());
         db.replace(7, inv_pw.at(0));
         db.replace(8, inv_pw.at(1));
         db.replace(9, uuid);

         dblist.replace(i, db);
         updated = true;
         break;
      }
   }

   if (!updated) {
      QStringList entry;
      entry << le_description->text() << le_username->text() << le_dbname->text() << le_host->text() << pw_list.at(0)
            << pw_list.at(1) << le_investigator_email->text() << inv_pw.at(0) << inv_pw.at(1) << uuid;

      dblist << entry;
   }

   // Update the list widget
   US_Settings::set_databases(dblist);
   update_lw(le_description->text());

   if (lw_entries->count() == 1) {
      save_default();
   }

   pb_save->setEnabled(true);
   pb_delete->setEnabled(true);
}

void US_Database::update_lw(const QString &current) {
   lw_entries->clear();

   dblist = US_Settings::databases();
   QStringList defaultDB = US_Settings::defaultDB();
   QString defaultDBname;
   //qDebug() << "USCFG: UpdLw: defaultDB" << defaultDB;
   //qDebug() << "USCFG: UpdLw:  current" << current;

   if (!defaultDB.empty()) {
      defaultDBname = defaultDB.at(0);
   }
   //qDebug() << "USCFG: UpdLw:  defaultDBname" << defaultDBname;

   for (const auto &i: dblist) {
      const QString &desc = i.at(0);
      QString display = desc;
      //qDebug() << "USCFG: UpdLw:   i desc email" << i << desc << dblist.at(i).at(6);

      if (desc == defaultDBname) {
         display.append(" (default)");
      }
      //qDebug() << "USCFG: UpdLw:      display" << display;

      QListWidgetItem *widget = new QListWidgetItem(display);
      lw_entries->addItem(widget);

      if (desc == current) {
         lw_entries->setCurrentItem(widget, QItemSelectionModel::Select);
      }
   }
}

bool US_Database::parse_database_url(const QString &database_url) {
   if (database_url.isEmpty() || !database_url.startsWith("mysql://")) {
      return false;
   }
   // regular expression for "mysql://{$username}:{$password}@{$host}/{$dbname}|{$_SESSION['email']}"
   QRegularExpression rx;
   rx.setPattern(R"(mysql:\/\/([^:]+):([^@]*)@([^\/]+)\/(\w+)\|([\w@.\-\+]+))");
   QRegularExpressionMatch match = rx.match(database_url);
   if (!match.hasMatch()) {
      return false;
   }
   QString username = match.captured(1).trimmed();
   QString password = match.captured(2).trimmed();
   QString host = match.captured(3).trimmed();
   QString dbname = match.captured(4).trimmed();
   QString email = match.captured(5).trimmed();
   if (le_description->text().trimmed().isEmpty() && !host.isEmpty() && !dbname.isEmpty()) {
      le_description->setText(host + " - " + dbname);
   }
   if (le_password->text().trimmed().isEmpty() && !password.isEmpty()) {
      le_password->setText(password);
   }
   le_username->setText(username);
   le_dbname->setText(dbname);
   le_host->setText(host);
   le_investigator_email->setText(email);
   // validate input fields
   values_updated();
   return true;
}

void US_Database::reset() {
   QStringList DB = US_Settings::defaultDB();
   QString defaultDB;
   if (!DB.isEmpty()) {
      defaultDB = US_Settings::defaultDB().at(0);
   }
   update_lw(defaultDB);

   le_description->setText("");
   le_username->setText("");
   le_password->setText("");
   le_dbname->setText("");
   le_host->setText("");
   le_investigator_email->setText("");
   le_investigator_pw->setText("");
   uuid = "";

   pb_save->setEnabled(false);
   pb_delete->setEnabled(false);
}

void US_Database::help() {
   US_Help *showhelp = new US_Help(this);
   showhelp->show_help("database_config.html");
}

void US_Database::save_default() {
   for (const auto &i: dblist) {
      const QString &desc = i.at(0);
      DbgLv(1) << "USCFG: svDef: desc" << desc;

      // Look for the current description
      if (desc == le_description->text()) {
         DbgLv(1) << "USCFG: svDef:  desc MATCH i" << i;
         if (i.at(9) == "") {
            DbgLv(1) << "USCFG: svDef:  e9(uuid)==NULL" << i;
            QMessageBox::information(
               this, tr("Default Database Problem"),
               tr("The current database information has not been tested "
                  "for connectivity.\n"
                  "The default database has not been updated."));
            return;
         }
         if (i.at(6).isEmpty()) {
            DbgLv(1) << "USCFG: svDef:  e6(email)==NULL" << i;
         }

         US_Settings::set_defaultDB(i);
         update_inv();
         reset();

         QMessageBox::information(this, tr("Default Database"), tr("The default database has been updated."));
         return;
      }
   }

   QMessageBox::warning(
      this, tr("Default Database Problem"),
      tr("The description does not match any in the database list.\n"
         "The default database has not been updated."));
}

void US_Database::deleteDB() {
   QString item = le_description->text();

   // Go through the list and delete the one matching description
   for (int i = 0; i < dblist.size(); i++) {
      QString desc = dblist.at(i).at(0);

      // Look for the current description
      if (desc == item) {
         dblist.removeAt(i);
         US_Settings::set_databases(dblist);

         // Check if the default DB matches
         QStringList defaultDB = US_Settings::defaultDB();

         if (defaultDB.at(0) == item) {
            if (!dblist.isEmpty()) {
               US_Settings::set_defaultDB(dblist.at(0));
            }
            else {
               US_Settings::set_defaultDB(QStringList());
            }
         }

         reset();

         QMessageBox::information(this, tr("Database Removed"), tr("The database has been removed."));
         return;
      }
   }

   QMessageBox::warning(
      this, tr("Database Problem"),
      tr("The description does not match any in the database list.\n"
         "The database has not been removed."));
}

void US_Database::paste_db() {
   // Get the clipboard
   QClipboard *clipboard = QApplication::clipboard();
   QString text = clipboard->text();
   bool ok = parse_database_url(text);
   if (!ok) {
      QMessageBox::warning(
         this, tr("Database Problem"),
         tr(qUtf8Printable(
            "The database URL could not be parsed.\n"
            "Database URL: "
            + text)));
   }
}

void US_Database::values_updated() const {
   QStringList problems;
   problems << validate_value(le_description, "description");
   problems << validate_value(le_username, "database username");
   problems << validate_value(le_password, "database password");
   problems << validate_value(le_dbname, "database name");
   problems << validate_value(le_host, "database host address");
   problems << validate_value(le_investigator_email, "investigator email");
   problems << validate_value(le_investigator_pw, "investigator password");
   qDebug() << "USCFG: values_updated: status" << problems.join("\n");
}

bool US_Database::test_connect() {
   if (
      le_host->text().isEmpty() || le_dbname->text().isEmpty() || le_username->text().isEmpty()
      || le_password->text().isEmpty() || le_investigator_email->text().isEmpty()
      || le_investigator_pw->text().isEmpty()) {
      QMessageBox::warning(this, tr("Missing Data"), tr("Please fill in all fields before testing the connection."));

      return false;
   }

   QString error;
   US_DB2 db;
   //qDebug() << "USCFG: call test_secure...  db" << &db;
   bool ok = db.test_secure_connection(
      le_host->text(), le_dbname->text(), le_username->text(), le_password->text(), le_investigator_email->text(),
      le_investigator_pw->text(), error);
   //qDebug() << "USCFG:  ok=" << ok;
   if (ok) {
      uuid = db.value(0).toString(); // Set class variable uuid
      //qDebug() << "USCFG:  uuid=" << uuid;
      if (uuid.isEmpty() || uuid.length() != 36) {
         uuid = "00000000-0000-0000-0000-000000000000";
         //qDebug() << "USCFG:   2)uuid=" << uuid;
      }

      // Make sure the uuid is updated in the dblist structure
      for (int i = 0; i < dblist.size(); i++) {
         QString desc = dblist.at(i).at(0).trimmed();

         // Look for the current description
         if (desc == le_description->text().trimmed()) {
            QStringList list = dblist.at(i);
            list.replace(9, uuid);
            dblist.replace(i, list);
            US_Settings::set_databases(dblist);
            break;
         }
      }

      QMessageBox::information(this, tr("Database Connection"), tr("The connection was successful."));

      pb_save->setEnabled(true);
   }
   else {
      //qDebug() << "USCFG:  FAIL error=" << error;
      //qDebug() << "USCFG:   host    :" << le_host->text();
      //qDebug() << "USCFG:   dbname  :" << le_dbname->text();
      //qDebug() << "USCFG:   username:" << le_username->text();
      //qDebug() << "USCFG:   password:" << le_password->text();
      //qDebug() << "USCFG:   email   :" << le_investigator_email->text();
      //qDebug() << "USCFG:   ipassw  :" << le_investigator_pw->text();
      QMessageBox::warning(this, tr("Database Connection"), tr("The connection failed.\n") + error);
   }

   return ok;
}
