//! \file us_admin.cpp
#include "us_admin.h"

#include "us_crypto.h"
#include "us_gui_settings.h"
#include "us_help.h"
#include "us_license_t.h"
#include "us_settings.h"

US_Admin::US_Admin(QWidget* w, Qt::WindowFlags flags)
    : US_Widgets(true, w, flags) {
  setWindowTitle("Change Master Password");
  setAttribute(Qt::WA_DeleteOnClose);

  int buttonh = 26;

  setPalette(US_GuiSettings::frameColor());

  QLabel* header = us_banner("Change Master Password");
  header->setMinimumHeight(buttonh * 3 / 2);

  QString oldPass = US_Settings::UltraScanPW();
  QLabel* oldPW = NULL;

  if (!oldPass.isEmpty()) {
    oldPW = us_label("Old Password:");
    oldPW->setMinimumHeight(buttonh);

    le_oldPasswd = us_lineedit("");
    le_oldPasswd->setEchoMode(QLineEdit::Password);
    le_oldPasswd->setMinimumHeight(buttonh);
  } else
    le_oldPasswd = NULL;

  QLabel* passwd1 = us_label("Enter New Password:");
  passwd1->setMinimumHeight(buttonh);

  le_passwd1 = us_lineedit("");
  le_passwd1->setEchoMode(QLineEdit::Password);
  le_passwd1->setMinimumHeight(buttonh);
  le_passwd1->setMinimumWidth(300);

  QLabel* passwd2 = us_label("Verify New Password:");
  passwd2->setMinimumHeight(buttonh);

  le_passwd2 = us_lineedit("");
  le_passwd2->setMinimumHeight(buttonh);
  le_passwd2->setEchoMode(QLineEdit::Password);

  pb_help = us_pushbutton("Help");
  pb_help->setMinimumHeight(buttonh);
  connect(pb_help, SIGNAL(clicked()), SLOT(help()));

  pb_save = us_pushbutton("Save");
  pb_save->setMinimumHeight(buttonh);
  connect(pb_save, SIGNAL(clicked()), SLOT(save()));

  pb_cancel = us_pushbutton("Close");
  pb_cancel->setMinimumHeight(buttonh);
  connect(pb_cancel, SIGNAL(clicked()), SLOT(close()));

  // Layout
  QGridLayout* passwords = new QGridLayout;
  passwords->addWidget(header, 0, 0, 1, 2);

  int row = 1;
  if (!oldPass.isEmpty()) {
    passwords->addWidget(oldPW, row, 0);
    passwords->addWidget(le_oldPasswd, row++, 1);
  }

  passwords->addWidget(passwd1, row, 0);
  passwords->addWidget(le_passwd1, row++, 1);
  passwords->addWidget(passwd2, row, 0);
  passwords->addWidget(le_passwd2, row++, 1);

  QHBoxLayout* buttonLine = new QHBoxLayout;
  buttonLine->addWidget(pb_help);
  buttonLine->addWidget(pb_save);
  buttonLine->addWidget(pb_cancel);

  QVBoxLayout* main = new QVBoxLayout;
  main->setSpacing(2);
  main->addLayout(passwords);
  main->addLayout(buttonLine);

  setLayout(main);
}

void US_Admin::save(void) {
  QByteArray oldPW = US_Settings::UltraScanPW();

  if (!oldPW.isEmpty()) {
    QByteArray calcsha1 = QCryptographicHash::hash(
        le_oldPasswd->text().toLatin1(), QCryptographicHash::Sha1);

    if (calcsha1 != oldPW) {
      QMessageBox::information(
          this, tr("Attention:"),
          tr("The old password is incorrect. Please re-input.\n"));
      return;
    }
  }

  if (le_passwd1->text() != le_passwd2->text()) {
    le_passwd1->setText("");
    le_passwd2->setText("");

    QMessageBox::information(
        this, tr("Attention:"),
        tr("The entered passwords are not same. Please re-input.\n"));
    return;
  }

  QString newPW = le_passwd1->text();

  if (newPW.isEmpty()) {
    QMessageBox::information(
        this, tr("Attention:"),
        tr("The new password cannot be empty. Please re-input.\n"));
    return;
  }

  /// Use settings

  QByteArray sha1string =
      QCryptographicHash::hash(newPW.toLatin1(), QCryptographicHash::Sha1);

  US_Settings::set_UltraScanPW(sha1string);

  // We need to reset any passwords in databases here
  // from le_oldPasswd->text()
  // to   le_passwd1->text()

  QStringList defaultDB = US_Settings::defaultDB();
  QString oldPass;

  if (le_oldPasswd != NULL) oldPass = le_oldPasswd->text();

  if (defaultDB.size() > 0) {
    // Decrypt with old password
    // 4 = cipher; 5 = initialization vector
    QString db_password =
        US_Crypto::decrypt(defaultDB.at(4), oldPass, defaultDB.at(5));

    // Encrypt with new password
    QStringList cipherText = US_Crypto::encrypt(db_password, newPW);

    defaultDB.replace(4, cipherText.at(0));
    defaultDB.replace(5, cipherText.at(1));

    US_Settings::set_defaultDB(defaultDB);
  }

  QList<QStringList> databases = US_Settings::databases();

  for (int i = 0; i < databases.size(); i++) {
    QStringList database = databases.at(i);

    QString db_password =
        US_Crypto::decrypt(database.at(4), oldPass, database.at(5));

    QStringList cipherText = US_Crypto::encrypt(db_password, newPW);

    database.replace(4, cipherText.at(0));
    database.replace(5, cipherText.at(1));

    databases.replace(i, database);
  }

  if (databases.size() > 0) US_Settings::set_databases(databases);

  g.setPasswd(newPW);
  close();
}

void US_Admin::help() {
  US_Help* online_help = new US_Help(this);
  online_help->show_help("manual/administrator.html");
}
