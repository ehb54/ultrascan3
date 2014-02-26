#include "../include/us_db_rtv_description.h"

//! Constructor
/*! 
  Constractor a new <var>US_DB_RtvDescription</var> interface, 
  with <var>p</var> as a parent and <var>us_rtvdescription</var> as object name. 
*/ 
US_DB_RtvDescription::US_DB_RtvDescription(QWidget *p, const char *name) : US_DB( p, name)
{
   int xpos, ypos, buttonw = 360,  buttonh = 26;
   
   sel_data = false;      //use for query data listbox select
   retrieve_flag = false;//use for control 'Retrieve' pushbutton
   ExpdataID = -1;
   Description = "";
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
         
   xpos = border;
   ypos = border;   
   
   QString str =tr("Retrieve Experimental Data by Description\nfrom the DB: ");
   str.append(login_list.dbname);
   lbl_blank = new QLabel(str, this);
   lbl_blank->setAlignment(AlignCenter|AlignVCenter);
   //   lbl_blank->setFrameStyle(QFrame::WinPanel|Raised);
   lbl_blank->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_blank->setGeometry(xpos, ypos, buttonw, buttonh*2);
   lbl_blank->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   xpos = border;
   ypos += buttonh*2 + spacing*2;

   lbl_description = new QLabel(tr(" Enter the description keyword:"),this);
   lbl_description->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_description->setGeometry(xpos, ypos, buttonw/2, buttonh);
   lbl_description->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   xpos += buttonw/2;

   le_description= new QLineEdit(this, "Description");;
   le_description->setAlignment(AlignLeft|AlignVCenter);   
   le_description->setGeometry(xpos, ypos, buttonw/2, buttonh);
   le_description->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   le_description->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   connect (le_description, SIGNAL(textChanged(const QString &)), SLOT(update_description(const QString &)));
   connect (le_description, SIGNAL(returnPressed()), SLOT(checkDescription()));

   xpos = border;
   ypos += buttonh + spacing;
   
   pb_chkDescription = new QPushButton(tr("Check for experimental data"), this);
   pb_chkDescription->setAutoDefault(false);
   pb_chkDescription->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_chkDescription->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_chkDescription->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_chkDescription, SIGNAL(clicked()), SLOT(checkDescription()));
   
   xpos = border;
   ypos += buttonh;
   
   lbl_instr = new QLabel(tr("Doubleclick on item to select:"),this);
   lbl_instr->setAlignment(AlignCenter|AlignVCenter);
   lbl_instr->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_instr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_instr->setGeometry(xpos, ypos, buttonw, buttonh);
   
   xpos = border;
   ypos += buttonh;
   
   lb_data = new QListBox(this, "data");
   lb_data->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lb_data->setGeometry(xpos, ypos, buttonw, buttonh*4);
   lb_data->setSelected(0, true);
   connect(lb_data, SIGNAL(selected(int)), SLOT(select_data(int)));
   
   xpos = border;
   ypos += buttonh*4 + spacing;
   
   lbl_item = new QLabel("Retrieve: not selected",this);
   lbl_item->setAlignment(AlignLeft|AlignVCenter);
   //   lbl_item->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_item->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_item->setGeometry(xpos, ypos, buttonw+spacing, buttonh);
   lbl_item->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   xpos = border;
   ypos += buttonh + spacing;
   
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
/*! destroy the US_DB_RtvDescription. */
US_DB_RtvDescription::~US_DB_RtvDescription()
{
   delete item_ExpdataID;
   delete display_Str;
   delete expdata_dlg;
}
void US_DB_RtvDescription::setup_GUI()
{
   QGridLayout * Grid = new QGridLayout(this,8,2,4,2);
   Grid->addMultiCellWidget(lbl_blank,0,0,0,1);
   Grid->addWidget(lbl_description,1,0);
   Grid->addWidget(le_description,1,1);
   Grid->addMultiCellWidget(pb_chkDescription,2,2,0,1);
   Grid->addMultiCellWidget(lbl_instr,3,3,0,1);
   Grid->addMultiCellWidget(lb_data,4,4,0,1);
   Grid->addMultiCellWidget(lbl_item,5,5,0,1);
   Grid->addWidget(pb_reset,6,0);
   Grid->addWidget(pb_retrieve,6,1);
   Grid->addWidget(pb_help,7,0);
   Grid->addWidget(pb_close,7,1);
   Grid->activate();
}

/*!
  Update <var>Description</var> by LineEdit input.
*/
void US_DB_RtvDescription::update_description(const QString &newText)
{
   Description = newText;
   lb_data->clear();
}

/*!
  If the description is found, this function will list that particular experimental data.
  If the description is not found, no experimental data will be returned.
  If no description is input, this function will list all experimental data stored in database.
*/
void US_DB_RtvDescription::checkDescription()
{
   QSqlCursor cur( "tblExpData" );
   QSqlIndex order = cur.index( "Description" );
   if(!Description.isEmpty())
   {
      //QSqlIndex filter = cur.index( "Description" );
      //cur.setValue( "Description", Description );
      //cur.select(filter,order);
      QString str="Description LIKE '%"+Description+"%'";
      cur.select(str, order);
   }
   else
   {
      cur.select(order);
   }
   int maxID = get_newID("tblExpData", "ExpDataID");
   int count = 0;
   item_ExpdataID = new int[maxID];
   display_Str = new QString[maxID];
   
   while(cur.next() )
   {
      int id = cur.value("ExpdataID").toInt();
      display_Str[count] = "ExpDataID (" + QString::number( id ) + "), "+   
         cur.value( "Description" ).toString();
      item_ExpdataID[count] = id;
      count++;               
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
      QString str1 = "No data matches this description keyword: " + Description;
      lb_data->clear();
      lb_data->insertItem(str1);
   }   
}

/*!
  If you find your expected experimental data in ListBox, doubleclick it.
  This item will be selected for your retrieving.
*/
void US_DB_RtvDescription::select_data(int item)
{
   QString str;
   if(sel_data)
   {
      ExpdataID = item_ExpdataID[item];
      Display = display_Str[item];      
      str.sprintf("Retrieve: "+Display);
      lbl_item->setText(str);
      retrieve_flag = true;         
   }
   else
   {
      QMessageBox::message(tr("Attention:"), 
                           tr("No experimental data were selected\n"));
      return;
   }
}

/*!
  Retrieve the selected experimental data from database. Pop-up US_ExpData_DB interface.
*/
void US_DB_RtvDescription::retrieve()
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
void US_DB_RtvDescription::reset()
{      
   sel_data = false;      
   retrieve_flag = false;
   ExpdataID = -1;
   Description = "";
   le_description->setText("");
   lbl_item->setText(tr(" not selected"));
   lb_data->clear();
}

/*! Open a netscape browser to load help page.*/
void US_DB_RtvDescription::help()
{
   US_Help *online_help; online_help = new US_Help(this);
   online_help->show_help("manual/rtv_description.html");
}

/*! Close the interface. */         
void US_DB_RtvDescription::quit()
{
   close();
}

