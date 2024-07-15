#include "../include/us_hydrodyn_saxs_hplc_bl.h"
//Added by qt3to4:
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QCloseEvent>

US_Hydrodyn_Saxs_Hplc_Bl::US_Hydrodyn_Saxs_Hplc_Bl(
                                                     void                     *              us_hydrodyn_saxs_hplc,
                                                     map < QString, QString > *              parameters,
                                                     QWidget *                               p,
                                                     const char *                            
                                                     ) : QDialog( p )
{
   this->us_hydrodyn_saxs_hplc                = us_hydrodyn_saxs_hplc;
   this->parameters                           = parameters;

   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle( us_tr( "US-SOMO: HPLC/KIN : Subtract Baseline" ) );

   setupGUI();
   update_enables();

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry( global_Xpos, global_Ypos, 0, 0 );
}

US_Hydrodyn_Saxs_Hplc_Bl::~US_Hydrodyn_Saxs_Hplc_Bl()
{
}

void US_Hydrodyn_Saxs_Hplc_Bl::setupGUI()
{
   int minHeight1  = 30;

   lbl_title =  new QLabel      ( windowTitle(), this );
   lbl_title -> setAlignment    ( Qt::AlignCenter | Qt::AlignVCenter );
   lbl_title -> setMinimumHeight( minHeight1 );
   lbl_title -> setPalette( PALET_LABEL );
   lbl_title -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold ) );

   rb_linear = new QRadioButton( us_tr("Linear baseline removal"), this);
   rb_linear->setEnabled(true);
   rb_linear->setChecked(true);
   rb_linear->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_linear->setPalette( PALET_NORMAL );
   connect( rb_linear, SIGNAL( clicked() ), SLOT( update_enables() ) );

   rb_integral = new QRadioButton( us_tr("Integral of I(t) baseline removal"), this);
   rb_integral->setEnabled(true);
   rb_integral->setChecked(false);
   rb_integral->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_integral->setPalette( PALET_NORMAL );
   connect( rb_integral, SIGNAL( clicked() ), SLOT( update_enables() ) );

#if 1 // QT_VERSION < 0x040000
   bg_bl_type = new QGroupBox( this );
   int bg_pos = 0;
   bg_bl_type->setExclusive(true);
   bg_bl_type->addButton( rb_linear, bg_pos++ );
   bg_bl_type->addButton( rb_integral, bg_pos++ );
   // connect( bg_bl_type, SIGNAL( buttonClicked( int id ) ), SLOT( update_enables() ) );
#else
   bg_bl_type = new QGroupBox();
   bg_bl_type->setFlat( true );

   {
      QHBoxLayout * bl = new QHBoxLayout; bl->setContentsMargins( 0, 0, 0, 0 ); bl->setSpacing( 0 );
      bl->addWidget( rb_linear );
      bl->addWidget( rb_integral );
      bg_bl_type->setLayout( bl );
   }
#endif

   cb_save_bl = new QCheckBox(this);
   cb_save_bl->setText( us_tr( "Produce separate baseline curves " ) );
   cb_save_bl->setEnabled( true );
   cb_save_bl->setChecked( false );
   cb_save_bl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_save_bl->setPalette( PALET_NORMAL );

   lbl_smooth =  new QLabel      ( us_tr( "Smoothing:" ), this );
   lbl_smooth -> setAlignment    ( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_smooth -> setPalette( PALET_LABEL );
   lbl_smooth -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   le_smooth = new QLineEdit( this );    le_smooth->setObjectName( "le_smooth Line Edit" );
   le_smooth->setText( "10" );
   le_smooth->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_smooth->setPalette( PALET_NORMAL );
   le_smooth->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QIntValidator *qdv = new QIntValidator( 0, 99, le_smooth );
      le_smooth->setValidator( qdv );
   }
   connect( le_smooth, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_smooth->setMinimumWidth( 60 );

   lbl_reps =  new QLabel      ( us_tr( "Maximum iterations:" ), this );
   lbl_reps -> setAlignment    ( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_reps -> setPalette( PALET_LABEL );
   lbl_reps -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   le_reps = new QLineEdit( this );    le_reps->setObjectName( "le_reps Line Edit" );
   le_reps->setText( "1" );
   le_reps->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_reps->setPalette( PALET_NORMAL );
   le_reps->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QIntValidator *qdv = new QIntValidator( 1, 20, le_reps );
      le_reps->setValidator( qdv );
   }
   connect( le_reps, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_reps->setMinimumWidth( 60 );

   pb_help =  new QPushButton ( us_tr( "Help" ), this );
   pb_help -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_help -> setMinimumHeight( minHeight1 );
   pb_help -> setPalette      ( PALET_PUSHB );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );

   pb_quit =  new QPushButton ( us_tr( "Quit" ), this );
   pb_quit -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_quit -> setMinimumHeight( minHeight1 );
   pb_quit -> setPalette      ( PALET_PUSHB );
   connect( pb_quit, SIGNAL( clicked() ), SLOT( quit() ) );

   pb_go =  new QPushButton ( us_tr( "Subtract baselines" ), this );
   pb_go -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_go -> setMinimumHeight( minHeight1 );
   pb_go -> setPalette      ( PALET_PUSHB );
   connect( pb_go, SIGNAL( clicked() ), SLOT( go() ) );

   QVBoxLayout * background = new QVBoxLayout( this ); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 );
   background->addWidget( lbl_title );
   background->addWidget( rb_linear );
   background->addWidget( rb_integral );

   QGridLayout * gl_bl = new QGridLayout( 0 ); gl_bl->setContentsMargins( 0, 0, 0, 0 ); gl_bl->setSpacing( 0 );

   gl_bl->addWidget         ( lbl_smooth , 0, 0 );
   gl_bl->addWidget         ( le_smooth  , 0, 1 );
   gl_bl->addWidget         ( lbl_reps   , 1, 0 );
   gl_bl->addWidget         ( le_reps    , 1, 1 );

   background->addLayout( gl_bl );
   background->addWidget( cb_save_bl );

   QHBoxLayout * hbl_bottom = new QHBoxLayout(); hbl_bottom->setContentsMargins( 0, 0, 0, 0 ); hbl_bottom->setSpacing( 0 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addWidget ( pb_quit );
   hbl_bottom->addWidget ( pb_go );

   background->addLayout ( hbl_bottom );

}

void US_Hydrodyn_Saxs_Hplc_Bl::quit()
{
   close();
}

void US_Hydrodyn_Saxs_Hplc_Bl::go()
{
   (*parameters)[ "go" ] = "true";
   if ( rb_linear->isChecked() )
   {
      (*parameters)[ "linear" ] = "true";
   } else {
      (*parameters)[ "integral" ] = "true";
      (*parameters)[ "smooth"   ] = le_smooth->text();
      (*parameters)[ "reps"     ] = le_reps->text();
   }
   if ( cb_save_bl->isChecked() )
   {
      (*parameters)[ "save_bl" ] = "true";
   }
   close();
}

void US_Hydrodyn_Saxs_Hplc_Bl::help()
{
   US_Help *online_help;
   online_help = new US_Help( this );
   online_help->show_help("manual/somo/saxs_hplc_bl.html");
}

void US_Hydrodyn_Saxs_Hplc_Bl::closeEvent( QCloseEvent *e )
{

   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Saxs_Hplc_Bl::update_enables()
{
   le_smooth ->setEnabled( rb_integral->isChecked() );
   le_reps   ->setEnabled( rb_integral->isChecked() );
   cb_save_bl->setEnabled( rb_integral->isChecked() );
}
