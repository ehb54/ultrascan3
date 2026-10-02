#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_xsr.h"
//Added by qt3to4:
#include <QTextStream>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
 //#include <Q3PopupMenu>
#include <QVBoxLayout>
#include <QBoxLayout>
#include <QCloseEvent>


// ------- the class to run xsr

US_Hydrodyn_Xsr::US_Hydrodyn_Xsr( 
                                 US_Hydrodyn  * us_hydrodyn,
                                 QWidget *p, 
                                 const char *
                                 ) : QFrame(  p )
{
   this->us_hydrodyn      = us_hydrodyn;
   this->our_saxs_options = & ( us_hydrodyn->saxs_options );
   this->saxs_widget      = & ( us_hydrodyn->saxs_plot_widget );
   this->saxs_window      = us_hydrodyn->saxs_plot_window;

   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle( us_tr( "US-SOMO: SAXS Cross Sectional Analysis" ) );

   setupGUI();
   running = false;

   update_enables();

   editor_msg("blue", "THIS WINDOW IS UNDER DEVELOPMENT" );

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry(global_Xpos, global_Ypos, 0, 0 );
}

US_Hydrodyn_Xsr::~US_Hydrodyn_Xsr()
{
}

void US_Hydrodyn_Xsr::setupGUI()
{
   int minHeight1 = 30;
#if !defined(Q_OS_MAC)
   int minHeight3 = 30;
#endif

   lbl_title = new QLabel( us_tr( "US-SOMO: SAXS Cross Sectional Analysis" ), this);
   lbl_title->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette( PALET_FRAME );
   AUTFBACK( lbl_title );
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   pb_start = new QPushButton(us_tr("Start"), this);
   pb_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_start->setMinimumHeight(minHeight1);
   pb_start->setPalette( PALET_PUSHB );
   connect(pb_start, SIGNAL(clicked()), SLOT(start()));

   pb_stop = new QPushButton(us_tr("Stop"), this);
   pb_stop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_stop->setMinimumHeight(minHeight1);
   pb_stop->setPalette( PALET_PUSHB );
   connect(pb_stop, SIGNAL(clicked()), SLOT(stop()));

   progress = new QProgressBar( this );
   progress->setMinimumHeight(minHeight1);
   progress->setPalette( PALET_NORMAL );
   AUTFBACK( progress );
   progress->reset();

   editor = new QTextEdit(this);
   editor->setPalette( PALET_NORMAL );
   AUTFBACK( editor );
   editor->setReadOnly(true);

# if defined(Q_OS_MAC)
   m = new QMenuBar( this );
   m->setObjectName( "menu" );
# else
   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight3);
   frame->setPalette( PALET_NORMAL );
   AUTFBACK( frame );
   m = new QMenuBar( frame );    m->setObjectName( "menu" );
# endif
   m->setMinimumHeight(minHeight1 - 5);
   m->setPalette( PALET_NORMAL );
   AUTFBACK( m );

   {
      QMenu * new_menu = m->addMenu( us_tr( "&File" ) );

      QAction *qa1 = new_menu->addAction( us_tr( "Font" ) );
      qa1->setShortcut( Qt::ALT+Qt::Key_F );
      connect( qa1, SIGNAL(triggered()), this, SLOT( update_font() ) );

      QAction *qa2 = new_menu->addAction( us_tr( "Save" ) );
      qa2->setShortcut( Qt::ALT+Qt::Key_S );
      connect( qa2, SIGNAL(triggered()), this, SLOT( save() ) );

      QAction *qa3 = new_menu->addAction( us_tr( "Clear Display" ) );
      qa3->setShortcut( Qt::ALT+Qt::Key_X );
      connect( qa3, SIGNAL(triggered()), this, SLOT( clear_display() ) );
   }
   
   editor->setWordWrapMode (QTextOption::WordWrap);
   // editor->setMinimumHeight(300);
   
   pb_help = new QPushButton(us_tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_cancel = new QPushButton(us_tr("Close"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   // build layout
   // grid for options

   QBoxLayout * vbl_editor_group = new QVBoxLayout( 0 ); vbl_editor_group->setContentsMargins( 0, 0, 0, 0 ); vbl_editor_group->setSpacing( 0 );
#if !defined(Q_OS_MAC)
   vbl_editor_group->addWidget( frame      );
#endif
   vbl_editor_group->addWidget( editor     );

   QHBoxLayout * hbl_controls = new QHBoxLayout(); hbl_controls->setContentsMargins( 0, 0, 0, 0 ); hbl_controls->setSpacing( 0 );
   hbl_controls->addSpacing(4);
   hbl_controls->addWidget(pb_start);
   hbl_controls->addSpacing(4);
   hbl_controls->addWidget(progress);
   hbl_controls->addWidget(pb_stop);
   hbl_controls->addSpacing(4);

   QVBoxLayout * vbl_target_controls = new QVBoxLayout( 0 ); vbl_target_controls->setContentsMargins( 0, 0, 0, 0 ); vbl_target_controls->setSpacing( 0 );
   vbl_target_controls->addLayout( hbl_controls );

   QHBoxLayout * hbl_bottom = new QHBoxLayout(); hbl_bottom->setContentsMargins( 0, 0, 0, 0 ); hbl_bottom->setSpacing( 0 );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_cancel );
   hbl_bottom->addSpacing( 4 );

   QVBoxLayout * background = new QVBoxLayout(this); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 );
   background->addSpacing( 4 );
   background->addWidget ( lbl_title );
   background->addSpacing( 4 );
   background->addLayout ( vbl_editor_group );
   background->addSpacing( 4 );
   background->addLayout ( vbl_target_controls );
   background->addSpacing( 4 );
   background->addLayout ( hbl_bottom );
   background->addSpacing( 4 );
}

void US_Hydrodyn_Xsr::cancel()
{
   close();
}

void US_Hydrodyn_Xsr::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo/somo_saxs_xsr.html");
}

void US_Hydrodyn_Xsr::closeEvent( QCloseEvent *e )
{
   // ((US_Hydrodyn *)us_hydrodyn)->saxs_2d_widget = false;

   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Xsr::clear_display()
{
   editor->clear( );
   editor->append("\n\n");
}

void US_Hydrodyn_Xsr::update_font()
{
   bool ok;
   QFont newFont;
   newFont = QFontDialog::getFont( &ok, ft, this );
   if ( ok )
   {
      ft = newFont;
   }
   editor->setFont(ft);
}

void US_Hydrodyn_Xsr::save()
{
   QString fn;
   fn = QFileDialog::getSaveFileName( this , windowTitle() , QString() , QString() );
   if(!fn.isEmpty() )
   {
      QString text = editor->toPlainText();
      QFile f( fn );
      if ( !f.open( QIODevice::WriteOnly | QIODevice::Text) )
      {
         return;
      }
      QTextStream t( &f );
      t << text;
      f.close();
 //      editor->setModified( false );
      setWindowTitle( fn );
   }
}

void US_Hydrodyn_Xsr::start()
{
}

void US_Hydrodyn_Xsr::stop()
{
   running = false;
   editor_msg("red", "Stopped by user request\n");
   update_enables();
}

void US_Hydrodyn_Xsr::update_enables()
{
   pb_start            ->setEnabled( !running );
   pb_stop             ->setEnabled( running );
}

void US_Hydrodyn_Xsr::editor_msg( QString color, QString msg )
{
   QColor save_color = editor->textColor();
   editor->setTextColor(color);
   editor->append(msg);
   editor->setTextColor(save_color);
}

// ------ US_Hydrodyn_Saxs entry 

void US_Hydrodyn_Saxs::saxs_xsr()
{

   if ( ( ( US_Hydrodyn * ) us_hydrodyn )->sas_options_xsr_widget )
   {
      if ( ((US_Hydrodyn *)us_hydrodyn)->sas_options_xsr_window->isVisible() )
      {
         ((US_Hydrodyn *)us_hydrodyn)->sas_options_xsr_window->raise();
      }
      else
      {
         ((US_Hydrodyn *)us_hydrodyn)->sas_options_xsr_window->show();
      }
      return;
   }
   else
   {
      ((US_Hydrodyn *)us_hydrodyn)->sas_options_xsr_window = new US_Hydrodyn_SasOptionsXsr( 
                                                                                           our_saxs_options, 
                                                                                           & ( ( ( US_Hydrodyn * ) us_hydrodyn )->sas_options_xsr_widget ), 
                                                                                           us_hydrodyn );
      US_Hydrodyn::fixWinButtons( ((US_Hydrodyn *)us_hydrodyn)->sas_options_xsr_window );
      ((US_Hydrodyn *)us_hydrodyn)->sas_options_xsr_window->show();
   }

   US_Hydrodyn_Xsr * uhxsr = new US_Hydrodyn_Xsr( ( US_Hydrodyn * ) us_hydrodyn );
   US_Hydrodyn::fixWinButtons( uhxsr );
   uhxsr->show();
}
