//
// C++ Implementation: us_enter_dna
//
// Description: 
//
//
// Author: Borries Demeler <demeler@biochem.uthscsa.edu>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
/**************************   Class US_Enter_DNA_DB    *******************************/
#include "../include/us_enter_dna.h"

US_Enter_DNA_DB::US_Enter_DNA_DB(int temp_InvID, QWidget *parent, const char *name) : US_DB(parent, name)
{
   
   InvID = temp_InvID;
   filename = "";
   description = "";
   sequence = "";
   vbar = 0.0;
   e280 = 0.0;
   e260 = 0.0;
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   
   lbl_bar = new QLabel(tr(" Enter Nucleotide Sequence Information:"),this);
   lbl_bar->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_bar->setAlignment(AlignHCenter|AlignVCenter);
   lbl_bar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize , QFont::Bold));

   lbl_description = new QLabel(tr(" Nucleotide Description:"),this);
   lbl_description->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_description->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_description = new QLineEdit("", this);
   le_description->setAlignment(AlignLeft|AlignVCenter);
   le_description->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_description->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
   connect(le_description, SIGNAL(textChanged(const QString &)), SLOT(update_description(const QString &)));

   lbl_vbar = new QLabel(tr(" Nucleotide vbar (optional):"),this);
   lbl_vbar->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_vbar = new QLineEdit("0.0", this);
   le_vbar->setAlignment(AlignLeft|AlignVCenter);
   le_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_vbar->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
   connect(le_vbar, SIGNAL(textChanged(const QString &)), SLOT(update_vbar(const QString &)));

   lbl_e260 = new QLabel(tr(" Nucleotide E260 (optional):"),this);
   lbl_e260->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_e260->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_e260 = new QLineEdit("0.0", this);
   le_e260->setAlignment(AlignLeft|AlignVCenter);
   le_e260->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_e260->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
   connect(le_e260, SIGNAL(textChanged(const QString &)), SLOT(update_e260(const QString &)));

   lbl_e280 = new QLabel(tr(" Nucleotide E280 (optional):"),this);
   lbl_e280->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_e280->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_e280 = new QLineEdit("0.0", this);
   le_e280->setAlignment(AlignLeft|AlignVCenter);
   le_e280->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_e280->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
   connect(le_e280, SIGNAL(textChanged(const QString &)), SLOT(update_e280(const QString &)));

   lbl_sequence = new QLabel(tr(" Nucleotide Sequence: "), this);
   lbl_sequence->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_sequence->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_sequence = new QTextEdit(this, "sequence_editor");
   le_sequence->setAlignment(AlignLeft|AlignVCenter);
   le_sequence->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_sequence->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
   connect(le_sequence, SIGNAL(textChanged()), SLOT(update_sequence()));

   pb_save_HD = new QPushButton(tr("Save to Hard Drive"), this);
   Q_CHECK_PTR(pb_save_HD);
   pb_save_HD->setAutoDefault(false);
   pb_save_HD->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_save_HD->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_save_HD, SIGNAL(clicked()), SLOT(save_HD()));

   pb_save_DB = new QPushButton(tr("Save to Database"), this);
   Q_CHECK_PTR(pb_save_DB);
   pb_save_DB->setAutoDefault(false);
   pb_save_DB->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_save_DB->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_save_DB, SIGNAL(clicked()), SLOT(save_DB()));

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setAutoDefault(false);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_close = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_close);
   pb_close->setAutoDefault(false);
   pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_close, SIGNAL(clicked()), SLOT(quit()));

   int j=0;

   QGridLayout * grid1 = new QGridLayout(this, 7, 2, 2);
   for (int i=1; i<7; i++)
   {
      grid1->setRowSpacing(i, 26);
   }
   grid1->setRowSpacing(0, 30);
   grid1->addMultiCellWidget(lbl_bar, j, j, 0, 1);
   j++;
   grid1->addWidget(lbl_description, j, 0);
   grid1->addWidget(le_description, j, 1);
   j++;
   grid1->addWidget(lbl_vbar, j, 0);
   grid1->addWidget(le_vbar, j, 1);
   j++;
   grid1->addWidget(lbl_e260, j, 0);
   grid1->addWidget(le_e260, j, 1);
   j++;
   grid1->addWidget(lbl_e280, j, 0);
   grid1->addWidget(le_e280, j, 1);
   j++;
   grid1->addWidget(lbl_sequence, j, 0);
   grid1->addWidget(le_sequence, j, 1);
   j++;
   grid1->addWidget(pb_save_HD, j, 0);
   grid1->addWidget(pb_save_DB, j, 1);
   j++;
   grid1->addWidget(pb_help, j, 0);
   grid1->addWidget(pb_close, j, 1);

   qApp->processEvents();

   QRect r = grid1->cellGeometry(0, 0);

   global_Xpos += 30;
   global_Ypos += 30;

   this->setGeometry(global_Xpos, global_Ypos, r.width(), r.height());
}

US_Enter_DNA_DB::~US_Enter_DNA_DB()
{
   global_Xpos -= 30;
   global_Ypos -= 30;
}

void US_Enter_DNA_DB::update_description(const QString &newText)
{
   description = newText;
}

void US_Enter_DNA_DB::update_sequence()
{
   sequence = le_sequence->text();
}

void US_Enter_DNA_DB::update_vbar(const QString & str)
{
   vbar = str.toFloat();
}

void US_Enter_DNA_DB::update_e280(const QString & str)
{
   e280 = str.toFloat();
}

void US_Enter_DNA_DB::update_e260(const QString & str)
{
   e260 = str.toFloat();
}

void US_Enter_DNA_DB::save_HD()
{
   if (description.isEmpty())
   {
      QMessageBox::message(tr("Attention:"), tr("Please enter a description for\n"
                                                "your nucleotide sequence before saving it!"));
      return;
   }
   if (sequence.isEmpty())
   {
      QMessageBox::message(tr("Attention:"), tr("Please enter a sequence for\n"
                                                "your nucleotide sequence  before saving it!"));
      return;
   }

   filename = QFileDialog::getSaveFileName(USglobal->config_list.root_dir, "*.seq", 0);
   if (!filename.isEmpty())
   {
      if (filename.right(4) != ".seq")
      {
         filename.append(".seq");
      }
      switch(QMessageBox::information(this, tr("UltraScan - Nucleotide Sequence:"),
                                      tr("Click 'OK' to save Nucleotide sequence information to :\n"
                                         + filename),
                                      tr("OK"), tr("CANCEL"),   0,1))
      {
      case 0:
         {
            QFile f(filename);
            f.open(IO_WriteOnly | IO_Translate);
            QTextStream ts (&f);
            ts << "DEFINITION" << "\t";
            ts << description << "\n";
            ts << "ORIGIN" << "\n";
            ts << sequence << "\n"<<"//\n";
            ts << vbar << endl;
            ts << e260 << endl;
            ts << e280;
            f.close();
            break;
         }
      case 1:
         {
            break;
         }
      }
   }
}

void US_Enter_DNA_DB::save_DB()
{
   if(InvID <= 0)
   {
      QMessageBox::message(tr("Attention:"), tr("Please 'Select Investigator' int the main interface!"));
      exit(0);
   }
   if (description.isEmpty())
   {
      QMessageBox::message(tr("Attention:"), tr("Please enter a description for\n"
                                                "your nucleotide sequence before saving it!"));
      return;
   }
   if (sequence.isEmpty())
   {
      QMessageBox::message(tr("Attention:"), tr("Please enter a sequence for\n"
                                                "your nucleotide sequence before saving it!"));
      return;
   }
   switch(QMessageBox::information(this, tr("UltraScan - nucleotide sequence Database:"), tr("Store this nucleotide sequence info into the database?"),
                                   tr("OK"), tr("CANCEL"),   0,1))
   {
   case 0:
      {
         QSqlQuery target;
         QString str;
         str.sprintf("INSERT INTO tblDNA(DNAFileName, Description, Sequence, vbar, e280, e260, InvestigatorID) VALUES('"
                     + filename + "', '" + description +"', '"+ sequence + "', %f, %f, %f, %d);", vbar, e280, e260, InvID);
         target.exec(str);
         break;
      }
   case 1:
      {
         break;
      }
   }
}

void US_Enter_DNA_DB::help()
{
   US_Help *online_help; online_help = new US_Help(this);
   online_help->show_help("manual/enter_dna.html");
}

void US_Enter_DNA_DB::quit()
{
   close();
}
