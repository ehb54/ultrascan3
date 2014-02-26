#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_addhybrid.h"
#include <qregexp.h>
//Added by qt3to4:
#include <Q3TextStream>
#include <QCloseEvent>
#include <Q3GridLayout>
#include <Q3Frame>
#include <QLabel>

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

US_AddHybridization::US_AddHybridization(bool *widget_flag, QWidget *p, const char *name) : QWidget( p, name)
{
   this->widget_flag = widget_flag;
   *widget_flag = true;
   USglobal = new US_Config();
   hybrid_filename = USglobal->config_list.system_dir + "/etc/somo.hybrid";
   setPalette( PALET_FRAME );
   setCaption(tr("SoMo: Modify Hybridization Lookup Tables"));
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_AddHybridization::~US_AddHybridization()
{
}

void US_AddHybridization::setupGUI()
{
   int minHeight1 = 30;

   lbl_info = new QLabel(tr("Add/Edit Hybridization Lookup Table:"), this);
   Q_CHECK_PTR(lbl_info);
   lbl_info->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Raised);
   lbl_info->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette( PALET_FRAME );
   AUTFBACK( lbl_info );
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   pb_select_file = new QPushButton(tr("Load Hybridization Definition File"), this);
   Q_CHECK_PTR(pb_select_file);
   pb_select_file->setMinimumHeight(minHeight1);
   pb_select_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_select_file->setPalette( PALET_PUSHB );
   connect(pb_select_file, SIGNAL(clicked()), SLOT(select_file()));

   lbl_table = new QLabel(tr(" not selected"),this);
   lbl_table->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Sunken);
   lbl_table->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_table->setPalette( PALET_EDIT );
   AUTFBACK( lbl_table );
   lbl_table->setMinimumHeight(minHeight1);
   lbl_table->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   cmb_hybrid = new Q3ComboBox(false, this, "Hybridization Listing" );
   cmb_hybrid->setPalette( PALET_NORMAL );
   AUTFBACK( cmb_hybrid );
   cmb_hybrid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cmb_hybrid->setSizeLimit(5);
   cmb_hybrid->setMinimumHeight(minHeight1);
   connect(cmb_hybrid, SIGNAL(activated(int)), this, SLOT(select_hybrid(int)));

   pb_select_saxs_file = new QPushButton(tr("Load SAXS Coefficient File"), this);
   Q_CHECK_PTR(pb_select_saxs_file);
   pb_select_saxs_file->setMinimumHeight(minHeight1);
   pb_select_saxs_file->setEnabled(false);
   pb_select_saxs_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_select_saxs_file->setPalette( PALET_PUSHB );
   connect(pb_select_saxs_file, SIGNAL(clicked()), SLOT(select_saxs_file()));

   lbl_table_saxs = new QLabel(tr(" not selected"),this);
   lbl_table_saxs->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Sunken);
   lbl_table_saxs->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_table_saxs->setPalette( PALET_EDIT );
   AUTFBACK( lbl_table_saxs );
   lbl_table_saxs->setMinimumHeight(minHeight1);
   lbl_table_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   cmb_saxs = new Q3ComboBox(false, this, "Hybridization Listing" );
   cmb_saxs->setPalette( PALET_NORMAL );
   AUTFBACK( cmb_saxs );
   cmb_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cmb_saxs->setSizeLimit(5);
   cmb_saxs->setMinimumHeight(minHeight1);
   connect(cmb_saxs, SIGNAL(activated(int)), this, SLOT(select_saxs(int)));

   lbl_mw = new QLabel(tr(" Molecular Weight:"), this);
   Q_CHECK_PTR(lbl_mw);
   lbl_mw->setMinimumHeight(minHeight1);
   lbl_mw->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_mw->setPalette( PALET_LABEL );
   AUTFBACK( lbl_mw );
   lbl_mw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_mw = new QLineEdit(this, "Molecular Weight Line Edit");
   le_mw->setPalette( PALET_NORMAL );
   AUTFBACK( le_mw );
   le_mw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_mw->setMinimumHeight(minHeight1);
   connect(le_mw, SIGNAL(textChanged(const QString &)), SLOT(update_mw(const QString &)));

   lbl_number_of_hybrids = new QLabel(tr(" Number of Hybridizations in File: 0"), this);
   Q_CHECK_PTR(lbl_number_of_hybrids);
   lbl_number_of_hybrids->setMinimumHeight(minHeight1);
   lbl_number_of_hybrids->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_number_of_hybrids->setPalette( PALET_LABEL );
   AUTFBACK( lbl_number_of_hybrids );
   lbl_number_of_hybrids->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_number_of_saxs = new QLabel(tr(" Number of SAXS Atoms in File: 0"), this);
   Q_CHECK_PTR(lbl_number_of_saxs);
   lbl_number_of_saxs->setMinimumHeight(minHeight1);
   lbl_number_of_saxs->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_number_of_saxs->setPalette( PALET_LABEL );
   AUTFBACK( lbl_number_of_saxs );
   lbl_number_of_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_radius = new QLabel(tr(" Radius (A):"), this);
   Q_CHECK_PTR(lbl_radius);
   lbl_radius->setMinimumHeight(minHeight1);
   lbl_radius->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_radius->setPalette( PALET_LABEL );
   AUTFBACK( lbl_radius );
   lbl_radius->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_radius = new QLineEdit(this, "Radius Line Edit");
   le_radius->setPalette( PALET_NORMAL );
   AUTFBACK( le_radius );
   le_radius->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_radius->setMinimumHeight(minHeight1);
   connect(le_radius, SIGNAL(textChanged(const QString &)), SLOT(update_radius(const QString &)));

   lbl_scat_len = new QLabel(tr(" Neutron scattering length in H2O (*10^-12 cm):"), this);
   Q_CHECK_PTR(lbl_scat_len);
   lbl_scat_len->setMinimumHeight(minHeight1);
   lbl_scat_len->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_scat_len->setPalette( PALET_LABEL );
   AUTFBACK( lbl_scat_len );
   lbl_scat_len->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_scat_len = new QLineEdit(this, "Scat_Len Line Edit");
   le_scat_len->setPalette( PALET_NORMAL );
   AUTFBACK( le_scat_len );
   le_scat_len->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_scat_len->setMinimumHeight(minHeight1);
   connect(le_scat_len, SIGNAL(textChanged(const QString &)), SLOT(update_scat_len(const QString &)));

   lbl_exch_prot = new QLabel(tr(" Number of exchangable protons:"), this);
   Q_CHECK_PTR(lbl_exch_prot);
   lbl_exch_prot->setMinimumHeight(minHeight1);
   lbl_exch_prot->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_exch_prot->setPalette( PALET_LABEL );
   AUTFBACK( lbl_exch_prot );
   lbl_exch_prot->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_exch_prot = new QLineEdit(this, "Exch_Prot Line Edit");
   le_exch_prot->setPalette( PALET_NORMAL );
   AUTFBACK( le_exch_prot );
   le_exch_prot->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_exch_prot->setMinimumHeight(minHeight1);
   connect(le_exch_prot, SIGNAL(textChanged(const QString &)), SLOT(update_exch_prot(const QString &)));

   lbl_num_elect = new QLabel(tr(" Total number of electrons:"), this);
   Q_CHECK_PTR(lbl_num_elect);
   lbl_num_elect->setMinimumHeight(minHeight1);
   lbl_num_elect->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_num_elect->setPalette( PALET_LABEL );
   AUTFBACK( lbl_num_elect );
   lbl_num_elect->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_num_elect = new QLineEdit(this, "Num_Elect Line Edit");
   le_num_elect->setPalette( PALET_NORMAL );
   AUTFBACK( le_num_elect );
   le_num_elect->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_num_elect->setMinimumHeight(minHeight1);
   connect(le_num_elect, SIGNAL(textChanged(const QString &)), SLOT(update_num_elect(const QString &)));

   lbl_name = new QLabel(tr(" Hybridization Name:"), this);
   Q_CHECK_PTR(lbl_name);
   lbl_name->setMinimumHeight(minHeight1);
   lbl_name->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_name->setPalette( PALET_LABEL );
   AUTFBACK( lbl_name );
   lbl_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_name = new QLineEdit(this, "Hybridization name Line Edit");
   le_name->setPalette( PALET_NORMAL );
   AUTFBACK( le_name );
   le_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_name->setMinimumHeight(minHeight1);
   connect(le_name, SIGNAL(textChanged(const QString &)), SLOT(update_name(const QString &)));

   pb_add = new QPushButton(tr("Add Hybridization to File"), this);
   Q_CHECK_PTR(pb_add);
   pb_add->setEnabled(true);
   pb_add->setMinimumHeight(minHeight1);
   pb_add->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_add->setPalette( PALET_PUSHB );
   connect(pb_add, SIGNAL(clicked()), SLOT(add()));

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_close = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_close);
   pb_close->setMinimumHeight(minHeight1);
   pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_close->setPalette( PALET_PUSHB );
   connect(pb_close, SIGNAL(clicked()), SLOT(close()));

   int rows=3, columns = 2, spacing = 2, j=0, margin=4;
   Q3GridLayout *background = new Q3GridLayout(this, rows, columns, margin, spacing);

   background->addMultiCellWidget(lbl_info, j, j, 0, 1);
   j++;
   background->addWidget(pb_select_file, j, 0);
   background->addWidget(lbl_table, j, 1);
   j++;
   background->addWidget(pb_select_saxs_file, j, 0);
   background->addWidget(lbl_table_saxs, j, 1);
   j++;
   background->addWidget(lbl_number_of_hybrids, j, 0);
   background->addWidget(cmb_hybrid, j, 1);
   j++;
   background->addWidget(lbl_number_of_saxs, j, 0);
   background->addWidget(cmb_saxs, j, 1);
   j++;
   background->addWidget(lbl_name, j, 0);
   background->addWidget(le_name, j, 1);
   j++;
   background->addWidget(lbl_mw, j, 0);
   background->addWidget(le_mw, j, 1);
   j++;
   background->addWidget(lbl_radius, j, 0);
   background->addWidget(le_radius, j, 1);
   j++;
   background->addWidget(lbl_scat_len, j, 0);
   background->addWidget(le_scat_len, j, 1);
   j++;
   background->addWidget(lbl_exch_prot, j, 0);
   background->addWidget(le_exch_prot, j, 1);
   j++;
   background->addWidget(lbl_num_elect, j, 0);
   background->addWidget(le_num_elect, j, 1);
   j++;
   background->addMultiCellWidget(pb_add, j, j, 0, 1);
   j++;
   background->addWidget(pb_help, j, 0);
   background->addWidget(pb_close, j, 1);

}

void US_AddHybridization::add()
{
   int item = -1;
   QString str1;
   for (int i=0; i<(int) hybrid_list.size(); i++)
   {
      if (hybrid_list[i].name.upper() == current_hybrid.name.upper())
      {
         item = i;
         hybrid_list[i].saxs_name = current_hybrid.saxs_name;
         hybrid_list[i].mw = current_hybrid.mw;
         hybrid_list[i].radius = current_hybrid.radius;
         hybrid_list[i].scat_len = current_hybrid.scat_len;
         hybrid_list[i].exch_prot = current_hybrid.exch_prot;
         hybrid_list[i].num_elect = current_hybrid.num_elect;
      }
   }
   if (item < 0)
   {
      hybrid_list.push_back(current_hybrid);
   }
   QFile f(hybrid_filename);
   if (f.open(QIODevice::WriteOnly|QIODevice::Text))
   {
      cmb_hybrid->clear();
      str1.sprintf(tr(" Number of Hybridizations in File: %d"), hybrid_list.size());
      Q3TextStream ts(&f);
      for (unsigned int i=0; i<hybrid_list.size(); i++)
      {
         ts << hybrid_list[i].saxs_name 
            << "\t" << hybrid_list[i].name.upper() 
            << "\t" << hybrid_list[i].mw 
            << "\t" << hybrid_list[i].radius 
            << "\t" << hybrid_list[i].scat_len
            << "\t" << hybrid_list[i].exch_prot
            << "\t" << hybrid_list[i].num_elect
            << endl;
         //         cout << "item: " << item << ", " << hybrid_list[i].name.upper() << "\t" << hybrid_list[i].mw << "\t" << hybrid_list[i].radius << ", " <<  hybrid_filename << endl;
         str1.sprintf("%d: ", i+1);
         str1 += hybrid_list[i].name.upper();
         cmb_hybrid->insertItem(str1);
      }
      f.close();
   }
   else
   {
      QMessageBox::message("Attention:", "Could not open the hybridization file:\n\n" + hybrid_filename);
   }
}

void US_AddHybridization::select_file()
{
   QString old_filename = hybrid_filename, str1, str2;
   hybrid_filename = Q3FileDialog::getOpenFileName(USglobal->config_list.system_dir + "/etc", "*.hybrid *.HYBRID", this);
   if (hybrid_filename.isEmpty())
   {
      hybrid_filename = old_filename;
   }
   else
   {
      lbl_table->setText(hybrid_filename);
      QFile f(hybrid_filename);
      hybrid_list.clear();
      cmb_hybrid->clear();
      unsigned int i=1;
      if (f.open(QIODevice::ReadOnly|QIODevice::Text))
      {
         Q3TextStream ts(&f);
         while (!ts.atEnd())
         {
            ts >> current_hybrid.saxs_name;
            ts >> current_hybrid.name;
            ts >> current_hybrid.mw;
            ts >> current_hybrid.radius;
            ts >> current_hybrid.scat_len;
            ts >> current_hybrid.exch_prot;
            ts >> current_hybrid.num_elect;
            str2 = ts.readLine(); // read rest of line
            if (!current_hybrid.name.isEmpty() && current_hybrid.radius > 0.0 && current_hybrid.mw > 0.0)
            {
               hybrid_list.push_back(current_hybrid);
               str1.sprintf("%d: ", i);
               str1 += current_hybrid.name;
               cmb_hybrid->insertItem(str1);
               i++;
            }
         }
         f.close();
         pb_select_saxs_file->setEnabled(true);
      }
   }
   str1.sprintf(tr(" Number of Hybridizations in File: %d"), hybrid_list.size());
   lbl_number_of_hybrids->setText(str1);
   pb_add->setEnabled(true);
   if (!hybrid_filename.isEmpty() && !saxs_filename.isEmpty())
   {
      select_hybrid(0);
   }
}

void US_AddHybridization::select_saxs_file()
{
   QString old_filename = saxs_filename, str1, str2;
   saxs_filename = Q3FileDialog::getOpenFileName(USglobal->config_list.system_dir + "/etc", "*.saxs_atoms *.SAXS_ATOMS", this);
   if (saxs_filename.isEmpty())
   {
      saxs_filename = old_filename;
   }
   else
   {
      lbl_table_saxs->setText(saxs_filename);
      QFile f(saxs_filename);
      saxs_list.clear();
      cmb_saxs->clear();
      unsigned int i=1;
      if (f.open(QIODevice::ReadOnly|QIODevice::Text))
      {
         map < QString, saxs > saxs_map;
         int line = 0;
         Q3TextStream ts( &f );
         while ( !ts.atEnd() )
         {
            QString    qs  = ts.readLine();
            line++;
            if ( qs.contains( QRegExp( "^\\s+#" ) ) )
            {
               continue;
            }
            qs.stripWhiteSpace();
            QStringList qsl = QStringList::split( QRegExp( "\\s+" ), qs );
            int pos = 0;
            if ( qsl.size() == 11 )
            {
               current_saxs.saxs_name = qsl[ pos++ ];
               if ( saxs_map.count( current_saxs.saxs_name ) )
               {
                  current_saxs = saxs_map[ current_saxs.saxs_name ];
               }
               for ( int j = 0; j < 4; j++ )
               {
                  current_saxs.a[ j ] = qsl[ pos++ ].toFloat();
                  current_saxs.b[ j ] = qsl[ pos++ ].toFloat();
               }
               current_saxs.c      = qsl[ pos++ ].toFloat();
               current_saxs.volume = qsl[ pos++ ].toFloat();
               saxs_map[ current_saxs.saxs_name ] = current_saxs;
               continue;
            } 

            if ( qsl.size() == 13 )
            {
               current_saxs.saxs_name = qsl[ pos++ ];
               if ( saxs_map.count( current_saxs.saxs_name ) )
               {
                  current_saxs = saxs_map[ current_saxs.saxs_name ];
               }
               for ( int j = 0; j < 5; j++ )
               {
                  current_saxs.a5[ j ] = qsl[ pos++ ].toFloat();
                  current_saxs.b5[ j ] = qsl[ pos++ ].toFloat();
               }
               current_saxs.c5     = qsl[ pos++ ].toFloat();
               current_saxs.volume = qsl[ pos++ ].toFloat();
               saxs_map[ current_saxs.saxs_name ] = current_saxs;
               continue;
            } 

            cout <<  
               QString(  "Warning: %1 on line %2, invalid number of tokens, ignoring" )
               .arg( saxs_filename )
               .arg( line );
         }

         for ( map < QString, saxs >::iterator it = saxs_map.begin();
               it != saxs_map.end();
               it++ )
         {
            saxs_list.push_back( it->second );
            str1.sprintf("%d: ", i);
            str1 += it->second.saxs_name;
            cmb_saxs->insertItem(str1);
            i++;
         }

         f.close();
      }
   }
   str1.sprintf(tr(" Number of SAXS Entries in File: %d"), saxs_list.size());
   lbl_number_of_saxs->setText(str1);
   if (!hybrid_filename.isEmpty() && !saxs_filename.isEmpty())
   {
      select_hybrid(0);
   }
}

void US_AddHybridization::update_mw(const QString &str)
{
   current_hybrid.mw = str.toFloat();
}

void US_AddHybridization::update_radius(const QString &str)
{
   current_hybrid.radius = str.toFloat();
}

void US_AddHybridization::update_scat_len(const QString &str)
{
   current_hybrid.scat_len = str.toFloat();
}

void US_AddHybridization::update_exch_prot(const QString &str)
{
   current_hybrid.exch_prot = str.toInt();
}

void US_AddHybridization::update_num_elect(const QString &str)
{
   current_hybrid.num_elect = str.toInt();
}

void US_AddHybridization::update_name(const QString &str)
{
   current_hybrid.name = str;
}

void US_AddHybridization::select_hybrid(int val)
{
   QString str;
   str.sprintf("%3.4f", hybrid_list[val].mw);
   le_mw->setText(str);
   str.sprintf("%3.4f", hybrid_list[val].radius);
   le_radius->setText(str);
   str.sprintf("%3.4f", hybrid_list[val].scat_len);
   le_scat_len->setText(str);
   str.sprintf("%d", hybrid_list[val].exch_prot);
   le_exch_prot->setText(str);
   str.sprintf("%d", hybrid_list[val].num_elect);
   le_num_elect->setText(str);
   le_name->setText(hybrid_list[val].name.upper());
   unsigned int i;
   for (i=0; i<saxs_list.size(); i++)
   {
      if (saxs_list[i].saxs_name == hybrid_list[val].saxs_name)
      {
         cmb_saxs->setCurrentItem(i);
         break;
      }
   }
}

void US_AddHybridization::select_saxs(int val)
{
   current_hybrid.saxs_name = saxs_list[val].saxs_name;
   cout << current_hybrid.saxs_name << endl;
}

void US_AddHybridization::closeEvent(QCloseEvent *e)
{
   global_Xpos -= 30;
   global_Ypos -= 30;

   *widget_flag = false;
   e->accept();
}

void US_AddHybridization::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_addhybridization.html");
}

