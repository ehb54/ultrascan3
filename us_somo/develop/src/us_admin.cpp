#include "../include/us_admin.h"

#include "../include/us3_defines.h"
#include "../include/us_encryption.h"
// Added by qt3to4:
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>

//! Constructor
/*!
  Constractor a new <var>US_Admin</var> interface,
  \param *temp_permission Boolean point variable for deleting permission.
  \param p Parent widget.
  \param name Widget name.
*/
US_Admin::US_Admin(QWidget* p, const char*) : QFrame(p) {
  int buttonh = 26;

  USglobal = new US_Config();
  setPalette(PALET_FRAME);

  lbl_blank = new QLabel(us_tr(" Change Administrator Password:"), this);
  lbl_blank->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  lbl_blank->setFrameStyle(QFrame::WinPanel | Raised);
  lbl_blank->setPalette(PALET_FRAME);
  AUTFBACK(lbl_blank);

  lbl_blank->setMinimumHeight(buttonh * 3 / 2);

  lbl_blank->setFont(QFont(USglobal->config_list.fontFamily,
                           USglobal->config_list.fontSize, QFont::Bold));

  lbl_psswd1 = new QLabel(us_tr(" Enter New Password:"), this);
  lbl_psswd1->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_psswd1->setPalette(PALET_LABEL);
  AUTFBACK(lbl_psswd1);
  lbl_psswd1->setMinimumHeight(buttonh);
  lbl_psswd1->setFont(QFont(USglobal->config_list.fontFamily,
                            USglobal->config_list.fontSize - 1, QFont::Bold));

  le_psswd1 = new QLineEdit("", this);
  le_psswd1->setPalette(PALET_EDIT);
  AUTFBACK(le_psswd1);
  le_psswd1->setFont(QFont(USglobal->config_list.fontFamily,
                           USglobal->config_list.fontSize - 1, QFont::Bold));
  le_psswd1->setEchoMode(QLineEdit::Password);
  le_psswd1->setMinimumHeight(buttonh);

  connect(le_psswd1, SIGNAL(textChanged(const QString&)),
          SLOT(update_psswd1(const QString&)));

  lbl_psswd2 = new QLabel(us_tr(" Verify New Password:"), this);
  lbl_psswd2->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_psswd2->setPalette(PALET_LABEL);
  AUTFBACK(lbl_psswd2);
  lbl_psswd2->setMinimumHeight(buttonh);
  lbl_psswd2->setFont(QFont(USglobal->config_list.fontFamily,
                            USglobal->config_list.fontSize - 1, QFont::Bold));

  le_psswd2 = new QLineEdit("", this);
  le_psswd2->setPalette(PALET_EDIT);
  AUTFBACK(le_psswd2);
  le_psswd2->setFont(QFont(USglobal->config_list.fontFamily,
                           USglobal->config_list.fontSize - 1, QFont::Bold));
  le_psswd2->setMinimumHeight(buttonh);
  le_psswd2->setEchoMode(QLineEdit::Password);
  connect(le_psswd2, SIGNAL(textChanged(const QString&)),
          SLOT(update_psswd2(const QString&)));

  pb_help = new QPushButton(us_tr("Help"), this);
  pb_help->setAutoDefault(false);
  pb_help->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  pb_help->setPalette(PALET_PUSHB);
  pb_help->setMinimumHeight(buttonh);
  pb_help->setEnabled(true);
  connect(pb_help, SIGNAL(clicked()), SLOT(help()));

  pb_save = new QPushButton(us_tr("Save"), this);
  pb_save->setAutoDefault(false);
  pb_save->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  pb_save->setPalette(PALET_PUSHB);
  pb_save->setMinimumHeight(buttonh);
  pb_save->setEnabled(true);
  connect(pb_save, SIGNAL(clicked()), SLOT(save()));

  pb_cancel = new QPushButton(us_tr("Cancel"), this);
  pb_cancel->setAutoDefault(false);
  pb_cancel->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  pb_cancel->setPalette(PALET_PUSHB);
  pb_cancel->setMinimumHeight(buttonh);
  pb_cancel->setEnabled(true);
  connect(pb_cancel, SIGNAL(clicked()), SLOT(quit()));

  QGridLayout* background = new QGridLayout(this);
  background->setContentsMargins(0, 0, 0, 0);
  background->setSpacing(0);
  background->setSpacing(4);
  background->setContentsMargins(4, 4, 4, 4);
  background->setColumnMinimumWidth(0, 200);
  background->setColumnMinimumWidth(1, 200);
  background->addWidget(lbl_blank, 0, 0, 1 + (0) - (0), 1 + (1) - (0));
  background->addWidget(lbl_psswd1, 1, 0);
  background->addWidget(le_psswd1, 1, 1);
  background->addWidget(lbl_psswd2, 2, 0);
  background->addWidget(le_psswd2, 2, 1);

  QHBoxLayout* buttonLine = new QHBoxLayout();
  buttonLine->setContentsMargins(0, 0, 0, 0);
  buttonLine->setSpacing(0);
  buttonLine->addWidget(pb_help);
  buttonLine->addWidget(pb_save);
  buttonLine->addWidget(pb_cancel);

  background->addLayout(buttonLine, 3, 0, 1 + (3) - (3), 1 + (1) - (0));
}

//! Destructor
/*! destroy the <var>US_Admin</var>. */
US_Admin::~US_Admin() {}

/*!
  Update <var>password1</var> by LineEdit input.
*/
void US_Admin::update_psswd1(const QString& newText) { password1 = newText; }

/*!
  Update <var>password2</var> by LineEdit input.
*/
void US_Admin::update_psswd2(const QString& newText) { password2 = newText; }

/*!
  Write new password to password file.
*/
void US_Admin::save() {
  if (QString::compare(password1, password2) != 0) {
    le_psswd1->setText("");
    le_psswd2->setText("");

    US_Static::us_message(
        us_tr("Attention:"),
        us_tr("The entered passwords are not same, please type again.\n"));
    return;
  }

  QString pdfile = QDir::toNativeSeparators(USglobal->config_list.system_dir +
                                            "/etc/db_admin.pd");

  FILE* fh;

  if ((fh = us_fopen(pdfile, "w")) == NULL) {
    US_Static::us_message(
        us_tr("Attention:"),
        us_tr("You have no permission to change the adminstrator password.\n"));
    return;
  }

  QString md5string = US_Encryption::md5(password1);

  fputs(qPrintable(md5string), fh);
  fclose(fh);

  quit();
}

/*!
  Open a browser to load help page.
*/
void US_Admin::help() {
  US_Help* online_help;
  online_help = new US_Help(this);
  online_help->show_help("manual/administrator.html");
}

/*! Close the interface. */
void US_Admin::quit() { close(); }
