#include "../include/us_hydrodyn_hydro_zeno.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_math.h"

US_Hydrodyn_Hydro_Zeno::US_Hydrodyn_Hydro_Zeno(struct hydro_options *hydro,
                                               bool *hydro_zeno_widget, 
                                               void *us_hydrodyn, 
                                               QWidget *p, 
                                               const char *name) : QFrame(p, name)
{
   this->hydro = hydro;
   this->hydro_zeno_widget = hydro_zeno_widget;
   this->us_hydrodyn = us_hydrodyn;
   *hydro_zeno_widget = true;
   USglobal=new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("SOMO  Hydrodynamic Calculation Zeno Options"));
   this->setMinimumWidth(500);
   setupGUI();
   update_enables();
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_Hydro_Zeno::~US_Hydrodyn_Hydro_Zeno()
{
   *hydro_zeno_widget = false;
}

void US_Hydrodyn_Hydro_Zeno::setupGUI()
{
   int minHeight1 = 30;
   QString str;   

   lbl_info = new QLabel(tr("SOMO Hydrodynamic Calculation Zeno Options:"), this);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   cb_zeno_zeno = new QCheckBox( this );
   cb_zeno_zeno->setText( tr( "Compute Zeno:") );
   cb_zeno_zeno->setEnabled( true );
   cb_zeno_zeno->setChecked( hydro->zeno_zeno );
   cb_zeno_zeno->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_zeno_zeno->setPalette( QPalette( USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal ) );
   connect( cb_zeno_zeno, SIGNAL( clicked() ), this, SLOT( set_zeno_zeno() ) );

   lbl_zeno_zeno_steps = new QLabel(tr(" Zeno Steps (Thousands):"), this );
   lbl_zeno_zeno_steps->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_zeno_zeno_steps->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_zeno_zeno_steps->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_zeno_zeno_steps = new QLineEdit( this, "Zeno_Zeno_Steps Line Edit");
   le_zeno_zeno_steps->setText(str.sprintf("%u",(*hydro).zeno_zeno_steps));
   le_zeno_zeno_steps->setAlignment(Qt::AlignVCenter);
   le_zeno_zeno_steps->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_zeno_zeno_steps->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_zeno_zeno_steps, SIGNAL(textChanged(const QString &)), SLOT(update_zeno_zeno_steps(const QString &)));

   cb_zeno_interior = new QCheckBox( this );
   cb_zeno_interior->setText( tr( "Compute Interior:") );
   cb_zeno_interior->setEnabled( true );
   cb_zeno_interior->setChecked( hydro->zeno_interior );
   cb_zeno_interior->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_zeno_interior->setPalette( QPalette( USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal ) );
   connect( cb_zeno_interior, SIGNAL( clicked() ), this, SLOT( set_zeno_interior() ) );

   lbl_zeno_interior_steps = new QLabel(tr(" Interior Steps (Thousands):"), this );
   lbl_zeno_interior_steps->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_zeno_interior_steps->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_zeno_interior_steps->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_zeno_interior_steps = new QLineEdit( this, "Zeno_Interior_Steps Line Edit");
   le_zeno_interior_steps->setText(str.sprintf("%u",(*hydro).zeno_interior_steps));
   le_zeno_interior_steps->setAlignment(Qt::AlignVCenter);
   le_zeno_interior_steps->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_zeno_interior_steps->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_zeno_interior_steps, SIGNAL(textChanged(const QString &)), SLOT(update_zeno_interior_steps(const QString &)));

   cb_zeno_surface = new QCheckBox( this );
   cb_zeno_surface->setText( tr( "Compute Surface:") );
   cb_zeno_surface->setEnabled( true );
   cb_zeno_surface->setChecked( hydro->zeno_surface );
   cb_zeno_surface->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_zeno_surface->setPalette( QPalette( USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal ) );
   connect( cb_zeno_surface, SIGNAL( clicked() ), this, SLOT( set_zeno_surface() ) );

   lbl_zeno_surface_steps = new QLabel(tr(" Surface Steps (Thousands):"), this );
   lbl_zeno_surface_steps->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_zeno_surface_steps->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_zeno_surface_steps->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_zeno_surface_steps = new QLineEdit( this, "Zeno_Surface_Steps Line Edit");
   le_zeno_surface_steps->setText(str.sprintf("%u",(*hydro).zeno_surface_steps));
   le_zeno_surface_steps->setAlignment(Qt::AlignVCenter);
   le_zeno_surface_steps->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_zeno_surface_steps->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_zeno_surface_steps, SIGNAL(textChanged(const QString &)), SLOT(update_zeno_surface_steps(const QString &)));

   lbl_zeno_surface_thickness = new QLabel(tr(" Skin Thickness (current units):"), this );
   lbl_zeno_surface_thickness->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_zeno_surface_thickness->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_zeno_surface_thickness->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_zeno_surface_thickness = new QLineEdit( this, "Zeno_Skin_Thickness Line Edit");
   le_zeno_surface_thickness->setText(str.sprintf("%f",(*hydro).zeno_surface_thickness));
   le_zeno_surface_thickness->setAlignment(Qt::AlignVCenter);
   le_zeno_surface_thickness->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_zeno_surface_thickness->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_zeno_surface_thickness, SIGNAL(textChanged(const QString &)), SLOT(update_zeno_surface_thickness(const QString &)));

   pb_cancel = new QPushButton(tr("Close"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   int rows = 0, columns = 3, spacing = 2, j = 0, margin = 4;
   QGridLayout *background=new QGridLayout(this, rows, columns, margin, spacing);

   background->addMultiCellWidget(lbl_info, j, j, 0, 2);
   j++;

   background->addWidget( cb_zeno_zeno       , j, 0 );
   background->addWidget( lbl_zeno_zeno_steps, j, 1 );
   background->addWidget( le_zeno_zeno_steps , j, 2 );
   j++;

   background->addWidget( cb_zeno_interior       , j, 0 );
   background->addWidget( lbl_zeno_interior_steps, j, 1 );
   background->addWidget( le_zeno_interior_steps , j, 2 );
   j++;

   background->addWidget( cb_zeno_surface       , j, 0 );
   background->addWidget( lbl_zeno_surface_steps, j, 1 );
   background->addWidget( le_zeno_surface_steps , j, 2 );
   j++;

   background->addWidget( lbl_zeno_surface_thickness, j, 1 );
   background->addWidget( le_zeno_surface_thickness , j, 2 );
   j++;

   QBoxLayout *hbl_help_cancel = new QHBoxLayout( 0 );

   hbl_help_cancel->addWidget( pb_help );
   hbl_help_cancel->addWidget( pb_cancel );
   background->addMultiCellLayout( hbl_help_cancel, j, j, 0, 2 );
}

void US_Hydrodyn_Hydro_Zeno::set_zeno_zeno()
{
   (*hydro).zeno_zeno = cb_zeno_zeno->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   update_enables();
}

void US_Hydrodyn_Hydro_Zeno::update_zeno_zeno_steps(const QString &str)
{
   (*hydro).zeno_zeno_steps = str.toUInt();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}


void US_Hydrodyn_Hydro_Zeno::set_zeno_interior()
{
   (*hydro).zeno_interior = cb_zeno_interior->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   update_enables();
}

void US_Hydrodyn_Hydro_Zeno::update_zeno_interior_steps(const QString &str)
{
   (*hydro).zeno_interior_steps = str.toUInt();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Hydro_Zeno::set_zeno_surface()
{
   (*hydro).zeno_surface = cb_zeno_surface->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   update_enables();
}

void US_Hydrodyn_Hydro_Zeno::update_zeno_surface_steps(const QString &str)
{
   (*hydro).zeno_surface_steps = str.toUInt();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Hydro_Zeno::update_zeno_surface_thickness(const QString &str)
{
   (*hydro).zeno_surface_thickness = str.toFloat();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Hydro_Zeno::cancel()
{
   close();
}

void US_Hydrodyn_Hydro_Zeno::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_hydro_zeno.html");
}

void US_Hydrodyn_Hydro_Zeno::closeEvent(QCloseEvent *e)
{
   *hydro_zeno_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Hydro_Zeno:: update_enables()
{
   le_zeno_zeno_steps    ->setEnabled( cb_zeno_zeno    ->isChecked() );
   le_zeno_surface_steps ->setEnabled( cb_zeno_surface ->isChecked() );
   le_zeno_interior_steps->setEnabled( cb_zeno_interior->isChecked() );
}
