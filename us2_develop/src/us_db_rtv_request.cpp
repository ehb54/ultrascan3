#include "../include/us_db_rtv_request.h"

//! Constructor
/*! 
  Constractor a new <var>US_DB_RtvRequest</var> interface, 
  with <var>p</var> as a parent and <var>us_rtvrequest</var> as object name. 
*/ 
US_DB_RtvRequest::US_DB_RtvRequest(QWidget *p, const char *name) : US_DB_RtvInvestigator( p, name)
{
   req_info.RequestID =0;
   QString str =tr("Show Investigator Request Information\nfrom DB:");
   str.append(login_list.dbname);
   lbl_blank->setText(str);
   
   lb_name->disconnect();
   connect(lb_name, SIGNAL(highlighted(int)), SLOT(select_name(int)));
   connect(lb_name, SIGNAL(selected(int)), SLOT(check_request(int)));

   pb_chkID->disconnect();
   connect(pb_chkID, SIGNAL(clicked()), SLOT(checkRequest()));
   lb_data->disconnect();
   connect(lb_data, SIGNAL(highlighted(int)), SLOT(select_request(int)));
   connect(lb_data, SIGNAL(selected(int)), SLOT(select_request(int)));

   pb_retrieve->disconnect();
   pb_retrieve->setText("Show Info");
   connect(pb_retrieve, SIGNAL(clicked()), SLOT(show_info()));
   
   pb_edit = new QPushButton(tr("Edit Notes"), this);
   pb_edit->setAutoDefault(false);
   pb_edit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_edit->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_edit->setEnabled(true);
   connect(pb_edit, SIGNAL(clicked()), SLOT(edit_notes()));
   
   pb_status = new QPushButton(tr("Check Status"), this);
   pb_status->setAutoDefault(false);
   pb_status->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_status->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_status->setEnabled(true);
   connect(pb_status, SIGNAL(clicked()), SLOT(check_status()));

   get_GUI();
}

//! Destructor
/*! destroy the US_DB_RtvRequest. */
US_DB_RtvRequest::~US_DB_RtvRequest()
{
}
void US_DB_RtvRequest::get_GUI()
{
   int xpos, ypos, buttonw = 340,  buttonh = 26;
   xpos = border;
   ypos = border;
   lbl_blank->setGeometry(xpos, ypos, buttonw, buttonh*2);
   xpos = border;
   ypos += buttonh*2 + spacing*2;
   lbl_LastName->setGeometry(xpos, ypos, buttonw/2, buttonh);
   xpos += buttonw/2;
   le_LastName->setGeometry(xpos, ypos, buttonw/2, buttonh);
   xpos = border;
   ypos += buttonh + spacing * 2;
   pb_chkname->setGeometry(xpos, ypos, buttonw, buttonh);
   xpos = border;
   ypos += buttonh + spacing * 2;
   lbl_instr1->setGeometry(xpos, ypos, buttonw, buttonh);
   
   xpos = border;
   ypos += buttonh;
   lb_name->setGeometry(xpos, ypos, buttonw, buttonh*3);
   xpos = border;
   ypos += buttonh*3 + spacing * 2;
   lbl_investigator->setGeometry(xpos, ypos, buttonw+spacing, buttonh);
   xpos = border;
   ypos += buttonh + spacing * 2;
   pb_chkID->setGeometry(xpos, ypos, buttonw, buttonh);
   xpos = border;
   ypos += buttonh + spacing * 2;
   lb_data->setGeometry(xpos, ypos, buttonw, buttonh*3);
   xpos = border;
   ypos += buttonh * 3 + spacing * 2;
   lbl_item->setGeometry(xpos, ypos, buttonw+spacing, buttonh);
   xpos = border;
   ypos += buttonh + spacing * 2;   
   pb_edit->setGeometry(xpos, ypos, buttonw/2-1, buttonh);
   xpos +=buttonw/2 + spacing;
   pb_retrieve->setGeometry(xpos, ypos, buttonw/2-1, buttonh);
   xpos = border;
   ypos += buttonh + spacing;
   pb_reset->setGeometry(xpos, ypos, buttonw/2-1, buttonh);      
   xpos += buttonw/2 + spacing;
   pb_status->setGeometry(xpos, ypos, buttonw/2-1, buttonh);
   xpos = border;
   ypos += buttonh + spacing;
   pb_help->setGeometry(xpos, ypos, buttonw/2-1, buttonh);   
   xpos += buttonw/2 + spacing;
   pb_close->setGeometry(xpos, ypos, buttonw/2-1, buttonh);

   
   xpos = buttonw +2*border;
   ypos += buttonh + spacing;
   setMinimumSize(xpos, ypos);
   
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, xpos, ypos);
   setup_GUI();
}

void US_DB_RtvRequest::setup_GUI()
{
   //QGridLayout *Grid = new QGridLayout(this,12,2,4,2);
   Grid->setResizeMode(QLayout::Auto);
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
   Grid->addWidget(pb_edit,10,0);
   Grid->addWidget(pb_retrieve,10,1);
   Grid->addWidget(pb_reset,11,0);
   Grid->addWidget(pb_status,11,1);
   Grid->addWidget(pb_help,12,0);
   Grid->addWidget(pb_close,12,1);
   Grid->activate();

}
void US_DB_RtvRequest::check_request(int item)
{
   select_name(item);
   checkRequest();
}

void US_DB_RtvRequest::checkRequest()
{
   QString str, *item_description;
   int maxID = get_newID("tblRequest", "RequestID");
   int count = 0;
   item_RequestID = new int[maxID];
   item_description = new QString[maxID];
   display_Str = new QString[maxID];

   if(check_ID)
   {
      str.sprintf("SELECT RequestID, ProjectDescription FROM tblRequest WHERE InvestigatorID = %d;", InvID);
      QSqlQuery query(str);
      if(query.isActive())
      {
         while(query.next())
         {
            item_RequestID[count] = query.value(0).toInt();
            item_description[count] = query.value(1).toString();
            display_Str[count] = "(" + QString::number( item_RequestID[count] ) + "), "+item_description[count];
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


void US_DB_RtvRequest::select_request(int item)
{
   QString str;
   if(sel_data)
   {
      req_info.RequestID = item_RequestID[item];
      Display = display_Str[item];      
      str.sprintf("Retrieve: "+ Display);
      lbl_item->setText(str);
      retrieve_flag = true;         
   }
   else
   {
      QMessageBox::message(tr("Attention:"), 
                           tr("No Request Data has been selected\n"));
      return;
   }
}

void US_DB_RtvRequest::check_status()
{
   if(!retrieve_flag)
   {
      QMessageBox::message(tr("Attention:"), 
                           tr("You have to select a dataset first!\n"));
      return;
   }
   US_RequestStatus *us_requestStatus;
   us_requestStatus = new US_RequestStatus(req_info.RequestID);
   us_requestStatus->setCaption("Edit Project Status");
   us_requestStatus->show();


}
void US_DB_RtvRequest::edit_notes()
{
   if(!retrieve_flag)
   {
      QMessageBox::message(tr("Attention:"), 
                           tr("You have to select a dataset first!\n"));
      return;
   }
   QString str;
   str.sprintf("SELECT Notes FROM tblRequest WHERE RequestID = %d;", req_info.RequestID);
   
   QSqlQuery query(str);
   if(query.isActive())
   {
      if(query.next())
      {
         req_info.Note = query.value(0).toString();
      }
      TextEdit *e;
      e = new TextEdit(req_info.RequestID);
      e->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
      e->setGeometry(global_Xpos + 30, global_Ypos + 30, 540, 400);
      e->load_text(req_info.Note);
      e->show();

   }
   else
   {
      QMessageBox::message(tr("Warning:"), 
                           tr("Problem to query tblRequest!\n"));
      return;
   }
   
}
void US_DB_RtvRequest::show_info()
{
   if(!retrieve_flag)
   {
      QMessageBox::message(tr("Attention:"), 
                           tr("You have to select a dataset first!\n"));
      return;
   }

   QString str;
   str.sprintf("SELECT Goals, Molecules, Purity, Expense, BufferComponents, SaltInformation, AUCQuestions, Notes, ProjectDescription FROM tblRequest WHERE RequestID = %d;", req_info.RequestID);
   
   QSqlQuery query(str);
   if(query.isActive())
   {
      if(query.next())
      {
         req_info.Goal = query.value(0).toString();
         req_info.Molecule = query.value(1).toString();   
         req_info.Purity = query.value(2).toString();         
         req_info.Expense = query.value(3).toString();
         req_info.Buffer = query.value(4).toString();
         req_info.Salt = query.value(5).toString();
         req_info.AUC = query.value(6).toString();
         req_info.Note = query.value(7).toString();
         req_info.Description = query.value(8).toString();
      }
   }
   else
   {
      QMessageBox::message(tr("Warning:"), 
                           tr("Problem to query tblRequest!\n"));
      return;
   }
   QString showText;
   showText = "**************************************************************************\n";
   showText+= "*                           Project Request Information                              *\n";
   showText+= "**************************************************************************\n";
   showText+= " Project description :\n";
   showText+= req_info.Description+"\n\n";
   showText+= "---------------------------------------------------------------------------------------------------------\n";
   showText+= " Reasearch Goals :\n\n";
   showText+= req_info.Goal+"\n";
   showText+= "---------------------------------------------------------------------------------------------------------\n";
   showText+= " Proteins, DNA, MW :\n\n";
   showText+= req_info.Molecule+"\n";
   showText+= "---------------------------------------------------------------------------------------------------------\n";
   showText+= " Purity :\n\n";
   showText+= req_info.Purity+"\n";
   showText+= "---------------------------------------------------------------------------------------------------------\n";   
   showText+= " Expense Info :\n\n";
   showText+= req_info.Expense+"\n";
   showText+= "---------------------------------------------------------------------------------------------------------\n";
   showText+= " Buffer Info :\n\n";
   showText+= req_info.Buffer+"\n";
   showText+= "---------------------------------------------------------------------------------------------------------\n";
   showText+= " Salt Info :\n\n";
   showText+= req_info.Salt+"\n";
   showText+= "---------------------------------------------------------------------------------------------------------\n";
   showText+= " Questions for AUC :\n\n";
   showText+= req_info.AUC+"\n";
   showText+= "---------------------------------------------------------------------------------------------------------\n";
   showText+= " Special Instructions :\n\n";
   showText+= req_info.Note+"\n";
   
   TextEdit *e;
   e = new TextEdit();
   e->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   e->setGeometry(global_Xpos + 30, global_Ypos + 30, 685, 600);
   e->load_text(showText);
   e->show();
   
   
}

/*************************************************************************************************/

US_RequestStatus::US_RequestStatus(int temp_RID, QWidget *p, const char *name) : US_DB(p, name)
{
   int border=2, spacing=2;
   int xpos = border, ypos = border;
   int buttonw = 240, buttonh = 26;
   RID = temp_RID;
   QString str;
   str.sprintf("SELECT Status FROM tblRequest WHERE RequestID = %d;", RID);
   
   QSqlQuery query(str);
   if(query.isActive())
   {
      if(query.next())
      {
         Status = query.value(0).toString();
      }
   }
   else
   {
      QMessageBox::message(tr("Warning:"), 
                           tr("Problem to query tblRequest!\n"));
      return;
   }
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_title = new QLabel("The Status of Project : "+QString::number(RID), this);
   lbl_title->setAlignment(AlignCenter|AlignVCenter);
   //   lbl_title->setFrameStyle(QFrame::WinPanel|Raised);
   lbl_title->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_title->setGeometry(xpos, ypos, buttonw, buttonh*2);
   lbl_title->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   xpos = border;
   ypos += buttonh*2 + spacing;
   cb1 = new QCheckBox(this);
   cb1->setGeometry(xpos+10, ypos+5, 15, 15);
   cb1->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb1, SIGNAL(clicked()), SLOT(select_cb1()));

   xpos+=32;
   lbl_s1 = new QLabel(tr(" Submitted"),this);
   lbl_s1->setAlignment(AlignLeft|AlignVCenter);
   lbl_s1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_s1->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_s1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   xpos = border;
   ypos += buttonh + spacing;
   cb2 = new QCheckBox(this);
   cb2->setGeometry(xpos+10, ypos+5, 15, 15);
   cb2->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb2, SIGNAL(clicked()), SLOT(select_cb2()));

   xpos+=32;
   lbl_s2 = new QLabel(tr(" Designed"),this);
   lbl_s2->setAlignment(AlignLeft|AlignVCenter);
   lbl_s2->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_s2->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_s2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   xpos = border;
   ypos += buttonh + spacing;
   
   cb3 = new QCheckBox(this);
   cb3->setGeometry(xpos+10, ypos+5, 15, 15);
   cb3->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb3->setChecked(selected);
   connect(cb3, SIGNAL(clicked()), SLOT(select_cb3()));

   xpos+=32;
   lbl_s3 = new QLabel(tr(" Scheduled"),this);
   lbl_s3->setAlignment(AlignLeft|AlignVCenter);
   lbl_s3->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_s3->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_s3->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));   
   
   xpos = border;
   ypos += buttonh + spacing;
   cb4 = new QCheckBox(this);
   cb4->setGeometry(xpos+10, ypos+5, 15, 15);
   cb4->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb4, SIGNAL(clicked()), SLOT(select_cb4()));

   xpos+=32;
   lbl_s4 = new QLabel(tr(" Uploaded"),this);
   lbl_s4->setAlignment(AlignLeft|AlignVCenter);
   lbl_s4->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_s4->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_s4->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   
   xpos = border;
   ypos += buttonh + spacing;
   cb5 = new QCheckBox(this);
   cb5->setGeometry(xpos+10, ypos+5, 15, 15);
   cb5->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb5, SIGNAL(clicked()), SLOT(select_cb5()));

   xpos+=32;
   lbl_s5 = new QLabel(tr(" Analyzed"),this);
   lbl_s5->setAlignment(AlignLeft|AlignVCenter);
   lbl_s5->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_s5->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_s5->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   
   xpos = border;
   ypos += buttonh + spacing;
   cb6 = new QCheckBox(this);
   cb6->setGeometry(xpos+10, ypos+5, 15, 15);
   cb6->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb6, SIGNAL(clicked()), SLOT(select_cb6()));

   xpos+=32;
   lbl_s6 = new QLabel(tr(" Invoiced"),this);
   lbl_s6->setAlignment(AlignLeft|AlignVCenter);
   lbl_s6->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_s6->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_s6->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   
   xpos = border;
   ypos += buttonh + spacing;
   cb7 = new QCheckBox(this);
   cb7->setGeometry(xpos+10, ypos+5, 15, 15);
   cb7->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb7, SIGNAL(clicked()), SLOT(select_cb7()));

   xpos+=32;
   lbl_s7 = new QLabel(tr(" Paid"),this);
   lbl_s7->setAlignment(AlignLeft|AlignVCenter);
   lbl_s7->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_s7->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_s7->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   xpos = border;
   ypos += buttonh + spacing;
      
   pb_update = new QPushButton(tr("Update"), this);
   pb_update->setAutoDefault(false);
   pb_update->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_update->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_update->setGeometry(xpos, ypos, buttonw/2-1, buttonh);
   pb_update->setEnabled(false);
   connect(pb_update, SIGNAL(clicked()), this, SLOT(update_status()));

   xpos += buttonw/2 + spacing;

   pb_cancel = new QPushButton(tr("Cancel"), this);
   pb_cancel->setAutoDefault(false);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_cancel->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_cancel->setGeometry(xpos, ypos, buttonw/2-1, buttonh);
   pb_cancel->setEnabled(true);
   connect(pb_cancel, SIGNAL(clicked()),this, SLOT(quit()));
   xpos = buttonw +2*border;
   ypos += buttonh + spacing;
   setMinimumSize(xpos, ypos);
   
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, xpos, ypos);
   
   if(Status=="designed")
      cb2->setChecked(true);
   else if(Status=="scheduled")
      cb3->setChecked(true);
   else if(Status=="uploaded")
      cb4->setChecked(true);
   else if(Status=="analyzed")
      cb5->setChecked(true);
   else if(Status=="invoiced")
      cb6->setChecked(true);
   else if(Status=="paid")
      cb7->setChecked(true);
   else
      cb1->setChecked(true);





}

US_RequestStatus::~US_RequestStatus()
{
}

void US_RequestStatus::select_cb1()
{
   cb1->setChecked(true);
   Status="submitted";
   cb2->setChecked(false);
   cb3->setChecked(false);
   cb4->setChecked(false);
   cb5->setChecked(false);
   cb6->setChecked(false);
   cb7->setChecked(false);
   pb_update->setEnabled(true);
}

void US_RequestStatus::select_cb2()
{
   cb2->setChecked(true);
   Status="designed";
   cb1->setChecked(false);
   cb3->setChecked(false);
   cb4->setChecked(false);
   cb5->setChecked(false);
   cb6->setChecked(false);
   cb7->setChecked(false);
   pb_update->setEnabled(true);
}
void US_RequestStatus::select_cb3()
{
   cb3->setChecked(true);
   Status="scheduled";
   cb1->setChecked(false);
   cb2->setChecked(false);
   cb4->setChecked(false);
   cb5->setChecked(false);
   cb6->setChecked(false);
   cb7->setChecked(false);
   pb_update->setEnabled(true);
}
void US_RequestStatus::select_cb4()
{
   cb4->setChecked(true);
   Status="uploaded";
   cb1->setChecked(false);
   cb2->setChecked(false);
   cb3->setChecked(false);
   cb5->setChecked(false);
   cb6->setChecked(false);
   cb7->setChecked(false);
   pb_update->setEnabled(true);

}
void US_RequestStatus::select_cb5()
{
   cb5->setChecked(true);
   Status="analyzed";
   cb1->setChecked(false);
   cb2->setChecked(false);
   cb3->setChecked(false);
   cb4->setChecked(false);
   cb6->setChecked(false);
   cb7->setChecked(false);
   pb_update->setEnabled(true);

}
void US_RequestStatus::select_cb6()
{
   cb6->setChecked(true);
   Status="invoiced";
   cb1->setChecked(false);
   cb2->setChecked(false);
   cb3->setChecked(false);
   cb4->setChecked(false);
   cb5->setChecked(false);
   cb7->setChecked(false);
}
void US_RequestStatus::select_cb7()
{
   cb7->setChecked(true);
   Status="paid";
   cb1->setChecked(false);
   cb2->setChecked(false);
   cb3->setChecked(false);
   cb4->setChecked(false);
   cb5->setChecked(false);
   cb6->setChecked(false);
   pb_update->setEnabled(true);

}

void US_RequestStatus::update_status()
{
   
   QString str;
   QSqlQuery query;
   str="UPDATE tblRequest SET Status = '"+Status+"' WHERE RequestID ="+QString::number(RID);
   bool flag = query.exec(str);
   if(!flag)
   {
      QMessageBox::message(tr("Attention:"), 
                           tr("Saving Notes to tblRequest failed.\n"));
      return;
   }
   emit statusChanged(Status);
   quit();
}
void US_RequestStatus::quit()
{
   close();
}

