#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_mals_parameters.h"
//Added by qt3to4:
#include <QGridLayout>
#include <QTextStream>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QCloseEvent>

US_Hydrodyn_Mals_Parameters::US_Hydrodyn_Mals_Parameters(
                                                         void                     *              us_hydrodyn,
                                                         map < QString, QString > *              parameters,
                                                         QWidget *                               p,
                                                         const char *                            
                                                         ) : QDialog( p )
{
   this->us_hydrodyn_mals                     = ((US_Hydrodyn *)us_hydrodyn)->mals_window;
   this->parameters                           = parameters;

   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle( us_tr( "US-SOMO: MALS : MALS Processing Parameters" ) );

   setupGUI();

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry( global_Xpos, global_Ypos, 0, 0 );
}

US_Hydrodyn_Mals_Parameters::~US_Hydrodyn_Mals_Parameters()
{
}

void US_Hydrodyn_Mals_Parameters::setupGUI()
{
   int minHeight1  = 30;

   lbl_title =  new QLabel      ( us_tr( "US-SOMO: MALS : MALS Processing Parameters" ), this );
   lbl_title -> setAlignment    ( Qt::AlignCenter | Qt::AlignVCenter );
   lbl_title -> setMinimumHeight( minHeight1 );
   lbl_title -> setPalette( PALET_LABEL );
   AUTFBACK( lbl_title );
   lbl_title -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold ) );

   lbl_mals_param_lambda = new QLabel( us_tr( "Lambda [nm] : " ), this );
   lbl_mals_param_lambda->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_mals_param_lambda->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_mals_param_lambda );
   lbl_mals_param_lambda->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   lbl_mals_param_lambda-> setMinimumHeight( minHeight1 );
   
   le_mals_param_lambda = new QLineEdit( this );    le_mals_param_lambda->setObjectName( "le_mals_param_lambda Line Edit" );
   le_mals_param_lambda->setText( (*parameters)[ "mals_param_lambda" ] );
   le_mals_param_lambda->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_mals_param_lambda->setPalette( PALET_NORMAL );
   AUTFBACK( le_mals_param_lambda );
   le_mals_param_lambda->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( 400, 600, 1, le_mals_param_lambda );
      le_mals_param_lambda->setValidator( qdv );
   }
   connect( le_mals_param_lambda, SIGNAL( textChanged( const QString & ) ), SLOT( set_mals_param_lambda( const QString & ) ) );
   le_mals_param_lambda->setMinimumWidth( 200 );
   le_mals_param_lambda-> setMinimumHeight( minHeight1 );

   if ( (( US_Hydrodyn_Mals *)us_hydrodyn_mals )->lb_files->count() > 0 ) {
      le_mals_param_lambda->setEnabled( false );
      le_mals_param_lambda->setToolTip( us_tr( "Files are loaded so this value can not be adjusted" ) );
      lbl_mals_param_lambda->setToolTip( us_tr( "Files are loaded so this value can not be adjusted" ) );
   }

   lbl_mals_param_n = new QLabel( us_tr( "Solvent refractive index : " ), this );
   lbl_mals_param_n->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_mals_param_n->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_mals_param_n );
   lbl_mals_param_n->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   lbl_mals_param_n-> setMinimumHeight( minHeight1 );
   
   le_mals_param_n = new QLineEdit( this );    le_mals_param_n->setObjectName( "le_mals_param_n Line Edit" );
   le_mals_param_n->setText( (*parameters)[ "mals_param_n" ] );
   le_mals_param_n->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_mals_param_n->setPalette( PALET_NORMAL );
   AUTFBACK( le_mals_param_n );
   le_mals_param_n->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( 0, 5, 4, le_mals_param_n );
      le_mals_param_n->setValidator( qdv );
   }
   connect( le_mals_param_n, SIGNAL( textChanged( const QString & ) ), SLOT( set_mals_param_n( const QString & ) ) );
   le_mals_param_n->setMinimumWidth( 200 );
   le_mals_param_n-> setMinimumHeight( minHeight1 );

   if ( (( US_Hydrodyn_Mals *)us_hydrodyn_mals )->lb_files->count() > 0 ) {
      le_mals_param_n->setEnabled( false );
      le_mals_param_n->setToolTip( us_tr( "Files are loaded so this value can not be adjusted" ) );
      lbl_mals_param_n->setToolTip( us_tr( "Files are loaded so this value can not be adjusted" ) );
   }

   lbl_mals_param_g_dndc = new QLabel( us_tr( "Global dn/dc [ml/g] : " ), this );
   lbl_mals_param_g_dndc->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_mals_param_g_dndc->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_mals_param_g_dndc );
   lbl_mals_param_g_dndc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   lbl_mals_param_g_dndc-> setMinimumHeight( minHeight1 );
   
   le_mals_param_g_dndc = new QLineEdit( this );    le_mals_param_g_dndc->setObjectName( "le_mals_param_g_dndc Line Edit" );
   le_mals_param_g_dndc->setText( (*parameters)[ "mals_param_g_dndc" ] );
   le_mals_param_g_dndc->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_mals_param_g_dndc->setPalette( PALET_NORMAL );
   AUTFBACK( le_mals_param_g_dndc );
   le_mals_param_g_dndc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( 0, 1, 5, le_mals_param_g_dndc );
      le_mals_param_g_dndc->setValidator( qdv );
   }
   connect( le_mals_param_g_dndc, SIGNAL( textChanged( const QString & ) ), SLOT( set_mals_param_g_dndc( const QString & ) ) );
   le_mals_param_g_dndc->setMinimumWidth( 200 );
   le_mals_param_g_dndc-> setMinimumHeight( minHeight1 );

   lbl_mals_param_g_extinction_coef = new QLabel( us_tr( "Global extinction coefficient [mL mg^-1 cm^-1]: " ), this );
   lbl_mals_param_g_extinction_coef->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_mals_param_g_extinction_coef->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_mals_param_g_extinction_coef );
   lbl_mals_param_g_extinction_coef->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   lbl_mals_param_g_extinction_coef-> setMinimumHeight( minHeight1 );
   
   le_mals_param_g_extinction_coef = new QLineEdit( this );    le_mals_param_g_extinction_coef->setObjectName( "le_mals_param_g_extinction_coef Line Edit" );
   le_mals_param_g_extinction_coef->setText( (*parameters)[ "mals_param_g_extinction_coef" ] );
   le_mals_param_g_extinction_coef->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_mals_param_g_extinction_coef->setPalette( PALET_NORMAL );
   AUTFBACK( le_mals_param_g_extinction_coef );
   le_mals_param_g_extinction_coef->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( 0, 1, 5, le_mals_param_g_extinction_coef );
      le_mals_param_g_extinction_coef->setValidator( qdv );
   }
   connect( le_mals_param_g_extinction_coef, SIGNAL( textChanged( const QString & ) ), SLOT( set_mals_param_g_extinction_coef( const QString & ) ) );
   le_mals_param_g_extinction_coef->setMinimumWidth( 200 );
   le_mals_param_g_extinction_coef-> setMinimumHeight( minHeight1 );


   lbl_mals_param_g_conc = new QLabel( us_tr( "Global concentration [mg/mL]: " ), this );
   lbl_mals_param_g_conc->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_mals_param_g_conc->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_mals_param_g_conc );
   lbl_mals_param_g_conc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   lbl_mals_param_g_conc-> setMinimumHeight( minHeight1 );
   
   le_mals_param_g_conc = new QLineEdit( this );    le_mals_param_g_conc->setObjectName( "le_mals_param_g_conc Line Edit" );
   le_mals_param_g_conc->setText( (*parameters)[ "mals_param_g_conc" ] );
   le_mals_param_g_conc->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_mals_param_g_conc->setPalette( PALET_NORMAL );
   AUTFBACK( le_mals_param_g_conc );
   le_mals_param_g_conc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( 0, 9, 5, le_mals_param_g_conc );
      le_mals_param_g_conc->setValidator( qdv );
   }
   connect( le_mals_param_g_conc, SIGNAL( textChanged( const QString & ) ), SLOT( set_mals_param_g_conc( const QString & ) ) );
   le_mals_param_g_conc->setMinimumWidth( 200 );
   le_mals_param_g_conc-> setMinimumHeight( minHeight1 );


   lbl_mals_param_DLS_detector = new QLabel( us_tr( "DLS detector number : " ), this );
   lbl_mals_param_DLS_detector->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_mals_param_DLS_detector->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_mals_param_DLS_detector );
   lbl_mals_param_DLS_detector->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   lbl_mals_param_DLS_detector-> setMinimumHeight( minHeight1 );
   
   le_mals_param_DLS_detector = new QLineEdit( this );    le_mals_param_DLS_detector->setObjectName( "le_mals_param_DLS_detector Line Edit" );
   le_mals_param_DLS_detector->setText( (*parameters)[ "mals_param_DLS_detector" ] );
   le_mals_param_DLS_detector->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_mals_param_DLS_detector->setPalette( PALET_NORMAL );
   AUTFBACK( le_mals_param_DLS_detector );
   le_mals_param_DLS_detector->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QIntValidator *qdv = new QIntValidator( 1, 99, le_mals_param_DLS_detector );
      le_mals_param_DLS_detector->setValidator( qdv );
   }
   connect( le_mals_param_DLS_detector, SIGNAL( textChanged( const QString & ) ), SLOT( set_mals_param_DLS_detector( const QString & ) ) );
   le_mals_param_DLS_detector->setMinimumWidth( 200 );
   le_mals_param_DLS_detector->setMinimumHeight( minHeight1 );
   
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

   QGridLayout * gbl = new QGridLayout( 0 ); gbl->setContentsMargins( 0, 0, 0, 0 ); gbl->setSpacing( 0 );

   {
      int row = 0;
      
      gbl->addWidget( lbl_mals_param_lambda, row, 0 );
      gbl->addWidget( le_mals_param_lambda,  row, 1 );
      ++row;
      
      gbl->addWidget( lbl_mals_param_n, row, 0 );
      gbl->addWidget( le_mals_param_n,  row, 1 );
      ++row;
      
      gbl->addWidget( lbl_mals_param_g_dndc, row, 0 );
      gbl->addWidget( le_mals_param_g_dndc,  row, 1 );
      ++row;
      
      gbl->addWidget( lbl_mals_param_g_extinction_coef, row, 0 );
      gbl->addWidget( le_mals_param_g_extinction_coef,  row, 1 );
      ++row;
      
      gbl->addWidget( lbl_mals_param_g_conc, row, 0 );
      gbl->addWidget( le_mals_param_g_conc,  row, 1 );
      ++row;
      
      gbl->addWidget( lbl_mals_param_DLS_detector, row, 0 );
      gbl->addWidget( le_mals_param_DLS_detector,  row, 1 );
      ++row;
      
   }

   background->addLayout( gbl );

   QHBoxLayout * hbl_bottom = new QHBoxLayout(); hbl_bottom->setContentsMargins( 0, 0, 0, 0 ); hbl_bottom->setSpacing( 0 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addWidget ( pb_quit );
   hbl_bottom->addWidget ( pb_save );
   hbl_bottom->addWidget ( pb_keep );

   background->addLayout ( hbl_bottom );
}

void US_Hydrodyn_Mals_Parameters::quit()
{
   close();
}

void US_Hydrodyn_Mals_Parameters::save()
{

   QString use_dir = 
      USglobal->config_list.root_dir + QDir::separator() + "etc" + 
      QDir::separator() + "somo_mals_default_mals_param.dat" ;

   QString fn = QFileDialog::getSaveFileName( this , us_tr( "Select a name to save the MALS parameters" ) , use_dir , "*_mals_param.dat" );

   if ( fn.isEmpty() )
   {
      return;
   }

   if ( fn.isEmpty() )
   {
      return;
   }

   fn.replace( QRegExp( "(|_mals_param)\\.(dat|DAT)$" ), "" );
   fn += "_mals_param.dat";

   QFile f( fn );
   if ( !f.open( QIODevice::WriteOnly ) )
   {
      return;
   }

   QTextStream ts( &f );

   ts << "# US-SOMO MALS parameter file\n";

   if ( !le_mals_param_lambda->text().isEmpty() )
   {
      ts << "# __mals_param_lambda: " << le_mals_param_lambda->text() << Qt::endl;
   }

   if ( !le_mals_param_n->text().isEmpty() )
   {
      ts << "# __mals_param_n: " << le_mals_param_n->text() << Qt::endl;
   }

   if ( !le_mals_param_g_dndc->text().isEmpty() )
   {
      ts << "# __mals_param_g_dndc: " << le_mals_param_g_dndc->text() << Qt::endl;
   }
   if ( !le_mals_param_g_extinction_coef->text().isEmpty() )
   {
      ts << "# __mals_param_g_extinction_coef: " << le_mals_param_g_extinction_coef->text() << Qt::endl;
   }

   if ( !le_mals_param_g_conc->text().isEmpty() )
   {
      ts << "# __mals_param_g_conc: " << le_mals_param_g_conc->text() << Qt::endl;
   }

   if ( !le_mals_param_DLS_detector->text().isEmpty() )
   {
      ts << "# __mals_param_DLS_detector: " << le_mals_param_DLS_detector->text() << Qt::endl;
   }
   
   f.close();

   // save to disk
   keep();
}

void US_Hydrodyn_Mals_Parameters::keep()
{
   (*parameters)[ "keep" ] = "set";
   close();
}

void US_Hydrodyn_Mals_Parameters::help()
{
   US_Help *online_help;
   online_help = new US_Help( this );
   online_help->show_help("manual/somo/mals_parameters.html");
}

void US_Hydrodyn_Mals_Parameters::closeEvent( QCloseEvent *e )
{

   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Mals_Parameters::set_mals_param_lambda( const QString & text )
{
   (*parameters)[ "mals_param_lambda" ] = text;
}

void US_Hydrodyn_Mals_Parameters::set_mals_param_n( const QString & text )
{
   (*parameters)[ "mals_param_n" ] = text;
}

void US_Hydrodyn_Mals_Parameters::set_mals_param_g_dndc( const QString & text )
{
   (*parameters)[ "mals_param_g_dndc" ] = text;
}

void US_Hydrodyn_Mals_Parameters::set_mals_param_g_extinction_coef( const QString & text )
{
   (*parameters)[ "mals_param_g_extinction_coef" ] = text;
}

void US_Hydrodyn_Mals_Parameters::set_mals_param_g_conc( const QString & text )
{
   (*parameters)[ "mals_param_g_conc" ] = text;
}

void US_Hydrodyn_Mals_Parameters::set_mals_param_DLS_detector( const QString & text )
{
   (*parameters)[ "mals_param_DLS_detector" ] = text;
}

