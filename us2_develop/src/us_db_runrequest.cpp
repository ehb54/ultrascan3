#include "../include/us_db_runrequest.h"

//! Constructor
/*! 
  Constractor a new <var>US_DB_RunRequest</var> interface, 
  with <var>p</var> as a parent and <var>us_runrequest</var> as object name. 
*/ 
US_DB_RunRequest::US_DB_RunRequest(QWidget *p, const char *name) : US_DB( p, name)
{
   int xpos, ypos, buttonw = 360,  buttonh = 26;
   
   sel_data = false;      //use for query data listbox select
   accept_flag = false;
   RunRequestID = 0;
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
         
   xpos = border;
   ypos = border;   
   

   lbl_blank = new QLabel(tr("Select Run Request Data from DB"), this);
   lbl_blank->setAlignment(AlignCenter|AlignVCenter);
   //   lbl_blank->setFrameStyle(QFrame::WinPanel|Raised);
   lbl_blank->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_blank->setGeometry(xpos, ypos, buttonw, buttonh*2);
   lbl_blank->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   xpos = border;
   ypos += buttonh*2 + spacing*2;
   
   pb_check = new QPushButton(tr("Check for Run Request"), this);
   pb_check->setAutoDefault(false);
   pb_check->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_check->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_check->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_check, SIGNAL(clicked()), SLOT(check()));
   
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
   
   lbl_item = new QLabel(" not selected",this);
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

   pb_accept = new QPushButton(tr("Accept"), this);
   pb_accept->setAutoDefault(false);
   pb_accept->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_accept->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_accept->setGeometry(xpos, ypos, buttonw/2-1, buttonh);
   pb_accept->setEnabled(true);
   connect(pb_accept, SIGNAL(clicked()), SLOT(accept()));


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

}

//! Destructor
/*! destroy the US_DB_RunRequest. */
US_DB_RunRequest::~US_DB_RunRequest()
{
   delete item_RRID;
   delete display_Str;
}

void US_DB_RunRequest::check()
{
   QString str, *item_description;
   int maxID = get_newID("tblRunRequest", "RunRequestID");
   display_Str = new QString[maxID];
   item_RRID = new int[maxID];
   item_description = new QString[maxID];
   
   int count = 0;

   str = "SELECT RunRequestID, RunDescription FROM tblRunRequest ORDER BY RunRequestID;";   
   QSqlQuery query(str);
   if(query.isActive())
   {
      while(query.next())
      {
         item_RRID[count] = query.value(0).toInt();
         item_description[count] = query.value(1).toString();
         display_Str[count] = "(" + QString::number( item_RRID[count] ) + "), "+
            item_description[count];
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
      QString str1 = "No Run Request Data in Database";
      lb_data->clear();
      lb_data->insertItem(str1);
   }   
}

/*!
  If you find your expected Run Request in ListBox, doubleclick it.
  This item will be selected for your accepting.
*/
void US_DB_RunRequest::select_data(int item)
{
   QString str;
   if(sel_data)
   {
      RunRequestID = item_RRID[item];
      Display = display_Str[item];      
      str.sprintf("Selected: "+Display);
      lbl_item->setText(str);
      accept_flag = true;         
   }
   else
   {
      QMessageBox::message(tr("Attention:"), 
                           tr("No Run Request data were selected\n"));
      return;
   }
}

void US_DB_RunRequest::accept()
{
   if(accept_flag)
   {
      emit issue_RRID(RunRequestID);
      quit();
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
void US_DB_RunRequest::reset()
{      
   sel_data = false;      
   accept_flag = false;
   RunRequestID = 0;
   lbl_item->setText(tr(" not selected"));
   lb_data->clear();
}

/*! Open a netscape browser to load help page.*/
void US_DB_RunRequest::help()
{
   US_Help *online_help; online_help = new US_Help(this);
   online_help->show_help("manual/run_request.html");
}

/*! Close the interface. */         
void US_DB_RunRequest::quit()
{
   close();
}

