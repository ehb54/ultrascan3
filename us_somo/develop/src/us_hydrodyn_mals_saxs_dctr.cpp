#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_mals_saxs_dctr.h"
//Added by qt3to4:
#include <QGridLayout>
#include <QTextStream>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QCloseEvent>

US_Hydrodyn_Mals_Saxs_Dctr::US_Hydrodyn_Mals_Saxs_Dctr(
                                                       void                     *              us_hydrodyn_mals_saxs,
                                                       map < QString, QString > *              parameters,
                                                       QWidget *                               p,
                                                       const char *                            
                                                       ) : QDialog( p )
{
   this->us_hydrodyn_mals_saxs                = us_hydrodyn_mals_saxs;
   this->parameters                           = parameters;

   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle( us_tr( "US-SOMO: MALS_SAXS : Concentration Detector" ) );

   setupGUI();

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry( global_Xpos, global_Ypos, 0, 0 );
}

US_Hydrodyn_Mals_Saxs_Dctr::~US_Hydrodyn_Mals_Saxs_Dctr()
{
}

void US_Hydrodyn_Mals_Saxs_Dctr::setupGUI()
{
   int minHeight1  = 30;

   lbl_title =  new QLabel      ( us_tr( "US-SOMO: MALS_SAXS : Concentration Detector" ), this );
   lbl_title -> setAlignment    ( Qt::AlignCenter | Qt::AlignVCenter );
   lbl_title -> setMinimumHeight( minHeight1 );
   lbl_title -> setPalette( PALET_LABEL );
   AUTFBACK( lbl_title );
   lbl_title -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold ) );

   lbl_type = new QLabel( us_tr( "Detector type : " ), this );
   lbl_type->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_type->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_type );
   lbl_type->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));

   cb_uv = new QCheckBox(this);
   cb_uv->setText( "UV  " );
   cb_uv->setEnabled( true );
   connect( cb_uv, SIGNAL( clicked() ), SLOT( set_uv() ) );
   cb_uv->setChecked( parameters->count( "uv" ) );
   cb_uv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_uv->setPalette( PALET_NORMAL );
   AUTFBACK( cb_uv );
   cb_uv -> setMinimumHeight( minHeight1 );

   lbl_uv_conv = new QLabel( us_tr( "Calibration constant : " ), this );
   lbl_uv_conv->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_uv_conv->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_uv_conv );
   lbl_uv_conv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   lbl_uv_conv-> setMinimumHeight( minHeight1 );
   
   le_uv_conv = new QLineEdit( this );    le_uv_conv->setObjectName( "le_uv_conv Line Edit" );
   le_uv_conv->setText( (*parameters)[ "uv_conv" ] );
   le_uv_conv->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_uv_conv->setPalette( PALET_NORMAL );
   AUTFBACK( le_uv_conv );
   le_uv_conv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( le_uv_conv );
      qdv->setDecimals( 8 );
      le_uv_conv->setValidator( qdv );
   }
   connect( le_uv_conv, SIGNAL( textChanged( const QString & ) ), SLOT( uv_conv_text( const QString & ) ) );
   le_uv_conv->setMinimumWidth( 200 );
   le_uv_conv-> setMinimumHeight( minHeight1 );

   cb_ri = new QCheckBox(this);
   cb_ri->setText( "RI  " );
   cb_ri->setEnabled( true );
   connect( cb_ri, SIGNAL( clicked() ), SLOT( set_ri() ) );
   cb_ri->setChecked( parameters->count( "ri" ) );
   cb_ri->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_ri->setPalette( PALET_NORMAL );
   AUTFBACK( cb_ri );
   cb_ri-> setMinimumHeight( minHeight1 );

   lbl_ri_conv = new QLabel( us_tr( "Calibration constant : " ), this );
   lbl_ri_conv->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_ri_conv->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_ri_conv );
   lbl_ri_conv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   lbl_ri_conv-> setMinimumHeight( minHeight1 );

   le_ri_conv = new QLineEdit( this );    le_ri_conv->setObjectName( "le_ri_conv Line Edit" );
   le_ri_conv->setText( (*parameters)[ "ri_conv" ] );
   le_ri_conv->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_ri_conv->setPalette( PALET_NORMAL );
   AUTFBACK( le_ri_conv );
   le_ri_conv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( le_ri_conv );
      qdv->setDecimals( 8 );
      le_ri_conv->setValidator( qdv );
   }
   connect( le_ri_conv, SIGNAL( textChanged( const QString & ) ), SLOT( ri_conv_text( const QString & ) ) );
   le_ri_conv->setMinimumWidth( 200 );
   le_ri_conv-> setMinimumHeight( minHeight1 );

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

   pb_save =  new QPushButton ( us_tr( "Save" ), this );
   pb_save -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_save -> setMinimumHeight( minHeight1 );
   pb_save -> setPalette      ( PALET_PUSHB );
   connect( pb_save, SIGNAL( clicked() ), SLOT( save() ) );

   pb_keep =  new QPushButton ( us_tr( "Keep" ), this );
   pb_keep -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_keep -> setMinimumHeight( minHeight1 );
   pb_keep -> setPalette      ( PALET_PUSHB );
   connect( pb_keep, SIGNAL( clicked() ), SLOT( keep() ) );

   QVBoxLayout * background = new QVBoxLayout( this ); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 );
   background->addSpacing(4);

   background->addWidget( lbl_title );
   background->addSpacing( 2 );
   background->addWidget( lbl_type );

   QGridLayout * gbl = new QGridLayout( 0 ); gbl->setContentsMargins( 0, 0, 0, 0 ); gbl->setSpacing( 0 );

   gbl->addWidget( cb_uv, 0, 0 );
   gbl->addWidget( lbl_uv_conv, 0, 1 );
   gbl->addWidget( le_uv_conv, 0, 2 );

   gbl->addWidget( cb_ri, 1, 0 );
   gbl->addWidget( lbl_ri_conv, 1, 1 );
   gbl->addWidget( le_ri_conv, 1, 2 );

   background->addLayout( gbl );

   QHBoxLayout * hbl_bottom = new QHBoxLayout(); hbl_bottom->setContentsMargins( 0, 0, 0, 0 ); hbl_bottom->setSpacing( 0 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addWidget ( pb_quit );
   hbl_bottom->addWidget ( pb_save );
   hbl_bottom->addWidget ( pb_keep );

   background->addLayout ( hbl_bottom );
}

void US_Hydrodyn_Mals_Saxs_Dctr::quit()
{
   close();
}

void US_Hydrodyn_Mals_Saxs_Dctr::save()
{

   QString use_dir = 
      USglobal->config_list.root_dir + QDir::separator() + "etc" + 
      QDir::separator() + "somo_mals_saxs_default_dctr.dat" ;

   QString fn = QFileDialog::getSaveFileName( this , us_tr( "Select a name to save the detector settings" ) , use_dir , "*_dctr.dat" );


   if ( fn.isEmpty() )
   {
      return;
   }

   if ( fn.isEmpty() )
   {
      return;
   }

   fn.replace( QRegExp( "(|_dctr)\\.(dat|DAT)$" ), "" );
   fn += "_dctr.dat";

   QFile f( fn );
   if ( !f.open( QIODevice::WriteOnly ) )
   {
      return;
   }

   QTextStream ts( &f );

   ts << "# US-SOMO Detector State file\n";

   if ( !le_uv_conv->text().isEmpty() )
   {
      ts << "# __detector_uv: " << le_uv_conv->text() << Qt::endl;
   }
   if ( !le_ri_conv->text().isEmpty() )
   {
      ts << "# __detector_ri: " << le_ri_conv->text() << Qt::endl;
   }
   if ( cb_uv->isChecked() )
   {
      ts << "# __detector_uv_set" << Qt::endl;
   } 
   if ( cb_ri->isChecked() )
   {
      ts << "# __detector_ri_set" << Qt::endl;
   } 

   f.close();

   // save to disk
   keep();
}

void US_Hydrodyn_Mals_Saxs_Dctr::keep()
{
   (*parameters)[ "keep" ] = "set";
   close();
}

void US_Hydrodyn_Mals_Saxs_Dctr::help()
{
   US_Help *online_help;
   online_help = new US_Help( this );
   online_help->show_help("manual/somo/mals_saxs_dctr.html");
}

void US_Hydrodyn_Mals_Saxs_Dctr::closeEvent( QCloseEvent *e )
{

   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Mals_Saxs_Dctr::set_uv()
{
   (*parameters)[ "uv" ] = cb_uv->isChecked() ? "true" : "false";
   if ( cb_uv->isChecked() )
   {
      cb_ri->setChecked( false );
      parameters->erase( "ri" );
   }
}

void US_Hydrodyn_Mals_Saxs_Dctr::set_ri()
{
   (*parameters)[ "ri" ] = cb_ri->isChecked() ? "true" : "false";
   if ( cb_ri->isChecked() )
   {
      cb_uv->setChecked( false );
      parameters->erase( "uv" );
   }
}

void US_Hydrodyn_Mals_Saxs_Dctr::uv_conv_text( const QString & text )
{
   (*parameters)[ "uv_conv" ] = text;
}

void US_Hydrodyn_Mals_Saxs_Dctr::ri_conv_text( const QString & text )
{
   (*parameters)[ "ri_conv" ] = text;
}
