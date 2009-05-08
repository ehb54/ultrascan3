#include "../include/us_htmledit.h"

US_htmlEdit::US_htmlEdit(QString temp_fileName, int temp_model, QWidget *p, const char *name) : QDialog(p, name, true)
{
   buttonh   = 26;
   buttonw   = 150;
   border   = 4;
   spacing   = 2;
   int xpos   = border;
   int ypos   = border;
   modified = true;
   QString str;
   fileName = temp_fileName;
   model = temp_model;
   
   USglobal = new US_Config();
   
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   
   lbl_info = new QLabel(this);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(AlignCenter|AlignVCenter);
   lbl_info->setText(tr("Please enter your comments for model:\n") + modelString[model]);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   mle_text = new QTextEdit(this);
   mle_text->setWordWrap(QTextEdit::WidgetWidth);
   mle_text->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   mle_text->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   mle_text->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   mle_text->setReadOnly(false);

   ypos += 2 * buttonh + spacing;
   xpos = border;

   pb_save = new QPushButton(tr("Save"), this);
   Q_CHECK_PTR(pb_save);
   pb_save->setAutoDefault(false);
   pb_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_save->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_save->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_save, SIGNAL(clicked()), SLOT(save()));

   ypos += buttonh + spacing;

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setAutoDefault(false);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_help->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   ypos += buttonh + spacing;

   pb_cancel = new QPushButton(tr("Cancel"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setAutoDefault(false);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_cancel->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   ypos += buttonh + spacing;

   pb_quit = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_quit);
   pb_quit->setAutoDefault(false);
   pb_quit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_quit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_quit->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_quit, SIGNAL(clicked()), SLOT(quit()));

   ypos += buttonh + border;

   global_Xpos += 30;
   global_Ypos += 30;
   
   setMinimumSize(4 * buttonw, ypos);
   setGeometry(global_Xpos, global_Ypos, 3 * buttonw, ypos);
}

US_htmlEdit::~US_htmlEdit()
{
}

void US_htmlEdit::closeEvent(QCloseEvent *e)
{
   e->accept();
   global_Xpos -= 30;
   global_Ypos -= 30;
}

void US_htmlEdit::resizeEvent(QResizeEvent *e)
{
   int panelwidth = 2 * border + buttonw;
   lbl_info->setGeometry(border, border, e->size().width() - 2 * border, 2 * buttonh);
   mle_text->setGeometry(panelwidth, 2 * buttonh + border + spacing,
                         e->size().width() - (panelwidth + border), e->size().height() - 2 * border - spacing - 2 * buttonh);
}

void US_htmlEdit::cancel()
{
   reject();
}

void US_htmlEdit::save()
{
   QString str1, str2;
   QFile f(fileName);
   if (f.exists())
   {
      f.remove();
   }
   if (f.open(IO_WriteOnly | IO_Translate))
   {
      QTextStream ts(&f);
      str1 = tr("Comments for Model ") + modelString[model];
      getHeader(&str2, str1);
      ts << str2;
      for (int i=0; i<mle_text->lines(); i++)
      {
         str2 = mle_text->text(i);
         if (str2.stripWhiteSpace() == "")
         {
            ts << "\n<p>\n";
         }
         else
         {
            ts << str2;
         }
      }
      ts << "\n<p>\n";
      getFooter(&str1);
      ts << str1;
      f.close();
   }
   else
   {
      QMessageBox::message(tr("UltraScan Error:"), tr("Couldn't write the following file to disk:\n\n")
                           + fileName + "\n\n"
                           + tr("Please check to make sure the disk is not full\n"
                                "or you don't have write permission."));
      return;
   }
   mle_text->setModified(false);
}

void US_htmlEdit::quit()
{
   if (mle_text->isModified())
   {
      QMessageBox mb(tr("Attention:"), tr("Do you want to save the file or discard it?"),
                     QMessageBox::Information,
                     QMessageBox::Yes | QMessageBox::Default,
                     QMessageBox::No,
                     QMessageBox::Cancel | QMessageBox::Escape);
      mb.setButtonText(QMessageBox::Yes, tr("Save"));
      mb.setButtonText(QMessageBox::No, tr("Discard"));
      switch(mb.exec())
      {
      case QMessageBox::Yes:
         {
            save();
            accept();
            break;
         }
      case QMessageBox::No:
         {
            accept();
            break;
         }
      case QMessageBox::Cancel:
         {
            return;
            break;
         }
      case QMessageBox::Escape:
         {
            return;
            break;
         }
      }
   }
   else
   {
      accept();
   }
}

void US_htmlEdit::help()
{
   US_Help *online_help; online_help = new US_Help(this);
   online_help->show_help("manual/htmledit.html");
}
