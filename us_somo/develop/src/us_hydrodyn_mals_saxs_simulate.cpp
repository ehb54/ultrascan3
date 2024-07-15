#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_saxs_util.h"
#include "../include/us_hydrodyn_mals_saxs_simulate.h"
//Added by qt3to4:
#include <QHBoxLayout>
#include <QCloseEvent>
#include <QGridLayout>
#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>

US_Hydrodyn_Mals_Saxs_Simulate::US_Hydrodyn_Mals_Saxs_Simulate(
                                                               void                         *          us_hydrodyn,
                                                               void                         *          mals_saxs_win,
                                                               QStringList                             files,
                                                               map < QString, vector < double > >      q,
                                                               map < QString, vector < double > >      I,
                                                               map < QString, vector < double > >      e,
                                                               QWidget *                               p,
                                                               const char *                            
                                                               ) : QFrame(  p )
{
   this->us_hydrodyn                = us_hydrodyn;
   this->mals_saxs_win                   = mals_saxs_win;
   this->files                      = files;
   this->q                          = q;
   this->I                          = I;
   this->e                          = e;


   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle( us_tr( "US-SOMO: HPLC SAXS Simulate" ) );

   setupGUI();
   ((US_Hydrodyn*)us_hydrodyn)->fixWinButtons( this );

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry( global_Xpos, global_Ypos, 0, 0 );

   setMinimumWidth( 1000 );

   displayData();
}

US_Hydrodyn_Mals_Saxs_Simulate::~US_Hydrodyn_Mals_Saxs_Simulate()
{
}

void US_Hydrodyn_Mals_Saxs_Simulate::setupGUI()
{
   if ( files.size() > 5 ) {
      QMessageBox::warning( this, windowTitle(), us_tr( "A maximum of 5 intensity curves may be selected for simulation" ) );
      close();
      return;
   }

   int minHeight1  = 30;

   lbl_title = new QLabel( "HPLC SAXS Simulate", this ); 
   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette( PALET_LABEL );
   AUTFBACK( lbl_title );
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_name = new QLabel( "Simulation name", this ); 
   lbl_name->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_name->setMinimumHeight(minHeight1);
   lbl_name->setPalette( PALET_LABEL );
   AUTFBACK( lbl_name );
   lbl_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_name = new QLineEdit( this );    le_name->setObjectName( "le_name Line Edit" );
   le_name->setText( "Simulate" );
   le_name->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_name->setPalette( PALET_NORMAL );
   AUTFBACK( le_name );
   le_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   lbl_frames = new QLabel( "Frames", this ); 
   lbl_frames->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_frames->setMinimumHeight(minHeight1);
   lbl_frames->setPalette( PALET_LABEL );
   AUTFBACK( lbl_frames );
   lbl_frames->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_frames = new QLineEdit( this );    le_frames->setObjectName( "le_frames Line Edit" );
   le_frames->setText( QString( "%1" ).arg( ( 1 + files.size() ) * 100 ) );
   le_frames->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_frames->setPalette( PALET_NORMAL );
   AUTFBACK( le_frames );
   le_frames->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_frames->setValidator( new QIntValidator( 1, 1000, le_frames ) );

   cb_noise = new QCheckBox(this);
   cb_noise->setText(us_tr("Random normal noise SD"));
   cb_noise->setEnabled( true );
   cb_noise->setChecked( false );
   cb_noise->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_noise->setPalette( PALET_NORMAL );
   AUTFBACK( cb_noise );
   connect( cb_noise, SIGNAL( clicked() ), SLOT( update_enables() ) );

   le_noise = new QLineEdit( this );    le_noise->setObjectName( "le_noise Line Edit" );
   le_noise->setText( ".05" );
   le_noise->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_noise->setPalette( PALET_NORMAL );
   AUTFBACK( le_noise );
   le_noise->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_noise->setValidator( new QDoubleValidator( 0, 25, 3, le_noise ) );

   cb_slope = new QCheckBox(this);
   cb_slope->setText(us_tr("Linear slope"));
   cb_slope->setEnabled( true );
   cb_slope->setChecked( false );
   cb_slope->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_slope->setPalette( PALET_NORMAL );
   AUTFBACK( cb_slope );
   connect( cb_slope, SIGNAL( clicked() ), SLOT( update_enables() ) );

   le_slope = new QLineEdit( this );    le_slope->setObjectName( "le_slope Line Edit" );
   le_slope->setText( "0.001" );
   le_slope->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_slope->setPalette( PALET_NORMAL );
   AUTFBACK( le_slope );
   le_slope->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_slope->setValidator( new QDoubleValidator( -1, 1, 5, le_slope ) );

   cb_alpha = new QCheckBox(this);
   cb_alpha->setText(us_tr("Global alpha"));
   cb_alpha->setEnabled( true );
   cb_alpha->setChecked( false );
   cb_alpha->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_alpha->setPalette( PALET_NORMAL );
   AUTFBACK( cb_alpha );
   connect( cb_alpha, SIGNAL( clicked() ), SLOT( update_enables() ) );

   le_alpha = new QLineEdit( this );    le_alpha->setObjectName( "le_alpha Line Edit" );
   le_alpha->setText( "0.02" );
   le_alpha->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_alpha->setPalette( PALET_NORMAL );
   AUTFBACK( le_alpha );
   le_alpha->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_alpha->setValidator( new QDoubleValidator( 0, 1, 5, le_alpha ) );

   cb_i_power = new QCheckBox(this);
   cb_i_power->setText(us_tr("Intensity exponent"));
   cb_i_power->setEnabled( true );
   cb_i_power->setChecked( false );
   cb_i_power->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_i_power->setPalette( PALET_NORMAL );
   AUTFBACK( cb_i_power );
   connect( cb_i_power, SIGNAL( clicked() ), SLOT( update_enables() ) );

   le_i_power = new QLineEdit( this );    le_i_power->setObjectName( "le_i_power Line Edit" );
   le_i_power->setText( "1" );
   le_i_power->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_i_power->setPalette( PALET_NORMAL );
   AUTFBACK( le_i_power );
   le_i_power->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_i_power->setValidator( new QDoubleValidator( 0, 5, 3, le_i_power ) );

   cb_gamma = new QCheckBox(this);
   cb_gamma->setText(us_tr("Gamma"));
   cb_gamma->setEnabled( true );
   cb_gamma->setChecked( false );
   cb_gamma->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_gamma->setPalette( PALET_NORMAL );
   AUTFBACK( cb_gamma );
   connect( cb_gamma, SIGNAL( clicked() ), SLOT( update_enables() ) );

   le_gamma = new QLineEdit( this );    le_gamma->setObjectName( "le_gamma Line Edit" );
   le_gamma->setText( "0" );
   le_gamma->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_gamma->setPalette( PALET_NORMAL );
   AUTFBACK( le_gamma );
   le_gamma->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_gamma->setValidator( new QDoubleValidator( 0, 1, 3, le_gamma ) );

   cb_exponent = new QCheckBox(this);
   cb_exponent->setText(us_tr("Accumulation factor"));
   cb_exponent->setEnabled( true );
   cb_exponent->setChecked( false );
   cb_exponent->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_exponent->setPalette( PALET_NORMAL );
   AUTFBACK( cb_exponent );
   connect( cb_exponent, SIGNAL( clicked() ), SLOT( update_enables() ) );

   le_exponent = new QLineEdit( this );    le_exponent->setObjectName( "le_exponent Line Edit" );
   le_exponent->setText( "0.5" );
   le_exponent->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_exponent->setPalette( PALET_NORMAL );
   AUTFBACK( le_exponent );
   le_exponent->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_exponent->setValidator( new QDoubleValidator( 0, 3, 3, le_exponent ) );

   pb_simulate =  new QPushButton ( us_tr( "Simulate" ), this );
   pb_simulate -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_simulate -> setMinimumHeight( minHeight1 );
   pb_simulate -> setPalette      ( PALET_PUSHB );
   connect( pb_simulate, SIGNAL( clicked() ), SLOT( simulate() ) );

   for ( int i = 0; i < (int) files.size(); ++i ) {
      uhshs_data this_data;

      this_data.lbl_name = new QLabel( files[ i ], this ); 
      this_data.lbl_name->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
      this_data.lbl_name->setMinimumHeight(minHeight1);
      this_data.lbl_name->setPalette( PALET_LABEL );
      this_data.lbl_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

      this_data.lbl_i_mult = new QLabel( us_tr( "Intensity multipler" ), this ); 
      this_data.lbl_i_mult->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
      this_data.lbl_i_mult->setMinimumHeight(minHeight1);
      this_data.lbl_i_mult->setPalette( PALET_LABEL );
      this_data.lbl_i_mult->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

      this_data.le_i_mult = new QLineEdit( this );       this_data.le_i_mult->setObjectName( "this_data.le_i_mult Line Edit" );
      this_data.le_i_mult->setText( "1" );
      this_data.le_i_mult->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
      this_data.le_i_mult->setPalette( PALET_NORMAL );
      this_data.le_i_mult->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
      this_data.le_i_mult->setValidator( new QDoubleValidator( 1e-3, 1e50, 3, this_data.le_i_mult ) );

      this_data.lbl_center = new QLabel( us_tr( "Peak frame center" ), this ); 
      this_data.lbl_center->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
      this_data.lbl_center->setMinimumHeight(minHeight1);
      this_data.lbl_center->setPalette( PALET_LABEL );
      this_data.lbl_center->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

      this_data.le_center = new QLineEdit( this );       this_data.le_center->setObjectName( "this_data.le_center Line Edit" );
      this_data.le_center->setText( QString( "%1" ).arg( 100 + i * 100 ) );
      this_data.le_center->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
      this_data.le_center->setPalette( PALET_NORMAL );
      this_data.le_center->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
      this_data.le_center->setValidator( new QDoubleValidator( this_data.le_center ) );

      this_data.lbl_width = new QLabel( us_tr( "Peak frame width" ), this ); 
      this_data.lbl_width->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
      this_data.lbl_width->setMinimumHeight(minHeight1);
      this_data.lbl_width->setPalette( PALET_LABEL );
      this_data.lbl_width->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

      this_data.le_width = new QLineEdit( this );       this_data.le_width->setObjectName( "this_data.le_width Line Edit" );
      this_data.le_width->setText( "20" );
      this_data.le_width->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
      this_data.le_width->setPalette( PALET_NORMAL );
      this_data.le_width->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
      this_data.le_width->setValidator( new QDoubleValidator( this_data.le_width ) );

      this_data.cb_alpha = new QCheckBox(this);
      this_data.cb_alpha->setText(us_tr("Species specific alpha"));
      this_data.cb_alpha->setEnabled( false );
      this_data.cb_alpha->setChecked( false );
      this_data.cb_alpha->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
      this_data.cb_alpha->setPalette( PALET_NORMAL );
      connect( this_data.cb_alpha, SIGNAL( clicked() ), SLOT( update_enables() ) );

      this_data.le_alpha = new QLineEdit( this );       this_data.le_alpha->setObjectName( "this_data.le_width Line Edit" );
      this_data.le_alpha->setText( "0" );
      this_data.le_alpha->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
      this_data.le_alpha->setPalette( PALET_NORMAL );
      this_data.le_alpha->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
      this_data.le_alpha->setValidator( new QDoubleValidator( this_data.le_alpha ) );

      data.push_back( this_data );
   }

   pb_help =  new QPushButton ( us_tr( "Help" ), this );
   pb_help -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_help -> setMinimumHeight( minHeight1 );
   pb_help -> setPalette      ( PALET_PUSHB );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );

   pb_close =  new QPushButton ( us_tr( "Close" ), this );
   pb_close -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_close -> setMinimumHeight( minHeight1 );
   pb_close -> setPalette      ( PALET_PUSHB );
   connect( pb_close, SIGNAL( clicked() ), SLOT( cancel() ) );

   QVBoxLayout * background = new QVBoxLayout( this ); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 );

   background->addWidget( lbl_title );

   {
      QGridLayout * gl = new QGridLayout( 0 ); gl->setContentsMargins( 0, 0, 0, 0 ); gl->setSpacing( 0 );
      int j = 0;
      gl->addWidget( lbl_name   , j  , 0 );
      gl->addWidget( le_name    , j  , 1 );
      gl->addWidget( lbl_frames , ++j, 0 );
      gl->addWidget( le_frames  , j  , 1 );
      gl->addWidget( cb_noise   , ++j, 0 );
      gl->addWidget( le_noise   , j  , 1 );
      gl->addWidget( cb_slope   , ++j, 0 );
      gl->addWidget( le_slope   , j  , 1 );
      gl->addWidget( cb_alpha   , ++j, 0 );
      gl->addWidget( le_alpha   , j  , 1 );
      gl->addWidget( cb_i_power , ++j, 0 );
      gl->addWidget( le_i_power , j  , 1 );
      gl->addWidget( cb_exponent, ++j, 0 );
      gl->addWidget( le_exponent, j  , 1 );
      gl->addWidget( cb_gamma   , ++j, 0 );
      gl->addWidget( le_gamma   , j  , 1 );
      background->addLayout( gl );
   }

   {
      QGridLayout * gl = new QGridLayout( 0 ); gl->setContentsMargins( 0, 0, 0, 0 ); gl->setSpacing( 0 );
      for ( int i = 0; i < (int) data.size(); ++i ) {
         int j = 0;
         gl->addWidget( data[ i ].lbl_name  , i, j++ );
         gl->addWidget( data[ i ].lbl_i_mult, i, j++ );
         gl->addWidget( data[ i ].le_i_mult , i, j++ );
         gl->addWidget( data[ i ].lbl_center, i, j++ );
         gl->addWidget( data[ i ].le_center , i, j++ );
         gl->addWidget( data[ i ].lbl_width , i, j++ );
         gl->addWidget( data[ i ].le_width  , i, j++ );
         gl->addWidget( data[ i ].cb_alpha  , i, j++ );
         gl->addWidget( data[ i ].le_alpha  , i, j++ );
      }
      background->addLayout( gl );
   }      

   QHBoxLayout * hbl_bottom = new QHBoxLayout(); hbl_bottom->setContentsMargins( 0, 0, 0, 0 ); hbl_bottom->setSpacing( 0 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addWidget ( pb_simulate );
   hbl_bottom->addWidget ( pb_close );

   background->addSpacing( 4 );
   background->addLayout ( hbl_bottom );
}

void US_Hydrodyn_Mals_Saxs_Simulate::cancel()
{
   close();
}

void US_Hydrodyn_Mals_Saxs_Simulate::help()
{
   US_Help *online_help;
   online_help = new US_Help( this );
   online_help->show_help("manual/somo/simulate.html");
}

void US_Hydrodyn_Mals_Saxs_Simulate::closeEvent( QCloseEvent *e )
{
   QMessageBox mb( this->windowTitle(), 
                   us_tr("Attention:\nAre you sure you want to exit the HPLC SAXS Simulation window?"),
                  QMessageBox::Information,
                  QMessageBox::Yes | QMessageBox::Default,
                  QMessageBox::Cancel | QMessageBox::Escape,
                  QMessageBox::NoButton);
   mb.setButtonText(QMessageBox::Yes, us_tr("Yes"));
   mb.setButtonText(QMessageBox::Cancel, us_tr("Cancel"));
   switch(mb.exec())
   {
   case QMessageBox::Cancel:
      {
         return;
      }
   }

   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Mals_Saxs_Simulate::displayData() {
   update_enables();
}

void US_Hydrodyn_Mals_Saxs_Simulate::update_enables() {
   le_noise    ->setEnabled( cb_noise   ->isChecked() );
   le_slope    ->setEnabled( cb_slope   ->isChecked() );
   le_alpha    ->setEnabled( cb_alpha   ->isChecked() );
   le_gamma    ->setEnabled( cb_gamma   ->isChecked() );

   if ( !cb_alpha->isChecked() ) {
      cb_i_power ->setChecked( false );
      cb_exponent->setChecked( false );
   }

   cb_i_power ->setEnabled( cb_alpha   ->isChecked() );
   le_i_power ->setEnabled( cb_i_power->isEnabled() && cb_i_power->isChecked() );

   cb_exponent ->setEnabled( cb_alpha   ->isChecked() );
   le_exponent ->setEnabled( cb_exponent->isEnabled() && cb_exponent->isChecked() );

   for ( int i = 0; i < (int) data.size(); ++i ) {
      data[ i ].le_alpha->setEnabled( data[ i ].cb_alpha->isChecked() );
   }
}

#define HW ((US_Hydrodyn_Mals_Saxs *)mals_saxs_win)

typedef struct {
  double x, y;
} double_pair;

static double_pair random_normal(double mean1, double sd1, double mean2, double sd2) {
   double s;
   double_pair v;
   do {
      v.x = 2 * drand48() - 1;
      v.y = 2 * drand48() - 1;
      s = v.x*v.x + v.y*v.y;
   } while (s > 1);
   s = sqrt(-2 * log(s) / s);
   v.x *= s * sd1;
   v.y *= s * sd2;
   v.x += mean1;
   v.y += mean2;
   return v;
}

void US_Hydrodyn_Mals_Saxs_Simulate::simulate() {
   
   us_qdebug( "run simulate" );

   vector < double > t;
   int frames = le_frames->text().toInt();

   for ( int i = 0; i < frames; ++i ) {
      t.push_back( (double) i );
   }

   // make a bunch of I(t)'s first... then convert to I(q) for output to mals_saxs

   vector < double >  qs = q[ files[ 0 ] ];
   int qpoints = (int) qs.size();

   vector < vector < vector < double > > > Its( data.size() );
   // data, qvalue, i(t)

   us_qdebug( QString( "gaussian type size %1 qpoints %2" ).arg( HW->gaussian_type_size ).arg( qpoints ) );

   vector < double > g( HW->gaussian_type_size, 0e0 );

   us_qdebug( "1" );

   for ( int i = 0; i < (int) data.size(); ++i ) {
      Its[ i ].resize( qpoints );
      g[ 1 ] = data[ i ].le_center->text().toDouble();
      g[ 2 ] = data[ i ].le_width->text().toDouble();
      double i_mult = data[ i ].le_i_mult->text().toDouble();

      for ( int j = 0; j < qpoints; ++j ) {
         g[ 0 ] = I[ files[ i ] ][ j ] * i_mult;
         Its[ i ][ j ] = HW->compute_gaussian_sum( t, g );
      }
   }

   us_qdebug( "2" );
   // compute sum
   vector < vector < double > > Itt( qpoints );
   for ( int j = 0; j < qpoints; ++j ) {
      Itt[ j ] = Its[ 0 ][ j ];
      for ( int i = 1; i < (int) data.size(); ++i ) {
         for ( int k = 0; k < frames; ++k ) {
            Itt[ j ][ k ] += Its[ i ][ j ][ k ];
         }
      }
   }
   us_qdebug( "3" );

   QString nameadd;

   if ( cb_alpha->isChecked() ) {
      double alpha = le_alpha->text().toDouble();
      nameadd += QString( "_a%1" ).arg( alpha );
      if ( cb_exponent->isChecked() ) {
         double exponent = le_exponent->text().toDouble();
         nameadd += QString( "_af%1" ).arg( exponent );
         if ( cb_i_power->isChecked() ) {
            double i_power = le_i_power->text().toDouble();
            nameadd += QString( "_ie%1" ).arg( i_power );
            for ( int j = 0; j < (int) Itt.size(); ++j ) {
               double accum = 1e-5;
               for ( int k = 0; k < (int) t.size(); ++k ) {
                  // accum += pow( accum, exponent ) + pow( Itt[ j ][ k ] * alpha, i_power );
                  // accum += ( accum + pow( Itt[ j ][ k ] * alpha, i_power ) ) * exponent;
                  accum += ( accum * alpha * pow( Itt[ j ][ k ], i_power ) ) * exponent;
                  Itt[ j ][ k ] += accum;
               }
            }
         } else {
            for ( int j = 0; j < (int) Itt.size(); ++j ) {
               double accum = 1e-5;
               for ( int k = 0; k < (int) t.size(); ++k ) {
                  // accum += accum * pow( accum, exponent ) + Itt[ j ][ k ] * alpha;
                  // accum += (accum + Itt[ j ][ k ] * alpha ) * exponent;
                  accum += (accum * Itt[ j ][ k ] * alpha ) * exponent;
                  Itt[ j ][ k ] += accum;
               }
            }
         }
      } else {
         if ( cb_i_power->isChecked() ) {
            double i_power = le_i_power->text().toDouble();
            nameadd += QString( "_ie%1" ).arg( i_power );
            for ( int j = 0; j < (int) Itt.size(); ++j ) {
               double accum = 0;
               for ( int k = 0; k < (int) t.size(); ++k ) {
                  accum += alpha * pow( Itt[ j ][ k ], i_power );
                  Itt[ j ][ k ] += accum;
               }
            }
         } else {
            for ( int j = 0; j < (int) Itt.size(); ++j ) {
               double accum = 0;
               for ( int k = 0; k < (int) t.size(); ++k ) {
                  accum += Itt[ j ][ k ] * alpha;
                  Itt[ j ][ k ] += accum;
               }
            }
         }
      }
   }

   if ( cb_slope->isChecked() ) {
      double slope = le_slope->text().toDouble();
      nameadd += QString( "_s%1" ).arg( slope );
      for ( int j = 0; j < (int) Itt.size(); ++j ) {
         for ( int k = 0; k < (int) t.size(); ++k ) {
            Itt[ j ][ k ] += t[ k ] * slope;
         }
      }
   }

   if ( cb_noise->isChecked() ) {
      double noise = le_noise->text().toDouble();
      nameadd += QString( "_n%1" ).arg( noise );
      for ( int j = 0; j < (int) Itt.size(); ++j ) {
         for ( int k = 0; k < (int) t.size(); ++k ) {
            double_pair v = random_normal( 0, noise, 0, noise );
            Itt[ j ][ k ] += v.x;
         }
      }
   }

   // plot'em
   us_qdebug( "4" );

   {
      map < QString, bool > current_files = HW->all_files_map();

      for ( int j = 0; j < qpoints; ++j ) {
         QString name = 
            QString( "%1_It_q%2" )
            .arg( lbl_name->text() )
            .arg( qs[ j ] )
            .replace( ".", "_" )
            ;
         QString use_name = name + nameadd;
         int ext = 0;
         while ( current_files.count( use_name ) ) {
            use_name = name + QString( "-%1" ).arg( ++ext );
         }
                    
         bool skip = false;
         for ( int k = 0; k < (int) Itt[ j ].size(); ++k ) {
            if ( us_isinf(  Itt[ j ][ k ] ) ||
                 us_isnan(  Itt[ j ][ k ] ) ) {
               skip = true;
               break;
            }
         }

         if ( skip ) {
            HW->editor_msg( "red", QString( "Could not add %1 due to nan or inf intensities" ).arg( use_name ) );
         } else {
            HW->add_plot( use_name, t, Itt[ j ], true, false );
         }
         us_qdebug( QString( "add plot %1" ).arg( use_name ) );
      }
      HW->update_enables();
   }
}

#undef HW

