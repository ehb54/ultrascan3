#include "../include/us_admin.h"
#include "../include/us_encryption.h"

//! Constructor
/*! 
  Constractor a new <var>US_Admin</var> interface,
  \param *temp_permission Boolean point variable for deleting permission. 
  \param p Parent widget.
  \param name Widget name.
*/  
US_Admin::US_Admin(QWidget *p, const char *name) : QFrame( p, name)
{
  int buttonh = 26;
  
  USglobal = new US_Config();
  setPalette(QPalette(USglobal->global_colors.cg_frame, 
                      USglobal->global_colors.cg_frame, 
                      USglobal->global_colors.cg_frame));
  
  lbl_blank = new QLabel(tr(" Change Administrator Password:"), this);
  lbl_blank->setAlignment(AlignCenter|AlignVCenter);
  lbl_blank->setFrameStyle(QFrame::WinPanel|Raised);  
  lbl_blank->setPalette(QPalette(USglobal->global_colors.cg_frame, 
                                 USglobal->global_colors.cg_frame, 
                                 USglobal->global_colors.cg_frame));
  
  lbl_blank->setMinimumHeight( buttonh * 3 / 2 );
  
  lbl_blank->setFont(QFont(USglobal->config_list.fontFamily, 
                           USglobal->config_list.fontSize, 
                           QFont::Bold));
    
  lbl_psswd1 = new QLabel(tr(" Enter New Password:"),this);
  lbl_psswd1->setAlignment(AlignLeft|AlignVCenter);
  lbl_psswd1->setPalette(QPalette(USglobal->global_colors.cg_label, 
                                  USglobal->global_colors.cg_label, 
                                  USglobal->global_colors.cg_label));
  lbl_psswd1->setMinimumHeight(buttonh);
  lbl_psswd1->setFont(QFont(USglobal->config_list.fontFamily, 
                            USglobal->config_list.fontSize - 1, 
                            QFont::Bold));
    
  le_psswd1 = new QLineEdit("",this);
  le_psswd1->setPalette(QPalette(USglobal->global_colors.cg_edit, 
                                 USglobal->global_colors.cg_edit, 
                                 USglobal->global_colors.cg_edit));
  le_psswd1->setFont(QFont(USglobal->config_list.fontFamily, 
                           USglobal->config_list.fontSize - 1, 
                           QFont::Bold));
  le_psswd1->setEchoMode(QLineEdit::Password);
  le_psswd1->setMinimumHeight(buttonh);
  
  connect (le_psswd1, SIGNAL( textChanged(   const QString& ) ), 
                      SLOT  ( update_psswd1( const QString& ) ) );
  
  lbl_psswd2 = new QLabel(tr(" Verify New Password:"),this);
  lbl_psswd2->setAlignment(AlignLeft|AlignVCenter);
  lbl_psswd2->setPalette(QPalette(USglobal->global_colors.cg_label, 
                                  USglobal->global_colors.cg_label, 
                                  USglobal->global_colors.cg_label));
  lbl_psswd2->setMinimumHeight(buttonh);
  lbl_psswd2->setFont(QFont(USglobal->config_list.fontFamily, 
                            USglobal->config_list.fontSize - 1, 
                            QFont::Bold));
  
  le_psswd2 = new QLineEdit("",this);
  le_psswd2->setPalette(QPalette(USglobal->global_colors.cg_edit, 
                                 USglobal->global_colors.cg_edit, 
                                 USglobal->global_colors.cg_edit));
  le_psswd2->setFont(QFont(USglobal->config_list.fontFamily, 
                           USglobal->config_list.fontSize - 1, 
                           QFont::Bold));
  le_psswd2->setMinimumHeight(buttonh);
  le_psswd2->setEchoMode(QLineEdit::Password);
  connect (le_psswd2, SIGNAL( textChanged(   const QString& ) ), 
                      SLOT  ( update_psswd2( const QString& ) ) );
  
  
  pb_help = new QPushButton(tr("Help"), this);
  pb_help->setAutoDefault(false);
  pb_help->setFont(QFont( USglobal->config_list.fontFamily, 
                          USglobal->config_list.fontSize));
  pb_help->setPalette(QPalette(USglobal->global_colors.cg_pushb, 
                               USglobal->global_colors.cg_pushb_disabled, 
                               USglobal->global_colors.cg_pushb_active));
  pb_help->setMinimumHeight(buttonh);
  pb_help->setEnabled(true);
  connect(pb_help, SIGNAL(clicked()), SLOT(help()));
  
  pb_save = new QPushButton(tr("Save"), this);
  pb_save->setAutoDefault(false);
  pb_save->setFont(QFont( USglobal->config_list.fontFamily, 
                          USglobal->config_list.fontSize));
  pb_save->setPalette(QPalette(USglobal->global_colors.cg_pushb, 
                               USglobal->global_colors.cg_pushb_disabled, 
                               USglobal->global_colors.cg_pushb_active));
  pb_save->setMinimumHeight(buttonh); pb_save->setEnabled(true);
  connect(pb_save, SIGNAL(clicked()), SLOT(save()));
          
  pb_cancel = new QPushButton(tr("Cancel"), this);
  pb_cancel->setAutoDefault(false);
  pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, 
                            USglobal->config_list.fontSize));
  pb_cancel->setPalette(QPalette(USglobal->global_colors.cg_pushb, 
                                 USglobal->global_colors.cg_pushb_disabled, 
                                 USglobal->global_colors.cg_pushb_active));
  pb_cancel->setMinimumHeight(buttonh);
  pb_cancel->setEnabled(true);
  connect(pb_cancel, SIGNAL(clicked()), SLOT(quit()));

  QGridLayout* background = new QGridLayout( this, 3, 2, 4 );
  background->setColSpacing(0,200);
  background->setColSpacing(1,200);
  background->addMultiCellWidget(lbl_blank,0,0,0,1);
  background->addWidget(lbl_psswd1,1,0);
  background->addWidget(le_psswd1,1,1);
  background->addWidget(lbl_psswd2,2,0);
  background->addWidget(le_psswd2,2,1);
  
  QHBoxLayout* buttonLine = new QHBoxLayout(4);
  buttonLine->addWidget(pb_help);
  buttonLine->addWidget(pb_save);
  buttonLine->addWidget(pb_cancel);
  
  background->addMultiCellLayout(buttonLine,3,3,0,1);
}

//! Destructor
/*! destroy the <var>US_Admin</var>. */
US_Admin::~US_Admin()
{
}

/*!
  Update <var>password1</var> by LineEdit input.
*/
void US_Admin::update_psswd1( const QString& newText )
{
  password1 = newText;
}

/*!
  Update <var>password2</var> by LineEdit input.
*/
void US_Admin::update_psswd2( const QString& newText )
{
  password2 = newText;
}

/*!
  Write new password to password file.
*/
void US_Admin::save()
{
  if ( QString::compare(password1, password2) != 0 )
  {
    le_psswd1->setText("");
    le_psswd2->setText("");

    QMessageBox::message(
        tr("Attention:"), 
        tr("The entered passwords are not same, please type again.\n"));
    return;
  }

  QString pdfile = QDir::convertSeparators(
     USglobal->config_list.system_dir + "/etc/db_admin.pd" );

  FILE* fh;

  if ( ( fh = fopen( pdfile, "w" ) ) == NULL ) 
  {
    QMessageBox::message(
        tr("Attention:"), 
        tr("You have no permission to change the adminstrator password.\n"));
    return;
  }

  QString md5string = US_Encryption::md5( password1 );
 
  fputs( md5string, fh ); 
  fclose( fh );

  quit();
}

/*!
  Open a browser to load help page.
*/
void US_Admin::help()
{
  US_Help* online_help; online_help = new US_Help( this );
  online_help->show_help("manual/administrator.html");
}

/*! Close the interface. */
void US_Admin::quit()
{
  close();
}

