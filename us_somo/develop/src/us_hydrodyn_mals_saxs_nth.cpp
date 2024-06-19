#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_mals_saxs_nth.h"
#include <set>
//Added by qt3to4:
#include <QHBoxLayout>
#include <QCloseEvent>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>

#define MALS_SAXS ((US_Hydrodyn_Mals_Saxs*)us_hydrodyn_mals_saxs)

US_Hydrodyn_Mals_Saxs_Nth::US_Hydrodyn_Mals_Saxs_Nth(
                                                     void                     *              us_hydrodyn_mals_saxs,
                                                     map < QString, QString > *              parameters,
                                                     QWidget *                               p,
                                                     const char *                            
                                                     ) : QDialog( p )
{
   this->us_hydrodyn_mals_saxs                = us_hydrodyn_mals_saxs;
   this->parameters                           = parameters;
   this->ggaussian_last_pfit_map              = &MALS_SAXS->ggaussian_last_pfit_map;

   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle( us_tr( "US-SOMO: MALS+SAXS : Select curves" ) );

   alpha        = parameters->count( "alpha" ) ? (*parameters)[ "alpha" ].toDouble() : 0.05;
   alpha_over_5 = 0.2 * alpha;

   setupGUI();
   update_enables();
   update_files_selected();

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry( global_Xpos, global_Ypos, 800, 0 );
}

US_Hydrodyn_Mals_Saxs_Nth::~US_Hydrodyn_Mals_Saxs_Nth()
{
}


void US_Hydrodyn_Mals_Saxs_Nth::setupGUI()
{
   bool started_in_expert_mode = ((US_Hydrodyn_Mals_Saxs*)us_hydrodyn_mals_saxs)->started_in_expert_mode;

   int minHeight1  = 24;

   lbl_title =  new QLabel      ( windowTitle(), this );
   lbl_title -> setAlignment    ( Qt::AlignCenter | Qt::AlignVCenter );
   lbl_title -> setMinimumHeight( minHeight1 );
   lbl_title -> setPalette( PALET_LABEL );
   AUTFBACK( lbl_title );
   lbl_title -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold ) );

   lbl_files =  new QLabel      ( us_tr( "Complete list of data files" ), this );
   lbl_files -> setAlignment    ( Qt::AlignCenter | Qt::AlignVCenter );
   lbl_files -> setPalette      ( PALET_NORMAL );
   AUTFBACK( lbl_files );
   lbl_files -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Normal) );

   lb_files = new QListWidget( this );
   lb_files -> setPalette( PALET_NORMAL );
   AUTFBACK( lb_files );
   lb_files -> setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lb_files -> setEnabled(true);
   lb_files -> setSelectionMode( QAbstractItemView::ExtendedSelection );
   lb_files -> setMinimumHeight( minHeight1 * 8 );

   for ( int i = 0; i < MALS_SAXS->lb_files->count(); ++i ) {
      lb_files->addItem( QString( "%1 : %2" ).arg( i + 1 ).arg( MALS_SAXS->lb_files->item( i )->text() ) );
      if ( MALS_SAXS->lb_files->item( i )->isSelected() ) {
         original_selections.insert( MALS_SAXS->lb_files->item( i )->text() );
         lb_files->item( i )->setSelected( true );
      }
   }
   connect( lb_files, SIGNAL( itemSelectionChanged() ), SLOT( update_files_selected() ) );

   if ( MALS_SAXS->check_files_selected_paired() ) {
      qDebug() << "we have a paired set of files!";
      paired_store_valid = true;
      paired_limit_valid = true;
   } else {
      qDebug() << "we do NOT have a paired set of files!";
      paired_store_valid = false;
      paired_limit_valid = false;
   }
      
   paired_restore_valid = MALS_SAXS->saved_nth_last_paired_valid();
   if ( paired_restore_valid && original_selections == MALS_SAXS->qsl_to_set( MALS_SAXS->saved_nth_last_paired_selections ) ) {
      paired_store_valid = false;
   }

   lbl_files_sel =  new QLabel      ( us_tr( "Selected data files" ), this );
   lbl_files_sel -> setAlignment    ( Qt::AlignCenter | Qt::AlignVCenter );
   lbl_files_sel -> setPalette      ( PALET_NORMAL );
   AUTFBACK( lbl_files_sel );
   lbl_files_sel -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Normal) );

   lb_files_sel = new QListWidget( this );
   lb_files_sel -> setPalette( PALET_NORMAL );
   AUTFBACK( lb_files_sel );
   lb_files_sel -> setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lb_files_sel -> setEnabled(true);
   lb_files_sel -> setSelectionMode( QListWidget::NoSelection );
   lb_files_sel -> setMinimumHeight( minHeight1 * 8 );

   lbl_files_selected =  new QLabel      ( "", this );
   lbl_files_selected -> setAlignment    ( Qt::AlignCenter | Qt::AlignVCenter );
   lbl_files_selected -> setPalette      ( PALET_NORMAL );
   AUTFBACK( lbl_files_selected );
   lbl_files_selected -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Normal) );

   pb_paired_store =  new QPushButton ( us_tr( "Save paired selections" ), this );
   pb_paired_store -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_paired_store -> setMinimumHeight( minHeight1 );
   pb_paired_store -> setPalette      ( PALET_PUSHB );
   connect( pb_paired_store, SIGNAL( clicked() ), SLOT( paired_store() ) );

   pb_paired_restore =  new QPushButton ( us_tr( "Restore previously stored paired selections" ), this );
   pb_paired_restore -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_paired_restore -> setMinimumHeight( minHeight1 );
   pb_paired_restore -> setPalette      ( PALET_PUSHB );
   connect( pb_paired_restore, SIGNAL( clicked() ), SLOT( paired_restore() ) );

   cb_paired_limit = new QCheckBox(this);
   cb_paired_limit->setText( us_tr("Select paired time curves" ) );
   cb_paired_limit->setChecked( paired_limit_valid );
   cb_paired_limit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_paired_limit->setPalette( PALET_NORMAL );
   AUTFBACK( cb_paired_limit );
   connect( cb_paired_limit, SIGNAL( clicked() ), SLOT( paired_limit() ) );

   lbl_select_nth = new QLabel ( us_tr( "Select every Nth" ), this);
   lbl_select_nth->setAlignment( Qt::AlignCenter | Qt::AlignVCenter);
   lbl_select_nth->setPalette  ( PALET_FRAME );
   AUTFBACK( lbl_select_nth );
   lbl_select_nth->setFont     ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   lbl_n =  new QLabel      ( us_tr( "Select every Nth:" ), this );
   lbl_n -> setAlignment    ( Qt::AlignRight | Qt::AlignVCenter );
   lbl_n -> setPalette( PALET_LABEL );
   AUTFBACK( lbl_n );
   lbl_n -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   le_n = new QLineEdit( this );    le_n->setObjectName( "le_n Line Edit" );
   le_n->setText( "2" );
   le_n->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_n->setPalette( PALET_NORMAL );
   AUTFBACK( le_n );
   le_n->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QIntValidator *qdv = new QIntValidator( 1, ((US_Hydrodyn_Mals_Saxs*)us_hydrodyn_mals_saxs)->lb_files->count(), le_n );
      le_n->setValidator( qdv );
   }
   connect( le_n, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_n->setMinimumWidth( 80 );

   lbl_start =  new QLabel      ( us_tr( "Starting curve offset:" ), this );
   lbl_start -> setAlignment    ( Qt::AlignRight | Qt::AlignVCenter );
   lbl_start -> setPalette( PALET_LABEL );
   AUTFBACK( lbl_start );
   lbl_start -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   le_start = new QLineEdit( this );    le_start->setObjectName( "le_start Line Edit" );
   le_start->setText( "1" );
   le_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_start->setPalette( PALET_NORMAL );
   AUTFBACK( le_start );
   le_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QIntValidator *qdv = new QIntValidator( 1, ((US_Hydrodyn_Mals_Saxs*)us_hydrodyn_mals_saxs)->lb_files->count(), le_start );
      le_start->setValidator( qdv );
   }
   connect( le_start, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_start->setMinimumWidth( 80 );
   
   lbl_start_name =  new QLabel      ( ( (US_Hydrodyn_Mals_Saxs*)us_hydrodyn_mals_saxs)->lb_files->item( le_start->text().toInt() - 1 )->text(), this );
   lbl_start_name -> setAlignment    ( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_start_name -> setPalette      ( PALET_NORMAL );
   AUTFBACK( lbl_start_name );
   lbl_start_name -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Normal) );

   lbl_end =  new QLabel      ( us_tr( "Ending curve offset:" ), this );
   lbl_end -> setAlignment    ( Qt::AlignRight | Qt::AlignVCenter );
   lbl_end -> setPalette      ( PALET_LABEL );
   AUTFBACK( lbl_end );
   lbl_end -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   le_end = new QLineEdit( this );    le_end->setObjectName( "le_end Line Edit" );
   le_end->setText( QString( "%1" ).arg( ((US_Hydrodyn_Mals_Saxs*)us_hydrodyn_mals_saxs)->lb_files->count() ) );
   le_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_end->setPalette( PALET_NORMAL );
   AUTFBACK( le_end );
   le_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QIntValidator *qdv = new QIntValidator( 2, ((US_Hydrodyn_Mals_Saxs*)us_hydrodyn_mals_saxs)->lb_files->count(), le_end );
      le_end->setValidator( qdv );
   }
   connect( le_end, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_end->setMinimumWidth( 80 );

   lbl_end_name =  new QLabel      ( ( (US_Hydrodyn_Mals_Saxs*)us_hydrodyn_mals_saxs)->lb_files->item( le_end->text().toInt() - 1 )->text(), this );
   lbl_end_name -> setAlignment    ( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_end_name -> setPalette      ( PALET_NORMAL );
   AUTFBACK( lbl_end_name );
   lbl_end_name -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Normal) );

   pb_nth_only =  new QPushButton ( us_tr( "Select Only" ), this );
   pb_nth_only -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_nth_only -> setMinimumHeight( minHeight1 );
   pb_nth_only -> setPalette      ( PALET_PUSHB );
   connect( pb_nth_only, SIGNAL( clicked() ), SLOT( nth_only() ) );

   pb_nth_add =  new QPushButton ( us_tr( "Select Additionally" ), this );
   pb_nth_add -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_nth_add -> setMinimumHeight( minHeight1 );
   pb_nth_add -> setPalette      ( PALET_PUSHB );
   connect( pb_nth_add, SIGNAL( clicked() ), SLOT( nth_add() ) );


   lbl_contain = new QLabel ( us_tr( "Select by name" ), this);
   lbl_contain->setAlignment( Qt::AlignCenter | Qt::AlignVCenter);
   lbl_contain->setPalette  ( PALET_FRAME );
   AUTFBACK( lbl_contain );
   lbl_contain->setFont     ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   lbl_contains =  new QLabel      ( us_tr( "Name contains:" ), this );
   lbl_contains -> setAlignment    ( Qt::AlignRight | Qt::AlignVCenter );
   lbl_contains -> setPalette      ( PALET_LABEL );
   AUTFBACK( lbl_contains );
   lbl_contains -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   le_contains = new QLineEdit( this );    le_contains->setObjectName( "le_contains Line Edit" );
   le_contains->setText( parameters->count( "mals_saxs_nth_contains" ) ? (*parameters)[ "mals_saxs_nth_contains" ] : "" );
   le_contains->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_contains->setPalette( PALET_NORMAL );
   AUTFBACK( le_contains );
   le_contains->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   connect( le_contains, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_contains->setMinimumWidth( 80 );

   pb_contains_only =  new QPushButton ( us_tr( "Select Only" ), this );
   pb_contains_only -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_contains_only -> setMinimumHeight( minHeight1 );
   pb_contains_only -> setPalette      ( PALET_PUSHB );
   connect( pb_contains_only, SIGNAL( clicked() ), SLOT( contains_only() ) );

   pb_contains_add =  new QPushButton ( us_tr( "Select Additionally" ), this );
   pb_contains_add -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_contains_add -> setMinimumHeight( minHeight1 );
   pb_contains_add -> setPalette      ( PALET_PUSHB );
   connect( pb_contains_add, SIGNAL( clicked() ), SLOT( contains_add() ) );

   lbl_pvalue = new QLabel ( us_tr( "Select by P-value" ), this);
   lbl_pvalue->setAlignment( Qt::AlignCenter | Qt::AlignVCenter);
   lbl_pvalue->setPalette  ( PALET_FRAME );
   AUTFBACK( lbl_pvalue );
   lbl_pvalue->setFont     ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   cb_pvalues_green = new QCheckBox(this);
   cb_pvalues_green->setText( us_tr("Green P values" ) );
   cb_pvalues_green->setChecked( true );
   cb_pvalues_green->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_pvalues_green->setPalette( PALET_NORMAL );
   AUTFBACK( cb_pvalues_green );
   connect( cb_pvalues_green, SIGNAL( clicked() ), SLOT( update_enables() ) );

   cb_pvalues_yellow = new QCheckBox(this);
   cb_pvalues_yellow->setText( us_tr("Yellow P values" ) );
   cb_pvalues_yellow->setChecked( true );
   cb_pvalues_yellow->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_pvalues_yellow->setPalette( PALET_NORMAL );
   AUTFBACK( cb_pvalues_yellow );
   connect( cb_pvalues_yellow, SIGNAL( clicked() ), SLOT( update_enables() ) );

   cb_pvalues_red = new QCheckBox(this);
   cb_pvalues_red->setText( us_tr("Red P values" ) );
   cb_pvalues_red->setChecked( true );
   cb_pvalues_red->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_pvalues_red->setPalette( PALET_NORMAL );
   AUTFBACK( cb_pvalues_red );
   connect( cb_pvalues_red, SIGNAL( clicked() ), SLOT( update_enables() ) );

   pb_pvalues_only =  new QPushButton ( us_tr( "Select Only" ), this );
   pb_pvalues_only -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_pvalues_only -> setMinimumHeight( minHeight1 );
   pb_pvalues_only -> setPalette      ( PALET_PUSHB );
   connect( pb_pvalues_only, SIGNAL( clicked() ), SLOT( pvalues_only() ) );

   pb_pvalues_add =  new QPushButton ( us_tr( "Select Additionally" ), this );
   pb_pvalues_add -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_pvalues_add -> setMinimumHeight( minHeight1 );
   pb_pvalues_add -> setPalette      ( PALET_PUSHB );
   connect( pb_pvalues_add, SIGNAL( clicked() ), SLOT( pvalues_add() ) );

   pb_help =  new QPushButton ( us_tr( "Help" ), this );
   pb_help -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_help -> setMinimumHeight( minHeight1 );
   pb_help -> setPalette      ( PALET_PUSHB );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );

   pb_quit =  new QPushButton ( us_tr( "Quit" ), this );
   pb_quit -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_quit -> setMinimumHeight( minHeight1 );
   pb_quit -> setPalette      ( PALET_PUSHB );
   connect( pb_quit, SIGNAL( clicked() ), SLOT( quit() ) );

   pb_go =  new QPushButton ( us_tr( "Transfer selections to main window" ), this );
   pb_go -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_go -> setMinimumHeight( minHeight1 );
   pb_go -> setPalette      ( PALET_PUSHB );
   connect( pb_go, SIGNAL( clicked() ), SLOT( go() ) );

   QVBoxLayout * background = new QVBoxLayout( this ); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 );
   background->addWidget( lbl_title );


   QHBoxLayout * hbl_files_pane = new QHBoxLayout(); hbl_files_pane->setContentsMargins( 0, 0, 0, 0 ); hbl_files_pane->setSpacing( 0 );
   {
      QVBoxLayout * vbl_files = new QVBoxLayout( 0 ); vbl_files->setContentsMargins( 0, 0, 0, 0 ); vbl_files->setSpacing( 0 );
      vbl_files->addWidget( lbl_files );
      vbl_files->addWidget( lb_files );
      hbl_files_pane->addLayout( vbl_files );
   }
   {
      QVBoxLayout * vbl_files_sel = new QVBoxLayout( 0 ); vbl_files_sel->setContentsMargins( 0, 0, 0, 0 ); vbl_files_sel->setSpacing( 0 );
      vbl_files_sel->addWidget( lbl_files_sel );
      vbl_files_sel->addWidget( lb_files_sel );
      hbl_files_pane->addLayout( vbl_files_sel );
   }

   background->addLayout( hbl_files_pane );
   background->addWidget( lbl_files_selected );

   {
      QHBoxLayout *hbl = new QHBoxLayout( 0 ); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
      hbl->addWidget( cb_paired_limit );
      hbl->addWidget( pb_paired_store );
      hbl->addWidget( pb_paired_restore );
      background->addLayout( hbl );
   }
      
   background->addWidget( lbl_select_nth );
   {
      QGridLayout * gl_nth = new QGridLayout( 0 ); gl_nth->setContentsMargins( 0, 0, 0, 0 ); gl_nth->setSpacing( 0 );

      gl_nth->addWidget         ( lbl_n          , 0, 0 );
      gl_nth->addWidget         ( le_n           , 0, 1 );
      gl_nth->addWidget         ( lbl_start      , 1, 0 );
      gl_nth->addWidget         ( le_start       , 1, 1 );
      gl_nth->addWidget         ( lbl_start_name , 1, 2 );
      gl_nth->addWidget         ( lbl_end        , 2, 0 );
      gl_nth->addWidget         ( le_end         , 2, 1 );
      gl_nth->addWidget         ( lbl_end_name   , 2, 2 );

      background->addLayout( gl_nth );
      QHBoxLayout * hbl_sel = new QHBoxLayout(); hbl_sel->setContentsMargins( 0, 0, 0, 0 ); hbl_sel->setSpacing( 0 );
      hbl_sel->addWidget ( pb_nth_only );
      hbl_sel->addWidget ( pb_nth_add );
      background->addLayout ( hbl_sel );
   }

   background->addWidget( lbl_contain );
   {
      QGridLayout * gl_contains = new QGridLayout( 0 ); gl_contains->setContentsMargins( 0, 0, 0, 0 ); gl_contains->setSpacing( 0 );

      gl_contains->addWidget         ( lbl_contains    , 0, 0 );
      gl_contains->addWidget         ( le_contains     , 0, 1 );

      background->addLayout( gl_contains );

      QHBoxLayout * hbl_sel = new QHBoxLayout(); hbl_sel->setContentsMargins( 0, 0, 0, 0 ); hbl_sel->setSpacing( 0 );
      hbl_sel->addWidget ( pb_contains_only );
      hbl_sel->addWidget ( pb_contains_add );
      background->addLayout ( hbl_sel );
   }

   background->addWidget( lbl_pvalue );
   {
      QHBoxLayout * hbl = new QHBoxLayout(); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
      hbl->addWidget ( cb_pvalues_green );
      hbl->addWidget ( cb_pvalues_yellow );
      hbl->addWidget ( cb_pvalues_red );
      background->addLayout ( hbl );
   }
   {
      QHBoxLayout * hbl = new QHBoxLayout(); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
      hbl->addWidget ( pb_pvalues_only );
      hbl->addWidget ( pb_pvalues_add );
      background->addLayout ( hbl );
   }

   QHBoxLayout * hbl_bottom = new QHBoxLayout(); hbl_bottom->setContentsMargins( 0, 0, 0, 0 ); hbl_bottom->setSpacing( 0 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addWidget ( pb_quit );
   hbl_bottom->addWidget ( pb_go );

   background->addSpacing( 4 );
   background->addLayout ( hbl_bottom );

   if ( !ggaussian_last_pfit_map->size() ||
        !started_in_expert_mode ) {
      lbl_pvalue        ->hide();
      cb_pvalues_green  ->hide();
      cb_pvalues_yellow ->hide();
      cb_pvalues_red    ->hide();
      pb_pvalues_only   ->hide();
      pb_pvalues_add    ->hide();
   }

   if ( paired_limit_valid ) {
      set_paired_ranges();
   }
}

void US_Hydrodyn_Mals_Saxs_Nth::quit()
{
   close();
}

void US_Hydrodyn_Mals_Saxs_Nth::go()
{
   (*parameters)[ "go"    ] = "true";
   
   for ( int i = 0; i < lb_files->count(); ++i )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         (*parameters)[ QString( "%1" ).arg( i ) ] = "1";
      }
   }

   (*parameters)[ "mals_saxs_nth_contains" ] = le_contains->text();

   close();
}

void US_Hydrodyn_Mals_Saxs_Nth::help()
{
   US_Help *online_help;
   online_help = new US_Help( this );
   online_help->show_help("manual/somo/saxs_hplc_nth.html");
}

void US_Hydrodyn_Mals_Saxs_Nth::closeEvent( QCloseEvent *e )
{

   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Mals_Saxs_Nth::update_enables()
{
   int fcnt = ( (US_Hydrodyn_Mals_Saxs*)us_hydrodyn_mals_saxs)->lb_files->count();
   int ofs = le_start->text().toInt();
   int end = le_end->text().toInt();
   if ( ofs >= 1 && ofs <= fcnt ) {
      lbl_start_name->setText( ( (US_Hydrodyn_Mals_Saxs*)us_hydrodyn_mals_saxs)->lb_files->item( ofs- 1 )->text() );
   } else {
      lbl_start_name->setText( "" );
   }
   if ( end >= 1 && end <= fcnt ) {
      lbl_end_name->setText( ( (US_Hydrodyn_Mals_Saxs*)us_hydrodyn_mals_saxs)->lb_files->item( end- 1 )->text() );
   } else {
      lbl_end_name->setText( "" );
   }

   int n   = le_n->text().toInt();

   set < int > selected;
   set < int > contains;
   set < int > pvalues;

   for ( int i = 0; i < lb_files->count(); ++i )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         selected.insert( i );
      }
      if ( ((US_Hydrodyn_Mals_Saxs*)us_hydrodyn_mals_saxs)->lb_files->item( i )->text().contains( le_contains->text() ) )
      {
         contains.insert( i );
      }
      {
         QString qs = ((US_Hydrodyn_Mals_Saxs*)us_hydrodyn_mals_saxs)->lb_files->item( i )->text();
         if ( ggaussian_last_pfit_map->count( qs ) ) {
            double pv = (*ggaussian_last_pfit_map)[ qs ];
            if ( ( cb_pvalues_green ->isChecked() && pv >= alpha ) ||
                 ( cb_pvalues_yellow->isChecked() && pv >= alpha_over_5 && pv < alpha ) ||
                 ( cb_pvalues_red->isChecked() && pv < alpha_over_5 ) )
            {
               pvalues.insert( i );
            }
         }
      }
   }

   int files_selected = (int) selected.size();

   bool any_in_range_not_selected = false;

   set < int > range;
   if ( n > 0 )
   {
      for ( int i = ofs - 1; i < end; i += n )
      {
         range.insert( i );
         if ( !any_in_range_not_selected &&
              !selected.count( i ) )
         {
            any_in_range_not_selected = true;
         }
      }
   }

   bool any_selected_not_in_range = false;
   for ( set < int >::iterator it = selected.begin();
         it != selected.end();
         it++ )
   {
      if ( !range.count( *it ) )
      {
         any_selected_not_in_range = true;
         break;
      }
   }

   pb_nth_only->setEnabled( any_in_range_not_selected || any_selected_not_in_range );
   pb_nth_add ->setEnabled( any_in_range_not_selected && any_selected_not_in_range );

   bool any_selected_not_contains = false;

   for ( set < int >::iterator it = selected.begin();
         it != selected.end();
         it++ )
   {
      if ( !contains.count( *it ) )
      {
         any_selected_not_contains = true;
         break;
      }
   }

   bool any_contains_not_selected = false;

   for ( set < int >::iterator it = contains.begin();
         it != contains.end();
         it++ )
   {
      if ( !selected.count( *it ) )
      {
         any_contains_not_selected = true;
         break;
      }
   }

   pb_contains_only->setEnabled( any_contains_not_selected || any_selected_not_contains );
   pb_contains_add ->setEnabled( any_contains_not_selected && any_selected_not_contains );

   bool any_selected_not_pvalues = false;

   for ( set < int >::iterator it = selected.begin();
         it != selected.end();
         it++ )
   {
      if ( !pvalues.count( *it ) )
      {
         any_selected_not_pvalues = true;
         break;
      }
   }

   bool any_pvalues_not_selected = false;

   for ( set < int >::iterator it = pvalues.begin();
         it != pvalues.end();
         it++ )
   {
      if ( !selected.count( *it ) )
      {
         any_pvalues_not_selected = true;
         break;
      }
   }

   pb_pvalues_only->setEnabled( any_pvalues_not_selected || any_selected_not_pvalues );
   pb_pvalues_add ->setEnabled( any_pvalues_not_selected && any_selected_not_pvalues );

   lbl_files_selected->setText( QString( "%1 of %2 selected" ).arg( files_selected ).arg( lb_files->count() ) );

   lb_files->setSelectionMode( cb_paired_limit->isChecked() ? QListWidget::NoSelection : QListWidget::ExtendedSelection );

   pb_paired_store->setEnabled( paired_store_valid );
   pb_paired_restore->setEnabled( paired_restore_valid );
   cb_paired_limit->setEnabled( paired_limit_valid );
}

void US_Hydrodyn_Mals_Saxs_Nth::nth_only()
{
   disconnect( lb_files, SIGNAL( itemSelectionChanged() ), 0, 0 );
   int n   = le_n->text().toInt();
   int ofs = le_start->text().toInt();
   int end = le_end->text().toInt();
   lb_files->clearSelection();
   lb_files->setSelectionMode( QListWidget::ExtendedSelection );
   if ( n > 0 )
   {
      for ( int i = ofs - 1; i < end; i += n )
      {
         lb_files->item( i )->setSelected( true );
         if ( cb_paired_limit->isChecked() && paired_map_to_pos.count( i ) ) {
            lb_files->item( paired_map_to_pos[ i ] )->setSelected( true );
         }
      }
   }
   lb_files->setSelectionMode( cb_paired_limit->isChecked() ? QListWidget::NoSelection : QListWidget::ExtendedSelection );
   connect( lb_files, SIGNAL( itemSelectionChanged() ), SLOT( update_files_selected() ) );
   update_files_selected();
}

void US_Hydrodyn_Mals_Saxs_Nth::nth_add()
{
   disconnect( lb_files, SIGNAL( itemSelectionChanged() ), 0, 0 );
   int n   = le_n->text().toInt();
   int ofs = le_start->text().toInt();
   int end = le_end->text().toInt();
   lb_files->setSelectionMode( QListWidget::ExtendedSelection );
   if ( n > 0 )
   {
      for ( int i = ofs - 1; i < end; i += n )
      {
         lb_files->item( i )->setSelected( true );
         if ( cb_paired_limit->isChecked() && paired_map_to_pos.count( i ) ) {
            lb_files->item( paired_map_to_pos[ i ] )->setSelected( true );
         }
      }
   }
   lb_files->setSelectionMode( cb_paired_limit->isChecked() ? QListWidget::NoSelection : QListWidget::ExtendedSelection );
   connect( lb_files, SIGNAL( itemSelectionChanged() ), SLOT( update_files_selected() ) );
   update_files_selected();
}

void US_Hydrodyn_Mals_Saxs_Nth::contains_only()
{
   disconnect( lb_files, SIGNAL( itemSelectionChanged() ), 0, 0 );
   lb_files->clearSelection();
   lb_files->setSelectionMode( QListWidget::ExtendedSelection );
   for ( int i = 0; i < lb_files->count(); ++i )
   {
      if ( ((US_Hydrodyn_Mals_Saxs*)us_hydrodyn_mals_saxs)->lb_files->item( i )->text().contains( le_contains->text() ) )
      {
         lb_files->item( i )->setSelected( true );
      }
   }
   lb_files->setSelectionMode( cb_paired_limit->isChecked() ? QListWidget::NoSelection : QListWidget::ExtendedSelection );
   connect( lb_files, SIGNAL( itemSelectionChanged() ), SLOT( update_files_selected() ) );
   update_files_selected();
}

void US_Hydrodyn_Mals_Saxs_Nth::contains_add()
{
   disconnect( lb_files, SIGNAL( itemSelectionChanged() ), 0, 0 );
   lb_files->setSelectionMode( QListWidget::ExtendedSelection );
   for ( int i = 0; i < lb_files->count(); ++i )
   {
      if ( ((US_Hydrodyn_Mals_Saxs*)us_hydrodyn_mals_saxs)->lb_files->item( i )->text().contains( le_contains->text() ) )
      {
         lb_files->item( i )->setSelected( true );
      }
   }
   lb_files->setSelectionMode( cb_paired_limit->isChecked() ? QListWidget::NoSelection : QListWidget::ExtendedSelection );
   connect( lb_files, SIGNAL( itemSelectionChanged() ), SLOT( update_files_selected() ) );
   update_files_selected();
}

void US_Hydrodyn_Mals_Saxs_Nth::pvalues_only()
{
   disconnect( lb_files, SIGNAL( itemSelectionChanged() ), 0, 0 );
   lb_files->clearSelection();
   lb_files->setSelectionMode( QListWidget::ExtendedSelection );
   for ( int i = 0; i < lb_files->count(); ++i )
   {
      QString qs = ((US_Hydrodyn_Mals_Saxs*)us_hydrodyn_mals_saxs)->lb_files->item( i )->text();
      if ( ggaussian_last_pfit_map->count( qs ) ) {
         double pv = (*ggaussian_last_pfit_map)[ qs ];
         if ( ( cb_pvalues_green ->isChecked() && pv >= alpha ) ||
              ( cb_pvalues_yellow->isChecked() && pv >= alpha_over_5 && pv < alpha ) ||
              ( cb_pvalues_red->isChecked() && pv < alpha_over_5 ) )
         {
            lb_files->item( i )->setSelected( true );
         }
      }
   }
   lb_files->setSelectionMode( cb_paired_limit->isChecked() ? QListWidget::NoSelection : QListWidget::ExtendedSelection );
   connect( lb_files, SIGNAL( itemSelectionChanged() ), SLOT( update_files_selected() ) );
   update_files_selected();
}

void US_Hydrodyn_Mals_Saxs_Nth::pvalues_add()
{
   disconnect( lb_files, SIGNAL( itemSelectionChanged() ), 0, 0 );
   lb_files->setSelectionMode( QListWidget::ExtendedSelection );
   for ( int i = 0; i < lb_files->count(); ++i )
   {
      QString qs = ((US_Hydrodyn_Mals_Saxs*)us_hydrodyn_mals_saxs)->lb_files->item( i )->text();
      if ( ggaussian_last_pfit_map->count( qs ) ) {
         double pv = (*ggaussian_last_pfit_map)[ qs ];
         if ( ( cb_pvalues_green ->isChecked() && pv >= alpha ) ||
              ( cb_pvalues_yellow->isChecked() && pv >= alpha_over_5 && pv < alpha ) ||
              ( cb_pvalues_red->isChecked() && pv < alpha_over_5 ) )
         {
            lb_files->item( i)->setSelected( true );
         }
      }
   }
   lb_files->setSelectionMode( cb_paired_limit->isChecked() ? QListWidget::NoSelection : QListWidget::ExtendedSelection );
   connect( lb_files, SIGNAL( itemSelectionChanged() ), SLOT( update_files_selected() ) );
   update_files_selected();
}

void US_Hydrodyn_Mals_Saxs_Nth::update_files_selected()
{
   lb_files_sel->clear( );
   for ( int i = 0; i < lb_files->count(); ++i )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         lb_files_sel->addItem( lb_files->item( i )->text() );
      }
   }
   update_enables();
}

void US_Hydrodyn_Mals_Saxs_Nth::paired_store() {
   // store paired selections
   qDebug() << "paired_store()";
   MALS_SAXS->saved_nth_last_paired_selections = MALS_SAXS->set_to_qsl( original_selections );
   paired_store_valid = false;
   update_enables();
}

void US_Hydrodyn_Mals_Saxs_Nth::paired_restore() {
   // restore paired selections
   qDebug() << "paired_restore()";
   disconnect( lb_files, SIGNAL( itemSelectionChanged() ), 0, 0 );

   lb_files->clearSelection();

   lb_files->setSelectionMode( QListWidget::ExtendedSelection );
   qDebug() << "lb_files->count() " << lb_files->count();
   
   for ( int i = 0; i < (int)lb_files->count(); ++i ) {
      QString tmp = lb_files->item( i )->text();
      tmp = tmp.replace( QRegularExpression( "^\\d+ : " ),"" );
      
      if ( MALS_SAXS->saved_nth_last_paired_selections.count( tmp ) ) {
         lb_files->item( i )->setSelected( true );
      }
   }
   lb_files->setSelectionMode( cb_paired_limit->isChecked() ? QListWidget::NoSelection : QListWidget::ExtendedSelection );
   connect( lb_files, SIGNAL( itemSelectionChanged() ), SLOT( update_files_selected() ) );

   paired_restore_valid = false;
   paired_limit_valid = true;
   cb_paired_limit->setChecked(true);
   MALS_SAXS->check_files_selected_paired( MALS_SAXS->saved_nth_last_paired_selections );
   set_paired_ranges();
   update_enables();
}

void US_Hydrodyn_Mals_Saxs_Nth::paired_limit() {
   // limit selections to the stored paired list
   if ( !cb_paired_limit->isChecked() ) {
      paired_limit_valid = false;
      paired_store_valid = false;
      ((QIntValidator *)le_start->validator())->setRange( 1, lb_files->count() );
      ((QIntValidator *)le_end  ->validator())->setRange( 1, lb_files->count() );
   }
   qDebug() << "paired_limit()";
   update_enables();
}

bool US_Hydrodyn_Mals_Saxs_Nth::set_paired_ranges() {
   qDebug() << "set_paired_ranges()";
   if ( MALS_SAXS->saved_nth_pair_names.size() != 2 ) {
      qDebug() << "set_paired_ranges() :: internal error";
      return false;
   }

   set < QString > names1;
   set < QString > names2;

   for ( auto const & name : MALS_SAXS->saved_nth_pair_names[0] ) {
      names1.insert( name );
   }
   
   for ( auto const & name : MALS_SAXS->saved_nth_pair_names[1] ) {
      names2.insert( name );
   }

   int name1_pos_start = 0;
   int name1_pos_end   = 0;
   int name2_pos_start = 0;
   int name2_pos_end   = 0;

   map < QString, int > names_to_pos;

   for ( int i = 0; i < (int)lb_files->count(); ++i ) {
      if ( lb_files->item(i)->isSelected() ) {
         QString tmp = lb_files->item(i)->text();
         tmp = tmp.replace( QRegularExpression( "^\\d+ : " ),"" );
         if ( names1.count( tmp ) ) {
            names_to_pos[ tmp ] = i;
            name1_pos_end = i + 1;
            if ( !name1_pos_start ) {
               name1_pos_start = name1_pos_end;
            }
         }
         if ( names2.count( tmp ) ) {
            names_to_pos[ tmp ] = i;
            name2_pos_end = i + 1;
            if ( !name2_pos_start ) {
               name2_pos_start = name2_pos_end;
            }
         }
      }
   }
            
   // use MALS_SAXS->saved_nth_pair_names to build paired_map_to_pos

   paired_map_to_pos.clear();

   int use_pos_start;
   int use_pos_end;

   set < QString > use_names;

   if ( name1_pos_start < name2_pos_start ) {
      use_pos_start = name1_pos_start;
      use_pos_end   = name1_pos_end;
      use_names     = names1;
   } else {
      use_pos_start = name2_pos_start;
      use_pos_end   = name2_pos_end;
      use_names     = names2;
   }      

   if ( use_names.count( MALS_SAXS->saved_nth_pair_names[0][0] ) ) {
      for ( int i = 0; i < (int) MALS_SAXS->saved_nth_pair_names[0].size(); ++i ) {
         // qDebug() << QString( "pos %1 : curve %2 pos %3 paired to curve %4 pos %5")
         //    .arg( i )
         //    .arg( MALS_SAXS->saved_nth_pair_names[0][i] )
         //    .arg( names_to_pos[ MALS_SAXS->saved_nth_pair_names[0][i] ] )
         //    .arg( MALS_SAXS->saved_nth_pair_names[1][i] )
         //    .arg( names_to_pos[ MALS_SAXS->saved_nth_pair_names[1][i] ] )
         //    ;
         paired_map_to_pos[ names_to_pos[ MALS_SAXS->saved_nth_pair_names[0][i] ] ] = names_to_pos[ MALS_SAXS->saved_nth_pair_names[1][i] ];
      }
   } else {
      for ( int i = 0; i < (int) MALS_SAXS->saved_nth_pair_names[0].size(); ++i ) {
         // qDebug() << QString( "pos %1 : curve %2 pos %3 paired to curve %4 pos %5")
         //    .arg( i )
         //    .arg( MALS_SAXS->saved_nth_pair_names[1][i] )
         //    .arg( names_to_pos[ MALS_SAXS->saved_nth_pair_names[1][i] ] )
         //    .arg( MALS_SAXS->saved_nth_pair_names[0][i] )
         //    .arg( names_to_pos[ MALS_SAXS->saved_nth_pair_names[0][i] ] )
         //    ;
         paired_map_to_pos[ names_to_pos[ MALS_SAXS->saved_nth_pair_names[1][i] ] ] = names_to_pos[ MALS_SAXS->saved_nth_pair_names[0][i] ];
      }
   }      
   
   le_start->setText( QString( "%1" ).arg( use_pos_start ) );
   le_end  ->setText( QString( "%1" ).arg( use_pos_end ) );

   // not qintvalidators don't really validate the range, just the number of digits

   ((QIntValidator *)le_start->validator())->setRange( use_pos_start, use_pos_end );
   ((QIntValidator *)le_end  ->validator())->setRange( use_pos_start, use_pos_end );
   

   return true;
}
