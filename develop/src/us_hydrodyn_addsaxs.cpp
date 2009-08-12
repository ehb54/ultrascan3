#include "../include/us_hydrodyn_addsaxs.h"

US_AddSaxs::US_AddSaxs(bool *widget_flag, QWidget *p, const char *name) : QWidget( p, name)
{
   this->widget_flag = widget_flag;
   *widget_flag = true;
   USglobal = new US_Config();
   saxs_filename = USglobal->config_list.system_dir + "/etc/somo.saxs_atoms";
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("SoMo: Modify Saxs Lookup Tables"));
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_AddSaxs::~US_AddSaxs()
{
}

void US_AddSaxs::setupGUI()
{
   int minHeight1 = 30;

   lbl_info = new QLabel(tr("Add/Edit SAXS Lookup Table:"), this);
   Q_CHECK_PTR(lbl_info);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(AlignCenter|AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   pb_select_file = new QPushButton(tr("Load SAXS Definition File"), this);
   Q_CHECK_PTR(pb_select_file);
   pb_select_file->setMinimumHeight(minHeight1);
   pb_select_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_select_file->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_select_file, SIGNAL(clicked()), SLOT(select_file()));

   lbl_table = new QLabel(tr(" not selected"),this);
   lbl_table->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_table->setAlignment(AlignCenter|AlignVCenter);
   lbl_table->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   lbl_table->setMinimumHeight(minHeight1);
   lbl_table->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   cmb_saxs = new QComboBox(false, this, "SAXS Entry Listing" );
   cmb_saxs->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cmb_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cmb_saxs->setSizeLimit(5);
   cmb_saxs->setMinimumHeight(minHeight1);
   connect(cmb_saxs, SIGNAL(activated(int)), this, SLOT(select_saxs(int)));

   lbl_number_of_saxs = new QLabel(tr(" Number of SAXS Entries in File: 0"), this);
   Q_CHECK_PTR(lbl_number_of_saxs);
   lbl_number_of_saxs->setMinimumHeight(minHeight1);
   lbl_number_of_saxs->setAlignment(AlignLeft|AlignVCenter);
   lbl_number_of_saxs->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_number_of_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_saxs_name = new QLabel(tr(" SAXS Atom Identifier:"), this);
   Q_CHECK_PTR(lbl_saxs_name);
   lbl_saxs_name->setMinimumHeight(minHeight1);
   lbl_saxs_name->setAlignment(AlignLeft|AlignVCenter);
   lbl_saxs_name->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_saxs_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_saxs_name = new QLineEdit(this, "SAXS name Line Edit");
   le_saxs_name->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_saxs_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_saxs_name->setMinimumHeight(minHeight1);
   connect(le_saxs_name, SIGNAL(textChanged(const QString &)), SLOT(update_saxs_name(const QString &)));

   lbl_a1 = new QLabel(tr(" Coefficient a(1):"), this);
   Q_CHECK_PTR(lbl_a1);
   lbl_a1->setMinimumHeight(minHeight1);
   lbl_a1->setAlignment(AlignLeft|AlignVCenter);
   lbl_a1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_a1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_a1 = new QLineEdit(this, "Coefficient a(1) Line Edit");
   le_a1->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_a1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_a1->setMinimumHeight(minHeight1);
   connect(le_a1, SIGNAL(textChanged(const QString &)), SLOT(update_a1(const QString &)));

   lbl_a2 = new QLabel(tr(" Coefficient a(2):"), this);
   Q_CHECK_PTR(lbl_a2);
   lbl_a2->setMinimumHeight(minHeight1);
   lbl_a2->setAlignment(AlignLeft|AlignVCenter);
   lbl_a2->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_a2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_a2 = new QLineEdit(this, "Coefficient a(2) Line Edit");
   le_a2->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_a2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_a2->setMinimumHeight(minHeight1);
   connect(le_a2, SIGNAL(textChanged(const QString &)), SLOT(update_a2(const QString &)));

   lbl_a3 = new QLabel(tr(" Coefficient a(3):"), this);
   Q_CHECK_PTR(lbl_a3);
   lbl_a3->setMinimumHeight(minHeight1);
   lbl_a3->setAlignment(AlignLeft|AlignVCenter);
   lbl_a3->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_a3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_a3 = new QLineEdit(this, "Coefficient a(3) Line Edit");
   le_a3->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_a3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_a3->setMinimumHeight(minHeight1);
   connect(le_a3, SIGNAL(textChanged(const QString &)), SLOT(update_a3(const QString &)));

   lbl_a4 = new QLabel(tr(" Coefficient a(4):"), this);
   Q_CHECK_PTR(lbl_a4);
   lbl_a4->setMinimumHeight(minHeight1);
   lbl_a4->setAlignment(AlignLeft|AlignVCenter);
   lbl_a4->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_a4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_a4 = new QLineEdit(this, "Coefficient a(4) Line Edit");
   le_a4->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_a4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_a4->setMinimumHeight(minHeight1);
   connect(le_a4, SIGNAL(textChanged(const QString &)), SLOT(update_a4(const QString &)));

   lbl_b1 = new QLabel(tr(" Coefficient b(1):"), this);
   Q_CHECK_PTR(lbl_b1);
   lbl_b1->setMinimumHeight(minHeight1);
   lbl_b1->setAlignment(AlignLeft|AlignVCenter);
   lbl_b1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_b1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_b1 = new QLineEdit(this, "Coefficient b(1) Line Edit");
   le_b1->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_b1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_b1->setMinimumHeight(minHeight1);
   connect(le_b1, SIGNAL(textChanged(const QString &)), SLOT(update_b1(const QString &)));

   lbl_b2 = new QLabel(tr(" Coefficient b(2):"), this);
   Q_CHECK_PTR(lbl_b2);
   lbl_b2->setMinimumHeight(minHeight1);
   lbl_b2->setAlignment(AlignLeft|AlignVCenter);
   lbl_b2->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_b2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_b2 = new QLineEdit(this, "Coefficient b(2) Line Edit");
   le_b2->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_b2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_b2->setMinimumHeight(minHeight1);
   connect(le_b2, SIGNAL(textChanged(const QString &)), SLOT(update_b2(const QString &)));

   lbl_b3 = new QLabel(tr(" Coefficient b(3):"), this);
   Q_CHECK_PTR(lbl_b3);
   lbl_b3->setMinimumHeight(minHeight1);
   lbl_b3->setAlignment(AlignLeft|AlignVCenter);
   lbl_b3->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_b3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_b3 = new QLineEdit(this, "Coefficient b(3) Line Edit");
   le_b3->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_b3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_b3->setMinimumHeight(minHeight1);
   connect(le_b3, SIGNAL(textChanged(const QString &)), SLOT(update_b3(const QString &)));

   lbl_b4 = new QLabel(tr(" Coefficient b(4):"), this);
   Q_CHECK_PTR(lbl_b4);
   lbl_b4->setMinimumHeight(minHeight1);
   lbl_b4->setAlignment(AlignLeft|AlignVCenter);
   lbl_b4->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_b4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_b4 = new QLineEdit(this, "Coefficient b(4) Line Edit");
   le_b4->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_b4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_b4->setMinimumHeight(minHeight1);
   connect(le_b4, SIGNAL(textChanged(const QString &)), SLOT(update_b4(const QString &)));

   lbl_c = new QLabel(tr(" Coefficient c:"), this);
   Q_CHECK_PTR(lbl_c);
   lbl_c->setMinimumHeight(minHeight1);
   lbl_c->setAlignment(AlignLeft|AlignVCenter);
   lbl_c->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_c->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_c = new QLineEdit(this, "Coefficient c Line Edit");
   le_c->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_c->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_c->setMinimumHeight(minHeight1);
   connect(le_c, SIGNAL(textChanged(const QString &)), SLOT(update_c(const QString &)));

   lbl_volume = new QLabel(tr(" Atomic Volume (A^3):"), this);
   Q_CHECK_PTR(lbl_volume);
   lbl_volume->setMinimumHeight(minHeight1);
   lbl_volume->setAlignment(AlignLeft|AlignVCenter);
   lbl_volume->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_volume->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_volume = new QLineEdit(this, "Atomic Volume Line Edit");
   le_volume->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_volume->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_volume->setMinimumHeight(minHeight1);
   connect(le_volume, SIGNAL(textChanged(const QString &)), SLOT(update_volume(const QString &)));

   pb_add = new QPushButton(tr("Add SAXS Atom to File"), this);
   Q_CHECK_PTR(pb_add);
   pb_add->setEnabled(true);
   pb_add->setMinimumHeight(minHeight1);
   pb_add->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_add->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_add, SIGNAL(clicked()), SLOT(add()));

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_close = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_close);
   pb_close->setMinimumHeight(minHeight1);
   pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_close, SIGNAL(clicked()), SLOT(close()));

   int rows=3, columns = 2, spacing = 2, j=0, margin=4;
   QGridLayout *background = new QGridLayout(this, rows, columns, margin, spacing);

   background->addMultiCellWidget(lbl_info, j, j, 0, 1);
   j++;
   background->addWidget(pb_select_file, j, 0);
   background->addWidget(lbl_table, j, 1);
   j++;
   background->addWidget(lbl_number_of_saxs, j, 0);
   background->addWidget(cmb_saxs, j, 1);
   j++;
   background->addWidget(lbl_saxs_name, j, 0);
   background->addWidget(le_saxs_name, j, 1);
   j++;
   background->addWidget(lbl_a1, j, 0);
   background->addWidget(le_a1, j, 1);
   j++;
   background->addWidget(lbl_b1, j, 0);
   background->addWidget(le_b1, j, 1);
   j++;
   background->addWidget(lbl_a2, j, 0);
   background->addWidget(le_a2, j, 1);
   j++;
   background->addWidget(lbl_b2, j, 0);
   background->addWidget(le_b2, j, 1);
   j++;
   background->addWidget(lbl_a3, j, 0);
   background->addWidget(le_a3, j, 1);
   j++;
   background->addWidget(lbl_b3, j, 0);
   background->addWidget(le_b3, j, 1);
   j++;
   background->addWidget(lbl_a4, j, 0);
   background->addWidget(le_a4, j, 1);
   j++;
   background->addWidget(lbl_b4, j, 0);
   background->addWidget(le_b4, j, 1);
   j++;
   background->addWidget(lbl_c, j, 0);
   background->addWidget(le_c, j, 1);
   j++;
   background->addWidget(lbl_volume, j, 0);
   background->addWidget(le_volume, j, 1);
   j++;
   background->addMultiCellWidget(pb_add, j, j, 0, 1);
   j++;
   background->addWidget(pb_help, j, 0);
   background->addWidget(pb_close, j, 1);

}

void US_AddSaxs::add()
{
   int item = -1;
   QString str1;
   for (int i=0; i<(int) saxs_list.size(); i++)
   {
      if (saxs_list[i].saxs_name.upper() == current_saxs.saxs_name.upper())
      {
         item = i;
         saxs_list[i].a[0] = current_saxs.a[0];
         saxs_list[i].b[0] = current_saxs.b[0];
         saxs_list[i].a[1] = current_saxs.a[1];
         saxs_list[i].b[1] = current_saxs.b[1];
         saxs_list[i].a[2] = current_saxs.a[2];
         saxs_list[i].b[2] = current_saxs.b[2];
         saxs_list[i].a[3] = current_saxs.a[3];
         saxs_list[i].b[3] = current_saxs.b[3];
         saxs_list[i].c = current_saxs.c;
         saxs_list[i].volume = current_saxs.volume;
      }
   }
   if (item < 0)
   {
      saxs_list.push_back(current_saxs);
   }
   QFile f(saxs_filename);
   if (f.open(IO_WriteOnly|IO_Translate))
   {
      cmb_saxs->clear();
      str1.sprintf(tr(" Number of SAXS Entries in File: %d"), saxs_list.size());
      QTextStream ts(&f);
      for (unsigned int i=0; i<saxs_list.size(); i++)
      {
         ts << saxs_list[i].saxs_name.upper() << "\t"
            << saxs_list[i].a[0] << "\t"
            << saxs_list[i].b[0] << "\t"
            << saxs_list[i].a[1] << "\t"
            << saxs_list[i].b[1] << "\t"
            << saxs_list[i].a[2] << "\t"
            << saxs_list[i].b[2] << "\t"
            << saxs_list[i].a[3] << "\t"
            << saxs_list[i].b[3] << "\t"
            << saxs_list[i].c << "\t"
            << saxs_list[i].volume << endl;
         str1.sprintf("%d: ", i+1);
         str1 += saxs_list[i].saxs_name.upper();
         cmb_saxs->insertItem(str1);
      }
      f.close();
   }
   else
   {
      QMessageBox::message("Attention:", "Could not open the SAXS Coefficient file:\n\n" + saxs_filename);
   }
}

void US_AddSaxs::select_file()
{
   QString old_filename = saxs_filename, str1, str2;
   saxs_filename = QFileDialog::getOpenFileName(USglobal->config_list.system_dir + "/etc", "*.saxs_atoms *.SAXS_ATOMS", this);
   if (saxs_filename.isEmpty())
   {
      saxs_filename = old_filename;
   }
   else
   {
      lbl_table->setText(saxs_filename);
      QFile f(saxs_filename);
      saxs_list.clear();
      cmb_saxs->clear();
      unsigned int i=1;
      if (f.open(IO_ReadOnly|IO_Translate))
      {
         QTextStream ts(&f);
         while (!ts.atEnd())
         {
            ts >> current_saxs.saxs_name;
            ts >> current_saxs.a[0];
            ts >> current_saxs.b[0];
            ts >> current_saxs.a[1];
            ts >> current_saxs.b[1];
            ts >> current_saxs.a[2];
            ts >> current_saxs.b[2];
            ts >> current_saxs.a[3];
            ts >> current_saxs.b[3];
            ts >> current_saxs.c;
            ts >> current_saxs.volume;
            str2 = ts.readLine(); // read rest of line
            if (!current_saxs.saxs_name.isEmpty())
            {
               saxs_list.push_back(current_saxs);
               str1.sprintf("%d: ", i);
               str1 += current_saxs.saxs_name;
               cmb_saxs->insertItem(str1);
               i++;
            }
         }
         f.close();
      }
   }
   str1.sprintf(tr(" Number of SAXS Entries in File: %d"), saxs_list.size());
   lbl_number_of_saxs->setText(str1);
   pb_add->setEnabled(true);
}

void US_AddSaxs::update_saxs_name(const QString &str)
{
   current_saxs.saxs_name = str;
}

void US_AddSaxs::update_a1(const QString &str)
{
   current_saxs.a[0] = str.toFloat();
}

void US_AddSaxs::update_a2(const QString &str)
{
   current_saxs.a[1] = str.toFloat();
}

void US_AddSaxs::update_a3(const QString &str)
{
   current_saxs.a[2] = str.toFloat();
}

void US_AddSaxs::update_a4(const QString &str)
{
   current_saxs.a[3] = str.toFloat();
}

void US_AddSaxs::update_b1(const QString &str)
{
   current_saxs.b[0] = str.toFloat();
}

void US_AddSaxs::update_b2(const QString &str)
{
   current_saxs.b[1] = str.toFloat();
}

void US_AddSaxs::update_b3(const QString &str)
{
   current_saxs.b[2] = str.toFloat();
}

void US_AddSaxs::update_b4(const QString &str)
{
   current_saxs.b[3] = str.toFloat();
}

void US_AddSaxs::update_c(const QString &str)
{
   current_saxs.c = str.toFloat();
}

void US_AddSaxs::update_volume(const QString &str)
{
   current_saxs.volume = str.toFloat();
}

void US_AddSaxs::select_saxs(int val)
{
   QString str;
   le_saxs_name->setText(saxs_list[val].saxs_name.upper());
   str.sprintf("%3.4f", saxs_list[val].a[0]);
   le_a1->setText(str);
   str.sprintf("%3.4f", saxs_list[val].b[0]);
   le_b1->setText(str);
   str.sprintf("%3.4f", saxs_list[val].a[1]);
   le_a2->setText(str);
   str.sprintf("%3.4f", saxs_list[val].b[1]);
   le_b2->setText(str);
   str.sprintf("%3.4f", saxs_list[val].a[2]);
   le_a3->setText(str);
   str.sprintf("%3.4f", saxs_list[val].b[2]);
   le_b3->setText(str);
   str.sprintf("%3.4f", saxs_list[val].a[3]);
   le_a4->setText(str);
   str.sprintf("%3.4f", saxs_list[val].b[3]);
   le_b4->setText(str);
   str.sprintf("%3.4f", saxs_list[val].c);
   le_c->setText(str);
   str.sprintf("%3.4f", saxs_list[val].volume);
   le_volume->setText(str);
}

void US_AddSaxs::closeEvent(QCloseEvent *e)
{
   global_Xpos -= 30;
   global_Ypos -= 30;

   *widget_flag = false;
   e->accept();
}

void US_AddSaxs::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_add_saxs.html");
}

