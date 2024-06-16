#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_dad_parameters.h"
//Added by qt3to4:
#include <QGridLayout>
#include <QTextStream>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QCloseEvent>

US_Hydrodyn_Dad_Parameters::US_Hydrodyn_Dad_Parameters(
                                                         void                     *              us_hydrodyn,
                                                         map < QString, QString > *              parameters,
                                                         QWidget *                               p,
                                                         const char *                            
                                                         ) : QDialog( p )
{
   this->us_hydrodyn_dad                     = ((US_Hydrodyn *)us_hydrodyn)->dad_window;
   this->parameters                           = parameters;

   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle( us_tr( "US-SOMO: UV-Vis : UV-Vis Processing Parameters" ) );

   setupGUI();

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry( global_Xpos, global_Ypos, 0, 0 );
}

US_Hydrodyn_Dad_Parameters::~US_Hydrodyn_Dad_Parameters()
{
}

void US_Hydrodyn_Dad_Parameters::setupGUI()
{
   int minHeight1  = 30;

   lbl_title =  new QLabel      ( us_tr( "US-SOMO: UV-Vis : UV-Vis Processing Parameters" ), this );
   lbl_title -> setAlignment    ( Qt::AlignCenter | Qt::AlignVCenter );
   lbl_title -> setMinimumHeight( minHeight1 );
   lbl_title -> setPalette( PALET_LABEL );
   AUTFBACK( lbl_title );
   lbl_title -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold ) );

   lbl_dad_param_lambda = new QLabel( us_tr( "Lambda [nm] : " ), this );
   lbl_dad_param_lambda->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_dad_param_lambda->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_dad_param_lambda );
   lbl_dad_param_lambda->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   lbl_dad_param_lambda-> setMinimumHeight( minHeight1 );
   
   le_dad_param_lambda = new QLineEdit( this );    le_dad_param_lambda->setObjectName( "le_dad_param_lambda Line Edit" );
   le_dad_param_lambda->setText( (*parameters)[ "dad_param_lambda" ] );
   le_dad_param_lambda->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_dad_param_lambda->setPalette( PALET_NORMAL );
   AUTFBACK( le_dad_param_lambda );
   le_dad_param_lambda->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( 400, 600, 1, le_dad_param_lambda );
      le_dad_param_lambda->setValidator( qdv );
   }
   connect( le_dad_param_lambda, SIGNAL( textChanged( const QString & ) ), SLOT( set_dad_param_lambda( const QString & ) ) );
   le_dad_param_lambda->setMinimumWidth( 200 );
   le_dad_param_lambda-> setMinimumHeight( minHeight1 );

#warning might need to bring disable lambda change back

   // if ( (( US_Hydrodyn_Dad *)us_hydrodyn_dad )->lb_files->count() > 0 ) {
   //    le_dad_param_lambda->setEnabled( false );
   //    le_dad_param_lambda->setToolTip( us_tr( "Files are loaded so this value can not be adjusted" ) );
   //    lbl_dad_param_lambda->setToolTip( us_tr( "Files are loaded so this value can not be adjusted" ) );
   // }

   lbl_dad_param_n = new QLabel( us_tr( "Solvent refractive index : " ), this );
   lbl_dad_param_n->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_dad_param_n->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_dad_param_n );
   lbl_dad_param_n->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   lbl_dad_param_n-> setMinimumHeight( minHeight1 );
   
   le_dad_param_n = new QLineEdit( this );    le_dad_param_n->setObjectName( "le_dad_param_n Line Edit" );
   le_dad_param_n->setText( (*parameters)[ "dad_param_n" ] );
   le_dad_param_n->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_dad_param_n->setPalette( PALET_NORMAL );
   AUTFBACK( le_dad_param_n );
   le_dad_param_n->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( 0, 5, 4, le_dad_param_n );
      le_dad_param_n->setValidator( qdv );
   }
   connect( le_dad_param_n, SIGNAL( textChanged( const QString & ) ), SLOT( set_dad_param_n( const QString & ) ) );
   le_dad_param_n->setMinimumWidth( 200 );
   le_dad_param_n-> setMinimumHeight( minHeight1 );

   if ( (( US_Hydrodyn_Dad *)us_hydrodyn_dad )->lb_files->count() > 0 ) {
      le_dad_param_n->setEnabled( false );
      le_dad_param_n->setToolTip( us_tr( "Files are loaded so this value can not be adjusted" ) );
      lbl_dad_param_n->setToolTip( us_tr( "Files are loaded so this value can not be adjusted" ) );
   }

   lbl_dad_param_g_dndc = new QLabel( us_tr( "Global dn/dc [ml/g] : " ), this );
   lbl_dad_param_g_dndc->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_dad_param_g_dndc->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_dad_param_g_dndc );
   lbl_dad_param_g_dndc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   lbl_dad_param_g_dndc-> setMinimumHeight( minHeight1 );
   
   le_dad_param_g_dndc = new QLineEdit( this );    le_dad_param_g_dndc->setObjectName( "le_dad_param_g_dndc Line Edit" );
   le_dad_param_g_dndc->setText( (*parameters)[ "dad_param_g_dndc" ] );
   le_dad_param_g_dndc->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_dad_param_g_dndc->setPalette( PALET_NORMAL );
   AUTFBACK( le_dad_param_g_dndc );
   le_dad_param_g_dndc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( 0, 1, 5, le_dad_param_g_dndc );
      le_dad_param_g_dndc->setValidator( qdv );
   }
   connect( le_dad_param_g_dndc, SIGNAL( textChanged( const QString & ) ), SLOT( set_dad_param_g_dndc( const QString & ) ) );
   le_dad_param_g_dndc->setMinimumWidth( 200 );
   le_dad_param_g_dndc-> setMinimumHeight( minHeight1 );

   lbl_dad_param_g_extinction_coef = new QLabel( us_tr( "Extinction coefficient [ml mg^-1 cm^-1]: " ), this );
   lbl_dad_param_g_extinction_coef->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_dad_param_g_extinction_coef->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_dad_param_g_extinction_coef );
   lbl_dad_param_g_extinction_coef->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   lbl_dad_param_g_extinction_coef-> setMinimumHeight( minHeight1 );
   
   le_dad_param_g_extinction_coef = new QLineEdit( this );    le_dad_param_g_extinction_coef->setObjectName( "le_dad_param_g_extinction_coef Line Edit" );
   le_dad_param_g_extinction_coef->setText( (*parameters)[ "dad_param_g_extinction_coef" ] );
   le_dad_param_g_extinction_coef->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_dad_param_g_extinction_coef->setPalette( PALET_NORMAL );
   AUTFBACK( le_dad_param_g_extinction_coef );
   le_dad_param_g_extinction_coef->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( 0, 1, 5, le_dad_param_g_extinction_coef );
      le_dad_param_g_extinction_coef->setValidator( qdv );
   }
   connect( le_dad_param_g_extinction_coef, SIGNAL( textChanged( const QString & ) ), SLOT( set_dad_param_g_extinction_coef( const QString & ) ) );
   le_dad_param_g_extinction_coef->setMinimumWidth( 200 );
   le_dad_param_g_extinction_coef-> setMinimumHeight( minHeight1 );


   lbl_dad_param_g_conc = new QLabel( us_tr( "Global concentration [mg/mL]: " ), this );
   lbl_dad_param_g_conc->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_dad_param_g_conc->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_dad_param_g_conc );
   lbl_dad_param_g_conc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   lbl_dad_param_g_conc-> setMinimumHeight( minHeight1 );
   
   le_dad_param_g_conc = new QLineEdit( this );    le_dad_param_g_conc->setObjectName( "le_dad_param_g_conc Line Edit" );
   le_dad_param_g_conc->setText( (*parameters)[ "dad_param_g_conc" ] );
   le_dad_param_g_conc->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_dad_param_g_conc->setPalette( PALET_NORMAL );
   AUTFBACK( le_dad_param_g_conc );
   le_dad_param_g_conc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( 0, 9, 5, le_dad_param_g_conc );
      le_dad_param_g_conc->setValidator( qdv );
   }
   connect( le_dad_param_g_conc, SIGNAL( textChanged( const QString & ) ), SLOT( set_dad_param_g_conc( const QString & ) ) );
   le_dad_param_g_conc->setMinimumWidth( 200 );
   le_dad_param_g_conc-> setMinimumHeight( minHeight1 );


   lbl_dad_param_DLS_detector = new QLabel( us_tr( "DLS detector number : " ), this );
   lbl_dad_param_DLS_detector->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_dad_param_DLS_detector->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_dad_param_DLS_detector );
   lbl_dad_param_DLS_detector->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   lbl_dad_param_DLS_detector-> setMinimumHeight( minHeight1 );
   
   le_dad_param_DLS_detector = new QLineEdit( this );    le_dad_param_DLS_detector->setObjectName( "le_dad_param_DLS_detector Line Edit" );
   le_dad_param_DLS_detector->setText( (*parameters)[ "dad_param_DLS_detector" ] );
   le_dad_param_DLS_detector->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_dad_param_DLS_detector->setPalette( PALET_NORMAL );
   AUTFBACK( le_dad_param_DLS_detector );
   le_dad_param_DLS_detector->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QIntValidator *qdv = new QIntValidator( 1, 99, le_dad_param_DLS_detector );
      le_dad_param_DLS_detector->setValidator( qdv );
   }
   connect( le_dad_param_DLS_detector, SIGNAL( textChanged( const QString & ) ), SLOT( set_dad_param_DLS_detector( const QString & ) ) );
   le_dad_param_DLS_detector->setMinimumWidth( 200 );
   le_dad_param_DLS_detector->setMinimumHeight( minHeight1 );
   
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
      
      gbl->addWidget( lbl_dad_param_lambda, row, 0 );
      gbl->addWidget( le_dad_param_lambda,  row, 1 );
      ++row;
      
      gbl->addWidget( lbl_dad_param_n, row, 0 );
      gbl->addWidget( le_dad_param_n,  row, 1 );
      ++row;
      
      gbl->addWidget( lbl_dad_param_g_dndc, row, 0 );
      gbl->addWidget( le_dad_param_g_dndc,  row, 1 );
      ++row;
      
      gbl->addWidget( lbl_dad_param_g_extinction_coef, row, 0 );
      gbl->addWidget( le_dad_param_g_extinction_coef,  row, 1 );
      ++row;
      
      gbl->addWidget( lbl_dad_param_g_conc, row, 0 );
      gbl->addWidget( le_dad_param_g_conc,  row, 1 );
      ++row;
      
      gbl->addWidget( lbl_dad_param_DLS_detector, row, 0 );
      gbl->addWidget( le_dad_param_DLS_detector,  row, 1 );
      ++row;
      
   }

   background->addLayout( gbl );

   QHBoxLayout * hbl_bottom = new QHBoxLayout(); hbl_bottom->setContentsMargins( 0, 0, 0, 0 ); hbl_bottom->setSpacing( 0 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addWidget ( pb_quit );
   hbl_bottom->addWidget ( pb_save );
   hbl_bottom->addWidget ( pb_keep );

   background->addLayout ( hbl_bottom );

   always_hide_widgets.insert(
                              {
                                 lbl_dad_param_n
                                    ,le_dad_param_n
                                    ,lbl_dad_param_g_dndc
                                    ,le_dad_param_g_dndc
                                    ,lbl_dad_param_g_conc
                                    ,le_dad_param_g_conc
                                    ,lbl_dad_param_DLS_detector
                                    ,le_dad_param_DLS_detector
                                    }
                              );
   
   ShowHide::hide_widgets( always_hide_widgets );

}

void US_Hydrodyn_Dad_Parameters::quit()
{
   close();
}

void US_Hydrodyn_Dad_Parameters::save()
{

   QString use_dir = 
      USglobal->config_list.root_dir + QDir::separator() + "etc" + 
      QDir::separator() + "somo_uv_vis_default_uv_vis_param.dat" ;

   QString fn = QFileDialog::getSaveFileName( this , us_tr( "Select a name to save the UV-Vis parameters" ) , use_dir , "*_uv_vis_param.dat" );

   if ( fn.isEmpty() )
   {
      return;
   }

   if ( fn.isEmpty() )
   {
      return;
   }

   fn = fn.replace( QRegExp( "(|_uv_vis_param)\\.(dat|DAT)$" ), "" );
   fn += "_uv_vis_param.dat";

   QFile f( fn );
   if ( !f.open( QIODevice::WriteOnly ) )
   {
      return;
   }

   QTextStream ts( &f );

   ts << "# US-SOMO UV-Vis parameter file\n";

   if ( !le_dad_param_lambda->text().isEmpty() )
   {
      ts << "# __uv_vis_param_lambda: " << le_dad_param_lambda->text() << Qt::endl;
   }

   if ( !le_dad_param_n->text().isEmpty() )
   {
      ts << "# __uv_vis_param_n: " << le_dad_param_n->text() << Qt::endl;
   }

   if ( !le_dad_param_g_dndc->text().isEmpty() )
   {
      ts << "# __uv_vis_param_g_dndc: " << le_dad_param_g_dndc->text() << Qt::endl;
   }
   if ( !le_dad_param_g_extinction_coef->text().isEmpty() )
   {
      ts << "# __uv_vis_param_g_extinction_coef: " << le_dad_param_g_extinction_coef->text() << Qt::endl;
   }

   if ( !le_dad_param_g_conc->text().isEmpty() )
   {
      ts << "# __uv_vis_param_g_conc: " << le_dad_param_g_conc->text() << Qt::endl;
   }

   if ( !le_dad_param_DLS_detector->text().isEmpty() )
   {
      ts << "# __uv_vis_param_DLS_detector: " << le_dad_param_DLS_detector->text() << Qt::endl;
   }
   
   f.close();

   // save to disk
   keep();
}

void US_Hydrodyn_Dad_Parameters::keep()
{
   (*parameters)[ "keep" ] = "set";
   close();
}

void US_Hydrodyn_Dad_Parameters::help()
{
   US_Help *online_help;
   online_help = new US_Help( this );
   online_help->show_help("manual/somo/uv_vis_options.html");
}

void US_Hydrodyn_Dad_Parameters::closeEvent( QCloseEvent *e )
{

   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Dad_Parameters::set_dad_param_lambda( const QString & text )
{
   (*parameters)[ "dad_param_lambda" ] = text;
}

void US_Hydrodyn_Dad_Parameters::set_dad_param_n( const QString & text )
{
   (*parameters)[ "dad_param_n" ] = text;
}

void US_Hydrodyn_Dad_Parameters::set_dad_param_g_dndc( const QString & text )
{
   (*parameters)[ "dad_param_g_dndc" ] = text;
}

void US_Hydrodyn_Dad_Parameters::set_dad_param_g_extinction_coef( const QString & text )
{
   (*parameters)[ "dad_param_g_extinction_coef" ] = text;
}

void US_Hydrodyn_Dad_Parameters::set_dad_param_g_conc( const QString & text )
{
   (*parameters)[ "dad_param_g_conc" ] = text;
}

void US_Hydrodyn_Dad_Parameters::set_dad_param_DLS_detector( const QString & text )
{
   (*parameters)[ "dad_param_DLS_detector" ] = text;
}

