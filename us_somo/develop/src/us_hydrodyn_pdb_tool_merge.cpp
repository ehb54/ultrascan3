#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_pdb_tool_merge.h"
//Added by qt3to4:
#include <QTextStream>
#include <QHBoxLayout>
#include <QLabel>
#include <QGridLayout>
#include <QFrame>
 //#include <Q3PopupMenu>
#include <QVBoxLayout>
#include <QBoxLayout>
#include <QCloseEvent>

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

#define SLASH QDir::separator()
#define USPTM_DEBUG
// #define USPTM_DEBUG_RUN_ONE
// #define USPTM_DEBUG2
// #define USPTM_TIMERS

ostream& operator<<(ostream& out, const range_entry& c)
{
   out << "chain: " << c.chain << " " << c.start << "-" << c.end;
   if ( c.gaps.size() )
   {
      out << " gaps:";
      map < unsigned int, bool > lg = c.gaps;
      for ( map  < unsigned int, bool >::iterator it = lg.begin();
            it != lg.end();
            it++ )
      {
         out << " " << it->first;
      }
   } else {
      out << " no gaps.";
   }

   return out;
}

class uhptm_sortable_uint {
public:
   unsigned int x;
   bool operator < (const uhptm_sortable_uint& objIn) const
   {
      return x < objIn.x;
   }
};

US_Hydrodyn_Pdb_Tool_Merge::US_Hydrodyn_Pdb_Tool_Merge(
                                                       void *us_hydrodyn, 
                                                       void *pdb_tool_window, 
                                                       QWidget *p, 
                                                       const char *
                                                       ) : QFrame( p )
{
   this->us_hydrodyn = us_hydrodyn;
   this->pdb_tool_window = pdb_tool_window;
   usu = ((US_Hydrodyn *)us_hydrodyn)->saxs_util;

   for ( map < QString, double >::iterator it = usu->atom_vdw.begin();
         it != usu->atom_vdw.end();
         ++it ) {
      us_qdebug( QString( "vdw %1 -> %2" ).arg( it->first ).arg( it->second ) );
   }

   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle(us_tr("US-SOMO: PDB Editor Cut/Splice Control"));

   ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->pdb_tool_merge_widget = true;
   lv_csv_from = ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->lv_csv;
   lv_csv_to   = ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->lv_csv2;

   reset_csv_commands();

   setupGUI();
   ((US_Hydrodyn*)us_hydrodyn)->fixWinButtons( this );

   running             = false;
   cache_range_ok      = false;
   extra_chains_done   = false;

   extra_chains_list.clear( );

   update_enables();

   editor_msg("blue", "THIS WINDOW IS UNDER DEVELOPMENT" );

   global_Xpos += 30;
   global_Ypos += 30;

   unsigned int csv_height = t_csv->rowHeight(0) + 30;
   unsigned int csv_width = t_csv->columnWidth(0) + 45;
   for ( int i = 0; i < t_csv->rowCount(); i++ )
   {
      csv_height += t_csv->rowHeight(i);
   }
   for ( int i = 1; i < t_csv->columnCount(); i++ )
   {
      csv_width += t_csv->columnWidth(i);
   }
   if ( csv_height > 700 )
   {
      csv_height = 700;
   }
   if ( csv_width > 1000 )
   {
      csv_width = 1000;
   }

   setGeometry(global_Xpos, global_Ypos, csv_width, 0 );
}

US_Hydrodyn_Pdb_Tool_Merge::~US_Hydrodyn_Pdb_Tool_Merge()
{
   ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->pdb_tool_merge_widget = false;
}

void US_Hydrodyn_Pdb_Tool_Merge::reset_csv_commands()
{
   csv_commands.name = "PDB Editor Cut/Splice Control";

   csv_commands.header.clear( );
   csv_commands.header_map.clear( );
   csv_commands.data.clear( );
   csv_commands.num_data.clear( );
   csv_commands.prepended_names.clear( );

   csv_commands.header.push_back("Chain");
   csv_commands.header.push_back("Merge start");
   csv_commands.header.push_back("Merge end");
   csv_commands.header.push_back("Fit start");
   csv_commands.header.push_back("Fit end");
   csv_commands.header.push_back("Cut start");
   csv_commands.header.push_back("Cut end");
}

void US_Hydrodyn_Pdb_Tool_Merge::setupGUI()
{
   int minHeight1 = 22;
#if QT_VERSION < 0x040000 || !defined(Q_OS_MAC)
   int minHeight3 = 30;
#endif

   lbl_title = new QLabel(csv_commands.name.left(80), this);
   lbl_title->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette( PALET_FRAME );
   AUTFBACK( lbl_title );
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   t_csv = new QTableWidget(csv_commands.data.size(), csv_commands.header.size(), this);
   t_csv->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   // t_csv->setMinimumHeight(minHeight1 * 3);
   // t_csv->setMinimumWidth(minWidth1);
   t_csv->setPalette( PALET_EDIT );
   AUTFBACK( t_csv );
   t_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   t_csv->setEnabled(true);

   for ( unsigned int i = 0; i < (unsigned int) csv_commands.header.size(); i++ )
   {
      t_csv->setHorizontalHeaderItem(i, new QTableWidgetItem( csv_commands.header[i]));
   }

   update_t_csv_data();

   t_csv->setSortingEnabled(false);
    t_csv->verticalHeader()->setSectionsMovable(true);
    t_csv->horizontalHeader()->setSectionsMovable(false);
   //{ for ( int i = 0; i < t_csv->rowCount(); ++i ) { for ( int j = 0; j < t_csv->columnCount(); ++j ) { t_csv->item( i, j )->setFlags( t_csv->item( i, j )->flags() ^ Qt::ItemIsEditable ); } } };

   //   t_csv->setColumnWidth(0, ?? );
   // { for ( int i = 0; i < t_csv->rowCount(); ++i ) { t_csv->item( i, t_csv->columnCount() - 1 )->setFlags( t_csv->item( i, t_csv->columnCount() - 1 )->flags() ^ Qt::ItemIsEditable ); } };

   { for ( int i = 0; i < t_csv->rowCount(); ++i ) { t_csv->item( i, 0 )->setFlags( t_csv->item( i, 0 )->flags() ^ Qt::ItemIsEditable ); } };
   t_csv->setSelectionMode( QAbstractItemView::MultiSelection );t_csv->setSelectionBehavior( QAbstractItemView::SelectRows );

   connect( t_csv, SIGNAL( cellChanged( int, int ) ), SLOT( table_value( int, int ) ) );
   connect( t_csv, SIGNAL( itemSelectionChanged() ), SLOT( update_enables() ) );

   pb_sel_auto = new QPushButton(us_tr("Compute Guess"), this);
   pb_sel_auto->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_sel_auto->setMinimumHeight(minHeight1);
   pb_sel_auto->setPalette( PALET_PUSHB );
   connect(pb_sel_auto, SIGNAL(clicked()), SLOT(sel_auto()));

   pb_sel_from_to_merge = new QPushButton(us_tr("From -> Merge"), this);
   pb_sel_from_to_merge->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_sel_from_to_merge->setMinimumHeight(minHeight1);
   pb_sel_from_to_merge->setPalette( PALET_PUSHB );
   connect(pb_sel_from_to_merge, SIGNAL(clicked()), SLOT(sel_from_to_merge()));

   pb_sel_from_to_fit = new QPushButton(us_tr("From -> Fit"), this);
   pb_sel_from_to_fit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_sel_from_to_fit->setMinimumHeight(minHeight1);
   pb_sel_from_to_fit->setPalette( PALET_PUSHB );
   connect(pb_sel_from_to_fit, SIGNAL(clicked()), SLOT(sel_from_to_fit()));

   pb_sel_to_to_fit = new QPushButton(us_tr("To -> Fit"), this);
   pb_sel_to_to_fit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_sel_to_to_fit->setMinimumHeight(minHeight1);
   pb_sel_to_to_fit->setPalette( PALET_PUSHB );
   connect(pb_sel_to_to_fit, SIGNAL(clicked()), SLOT(sel_to_to_fit()));

   pb_sel_to_to_cut = new QPushButton(us_tr("To -> Cut"), this);
   pb_sel_to_to_cut->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_sel_to_to_cut->setMinimumHeight(minHeight1);
   pb_sel_to_to_cut->setPalette( PALET_PUSHB );
   connect(pb_sel_to_to_cut, SIGNAL(clicked()), SLOT(sel_to_to_cut()));

   pb_extra_chains = new QPushButton(us_tr("Extra Chains"), this);
   pb_extra_chains->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_extra_chains->setMinimumHeight(minHeight1);
   pb_extra_chains->setPalette( PALET_PUSHB );
   connect(pb_extra_chains, SIGNAL(clicked()), SLOT(extra_chains()));

   pb_only_closest = new QPushButton(us_tr("Only Closest Chains"), this);
   pb_only_closest->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_only_closest->setMinimumHeight(minHeight1);
   pb_only_closest->setPalette( PALET_PUSHB );
   connect(pb_only_closest, SIGNAL(clicked()), SLOT(only_closest()));

   pb_delete_row = new QPushButton(us_tr("Delete Rows"), this);
   pb_delete_row->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_delete_row->setMinimumHeight(minHeight1);
   pb_delete_row->setPalette( PALET_PUSHB );
   connect(pb_delete_row, SIGNAL(clicked()), SLOT(delete_row()));

   pb_clear = new QPushButton(us_tr("Clear"), this);
   pb_clear->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_clear->setMinimumHeight(minHeight1);
   pb_clear->setPalette( PALET_PUSHB );
   connect(pb_clear, SIGNAL(clicked( )), SLOT(clear( )));

   pb_load = new QPushButton(us_tr("Load"), this);
   pb_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_load->setMinimumHeight(minHeight1);
   pb_load->setPalette( PALET_PUSHB );
   connect(pb_load, SIGNAL(clicked()), SLOT(load()));

   pb_validate = new QPushButton(us_tr("Validate"), this);
   pb_validate->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_validate->setMinimumHeight(minHeight1);
   pb_validate->setPalette( PALET_PUSHB );
   connect(pb_validate, SIGNAL(clicked()), SLOT(validate()));

   pb_csv_save = new QPushButton(us_tr("Save"), this);
   pb_csv_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_save->setMinimumHeight(minHeight1);
   pb_csv_save->setPalette( PALET_PUSHB );
   connect(pb_csv_save, SIGNAL(clicked()), SLOT(csv_save()));

   pb_chains_from = new QPushButton(us_tr("Chains From"), this);
   pb_chains_from->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_chains_from->setMinimumHeight(minHeight1);
   pb_chains_from->setPalette( PALET_PUSHB );
   connect(pb_chains_from, SIGNAL(clicked()), SLOT(chains_from()));

   le_chains_from = new QLineEdit( this );    le_chains_from->setObjectName( "chains_from Line Edit" );
   le_chains_from->setText("");
   le_chains_from->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   // le_chains_from->setMinimumWidth(150);
   le_chains_from->setPalette( PALET_NORMAL );
   AUTFBACK( le_chains_from );
   le_chains_from->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_chains_from->setReadOnly(true);

   pb_chains_to = new QPushButton(us_tr("Chains To"), this);
   pb_chains_to->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_chains_to->setMinimumHeight(minHeight1);
   pb_chains_to->setPalette( PALET_PUSHB );
   connect(pb_chains_to, SIGNAL(clicked()), SLOT(chains_to()));

   le_chains_to = new QLineEdit( this );    le_chains_to->setObjectName( "chains_to Line Edit" );
   le_chains_to->setText("");
   le_chains_to->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   // le_chains_to->setMinimumWidth(150);
   le_chains_to->setPalette( PALET_NORMAL );
   AUTFBACK( le_chains_to );
   le_chains_to->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_chains_to->setReadOnly(true);

   pb_target = new QPushButton(us_tr("Target"), this);
   pb_target->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_target->setMinimumHeight(minHeight1);
   pb_target->setPalette( PALET_PUSHB );
   connect(pb_target, SIGNAL(clicked()), SLOT(target()));

   le_target = new QLineEdit( this );    le_target->setObjectName( "target Line Edit" );
   le_target->setText("");
   le_target->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   // le_target->setMinimumWidth(150);
   le_target->setPalette( PALET_NORMAL );
   AUTFBACK( le_target );
   le_target->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_target->setReadOnly(true);

   cb_filter = new QCheckBox( this );
   cb_filter -> setText( us_tr( "Filter out steric clashes from results" ) );
   cb_filter -> setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_filter -> setPalette( PALET_NORMAL );
   AUTFBACK( cb_filter );

   progress = new QProgressBar( this );
   progress->setMinimumHeight(minHeight1);
   progress->setPalette( PALET_NORMAL );
   AUTFBACK( progress );
   progress->reset();

   pb_start = new QPushButton(us_tr("Start"), this);
   pb_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_start->setMinimumHeight(minHeight1);
   pb_start->setPalette( PALET_PUSHB );
   connect(pb_start, SIGNAL(clicked()), SLOT(start()));

   pb_trial = new QPushButton(us_tr("Trial"), this);
   pb_trial->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_trial->setMinimumHeight(minHeight1);
   pb_trial->setPalette( PALET_PUSHB );
   connect(pb_trial, SIGNAL(clicked()), SLOT(trial()));

   pb_stop = new QPushButton(us_tr("Stop"), this);
   pb_stop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_stop->setMinimumHeight(minHeight1);
   pb_stop->setPalette( PALET_PUSHB );
   connect(pb_stop, SIGNAL(clicked()), SLOT(stop()));

   editor = new QTextEdit(this);
   editor->setPalette( PALET_NORMAL );
   AUTFBACK( editor );
   editor->setReadOnly(true);

#if QT_VERSION < 0x040000
# if QT_VERSION >= 0x040000 && defined(Q_OS_MAC)
   {
 //      Q3PopupMenu * file = new Q3PopupMenu;
      file->insertItem( us_tr("&Font"),  this, SLOT(update_font()),    Qt::ALT+Qt::Key_F );
      file->insertItem( us_tr("&Save"),  this, SLOT(save()),    Qt::ALT+Qt::Key_S );
      file->insertItem( us_tr("Clear Display"), this, SLOT(clear_display()),   Qt::ALT+Qt::Key_X );

      QMenuBar *menu = new QMenuBar( this );
      AUTFBACK( menu );

      menu->insertItem(us_tr("&Messages"), file );
   }
# else
   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight3);

   m = new QMenuBar( frame );    m->setObjectName( "menu" );
   m->setMinimumHeight(minHeight1 - 5);
   m->setPalette( PALET_NORMAL );
   AUTFBACK( m );
 //   Q3PopupMenu * file = new Q3PopupMenu(editor);
   m->insertItem( us_tr("&File"), file );
   file->insertItem( us_tr("Font"),  this, SLOT(update_font()),    Qt::ALT+Qt::Key_F );
   file->insertItem( us_tr("Save"),  this, SLOT(save()),    Qt::ALT+Qt::Key_S );
   file->insertItem( us_tr("Clear Display"), this, SLOT(clear_display()),   Qt::ALT+Qt::Key_X );
# endif
#else
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
#endif


   editor->setWordWrapMode (QTextOption::WordWrap);
   editor->setMinimumHeight(100);
   
   pb_help = new QPushButton(us_tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_pdb_tool = new QPushButton(us_tr("PDB Editor"), this);
   pb_pdb_tool->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_pdb_tool->setMinimumHeight(minHeight1);
   pb_pdb_tool->setPalette( PALET_PUSHB );
   connect(pb_pdb_tool, SIGNAL(clicked()), SLOT(pdb_tool()));

   pb_cancel = new QPushButton(us_tr("Close"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   // build layout

   QGridLayout * gl_files = new QGridLayout(0); gl_files->setContentsMargins( 0, 0, 0, 0 ); gl_files->setSpacing( 0 );

   gl_files->addWidget( pb_chains_from, 0, 0 );
   gl_files->addWidget( le_chains_from, 0, 1 );
   gl_files->addWidget( pb_chains_to  , 1, 0 );
   gl_files->addWidget( le_chains_to  , 1, 1 );
   gl_files->addWidget( pb_target     , 2, 0 );
   gl_files->addWidget( le_target     , 2, 1 );

   QHBoxLayout * hbl_sel_cmds = new QHBoxLayout(); hbl_sel_cmds->setContentsMargins( 0, 0, 0, 0 ); hbl_sel_cmds->setSpacing( 0 );
   hbl_sel_cmds->addWidget( pb_sel_auto );
   hbl_sel_cmds->addWidget( pb_sel_from_to_merge );
   hbl_sel_cmds->addWidget( pb_sel_from_to_fit );
   hbl_sel_cmds->addWidget( pb_sel_to_to_fit );
   hbl_sel_cmds->addWidget( pb_sel_to_to_cut );

   QHBoxLayout * hbl_extra_chains = new QHBoxLayout(); hbl_extra_chains->setContentsMargins( 0, 0, 0, 0 ); hbl_extra_chains->setSpacing( 0 );
   hbl_extra_chains->addWidget( pb_extra_chains );
   hbl_extra_chains->addWidget( pb_only_closest );
   hbl_extra_chains->addWidget( pb_delete_row );

   QHBoxLayout * hbl_load_save = new QHBoxLayout(); hbl_load_save->setContentsMargins( 0, 0, 0, 0 ); hbl_load_save->setSpacing( 0 );
   hbl_load_save->addWidget( pb_clear );
   hbl_load_save->addWidget( pb_load );
   hbl_load_save->addWidget( pb_validate );
   hbl_load_save->addWidget( pb_csv_save );

   QHBoxLayout * hbl_controls = new QHBoxLayout(); hbl_controls->setContentsMargins( 0, 0, 0, 0 ); hbl_controls->setSpacing( 0 );
   hbl_controls->addSpacing( 2 );
   hbl_controls->addWidget ( pb_start );
   hbl_controls->addSpacing( 2 );
   hbl_controls->addWidget ( pb_trial );
   hbl_controls->addSpacing( 2 );
   hbl_controls->addWidget ( pb_stop );
   hbl_controls->addSpacing( 2 );

   QHBoxLayout * hbl_bottom = new QHBoxLayout(); hbl_bottom->setContentsMargins( 0, 0, 0, 0 ); hbl_bottom->setSpacing( 0 );
   hbl_bottom->addSpacing( 2 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addSpacing( 2 );
   hbl_bottom->addWidget ( pb_pdb_tool );
   hbl_bottom->addSpacing( 2 );
   hbl_bottom->addWidget ( pb_cancel );
   hbl_bottom->addSpacing( 2 );

   QBoxLayout * vbl_editor_group = new QVBoxLayout(0); vbl_editor_group->setContentsMargins( 0, 0, 0, 0 ); vbl_editor_group->setSpacing( 0 );
#if QT_VERSION < 0x040000 || !defined(Q_OS_MAC)
   vbl_editor_group->addWidget( frame );
#endif
   vbl_editor_group->addWidget( editor );

   QVBoxLayout * background = new QVBoxLayout(this); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 );
   background->addSpacing( 2 );
   background->addWidget ( lbl_title );
   background->addSpacing( 2 );
   background->addWidget ( t_csv );
   background->addSpacing( 2 );
   background->addLayout ( hbl_sel_cmds );
   background->addSpacing( 2 );
   background->addLayout ( hbl_extra_chains );
   background->addSpacing( 2 );
   background->addLayout ( hbl_load_save );
   background->addSpacing( 2 );
   background->addLayout ( vbl_editor_group );
   background->addSpacing( 2 );
   background->addLayout ( gl_files );
   background->addSpacing( 2 );
   background->addWidget ( cb_filter );
   background->addSpacing( 2 );
   background->addWidget ( progress );
   background->addSpacing( 2 );
   background->addLayout ( hbl_controls );
   background->addSpacing( 2 );
   background->addLayout ( hbl_bottom );
   background->addSpacing( 2 );
}

void US_Hydrodyn_Pdb_Tool_Merge::cancel()
{
   close();
}

void US_Hydrodyn_Pdb_Tool_Merge::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo/somo_pdb_editor_merge.html");
}

void US_Hydrodyn_Pdb_Tool_Merge::closeEvent(QCloseEvent *e)
{
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Pdb_Tool_Merge::clear_display()
{
   editor->clear( );
   editor->append("\n\n");
}

void US_Hydrodyn_Pdb_Tool_Merge::update_font()
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

void US_Hydrodyn_Pdb_Tool_Merge::save()
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

void US_Hydrodyn_Pdb_Tool_Merge::start()
{
   running = true;
   update_enables();

   ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv2_clear( );
   QString filename = le_chains_to->text();
   ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->load( lv_csv_to,
                                                    filename,
                                                    true );

   if ( !validate_commands() )
   {
      running = false;
      update_enables();
      editor_msg( "red", "Validation errors" );
      return;
   }
   editor_msg( "dark blue", "Validation OK" );

   // process

   QFile f( le_chains_to->text() );

   if ( !f.open( QIODevice::ReadOnly ) )
   {
      QMessageBox::warning( this,
                            us_tr("Could not open file"),
                            QString("An error occured when trying to open file\n"
                                    "%1\n"
                                    "Please check the permissions and try again\n")
                            .arg( f.fileName() )
                            );
      running = false;
      update_enables();
      return;
   }

   QRegExp rx_model("^MODEL");
   QRegExp rx_end("^END");
   QRegExp rx_save_header("^("
                          "HEADER|"
                          "TITLE|"
                          "COMPND|"
                          "SOURCE|"
                          "KEYWDS|"
                          "AUTHOR|"
                          "REVDAT|"
                          "JRNL|"
                          "REMARK|"
                          "SEQRES|"
                          "SHEET|"
                          "HELIX|"
                          "SSBOND|"
                          "DBREF|"
                          "ORIGX|"
                          "SCALE"
                          ")\\.*" );
   
   unsigned int model_count = 0;

   editor_msg( "dark blue", QString( us_tr( "Checking file %1" ).arg( f.fileName() ) ) );

   map    < QString, bool >     model_names;
   vector < QString >           model_name_vector;
   unsigned int                 max_model_name_len = 0;
   QString                      model_header;
   map    < QString, QString >  model_remarks;
   QString                      last_model_name;
   bool                         in_model = false;

   bool dup_model_name_msg_done = false;
   
   {
      QTextStream ts( &f );
      unsigned int line_count = 0;
   
      while ( !ts.atEnd() )
      {
         QString qs = ts.readLine();
         line_count++;
         if ( line_count && !(line_count % 100000 ) )
         {
            editor_msg( "dark blue", QString( us_tr( "Lines read %1" ).arg( line_count ) ) );
            qApp->processEvents();
         }
         if ( qs.contains( rx_save_header ) )
         {
            if ( in_model )
            {
               model_remarks[ last_model_name ] += qs + "\n";
            } else {
               model_header += qs + "\n";
            }
         }
         
         if ( qs.contains( rx_model ) )
         {
            model_count++;
            QStringList qsl = (qs.left(20).split( QRegExp("\\s+") , Qt::SkipEmptyParts ) );
            QString model_name;
            if ( qsl.size() == 1 )
            {
               model_name = QString("%1").arg( model_count );
            } else {
               model_name = qsl[1];
            }
            if ( model_names.count( model_name ) )
            {
               unsigned int mext = 1;
               QString use_model_name;
               do {
                  use_model_name = model_name + QString("-%1").arg( mext );
               } while ( model_names.count( use_model_name ) );
               model_name = use_model_name;
               if ( !dup_model_name_msg_done )
               {
                  dup_model_name_msg_done = true;
                  // editor_msg( "red", us_tr( "Duplicate or missing model names found, -# extensions added" ) );
               }
            }
            last_model_name = model_name;
            in_model = true;
            model_names[ model_name ] = true;
            model_name_vector.push_back ( model_name );
            if ( (unsigned int) model_name.length() > max_model_name_len )
            {
               max_model_name_len = model_name.length();
            }
         }
      }
   }

   f.close();

   bool no_model = false;

   if ( model_count == 0 )
   {
      model_count = 1;
      no_model = true;
   }

   editor_msg( "dark blue", QString( us_tr( "File %1 contains %2 models" ) ).arg( f.fileName() ).arg( model_count ) );

   progress->setValue( 1 ); progress->setMaximum( model_count + 1 );
   qApp->processEvents();

   if ( QFile::exists( le_target->text() ) )
   {
      le_target->setText( ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( le_target->text(), 0, this ) );
   }

   QFile fn( le_target->text() );

   if ( !fn.open( QIODevice::WriteOnly ) )
   {
      QMessageBox::warning( this, "US-SOMO: PDB Editor : Cut/Splice",
                            QString(us_tr("Could not open %1 for writing!")).arg( fn.fileName() ) );

      running = false;
      update_enables();
      return;
   }

   QFile ff( le_target->text().replace( QRegExp( ".pdb$", Qt::CaseInsensitive ) , "" ) + "_filtered.pdb" );
   if ( cb_filter->isChecked() && !ff.open( QIODevice::WriteOnly ) )
   {
      QMessageBox::warning( this, "US-SOMO: PDB Editor : Cut/Splice",
                            QString(us_tr("Could not open %1 for writing!")).arg( ff.fileName() ) );

      running = false;
      update_enables();
      return;
   }

   if ( !f.open( QIODevice::ReadOnly ) )
   {
      QMessageBox::warning( this,
                            us_tr("Could not open file"),
                            QString("An error occured when trying to open file\n"
                                    "%1\n"
                                    "Please check the permissions and try again\n")
                            .arg( f.fileName() )
                            );

      running = false;
      update_enables();
      fn.close();
      return;
   }

   QTextStream ts( &f );
   QTextStream tso( &fn );
   QTextStream tsof( &ff );

   tso << QString("HEADER    Processed by US-SOMO Cut/Splice\n");
   tso << model_header;

   if ( cb_filter->isChecked() ) {
      tsof << QString("HEADER    Processed by US-SOMO Cut/Splice and *filtered* due to steric clashes\n");
      tsof << model_header;
   }      

   QStringList   model_lines;
   unsigned int  pos = 0;
   in_model = no_model;

   if ( !ts.atEnd() )
   {
      do 
      {
         if ( !running )
         {
            fn.close();
            return;
         }

         QString qs = ts.readLine();
         if ( qs.contains( rx_model ) || qs.contains( rx_end ) || ts.atEnd() )
         {
            if ( model_lines.size() )
            {
               progress->setValue( pos ); progress->setMaximum( model_count + 1 );
               qApp->processEvents();
               // process model
               if ( !no_model )
               {
                  editor_msg( "dark blue", QString( "Processing model %1\n" ).arg( model_name_vector[ pos ] ) );
               }
               qApp->processEvents();
               ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv2_clear( );
               ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->load_from_qsl( lv_csv_to, model_lines, QString( "model %1\n" ).arg( no_model ? "1" : model_name_vector[ pos ] ) );
               qApp->processEvents();
               run_one();
               if ( !running )
               {
                  fn.close();
                  return;
               }
               
               if ( filtered ) {
                  editor_msg( "darkred", QString( "Model %1 filtered due to steric clashes\n" ).arg( model_name_vector[ pos ] ) );
                  if ( cb_filter->isChecked() ) {
                     if ( !no_model )
                     {
                        tsof << QString("").sprintf("MODEL  %7s\n", model_name_vector[ pos ].toLatin1().data() );
                        tsof << model_remarks[ model_name_vector[ pos ] ];
                     }
                     tsof << ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv_to_pdb( csv_to, true );
                     if ( !no_model )
                     {
                        tsof << "ENDMDL\n";
                     }
                  }
               } else {
                  if ( !no_model )
                  {
                     tso << QString("").sprintf("MODEL  %7s\n", model_name_vector[ pos ].toLatin1().data() );
                     tso << model_remarks[ model_name_vector[ pos ] ];
                  }
                  tso << ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv_to_pdb( csv_to, true );
                  if ( !no_model )
                  {
                     tso << "ENDMDL\n";
                  }
               }
               in_model = false;
               model_lines.clear( );
               if ( !no_model )
               {
                  editor_msg( "dark blue", QString( "Processing model %1 done\n" ).arg( model_name_vector[ pos ] ) );
               }
               pos++;
            }
            if ( qs.contains( rx_model ) )
            {
               in_model = true;
               model_lines.clear( );
            }
         } else {
            if ( in_model )
            {
               model_lines << qs;
            }
         }
      } while ( !ts.atEnd() );
   }
   tso << "END\n";
   f.close();
   fn.close();
   if ( cb_filter->isChecked() ) {
      tsof << "END\n";
      ff.close();
   }
   progress->setValue( 1 ); progress->setMaximum( 1 );
   editor_msg( "black" , QString("File written: %1").arg( fn.fileName() ) );
   if ( cb_filter->isChecked() ) {
      editor_msg( "black" , QString("File written: %1").arg( ff.fileName() ) );
   }
   running = false;
   update_enables();

}

void US_Hydrodyn_Pdb_Tool_Merge::trial()
{
   running = true;
   update_enables();

   ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv2_clear( );
   QString filename = le_chains_to->text();
   ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->load( lv_csv_to,
                                                    filename,
                                                    true );
   if ( !validate_commands() )
   {
      running = false;
      update_enables();
      editor_msg( "red", "Validation errors" );
      return;
   }
   editor_msg( "dark blue", "Validation OK" );

   // process trial

   // clear, reload pdbs into pdb_tool ? assume for now ok

   // grab from & to csvs

   csv csv_from = 
      ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->to_csv( lv_csv_from, 
                                                         ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv1 ,
                                                         false );

   // editor_msg("blue", "from csv name: " + csv_from.name );

   csv csv_to =
      ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->to_csv( lv_csv_to, 
                                                         ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv2[ ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv2_pos ] ,
                                                         false );

   // editor_msg("blue", "to csv name: " + csv_to.name );

   update_csv_commands_from_table();

   map < QString, range_entry > merge_map;
   map < QString, range_entry > fit_map;
   map < QString, range_entry > cut_map;

   map < QString, vector < QString > > cross_fit_chains;
   map < QString, vector < QString > > cross_cross_chains;

   range_entry merge_range;
   range_entry fit_range;
   range_entry cut_range;

   bool any_cuts = false;
   for ( unsigned int i = 0; i < (unsigned int) csv_commands.data.size(); i++ )
   {
      if ( csv_commands.data[ i ].size() < 7 ||
           csv_commands.num_data[ i ].size() < 7 )
      {
         editor_msg( "red", us_tr( "Internal error: Invalid command directive format" ) );
         running = false;
         update_enables();
         return;
      }
      
      if ( csv_commands.data[ i ][ 0 ].length() <= 1 )
      {
         merge_range.chain = csv_commands.data[ i ][ 0 ];
         merge_range.start = (unsigned int) csv_commands.num_data[ i ][ 1 ];
         merge_range.end   = (unsigned int) csv_commands.num_data[ i ][ 2 ];

         fit_range.chain   = csv_commands.data[ i ][ 0 ];
         fit_range.start   = (unsigned int) csv_commands.num_data[ i ][ 3 ];
         fit_range.end     = (unsigned int) csv_commands.num_data[ i ][ 4 ];

         cut_range.chain   = csv_commands.data[ i ][ 0 ];
         cut_range.start   = (unsigned int) csv_commands.num_data[ i ][ 5 ];
         cut_range.end     = (unsigned int) csv_commands.num_data[ i ][ 6 ];

         merge_map[ merge_range.chain ] = merge_range;
         // cout << "merge map " << merge_range << endl;
         fit_map  [ fit_range  .chain ] = fit_range;
         // cout << "fit map " << fit_range << endl;
         if ( cut_range.start )
         {
            any_cuts = true;
            cut_map  [ cut_range  .chain ] = cut_range;
            // cout << "cut map " << cut_range << endl;
         }
      } else {
         merge_range.chain = csv_commands.data[ i ][ 0 ];
         merge_range.start = (unsigned int) csv_commands.num_data[ i ][ 1 ];
         merge_range.end   = (unsigned int) csv_commands.num_data[ i ][ 2 ];

         fit_range.chain   = csv_commands.data[ i ][ 0 ];
         fit_range.start   = (unsigned int) csv_commands.num_data[ i ][ 3 ];
         fit_range.end     = (unsigned int) csv_commands.num_data[ i ][ 4 ];

         merge_map[ merge_range.chain ] = merge_range;
         fit_map  [ fit_range  .chain ] = fit_range;
         
         QString fit_chain;
         QString cross_chain;
         if ( !get_chains( merge_range.chain, fit_chain, cross_chain ) )
         {
            editor_msg( "red", QString( us_tr( "Internal error: Extra chain not in proper format <%1>, please label each chain" ) ).arg( merge_range.chain ) );
            continue;
         }
         cross_fit_chains  [ fit_chain   ].push_back( cross_chain );
         cross_cross_chains[ cross_chain ].push_back( fit_chain   );
      }
   }

   // check cross fit chains ranges,
   // if they are in the merge ranges, copy over the to range for the base chain

   for ( map < QString, range_entry >::iterator it = fit_map.begin();
         it != fit_map.end();
         it++ )
   {
      QString fit_chain;
      QString cross_chain;
      if ( get_chains( it->first, fit_chain, cross_chain ) )
      {
         update_cache_range();
         if ( !cache_from_range_pos.count( fit_chain ) || !cache_to_range_pos.count( fit_chain ) )
         {
            editor_msg( "red", QString( us_tr( "Internal error: chain dereferencing <%1>" ) ).arg( fit_chain ) );
            continue;
         }
         if ( !cache_use_start.count( fit_chain ) )
         {
            editor_msg( "red", QString( us_tr( "Internal error: chain start map entry missing  <%1>" ) ).arg( fit_chain ) );
            continue;
         }

         range_entry from_range = cache_from_ranges[ cache_from_range_pos[ fit_chain ] ];
         range_entry to_range   = cache_to_ranges  [ cache_to_range_pos  [ fit_chain ] ];
         bool        use_start  = cache_use_start  [ fit_chain ];

         // is are fit range for the cross outside of the to_range ?
         if ( ( use_start  && it->second.start < to_range.start ) ||
              ( !use_start && it->second.end   > to_range.end   ) )
         {
            if ( !fit_map.count( fit_chain ) )
            {
               editor_msg( "red", QString( us_tr( "Internal error: chain fit map entry missing  <%1>" ) ).arg( fit_chain ) );
               continue;
            }
            editor_msg( "blue", QString( us_tr( "Notice: setting fitting for extra chain %1 to fit region for chain %2" ) ).arg( cross_chain ).arg( fit_chain ) );
            it->second = fit_map[ fit_chain ];
         }
      }
   }

   // for each chain, a set of fit points (currently on CA)
   map < QString, vector < point > > from_fit_points;
   map < QString, vector < point > > to_fit_points;

   csv csv_merge;
   csv csv_fit;

   // vector of merge_data for each chain

   map < QString, vector < vector < QString > > > merge_data;

   map < QString, bool > extra_chain_reported;
   map < QString, bool > extra_cross_chain_reported;

   // extract the merge & fit bits
   {
      csv_merge = csv_from;
      csv_merge.data.clear( );
      csv_fit = csv_from;
      csv_fit.data.clear( );

      for ( unsigned int i = 0; i < (unsigned int) csv_from.data.size(); i++ )
      {
         if ( csv_from.data[ i ].size() < 14 )
         {
            editor_msg( "red", us_tr( "Internal error: Invalid \"to\" csv" ) );
            running = false;
            update_enables();
            return;
         }

         QString chain    = csv_from.data[ i ][ 1 ];
         unsigned int pos = csv_from.data[ i ][ 3 ].toUInt();
         QString atom     = csv_from.data[ i ][ 4 ];
         if (
             ( merge_map.count( chain ) &&
               merge_map[ chain ].start <= pos &&
               merge_map[ chain ].end   >= pos ) )
         {
            csv_merge.data.push_back( csv_from.data[ i ] );
            merge_data[ chain ].push_back( csv_from.data[ i ] );
         } 

         if (
             ( fit_map.count( chain ) &&
               fit_map[ chain ].start <= pos &&
               fit_map[ chain ].end   >= pos ) )
         {
            csv_fit.data.push_back( csv_from.data[ i ] );
         } 

         if ( fit_map.count( chain ) &&
              fit_map[ chain ].start <= pos &&
              fit_map[ chain ].end   >= pos &&
              atom == " CA " ) 
         {
            point p;
            p.axis[ 0 ] = csv_from.data[ i ][ 8  ].toDouble();
            p.axis[ 1 ] = csv_from.data[ i ][ 9  ].toDouble();
            p.axis[ 2 ] = csv_from.data[ i ][ 10 ].toDouble();
            from_fit_points[ chain ].push_back( p );
            // editor_msg("blue", QString("adding fit point for %1 %2 %3").arg(chain).arg(pos).arg(atom));
#if defined( USPTM_DEBUG2 )
            cout << QString("adding from fit_point for %1 %2 %3\n").arg(chain).arg(pos).arg(atom);
#endif
         }

         // check for cross/extra chains

         if ( cross_fit_chains.count( chain ) )
         {
            for ( unsigned int j = 0; j < (unsigned int) cross_fit_chains[ chain ].size(); j++ )
            {
               QString fit_chain   = chain;
               QString cross_chain = cross_fit_chains[ fit_chain ][ j ];
               QString chain       = QString( "%1+%2" ).arg( fit_chain ).arg( cross_chain );
               if ( !extra_chain_reported.count( chain ) )
               {
                  editor_msg( "blue", QString( "Adding extra fit chains for %1" ).arg( chain ) );
                  extra_chain_reported[ chain ] = true;
               }

               if ( fit_map.count( chain ) &&
                    fit_map[ chain ].start <= pos &&
                    fit_map[ chain ].end   >= pos &&
                    atom == " CA " ) 
               {
                  point p;
                  p.axis[ 0 ] = csv_from.data[ i ][ 8  ].toDouble();
                  p.axis[ 1 ] = csv_from.data[ i ][ 9  ].toDouble();
                  p.axis[ 2 ] = csv_from.data[ i ][ 10 ].toDouble();
                  from_fit_points[ chain ].push_back( p );
                  // editor_msg("blue", QString("adding fit point for %1 %2 %3").arg(chain).arg(pos).arg(atom));
#if defined( USPTM_DEBUG2 )
                  cout << QString("adding xchain from_fit point for %1 %2 %3\n").arg(chain).arg(pos).arg(atom);
#endif
               }
            }
         }               

         if ( cross_cross_chains.count( chain ) )
         {
            for ( unsigned int j = 0; j < (unsigned int) cross_cross_chains[ chain ].size(); j++ )
            {
               QString cross_chain = chain;
               QString fit_chain   = cross_cross_chains[ cross_chain ][ j ];
               QString chain       = QString( "%1+%2" ).arg( fit_chain ).arg( cross_chain );
               if ( !extra_cross_chain_reported.count( chain ) )
               {
                  editor_msg( "blue", QString( "Adding extra merge chains for %1" ).arg( chain ) );
                  extra_cross_chain_reported[ chain ] = true;
               }
               // csv_merge.data.push_back( csv_from.data[ i ] );
               merge_data[ chain ].push_back( csv_from.data[ i ] );
            }
         }               
      }
      //      ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv_setup_keys( csv_merge );
      //      ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv_clipboard = csv_merge;
      //      ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv_clipboard.name = "merge bits of " + csv_from.name;
      //      ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv2_paste_new();

      //      ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv_setup_keys( csv_fit );
      //      ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv_clipboard = csv_fit;
      //      ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv_clipboard.name = "fit bits of " + csv_from.name;
      //      ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv2_paste_new();
   }

   // go thru the csv_to & cut
   if ( any_cuts )
   {
      csv new_csv = csv_to;
      new_csv.data.clear( );

      for ( unsigned int i = 0; i < (unsigned int) csv_to.data.size(); i++ )
      {
         if ( csv_to.data[ i ].size() < 14 )
         {
            editor_msg( "red", us_tr( "Internal error: Invalid \"to\" csv" ) );
            running = false;
            update_enables();
            return;
         }
         QString chain    = csv_to.data[ i ][ 1 ];
         unsigned int pos = csv_to.data[ i ][ 3 ].toUInt();
         if ( !cut_map.count( chain ) ||
              cut_map[ chain ].start > pos ||
              cut_map[ chain ].end   < pos )
         {
            new_csv.data.push_back( csv_to.data[ i ] );
         }
      }
      csv_to = new_csv;
      
      //      ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv_setup_keys( csv_to );
      //      ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv_clipboard = csv_to;
      //      ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv_clipboard.name = "after cuts " + csv_to.name;
      //      ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv2_paste_new();
   }

   // determine to ranges for merge position 
   map < QString, range_entry > to_range_map;

   for ( unsigned int i = 0; i < (unsigned int) csv_to.data.size(); i++ )
   {
      if ( csv_to.data[ i ].size() < 14 )
      {
         editor_msg( "red", us_tr( "Internal error: Invalid \"to\" csv" ) );
         running = false;
         update_enables();
         return;
      }
      QString chain    = csv_to.data[ i ][ 1 ];
      unsigned int pos = csv_to.data[ i ][ 3 ].toUInt();

      if ( to_range_map.count( chain ) )
      {
         if ( to_range_map[ chain ].start > pos )
         {
            to_range_map[ chain ].start = pos;
         }
         if ( to_range_map[ chain ].end < pos )
         {
            to_range_map[ chain ].end = pos;
         }
      } else {
         to_range_map[ chain ].chain = chain;
         to_range_map[ chain ].start = pos;
         to_range_map[ chain ].end   = pos;
      }
   }   

   // determine front or back
   map < QString, bool > chain_to_front;
   for ( map < QString, range_entry >::iterator it = to_range_map.begin();
         it != to_range_map.end();
         it++ )
   {
      if ( merge_map.count( it->first ) )
      {
         chain_to_front[ it->first ] = merge_map[ it->first ].end < it->second.start;
         // editor_msg( "blue", QString("Chain %1, merge chain to be added %2").arg( it->first ).arg( chain_to_front[ it->first ] ? "front" : "back" ) );
      }
   }

   // extract to fit points
   extra_chain_reported.clear( );

   for ( unsigned int i = 0; i < (unsigned int) csv_to.data.size(); i++ )
   {
      if ( csv_to.data[ i ].size() < 14 )
      {
         editor_msg( "red", us_tr( "Internal error: Invalid \"to\" csv" ) );
         running = false;
         update_enables();
         return;
      }

      QString      chain = csv_to.data[ i ][ 1 ];
      unsigned int pos   = csv_to.data[ i ][ 3 ].toUInt();
      QString      atom  = csv_to.data[ i ][ 4 ];
      if ( fit_map.count( chain ) &&
           fit_map[ chain ].start <= pos &&
           fit_map[ chain ].end   >= pos &&
           atom == " CA " ) 
      {
         point p;
         p.axis[ 0 ] = csv_to.data[ i ][ 8  ].toDouble();
         p.axis[ 1 ] = csv_to.data[ i ][ 9  ].toDouble();
         p.axis[ 2 ] = csv_to.data[ i ][ 10 ].toDouble();
         to_fit_points[ chain ].push_back( p );
#if defined( USPTM_DEBUG2 )
         cout << QString("adding to_fit point for %1 %2 %3\n").arg(chain).arg(pos).arg(atom) ;
#endif
      }

      if ( cross_fit_chains.count( chain ) )
      {
         for ( unsigned int j = 0; j < (unsigned int) cross_fit_chains[ chain ].size(); j++ )
         {
            QString fit_chain   = chain;
            QString cross_chain = cross_fit_chains[ fit_chain ][ j ];
            QString chain       = QString( "%1+%2" ).arg( fit_chain ).arg( cross_chain );
            if ( !extra_chain_reported.count( chain ) )
            {
               editor_msg( "blue", QString( "Adding to extra fit chains for %1" ).arg( chain ) );
               extra_chain_reported[ chain ] = true;
            }

            if ( fit_map.count( chain ) &&
                 fit_map[ chain ].start <= pos &&
                 fit_map[ chain ].end   >= pos &&
                 atom == " CA " ) 
            {
               point p;
               p.axis[ 0 ] = csv_to.data[ i ][ 8  ].toDouble();
               p.axis[ 1 ] = csv_to.data[ i ][ 9  ].toDouble();
               p.axis[ 2 ] = csv_to.data[ i ][ 10 ].toDouble();
               to_fit_points[ chain ].push_back( p );
#if defined( USPTM_DEBUG2 )
               cout << QString("adding xchain to_fit point for %1 %2 %3\n").arg(chain).arg(pos).arg(atom) ;
#endif
            }
         }
      }               
   }

   // validate from/to fit points
#if defined( USPTM_DEBUG2 )
   cout << "Trial Validate fit points\n";
#endif
   for ( map < QString, vector < point > >::iterator it = from_fit_points.begin();
         it != from_fit_points.end();
         it++ )
   {
      if ( !to_fit_points.count( it->first ) ||
           to_fit_points[ it->first ].size() != it->second.size() )
      {
         editor_msg( "red", QString( "Error (trial): Fit point mismatch for %1 (%2 vs %3)" ).arg( it->first ).arg( to_fit_points.count( it->first ) ? to_fit_points[ it->first ].size() : -1 ).arg( it->second.size() ) );
         running = false;
         update_enables();
         return;
      } 
#if defined( USPTM_DEBUG2 )
      else {
         cout << 
            QString( "Ok (trial): Fit points for %1 (%2 vs %3)\n" ).arg( it->first ).arg( to_fit_points.count( it->first ) ? to_fit_points[ it->first ].size() : -1 ).arg( it->second.size() );
      }
#endif
   }

   // editor_msg("dark red", "transform next");

   // go through csv_to, for each chain with a transformation, add merge_bits transformed

   map < QString, bool > chain_added;
   QString last_chain = "__first__";

   {
      csv new_csv = csv_to;
      new_csv.data.clear( );

      for ( unsigned int i = 0; i < (unsigned int) csv_to.data.size(); i++ )
      {
         if ( csv_to.data[ i ].size() < 14 )
         {
            editor_msg( "red", us_tr( "Internal error: Invalid \"to\" csv" ) );
            running = false;
            update_enables();
            return;
         }

         QString chain    = csv_to.data[ i ][ 1 ];
         // unsigned int pos = csv_to.data[ i ][ 3 ].toUInt();
         if ( chain != last_chain &&
              merge_data.count( last_chain ) && 
              !chain_added.count( last_chain ) &&
              chain_to_front.count( last_chain ) &&
              chain_to_front[ last_chain ] == false )
         {
            // cout << QString("adding merge for previous chain %1 (current %2) before pos %3\n").arg(last_chain).arg(chain).arg(pos);
            chain_added[ last_chain ] = true;
            for ( unsigned int j = 0; j < (unsigned int) merge_data[ last_chain ].size(); j++ )
            {
               vector < point > ps;
               point p;

               p.axis[ 0 ] = merge_data[ last_chain ][ j ][ 8  ].toDouble();
               p.axis[ 1 ] = merge_data[ last_chain ][ j ][ 9  ].toDouble();
               p.axis[ 2 ] = merge_data[ last_chain ][ j ][ 10  ].toDouble();
               ps.push_back( p );
               vector < point > result;
               QString error_msg;
               if ( !((US_Hydrodyn *)us_hydrodyn)->atom_align( from_fit_points[ last_chain ], 
                                                               to_fit_points  [ last_chain ], 
                                                               ps, 
                                                               result,
                                                               error_msg ) )
               {
                  editor_msg( "red", error_msg );
                  running = false;
                  update_enables();
                  return;
               }
               unsigned int datapos = new_csv.data.size();
               new_csv.data.push_back( merge_data[ last_chain ][ j ] );
               new_csv.data[ datapos ][ 8  ] = QString( "%1" ).arg( result[ 0 ].axis[ 0 ] );
               new_csv.data[ datapos ][ 9  ] = QString( "%1" ).arg( result[ 0 ].axis[ 1 ] );
               new_csv.data[ datapos ][ 10 ] = QString( "%1" ).arg( result[ 0 ].axis[ 2 ] );
            }
         }            

         if ( ( merge_data.count( chain ) && 
                !chain_added.count( chain ) &&
                chain_to_front.count( chain ) &&
                chain_to_front[ chain ] == true ) )
         {
            // cout << QString("adding merge for chain %1 before pos %2\n").arg(chain).arg(pos);
            chain_added[ chain ] = true;
            // xform & add chain 
            for ( unsigned int j = 0; j < (unsigned int) merge_data[ chain ].size(); j++ )
            {
               vector < point > ps;
               point p;

               p.axis[ 0 ] = merge_data[ chain ][ j ][ 8  ].toDouble();
               p.axis[ 1 ] = merge_data[ chain ][ j ][ 9  ].toDouble();
               p.axis[ 2 ] = merge_data[ chain ][ j ][ 10  ].toDouble();
               ps.push_back( p );
               vector < point > result;
               QString error_msg;
               if ( !((US_Hydrodyn *)us_hydrodyn)->atom_align( from_fit_points[ chain ], 
                                                               to_fit_points  [ chain ], 
                                                               ps, 
                                                               result,
                                                               error_msg ) )
               {
                  editor_msg( "red", error_msg );
                  running = false;
                  update_enables();
                  return;
               }
               unsigned int datapos = new_csv.data.size();
               new_csv.data.push_back( merge_data[ chain ][ j ] );
               new_csv.data[ datapos ][ 8  ] = QString( "%1" ).arg( result[ 0 ].axis[ 0 ] );
               new_csv.data[ datapos ][ 9  ] = QString( "%1" ).arg( result[ 0 ].axis[ 1 ] );
               new_csv.data[ datapos ][ 10 ] = QString( "%1" ).arg( result[ 0 ].axis[ 2 ] );
            }
         } 
         // cout << QString("pushing back chain %1 pos %2\n").arg(chain).arg(pos);
         new_csv.data.push_back( csv_to.data[ i ] );
         last_chain = chain;
      }

      if ( merge_data.count( last_chain ) && 
           !chain_added.count( last_chain ) &&
           chain_to_front.count( last_chain ) &&
           chain_to_front[ last_chain ] == false )
      {
         // cout << QString("adding merge at end previous chain %1\n").arg(last_chain);
         chain_added[ last_chain ] = true;
         for ( unsigned int j = 0; j < (unsigned int) merge_data[ last_chain ].size(); j++ )
         {
            // not yet xformed
            vector < point > ps;
            point p;
            
            p.axis[ 0 ] = merge_data[ last_chain ][ j ][ 8  ].toDouble();
            p.axis[ 1 ] = merge_data[ last_chain ][ j ][ 9  ].toDouble();
            p.axis[ 2 ] = merge_data[ last_chain ][ j ][ 10  ].toDouble();
            ps.push_back( p );
            vector < point > result;
            QString error_msg;
            if ( !((US_Hydrodyn *)us_hydrodyn)->atom_align( from_fit_points[ last_chain ], 
                                                            to_fit_points  [ last_chain ], 
                                                            ps, 
                                                            result,
                                                            error_msg ) )
            {
               editor_msg( "red", error_msg );
               running = false;
               update_enables();
               return;
            }
            unsigned int datapos = new_csv.data.size();
            new_csv.data.push_back( merge_data[ last_chain ][ j ] );
            new_csv.data[ datapos ][ 8  ] = QString( "%1" ).arg( result[ 0 ].axis[ 0 ] );
            new_csv.data[ datapos ][ 9  ] = QString( "%1" ).arg( result[ 0 ].axis[ 1 ] );
            new_csv.data[ datapos ][ 10 ] = QString( "%1" ).arg( result[ 0 ].axis[ 2 ] );
         }
      }            

      // add extra chains if present
      
      // this could also be run over the merge map:
      //  for ( map < QString, vector < vector < QString > > > merge_data.begin(); etc

      for ( unsigned int i = 0; i < (unsigned int) csv_from.data.size(); i++ )
      {
         if ( csv_from.data[ i ].size() < 14 )
         {
            editor_msg( "red", us_tr( "Internal error: Invalid \"from\" csv" ) );
            running = false;
            update_enables();
            return;
         }

         QString chain    = csv_from.data[ i ][ 1 ];

         // is this a chain we need to add ?
         if ( !cross_cross_chains.count( chain ) )
         {
            continue;
         }

         if ( cross_cross_chains[ chain ].size() != 1 )
         {
            editor_msg( "red", QString( us_tr( "Error: only one fitting chain currently allowed per extra chain %1" ) ).arg( chain ) );
            running = false;
            update_enables();
            return;
         }

         {
            QString cross_chain = chain;
            QString fit_chain   = cross_cross_chains[ chain ][ 0 ];
            QString chain       = QString( "%1+%2" ).arg( fit_chain ).arg( cross_chain );

            if ( !from_fit_points.count( chain ) || !to_fit_points.count( chain ) )
            {
               editor_msg( "red", QString( us_tr( "Error: missing fit points for %1" ) ).arg( chain ) );
               running = false;
               update_enables();
               return;
            }

            if ( !merge_data.count( chain ) )
            {
               editor_msg( "red", QString( us_tr( "Error: merge data for %1" ) ).arg( chain ) );
               running = false;
               update_enables();
               return;
            }

            // make the transform:
         
            {
               vector < point > ps;
               point p;

               p.axis[ 0 ] = csv_from.data[ i ][ 8  ].toDouble();
               p.axis[ 1 ] = csv_from.data[ i ][ 9  ].toDouble();
               p.axis[ 2 ] = csv_from.data[ i ][ 10 ].toDouble();
               ps.push_back( p );
               vector < point > result;
               QString error_msg;
               if ( !((US_Hydrodyn *)us_hydrodyn)->atom_align( from_fit_points[ chain ], 
                                                               to_fit_points  [ chain ], 
                                                               ps, 
                                                               result,
                                                               error_msg ) )
               {
                  editor_msg( "red", error_msg );
                  running = false;
                  update_enables();
                  return;
               }
               unsigned int datapos = new_csv.data.size();
               new_csv.data.push_back( csv_from.data[ i ] );
               new_csv.data[ datapos ][ 8  ] = QString( "%1" ).arg( result[ 0 ].axis[ 0 ] );
               new_csv.data[ datapos ][ 9  ] = QString( "%1" ).arg( result[ 0 ].axis[ 1 ] );
               new_csv.data[ datapos ][ 10 ] = QString( "%1" ).arg( result[ 0 ].axis[ 2 ] );
            }
         }
      }            

      last_chain = "__first__";
      unsigned atompos = 0;
      for ( unsigned int i = 0; i < (unsigned int) new_csv.data.size(); i++ )
      {
         QString chain    = new_csv.data[ i ][ 1 ];
         if ( chain != last_chain )
          {
            // editor_msg("red", QString("chain <%1> last chain <%2>").arg( chain ).arg(last_chain));
            atompos = 0;
            last_chain = chain;
         }
         new_csv.data[ i ][ 5 ] = QString( "%1" ).arg( ++atompos );
      }

      csv_to = new_csv;
      
      ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv_setup_keys( csv_to );
      ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv_clipboard = csv_to;
      ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv_clipboard.name = "merged " + csv_to.name;
      ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv2_paste_new();
   }
   
   running = false;
   update_enables();
}

void US_Hydrodyn_Pdb_Tool_Merge::stop()
{
   running = false;
   editor_msg("red", "Stopped by user request\n");
   update_enables();
}

void US_Hydrodyn_Pdb_Tool_Merge::update_enables()
{
   if ( !running ) {
      progress->reset();
   }

   bool from_exists        = QFile::exists( le_chains_from->text() );
   bool to_exists          = QFile::exists( le_chains_to->text() );
   bool target_set         = !le_target->text().isEmpty();
   bool any_from_selected  = ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->any_selected( lv_csv_from );
   bool any_to_selected    = ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->any_selected( lv_csv_to );

   // pdb_sel_count counts    = ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->count_selected( lv_csv_to );

   cb_filter           ->setEnabled( !running );
   pb_clear            ->setEnabled( !running && t_csv->rowCount() );
   pb_load             ->setEnabled( !running );
   pb_validate         ->setEnabled( !running && from_exists && to_exists && t_csv->rowCount() );
   pb_csv_save         ->setEnabled( !running && t_csv->rowCount() );

   pb_extra_chains     ->setEnabled( !running && extra_chains_list.size() && !extra_chains_done );
   pb_only_closest     ->setEnabled( !running && extra_chains_list.size() && !extra_chains_done );
#if QT_VERSION < 0x040000
   pb_delete_row       ->setEnabled( !running && t_csv->numSelections() );
#else
   pb_delete_row       ->setEnabled( !running && t_csv->selectedRanges().count() );
#endif
   pb_start            ->setEnabled( !running && from_exists && to_exists && t_csv->rowCount() && target_set );
   pb_trial            ->setEnabled( !running && from_exists && to_exists && t_csv->rowCount() );
   pb_stop             ->setEnabled( running );

   pb_sel_auto         ->setEnabled( !running && from_exists && to_exists );
   pb_sel_from_to_merge->setEnabled( !running && any_from_selected );
   pb_sel_from_to_fit  ->setEnabled( !running && any_from_selected );
   pb_sel_to_to_fit    ->setEnabled( !running && any_to_selected );
   pb_sel_to_to_cut    ->setEnabled( !running && any_to_selected );
}

void US_Hydrodyn_Pdb_Tool_Merge::editor_msg( QString color, QString msg )
{
   QColor save_color = editor->textColor();
   editor->setTextColor(color);
   editor->append(msg);
   editor->setTextColor(save_color);
}

void US_Hydrodyn_Pdb_Tool_Merge::chains_from()
{
   QString filename;
   ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->load( lv_csv_from,
                                                    filename,
                                                    true );
   le_chains_from->setText( filename );
   update_enables();
   raise();
}

void US_Hydrodyn_Pdb_Tool_Merge::chains_to()
{
   QString filename;
   ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->load( lv_csv_to,
                                                    filename,
                                                    true );
   le_chains_to->setText( filename );
   update_enables();
   raise();
}

void US_Hydrodyn_Pdb_Tool_Merge::target()
{
   QString filename = QFileDialog::getSaveFileName( this , "Choose a name to save the result" , "" , "PDB (*.pdb *.PDB)" );


   if ( !filename.contains( QRegExp( ".pdb$", Qt::CaseInsensitive ) ) )
   {
      filename += ".pdb";
   }

   le_target->setText( filename );

   if ( filename.isEmpty() )
   {
      return;
   }
   update_enables();
}

void US_Hydrodyn_Pdb_Tool_Merge::sel_auto()
{
   clear( );

   vector < range_entry > from_ranges;
   vector < range_entry > to_ranges;

   vector < range_entry > new_merge_ranges;
   vector < range_entry > new_fit_ranges;

   sel_to_range( lv_csv_from, from_ranges, false );
   sel_to_range( lv_csv_to  , to_ranges  , false );

   // setup extra chains
   extra_chains_list.clear( );
   map < QString, bool > chains_in_to;
   for ( unsigned int i = 0; i < (unsigned int) to_ranges.size(); i++ )
   {
      chains_in_to[ to_ranges[ i ].chain ] = true;
   }
   for ( unsigned int i = 0; i < (unsigned int) from_ranges.size(); i++ )
   {
      if ( !chains_in_to.count( from_ranges[ i ].chain ) )
      {
         extra_chains_list.push_back( from_ranges[ i ].chain );
      }
   }

   for ( unsigned int i = 0; i < (unsigned int) from_ranges.size(); i++ )
   {
      cout << QString( "From ranges %1: " ).arg( i ) << from_ranges[ i ] << endl;
   }
   for ( unsigned int i = 0; i < (unsigned int) to_ranges.size(); i++ )
   {
      cout << QString( "To ranges %1: " ).arg( i ) << to_ranges[ i ] << endl;
   }

   map < QString, unsigned int > from_range_pos;

   for ( unsigned int i = 0; i < (unsigned int) from_ranges.size(); i++ )
   {
      from_range_pos[ from_ranges[ i ].chain ] = i;
   }

   for ( unsigned int i = 0; i < (unsigned int) to_ranges.size(); i++ )
   {
#if defined( USPTM_DEBUG )
      cout << "checking to range: " << to_ranges[ i ] << endl;
#endif
      if ( from_range_pos.count( to_ranges[ i ].chain ) )
      {
#if defined( USPTM_DEBUG )
         cout << "common chain: found matching from range: " << from_ranges[ from_range_pos[ to_ranges[ i ].chain ] ] << endl;
#endif
         // common chain
         // only handling one end for now
         range_entry from_range = from_ranges[ from_range_pos[ to_ranges[ i ].chain ] ];
         range_entry new_merge_range;
         range_entry new_fit_range;

         bool start_good = 
            ( from_range.start   <   to_ranges[ i ].start && 
              from_range.end     >=  to_ranges[ i ].start + 2 &&
              to_ranges[ i ].end >=  to_ranges[ i ].start + 2 );
         bool start_ok = 
            ( from_range.start   <=  to_ranges[ i ].start && 
              from_range.end     >=  to_ranges[ i ].start + 2 &&
              to_ranges[ i ].end >=  to_ranges[ i ].start + 2 );
         bool end_good =
            ( from_range.end       > to_ranges[ i ].end &&
              from_range.start     <= to_ranges[ i ].end - 2 &&
              to_ranges[ i ].start <= to_ranges[ i ].end - 2 );
         bool end_ok =
            ( from_range.end       >= to_ranges[ i ].end &&
              from_range.start     <= to_ranges[ i ].end - 2 &&
              to_ranges[ i ].start <= to_ranges[ i ].end - 2 );

         bool use_start = false;
         bool use_end   = false;

         if ( start_good && !end_ok )
         {
            use_start = true;
         } else {
            if ( end_good )
            {
               use_end = true;
            } else {
               if ( start_ok )
               {
                  use_start = true;
               } else {
                  if ( end_ok )
                  {
                     use_end = true;
                  }
               }
            }
         }

         if ( use_start )
         {
#if defined( USPTM_DEBUG )
            cout << "add at front\n";
#endif
            // add at front
            // merge:
            new_merge_range.chain = to_ranges[ i ].chain;
            new_merge_range.start = from_range.start;
            new_merge_range.end   = to_ranges[ i ].start - 1;

            new_fit_range.chain = to_ranges[ i ].chain;
            new_fit_range.start = to_ranges[ i ].start;
            new_fit_range.end   = to_ranges[ i ].start + 2;

            new_merge_ranges.push_back( new_merge_range );
            new_fit_ranges  .push_back( new_fit_range   );
            cache_use_start[ new_merge_range.chain ] = true;
         } else {
            if ( use_end )
            {
#if defined( USPTM_DEBUG )
               cout << "add at end\n";
#endif
               // add at end
               new_merge_range.chain = to_ranges[ i ].chain;
               new_merge_range.start = to_ranges[ i ].end + 1;
               new_merge_range.end   = from_range.end;
               
               new_fit_range.chain = to_ranges[ i ].chain;
               new_fit_range.start = to_ranges[ i ].end - 2;
               new_fit_range.end   = to_ranges[ i ].end;

               new_merge_ranges.push_back( new_merge_range );
               new_fit_ranges  .push_back( new_fit_range   );
               cache_use_start[ new_merge_range.chain ] = false;
            }
#if defined( USPTM_DEBUG )
            else 
            {
               cout << "neither front nor end\n";
            }
#endif
         }
      }
   }

   update_t_csv_range( new_merge_ranges, 1, 2 );
   update_t_csv_range( new_fit_ranges  , 3, 4 );

   update_cache_range();
   for ( int row = 0; row < t_csv->rowCount(); row++ )
   {
      QString chain       = t_csv->item( row, 0 )->text().left( 1 );
      if ( chain == "+" )
      {
         chain = " ";
      }
      if ( !cache_from_range_pos.count( chain ) || !cache_to_range_pos.count( chain ) )
      {
         editor_msg( "red", QString( us_tr( "Internal error: chain dereferencing <%1>" ) ).arg( chain ) );
      } else {
         range_entry from_range = cache_from_ranges[ cache_from_range_pos[ chain ] ];
         range_entry to_range   = cache_to_ranges  [ cache_to_range_pos  [ chain ] ];
         // bool        use_start  = cache_use_start  [ chain ];
         unsigned current_merge_start = t_csv->item( row, 1 )->text().toUInt();
         unsigned current_merge_end   = t_csv->item( row, 2 )->text().toUInt();
         if ( current_merge_start > current_merge_end )
         {
            current_merge_end = current_merge_start;
            t_csv->setItem( row, 2, new QTableWidgetItem( QString( "%1" ).arg( current_merge_end ) ) );
            table_value( row, 2 );
         }
         unsigned current_fit_start   = t_csv->item( row, 3 )->text().toUInt();
         unsigned current_fit_end     = t_csv->item( row, 4 )->text().toUInt();
         unsigned int current_fit_length_from = residue_length( true , chain, current_fit_start  , current_fit_end   );
         unsigned int current_fit_length_to   = residue_length( false, chain, current_fit_start  , current_fit_end   );
         unsigned int current_fit_length      = 
            current_fit_length_from < current_fit_length_to ?
            current_fit_length_from : current_fit_length_to;
         if ( current_fit_length < 3 )
         {
            current_fit_end            = residue_offset_position( true,  chain, current_fit_start, 3 );
            unsigned int alt_fit_end   = residue_offset_position( false, chain, current_fit_start, 3 );
            if ( current_fit_end < alt_fit_end )
            {
               current_fit_end = alt_fit_end;
            }
            if ( current_fit_end > to_range.end ||
                 current_fit_end > from_range.end )
            {
               current_fit_start = 0;
               current_fit_end   = 0;
            }
            t_csv->setItem( row, 3, new QTableWidgetItem( QString( "%1" ).arg( current_fit_start ) ) );
            t_csv->setItem( row, 4, new QTableWidgetItem( QString( "%1" ).arg( current_fit_end   ) ) );
         }       
      }  
   }

#if defined( USPTM_DEBUG2 )
   cout << "cache summary:\n";
   cout << "From ranges:\n";
   for ( unsigned int i = 0; i < ( unsigned int ) cache_from_ranges.size(); i++ )
   {
      cout << cache_from_ranges[ i ] << endl;
   }
   cout << "To ranges:\n";
   for ( unsigned int i = 0; i < ( unsigned int ) cache_to_ranges.size(); i++ )
   {
      cout << cache_to_ranges[ i ] << endl;
   }
   cout << "From range pos:\n";
   for ( map < QString, unsigned int >::iterator it = cache_from_range_pos.begin();
         it != cache_from_range_pos.end();
         it++ )
   {
      cout << it->first << " pos " << it->second << endl;
   }
   cout << "To range pos:\n";
   for ( map < QString, unsigned int >::iterator it = cache_to_range_pos.begin();
         it != cache_to_range_pos.end();
         it++ )
   {
      cout << it->first << " pos " << it->second << endl;
   }
   cout << "Use start:\n";
   for ( map < QString, bool >::iterator it = cache_use_start.begin();
         it != cache_use_start.end();
         it++ )
   {
      cout << it->first << " " << QString( "%1" ).arg( it->second ? "true" : "false" ) << endl;
   }
#endif

   update_enables();
}

void US_Hydrodyn_Pdb_Tool_Merge::sel_to_range( QTreeWidget *lv, vector < range_entry > &ranges, bool just_selected )
{
   csv csv1 = ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->to_csv( lv, 
                                                                 lv == lv_csv_from ?
                                                                 ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv1 :
                                                                 ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv2[ ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv2_pos ],
                                                                 just_selected );
   ranges.clear( );

   range_entry range;

   map < QString, unsigned int > range_pos;

   for ( unsigned int i = 0; i < (unsigned int) csv1.data.size(); i++ )
   {
      if ( csv1.data[ i ].size() < 14 )
      {
         editor_msg( "red", us_tr( "Internal error: Invalid selection csv" ) );
         return;
      }
      range.chain      = csv1.data[ i ][ 1 ];
      unsigned int pos = csv1.data[ i ][ 3 ].toUInt();
      if ( range_pos.count( range.chain ) )
      {
         unsigned int rpos = range_pos[ range.chain ];
         ranges[ rpos ].residues[ pos ] = true;
         if ( ranges[ rpos ].start > pos )
         {
            ranges[ rpos ].start = pos;
         }
         if ( ranges[ rpos ].end < pos )
         {
            ranges[ rpos ].end = pos;
         }
      } else {
         range.end   = pos;
         range.start = pos;
         range.gaps.clear( );
         range.residues.clear( );
         range.residues[ pos ] = true;
         range_pos[ range.chain ] = ranges.size();
         ranges.push_back( range );
      }
   }

   // compute gaps:

   uhptm_sortable_uint usui;

   for ( unsigned int i = 0; i < (unsigned int) ranges.size(); i++ )
   {
      list < uhptm_sortable_uint > lusui;
      lusui.clear( );
      for ( map  < unsigned int, bool >::iterator it = ranges[ i ].residues.begin();
            it  != ranges[ i ].residues.end();
            it++ )
      {
         usui.x = it->first;
         lusui.push_back( usui );
      }
      lusui.sort();
      list < uhptm_sortable_uint >::iterator lit = lusui.begin();
      unsigned int last_pos = lit->x;
      for ( ;
            lit != lusui.end();
            lit++ )
      {
         while ( last_pos < lit->x )
         {
            ranges[ i ].gaps[ last_pos ] = true;
            last_pos++;
         }
         last_pos = lit->x + 1;
      }
   }
         
#if defined( USPTM_DEBUG )
   for ( unsigned int i = 0; i < (unsigned int) ranges.size(); i++ )
   {
      cout << ranges[ i ] << endl;
      
   }
#endif
}

void US_Hydrodyn_Pdb_Tool_Merge::make_csv_chain_map()
{
   csv_chain_map.clear( );
   for (int i = 0; i < t_csv->rowCount(); i++ )
   {
      csv_chain_map[ t_csv->item( i, 0 )->text() ] = i;
   }
}

void US_Hydrodyn_Pdb_Tool_Merge::update_t_csv_range( vector < range_entry > &ranges,
                                                     unsigned int col_start,
                                                     unsigned int col_end )
{
   make_csv_chain_map();
   unsigned int pos;
   for ( unsigned int i = 0; i < (unsigned int) ranges.size(); i++ )
   {
      if ( csv_chain_map.count( ranges[ i ].chain ) )
      {
         pos = csv_chain_map[ ranges[ i ].chain ];
      } else {
         pos = ( unsigned int ) t_csv->rowCount();
         t_csv->setRowCount( pos + 1 );
         t_csv->setItem( pos, 0, new QTableWidgetItem( ranges[ i ].chain ) );
         fill_csv_empty_items();
      }
      disconnect( t_csv, SIGNAL( cellChanged( int, int ) ), 0, 0 );
      t_csv->setItem( pos, col_start, new QTableWidgetItem( QString("%1").arg( ranges[ i ].start ) ) );
      t_csv->setItem( pos, col_end  , new QTableWidgetItem( QString("%1").arg( ranges[ i ].end ) ) );
      connect( t_csv, SIGNAL( cellChanged( int, int ) ), SLOT( table_value( int, int ) ) );
   }
}

void US_Hydrodyn_Pdb_Tool_Merge::sel_from_to_merge()
{
   vector < range_entry > ranges;
   sel_to_range( lv_csv_from, ranges );
   update_t_csv_range( ranges, 1, 2 );
   update_cache_range();
   for ( unsigned int i = 0; i < (unsigned int) ranges.size(); i++ )
   {
      QString chain = ranges[ i ].chain;
      if ( cache_to_range_pos.count( chain ) )
      {
         recalc_from_merge( cache_to_range_pos[ chain ], 1 );
      }
   }
   update_enables();
}

void US_Hydrodyn_Pdb_Tool_Merge::sel_from_to_fit()
{
   vector < range_entry > ranges;
   sel_to_range( lv_csv_from, ranges );
   update_t_csv_range( ranges, 3, 4 );
   update_cache_range();
   for ( unsigned int i = 0; i < (unsigned int) ranges.size(); i++ )
   {
      QString chain = ranges[ i ].chain;
      if ( cache_to_range_pos.count( chain ) )
      {
         recalc_from_fit( cache_to_range_pos[ chain ], 3 );
      }
   }
   update_enables();
}

void US_Hydrodyn_Pdb_Tool_Merge::sel_to_to_fit()
{
   vector < range_entry > ranges;
   sel_to_range( lv_csv_to, ranges );
   update_t_csv_range( ranges, 3, 4 );
   update_cache_range();
   for ( unsigned int i = 0; i < (unsigned int) ranges.size(); i++ )
   {
      QString chain = ranges[ i ].chain;
      if ( cache_to_range_pos.count( chain ) )
      {
         recalc_from_fit( cache_to_range_pos[ chain ], 3 );
      }
   }
   update_enables();
}

void US_Hydrodyn_Pdb_Tool_Merge::sel_to_to_cut()
{
   vector < range_entry > ranges;
   sel_to_range( lv_csv_to, ranges );
   update_t_csv_range( ranges, 5, 6 );
   update_cache_range();
   for ( unsigned int i = 0; i < (unsigned int) ranges.size(); i++ )
   {
      QString chain = ranges[ i ].chain;
      if ( cache_to_range_pos.count( chain ) )
      {
         recalc_from_cut( cache_to_range_pos[ chain ], 5 );
      }
   }
   update_enables();
}

void US_Hydrodyn_Pdb_Tool_Merge::clear( )
{
   for ( int i = t_csv->rowCount() - 1; i >= 0; i-- )
   {
      t_csv->removeRow( i );
   }
   extra_chains_list   .clear( );
   cache_from_ranges   .clear( );
   cache_to_ranges     .clear( );
   cache_from_range_pos.clear( );
   cache_to_range_pos  .clear( );
   cache_use_start     .clear( );
   extra_chains_done   = false;

   cache_range_ok = false;
   
   update_enables();
}

void US_Hydrodyn_Pdb_Tool_Merge::validate()
{
   running = true;
   update_enables();

   ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv2_clear( );
   QString filename = le_chains_to->text();
   ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->load( lv_csv_to,
                                                    filename,
                                                    true );

   if ( validate_commands() )
   {
      editor_msg( "dark blue", "Validation OK" );
   } else {
      editor_msg( "red", "Validation errors" );
   }
   running = false;
   update_enables();
}

bool US_Hydrodyn_Pdb_Tool_Merge::validate_commands()
{

   vector < range_entry > from_ranges;
   vector < range_entry > to_ranges;

   sel_to_range( lv_csv_from, from_ranges, false );
   sel_to_range( lv_csv_to  , to_ranges  , false );

   map < QString, unsigned int > from_range_pos;
   map < QString, unsigned int > to_range_pos;

   for ( unsigned int i = 0; i < (unsigned int) from_ranges.size(); i++ )
   {
      from_range_pos[ from_ranges[ i ].chain ] = i;
   }

   for ( unsigned int i = 0; i < (unsigned int) to_ranges.size(); i++ )
   {
      to_range_pos[ to_ranges[ i ].chain ] = i;
   }

   cache_from_ranges    = from_ranges;
   cache_to_ranges      = to_ranges;
   cache_from_range_pos = from_range_pos;
   cache_to_range_pos   = to_range_pos;
   cache_range_ok       = true;

   update_csv_commands_from_table();

   vector < range_entry > merge_ranges;
   vector < range_entry > fit_ranges;
   vector < range_entry > cut_ranges;

   range_entry merge_range;
   range_entry fit_range;
   range_entry cut_range;

   map < QString, QString > dup_extra_chains;
   QString fit_chain;
   QString cross_chain;

   unsigned int errors = 0;

   for ( unsigned int i = 0; i < (unsigned int) csv_commands.data.size(); i++ )
   {
      if ( csv_commands.data[ i ].size() < 7 ||
           csv_commands.num_data[ i ].size() < 7 )
      {
         editor_msg( "red", us_tr( "Internal error: Invalid command directive format" ) );
         return false;
      }
      
      if ( !get_chains( csv_commands.data[ i ][ 0 ], fit_chain, cross_chain ) )
      {
         merge_range.chain = csv_commands.data[ i ][ 0 ];
         merge_range.start = (unsigned int) csv_commands.num_data[ i ][ 1 ];
         merge_range.end   = (unsigned int) csv_commands.num_data[ i ][ 2 ];

         fit_range.chain   = csv_commands.data[ i ][ 0 ];
         fit_range.start   = (unsigned int) csv_commands.num_data[ i ][ 3 ];
         fit_range.end     = (unsigned int) csv_commands.num_data[ i ][ 4 ];

         cut_range.chain   = csv_commands.data[ i ][ 0 ];
         cut_range.start   = (unsigned int) csv_commands.num_data[ i ][ 5 ];
         cut_range.end     = (unsigned int) csv_commands.num_data[ i ][ 6 ];

         merge_ranges.push_back( merge_range );
         fit_ranges  .push_back( fit_range   );
         cut_ranges  .push_back( cut_range   );
      } else {
         fit_range.chain   = csv_commands.data[ i ][ 0 ];
         fit_range.start   = (unsigned int) csv_commands.num_data[ i ][ 3 ];
         fit_range.end     = (unsigned int) csv_commands.num_data[ i ][ 4 ];
         
         fit_ranges  .push_back( fit_range   );

         if ( !dup_extra_chains.count( cross_chain ) )
         {
            dup_extra_chains[ cross_chain ] = csv_commands.data[ i ][ 0 ];
         } else {
            editor_msg( "red", 
                        QString( us_tr( "Error: Extra chain %1 has repeated usage: %2 and %3, only one usage allowed\n" ) )
                        .arg( cross_chain )
                        .arg( csv_commands.data[ i ][ 0 ] )
                        .arg( dup_extra_chains[ cross_chain ] ) );
            errors++;
         }
      }
   }

   for ( unsigned int i = 0; i < (unsigned int) fit_ranges.size(); i++ )
   {
      if ( !get_chains( fit_ranges[ i ].chain, fit_chain, cross_chain ) )
      {
         continue;
      }

      if ( !from_range_pos.count( fit_chain ) )
      {
         editor_msg( "red", QString( "Error: Chain %1 missing in \"Chains From\" PDB" ).arg( fit_chain ) );
         errors++;
      } else {
         range_entry from_range = from_ranges[ from_range_pos[ fit_chain ] ];
         if ( from_range.start > fit_ranges[ i ].start ||
              from_range.end   < fit_ranges[ i ].end )
         {
            editor_msg( "red", QString( "Error: Chain %1 \"Chains From\" does not have residues in fit range" ).arg( fit_ranges[ i ].chain ) );
            errors++;
         }
      }
         
      if ( !to_range_pos.count( fit_chain ) )
      {
         editor_msg( "red", QString( "Error: Chain %1 missing in \"Chains To\" PDB" ).arg( fit_chain ) );
         errors++;
      }
   }      

   for ( unsigned int i = 0; i < (unsigned int) merge_ranges.size(); i++ )
   {
      cut_range = cut_ranges[ i ];
      if ( cut_range.start || cut_range.end )
      {
         if ( ( !cut_range.start && cut_range.end  ) ||
              ( cut_range.start  && !cut_range.end ) )
         {
            editor_msg( "red", QString( "Error: Chain %1 cut must have both cut range values set or none (zero is not a set value)" ).arg( cut_range.chain ) );
            errors++;
         }
         if ( cut_range.start > cut_range.end  )
         {
            editor_msg( "red", QString( "Error: Chain %1 cut range must be ascending" ).arg( cut_range.chain ) );
            errors++;
         }
               
         if ( !to_range_pos.count( cut_range.chain ) )
         {
            editor_msg( "red", QString( "Error: Chain %1 missing in \"Chains To\" PDB" ).arg( cut_range.chain ) );
            errors++;
         } else {
            // make sure cut area present in "to"
            range_entry to_range = to_ranges[ to_range_pos[ cut_range.chain ] ];
            if ( to_range.start > cut_range.start ||
                 to_range.end < cut_range.end )
            {
               editor_msg( "red", QString( "Error: Chain %1 \"Chains To\" do not have residues in entire cut range" ).arg( cut_range.chain ) );
               errors++;
            } else {
               // make sure cut area touches one end
               if ( to_range.start != cut_range.start &&
                    to_range.end != cut_range.end )
               {
                  editor_msg( "red", QString( "Error: Chain %1 \"Chains To\" cut start or cut end must equal one end of the chain" ).arg( cut_range.chain ) );
                  errors++;
               } else {
                  // make sure cut area doesn't touch both ends!
                  if ( to_range.start == cut_range.start &&
                       to_range.end == cut_range.end )
                  {
                     editor_msg( "red", QString( "Error: Chain %1 \"Chains To\" can not cut entire chain" ).arg( cut_range.chain ) );
                     errors++;
                  } else {
                     // apply cut
                     if ( to_range.start == cut_range.start )
                     {
                        to_range.start = cut_range.end + 1;
                     } else {
                        to_range.end = cut_range.start - 1;
                     }
                     if ( to_range.start > to_range.end )
                     {
                        editor_msg( "red", QString( "Error: Chain %1 \"Chains To\" chain cut error" ).arg( cut_range.chain ) );
                        errors++;
                     } else {
                        to_ranges[ to_range_pos[ cut_range.chain ] ] = to_range;
                     }
                  }                        
               }
            }
         }
      }

      merge_range = merge_ranges[ i ];
      if ( merge_range.start || merge_range.end )
      {
         if ( ( !merge_range.start && merge_range.end  ) ||
              ( merge_range.start  && !merge_range.end ) )
         {
            editor_msg( "red", QString( "Error: Chain %1 merge must have both merge range values set or none of them (zero is not a set value)" ).arg( merge_range.chain ) );
            errors++;
         }
         if ( merge_range.start > merge_range.end  )
         {
            editor_msg( "red", QString( "Error: Chain %1 merge range must be ascending" ).arg( merge_range.chain ) );
            errors++;
         }
         if ( !from_range_pos.count( merge_range.chain ) )
         {
            editor_msg( "red", QString( "Error: Chain %1 missing in \"Chains From\" PDB" ).arg( merge_range.chain ) );
            errors++;
         } else {
            range_entry from_range = from_ranges[ from_range_pos[ merge_range.chain ] ];
            // make sure merge area present in "from"
            if ( from_range.start > merge_range.start ||
                 from_range.end < merge_range.end )
            {
               editor_msg( "red", QString( "Error: Chain %1 \"Chains From\" does not have residues in entire merge range" ).arg( merge_range.chain ) );
               errors++;
            }
         }
               
         if ( !to_range_pos.count( merge_range.chain ) )
         {
            editor_msg( "red", QString( "Error: Chain %1 missing in \"Chains To\" PDB" ).arg( merge_range.chain ) );
            errors++;
         } else {
            range_entry to_range = to_ranges[ to_range_pos[ merge_range.chain ] ];
            // make sure merge area NOT present in "to" , could happen at either end
            if ( to_range.start <= merge_range.end && to_range.end >= merge_range.start )
            {
               editor_msg( "red", QString( "Error: Chain %1 \"Chains To\" has values in merge range and therefore needs to be cut" ).arg( merge_range.chain ) );
               errors++;
            }
         }
      }

      fit_range = fit_ranges[ i ];
      if ( fit_range.start || fit_range.end )
      {
         if ( ( !fit_range.start && fit_range.end  ) ||
              ( fit_range.start  && !fit_range.end ) )
         {
            editor_msg( "red", QString( "Error: Chain %1 fit range must have both value set or none (zero not a set value)" ).arg( fit_range.chain ) );
            errors++;
         }
         if ( fit_range.start > fit_range.end  )
         {
            editor_msg( "red", QString( "Error: Chain %1 fit range must be ascending" ).arg( fit_range.chain ) );
            errors++;
         }
         if ( !from_range_pos.count( fit_range.chain ) )
         {
            editor_msg( "red", QString( "Error: Chain %1 missing in \"Chains From\" PDB" ).arg( fit_range.chain ) );
            errors++;
         } else {
            range_entry from_range = from_ranges[ from_range_pos[ fit_range.chain ] ];
            // make sure fit area present in "from"
            if ( from_range.start > fit_range.start ||
                 from_range.end < fit_range.end )
            {
               editor_msg( "red", QString( "Error: Chain %1 \"Chains From\" does not have residues in entire fit range" ).arg( fit_range.chain ) );
               errors++;
            }
         }
               
         if ( !to_range_pos.count( fit_range.chain ) )
         {
            editor_msg( "red", QString( "Error: Chain %1 missing in \"Chains To\" PDB" ).arg( fit_range.chain ) );
            errors++;
         } else {
            range_entry to_range = to_ranges[ to_range_pos[ fit_range.chain ] ];
            // make sure fit area present in "to"
            if ( to_range.start > fit_range.start ||
                 to_range.end < fit_range.end )
            {
               editor_msg( "red", QString( "Error: Chain %1 \"Chains To\" does not have residues in entire fit range" ).arg( fit_range.chain ) );
               errors++;
            }
         }
      }
   }

   return errors == 0;
}

void US_Hydrodyn_Pdb_Tool_Merge::load()
{
   QString use_dir = ((US_Hydrodyn *)us_hydrodyn)->somo_dir;

   QString filename = QFileDialog::getOpenFileName( this , windowTitle() , use_dir , "*.csc *.CSC" );
   
   if ( filename.isEmpty() )
   {
      return;
   }

   QFile f(filename);

   if ( !QFile::exists( filename ) )
   {
      QMessageBox::warning( this,
                            us_tr("Could not open file"),
                            QString( us_tr( "An error occured when trying to open file\n"
                                         "%1\n"
                                         "The file does not exist" ) )
                            .arg( filename )
                            );
      return;
   }

   if ( !f.open( QIODevice::ReadOnly ) )
   {
      QMessageBox::warning( this,
                            us_tr("Could not open file"),
                            QString("An error occured when trying to open file\n"
                                    "%1\n"
                                    "Please check the permissions and try again\n")
                            .arg( filename )
                            );
      return;
   }

   QTextStream ts( &f );

   QStringList qsl;

   while( !ts.atEnd() )
   {
      qsl << ts.readLine();
   }

   f.close();

   csv new_csv = csv_commands;

   new_csv.data.clear( );

   QStringList qsl_h = csv_parse_line(qsl[0]);

   {
      QStringList::iterator it = qsl.begin();
      it++;
      for ( ;
            it != qsl.end();
            it++ )
      {
         QStringList qsl_d = csv_parse_line(*it);
         vector < QString > data;
         if ( qsl_d.size() )
         {
            for ( QStringList::iterator it2 = qsl_d.begin();
                  it2 != qsl_d.end();
                  it2++ )
            {
               data.push_back(*it2);
            }
            new_csv.data.push_back( data );
         }
      }
   }

   csv_commands = new_csv;
   update_t_csv_data();
   update_enables();
}

void US_Hydrodyn_Pdb_Tool_Merge::csv_save()
{
   QString use_dir = ((US_Hydrodyn *)us_hydrodyn)->somo_dir;
   QString filename = QFileDialog::getSaveFileName( this , us_tr("Choose a filename to save the data") , use_dir , "*.csc *.CSC" );



   if ( filename.isEmpty() )
   {
      return;
   }

   if ( !filename.contains(QRegExp(".csc$", Qt::CaseInsensitive )) )
   {
      filename += ".csc";
   }

   if ( QFile::exists(filename) )
   {
      filename = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( filename, 0, this );
   }

   QFile f(filename);

   if ( !f.open( QIODevice::WriteOnly ) )
   {
      QMessageBox::warning( this, "UltraScan",
                            QString(us_tr("Could not open %1 for writing!")).arg(filename) );
      return;
   }

   update_csv_commands_from_table();

   QTextStream t( &f );

   QString qs;

   for ( unsigned int i = 0; i < (unsigned int) csv_commands.header.size(); i++ )
   {
      qs += QString("%1\"%2\"").arg(i ? "," : "").arg(csv_commands.header[i]);
   }

   t << qs << Qt::endl;

   for ( unsigned int i = 0; i < (unsigned int) csv_commands.data.size(); i++ )
   {
      qs = "";
      for ( unsigned int j = 0; j < (unsigned int) csv_commands.data[i].size(); j++ )
      {
         qs += QString("%1%2").arg(j ? "," : "").arg(csv_commands.data[i][j]);
      }
      t << qs << Qt::endl;
   }
   f.close();
   editor_msg("black", QString("File %1 written\n").arg( filename ) );
}

void US_Hydrodyn_Pdb_Tool_Merge::pdb_tool()
{
   ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->raise();   
}

void US_Hydrodyn_Pdb_Tool_Merge::update_csv_commands_from_table()
{
   csv_commands.data.clear( );
   csv_commands.num_data.clear( );

   vector < QString > data    ( t_csv->columnCount() );
   vector < double  > num_data( t_csv->columnCount() );

   for ( int i = 0; i < t_csv->rowCount(); i++ )
   {
      for ( int j = 0; j < t_csv->columnCount(); j++ )
      {
         data[ j ]     = t_csv->item( i, j )->text();
         num_data[ j ] = t_csv->item( i, j )->text().toDouble();
      }
      csv_commands.data    .push_back( data     );
      csv_commands.num_data.push_back( num_data );
   }
}

QStringList US_Hydrodyn_Pdb_Tool_Merge::csv_parse_line( QString qs )
{
   // cout << QString("csv_parse_line:\ninital string <%1>\n").arg(qs);
   QStringList qsl;
   if ( qs.isEmpty() )
   {
      // cout << QString("csv_parse_line: empty\n");
      return qsl;
   }
   if ( !qs.contains(",") )
   {
      // cout << QString("csv_parse_line: one token\n");
      qsl << qs;
      return qsl;
   }

   QStringList qsl_chars = (qs).split( "" , Qt::SkipEmptyParts );
   QString token = "";

   bool in_quote = false;

   for ( QStringList::iterator it = qsl_chars.begin();
         it != qsl_chars.end();
         it++ )
   {
      if ( !in_quote && *it == "," )
      {
         qsl << token;
         token = "";
         continue;
      }
      if ( in_quote && *it == "\"" )
      {
         in_quote = false;
         continue;
      }
      if ( !in_quote && *it == "\"" )
      {
         in_quote = true;
         continue;
      }
      if ( !in_quote && *it == "\"" )
      {
         in_quote = false;
         continue;
      }
      token += *it;
   }
   if ( !token.isEmpty() )
   {
      qsl << token;
   }
   // cout << QString("csv_parse_line results:\n<%1>\n").arg(qsl.join(">\n<"));
   return qsl;
}

void US_Hydrodyn_Pdb_Tool_Merge::update_t_csv_data()
{
   t_csv->setRowCount( csv_commands.data.size() );

   for ( unsigned int i = 0; i < (unsigned int) csv_commands.data.size(); i++ )
   {
      for ( unsigned int j = 0; j < (unsigned int) csv_commands.data[i].size(); j++ )
      {
         t_csv->setItem( i, j, new QTableWidgetItem( csv_commands.data[i][j] ) );
      }
   }
}


void US_Hydrodyn_Pdb_Tool_Merge::run_one()
{
#if defined( USPTM_DEBUG_RUN_ONE )
   us_qdebug( "run one" );
#endif

   filtered = false;

   // process trial

   // clear, reload pdbs into pdb_tool ? assume for now ok

   // grab from & to csvs

   csv csv_from = 
      ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->to_csv( lv_csv_from, 
                                                         ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv1 ,
                                                         false );

   // editor_msg("blue", "from csv name: " + csv_from.name );

   csv_to =
      ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->to_csv( lv_csv_to, 
                                                         ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv2[ ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv2_pos ] ,
                                                         false );

   // editor_msg("blue", "to csv name: " + csv_to.name );

   update_csv_commands_from_table();

   map < QString, range_entry > merge_map;
   map < QString, range_entry > fit_map;
   map < QString, range_entry > cut_map;

   map < QString, vector < QString > > cross_fit_chains;
   map < QString, vector < QString > > cross_cross_chains;

   range_entry merge_range;
   range_entry fit_range;
   range_entry cut_range;

   bool any_cuts = false;
   for ( unsigned int i = 0; i < (unsigned int) csv_commands.data.size(); i++ )
   {
      if ( csv_commands.data[ i ].size() < 7 ||
           csv_commands.num_data[ i ].size() < 7 )
      {
         editor_msg( "red", us_tr( "Internal error: Invalid command directive format" ) );
         running = false;
         update_enables();
         return;
      }
      
      if ( csv_commands.data[ i ][ 0 ].length() <= 1 )
      {
         merge_range.chain = csv_commands.data[ i ][ 0 ];
         merge_range.start = (unsigned int) csv_commands.num_data[ i ][ 1 ];
         merge_range.end   = (unsigned int) csv_commands.num_data[ i ][ 2 ];

         fit_range.chain   = csv_commands.data[ i ][ 0 ];
         fit_range.start   = (unsigned int) csv_commands.num_data[ i ][ 3 ];
         fit_range.end     = (unsigned int) csv_commands.num_data[ i ][ 4 ];

         cut_range.chain   = csv_commands.data[ i ][ 0 ];
         cut_range.start   = (unsigned int) csv_commands.num_data[ i ][ 5 ];
         cut_range.end     = (unsigned int) csv_commands.num_data[ i ][ 6 ];

         merge_map[ merge_range.chain ] = merge_range;
         // cout << "merge map " << merge_range << endl;
         fit_map  [ fit_range  .chain ] = fit_range;
         // cout << "fit map " << fit_range << endl;
         if ( cut_range.start )
         {
            any_cuts = true;
            cut_map  [ cut_range  .chain ] = cut_range;
            // cout << "cut map " << cut_range << endl;
         }
      } else {
         merge_range.chain = csv_commands.data[ i ][ 0 ];
         merge_range.start = (unsigned int) csv_commands.num_data[ i ][ 1 ];
         merge_range.end   = (unsigned int) csv_commands.num_data[ i ][ 2 ];

         fit_range.chain   = csv_commands.data[ i ][ 0 ];
         fit_range.start   = (unsigned int) csv_commands.num_data[ i ][ 3 ];
         fit_range.end     = (unsigned int) csv_commands.num_data[ i ][ 4 ];

         merge_map[ merge_range.chain ] = merge_range;
         fit_map  [ fit_range  .chain ] = fit_range;
         
         QString fit_chain;
         QString cross_chain;
         if ( !get_chains( merge_range.chain, fit_chain, cross_chain ) )
         {
            editor_msg( "red", QString( us_tr( "Internal error: Extra chain not in proper format <%1>, please label each chain" ) ).arg( merge_range.chain ) );
            continue;
         }
         cross_fit_chains  [ fit_chain   ].push_back( cross_chain );
         cross_cross_chains[ cross_chain ].push_back( fit_chain   );
      }
   }

   // check cross fit chains ranges,
   // if they are in the merge ranges, copy over the to range for the base chain

   for ( map < QString, range_entry >::iterator it = fit_map.begin();
         it != fit_map.end();
         it++ )
   {
      QString fit_chain;
      QString cross_chain;
      if ( get_chains( it->first, fit_chain, cross_chain ) )
      {
         update_cache_range();
         if ( !cache_from_range_pos.count( fit_chain ) || !cache_to_range_pos.count( fit_chain ) )
         {
            editor_msg( "red", QString( us_tr( "Internal error: chain dereferencing <%1>" ) ).arg( fit_chain ) );
            continue;
         }
         if ( !cache_use_start.count( fit_chain ) )
         {
            editor_msg( "red", QString( us_tr( "Internal error: chain start map entry missing  <%1>" ) ).arg( fit_chain ) );
            continue;
         }

         range_entry from_range = cache_from_ranges[ cache_from_range_pos[ fit_chain ] ];
         range_entry to_range   = cache_to_ranges  [ cache_to_range_pos  [ fit_chain ] ];
         bool        use_start  = cache_use_start  [ fit_chain ];

         // is are fit range for the cross outside of the to_range ?
         if ( ( use_start  && it->second.start < to_range.start ) ||
              ( !use_start && it->second.end   > to_range.end   ) )
         {
            if ( !fit_map.count( fit_chain ) )
            {
               editor_msg( "red", QString( us_tr( "Internal error: chain fit map entry missing  <%1>" ) ).arg( fit_chain ) );
               continue;
            }
            editor_msg( "blue", QString( us_tr( "Notice: setting fitting for extra chain %1 to fit region for chain %2" ) ).arg( cross_chain ).arg( fit_chain ) );
            it->second = fit_map[ fit_chain ];
         }
      }
   }

   // for each chain, a set of fit points (currently on CA)
   map < QString, vector < point > > from_fit_points;
   map < QString, vector < point > > to_fit_points;

   csv csv_merge;
   csv csv_fit;

   // vector of merge_data for each chain

   map < QString, vector < vector < QString > > > merge_data;

   map < QString, bool > extra_chain_reported;
   map < QString, bool > extra_cross_chain_reported;

   // extract the merge & fit bits
   {
      csv_merge = csv_from;
      csv_merge.data.clear( );
      csv_fit = csv_from;
      csv_fit.data.clear( );

      for ( unsigned int i = 0; i < (unsigned int) csv_from.data.size(); i++ )
      {
         if ( csv_from.data[ i ].size() < 14 )
         {
            editor_msg( "red", us_tr( "Internal error: Invalid \"to\" csv" ) );
            running = false;
            update_enables();
            return;
         }

         QString chain    = csv_from.data[ i ][ 1 ];
         unsigned int pos = csv_from.data[ i ][ 3 ].toUInt();
         QString atom     = csv_from.data[ i ][ 4 ];
         if (
             ( merge_map.count( chain ) &&
               merge_map[ chain ].start <= pos &&
               merge_map[ chain ].end   >= pos ) )
         {
            csv_merge.data.push_back( csv_from.data[ i ] );
            merge_data[ chain ].push_back( csv_from.data[ i ] );
         } 

         if (
             ( fit_map.count( chain ) &&
               fit_map[ chain ].start <= pos &&
               fit_map[ chain ].end   >= pos ) )
         {
            csv_fit.data.push_back( csv_from.data[ i ] );
         } 

         if ( fit_map.count( chain ) &&
              fit_map[ chain ].start <= pos &&
              fit_map[ chain ].end   >= pos &&
              atom == " CA " ) 
         {
            point p;
            p.axis[ 0 ] = csv_from.data[ i ][ 8  ].toDouble();
            p.axis[ 1 ] = csv_from.data[ i ][ 9  ].toDouble();
            p.axis[ 2 ] = csv_from.data[ i ][ 10 ].toDouble();
            from_fit_points[ chain ].push_back( p );
            // editor_msg("blue", QString("adding fit point for %1 %2 %3").arg(chain).arg(pos).arg(atom));
         }

         // check for cross/extra chains

         if ( cross_fit_chains.count( chain ) )
         {
            for ( unsigned int j = 0; j < (unsigned int) cross_fit_chains[ chain ].size(); j++ )
            {
               QString fit_chain   = chain;
               QString cross_chain = cross_fit_chains[ fit_chain ][ j ];
               QString chain       = QString( "%1+%2" ).arg( fit_chain ).arg( cross_chain );
               if ( !extra_chain_reported.count( chain ) )
               {
                  editor_msg( "blue", QString( "Adding extra fit chains for %1" ).arg( chain ) );
                  extra_chain_reported[ chain ] = true;
               }

               if ( fit_map.count( chain ) &&
                    fit_map[ chain ].start <= pos &&
                    fit_map[ chain ].end   >= pos &&
                    atom == " CA " ) 
               {
                  point p;
                  p.axis[ 0 ] = csv_from.data[ i ][ 8  ].toDouble();
                  p.axis[ 1 ] = csv_from.data[ i ][ 9  ].toDouble();
                  p.axis[ 2 ] = csv_from.data[ i ][ 10 ].toDouble();
                  from_fit_points[ chain ].push_back( p );
                  // editor_msg("blue", QString("adding fit point for %1 %2 %3").arg(chain).arg(pos).arg(atom));
               }
            }
         }               

         if ( cross_cross_chains.count( chain ) )
         {
            for ( unsigned int j = 0; j < (unsigned int) cross_cross_chains[ chain ].size(); j++ )
            {
               QString cross_chain = chain;
               QString fit_chain   = cross_cross_chains[ cross_chain ][ j ];
               QString chain       = QString( "%1+%2" ).arg( fit_chain ).arg( cross_chain );
               if ( !extra_cross_chain_reported.count( chain ) )
               {
                  editor_msg( "blue", QString( "Adding extra merge chains for %1" ).arg( chain ) );
                  extra_cross_chain_reported[ chain ] = true;
               }
               // csv_merge.data.push_back( csv_from.data[ i ] );
               merge_data[ chain ].push_back( csv_from.data[ i ] );
            }
         }               
      }
      //      ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv_setup_keys( csv_merge );
      //      ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv_clipboard = csv_merge;
      //      ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv_clipboard.name = "merge bits of " + csv_from.name;
      //      ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv2_paste_new();

      //      ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv_setup_keys( csv_fit );
      //      ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv_clipboard = csv_fit;
      //      ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv_clipboard.name = "fit bits of " + csv_from.name;
      //      ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv2_paste_new();
   }

   // go thru the csv_to & cut
   if ( any_cuts )
   {
      csv new_csv = csv_to;
      new_csv.data.clear( );

      for ( unsigned int i = 0; i < (unsigned int) csv_to.data.size(); i++ )
      {
         if ( csv_to.data[ i ].size() < 14 )
         {
            editor_msg( "red", us_tr( "Internal error: Invalid \"to\" csv" ) );
            running = false;
            update_enables();
            return;
         }
         QString chain    = csv_to.data[ i ][ 1 ];
         unsigned int pos = csv_to.data[ i ][ 3 ].toUInt();
         if ( !cut_map.count( chain ) ||
              cut_map[ chain ].start > pos ||
              cut_map[ chain ].end   < pos )
         {
            new_csv.data.push_back( csv_to.data[ i ] );
         }
      }
      csv_to = new_csv;
      
      //      ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv_setup_keys( csv_to );
      //      ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv_clipboard = csv_to;
      //      ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv_clipboard.name = "after cuts " + csv_to.name;
      //      ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv2_paste_new();
   }

   // determine to ranges for merge position 
   map < QString, range_entry > to_range_map;

   for ( unsigned int i = 0; i < (unsigned int) csv_to.data.size(); i++ )
   {
      if ( csv_to.data[ i ].size() < 14 )
      {
         editor_msg( "red", us_tr( "Internal error: Invalid \"to\" csv" ) );
         running = false;
         update_enables();
         return;
      }
      QString chain    = csv_to.data[ i ][ 1 ];
      unsigned int pos = csv_to.data[ i ][ 3 ].toUInt();

      if ( to_range_map.count( chain ) )
      {
         if ( to_range_map[ chain ].start > pos )
         {
            to_range_map[ chain ].start = pos;
         }
         if ( to_range_map[ chain ].end < pos )
         {
            to_range_map[ chain ].end = pos;
         }
      } else {
         to_range_map[ chain ].chain = chain;
         to_range_map[ chain ].start = pos;
         to_range_map[ chain ].end   = pos;
      }
   }   

   // determine front or back
   map < QString, bool > chain_to_front;
   for ( map < QString, range_entry >::iterator it = to_range_map.begin();
         it != to_range_map.end();
         it++ )
   {
      if ( merge_map.count( it->first ) )
      {
         chain_to_front[ it->first ] = merge_map[ it->first ].end < it->second.start;
         // editor_msg( "blue", QString("Chain %1, merge chain to be added %2").arg( it->first ).arg( chain_to_front[ it->first ] ? "front" : "back" ) );
      }
   }
   // extract to fit points
   for ( unsigned int i = 0; i < (unsigned int) csv_to.data.size(); i++ )
   {
      if ( csv_to.data[ i ].size() < 14 )
      {
         editor_msg( "red", us_tr( "Internal error: Invalid \"to\" csv" ) );
         running = false;
         update_enables();
         return;
      }

      QString chain    = csv_to.data[ i ][ 1 ];
      unsigned int pos = csv_to.data[ i ][ 3 ].toUInt();
      QString atom     = csv_to.data[ i ][ 4 ];
      if ( fit_map.count( chain ) &&
           fit_map[ chain ].start <= pos &&
           fit_map[ chain ].end   >= pos &&
           atom == " CA " ) 
      {
         point p;
         p.axis[ 0 ] = csv_to.data[ i ][ 8  ].toDouble();
         p.axis[ 1 ] = csv_to.data[ i ][ 9  ].toDouble();
         p.axis[ 2 ] = csv_to.data[ i ][ 10 ].toDouble();
         to_fit_points[ chain ].push_back( p );
      }

      if ( cross_fit_chains.count( chain ) )
      {
         for ( unsigned int j = 0; j < (unsigned int) cross_fit_chains[ chain ].size(); j++ )
         {
            QString fit_chain   = chain;
            QString cross_chain = cross_fit_chains[ fit_chain ][ j ];
            QString chain       = QString( "%1+%2" ).arg( fit_chain ).arg( cross_chain );
            if ( !extra_chain_reported.count( chain ) )
            {
               editor_msg( "blue", QString( "Adding to extra fit chains for %1" ).arg( chain ) );
               extra_chain_reported[ chain ] = true;
            }

            if ( fit_map.count( chain ) &&
                 fit_map[ chain ].start <= pos &&
                 fit_map[ chain ].end   >= pos &&
                 atom == " CA " ) 
            {
               point p;
               p.axis[ 0 ] = csv_to.data[ i ][ 8  ].toDouble();
               p.axis[ 1 ] = csv_to.data[ i ][ 9  ].toDouble();
               p.axis[ 2 ] = csv_to.data[ i ][ 10 ].toDouble();
               to_fit_points[ chain ].push_back( p );
            }
         }
      }
   }

   // validate from/to fit points

   for ( map < QString, vector < point > >::iterator it = from_fit_points.begin();
         it != from_fit_points.end();
         it++ )
   {
      if ( !to_fit_points.count( it->first ) ||
           to_fit_points[ it->first ].size() != it->second.size() )
      {
         editor_msg( "red", QString( "Error (run): Fit point mismatch for %1 (%2 vs %3)" ).arg( it->first ).arg( to_fit_points[ it->first ].size() ).arg( it->second.size() ) );
         running = false;
         update_enables();
         return;
      }
#if defined( USPTM_DEBUG2 )
      else {
         cout << 
            QString( "Ok (run): Fit points for %1 (%2 vs %3)\n" ).arg( it->first ).arg( to_fit_points.count( it->first ) ? to_fit_points[ it->first ].size() : -1 ).arg( it->second.size() );
      }
#endif
   }

   // editor_msg("dark red", "transform next");

   // go through csv_to, for each chain with a transformation, add merge_bits transformed

   map < QString, bool > chain_added;
   QString last_chain = "__first__";

   {
      csv new_csv = csv_to;
      new_csv.data.clear( );

      map < unsigned int, int    > pos_type;  // for filter
      map < unsigned int, point  > pos_coord; // for filter
      map < unsigned int, double > pos_vdw;   // for filter

      for ( unsigned int i = 0; i < (unsigned int) csv_to.data.size(); i++ )
      {
         if ( csv_to.data[ i ].size() < 14 )
         {
            editor_msg( "red", us_tr( "Internal error: Invalid \"to\" csv" ) );
            running = false;
            update_enables();
            return;
         }

         QString chain    = csv_to.data[ i ][ 1 ];
         // unsigned int pos = csv_to.data[ i ][ 3 ].toUInt();
         if ( chain != last_chain &&
              merge_data.count( last_chain ) && 
              !chain_added.count( last_chain ) &&
              chain_to_front.count( last_chain ) &&
              chain_to_front[ last_chain ] == false )
         {
            // cout << QString("adding merge for previous chain %1 (current %2) before pos %3\n").arg(last_chain).arg(chain).arg(pos);
            chain_added[ last_chain ] = true;
            for ( unsigned int j = 0; j < (unsigned int) merge_data[ last_chain ].size(); j++ )
            {
               vector < point > ps;
               point p;

               p.axis[ 0 ] = merge_data[ last_chain ][ j ][ 8  ].toDouble();
               p.axis[ 1 ] = merge_data[ last_chain ][ j ][ 9  ].toDouble();
               p.axis[ 2 ] = merge_data[ last_chain ][ j ][ 10  ].toDouble();
               ps.push_back( p );
               vector < point > result;
               QString error_msg;
               if ( !((US_Hydrodyn *)us_hydrodyn)->atom_align( from_fit_points[ last_chain ], 
                                                               to_fit_points  [ last_chain ], 
                                                               ps, 
                                                               result,
                                                               error_msg ) )
               {
                  editor_msg( "red", error_msg );
                  running = false;
                  update_enables();
                  return;
               }
               unsigned int datapos = new_csv.data.size();
               new_csv.data.push_back( merge_data[ last_chain ][ j ] );
               new_csv.data[ datapos ][ 8  ] = QString( "%1" ).arg( result[ 0 ].axis[ 0 ] );
               new_csv.data[ datapos ][ 9  ] = QString( "%1" ).arg( result[ 0 ].axis[ 1 ] );
               new_csv.data[ datapos ][ 10 ] = QString( "%1" ).arg( result[ 0 ].axis[ 2 ] );
               pos_type [ datapos ] = 0;
               pos_coord[ datapos ] = result[ 0 ];
#if defined( USPTM_DEBUG_RUN_ONE )
               us_qdebug( QString( "type 0 checking for vdw of %1 at datapos %2" ).arg( new_csv.data[ datapos ][ 13 ] ).arg( datapos ) );
#endif
               pos_vdw  [ datapos ] = usu->atom_vdw.count( new_csv.data[ datapos ][ 13 ] ) ?
                  usu->atom_vdw[ new_csv.data[ datapos ][ 13 ] ] : 
                  ( usu->atom_vdw.count( "default" ) ? 
                    usu->atom_vdw[ "default" ] : 1.8 );
            }
         }            

         if ( ( merge_data.count( chain ) && 
                !chain_added.count( chain ) &&
                chain_to_front.count( chain ) &&
                chain_to_front[ chain ] == true ) )
         {
            // cout << QString("adding merge for chain %1 before pos %2\n").arg(chain).arg(pos);
            chain_added[ chain ] = true;
            // xform & add chain 
            for ( unsigned int j = 0; j < (unsigned int) merge_data[ chain ].size(); j++ )
            {
               vector < point > ps;
               point p;

               p.axis[ 0 ] = merge_data[ chain ][ j ][ 8  ].toDouble();
               p.axis[ 1 ] = merge_data[ chain ][ j ][ 9  ].toDouble();
               p.axis[ 2 ] = merge_data[ chain ][ j ][ 10  ].toDouble();
               ps.push_back( p );
               vector < point > result;
               QString error_msg;
               if ( !((US_Hydrodyn *)us_hydrodyn)->atom_align( from_fit_points[ chain ], 
                                                               to_fit_points  [ chain ], 
                                                               ps, 
                                                               result,
                                                               error_msg ) )
               {
                  editor_msg( "red", error_msg );
                  running = false;
                  update_enables();
                  return;
               }
               unsigned int datapos = new_csv.data.size();
               new_csv.data.push_back( merge_data[ chain ][ j ] );
               new_csv.data[ datapos ][ 8  ] = QString( "%1" ).arg( result[ 0 ].axis[ 0 ] );
               new_csv.data[ datapos ][ 9  ] = QString( "%1" ).arg( result[ 0 ].axis[ 1 ] );
               new_csv.data[ datapos ][ 10 ] = QString( "%1" ).arg( result[ 0 ].axis[ 2 ] );

               pos_type [ datapos ] = 1;
               pos_coord[ datapos ] = result[ 0 ];
#if defined( USPTM_DEBUG_RUN_ONE )
               us_qdebug( QString( "type 1 checking for vdw of %1 at datapos %2" ).arg( new_csv.data[ datapos ][ 13 ] ).arg( datapos ) );
#endif
               pos_vdw  [ datapos ] = usu->atom_vdw.count( new_csv.data[ datapos ][ 13 ] ) ?
                  usu->atom_vdw[ new_csv.data[ datapos ][ 13 ] ] : 
                  ( usu->atom_vdw.count( "default" ) ? 
                    usu->atom_vdw[ "default" ] : 1.8 );;
            }
         } 
         // cout << QString("pushing back chain %1 pos %2\n").arg(chain).arg(pos);
         new_csv.data.push_back( csv_to.data[ i ] );
         last_chain = chain;
      }

      if ( merge_data.count( last_chain ) && 
           !chain_added.count( last_chain ) &&
           chain_to_front.count( last_chain ) &&
           chain_to_front[ last_chain ] == false )
      {
         // cout << QString("adding merge at end previous chain %1\n").arg(last_chain);
         chain_added[ last_chain ] = true;
         for ( unsigned int j = 0; j < (unsigned int) merge_data[ last_chain ].size(); j++ )
         {
            // not yet xformed
            vector < point > ps;
            point p;
            
            p.axis[ 0 ] = merge_data[ last_chain ][ j ][ 8  ].toDouble();
            p.axis[ 1 ] = merge_data[ last_chain ][ j ][ 9  ].toDouble();
            p.axis[ 2 ] = merge_data[ last_chain ][ j ][ 10  ].toDouble();
            ps.push_back( p );
            vector < point > result;
            QString error_msg;
            if ( !((US_Hydrodyn *)us_hydrodyn)->atom_align( from_fit_points[ last_chain ], 
                                                            to_fit_points  [ last_chain ], 
                                                            ps, 
                                                            result,
                                                            error_msg ) )
            {
               editor_msg( "red", error_msg );
               running = false;
               update_enables();
               return;
            }
            unsigned int datapos = new_csv.data.size();
            new_csv.data.push_back( merge_data[ last_chain ][ j ] );
            new_csv.data[ datapos ][ 8  ] = QString( "%1" ).arg( result[ 0 ].axis[ 0 ] );
            new_csv.data[ datapos ][ 9  ] = QString( "%1" ).arg( result[ 0 ].axis[ 1 ] );
            new_csv.data[ datapos ][ 10 ] = QString( "%1" ).arg( result[ 0 ].axis[ 2 ] );
            pos_type [ datapos ] = 2;
            pos_coord[ datapos ] = result[ 0 ];
#if defined( USPTM_DEBUG_RUN_ONE )
            us_qdebug( QString( "type 2 checking for vdw of %1 at datapos %2" ).arg( new_csv.data[ datapos ][ 13 ] ).arg( datapos ) );
#endif
            pos_vdw  [ datapos ] = usu->atom_vdw.count( new_csv.data[ datapos ][ 13 ] ) ?
               usu->atom_vdw[ new_csv.data[ datapos ][ 13 ] ] : 
               ( usu->atom_vdw.count( "default" ) ? 
                 usu->atom_vdw[ "default" ] : 1.8 );;
         }
      }            
      
      // add extra chains if present
      
      // this could also be run over the merge map:
      //  for ( map < QString, vector < vector < QString > > > merge_data.begin(); etc

      for ( unsigned int i = 0; i < (unsigned int) csv_from.data.size(); i++ )
      {
         if ( csv_from.data[ i ].size() < 14 )
         {
            editor_msg( "red", us_tr( "Internal error: Invalid \"from\" csv" ) );
            running = false;
            update_enables();
            return;
         }

         QString chain    = csv_from.data[ i ][ 1 ];

         // is this a chain we need to add ?
         if ( !cross_cross_chains.count( chain ) )
         {
            continue;
         }

         if ( cross_cross_chains[ chain ].size() != 1 )
         {
            editor_msg( "red", QString( us_tr( "Error: only one fitting chain currently allowed per extra chain %1" ) ).arg( chain ) );
            running = false;
            update_enables();
            return;
         }

         {
            QString cross_chain = chain;
            QString fit_chain   = cross_cross_chains[ chain ][ 0 ];
            QString chain       = QString( "%1+%2" ).arg( fit_chain ).arg( cross_chain );

            if ( !from_fit_points.count( chain ) || !to_fit_points.count( chain ) )
            {
               editor_msg( "red", QString( us_tr( "Error: missing fit points for %1" ) ).arg( chain ) );
               running = false;
               update_enables();
               return;
            }

            if ( !merge_data.count( chain ) )
            {
               editor_msg( "red", QString( us_tr( "Error: merge data for %1" ) ).arg( chain ) );
               running = false;
               update_enables();
               return;
            }

            // make the transform:
         
            {
               vector < point > ps;
               point p;

               p.axis[ 0 ] = csv_from.data[ i ][ 8  ].toDouble();
               p.axis[ 1 ] = csv_from.data[ i ][ 9  ].toDouble();
               p.axis[ 2 ] = csv_from.data[ i ][ 10 ].toDouble();
               ps.push_back( p );
               vector < point > result;
               QString error_msg;
               if ( !((US_Hydrodyn *)us_hydrodyn)->atom_align( from_fit_points[ chain ], 
                                                               to_fit_points  [ chain ], 
                                                               ps, 
                                                               result,
                                                               error_msg ) )
               {
                  editor_msg( "red", error_msg );
                  running = false;
                  update_enables();
                  return;
               }
               unsigned int datapos = new_csv.data.size();
               new_csv.data.push_back( csv_from.data[ i ] );
               new_csv.data[ datapos ][ 8  ] = QString( "%1" ).arg( result[ 0 ].axis[ 0 ] );
               new_csv.data[ datapos ][ 9  ] = QString( "%1" ).arg( result[ 0 ].axis[ 1 ] );
               new_csv.data[ datapos ][ 10 ] = QString( "%1" ).arg( result[ 0 ].axis[ 2 ] );
               pos_type [ datapos ] = 3;
               pos_coord[ datapos ] = result[ 0 ];
#if defined( USPTM_DEBUG_RUN_ONE )
               us_qdebug( QString( "type 3 checking for vdw of %1 at datapos %2" ).arg( new_csv.data[ datapos ][ 13 ] ).arg( datapos ) );
#endif
               pos_vdw  [ datapos ] = usu->atom_vdw.count( new_csv.data[ datapos ][ 13 ] ) ?
                  usu->atom_vdw[ new_csv.data[ datapos ][ 13 ] ] : 
                  ( usu->atom_vdw.count( "default" ) ? 
                    usu->atom_vdw[ "default" ] : 1.8 );;
            }
         }
      }            

#if defined( USPTM_DEBUG_RUN_ONE )
      us_qdebug( "run one 2" );
#endif
      last_chain = "__first__";
      unsigned atompos = 0;
      for ( unsigned int i = 0; i < (unsigned int) new_csv.data.size(); i++ )
      {
         QString chain    = new_csv.data[ i ][ 1 ];
         if ( chain != last_chain )
         {
            // editor_msg("red", QString("chain <%1> last chain <%2>").arg( chain ).arg(last_chain));
            atompos = 0;
            last_chain = chain;
         }
         new_csv.data[ i ][ 5 ] = QString( "%1" ).arg( ++atompos );
      }

      if ( cb_filter->isChecked() ) {
         map < int, vector < unsigned int > > types_to_pos;

         for ( unsigned int i = 0; i < (unsigned int) new_csv.data.size(); i++ ) {
            if ( !pos_type.count( i ) ) {
               point p;
               p.axis[ 0 ] = new_csv.data[ i ][ 8  ].toDouble();
               p.axis[ 1 ] = new_csv.data[ i ][ 9  ].toDouble();
               p.axis[ 2 ] = new_csv.data[ i ][ 10 ].toDouble();
               pos_type [ i ] = 4;
               pos_coord[ i ] = p;
               pos_vdw  [ i ] =
                  usu->atom_vdw.count( new_csv.data[ i ][ 13 ] ) ?
                  usu->atom_vdw[ new_csv.data[ i ][ 13 ] ] : 
                  ( usu->atom_vdw.count( "default" ) ? 
                    usu->atom_vdw[ "default" ] : 1.8 );;
            }
            types_to_pos[ pos_type[ i ] ].push_back( i );
         }

#if defined( USPTM_DEBUG_RUN_ONE )
         for ( map < unsigned int, int >::iterator it = pos_type.begin();
               it != pos_type.end();
               ++it ) {
            us_qdebug(
                   QString( "pos %1 type %2 vdw %3 coord %4 %5 %6" )
                   .arg( it->first )
                   .arg( it->second )
                   .arg( pos_vdw  [ it->first ] )
                   .arg( pos_coord[ it->first ].axis[ 0 ] )
                   .arg( pos_coord[ it->first ].axis[ 1 ] )
                   .arg( pos_coord[ it->first ].axis[ 2 ] )
                   );
         }
#endif
         
         // compare type 4 to the others

         {
            double x2;
            double y2;
            double z2;
            double vdw;

            for ( map < int, vector < unsigned int > >::iterator it = types_to_pos.begin();
                  it != types_to_pos.end() && !filtered;
                  ++it ) {
               if ( it->first == 4 ) {
                  continue;
               }

               for ( int j = 0; j < (int) types_to_pos[ 4 ].size() && !filtered; ++j ) {
                  for ( int k = 0; k < (int) it->second.size(); ++k ) {
                     x2 = 
                        pos_coord[ types_to_pos[ 4 ][ j ] ].axis[ 0 ] -
                        pos_coord[ it->second       [ k ] ].axis[ 0 ];

                     y2 = 
                        pos_coord[ types_to_pos[ 4 ][ j ] ].axis[ 1 ] -
                        pos_coord[ it->second       [ k ] ].axis[ 1 ];

                     z2 = 
                        pos_coord[ types_to_pos[ 4 ][ j ] ].axis[ 2 ] -
                        pos_coord[ it->second       [ k ] ].axis[ 2 ];

                     x2 *= x2;
                     y2 *= y2;
                     z2 *= z2;

                     vdw = 
                        pos_vdw[ types_to_pos[ 4 ][ j ] ] +
                        pos_vdw[ it->second       [ k ] ];

                     if ( x2 + y2 + z2 <= vdw * vdw ) {
                        // exclude fit end
                        bool excluded = false;
                        QString chain4 = new_csv.data[ types_to_pos[ 4 ][ j ] ][ 1 ];
                        QString chaina = new_csv.data[ it->second       [ k ] ][ 1 ];
                        unsigned int res4no = new_csv.data[ types_to_pos[ 4 ][ j ] ][ 3 ].toUInt();
                        unsigned int resano = new_csv.data[ it->second       [ k ] ][ 3 ].toUInt();
                        
#if defined( USPTM_DEBUG_RUN_ONE )
                        us_qdebug( QString( "chain4 %1 resno %2 chainalt %3 resno %4" )
                                .arg( chain4 )
                                .arg( res4no )
                                .arg( chaina )
                                .arg( resano ) );
#endif

                        // exclude fit range for now
                        if (
                            ( fit_map.count( chain4 ) &&
                              fit_map[ chain4 ].start <= res4no &&
                              fit_map[ chain4 ].end   >= res4no ) ||
                            ( fit_map.count( chaina ) &&
                              fit_map[ chaina ].start <= resano &&
                              fit_map[ chaina ].end   >= resano ) ) {
                           excluded = true;
                        }
                             
                        // if ( chain4 == chaina &&
                        //      fit_map.count( chain4 ) &&
                        //      fit_map[ chain4 ].end == res4no &&
                        //      resano == 1 + res4no ) {
                        //    excluded = true;
                        // }

#if defined( USPTM_DEBUG_RUN_ONE )
                        if ( excluded ) {
                           us_qdebug( "excluded" );
                        }
#endif

                        if ( !excluded ) {
                           QString msg = 
                              QString( "first steric clash between types 4 %1 at pstns %2 %3 vdws %4 %5" )
                              .arg( it->first )
                              .arg( types_to_pos[ 4 ][ j ] )
                              .arg( it->second[ k ] )
                              .arg( pos_vdw[ types_to_pos[ 4 ][ j ] ] )
                              .arg( pos_vdw[ it->second[ k ] ] )
                              ;
                           editor_msg( "dark red", msg );
#if defined( USPTM_DEBUG_RUN_ONE )
                           us_qdebug( msg );
#endif

#if defined( USPTM_DEBUG_RUN_ONE )
                           {
                              int i = types_to_pos[ 4 ][ j ];
                              us_qdebug( 
                                     QString("")
                                     .sprintf(     
                                              "type 4 info:\n"
                                              "ATOM  %5d%5s%4s %1s%4d    %8.3f%8.3f%8.3f%6.2f%6.2f          %2s\n",
                                              new_csv.data[ i ][ 5  ].toUInt(),
                                              new_csv.data[ i ][ 4  ].toLatin1().data(),
                                              new_csv.data[ i ][ 2  ].toLatin1().data(),
                                              new_csv.data[ i ][ 1  ].toLatin1().data(),
                                              new_csv.data[ i ][ 3  ].toUInt(),
                                              new_csv.data[ i ][ 8  ].toFloat(),
                                              new_csv.data[ i ][ 9  ].toFloat(),
                                              new_csv.data[ i ][ 10 ].toFloat(),
                                              new_csv.data[ i ][ 11 ].toFloat(),
                                              new_csv.data[ i ][ 12 ].toFloat(),
                                              new_csv.data[ i ][ 13 ].toLatin1().data()
                                                   )
                                      );
                           }
                           {
                              int i = it->second[ k ];
                              us_qdebug( 
                                     QString("")
                                     .sprintf(     
                                              "type %d info:\n"
                                              "ATOM  %5d%5s%4s %1s%4d    %8.3f%8.3f%8.3f%6.2f%6.2f          %2s\n",
                                              it->first,
                                              new_csv.data[ i ][ 5  ].toUInt(),
                                              new_csv.data[ i ][ 4  ].toLatin1().data(),
                                              new_csv.data[ i ][ 2  ].toLatin1().data(),
                                              new_csv.data[ i ][ 1  ].toLatin1().data(),
                                              new_csv.data[ i ][ 3  ].toUInt(),
                                              new_csv.data[ i ][ 8  ].toFloat(),
                                              new_csv.data[ i ][ 9  ].toFloat(),
                                              new_csv.data[ i ][ 10 ].toFloat(),
                                              new_csv.data[ i ][ 11 ].toFloat(),
                                              new_csv.data[ i ][ 12 ].toFloat(),
                                              new_csv.data[ i ][ 13 ].toLatin1().data()
                                                   )
                                      );
                           }
#endif
                               
                           filtered = true;
                           break;
                        }
                     } 
                  }
               }
            }
         }
      }

#if defined( USPTM_DEBUG_RUN_ONE )
      us_qdebug( "run one 3" );
#endif

      csv_to = new_csv;
      
      ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv_setup_keys( csv_to );
      ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv_clipboard = csv_to;
      ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv_clipboard.name = "merged " + csv_to.name;
      ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv2_paste_new();
   }
}


void US_Hydrodyn_Pdb_Tool_Merge::extra_chains()
{
   if ( !extra_chains_list.size() )
   {
      return;
   }

   // for each existing chain and each extra chain, add cross chain if it doesn't already exist
   make_csv_chain_map();

   for ( map < QString, unsigned int >::iterator it = csv_chain_map.begin();
         it != csv_chain_map.end();
         it++ )
   {
      if ( it->first.length() <= 1 )
      {
         for ( unsigned int i = 0; i < (unsigned int) extra_chains_list.size(); i++ )
         {
            QString cross_chain = QString( "%1+%2" ).arg( it->first ).arg( extra_chains_list[ i ] );
            if ( !csv_chain_map.count( cross_chain ) )
            {
               unsigned int pos = ( unsigned int ) t_csv->rowCount();
               t_csv->setRowCount( pos + 1 );
               t_csv->setItem( pos, 0, new QTableWidgetItem( cross_chain ) );
               t_csv->setItem( pos, 3, new QTableWidgetItem( t_csv->item( it->second, 3 )->text() ) );
               t_csv->setItem( pos, 4, new QTableWidgetItem( t_csv->item( it->second, 4 )->text() ) );
               fill_csv_empty_items();
            }
         }
      }
   }
   extra_chains_done = true;
   update_enables();
}

void US_Hydrodyn_Pdb_Tool_Merge::only_closest()
{
   if ( !extra_chains_list.size() )
   {
      return;
   }

   // for each existing chain and each extra chain, add cross chain if it doesn't already exist
   make_csv_chain_map();

   map < QString, QStringList > chains_to_check;
   unsigned int size = 0;

   for ( map < QString, unsigned int >::iterator it = csv_chain_map.begin();
         it != csv_chain_map.end();
         it++ )
   {
      if ( it->first.length() <= 1 )
      {
         for ( unsigned int i = 0; i < (unsigned int) extra_chains_list.size(); i++ )
         {
            QString cross_chain = QString( "%1+%2" ).arg( it->first ).arg( extra_chains_list[ i ] );
            if ( !csv_chain_map.count( cross_chain ) )
            {
               chains_to_check[ extra_chains_list[ i ] ] << it->first;
               size++;
            }
         }
      }
   }
   
   if ( !size )
   {
      return;
   }

   running = true;
   update_enables();

   unsigned int pos = 0;
   progress->setValue( pos ); progress->setMaximum( size );

#if defined( USPTM_TIMERS )
   US_Timer             us_timers;
   us_timers            .clear_timers();
               
   us_timers.init_timer ( "only_closest() chains_to_check_loop" );
   us_timers.init_timer ( "only_closest() select chains in pdb editor" );
   us_timers.init_timer ( "only_closest() minimum pair distance" );
   us_timers.start_timer( "only_closest() chains_to_check_loop" );
#endif

   for ( map < QString, QStringList >::iterator it = chains_to_check.begin();
         it != chains_to_check.end();
         it++ )
   {
      double  minimum_distance = 1e99;
      QString minimum_key_a;
      QString minimum_key_b;

      for ( unsigned int i = 0; i < (unsigned int) it->second.size(); i++ )
      {
         progress->setValue( pos++ ); 
         qApp->processEvents();
         if ( !running )
         {
            return;
         }
         
         // find distance of nearest residue pair in chain
         editor_msg( "dark gray", QString( us_tr( "Finding closest pair of chain %1 and %2" ) ).arg( it->first ).arg( it->second[ i ] ) );
         // select chains in pdb editor for the fun of it
         if ( 0 ) {
            QStringList chains;
            chains << it->first;
            chains << it->second[ i ];
#if defined( USPTM_TIMERS )
            us_timers.start_timer ( "only_closest() select chains in pdb editor" );
#endif
            ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->select_chain( lv_csv_from, chains );
#if defined( USPTM_TIMERS )
            us_timers.end_timer ( "only_closest() select chains in pdb editor" );
#endif
         }
         QString key_chain_a;
         QString key_chain_b;

#if defined( USPTM_TIMERS )
         us_timers.start_timer ( "only_closest() minimum pair distance" );
#endif
         double distance = 
            ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)
            ->minimum_pair_distance( lv_csv_from, it->first, it->second[ i ], key_chain_a, key_chain_b );
#if defined( USPTM_TIMERS )
         us_timers.end_timer ( "only_closest() minimum pair distance" );
#endif

         if ( !i )
         {
            minimum_distance = distance;
            minimum_key_a    = key_chain_a;
            minimum_key_b    = key_chain_b;
         } else {
            if ( minimum_distance > distance )
            {
               minimum_distance = distance;
               minimum_key_a    = key_chain_a;
               minimum_key_b    = key_chain_b;
            }               
         }
      }
      editor_msg( "blue", QString( us_tr( "Closest chain to %1 [%2] is [%3] distance %4" ) )
                  .arg( it->first )
                  .arg( minimum_key_a )
                  .arg( minimum_key_b )
                  .arg( minimum_distance )
                  );

      {
         QStringList chain_a_list = (minimum_key_a ).split( "~" , Qt::SkipEmptyParts );
         QStringList chain_b_list = (minimum_key_b ).split( "~" , Qt::SkipEmptyParts );

         QString chain_a = chain_a_list.size() > 1 ? chain_a_list[ 1 ] : "?";
         QString chain_b = chain_b_list.size() > 1 ? chain_b_list[ 1 ] : "?";

         // QString residue_a = chain_a_list.size() > 2 ? chain_a_list[ 2 ] : "?";
         // residue_a.replace( QRegExp( "^\\D*" ), "" );
         // unsigned int residue_a_pos = residue_a.toUInt();

         QString residue_b = chain_b_list.size() > 2 ? chain_b_list[ 2 ] : "?";
         residue_b.replace( QRegExp( "^\\D*" ), "" );
         unsigned int residue_b_pos = residue_b.toUInt();

         if ( residue_b_pos < 2 )
         {
            residue_b_pos = 2;
         }

         QString cross_chain = QString( "%1+%2" ).arg( chain_b ).arg( chain_a );

         // need to add gap check here:
         unsigned int pos = ( unsigned int ) t_csv->rowCount();
         t_csv->setRowCount( pos + 1 );
         disconnect( t_csv, SIGNAL( cellChanged( int, int ) ), 0, 0 );
         t_csv->setItem( pos, 0, new QTableWidgetItem( cross_chain ) );
         t_csv->setItem( pos, 3, new QTableWidgetItem( QString( "%1" ).arg( residue_b_pos - 1 ) ) );
         t_csv->setItem( pos, 4, new QTableWidgetItem( QString( "%1" ).arg( residue_b_pos + 1 ) ) );
         fill_csv_empty_items();
         connect( t_csv, SIGNAL( cellChanged( int, int ) ), SLOT( table_value( int, int ) ) );
      }
   }
#if defined( USPTM_TIMERS )
   us_timers.end_timer( "only_closest() chains_to_check_loop" );
   QTextStream( stdout ) << us_timers.list_times() << "\n";
#endif
   progress->setValue( 1 ); progress->setMaximum( 1 );
   running = false;
   extra_chains_done = true;
   update_enables();
}

void US_Hydrodyn_Pdb_Tool_Merge::delete_row()
{
   for ( int i = t_csv->rowCount() - 1; i >= 0; i-- )
   {
      if ( t_csv->item( i , 0 )->isSelected() )
      {
         t_csv->removeRow( i );
      }
   }
   update_enables();
}

void US_Hydrodyn_Pdb_Tool_Merge::update_cache_range()
{
   if ( !cache_range_ok )
   {
      vector < range_entry > from_ranges;
      vector < range_entry > to_ranges;

      sel_to_range( lv_csv_from, from_ranges, false );
      sel_to_range( lv_csv_to  , to_ranges  , false );

      map < QString, unsigned int > from_range_pos;
      map < QString, unsigned int > to_range_pos;

      for ( unsigned int i = 0; i < (unsigned int) from_ranges.size(); i++ )
      {
         from_range_pos[ from_ranges[ i ].chain ] = i;
      }

      for ( unsigned int i = 0; i < (unsigned int) to_ranges.size(); i++ )
      {
         to_range_pos[ to_ranges[ i ].chain ] = i;
      }

      cache_from_ranges    = from_ranges;
      cache_to_ranges      = to_ranges;
      cache_from_range_pos = from_range_pos;
      cache_to_range_pos   = to_range_pos;
      cache_range_ok       = true;
   }   
}

void US_Hydrodyn_Pdb_Tool_Merge::table_value( int row, int col )
{
   update_cache_range();
   QString chain       = t_csv->item( row, 0 )->text();
   bool    extra_chain = chain.length() > 2;

   if ( extra_chain )
   {
      if ( col < 3 || col > 4 )
      {
         disconnect( t_csv, SIGNAL( cellChanged( int, int ) ), 0, 0 );
         t_csv->setItem( row, col, new QTableWidgetItem( "" ) );
         connect( t_csv, SIGNAL( cellChanged( int, int ) ), SLOT( table_value( int, int ) ) );
         return;
      }
      QString actual_chain = chain.length() == 2 ? " " : chain.left( 1 );
      if ( !cache_from_range_pos.count( actual_chain ) )
      {
         editor_msg( "red", QString( us_tr( "Internal error: chain dereferencing <%1>" ) ).arg( actual_chain ) );
         return;
      }
      if ( cache_from_ranges[ cache_from_range_pos[ actual_chain ] ].start > 
           t_csv->item( row, col )->text().toUInt() ) 
      {
         disconnect( t_csv, SIGNAL( cellChanged( int, int ) ), 0, 0 );
         t_csv->setItem( row, col, new QTableWidgetItem( QString( "%1" ).arg( cache_from_ranges[ cache_from_range_pos[ actual_chain ] ].start ) ) );
         connect( t_csv, SIGNAL( cellChanged( int, int ) ), SLOT( table_value( int, int ) ) );
      }
      if ( cache_from_ranges[ cache_from_range_pos[ actual_chain ] ].end <
           t_csv->item( row, col )->text().toUInt() ) 
      {
         disconnect( t_csv, SIGNAL( cellChanged( int, int ) ), 0, 0 );
         t_csv->setItem( row, col, new QTableWidgetItem( QString( "%1" ).arg( cache_from_ranges[ cache_from_range_pos[ actual_chain ] ].end ) ) );
         connect( t_csv, SIGNAL( cellChanged( int, int ) ), SLOT( table_value( int, int ) ) );
      }
      if ( col == 3 )
      {
         if ( t_csv->item( row, col )->text().toUInt() >
              t_csv->item( row, col + 1 )->text().toUInt() )
         {
            disconnect( t_csv, SIGNAL( cellChanged( int, int ) ), 0, 0 );
            t_csv->setItem( row, col + 1, new QTableWidgetItem( t_csv->item( row, col )->text() ) );
            connect( t_csv, SIGNAL( cellChanged( int, int ) ), SLOT( table_value( int, int ) ) );
         }
         return;
      }
      if ( col == 4 )
      {
         if ( t_csv->item( row, col - 1 )->text().toUInt() >
              t_csv->item( row, col )->text().toUInt() )
         {
            disconnect( t_csv, SIGNAL( cellChanged( int, int ) ), 0, 0 );
            t_csv->setItem( row, col - 1, new QTableWidgetItem( t_csv->item( row, col )->text() ) );
            connect( t_csv, SIGNAL( cellChanged( int, int ) ), SLOT( table_value( int, int ) ) );
         }
         return;
      }
      return;
   }

   if ( !cache_from_range_pos.count( chain ) || !cache_to_range_pos.count( chain ) )
   {
      editor_msg( "red", QString( us_tr( "Internal error: chain dereferencing <%1>" ) ).arg( chain ) );
      return;
   }

   if ( !cache_use_start.count( chain ) )
   {
      editor_msg( "red", QString( us_tr( "Internal error: chain start map entry missing  <%1>" ) ).arg( chain ) );
      return;
   }

   if ( col == 1 || col == 2 )
   {
      return recalc_from_merge( row, col );
   }

   if ( col == 3 || col == 4 )
   {
      return recalc_from_fit  ( row, col );
   }

   if ( col == 5 || col == 6 )
   {
      return recalc_from_cut  ( row, col );
   }
}
         
unsigned int US_Hydrodyn_Pdb_Tool_Merge::residue_length( bool         use_from,
                                                         QString      chain, 
                                                         unsigned int start_residue, 
                                                         unsigned int end_residue 
                                                        )
{
   update_cache_range();
   if ( use_from ? !cache_from_range_pos.count( chain ) : !cache_to_range_pos.count( chain ) )
   {
      editor_msg( "red", QString( us_tr( "Internal error: chain start map entry missing  <%1>" ) ).arg( chain ) );
      return 0;
   }

   range_entry * re =
      use_from ?
      & cache_from_ranges[ cache_from_range_pos[ chain ] ] :
      & cache_to_ranges  [ cache_to_range_pos  [ chain ] ] ;

   unsigned int count = 0;

   for ( unsigned int i = start_residue; i <= end_residue; i++ )
   {
      if ( !re->gaps.count( i ) )
      {
         count++;
      }
   }
   return count;
}

unsigned int US_Hydrodyn_Pdb_Tool_Merge::residue_offset_position( bool         use_from,
                                                                  QString      chain, 
                                                                  unsigned int start_residue, 
                                                                  int          offset
                                                                  )
{
   if ( offset == 0 )
   {
      return start_residue;
   }

   update_cache_range();
   if ( use_from ? !cache_from_range_pos.count( chain ) : !cache_to_range_pos.count( chain ) )
   {
      editor_msg( "red", QString( us_tr( "Internal error: chain start map entry missing  <%1>" ) ).arg( chain ) );
      return 0;
   }

   range_entry * re =
      use_from ?
      & cache_from_ranges[ cache_from_range_pos[ chain ] ] :
      & cache_to_ranges  [ cache_to_range_pos  [ chain ] ] ;

   int count = 0;
   int step  = offset > 0 ? 1 : -1;
   int pos   = start_residue;

   while ( count != offset )
   {
      pos += step;
      if ( !re->gaps.count( pos ) )
      {
         count += step;
      }
   }
   if ( pos < ( int ) re->start )
   {
      pos = ( int ) re->start;
   }
   if ( pos > ( int ) re->end )
   {
      pos = ( int ) re->end;
   }

   return ( unsigned int ) pos;
}

void US_Hydrodyn_Pdb_Tool_Merge::recalc_from_merge( int row, int col )
{
   if ( row >= t_csv->rowCount() )
   {
      editor_msg( "red", QString( us_tr( "Internal error: row out of range  <%1>" ) ).arg( row + 1 ) );
      return;
   }
      
   update_cache_range();

   QString chain       = t_csv->item( row, 0 )->text();
   bool    extra_chain = chain.length() > 2;
   
   if ( extra_chain )
   {
      // editor_msg( "red", QString( us_tr( "Internal error: update merge, but merge not valid for row <%1>" ) ).arg( row + 1 ) );
      return;
   }
      
   if ( !cache_from_range_pos.count( chain ) || !cache_to_range_pos.count( chain ) )
   {
      editor_msg( "red", QString( us_tr( "Internal error: chain dereferencing <%1>" ) ).arg( chain ) );
      return;
   }

   if ( !cache_use_start.count( chain ) )
   {
      editor_msg( "red", QString( us_tr( "Internal error: chain start map entry missing  <%1>" ) ).arg( chain ) );
      return;
   }

   unsigned current_merge_start = t_csv->item( row, 1 )->text().toUInt();
   unsigned current_merge_end   = t_csv->item( row, 2 )->text().toUInt();
   unsigned current_fit_start   = t_csv->item( row, 3 )->text().toUInt();
   unsigned current_fit_end     = t_csv->item( row, 4 )->text().toUInt();
   unsigned current_cut_start   = t_csv->item( row, 5 )->text().toUInt();
   unsigned current_cut_end     = t_csv->item( row, 6 )->text().toUInt();

   bool     current_merge_start_changed = false;
   bool     current_merge_end_changed   = false;
   bool     current_fit_start_changed   = false;
   bool     current_fit_end_changed     = false;
   bool     current_cut_start_changed   = false;
   bool     current_cut_end_changed     = false;
   bool     any_changed                 = false;

   range_entry from_range = cache_from_ranges[ cache_from_range_pos[ chain ] ];
   range_entry to_range   = cache_to_ranges  [ cache_to_range_pos  [ chain ] ];
   bool        use_start  = cache_use_start  [ chain ];

   unsigned int current_fit_length_from = residue_length( true , chain, current_fit_start  , current_fit_end   );
   unsigned int current_fit_length_to   = residue_length( false, chain, current_fit_start  , current_fit_end   );
   unsigned int current_fit_length      = 
      current_fit_length_from < current_fit_length_to ?
      current_fit_length_from : current_fit_length_to;

   unsigned int current_merge_length    = residue_length( true , chain, current_merge_start, current_merge_end );
   unsigned int current_cut_length      = residue_length( false, chain, current_cut_start  , current_cut_end   );

   cout << QString( "current_fit_length %1\n" ).arg( current_fit_length );
   cout << QString( "current_merge_length %1\n" ).arg( current_merge_length );
   cout << QString( "current_cut_length %1\n" ).arg( current_cut_length );

   unsigned int min_merge_start = 
      use_start ? from_range.start : residue_offset_position( true, chain, from_range.start, 2 );
   unsigned int max_merge_end   = 
      use_start ? residue_offset_position( true, chain, from_range.end, -2 ) : from_range.end;

   if ( min_merge_start > max_merge_end )
   {
      min_merge_start = 0;
      max_merge_end   = 0;
   }

   if ( current_merge_start < min_merge_start )
   {
      current_merge_start         = min_merge_start;
      current_merge_start_changed = true;
      any_changed                 = true;
      col                         = 1;
   }

   if ( current_merge_end > max_merge_end )
   {
      current_merge_end           = max_merge_end;
      current_merge_end_changed   = true;
      any_changed                 = true;
      col                         = 2;
   }

   if ( current_merge_start > current_merge_end )
   {
      if ( col == 1 )
      {
         current_merge_end           = current_merge_start;
         current_merge_end_changed   = true;
         any_changed                 = true;
      } else {
         current_merge_start         = current_merge_end;
         current_merge_start_changed = true;
         any_changed                 = true;
      }
   }

   // now set cut range

   if ( use_start )
   {
      if ( to_range.start <= current_merge_end && current_cut_end <= current_merge_end )
      {
         current_cut_start         = to_range.start;
         current_cut_end           = current_merge_end;
         while ( to_range.gaps.count( current_cut_end ) && current_cut_end < to_range.end )
         {
            current_cut_end++;
         }
         current_cut_start_changed = true;
         current_cut_end_changed   = true;
         any_changed               = true;
      }
   } else {
      if ( to_range.end >= current_merge_start && ( !current_cut_start || current_cut_start >= current_merge_start ) )
      {
         current_cut_start         = current_merge_start;
         while ( to_range.gaps.count( current_cut_start ) && current_cut_start > to_range.start )
         {
            current_cut_start--;
         }
         current_cut_end           = to_range.end;
         current_cut_start_changed = true;
         current_cut_end_changed   = true;
         any_changed               = true;
      }
   }

   // did fit range get effected by cut?
   // i.e. fit can not be cut
   if ( current_cut_start_changed || current_cut_end_changed )
   {
      if ( use_start )
      {
         if ( current_fit_start <= current_cut_end )
         {
            current_fit_start          = residue_offset_position( true,  chain, current_cut_end, 1 );
            unsigned int alt_fit_start = residue_offset_position( false, chain, current_cut_end, 1 );
            if ( current_fit_start < alt_fit_start )
            {
               current_fit_start = alt_fit_start;
            }
            current_fit_end            = residue_offset_position( true,  chain, current_fit_start, current_fit_length - 1 );
            unsigned int alt_fit_end   = residue_offset_position( false, chain, current_fit_start, current_fit_length - 1 );
            if ( current_fit_end < alt_fit_end )
            {
               current_fit_end = alt_fit_end;
            }
            if ( current_fit_end > to_range.end ||
                 current_fit_end > from_range.end )
            {
               current_fit_start = 0;
               current_fit_end   = 0;
            }
            current_fit_start_changed = true;
            current_fit_end_changed   = true;
            any_changed               = true;
         }
      } else {
         if ( current_fit_end >= current_cut_start )
         {
            current_fit_end            = residue_offset_position( true,  chain, current_cut_start, -1 );
            unsigned int alt_fit_end   = residue_offset_position( false, chain, current_cut_start, -1 );
            if ( current_fit_end < alt_fit_end  )
            {
               current_fit_end = alt_fit_end;
            }
            current_fit_start          = residue_offset_position( true,  chain, current_fit_end, - (int) ( current_fit_length - 1 ) );
            unsigned int alt_fit_start = residue_offset_position( false, chain, current_fit_end, - (int) ( current_fit_length - 1 ) );
            if ( current_fit_start < alt_fit_start )
            {
               current_fit_start = alt_fit_start;
            }
            if ( current_fit_start < to_range.start ||
                 current_fit_start < from_range.start )
            {
               current_fit_start = 0;
               current_fit_end   = 0;
            }
            current_fit_start_changed = true;
            current_fit_end_changed   = true;
            any_changed               = true;
         }
      }
   }

   if ( any_changed )
   {
      disconnect( t_csv, SIGNAL( cellChanged( int, int ) ), 0, 0 );
      if ( current_merge_start_changed )
      {
         QString toset = current_merge_start ? QString( "%1" ).arg( current_merge_start ) : "";
         t_csv->setItem( row, 1, new QTableWidgetItem( toset ) );
      }
      if ( current_merge_end_changed  )
      {
         QString toset = current_merge_end   ? QString( "%1" ).arg( current_merge_end   ) : "";
         t_csv->setItem( row, 2, new QTableWidgetItem( toset ) );
      }
      if ( current_fit_start_changed )
      {
         QString toset = current_fit_start   ? QString( "%1" ).arg( current_fit_start   ) : "";
         t_csv->setItem( row, 3, new QTableWidgetItem( toset ) );
      }
      if ( current_fit_end_changed  )
      {
         QString toset = current_fit_end     ? QString( "%1" ).arg( current_fit_end     ) : "";
         t_csv->setItem( row, 4, new QTableWidgetItem( toset ) );
      }
      if ( current_cut_start_changed )
      {
         QString toset = current_cut_start ? QString( "%1" ).arg( current_cut_start     ) : "";
         t_csv->setItem( row, 5, new QTableWidgetItem( toset ) );
      }
      if ( current_cut_end_changed  )
      {
         QString toset = current_cut_end   ? QString( "%1" ).arg( current_cut_end       ) : "";
         t_csv->setItem( row, 6, new QTableWidgetItem( toset ) );
      }
      connect( t_csv, SIGNAL( cellChanged( int, int ) ), SLOT( table_value( int, int ) ) );
   }
}         

void US_Hydrodyn_Pdb_Tool_Merge::recalc_from_fit( int row, int col )
{
   if ( row >= t_csv->rowCount() )
   {
      editor_msg( "red", QString( us_tr( "Internal error: row out of range  <%1>" ) ).arg( row + 1 ) );
      return;
   }
      
   update_cache_range();

   QString chain       = t_csv->item( row, 0 )->text();
   bool    extra_chain = chain.length() > 2;
   
   if ( extra_chain )
   {
      // editor_msg( "red", QString( us_tr( "Internal error: update fit, but fit not valid for row <%1>" ) ).arg( row + 1 ) );
      return;
   }
      
   if ( !cache_from_range_pos.count( chain ) || !cache_to_range_pos.count( chain ) )
   {
      editor_msg( "red", QString( us_tr( "Internal error: chain dereferencing <%1>" ) ).arg( chain ) );
      return;
   }

   if ( !cache_use_start.count( chain ) )
   {
      editor_msg( "red", QString( us_tr( "Internal error: chain start map entry missing  <%1>" ) ).arg( chain ) );
      return;
   }

   unsigned current_merge_start = t_csv->item( row, 1 )->text().toUInt();
   unsigned current_merge_end   = t_csv->item( row, 2 )->text().toUInt();
   unsigned current_fit_start   = t_csv->item( row, 3 )->text().toUInt();
   unsigned current_fit_end     = t_csv->item( row, 4 )->text().toUInt();
   unsigned current_cut_start   = t_csv->item( row, 5 )->text().toUInt();
   unsigned current_cut_end     = t_csv->item( row, 6 )->text().toUInt();

   bool     current_merge_start_changed = false;
   bool     current_merge_end_changed   = false;
   bool     current_fit_start_changed   = false;
   bool     current_fit_end_changed     = false;
   bool     current_cut_start_changed   = false;
   bool     current_cut_end_changed     = false;
   bool     any_changed                 = false;

   range_entry from_range = cache_from_ranges[ cache_from_range_pos[ chain ] ];
   range_entry to_range   = cache_to_ranges  [ cache_to_range_pos  [ chain ] ];
   bool        use_start  = cache_use_start  [ chain ];

   unsigned int current_fit_length_from = residue_length( true , chain, current_fit_start  , current_fit_end   );
   unsigned int current_fit_length_to   = residue_length( false, chain, current_fit_start  , current_fit_end   );
   unsigned int current_fit_length      = 
      current_fit_length_from < current_fit_length_to ?
      current_fit_length_from : current_fit_length_to;

   unsigned int current_merge_length    = residue_length( true , chain, current_merge_start, current_merge_end );
   unsigned int current_cut_length      = residue_length( false, chain, current_cut_start  , current_cut_end   );

   cout << QString( "current_fit_length %1\n" ).arg( current_fit_length );
   cout << QString( "current_merge_length %1\n" ).arg( current_merge_length );
   cout << QString( "current_cut_length %1\n" ).arg( current_cut_length );

   unsigned int min_fit_start     = from_range.start; // residue_offset_position( true , chain, from_range.start, 1 );
   unsigned int alt_min_fit_start = to_range.start;   // residue_offset_position( false, chain, to_range.start  , 1 );
   if ( min_fit_start < alt_min_fit_start )
   {
      min_fit_start = alt_min_fit_start;
   }

   unsigned int max_fit_end       = from_range.end; // residue_offset_position( true , chain, from_range.end , -1 );
   unsigned int alt_max_fit_end   = to_range.end;   // residue_offset_position( false, chain, to_range.end   , -1 );
   if ( max_fit_end > alt_max_fit_end )
   {
      max_fit_end = alt_max_fit_end;
   }

   cout << QString( "checking fit change:col %1 start %2 end %3 len %4\n" )
      .arg( col )
      .arg( current_fit_start )
      .arg( current_fit_end )
      .arg( current_fit_length )
      ;

   if ( current_fit_length < 3 )
   {
      if ( col == 3 )
      {
         current_fit_end          = residue_offset_position( false, chain, current_fit_start , 3 );
         unsigned int alt_fit_end = residue_offset_position( true , chain, current_fit_start , 3 );

         if ( current_fit_end < alt_fit_end )
         {
            current_fit_end = alt_fit_end;
         }
         current_fit_end_changed   = true;
         any_changed               = true;
      } else {
         current_fit_start          = residue_offset_position( false, chain, current_fit_end , -3 );
         unsigned int alt_fit_start = residue_offset_position( true , chain, current_fit_end , -3 );

         if ( current_fit_start > alt_fit_start )
         {
            current_fit_start = alt_fit_start;
         }
         current_fit_start_changed = true;
         any_changed               = true;
      }
      current_fit_length        = 3;
   }

   if ( current_fit_start < min_fit_start )
   {
      cout << "m1\n";
      current_fit_start         = min_fit_start;
      current_fit_start_changed = true;
      any_changed               = true;
   }
   if ( current_fit_end   > max_fit_end )
   {
      cout << "m2\n";
      current_fit_end           = max_fit_end;
      current_fit_end_changed   = true;
      any_changed               = true;
   }

   current_fit_length_from = residue_length( true , chain, current_fit_start  , current_fit_end   );
   current_fit_length_to   = residue_length( false, chain, current_fit_start  , current_fit_end   );
   current_fit_length      = 
      current_fit_length_from < current_fit_length_to ?
      current_fit_length_from : current_fit_length_to;
   if ( current_fit_length < 3 )
   {
      //      current_fit_end = residue_offset_position( false, chain, current_

      editor_msg( "red", us_tr( "Error: could not find 3 residues to fit in this chain\n" ) );
      current_fit_start         = 0;
      current_fit_end           = 0;
      current_fit_start_changed = true;
      current_fit_end_changed   = true;
      any_changed               = true;
   }

   // fit range must not be cut

   if ( use_start )
   {
      if ( current_cut_end >= current_fit_start )
      {
         cout << "m4\n";
         current_cut_start         = to_range.start;
         current_cut_end           = residue_offset_position( true , chain, current_fit_start, -1 );
         while ( to_range.gaps.count( current_cut_end ) && current_cut_end >= to_range.start )
         {
            current_cut_end--;
         }
         current_cut_start_changed = true;
         current_cut_end_changed   = true;
         any_changed               = true;
      }
   } else {
      if ( current_cut_start && current_cut_start <= current_fit_end )
      {
         cout << "m5\n";
         current_cut_start         = residue_offset_position( true , chain, current_fit_end, 1 );
         while ( to_range.gaps.count( current_cut_start ) && current_cut_start <  to_range.end )
         {
            current_cut_start++;
         }
         current_cut_end           = to_range.end;
         current_cut_start_changed = true;
         current_cut_end_changed   = true;
         any_changed               = true;
      }
   }

   // fit range must not be in merge

   if ( use_start )
   {
      if ( current_merge_end >= current_fit_start )
      {
         current_merge_end         = residue_offset_position( true , chain, current_fit_start, -1 );
         while ( from_range.gaps.count( current_merge_end ) && current_merge_end >= from_range.start )
         {
            current_merge_end--;
         }
         current_merge_end_changed = true;
         any_changed               = true;
      }
   } else {
      if ( current_merge_start <= current_fit_end )
      {
         current_merge_start       = residue_offset_position( true , chain, current_fit_end, 1 );
         while ( from_range.gaps.count( current_merge_start ) && current_merge_start < from_range.end )
         {
            current_merge_start++;
         }
         current_merge_start_changed = true;
         any_changed                 = true;
      }
   }

   if ( any_changed )
   {
      disconnect( t_csv, SIGNAL( cellChanged( int, int ) ), 0, 0 );
      if ( current_merge_start_changed )
      {
         QString toset = current_merge_start ? QString( "%1" ).arg( current_merge_start ) : "";
         t_csv->setItem( row, 1, new QTableWidgetItem( toset ) );
      }
      if ( current_merge_end_changed  )
      {
         QString toset = current_merge_end   ? QString( "%1" ).arg( current_merge_end   ) : "";
         t_csv->setItem( row, 2, new QTableWidgetItem( toset ) );
      }
      if ( current_fit_start_changed )
      {
         QString toset = current_fit_start   ? QString( "%1" ).arg( current_fit_start   ) : "";
         t_csv->setItem( row, 3, new QTableWidgetItem( toset ) );
      }
      if ( current_fit_end_changed  )
      {
         QString toset = current_fit_end     ? QString( "%1" ).arg( current_fit_end     ) : "";
         t_csv->setItem( row, 4, new QTableWidgetItem( toset ) );
      }
      if ( current_cut_start_changed )
      {
         QString toset = current_cut_start ? QString( "%1" ).arg( current_cut_start     ) : "";
         t_csv->setItem( row, 5, new QTableWidgetItem( toset ) );
      }
      if ( current_cut_end_changed  )
      {
         QString toset = current_cut_end   ? QString( "%1" ).arg( current_cut_end       ) : "";
         t_csv->setItem( row, 6, new QTableWidgetItem( toset ) );
      }
      connect( t_csv, SIGNAL( cellChanged( int, int ) ), SLOT( table_value( int, int ) ) );
   }
}

void US_Hydrodyn_Pdb_Tool_Merge::recalc_from_cut( int row, int col )
{
   if ( row >= t_csv->rowCount() )
   {
      editor_msg( "red", QString( us_tr( "Internal error: row out of range  <%1>" ) ).arg( row + 1 ) );
      return;
   }
      
   update_cache_range();

   QString chain       = t_csv->item( row, 0 )->text();
   bool    extra_chain = chain.length() > 2;
   
   if ( extra_chain )
   {
      // editor_msg( "red", QString( us_tr( "Internal error: update cut, but cut not valid for row <%1>" ) ).arg( row + 1 ) );
      return;
   }
      
   if ( !cache_from_range_pos.count( chain ) || !cache_to_range_pos.count( chain ) )
   {
      editor_msg( "red", QString( us_tr( "Internal error: chain dereferencing <%1>" ) ).arg( chain ) );
      return;
   }

   if ( !cache_use_start.count( chain ) )
   {
      editor_msg( "red", QString( us_tr( "Internal error: chain start map entry missing  <%1>" ) ).arg( chain ) );
      return;
   }

   unsigned current_merge_start = t_csv->item( row, 1 )->text().toUInt();
   unsigned current_merge_end   = t_csv->item( row, 2 )->text().toUInt();
   unsigned current_fit_start   = t_csv->item( row, 3 )->text().toUInt();
   unsigned current_fit_end     = t_csv->item( row, 4 )->text().toUInt();
   unsigned current_cut_start   = t_csv->item( row, 5 )->text().toUInt();
   unsigned current_cut_end     = t_csv->item( row, 6 )->text().toUInt();

   bool     current_merge_start_changed = false;
   bool     current_merge_end_changed   = false;
   bool     current_fit_start_changed   = false;
   bool     current_fit_end_changed     = false;
   bool     current_cut_start_changed   = false;
   bool     current_cut_end_changed     = false;
   bool     any_changed                 = false;

   range_entry from_range = cache_from_ranges[ cache_from_range_pos[ chain ] ];
   range_entry to_range   = cache_to_ranges  [ cache_to_range_pos  [ chain ] ];
   bool        use_start  = cache_use_start  [ chain ];

   unsigned int current_fit_length_from = residue_length( true , chain, current_fit_start  , current_fit_end   );
   unsigned int current_fit_length_to   = residue_length( false, chain, current_fit_start  , current_fit_end   );
   unsigned int current_fit_length      = 
      current_fit_length_from < current_fit_length_to ?
      current_fit_length_from : current_fit_length_to;

   if ( current_fit_length < 3 )
   {
      current_fit_length = 3;
   }

   unsigned int current_merge_length    = residue_length( true , chain, current_merge_start, current_merge_end );
   unsigned int current_cut_length      = residue_length( false, chain, current_cut_start  , current_cut_end   );

   cout << QString( "current_fit_length %1\n" ).arg( current_fit_length );
   cout << QString( "current_merge_length %1\n" ).arg( current_merge_length );
   cout << QString( "current_cut_length %1\n" ).arg( current_cut_length );

   // clear if current set one is zero

   if ( col == 5 && !current_cut_start )
   {
      current_cut_end         = 0;
      current_cut_end_changed = true;
      any_changed             = true;
   } else {
      if ( col == 6 && !current_cut_end )
      {
         current_cut_start         = 0;
         current_cut_start_changed = true;
         any_changed               = true;
      }
   }

   if ( col == 5 && current_cut_start )
   {
      if ( !current_cut_end || current_cut_start > current_cut_end )
      {
         current_cut_end           = current_cut_start;
         current_cut_end_changed   = true;
         any_changed               = true;
      }
   }
   if ( col == 6 && current_cut_end )
   {
      if ( !current_cut_start || current_cut_start > current_cut_end )
      {
         current_cut_start         = current_cut_end;
         current_cut_start_changed = true;
         any_changed               = true;
      }
   }

   if ( current_cut_start || current_cut_end )
   {
      if ( use_start )
      {
         if ( current_cut_start != to_range.start )
         {
            current_cut_start         = to_range.start;
            current_cut_start_changed = true;
            any_changed               = true;
         }
         unsigned int max_cut_end = 
            residue_offset_position( false, chain, to_range.end, -3 );
         if ( current_cut_end > max_cut_end )
         {
            current_cut_end           = max_cut_end;
            current_cut_end_changed   = true;
            any_changed               = true;
         }
      } else {
         if ( current_cut_end != to_range.end )
         {
            current_cut_end           = to_range.end;
            current_cut_end_changed   = true;
            any_changed               = true;
         }
         unsigned int min_cut_start = 
            residue_offset_position( false, chain, to_range.start, 3 );
         if ( current_cut_start < min_cut_start )
         {
            current_cut_start         = min_cut_start;
            current_cut_start_changed = true;
            any_changed               = true;
         }
      }
   }

   if ( current_cut_start > current_cut_end )
   {
      current_cut_start         = 0;
      current_cut_end           = 0;
      current_cut_end_changed   = true;
      current_cut_start_changed = true;
      any_changed               = true;
   }

   // set merge ragne
   if ( use_start )
   {
      if ( current_merge_end > current_cut_end )
      {
         current_merge_end         = current_cut_end;
         while ( from_range.gaps.count( current_merge_end ) && current_merge_end >= from_range.start )
         {
            current_merge_end--;
         }
         current_merge_end_changed = true;
         any_changed               = true;
         if ( current_merge_start > current_merge_end )
         {
            current_merge_start         = current_merge_end;
            current_merge_start_changed = true;
         }
      }
   } else {
      if ( current_merge_start < current_cut_start )
      {
         current_merge_start       = current_cut_start;
         while ( from_range.gaps.count( current_merge_start ) && current_merge_start <= from_range.start )
         {
            current_merge_start++;
         }
         current_merge_start_changed = true;
         any_changed                 = true;
         if ( current_merge_end < current_merge_start )
         {
            current_merge_end         = current_merge_start;
            current_merge_end_changed = true;
         }
      }
   }

   // did fit range get effected by cut?
   // i.e. fit can not be cut
   if ( use_start )
   {
      if ( current_fit_start <= current_cut_end )
      {
         current_fit_start          = residue_offset_position( true,  chain, current_cut_end, 1 );
         unsigned int alt_fit_start = residue_offset_position( false, chain, current_cut_end, 1 );
         if ( current_fit_start < alt_fit_start )
         {
            current_fit_start = alt_fit_start;
         }
         current_fit_end            = residue_offset_position( true,  chain, current_fit_start, current_fit_length - 1 );
         unsigned int alt_fit_end   = residue_offset_position( false, chain, current_fit_start, current_fit_length - 1 );
         if ( current_fit_end < alt_fit_end )
         {
            current_fit_end = alt_fit_end;
         }
         if ( current_fit_end > to_range.end ||
              current_fit_end > from_range.end )
         {
            current_fit_start = 0;
            current_fit_end   = 0;
         }
         current_fit_start_changed = true;
         current_fit_end_changed   = true;
         any_changed               = true;
      }
   } else {
      if ( current_fit_end >= current_cut_start )
      {
         current_fit_end            = residue_offset_position( true,  chain, current_cut_start, -1 );
         unsigned int alt_fit_end   = residue_offset_position( false, chain, current_cut_start, -1 );
         if ( current_fit_end < alt_fit_end  )
         {
            current_fit_end = alt_fit_end;
         }
         current_fit_start          = residue_offset_position( true,  chain, current_fit_end, - (int) ( current_fit_length - 1 ) );
         unsigned int alt_fit_start = residue_offset_position( false, chain, current_fit_end, - (int) ( current_fit_length - 1 ) );
         if ( current_fit_start < alt_fit_start )
         {
            current_fit_start = alt_fit_start;
         }
         if ( current_fit_start < to_range.start ||
              current_fit_start < from_range.start )
         {
            current_fit_start = 0;
            current_fit_end   = 0;
         }
         current_fit_start_changed = true;
         current_fit_end_changed   = true;
         any_changed               = true;
      }
   }

   if ( any_changed )
   {
      disconnect( t_csv, SIGNAL( cellChanged( int, int ) ), 0, 0 );
      if ( current_merge_start_changed )
      {
         QString toset = current_merge_start ? QString( "%1" ).arg( current_merge_start ) : "";
         t_csv->setItem( row, 1, new QTableWidgetItem( toset ) );
      }
      if ( current_merge_end_changed  )
      {
         QString toset = current_merge_end   ? QString( "%1" ).arg( current_merge_end   ) : "";
         t_csv->setItem( row, 2, new QTableWidgetItem( toset ) );
      }
      if ( current_fit_start_changed )
      {
         QString toset = current_fit_start   ? QString( "%1" ).arg( current_fit_start   ) : "";
         t_csv->setItem( row, 3, new QTableWidgetItem( toset ) );
      }
      if ( current_fit_end_changed  )
      {
         QString toset = current_fit_end     ? QString( "%1" ).arg( current_fit_end     ) : "";
         t_csv->setItem( row, 4, new QTableWidgetItem( toset ) );
      }
      if ( current_cut_start_changed )
      {
         QString toset = current_cut_start ? QString( "%1" ).arg( current_cut_start     ) : "";
         t_csv->setItem( row, 5, new QTableWidgetItem( toset ) );
      }
      if ( current_cut_end_changed  )
      {
         QString toset = current_cut_end   ? QString( "%1" ).arg( current_cut_end       ) : "";
         t_csv->setItem( row, 6, new QTableWidgetItem( toset ) );
      }
      connect( t_csv, SIGNAL( cellChanged( int, int ) ), SLOT( table_value( int, int ) ) );
   }
}

bool US_Hydrodyn_Pdb_Tool_Merge::get_chains( QString chain )
{
   if ( chain.length() <= 1 )
   {
      return false;
   }
   QStringList qsl = (chain ).split( "+" , Qt::SkipEmptyParts );
   if ( qsl.size() != 2 )
   {
      editor_msg( "red", QString( us_tr( "Error: Extra chain not in proper format <%1>, please label each chain" ) ).arg( chain ) );
      return false;
   }
   return true;
}

bool US_Hydrodyn_Pdb_Tool_Merge::get_chains( QString chain, QString &fit_chain, QString &cross_chain )
{
   fit_chain = chain;
   if ( fit_chain.length() <= 1 )
   {
      cross_chain = "";
      return false;
   }
   QStringList qsl = (chain ).split( "+" , Qt::SkipEmptyParts );
   if ( qsl.size() != 2 )
   {
      editor_msg( "red", QString( us_tr( "Error: Extra chain not in proper format <%1>, please label each chain" ) ).arg( chain ) );
      cross_chain = "";
      return false;
   }

   fit_chain   = qsl[ 0 ];
   cross_chain = qsl[ 1 ];
   return true;
}

void US_Hydrodyn_Pdb_Tool_Merge::fill_csv_empty_items() {
   int cols = (int) t_csv->columnCount();
   int rows = (int) t_csv->rowCount();

   disconnect( t_csv, SIGNAL( cellChanged( int, int ) ), 0, 0 );

   for ( int i = 0; i < rows; ++i ) {
      for ( int j = 0; j < cols; ++j ) {
         if ( !t_csv->item( i, j ) ) {
            // qDebug() << "setting empty t_csv item " << i << j;
            t_csv->setItem( i, j, new QTableWidgetItem( QString("" ) ) );
         }
      }
   }
   connect( t_csv, SIGNAL( cellChanged( int, int ) ), SLOT( table_value( int, int ) ) );
}

void US_Hydrodyn_Pdb_Tool_Merge::info_csv( const QString & msg ) {
   int cols = (int) t_csv->columnCount();
   int rows = (int) t_csv->rowCount();
   QTextStream( stdout )
      << "=============================================================\n"
      << msg << "\n" 
      << "-------------------------------------------------------------\n"
      ;

   for ( int i = 0; i < rows; ++i ) {
      QTextStream( stdout ) << "row " << i << " ";
      for ( int j = 0; j < cols; ++j ) {
         if ( t_csv->item( i, j ) ) {
            QTextStream( stdout ) <<  t_csv->item( i, j )->text();
         } else {
            QTextStream( stdout ) << "no item";
         }
         QTextStream( stdout ) << " | ";
      }
      QTextStream( stdout ) << "\n";
   }
   QTextStream( stdout )
      << "=============================================================\n"
      ;
}
