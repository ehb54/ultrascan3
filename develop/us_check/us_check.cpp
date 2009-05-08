#include "us_check.h"

Confirm::Confirm( QWidget *parent, 
                  const char *name) : QDialog( parent, name, true )
{
   int xpos, ypos,spacing, buttonh, buttonw;
   
   xpos = 4;
   ypos = 4;
   buttonh = 30;
   buttonw = 150;
   spacing = 4;
   db_info.username = "";
   db_info.password = "";
   db_info.host = "";
   lbl_bar = new QLabel("Input the Database Parameter :", this);
   lbl_bar->setAlignment(AlignCenter|AlignVCenter);
   lbl_bar->setGeometry(xpos,ypos,buttonw*2+spacing*3,buttonh);

   xpos = 4;
   ypos += buttonh+spacing;
   
   QLabel *lbl_username;
   lbl_username = new QLabel(tr(" User Name:"),this);
   lbl_username->setAlignment(AlignLeft|AlignVCenter);
   lbl_username->setGeometry(xpos, ypos, buttonw, buttonh);
   
   xpos += buttonw;
   
   QLineEdit *le_username;
   le_username = new QLineEdit(this, "username");
   le_username->setGeometry(xpos, ypos, buttonw, buttonh);
   connect (le_username, SIGNAL(textChanged(const QString &)), SLOT(update_username(const QString &)));

   xpos = 4;
   ypos += buttonh + spacing;
   
   QLabel *lbl_password;
   lbl_password = new QLabel(tr(" Password:"),this);
   lbl_password->setAlignment(AlignLeft|AlignVCenter);
   lbl_password->setGeometry(xpos, ypos, buttonw, buttonh);
   
   xpos += buttonw;
   
   QLineEdit *le_password;
   le_password = new QLineEdit(this, "password");
   le_password->setGeometry(xpos, ypos, buttonw, buttonh);
   le_password->setEchoMode(QLineEdit::Password);
   connect (le_password, SIGNAL(textChanged(const QString &)), SLOT(update_password(const QString &)));

   xpos = 4;
   ypos += buttonh + spacing;

   QLabel *lbl_host;
   lbl_host = new QLabel(tr(" Host Address:"),this);
   lbl_host->setAlignment(AlignLeft|AlignVCenter);
   lbl_host->setGeometry(xpos, ypos, buttonw, buttonh);

   xpos += buttonw;
   
   QLineEdit *le_host;
   le_host = new QLineEdit(this, "host");
   le_host->setGeometry(xpos, ypos, buttonw, buttonh);
   connect (le_host, SIGNAL(textChanged(const QString &)), SLOT(update_host(const QString &)));
   
   xpos = 4;
   ypos += buttonh + spacing*2;
   
   ok = new QPushButton( tr("Ok"), this );
   ok->setAutoDefault(false);
   ok->setGeometry( xpos,ypos, buttonw - spacing*2,buttonh );
   connect( ok, SIGNAL(clicked()), SLOT(accept()) );

   xpos += buttonw + spacing*2;
   cancel = new QPushButton( tr("Cancel"), this );
   cancel->setAutoDefault(false);
   cancel->setGeometry( xpos,ypos, buttonw - spacing*2,buttonh );
   connect( cancel, SIGNAL(clicked()), SLOT(reject()) );

}
Confirm::~Confirm()
{
   delete cancel;
   delete ok;   
}

void Confirm::update_username(const QString &newText)
{
   db_info.username = newText;
}

void Confirm::update_password(const QString &newText)
{
   db_info.password = newText;
}

void Confirm::update_host(const QString &newText)
{
   db_info.host = newText;
}

void Confirm::accept()
{
   db_connect();
   close();
}
int Confirm::db_connect()
{
   QSqlDatabase *defaultDB = QSqlDatabase::addDatabase("QMYSQL3" );
   defaultDB->setDatabaseName( "us_register" );   
   defaultDB->setUserName(db_info.username );
   defaultDB->setPassword(db_info.password );
   defaultDB->setHostName( db_info.host );
   if ( ! defaultDB ) 
   {
      QMessageBox::message("Attention:", 
                           "Fail to connect MySQL Driver");
      
      return -1;
   }
   /*
     defaultDB->setUserName("us_register" );
     defaultDB->setPassword( "thr33dogdown" );
     defaultDB->setHostName( "biochem.uthscsa.edu" );

     defaultDB->setUserName("us_register" );
     defaultDB->setPassword( "ultrascan" );
     defaultDB->setHostName( "192.168.0.1" );
   */
   if ( ! defaultDB->open() )
   {
      QString str = "Failed to open database: " + defaultDB->lastError().databaseText();
      QMessageBox::message("Attention:", str);
      return -2;
   }
   return 0;
}

US_Check::US_Check( QWidget *p, const char *name) : QFrame( p, name)
{
   int xpos, ypos, buttonw = 420,  buttonh = 30;
   int border = 4, spacing =2;
   
   input_flag = false;   // If you type in LineEdit, set it to true.
   sel_key = false;      //use for query data listbox select
   sel_name = false;      //use for show all name
   show_flag = false;   //use for control 'show info' pushbutton
   export_flag = false; // use for control when export printable info

   input = "";
   Key = "";
   input_type = 0;
   RID = Rid = 0;
   Lid = 0;
   QColor  background_color=Qt::lightGray, highlight_color=Qt::black;
   
   setPalette(QPalette(background_color));
   confirmDlg = new Confirm();
   confirmDlg->show();
   //db_connect();
         
   xpos = border;
   ypos = border;   
   
   lbl_blank = new QLabel("Check Registration from DB: ", this);
   lbl_blank->setAlignment(AlignCenter|AlignVCenter);
   lbl_blank->setFrameStyle(QFrame::WinPanel|Raised);
   lbl_blank->setPalette(background_color);
   lbl_blank->setGeometry(xpos, ypos, buttonw, buttonh*2-15);
      
   xpos = border;
   ypos += buttonh*2-15 + spacing;

   lbl_type = new QLabel(" Choose Check Type:",this);
   lbl_type->setAlignment(AlignLeft|AlignVCenter);
   lbl_type->setFrameStyle(QFrame::WinPanel|Raised);
   lbl_type->setPalette(QPalette(black));
   lbl_type->setGeometry(xpos, ypos, buttonw/2, buttonh);
   
   xpos += buttonw/2;
   cmbb_type = new QComboBox(false, this, " ");
   cmbb_type->setPalette( QPalette(gray));
   cmbb_type->setGeometry(xpos, ypos, buttonw/2, buttonh);
   cmbb_type->insertItem("not selected");
   cmbb_type->insertItem("Lastname");
   cmbb_type->insertItem("Organization");
   cmbb_type->insertItem("Zipcode");
   cmbb_type->insertItem("Email");
   cmbb_type->insertItem("Show All");
   connect(cmbb_type, SIGNAL(activated(int)), SLOT(sel_type(int)));
   
   xpos = border;
   ypos += buttonh + spacing;
   
   lbl_input = new QLabel(" Choose check type first! ",this);
   lbl_input->setAlignment(AlignLeft|AlignVCenter);
   lbl_input->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_input->setPalette( QPalette(black));
   lbl_input->setGeometry(xpos, ypos, buttonw/2, buttonh);
   
   xpos += buttonw/2;
   
   le_input = new QLineEdit(this, "");
   le_input->setGeometry(xpos, ypos, buttonw/2, buttonh);
   le_input->setPalette(QPalette(white));
   le_input->setEnabled(false);
   connect (le_input, SIGNAL(textChanged(const QString &)), SLOT(update_input(const QString &)));

   xpos = border;
   ypos += buttonh + spacing;
   
   pb_reset = new QPushButton("Reset", this);
   pb_reset->setAutoDefault(false);
   pb_reset->setPalette(QPalette(cyan));
   pb_reset->setGeometry(xpos, ypos, buttonw/2, buttonh);
   pb_reset->setEnabled(true);
   connect(pb_reset, SIGNAL(clicked()), SLOT(reset()));

   xpos += buttonw/2 + spacing;
   
   pb_chkLC = new QPushButton("Check License", this);
   pb_chkLC->setAutoDefault(false);
   pb_chkLC->setPalette(QPalette( cyan));
   pb_chkLC->setGeometry(xpos, ypos, buttonw/2, buttonh);
   pb_chkLC->setEnabled(true);
   connect(pb_chkLC, SIGNAL(clicked()), SLOT(checklicense()));
   
   xpos = border;
   ypos += buttonh;
   
   lbl_instr1 = new QLabel("Doubleclick on item to select:",this);
   lbl_instr1->setAlignment(AlignCenter|AlignVCenter);
   lbl_instr1->setPalette( QPalette(background_color));
   lbl_instr1->setGeometry(xpos, ypos, buttonw, buttonh);

   xpos = border;
   ypos += buttonh;
   
   lb_key = new QListBox(this, "license key");
   lb_key->setPalette(QPalette(white));
   lb_key->setGeometry(xpos, ypos, buttonw, buttonh*6+ spacing *6);
   lb_key->setSelected(0, true);
   connect(lb_key, SIGNAL(selected(int)), SLOT(select_name(int)));
   connect(lb_key, SIGNAL(selected(int)), SLOT(select_key(int)));
   
   xpos = border;
   ypos += buttonh*6 + 8*spacing;

   lbl_license = new QLabel("License Key: not selected",this);
   lbl_license->setAlignment(AlignLeft|AlignVCenter);
   lbl_license->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_license->setPalette( QPalette(black));
   lbl_license->setGeometry(xpos, ypos, buttonw, buttonh);

   xpos = border;
   ypos += buttonh + 2*spacing;
   
   pb_delete = new QPushButton("Delete License", this);
   pb_delete->setAutoDefault(false);
   pb_delete->setPalette(QPalette(cyan));
   pb_delete->setGeometry(xpos, ypos, buttonw/2, buttonh);
   pb_delete->setEnabled(true);
   connect(pb_delete, SIGNAL(clicked()), SLOT(del()));
   
   xpos +=buttonw/2 +spacing;
   
   pb_chkID = new QPushButton("Show Info", this);
   pb_chkID->setAutoDefault(false);
   pb_chkID->setPalette(QPalette(cyan));
   pb_chkID->setGeometry(xpos, ypos, buttonw/2, buttonh);
   connect(pb_chkID, SIGNAL(clicked()), SLOT(show_info()));
   
   xpos = border;
   ypos += buttonh + spacing;
   
   pb_approve = new QPushButton("Approve License", this);
   pb_approve->setAutoDefault(false);
   pb_approve->setPalette(QPalette(cyan));
   pb_approve->setGeometry(xpos, ypos, buttonw/2, buttonh);
   pb_approve->setEnabled(true);
   connect(pb_approve, SIGNAL(clicked()), SLOT(approve()));
   
   xpos +=buttonw/2 +spacing;
   
   pb_close = new QPushButton("Close", this);
   pb_close->setAutoDefault(false);
   pb_close->setPalette(QPalette(cyan));
   pb_close->setGeometry(xpos, ypos, buttonw/2, buttonh);
   pb_close->setEnabled(true);
   connect(pb_close, SIGNAL(clicked()), SLOT(quit()));

   xpos = buttonw + border*4;
   ypos = border;
   
   lbl_info = new QLabel(" ", this);
   lbl_info->setAlignment(AlignCenter|AlignVCenter);
   lbl_info->setPalette(highlight_color);
   lbl_info->setGeometry(xpos, ypos, buttonw, buttonh*14+spacing*13-15);
   
   //xpos += border;
   //ypos += spacing;
   
   lbl_instr2 = new QLabel("License Registration",this);
   lbl_instr2->setAlignment(AlignCenter|AlignVCenter);
   lbl_instr2->setPalette( QPalette(background_color));
   lbl_instr2->setGeometry(xpos, ypos, buttonw, buttonh*2-15);
   
   xpos = buttonw + border*4;
   ypos += buttonh*2-15 +spacing;
   
   lbl_firstname = new QLabel(" First Name:",this);
   lbl_firstname->setAlignment(AlignLeft|AlignVCenter);
   lbl_firstname->setPalette(QPalette(background_color));
   lbl_firstname->setGeometry(xpos, ypos, buttonw/3, buttonh);
   
   xpos += buttonw/3;
   lbl_fname = new QLabel(" ",this);
   lbl_fname->setAlignment(AlignLeft|AlignVCenter);
   lbl_fname->setPalette(QPalette(background_color));
   lbl_fname->setGeometry(xpos, ypos, buttonw/3*2, buttonh);
      
   xpos = buttonw +4*border;
   ypos += buttonh + spacing;
   
   lbl_lastname = new QLabel(" Last Name:",this);
   lbl_lastname->setAlignment(AlignLeft|AlignVCenter);
   lbl_lastname->setPalette(QPalette(background_color));
   lbl_lastname->setGeometry(xpos, ypos, buttonw/3, buttonh);
   
   xpos += buttonw/3;
   
   lbl_lname = new QLabel(" ",this);
   lbl_lname->setAlignment(AlignLeft|AlignVCenter);
   lbl_lname->setPalette(QPalette(background_color));
   lbl_lname->setGeometry(xpos, ypos, buttonw/3*2, buttonh);

   xpos = buttonw +4*border;
   ypos += buttonh + spacing;
   
   lbl_organization = new QLabel(" Organization:",this);
   lbl_organization->setAlignment(AlignLeft|AlignVCenter);
   lbl_organization->setPalette(QPalette(background_color));
   lbl_organization->setGeometry(xpos, ypos, buttonw/3, buttonh);
   
   xpos += buttonw/3;
   
   lbl_org = new QLabel(" ",this);
   lbl_org->setAlignment(AlignLeft|AlignVCenter);
   lbl_org->setPalette(QPalette(background_color));
   lbl_org->setGeometry(xpos, ypos, buttonw/3*2, buttonh);
   
   xpos = buttonw +4*border;
   ypos += buttonh + spacing;
   
   lbl_address = new QLabel(" Address:",this);
   lbl_address->setAlignment(AlignLeft|AlignVCenter);
   lbl_address->setPalette(QPalette(background_color));
   lbl_address->setGeometry(xpos, ypos, buttonw/3, buttonh);
   
   xpos += buttonw/3;
   
   lbl_addr = new QLabel(" ",this);
   lbl_addr->setAlignment(AlignLeft|AlignVCenter);
   lbl_addr->setPalette(QPalette(background_color));
   lbl_addr->setGeometry(xpos, ypos, buttonw/3*2, buttonh);
   
   xpos = buttonw +4*border;
   ypos += buttonh + spacing;
   
   lbl_city = new QLabel(" City:",this);
   lbl_city->setAlignment(AlignLeft|AlignVCenter);
   lbl_city->setPalette(QPalette(background_color));
   lbl_city->setGeometry(xpos, ypos, buttonw/3, buttonh);
   
   xpos += buttonw/3;
   
   lbl_ct = new QLabel(" ",this);
   lbl_ct->setAlignment(AlignLeft|AlignVCenter);
   lbl_ct->setPalette(QPalette(background_color));
   lbl_ct->setGeometry(xpos, ypos, buttonw/3*2, buttonh);
   
   xpos = buttonw +4*border;
   ypos += buttonh + spacing;
   
   lbl_state = new QLabel(" State:",this);
   lbl_state->setAlignment(AlignLeft|AlignVCenter);
   lbl_state->setPalette(QPalette(background_color));
   lbl_state->setGeometry(xpos, ypos, buttonw/4, buttonh);
   
   xpos += buttonw/4;
   
   lbl_st = new QLabel(" ",this);
   lbl_st->setAlignment(AlignLeft|AlignVCenter);
   lbl_st->setPalette(QPalette(background_color));
   lbl_st->setGeometry(xpos, ypos, buttonw/4, buttonh);

   xpos += buttonw/4;

   lbl_zipcode = new QLabel(" Zip:",this);
   lbl_zipcode->setAlignment(AlignLeft|AlignVCenter);
   lbl_zipcode->setPalette(QPalette(background_color));
   lbl_zipcode->setGeometry(xpos, ypos, buttonw/4, buttonh);
   
   xpos += buttonw/4;
   
   lbl_zip = new QLabel(" ",this);
   lbl_zip->setAlignment(AlignLeft|AlignVCenter);
   lbl_zip->setPalette(QPalette(background_color));
   lbl_zip->setGeometry(xpos, ypos, buttonw/4, buttonh);
   
   xpos = buttonw +4*border;
   ypos += buttonh + spacing;
   
   lbl_telephone = new QLabel(" Phone:",this);
   lbl_telephone->setAlignment(AlignLeft|AlignVCenter);
   lbl_telephone->setPalette(QPalette(background_color));
   lbl_telephone->setGeometry(xpos, ypos, buttonw/3, buttonh);
   
   xpos += buttonw/3;
   
   lbl_phone = new QLabel(" ",this);
   lbl_phone->setAlignment(AlignLeft|AlignVCenter);
   lbl_phone->setPalette(QPalette(background_color));
   lbl_phone->setGeometry(xpos, ypos, buttonw/3*2, buttonh);

   xpos = buttonw +4*border;
   ypos += buttonh + spacing;
   
   lbl_emailaddr = new QLabel(" Email:",this);
   lbl_emailaddr->setAlignment(AlignLeft|AlignVCenter);
   lbl_emailaddr->setPalette(QPalette(background_color));
   lbl_emailaddr->setGeometry(xpos, ypos, buttonw/3, buttonh);
   
   xpos += buttonw/3;
   
   lbl_email = new QLabel(" ",this);
   lbl_email->setAlignment(AlignLeft|AlignVCenter);
   lbl_email->setPalette(QPalette(background_color));
   lbl_email->setGeometry(xpos, ypos, buttonw/3*2, buttonh);
   
   xpos = buttonw +4*border;
   ypos += buttonh + spacing;
   
   lbl_platform = new QLabel(" Platform:",this);
   lbl_platform->setAlignment(AlignLeft|AlignVCenter);
   lbl_platform->setPalette(QPalette(background_color));
   lbl_platform->setGeometry(xpos, ypos, buttonw/3, buttonh);
      
   xpos += buttonw/3;
   
   lbl_os = new QLabel(" ",this);
   lbl_os->setAlignment(AlignLeft|AlignVCenter);
   lbl_os->setPalette(QPalette(background_color));
   lbl_os->setGeometry(xpos, ypos, buttonw/3*2, buttonh);
   
   xpos = buttonw +4*border;
   ypos += buttonh + spacing;
   
   lbl_licensetype = new QLabel(" License Type:",this);
   lbl_licensetype->setAlignment(AlignLeft|AlignVCenter);
   lbl_licensetype->setPalette(QPalette(background_color));
   lbl_licensetype->setGeometry(xpos, ypos, buttonw/4, buttonh);
   
   xpos += buttonw/4;
   
   lbl_lctype = new QLabel(" ",this);
   lbl_lctype->setAlignment(AlignLeft|AlignVCenter);
   lbl_lctype->setPalette(QPalette(background_color));
   lbl_lctype->setGeometry(xpos, ypos, buttonw/4, buttonh);
   
   xpos += buttonw/4;
   
   lbl_version = new QLabel(" Version:",this);
   lbl_version->setAlignment(AlignLeft|AlignVCenter);
   lbl_version->setPalette(QPalette(background_color));
   lbl_version->setGeometry(xpos, ypos, buttonw/4, buttonh);
   
   xpos += buttonw/4;
   
   lbl_vs = new QLabel(" ",this);
   lbl_vs->setAlignment(AlignLeft|AlignVCenter);
   lbl_vs->setPalette(QPalette(background_color));
   lbl_vs->setGeometry(xpos, ypos, buttonw/4, buttonh);
   
   xpos = buttonw +4*border;
   ypos += buttonh + spacing;
   
   lbl_expiration = new QLabel(" Expiration:",this);
   lbl_expiration->setAlignment(AlignLeft|AlignVCenter);
   lbl_expiration->setPalette(QPalette(background_color));
   lbl_expiration->setGeometry(xpos, ypos, buttonw/3, buttonh);
      
   xpos += buttonw/3;
   
   lbl_exp = new QLabel(" ",this);
   lbl_exp->setAlignment(AlignLeft|AlignVCenter);
   lbl_exp->setPalette(QPalette(background_color));
   lbl_exp->setGeometry(xpos, ypos, buttonw/3*2, buttonh);
   
   xpos = buttonw +4*border;
   ypos += buttonh + spacing;
   
   lbl_mailinglist = new QLabel(" Mailinglist:",this);
   lbl_mailinglist->setAlignment(AlignLeft|AlignVCenter);
   lbl_mailinglist->setPalette(QPalette(background_color));
   lbl_mailinglist->setGeometry(xpos, ypos, buttonw/4, buttonh);
   
   xpos += buttonw/4;
   
   lbl_mlist = new QLabel(" ",this);
   lbl_mlist->setAlignment(AlignLeft|AlignVCenter);
   lbl_mlist->setPalette(QPalette(background_color));
   lbl_mlist->setGeometry(xpos, ypos, buttonw/4, buttonh);
   
   xpos += buttonw/4;
   
   lbl_approved = new QLabel(" Approved:",this);
   lbl_approved->setAlignment(AlignLeft|AlignVCenter);
   lbl_approved->setPalette(QPalette(background_color));
   lbl_approved->setGeometry(xpos, ypos, buttonw/4, buttonh);
      
   xpos += buttonw/4;
   
   lbl_apprv = new QLabel(" ",this);
   lbl_apprv->setAlignment(AlignLeft|AlignVCenter);
   lbl_apprv->setPalette(QPalette(background_color));
   lbl_apprv->setGeometry(xpos, ypos, buttonw/4, buttonh);
   
   xpos = buttonw +3*border;
   ypos += buttonh + border;
   
   pb_license = new QPushButton("Printable License", this);
   pb_license->setAutoDefault(false);
   pb_license->setPalette(QPalette(cyan));
   pb_license->setGeometry(xpos, ypos, buttonw/2, buttonh);
   pb_license->setEnabled(true);
   connect(pb_license, SIGNAL(clicked()), SLOT(license_show()));
   
   xpos += buttonw/2 +spacing;
      
   pb_text = new QPushButton("Printable Info", this);
   pb_text->setAutoDefault(false);
   pb_text->setPalette(QPalette(cyan));
   pb_text->setGeometry(xpos, ypos, buttonw/2, buttonh);
   pb_text->setEnabled(true);
   connect(pb_text, SIGNAL(clicked()), SLOT(text_show()));

   pb_delete->setEnabled(false);
   pb_approve->setEnabled(false);
   pb_chkID->setEnabled(false);
   pb_license->setEnabled(false);
   pb_text->setEnabled(false);
   
   xpos = buttonw*2 + 6*border;
   ypos += buttonh + border;
   
   setMinimumSize(xpos, ypos);   
   setGeometry(200, 200, xpos, ypos);

}

US_Check::~US_Check()
{
}

void US_Check::update_input(const QString &newText)
{
   if(!newText.isEmpty())
   {
      input = newText;
      input_flag = true;
   }
}

void US_Check::sel_type(int item)
{
   QString content, str;
   
   input_type = item;
   if(input_type<5)
   {
      content = cmbb_type->text(item);
      str = " Input   "+content + ":"; 
      if(item >0)
      {
         lbl_input->setText(str);
         le_input->setEnabled(true); 
      }
      else
      {
         lbl_input->setText(" Choose check type first!");
         le_input->setEnabled(false);
      }
   }
   if(input_type == 5)   //Show All
   {
      lbl_input->setText("Show all license record");
      show_all();
   }
   
}

void US_Check::show_all()
{
   lb_key->clear();   
   int maxid = 0;   
   QString str = "SELECT id FROM registration";
   QSqlQuery query( str );                       
   if ( query.isActive() ) 
   {
      query.last();
      maxid = query.value(0).toInt();
   }
   QString *display_Str;
   display_Str = new QString[maxid];
   
   QSqlCursor cur_r( "registration" );
   QStringList fields = QStringList() << "last_name" << "first_name";
   QSqlIndex nameOrder = cur_r.index( fields );
   cur_r.select(nameOrder);
   int rid, count = 0;
   QString firstname, lastname, organization, address, city, state, zip, phone, email, mlist;
   while(cur_r.next())
   {
      rid = cur_r.value( "id" ).toInt();         
      firstname = cur_r.value("first_name").toString();
      lastname = cur_r.value("last_name").toString();
      organization = cur_r.value("organization").toString();
      address = cur_r.value("address").toString();
      city= cur_r.value("city").toString();
      state = cur_r.value("state").toString();
      zip = cur_r.value("zipcode").toString();
      phone = cur_r.value("phone").toString();
      email = cur_r.value("email").toString();
      mlist = cur_r.value("mailing_list").toString();
      display_Str[count] = lastname +" " + firstname + ", "+ organization + ", "
         + city+", " + state +", " + QString::number(rid);
      count++;
   }
   if(count>0)
   {
      for ( int i=0; i<count; i++)
      {
         lb_key->insertItem(display_Str[i]);
      }
      sel_name =true;
   }
   else         //No record be found in table registration.
   {
      lb_key->clear();
      QString str = "SORRY, NO REGISTRATION RECORD IN DB";
      lb_key->insertItem(str);         
   }
   //delete display_Str;
}

void US_Check::select_name(int item)
{
   if(sel_name)
   {
      QString rid_str;
      rid_str = lb_key->text(item);
      rid_str = rid_str.section(',', -1);
      RID = rid_str.stripWhiteSpace().toInt();   
      input_flag = true;
      sel_name = false;
      show_flag = true;
      checklicense();
      item = -1;
      pb_delete->setEnabled(false);
      pb_approve->setEnabled(false);
      pb_chkID->setEnabled(false);
      pb_text->setEnabled(false); 
   }
}

void US_Check::checklicense()
{
   lb_key->clear();
   if(input_flag)
   {
      QSqlCursor cur_r( "registration" );
      QSqlIndex order_r = cur_r.index( "id" );
      QSqlIndex filter_r;
      switch (input_type)
      {
         
      case 1:
         {
            filter_r = cur_r.index( "last_name" );
            cur_r.setValue( "last_name",input );
            break;
         }
      case 2:
         {
            filter_r = cur_r.index( "organization" );
            cur_r.setValue( "organization",input );
            break;
         }
      case 3:
         {
            filter_r = cur_r.index( "zipcode" );
            cur_r.setValue( "zipcode",input );
            break;
         }
      case 4:
         {
            filter_r = cur_r.index( "email" );
            cur_r.setValue( "email",input );
            break;
         }
      case 5:
         {
            filter_r = cur_r.index( "id" );
            cur_r.setValue( "id", RID );
            break;
         }

      }
      cur_r.select(filter_r, order_r);
      while ( cur_r.next() )
      {
         Rid = cur_r.value( "id" ).toInt();
         if(Rid !=0)
         {
            QSqlCursor cur_l( "license" );
            QStringList orderFields_l = QStringList() <<"id"<<"license_type"<<"license_key";
            QSqlIndex order_l = cur_l.index( orderFields_l );
            QSqlIndex filter_l = cur_l.index( "owner_id" );
            cur_l.setValue( "owner_id",Rid );
            cur_l.select(filter_l, order_l);
         
            int count = 0;
            int lid[32];
            QString LC_Str[32], lctype[32],key[32];
            while( cur_l.next() )
            {
               lid[count] = cur_l.value("id").toInt();
               lctype[count] = cur_l.value("license_type").toString();
               key[count] = cur_l.value("license_key").toString();         
               LC_Str[count] = "("+QString::number(lid[count]) +") " + lctype[count] +",  " + key[count];
               count++;
            }
            if(count>0)
            {
               for ( int i=0; i<count; i++)
               {
                  lb_key->insertItem(LC_Str[i]);
               }
               sel_key = true;      // you got key to select
            }
            else      //No record be found in table license. 
            {
               lb_key->clear();
               lb_key->insertItem("SORRY, NO LICENSE IN DB MATCH YOUR CONDITION");
            }
         }
         else         //No record be found in table registration.
         {
            lb_key->clear();
            QString str = "SORRY, NO REGISTRATION RECORD ABOUT:  " + input;
            lb_key->insertItem(str);         
         }
      }
   }
   else
   {
      QMessageBox::message(tr("Attention:"), 
                           tr("You must choose check type first and then input your condition!\n"));
      return;
   }


}


void US_Check::select_key(int item)
{
   if(sel_key)
   {   
      int pos;
      QString str, lid_str, key_str;
      str = lb_key->text(item);
      pos = str.find(")", 0);
      lid_str = str.left(pos);
      lid_str = lid_str.remove(0,1);
      Lid = lid_str.toInt();
      str = str.section(',', -1);
      info_list.key = str.stripWhiteSpace();   
      key_str.sprintf("License Key:   "+ info_list.key);
      lbl_license->setText(key_str);
      pb_delete->setEnabled(true);
      pb_approve->setEnabled(true);
      pb_chkID->setEnabled(true);
      pb_license->setEnabled(true);
      pb_text->setEnabled(true);
      show_flag = true;
   }
}

void US_Check::del()
{
   
   if(show_flag)
   {
      QSqlCursor cur( "license");
      cur.setMode( QSqlCursor::Delete);
      QSqlIndex filter = cur.index("id");
      cur.setValue("id", Lid);
      cur.select(filter);
      cur.primeDelete();
      
      QString str = "Clicking 'OK' will delete license key : \n" + info_list.key;
      switch(QMessageBox::information(this, tr("Do you want to Delete?"), 
                                      str, "OK", "CANCEL",   0,1))
      {
      case 0:
         {
            cur.del();
            lb_key->clear();
            clear();
            break;
         }   
      case 1:
         {
            break;
         }
      }
   }
   else
   {
      QMessageBox::message(tr("Attention:"), 
                           tr("You must select License Key first!\n"));
      return;
   }
   
}

void US_Check::approve()
{
   if(show_flag)
   {
      QSqlCursor cur( "license");
      cur.setMode( QSqlCursor::Update);
      QSqlIndex filter = cur.index("id");
      cur.setValue("id", Lid);
      cur.select(filter);
      QSqlRecord *buf = cur.primeUpdate();
      
      buf->setValue("owner_id", Rid);
      buf->setValue("license_type", info_list.lctype);
      buf->setValue("platform", info_list.platform);
      buf->setValue("operating_system", info_list.os);
      buf->setValue("version", info_list.version);
      buf->setValue("license_key",info_list.key);
      buf->setValue("approved", "YES" );
      buf->setValue("expiration", info_list.expiration); 
      QString str = "Clicking 'OK' will approve license key : \n" + info_list.key;
      switch(QMessageBox::information(this, tr("Do you want to Approve?"), 
                                      str, "OK", "CANCEL",   0,1))
      {
      case 0:
         {
            cur.update();
            info_list.approved = "YES";
            lbl_apprv->setText("YES");
            break;
         }   
      case 1:
         {
            break;
         }         
      }
   }
   else
   {
      QMessageBox::message(tr("Attention:"), 
                           tr("You must select License Key first!\n"));
      return;
   }
}   

void US_Check::show_info()
{
   if(show_flag)
   {   
      QSqlCursor cur_l( "license");
      QStringList orderFields_l = QStringList() <<"owner_id"<<"license_type" << "platform"<<"operating_system"
                                                <<"version"<<"approved"<<"expiration";
      QSqlIndex order_l = cur_l.index( orderFields_l );
      QSqlIndex filter_l = cur_l.index( "license_key" );
      cur_l.setValue( "license_key",info_list.key );
      cur_l.select(filter_l, order_l);
      if( cur_l.next() )
      {
         info_list.rid = cur_l.value("owner_id").toInt();
         info_list.lctype = cur_l.value("license_type").toString();
         info_list.platform = cur_l.value("platform").toString();
         info_list.os = cur_l.value("operating_system").toString();
         info_list.key = cur_l.value("license_key").toString();
         info_list.approved = cur_l.value("approved").toString();
         info_list.expiration = cur_l.value("expiration").toString();
         info_list.version = cur_l.value("version").toString();
      }
      QString cond;
      QSqlCursor cur_r( "registration" );
      cond.sprintf("id = %d", info_list.rid);
      cur_r.select(cond);
      if ( cur_r.next() )
      {
         info_list.lastname = cur_r.value("last_name").toString();
         info_list.firstname = cur_r.value("first_name").toString();
         info_list.organization = cur_r.value("organization").toString();
         info_list.address = cur_r.value("address").toString();
         info_list.city = cur_r.value("city").toString();
         info_list.state = cur_r.value("state").toString();
         info_list.zip = cur_r.value("zipcode").toString();
         info_list.phone = cur_r.value("phone").toString();
         info_list.email = cur_r.value("email").toString();
         info_list.mlist = cur_r.value("mailing_list").toString();
      }
      QString str;
      lbl_fname->setText(info_list.firstname);
      lbl_lname->setText(info_list.lastname);
      lbl_org->setText(info_list.organization);
      lbl_addr->setText(info_list.address);
      lbl_ct->setText(info_list.city);
      lbl_st->setText(info_list.state);
      lbl_zip->setText(info_list.zip);
      lbl_phone->setText(info_list.phone);
      lbl_email->setText(info_list.email);
      lbl_lctype->setText(info_list.lctype);
      lbl_exp->setText(info_list.expiration);
      lbl_vs->setText(info_list.version);
      str = info_list.platform + " / " + info_list.os;
      lbl_os->setText(str);
      if(info_list.mlist == "C" || info_list.mlist == "YES")
      {
         info_list.mlist = "YES";
         lbl_mlist->setText("YES");
      }
      else
      {
         info_list.mlist = "NO";
         lbl_mlist->setText("NO");
      }
      if(info_list.approved == "YES")
      {
         info_list.approved = "YES";
         lbl_apprv->setText("YES");
      }
      else
      {
         info_list.approved = "NO";
         lbl_apprv->setText("NO");
      }
      export_flag = true;      //now let it export printable information
   }
   else
   {   
      QMessageBox::message(tr("Attention:"), 
                           tr("You must select License Key first!\n"));
      return;
   }
}

void US_Check::license_show()
{
   TextShow *license_show;
   //type = 0 show license
   license_show = new TextShow(info_list, 0);
   if(export_flag)
   {
      license_show->setCaption("License");
      license_show->resize(480, 550);
      license_show->show();
   }
   else
   {
      QMessageBox::message(tr("Attention:"), 
                           tr("Please click 'Show Info' first!\n"));
      return;
   }
}

void US_Check::text_show()
{
   TextShow *text_show;
   //type = 1 show text
   text_show = new TextShow(info_list, 1);
   if(export_flag)
   {
      text_show->setCaption("License Information");
      text_show->resize(480, 550);
      text_show->show();
   }
   else
   {
      QMessageBox::message(tr("Attention:"), 
                           tr("Please click 'Show Info' first!\n"));
      return;
   }
}

void US_Check::reset()
{
   input_type = 0;
   Rid = 0;
   RID = 0;
   Lid = 0;
   input = "";
   Key = "";
   input_flag = false;
   sel_name = false;
   sel_key = false;
   show_flag = false;
   export_flag = false;
   cmbb_type->setCurrentItem(0);
   lbl_input->setText(" Choose check type first!");
   le_input->setText("");
   le_input->setEnabled(false);
   info_list.firstname = "";
   info_list.lastname ="";
   info_list.organization = "";
   info_list.address = "";
   info_list.city = "";
   info_list.state = "";
   info_list.zip = "";
   info_list.phone = "";
   info_list.email = "";
   info_list.mlist = "";
   info_list.platform = "";
   info_list.os = "";
   info_list.lctype = "";
   info_list.version = "0.0";
   info_list.key = "";
   info_list.expiration = "";
   info_list.approved = "";
   clear();
   lb_key->clear();
}

void US_Check::clear()
{
   lbl_license->setText("License Key: not selected");
   lbl_fname->setText("");
   lbl_lname->setText("");
   lbl_org->setText("");
   lbl_addr->setText("");
   lbl_ct->setText("");
   lbl_st->setText("");
   lbl_zip->setText("");
   lbl_phone->setText("");
   lbl_email->setText("");
   lbl_lctype->setText("");
   lbl_exp->setText("");
   lbl_vs->setText("");
   lbl_os->setText("");
   lbl_mlist->setText("");
   lbl_apprv->setText("");
   pb_delete->setEnabled(false);
   pb_approve->setEnabled(false);
   pb_chkID->setEnabled(false);
   pb_license->setEnabled(false);
   pb_text->setEnabled(false);

}

void US_Check::quit()
{
   close();
}

TextShow::TextShow(struct LCinfo info_list, int type, QWidget * parent , const char * name )
   : QWidget( parent, name )
{
   m = new QMenuBar( this, "menu" );
   QPopupMenu * file = new QPopupMenu();
   Q_CHECK_PTR( file );
   m->insertItem( tr("&File"), file );
   file->insertItem( tr("Save"), this, SLOT(save()),   ALT+Key_S );
   file->insertSeparator();
   file->insertItem( tr("Print"), this, SLOT(print()),   ALT+Key_P );
   file->insertSeparator();
   file->insertItem( tr("Close"), this, SLOT(closeDoc()),ALT+Key_W );
      
   e = new QTextEdit( this, "editor" );
   QFont ft("Courier", 12);
   e->setFont(ft);
   e->setReadOnly(true);
   load(info_list, type);
}

TextShow::~TextShow()
{   
}

void TextShow::load(struct LCinfo info_list, int type)
{
   QString str;
   e->clear();
   if(type == 0)
   {
   
      str = info_list.firstname + "\n";
      e->append(str);
      str = info_list.lastname + "\n";
      e->append(str);
      str = info_list.organization + "\n";
      e->append(str); 
      str = info_list.address + "\n";
      e->append(str);
      str = info_list.city + "\n";
      e->append(str);
      str = info_list.state + "\n";
      e->append(str);
      str = info_list.zip + "\n";
      e->append(str);
      str = info_list.phone + "\n";
      e->append(str);
      str = info_list.email + "\n";
      e->append(str);
      str = info_list.platform + "\n";
      e->append(str);
      str = info_list.os + "\n";
      e->append(str);
      str = info_list.version + "\n";
      e->append(str);
      str = info_list.lctype + "\n";
      e->append(str);
      str = info_list.key + "\n";
      e->append(str);
      str = info_list.expiration + "\n";
      e->append(str);
   }

   if(type == 1)
   {
      e->append("**********************************************\n");
      e->append("*             License Registration           *\n");
      e->append("**********************************************\n\n\n");
      str ="First Name :      \t" + info_list.firstname + "\n";
      e->append(str);
      str ="Last Name :       \t" + info_list.lastname + "\n";
      e->append(str);
      str ="Organization :    \t" + info_list.organization + "\n";
      e->append(str); 
      str ="Address :         \t" + info_list.address + "\n";
      e->append(str);
      str ="City :            \t" + info_list.city + "\n";
      e->append(str);
      str ="State :           \t" + info_list.state + "\n";
      e->append(str);
      str ="Zipcode :         \t"+ info_list.zip + "\n";
      e->append(str);
      str ="Phone :           \t" + info_list.phone + "\n";
      e->append(str);
      str ="Email :           \t" + info_list.email + "\n";
      e->append(str);
      str ="Mailinglist :     \t" + info_list.mlist + "\n";
      e->append(str);
      str ="Platform :        \t" + info_list.platform + "\n";
      e->append(str);
      str ="Operating System :\t" + info_list.os + "\n";
      e->append(str);
      str ="License Type :    \t" + info_list.lctype + "\n";
      e->append(str);
      str ="Version :         \t" + info_list.version + "\n";
      e->append(str);
      str ="License Key :     \t" + info_list.key + "\n";
      e->append(str);
      str ="Expiration :      \t" + info_list.expiration + "\n";
      e->append(str);
      str ="Approved :        \t" + info_list.approved + "\n";
      e->append(str);
   }
}
void TextShow::save()
{
   QString fn;
   fn = QFileDialog::getSaveFileName(QString::null, QString::null,this );
   if(!fn.isEmpty() ) 
   {
      int lines = e->lines();
      QFile f( fn );
      if ( !f.open( IO_WriteOnly ) ) 
      {
         return;
      }
      QTextStream t( &f );
      for(int i=0; i<lines-1; i++)
      {
         QString str = e->text(i);
         str.stripWhiteSpace();
         t << str<<"\n";
      }
      f.close();
      e->setModified( false );
      setCaption( fn );
   }
}
void TextShow::print()
{
   const int MARGIN = 10;

   if ( printer.setup(this) ) 
   {      // opens printer dialog
      QPainter p;
      p.begin( &printer );         // paint on printer
      p.setFont( e->font() );
      int yPos   = 0;         // y position for each line
      QFontMetrics fm = p.fontMetrics();
      QPaintDeviceMetrics metrics( &printer ); // need width/height
      // of printer surface
      for( int i = 0 ; i < e->lines() ; i++ ) 
      {
         if ( MARGIN + yPos > metrics.height() - MARGIN ) 
         {
            printer.newPage();      // no more room on this page
            yPos = 0;         // back to top of page
         }
         p.drawText( MARGIN, MARGIN + yPos, metrics.width(), fm.lineSpacing(),
                     ExpandTabs | DontClip, e->text( i ) );
         yPos = yPos + fm.lineSpacing();
      }
      p.end();            // send job to printer
   }
}
void TextShow::closeDoc()
{
   close();               // will call closeEvent()
}
void TextShow::resizeEvent( QResizeEvent * )
{
   if ( e && m )
      e->setGeometry( 0, m->height(), width(), height() - m->height() );
}

