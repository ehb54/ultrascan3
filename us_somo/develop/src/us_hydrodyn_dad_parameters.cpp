#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_dad_parameters.h"
#include "../include/us_unicode.h"

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
   this->us_hydrodyn                         = us_hydrodyn;
   this->us_hydrodyn_dad                     = ((US_Hydrodyn *)us_hydrodyn)->dad_window;
   this->parameters                          = parameters;

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

   lbl_dad_param_desc = new QLabel( us_tr( "Description : " ), this );
   lbl_dad_param_desc->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_dad_param_desc->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_dad_param_desc );
   lbl_dad_param_desc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   lbl_dad_param_desc-> setMinimumHeight( minHeight1 );
   
   le_dad_param_desc = new QLineEdit( this );    le_dad_param_desc->setObjectName( "le_dad_param_desc Line Edit" );
   le_dad_param_desc->setText( (*parameters)[ "dad_param_desc" ] );
   le_dad_param_desc->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_dad_param_desc->setPalette( PALET_NORMAL );
   AUTFBACK( le_dad_param_desc );
   le_dad_param_desc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   connect( le_dad_param_desc, SIGNAL( textChanged( const QString & ) ), SLOT( set_dad_param_desc( const QString & ) ) );
   le_dad_param_desc->setMinimumWidth( 200 );
   le_dad_param_desc-> setMinimumHeight( minHeight1 );

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

   lbl_dad_param_g_extinction_coef = new QLabel( us_tr( "Extinction coefficient [mL mg^-1 cm^-1]: " ), this );
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
   
   lbl_lambda_dependence_coeff =  new QLabel      ( us_tr( UNICODE_LAMBDA_QS + "-dependence coefficients:" ), this );
   lbl_lambda_dependence_coeff -> setAlignment    ( Qt::AlignCenter | Qt::AlignVCenter );
   lbl_lambda_dependence_coeff -> setMinimumHeight( minHeight1 );
   lbl_lambda_dependence_coeff -> setPalette( PALET_LABEL );
   AUTFBACK( lbl_lambda_dependence_coeff );
   lbl_lambda_dependence_coeff -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold ) );

   lbl_dad_param_dndc2_a = new QLabel( us_tr( "[[dn/dc](" + UNICODE_LAMBDA_QS + ")]^0.5 coeff. A: " ), this );
   lbl_dad_param_dndc2_a->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_dad_param_dndc2_a->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_dad_param_dndc2_a );
   lbl_dad_param_dndc2_a->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   lbl_dad_param_dndc2_a-> setMinimumHeight( minHeight1 );
   
   le_dad_param_dndc2_a = new QLineEdit( this );    le_dad_param_dndc2_a->setObjectName( "le_dad_param_dndc2_a Line Edit" );
   le_dad_param_dndc2_a->setText( (*parameters)[ "dad_param_dndc2_a" ] );
   le_dad_param_dndc2_a->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_dad_param_dndc2_a->setPalette( PALET_NORMAL );
   AUTFBACK( le_dad_param_dndc2_a );
   le_dad_param_dndc2_a->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( 0, 9, 5, le_dad_param_dndc2_a );
      le_dad_param_dndc2_a->setValidator( qdv );
   }
   connect( le_dad_param_dndc2_a, SIGNAL( textChanged( const QString & ) ), SLOT( set_dad_param_dndc2_a( const QString & ) ) );
   le_dad_param_dndc2_a->setMinimumWidth( 200 );
   le_dad_param_dndc2_a-> setMinimumHeight( minHeight1 );

   lbl_dad_param_dndc2_b = new QLabel( us_tr( "[[dn/dc](" + UNICODE_LAMBDA_QS + ")]^0.5 coeff. B: " ), this );
   lbl_dad_param_dndc2_b->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_dad_param_dndc2_b->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_dad_param_dndc2_b );
   lbl_dad_param_dndc2_b->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   lbl_dad_param_dndc2_b-> setMinimumHeight( minHeight1 );
   
   le_dad_param_dndc2_b = new QLineEdit( this );    le_dad_param_dndc2_b->setObjectName( "le_dad_param_dndc2_b Line Edit" );
   le_dad_param_dndc2_b->setText( (*parameters)[ "dad_param_dndc2_b" ] );
   le_dad_param_dndc2_b->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_dad_param_dndc2_b->setPalette( PALET_NORMAL );
   AUTFBACK( le_dad_param_dndc2_b );
   le_dad_param_dndc2_b->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( 0, 9, 5, le_dad_param_dndc2_b );
      le_dad_param_dndc2_b->setValidator( qdv );
   }
   connect( le_dad_param_dndc2_b, SIGNAL( textChanged( const QString & ) ), SLOT( set_dad_param_dndc2_b( const QString & ) ) );
   le_dad_param_dndc2_b->setMinimumWidth( 200 );
   le_dad_param_dndc2_b-> setMinimumHeight( minHeight1 );

   lbl_dad_param_dndc2_c = new QLabel( us_tr( "[[dn/dc](" + UNICODE_LAMBDA_QS + ")]^0.5 coeff. C: " ), this );
   lbl_dad_param_dndc2_c->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_dad_param_dndc2_c->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_dad_param_dndc2_c );
   lbl_dad_param_dndc2_c->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   lbl_dad_param_dndc2_c-> setMinimumHeight( minHeight1 );
   
   le_dad_param_dndc2_c = new QLineEdit( this );    le_dad_param_dndc2_c->setObjectName( "le_dad_param_dndc2_c Line Edit" );
   le_dad_param_dndc2_c->setText( (*parameters)[ "dad_param_dndc2_c" ] );
   le_dad_param_dndc2_c->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_dad_param_dndc2_c->setPalette( PALET_NORMAL );
   AUTFBACK( le_dad_param_dndc2_c );
   le_dad_param_dndc2_c->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( 0, 9, 5, le_dad_param_dndc2_c );
      le_dad_param_dndc2_c->setValidator( qdv );
   }
   connect( le_dad_param_dndc2_c, SIGNAL( textChanged( const QString & ) ), SLOT( set_dad_param_dndc2_c( const QString & ) ) );
   le_dad_param_dndc2_c->setMinimumWidth( 200 );
   le_dad_param_dndc2_c-> setMinimumHeight( minHeight1 );

   lbl_dad_param_n2_a = new QLabel( us_tr( "Refractive index [n(" + UNICODE_LAMBDA_QS + ")]^0.5 coeff. A: " ), this );
   lbl_dad_param_n2_a->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_dad_param_n2_a->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_dad_param_n2_a );
   lbl_dad_param_n2_a->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   lbl_dad_param_n2_a-> setMinimumHeight( minHeight1 );
   
   le_dad_param_n2_a = new QLineEdit( this );    le_dad_param_n2_a->setObjectName( "le_dad_param_n2_a Line Edit" );
   le_dad_param_n2_a->setText( (*parameters)[ "dad_param_n2_a" ] );
   le_dad_param_n2_a->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_dad_param_n2_a->setPalette( PALET_NORMAL );
   AUTFBACK( le_dad_param_n2_a );
   le_dad_param_n2_a->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( 0, 9, 5, le_dad_param_n2_a );
      le_dad_param_n2_a->setValidator( qdv );
   }
   connect( le_dad_param_n2_a, SIGNAL( textChanged( const QString & ) ), SLOT( set_dad_param_n2_a( const QString & ) ) );
   le_dad_param_n2_a->setMinimumWidth( 200 );
   le_dad_param_n2_a-> setMinimumHeight( minHeight1 );

   lbl_dad_param_n2_b = new QLabel( us_tr( "Refractive index [n(" + UNICODE_LAMBDA_QS + ")]^0.5 coeff. B: " ), this );
   lbl_dad_param_n2_b->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_dad_param_n2_b->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_dad_param_n2_b );
   lbl_dad_param_n2_b->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   lbl_dad_param_n2_b-> setMinimumHeight( minHeight1 );
   
   le_dad_param_n2_b = new QLineEdit( this );    le_dad_param_n2_b->setObjectName( "le_dad_param_n2_b Line Edit" );
   le_dad_param_n2_b->setText( (*parameters)[ "dad_param_n2_b" ] );
   le_dad_param_n2_b->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_dad_param_n2_b->setPalette( PALET_NORMAL );
   AUTFBACK( le_dad_param_n2_b );
   le_dad_param_n2_b->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( 0, 9, 5, le_dad_param_n2_b );
      le_dad_param_n2_b->setValidator( qdv );
   }
   connect( le_dad_param_n2_b, SIGNAL( textChanged( const QString & ) ), SLOT( set_dad_param_n2_b( const QString & ) ) );
   le_dad_param_n2_b->setMinimumWidth( 200 );
   le_dad_param_n2_b-> setMinimumHeight( minHeight1 );

   lbl_dad_param_n2_c = new QLabel( us_tr( "Refractive index [n(" + UNICODE_LAMBDA_QS + ")]^0.5 coeff. C: " ), this );
   lbl_dad_param_n2_c->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_dad_param_n2_c->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_dad_param_n2_c );
   lbl_dad_param_n2_c->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   lbl_dad_param_n2_c-> setMinimumHeight( minHeight1 );
   
   le_dad_param_n2_c = new QLineEdit( this );    le_dad_param_n2_c->setObjectName( "le_dad_param_n2_c Line Edit" );
   le_dad_param_n2_c->setText( (*parameters)[ "dad_param_n2_c" ] );
   le_dad_param_n2_c->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_dad_param_n2_c->setPalette( PALET_NORMAL );
   AUTFBACK( le_dad_param_n2_c );
   le_dad_param_n2_c->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( 0, 9, 5, le_dad_param_n2_c );
      le_dad_param_n2_c->setValidator( qdv );
   }
   connect( le_dad_param_n2_c, SIGNAL( textChanged( const QString & ) ), SLOT( set_dad_param_n2_c( const QString & ) ) );
   le_dad_param_n2_c->setMinimumWidth( 200 );
   le_dad_param_n2_c-> setMinimumHeight( minHeight1 );

   
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

   pb_load =  new QPushButton ( us_tr( "Load" ), this );
   pb_load -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_load -> setMinimumHeight( minHeight1 );
   pb_load -> setPalette      ( PALET_PUSHB );
   connect( pb_load, SIGNAL( clicked() ), SLOT( load() ) );

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
      
      gbl->addWidget( lbl_dad_param_desc, row, 0 );
      gbl->addWidget( le_dad_param_desc,  row, 1 );
      ++row;
      
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
      
      gbl->addWidget( lbl_lambda_dependence_coeff, row, 0, 1, 2 );
      ++row;

      gbl->addWidget( lbl_dad_param_dndc2_a, row, 0 );
      gbl->addWidget( le_dad_param_dndc2_a,  row, 1 );
      ++row;

      gbl->addWidget( lbl_dad_param_dndc2_b, row, 0 );
      gbl->addWidget( le_dad_param_dndc2_b,  row, 1 );
      ++row;

      gbl->addWidget( lbl_dad_param_dndc2_c, row, 0 );
      gbl->addWidget( le_dad_param_dndc2_c,  row, 1 );
      ++row;
      
      gbl->addWidget( lbl_dad_param_n2_a, row, 0 );
      gbl->addWidget( le_dad_param_n2_a,  row, 1 );
      ++row;

      gbl->addWidget( lbl_dad_param_n2_b, row, 0 );
      gbl->addWidget( le_dad_param_n2_b,  row, 1 );
      ++row;

      gbl->addWidget( lbl_dad_param_n2_c, row, 0 );
      gbl->addWidget( le_dad_param_n2_c,  row, 1 );
      ++row;
   }

   background->addLayout( gbl );

   QHBoxLayout * hbl_bottom = new QHBoxLayout(); hbl_bottom->setContentsMargins( 0, 0, 0, 0 ); hbl_bottom->setSpacing( 0 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addWidget ( pb_quit );
   hbl_bottom->addWidget ( pb_load );
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

   if ( !le_dad_param_desc->text().isEmpty() )
   {
      ts << "# __uv_vis_param_desc: " << le_dad_param_desc->text() << Qt::endl;
   }

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

   if ( !le_dad_param_dndc2_a->text().isEmpty() )
   {
      ts << "# __uv_vis_param_dndc2_a: " << le_dad_param_dndc2_a->text() << Qt::endl;
   }
   
   if ( !le_dad_param_dndc2_b->text().isEmpty() )
   {
      ts << "# __uv_vis_param_dndc2_b: " << le_dad_param_dndc2_b->text() << Qt::endl;
   }

   if ( !le_dad_param_dndc2_c->text().isEmpty() )
   {
      ts << "# __uv_vis_param_dndc2_c: " << le_dad_param_dndc2_c->text() << Qt::endl;
   }

   if ( !le_dad_param_n2_a->text().isEmpty() )
   {
      ts << "# __uv_vis_param_n2_a: " << le_dad_param_n2_a->text() << Qt::endl;
   }
   
   if ( !le_dad_param_n2_b->text().isEmpty() )
   {
      ts << "# __uv_vis_param_n2_b: " << le_dad_param_n2_b->text() << Qt::endl;
   }

   if ( !le_dad_param_n2_c->text().isEmpty() )
   {
      ts << "# __uv_vis_param_n2_c: " << le_dad_param_n2_c->text() << Qt::endl;
   }

   f.close();

   ((US_Hydrodyn  *)us_hydrodyn)->add_to_directory_history( fn );

   // save to disk
   keep();
}

void US_Hydrodyn_Dad_Parameters::load()
{
   // alternatively, could use dad_win->load()
   // remove some code duplication, but would require friending class & saving & restoring params in case user did not "keep"
   // could also have some unintended side-effects - if they load a different file

   qDebug() << "US_Hydrodyn_Dad_Parameters::load()";

   QString use_dir = 
      USglobal->config_list.root_dir + QDir::separator() + "etc";

   QString filename = 
      QFileDialog::getOpenFileName(
                                   this
                                   , us_tr( "Load UV-Vis parameter file" )
                                   ,use_dir
                                   , "UV-Vis parameter files (*_uv_vis_param.dat);;"
                                   "All files (*);;"
                                   );
   
   if ( filename.isEmpty() ) {
      return;
   }

   QFile f( filename );

   QString errormsg;

   do {
      if ( !f.open( QIODevice::ReadOnly ) )
      {
         errormsg = QString("Error: can not open %1, check permissions ").arg( filename );
         break;
      }

      QTextStream ts(&f);
      vector < QString > qv;
      QStringList qsl;
      QStringList qsl_nb;

      while ( !ts.atEnd() )
      {
         QString qs = ts.readLine();
         qv.push_back( qs );

         qsl << qs;

         if ( !qs.contains( QRegularExpression( "^\\s*$" ) ) ) {
            qsl_nb << qs.trimmed();
         }
      }
      f.close();

      if ( !qv.size() )
      {
         errormsg = QString("Error: the file %1 is empty ").arg( filename );
         break;
      }

      if ( qv[ 0 ].contains( " UV-Vis parameter file" ) ) {

         QRegExp rx_dad_param_desc              ( "^# __uv_vis_param_desc: (.*)\\s*$" );
         QRegExp rx_dad_param_lambda            ( "^# __uv_vis_param_lambda: (\\S+)\\s*$" );
         QRegExp rx_dad_param_n                 ( "^# __uv_vis_param_n: (\\S+)\\s*$" );
         QRegExp rx_dad_param_g_dndc            ( "^# __uv_vis_param_g_dndc: (\\S+)\\s*$" );
         QRegExp rx_dad_param_g_extinction_coef ( "^# __uv_vis_param_g_extinction_coef: (\\S+)\\s*$" );
         QRegExp rx_dad_param_g_conc            ( "^# __uv_vis_param_g_conc: (\\S+)\\s*$" );
         QRegExp rx_dad_param_DLS_detector      ( "^# __uv_vis_param_DLS_detector: (\\S+)\\s*$" );
         QRegExp rx_dad_param_dndc2_a           ( "^# __uv_vis_param_dndc2_a: (\\S+)\\s*$" );
         QRegExp rx_dad_param_dndc2_b           ( "^# __uv_vis_param_dndc2_b: (\\S+)\\s*$" );
         QRegExp rx_dad_param_dndc2_c           ( "^# __uv_vis_param_dndc2_c: (\\S+)\\s*$" );
         QRegExp rx_dad_param_n2_a              ( "^# __uv_vis_param_n2_a: (\\S+)\\s*$" );
         QRegExp rx_dad_param_n2_b              ( "^# __uv_vis_param_n2_b: (\\S+)\\s*$" );
         QRegExp rx_dad_param_n2_c              ( "^# __uv_vis_param_n2_c: (\\S+)\\s*$" );

         for ( int i = 1; i < (int) qv.size(); i++ ) {

            if ( rx_dad_param_desc.indexIn( qv[ i ] ) != -1 ) {
               le_dad_param_desc->setText( rx_dad_param_desc.cap( 1 ) );
               continue;
            }

            if ( rx_dad_param_lambda.indexIn( qv[ i ] ) != -1 ) {
               le_dad_param_lambda->setText( rx_dad_param_lambda.cap( 1 ) );
               continue;
            }
            if ( rx_dad_param_n.indexIn( qv[ i ] ) != -1 ) {
               le_dad_param_n->setText( rx_dad_param_n.cap( 1 ) );
               continue;
            }
            if ( rx_dad_param_g_dndc.indexIn( qv[ i ] ) != -1 ) {
               le_dad_param_g_dndc->setText( rx_dad_param_g_dndc.cap( 1 ) );
               continue;
            }
            if ( rx_dad_param_g_extinction_coef.indexIn( qv[ i ] ) != -1 ) {
               le_dad_param_g_extinction_coef->setText( rx_dad_param_g_extinction_coef.cap( 1 ) );
               continue;
            }
            if ( rx_dad_param_g_conc.indexIn( qv[ i ] ) != -1 ) {
               le_dad_param_g_conc->setText( rx_dad_param_g_conc.cap( 1 ) );
               continue;
            }
            if ( rx_dad_param_DLS_detector.indexIn( qv[ i ] ) != -1 ) {
               le_dad_param_DLS_detector->setText( rx_dad_param_DLS_detector.cap( 1 ) );
               continue;
            }

            if ( rx_dad_param_dndc2_a.indexIn( qv[ i ] ) != -1 ) {
               le_dad_param_dndc2_a->setText( rx_dad_param_dndc2_a.cap( 1 ) );
               continue;
            }
            if ( rx_dad_param_dndc2_b.indexIn( qv[ i ] ) != -1 ) {
               le_dad_param_dndc2_b->setText( rx_dad_param_dndc2_b.cap( 1 ) );
               continue;
            }
            if ( rx_dad_param_dndc2_c.indexIn( qv[ i ] ) != -1 ) {
               le_dad_param_dndc2_c->setText( rx_dad_param_dndc2_c.cap( 1 ) );
               continue;
            }

            if ( rx_dad_param_n2_a.indexIn( qv[ i ] ) != -1 ) {
               le_dad_param_n2_a->setText( rx_dad_param_n2_a.cap( 1 ) );
               continue;
            }
            if ( rx_dad_param_n2_b.indexIn( qv[ i ] ) != -1 ) {
               le_dad_param_n2_b->setText( rx_dad_param_n2_b.cap( 1 ) );
               continue;
            }
            if ( rx_dad_param_n2_c.indexIn( qv[ i ] ) != -1 ) {
               le_dad_param_n2_c->setText( rx_dad_param_n2_c.cap( 1 ) );
               continue;
            }

            errormsg = QString( us_tr( "Error: loading %1 line %2 unrecognied directive %3" ) ).arg( filename ).arg( i + 1 ).arg( qv[ i ] );
            break;
         }
      }
   } while( 0 );
   
   if ( !errormsg.isEmpty() ) {
      QMessageBox::warning(
                           this
                           ,windowTitle()
                           ,QString( us_tr( "Loading file %1:\n%2" ) ).arg( filename ).arg( errormsg )
                           );
   } else {
      ((US_Hydrodyn  *)us_hydrodyn)->add_to_directory_history( filename );
   }
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

void US_Hydrodyn_Dad_Parameters::set_dad_param_desc( const QString & text )
{
   (*parameters)[ "dad_param_desc" ] = text;
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

void US_Hydrodyn_Dad_Parameters::set_dad_param_dndc2_a( const QString & text )
{
   (*parameters)[ "dad_param_dndc2_a" ] = text;
}

void US_Hydrodyn_Dad_Parameters::set_dad_param_dndc2_b( const QString & text )
{
   (*parameters)[ "dad_param_dndc2_b" ] = text;
}

void US_Hydrodyn_Dad_Parameters::set_dad_param_dndc2_c( const QString & text )
{
   (*parameters)[ "dad_param_dndc2_c" ] = text;
}

void US_Hydrodyn_Dad_Parameters::set_dad_param_n2_a( const QString & text )
{
   (*parameters)[ "dad_param_n2_a" ] = text;
}

void US_Hydrodyn_Dad_Parameters::set_dad_param_n2_b( const QString & text )
{
   (*parameters)[ "dad_param_n2_b" ] = text;
}

void US_Hydrodyn_Dad_Parameters::set_dad_param_n2_c( const QString & text )
{
   (*parameters)[ "dad_param_n2_c" ] = text;
}
