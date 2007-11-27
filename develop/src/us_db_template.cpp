#include "../include/us_db_template.h"

//! Constructor
/*! 
	Constractor a new <var>US_DB_Template</var> interface, 
	\param p Parent widget.
	\param name Widget name.
*/	
US_DB_Template::US_DB_Template(QWidget *p, const char *name) : QFrame( p, name)
{
	int xpos, ypos, buttonw = 160,   buttonh = 26;

	USglobal = new US_Config();
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	
	read();
	xpos = border;
	ypos = border;	
	
	lbl_blank = new QLabel(tr(" Database Template Initialization"), this);
	lbl_blank->setAlignment(AlignCenter|AlignVCenter);
	lbl_blank->setFrameStyle(QFrame::WinPanel|Raised);
	lbl_blank->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_blank->setGeometry(xpos, ypos, buttonw*2 + spacing, buttonh);
	lbl_blank->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));



	xpos = border;
	ypos += buttonh + spacing;

	lbl_dbname = new QLabel(tr(" Initialize DB Template:"),this);
	lbl_dbname->setAlignment(AlignLeft|AlignVCenter);
	lbl_dbname->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_dbname->setGeometry(xpos, ypos, buttonw, buttonh);
	lbl_dbname->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	xpos += buttonw + spacing;
	
	lbl_dbname2 = new QLabel("",this);
	lbl_dbname2->setGeometry(xpos, ypos, buttonw, buttonh);
	lbl_dbname2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_dbname2->setText(login_list.dbname);
	
	ypos += buttonh + 2* spacing;
	xpos = border;
	
	pb_submit = new QPushButton(tr("Initialize"), this);
	pb_submit->setAutoDefault(false);
	pb_submit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_submit->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_submit->setGeometry(xpos, ypos, buttonw*2 + spacing, buttonh);
	pb_submit->setEnabled(true);
	connect(pb_submit, SIGNAL(clicked()), SLOT(check_permission()));
	
	xpos = border;
	ypos += buttonh + spacing;
	
	pb_help = new QPushButton(tr("Help"), this);
	pb_help->setAutoDefault(false);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_help->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_help->setGeometry(xpos, ypos, buttonw, buttonh);
	pb_help->setEnabled(true);
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));

		
	xpos += buttonw + spacing;
		
	pb_close = new QPushButton(tr("Close"), this);
	pb_close->setAutoDefault(false);
	pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_close->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_close->setGeometry(xpos, ypos, buttonw, buttonh);
	pb_close->setEnabled(true);
	connect(pb_close, SIGNAL(clicked()), SLOT(quit()));

	ypos += buttonh + border;
	xpos = 2 * border + 2 * buttonw + spacing;

	global_Xpos += 30;
	global_Ypos += 30;

	setMinimumSize(xpos, ypos);	
	setGeometry(global_Xpos, global_Ypos, xpos, ypos);
	setup_GUI();
}

//! Destructor
/*! destroy the <var>US_DB_Template</var>. */
US_DB_Template::~US_DB_Template()
{
}

void US_DB_Template::setup_GUI()
{
	QGridLayout * Grid = new QGridLayout(this,4,2,4,2);
	Grid->addMultiCellWidget(lbl_blank,0,0,0,1);
	Grid->addWidget(lbl_dbname,1,0);
	Grid->addWidget(lbl_dbname2,1,1);
	Grid->addMultiCellWidget(pb_submit,2,2,0,1);
	Grid->addWidget(pb_help,3,0);
	Grid->addWidget(pb_close,3,1);
	Grid->activate();

}

/*!
	Read DB file and assign the parameters to struct US_LoginData.
	If DB file can not be found, return error message.
*/
void US_DB_Template::read()
{
	QString dbfile;

	dbfile = USglobal->home.copy();
	dbfile.append("us.db");
	QFile f(dbfile);
	if (f.exists())
	{
		f.open(IO_ReadOnly);
		QDataStream ds (&f);
		double num[64];
		int code[64];
		int i,length;
		
			ds >> length;
			for(i=0; i<length; i++)
			{
				ds >> num[i];
				code[i] = (int)(num[i]+0.5);
				login_list.dbname[i] = QChar(code[i]);
			}
			ds >> length;
			for(i=0; i<length; i++)
			{
				ds >> num[i];
				code[i] = (int)(num[i]+0.5);
				login_list.username[i] = QChar(code[i]);
			}
			ds >> length;
			for(i=0; i<length; i++)
			{
				ds >> num[i];
				code[i] = (int)(num[i]+0.5);
				login_list.password[i] = QChar(code[i]);
			}
			ds >> length;
			for(i=0; i<length; i++)
			{
				ds >> num[i];
				code[i] = (int)(num[i]+0.5);
				login_list.host[i] = QChar(code[i]);
			}
			ds >> length;
			for(i=0; i<length; i++)
			{
				ds >> num[i];
				code[i] = (int)(num[i]+0.5);
				login_list.driver[i] = QChar(code[i]);
			}
			f.close();				
	}
	else
	{															
		QMessageBox::message(tr("Attention:"), 
									tr("The data file with your login info could not be found.\n"
									"Please check: 'File/Configuration/Database Preferences'."));
		return;				
		
	}
}
/*!
	Open US_DB_Admin to check delete permission.
*/
void US_DB_Template::check_permission()
{
	US_DB_Admin *db_admin;
	db_admin = new US_DB_Admin("");
	db_admin->show();
	connect(db_admin, SIGNAL(issue_permission(bool)), SLOT(initialize(bool)));
}

/*! 
	Connect to Database, check DB name list.
	If the DB name is not there, create() a new DB template and initialize all tables and their fields.
	If the DB name is there, you can choose leave it away,
	or delete old one and create() a new DB template as same name.
*/
void US_DB_Template::initialize(bool permission)
{
	if(!permission)
	{
		QMessageBox::message(tr("Attention:"), 
									tr("Permission denied, please contact your system administrator"));
		return;
	}

	QSqlDatabase *defaultDB = QSqlDatabase::addDatabase(login_list.driver );
   if ( ! defaultDB ) 
	{
   	QMessageBox::message(tr("Attention:"), 
									tr("The database selected in your login information is not\n"
										"available on the selected host.\n"
										"Please select the correct database in:\n"
										"'File/Configuration/Database Preferences'"));
		
      return;
   }
	defaultDB->setDatabaseName( login_list.dbname );
   defaultDB->setUserName(login_list.username );
   defaultDB->setPassword( login_list.password );
   defaultDB->setHostName( login_list.host );
   if ( ! defaultDB->open() )
	{
		qWarning( defaultDB->lastError().driverText() );
		QString str = defaultDB->lastError().databaseText();
		str.append("\n\nThe information saved in your database login information\n");
		str.append("\ndoes not match your database settings.\n");
		str.append("\nAlso, check access permissions for the database.\n");
		str.append("\nContact your system administrator for assistance.");
		QMessageBox::message(tr( "Failed to open database:"),str);
      return;
   }
	int count = 0;
	table_name = new QString[64];		//now only have six tables
	QString Str = "";
	QStringList list = defaultDB->tables();
	for (QStringList::Iterator it = list.begin(); it != list.end(); ++it ) 
	{
		Str.append("<");
		table_name[count] = (*it).latin1();
		Str.append(table_name[count]);
		Str.append(">");
		Str.append("\n");
		count++;
   }
	if(count>0)	//there are some tables exist
	{
		QString str1;
		str1="The Database '";
		str1.append(login_list.dbname);
		str1.append("' already exists. ");
		str1.append("It contains tables: \n");
		str1.append(Str);
		switch(QMessageBox::information(this, tr("Attention!"), str1 +
										tr("\n\nDelete the existing database and create a new database template?"),
										tr("OK"), tr("CANCEL"),	0,1))
		{ 
			case 0:
			{
				QSqlQuery q;
				for(int i=0; i<count; i++)
				{
					QString str2 = "DROP TABLE ";
					str2.append(table_name[i]);
   				q.exec( str2 );				
				}
				create();
				break;
			}	
			case 1:
			{
				break;
			}		
		}			
	}
	else			//it is an empty database name
	{	
		switch(QMessageBox::information(this, tr("Initialize the database?"), 
										tr("Clicking 'OK' will initialize the database\nand delete existing data."),
										tr("OK"), tr("CANCEL"),	0,1))
		{
			case 0:
			{
				create();
				break;
			}	
			case 1:
			{
				break;
			}		
		}
	}								
}

/*!
	Create DB Tables and all these tables' fields.
*/
void US_DB_Template::create()
{
	QSqlQuery q;
	QString test, cmd;
	QString sqlFile = USglobal->config_list.system_dir +"/etc/mysql.dll";
	QFile f(sqlFile);
	if(f.exists())
	{
		if(f.open(IO_ReadOnly))
		{
			QTextStream ts(&f);
			while(!f.atEnd())
			{
				cmd ="";
				test = ts.readLine();
				if(test.contains("CREATE", false))
				{
					cmd.append(test);
					do
					{
						test = ts.readLine();
						cmd.append(test);
					}
					while(!test.contains(';', false));
					q.exec(cmd);	
				}
			}
			QMessageBox::message(tr("Congratulation"), "Successful Initialization!" );
		}
		else
		{
			QMessageBox::message(tr("Warning:"), "Can not read the MySQL script file" );
			return;
		}
			
	}
	else
	{
		QMessageBox::message(tr("Warning:"), "The MySql Script file : '"+ sqlFile + "' not exists" );
		return;
	}
	
/*	QString cmd;
	cmd = "mysql -D ";
	cmd += login_list.dbname;
	cmd += " -u ";
	cmd += login_list.username;
	cmd += " --password=";
	cmd += login_list.password;
	cmd += " < ";
	cmd += USglobal->config_list.system_dir;
	cmd += "/etc/mysql.dll";
	cout<<cmd<<endl;
	system(cmd);
*/
}

void US_DB_Template::readFromStderr()
{
	QString str = proc->readLineStderr(); 
	QMessageBox::message("UltraScan Error:", str);
}

/*!
	Open a netscape browser to load help page.
*/
void US_DB_Template::help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/db_template.html");
}

/*! Close the interface. */
void US_DB_Template::quit()
{
	close();
}



