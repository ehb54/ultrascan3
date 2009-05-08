#include "../include/us_db_rtv_investigator.h"

//! Constructor
/*! 
  Constractor a new <var>US_DB_RtvInvestigator</var> interface, 
  with <var>p</var> as a parent and <var>us_rtvinvestigator</var> as object name. 
*/ 
US_DB_RtvInvestigator::US_DB_RtvInvestigator(QWidget *p, const char *name) : US_DB( p, name)
{
   int xpos, ypos, buttonw = 340,  buttonh = 26;
   
   sel_name = false;      // use for query name listbox select
   check_ID = false;      // use for control click check experimental data button
   sel_data = false;      //use for query data listbox select
   retrieve_flag = false;//use for control 'Retrieve' pushbutton
   InvID = -1;
   ExpdataID = -1;
   LastName = "";
   Display ="";
   
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption("Retrieve by Investigator");
             
   xpos = border;
   ypos = border;   
   
   QString str =tr("Retrieve Experimental Data by Investigator\nfrom DB: ");
   str.append(login_list.dbname);
   lbl_blank = new QLabel(str, this);
   lbl_blank->setAlignment(AlignCenter|AlignVCenter);
   //   lbl_blank->setFrameStyle(QFrame::WinPanel|Raised);
   lbl_blank->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_blank->setGeometry(xpos, ypos, buttonw, buttonh*2);
   lbl_blank->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   xpos = border;
   ypos += buttonh*2 + spacing*2;

   lbl_LastName = new QLabel(tr(" Investigator's last name:"),this);
   lbl_LastName->setAlignment(AlignLeft|AlignVCenter);
   lbl_LastName->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_LastName->setGeometry(xpos, ypos, buttonw/2, buttonh);
   lbl_LastName->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   xpos += buttonw/2;

   le_LastName = new QLineEdit(this, "LastName");
   le_LastName->setGeometry(xpos, ypos, buttonw/2, buttonh);
   le_LastName->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect (le_LastName, SIGNAL(textChanged(const QString &)), SLOT(update_lastname(const QString &)));
   connect (le_LastName, SIGNAL(returnPressed()), SLOT(checkname()));

   xpos = border;
   ypos += buttonh + spacing * 2;

   pb_chkname = new QPushButton(tr("Check Database for Investigator Record"), this);
   pb_chkname->setAutoDefault(false);
   pb_chkname->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_chkname->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_chkname->setGeometry(xpos, ypos, buttonw, buttonh);
   pb_chkname->setEnabled(true);
   connect(pb_chkname, SIGNAL(clicked()), SLOT(checkname()));
   
   xpos = border;
   ypos += buttonh + spacing * 2;
   
   lbl_instr1 = new QLabel(tr("Doubleclick on item to select:"),this);
   //   lbl_instr1->setFrameStyle(QFrame::WinPanel|Raised);
   lbl_instr1->setAlignment(AlignCenter|AlignVCenter);
   lbl_instr1->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_instr1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_instr1->setGeometry(xpos, ypos, buttonw, buttonh);

   xpos = border;
   ypos += buttonh;
   
   lb_name = new QListBox(this, "Names");
   lb_name->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lb_name->setGeometry(xpos, ypos, buttonw, buttonh*3);
   lb_name->setSelected(0, true);
   connect(lb_name, SIGNAL(highlighted(int)), SLOT(select_name(int)));
   connect(lb_name, SIGNAL(selected(int)), SLOT(check_data(int)));

   
   xpos = border;
   ypos += buttonh*3 + spacing * 2;

   lbl_investigator = new QLabel(" Nothing selected",this);
   lbl_investigator->setAlignment(AlignLeft|AlignVCenter);
   //   lbl_investigator->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_investigator->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_investigator->setGeometry(xpos, ypos, buttonw+spacing, buttonh);
   lbl_investigator->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   xpos = border;
   ypos += buttonh + spacing * 2;
   
   pb_chkID = new QPushButton(tr("Search for Data in Database"), this);
   pb_chkID->setAutoDefault(false);
   pb_chkID->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_chkID->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_chkID->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_chkID, SIGNAL(clicked()), SLOT(checkID()));
   
   xpos = border;
   ypos += buttonh + spacing * 2;
   
   lbl_instr2 = new QLabel(tr("Doubleclick on item to select:"),this);
   //   lbl_instr2->setFrameStyle(QFrame::WinPanel|Raised);
   lbl_instr2->setAlignment(AlignCenter|AlignVCenter);
   lbl_instr2->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_instr2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_instr2->setGeometry(xpos, ypos, buttonw, buttonh);
   
   xpos = border;
   ypos += buttonh;
   
   lb_data = new QListBox(this, "data");
   lb_data->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lb_data->setGeometry(xpos, ypos, buttonw, buttonh*3);
   lb_data->setSelected(0, true);
   connect(lb_data, SIGNAL(highlighted(int)), SLOT(select_data(int)));
   connect(lb_data, SIGNAL(selected(int)), SLOT(select_data(int)));
   
   xpos = border;
   ypos += buttonh * 3 + spacing * 2;
   
   lbl_item = new QLabel(" Nothing selected",this);
   lbl_item->setAlignment(AlignLeft|AlignVCenter);
   //   lbl_item->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_item->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_item->setGeometry(xpos, ypos, buttonw+spacing, buttonh);
   lbl_item->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   xpos = border;
   ypos += buttonh + spacing * 2;
   
   pb_reset = new QPushButton(tr("Reset"), this);
   pb_reset->setAutoDefault(false);
   pb_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_reset->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_reset->setGeometry(xpos, ypos, buttonw/2-1, buttonh);
   pb_reset->setEnabled(true);
   connect(pb_reset, SIGNAL(clicked()), SLOT(reset()));

   xpos +=buttonw/2 + spacing;

   pb_retrieve = new QPushButton(tr("Retrieve"), this);
   pb_retrieve->setAutoDefault(false);
   pb_retrieve->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_retrieve->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_retrieve->setGeometry(xpos, ypos, buttonw/2-1, buttonh);
   pb_retrieve->setEnabled(true);
   connect(pb_retrieve, SIGNAL(clicked()), SLOT(retrieve()));


   xpos = border;
   ypos += buttonh + spacing;
      
   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setAutoDefault(false);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_help->setGeometry(xpos, ypos, buttonw/2-1, buttonh);
   pb_help->setEnabled(true);
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   xpos += buttonw/2 + spacing;

   pb_close = new QPushButton(tr("Close"), this);
   pb_close->setAutoDefault(false);
   pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_close->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_close->setGeometry(xpos, ypos, buttonw/2-1, buttonh);
   pb_close->setEnabled(true);
   connect(pb_close, SIGNAL(clicked()), SLOT(quit()));
   
   xpos = buttonw +2*border;
   ypos += buttonh + spacing;
   setMinimumSize(xpos, ypos);
   
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, xpos, ypos);

   setup_GUI();
}

//! Destructor
/*! destroy the US_DB_RtvInvestigator. */
US_DB_RtvInvestigator::~US_DB_RtvInvestigator()
{
   delete nameString;
   delete item_InvID;
   delete item_ExpdataID;
   delete display_Str;
   delete expdata_dlg;
}

void US_DB_RtvInvestigator::setup_GUI()
{
   Grid = new QGridLayout(this,12,2,4,2);
   Grid->addMultiCellWidget(lbl_blank,0,0,0,1);
   Grid->addWidget(lbl_LastName,1,0);
   Grid->addWidget(le_LastName,1,1);
   Grid->addMultiCellWidget(pb_chkname,2,2,0,1);
   Grid->addMultiCellWidget(lbl_instr1,3,3,0,1);
   Grid->addMultiCellWidget(lb_name,4,4,0,1);
   Grid->addMultiCellWidget(lbl_investigator,5,5,0,1);
   Grid->addMultiCellWidget(pb_chkID,6,6,0,1);
   Grid->addMultiCellWidget(lbl_instr2,7,7,0,1);
   Grid->addMultiCellWidget(lb_data,8,8,0,1);
   Grid->addMultiCellWidget(lbl_item,9,9,0,1);
   Grid->addWidget(pb_reset,10,0);
   Grid->addWidget(pb_retrieve,10,1);
   Grid->addWidget(pb_help,11,0);
   Grid->addWidget(pb_close,11,1);
   Grid->activate();
}
/*!
  Update <var>LastName</var> by LineEdit input.
*/
void US_DB_RtvInvestigator::update_lastname(const QString &newText)
{
   LastName = newText;   
}

/*!
  If the last name is found, this function will list all investigator's name matching that last name.
  If the last name is not found, no investigator name will be returned.
  If no last name is input, this function will list all investigator name stored in database.
*/
void US_DB_RtvInvestigator::checkname()
{
   QString str, *item_firstname, *item_lastname;
   int maxID = get_newID("tblInvestigators", "InvID");
   nameString = new QString[maxID];
   item_InvID = new int[maxID];
   item_firstname = new QString[maxID];
   item_lastname = new QString[maxID];
   int nameCount = 0;
   LastName = le_LastName->text();
   if(LastName != "")
   {
      str = "SELECT InvID, FirstName, LastName FROM tblInvestigators WHERE LastName = '" + LastName + 
         "' ORDER BY FirstName;";
   }
   else
   {
      str = "SELECT InvID, FirstName, LastName FROM tblInvestigators ORDER BY FirstName;";
   }
   QSqlQuery query(str);
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
      sel_name = true;      // you got item to select
   }
   else
   {
      lb_name->clear();
      lb_name->insertItem("This investigator was not found in the database");
   }
}
/*!
  If the investigator list name was double clicked, 
  this function will do  both select_name() and checkID() for your convenience.
*/
void US_DB_RtvInvestigator::check_data(int item)
{
   select_name(item);
   checkID();
}

/*!
  If you find the investigator name in ListBox, doubleclick it.
  This investigator will be selected as condition for retrieving experimental data.
*/
void US_DB_RtvInvestigator::select_name(int item)
{
   QString str;
   if(sel_name)
   {
      InvID = item_InvID[item];      
      str.sprintf("Investigator: "+nameString[item]);
      lbl_investigator->setText(str);
      name = str.remove(0, str.find(",", 0)+2);
      str = "Check data for " + name;
      pb_chkID->setText(str);
      lb_data->clear();
      check_ID = true;         
   }
}

/*!
  If the investigator has experimental data, this function will list all of them.
  If the investigator has not experimental data, no data will be returned.
*/
void US_DB_RtvInvestigator::checkID()
{
   QString str, *item_description;
   int maxID = get_newID("tblExpData", "ExpdataID");
   int count = 0;
   item_ExpdataID = new int[maxID];
   item_description = new QString[maxID];
   display_Str = new QString[maxID];

   if(check_ID)
   {
      str.sprintf("SELECT ExpdataID, Description FROM tblExpData WHERE Invid = %d;", InvID);
      QSqlQuery query(str);
      if(query.isActive())
      {
         while(query.next())
         {
            item_ExpdataID[count] = query.value(0).toInt();
            item_description[count] = query.value(1).toString();
            display_Str[count] = "ExpDataID (" + QString::number( item_ExpdataID[count] ) + "), "+item_description[count];
            count++;
         }
      }

      if(count>0)
      {
         lb_data->clear();
         for( int i=0; i<count; i++)
         {
            lb_data->insertItem(display_Str[i]);
         }   
         sel_data = true;
      }
      else
      {   
         QString str;
         str = "No database records available for: " + name;
         lb_data->clear();
         lb_data->insertItem(str);
      }
   }
   else
   {
      QMessageBox::message(tr("Attention:"), 
                           tr("Please select an investigator first!\n"));
      return;
   }
}

/*!
  If you find your expected experimental data in ListBox, doubleclick it.
  This item will be selected for your retrieving.
*/
void US_DB_RtvInvestigator::select_data(int item)
{
   QString str;
   if(sel_data)
   {
      ExpdataID = item_ExpdataID[item];
      Display = display_Str[item];      
      str = "Retrieve: " + Display;
      lbl_item->setText(str);
      retrieve_flag = true;         
   }
   else
   {
      QMessageBox::message(tr("Attention:"), 
                           tr("No Experimental Data has been selected\n"));
      return;
   }
}

/*!
  Retrieve the selected experimental data from database. Pop-up US_ExpData_DB interface.
*/
void US_DB_RtvInvestigator::retrieve()
{
   if(retrieve_flag)
   {
      expdata_dlg = new US_ExpData_DB();
      if(expdata_dlg->retrieve_all(ExpdataID, Display))
      {
         expdata_dlg->show();
      }
      else
      {
         return;
      }
   }
   else
   {
      QMessageBox::message(tr("Attention:"), 
                           tr("You have to select a dataset first!\n"));
      return;
   }
}

/*!
  Reset all variables to initialization status.
*/
void US_DB_RtvInvestigator::reset()
{
   sel_name = false;   
   check_ID = false;      
   sel_data = false;      
   retrieve_flag = false;
   InvID = -1;
   ExpdataID = -1;
   LastName = "";
   Display ="";
   le_LastName->setText("");
   lbl_item->setText(tr(" Nothing selected"));
   lbl_investigator->setText(tr("Investigator: not selected"));
   pb_chkID->setText(tr("Check for experimental data"));
   lb_name->clear();
   lb_data->clear();
}

/*! Open a netscape browser to load help page.*/
void US_DB_RtvInvestigator::help()
{
   US_Help *online_help; online_help = new US_Help(this);
   online_help->show_help("manual/rtv_investigator.html");
}

/*! Close the interface. */         
void US_DB_RtvInvestigator::quit()
{
   close();
}

