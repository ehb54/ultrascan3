#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_saxs_buffer_nth.h"
#include <set>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <QCloseEvent>
#include <Q3BoxLayout>
#include <Q3GridLayout>
#include <QLabel>
#include <Q3VBoxLayout>

US_Hydrodyn_Saxs_Buffer_Nth::US_Hydrodyn_Saxs_Buffer_Nth(
                                                     void                     *              us_hydrodyn_saxs_buffer,
                                                     map < QString, QString > *              parameters,
                                                     QWidget *                               p,
                                                     const char *                            name
                                                     ) : QDialog( p, name )
{
   this->us_hydrodyn_saxs_buffer                = us_hydrodyn_saxs_buffer;
   this->parameters                           = parameters;

   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setCaption( tr( "US-SOMO: SAXS Data Utility : Select curves" ) );

   setupGUI();
   update_enables();
   update_files_selected();

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry( global_Xpos, global_Ypos, 0, 0 );
}

US_Hydrodyn_Saxs_Buffer_Nth::~US_Hydrodyn_Saxs_Buffer_Nth()
{
}

void US_Hydrodyn_Saxs_Buffer_Nth::setupGUI()
{
   int minHeight1  = 24;

   lbl_title =  new QLabel      ( caption(), this );
   lbl_title -> setAlignment    ( Qt::AlignCenter | Qt::AlignVCenter );
   lbl_title -> setMinimumHeight( minHeight1 );
   lbl_title -> setPalette( PALET_LABEL );
   AUTFBACK( lbl_title );
   lbl_title -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold ) );

   lbl_files =  new QLabel      ( tr( "Complete list of data files" ), this );
   lbl_files -> setAlignment    ( Qt::AlignCenter | Qt::AlignVCenter );
   lbl_files -> setPalette      ( PALET_NORMAL );
   AUTFBACK( lbl_files );
   lbl_files -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Normal) );

   lb_files =  new Q3ListBox(this, "files files listbox" );
   lb_files -> setPalette( PALET_NORMAL );
   AUTFBACK( lb_files );
   lb_files -> setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lb_files -> setEnabled(true);
   lb_files -> setSelectionMode( Q3ListBox::Extended );
   lb_files -> setMinimumHeight( minHeight1 * 8 );

   for ( int i = 0; i < ((US_Hydrodyn_Saxs_Buffer*)us_hydrodyn_saxs_buffer)->lb_files->numRows(); ++i )
   {
      lb_files->insertItem( QString( "%1 : %2" ).arg( i + 1 ).arg( ((US_Hydrodyn_Saxs_Buffer*)us_hydrodyn_saxs_buffer)->lb_files->text( i ) ) );
      if ( ((US_Hydrodyn_Saxs_Buffer*)us_hydrodyn_saxs_buffer)->lb_files->isSelected( i ) )
      {
         lb_files->setSelected( i , true );
      }
   }

   connect( lb_files, SIGNAL( selectionChanged() ), SLOT( update_files_selected() ) );

   lbl_files_sel =  new QLabel      ( tr( "Selected data files" ), this );
   lbl_files_sel -> setAlignment    ( Qt::AlignCenter | Qt::AlignVCenter );
   lbl_files_sel -> setPalette      ( PALET_NORMAL );
   AUTFBACK( lbl_files_sel );
   lbl_files_sel -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Normal) );

   lb_files_sel =  new Q3ListBox(this, "files_sel files_sel listbox" );
   lb_files_sel -> setPalette( PALET_NORMAL );
   AUTFBACK( lb_files_sel );
   lb_files_sel -> setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lb_files_sel -> setEnabled(true);
   lb_files_sel -> setSelectionMode( Q3ListBox::NoSelection );
   lb_files_sel -> setMinimumHeight( minHeight1 * 8 );

   lbl_files_selected =  new QLabel      ( "", this );
   lbl_files_selected -> setAlignment    ( Qt::AlignCenter | Qt::AlignVCenter );
   lbl_files_selected -> setPalette      ( PALET_NORMAL );
   AUTFBACK( lbl_files_selected );
   lbl_files_selected -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Normal) );

   // select

   lbl_select_nth = new mQLabel ( tr( "Select every Nth over range" ), this);
   lbl_select_nth->setAlignment( Qt::AlignCenter | Qt::AlignVCenter);
   lbl_select_nth->setPalette( PALET_LABEL );
   AUTFBACK( lbl_select_nth );
   lbl_select_nth->setFont     ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   connect( lbl_select_nth, SIGNAL( pressed() ), SLOT( hide_select() ) );

   lbl_n =  new QLabel      ( tr( "Select every Nth:" ), this );
   lbl_n -> setAlignment    ( Qt::AlignRight | Qt::AlignVCenter );
   lbl_n -> setPalette( PALET_LABEL );
   AUTFBACK( lbl_n );
   lbl_n -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   select_widgets.push_back( lbl_n );

   le_n = new QLineEdit(this, "le_n Line Edit");
   le_n->setText( "1" );
   le_n->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_n->setPalette( PALET_NORMAL );
   AUTFBACK( le_n );
   le_n->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QIntValidator *qdv = new QIntValidator( 1, ((US_Hydrodyn_Saxs_Buffer*)us_hydrodyn_saxs_buffer)->lb_files->numRows(), le_n );
      le_n->setValidator( qdv );
   }
   connect( le_n, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_n->setMinimumWidth( 80 );
   select_widgets.push_back( le_n );

   lbl_start =  new QLabel      ( tr( "Starting curve offset:" ), this );
   lbl_start -> setAlignment    ( Qt::AlignRight | Qt::AlignVCenter );
   lbl_start -> setPalette( PALET_LABEL );
   AUTFBACK( lbl_start );
   lbl_start -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );
   select_widgets.push_back( lbl_start );

   le_start = new QLineEdit(this, "le_start Line Edit");
   le_start->setText( "1" );
   le_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_start->setPalette( PALET_NORMAL );
   AUTFBACK( le_start );
   le_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QIntValidator *qdv = new QIntValidator( 1, ((US_Hydrodyn_Saxs_Buffer*)us_hydrodyn_saxs_buffer)->lb_files->numRows(), le_start );
      le_start->setValidator( qdv );
   }
   connect( le_start, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_start->setMinimumWidth( 80 );
   select_widgets.push_back( le_start );
   
   lbl_start_name =  new QLabel      ( ( (US_Hydrodyn_Saxs_Buffer*)us_hydrodyn_saxs_buffer)->lb_files->text( le_start->text().toInt() - 1 ), this );
   lbl_start_name -> setAlignment    ( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_start_name -> setPalette      ( PALET_NORMAL );
   AUTFBACK( lbl_start_name );
   lbl_start_name -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Normal) );

   select_widgets.push_back( lbl_start_name );

   lbl_end =  new QLabel      ( tr( "Ending curve offset:" ), this );
   lbl_end -> setAlignment    ( Qt::AlignRight | Qt::AlignVCenter );
   lbl_end -> setPalette      ( PALET_LABEL );
   AUTFBACK( lbl_end );
   lbl_end -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   select_widgets.push_back( lbl_end );

   le_end = new QLineEdit(this, "le_end Line Edit");
   le_end->setText( QString( "%1" ).arg( ((US_Hydrodyn_Saxs_Buffer*)us_hydrodyn_saxs_buffer)->lb_files->numRows() ) );
   le_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_end->setPalette( PALET_NORMAL );
   AUTFBACK( le_end );
   le_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QIntValidator *qdv = new QIntValidator( 2, ((US_Hydrodyn_Saxs_Buffer*)us_hydrodyn_saxs_buffer)->lb_files->numRows(), le_end );
      le_end->setValidator( qdv );
   }
   connect( le_end, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_end->setMinimumWidth( 80 );
   select_widgets.push_back( le_end );

   lbl_end_name =  new QLabel      ( ( (US_Hydrodyn_Saxs_Buffer*)us_hydrodyn_saxs_buffer)->lb_files->text( le_end->text().toInt() - 1 ), this );
   lbl_end_name -> setAlignment    ( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_end_name -> setPalette      ( PALET_NORMAL );
   AUTFBACK( lbl_end_name );
   lbl_end_name -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Normal) );

   select_widgets.push_back( lbl_end_name );

   pb_nth_only =  new QPushButton ( tr( "Select Only" ), this );
   pb_nth_only -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_nth_only -> setMinimumHeight( minHeight1 );
   pb_nth_only -> setPalette      ( PALET_PUSHB );
   connect( pb_nth_only, SIGNAL( clicked() ), SLOT( nth_only() ) );
   select_widgets.push_back( pb_nth_only );

   pb_nth_add =  new QPushButton ( tr( "Select Additionally" ), this );
   pb_nth_add -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_nth_add -> setMinimumHeight( minHeight1 );
   pb_nth_add -> setPalette      ( PALET_PUSHB );
   connect( pb_nth_add, SIGNAL( clicked() ), SLOT( nth_add() ) );
   select_widgets.push_back( pb_nth_add );

   // contain

   lbl_contain = new mQLabel ( tr( "Select by name" ), this);
   lbl_contain->setAlignment( Qt::AlignCenter | Qt::AlignVCenter);
   lbl_contain->setPalette( PALET_LABEL );
   AUTFBACK( lbl_contain );
   lbl_contain->setFont     ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   connect( lbl_contain, SIGNAL( pressed() ), SLOT( hide_contain() ) );

   lbl_contains =  new QLabel      ( tr( "Name contains:" ), this );
   lbl_contains -> setAlignment    ( Qt::AlignRight | Qt::AlignVCenter );
   lbl_contains -> setPalette      ( PALET_LABEL );
   AUTFBACK( lbl_contains );
   lbl_contains -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   contain_widgets.push_back( lbl_contains );

   le_contains = new QLineEdit(this, "le_contains Line Edit");
   le_contains->setText( parameters->count( "buffer_nth_contains" ) ? (*parameters)[ "buffer_nth_contains" ] : "" );
   le_contains->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_contains->setPalette( PALET_NORMAL );
   AUTFBACK( le_contains );
   le_contains->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   connect( le_contains, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_contains->setMinimumWidth( 80 );
   contain_widgets.push_back( le_contains );

   pb_contains_only =  new QPushButton ( tr( "Select Only" ), this );
   pb_contains_only -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_contains_only -> setMinimumHeight( minHeight1 );
   pb_contains_only -> setPalette      ( PALET_PUSHB );
   connect( pb_contains_only, SIGNAL( clicked() ), SLOT( contains_only() ) );

   contain_widgets.push_back( pb_contains_only );

   pb_contains_add =  new QPushButton ( tr( "Select Additionally" ), this );
   pb_contains_add -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_contains_add -> setMinimumHeight( minHeight1 );
   pb_contains_add -> setPalette      ( PALET_PUSHB );
   connect( pb_contains_add, SIGNAL( clicked() ), SLOT( contains_add() ) );

   contain_widgets.push_back( pb_contains_add );

   // intensity

   lbl_intensity = new mQLabel ( tr( "Select by intensity" ), this);
   lbl_intensity->setAlignment( Qt::AlignCenter | Qt::AlignVCenter);
   lbl_intensity->setPalette( PALET_LABEL );
   AUTFBACK( lbl_intensity );
   lbl_intensity->setFont     ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   connect( lbl_intensity, SIGNAL( pressed() ), SLOT( hide_intensity() ) );

   cb_q_range =  new QCheckBox   ( tr( "q range [A]:" ), this );
   cb_q_range -> setPalette      ( PALET_NORMAL );
   AUTFBACK( cb_q_range );
   cb_q_range -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   connect( cb_q_range, SIGNAL( clicked() ), SLOT( update_enables() ) );

   intensity_widgets.push_back( cb_q_range );

   // probably compute min/max q range over all

   le_q_start = new QLineEdit(this, "le_q_start Line Edit");
   le_q_start->setText( "" );
   le_q_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_q_start->setPalette( PALET_NORMAL );
   AUTFBACK( le_q_start );
   le_q_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   le_q_start->setValidator( new QDoubleValidator( le_q_start ) );
   ( (QDoubleValidator *)le_q_start->validator() )->setRange( 0, .5, 3 );
   connect( le_q_start, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_q_start->setMinimumWidth( 40 );
   intensity_widgets.push_back( le_q_start );

   le_q_end = new QLineEdit(this, "le_q_end Line Edit");
   le_q_end->setText( "" );
   le_q_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_q_end->setPalette( PALET_NORMAL );
   AUTFBACK( le_q_end );
   le_q_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   le_q_end->setValidator( new QDoubleValidator( le_q_end ) );
   ( (QDoubleValidator *)le_q_end->validator() )->setRange( 0, .5, 3 );
   connect( le_q_end, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_q_end->setMinimumWidth( 40 );
   intensity_widgets.push_back( le_q_end );

   pb_i_avg_all =  new QPushButton ( tr( "Compute average intensity of all" ), this );
   pb_i_avg_all -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_i_avg_all -> setMinimumHeight( minHeight1 );
   pb_i_avg_all -> setPalette      ( PALET_PUSHB );
   connect( pb_i_avg_all, SIGNAL( clicked() ), SLOT( i_avg_all() ) );

   intensity_widgets.push_back( pb_i_avg_all );

   pb_i_avg_sel =  new QPushButton ( tr( "Compute average intensity of selected" ), this );
   pb_i_avg_sel -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_i_avg_sel -> setMinimumHeight( minHeight1 );
   pb_i_avg_sel -> setPalette      ( PALET_PUSHB );
   connect( pb_i_avg_sel, SIGNAL( clicked() ), SLOT( i_avg_sel() ) );

   intensity_widgets.push_back( pb_i_avg_sel );

   te_q = new Q3TextEdit(this, "te_q Line Edit");
   te_q->setText( "" );
   te_q->setPalette( PALET_NORMAL );
   AUTFBACK( te_q );
   te_q->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   te_q->setMinimumWidth( 80 );

   intensity_widgets.push_back( te_q );

   cb_i_above =  new QCheckBox   ( tr( "Above intensity level [A.U.]:" ), this );
   cb_i_above -> setPalette      ( PALET_NORMAL );
   AUTFBACK( cb_i_above );
   cb_i_above -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   connect( cb_i_above, SIGNAL( clicked() ), SLOT( update_enables() ) );

   intensity_widgets.push_back( cb_i_above );

   // probably compute min/max q range over all

   le_i_level = new QLineEdit(this, "le_i_level Line Edit");
   le_i_level->setText( "" );
   le_i_level->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_i_level->setPalette( PALET_NORMAL );
   AUTFBACK( le_i_level );
   le_i_level->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   le_i_level->setValidator( new QDoubleValidator( le_i_level ) );
   // ( (QDoubleValidator *)le_i_level->validator() )->setRange( 0, .5, 3 );
   connect( le_i_level, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_i_level->setMinimumWidth( 80 );
   intensity_widgets.push_back( le_i_level );

   pb_i_only =  new QPushButton ( tr( "Select Only" ), this );
   pb_i_only -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_i_only -> setMinimumHeight( minHeight1 );
   pb_i_only -> setPalette      ( PALET_PUSHB );
   connect( pb_i_only, SIGNAL( clicked() ), SLOT( i_only() ) );

   intensity_widgets.push_back( pb_i_only );

   pb_i_add =  new QPushButton ( tr( "Select Additionally" ), this );
   pb_i_add -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_i_add -> setMinimumHeight( minHeight1 );
   pb_i_add -> setPalette      ( PALET_PUSHB );
   connect( pb_i_add, SIGNAL( clicked() ), SLOT( i_add() ) );

   intensity_widgets.push_back( pb_i_add );



   // bottom


   pb_help =  new QPushButton ( tr( "Help" ), this );
   pb_help -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_help -> setMinimumHeight( minHeight1 );
   pb_help -> setPalette      ( PALET_PUSHB );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );

   pb_quit =  new QPushButton ( tr( "Quit" ), this );
   pb_quit -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_quit -> setMinimumHeight( minHeight1 );
   pb_quit -> setPalette      ( PALET_PUSHB );
   connect( pb_quit, SIGNAL( clicked() ), SLOT( quit() ) );

   pb_do_select =  new QPushButton ( tr( "Select in main window" ), this );
   pb_do_select -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_do_select -> setMinimumHeight( minHeight1 );
   pb_do_select -> setPalette      ( PALET_PUSHB );
   connect( pb_do_select, SIGNAL( clicked() ), SLOT( do_select() ) );

   pb_go =  new QPushButton ( tr( "Select in main window and exit" ), this );
   pb_go -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_go -> setMinimumHeight( minHeight1 );
   pb_go -> setPalette      ( PALET_PUSHB );
   connect( pb_go, SIGNAL( clicked() ), SLOT( go() ) );

   Q3VBoxLayout *background = new Q3VBoxLayout( this );
   background->addWidget( lbl_title );


   Q3HBoxLayout *hbl_files_pane = new Q3HBoxLayout( 0 );
   {
      Q3VBoxLayout *vbl_files = new Q3VBoxLayout( 0 );
      vbl_files->addWidget( lbl_files );
      vbl_files->addWidget( lb_files );
      hbl_files_pane->addLayout( vbl_files );
   }
   {
      Q3VBoxLayout *vbl_files_sel = new Q3VBoxLayout( 0 );
      vbl_files_sel->addWidget( lbl_files_sel );
      vbl_files_sel->addWidget( lb_files_sel );
      hbl_files_pane->addLayout( vbl_files_sel );
   }

   background->addLayout( hbl_files_pane );
   background->addWidget( lbl_files_selected );

   background->addWidget( lbl_select_nth );
   {
      Q3GridLayout *gl_nth = new Q3GridLayout( 0 );

      gl_nth->addWidget         ( lbl_n          , 0, 0 );
      gl_nth->addWidget         ( le_n           , 0, 1 );
      gl_nth->addWidget         ( lbl_start      , 1, 0 );
      gl_nth->addWidget         ( le_start       , 1, 1 );
      gl_nth->addWidget         ( lbl_start_name , 1, 2 );
      gl_nth->addWidget         ( lbl_end        , 2, 0 );
      gl_nth->addWidget         ( le_end         , 2, 1 );
      gl_nth->addWidget         ( lbl_end_name   , 2, 2 );

      background->addLayout( gl_nth );
      Q3HBoxLayout *hbl_sel = new Q3HBoxLayout( 0 );
      hbl_sel->addWidget ( pb_nth_only );
      hbl_sel->addWidget ( pb_nth_add );
      background->addLayout ( hbl_sel );
   }

   background->addWidget( lbl_contain );
   {
      Q3GridLayout *gl_contains = new Q3GridLayout( 0 );

      gl_contains->addWidget         ( lbl_contains    , 0, 0 );
      gl_contains->addWidget         ( le_contains     , 0, 1 );

      background->addLayout( gl_contains );

      Q3HBoxLayout *hbl_sel = new Q3HBoxLayout( 0 );
      hbl_sel->addWidget ( pb_contains_only );
      hbl_sel->addWidget ( pb_contains_add );
      background->addLayout ( hbl_sel );
   }

   background->addWidget( lbl_intensity );
   {
      Q3GridLayout *gl_intensity = new Q3GridLayout( 0 );

      int j = 0;

      gl_intensity->addWidget         ( cb_q_range    , j, 0 );

      {
         Q3BoxLayout *hbl = new Q3HBoxLayout( 0 );
         hbl->addWidget( le_q_start );
         hbl->addWidget( le_q_end );
         gl_intensity->addMultiCellLayout( hbl, j, j, 1, 2 );
         ++j;
      }
      {
         Q3BoxLayout *hbl = new Q3HBoxLayout( 0 );
         hbl->addWidget( pb_i_avg_all );
         hbl->addWidget( pb_i_avg_sel );
         gl_intensity->addMultiCellLayout( hbl, j, j, 0, 2 );
         ++j;
      }

      gl_intensity->addMultiCellWidget( te_q, j, j + 2, 0, 2);
      j += 3;

      gl_intensity->addWidget         ( cb_i_above, j, 0 );
      gl_intensity->addMultiCellWidget( le_i_level, j, j, 1, 2 );
      ++j;

      background->addLayout( gl_intensity );

      Q3HBoxLayout *hbl_sel = new Q3HBoxLayout( 0 );
      hbl_sel->addWidget ( pb_i_only );
      hbl_sel->addWidget ( pb_i_add );
      background->addLayout ( hbl_sel );
   }

   Q3HBoxLayout *hbl_bottom = new Q3HBoxLayout( 0 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addWidget ( pb_quit );
   hbl_bottom->addWidget ( pb_do_select );
   hbl_bottom->addWidget ( pb_go );

   background->addSpacing( 4 );
   background->addLayout ( hbl_bottom );

   ShowHide::hide_widgets( select_widgets );
   ShowHide::hide_widgets( contain_widgets );
   ShowHide::hide_widgets( intensity_widgets );
   if ( (*parameters)[ "buffer_nth_shown" ] == "contain" )
   {
      ShowHide::hide_widgets( contain_widgets, false, this );
   } else {
      if ( (*parameters)[ "buffer_nth_shown" ] == "intensity" )
      {
         ShowHide::hide_widgets( intensity_widgets, false, this );
      } else {
         ShowHide::hide_widgets( intensity_widgets, false, this );
      }
   }
}

void US_Hydrodyn_Saxs_Buffer_Nth::quit()
{
   close();
}

void US_Hydrodyn_Saxs_Buffer_Nth::do_select( bool update )
{

   for ( int i = 0; i < lb_files->numRows(); ++i )
   {
      if ( lb_files->isSelected( i ) )
      {
         (*parameters)[ QString( "%1" ).arg( i ) ] = "1";
      } else {
         if ( parameters->count( QString( "%1" ).arg( i ) ) )
         {
            parameters->erase( QString( "%1" ).arg( i ) );
         }
      }         
   }

   if ( update )
   {
      ((US_Hydrodyn_Saxs_Buffer*)us_hydrodyn_saxs_buffer)->select_these( *parameters );
   }
}

void US_Hydrodyn_Saxs_Buffer_Nth::go()
{
   (*parameters)[ "go"    ] = "true";
   
   do_select( false );

   (*parameters)[ "buffer_nth_contains" ] = le_contains->text();

   close();
}

void US_Hydrodyn_Saxs_Buffer_Nth::help()
{
   US_Help *online_help;
   online_help = new US_Help( this );
   online_help->show_help("manual/saxs_buffer_nth.html");
}

void US_Hydrodyn_Saxs_Buffer_Nth::closeEvent( QCloseEvent *e )
{

   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Saxs_Buffer_Nth::update_enables()
{
   lbl_start_name->setText( ( (US_Hydrodyn_Saxs_Buffer*)us_hydrodyn_saxs_buffer)->lb_files->text( le_start->text().toInt() - 1 ) );
   lbl_end_name  ->setText( ( (US_Hydrodyn_Saxs_Buffer*)us_hydrodyn_saxs_buffer)->lb_files->text( le_end  ->text().toInt() - 1 ) );

   int n   = le_n->text().toInt();
   int ofs = le_start->text().toInt();
   int end = le_end->text().toInt();

   set < int > selected;
   set < int > contains;

   for ( int i = 0; i < lb_files->numRows(); ++i )
   {
      if ( lb_files->isSelected( i ) )
      {
         selected.insert( i );
      }
      if ( ((US_Hydrodyn_Saxs_Buffer*)us_hydrodyn_saxs_buffer)->lb_files->text( i ).contains( le_contains->text() ) )
      {
         contains.insert( i );
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

   lbl_files_selected->setText( QString( "%1 of %2 selected" ).arg( files_selected ).arg( lb_files->numRows() ) );

   le_q_start  ->setEnabled( cb_q_range->isChecked() );
   le_q_end    ->setEnabled( cb_q_range->isChecked() );
   pb_i_avg_sel->setEnabled( files_selected );
}

void US_Hydrodyn_Saxs_Buffer_Nth::nth_only()
{
   disconnect( lb_files, SIGNAL( selectionChanged() ), 0, 0 );
   int n   = le_n->text().toInt();
   int ofs = le_start->text().toInt();
   int end = le_end->text().toInt();
   lb_files->clearSelection();
   if ( n > 0 )
   {
      for ( int i = ofs - 1; i < end; i += n )
      {
         lb_files->setSelected( i, true );
      }
   }
   connect( lb_files, SIGNAL( selectionChanged() ), SLOT( update_files_selected() ) );
   update_files_selected();
}

void US_Hydrodyn_Saxs_Buffer_Nth::nth_add()
{
   disconnect( lb_files, SIGNAL( selectionChanged() ), 0, 0 );
   int n   = le_n->text().toInt();
   int ofs = le_start->text().toInt();
   int end = le_end->text().toInt();
   if ( n > 0 )
   {
      for ( int i = ofs - 1; i < end; i += n )
      {
         lb_files->setSelected( i, true );
      }
   }
   connect( lb_files, SIGNAL( selectionChanged() ), SLOT( update_files_selected() ) );
   update_files_selected();
}

void US_Hydrodyn_Saxs_Buffer_Nth::contains_only()
{
   disconnect( lb_files, SIGNAL( selectionChanged() ), 0, 0 );
   lb_files->clearSelection();
   for ( int i = 0; i < lb_files->numRows(); ++i )
   {
      if ( ((US_Hydrodyn_Saxs_Buffer*)us_hydrodyn_saxs_buffer)->lb_files->text( i ).contains( le_contains->text() ) )
      {
         lb_files->setSelected( i, true );
      }
   }
   connect( lb_files, SIGNAL( selectionChanged() ), SLOT( update_files_selected() ) );
   update_files_selected();
}

void US_Hydrodyn_Saxs_Buffer_Nth::contains_add()
{
   disconnect( lb_files, SIGNAL( selectionChanged() ), 0, 0 );
   for ( int i = 0; i < lb_files->numRows(); ++i )
   {
      if ( ((US_Hydrodyn_Saxs_Buffer*)us_hydrodyn_saxs_buffer)->lb_files->text( i ).contains( le_contains->text() ) )
      {
         lb_files->setSelected( i, true );
      }
   }
   connect( lb_files, SIGNAL( selectionChanged() ), SLOT( update_files_selected() ) );
   update_files_selected();
}

void US_Hydrodyn_Saxs_Buffer_Nth::update_files_selected()
{
   lb_files_sel->clear();
   for ( int i = 0; i < lb_files->numRows(); ++i )
   {
      if ( lb_files->isSelected( i ) )
      {
         lb_files_sel->insertItem( lb_files->text( i ) );
      }
   }
   update_enables();
}

void US_Hydrodyn_Saxs_Buffer_Nth::hide_select()
{
   if ( select_widgets.size() )
   {
      ShowHide::hide_widgets( select_widgets, select_widgets[ 0 ]->isVisible(), this );
      if ( select_widgets[ 0 ]->isVisible() )
      {
         (*parameters)[ "buffer_nth_shown" ] = "select";
      }
   }
   ShowHide::hide_widgets( contain_widgets, true, this );
   ShowHide::hide_widgets( intensity_widgets, true, this );
}

void US_Hydrodyn_Saxs_Buffer_Nth::hide_contain()
{
   if ( contain_widgets.size() )
   {
      ShowHide::hide_widgets( contain_widgets, contain_widgets[ 0 ]->isVisible(), this );
      if ( contain_widgets[ 0 ]->isVisible() )
      {
         (*parameters)[ "buffer_nth_shown" ] = "contain";
      }
   }
   ShowHide::hide_widgets( select_widgets, true, this );
   ShowHide::hide_widgets( intensity_widgets, true, this );
}
        

void US_Hydrodyn_Saxs_Buffer_Nth::hide_intensity()
{
   if ( intensity_widgets.size() )
   {
      ShowHide::hide_widgets( intensity_widgets, intensity_widgets[ 0 ]->isVisible(), this );
      if ( intensity_widgets[ 0 ]->isVisible() )
      {
         (*parameters)[ "buffer_nth_shown" ] = "intensity";
      }
   }
   ShowHide::hide_widgets( select_widgets, true, this );
   ShowHide::hide_widgets( contain_widgets, true, this );
}
        
void US_Hydrodyn_Saxs_Buffer_Nth::i_avg_all()
{
}

void US_Hydrodyn_Saxs_Buffer_Nth::i_avg_sel()
{
}

void US_Hydrodyn_Saxs_Buffer_Nth::i_only()
{
}

void US_Hydrodyn_Saxs_Buffer_Nth::i_add()
{
}

