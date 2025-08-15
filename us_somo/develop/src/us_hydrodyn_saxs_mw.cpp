#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_saxs_mw.h"
#include "../include/us_hydrodyn.h"
//Added by qt3to4:
#include <QGridLayout>
#include <QLabel>

US_Hydrodyn_Saxs_Mw::US_Hydrodyn_Saxs_Mw(
                                         QString msg,
                                         float *mw,
                                         float *last_mw,
                                         bool *remember,
                                         bool *use_partial,
                                         QString *partial,
                                         map < QString, float > * remember_mw,
                                         map < QString, QString > * remember_mw_source,
                                         bool allow_none,
                                         QWidget *p,
                                         const char *
                                         ) : QDialog( p )
{
   this->msg = msg;
   this->mw = mw;
   this->last_mw = last_mw;
   this->remember = remember;
   this->use_partial = use_partial;
   this->partial = partial;
   this->remember_mw = remember_mw;
   this->remember_mw_source = remember_mw_source;
   this->allow_none = allow_none;

   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle( us_tr( "US-SOMO SAS: Set MW for P(r) calculation" ) );
   setupGUI();
   global_Xpos = 200;
   global_Ypos = 150;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_Saxs_Mw::~US_Hydrodyn_Saxs_Mw()
{
}

void US_Hydrodyn_Saxs_Mw::setupGUI()
{
   int minHeight1 = 30;
   int minHeight2 = 30;

   lbl_info = new QLabel(msg, this);
   lbl_info->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info->setMinimumHeight(minHeight2);
   lbl_info->setPalette( PALET_LABEL );
   AUTFBACK( lbl_info );
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   lbl_mw = new QLabel(us_tr(" Molecular Weight (Daltons):"), this);
   lbl_mw->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_mw->setMinimumHeight(minHeight2);
   lbl_mw->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_mw );
   lbl_mw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   le_mw = new QLineEdit( this );    le_mw->setObjectName( "mw Line Edit" );
   le_mw->setText(QString::asprintf( "%5.3f", *mw ));
   le_mw->setReadOnly(false);
   le_mw->setMinimumWidth(100);
   le_mw->setMinimumHeight(minHeight2);
   le_mw->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_mw->setPalette( PALET_NORMAL );
   AUTFBACK( le_mw );
   le_mw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1));
   connect(le_mw, SIGNAL(textChanged(const QString &)), SLOT(update_mw(const QString &)));

   pb_known_mw = new QPushButton(us_tr("Previously recorded MWs"), this);
   pb_known_mw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_known_mw->setMinimumHeight(minHeight1);
   pb_known_mw->setPalette( PALET_PUSHB );
   pb_known_mw->setEnabled( remember_mw->size() );
   connect(pb_known_mw, SIGNAL(clicked()), SLOT(set_known_mw()));

   if ( allow_none ) {
      pb_do_not_normalize = new QPushButton(us_tr("Do not normalize"), this);
      pb_do_not_normalize->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
      pb_do_not_normalize->setMinimumHeight(minHeight1);
      pb_do_not_normalize->setPalette( PALET_PUSHB );
      pb_do_not_normalize->setEnabled( true );
      connect(pb_do_not_normalize, SIGNAL(clicked()), SLOT(set_do_not_normalize()));
   }

   pb_set_to_last_used_mw = new QPushButton(us_tr("Set to last used weight:"), this);
   pb_set_to_last_used_mw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_set_to_last_used_mw->setMinimumHeight(minHeight1);
   pb_set_to_last_used_mw->setPalette( PALET_PUSHB );
   pb_set_to_last_used_mw->setEnabled(*last_mw > 0.0);
   connect(pb_set_to_last_used_mw, SIGNAL(clicked()), SLOT(set_to_last_used_mw()));

   lbl_last_used_mw = new QLabel(QString::asprintf( "%5.3f", *last_mw ), this);
   lbl_last_used_mw->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_last_used_mw->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_last_used_mw );

   cb_remember = new QCheckBox(this);
   cb_remember->setText(us_tr(" Remember these values ?"));
   cb_remember->setChecked(*remember);
   cb_remember->setMinimumHeight(minHeight1);
   cb_remember->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_remember->setPalette( PALET_NORMAL );
   AUTFBACK( cb_remember );
   connect(cb_remember, SIGNAL(clicked()), SLOT(set_remember()));

   cb_use_partial = new QCheckBox(this);
   cb_use_partial->setText(us_tr(" Remember for all files that contain: "));
   cb_use_partial->setChecked(*use_partial);
   cb_use_partial->setMinimumHeight(minHeight1);
   cb_use_partial->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_use_partial->setPalette( PALET_NORMAL );
   AUTFBACK( cb_use_partial );
   connect(cb_use_partial, SIGNAL(clicked()), SLOT(set_use_partial()));

   le_partial = new QLineEdit( this );    le_partial->setObjectName( "partial Line Edit" );
   le_partial->setText(*partial);
   le_partial->setReadOnly(false);
   le_partial->setMinimumWidth(250);
   le_partial->setMinimumHeight(minHeight2);
   le_partial->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_partial->setPalette( PALET_NORMAL );
   AUTFBACK( le_partial );
   le_partial->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1));
   connect(le_partial, SIGNAL(textChanged(const QString &)), SLOT(update_partial(const QString &)));

   pb_cancel = new QPushButton(us_tr("Close"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_help = new QPushButton("Help", this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight2);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   int j = 0;

   QGridLayout * background = new QGridLayout( this ); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 ); background->setSpacing( 2 ); background->setContentsMargins( 4, 4, 4, 4 );

   background->addWidget( lbl_info , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
   j++;
   background->addWidget(lbl_mw, j, 0);
   background->addWidget(le_mw, j, 1);
   j++;
   if ( allow_none ) {
      background->addWidget( pb_known_mw, j, 0 );
      background->addWidget( pb_do_not_normalize, j, 1 );
   } else {
      background->addWidget( pb_known_mw , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1  ) - ( 0 ) );
   }
   j++;
   background->addWidget(pb_set_to_last_used_mw, j, 0);
   background->addWidget(lbl_last_used_mw, j, 1);
   j++;
   background->addWidget( cb_remember , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
   j++;
   background->addWidget(cb_use_partial, j, 0);
   background->addWidget(le_partial, j, 1);
   j++;
   background->addWidget(pb_help, j, 0);
   background->addWidget(pb_cancel, j, 1);

   US_Hydrodyn::fixWinButtons( this );
}

void US_Hydrodyn_Saxs_Mw::set_to_last_used_mw()
{
   le_mw->setText(QString::asprintf( "%5.3f", *last_mw ));
   *mw = *last_mw;
}

void US_Hydrodyn_Saxs_Mw::cancel()
{
   close();
}

void US_Hydrodyn_Saxs_Mw::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo/somo_saxs_mw.html");
}

void US_Hydrodyn_Saxs_Mw::update_mw(const QString &str)
{
   *mw = str.toDouble();
}

void US_Hydrodyn_Saxs_Mw::update_partial(const QString &str)
{
   *partial = str;
}

void US_Hydrodyn_Saxs_Mw::set_remember()
{
   *remember = cb_remember->isChecked();
}

void US_Hydrodyn_Saxs_Mw::set_use_partial()
{
   *use_partial = cb_use_partial->isChecked();
}

void US_Hydrodyn_Saxs_Mw::set_do_not_normalize()
{
   le_mw->setText( "-1" );
   close();
}

void US_Hydrodyn_Saxs_Mw::set_known_mw()
{
   // for ( map < QString, float >::iterator it = remember_mw->begin();
   //       it != remember_mw->end();
   //       ++it ) {
   //    us_qdebug( QString( "%1 -> %2\n" ).arg( it->first ).arg( it->second ) );
   // }
   QStringList qsl;
   map < QString, float > good_mw;
   map < QString, QString > good_mw_source;
   set < QString > skip_mws;

   // pass 1 get "left" mw's

   QRegExp rx( "^(\\S+)\\s*( Model \\d+|)\\s*$" );

   for ( map < QString, float >::iterator it = remember_mw->begin();
         it != remember_mw->end();
         ++it ) {
      if ( rx.indexIn( it->first ) != -1 &&
           !rx.cap(1).isEmpty() ) {
         if ( !good_mw.count( rx.cap( 1 ) ) ) {
            good_mw[ rx.cap( 1 ) ] = it->second;
            if ( remember_mw_source->count( it->first ) ) {
               good_mw_source[ rx.cap( 1 ) ] = (*remember_mw_source)[ it->first ];
            }
         } else {
            if ( good_mw[ rx.cap( 1 ) ] != it->second ) {
               good_mw[ it->first ] = it->second;
               if ( remember_mw_source->count( it->first ) ) {
                  good_mw_source[ it->first ] = (*remember_mw_source)[ it->first ];
               }
            }
         }
      } else {
         good_mw[ it->first ] = it->second;
         if ( remember_mw_source->count( it->first ) ) {
            good_mw_source[ it->first ] = (*remember_mw_source)[ it->first ];
         }
      }
   }

   {
      map < QString, float > mw_as_displayed;
      for ( map < QString, float >::iterator it = good_mw.begin();
            it != good_mw.end();
            ++it ) {
         qsl <<
            QString( "%1 [%2] %3" )
            .arg( it->first )
            .arg( good_mw_source.count( it->first )
                  ? good_mw_source[ it->first ]
                  : QString( "" ) )
            .arg( it->second )
            ;
         mw_as_displayed[ qsl.back() ] = it->second;
      }

      {
         bool ok;
         QString choice = US_Static::getItem(
                                                us_tr( "US-SOMO SAS: previously recorded MWs" )
                                                ,us_tr( "Select from a previously recorded MWs or Cancel" )
                                                ,qsl
                                                ,0
                                                ,false
                                                ,&ok
                                                ,this
                                                );
         if ( !ok ) {
            return;
         }
         if ( !mw_as_displayed.count( choice ) ) {
            US_Static::us_message( us_tr("US-SOMO SAS: MW for P(r)"), 
                                  QString( us_tr("Internal error: choice not found in choices" ) ) );
            return;
         }
         le_mw->setText( QString::asprintf( "%5.3f", mw_as_displayed[ choice ] ) );
      }
   }
}
