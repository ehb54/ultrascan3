#include "../include/us_db_tbl_investigator.h"

//! Constructor
/*!
	Constractor a new <var>US_DB_TblInvestigator</var> interface,
	with <var>p</var> as a parent and <var>us_tblinvestigator</var> as object name.
*/
US_DB_TblInvestigator::US_DB_TblInvestigator(QWidget *p, const char *name) : US_DB( p, name)
{
	int xpos, ypos, column1 = 160, column2 = 220,  buttonh = 26;
	int buttonw1 = column1;
	int buttonw2 = column2;
	int buttonw3 = (buttonw1 + buttonw2)/3;

	select_flag = false;		// use for query listbox select
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));

 	changed_flag = 0; //0 no change and 1 changed, flag for update()
	new_register = 0; //0 existed guy and 1 new guy, flag for save()
	info_list.InvID = -1;
	info_list.LastName = "";
	info_list.FirstName = "";
	info_list.Address = "";
	info_list.City = "";
	info_list.State = "";
	info_list.Zip = "";
	info_list.Phone = "";
	info_list.Email = "";
	info_list.Display = "";

	xpos = border;
	ypos = border;

	lbl_blank = new QLabel(tr("Investigator Information:"), this);
	lbl_blank->setAlignment(AlignCenter|AlignVCenter);
//	lbl_blank->setFrameStyle(QFrame::WinPanel|Raised);
	lbl_blank->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_blank->setGeometry(xpos, ypos, column1 + column2 + spacing, buttonh);
	lbl_blank->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));



	xpos = border;
	ypos += buttonh + spacing;

	lbl_InvID = new QLabel(tr(" Investigator ID:"),this);
	lbl_InvID->setAlignment(AlignLeft|AlignVCenter);
	lbl_InvID->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_InvID->setGeometry(xpos, ypos, column1, buttonh);
	lbl_InvID->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	xpos += column1 + spacing;


	newInvID =get_newID("tblInvestigators", "InvID");
	lbl_IdNumber = new QLabel("",this);
	lbl_IdNumber->setAlignment(AlignVCenter);
	lbl_IdNumber->setGeometry(xpos, ypos, column2, buttonh);
	lbl_IdNumber->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_IdNumber->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1));
	lbl_IdNumber->setNum(newInvID);

	xpos = border;
	ypos += buttonh + spacing;

	lbl_LastName = new QLabel(tr(" Last Name:"),this);
	lbl_LastName->setAlignment(AlignLeft|AlignVCenter);
	lbl_LastName->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_LastName->setGeometry(xpos, ypos, column1, buttonh);
	lbl_LastName->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	xpos += column1 + spacing;

	le_LastName = new QLineEdit(this, "LastName");
	le_LastName->setGeometry(xpos, ypos, column2, buttonh);
	le_LastName->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect (le_LastName, SIGNAL(textChanged(const QString &)), SLOT(update_lastname(const QString &)));
	le_LastName->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	connect (le_LastName, SIGNAL(returnPressed()), SLOT(checkname()));

	ypos += buttonh + spacing;
	xpos = border;

	pb_chkname = new QPushButton(tr("Check Last Name"), this);
	pb_chkname->setAutoDefault(false);
	pb_chkname->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_chkname->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_chkname->setGeometry(xpos, ypos, buttonw1, buttonh);
	pb_chkname->setEnabled(true);
	connect(pb_chkname, SIGNAL(clicked()), SLOT(checkname()));

	xpos +=column1 + spacing;

	lb_name = new QListBox(this, "Names");
	lb_name->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	lb_name->setGeometry(xpos, ypos, column2, (buttonh*4));
	lb_name->setSelected(0, true);
	lb_name->setCurrentItem(0);
	lb_name->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	connect(lb_name, SIGNAL(selected(int)), SLOT(select_name(int)));

	xpos = border;
	ypos += 4 * buttonh + spacing;

	lbl_FirstName = new QLabel(tr(" First Name:"),this);
	lbl_FirstName->setAlignment(AlignLeft|AlignVCenter);
	lbl_FirstName->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_FirstName->setGeometry(xpos, ypos, column1, buttonh);
	lbl_FirstName->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	xpos += column1 + spacing;

	le_FirstName = new QLineEdit(this, "FirstName");
	le_FirstName->setGeometry(xpos, ypos, column2, buttonh);
	le_FirstName->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_FirstName->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	connect(le_FirstName, SIGNAL(textChanged(const QString &)), this, SLOT(update_firstname(const QString &)));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_Address = new QLabel(tr(" Address:"),this);
	lbl_Address->setAlignment(AlignLeft|AlignVCenter);
	lbl_Address->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_Address->setGeometry(xpos, ypos, column1, buttonh);
	lbl_Address->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	xpos += column1 + spacing;

	le_Address = new QLineEdit(this, "address");
	le_Address->setGeometry(xpos, ypos, column2, buttonh);
	le_Address->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_Address->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	connect (le_Address, SIGNAL(textChanged(const QString &)), SLOT(update_address(const QString &)));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_City = new QLabel(tr(" City:"),this);
	lbl_City->setAlignment(AlignLeft|AlignVCenter);
	lbl_City->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_City->setGeometry(xpos, ypos, column1, buttonh);
	lbl_City->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	xpos += column1 + spacing;

	le_City = new QLineEdit(this, "city");
	le_City->setGeometry(xpos, ypos, column2, buttonh);
	le_City->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_City->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	connect (le_City, SIGNAL(textChanged(const QString &)), SLOT(update_city(const QString &)));

	ypos += buttonh + spacing;
	xpos = border;

	lbl_State = new QLabel(tr(" State:"), this);
	lbl_State->setAlignment(AlignLeft|AlignVCenter);
	lbl_State->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_State->setGeometry(xpos, ypos, column1, buttonh);
	lbl_State->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	xpos += column1 + spacing;

	le_State = new QLineEdit(this, "state");
	le_State->setGeometry(xpos, ypos, column2, buttonh);
	le_State->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_State->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	connect (le_State, SIGNAL(textChanged(const QString &)), SLOT(update_state(const QString &)));


	xpos = border;
	ypos += buttonh + spacing;

	lbl_Zip = new QLabel(tr(" Zip:"),this);
	lbl_Zip->setAlignment(AlignLeft|AlignVCenter);
	lbl_Zip->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_Zip->setGeometry(xpos, ypos, column1, buttonh);
	lbl_Zip->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	xpos += column1 + spacing;

   le_Zip = new QLineEdit(this, "zip");
	le_Zip->setGeometry(xpos, ypos, column2, buttonh);
	le_Zip->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_Zip->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	connect (le_Zip, SIGNAL(textChanged(const QString &)), SLOT(update_zip(const QString &)));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_Phone = new QLabel(tr(" Phone:"),this);
	lbl_Phone->setAlignment(AlignLeft|AlignVCenter);
	lbl_Phone->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_Phone->setGeometry(xpos, ypos, column1, buttonh);
	lbl_Phone->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	xpos += column1 + spacing;

   le_Phone = new QLineEdit(this, "phone");
	le_Phone->setGeometry(xpos, ypos, column2, buttonh);
	le_Phone->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_Phone->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	connect (le_Phone, SIGNAL(textChanged(const QString &)), SLOT(update_phone(const QString &)));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_Email = new QLabel(tr(" Email:"),this);
	lbl_Email->setAlignment(AlignLeft|AlignVCenter);
	lbl_Email->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_Email->setGeometry(xpos, ypos, column1, buttonh);
	lbl_Email->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	xpos += column1 + spacing;

   le_Email = new QLineEdit(this, "email");
	le_Email->setGeometry(xpos, ypos, column2, buttonh);
	le_Email->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_Email->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	connect (le_Email, SIGNAL(textChanged(const QString &)), SLOT(update_email(const QString &)));


	ypos += buttonh + 5 * spacing;
	xpos = border;

	pb_reset = new QPushButton(tr("Reset"), this);
	pb_reset->setAutoDefault(false);
	pb_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_reset->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_reset->setGeometry(xpos, ypos, buttonw3, buttonh);
	pb_reset->setEnabled(true);
	connect(pb_reset, SIGNAL(clicked()), SLOT(reset()));

	xpos +=buttonw3 + spacing;

	pb_update = new QPushButton(tr("Update"), this);
	pb_update->setAutoDefault(false);
	pb_update->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_update->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_update->setGeometry(xpos, ypos, buttonw3, buttonh);
	pb_update->setEnabled(false);
	connect(pb_update, SIGNAL(clicked()), SLOT(update()));

	xpos +=buttonw3 + spacing;

	pb_commit = new QPushButton(tr("Commit"), this);
	pb_commit->setAutoDefault(false);
	pb_commit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_commit->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_commit->setGeometry(xpos, ypos, buttonw3, buttonh);
	pb_commit->setEnabled(true);
	connect(pb_commit, SIGNAL(clicked()), SLOT(save()));


	xpos = border;
	ypos += buttonh + spacing;

	pb_delete = new QPushButton(tr("Delete"), this);
	pb_delete->setAutoDefault(false);
	pb_delete->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_delete->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_delete->setGeometry(xpos, ypos, buttonw3, buttonh);
	connect(pb_delete, SIGNAL(clicked()), SLOT(check_permission()));

	xpos += buttonw3 + spacing;

	pb_help = new QPushButton(tr("Help"), this);
	pb_help->setAutoDefault(false);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_help->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_help->setGeometry(xpos, ypos, buttonw3, buttonh);
	pb_help->setEnabled(true);
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));

	xpos += buttonw3 + spacing;

	pb_exit = new QPushButton(tr("Exit"), this);
	pb_exit->setAutoDefault(false);
	pb_exit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_exit->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_exit->setGeometry(xpos, ypos, buttonw3, buttonh);
	pb_exit->setEnabled(true);
	connect(pb_exit, SIGNAL(clicked()), SLOT(quit()));

	xpos = column1+column2 + spacing +2*border;
	ypos += buttonh + spacing;
	setMinimumSize(xpos, ypos);

	global_Xpos += 30;
	global_Ypos += 30;
	setGeometry(global_Xpos, global_Ypos, xpos, ypos);
	GUI();
}

//! Destructor
/*! destroy the <var>US_DB_TblInvestigator</var>. */
US_DB_TblInvestigator::~US_DB_TblInvestigator()
{
}

void US_DB_TblInvestigator::GUI()
{
	QBoxLayout *mainbox = new QVBoxLayout(this,4);
	mainbox->addWidget(lbl_blank);
	QGridLayout * lineGrid = new QGridLayout(mainbox,10,2,2);
	lineGrid->addWidget(lbl_InvID,0,0);
	lineGrid->addWidget(lbl_IdNumber,0,1);
	lineGrid->addWidget(lbl_LastName,1,0);
	lineGrid->addWidget(le_LastName,1,1);
	lineGrid->addWidget(pb_chkname,2,0);
	lineGrid->addWidget(lb_name,2,1);
	lineGrid->addWidget(lbl_FirstName,3,0);
	lineGrid->addWidget(le_FirstName,3,1);
	lineGrid->addWidget(lbl_Address,4,0);
	lineGrid->addWidget(le_Address,4,1);
	lineGrid->addWidget(lbl_City,5,0);
	lineGrid->addWidget(le_City,5,1);
	lineGrid->addWidget(lbl_State,6,0);
	lineGrid->addWidget(le_State,6,1);
	lineGrid->addWidget(lbl_Zip,7,0);
	lineGrid->addWidget(le_Zip,7,1);
	lineGrid->addWidget(lbl_Phone,8,0);
	lineGrid->addWidget(le_Phone,8,1);
	lineGrid->addWidget(lbl_Email,9,0);
	lineGrid->addWidget(le_Email,9,1);
	QGridLayout * buttonGrid = new QGridLayout(mainbox,2,3);
	buttonGrid->addWidget(pb_reset,0,0);
	buttonGrid->addWidget(pb_update,0,1);
	buttonGrid->addWidget(pb_commit,0,2);
	buttonGrid->addWidget(pb_delete,1,0);
	buttonGrid->addWidget(pb_help,1,1);
	buttonGrid->addWidget(pb_exit,1,2);
	mainbox->activate();
}
/*!Replace struct US_InvestigatorData <var>FirstName</var> value with newText only if old value has been changed.*/
void US_DB_TblInvestigator::update_firstname(const QString &newText)
{
	if(info_list.FirstName != newText)
	{
		info_list.FirstName = newText;
		changed_flag = 1;
	}
}

/*!Replace struct US_InvestigatorData <var>LastName</var> value with newText only if old value has been changed.*/
void US_DB_TblInvestigator::update_lastname(const QString &newText)
{
	if(info_list.LastName != newText)
	{
		info_list.LastName = newText;
		changed_flag = 1;
	}
}

/*!Replace struct US_InvestigatorData <var>Address</var> value with newText only if old value has been changed.*/
void US_DB_TblInvestigator::update_address(const QString &newText)
{
	if(info_list.Address != newText)
	{
		info_list.Address = newText;
		changed_flag = 1;
	}
}

/*!Replace struct US_InvestigatorData <var>City</var> value with newText only if old value has been changed.*/
void US_DB_TblInvestigator::update_city(const QString &newText)
{
	if(info_list.City != newText)
	{
		info_list.City = newText;
		changed_flag = 1;
	}
}

/*!Replace struct US_InvestigatorData <var>State</var> value with newText only if old value has been changed.*/
void US_DB_TblInvestigator::update_state(const QString &newText)
{
	if(info_list.State != newText)
	{
		info_list.State = newText;
		changed_flag = 1;
	}
}

/*!Replace struct US_InvestigatorData <var>Zip</var> value with newText only if old value has been changed.*/
void US_DB_TblInvestigator::update_zip(const QString &newText)
{
	if(info_list.Zip != newText)
	{
		info_list.Zip = newText;
		changed_flag = 1;
	}
}
/*!Replace struct US_InvestigatorData <var>Phone</var> value with newText only if old value has been changed.*/
void US_DB_TblInvestigator::update_phone(const QString &newText)
{
	if(info_list.Phone != newText)
	{
		info_list.Phone = newText;
		changed_flag = 1;
	}
}
/*!Replace struct US_InvestigatorData <var>Email</var> value with newText only if old value has been changed.*/
void US_DB_TblInvestigator::update_email(const QString &newText)
{
	if(info_list.Email != newText)
	{
		info_list.Email = newText;
		changed_flag = 1;
	}
}

/*!
	Compare the Last Name that you input with the data in tblInvestigators.\n
	If find the matched Last Name(non-case sensitive), display all Entries in ListBox. \n
	If not, display 'YOUR NAME NOT HERE, PLEASE REGISTER' in ListBox.
*/
void US_DB_TblInvestigator::checkname()
{
	QString str;
	int maxID = get_newID("tblInvestigators", "InvID");
	item_firstname.resize(maxID);
	item_lastname.resize(maxID);
	item_InvID.resize(maxID);
	nameString.resize(maxID);
	QString checkstr = le_LastName->text();
	if(!checkstr.isEmpty())
	{
		str = "SELECT InvID, FirstName, LastName FROM tblInvestigators WHERE LastName LIKE '%" + checkstr +
				"%' OR FirstName LIKE '%" + checkstr + "%' ORDER BY LastName;";
	}
	else
	{
		str = "SELECT InvID, FirstName, LastName FROM tblInvestigators ORDER BY LastName;";
	}
	QSqlQuery query(str);
	int nameCount = 0;
	if(query.isActive())
	{
		while(query.next())
		{
			item_InvID[nameCount] = query.value(0).toInt();
			item_firstname[nameCount] = query.value(1).toString();
			item_lastname[nameCount] = query.value(2).toString();
			nameString[nameCount] = "InvID (" + QString::number( item_InvID[nameCount] ) + "), "+
							 item_firstname[nameCount] + " " + item_lastname[nameCount];
			nameCount++;
		}
	}

	if(nameCount > 0)
	{
		lb_name->clear();
		for ( int i=0; i<nameCount; i++)
		{
			lb_name->insertItem(nameString[i]);
		}
		select_flag = true;
		pb_commit->setEnabled(false);
	}
	else
	{
		lb_name->clear();
		lb_name->insertItem("Name not found in DB");
		lb_name->insertItem("Please try again...");
		pb_update->setEnabled(false);
		pb_commit->setEnabled(true);
		lbl_IdNumber->setText("");
		le_FirstName->setText("");
		le_Address->setText("");
		le_City->setText("");
		le_State->setText("");
		le_Zip->setText("");
		le_Phone->setText("");
		le_Email->setText("");
		new_register = 1;
	}

/*
	QSqlCursor cur_i( "tblInvestigators" );
   QStringList orderFields = QStringList() <<"FirstName" << "LastName";
   QSqlIndex order = cur_i.index( orderFields );
	if(!info_list.LastName.isEmpty())
	{
   	QSqlIndex filter = cur_i.index( "LastName" );
  		cur_i.setValue( "LastName", info_list.LastName );
   	cur_i.select( filter,order);
	}
	else
	{
		cur_i.select(order);
	}
	int maxID = get_newID("tblInvestigators", "InvID");
	nameString = new QString[maxID];
	item_InvID = new int[maxID];
	item_firstname = new QString[maxID];
	item_lastname = new QString[maxID];
	int nameCount = 0;
   while ( cur_i.next() )
   {

   	int id = cur_i.value( "InvID" ).toInt();
		//Display on QListBox
      nameString[nameCount] = "InvID (" + QString::number( id ) + "): "+
							 cur_i.value( "FirstName" ).toString() +
 							 " " + cur_i.value( "LastName" ).toString();

		item_InvID[nameCount] = id;
		item_firstname[nameCount] = cur_i.value("FirstName").toString();
		item_lastname[nameCount] = cur_i.value("LastName").toString();
		nameCount++;
   }
*/
}

/*!
	If you find your name in the ListBox by checkname(). Doubleclick it,
	you will get your all register information.
	\param item The number of items in ListBox, count start from 0.
*/
void US_DB_TblInvestigator::select_name(int item)
{
	if(select_flag)
	{
		info_list.InvID = item_InvID[item];
		info_list.FirstName = item_firstname[item];
		info_list.LastName = item_lastname[item];
		info_list.Display = nameString[item];
		emit valueChanged(info_list.Display, info_list.InvID);
		QSqlCursor cur( "tblInvestigators" );
   	QStringList orderFields = QStringList() <<"Address"<<"City"<<"State"<<"Zip"<<"Phone"<<"Email";
   	QSqlIndex order = cur.index( orderFields );
   	QSqlIndex filter = cur.index( "InvID" );
   	cur.setValue( "InvID", info_list.InvID );
		cur.select(filter, order);
		while(cur.next())
		{
			info_list.Address = cur.value("Address").toString();
			info_list.City = cur.value("City").toString();
			info_list.State = cur.value("State").toString();
			info_list.Zip = cur.value("Zip").toString();
			info_list.Phone = cur.value("Phone").toString();
			info_list.Email = cur.value("Email").toString();
		}

		lbl_IdNumber->setNum(info_list.InvID);
		le_FirstName->setText(info_list.FirstName);
		le_LastName->setText(info_list.LastName);
		le_Address->setText(info_list.Address);
		le_City->setText(info_list.City);
		le_State->setText(info_list.State);
		le_Zip->setText(info_list.Zip);
		le_Phone->setText(info_list.Phone);
		le_Email->setText(info_list.Email);
		pb_commit->setEnabled(false);
		pb_update->setEnabled(true);
		changed_flag = 0;
	}
	else
	{
		QMessageBox::message(tr("Attention:"),
									tr("No records available\n"));
		return;
	}
}

/*!
	Open US_DB_Admin to check delete permission.
*/
void US_DB_TblInvestigator::check_permission()
{
	US_DB_Admin *db_admin;
	db_admin = new US_DB_Admin("");
	db_admin->show();
	connect(db_admin, SIGNAL(issue_permission(bool)), SLOT(del(bool)));
}

/*! Delete selected entry of DB table: tblInvestigators. */
void US_DB_TblInvestigator::del(bool permission)
{
	QString str;
	if(!permission)
	{
		QMessageBox::message(tr("Attention:"),
									tr("Permission denied"));
		return;
	}
	int id = info_list.InvID;
	if(id == -1)
	{
		QMessageBox::message(tr("Attention:"),
									tr("First select a database entry\n"
										"to be deleted from the database"));

	}
	else
	{

	/*	QSqlCursor cur( "tblInvestigators");
		cur.setMode( QSqlCursor::Delete);
		QSqlIndex filter = cur.index("InvID");
		cur.setValue("InvID", id);
		cur.select(filter);
		cur.primeDelete();
*/
		switch(QMessageBox::information(this, tr("Do you want to delete this entry?"),
										tr("Clicking 'OK' will delete the information from the database"),
										tr("OK"), tr("CANCEL"),	0,1))
		{
			case 0:
			{
				//cur.del();
				QSqlQuery del;
				str.sprintf("DELETE FROM tblInvestigators WHERE InvID = %d;", id);
				bool done=del.exec(str);
				if(!done)
				{
					QSqlError sqlerr = del.lastError();
					QMessageBox::message(tr("Attention:"),
                           tr("Delete failed.\n"
                              "Attempted to execute this command:\n\n"
                              + str + "\n\n"
                              "Causing the following error:\n\n")
                              + sqlerr.text());
				}
				lb_name->clear();
				clear();
				break;
			}
			case 1:
			{
				break;
			}
		}
	}
}


/*! Open a netscape browser to load help page.*/
void US_DB_TblInvestigator::help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/tbl_investigator.html");
	close_db();
}

/*!
	Reset all values to empty.
*/
void US_DB_TblInvestigator::reset()
{
	clear();
	lb_name->clear();
	pb_commit->setEnabled(true);
	pb_update->setEnabled(false);
	changed_flag = 0;
	new_register = 1;
	select_flag = false;
}

/*!
	Update the old tblInvestigators entry with new values and exit.
*/
void US_DB_TblInvestigator::update()
{
	if (!check_fields())
	{
		return;
	}
	if(changed_flag == 1)
  	{
		QSqlCursor cur( "tblInvestigators");
		cur.setMode( QSqlCursor::Update);
		QSqlIndex filter = cur.index("InvID");
		cur.setValue("InvID", info_list.InvID);
		cur.select(filter);
		QSqlRecord *buf = cur.primeUpdate();

   	buf->setValue( "FirstName", info_list.FirstName );
   	buf->setValue( "LastName", info_list.LastName );
 		buf->setValue( "Address", info_list.Address );
 		buf->setValue( "City", info_list.City );
 		buf->setValue( "State", info_list.State );
 		buf->setValue( "Zip", info_list.Zip );
 		buf->setValue( "Phone", info_list.Phone );
 		buf->setValue( "Email", info_list.Email );

		info_list.Display = "InvID (" +QString::number(info_list.InvID) + "): " +
							 info_list.FirstName + " " + info_list.LastName;
		switch(QMessageBox::information(this, tr("Save the updated entry?"),
										tr("Clicking 'OK' will update the information\n"
											"for this investigator in the database."),
										tr("OK"), tr("CANCEL"),	0,1))
		{
			case 0:
			{
				cur.update();
				quit();
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
		QMessageBox::message(tr("Attention:"), tr("No changes recorded for this entry."));
		return;
	}
}

/*!
	Save all new investigator's register information to DB table: tblInvestigators and exit.
*/
void US_DB_TblInvestigator::save()
{
	if (!check_fields())
	{
		return;
	}
  	if( new_register == 1)
  	{
		QSqlCursor cur( "tblInvestigators");
		cur.setMode( QSqlCursor::Insert);
		QSqlRecord *buffer = cur.primeInsert();
//   	buffer->setValue( "InvID", newInvID );
   	buffer->setValue( "FirstName", info_list.FirstName );
   	buffer->setValue( "LastName", info_list.LastName );
 		buffer->setValue( "Address", info_list.Address );
 		buffer->setValue( "City", info_list.City );
 		buffer->setValue( "State", info_list.State );
 		buffer->setValue( "Zip", info_list.Zip );
 		buffer->setValue( "Phone", info_list.Phone );
 		buffer->setValue( "Email", info_list.Email );

		info_list.Display = "InvID (" +QString::number(newInvID) + "): " +
							 info_list.FirstName + " " + info_list.LastName;
  	  	info_list.InvID = newInvID;
		switch(QMessageBox::information(this, tr("Save this entry?"),
										tr("Clicking 'OK' will save the information\n"
											"for this investigator to the database."),
										tr("OK"), tr("CANCEL"),	0,1))
		{
			case 0:
			{
				cur.insert();

				//Insert investigator information into tblAuth
				QDateTime now = QDateTime::currentDateTime();
				QSqlQuery target;
				QString str;
				str = "INSERT INTO tblAuth(InvestigatorID, Status, Classification, Balance, Activated, Userlevel, Signup, LastLogin, Password) VALUES(";
				str+= QString::number(newInvID)+", ";
				str+=	"'new', 'Academic User', ";
				str+= "00000.00, 1, 0, ";
				str+= "'"+now.toString(Qt::ISODate)+"', '"+now.toString(Qt::ISODate)+"', ";
				str+="'ffdbd2aadf002da88ffce9b31d3d7499');";
				target.exec(str);

				quit();
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
		QMessageBox::message(tr("Attention:"), tr("Please check your last name first!"));
		return;
	}
}

/*! Clear struct US_InvestigatorData to empty. */
void US_DB_TblInvestigator::clear()
{
	info_list.InvID = -1;
	info_list.LastName = "";
	info_list.FirstName = "";
	info_list.Address = "";
	info_list.City = "";
	info_list.State = "";
	info_list.Zip = "";
	info_list.Phone = "";
	info_list.Email = "";
	info_list.Display = "";

	lbl_IdNumber->setText("");
	le_FirstName->setText("");
	le_LastName->setText("");
	le_Address->setText("");
	le_City->setText("");
	le_State->setText("");
	le_Zip->setText("");
	le_Phone->setText("");
	le_Email->setText("");
}

/*! Emit valueChanged(), clean all values and exit.*/
void US_DB_TblInvestigator::quit()
{
		emit valueChanged(info_list.Display, info_list.InvID);
		clear();
		accept();
}

bool US_DB_TblInvestigator::check_fields()
{
	if(info_list.FirstName.isEmpty())
	{
		QMessageBox::message(tr("Attention:"), tr("Please enter a first name\n"
													  			"before committing data!"));
		return(false);
	}
	if(info_list.LastName.isEmpty())
	{
		QMessageBox::message(tr("Attention:"), tr("Please enter a last name\n"
													  			"before committing data!"));
		return(false);
	}
	if(info_list.Address.isEmpty())
	{
		QMessageBox::message(tr("Attention:"), tr("Please enter an address\n"
													  			"before committing data!"));
		return(false);
	}
	if(info_list.City.isEmpty())
	{
		QMessageBox::message(tr("Attention:"), tr("Please enter a city\n"
													  			"before committing data!"));
		return(false);
	}
	if(info_list.State.isEmpty())
	{
		QMessageBox::message(tr("Attention:"), tr("Please enter a state\n"
													  			"before committing data!"));
		return(false);
	}
	if(info_list.Zip.isEmpty())
	{
		QMessageBox::message(tr("Attention:"), tr("Please enter a zip code\n"
													  			"before committing data!"));
		return(false);
	}
	if(info_list.Phone.isEmpty())
	{
		QMessageBox::message(tr("Attention:"), tr("Please enter a phone number\n"
													  			"before committing data!"));
		return(false);
	}
	if(info_list.Email.isEmpty())
	{
		QMessageBox::message(tr("Attention:"), tr("Please enter an e-mail address\n"
													  			"before committing data!"));
		return(false);
	}
	return(true);
}
