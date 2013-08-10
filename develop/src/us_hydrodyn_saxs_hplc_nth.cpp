#include "../include/us_hydrodyn_saxs_hplc_nth.h"
#include <set>

US_Hydrodyn_Saxs_Hplc_Nth::US_Hydrodyn_Saxs_Hplc_Nth(
                                                     void                     *              us_hydrodyn_saxs_hplc,
                                                     map < QString, QString > *              parameters,
                                                     QWidget *                               p,
                                                     const char *                            name
                                                     ) : QDialog( p, name )
{
   this->us_hydrodyn_saxs_hplc                = us_hydrodyn_saxs_hplc;
   this->parameters                           = parameters;

   USglobal = new US_Config();
   setPalette( QPalette( USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame ) );
   setCaption( tr( "US-SOMO: SAXS HPLC : Select every Nth file" ) );

   setupGUI();
   update_enables();
   update_files_selected();

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry( global_Xpos, global_Ypos, 0, 0 );
}

US_Hydrodyn_Saxs_Hplc_Nth::~US_Hydrodyn_Saxs_Hplc_Nth()
{
}

void US_Hydrodyn_Saxs_Hplc_Nth::setupGUI()
{
   int minHeight1  = 24;

   lbl_title =  new QLabel      ( caption(), this );
   lbl_title -> setAlignment    ( Qt::AlignCenter | Qt::AlignVCenter );
   lbl_title -> setMinimumHeight( minHeight1 );
   lbl_title -> setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_title -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold ) );

   lbl_files =  new QLabel      ( tr( "Complete list of data files" ), this );
   lbl_files -> setAlignment    ( Qt::AlignCenter | Qt::AlignVCenter );
   lbl_files -> setPalette      ( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_files -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Normal) );

   lb_files =  new QListBox(this, "files files listbox" );
   lb_files -> setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lb_files -> setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lb_files -> setEnabled(true);
   lb_files -> setSelectionMode( QListBox::Extended );
   lb_files -> setMinimumHeight( minHeight1 * 8 );

   for ( int i = 0; i < ((US_Hydrodyn_Saxs_Hplc*)us_hydrodyn_saxs_hplc)->lb_files->numRows(); ++i )
   {
      lb_files->insertItem( QString( "%1 : %2" ).arg( i + 1 ).arg( ((US_Hydrodyn_Saxs_Hplc*)us_hydrodyn_saxs_hplc)->lb_files->text( i ) ) );
      if ( ((US_Hydrodyn_Saxs_Hplc*)us_hydrodyn_saxs_hplc)->lb_files->isSelected( i ) )
      {
         lb_files->setSelected( i , true );
      }
   }

   connect( lb_files, SIGNAL( selectionChanged() ), SLOT( update_files_selected() ) );

   lbl_files_sel =  new QLabel      ( tr( "Selected data files" ), this );
   lbl_files_sel -> setAlignment    ( Qt::AlignCenter | Qt::AlignVCenter );
   lbl_files_sel -> setPalette      ( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_files_sel -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Normal) );

   lb_files_sel =  new QListBox(this, "files_sel files_sel listbox" );
   lb_files_sel -> setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lb_files_sel -> setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lb_files_sel -> setEnabled(true);
   lb_files_sel -> setSelectionMode( QListBox::NoSelection );
   lb_files_sel -> setMinimumHeight( minHeight1 * 8 );


   lbl_files_selected =  new QLabel      ( "", this );
   lbl_files_selected -> setAlignment    ( Qt::AlignCenter | Qt::AlignVCenter );
   lbl_files_selected -> setPalette      ( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_files_selected -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Normal) );

   lbl_n =  new QLabel      ( tr( "Select every Nth:" ), this );
   lbl_n -> setAlignment    ( Qt::AlignRight | Qt::AlignVCenter );
   lbl_n -> setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_n -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   le_n = new QLineEdit(this, "le_n Line Edit");
   le_n->setText( "2" );
   le_n->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_n->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_n->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QIntValidator *qdv = new QIntValidator( 2, ((US_Hydrodyn_Saxs_Hplc*)us_hydrodyn_saxs_hplc)->lb_files->numRows(), le_n );
      le_n->setValidator( qdv );
   }
   connect( le_n, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_n->setMinimumWidth( 80 );

   lbl_start =  new QLabel      ( tr( "Starting curve offset:" ), this );
   lbl_start -> setAlignment    ( Qt::AlignRight | Qt::AlignVCenter );
   lbl_start -> setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_start -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   le_start = new QLineEdit(this, "le_start Line Edit");
   le_start->setText( "1" );
   le_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_start->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QIntValidator *qdv = new QIntValidator( 1, ((US_Hydrodyn_Saxs_Hplc*)us_hydrodyn_saxs_hplc)->lb_files->numRows(), le_start );
      le_start->setValidator( qdv );
   }
   connect( le_start, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_start->setMinimumWidth( 80 );
   
   lbl_start_name =  new QLabel      ( ( (US_Hydrodyn_Saxs_Hplc*)us_hydrodyn_saxs_hplc)->lb_files->text( le_start->text().toInt() - 1 ), this );
   lbl_start_name -> setAlignment    ( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_start_name -> setPalette      ( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_start_name -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Normal) );

   lbl_end =  new QLabel      ( tr( "Ending curve offset:" ), this );
   lbl_end -> setAlignment    ( Qt::AlignRight | Qt::AlignVCenter );
   lbl_end -> setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_end -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   le_end = new QLineEdit(this, "le_end Line Edit");
   le_end->setText( QString( "%1" ).arg( ((US_Hydrodyn_Saxs_Hplc*)us_hydrodyn_saxs_hplc)->lb_files->numRows() ) );
   le_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_end->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QIntValidator *qdv = new QIntValidator( 2, ((US_Hydrodyn_Saxs_Hplc*)us_hydrodyn_saxs_hplc)->lb_files->numRows(), le_end );
      le_end->setValidator( qdv );
   }
   connect( le_end, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_end->setMinimumWidth( 80 );

   lbl_end_name =  new QLabel      ( ( (US_Hydrodyn_Saxs_Hplc*)us_hydrodyn_saxs_hplc)->lb_files->text( le_end->text().toInt() - 1 ), this );
   lbl_end_name -> setAlignment    ( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_end_name -> setPalette      ( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_end_name -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Normal) );

   pb_select_only =  new QPushButton ( tr( "Select Only" ), this );
   pb_select_only -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_select_only -> setMinimumHeight( minHeight1 );
   pb_select_only -> setPalette      ( QPalette( USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active ) );
   connect( pb_select_only, SIGNAL( clicked() ), SLOT( select_only() ) );

   pb_select_add =  new QPushButton ( tr( "Select Additionally" ), this );
   pb_select_add -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_select_add -> setMinimumHeight( minHeight1 );
   pb_select_add -> setPalette      ( QPalette( USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active ) );
   connect( pb_select_add, SIGNAL( clicked() ), SLOT( select_add() ) );

   pb_help =  new QPushButton ( tr( "Help" ), this );
   pb_help -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_help -> setMinimumHeight( minHeight1 );
   pb_help -> setPalette      ( QPalette( USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );

   pb_quit =  new QPushButton ( tr( "Quit" ), this );
   pb_quit -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_quit -> setMinimumHeight( minHeight1 );
   pb_quit -> setPalette      ( QPalette( USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active ) );
   connect( pb_quit, SIGNAL( clicked() ), SLOT( quit() ) );

   pb_go =  new QPushButton ( tr( "Select in main window" ), this );
   pb_go -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_go -> setMinimumHeight( minHeight1 );
   pb_go -> setPalette      ( QPalette( USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active ) );
   connect( pb_go, SIGNAL( clicked() ), SLOT( go() ) );

   QVBoxLayout *background = new QVBoxLayout( this );
   background->addWidget( lbl_title );


   QHBoxLayout *hbl_files_pane = new QHBoxLayout( 0 );
   {
      QVBoxLayout *vbl_files = new QVBoxLayout( 0 );
      vbl_files->addWidget( lbl_files );
      vbl_files->addWidget( lb_files );
      hbl_files_pane->addLayout( vbl_files );
   }
   {
      QVBoxLayout *vbl_files_sel = new QVBoxLayout( 0 );
      vbl_files_sel->addWidget( lbl_files_sel );
      vbl_files_sel->addWidget( lb_files_sel );
      hbl_files_pane->addLayout( vbl_files_sel );
   }

   background->addLayout( hbl_files_pane );
   background->addWidget( lbl_files_selected );

   QGridLayout *gl_bl = new QGridLayout( 0 );

   gl_bl->addWidget         ( lbl_n          , 0, 0 );
   gl_bl->addWidget         ( le_n           , 0, 1 );
   gl_bl->addWidget         ( lbl_start      , 1, 0 );
   gl_bl->addWidget         ( le_start       , 1, 1 );
   gl_bl->addWidget         ( lbl_start_name , 1, 2 );
   gl_bl->addWidget         ( lbl_end        , 2, 0 );
   gl_bl->addWidget         ( le_end         , 2, 1 );
   gl_bl->addWidget         ( lbl_end_name   , 2, 2 );

   background->addLayout( gl_bl );

   QHBoxLayout *hbl_sel = new QHBoxLayout( 0 );
   hbl_sel->addWidget ( pb_select_only );
   hbl_sel->addWidget ( pb_select_add );
   background->addLayout ( hbl_sel );

   QHBoxLayout *hbl_bottom = new QHBoxLayout( 0 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addWidget ( pb_quit );
   hbl_bottom->addWidget ( pb_go );

   background->addLayout ( hbl_bottom );
}

void US_Hydrodyn_Saxs_Hplc_Nth::quit()
{
   close();
}

void US_Hydrodyn_Saxs_Hplc_Nth::go()
{
   (*parameters)[ "go"    ] = "true";
   
   for ( int i = 0; i < lb_files->numRows(); ++i )
   {
      if ( lb_files->isSelected( i ) )
      {
         (*parameters)[ QString( "%1" ).arg( i ) ] = "1";
      }
   }
   close();
}

void US_Hydrodyn_Saxs_Hplc_Nth::help()
{
   US_Help *online_help;
   online_help = new US_Help( this );
   online_help->show_help("manual/saxs_hplc_bl.html");
}

void US_Hydrodyn_Saxs_Hplc_Nth::closeEvent( QCloseEvent *e )
{

   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Saxs_Hplc_Nth::update_enables()
{
   lbl_start_name->setText( ( (US_Hydrodyn_Saxs_Hplc*)us_hydrodyn_saxs_hplc)->lb_files->text( le_start->text().toInt() - 1 ) );
   lbl_end_name  ->setText( ( (US_Hydrodyn_Saxs_Hplc*)us_hydrodyn_saxs_hplc)->lb_files->text( le_end  ->text().toInt() - 1 ) );

   int n   = le_n->text().toInt();
   int ofs = le_start->text().toInt();
   int end = le_end->text().toInt();

   set < int > selected;

   for ( int i = 0; i < lb_files->numRows(); ++i )
   {
      if ( lb_files->isSelected( i ) )
      {
         selected.insert( i );
         
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

   pb_select_only->setEnabled( any_in_range_not_selected || any_selected_not_in_range );
   pb_select_add ->setEnabled( any_in_range_not_selected && any_selected_not_in_range );

   lbl_files_selected->setText( QString( "%1 of %2 selected" ).arg( files_selected ).arg( lb_files->numRows() ) );
}

void US_Hydrodyn_Saxs_Hplc_Nth::select_only()
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

void US_Hydrodyn_Saxs_Hplc_Nth::select_add()
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

void US_Hydrodyn_Saxs_Hplc_Nth::update_files_selected()
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
