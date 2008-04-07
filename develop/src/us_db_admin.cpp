#include "../include/us_db_admin.h"
#include "../include/us_encryption.h"

//! Constructor
/*! 
  Constractor a new <var>US_DB_Admin</var> interface,
  \param password QString variable for checking permission. 
  \param p Parent widget.
  \param name Widget name.
*/  
US_DB_Admin::US_DB_Admin( QString temp_password, QWidget* p, const char* name) 
   : QFrame( p, name)
{
  int xpos, ypos, buttonw = 160,   buttonh = 26;
  
  password = temp_password;
  USglobal = new US_Config();
  setPalette(QPalette(USglobal->global_colors.cg_frame, 
                      USglobal->global_colors.cg_frame, 
                      USglobal->global_colors.cg_frame));
  
  xpos = border;
  ypos = border;  
  
  QString str;

  if ( password.isEmpty() )
  {
    str = tr( "Please input administrator password\n"
              "for getting permission!" );
  }
  else
  {
    str = tr( "Please input your database user password!" );
  }

  lbl_blank = new QLabel(str, this);
  lbl_blank->setAlignment(AlignCenter|AlignVCenter);
  lbl_blank->setFrameStyle(QFrame::WinPanel|Raised);  
  lbl_blank->setPalette(QPalette(USglobal->global_colors.cg_frame, 
                                 USglobal->global_colors.cg_frame, 
                                 USglobal->global_colors.cg_frame));
  lbl_blank->setGeometry(xpos, ypos, buttonw*2 + spacing, buttonh*2);
  lbl_blank->setFont(QFont(USglobal->config_list.fontFamily, 
                           USglobal->config_list.fontSize, 
                           QFont::Bold));

  xpos = border;
  ypos += buttonh*2 + buttonh/2;

  lbl_psswd = new QLabel(tr(" Password:"),this);
  lbl_psswd->setAlignment(AlignLeft|AlignVCenter);
  lbl_psswd->setPalette(QPalette(USglobal->global_colors.cg_label, 
                                 USglobal->global_colors.cg_label, 
                                 USglobal->global_colors.cg_label));
  lbl_psswd->setGeometry(xpos, ypos, buttonw, buttonh);
  lbl_psswd->setFont(QFont(USglobal->config_list.fontFamily, 
                           USglobal->config_list.fontSize - 1, 
                           QFont::Bold));
  
  xpos += buttonw + spacing;
  
  le_psswd = new QLineEdit("",this);
  le_psswd->setGeometry(xpos, ypos, buttonw, buttonh);
  le_psswd->setPalette(QPalette(USglobal->global_colors.cg_edit, 
                                USglobal->global_colors.cg_edit, 
                                USglobal->global_colors.cg_edit));
  le_psswd->setFont(QFont(USglobal->config_list.fontFamily, 
                          USglobal->config_list.fontSize - 1, 
                          QFont::Bold));
  le_psswd->setEchoMode(QLineEdit::Password);
  
  connect (le_psswd, SIGNAL(textChanged(const QString &)), 
                     SLOT(update_psswd(const QString &)));
  connect (le_psswd, SIGNAL(returnPressed()), 
                     SLOT(submit()));
  
  ypos += buttonh + buttonh/2;
  xpos = border;
  
  pb_submit = new QPushButton(tr("Submit"), this);
  pb_submit->setAutoDefault(false);
  pb_submit->setFont(QFont( USglobal->config_list.fontFamily, 
                            USglobal->config_list.fontSize));
  pb_submit->setPalette(QPalette(USglobal->global_colors.cg_pushb, 
                                 USglobal->global_colors.cg_pushb_disabled, 
                                 USglobal->global_colors.cg_pushb_active));
  pb_submit->setGeometry(xpos, ypos, buttonw, buttonh);
  pb_submit->setEnabled(true);
  connect(pb_submit, SIGNAL( clicked() ), 
                     SLOT  ( submit()  ) );
      
  xpos += buttonw + spacing;
    
  pb_cancel = new QPushButton(tr("Cancel"), this);
  pb_cancel->setAutoDefault(false);
  pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, 
                            USglobal->config_list.fontSize));
  pb_cancel->setPalette(QPalette(USglobal->global_colors.cg_pushb, 
                                 USglobal->global_colors.cg_pushb_disabled, 
                                 USglobal->global_colors.cg_pushb_active));
  pb_cancel->setGeometry(xpos, ypos, buttonw, buttonh);
  pb_cancel->setEnabled(true);
  connect( pb_cancel, SIGNAL( clicked() ), 
                      SLOT  ( quit()    ) );

  ypos += buttonh + buttonh/2;
  xpos  = 2 * border + 2 * buttonw + spacing;

  global_Xpos += 50;
  global_Ypos += 50;

  setMinimumSize(xpos, ypos); 
  setGeometry(global_Xpos, global_Ypos, xpos, ypos);
}

//! Destructor
/*! destroy the <var>US_DB_Admin</var>. */
US_DB_Admin::~US_DB_Admin()
{
}

/*!
  Update <var>Password</var> by LineEdit input.
*/
void US_DB_Admin::update_psswd( const QString& newText )
{
  input_password = newText;
}

/*!
  Read password file and compare with the input.
  If the password is correct, issue signal bool <var>permission</var>.
*/
void US_DB_Admin::submit()
{
  // For adminstrator password
  if ( password.isEmpty() )
  {
    QString pdfile = QDir::convertSeparators(
       USglobal->config_list.system_dir + "/etc/db_admin.pd" );
    
    QFile f( QDir::convertSeparators( pdfile ) );
    
    if ( ! f.exists() )
    {
      QMessageBox::message(
          tr("Attention:"), 
          tr("The data file with administator password info "
             "could not be found.\n"));
      return;           
    }

    // Read the pasword
    
#define HASHSIZE 33  // Add 1 for trailing null
    
    FILE* fh;
    char pw_hash[HASHSIZE];

    fh = fopen( pdfile, "r" );
    fgets( pw_hash, HASHSIZE, fh);
    fclose (fh );


    // Does password match?
    QString pw1_md5 = QString(pw_hash);
    QString pw2_md5 = US_Encryption::md5( input_password );

    if ( QString::compare( pw1_md5, pw2_md5 ) == 0 )
    {
      emit issue_permission( true );
      quit();
    }
    else
    {
      QMessageBox::message(
          tr( "Attention:" ), 
          tr( "Your input password is incorrect.\n") );
      return;     
    } 
  }

  // For database selection password, encrypt the input password
  else
  {
    QString pw1_md5 = US_Encryption::md5( input_password );
  
    if ( QString::compare( pw1_md5, password) == 0 )
    {
      emit issue_pass( input_password );
      quit();
    }

    else
    {
      QMessageBox::message(
          tr("Attention:"), 
          tr("Your input password is incorrect.\n"));
      return;     
    }
  } 
}

/*! Close the interface. */
void US_DB_Admin::quit()
{
  close();
}

