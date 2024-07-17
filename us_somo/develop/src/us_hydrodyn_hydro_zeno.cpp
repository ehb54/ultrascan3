#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_hydro_zeno.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_math.h"
//Added by qt3to4:
#include <QHBoxLayout>
#include <QCloseEvent>
#include <QBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QLabel>

US_Hydrodyn_Hydro_Zeno::US_Hydrodyn_Hydro_Zeno(struct hydro_options *hydro,
                                               bool *hydro_zeno_widget, 
                                               void *us_hydrodyn, 
                                               QWidget *p, 
                                               const char *) : QFrame( p )
{
   this->hydro = hydro;
   this->hydro_zeno_widget = hydro_zeno_widget;
   this->us_hydrodyn = us_hydrodyn;
   *hydro_zeno_widget = true;
   USglobal=new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle(us_tr("SOMO  Hydrodynamic Calculation Zeno Options"));
   this->setMinimumWidth(1000);
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

   lbl_info = new QLabel(us_tr("SOMO Hydrodynamic Calculation Zeno Options:"), this);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette( PALET_FRAME );
   AUTFBACK( lbl_info );
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   cb_zeno_zeno = new QCheckBox( this );
   cb_zeno_zeno->setText( us_tr( "Compute Zeno:") );
   cb_zeno_zeno->setEnabled( true );
   cb_zeno_zeno->setChecked( hydro->zeno_zeno );
   cb_zeno_zeno->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_zeno_zeno->setPalette( PALET_NORMAL );
   AUTFBACK( cb_zeno_zeno );
   connect( cb_zeno_zeno, SIGNAL( clicked() ), this, SLOT( set_zeno_zeno() ) );

   lbl_zeno_zeno_steps = new QLabel(us_tr(" Zeno Steps (Thousands):"), this );
   lbl_zeno_zeno_steps->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_zeno_zeno_steps->setPalette( PALET_LABEL );
   AUTFBACK( lbl_zeno_zeno_steps );
   lbl_zeno_zeno_steps->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_zeno_zeno_steps = new QLineEdit(  this );    le_zeno_zeno_steps->setObjectName( "Zeno_Zeno_Steps Line Edit" );
   le_zeno_zeno_steps->setText(str.sprintf("%u",(*hydro).zeno_zeno_steps));
   le_zeno_zeno_steps->setAlignment(Qt::AlignVCenter);
   le_zeno_zeno_steps->setPalette( PALET_NORMAL );
   AUTFBACK( le_zeno_zeno_steps );
   le_zeno_zeno_steps->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_zeno_zeno_steps, SIGNAL(textChanged(const QString &)), SLOT(update_zeno_zeno_steps(const QString &)));

   cb_zeno_max_cap = new QCheckBox( this );
   cb_zeno_max_cap->setText( us_tr( "Early termination:") );
   cb_zeno_max_cap->setEnabled( true );
   cb_zeno_max_cap->setChecked( ((US_Hydrodyn *)us_hydrodyn)->gparams.count( "zeno_max_cap" ) &&
                                ((US_Hydrodyn *)us_hydrodyn)->gparams[ "zeno_max_cap" ] == "true"  );
   cb_zeno_max_cap->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_zeno_max_cap->setPalette( PALET_NORMAL );
   AUTFBACK( cb_zeno_max_cap );
   connect( cb_zeno_max_cap, SIGNAL( clicked() ), this, SLOT( set_zeno_max_cap() ) );

   lbl_zeno_max_cap = new QLabel(us_tr(" Stokes Radius S.D./Mean in percent:"), this );
   lbl_zeno_max_cap->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_zeno_max_cap->setPalette( PALET_LABEL );
   AUTFBACK( lbl_zeno_max_cap );
   lbl_zeno_max_cap->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_zeno_max_cap = new QLineEdit(  this );    le_zeno_max_cap->setObjectName( "Zeno_Max_Cap Line Edit" );
   le_zeno_max_cap->setText(((US_Hydrodyn *)us_hydrodyn)->gparams[ "zeno_max_cap_pct" ] );
   le_zeno_max_cap->setAlignment(Qt::AlignVCenter);
   le_zeno_max_cap->setPalette( PALET_NORMAL );
   AUTFBACK( le_zeno_max_cap );
   le_zeno_max_cap->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   {
      QDoubleValidator *qdv = new QDoubleValidator( 0.01, 10, 2, le_zeno_max_cap );
      le_zeno_max_cap->setValidator( qdv );
   }
   connect(le_zeno_max_cap, SIGNAL(textChanged(const QString &)), SLOT(update_zeno_max_cap(const QString &)));

   cb_zeno_interior = new QCheckBox( this );
   cb_zeno_interior->setText( us_tr( "Compute Interior:") );
   cb_zeno_interior->setEnabled( true );
   cb_zeno_interior->setChecked( hydro->zeno_interior );
   cb_zeno_interior->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_zeno_interior->setPalette( PALET_NORMAL );
   AUTFBACK( cb_zeno_interior );
   connect( cb_zeno_interior, SIGNAL( clicked() ), this, SLOT( set_zeno_interior() ) );

   lbl_zeno_interior_steps = new QLabel(us_tr(" Interior Steps (Thousands):"), this );
   lbl_zeno_interior_steps->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_zeno_interior_steps->setPalette( PALET_LABEL );
   AUTFBACK( lbl_zeno_interior_steps );
   lbl_zeno_interior_steps->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_zeno_interior_steps = new QLineEdit(  this );    le_zeno_interior_steps->setObjectName( "Zeno_Interior_Steps Line Edit" );
   le_zeno_interior_steps->setText(str.sprintf("%u",(*hydro).zeno_interior_steps));
   le_zeno_interior_steps->setAlignment(Qt::AlignVCenter);
   le_zeno_interior_steps->setPalette( PALET_NORMAL );
   AUTFBACK( le_zeno_interior_steps );
   le_zeno_interior_steps->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_zeno_interior_steps, SIGNAL(textChanged(const QString &)), SLOT(update_zeno_interior_steps(const QString &)));

   cb_zeno_surface = new QCheckBox( this );
   cb_zeno_surface->setText( us_tr( "Compute Surface:") );
   cb_zeno_surface->setEnabled( true );
   cb_zeno_surface->setChecked( hydro->zeno_surface );
   cb_zeno_surface->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_zeno_surface->setPalette( PALET_NORMAL );
   AUTFBACK( cb_zeno_surface );
   connect( cb_zeno_surface, SIGNAL( clicked() ), this, SLOT( set_zeno_surface() ) );

   lbl_zeno_surface_steps = new QLabel(us_tr(" Surface Steps (Thousands):"), this );
   lbl_zeno_surface_steps->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_zeno_surface_steps->setPalette( PALET_LABEL );
   AUTFBACK( lbl_zeno_surface_steps );
   lbl_zeno_surface_steps->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_zeno_surface_steps = new QLineEdit(  this );    le_zeno_surface_steps->setObjectName( "Zeno_Surface_Steps Line Edit" );
   le_zeno_surface_steps->setText(str.sprintf("%u",(*hydro).zeno_surface_steps));
   le_zeno_surface_steps->setAlignment(Qt::AlignVCenter);
   le_zeno_surface_steps->setPalette( PALET_NORMAL );
   AUTFBACK( le_zeno_surface_steps );
   le_zeno_surface_steps->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_zeno_surface_steps, SIGNAL(textChanged(const QString &)), SLOT(update_zeno_surface_steps(const QString &)));

   lbl_zeno_surface_thickness = new QLabel(us_tr(" Skin Thickness (current units):"), this );
   lbl_zeno_surface_thickness->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_zeno_surface_thickness->setPalette( PALET_LABEL );
   AUTFBACK( lbl_zeno_surface_thickness );
   lbl_zeno_surface_thickness->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_zeno_surface_thickness = new QLineEdit(  this );    le_zeno_surface_thickness->setObjectName( "Zeno_Skin_Thickness Line Edit" );
   le_zeno_surface_thickness->setText(str.sprintf("%.3f",(*hydro).zeno_surface_thickness));
   le_zeno_surface_thickness->setAlignment(Qt::AlignVCenter);
   le_zeno_surface_thickness->setPalette( PALET_NORMAL );
   AUTFBACK( le_zeno_surface_thickness );
   le_zeno_surface_thickness->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_zeno_surface_thickness, SIGNAL(textChanged(const QString &)), SLOT(update_zeno_surface_thickness(const QString &)));

   cb_zeno_surface_thickness_from_rg = new QCheckBox( this );
   cb_zeno_surface_thickness_from_rg->setText( us_tr( "Compute Skin Thickness from sigmoid fit of Rg-dependence:") );
   cb_zeno_surface_thickness_from_rg->setEnabled( true );
   cb_zeno_surface_thickness_from_rg->setChecked( (*hydro).zeno_surface_thickness_from_rg );
   cb_zeno_surface_thickness_from_rg->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_zeno_surface_thickness_from_rg->setPalette( PALET_NORMAL );
   AUTFBACK( cb_zeno_surface_thickness_from_rg );
   connect( cb_zeno_surface_thickness_from_rg, SIGNAL( clicked() ), this, SLOT( set_zeno_surface_thickness_from_rg() ) );

   lbl_zeno_surface_thickness_from_rg_a = new QLabel(us_tr("Skin Thickness from sigmoid fit of Rg-dep. coeff. a:"), this );
   lbl_zeno_surface_thickness_from_rg_a->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_zeno_surface_thickness_from_rg_a->setPalette( PALET_LABEL );
   AUTFBACK( lbl_zeno_surface_thickness_from_rg_a );
   lbl_zeno_surface_thickness_from_rg_a->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_zeno_surface_thickness_from_rg_a = new QLineEdit(  this );    le_zeno_surface_thickness_from_rg_a->setObjectName( "Zeno_Skin_Thickness_From_Rg_A Line Edit" );
   le_zeno_surface_thickness_from_rg_a->setText(str.sprintf("%.4f",(*hydro).zeno_surface_thickness_from_rg_a));
   le_zeno_surface_thickness_from_rg_a->setAlignment(Qt::AlignVCenter);
   le_zeno_surface_thickness_from_rg_a->setPalette( PALET_NORMAL );
   AUTFBACK( le_zeno_surface_thickness_from_rg_a );
   le_zeno_surface_thickness_from_rg_a->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_zeno_surface_thickness_from_rg_a, SIGNAL(textChanged(const QString &)), SLOT(update_zeno_surface_thickness_from_rg_a(const QString &)));

   lbl_zeno_surface_thickness_from_rg_b = new QLabel(us_tr("Skin Thickness from sigmoid fit of Rg-dep. coeff. b:"), this );
   lbl_zeno_surface_thickness_from_rg_b->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_zeno_surface_thickness_from_rg_b->setPalette( PALET_LABEL );
   AUTFBACK( lbl_zeno_surface_thickness_from_rg_b );
   lbl_zeno_surface_thickness_from_rg_b->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_zeno_surface_thickness_from_rg_b = new QLineEdit(  this );    le_zeno_surface_thickness_from_rg_b->setObjectName( "Zeno_Skin_Thickness_From_Rg_B Line Edit" );
   le_zeno_surface_thickness_from_rg_b->setText(str.sprintf("%.4f",(*hydro).zeno_surface_thickness_from_rg_b));
   le_zeno_surface_thickness_from_rg_b->setAlignment(Qt::AlignVCenter);
   le_zeno_surface_thickness_from_rg_b->setPalette( PALET_NORMAL );
   AUTFBACK( le_zeno_surface_thickness_from_rg_b );
   le_zeno_surface_thickness_from_rg_b->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_zeno_surface_thickness_from_rg_b, SIGNAL(textChanged(const QString &)), SLOT(update_zeno_surface_thickness_from_rg_b(const QString &)));

   lbl_zeno_surface_thickness_from_rg_c = new QLabel(us_tr("Skin Thickness from sigmoid fit of Rg-dep. coeff. c:"), this );
   lbl_zeno_surface_thickness_from_rg_c->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_zeno_surface_thickness_from_rg_c->setPalette( PALET_LABEL );
   AUTFBACK( lbl_zeno_surface_thickness_from_rg_c );
   lbl_zeno_surface_thickness_from_rg_c->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_zeno_surface_thickness_from_rg_c = new QLineEdit(  this );    le_zeno_surface_thickness_from_rg_c->setObjectName( "Zeno_Skin_Thickness_From_Rg_C Line Edit" );
   le_zeno_surface_thickness_from_rg_c->setText(str.sprintf("%.4f",(*hydro).zeno_surface_thickness_from_rg_c));
   le_zeno_surface_thickness_from_rg_c->setAlignment(Qt::AlignVCenter);
   le_zeno_surface_thickness_from_rg_c->setPalette( PALET_NORMAL );
   AUTFBACK( le_zeno_surface_thickness_from_rg_c );
   le_zeno_surface_thickness_from_rg_c->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_zeno_surface_thickness_from_rg_c, SIGNAL(textChanged(const QString &)), SLOT(update_zeno_surface_thickness_from_rg_c(const QString &)));

   cb_zeno_cxx = new QCheckBox( this );
   cb_zeno_cxx->setText( us_tr( "Test experimental new Zeno version") );
   cb_zeno_cxx->setEnabled( true );
   cb_zeno_cxx->setChecked( ((US_Hydrodyn *)us_hydrodyn)->gparams.count( "zeno_cxx" ) &&
                            ((US_Hydrodyn *)us_hydrodyn)->gparams[ "zeno_cxx" ] == "true" );
   cb_zeno_cxx->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_zeno_cxx->setPalette( PALET_NORMAL );
   AUTFBACK( cb_zeno_cxx );
   connect( cb_zeno_cxx, SIGNAL( clicked() ), this, SLOT( set_zeno_cxx() ) );
   if ( !U_EXPT ) {
      cb_zeno_cxx->hide();
   }

   lbl_zeno_repeats = new QLabel(us_tr(" Zeno repetitions:"), this );
   lbl_zeno_repeats->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_zeno_repeats->setPalette( PALET_LABEL );
   AUTFBACK( lbl_zeno_repeats );
   lbl_zeno_repeats->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_zeno_repeats = new QLineEdit(  this );    le_zeno_repeats->setObjectName( "Zeno_Repeats Line Edit" );
   le_zeno_repeats->setText( ((US_Hydrodyn *)us_hydrodyn)->gparams["zeno_repeats" ] );
   le_zeno_repeats->setAlignment(Qt::AlignVCenter);
   le_zeno_repeats->setPalette( PALET_NORMAL );
   AUTFBACK( le_zeno_repeats );
   le_zeno_repeats->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_zeno_repeats, SIGNAL(textChanged(const QString &)), SLOT(update_zeno_repeats(const QString &)));

   pb_cancel = new QPushButton(us_tr("Close"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_help = new QPushButton(us_tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   int /* rows = 0, columns = 3, */ spacing = 2, j = 0, margin = 4;
   QGridLayout * background = new QGridLayout( this ); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 ); background->setSpacing( spacing ); background->setContentsMargins( margin, margin, margin, margin );

   background->addWidget( lbl_info , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 2 ) - ( 0 ) );
   j++;

   background->addWidget( cb_zeno_zeno       , j, 0 );
   background->addWidget( lbl_zeno_zeno_steps, j, 1 );
   background->addWidget( le_zeno_zeno_steps , j, 2 );
   j++;

   background->addWidget( cb_zeno_max_cap     , j, 0 );
   background->addWidget( lbl_zeno_max_cap    , j, 1 );
   background->addWidget( le_zeno_max_cap     , j, 2 );
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

   background->addWidget( cb_zeno_surface_thickness_from_rg, j, 0 );
   background->addWidget( lbl_zeno_surface_thickness_from_rg_a, j, 1 );
   background->addWidget( le_zeno_surface_thickness_from_rg_a , j, 2 );
   j++;

   background->addWidget( lbl_zeno_surface_thickness_from_rg_b, j, 1 );
   background->addWidget( le_zeno_surface_thickness_from_rg_b , j, 2 );
   j++;

   background->addWidget( lbl_zeno_surface_thickness_from_rg_c, j, 1 );
   background->addWidget( le_zeno_surface_thickness_from_rg_c , j, 2 );
   j++;

   background->addWidget( cb_zeno_cxx , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 2  ) - ( 0 ) );
   j++;

   background->addWidget( lbl_zeno_repeats, j, 1 );
   background->addWidget( le_zeno_repeats , j, 2 );
   j++;

   QBoxLayout * hbl_help_cancel = new QHBoxLayout(); hbl_help_cancel->setContentsMargins( 0, 0, 0, 0 ); hbl_help_cancel->setSpacing( 0 );

   hbl_help_cancel->addWidget( pb_help );
   hbl_help_cancel->addWidget( pb_cancel );
   background->addLayout( hbl_help_cancel , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 2  ) - ( 0 ) );

#if !defined(USE_OLD_ZENO) && QT_VERSION >= 0x040000
   cb_zeno_interior->hide();
   lbl_zeno_interior_steps->hide();
   le_zeno_interior_steps->hide();
   cb_zeno_surface->hide();
   lbl_zeno_surface_steps->hide();
   le_zeno_surface_steps->hide();
   cb_zeno_cxx->hide();
   cb_zeno_zeno->hide();
   (*hydro).zeno_zeno = true;
#endif
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

void US_Hydrodyn_Hydro_Zeno::set_zeno_max_cap()
{
   ((US_Hydrodyn *)us_hydrodyn)->gparams[ "zeno_max_cap" ] = ( cb_zeno_max_cap->isChecked() ? "true" : "false" );
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   update_enables();
}

void US_Hydrodyn_Hydro_Zeno::update_zeno_max_cap(const QString &str)
{
   double val = str.toDouble();
   // bool changed = false;
   // if ( val >= 25 ) {
   //    val = 25;
   //    changed = true;
   // }
   // if ( val < 0.01 ) {
   //    val = 0.01;
   //    changed = true;
   // }
   
   // if ( changed ) {
   //    le_zeno_max_cap->setText( QString( "%1" ).arg( val ) );
   // }

   ((US_Hydrodyn *)us_hydrodyn)->gparams[ "zeno_max_cap_pct" ] = QString( "%1" ).arg( val );
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Hydro_Zeno::set_zeno_interior()
{
   (*hydro).zeno_interior = cb_zeno_interior->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( cb_zeno_cxx->isChecked() ) {
      cb_zeno_cxx->setChecked( false );
   }
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
   if ( cb_zeno_cxx->isChecked() ) {
      cb_zeno_cxx->setChecked( false );
   }
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

void US_Hydrodyn_Hydro_Zeno::set_zeno_surface_thickness_from_rg()
{
   (*hydro).zeno_surface_thickness_from_rg = cb_zeno_surface_thickness_from_rg->isChecked();
   update_enables();
}

void US_Hydrodyn_Hydro_Zeno::update_zeno_surface_thickness_from_rg_a(const QString &str)
{
   (*hydro).zeno_surface_thickness_from_rg_a = str.toFloat();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Hydro_Zeno::update_zeno_surface_thickness_from_rg_b(const QString &str)
{
   (*hydro).zeno_surface_thickness_from_rg_b = str.toFloat();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Hydro_Zeno::update_zeno_surface_thickness_from_rg_c(const QString &str)
{
   (*hydro).zeno_surface_thickness_from_rg_c = str.toFloat();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Hydro_Zeno::update_zeno_repeats(const QString &str)
{
   ((US_Hydrodyn *)us_hydrodyn)->gparams[ "zeno_repeats" ] = QString( "%1" ).arg( str.toUInt() );
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Hydro_Zeno::set_zeno_cxx()
{
   ((US_Hydrodyn *)us_hydrodyn)->gparams[ "zeno_cxx" ] = cb_zeno_cxx->isChecked() ? "true" : "false";
   // should set surface & interior off ?
   if ( cb_zeno_cxx->isChecked() ) {
      cb_zeno_interior->setChecked( false );
      cb_zeno_surface ->setChecked( false );
   }      
   update_enables();
}

void US_Hydrodyn_Hydro_Zeno::cancel()
{
   close();
}

void US_Hydrodyn_Hydro_Zeno::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo/somo_hydro_zeno.html");
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
   le_zeno_max_cap       ->setEnabled( cb_zeno_max_cap ->isChecked() );
   le_zeno_surface_steps ->setEnabled( cb_zeno_surface ->isChecked() );
   le_zeno_interior_steps->setEnabled( cb_zeno_interior->isChecked() );

   le_zeno_surface_thickness          ->setEnabled( !cb_zeno_surface_thickness_from_rg->isChecked() );
   le_zeno_surface_thickness_from_rg_a->setEnabled( cb_zeno_surface_thickness_from_rg->isChecked() );
   le_zeno_surface_thickness_from_rg_b->setEnabled( cb_zeno_surface_thickness_from_rg->isChecked() );
   le_zeno_surface_thickness_from_rg_c->setEnabled( cb_zeno_surface_thickness_from_rg->isChecked() );
}
