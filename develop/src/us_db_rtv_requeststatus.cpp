#include "../include/us_db_rtv_requeststatus.h"

/*************************************************************************************************/

US_DB_RequestStatus::US_DB_RequestStatus(QWidget *p, const char *name) : US_DB(p, name)
{
   int border=2, spacing=2;
   int xpos = border, ypos = border;
   int buttonw = 120, buttonh = 26;
   s1=s2=s3=s4=s5=s6=s7=false;
   RID=0;
   Item=0;
   
   QString str;
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_title = new QLabel("Project Request Status : ", this);
   lbl_title->setAlignment(AlignCenter|AlignVCenter);
   //   lbl_title->setFrameStyle(QFrame::WinPanel|Raised);
   lbl_title->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_title->setGeometry(xpos, ypos, buttonw*5+spacing*4+34, buttonh*2);
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

   xpos +=buttonw+spacing*2;
   lb_status = new QListBox(this, "Status");
   lb_status->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lb_status->setGeometry(xpos, ypos, buttonw*4, buttonh*7+spacing*6);
   lb_status->setSelected(0, true);
   connect(lb_status, SIGNAL(selected(int)), SLOT(select_status(int)));
   
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
   ypos += buttonh + spacing*2;
      
   pb_search = new QPushButton(tr("Search"), this);
   pb_search->setAutoDefault(false);
   pb_search->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_search->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_search->setGeometry(xpos, ypos, buttonw+34, buttonh);
   connect(pb_search, SIGNAL(clicked()), this, SLOT(show_status()));
   
   
   xpos += buttonw+34 + spacing;
   
   pb_update = new QPushButton(tr("Update"), this);
   pb_update->setAutoDefault(false);
   pb_update->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_update->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_update->setGeometry(xpos, ypos, buttonw, buttonh);
   pb_update->setEnabled(false);
   connect(pb_update, SIGNAL(clicked()), SLOT(update_status()));

   xpos +=buttonw+spacing;
   pb_reset = new QPushButton(tr("Reset"), this);
   pb_reset->setAutoDefault(false);
   pb_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_reset->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_reset->setGeometry(xpos, ypos, buttonw, buttonh);
   pb_reset->setEnabled(true);
   connect(pb_reset, SIGNAL(clicked()),this, SLOT(reset()));

   xpos +=buttonw+spacing;
   
   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setAutoDefault(false);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_help->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));
   
   xpos +=buttonw+spacing;
   pb_cancel = new QPushButton(tr("Close"), this);
   pb_cancel->setAutoDefault(false);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_cancel->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_cancel->setGeometry(xpos, ypos, buttonw, buttonh);
   pb_cancel->setEnabled(true);
   connect(pb_cancel, SIGNAL(clicked()),this, SLOT(quit()));
   xpos = buttonw*5+32+spacing*4+2*border;
   ypos += buttonh + spacing;
   setMinimumSize(xpos, ypos);
   
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, xpos, ypos);
   

}

US_DB_RequestStatus::~US_DB_RequestStatus()
{
}

void US_DB_RequestStatus::select_cb1()
{
   if(s1)
   {
      s1 = false;
      cb1->setChecked(false);
   }
   else
   {
      s1 = true;
      cb1->setChecked(true);
   }
}

void US_DB_RequestStatus::select_cb2()
{
   if(s2)
   {
      s2 = false;
      cb2->setChecked(false);
   }
   else
   {
      s2 = true;
      cb2->setChecked(true);
   }
}
void US_DB_RequestStatus::select_cb3()
{
   if(s3)
   {
      s3 = false;
      cb3->setChecked(false);
   }
   else
   {
      s3 = true;
      cb3->setChecked(true);
   }
}
void US_DB_RequestStatus::select_cb4()
{
   if(s4)
   {
      s4 = false;
      cb4->setChecked(false);
   }
   else
   {
      s4 = true;
      cb4->setChecked(true);
   }
}
void US_DB_RequestStatus::select_cb5()
{
   if(s5)
   {
      s5 = false;
      cb5->setChecked(false);
   }
   else
   {
      s5 = true;
      cb5->setChecked(true);
   }
}
void US_DB_RequestStatus::select_cb6()
{
   if(s6)
   {
      s6 = false;
      cb6->setChecked(false);
   }
   else
   {
      s6 = true;
      cb6->setChecked(true);
   }
}
void US_DB_RequestStatus::select_cb7()
{
   if(s7)
   {
      s7 = false;
      cb7->setChecked(false);
   }
   else
   {
      s7 = true;
      cb7->setChecked(true);
   }
}

void US_DB_RequestStatus::show_status()
{
   QString str, str1, str2,str3,Display, Status, Description, Fname, Lname, wholename;
   int RID, InvID;
   str1="SELECT RequestID, InvestigatorID, ProjectDescription, Status FROM tblRequest ";
   str2="";
   if(s1||s2||s3||s4||s5||s6||s7)
   {
      str2 = "WHERE ";
      if(s1) str2 +="Status = 'submitted' OR ";
      if(s2) str2 +="Status = 'designed' OR ";
      if(s3) str2 +="Status = 'scheduled' OR ";
      if(s4) str2 +="Status = 'uploaded' OR ";
      if(s5) str2 +="Status = 'analyzed' OR ";
      if(s6) str2 +="Status = 'invoiced' OR ";
      if(s7) str2 +="Status = 'paid' OR ";
      str2.truncate(str2.length()-3);   
   }
   str=str1+str2;
   lb_status->clear();
   QSqlQuery query(str);
   if(query.isActive())
   {
      while(query.next())
      {
         RID = query.value(0).toInt();
         InvID=query.value(1).toInt();
         Description=query.value(2).toString();
         Status=query.value(3).toString();
         if(Status.isEmpty())
            Status="submitted";
         str3.sprintf("SELECT FirstName, LastName FROM tblInvestigators WHERE InvID = %d",InvID);
         QSqlQuery q1(str3);
         if(q1.isActive())
         {
            if(q1.next())
            {
               Fname = q1.value(0).toString();
               Lname = q1.value(1).toString();
               wholename = " (" + QString::number( InvID ) + ", "+ Fname + " "+ Lname+")";
            }
         }
         Display=Status+"->"+QString::number(RID)+" : "+Description + wholename;
         lb_status->insertItem(Display);
      }
   }
   else
   {
      QMessageBox::message(tr("Warning:"), 
                           tr("Problem to query tblRequest!\n"));
      return;
   }
}
void US_DB_RequestStatus::select_status(int item)
{
   Item=item;
   Str=lb_status->currentText();
   Str=Str.remove(0, Str.find("->",0)+2);
   RID=Str.left(Str.find(":",0)).toInt();   
   pb_update->setEnabled(true);
}
void US_DB_RequestStatus::update_status()
{
   US_RequestStatus *us_requestStatus;
   us_requestStatus = new US_RequestStatus(RID);
   us_requestStatus->setCaption("Edit Project Status");
   us_requestStatus->show();
   connect(us_requestStatus, SIGNAL(statusChanged(QString)), SLOT(update_display(QString)));
}

void US_DB_RequestStatus::update_display(QString newStatus)
{
   QString newStr;
   newStr=newStatus+"->"+Str;
   lb_status->changeItem(newStr, Item);
   pb_update->setEnabled(false);
}
void US_DB_RequestStatus::reset()
{
   s1=s2=s3=s4=s5=s6=s7=false;
   RID=0;
   Item=0;
   Str="";
   cb1->setChecked(false);
   cb2->setChecked(false);
   cb3->setChecked(false);
   cb4->setChecked(false);
   cb5->setChecked(false);
   cb6->setChecked(false);
   cb7->setChecked(false);
   pb_update->setEnabled(false);
   lb_status->clear();
}
void US_DB_RequestStatus::help()
{

}

void US_DB_RequestStatus::quit()
{
   close();
}
void US_DB_RequestStatus::resizeEvent(QResizeEvent *e)
{
   int border=2, spacing=2;
   int xpos = border, ypos = border;
   int buttonw = 120, buttonh = 26;
   int boxwidth=e->size().width()-buttonw-36-border*2;
   int boxheight=e->size().height() - buttonh*3 - spacing*2 - 2 * border;

   lbl_title->setGeometry(xpos, ypos, e->size().width()-border*2, buttonh*2);
   xpos = border;
   ypos += buttonh*2 + spacing;
   cb1->setGeometry(xpos+10, ypos+5, 15, 15);
   xpos+=32;
   lbl_s1->setGeometry(xpos, ypos, buttonw, buttonh);
   xpos +=buttonw+spacing*2;
   lb_status->setGeometry(xpos, ypos, boxwidth, boxheight);
   xpos = border;
   ypos += buttonh + spacing;
   cb2->setGeometry(xpos+10, ypos+5, 15, 15);
   xpos+=32;
   lbl_s2->setGeometry(xpos, ypos, buttonw, buttonh);
   xpos = border;
   ypos += buttonh + spacing;
   cb3->setGeometry(xpos+10, ypos+5, 15, 15);
   xpos+=32;
   lbl_s3->setGeometry(xpos, ypos, buttonw, buttonh);
   xpos = border;
   ypos += buttonh + spacing;
   cb4->setGeometry(xpos+10, ypos+5, 15, 15);
   xpos+=32;
   lbl_s4->setGeometry(xpos, ypos, buttonw, buttonh);
   xpos = border;
   ypos += buttonh + spacing;
   cb5->setGeometry(xpos+10, ypos+5, 15, 15);
   xpos+=32;
   lbl_s5->setGeometry(xpos, ypos, buttonw, buttonh);
   xpos = border;
   ypos += buttonh + spacing;
   cb6->setGeometry(xpos+10, ypos+5, 15, 15);
   xpos+=32;
   lbl_s6->setGeometry(xpos, ypos, buttonw, buttonh);
   xpos = border;
   ypos += buttonh + spacing;
   cb7->setGeometry(xpos+10, ypos+5, 15, 15);
   xpos+=32;
   lbl_s7->setGeometry(xpos, ypos, buttonw, buttonh);
   xpos = border;
   ypos += buttonh + spacing*2;
   pb_search->setGeometry(xpos, ypos, buttonw+34, buttonh);
   xpos += buttonw+34 + spacing;
   pb_update->setGeometry(xpos, e->size().height()-buttonh-border, buttonw, buttonh);
   xpos +=buttonw+spacing;
   pb_reset->setGeometry(xpos, e->size().height()-buttonh-border,buttonw, buttonh);
   xpos +=buttonw+spacing;
   pb_help->setGeometry(xpos, e->size().height()-buttonh-border,buttonw, buttonh);
   xpos +=buttonw+spacing;
   pb_cancel->setGeometry(xpos, e->size().height()-buttonh-border, buttonw, buttonh);
}
void US_DB_RequestStatus::closeEvent(QCloseEvent *e)
{
   e->accept();
   global_Xpos -= 30;
   global_Ypos -= 30;
}
