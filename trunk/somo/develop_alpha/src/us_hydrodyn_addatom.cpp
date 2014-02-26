#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_addatom.h"
#include <list>
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

US_AddAtom::US_AddAtom(bool *widget_flag, QWidget *p, const char *name) : QWidget( p, name)
{
   this->widget_flag = widget_flag;
   *widget_flag = true;
   USglobal = new US_Config();
   atom_filename = USglobal->config_list.system_dir + "/etc/somo.atom";
   setPalette( PALET_FRAME );
   setCaption(tr("SoMo: Modify Atom Lookup Tables"));
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_AddAtom::~US_AddAtom()
{
}

void US_AddAtom::setupGUI()
{
   int minHeight1 = 30;

   lbl_info = new QLabel(tr("Add/Edit Atom Lookup Table:"), this);
   Q_CHECK_PTR(lbl_info);
   lbl_info->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Raised);
   lbl_info->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette( PALET_FRAME );
   AUTFBACK( lbl_info );
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   pb_select_atom_file = new QPushButton(tr("Load Atom Definition File"), this);
   Q_CHECK_PTR(pb_select_atom_file);
   pb_select_atom_file->setMinimumHeight(minHeight1);
   pb_select_atom_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_select_atom_file->setPalette( PALET_PUSHB );
   connect(pb_select_atom_file, SIGNAL(clicked()), SLOT(select_atom_file()));

   pb_select_hybrid_file = new QPushButton(tr("Load Hybridization File"), this);
   Q_CHECK_PTR(pb_select_hybrid_file);
   pb_select_hybrid_file->setMinimumHeight(minHeight1);
   pb_select_hybrid_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_select_hybrid_file->setPalette( PALET_PUSHB );
   connect(pb_select_hybrid_file, SIGNAL(clicked()), SLOT(select_hybrid_file()));

   pb_select_saxs_file = new QPushButton(tr("Load SAXS Coefficient File"), this);
   Q_CHECK_PTR(pb_select_saxs_file);
   pb_select_saxs_file->setMinimumHeight(minHeight1);
   pb_select_saxs_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_select_saxs_file->setPalette( PALET_PUSHB );
   connect(pb_select_saxs_file, SIGNAL(clicked()), SLOT(select_saxs_file()));

   lbl_atom_table = new QLabel(tr(" not selected"),this);
   lbl_atom_table->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Sunken);
   lbl_atom_table->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_atom_table->setPalette( PALET_EDIT );
   AUTFBACK( lbl_atom_table );
   lbl_atom_table->setMinimumHeight(minHeight1);
   lbl_atom_table->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_hybrid_table = new QLabel(tr(" not selected"),this);
   lbl_hybrid_table->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Sunken);
   lbl_hybrid_table->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_hybrid_table->setPalette( PALET_EDIT );
   AUTFBACK( lbl_hybrid_table );
   lbl_hybrid_table->setMinimumHeight(minHeight1);
   lbl_hybrid_table->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_saxs_table = new QLabel(tr(" not selected"),this);
   lbl_saxs_table->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Sunken);
   lbl_saxs_table->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_saxs_table->setPalette( PALET_EDIT );
   AUTFBACK( lbl_saxs_table );
   lbl_saxs_table->setMinimumHeight(minHeight1);
   lbl_saxs_table->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   cmb_atom = new Q3ComboBox(false, this, "Atom Listing" );
   cmb_atom->setPalette( PALET_NORMAL );
   AUTFBACK( cmb_atom );
   cmb_atom->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cmb_atom->setSizeLimit(5);
   cmb_atom->setMinimumHeight(minHeight1);
   connect(cmb_atom, SIGNAL(activated(int)), this, SLOT(select_atom(int)));

   cmb_hybrid = new Q3ComboBox(false, this, "Hybrid Listing" );
   cmb_hybrid->setPalette( PALET_NORMAL );
   AUTFBACK( cmb_hybrid );
   cmb_hybrid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cmb_hybrid->setSizeLimit(5);
   cmb_hybrid->setMinimumHeight(minHeight1);
   connect(cmb_hybrid, SIGNAL(activated(int)), this, SLOT(select_hybrid(int)));

   lbl_mw1 = new QLabel(tr(" Molecular Weight:"), this);
   Q_CHECK_PTR(lbl_mw1);
   lbl_mw1->setMinimumHeight(minHeight1);
   lbl_mw1->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_mw1->setPalette( PALET_LABEL );
   AUTFBACK( lbl_mw1 );
   lbl_mw1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_mw2 = new QLabel("", this);
   Q_CHECK_PTR(lbl_mw2);
   lbl_mw2->setMinimumHeight(minHeight1);
   lbl_mw2->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_mw2->setPalette( PALET_EDIT );
   AUTFBACK( lbl_mw2 );
   lbl_mw2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_saxs_name1 = new QLabel(tr(" SAXS Atom Identifier:"), this);
   Q_CHECK_PTR(lbl_saxs_name1);
   lbl_saxs_name1->setMinimumHeight(minHeight1);
   lbl_saxs_name1->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_saxs_name1->setPalette( PALET_LABEL );
   AUTFBACK( lbl_saxs_name1 );
   lbl_saxs_name1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_saxs_name2 = new QLabel("", this);
   Q_CHECK_PTR(lbl_saxs_name2);
   lbl_saxs_name2->setMinimumHeight(minHeight1);
   lbl_saxs_name2->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_saxs_name2->setPalette( PALET_EDIT );
   AUTFBACK( lbl_saxs_name2 );
   lbl_saxs_name2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_radius1 = new QLabel(tr(" Radius (A):"), this);
   Q_CHECK_PTR(lbl_radius1);
   lbl_radius1->setMinimumHeight(minHeight1);
   lbl_radius1->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_radius1->setPalette( PALET_LABEL );
   AUTFBACK( lbl_radius1 );
   lbl_radius1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_radius2 = new QLabel("", this);
   Q_CHECK_PTR(lbl_radius2);
   lbl_radius2->setMinimumHeight(minHeight1);
   lbl_radius2->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_radius2->setPalette( PALET_EDIT );
   AUTFBACK( lbl_radius2 );
   lbl_radius2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_hybrid1 = new QLabel(tr(" Hybridization:"), this);
   Q_CHECK_PTR(lbl_hybrid1);
   lbl_hybrid1->setMinimumHeight(minHeight1);
   lbl_hybrid1->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_hybrid1->setPalette( PALET_LABEL );
   AUTFBACK( lbl_hybrid1 );
   lbl_hybrid1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_hybrid2 = new QLabel("", this);
   Q_CHECK_PTR(lbl_hybrid2);
   lbl_hybrid2->setMinimumHeight(minHeight1);
   lbl_hybrid2->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_hybrid2->setPalette( PALET_EDIT );
   AUTFBACK( lbl_hybrid2 );
   lbl_hybrid2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_saxs_excl_vol1 = new QLabel(tr(" Excluded Vol. for SAXS Atom:"), this);
   Q_CHECK_PTR(lbl_saxs_excl_vol1);
   lbl_saxs_excl_vol1->setMinimumHeight(minHeight1);
   lbl_saxs_excl_vol1->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_saxs_excl_vol1->setPalette( PALET_LABEL );
   AUTFBACK( lbl_saxs_excl_vol1 );
   lbl_saxs_excl_vol1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_saxs_excl_vol2 = new QLabel(tr(" Enter Excluded Volume (A^3):"), this);
   Q_CHECK_PTR(lbl_saxs_excl_vol2);
   lbl_saxs_excl_vol2->setMinimumHeight(minHeight1);
   lbl_saxs_excl_vol2->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_saxs_excl_vol2->setPalette( PALET_LABEL );
   AUTFBACK( lbl_saxs_excl_vol2 );
   lbl_saxs_excl_vol2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_number_of_atoms = new QLabel(tr(" Number of Atoms in File: 0"), this);
   Q_CHECK_PTR(lbl_number_of_atoms);
   lbl_number_of_atoms->setMinimumHeight(minHeight1);
   lbl_number_of_atoms->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_number_of_atoms->setPalette( PALET_LABEL );
   AUTFBACK( lbl_number_of_atoms );
   lbl_number_of_atoms->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_number_of_hybrids = new QLabel(tr(" Number of Hybridizations in File: 0"), this);
   Q_CHECK_PTR(lbl_number_of_hybrids);
   lbl_number_of_hybrids->setMinimumHeight(minHeight1);
   lbl_number_of_hybrids->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_number_of_hybrids->setPalette( PALET_LABEL );
   AUTFBACK( lbl_number_of_hybrids );
   lbl_number_of_hybrids->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_name = new QLabel(tr(" Atom Name:"), this);
   Q_CHECK_PTR(lbl_name);
   lbl_name->setMinimumHeight(minHeight1);
   lbl_name->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_name->setPalette( PALET_LABEL );
   AUTFBACK( lbl_name );
   lbl_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_name = new QLineEdit(this, "Atom name Line Edit");
   le_name->setPalette( PALET_NORMAL );
   AUTFBACK( le_name );
   le_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_name->setMinimumHeight(minHeight1);
   connect(le_name, SIGNAL(textChanged(const QString &)), SLOT(update_name(const QString &)));

   le_excl_vol = new QLineEdit(this, "SAXS excluded volume Line Edit");
   le_excl_vol->setPalette( PALET_NORMAL );
   AUTFBACK( le_excl_vol );
   le_excl_vol->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_excl_vol->setMinimumHeight(minHeight1);
   le_excl_vol->setEnabled(false);
   connect(le_excl_vol, SIGNAL(textChanged(const QString &)), SLOT(update_excl_vol(const QString &)));

   pb_add = new QPushButton(tr("Add Atom to File"), this);
   Q_CHECK_PTR(pb_add);
   pb_add->setEnabled(false);
   pb_add->setMinimumHeight(minHeight1);
   pb_add->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_add->setPalette( PALET_PUSHB );
   connect(pb_add, SIGNAL(clicked()), SLOT(add()));

   pb_delete = new QPushButton(tr("Delete Atom"), this);
   Q_CHECK_PTR(pb_delete);
   pb_delete->setEnabled(false);
   pb_delete->setMinimumHeight(minHeight1);
   pb_delete->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_delete->setPalette( PALET_PUSHB );
   connect(pb_delete, SIGNAL(clicked()), SLOT(delete_atom()));

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

   cb_excl_vol = new QCheckBox(this, "excluded volume checkbox");
   cb_excl_vol->setText(tr(" Use average value (from SAXS table) "));
   cb_excl_vol->setChecked(true);
   cb_excl_vol->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_excl_vol->setPalette( PALET_NORMAL );
   AUTFBACK( cb_excl_vol );
   connect(cb_excl_vol, SIGNAL(clicked()), this, SLOT(set_excl_vol()));

   int rows=3, columns = 2, spacing = 2, j=0, margin=4;
   Q3GridLayout *background = new Q3GridLayout(this, rows, columns, margin, spacing);

   background->addMultiCellWidget(lbl_info, j, j, 0, 1);
   j++;
   background->addWidget(pb_select_atom_file, j, 0);
   background->addWidget(lbl_atom_table, j, 1);
   j++;
   background->addWidget(pb_select_hybrid_file, j, 0);
   background->addWidget(lbl_hybrid_table, j, 1);
   j++;
   background->addWidget(pb_select_saxs_file, j, 0);
   background->addWidget(lbl_saxs_table, j, 1);
   j++;
   background->addWidget(lbl_number_of_atoms, j, 0);
   background->addWidget(cmb_atom, j, 1);
   j++;
   background->addWidget(lbl_number_of_hybrids, j, 0);
   background->addWidget(cmb_hybrid, j, 1);
   j++;
   background->addWidget(lbl_saxs_name1, j, 0);
   background->addWidget(lbl_saxs_name2, j, 1);
   j++;
   background->addWidget(lbl_name, j, 0);
   background->addWidget(le_name, j, 1);
   j++;
   background->addWidget(lbl_hybrid1, j, 0);
   background->addWidget(lbl_hybrid2, j, 1);
   j++;
   background->addWidget(lbl_mw1, j, 0);
   background->addWidget(lbl_mw2, j, 1);
   j++;
   background->addWidget(lbl_radius1, j, 0);
   background->addWidget(lbl_radius2, j, 1);
   j++;
   background->addWidget(lbl_saxs_excl_vol1, j, 0);
   background->addWidget(cb_excl_vol, j, 1);
   j++;
   background->addWidget(lbl_saxs_excl_vol2, j, 0);
   background->addWidget(le_excl_vol, j, 1);
   j++;
   background->addWidget(pb_add, j, 0);
   background->addWidget(pb_delete, j, 1);
   j++;
   background->addWidget(pb_help, j, 0);
   background->addWidget(pb_close, j, 1);
}

void US_AddAtom::add()
{
   int item = -1;
   for (int i=0; i<(int) atom_list.size(); i++) //check if the same atom was defined previously
   {
      if (atom_list[i].name.upper() == current_atom.name.upper()
          &&  atom_list[i].hybrid.name.upper() == current_atom.hybrid.name.upper())
      {
         item = i;
         atom_list[i].hybrid.mw = current_atom.hybrid.mw;
         atom_list[i].hybrid.radius = current_atom.hybrid.radius;
         atom_list[i].hybrid.scat_len = current_atom.hybrid.scat_len;
         atom_list[i].hybrid.exch_prot = current_atom.hybrid.exch_prot;
         atom_list[i].hybrid.num_elect = current_atom.hybrid.num_elect;
         atom_list[i].hybrid.saxs_name = current_atom.hybrid.saxs_name;
         atom_list[i].saxs_excl_vol = current_atom.saxs_excl_vol;
      }
   }
   if (item < 0)
   {
      atom_list.push_back(current_atom);
   }
   write_atom_file();
   pb_delete->setEnabled(true);
}

class sortable_atom {
public:
   atom our_atom;
   bool operator < (const sortable_atom& objIn) const
   {
      QString atom_cmp = our_atom.name.upper() + "\t" + our_atom.hybrid.name.upper();
      QString obj_cmp = objIn.our_atom.name.upper() + "\t" + objIn.our_atom.hybrid.name.upper();
      if (
          atom_cmp
          <
          obj_cmp
          )
      {
         return (true);
      }
      else
      {
         return (false);
      }
   }
};

void US_AddAtom::sort_atoms()
{
   list < sortable_atom > atoms_to_sort;
   sortable_atom tmp_atom;
   for (unsigned int i=0; i<atom_list.size(); i++)
   {
      tmp_atom.our_atom = atom_list[i];
      atoms_to_sort.push_back(tmp_atom);
   }
   atoms_to_sort.sort();
   atom_list.clear();
   while(atoms_to_sort.size())
   {
      atom_list.push_back((atoms_to_sort.front()).our_atom);
      atoms_to_sort.pop_front();
   }
}

void US_AddAtom::write_atom_file()
{
   QString str1;
   sort_atoms();
   QFile f(atom_filename);
   if (f.open(QIODevice::WriteOnly|QIODevice::Text))
   {
      cmb_atom->clear();
      str1.sprintf(tr(" Number of Atoms in File: %d"), atom_list.size());
      Q3TextStream ts(&f);
      for (unsigned int i=0; i<atom_list.size(); i++)
      {
         ts << atom_list[i].name.upper() << "\t" << atom_list[i].hybrid.name.upper() << "\t" <<
               atom_list[i].hybrid.mw << "\t" << atom_list[i].hybrid.radius <<  "\t" <<
               atom_list[i].saxs_excl_vol << endl;
         str1.sprintf("%d: ", i+1);
         str1 += atom_list[i].name.upper();
         str1 += " (";
         str1 += atom_list[i].hybrid.name.upper();
         str1 += ")";
         cmb_atom->insertItem(str1);
      }
      f.close();
   }
   else
   {
      QMessageBox::message("Attention:", "Could not open the atom file:\n\n" + atom_filename);
   }
}

void US_AddAtom::select_atom_file()
{
   QString old_filename = atom_filename, str1, str2;
   atom_filename = Q3FileDialog::getOpenFileName(USglobal->config_list.system_dir + "/etc", "*.atom *.ATOM", this);
   if (atom_filename.isEmpty())
   {
      atom_filename = old_filename;
   }
   else
   {
      lbl_atom_table->setText(atom_filename);
      QFile f(atom_filename);
      atom_list.clear();
      cmb_atom->clear();
      unsigned int i=1;
      if (f.open(QIODevice::ReadOnly|QIODevice::Text))
      {
         Q3TextStream ts(&f);
         while (!ts.atEnd())
         {
            ts >> current_atom.name;
            ts >> current_atom.hybrid.name;
            ts >> current_atom.hybrid.mw;
            ts >> current_atom.hybrid.radius;
            ts >> current_atom.saxs_excl_vol;
            str2 = ts.readLine(); // read rest of line
            if (!current_atom.name.isEmpty() && current_atom.hybrid.radius > 0.0 && current_atom.hybrid.mw > 0.0)
            {
               atom_list.push_back(current_atom);
               str1.sprintf("%d: ", i);
               str1 += current_atom.name;
               str1 += " (";
               str1 += current_atom.hybrid.name.upper();
               str1 += ")";
               cmb_atom->insertItem(str1);
               i++;
            }
         }
         f.close();
      }
   }
   cb_excl_vol->setChecked(false);
   le_excl_vol->setEnabled(true);
   str1.sprintf(tr(" Number of Atoms in File: %d"), atom_list.size());
   lbl_number_of_atoms->setText(str1);
   pb_add->setEnabled(true);
   if (!atom_filename.isEmpty() && !hybrid_filename.isEmpty() && !saxs_filename.isEmpty())
   {
      select_atom(0);
   }
}

void US_AddAtom::select_hybrid_file()
{
   QString old_filename = hybrid_filename, str1, str2;
   hybrid_filename = Q3FileDialog::getOpenFileName(USglobal->config_list.system_dir + "/etc", "*.hybrid *.HYBRID", this);
   if (hybrid_filename.isEmpty())
   {
      hybrid_filename = old_filename;
   }
   else
   {
      lbl_hybrid_table->setText(hybrid_filename);
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
      }
   }
   str1.sprintf(tr(" Number of Hybridizations in File: %d"), hybrid_list.size());
   lbl_number_of_hybrids->setText(str1);
   pb_add->setEnabled(true);
   if (!atom_filename.isEmpty() && !hybrid_filename.isEmpty() && !saxs_filename.isEmpty())
   {
      select_atom(0);
   }
}

void US_AddAtom::select_saxs_file()
{
   QString old_filename = saxs_filename, str1, str2;
   saxs_filename = Q3FileDialog::getOpenFileName(USglobal->config_list.system_dir + "/etc", "*.saxs_atoms *.SAXS_ATOMS", this);
   if (saxs_filename.isEmpty())
   {
      saxs_filename = old_filename;
   }
   else
   {
      map < QString, saxs > saxs_map;
      lbl_saxs_table->setText(saxs_filename);
      QFile f(saxs_filename);
      saxs_list.clear();
      if (f.open(QIODevice::ReadOnly|QIODevice::Text))
      {
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
         }

         f.close();
      }
   }
   if (!atom_filename.isEmpty() && !hybrid_filename.isEmpty() && !saxs_filename.isEmpty())
   {
      select_atom(0);
   }
}

void US_AddAtom::select_hybrid(int i)
{
   if (saxs_list.size() == 0) return;
   QString str;
   float val=0.0;
   current_atom.hybrid.saxs_name = hybrid_list[i].saxs_name;
   current_atom.hybrid.mw = hybrid_list[i].mw;
   current_atom.hybrid.name = hybrid_list[i].name;
   current_atom.hybrid.radius = hybrid_list[i].radius;
   current_atom.hybrid.scat_len = hybrid_list[i].scat_len;
   current_atom.hybrid.exch_prot = hybrid_list[i].exch_prot;
   current_atom.hybrid.num_elect = hybrid_list[i].num_elect;
   str.sprintf("%3.2f: ", current_atom.hybrid.mw);
   lbl_mw2->setText(str);
   str.sprintf("%3.2f: ", current_atom.hybrid.radius);
   lbl_radius2->setText(str);
   lbl_hybrid2->setText(current_atom.hybrid.name);
   for (unsigned int j=0; j<saxs_list.size(); j++)
   {
      if (saxs_list[j].saxs_name == current_atom.hybrid.saxs_name)
      {
         lbl_saxs_name2->setText(current_atom.hybrid.saxs_name);
         val = saxs_list[j].volume;
         break;
      }
   }
   if (cb_excl_vol->isChecked())
   {
      str.sprintf("%3.2f", val);
      le_excl_vol->setText(str);
   }
   else
   {
      str.sprintf("%3.2f", current_atom.saxs_excl_vol);
      le_excl_vol->setText(str);
   }
}

void US_AddAtom::update_name(const QString &str)
{
   current_atom.name = str;
}

void US_AddAtom::update_excl_vol(const QString &str)
{
   current_atom.saxs_excl_vol = str.toFloat();
}

void US_AddAtom::update_hybridization_name(const QString &str)
{
   current_atom.hybrid.name = str;
}

void US_AddAtom::select_atom(int val)
{
   QString str;
   unsigned int i, j;
   current_atom.name = atom_list[val].name.upper();
   current_atom.hybrid.mw = atom_list[val].hybrid.mw;
   current_atom.hybrid.name = atom_list[val].hybrid.name;
   current_atom.hybrid.radius = atom_list[val].hybrid.radius;
   current_atom.hybrid.scat_len = atom_list[val].hybrid.scat_len;
   current_atom.hybrid.exch_prot = atom_list[val].hybrid.exch_prot;
   current_atom.hybrid.num_elect = atom_list[val].hybrid.num_elect;
   current_atom.hybrid.saxs_name = atom_list[val].hybrid.saxs_name;
   current_atom.saxs_excl_vol = atom_list[val].saxs_excl_vol;

   str.sprintf("%3.2f", atom_list[val].hybrid.mw);
   lbl_mw2->setText(str);
   str.sprintf("%3.2f", atom_list[val].hybrid.radius);
   lbl_radius2->setText(str);
   str.sprintf("%3.2f", atom_list[val].saxs_excl_vol);
   le_excl_vol->setText(str);
   le_name->setText(atom_list[val].name.upper());
   lbl_hybrid2->setText(atom_list[val].hybrid.name.upper());
   pb_delete->setEnabled(true);
   if (atom_list.size() > 0)
   {
      if (hybrid_list.size() > 0)
      {
         j = cmb_atom->currentItem();
         for (i=0; i<hybrid_list.size(); i++)
         {
            if (hybrid_list[i].name == atom_list[j].hybrid.name)
            {
               current_atom.hybrid.saxs_name = hybrid_list[i].saxs_name;
               cmb_hybrid->setCurrentItem(i);
               break;
            }
         }
      }
      if (saxs_list.size() > 0)
      {
         for (i=0; i<saxs_list.size(); i++)
         {
            if (saxs_list[i].saxs_name == current_atom.hybrid.saxs_name)
            {
               lbl_saxs_name2->setText(current_atom.hybrid.saxs_name);
               break;
            }
         }
      }
   }
}

void US_AddAtom::closeEvent(QCloseEvent *e)
{
   global_Xpos -= 30;
   global_Ypos -= 30;

   *widget_flag = false;
   e->accept();
}

void US_AddAtom::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_addatom.html");
}

void US_AddAtom::delete_atom()
{
   vector <struct atom>::iterator it;
   for (it=atom_list.begin(); it != atom_list.end(); it++)
   {
      if ((*it).name == current_atom.name && (*it).hybrid.name == current_atom.hybrid.name)
      {
         atom_list.erase(it);
         break;
      }
   }
   write_atom_file();
}

void US_AddAtom::set_excl_vol()
{
   unsigned int i;
   le_excl_vol->setEnabled(!cb_excl_vol->isChecked());
   if (cb_excl_vol->isChecked())
   {
      if (saxs_list.size() > 0)
      {
         for (i=0; i<saxs_list.size(); i++)
         {
            if (saxs_list[i].saxs_name == current_atom.hybrid.saxs_name)
            {
               current_atom.saxs_excl_vol = saxs_list[i].volume;
               QString str;
               str.sprintf("%3.2f", current_atom.saxs_excl_vol);
               le_excl_vol->setText(str);
               break;
            }
         }
      }
   }
}

