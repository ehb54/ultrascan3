#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_addresidue.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_math.h"

#include "../include/color_black.xpm"
#include "../include/color_blue.xpm"
#include "../include/color_brightwhite.xpm"
#include "../include/color_brown.xpm"
#include "../include/color_cyan.xpm"
#include "../include/color_green.xpm"
#include "../include/color_grey.xpm"
#include "../include/color_lightblue.xpm"
#include "../include/color_lightcyan.xpm"
#include "../include/color_lightgreen.xpm"
#include "../include/color_lightmagenta.xpm"
#include "../include/color_lightred.xpm"
#include "../include/color_magenta.xpm"
#include "../include/color_red.xpm"
#include "../include/color_white.xpm"
#include "../include/color_yellow.xpm"
//Added by qt3to4:
#include <Q3TextStream>
#include <Q3HBoxLayout>
#include <QLabel>
#include <Q3GridLayout>
#include <QPixmap>
#include <Q3Frame>
#include <Q3BoxLayout>
#include <QCloseEvent>

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

// #define DEBUG_RESIDUE

US_AddResidue::US_AddResidue(bool *widget_flag, const double hydrovol, QWidget *p, const char *name) : QWidget( p, name)
{
   this->widget_flag = widget_flag;
   this->hydrovol = hydrovol;
   *widget_flag = true;
   online_help = NULL;
   USglobal = new US_Config();
   position_flag = false;
   hydration_flag = false;
   existing_residue = false;
   current_atom = 0;
   current_bead = 0;
   atom_hydration = 0;
   atom_filename = USglobal->config_list.system_dir + "/etc/somo.atom";
   residue_filename = USglobal->config_list.system_dir + "/etc/somo.residue";
   setPalette( PALET_FRAME );
   setCaption(tr("SoMo: Modify Residue Lookup Tables"));
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;
   new_residue.type = 0;
   new_bead.color = 0;
   new_bead.placing_method = 0;
   new_bead.hydration = 0;
   new_bead.visibility = 0;
   new_bead.volume = 0;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
   hybrids.clear();
   atoms.clear();
}

US_AddResidue::~US_AddResidue()
{
}

void US_AddResidue::setupGUI()
{
   int minHeight1 = 24;
   int minWidth1  = 144;

   lbl_info1 = new QLabel(tr(" 1: Define Residue Properties: "), this);
   Q_CHECK_PTR(lbl_info1);
   lbl_info1->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Raised);
   lbl_info1->setMinimumHeight(minHeight1);
   lbl_info1->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info1->setPalette( PALET_FRAME );
   AUTFBACK( lbl_info1 );
   lbl_info1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   pb_select_atom_file = new QPushButton(tr("Load Atom Definition File"), this);
   Q_CHECK_PTR(pb_select_atom_file);
   pb_select_atom_file->setMinimumHeight(minHeight1);
   pb_select_atom_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_select_atom_file->setPalette( PALET_PUSHB );
   connect(pb_select_atom_file, SIGNAL(clicked()), SLOT(select_atom_file()));

   lbl_atom_file = new QLabel(tr(" not selected"),this);
   lbl_atom_file->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Sunken);
   lbl_atom_file->setMinimumHeight(minHeight1);
   lbl_atom_file->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_atom_file->setPalette( PALET_EDIT );
   AUTFBACK( lbl_atom_file );
   lbl_atom_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   pb_select_residue_file = new QPushButton(tr("Load Residue Definition File"), this);
   Q_CHECK_PTR(pb_select_residue_file);
   pb_select_residue_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_select_residue_file->setEnabled(false);
   pb_select_residue_file->setMinimumHeight(minHeight1);
   pb_select_residue_file->setPalette( PALET_PUSHB );
   connect(pb_select_residue_file, SIGNAL(clicked()), SLOT(select_residue_file()));

   lbl_residue_file = new QLabel(tr(" not selected"),this);
   lbl_residue_file->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Sunken);
   lbl_residue_file->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_residue_file->setMinimumHeight(minHeight1);
   lbl_residue_file->setPalette( PALET_EDIT );
   AUTFBACK( lbl_residue_file );
   lbl_residue_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_numresidues = new QLabel(tr(" Number of Residues in File: 0"), this);
   Q_CHECK_PTR(lbl_numresidues);
   lbl_numresidues->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_numresidues->setMinimumHeight(minHeight1);
   lbl_numresidues->setPalette( PALET_LABEL );
   AUTFBACK( lbl_numresidues );
   lbl_numresidues->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   lb_residues = new Q3ListBox(this, "Residue Listing" );
   lb_residues->setPalette( PALET_NORMAL );
   AUTFBACK( lb_residues );
   lb_residues->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   lb_residues->setMinimumHeight(minHeight1);
   connect(lb_residues, SIGNAL(selected(int)), this, SLOT(select_residue(int)));

   lbl_residue_name = new QLabel(tr(" Residue Name:"), this);
   Q_CHECK_PTR(lbl_residue_name);
   lbl_residue_name->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_residue_name->setMinimumHeight(minHeight1);
   lbl_residue_name->setPalette( PALET_LABEL );
   AUTFBACK( lbl_residue_name );
   lbl_residue_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_residue_name = new QLineEdit(this, "Residue name Line Edit");
   le_residue_name->setMinimumHeight(minHeight1);
   le_residue_name->setEnabled(false);
   le_residue_name->setPalette( PALET_NORMAL );
   AUTFBACK( le_residue_name );
   le_residue_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_residue_name, SIGNAL(textChanged(const QString &)), SLOT(update_name(const QString &)));

   lbl_residue_comment = new QLabel(tr(" Description:"), this);
   Q_CHECK_PTR(lbl_residue_comment);
   lbl_residue_comment->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_residue_comment->setMinimumHeight(minHeight1);
   lbl_residue_comment->setPalette( PALET_LABEL );
   AUTFBACK( lbl_residue_comment );
   lbl_residue_comment->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_residue_comment = new QLineEdit(this, "Residue name Line Edit");
   le_residue_comment->setMinimumHeight(minHeight1);
   le_residue_comment->setEnabled(false);
   le_residue_comment->setPalette( PALET_NORMAL );
   AUTFBACK( le_residue_comment );
   le_residue_comment->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_residue_comment, SIGNAL(textChanged(const QString &)), SLOT(update_comment(const QString &)));

   lbl_numatoms = new QLabel(tr(" Number of Atoms in Residue:"), this);
   Q_CHECK_PTR(lbl_numatoms);
   lbl_numatoms->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_numatoms->setMinimumHeight(minHeight1);
   lbl_numatoms->setPalette( PALET_LABEL );
   AUTFBACK( lbl_numatoms );
   lbl_numatoms->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_numatoms= new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_numatoms );
   Q_CHECK_PTR(cnt_numatoms);
   cnt_numatoms->setRange(0, 1000, 1);
   cnt_numatoms->setValue(0);
   cnt_numatoms->setMinimumWidth( minWidth1 );
   cnt_numatoms->setEnabled(false);
   cnt_numatoms->setNumButtons(3);
   cnt_numatoms->setPalette( PALET_NORMAL );
   AUTFBACK( cnt_numatoms );
   connect(cnt_numatoms, SIGNAL(valueChanged(double)), SLOT(update_numatoms(double)));

   lbl_numbeads = new QLabel(tr(" Number of Beads for Residue:"), this);
   Q_CHECK_PTR(lbl_numbeads);
   lbl_numbeads->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_numbeads->setPalette( PALET_LABEL );
   AUTFBACK( lbl_numbeads );
   lbl_numbeads->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));
   lbl_numbeads->setMinimumHeight(minHeight1);

   cnt_numbeads= new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_numbeads );
   Q_CHECK_PTR(cnt_numbeads);
   cnt_numbeads->setRange(0, 1000, 1);
   cnt_numbeads->setValue(0);
   cnt_numbeads->setEnabled(false);
   cnt_numbeads->setNumButtons(3);
   cnt_numbeads->setMinimumWidth( minWidth1 );
   cnt_numbeads->setPalette( PALET_NORMAL );
   AUTFBACK( cnt_numbeads );
   connect(cnt_numbeads, SIGNAL(valueChanged(double)), SLOT(update_numbeads(double)));

   lbl_type = new QLabel(tr(" Residue Type:"), this);
   Q_CHECK_PTR(lbl_type);
   lbl_type->setMinimumHeight(minHeight1);
   lbl_type->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_type->setPalette( PALET_LABEL );
   AUTFBACK( lbl_type );
   lbl_type->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cmb_type = new Q3ComboBox(false, this, "Bead Color" );
   cmb_type->setPalette( PALET_NORMAL );
   AUTFBACK( cmb_type );
   cmb_type->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cmb_type->setSizeLimit(6);
   cmb_type->setEnabled(false);
   cmb_type->insertItem("Amino Acid");
   cmb_type->insertItem("Sugar Moiety");
   cmb_type->insertItem("Nucleotide");
   cmb_type->insertItem("Heme");
   cmb_type->insertItem("Phosphate");
   cmb_type->insertItem("Co-factor");
   cmb_type->insertItem("Ion");
   cmb_type->insertItem("Detergent");
   cmb_type->insertItem("Lipid");
   cmb_type->insertItem("Other");
   cmb_type->setMinimumHeight(minHeight1);
   connect(cmb_type, SIGNAL(activated(int)), this, SLOT(select_type(int)));

   lbl_molvol = new QLabel(tr(" Residue anhydrous mol. vol. (A^3):"), this);
   Q_CHECK_PTR(lbl_molvol);
   lbl_molvol->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_molvol->setMinimumHeight(minHeight1);
   lbl_molvol->setPalette( PALET_LABEL );
   AUTFBACK( lbl_molvol );
   lbl_molvol->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_molvol = new QLineEdit(this, "Residue molvol Line Edit");
   le_molvol->setPalette( PALET_NORMAL );
   AUTFBACK( le_molvol );
   le_molvol->setMinimumHeight(minHeight1);
   le_molvol->setEnabled(false);
   le_molvol->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_molvol, SIGNAL(textChanged(const QString &)), SLOT(update_molvol(const QString &)));

   lbl_vbar = new QLabel(tr(" Residue partial spec. vol. (cm^3/g):"), this);
   Q_CHECK_PTR(lbl_vbar);
   lbl_vbar->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_vbar->setMinimumHeight(minHeight1);
   lbl_vbar->setPalette( PALET_LABEL );
   AUTFBACK( lbl_vbar );
   lbl_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_vbar = new QLineEdit(this, "Residue vbar Line Edit");
   le_vbar->setPalette( PALET_NORMAL );
   AUTFBACK( le_vbar );
   le_vbar->setMinimumHeight(minHeight1);
   le_vbar->setEnabled(false);
   le_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_vbar, SIGNAL(textChanged(const QString &)), SLOT(update_vbar(const QString &)));

   lbl_asa = new QLabel(tr(" Max. Accessible Surface Area (A^2):"), this);
   Q_CHECK_PTR(lbl_asa);
   lbl_asa->setMinimumHeight(minHeight1);
   lbl_asa->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_asa->setPalette( PALET_LABEL );
   AUTFBACK( lbl_asa );
   lbl_asa->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_asa = new QLineEdit(this, "Residue asa Line Edit");
   le_asa->setPalette( PALET_NORMAL );
   AUTFBACK( le_asa );
   le_asa->setMinimumHeight(minHeight1);
   le_asa->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_asa->setEnabled(false);
   connect(le_asa, SIGNAL(textChanged(const QString &)), SLOT(update_asa(const QString &)));

   pb_accept_residue = new QPushButton(tr(" Accept Residue and Continue "), this);
   Q_CHECK_PTR(pb_accept_residue);
   pb_accept_residue->setEnabled(false);
   pb_accept_residue->setMinimumHeight(minHeight1);
   pb_accept_residue->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_accept_residue->setPalette( PALET_PUSHB );
   connect(pb_accept_residue, SIGNAL(clicked()), SLOT(accept_residue()));

   lbl_info2 = new QLabel(tr(" 2. Define Residue Atoms: "), this);
   Q_CHECK_PTR(lbl_info2);
   lbl_info2->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Raised);
   lbl_info2->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info2->setMinimumHeight(minHeight1);
   lbl_info2->setPalette( PALET_FRAME );
   AUTFBACK( lbl_info2 );
   lbl_info2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_r_atoms = new QLabel(tr(" Select Residue Atom to be defined:  "), this);
   Q_CHECK_PTR(lbl_r_atoms);
   lbl_r_atoms->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_r_atoms->setMinimumHeight(minHeight1);
   lbl_r_atoms->setPalette( PALET_LABEL );
   AUTFBACK( lbl_r_atoms );
   lbl_r_atoms->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cmb_r_atoms = new Q3ComboBox(false, this, "Residue's Atom Listing" );
   cmb_r_atoms->setPalette( PALET_NORMAL );
   AUTFBACK( cmb_r_atoms );
   cmb_r_atoms->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cmb_r_atoms->setSizeLimit(5);
   cmb_r_atoms->setMinimumHeight(minHeight1);
   connect(cmb_r_atoms, SIGNAL(activated(int)), this, SLOT(select_r_atom(int)));

   lbl_define_atom = new QLabel(tr(" Select Atom from Lookup Table:"), this);
   Q_CHECK_PTR(lbl_define_atom);
   lbl_define_atom->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_define_atom->setMinimumHeight(minHeight1);
   lbl_define_atom->setPalette( PALET_LABEL );
   AUTFBACK( lbl_define_atom );
   lbl_define_atom->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cmb_atoms = new Q3ComboBox(false, this, "Atom Listing" );
   cmb_atoms->setPalette( PALET_NORMAL );
   AUTFBACK( cmb_atoms );
   cmb_atoms->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cmb_atoms->setSizeLimit(5);
   cmb_atoms->setMinimumHeight(minHeight1);
   connect(cmb_atoms, SIGNAL(activated(int)), this, SLOT(update_hybrid(int)));

   lbl_define_hybrid = new QLabel(tr(" Select Hybridization for Atom:"), this);
   Q_CHECK_PTR(lbl_define_hybrid);
   lbl_define_hybrid->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_define_hybrid->setMinimumHeight(minHeight1);
   lbl_define_hybrid->setPalette( PALET_LABEL );
   AUTFBACK( lbl_define_hybrid );
   lbl_define_hybrid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cmb_hybrids = new Q3ComboBox(false, this, "Hybridization Listing" );
   cmb_hybrids->setPalette( PALET_NORMAL );
   AUTFBACK( cmb_hybrids );
   cmb_hybrids->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cmb_hybrids->setSizeLimit(5);
   cmb_hybrids->setMinimumHeight(minHeight1);

   lbl_positioning = new QLabel(tr(" Atom determines Position: "), this);
   Q_CHECK_PTR(lbl_positioning);
   lbl_positioning->setMinimumHeight(minHeight1);
   lbl_positioning->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_positioning->setPalette( PALET_LABEL );
   AUTFBACK( lbl_positioning );
   lbl_positioning->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cb_positioning = new QCheckBox(this);
   cb_positioning->setText(tr(" (Check if true)"));
   cb_positioning->setChecked(position_flag);
   cb_positioning->setEnabled(false);
   cb_positioning->setMinimumHeight(minHeight1);
   cb_positioning->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_positioning->setPalette( PALET_NORMAL );
   AUTFBACK( cb_positioning );
   connect(cb_positioning, SIGNAL(clicked()), SLOT(set_positioning()));

   lbl_atom_hydration = new QLabel(tr(" Hydration Number for Atom: "), this);
   Q_CHECK_PTR(lbl_atom_hydration);
   lbl_atom_hydration->setMinimumHeight(minHeight1);
   lbl_atom_hydration->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_atom_hydration->setPalette( PALET_LABEL );
   AUTFBACK( lbl_atom_hydration );
   lbl_atom_hydration->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_atom_hydration = new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_atom_hydration );
   Q_CHECK_PTR(cnt_atom_hydration);
   cnt_atom_hydration->setRange(0, 100, 1);
   cnt_atom_hydration->setValue(0);
   cnt_atom_hydration->setMinimumWidth( minWidth1 );
   cnt_atom_hydration->setEnabled(true);
   cnt_atom_hydration->setNumButtons(3);
   cnt_atom_hydration->setEnabled(false);
   cnt_atom_hydration->setPalette( PALET_NORMAL );
   AUTFBACK( cnt_atom_hydration );
   connect(cnt_atom_hydration, SIGNAL(valueChanged(double)), SLOT(update_atom_hydration(double)));

   pb_accept_atom = new QPushButton(tr("Assign Current Atom"), this);
   Q_CHECK_PTR(pb_accept_atom);
   pb_accept_atom->setEnabled(false);
   pb_accept_atom->setMinimumHeight(minHeight1);
   pb_accept_atom->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_accept_atom->setPalette( PALET_PUSHB );
   connect(pb_accept_atom, SIGNAL(clicked()), SLOT(accept_atom()));

   pb_atom_continue = new QPushButton(tr("Continue"), this);
   Q_CHECK_PTR(pb_atom_continue);
   pb_atom_continue->setEnabled(false);
   pb_atom_continue->setMinimumHeight(minHeight1);
   pb_atom_continue->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_atom_continue->setPalette( PALET_PUSHB );
   connect(pb_atom_continue, SIGNAL(clicked()), SLOT(atom_continue()));

   lbl_info3 = new QLabel(tr(" 3. Define Residue Bead Properties: "), this);
   Q_CHECK_PTR(lbl_info3);
   lbl_info3->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Raised);
   lbl_info3->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info3->setMinimumHeight(minHeight1);
   lbl_info3->setPalette( PALET_FRAME );
   AUTFBACK( lbl_info3 );
   lbl_info3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_r_beads = new QLabel(tr(" Select Residue Bead to be defined: "), this);
   Q_CHECK_PTR(lbl_r_beads);
   lbl_r_beads->setMinimumHeight(minHeight1);
   lbl_r_beads->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_r_beads->setPalette( PALET_LABEL );
   AUTFBACK( lbl_r_beads );
   lbl_r_beads->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cmb_r_beads = new Q3ComboBox(false, this, "bead Listing" );
   cmb_r_beads->setPalette( PALET_NORMAL );
   AUTFBACK( cmb_r_beads );
   cmb_r_beads->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cmb_r_beads->setSizeLimit(5);
   cmb_r_beads->setEnabled(false);
   cmb_r_beads->setMinimumHeight(minHeight1);
   connect(cmb_r_beads, SIGNAL(activated(int)), this, SLOT(select_r_bead(int)));

   lbl_bead_color = new QLabel(tr(" Select Bead Color: "), this);
   Q_CHECK_PTR(lbl_bead_color);
   lbl_bead_color->setMinimumHeight(minHeight1);
   lbl_bead_color->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_bead_color->setPalette( PALET_LABEL );
   AUTFBACK( lbl_bead_color );
   lbl_bead_color->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cmb_bead_color = new Q3ComboBox(false, this, "Bead Color" );
   cmb_bead_color->setPalette( PALET_NORMAL );
   AUTFBACK( cmb_bead_color );
   cmb_bead_color->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cmb_bead_color->setSizeLimit(5);
   cmb_bead_color->setMinimumHeight(minHeight1);
   cmb_bead_color->insertItem(QPixmap(color_black), " (0)");
   cmb_bead_color->insertItem(QPixmap(color_blue), " (1)");
   cmb_bead_color->insertItem(QPixmap(color_green), " (2)");
   cmb_bead_color->insertItem(QPixmap(color_cyan), " (3)");
   cmb_bead_color->insertItem(QPixmap(color_red), " (4)");
   cmb_bead_color->insertItem(QPixmap(color_magenta), " (5)");
   cmb_bead_color->insertItem(QPixmap(color_brown), " (6)");
   cmb_bead_color->insertItem(QPixmap(color_white), " (7)");
   cmb_bead_color->insertItem(QPixmap(color_grey), " (8)");
   cmb_bead_color->insertItem(QPixmap(color_lightblue), " (9)");
   cmb_bead_color->insertItem(QPixmap(color_lightgreen), " (10)");
   cmb_bead_color->insertItem(QPixmap(color_lightcyan), " (11)");
   cmb_bead_color->insertItem(QPixmap(color_lightred), " (12)");
   cmb_bead_color->insertItem(QPixmap(color_lightmagenta), " (13)");
   cmb_bead_color->insertItem(QPixmap(color_yellow), " (14)");
   cmb_bead_color->insertItem(QPixmap(color_brightwhite), " (15)");
   cmb_bead_color->setCurrentItem(1);
   cmb_bead_color->setEnabled(false);
   connect(cmb_bead_color, SIGNAL(activated(int)), this, SLOT(select_bead_color(int)));

   cb_hydration = new QCheckBox(this);
   cb_hydration->setText(tr(" Override Bead Hydration Value: "));
   cb_hydration->setChecked(hydration_flag);
   cb_hydration->setEnabled(false);
   cb_hydration->setMinimumHeight(minHeight1);
   //   cb_hydration->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   cb_hydration->setPalette( PALET_LABEL );
   AUTFBACK( cb_hydration );
   cb_hydration->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));
   connect(cb_hydration, SIGNAL(clicked()), SLOT(set_hydration()));

   cnt_hydration= new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_hydration );
   Q_CHECK_PTR(cnt_hydration);
   cnt_hydration->setRange(0, 100, 1);
   cnt_hydration->setValue(0);
   cnt_hydration->setMinimumWidth( minWidth1 );
   cnt_hydration->setEnabled(true);
   cnt_hydration->setNumButtons(3);
   cnt_hydration->setEnabled(false);
   cnt_hydration->setPalette( PALET_NORMAL );
   AUTFBACK( cnt_hydration );
   connect(cnt_hydration, SIGNAL(valueChanged(double)), SLOT(update_hydration(double)));

   lbl_placing = new QLabel(tr(" Select Positioning Method: "), this);
   Q_CHECK_PTR(lbl_placing);
   lbl_placing->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_placing->setMinimumHeight(minHeight1);
   lbl_placing->setPalette( PALET_LABEL );
   AUTFBACK( lbl_placing );
   lbl_placing->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cmb_placing = new Q3ComboBox(false, this, "Placing Combo" );
   cmb_placing->setPalette( PALET_NORMAL );
   AUTFBACK( cmb_placing );
   cmb_placing->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cmb_placing->setSizeLimit(5);
   cmb_placing->insertItem("Center of Gravity");
   cmb_placing->insertItem("Farthest Atom");
   cmb_placing->insertItem("No Positioning");
   cmb_placing->setEnabled(false);
   cmb_placing->setMinimumHeight(minHeight1);
   connect(cmb_placing, SIGNAL(activated(int)), this, SLOT(select_placing_method(int)));

   lbl_select_beadatom = new QLabel(tr(" Select Atom for Bead (multi-selection OK): "), this);
   Q_CHECK_PTR(lbl_select_beadatom);
   lbl_select_beadatom->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_select_beadatom->setMinimumHeight(minHeight1);
   lbl_select_beadatom->setPalette( PALET_LABEL );
   AUTFBACK( lbl_select_beadatom );
   lbl_select_beadatom->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   lbl_list_beadatom = new QLabel(tr(" Currently defined Atoms for Bead: "), this);
   Q_CHECK_PTR(lbl_list_beadatom);
   lbl_list_beadatom->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_list_beadatom->setMinimumHeight(minHeight1);
   lbl_list_beadatom->setPalette( PALET_LABEL );
   AUTFBACK( lbl_list_beadatom );
   lbl_list_beadatom->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   lb_select_beadatom = new Q3ListBox(this, "beadatom selection listbox" );
   lb_select_beadatom->setPalette( PALET_NORMAL );
   AUTFBACK( lb_select_beadatom );
   lb_select_beadatom->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //lb_select_beadatom->setEnabled(false);
   lb_select_beadatom->setSelectionMode(Q3ListBox::Extended);
   lb_select_beadatom->setMinimumHeight(minHeight1);
   lb_select_beadatom->setHScrollBarMode(Q3ScrollView::Auto);
   lb_select_beadatom->setVScrollBarMode(Q3ScrollView::Auto);
   connect(lb_select_beadatom, SIGNAL(selectionChanged()), this, SLOT(select_beadatom()));

   lb_list_beadatom = new Q3ListBox(this, "beadatom list listbox" );
   lb_list_beadatom->setPalette( PALET_NORMAL );
   AUTFBACK( lb_list_beadatom );
   lb_list_beadatom->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //lb_list_beadatom->setEnabled(false);
   lb_list_beadatom->setSelectionMode(Q3ListBox::Extended);
   lb_list_beadatom->setHScrollBarMode(Q3ScrollView::Auto);
   lb_list_beadatom->setVScrollBarMode(Q3ScrollView::Auto);
   lb_list_beadatom->setMinimumHeight(minHeight1);

   lbl_beadchain = new QLabel(tr(" This Bead is part of the: "), this);
   Q_CHECK_PTR(lbl_beadchain);
   lbl_beadchain->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_beadchain->setMinimumHeight(minHeight1);
   lbl_beadchain->setPalette( PALET_LABEL );
   AUTFBACK( lbl_beadchain );
   lbl_beadchain->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   rb_backbone = new QRadioButton(tr("Backbone"), this);
   rb_backbone->setMinimumHeight(minHeight1);
   rb_backbone->setEnabled(false);
   rb_backbone->setChecked(true);
   rb_backbone->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_backbone->setPalette( PALET_NORMAL );
   AUTFBACK( rb_backbone );

   rb_sidechain = new QRadioButton(tr("Sidechain"), this);
   rb_sidechain->setEnabled(false);
   rb_sidechain->setChecked(false);
   rb_sidechain->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_sidechain->setMinimumHeight(minHeight1);
   rb_sidechain->setPalette( PALET_NORMAL );
   AUTFBACK( rb_sidechain );

   bg_chain = new Q3ButtonGroup(1, Qt::Horizontal, 0);
   bg_chain->setRadioButtonExclusive(true);
   bg_chain->insert(rb_backbone);
   bg_chain->insert(rb_sidechain);
   bg_chain->setMinimumHeight(minHeight1);
   connect(bg_chain, SIGNAL(clicked(int)), SLOT(set_chain(int)));

   lbl_bead_volume = new QLabel(tr(" Bead Volume: "), this);
   Q_CHECK_PTR(lbl_bead_volume);
   lbl_bead_volume->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_bead_volume->setMinimumHeight(minHeight1);
   lbl_bead_volume->setPalette( PALET_LABEL );
   AUTFBACK( lbl_bead_volume );
   lbl_bead_volume->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_bead_volume = new QLineEdit(this, "Bead Volume Line Edit");
   le_bead_volume->setPalette( PALET_NORMAL );
   AUTFBACK( le_bead_volume );
   le_bead_volume->setMinimumHeight(minHeight1);
   le_bead_volume->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_bead_volume->setEnabled(false);
   connect(le_bead_volume, SIGNAL(textChanged(const QString &)), SLOT(update_bead_volume(const QString &)));

   lbl_bead_mw = new QLabel(tr(" Bead Mol. Weight: "), this);
   Q_CHECK_PTR(lbl_bead_mw);
   lbl_bead_mw->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_bead_mw->setMinimumHeight(minHeight1);
   lbl_bead_mw->setPalette( PALET_LABEL );
   AUTFBACK( lbl_bead_mw );
   lbl_bead_mw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_bead_mw = new QLineEdit(this, "Bead MW Line Edit");
   le_bead_mw->setPalette( PALET_NORMAL );
   AUTFBACK( le_bead_mw );
   le_bead_mw->setMinimumHeight(minHeight1);
   le_bead_mw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_bead_mw->setEnabled(true);
   le_bead_mw->setReadOnly(true);

   lbl_bead_hydro_from_atom = new QLabel(tr(" Bead Hydration from Atoms' Values: "), this);
   Q_CHECK_PTR(lbl_bead_hydro_from_atom);
   lbl_bead_hydro_from_atom->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_bead_hydro_from_atom->setMinimumHeight(minHeight1);
   lbl_bead_hydro_from_atom->setPalette( PALET_LABEL );
   AUTFBACK( lbl_bead_hydro_from_atom );
   lbl_bead_hydro_from_atom->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_bead_hydro_from_atom = new QLineEdit(this, "Bead Hydration from Atoms' Line Edit");
   le_bead_hydro_from_atom->setPalette( PALET_NORMAL );
   AUTFBACK( le_bead_hydro_from_atom );
   le_bead_hydro_from_atom->setMinimumHeight(minHeight1);
   le_bead_hydro_from_atom->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_bead_hydro_from_atom->setEnabled(true);
   le_bead_hydro_from_atom->setReadOnly(true);

   lbl_bead_hydrovol = new QLabel(tr(" Bead hydrated Volume, Radius: "), this);
   Q_CHECK_PTR(lbl_bead_hydrovol);
   lbl_bead_hydrovol->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_bead_hydrovol->setMinimumHeight(minHeight1);
   lbl_bead_hydrovol->setPalette( PALET_LABEL );
   AUTFBACK( lbl_bead_hydrovol );
   lbl_bead_hydrovol->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_bead_hydrovol = new QLineEdit(this, "Residue Volume Line Edit");
   le_bead_hydrovol->setPalette( PALET_NORMAL );
   AUTFBACK( le_bead_hydrovol );
   le_bead_hydrovol->setMinimumHeight(minHeight1);
   le_bead_hydrovol->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_bead_hydrovol->setEnabled(true);
   le_bead_hydrovol->setReadOnly(true);

   pb_accept_bead = new QPushButton(tr("Accept Bead Definition"), this);
   Q_CHECK_PTR(pb_accept_bead);
   pb_accept_bead->setMinimumHeight(minHeight1);
   pb_accept_bead->setEnabled(false);
   pb_accept_bead->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_accept_bead->setPalette( PALET_PUSHB );
   connect(pb_accept_bead, SIGNAL(clicked()), SLOT(accept_bead()));

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_reset = new QPushButton(tr("Reset"), this);
   Q_CHECK_PTR(pb_reset);
   pb_reset->setMinimumHeight(minHeight1);
   pb_reset->setEnabled(false);
   pb_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_reset->setPalette( PALET_PUSHB );
   connect(pb_reset, SIGNAL(clicked()), SLOT(reset()));

   pb_add = new QPushButton(tr("Add Residue to File"), this);
   Q_CHECK_PTR(pb_add);
   pb_add->setEnabled(false);
   pb_add->setMinimumHeight(minHeight1);
   pb_add->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_add->setPalette( PALET_PUSHB );
   connect(pb_add, SIGNAL(clicked()), SLOT(add()));

   pb_delete_residue = new QPushButton(tr("Delete Residue"), this);
   Q_CHECK_PTR(pb_delete_residue);
   pb_delete_residue->setMinimumHeight(minHeight1);
   pb_delete_residue->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_delete_residue->setEnabled(false);
   pb_delete_residue->setPalette( PALET_PUSHB );
   connect(pb_delete_residue, SIGNAL(clicked()), SLOT(delete_residue()));

   pb_close = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_close);
   pb_close->setMinimumHeight(minHeight1);
   pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_close->setPalette( PALET_PUSHB );
   connect(pb_close, SIGNAL(clicked()), SLOT(close()));

   int rows=24, columns = 5, spacing = 2, j=0, margin=4, colspace=10;
   Q3GridLayout *background = new Q3GridLayout(this, rows, columns, margin, spacing);

   background->setColSpacing(2, colspace);
   //section 1
   background->addMultiCellWidget(lbl_info1, j, j, 0, 1);
   j++;
   background->addWidget(pb_select_atom_file, j, 0);
   background->addWidget(lbl_atom_file, j, 1);
   j++;
   background->addWidget(pb_select_residue_file, j, 0);
   background->addWidget(lbl_residue_file, j, 1);
   j++;
   background->addWidget(lbl_residue_name, j, 0);
   background->addWidget(le_residue_name, j, 1);
   j++;
   background->addWidget(lbl_residue_comment, j, 0);
   background->addWidget(le_residue_comment, j, 1);
   j++;
   background->addWidget(lbl_numatoms, j, 0);
   background->addWidget(cnt_numatoms, j, 1);
   j++;
   background->addWidget(lbl_numbeads, j, 0);
   background->addWidget(cnt_numbeads, j, 1);
   j++;
   background->addWidget(lbl_type, j, 0);
   background->addWidget(cmb_type, j, 1);
   j++;
   background->addWidget(lbl_molvol, j, 0);
   background->addWidget(le_molvol, j, 1);
   j++;
   background->addWidget(lbl_vbar, j, 0);
   background->addWidget(le_vbar, j, 1);
   j++;
   background->addWidget(lbl_asa, j, 0);
   background->addWidget(le_asa, j, 1);
   j++;
   background->addWidget(lbl_numresidues, j, 0);
   background->addMultiCellWidget(lb_residues, j, j+3, 1, 1);
   background->addWidget(pb_accept_residue, j+1, 0);
   j+=4;
   //section 2
   background->addMultiCellWidget(lbl_info2, j, j, 0, 1);
   j++;
   background->addWidget(lbl_r_atoms, j, 0);
   background->addWidget(cmb_r_atoms, j, 1);
   j++;
   background->addWidget(lbl_define_atom, j, 0);
   background->addWidget(cmb_atoms, j, 1);
   j++;
   background->addWidget(lbl_define_hybrid, j, 0);
   background->addWidget(cmb_hybrids, j, 1);
   j++;
   background->addWidget(lbl_positioning, j, 0);
   background->addWidget(cb_positioning, j, 1);
   j++;
   background->addWidget(lbl_atom_hydration, j, 0);
   background->addWidget(cnt_atom_hydration, j, 1);
   j++;
   background->addWidget(pb_accept_atom, j, 0);
   background->addWidget(pb_atom_continue, j, 1);
   j = 0;
   //section 3
   background->addMultiCellWidget(lbl_info3, j, j, 3, 4);
   j++;
   background->addWidget(lbl_r_beads, j, 3);
   background->addWidget(cmb_r_beads, j, 4);
   j++;
   background->addWidget(lbl_bead_color, j, 3);
   background->addWidget(cmb_bead_color, j, 4);
   j++;
   background->addWidget(lbl_placing, j, 3);
   background->addWidget(cmb_placing, j, 4);
   j++;
   background->addWidget(lbl_beadchain, j, 3);
   Q3BoxLayout *bl = new Q3HBoxLayout(0);
   bl->addWidget(rb_backbone);
   bl->addWidget(rb_sidechain);
   background->addLayout(bl, j, 4);
   j++;
   background->addWidget(lbl_list_beadatom, j, 3);
   background->addWidget(lbl_select_beadatom, j, 4);
   j++;
   background->addMultiCellWidget(lb_list_beadatom, j, j+7, 3, 3);
   background->addMultiCellWidget(lb_select_beadatom, j, j+7, 4, 4);
   j+=8;
   background->addWidget(lbl_bead_volume, j, 3);
   background->addWidget(le_bead_volume, j, 4);
   j++;
   background->addWidget(lbl_bead_mw, j, 3);
   background->addWidget(le_bead_mw, j, 4);
   j++;
   background->addWidget(lbl_bead_hydro_from_atom, j, 3);
   background->addWidget(le_bead_hydro_from_atom, j, 4);
   j++;
   background->addWidget(cb_hydration, j, 3);
   background->addWidget(cnt_hydration, j, 4);
   j++;
   background->addWidget(lbl_bead_hydrovol, j, 3);
   background->addWidget(le_bead_hydrovol, j, 4);
   j++;
   background->addWidget(pb_accept_bead, j, 3);
   background->addWidget(pb_reset, j, 4);
   j++;
   background->addWidget(pb_add, j, 3);
   background->addWidget(pb_delete_residue, j, 4);
   j++;
   background->addWidget(pb_help, j, 3);
   background->addWidget(pb_close, j, 4);

   enable_area_1(false);
   enable_area_2(false);
   enable_area_3(false);
}

void US_AddResidue::enable_area_1(bool state)
{
   cnt_numatoms->setEnabled(state);
   cnt_numbeads->setEnabled(state);
   le_asa->setEnabled(state);
   le_molvol->setEnabled(state);
   le_vbar->setEnabled(state);
   cmb_type->setEnabled(state);
   le_residue_name->setEnabled(state);
   le_residue_comment->setEnabled(state);
   pb_reset->setEnabled(state);
   pb_accept_residue->setEnabled(state);
}

void US_AddResidue::enable_area_2(bool state)
{
   cmb_r_atoms->setEnabled(state);
   cmb_atoms->setEnabled(state);
   cmb_atoms->setEnabled(state);
   cmb_hybrids->setEnabled(state);
   cb_positioning->setEnabled(state);
   cnt_atom_hydration->setEnabled(state);
   pb_accept_atom->setEnabled(state);
   pb_atom_continue->setEnabled(state);
}

void US_AddResidue::enable_area_3(bool state)
{
   cmb_r_beads->setEnabled(state);
   cmb_bead_color->setEnabled(state);
   cmb_placing->setEnabled(state);
   rb_backbone->setEnabled(state);
   rb_sidechain->setEnabled(state);
   le_bead_volume->setEnabled(state);
   le_bead_mw->setEnabled(state);
   le_bead_hydro_from_atom->setEnabled(state);
   cb_hydration->setEnabled(false);
   le_bead_hydrovol->setEnabled(state);
   lb_list_beadatom->setEnabled(state);
   lb_select_beadatom->setEnabled(state);
}

void US_AddResidue::add()
{
   int item = -1;
   unsigned int i, j;
   QString str1;
   double sum = 0e0;
   for (i = 0; i < new_residue.r_bead.size(); i++)
   {
      sum += (double)new_residue.r_bead[i].volume;
   }
   QString cmp1 = QString("").sprintf("%7.2f", new_residue.molvol);
   QString cmp2 = QString("").sprintf("%7.2f", sum);

   if ( cmp1 != cmp2 )
   {
      switch ( QMessageBox::warning(this, 
                                    tr("Attention:"),
                                    tr("The residue volume does not match the volume of the beads:\n\n") +
                                    QString(tr("Residue volume: %1 A^3, Sum of beads: %2 A^3\n\n"))
                                    .arg(cmp1).arg(cmp2) +
                                    tr("What would you like to do?\n"),
                                    tr("&Correct the bead volume manually"), 
                                    tr("&Override"),
                                    0, 
                                    0,
                                    0 
                                    ) )
      {
      case 0 : // correct manually
         pb_add->setEnabled(false);
         return;
         break;
      case 1 : // override
         break;
      }
      //      QMessageBox::message("Attention:", "The residue volume does not match the volume of the beads:\n\n" + str1);
      //      return;
   }
   for (i=0; i<residue_list.size(); i++)
   {
      if (residue_list[i].name.upper() == new_residue.name.upper() && residue_list[i].comment.upper() == new_residue.comment.upper())
      {
         item = (int) i;
         residue_list[i].type = new_residue.type;
         residue_list[i].molvol = new_residue.molvol;
         residue_list[i].vbar = new_residue.vbar;
         residue_list[i].asa = new_residue.asa;
         residue_list[i].r_atom.clear();
         residue_list[i].r_bead.clear();
         for (j=0; j<new_residue.r_atom.size(); j++)
         {
            residue_list[i].r_atom.push_back(new_residue.r_atom[j]);
         }
         for (j=0; j<new_residue.r_bead.size(); j++)
         {
            residue_list[i].r_bead.push_back(new_residue.r_bead[j]);
         }
      }
   }
   if (item < 0)
   {
      residue_list.push_back(new_residue);
   }
   write_residue_file();
   str1.sprintf(tr(" Number of Residues in File: %d"), residue_list.size());
   lbl_numresidues->setText(str1);
   pb_accept_bead->setEnabled(false);
   pb_add->setEnabled(false);
   existing_residue = false;
   reset();
}

void US_AddResidue::read_residue_file(const QString & filename)
{
   QString str1, str2;
   unsigned int numatoms, numbeads, i, j, positioner;
   QFile f(filename);
   residue_list.clear();
   lb_residues->clear();
   i=1;
   if (f.open(QIODevice::ReadOnly))
   {
      Q3TextStream ts(&f);
      while (!ts.atEnd())
      {
         new_residue.comment = ts.readLine();
         ts >> new_residue.name;
         ts >> new_residue.type;
         ts >> new_residue.molvol;
         ts >> new_residue.asa;
         ts >> numatoms;
         ts >> numbeads;
         ts >> new_residue.vbar;
         //            cout << "name: " << new_residue.name << ", type: " << new_residue.type
         //                  << ", molvol: " << new_residue.molvol << ", asa: " << new_residue.asa <<
         //                  ", numatoms: " << numatoms << ", beads: " << numbeads << ", vbar: " << new_residue.vbar << endl;
         ts.readLine(); // read rest of line
         new_residue.r_atom.clear();
         new_residue.r_bead.clear();
         for (j=0; j<numatoms; j++)
         {
            ts >> new_atom.name;
            ts >> new_atom.hybrid.name;
            ts >> new_atom.hybrid.mw;
            ts >> new_atom.hybrid.radius;
            ts >> new_atom.bead_assignment;
            ts >> positioner;
            //               cout << "Atom: " << new_atom.name << ": " << new_atom.mw << ", " << new_atom.radius << ", "
            //               << new_atom.bead_assignment << endl;
            if(positioner == 0)
            {
               new_atom.positioner = false;
            }
            else
            {
               new_atom.positioner = true;
            }
            ts >> new_atom.serial_number;
            ts >> new_atom.hydration;
            str2 = ts.readLine(); // read rest of line
            if (!new_atom.name.isEmpty() && new_atom.hybrid.radius > 0.0 && new_atom.hybrid.mw > 0.0)
            {
               new_residue.r_atom.push_back(new_atom);
               /*                  str1.sprintf("%d: ", j+1);
                                   str1 += new_atom.name;
                                   cmb_r_atoms->insertItem(str1);
               */
            }
            else
            {
               QMessageBox::warning(this, tr("UltraScan Warning"),
                                    tr("Please note:\n\nThere was an error reading\nthe selected Residue File!\n\nAtom "
                                       + new_atom.name + " cannot be read and will be deleted from List."),
                                    QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
            }
         }
         for (j=0; j<numbeads; j++)
         {
            ts >> new_bead.hydration;
            ts >> new_bead.color;
            ts >> new_bead.placing_method;
            ts >> new_bead.chain;
            ts >> new_bead.volume;
            str2 = ts.readLine(); // read rest of line
            new_residue.r_bead.push_back(new_bead);
            //               str1.sprintf("Bead %d: defined", j+1);
            //               cmb_r_beads->insertItem(str1);
         }
         calc_bead_mw(&new_residue);
         if ( !new_residue.name.isEmpty()
              && new_residue.molvol > 0.0
              && new_residue.asa > 0.0)
         {
            QString str3 ;
            switch (new_residue.type)
            {
            case 0:
               {
                  str3 = "Amino Acid";
                  break;
               }
            case 1:
               {
                  str3 = "Sugar Moiety";
                  break;
               }
            case 2:
               {
                  str3 = "Nucleotide";
                  break;
               }
            case 3:
               {
                  str3 = "Heme";
                  break;
               }
            case 4:
               {
                  str3 = "Phosphate";
                  break;
               }
            case 5:
               {
                  str3 = "Co-factor";
                  break;
               }
            case 6:
               {
                  str3 = "Ion";
                  break;
               }
            case 7:
               {
                  str3 = "Detergent";
                  break;
               }
            case 8:
               {
                  str3 = "Lipid";
                  break;
               }
            case 9:
               {
                  str3 = "Other";
                  break;
               }
            }
            residue_list.push_back(new_residue);
            str1.sprintf("%d: ", i);
            str1 += str3 + ", ";
            if (new_residue.comment.isEmpty())
            {
               str1 += new_residue.name;
            }
            else
            {
               str1 += new_residue.name + " (" + new_residue.comment + ")";
            }
            lb_residues->insertItem(str1);
            i++;
         }
      }
      f.close();
   }
   else
   {
      QMessageBox::warning(this, tr("UltraScan Warning"),
                           tr("Please note:\n\nThere was an error reading\nthe selected Residue File!\n"
                              "Please select a different file."),
                           QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
   }
}

void US_AddResidue::select_residue_file()
{
   QString old_filename = residue_filename, str1;
   residue_filename = Q3FileDialog::getOpenFileName(USglobal->config_list.system_dir + "/etc", "*.residue *.RESIDUE", this);
   if (residue_filename.isEmpty())
   {
      residue_filename = old_filename;
   }
   else
   {
      lbl_residue_file->setText(residue_filename);
      QFile f(residue_filename);
      residue_list.clear();
      lb_residues->clear();
      if (f.open(QIODevice::ReadWrite|QIODevice::Text))
      {
         f.close();
         read_residue_file(residue_filename);
         pb_select_atom_file->setEnabled(false);
         pb_select_residue_file->setEnabled(false);
         
         enable_area_1(true);
      }
      else
      {
         QMessageBox mb(tr("UltraScan Warning"),
                        tr("Please note:\n\nThere was a problem opening this\nfile with read/write permission.\n"
                           "This file cannot be modified.\n\nDo you want to open the file in read-only mode"),
                        QMessageBox::Question,
                        QMessageBox::Yes | QMessageBox::Default,
                        QMessageBox::No,
                        QMessageBox::Cancel | QMessageBox::Escape, 0, 0, false, 0);
         switch (mb.exec())
         {
         case QMessageBox::Yes:
            {
               read_residue_file(residue_filename);
               pb_select_atom_file->setEnabled(false);
               pb_select_residue_file->setEnabled(false);
               break;
            }
         default:
            {
               return;   
            }
         }
      }
   }
   str1.sprintf(tr(" Number of Residues in File: %d"), residue_list.size());
   lbl_numresidues->setText(str1);
}

void US_AddResidue::calc_bead_mw(struct residue *res)
{
   for (unsigned int i=0; i<(*res).r_bead.size(); i++)
   {
      (*res).r_bead[i].mw = 0.0;
      (*res).r_bead[i].atom_hydration = 0;
      for (unsigned int j=0; j<(*res).r_atom.size(); j++)
      {
         if ((*res).r_atom[j].bead_assignment == i)
         {
            (*res).r_bead[i].mw += (*res).r_atom[j].hybrid.mw;
            (*res).r_bead[i].atom_hydration += (*res).r_atom[j].hydration;
         }
      }
   }
}

void US_AddResidue::select_atom_file()
{
   QString old_filename = atom_filename, str1, str2;
   atom_filename = Q3FileDialog::getOpenFileName(USglobal->config_list.system_dir + "/etc", "*.atom *.ATOM", this);
   if (atom_filename.isEmpty())
   {
      atom_filename = old_filename;
   }
   else
   {
      lbl_atom_file->setText(atom_filename);
      QFile f(atom_filename);
      atom_list.clear();
      cmb_atoms->clear();
      unsigned int i, j;
      if (f.open(QIODevice::ReadOnly|QIODevice::Text))
      {
         Q3TextStream ts(&f);
         while (!ts.atEnd())
         {
            ts >> new_atom.name;
            ts >> new_atom.hybrid.name;
            ts >> new_atom.hybrid.mw;
            ts >> new_atom.hybrid.radius;
            str2 = ts.readLine(); // read rest of line
            if (!new_atom.name.isEmpty() && new_atom.hybrid.radius > 0.0 && new_atom.hybrid.mw > 0.0)
            {
               atom_list.push_back(new_atom);
            }
         }
         f.close();
         bool flag;
         atoms.clear();
         for (i=0; i<atom_list.size(); i++)
         {
            flag = false;
            for (j=0; j<atoms.size(); j++)
            {
               if (atoms[j] == atom_list[i].name)
               {
                  flag = true; // this atom_list item is already in the atoms vector
               }
            }
            if (flag)
            {
               flag = false; // reset flag
            }
            else // add it to the list
            {
               atoms.push_back(atom_list[i].name);
               cmb_atoms->insertItem(atom_list[i].name);
            }
         }
         pb_select_residue_file->setEnabled(true);
      }
   }
}

void US_AddResidue::update_name(const QString &str)
{
   new_residue.name = str;
}

void US_AddResidue::update_comment(const QString &str)
{
   new_residue.comment = str;
}

void US_AddResidue::update_numatoms(double val)
{
   QString str;
   new_residue.r_atom.clear();
   new_residue.r_atom.resize((unsigned int) val);
   cmb_r_atoms->clear();
   for (unsigned int i=0; i<(unsigned int) val; i++)
   {
      str.sprintf("Atom %d: (undefined)", i+1);
      cmb_r_atoms->insertItem(str);
   }
}

void US_AddResidue::update_numbeads(double val)
{
   QString str;
   new_residue.r_bead.clear();
   new_residue.r_bead.resize((unsigned int) val);
   cmb_r_beads->clear();
   for (unsigned int i=0; i<(unsigned int) val; i++)
   {
      str.sprintf("Bead %d: (undefined)", i+1);
      cmb_r_beads->insertItem(str);
      new_residue.r_bead[i].color = 0;
      new_residue.r_bead[i].chain = 0;
      new_residue.r_bead[i].hydration = 0;
      new_residue.r_bead[i].placing_method = 0;
   }
}

void US_AddResidue::update_molvol(const QString &str)
{
   new_residue.molvol = str.toFloat();
}

void US_AddResidue::update_vbar(const QString &str)
{
   new_residue.vbar = str.toFloat();
}

void US_AddResidue::update_asa(const QString &str)
{
   new_residue.asa = str.toFloat();
}

void US_AddResidue::select_type(int val)
{
   new_residue.type = (unsigned int) val;
}

void US_AddResidue::update_hybrid(int val)
{
   cmb_hybrids->clear();
#if defined(DEBUG_RESIDUE)
   cout << "update hybrid val " << val << endl;
#endif
   for (unsigned int i=0; i<atom_list.size(); i++)
   {
      if(atom_list[i].name == cmb_atoms->text(val))
      {
#if defined(DEBUG_RESIDUE)
         cout << "adding atom_list[" << i << "].name = " << atom_list[i].name 
              << " hybrid.name " << atom_list[i].hybrid.name << endl;
#endif
         cmb_hybrids->insertItem(atom_list[i].hybrid.name);
      }
   }
}

void US_AddResidue::select_beadatom()
{
   unsigned int i;
   QString str;
   lb_list_beadatom->clear();
   for (i=0; i<lb_select_beadatom->count(); i++)
   {
      if (new_residue.r_atom[i].bead_assignment == current_bead)
      {
         new_residue.r_atom[i].bead_assignment = 100000; // reset all atoms previously assigned to this bead to a different value
      }
   }
   for (i=0; i<lb_select_beadatom->count(); i++)
   {
      if(lb_select_beadatom->isSelected(i))
      {
         lb_list_beadatom->insertItem(lb_select_beadatom->text(i));
         //         cout << "Current bead 1: " << current_bead << endl;
         new_residue.r_atom[i].bead_assignment = current_bead;
      }
   }
   calc_bead_mw(&new_residue);
   str.sprintf("%7.2f", new_residue.r_bead[current_bead].mw);
   le_bead_mw->setText(str);
   str.sprintf("%f",new_residue.r_bead[current_bead].atom_hydration);
   le_bead_hydro_from_atom->setText(str);
   new_residue.r_bead[current_bead].hydration = new_residue.r_bead[current_bead].atom_hydration;
   cnt_hydration->setValue(new_residue.r_bead[current_bead].hydration);
   cb_hydration->setChecked(false);
}

void US_AddResidue::update_hydration(double val)
{
   new_bead.hydration = (unsigned int) val;
   cb_hydration->setChecked ( new_bead.hydration != 
                              new_bead.atom_hydration );
}

void US_AddResidue::update_atom_hydration(double val)
{
   new_atom.hydration = (unsigned int) val;
}

void US_AddResidue::select_bead_color(int val)
{
   if (val == 0 || val == 6)
   {
      QMessageBox::warning(this, tr("UltraScan Warning"),
                           tr("Please note:\n\nBlack and brown are reserved colors,\nplease choose a different color."),
                           QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
      cmb_bead_color->setCurrentItem(1);
      return;
   }
   new_bead.color = (unsigned int) val;
}

void US_AddResidue::select_placing_method(int val)
{
   new_bead.placing_method = (unsigned int) val;
}

void US_AddResidue::select_r_bead(int val)
{
   QString str;
   current_bead = (unsigned int) val;
   if (existing_residue)
   {
      new_bead.color = new_residue.r_bead[current_bead].color;
      new_bead.chain = new_residue.r_bead[current_bead].chain;
      new_bead.placing_method = new_residue.r_bead[current_bead].placing_method;
      new_bead.hydration = new_residue.r_bead[current_bead].hydration;
      new_bead.volume = new_residue.r_bead[current_bead].volume;
      cmb_bead_color->setCurrentItem(new_residue.r_bead[current_bead].color);
      cmb_placing->setCurrentItem(new_residue.r_bead[current_bead].placing_method);
      cnt_hydration->setValue(new_residue.r_bead[current_bead].hydration);
      str.sprintf("%7.2f", new_residue.r_bead[current_bead].volume);
      le_bead_volume->setText(str);
      calc_bead_mw(&new_residue);
      str.sprintf("%7.2f", new_residue.r_bead[current_bead].mw);
      le_bead_mw->setText(str);
      str.sprintf("%f",new_residue.r_bead[current_bead].atom_hydration);
      le_bead_hydro_from_atom->setText(str);
      cb_hydration->setChecked ( new_residue.r_bead[current_bead].atom_hydration != 
                                 new_residue.r_bead[current_bead].hydration );
      new_bead.atom_hydration = new_residue.r_bead[current_bead].atom_hydration;
      float h_volume;
      h_volume = new_residue.r_bead[current_bead].hydration * hydrovol + new_residue.r_bead[current_bead].volume;
      float radius = pow((h_volume * (3.0/(4.0*M_PI))), 1.0/3.0);
      str.sprintf("%7.2f A^3, %7.2f A", h_volume, radius);
      le_bead_hydrovol->setText(str);

      if (new_residue.r_bead[current_bead].chain)
      {
         bg_chain->setButton(1);
      }
      else
      {
         bg_chain->setButton(0);
      }
   }
   else
   {
      set_chain(0); // default is backbone for a new bead
   }
   unsigned int i;
   lb_list_beadatom->clear();
   //   cout << "Current bead 2: " << current_bead << endl;
   for (i=0; i<new_residue.r_atom.size(); i++)
   {
      if (new_residue.r_atom[i].bead_assignment == current_bead)
      {
         str.sprintf("Atom %d: ", new_residue.r_atom[i].serial_number + 1);
         str += new_residue.r_atom[i].name;
         if (new_residue.r_atom[i].positioner)
         {
            str += " (" + new_residue.r_atom[i].hybrid.name + ", Positioning: yes)";
         }
         else
         {
            str += " (" + new_residue.r_atom[i].hybrid.name + ", Positioning: no)";
         }
         lb_list_beadatom->insertItem(str);
      }
   }
}

void US_AddResidue::select_r_atom(int val)
{
   if (1 || existing_residue)
   {
      if (new_residue.r_atom[val].positioner)
      {
         cb_positioning->setChecked(true);
      }
      else
      {
         cb_positioning->setChecked(false);
      }
      for ( unsigned int i = 0; i < (unsigned int) cmb_atoms->count(); i++ )
      {
         if (cmb_atoms->text(i) == new_residue.r_atom[val].name) 
         {
            cmb_atoms->setCurrentItem(i);
            update_hybrid(i);
            for ( unsigned int j = 0; j < (unsigned int) cmb_hybrids->count(); j++ ) 
            {
               if (cmb_hybrids->text(j) == new_residue.r_atom[val].hybrid.name) 
               {
                  cmb_hybrids->setCurrentItem(j);
                  break;
               }
            }
            break;
         }
      }
      cnt_atom_hydration->setValue((double) new_residue.r_atom[val].hydration);
   } 
}

void US_AddResidue::select_residue(int val)
{
   reset();
   existing_residue = true;
   QString str;
   new_residue.r_atom.clear();
   new_residue.r_bead.clear();
   le_residue_name->setText(residue_list[val].name);
   le_residue_comment->setText(residue_list[val].comment);
   cnt_numatoms->setValue(residue_list[val].r_atom.size());
   cnt_numbeads->setValue(residue_list[val].r_bead.size());
   cmb_type->setCurrentItem(residue_list[val].type);
   le_molvol->setText(str.sprintf("%7.2f", residue_list[val].molvol));
   le_vbar->setText(str.sprintf("%5.3f", residue_list[val].vbar));
   le_asa->setText(str.sprintf("%7.2f", residue_list[val].asa));
   new_residue = residue_list[val];
#if defined(DEBUG_RESIDUE)
   print_residue (new_residue);
   print_residue (residue_list[val]);
#endif
}

void US_AddResidue::print_residue(struct residue res)
{
   unsigned int i;
   cout << "Residue name: " << res.name << endl;
   cout << "Residue Description: " << res.comment << endl;
   cout << "Residue type: " << res.type << endl;
   cout << "Residue molvol: " << res.molvol << endl;
   cout << "Residue vbar: " << res.vbar << endl;
   cout << "Residue asa: " << res.asa << endl;
   cout << "Number of atoms: " << res.r_atom.size() << endl;
   for (i=0; i<res.r_atom.size(); i++)
   {
      cout << "Atom " << i+1 << ":\n";
      cout << "\tName:" << res.r_atom[i].name << endl;
      cout << "\tMW:" << res.r_atom[i].hybrid.mw << endl;
      cout << "\tRadius:" << res.r_atom[i].hybrid.radius << endl;
      cout << "\tBead Assignment:" << res.r_atom[i].bead_assignment << endl;
      cout << "\tPositioner:" << res.r_atom[i].positioner << endl;
      cout << "\tSerial #:" << res.r_atom[i].serial_number << endl;
      cout << "\tHydration:" << res.r_atom[i].hydration << endl;
   }
   cout << endl;
   cout << "Number of beads: " << res.r_bead.size() << endl;
   for (i=0; i<res.r_bead.size(); i++)
   {
      cout << "Bead " << i+1 << ":\n";
      cout << "\tHydration:" << res.r_bead[i].hydration << endl;
      cout << "\tColor:" << res.r_bead[i].color << endl;
      cout << "\tPlacing Method:" << res.r_bead[i].placing_method << endl;
      cout << "\tVisibility:" << res.r_bead[i].visibility << endl;
      cout << "\tChain:" << res.r_bead[i].chain << endl;
   }
}

void US_AddResidue::help()
{
   if (online_help == NULL)
   {
      online_help = new US_Help(this);
   }
   online_help->show_help("manual/somo_residue.html");
}

void US_AddResidue::reset()
{
   pb_delete_residue->setEnabled(false);
   pb_select_atom_file->setEnabled(false);
   pb_select_residue_file->setEnabled(false);
   le_residue_name->setEnabled(true);
   le_residue_comment->setEnabled(true);
   le_residue_name->setText("");
   le_residue_comment->setText("");
   le_bead_volume->setText("0.0");
   le_bead_mw->setText("0.0");
   le_bead_hydrovol->setText("0.0, 0.0");
   le_bead_hydro_from_atom->setText("0");
   cnt_numbeads->setEnabled(true);
   cnt_numbeads->setValue(0);
   cnt_numatoms->setEnabled(true);
   cnt_numatoms->setValue(0);
   cmb_type->setEnabled(true);
   le_molvol->setEnabled(true);
   le_molvol->setText("");
   le_vbar->setEnabled(true);
   le_vbar->setText("");
   le_asa->setEnabled(true);
   le_asa->setText("");

   pb_accept_residue->setEnabled(true);
   pb_accept_atom->setEnabled(false);
   pb_atom_continue->setEnabled(false);
   pb_accept_atom->setEnabled(false);
   cmb_r_atoms->clear();
   //   cmb_atoms->setCurrentItem(0);
   cmb_r_atoms->setEnabled(false);
   cmb_hybrids->setEnabled(false);

   cmb_r_beads->setEnabled(false);
   cmb_r_beads->clear();
   cmb_bead_color->setEnabled(false);
   cmb_bead_color->setCurrentItem(0);
   cb_hydration->setEnabled(false);
   cb_hydration->setChecked(false);
   cnt_hydration->setEnabled(false);
   cnt_hydration->setValue(0);
   cnt_atom_hydration->setEnabled(false);
   cnt_atom_hydration->setValue(0);
   cmb_placing->setCurrentItem(0);
   cmb_placing->setEnabled(false);
   /*
     rb_backbone->setEnabled(false);
     rb_backbone->setChecked(true);
     rb_sidechain->setEnabled(false);
     rb_sidechain->setChecked(false);
   */
   bg_chain->setEnabled(false);
   bg_chain->setButton(0);
   lb_select_beadatom->setEnabled(false);
   lb_select_beadatom->clear();
   lb_list_beadatom->setEnabled(false);
   lb_list_beadatom->clear();
   cb_positioning->setEnabled(false);
   cb_positioning->setChecked(false);
   existing_residue = false;
   enable_area_2(false);
   enable_area_3(false);
}

void US_AddResidue::accept_residue()
{
   if(new_residue.name.isEmpty()
      || new_residue.r_atom.size() == 0
      || new_residue.r_bead.size() == 0
      || new_residue.molvol == 0
      || new_residue.vbar == 0
      || new_residue.asa == 0)
   {
      QMessageBox::warning(this, tr("UltraScan Warning"),
                           tr("Please define all residue fields\nbefore accepting the residue!"),
                           QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
   }
   else
   {
      le_residue_name->setEnabled(false);
      le_residue_comment->setEnabled(false);
      le_molvol->setEnabled(false);
      le_vbar->setEnabled(false);
      le_asa->setEnabled(false);
      cmb_type->setEnabled(false);
      cmb_r_atoms->setEnabled(true);
      cmb_hybrids->setEnabled(true);
      cnt_numbeads->setEnabled(false);
      cnt_numatoms->setEnabled(false);
      pb_accept_residue->setEnabled(false);
      pb_accept_atom->setEnabled(true);
      cb_positioning->setEnabled(true);
      cnt_atom_hydration->setEnabled(true);
      pb_select_atom_file->setEnabled(false);
      pb_select_residue_file->setEnabled(false);
      current_atom = 0;
      cmb_r_atoms->setCurrentItem(current_atom);
      enable_area_2(true);
   }
   if(existing_residue)
   {
      QString str;
      cmb_r_atoms->clear();
      for (unsigned int i=0; i<new_residue.r_atom.size(); i++)
      {
         str.sprintf("Atom %d: ", i + 1);
         str += new_residue.r_atom[i].name;
         if (new_residue.r_atom[i].positioner)
         {
            str += " (" + new_residue.r_atom[i].hybrid.name + ", Positioning: yes)";
         }
         else
         {
            str += " (" + new_residue.r_atom[i].hybrid.name + ", Positioning: no)";
         }
         cmb_r_atoms->insertItem(str);
      }
      cmb_r_atoms->setCurrentItem(0);
      for ( unsigned int i = 0; i < (unsigned int) cmb_atoms->count(); i++ )
      {
         if (cmb_atoms->text(i) == new_residue.r_atom[0].name) 
         {
            cmb_atoms->setCurrentItem(i);
            update_hybrid(i);
            for ( unsigned int j = 0; j < (unsigned int) cmb_hybrids->count(); j++ ) 
            {
               if (cmb_hybrids->text(j) == new_residue.r_atom[0].hybrid.name) 
               {
                  cmb_hybrids->setCurrentItem(j);
                  break;
               }
            }
            break;
         }
      }
      if (new_residue.r_atom[0].positioner)
      {
         cb_positioning->setChecked(true);
      }
      else
      {
         cb_positioning->setChecked(false);
      }
      cnt_atom_hydration->setValue((double) new_residue.r_atom[0].hydration);
   }
   else
   {
      for ( unsigned int i = 0; i < new_residue.r_atom.size(); i++ )
      {
         new_residue.r_atom[i].hydration = 0;
         new_residue.r_atom[i].positioner = true;
      }
      cb_positioning->setChecked(true);
      cnt_atom_hydration->setValue(0);
      cnt_atom_hydration->setEnabled(true);
   }
   // if we loaded an existing residue for editing and all atoms have been defined, we can activate the
   // Continue button at this point. First, we need to check that the atoms all have been defined.
   QString str;
   bool flag = true;
   for (int i=0; i<cmb_r_atoms->count(); i++)
   {
      str = cmb_r_atoms->text(i);
      if (str.contains("undefined"))
      {
         flag = false;
      }
   }
   if (flag)
   {
      pb_atom_continue->setEnabled(true); // all atoms are defined now
   }
}

void US_AddResidue::atom_continue()
{
   QString str;
   pb_accept_atom->setEnabled(false);
   cb_hydration->setEnabled(true);
   cb_hydration->setEnabled(true);
   cb_hydration->setChecked(false);
   cnt_hydration->setEnabled(true);
   cmb_r_beads->setEnabled(true);
   cmb_bead_color->setEnabled(true);
   cmb_placing->setEnabled(true);
   lb_select_beadatom->setEnabled(true);
   le_bead_volume->setEnabled(true);
   cb_positioning->setEnabled(false);
   rb_backbone->setEnabled(true);
   rb_backbone->setChecked(true);
   rb_sidechain->setEnabled(true);
   rb_sidechain->setChecked(false);
   bg_chain->setEnabled(true);
   bg_chain->setButton(0);
   lb_list_beadatom->setEnabled(true);
   lb_list_beadatom->clear();
   lb_select_beadatom->setEnabled(true);
   lb_select_beadatom->clear();
   cmb_r_beads->clear();
   if(existing_residue)
   {
      for (unsigned int i=0; i<new_residue.r_bead.size(); i++)
      {
         str.sprintf("Bead %d: defined", i + 1);
         cmb_r_beads->insertItem(str);
      }
   }
   else
   {
      for (unsigned int i=0; i<new_residue.r_bead.size(); i++)
      {
         str.sprintf("Bead %d: undefined", i + 1);
         cmb_r_beads->insertItem(str);
      }
   }
   cmb_r_beads->setCurrentItem(0);
   current_bead = 0;
   current_atom = 0;
   select_r_bead(0);
   for (int i=0; i<cmb_r_atoms->count(); i++)
   {
      lb_select_beadatom->insertItem(cmb_r_atoms->text(i));
   }
   pb_accept_bead->setEnabled(true);
   pb_atom_continue->setEnabled(false);
   enable_area_2(false);
   enable_area_3(true);
   /*
     lb_select_beadatom->setCurrentItem(0);

     cmb_r_beads->clear();
     for (unsigned int i=0; i<new_residue.r_bead.size(); i++)
     {
     str.sprintf("Bead %d: defined", i + 1);
     cmb_r_beads->insertItem(str);
     }
     cmb_r_beads->setCurrentItem(0);
     cmb_bead_color->setCurrentItem(new_residue.r_bead[0].color);
     cnt_hydration->setValue(new_residue.r_bead[0].hydration);
     cmb_placing->setCurrentItem(new_residue.r_bead[0].placing_method);
     if (new_residue.r_bead[0].chain)
     {
     bg_chain->setButton(1);
     }
     else
     {
     bg_chain->setButton(0);
     }
     str.sprintf("%f", new_residue.r_bead[0].volume);
     le_bead_volume->setText(str);
     for (unsigned int i=0; i<new_residue.r_atom.size(); i++)
     {
     if(new_residue.r_atom[i].bead_assignment == 0)
     {
     lb_list_beadatom->insertItem(lb_select_beadatom->text(i));
     }
     }
   */
}

void US_AddResidue::accept_bead()
{
   if (new_bead.volume == 0.0)
   {
      QMessageBox::message("Attention:", "No bead volume entered for this bead!\nPlease correct this and try again...");
      pb_add->setEnabled(false);
      return;
   }
   if (new_bead.color == 0 || new_bead.color == 6)
   {
      QMessageBox::warning(this, tr("UltraScan Warning"),
                           tr("Please note:\n\nBlack and brown are reserved colors,\nplease choose a different bead color."),
                           QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
      cmb_bead_color->setCurrentItem(1);
      return;
   }
   
   QString str;
   str.sprintf("Bead %d: defined", current_bead + 1);
   cmb_r_beads->changeItem(str, current_bead);
   bool flag = true;
   for (int i=0; i<cmb_r_beads->count(); i++)
   {
      str = cmb_r_beads->text(i);
      if (str.contains("undefined"))
      {
         flag = false;
      }
   }
   if (flag)
   {
      pb_add->setEnabled(true);
      pb_delete_residue->setEnabled(true);
   }
   new_residue.r_bead[current_bead] = new_bead;
   new_bead.volume = 0.0;
}

void US_AddResidue::accept_atom()
{
   int current_item1, current_item2=0;
   if (existing_residue)
   {
         position_flag = new_residue.r_atom[cmb_r_atoms->currentItem()].positioner;
   }
#if defined(DEBUG_RESIDUE)
   cout << "Residue print accept_atom() - 1: \n";
   print_residue(new_residue);
#endif
   QString str;
   // find current hybridization in atom_list:
   current_item1 = cmb_r_atoms->currentItem();
   for (unsigned int i=0; i<atom_list.size(); i++)
   {
      if (cmb_atoms->currentText() == atom_list[i].name && cmb_hybrids->currentText() == atom_list[i].hybrid.name)
      {
         current_item2 = i;
         break;
      }
   }
   if (existing_residue)
   {
      unsigned int bead = new_residue.r_atom[current_item1].bead_assignment;
      unsigned int snum = new_residue.r_atom[current_item1].serial_number;
      bool pos = new_residue.r_atom[current_item1].positioner;
      float hydration =  new_residue.r_atom[current_item1].hydration;
      new_residue.r_atom[current_item1] = atom_list[current_item2];
      new_residue.r_atom[current_item1].bead_assignment = bead;
      new_residue.r_atom[current_item1].positioner = pos;
      new_residue.r_atom[current_item1].serial_number = snum;
      new_residue.r_atom[current_item1].hydration = hydration;
   }
   else
   {
      new_residue.r_atom[current_item1] = atom_list[current_item2];
   }

   // assign a serial number specific to this residue so atoms can be distinguished
   // and prevented from being used twice in different beads

   new_residue.r_atom[current_item1].hydration = (unsigned int) cnt_atom_hydration->value();
   new_residue.r_atom[current_item1].serial_number = current_item1;
   new_residue.r_atom[current_item1].positioner = cb_positioning->isChecked(); // position_flag;
   str.sprintf("Atom %d: " + atom_list[current_item2].name, current_item1+1);
   str += " (" + new_residue.r_atom[current_item1].hybrid.name + ", Positioning: ";
   if (new_residue.r_atom[current_item1].positioner)
   {
      str += "yes)";
   }
   else
   {
      str += "no)";
   }
   cmb_r_atoms->changeItem(str, current_item1);
   bool flag = true;
   for (int i=0; i<cmb_r_atoms->count(); i++)
   {
      str = cmb_r_atoms->text(i);
      if (str.contains("undefined"))
      {
         flag = false;
      }
   }
   if (flag)
   {
      pb_atom_continue->setEnabled(true); // all atoms are defined now
   }
#if defined(DEBUG_RESIDUE)
   cout << "Residue print accept_atom() - 2: \n";
   print_residue(new_residue);
#endif
}

void US_AddResidue::set_chain(int val)
{
   new_bead.chain = val;
}

void US_AddResidue::set_positioning()
{
   if (cb_positioning->isChecked())
   {
      position_flag = true;
   }
   else
   {
      position_flag = false;
   }
}

void US_AddResidue::set_hydration()
{
   if (cb_hydration->isChecked())
   {
      hydration_flag = true;
   }
   else
   {
      hydration_flag = false;
   }
}

void US_AddResidue::closeEvent(QCloseEvent *e)
{
   global_Xpos -= 30;
   global_Ypos -= 30;

   *widget_flag = false;
   e->accept();
}

void US_AddResidue::update_bead_volume(const QString & val)
{
   new_bead.volume = val.toFloat();
}

void US_AddResidue::delete_residue()
{
   QString str1;
   vector <struct residue>::iterator it;
   for (it=residue_list.begin(); it != residue_list.end(); it++)
   {
      if ((*it).name.upper() == new_residue.name.upper())
      {
         residue_list.erase(it);
         break;
      }
   }
   write_residue_file();
   str1.sprintf(tr(" Number of Residues in File: %d"), residue_list.size());
   lbl_numresidues->setText(str1);
   pb_accept_bead->setEnabled(false);
   pb_add->setEnabled(false);
   existing_residue = false;
   reset();
}

void US_AddResidue::write_residue_file()
{
   QString str1;
   QFile f(residue_filename);
   if (f.open(QIODevice::WriteOnly|QIODevice::Text))
   {
      lb_residues->clear();
      Q3TextStream ts(&f);
      for (unsigned int i=0; i<residue_list.size(); i++)
      {
         ts << residue_list[i].comment << endl;
         ts << residue_list[i].name.upper()
            << "\t" << residue_list[i].type
            << "\t" << str1.sprintf("%7.2f", residue_list[i].molvol)
            << "\t" << residue_list[i].asa
            << "\t" << residue_list[i].r_atom.size()
            << "\t" << residue_list[i].r_bead.size()
            << "\t" << residue_list[i].vbar << endl;
         for (unsigned int j=0; j<residue_list[i].r_atom.size(); j++)
         {
            ts << residue_list[i].r_atom[j].name.upper()
               << "\t" << residue_list[i].r_atom[j].hybrid.name
               << "\t" << residue_list[i].r_atom[j].hybrid.mw
               << "\t" << residue_list[i].r_atom[j].hybrid.radius
               << "\t" << residue_list[i].r_atom[j].bead_assignment
               << "\t" << (unsigned int) residue_list[i].r_atom[j].positioner
               << "\t" << residue_list[i].r_atom[j].serial_number 
               << "\t" << residue_list[i].r_atom[j].hydration << endl;
         }
         for (unsigned int j=0; j<residue_list[i].r_bead.size(); j++)
         {
            ts << residue_list[i].r_bead[j].hydration
               << "\t" << residue_list[i].r_bead[j].color
               << "\t" << residue_list[i].r_bead[j].placing_method
               << "\t" << residue_list[i].r_bead[j].chain
               << "\t" << residue_list[i].r_bead[j].volume << endl;
         }
         str1.sprintf("%d: ", i+1);

         QString str3 ;
         switch (residue_list[i].type)
         {
         case 0:
            {
               str3 = "Amino Acid";
               break;
            }
         case 1:
            {
               str3 = "Sugar Moiety";
               break;
            }
         case 2:
            {
               str3 = "Nucleotide";
               break;
            }
         case 3:
            {
               str3 = "Heme";
               break;
            }
         case 4:
            {
               str3 = "Phosphate";
               break;
            }
         case 5:
            {
               str3 = "Co-factor";
               break;
            }
         case 6:
            {
               str3 = "Ion";
               break;
            }
         case 7:
            {
               str3 = "Detergent";
               break;
            }
         case 8:
            {
               str3 = "Lipid";
               break;
            }
         case 9:
            {
               str3 = "Other";
               break;
            }
         }
         str1.sprintf("%d: ", i + 1);
         str1 += str3 + ", ";
         if (residue_list[i].comment.isEmpty())
         {
            str1 += residue_list[i].name;
         }
         else
         {
            str1 += residue_list[i].name + " (" + residue_list[i].comment + ")";
         }
         
         lb_residues->insertItem(str1);
      }
      f.close();
   }
   else
   {
      QMessageBox::warning(this, tr("UltraScan Warning"),
                           tr("Please note:\n\nThis file was opened in read-only mode.\n"
                              "This file cannot be edited.\nPlease select a different file."),
                           QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
   }
}
