#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_pdb_visualization.h"
//Added by qt3to4:
#include <Q3GridLayout>
#include <QLabel>
#include <Q3Frame>
#include <QCloseEvent>

US_Hydrodyn_PDB_Visualization::US_Hydrodyn_PDB_Visualization(struct pdb_visualization *pdb,
                                                             bool *pdb_visualization_widget, QWidget *p, const char *name) : Q3Frame(p, name)
{
   this->pdb = pdb;
   this->pdb_visualization_widget = pdb_visualization_widget;
   *pdb_visualization_widget = true;
   USglobal=new US_Config();
   setPalette( PALET_FRAME );
   setCaption(tr("SOMO PDB Visualization Options"));
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_PDB_Visualization::~US_Hydrodyn_PDB_Visualization()
{
   *pdb_visualization_widget = false;
}

void US_Hydrodyn_PDB_Visualization::setupGUI()
{
   int minHeight1 = 30;

   lbl_info = new QLabel(tr("SOMO PDB Visualization Options:"), this);
   Q_CHECK_PTR(lbl_info);
   lbl_info->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Raised);
   lbl_info->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette( PALET_FRAME );
   AUTFBACK( lbl_info );
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   bg_visualization = new Q3ButtonGroup(3, Qt::Vertical, "Visualization Options", this);
   bg_visualization->setExclusive(true);
   connect(bg_visualization, SIGNAL(clicked(int)), this, SLOT(select_option(int)));

   cb_default = new QCheckBox(bg_visualization);
   cb_default->setText(tr(" Default RasMol Settings"));
   cb_default->setEnabled(true);
   cb_default->setMinimumHeight(minHeight1);
   cb_default->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_default->setPalette( PALET_NORMAL );
   AUTFBACK( cb_default );

   cb_spacefilling = new QCheckBox(bg_visualization);
   cb_spacefilling->setText(tr(" Space-filling, colors as bead colors"));
   cb_spacefilling->setEnabled(false);
   cb_spacefilling->setMinimumHeight(minHeight1);
   cb_spacefilling->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_spacefilling->setPalette( PALET_NORMAL );
   AUTFBACK( cb_spacefilling );

   cb_custom = new QCheckBox(bg_visualization);
   cb_custom->setText(tr(" Custom script file"));
   cb_custom->setEnabled(false);
   cb_custom->setMinimumHeight(minHeight1);
   cb_custom->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_custom->setPalette( PALET_NORMAL );
   AUTFBACK( cb_custom );

   bg_visualization->setButton((*pdb).visualization);

   pb_filename = new QPushButton(tr("Custom File:"), this);
   Q_CHECK_PTR(pb_filename);
   pb_filename->setEnabled(false);
   pb_filename->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_filename->setMinimumHeight(minHeight1);
   pb_filename->setPalette( PALET_PUSHB );
   connect(pb_filename, SIGNAL(clicked()), SLOT(select_filename()));

   le_filename = new QLineEdit(this, "filename Line Edit");
   le_filename->setText((*pdb).filename);
   le_filename->setEnabled(false);
   le_filename->setMinimumHeight(minHeight1);
   le_filename->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_filename->setPalette( PALET_NORMAL );
   AUTFBACK( le_filename );
   le_filename->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_filename, SIGNAL(textChanged(const QString &)), SLOT(update_filename(const QString &)));

   pb_cancel = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   int rows=11, columns = 2, spacing = 2, j=0, margin=4;
   Q3GridLayout *background=new Q3GridLayout(this, rows, columns, margin, spacing);

   background->addMultiCellWidget(lbl_info, j, j, 0, 1);
   j++;
   background->addMultiCellWidget(bg_visualization, j, j, 0, 1);
   j++;
   background->addWidget(pb_filename, j, 0);
   background->addWidget(le_filename, j, 1);
   j++;
   background->addWidget(pb_help, j, 0);
   background->addWidget(pb_cancel, j, 1);
}

void US_Hydrodyn_PDB_Visualization::select_option(int val)
{
   (*pdb).visualization = val;
}

void US_Hydrodyn_PDB_Visualization::update_filename(const QString &filename)
{
   (*pdb).filename = filename;
}

void US_Hydrodyn_PDB_Visualization::select_filename()
{
   QString filename = Q3FileDialog::getOpenFileName(USglobal->config_list.root_dir + "/somo",
                                                   "RasMol Scripts (*.spt *.SPT)",
                                                   this,
                                                   "Open RasMol Script files",
                                                   "Please select a RasMol Script file...");
   le_filename->setText(QDir::convertSeparators( filename ));   
   (*pdb).filename = filename;
}

void US_Hydrodyn_PDB_Visualization::cancel()
{
   close();
}

void US_Hydrodyn_PDB_Visualization::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_pdb_visualization.html");
}

void US_Hydrodyn_PDB_Visualization::closeEvent(QCloseEvent *e)
{
   *pdb_visualization_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

