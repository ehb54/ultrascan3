#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
//Added by qt3to4:
#include <QBoxLayout>
#include <QLabel>
#include <QCloseEvent>
#include <QGridLayout>
#include <QTextStream>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
 //#include <Q3PopupMenu>

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

#define SLASH QDir::separator()

class sortable_qli_double {
public:
   double         d;
   QTreeWidgetItem  *lvi;
   bool operator < (const sortable_qli_double& objIn) const
   {
      return ( d < objIn.d );
   }
};

class lvipair
{
public:
   QTreeWidgetItem *lvi1;
   QTreeWidgetItem *lvi2;
   bool operator < (const lvipair& objIn) const
   {
      return ( lvi1 == objIn.lvi1 ? lvi2 < objIn.lvi2 : lvi1 < objIn.lvi1 );
   }
};

class sortable_qlipair_double {
public:
   double  d;
   lvipair lvip;
   bool operator < (const sortable_qlipair_double& objIn) const
   {
      return ( d < objIn.d );
   }
};

class sortable_dist_name {
public:
   double  dist;
   int     pairs;
   QString name;
   bool operator < (const sortable_dist_name& objIn) const
   {
      return ( dist < objIn.dist );
   }
};

double US_Hydrodyn_Pdb_Tool::pair_dist( QTreeWidgetItem *item1, QTreeWidgetItem *item2 )
{
   double dx = item1->text( 3 ).toDouble() - item2->text( 3 ).toDouble();
   double dy = item1->text( 4 ).toDouble() - item2->text( 4 ).toDouble();
   double dz = item1->text( 5 ).toDouble() - item2->text( 5 ).toDouble();

   return sqrt( dx * dx + dy * dy + dz * dz );
}

US_Hydrodyn_Pdb_Tool::US_Hydrodyn_Pdb_Tool(
                                           csv csv1,
                                           void *us_hydrodyn, 
                                           QWidget *p
                                           ) : QFrame( p )
{
   this->csv1 = csv1;
   this->us_hydrodyn = us_hydrodyn;
   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle( us_tr("US-SOMO: PDB editor") );
   csv2_pos = 0;
   csv2.resize(1);
   csv2_undos.resize(1);
   setupGUI();
   this->csv1.visible.resize( this->csv1.data.size() );
   this->csv1.selected.resize( this->csv1.selected.size() );
   this->csv1.open.resize( this->csv1.open.size() );
   csv_setup_keys( this->csv1 );
   selection_since_count_csv1 = true;
   selection_since_clean_csv1 = true;
   selection_since_count_csv2 = true;
   selection_since_clean_csv2 = true;
   pdb_tool_merge_widget = false;
   usu = new US_Saxs_Util();
   bm_active = true;

   if ( 
       !usu->load_mw_json( USglobal->config_list.system_dir + 
                           QDir::separator() + "etc" +
                           QDir::separator() + "mw.json" ) )
   {
      // us_qdebug( "no mw.json load" );
      bm_active = false;
   }

   if ( 
       !usu->load_vdw_json( USglobal->config_list.system_dir + 
                            QDir::separator() + "etc" +
                            QDir::separator() + "vdw.json" ) )
   {
      // us_qdebug( "no vdw.json load" );
      bm_active = false;
   }

   update_enables();

   editor_msg("blue", "THIS WINDOW IS UNDER DEVELOPMENT" );

   global_Xpos += 30;
   global_Ypos += 30;

   // unsigned int csv_height = lv_csv->rowHeight(0);
   // unsigned int csv_width = lv_csv->columnWidth(0);
   // for ( int i = 0; i < lv_csv->numRows(); i++ )
   // {
   // csv_height += lv_csv->rowHeight(i);
   // }
   // for ( int i = 0; i < lv_csv->numCols(); i++ )
   // {
   // csv_width += lv_csv->columnWidth(i);
   // }
   // if ( csv_height > 700 )
   // {
   // csv_height = 700;
   // }
   // if ( csv_width > 1000 )
   // {
   // csv_width = 1000;
   // }

   // cout << QString("csv size %1 %2\n").arg(csv_height).arg(csv_width);
   editor->setMaximumWidth( editor->width() * 3 );

   setGeometry(global_Xpos, global_Ypos, 0, 0 ); // csv_width, 100 + csv_height );
}
   
US_Hydrodyn_Pdb_Tool::~US_Hydrodyn_Pdb_Tool()
{
}

void US_Hydrodyn_Pdb_Tool::setupGUI()
{
   int minHeight1 = 22;

   lbl_title = new QLabel("PDB Editor", this);
   lbl_title->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette( PALET_FRAME );
   AUTFBACK( lbl_title );
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   // left pane

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
   QFrame *frame = new QFrame(this);
   frame->setMinimumHeight(minHeight1);

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
   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight1);

   m = new QMenuBar( frame );    m->setObjectName( "menu" );
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
   editor->setMinimumHeight(300);

   pb_split_pdb = new QPushButton(us_tr("Split"), this);
   pb_split_pdb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_split_pdb->setMinimumHeight(minHeight1);
   pb_split_pdb->setPalette( PALET_PUSHB );
   connect(pb_split_pdb, SIGNAL(clicked()), SLOT(split_pdb()));

   pb_join_pdbs = new QPushButton(us_tr("Join"), this);
   pb_join_pdbs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_join_pdbs->setMinimumHeight(minHeight1);
   pb_join_pdbs->setPalette( PALET_PUSHB );
   pb_join_pdbs->setEnabled( true );
   connect(pb_join_pdbs, SIGNAL(clicked()), SLOT(join_pdbs()));

   pb_merge = new QPushButton(us_tr("Cut/Splice"), this);
   pb_merge->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_merge->setMinimumHeight(minHeight1);
   pb_merge->setPalette( PALET_PUSHB );
   connect(pb_merge, SIGNAL(clicked()), SLOT(merge()));

   pb_renum_pdb = new QPushButton(us_tr("Renumber"), this);
   pb_renum_pdb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_renum_pdb->setMinimumHeight(minHeight1);
   pb_renum_pdb->setPalette( PALET_PUSHB );
   connect(pb_renum_pdb, SIGNAL(clicked()), SLOT(renum_pdb()));

   pb_hybrid_split = new QPushButton(us_tr("Hybrid extract"), this);
   pb_hybrid_split->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_hybrid_split->setMinimumHeight(minHeight1);
   pb_hybrid_split->setPalette( PALET_PUSHB );
   connect(pb_hybrid_split, SIGNAL(clicked()), SLOT(hybrid_split()));

   pb_h_to_chainX = new QPushButton(us_tr("H to chain X"), this);
   pb_h_to_chainX->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_h_to_chainX->setMinimumHeight(minHeight1);
   pb_h_to_chainX->setPalette( PALET_PUSHB );
   connect(pb_h_to_chainX, SIGNAL(clicked()), SLOT(h_to_chainX()));

   pb_help = new QPushButton(us_tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_cancel = new QPushButton(us_tr("Close"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   // center pane

   lbl_csv = new mQLabel(us_tr("Panel 1"), this);
   lbl_csv->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_csv->setMinimumHeight(minHeight1);
   lbl_csv->setPalette( PALET_LABEL );
   AUTFBACK( lbl_csv );
   lbl_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   connect( lbl_csv, SIGNAL( pressed() ), SLOT( hide_csv() ) );

   lv_csv = new QTreeWidget( this );
   lv_csv->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lv_csv->setPalette( PALET_EDIT );
   AUTFBACK( lv_csv );
   lv_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lv_csv->setEnabled(true);
   lv_csv->setMinimumWidth( 175 );

#if QT_VERSION < 0x040000
   lv_csv->addColumn( "Models" );
   for ( unsigned int i = 6; i < (unsigned int)csv1.header.size() - 2; i++ )
   {
      lv_csv->addColumn( csv1.header[i] );
   }
   lv_csv->addColumn( csv1.header.back() );
#else
   {
      QStringList qsl;
      qsl << "Models";
      for ( unsigned int i = 6; i < (unsigned int)csv1.header.size() - 2; i++ ) {
         qsl << csv1.header[i];
      }
      qsl << csv1.header.back();
      lv_csv->setColumnCount( qsl.size() );
      lv_csv->setHeaderLabels( qsl );
   }
#endif

   lv_csv->setSortingEnabled        ( false );
   lv_csv->setRootIsDecorated( true );
   lv_csv->setSelectionMode( QAbstractItemView::MultiSelection );
   connect(lv_csv, SIGNAL(itemSelectionChanged()), SLOT(csv_selection_changed()));

   panel1_widgets.push_back( lv_csv );

   lbl_csv->setText( csv1.name.isEmpty() ? us_tr( "Panel 1" ) : csv1.name );
   csv_to_lv( csv1, lv_csv );

   te_csv = new QTextEdit(this);
   te_csv->setPalette( PALET_NORMAL );
   AUTFBACK( te_csv );
   te_csv->setReadOnly(true);
   te_csv->setMaximumHeight( minHeight1 * 4 );
   
   panel1_widgets.push_back( te_csv );

   pb_csv_load_1 = new QPushButton(us_tr("Load"), this);
   pb_csv_load_1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_load_1->setMinimumHeight(minHeight1);
   pb_csv_load_1->setPalette( PALET_PUSHB );
   connect(pb_csv_load_1, SIGNAL(clicked()), SLOT(csv_load_1()));

   panel1_widgets.push_back( pb_csv_load_1 );

   pb_csv_load = new QPushButton(us_tr("Load All"), this);
   pb_csv_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_load->setMinimumHeight(minHeight1);
   pb_csv_load->setPalette( PALET_PUSHB );
   connect(pb_csv_load, SIGNAL(clicked()), SLOT(csv_load()));

   panel1_widgets.push_back( pb_csv_load );

   pb_csv_visualize = new QPushButton(us_tr("Visualize"), this);
   pb_csv_visualize->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_visualize->setMinimumHeight(minHeight1);
   pb_csv_visualize->setPalette( PALET_PUSHB );
   connect(pb_csv_visualize, SIGNAL(clicked()), SLOT(csv_visualize()));

   panel1_widgets.push_back( pb_csv_visualize );

   pb_csv_save = new QPushButton(us_tr("Save"), this);
   pb_csv_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_save->setMinimumHeight(minHeight1);
   pb_csv_save->setPalette( PALET_PUSHB );
   connect(pb_csv_save, SIGNAL(clicked()), SLOT(csv_save()));

   panel1_widgets.push_back( pb_csv_save );

   pb_csv_undo = new QPushButton(us_tr("Undo"), this);
   pb_csv_undo->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_undo->setMinimumHeight(minHeight1);
   pb_csv_undo->setPalette( PALET_PUSHB );
   connect(pb_csv_undo, SIGNAL(clicked()), SLOT(csv_undo()));

   panel1_widgets.push_back( pb_csv_undo );

   pb_csv_clear = new QPushButton(us_tr("Clear"), this);
   pb_csv_clear->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_clear->setMinimumHeight(minHeight1);
   pb_csv_clear->setPalette( PALET_PUSHB );
   connect(pb_csv_clear, SIGNAL(clicked( )), SLOT(csv_clear( )));

   panel1_widgets.push_back( pb_csv_clear );

   pb_csv_cut = new QPushButton(us_tr("Cut"), this);
   pb_csv_cut->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_cut->setMinimumHeight(minHeight1);
   pb_csv_cut->setPalette( PALET_PUSHB );
   connect(pb_csv_cut, SIGNAL(clicked()), SLOT(csv_cut()));

   panel1_widgets.push_back( pb_csv_cut );

   pb_csv_copy = new QPushButton(us_tr("Copy"), this);
   pb_csv_copy->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_copy->setMinimumHeight(minHeight1);
   pb_csv_copy->setPalette( PALET_PUSHB );
   connect(pb_csv_copy, SIGNAL(clicked()), SLOT(csv_copy()));

   panel1_widgets.push_back( pb_csv_copy );

   pb_csv_paste = new QPushButton(us_tr("Paste"), this);
   pb_csv_paste->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_paste->setMinimumHeight(minHeight1);
   pb_csv_paste->setPalette( PALET_PUSHB );
   connect(pb_csv_paste, SIGNAL(clicked()), SLOT(csv_paste()));

   panel1_widgets.push_back( pb_csv_paste );

   pb_csv_paste_new = new QPushButton(us_tr("Paste as new"), this);
   pb_csv_paste_new->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_paste_new->setMinimumHeight(minHeight1);
   pb_csv_paste_new->setPalette( PALET_PUSHB );
   connect(pb_csv_paste_new, SIGNAL(clicked()), SLOT(csv_paste_new()));

   panel1_widgets.push_back( pb_csv_paste_new );

   pb_csv_merge = new QPushButton(us_tr("Merge"), this);
   pb_csv_merge->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_merge->setMinimumHeight(minHeight1);
   pb_csv_merge->setPalette( PALET_PUSHB );
   connect(pb_csv_merge, SIGNAL(clicked()), SLOT(csv_merge()));

   panel1_widgets.push_back( pb_csv_merge );

   pb_csv_angle = new QPushButton(us_tr("Angle"), this);
   pb_csv_angle->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_angle->setMinimumHeight(minHeight1);
   pb_csv_angle->setPalette( PALET_PUSHB );
   connect(pb_csv_angle, SIGNAL(clicked()), SLOT(csv_angle()));

   panel1_widgets.push_back( pb_csv_angle );

   pb_csv_sol2wat = new QPushButton(us_tr("SOL->WAT"), this);
   pb_csv_sol2wat->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_sol2wat->setMinimumHeight(minHeight1);
   pb_csv_sol2wat->setPalette( PALET_PUSHB );
   connect(pb_csv_sol2wat, SIGNAL(clicked()), SLOT(csv_sol2wat()));

   panel1_widgets.push_back( pb_csv_sol2wat );

   pb_csv_reseq = new QPushButton(us_tr("Reseq"), this);
   pb_csv_reseq->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_reseq->setMinimumHeight(minHeight1);
   pb_csv_reseq->setPalette( PALET_PUSHB );
   connect(pb_csv_reseq, SIGNAL(clicked()), SLOT(csv_reseq()));

   panel1_widgets.push_back( pb_csv_reseq );

   pb_csv_check = new QPushButton(us_tr("Check"), this);
   pb_csv_check->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_check->setMinimumHeight(minHeight1);
   pb_csv_check->setPalette( PALET_PUSHB );
   connect(pb_csv_check, SIGNAL(clicked()), SLOT(csv_check()));

   panel1_widgets.push_back( pb_csv_check );

   pb_csv_sort = new QPushButton(us_tr("Sort"), this);
   pb_csv_sort->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_sort->setMinimumHeight(minHeight1);
   pb_csv_sort->setPalette( PALET_PUSHB );
   connect(pb_csv_sort, SIGNAL(clicked()), SLOT(csv_sort()));

   panel1_widgets.push_back( pb_csv_sort );

   pb_csv_find_alt = new QPushButton(us_tr("Find alternate matching residues"), this);
   pb_csv_find_alt->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_find_alt->setMinimumHeight(minHeight1);
   pb_csv_find_alt->setPalette( PALET_PUSHB );
   connect(pb_csv_find_alt, SIGNAL(clicked()), SLOT(csv_find_alt()));

   panel1_widgets.push_back( pb_csv_find_alt );

   pb_csv_bm = new QPushButton(us_tr("BM"), this);
   pb_csv_bm->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_bm->setMinimumHeight(minHeight1);
   pb_csv_bm->setPalette( PALET_PUSHB );
   connect(pb_csv_bm, SIGNAL(clicked()), SLOT(csv_bm()));

   panel1_widgets.push_back( pb_csv_bm );

   pb_csv_clash_report = new QPushButton(us_tr("PW distance"), this);
   pb_csv_clash_report->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_clash_report->setMinimumHeight(minHeight1);
   pb_csv_clash_report->setPalette( PALET_PUSHB );
   connect(pb_csv_clash_report, SIGNAL(clicked()), SLOT(csv_clash_report()));

   panel1_widgets.push_back( pb_csv_clash_report );

   pb_csv_sel = new QPushButton(us_tr("Select"), this);
   pb_csv_sel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_sel->setMinimumHeight(minHeight1);
   pb_csv_sel->setPalette( PALET_PUSHB );
   connect(pb_csv_sel, SIGNAL(clicked()), SLOT(csv_sel()));

   panel1_widgets.push_back( pb_csv_sel );

   pb_csv_sel_clear = new QPushButton(us_tr("Clear"), this);
   pb_csv_sel_clear->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_sel_clear->setMinimumHeight(minHeight1);
   pb_csv_sel_clear->setPalette( PALET_PUSHB );
   connect(pb_csv_sel_clear, SIGNAL(clicked( )), SLOT(csv_sel_clear( )));

   panel1_widgets.push_back( pb_csv_sel_clear );

   pb_csv_sel_clean = new QPushButton(us_tr("Clean"), this);
   pb_csv_sel_clean->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_sel_clean->setMinimumHeight(minHeight1);
   pb_csv_sel_clean->setPalette( PALET_PUSHB );
   connect(pb_csv_sel_clean, SIGNAL(clicked()), SLOT(csv_sel_clean()));

   panel1_widgets.push_back( pb_csv_sel_clean );

   pb_csv_sel_invert = new QPushButton(us_tr("Invert"), this);
   pb_csv_sel_invert->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_sel_invert->setMinimumHeight(minHeight1);
   pb_csv_sel_invert->setPalette( PALET_PUSHB );
   connect(pb_csv_sel_invert, SIGNAL(clicked()), SLOT(csv_sel_invert()));

#if QT_VERSION >= 0x050000
   pb_csv_sel_invert->hide();
#else
   panel1_widgets.push_back( pb_csv_sel_invert );
#endif

   pb_csv_sel_chain = new QPushButton(us_tr("Chain"), this);
   pb_csv_sel_chain->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_sel_chain->setMinimumHeight(minHeight1);
   pb_csv_sel_chain->setPalette( PALET_PUSHB );
   connect(pb_csv_sel_chain, SIGNAL(clicked()), SLOT(csv_sel_chain()));

   panel1_widgets.push_back( pb_csv_sel_chain );

   pb_csv_sel_nearest_atoms = new QPushButton(us_tr("Nearest Atoms"), this);
   pb_csv_sel_nearest_atoms->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_sel_nearest_atoms->setMinimumHeight(minHeight1);
   pb_csv_sel_nearest_atoms->setPalette( PALET_PUSHB );
   connect(pb_csv_sel_nearest_atoms, SIGNAL(clicked()), SLOT(csv_sel_nearest_atoms()));

   panel1_widgets.push_back( pb_csv_sel_nearest_atoms );

   pb_csv_sel_nearest_residues = new QPushButton(us_tr("Nearest Residues"), this);
   pb_csv_sel_nearest_residues->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_sel_nearest_residues->setMinimumHeight(minHeight1);
   pb_csv_sel_nearest_residues->setPalette( PALET_PUSHB );
   connect(pb_csv_sel_nearest_residues, SIGNAL(clicked()), SLOT(csv_sel_nearest_residues()));

   panel1_widgets.push_back( pb_csv_sel_nearest_residues );

   lbl_csv_sel_msg = new QLabel("", this);
   lbl_csv_sel_msg->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_csv_sel_msg->setMinimumHeight(minHeight1);
   lbl_csv_sel_msg->setPalette( PALET_LABEL );
   AUTFBACK( lbl_csv_sel_msg );
   lbl_csv_sel_msg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   panel1_widgets.push_back( lbl_csv_sel_msg );

   // right pane

   lbl_csv2 = new mQLabel(us_tr("Panel 2"), this);
   lbl_csv2->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_csv2->setMinimumHeight(minHeight1);
   lbl_csv2->setPalette( PALET_LABEL );
   AUTFBACK( lbl_csv2 );
   lbl_csv2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   connect( lbl_csv2, SIGNAL( pressed() ), SLOT( hide_csv2() ) );

   lv_csv2 = new QTreeWidget( this );
   lv_csv2->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lv_csv2->setPalette( PALET_EDIT );
   AUTFBACK( lv_csv2 );
   lv_csv2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lv_csv2->setEnabled(true);
   lv_csv2->setMinimumWidth( 175 );

#if QT_VERSION < 0x040000
   lv_csv2->addColumn( "Models" );
   for ( unsigned int i = 6; i < (unsigned int)csv1.header.size(); i++ )
   {
      lv_csv2->addColumn( csv1.header[i] );
   }
#else
   {
      QStringList qsl;
      qsl << "Models";
      for ( unsigned int i = 6; i < (unsigned int)csv1.header.size() - 2; i++ ) {
         qsl << csv1.header[i];
      }
      lv_csv2->setColumnCount( qsl.size() );
      lv_csv2->setHeaderLabels( qsl );
   }
#endif

   lv_csv2->setSortingEnabled        ( false );
   lv_csv2->setRootIsDecorated( true );
   lv_csv2->setSelectionMode( QAbstractItemView::MultiSelection );
   connect(lv_csv2, SIGNAL(itemSelectionChanged()), SLOT(csv2_selection_changed()));

   panel2_widgets.push_back( lv_csv2 );

   te_csv2 = new QTextEdit(this);
   te_csv2->setPalette( PALET_NORMAL );
   AUTFBACK( te_csv2 );
   te_csv2->setReadOnly(true);
   te_csv2->setMaximumHeight( minHeight1 * 4 );

   panel2_widgets.push_back( te_csv2 );

   pb_csv2_load_1 = new QPushButton(us_tr("Load"), this);
   pb_csv2_load_1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_load_1->setMinimumHeight(minHeight1);
   pb_csv2_load_1->setPalette( PALET_PUSHB );
   connect(pb_csv2_load_1, SIGNAL(clicked()), SLOT(csv2_load_1()));

   panel2_widgets.push_back( pb_csv2_load_1 );

   pb_csv2_load = new QPushButton(us_tr("Load All"), this);
   pb_csv2_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_load->setMinimumHeight(minHeight1);
   pb_csv2_load->setPalette( PALET_PUSHB );
   connect(pb_csv2_load, SIGNAL(clicked()), SLOT(csv2_load()));

   panel2_widgets.push_back( pb_csv2_load );

   pb_csv2_visualize = new QPushButton(us_tr("Visualize"), this);
   pb_csv2_visualize->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_visualize->setMinimumHeight(minHeight1);
   pb_csv2_visualize->setPalette( PALET_PUSHB );
   connect(pb_csv2_visualize, SIGNAL(clicked()), SLOT(csv2_visualize()));

   panel2_widgets.push_back( pb_csv2_visualize );

   pb_csv2_dup = new QPushButton(us_tr("Dup"), this);
   pb_csv2_dup->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_dup->setMinimumHeight(minHeight1);
   pb_csv2_dup->setPalette( PALET_PUSHB );
   connect(pb_csv2_dup, SIGNAL(clicked()), SLOT(csv2_dup()));

   panel2_widgets.push_back( pb_csv2_dup );

   pb_csv2_save = new QPushButton(us_tr("Save"), this);
   pb_csv2_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_save->setMinimumHeight(minHeight1);
   pb_csv2_save->setPalette( PALET_PUSHB );
   connect(pb_csv2_save, SIGNAL(clicked()), SLOT(csv2_save()));

   panel2_widgets.push_back( pb_csv2_save );

   pb_csv2_undo = new QPushButton(us_tr("Undo"), this);
   pb_csv2_undo->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_undo->setMinimumHeight(minHeight1);
   pb_csv2_undo->setPalette( PALET_PUSHB );
   connect(pb_csv2_undo, SIGNAL(clicked()), SLOT(csv2_undo()));

   panel2_widgets.push_back( pb_csv2_undo );

   pb_csv2_clear = new QPushButton(us_tr("Clear"), this);
   pb_csv2_clear->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_clear->setMinimumHeight(minHeight1);
   pb_csv2_clear->setPalette( PALET_PUSHB );
   connect(pb_csv2_clear, SIGNAL(clicked( )), SLOT(csv2_clear( )));

   panel2_widgets.push_back( pb_csv2_clear );

   pb_csv2_cut = new QPushButton(us_tr("Cut"), this);
   pb_csv2_cut->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_cut->setMinimumHeight(minHeight1);
   pb_csv2_cut->setPalette( PALET_PUSHB );
   connect(pb_csv2_cut, SIGNAL(clicked()), SLOT(csv2_cut()));

   panel2_widgets.push_back( pb_csv2_cut );

   pb_csv2_copy = new QPushButton(us_tr("Copy"), this);
   pb_csv2_copy->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_copy->setMinimumHeight(minHeight1);
   pb_csv2_copy->setPalette( PALET_PUSHB );
   connect(pb_csv2_copy, SIGNAL(clicked()), SLOT(csv2_copy()));

   panel2_widgets.push_back( pb_csv2_copy );

   pb_csv2_paste = new QPushButton(us_tr("Paste"), this);
   pb_csv2_paste->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_paste->setMinimumHeight(minHeight1);
   pb_csv2_paste->setPalette( PALET_PUSHB );
   connect(pb_csv2_paste, SIGNAL(clicked()), SLOT(csv2_paste()));

   panel2_widgets.push_back( pb_csv2_paste );

   pb_csv2_paste_new = new QPushButton(us_tr("Paste as new"), this);
   pb_csv2_paste_new->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_paste_new->setMinimumHeight(minHeight1);
   pb_csv2_paste_new->setPalette( PALET_PUSHB );
   connect(pb_csv2_paste_new, SIGNAL(clicked()), SLOT(csv2_paste_new()));

   panel2_widgets.push_back( pb_csv2_paste_new );

   pb_csv2_merge = new QPushButton(us_tr("Merge"), this);
   pb_csv2_merge->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_merge->setMinimumHeight(minHeight1);
   pb_csv2_merge->setPalette( PALET_PUSHB );
   connect(pb_csv2_merge, SIGNAL(clicked()), SLOT(csv2_merge()));

   panel2_widgets.push_back( pb_csv2_merge );

   pb_csv2_angle = new QPushButton(us_tr("Angle"), this);
   pb_csv2_angle->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_angle->setMinimumHeight(minHeight1);
   pb_csv2_angle->setPalette( PALET_PUSHB );
   connect(pb_csv2_angle, SIGNAL(clicked()), SLOT(csv2_angle()));

   panel2_widgets.push_back( pb_csv2_angle );

   pb_csv2_sol2wat = new QPushButton(us_tr("SOL->WAT"), this);
   pb_csv2_sol2wat->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_sol2wat->setMinimumHeight(minHeight1);
   pb_csv2_sol2wat->setPalette( PALET_PUSHB );
   connect(pb_csv2_sol2wat, SIGNAL(clicked()), SLOT(csv2_sol2wat()));

   // panel2_widgets.push_back( pb_csv2_sol2wat );
   pb_csv2_sol2wat->hide();

   pb_csv2_reseq = new QPushButton(us_tr("Reseq"), this);
   pb_csv2_reseq->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_reseq->setMinimumHeight(minHeight1);
   pb_csv2_reseq->setPalette( PALET_PUSHB );
   connect(pb_csv2_reseq, SIGNAL(clicked()), SLOT(csv2_reseq()));

   panel2_widgets.push_back( pb_csv2_reseq );

   pb_csv2_check = new QPushButton(us_tr("Check"), this);
   pb_csv2_check->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_check->setMinimumHeight(minHeight1);
   pb_csv2_check->setPalette( PALET_PUSHB );
   connect(pb_csv2_check, SIGNAL(clicked()), SLOT(csv2_check()));

   panel2_widgets.push_back( pb_csv2_check );

   pb_csv2_sort = new QPushButton(us_tr("Sort"), this);
   pb_csv2_sort->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_sort->setMinimumHeight(minHeight1);
   pb_csv2_sort->setPalette( PALET_PUSHB );
   connect(pb_csv2_sort, SIGNAL(clicked()), SLOT(csv2_sort()));

   panel2_widgets.push_back( pb_csv2_sort );

   pb_csv2_find_alt = new QPushButton(us_tr("Find alternate matching residues"), this);
   pb_csv2_find_alt->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_find_alt->setMinimumHeight(minHeight1);
   pb_csv2_find_alt->setPalette( PALET_PUSHB );
   connect(pb_csv2_find_alt, SIGNAL(clicked()), SLOT(csv2_find_alt()));

   panel2_widgets.push_back( pb_csv2_find_alt );

   pb_csv2_bm = new QPushButton(us_tr("BM"), this);
   pb_csv2_bm->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_bm->setMinimumHeight(minHeight1);
   pb_csv2_bm->setPalette( PALET_PUSHB );
   connect(pb_csv2_bm, SIGNAL(clicked()), SLOT(csv2_bm()));

   panel2_widgets.push_back( pb_csv2_bm );

   pb_csv2_clash_report = new QPushButton(us_tr("PW Distance"), this);
   pb_csv2_clash_report->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_clash_report->setMinimumHeight(minHeight1);
   pb_csv2_clash_report->setPalette( PALET_PUSHB );
   connect(pb_csv2_clash_report, SIGNAL(clicked()), SLOT(csv2_clash_report()));

   panel2_widgets.push_back( pb_csv2_clash_report );

   pb_csv2_sel = new QPushButton(us_tr("Select"), this);
   pb_csv2_sel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_sel->setMinimumHeight(minHeight1);
   pb_csv2_sel->setPalette( PALET_PUSHB );
   connect(pb_csv2_sel, SIGNAL(clicked()), SLOT(csv2_sel()));

   panel2_widgets.push_back( pb_csv2_sel );

   pb_csv2_sel_clear = new QPushButton(us_tr("Clear"), this);
   pb_csv2_sel_clear->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_sel_clear->setMinimumHeight(minHeight1);
   pb_csv2_sel_clear->setPalette( PALET_PUSHB );
   connect(pb_csv2_sel_clear, SIGNAL(clicked( )), SLOT(csv2_sel_clear( )));

   panel2_widgets.push_back( pb_csv2_sel_clear );

   pb_csv2_sel_clean = new QPushButton(us_tr("Clean"), this);
   pb_csv2_sel_clean->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_sel_clean->setMinimumHeight(minHeight1);
   pb_csv2_sel_clean->setPalette( PALET_PUSHB );
   connect(pb_csv2_sel_clean, SIGNAL(clicked()), SLOT(csv2_sel_clean()));

   panel2_widgets.push_back( pb_csv2_sel_clean );

   pb_csv2_sel_invert = new QPushButton(us_tr("Invert"), this);
   pb_csv2_sel_invert->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_sel_invert->setMinimumHeight(minHeight1);
   pb_csv2_sel_invert->setPalette( PALET_PUSHB );
   connect(pb_csv2_sel_invert, SIGNAL(clicked()), SLOT(csv2_sel_invert()));

#if QT_VERSION >= 0x050000
   pb_csv2_sel_invert->hide();
#else
   panel2_widgets.push_back( pb_csv2_sel_invert );
#endif

   pb_csv2_sel_chain = new QPushButton(us_tr("Chain"), this);
   pb_csv2_sel_chain->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_sel_chain->setMinimumHeight(minHeight1);
   pb_csv2_sel_chain->setPalette( PALET_PUSHB );
   connect(pb_csv2_sel_chain, SIGNAL(clicked()), SLOT(csv2_sel_chain()));

   panel2_widgets.push_back( pb_csv2_sel_chain );

   pb_csv2_sel_nearest_atoms = new QPushButton(us_tr("Nearest Atoms"), this);
   pb_csv2_sel_nearest_atoms->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_sel_nearest_atoms->setMinimumHeight(minHeight1);
   pb_csv2_sel_nearest_atoms->setPalette( PALET_PUSHB );
   connect(pb_csv2_sel_nearest_atoms, SIGNAL(clicked()), SLOT(csv2_sel_nearest_atoms()));

   panel2_widgets.push_back( pb_csv2_sel_nearest_atoms );

   pb_csv2_sel_nearest_residues = new QPushButton(us_tr("Nearest Residues"), this);
   pb_csv2_sel_nearest_residues->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_sel_nearest_residues->setMinimumHeight(minHeight1);
   pb_csv2_sel_nearest_residues->setPalette( PALET_PUSHB );
   connect(pb_csv2_sel_nearest_residues, SIGNAL(clicked()), SLOT(csv2_sel_nearest_residues()));

   panel2_widgets.push_back( pb_csv2_sel_nearest_residues );

   lbl_csv2_sel_msg = new QLabel("", this);
   lbl_csv2_sel_msg->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_csv2_sel_msg->setMinimumHeight(minHeight1);
   lbl_csv2_sel_msg->setPalette( PALET_LABEL );
   AUTFBACK( lbl_csv2_sel_msg );
   lbl_csv2_sel_msg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   panel2_widgets.push_back( lbl_csv2_sel_msg );

   qwtw_wheel = new QwtWheel( this );
   qwtw_wheel->setOrientation  ( Qt::Vertical );
   qwtw_wheel->setMass         ( 1.0 );
   qwtw_wheel->setRange( 0.0, 0.0); qwtw_wheel->setSingleStep( 1 );
   // qwtw_wheel->setMinimumHeight( minHeight1 );
   connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );

   panel2_widgets.push_back( qwtw_wheel );

   lbl_pos_range = new QLabel("1\nof\n1", this);
   lbl_pos_range->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
   lbl_pos_range->setMaximumHeight( minHeight1 * 2 + minHeight1 / 2 );
   lbl_pos_range->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_pos_range );
   lbl_pos_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   panel2_widgets.push_back( lbl_pos_range );

   // build layout
   QGridLayout * gl_panes = new QGridLayout; gl_panes->setContentsMargins( 0, 0, 0, 0 ); gl_panes->setSpacing( 0 ); gl_panes->setSpacing( 1 ); gl_panes->setContentsMargins( 1, 1, 1, 1 );

   // left pane
   QBoxLayout * vbl_editor_group = new QVBoxLayout; vbl_editor_group->setContentsMargins( 0, 0, 0, 0 ); vbl_editor_group->setSpacing( 0 );
#if QT_VERSION < 0x040000 || !defined(Q_OS_MAC)
   vbl_editor_group->addWidget(frame);
#endif
   vbl_editor_group->addWidget(editor);

   gl_panes->addLayout( vbl_editor_group, 0, 0 );

   QBoxLayout * hbl_left_buttons_row_1 = new QHBoxLayout; hbl_left_buttons_row_1->setContentsMargins( 0, 0, 0, 0 ); hbl_left_buttons_row_1->setSpacing( 0 );
   hbl_left_buttons_row_1->addWidget( pb_split_pdb );
   hbl_left_buttons_row_1->addSpacing( 2 );
   hbl_left_buttons_row_1->addWidget( pb_join_pdbs );
   
   QBoxLayout * hbl_left_buttons_row_2 = new QHBoxLayout; hbl_left_buttons_row_2->setContentsMargins( 0, 0, 0, 0 ); hbl_left_buttons_row_2->setSpacing( 0 );
   hbl_left_buttons_row_2->addWidget( pb_merge );
   hbl_left_buttons_row_2->addWidget( pb_renum_pdb );

   QBoxLayout * hbl_left_buttons_row_3 = new QHBoxLayout; hbl_left_buttons_row_3->setContentsMargins( 0, 0, 0, 0 ); hbl_left_buttons_row_3->setSpacing( 0 );
   hbl_left_buttons_row_3->addWidget( pb_hybrid_split );
   hbl_left_buttons_row_3->addWidget( pb_h_to_chainX );

   QBoxLayout * hbl_left_buttons_row_4 = new QHBoxLayout; hbl_left_buttons_row_4->setContentsMargins( 0, 0, 0, 0 ); hbl_left_buttons_row_4->setSpacing( 0 );
   hbl_left_buttons_row_4->addWidget( pb_help );
   hbl_left_buttons_row_4->addSpacing( 2 );
   hbl_left_buttons_row_4->addWidget( pb_cancel );

   QBoxLayout * vbl_left_buttons = new QVBoxLayout; vbl_left_buttons->setContentsMargins( 0, 0, 0, 0 ); vbl_left_buttons->setSpacing( 0 );
   vbl_left_buttons->addLayout( hbl_left_buttons_row_1 );
   vbl_left_buttons->addLayout( hbl_left_buttons_row_2 );
   vbl_left_buttons->addLayout( hbl_left_buttons_row_3 );
   vbl_left_buttons->addLayout( hbl_left_buttons_row_4 );

   gl_panes->addLayout( vbl_left_buttons, 1, 0 );

   // center pane
   QBoxLayout * vbl_center_top = new QVBoxLayout; vbl_center_top->setContentsMargins( 0, 0, 0, 0 ); vbl_center_top->setSpacing( 0 );
   vbl_center_top->addWidget( lbl_csv );
   vbl_center_top->addWidget( lv_csv );
   vbl_center_top->addWidget( lbl_csv_sel_msg );
   vbl_center_top->addWidget( te_csv );

   gl_panes->addLayout( vbl_center_top, 0, 1 );

   QBoxLayout * hbl_center_buttons_row_1 = new QHBoxLayout; hbl_center_buttons_row_1->setContentsMargins( 0, 0, 0, 0 ); hbl_center_buttons_row_1->setSpacing( 0 );
   hbl_center_buttons_row_1->addWidget( pb_csv_load_1 );
   hbl_center_buttons_row_1->addSpacing( 2 );
   hbl_center_buttons_row_1->addWidget( pb_csv_load );
   hbl_center_buttons_row_1->addSpacing( 2 );
   hbl_center_buttons_row_1->addWidget( pb_csv_visualize );
   hbl_center_buttons_row_1->addSpacing( 2 );
   hbl_center_buttons_row_1->addWidget( pb_csv_save );
   hbl_center_buttons_row_1->addSpacing( 2 );
   hbl_center_buttons_row_1->addWidget( pb_csv_undo );
   hbl_center_buttons_row_1->addSpacing( 2 );
   hbl_center_buttons_row_1->addWidget( pb_csv_clear );
   
   QBoxLayout * hbl_center_buttons_row_2 = new QHBoxLayout; hbl_center_buttons_row_2->setContentsMargins( 0, 0, 0, 0 ); hbl_center_buttons_row_2->setSpacing( 0 );
   hbl_center_buttons_row_2->addWidget( pb_csv_cut );
   hbl_center_buttons_row_2->addSpacing( 2 );
   hbl_center_buttons_row_2->addWidget( pb_csv_copy );
   hbl_center_buttons_row_2->addSpacing( 2 );
   hbl_center_buttons_row_2->addWidget( pb_csv_paste );
   hbl_center_buttons_row_2->addSpacing( 2 );
   hbl_center_buttons_row_2->addWidget( pb_csv_paste_new );
   hbl_center_buttons_row_2->addSpacing( 2 );
   hbl_center_buttons_row_2->addWidget( pb_csv_merge );
   hbl_center_buttons_row_2->addSpacing( 2 );
   hbl_center_buttons_row_2->addWidget( pb_csv_angle );
   hbl_center_buttons_row_2->addSpacing( 2 );
   hbl_center_buttons_row_2->addWidget( pb_csv_sol2wat );

   QBoxLayout * hbl_center_buttons_row_3 = new QHBoxLayout; hbl_center_buttons_row_3->setContentsMargins( 0, 0, 0, 0 ); hbl_center_buttons_row_3->setSpacing( 0 );
   hbl_center_buttons_row_3->addWidget( pb_csv_reseq );
   hbl_center_buttons_row_3->addSpacing( 2 );
   hbl_center_buttons_row_3->addWidget( pb_csv_check );
   hbl_center_buttons_row_3->addSpacing( 2 );
   hbl_center_buttons_row_3->addWidget( pb_csv_sort );
   hbl_center_buttons_row_3->addSpacing( 2 );
   hbl_center_buttons_row_3->addWidget( pb_csv_find_alt );
   hbl_center_buttons_row_3->addSpacing( 2 );
   hbl_center_buttons_row_3->addWidget( pb_csv_bm );
   hbl_center_buttons_row_3->addSpacing( 2 );
   hbl_center_buttons_row_3->addWidget( pb_csv_clash_report );

   QBoxLayout * hbl_center_buttons_row_4 = new QHBoxLayout; hbl_center_buttons_row_4->setContentsMargins( 0, 0, 0, 0 ); hbl_center_buttons_row_4->setSpacing( 0 );
   hbl_center_buttons_row_4->addWidget( pb_csv_sel );
   hbl_center_buttons_row_4->addSpacing( 2 );
   hbl_center_buttons_row_4->addWidget( pb_csv_sel_clear );
   hbl_center_buttons_row_4->addSpacing( 2 );
   hbl_center_buttons_row_4->addWidget( pb_csv_sel_clean );
   hbl_center_buttons_row_4->addSpacing( 2 );
   hbl_center_buttons_row_4->addWidget( pb_csv_sel_invert );
   hbl_center_buttons_row_4->addSpacing( 2 );
   hbl_center_buttons_row_4->addWidget( pb_csv_sel_chain );
   hbl_center_buttons_row_4->addSpacing( 2 );
   hbl_center_buttons_row_4->addWidget( pb_csv_sel_nearest_atoms );
   hbl_center_buttons_row_4->addSpacing( 2 );
   hbl_center_buttons_row_4->addWidget( pb_csv_sel_nearest_residues );

   QBoxLayout * vbl_center_buttons = new QVBoxLayout; vbl_center_buttons->setContentsMargins( 0, 0, 0, 0 ); vbl_center_buttons->setSpacing( 0 );
   vbl_center_buttons->addLayout( hbl_center_buttons_row_1 );
   vbl_center_buttons->addLayout( hbl_center_buttons_row_2 );
   vbl_center_buttons->addLayout( hbl_center_buttons_row_3 );
   vbl_center_buttons->addLayout( hbl_center_buttons_row_4 );

   gl_panes->addLayout( vbl_center_buttons, 1, 1 );

   // right pane
   QBoxLayout * vbl_wheel = new QVBoxLayout; vbl_wheel->setContentsMargins( 0, 0, 0, 0 ); vbl_wheel->setSpacing( 0 );
   vbl_wheel->addWidget( qwtw_wheel );
   vbl_wheel->addWidget( lbl_pos_range );

   QBoxLayout * hbl_csv2_wheel = new QHBoxLayout; hbl_csv2_wheel->setContentsMargins( 0, 0, 0, 0 ); hbl_csv2_wheel->setSpacing( 0 );
   hbl_csv2_wheel->addWidget( lv_csv2 );
   hbl_csv2_wheel->addLayout( vbl_wheel );

   QBoxLayout * vbl_right_top = new QVBoxLayout; vbl_right_top->setContentsMargins( 0, 0, 0, 0 ); vbl_right_top->setSpacing( 0 );
   vbl_right_top->addWidget( lbl_csv2 );
   vbl_right_top->addLayout( hbl_csv2_wheel );
   vbl_right_top->addWidget( lbl_csv2_sel_msg );
   vbl_right_top->addWidget( te_csv2 );

   gl_panes->addLayout( vbl_right_top, 0, 2 );

   QBoxLayout * hbl_right_buttons_row_1 = new QHBoxLayout; hbl_right_buttons_row_1->setContentsMargins( 0, 0, 0, 0 ); hbl_right_buttons_row_1->setSpacing( 0 );
   hbl_right_buttons_row_1->addWidget( pb_csv2_load_1 );
   hbl_right_buttons_row_1->addSpacing( 2 );
   hbl_right_buttons_row_1->addWidget( pb_csv2_load );
   hbl_right_buttons_row_1->addSpacing( 2 );
   hbl_right_buttons_row_1->addWidget( pb_csv2_visualize );
   hbl_right_buttons_row_1->addSpacing( 2 );
   hbl_right_buttons_row_1->addWidget( pb_csv2_dup );
   hbl_right_buttons_row_1->addSpacing( 2 );
   hbl_right_buttons_row_1->addWidget( pb_csv2_save );
   hbl_right_buttons_row_1->addSpacing( 2 );
   hbl_right_buttons_row_1->addWidget( pb_csv2_undo );
   hbl_right_buttons_row_1->addSpacing( 2 );
   hbl_right_buttons_row_1->addWidget( pb_csv2_clear );
   
   QBoxLayout * hbl_right_buttons_row_2 = new QHBoxLayout; hbl_right_buttons_row_2->setContentsMargins( 0, 0, 0, 0 ); hbl_right_buttons_row_2->setSpacing( 0 );
   hbl_right_buttons_row_2->addWidget( pb_csv2_cut );
   hbl_right_buttons_row_2->addSpacing( 2 );
   hbl_right_buttons_row_2->addWidget( pb_csv2_copy );
   hbl_right_buttons_row_2->addSpacing( 2 );
   hbl_right_buttons_row_2->addWidget( pb_csv2_paste );
   hbl_right_buttons_row_2->addSpacing( 2 );
   hbl_right_buttons_row_2->addWidget( pb_csv2_paste_new );
   hbl_right_buttons_row_2->addSpacing( 2 );
   hbl_right_buttons_row_2->addWidget( pb_csv2_merge );
   hbl_right_buttons_row_2->addSpacing( 2 );
   hbl_right_buttons_row_2->addWidget( pb_csv2_angle );
   hbl_right_buttons_row_2->addSpacing( 2 );
   hbl_right_buttons_row_2->addWidget( pb_csv2_sol2wat );

   QBoxLayout * hbl_right_buttons_row_3 = new QHBoxLayout; hbl_right_buttons_row_3->setContentsMargins( 0, 0, 0, 0 ); hbl_right_buttons_row_3->setSpacing( 0 );
   hbl_right_buttons_row_3->addWidget( pb_csv2_reseq );
   hbl_right_buttons_row_3->addSpacing( 2 );
   hbl_right_buttons_row_3->addWidget( pb_csv2_check );
   hbl_right_buttons_row_3->addSpacing( 2 );
   hbl_right_buttons_row_3->addWidget( pb_csv2_sort );
   hbl_right_buttons_row_3->addSpacing( 2 );
   hbl_right_buttons_row_3->addWidget( pb_csv2_find_alt );
   hbl_right_buttons_row_3->addSpacing( 2 );
   hbl_right_buttons_row_3->addWidget( pb_csv2_bm );
   hbl_right_buttons_row_3->addSpacing( 2 );
   hbl_right_buttons_row_3->addWidget( pb_csv2_clash_report );

   QBoxLayout * hbl_right_buttons_row_4 = new QHBoxLayout; hbl_right_buttons_row_4->setContentsMargins( 0, 0, 0, 0 ); hbl_right_buttons_row_4->setSpacing( 0 );
   hbl_right_buttons_row_4->addWidget( pb_csv2_sel );
   hbl_right_buttons_row_4->addSpacing( 2 );
   hbl_right_buttons_row_4->addWidget( pb_csv2_sel_clear );
   hbl_right_buttons_row_4->addSpacing( 2 );
   hbl_right_buttons_row_4->addWidget( pb_csv2_sel_clean );
   hbl_right_buttons_row_4->addSpacing( 2 );
   hbl_right_buttons_row_4->addWidget( pb_csv2_sel_invert );
   hbl_right_buttons_row_4->addSpacing( 2 );
   hbl_right_buttons_row_4->addWidget( pb_csv2_sel_chain );
   hbl_right_buttons_row_4->addSpacing( 2 );
   hbl_right_buttons_row_4->addWidget( pb_csv2_sel_nearest_atoms );
   hbl_right_buttons_row_4->addSpacing( 2 );
   hbl_right_buttons_row_4->addWidget( pb_csv2_sel_nearest_residues );

   QBoxLayout * vbl_right_buttons = new QVBoxLayout; vbl_right_buttons->setContentsMargins( 0, 0, 0, 0 ); vbl_right_buttons->setSpacing( 0 );
   vbl_right_buttons->addLayout( hbl_right_buttons_row_1 );
   vbl_right_buttons->addLayout( hbl_right_buttons_row_2 );
   vbl_right_buttons->addLayout( hbl_right_buttons_row_3 );
   vbl_right_buttons->addLayout( hbl_right_buttons_row_4 );

   gl_panes->addLayout( vbl_right_buttons, 1, 2 );

   QVBoxLayout * background = new QVBoxLayout(this); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 );
   background->addSpacing( 2 );
   background->addWidget ( lbl_title );
   background->addSpacing( 2 );
   background->addLayout ( gl_panes );

   US_Hydrodyn::fixWinButtons( this );
}

void US_Hydrodyn_Pdb_Tool::cancel()
{
   close();
}

void US_Hydrodyn_Pdb_Tool::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo/somo_pdb_editor.html");
}

void US_Hydrodyn_Pdb_Tool::closeEvent(QCloseEvent *e)
{
   global_Xpos -= 30;
   global_Ypos -= 30;
   delete usu;

   if ( pdb_tool_merge_widget )
   {
      pdb_tool_merge_window->close();
   }

   e->accept();
}

void US_Hydrodyn_Pdb_Tool::clear_display()
{
   editor->clear( );
   editor->append("\n\n");
}

void US_Hydrodyn_Pdb_Tool::update_font()
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

void US_Hydrodyn_Pdb_Tool::save()
{
   QString use_dir = ((US_Hydrodyn *)us_hydrodyn)->somo_pdb_dir;
   ((US_Hydrodyn *)us_hydrodyn)->select_from_directory_history( use_dir, this );

   QString fn;
   fn = QFileDialog::getSaveFileName( this , windowTitle() + us_tr( ": Save" ) , use_dir , "*.pdb *.PDB" );
   if(!fn.isEmpty() )
   {
      ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( fn );
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

void US_Hydrodyn_Pdb_Tool::update_enables_csv()
{
   bool           any_csv_selected  = any_selected  ( lv_csv );
   bool           any_csv2_selected = any_selected  ( lv_csv2 );
   pdb_sel_count  counts            = count_selected( lv_csv );

   pb_csv_load_1               ->setEnabled( true );
   pb_csv_load                 ->setEnabled( true );
   pb_csv_visualize            ->setEnabled( csv1.data.size() );
   pb_csv_save                 ->setEnabled( csv1.data.size() );
   pb_csv_undo                 ->setEnabled( csv_undos.size() );
   pb_csv_clear                ->setEnabled( csv1.data.size() );
   pb_csv_cut                  ->setEnabled( any_csv_selected );
   pb_csv_copy                 ->setEnabled( any_csv_selected );
   pb_csv_paste                ->setEnabled( csv_clipboard.data.size() && csv1.data.size() );
   pb_csv_paste_new            ->setEnabled( csv_clipboard.data.size() );
   pb_csv_merge                ->setEnabled( any_csv_selected && merge_ok() );
   pb_csv2_merge               ->setEnabled( any_csv2_selected && merge_ok() );
   pb_csv_angle                ->setEnabled( counts.atoms == 3 );
   pb_csv_sol2wat              ->setEnabled( true /* counts.SOLs */ );
   pb_csv_reseq                ->setEnabled( csv1.data.size() );
   pb_csv_check                ->setEnabled( csv1.data.size() );
   pb_csv_sort                 ->setEnabled( counts.models > 1 );
   pb_csv_find_alt             ->setEnabled( counts.residues == 1 );
   pb_csv_bm                   ->setEnabled( bm_active && any_csv_selected );
   pb_csv_clash_report         ->setEnabled( any_csv_selected );
   pb_csv_sel                  ->setEnabled( csv1.data.size() );
   pb_csv_sel_clear            ->setEnabled( any_csv_selected );
   pb_csv_sel_clean            ->setEnabled( selection_since_clean_csv1 && any_csv_selected );
   pb_csv_sel_invert           ->setEnabled( csv1.data.size() );
   pb_csv_sel_chain            ->setEnabled( counts.chains > 1 );
   pb_csv_sel_nearest_atoms    ->setEnabled( any_csv_selected && counts.not_selected_atoms > 1 );
   pb_csv_sel_nearest_residues ->setEnabled( any_csv_selected && counts.not_selected_atoms > 1 );

   csv_sel_msg();

   if ( pdb_tool_merge_widget )
   {
      pdb_tool_merge_window->update_enables();
   }
}

void US_Hydrodyn_Pdb_Tool::update_enables_csv2()
{
   bool           any_csv_selected  = any_selected  ( lv_csv );
   bool           any_csv2_selected = any_selected  ( lv_csv2 );
   pdb_sel_count  counts            = count_selected( lv_csv2 );

   pb_csv2_load_1               ->setEnabled( true );
   pb_csv2_load                 ->setEnabled( true );
   pb_csv2_visualize            ->setEnabled( csv2[ csv2_pos ].data.size() );
   pb_csv2_dup                  ->setEnabled( csv1.data.size() );
   pb_csv2_save                 ->setEnabled( csv2[ csv2_pos ].data.size() );
   pb_csv2_undo                 ->setEnabled( csv2_undos.size() > csv2_pos && csv2_undos[ csv2_pos ].size() );
   pb_csv2_clear                ->setEnabled( csv2.size() > 1 || (csv2.size() == 1 && csv2[ 0 ].data.size() ) );
   pb_csv2_cut                  ->setEnabled( any_csv2_selected );
   pb_csv2_copy                 ->setEnabled( any_csv2_selected );
   pb_csv2_paste                ->setEnabled( csv_clipboard.data.size() && csv2[ csv2_pos ].data.size() );
   pb_csv2_paste_new            ->setEnabled( csv_clipboard.data.size() );
   pb_csv_merge                 ->setEnabled( any_csv_selected && merge_ok() );
   pb_csv2_merge                ->setEnabled( any_csv2_selected && merge_ok() );
   pb_csv2_angle                ->setEnabled( counts.atoms == 3 );
   pb_csv2_sol2wat              ->setEnabled( true /* counts.SOLs */ );
   pb_csv2_reseq                ->setEnabled( csv2[ csv2_pos ].data.size() );
   pb_csv2_check                ->setEnabled( csv2[ csv2_pos ].data.size() );
   pb_csv2_sort                 ->setEnabled( counts.models > 1 );
   pb_csv2_find_alt             ->setEnabled( counts.residues == 1 );
   pb_csv2_bm                   ->setEnabled( bm_active && any_csv_selected );
   pb_csv2_clash_report         ->setEnabled( any_csv2_selected );
   pb_csv2_sel                  ->setEnabled( csv2[ csv2_pos ].data.size() );
   pb_csv2_sel_clear            ->setEnabled( any_csv2_selected );
   pb_csv2_sel_clean            ->setEnabled( selection_since_clean_csv2 && any_csv2_selected );
   pb_csv2_sel_invert           ->setEnabled( csv2[ csv2_pos ].data.size()  );
   pb_csv2_sel_chain            ->setEnabled( counts.chains > 1 );
   pb_csv2_sel_nearest_atoms    ->setEnabled( any_csv2_selected && counts.not_selected_atoms > 1 );
   pb_csv2_sel_nearest_residues ->setEnabled( any_csv2_selected && counts.not_selected_atoms > 1 );

   csv2_sel_msg();

   if ( pdb_tool_merge_widget )
   {
      pdb_tool_merge_window->update_enables();
   }
}

void US_Hydrodyn_Pdb_Tool::update_enables()
{
   update_enables_csv();
   update_enables_csv2();
}

void US_Hydrodyn_Pdb_Tool::editor_msg( QString color, QString msg )
{
   QColor save_color = editor->textColor();
   editor->setTextColor(color);
   editor->append(msg);
   editor->setTextColor(save_color);
}

void US_Hydrodyn_Pdb_Tool::csv_msg( QString color, QString msg )
{
   QColor save_color = te_csv->textColor();
   te_csv->setTextColor(color);
   te_csv->append(msg);
   te_csv->setTextColor(save_color);
}

void US_Hydrodyn_Pdb_Tool::csv2_msg( QString color, QString msg )
{
   QColor save_color = te_csv2->textColor();
   te_csv2->setTextColor(color);
   te_csv2->append(msg);
   te_csv2->setTextColor(save_color);
}

void US_Hydrodyn_Pdb_Tool::csv_load()
{
   QString f;
   load( lv_csv, f );
}

void US_Hydrodyn_Pdb_Tool::csv_load_1()
{
   QString f;
   load( lv_csv, f, true );
}

void US_Hydrodyn_Pdb_Tool::csv_save()
{
   save_csv( lv_csv );
}

void US_Hydrodyn_Pdb_Tool::csv_undo()
{
   if ( !csv_undos.size() )
   {
      editor_msg("red", "Internal error: undo called with empty undo buffer" );
      return;
   }
   csv1 = csv_undos[ csv_undos.size() - 1 ];
   csv_undos.pop_back();
   csv_to_lv( csv1, lv_csv );
   update_enables_csv();
}

void US_Hydrodyn_Pdb_Tool::csv_cut()
{
   // lv_csv_undo.push_back( *lv_csv );
   csv_undos.push_back( to_csv( lv_csv, csv1, false ) );
   csv_copy();
   if ( !csv1.name.contains(QRegExp("^edit of")) )
   {
      csv1.name = "edit of " + csv1.name;
      lbl_csv->setText( csv1.name );
   }

   // cout << "before cut:\n";
   // cout << list_keys( csv_undos[ csv_undos.size() - 1 ] );

   QTreeWidgetItemIterator it( lv_csv );
   while ( (*it) ) 
   {
      if ( (*it) ->isSelected() )
      {
         delete (*it);
      }
      ++it;
   }
   selection_since_count_csv1 = true;
   update_enables();
   // cout << "after cut:\n";
   csv tmp_csv = to_csv( lv_csv, csv1, false );
   // cout << list_keys( tmp_csv );
}

void US_Hydrodyn_Pdb_Tool::csv_copy()
{
   csv_clipboard = to_csv( lv_csv, csv1, true );
   csv_clipboard.name = "selection of " + csv_clipboard.name;
   update_enables();
}

void US_Hydrodyn_Pdb_Tool::csv_paste()
{
   csv last_csv = to_csv( lv_csv, csv1, false );
   csv_undos.push_back( last_csv );
   csv1 = merge_csvs( last_csv, csv_clipboard );
   csv_to_lv( csv1, lv_csv );
   update_enables_csv();
}

void US_Hydrodyn_Pdb_Tool::csv_paste_new()
{
   csv_undos.push_back( to_csv( lv_csv, csv1, false ) );
   csv1 = csv_clipboard;
   csv_to_lv( csv1, lv_csv );
   update_enables_csv();
}

void US_Hydrodyn_Pdb_Tool::csv_merge()
{
}

void US_Hydrodyn_Pdb_Tool::csv_reseq()
{
   
}

csv US_Hydrodyn_Pdb_Tool::reseq_csv( 
                                    QTreeWidget * /* lv           */, 
                                    csv &       ref_csv, 
                                    bool        /* only_selected */
                                    )
{
   return ref_csv;
}

void US_Hydrodyn_Pdb_Tool::csv_visualize()
{
   visualize( lv_csv );
}

void US_Hydrodyn_Pdb_Tool::csv2_load()
{
   QString f;
   load( lv_csv2, f );
}

void US_Hydrodyn_Pdb_Tool::csv2_load_1()
{
   QString f;
   load( lv_csv2, f, true );
}

void US_Hydrodyn_Pdb_Tool::csv2_dup()
{
   if ( csv2[ csv2_pos ].data.size() )
   {
      csv2_push( true );
   }
   // csv2_undos[ csv2_pos ].push_back( to_csv( lv_csv2, csv2[ csv2_pos ], false ) );
   csv2[ csv2_pos ] = to_csv( lv_csv, csv1 );
   csv2[ csv2_pos ].name = "duplicate of " + csv2[ csv2_pos ].name;
   csv_to_lv( csv2[ csv2_pos ], lv_csv2 );
   update_enables_csv2();
}

void US_Hydrodyn_Pdb_Tool::csv2_save()
{
   save_csv( lv_csv2 );
}

void US_Hydrodyn_Pdb_Tool::save_csv( QTreeWidget *lv, QString filename )
{
   csv csv_to_save;
   if ( lv == lv_csv )
   {
      csv_to_save = to_csv( lv_csv, csv1 );
   } else {
      csv_to_save = to_csv( lv_csv2, csv2[ csv2_pos ] );
   }

   QString use_dir = 
      ((US_Hydrodyn *)us_hydrodyn)->path_view_pdb.isEmpty() ?
      ((US_Hydrodyn *)us_hydrodyn)->somo_pdb_dir :
      ((US_Hydrodyn *)us_hydrodyn)->path_view_pdb;

   us_qdebug( QString( "csv_to_save.name %1 csv_to_save.filename %2" ).arg( csv_to_save.name ).arg( csv_to_save.filename ) );
   
   if ( filename.isEmpty() ) {
      if ( !((US_Hydrodyn *)us_hydrodyn)->select_from_directory_history( use_dir, this ) ) {
         use_dir = QFileInfo( csv_to_save.name ).path();
      }

      filename = QFileDialog::getSaveFileName( this , us_tr("Choose a filename to save the pdb") , use_dir + "/" + QFileInfo( csv_to_save.name ).fileName(), "*.pdb *.PDB" );

      if ( filename.isEmpty() )
      {
         return;
      }

      if ( !filename.contains(QRegExp(".pdb$", Qt::CaseInsensitive )) )
      {
         filename += ".pdb";
      }

      if ( QFile::exists(filename) )
      {
         filename = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( filename, 0, this );
      }
   }

   QFile f(filename);

   if ( !f.open( QIODevice::WriteOnly ) )
   {
      QMessageBox::warning( this, windowTitle(),
                            QString(us_tr("Could not open %1 for writing!")).arg(filename) );
      return;
   }

   QTextStream t( &f );


   t << csv_to_pdb( csv_to_save );
   
   f.close();
   if ( lv == lv_csv )
   {
      csv_msg("black", QString("File %1 written\n").arg( filename ) );
   } else {
      csv2_msg("black", QString("File %1 written\n").arg( filename ) );
   }
   ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( filename );
}

void US_Hydrodyn_Pdb_Tool::csv2_undo()
{
   if ( !csv2_undos[ csv2_pos ].size() )
   {
      editor_msg("red", "Internal error: undo called with empty undo buffer" );
      return;
   }
   csv2[ csv2_pos ] = csv2_undos[ csv2_pos ][ csv2_undos[ csv2_pos ].size() - 1 ];
   csv2_undos[ csv2_pos ].pop_back();
   csv_to_lv( csv2[ csv2_pos ], lv_csv2 );
   update_enables_csv2();
}

void US_Hydrodyn_Pdb_Tool::csv2_cut()
{
   // lv_csv2_undo.push_back( *lv_csv2 );
   csv2_undos[ csv2_pos ].push_back( to_csv( lv_csv2, csv2[ csv2_pos ], false ) );
   csv2_copy();
   if ( !csv2[ csv2_pos ].name.contains(QRegExp("^edit of")) )
   {
      csv2[ csv2_pos ].name = "edit of " + csv2[ csv2_pos ].name;
      lbl_csv2->setText( csv2[ csv2_pos ].name );
   }
   QTreeWidgetItemIterator it( lv_csv2 );
   while ( (*it) ) {
      if ( (*it) ->isSelected() )
      {
         delete (*it);
      }
      ++it;
   }
   selection_since_count_csv2 = true;
   update_enables_csv2();
}

void US_Hydrodyn_Pdb_Tool::csv2_copy()
{
   csv_clipboard = to_csv( lv_csv2, csv2[ csv2_pos ], true );
   csv_clipboard.name = "selection of " + csv_clipboard.name;
   update_enables();
}

void US_Hydrodyn_Pdb_Tool::csv2_paste()
{
   csv last_csv = to_csv( lv_csv2, csv2[ csv2_pos ], false );
   csv2_undos[ csv2_pos ].push_back( last_csv );
   csv2[ csv2_pos ] = merge_csvs( last_csv, csv_clipboard );
   csv_to_lv( csv2[ csv2_pos ], lv_csv2 );
   update_enables_csv2();
}

void US_Hydrodyn_Pdb_Tool::csv2_paste_new()
{
   if ( csv2[ csv2_pos ].data.size() )
   {
      csv2_push( true );
   }
   // csv2_undos[ csv2_pos ].push_back( to_csv( lv_csv2, csv2[ csv2_pos ], false ) );
   csv2[ csv2_pos ] = csv_clipboard;
   csv_to_lv( csv2[ csv2_pos ], lv_csv2 );
   update_enables_csv2();
}

void US_Hydrodyn_Pdb_Tool::csv2_merge()
{
}

void US_Hydrodyn_Pdb_Tool::csv2_reseq()
{
}

void US_Hydrodyn_Pdb_Tool::csv2_visualize()
{
   visualize( lv_csv2 );
}

void US_Hydrodyn_Pdb_Tool::adjust_wheel( double pos )
{
   // cout << QString("wheel %1\n").arg(pos);
   csv2_redisplay( (unsigned int) pos );
}

void US_Hydrodyn_Pdb_Tool::csv2_push( bool save_current )
{
   if ( save_current )
   {
      csv2[ csv2_pos ] = to_csv( lv_csv2, csv2[ csv2_pos ] );
   }

   csv2_pos = csv2.size();
   csv2.resize( csv2_pos + 1 );
   csv2_undos.resize( csv2_pos + 1 );
   lv_csv2->clear( );
   update_enables_csv2();

   {
      double rs =
         ( csv2.size() * csv2.size() * 0.1 < 1.0 )
         ? 
         csv2.size() * csv2.size() * 0.05
         :
         1.0
         ;
      qwtw_wheel->setRange( csv2.size() - 0.5, -0.5); qwtw_wheel->setSingleStep( rs );
   }
   lbl_pos_range->setText( QString( "%1\nof\n%2" ).arg( csv2_pos + 1 ).arg( csv2.size() ) );
}

void US_Hydrodyn_Pdb_Tool::csv2_redisplay( unsigned int pos )
{
   if ( csv2_pos == pos || csv2.size() < pos )
   {
      return;
   }

   // save current csv
   csv2[ csv2_pos ] = to_csv( lv_csv2, csv2[ csv2_pos ] );

   csv2_pos = pos;
   // update new csv
   csv_to_lv( csv2[ csv2_pos ], lv_csv2 );
   lbl_pos_range->setText( QString( "%1\nof\n%2" ).arg( csv2_pos + 1 ).arg( csv2.size() ) );
   update_enables_csv2();
}

pdb_sel_count US_Hydrodyn_Pdb_Tool::count_selected( QTreeWidget *lv )
{
   bool lv_is_csv = ( lv == lv_csv );

   if ( lv_is_csv && !selection_since_count_csv1 )
   {
      return last_count_csv1;
   }
   if ( !lv_is_csv && !selection_since_count_csv2 )
   {
      return last_count_csv2;
   }
   if ( lv_is_csv )
   {
      csv_selected_element_counts.clear( );
   } else {
      csv2_selected_element_counts.clear( );
   }
   
   pdb_sel_count counts;

   counts.models              = 0;
   counts.chains              = 0;
   counts.residues            = 0;
   counts.atoms               = 0;
   counts.model_partial       = false;
   counts.chain_partial       = false;
   counts.residue_partial     = false;
   counts.not_selected_atoms  = 0;
   counts.SOLs                = 0;

   QTreeWidgetItemIterator it( lv );
   while ( (*it) ) 
   {
      QTreeWidgetItem *item = (*it);
      if ( item->childCount() )
      {
         if ( is_selected( item ) || child_selected( item ) )
         {
            switch ( US_Static::lvi_depth( item ) )
            {
            case 0 : // models
               counts.models++;
               if ( !counts.model_partial )
               {
                  counts.model_partial = !all_children_selected( item );
               }
               break;
            case 1 : // chains
               counts.chains++;
               if ( !counts.chain_partial )
               {
                  counts.chain_partial = !all_children_selected( item );
               }
               break;
            case 2 : // residues
               counts.residues++;
               if ( !counts.residue_partial )
               {
                  counts.residue_partial = !all_children_selected( item );
               }
               break;
            default :
               editor_msg("red", "internal error: unexpected depth in count_selected()" );
               break;
            }
         }
      } else {
         if ( is_selected( item ) )
         {
            counts.atoms++;
            if ( lv_is_csv )
            {
               if ( !csv1.key.count( key( item ) ) )
               {
                  editor_msg( "red", QString( us_tr( "Internal error: missing key %1" ) ).arg( key( item ) ) );
               } else {
                  vector < QString > *csvdata = &csv1.data[ csv1.key[ key( item ) ] ];
                  // csv_selected_element_counts [ csv1.data[ csv1.key[ key( item ) ] ][ 13 ] ]++;
                  csv_selected_element_counts [ (*csvdata)[ 13 ] ]++;
                  if ( (*csvdata)[ 2 ] == "SOL" ) {
                     counts.SOLs++;
                  }
               }
            } else {
               if ( !csv2[ csv2_pos].key.count( key( item ) ) )
               {
                  editor_msg( "red", QString( us_tr( "Internal error: missing key %1" ) ).arg( key( item ) ) );
               } else {
                  vector < QString > *csvdata = &csv2[ csv2_pos ].data[ csv2[ csv2_pos ].key[ key( item ) ] ];
                  // csv2_selected_element_counts[ csv2[ csv2_pos ].data[ csv2[ csv2_pos ].key[ key( item ) ] ][ 13 ] ]++;
                  csv_selected_element_counts [ (*csvdata)[ 13 ] ]++;
                  if ( (*csvdata)[ 2 ] == "SOL" ) {
                     counts.SOLs++;
                  }
               }
            }
         } else {
            counts.not_selected_atoms++;
         }
      }
      ++it;
   }
   
   if ( lv_is_csv )
   {
      selection_since_count_csv1 = false;
      last_count_csv1 = counts;
   } else {
      selection_since_count_csv2 = false;
      last_count_csv2 = counts;
   }
   // us_qdebug( QString( "SOLs counts %1" ).arg( counts.SOLs ) );
   return counts;
}

bool US_Hydrodyn_Pdb_Tool::any_selected( QTreeWidget *lv )
{
   if ( lv == lv_csv && !selection_since_count_csv1 )
   {
      return last_count_csv1.atoms != 0;
   }
   if ( lv == lv_csv2 && !selection_since_count_csv2 )
   {
      return last_count_csv2.atoms != 0;
   }

   QTreeWidgetItemIterator it( lv );
   while ( (*it) ) {
      // QListViewItem *item = (*it);
      if ( (*it)->isSelected() )
      {
         return true;
      }
      ++it;
   }
   return false;
}

void US_Hydrodyn_Pdb_Tool::csv_selection_changed()
{
   // csv_msg("black", QString("selection changed # selected %1").arg( count_selected( lv_csv ).atoms ) );
   selection_since_count_csv1 = true;
   selection_since_clean_csv1 = true;
   // clean_selection( lv_csv );
   update_enables_csv();
}

void US_Hydrodyn_Pdb_Tool::csv2_selection_changed()
{
   // csv2_msg("black", QString("selection changed # selected %1").arg( count_selected( lv_csv2 ).atoms ) );
   selection_since_count_csv2 = true;
   selection_since_clean_csv2 = true;
   // clean_selection( lv_csv2 );
   update_enables_csv2();
}

csv US_Hydrodyn_Pdb_Tool::to_csv( QTreeWidget *lv, csv &ref_csv, bool only_selected )
{
   QTreeWidgetItemIterator it( lv );
   csv csv1;
   csv1.header        = ref_csv.header;
   csv1.name          = ref_csv.name;
   csv1.filename      = ref_csv.filename;
   csv1.title_text    = ref_csv.title_text;
   csv1.header_text   = ref_csv.header_text;

   while ( (*it) ) {
      QTreeWidgetItem *item = (*it);
      if ( !only_selected || is_selected( item ) || child_selected( item ) )
      {
         if ( !item->childCount() )
         {
            if ( !ref_csv.key.count( key( item ) ) )
            {
               editor_msg("red", QString( us_tr("Error: Can not find reference key %1 in %2\n") ).arg( key( item ) ).arg( ref_csv.name ) );
            } else {
               csv1.data.push_back( ref_csv.data[ ref_csv.key[ key( item ) ] ] );
#if QT_VERSION < 0x040000
               csv1.visible.push_back( item->isVisible() );
#endif
               csv1.selected.push_back( item->isSelected() );
               csv1.open.push_back( item->isExpanded() );
               if ( item == lv->currentItem() )
               {
                  csv1.current_item_key = key( item );
               }
            }
         } else {
            csv1.nd_key[ key( item ) ] = csv1.nd_selected.size();
#if QT_VERSION < 0x040000
            csv1.nd_visible.push_back( item->isVisible() );
#endif
            csv1.nd_selected.push_back( item->isSelected() );
            csv1.nd_open.push_back( item->isExpanded() );
            if ( item == lv->currentItem() )
            {
               csv1.current_item_key = key( item );
            }
         }               
      } 
         
      ++it;
   }
   csv_setup_keys( csv1 );
   return csv1;
}

bool US_Hydrodyn_Pdb_Tool::is_selected( QTreeWidgetItem *lvi )
{
   if ( lvi->isSelected() )
   {
      return true;
   }
   while ( lvi->parent() )
   {
      lvi = lvi->parent();
      if ( lvi->isSelected() )
      {
         return true;
      } 
   }
   return false;
}

bool US_Hydrodyn_Pdb_Tool::child_selected( QTreeWidgetItem *lvi )
{
   if ( lvi->isSelected() )
   {
      return true;
   }

#if QT_VERSION < 0x040000
   if ( lvi->childCount() )
   {
      QTreeWidgetItem *myChild = lvi->firstChild();
      while( myChild ) 
      {
         if ( myChild->isSelected() ||
              child_selected( myChild ) )
         {
            return true;
         }
         myChild = myChild->nextSibling();
      }
   }
#else
   int children = lvi->childCount();
   if ( children ) { 
      for ( int i = 0; i < children; ++i ) {
         QTreeWidgetItem *myChild = lvi->child( i );
         if( myChild ) {
            if ( myChild->isSelected() ||
                 child_selected( myChild ) )
            {
               return true;
            }
         }
      }
   }
#endif

   return false;
}

bool US_Hydrodyn_Pdb_Tool::all_children_selected( QTreeWidgetItem *lvi )
{
#if QT_VERSION < 0x040000
   if ( lvi->childCount() )
   {
      QTreeWidgetItem *myChild = lvi->firstChild();
      while( myChild ) 
      {
         if ( myChild->childCount() )
         {
            if ( !all_children_selected( myChild ) )
            {
               return false;
            }
         } else {
            if ( !is_selected( myChild ) )
            {
               return false;
            }
         }
         myChild = myChild->nextSibling();
      }
   } else {
      return lvi->isSelected();
   }
#else
   int children = lvi->childCount();
   if ( children ) { 
      for ( int i = 0; i < children; ++i ) {
         QTreeWidgetItem *myChild = lvi->child( i );
         if( myChild ) {
            if ( myChild->childCount() )
            {
               if ( !all_children_selected( myChild ) )
               {
                  return false;
               }
            } else {
               if ( !is_selected( myChild ) )
               {
                  return false;
               }
            }
         }
      }
   } else {
      return lvi->isSelected();
   }
#endif

   return true;
}

QString US_Hydrodyn_Pdb_Tool::key( QTreeWidgetItem *lvi )
{
   QString key = lvi->text( 0 );
   while ( lvi->parent() )
   {
      lvi = lvi->parent();
      key = lvi->text( 0 ) + "~" + key;
   }
   return key;
}

QString US_Hydrodyn_Pdb_Tool::data_to_key( vector < QString > &data )
{
   return QString( "%1~%2~%3~%4" )
      .arg( data[ 0 ] )
      .arg( data[ 1 ] )
      .arg( data[ 2 ] + " " + data[ 3 ] )
      .arg( data[ 4 ] + " " + data[ 5 ] );
}

void US_Hydrodyn_Pdb_Tool::csv_setup_keys( csv &csv1 )
{
   csv1.key.clear( );

   for ( unsigned int i = 0; i < (unsigned int)csv1.data.size(); i++ )
   {
      // cout << QString("csv setup keys %1 data size %2\n").arg( i ).arg( csv1.data[ i ].size() );
      
      csv1.key[ data_to_key( csv1.data[ i ] ) ] = i;
   }
}

void US_Hydrodyn_Pdb_Tool::list_csv_keys( csv &csv1 )
{
   for ( map < QString, unsigned int >::iterator it = csv1.key.begin();
         it != csv1.key.end();
         it++ )
   {
      editor_msg("green", QString("key <%1> value <%2>").arg( it->first ).arg( it->second ) );
   }
}
      
void US_Hydrodyn_Pdb_Tool::csv_to_lv( csv &csv1, QTreeWidget *lv )
{
   bool lv_is_csv = ( lv == lv_csv );

   // qDebug() << "csv_to_lv start";
   if ( lv_is_csv ) {
      disconnect( lv_csv, SIGNAL(itemSelectionChanged()), 0, 0 );
   } else {
      disconnect( lv_csv2, SIGNAL(itemSelectionChanged()), 0, 0 );
   }

   lv->clear( );

#if QT_VERSION >= 0x040000
   vector < QString > to_select;
   for ( map < QString, unsigned int >::iterator it = csv1.nd_key.begin();
         it != csv1.nd_key.end();
         it++ ) {
      // qDebug() << "nd_key:" << it->first << " index:" << it->second << " "
      //          << ( csv1.nd_selected.size() > it->second ? ( csv1.nd_selected[ it->second ] ? " isSelected" : "" ) : " missing_selected" )
      //          << ( csv1.nd_open.size() > it->second ? ( csv1.nd_open[ it->second ] ? " isExpanded" : "" ) : " missing_expanded" )
      //    ;
      if ( csv1.nd_selected.size() > it->second && csv1.nd_selected[ it->second ] ) {
         to_select.push_back( it->first );
      }
   }         
#endif

   map < QString, QTreeWidgetItem * >   models;
   map < QString, QTreeWidgetItem * >   model_chains;
   map < QString, QTreeWidgetItem * >   model_chain_residues;
   map < QString, QTreeWidgetItem * >   model_chain_residue_atoms;

   QString last_model   = "none";
   QString last_chain   = "none";
   QString last_residue = "none";

   QTreeWidgetItem *current = ( QTreeWidgetItem * )0;

   for ( int i = (int)csv1.data.size() - 1; i >= 0; i-- )
   {
      if ( csv1.data[ i ].size() < 6 )
      {
         editor_msg( "red", 
                     QString( us_tr( "Error: data row %1 too few tokens (%2), skipping" ) )
                     .arg( i + 1 )
                     .arg( csv1.data[ i ].size() ) );
         continue;
      }

      QString model      = csv1.data[ i ][ 0 ];
      QString chain      = csv1.data[ i ][ 1 ];
      QString residue    = csv1.data[ i ][ 2 ] + " " + csv1.data[ i ][ 3 ];
      QString atom       = csv1.data[ i ][ 4 ] + " " + csv1.data[ i ][ 5 ];

      QString model_chain              = model + "~" + chain;
      QString model_chain_residue      = model + "~" + chain + "~" + residue;
      QString model_chain_residue_atom = model + "~" + chain + "~" + residue + "~" + atom;

      if ( last_model != model )
      {
         last_model = model;
         if ( models.count( model ) )
         {
            editor_msg( "red", 
                        QString( us_tr( "Error: data row %1, model %2 duplicated , skipping" ) )
                        .arg( i + 1 )
                        .arg( model ) );
            continue;
         }
#if QT_VERSION < 0x040000
         models[ model ] = new QTreeWidgetItem( lv, model );
#else
         {
            QTreeWidgetItem *lvi = new QTreeWidgetItem( QStringList() << model );
            models[ model ] = lvi;
            lv->insertTopLevelItem( 0, lvi );
         }
#endif
         if ( csv1.current_item_key == model )
         {
            current = models[ model ];
         }
         if ( csv1.nd_key.count( model ) )
         {
#if QT_VERSION < 0x040000
            models[ model ]->setVisible ( csv1.nd_visible [ csv1.nd_key[ model ] ] );
#endif
            models[ model ]->setSelected( csv1.nd_selected[ csv1.nd_key[ model ] ] );
            models[ model ]->setExpanded    ( csv1.nd_open    [ csv1.nd_key[ model ] ] );
         } else {
            // editor_msg("red", QString( "Error: model %1 visi/sel/open not key not found" ).arg( model ) );
         }
         last_chain   = "none";
         last_residue = "none";
      }      
      if ( last_chain != chain )
      {
         last_chain = chain;
         if ( model_chains.count( model_chain ) )
         {
            editor_msg( "red", 
                        QString( us_tr( "Error: data row %1, model~chain %2 duplicated , skipping" ) )
                        .arg( i + 1 )
                        .arg( model_chain ) );
            continue;
         }
         if ( !models.count( model ) )
         {
            editor_msg( "red", 
                        QString( us_tr( "Error: data row %1, model %2 not found int models! , skipping" ) )
                        .arg( i + 1 )
                        .arg( model ) );
            continue;
         }
            
#if QT_VERSION < 0x040000
         model_chains[ model_chain ] = new QTreeWidgetItem( models[ model ], chain );
#else
         model_chains[ model_chain ] = new QTreeWidgetItem( QStringList() << chain );
         models[ model ]->insertChild( 0, model_chains[ model_chain ] );
#endif
         if ( csv1.current_item_key == model_chain )
         {
            current = model_chains[ model_chain ];
         }
         if ( csv1.nd_key.count( model_chain ) )
         {
#if QT_VERSION < 0x040000
            model_chains[ model_chain ]->setVisible ( csv1.nd_visible [ csv1.nd_key[ model_chain ] ] );
#endif
            model_chains[ model_chain ]->setSelected( csv1.nd_selected[ csv1.nd_key[ model_chain ] ] );
            model_chains[ model_chain ]->setExpanded    ( csv1.nd_open    [ csv1.nd_key[ model_chain ] ] );
         } else {
            // editor_msg("red", QString( "Error: model_chain %1 visi/sel/open not key not found" ).arg( model_chain ) );
         }
         // cout << QString( "added %1\n" ).arg( model_chain );
         last_residue = "none";
      }
      if ( last_residue != residue )
      {
         last_residue = residue;
         if ( model_chain_residues.count( model_chain_residue ) )
         {
            editor_msg( "red", 
                        QString( us_tr( "Error: data row %1, model~chain~residue %2 duplicated , skipping" ) )
                        .arg( i + 1 )
                        .arg( model_chain_residue ) );
            continue;
         }
         if ( !model_chains.count( model_chain ) )
         {
            editor_msg( "red", 
                        QString( us_tr( "Error: data row %1, model~chain %2 not found int models! , skipping" ) )
                        .arg( i + 1 )
                        .arg( model_chain ) );
            continue;
         }
#if QT_VERSION < 0x040000
         model_chain_residues[ model_chain_residue ] = new QTreeWidgetItem( model_chains[ model_chain ], residue );
#else
         model_chain_residues[ model_chain_residue ] = new QTreeWidgetItem( QStringList() << residue );
         model_chains[ model_chain ]->insertChild( 0, model_chain_residues[ model_chain_residue ] );
#endif
         if ( csv1.current_item_key == model_chain_residue )
         {
            current = model_chain_residues[ model_chain_residue ];
         }
         if ( csv1.nd_key.count( model_chain_residue ) )
         {
#if QT_VERSION < 0x040000
            model_chain_residues[ model_chain_residue ]->setVisible ( csv1.nd_visible [ csv1.nd_key[ model_chain_residue ] ] );
#endif
            model_chain_residues[ model_chain_residue ]->setSelected( csv1.nd_selected[ csv1.nd_key[ model_chain_residue ] ] );
            model_chain_residues[ model_chain_residue ]->setExpanded    ( csv1.nd_open    [ csv1.nd_key[ model_chain_residue ] ] );
         } else {
            // editor_msg("red", QString( "Error: model_chain_residue %1 visi/sel/open not key not found" ).arg( model_chain_residue) );
         }
      }
      if ( !model_chain_residues.count( model_chain_residue ) )
      {
         editor_msg( "red", 
                     QString( us_tr( "Error: data row %1, model~chain~residue %2 not found int models! , skipping" ) )
                     .arg( i + 1 )
                     .arg( model_chain_residue ) );
         continue;
      }
#if QT_VERSION < 0x040000
      model_chain_residue_atoms[ model_chain_residue_atom ] = 
         new QTreeWidgetItem( model_chain_residues[ model_chain_residue ], 
                            atom,
                            csv1.data[ i ][ 6 ],
                            csv1.data[ i ][ 7 ],
                            csv1.data[ i ][ 8 ],
                            csv1.data[ i ][ 9 ],
                            csv1.data[ i ][ 10 ],
                            csv1.data[ i ][ 11 ],
                            // csv1.data[ i ][ 12 ],
                            csv1.data[ i ][ 13 ]
                            );
#else
      model_chain_residue_atoms[ model_chain_residue_atom ] =
                 new QTreeWidgetItem( QStringList()
                                      << atom
                                      << csv1.data[ i ][ 6 ]
                                      << csv1.data[ i ][ 7 ]
                                      << csv1.data[ i ][ 8 ]
                                      << csv1.data[ i ][ 9 ]
                                      << csv1.data[ i ][ 10 ]
                                      << csv1.data[ i ][ 11 ]
                                      // << csv1.data[ i ][ 12 ]
                                      << csv1.data[ i ][ 13 ]
                                      );
      model_chain_residues[ model_chain_residue ]->insertChild( 0, model_chain_residue_atoms[ model_chain_residue_atom ] );
#endif      
      if ( csv1.current_item_key == model_chain_residue_atom )
      {
         current = model_chain_residue_atoms[ model_chain_residue_atom ];
      }
      if ( csv1.key.count( model_chain_residue_atom ) )
      {
#if QT_VERSION < 0x040000
         model_chain_residue_atoms[ model_chain_residue_atom ]->setVisible ( 
                                                                            (unsigned int)csv1.visible.size() > csv1.key[ model_chain_residue_atom ]  ?
                                                                            csv1.visible [ csv1.key[ model_chain_residue_atom ] ] : false 
                                                                             );
#endif
         model_chain_residue_atoms[ model_chain_residue_atom ]->setSelected( 
                                                                            (unsigned int)csv1.selected.size() > csv1.key[ model_chain_residue_atom ] ?
                                                                            csv1.selected[ csv1.key[ model_chain_residue_atom ] ] : false
                                                                             );
         model_chain_residue_atoms[ model_chain_residue_atom ]->setExpanded    ( 
                                                                            (unsigned int)csv1.open.size() > csv1.key[ model_chain_residue_atom ] ?
                                                                            csv1.open    [ csv1.key[ model_chain_residue_atom ] ] : false
                                                                             );
      } else {
         // editor_msg("red", QString( "Error: model_chain_residue_atom %1 visi/sel/open not key not found" ).arg( model_chain_residue_atom ) );
      }
   }
   lv->setCurrentItem( current );

   // qDebug() << "csv_to_lv end";
   if ( lv_is_csv )
   {
#if QT_VERSION >= 0x040000
      select_these( lv_csv, to_select );
#endif
      connect(lv_csv, SIGNAL(itemSelectionChanged()), SLOT(csv_selection_changed()));
      lbl_csv->setText( csv1.name.isEmpty() ? us_tr( "Panel 1" ) : csv1.name );
      selection_since_count_csv1 = true;
   } else {
#if QT_VERSION >= 0x040000
      select_these( lv_csv2, to_select );
#endif
      connect(lv_csv2, SIGNAL(itemSelectionChanged()), SLOT(csv2_selection_changed()));
      lbl_csv2->setText( (unsigned int)csv2.size() > csv2_pos ? csv2[ csv2_pos ].name : us_tr( "Panel 2" ) );
      selection_since_count_csv2 = true;
   }
}

bool US_Hydrodyn_Pdb_Tool::merge_ok()
{
   unsigned int sel1 = count_selected( lv_csv ).atoms;
   unsigned int sel2 = count_selected( lv_csv2 ).atoms;
   if ( !sel1 || sel1 != sel2 )
   {
      return false;
   }

   // make sure elements match
   for ( map < QString, unsigned int >::iterator it = csv_selected_element_counts.begin();
         it != csv_selected_element_counts.end();
         it++ )
   {
      if ( !csv2_selected_element_counts.count( it->first ) ||
           it->second != csv2_selected_element_counts[ it->first ] )
      {
         return false;
      }
   }

   return true;
}

QString US_Hydrodyn_Pdb_Tool::csv_to_pdb( csv &csv1, bool only_atoms )
{
   QString s;

   if ( !only_atoms )
   {
      for ( unsigned int i = 0; i < (unsigned int)csv1.header_text.size(); i++ )
      {
         s += QString( "HEADER   %1" + csv1.header_text[ i ] + "\n" )
            .arg( i ? QString("%1").arg(i + 1) : " " );
      }
      
      for ( unsigned int i = 0; i < (unsigned int)csv1.title_text.size(); i++ )
      {
         s += QString( "TITLE    %1" + csv1.title_text[ i ] + "\n" )
            .arg( i ? QString("%1").arg(i + 1) : " " );
      }
   }

   QString last_model = "none";

   for ( unsigned int i = 0; i < (unsigned int)csv1.data.size(); i++ )
   {
      QString model      = csv1.data[ i ][ 0 ];
      
      if ( !only_atoms )
      {
         if ( last_model != model )
         {
            if ( last_model != "none" )
            {
               s += "ENDMDL\n";
            }
            last_model = model;
            s += "MODEL     " + model + "\n";
         }         
      }

      s +=
         QString("")
         .sprintf(     
                  "ATOM  %5d%5s%4s %1s%4d    %8.3f%8.3f%8.3f%6.2f%6.2f          %2s\n",
                  csv1.data[ i ][ 5  ].toUInt(),
                  csv1.data[ i ][ 4  ].toLatin1().data(),
                  csv1.data[ i ][ 2  ].toLatin1().data(),
                  csv1.data[ i ][ 1  ].toLatin1().data(),
                  csv1.data[ i ][ 3  ].toUInt(),
                  csv1.data[ i ][ 8  ].toFloat(),
                  csv1.data[ i ][ 9  ].toFloat(),
                  csv1.data[ i ][ 10 ].toFloat(),
                  csv1.data[ i ][ 11 ].toFloat(),
                  csv1.data[ i ][ 12 ].toFloat(),
                  csv1.data[ i ][ 13 ].toLatin1().data()
                  );
   }
   if ( !only_atoms )
   {
      s += "ENDMDL\nEND\n";
   }

   return s;
}

QStringList US_Hydrodyn_Pdb_Tool::csv_to_pdb_qsl( csv &csv1, bool only_atoms )
{
   QStringList qsl;

   if ( !only_atoms )
   {
      for ( unsigned int i = 0; i < (unsigned int)csv1.header_text.size(); i++ )
      {
         qsl << QString( "HEADER   %1" + csv1.header_text[ i ] + "\n" )
            .arg( i ? QString("%1").arg(i + 1) : " " );
      }
      
      for ( unsigned int i = 0; i < (unsigned int)csv1.title_text.size(); i++ )
      {
         qsl << QString( "TITLE    %1" + csv1.title_text[ i ] + "\n" )
            .arg( i ? QString("%1").arg(i + 1) : " " );
      }
   }

   QString last_model = "none";

   for ( unsigned int i = 0; i < (unsigned int)csv1.data.size(); i++ )
   {
      QString model      = csv1.data[ i ][ 0 ];
      
      if ( !only_atoms )
      {
         if ( last_model != model )
         {
            if ( last_model != "none" )
            {
               qsl << "ENDMDL\n";
            }
            last_model = model;
            qsl <<  "MODEL " + model + "\n";
         }         
      }

      qsl <<
         QString("")
         .sprintf(     
                  "ATOM  %5d%5s%4s %1s%4d    %8.3f%8.3f%8.3f%6.2f%6.2f          %2s\n",
                  csv1.data[ i ][ 5  ].toUInt(),
                  csv1.data[ i ][ 4  ].toLatin1().data(),
                  csv1.data[ i ][ 2  ].toLatin1().data(),
                  csv1.data[ i ][ 1  ].toLatin1().data(),
                  csv1.data[ i ][ 3  ].toUInt(),
                  csv1.data[ i ][ 8  ].toFloat(),
                  csv1.data[ i ][ 9  ].toFloat(),
                  csv1.data[ i ][ 10 ].toFloat(),
                  csv1.data[ i ][ 11 ].toFloat(),
                  csv1.data[ i ][ 12 ].toFloat(),
                  csv1.data[ i ][ 13 ].toLatin1().data()
                  );
   }
   if ( !only_atoms )
   {
      qsl << "ENDMDL\nEND\n";
   }

   return qsl;
}

void US_Hydrodyn_Pdb_Tool::visualize( QTreeWidget *lv )
{
   csv tmp_csv;

   if ( lv == lv_csv )
   {
      tmp_csv = to_csv( lv, csv1 );
   } else {
      tmp_csv = to_csv( lv, csv2[ csv2_pos ] );
   }

   QString use_dir = ((US_Hydrodyn *)us_hydrodyn)->somo_tmp_dir + QDir::separator();

   unsigned int pos = 0;
   QString filename;

   do {
      filename = QString("%1temp%2.pdb").arg( use_dir ).arg( pos );
      pos++;
   } while( QFile::exists( filename ) );

   QFile f( filename );

   pos = 0;
   QString spt_filename;

   do {
      spt_filename = QString("%1temp%2.spt").arg( use_dir ).arg( pos );
      pos++;
   } while( QFile::exists( spt_filename ) );

   QFile f_spt( spt_filename );

   if ( !f.open( QIODevice::WriteOnly ) )
   {
      QMessageBox::warning( this, "UltraScan",
                            QString(us_tr("Could not open %1 for writing!")).arg(filename) );
      return;
   }

   {
      QTextStream t( &f );
      t << QString( "TITLE %1\n" ).arg( tmp_csv.name );
      t << csv_to_pdb( tmp_csv );
      f.close();
   }

   if ( !f_spt.open( QIODevice::WriteOnly ) )
   {
      QMessageBox::warning( this, "UltraScan",
                            QString(us_tr("Could not open %1 for writing!")).arg(spt_filename) );
      return;
   }

   {
      QTextStream t( &f_spt );
      t << QString( 
                   "load %1\n" 
                   "spacefill\n"
                   "select all\n"
                   "color white\n"
                   )
         .arg( filename )
         ;
      QStringList qsl = atom_sel_rasmol( lv );
      for ( int i = 0; i < (int) qsl.size(); i++ )
      {
         t << qsl[ i ];
         t << "color red\n";
      }
      t << "background white\n";
      t << "select all\n";
      f_spt.close();
   }

   ((US_Hydrodyn *)us_hydrodyn)->model_viewer( spt_filename, "-script" );

   //    QStringList qsl = atom_sel_rasmol( lv );
   //    cout << QString( "atom set %1\n" ).arg( qsl.size() );
   //    for ( int i = 0; i < (int) qsl.size(); i++ )
   //    {
   //       cout << QString( "%1: %2\n" ).arg( i ).arg( qsl[ i ] );
   //    }
   //    cout << "after atom set\n";
   //    cout << flush;
}

void US_Hydrodyn_Pdb_Tool::load( QTreeWidget *lv, QString &filename, bool only_first_nmr )
{
   QString use_dir = ((US_Hydrodyn *)us_hydrodyn)->somo_pdb_dir;

   if ( filename.isEmpty() )
   {
      ((US_Hydrodyn *)us_hydrodyn)->select_from_directory_history( use_dir, this );

      filename = QFileDialog::getOpenFileName( this , windowTitle() , use_dir , "*.pdb *.PDB" );
   }

   if ( filename.isEmpty() )
   {
      return;
   }

   ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( filename );

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

   QFile f( filename );

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

   csv new_csv;

   new_csv.name     = filename;
   new_csv.filename = filename;

   new_csv.header.push_back("Model");
   new_csv.header.push_back("Chain");
   new_csv.header.push_back("Residue");
   new_csv.header.push_back("Residue Number");
   new_csv.header.push_back("Atom");
   new_csv.header.push_back("Atom Number");
   new_csv.header.push_back("Alt");
   new_csv.header.push_back("iC");
   new_csv.header.push_back("X");
   new_csv.header.push_back("Y");
   new_csv.header.push_back("Z");
   new_csv.header.push_back("Occ");
   new_csv.header.push_back("TF");
   new_csv.header.push_back("Ele");
   // new_csv.header.push_back("Charge");
   // new_csv.header.push_back("Accessibility");

   QTextStream ts( &f );

   unsigned int model           = 0;
   unsigned int line_count      = 0;
   bool         last_was_ENDMDL = true;

   while ( !ts.atEnd() )
   {
      QString qs = ts.readLine();
      line_count++;

      QString left6 = qs.left( 6 ).toUpper();
      QString left5 = qs.left( 5 ).toUpper();

      if ( left6 == "TITLE " )
      {
         new_csv.title_text << qs.mid( 10 );
         continue;
      }

      if ( left6 == "HEADER" )
      {
         new_csv.header_text << qs.mid( 10 );
         continue;
      }

      if ( left6 == "MODEL " )
      {
         model++;
         last_was_ENDMDL = false;
         continue;
      }

      if ( left6 == "ENDMDL" )
      {
         last_was_ENDMDL = true;
         if ( only_first_nmr )
         {
            break;
         }
         continue;
      }

      if ( ( left5 == "ATOM " ||
             left5 == "HETAT" ) && 
           last_was_ENDMDL )
      {
         model++;
         last_was_ENDMDL = false;
      }

      if ( left5 != "ATOM " && left5 != "HETAT" ) 
      {
         // not supporting anything else for now
         // later we should store, save remarks connct's etc

         continue;
      }

      /*
     http://www.rcsb.org/pdb/docs/format/pdbguide2.2/part_11.html

     ATOM record:
     COLUMNS        DATA TYPE       FIELD         DEFINITION
     ---------------------------------------------------------------------------------
     1 -  6        Record name     "ATOM  "
     7 - 11        Integer         serial        Atom serial number.
     13 - 16        Atom            name          Atom name. (sometimes starts at 12)
     17             Character       altLoc        Alternate location indicator.
     18 - 20        Residue name    resName       Residue name.
     22             Character       chainID       Chain identifier.
     23 - 26        Integer         resSeq        Residue sequence number.
     27             AChar           iCode         Code for insertion of residues.
     31 - 38        Real(8.3)       x             Orthogonal coordinates for X in Angstroms.
     39 - 46        Real(8.3)       y             Orthogonal coordinates for Y in Angstroms.
     47 - 54        Real(8.3)       z             Orthogonal coordinates for Z in Angstroms.
     55 - 60        Real(6.2)       occupancy     Occupancy.
     61 - 66        Real(6.2)       tempFactor    Temperature factor.
     73 - 76        LString(4)      segID         Segment identifier, left-justified.
     77 - 78        LString(2)      element       Element symbol, right-justified.
     79 - 80        LString(2)      charge        Charge on the atom.
      */
      
      vector < QString > data;

      data.push_back( QString("%1").arg( model ) ); // 0 model
      data.push_back( qs.mid( 21 , 1 ) );           // 1 chain
      data.push_back( qs.mid( 17 , 3 ) );           // 2 residue name
      data.push_back( qs.mid( 22 , 4 ) );           // 3 residue #
      data.push_back( qs.mid( 12 , 4 ) );           // 4 atom name
      data.push_back( qs.mid( 6  , 5 ) );           // 5 atom number
      data.push_back( qs.mid( 16 , 1 ) );           // 6 alt loc
      data.push_back( qs.mid( 26 , 1 ) );           // 7 code for insertion of residues?
      data.push_back( qs.mid( 30 , 8 ) );           // 8 X
      data.push_back( qs.mid( 38 , 8 ) );           // 9 Y
      data.push_back( qs.mid( 46 , 8 ) );           // 10 Z
      data.push_back( qs.mid( 54 , 6 ) );           // 11 occupancy
      data.push_back( qs.mid( 60 , 6 ) );           // 12 temperature factor
      data.push_back( qs.mid( 76 , 2 ).trimmed() ); // 13 element symbol
      // data.push_back( qs.mid( 78 , 2 ) );

      new_csv.data.push_back( data );
   }
   f.close();

   if ( lv == lv_csv )
   {
      if ( csv1.data.size() )
      {
         csv_undos.push_back( to_csv( lv_csv, csv1, false ) );
      }
      csv1 = new_csv;
      csv_to_lv( csv1, lv );
      csv_setup_keys( csv1 );
   } else {
      if ( csv2[ csv2_pos ].data.size() )
      {
         csv2_push( true );
         // csv2_undos[ csv2_pos ].push_back( to_csv( lv_csv2, csv2[ csv2_pos ], false ) );
      }
      csv2[ csv2_pos ] = new_csv;
      csv_to_lv( csv2[ csv2_pos ], lv );
      csv_setup_keys( csv2[ csv2_pos ] );
   }
   update_enables();
}

void US_Hydrodyn_Pdb_Tool::load_from_qsl( QTreeWidget *lv, QStringList &pdb_text, QString title )
{
   csv new_csv;

   new_csv.name     = title;
   new_csv.filename = title;

   new_csv.header.push_back("Model");
   new_csv.header.push_back("Chain");
   new_csv.header.push_back("Residue");
   new_csv.header.push_back("Residue Number");
   new_csv.header.push_back("Atom");
   new_csv.header.push_back("Atom Number");
   new_csv.header.push_back("Alt");
   new_csv.header.push_back("iC");
   new_csv.header.push_back("X");
   new_csv.header.push_back("Y");
   new_csv.header.push_back("Z");
   new_csv.header.push_back("Occ");
   new_csv.header.push_back("TF");
   new_csv.header.push_back("Ele");
   // new_csv.header.push_back("Charge");
   // new_csv.header.push_back("Accessibility");

   unsigned int model           = 0;
   unsigned int line_count      = 0;
   bool         last_was_ENDMDL = true;

   for ( unsigned int i = 0; i < (unsigned int)pdb_text.size(); i++ )
   {
      QString qs = pdb_text[ i ];
      line_count++;

      QString left6 = qs.left( 6 ).toUpper();
      QString left5 = qs.left( 5 ).toUpper();

      if ( left6 == "TITLE " )
      {
         new_csv.title_text << qs.mid( 10 );
         continue;
      }

      if ( left6 == "HEADER" )
      {
         new_csv.header_text << qs.mid( 10 );
         continue;
      }

      if ( left6 == "MODEL " )
      {
         model++;
         last_was_ENDMDL = false;
         continue;
      }

      if ( left6 == "ENDMDL" )
      {
         last_was_ENDMDL = true;
         continue;
      }

      if ( ( left5 == "ATOM " ||
             left5 == "HETAT" ) && 
           last_was_ENDMDL )
      {
         model++;
         last_was_ENDMDL = false;
      }

      if ( left5 != "ATOM " && left5 != "HETAT" ) 
      {
         // not supporting anything else for now
         // later we should store, save remarks connct's etc
         continue;
      }

      /*
     http://www.rcsb.org/pdb/docs/format/pdbguide2.2/part_11.html

     ATOM record:
     COLUMNS        DATA TYPE       FIELD         DEFINITION
     ---------------------------------------------------------------------------------
     1 -  6        Record name     "ATOM  "
     7 - 11        Integer         serial        Atom serial number.
     13 - 16        Atom            name          Atom name. (sometimes starts at 12)
     17             Character       altLoc        Alternate location indicator.
     18 - 20        Residue name    resName       Residue name.
     22             Character       chainID       Chain identifier.
     23 - 26        Integer         resSeq        Residue sequence number.
     27             AChar           iCode         Code for insertion of residues.
     31 - 38        Real(8.3)       x             Orthogonal coordinates for X in Angstroms.
     39 - 46        Real(8.3)       y             Orthogonal coordinates for Y in Angstroms.
     47 - 54        Real(8.3)       z             Orthogonal coordinates for Z in Angstroms.
     55 - 60        Real(6.2)       occupancy     Occupancy.
     61 - 66        Real(6.2)       tempFactor    Temperature factor.
     73 - 76        LString(4)      segID         Segment identifier, left-justified.
     77 - 78        LString(2)      element       Element symbol, right-justified.
     79 - 80        LString(2)      charge        Charge on the atom.
      */
      
      vector < QString > data;

      data.push_back( QString("%1").arg( model ) );
      data.push_back( qs.mid( 21 , 1 ) );
      data.push_back( qs.mid( 17 , 3 ) );
      data.push_back( qs.mid( 22 , 4 ) );
      data.push_back( qs.mid( 12 , 4 ) );
      data.push_back( qs.mid( 6  , 5 ) );
      data.push_back( qs.mid( 16 , 1 ) );
      data.push_back( qs.mid( 26 , 1 ) );
      data.push_back( qs.mid( 30 , 8 ) );
      data.push_back( qs.mid( 38 , 8 ) );
      data.push_back( qs.mid( 46 , 8 ) );
      data.push_back( qs.mid( 54 , 6 ) );
      data.push_back( qs.mid( 60 , 6 ) );
      data.push_back( qs.mid( 76 , 2 ).trimmed() );
      // data.push_back( qs.mid( 78 , 2 ) );

      new_csv.data.push_back( data );
   }

   if ( lv == lv_csv )
   {
      if ( csv1.data.size() )
      {
         csv_undos.push_back( to_csv( lv_csv, csv1, false ) );
      }
      csv1 = new_csv;
      csv_to_lv( csv1, lv );
      csv_setup_keys( csv1 );
   } else {
      if ( csv2[ csv2_pos ].data.size() )
      {
         csv2_push( true );
         // csv2_undos[ csv2_pos ].push_back( to_csv( lv_csv2, csv2[ csv2_pos ], false ) );
      }
      csv2[ csv2_pos ] = new_csv;
      csv_to_lv( csv2[ csv2_pos ], lv );
      csv_setup_keys( csv2[ csv2_pos ] );
   }
   update_enables();
}

void US_Hydrodyn_Pdb_Tool::csv_sel()
{
   sel( lv_csv );
   update_enables_csv();
}

void US_Hydrodyn_Pdb_Tool::csv_sel_clear( )
{
   lv_csv->clearSelection();
   update_enables_csv();
}

void US_Hydrodyn_Pdb_Tool::csv_sel_clean()
{
   clean_selection( lv_csv );
   update_enables_csv();
}

void US_Hydrodyn_Pdb_Tool::csv_sel_invert()
{
   invert_selection( lv_csv );
   update_enables_csv();
}

void US_Hydrodyn_Pdb_Tool::csv_sel_chain()
{
   select_chain( lv_csv );
   update_enables_csv();
}

void US_Hydrodyn_Pdb_Tool::csv_sel_nearest_atoms()
{
   sel_nearest_atoms( lv_csv );
}

void US_Hydrodyn_Pdb_Tool::sel_nearest_atoms( QTreeWidget *lv )
{
   editor_msg( "blue", "sel nearest atoms");
   pdb_sel_count counts = count_selected( lv );
   bool ok;
   unsigned int atoms = ( unsigned int )US_Static::getInteger(
                                                                 "US-SOMO: PDB Editor", 
                                                                 QString( us_tr( "Enter the of nearest atoms to find (maximum %1):" ) )
                                                                 .arg( counts.not_selected_atoms )
                                                                 ,
                                                                 1,
                                                                 1, 
                                                                 counts.not_selected_atoms,
                                                                 1,
                                                                 &ok, 
                                                                 this );
   if ( !ok ) 
   {
      return;
   }

   map < QTreeWidgetItem *, double > distmap;

   QTreeWidgetItemIterator it1( lv );
   while ( (*it1) ) 
   {
      QTreeWidgetItem *item1 = (*it1);
      if ( !item1->childCount() && is_selected( item1 ) )
      {
         QTreeWidgetItemIterator it2( lv );
         while ( (*it2) ) 
         {
            QTreeWidgetItem *item2 = (*it2);
            if ( item1 != item2 && !item2->childCount() && !is_selected( item2 ) )
            {
               double d = pair_dist( item1, item2 );
               if ( !distmap.count( item2 ) )
               {
                  distmap[ item2 ] = d;
               } else {
                  if ( distmap[ item2 ] > d )
                  {
                     distmap[ item2 ] = d;
                  }
               }
            }
            ++it2;
         }
      }
      ++it1;
   }

   list < sortable_qli_double > lsqd;

   for ( map < QTreeWidgetItem *, double >::iterator it = distmap.begin(); 
         it != distmap.end(); 
         it++ ) 
   {
      sortable_qli_double sqd;
      sqd.lvi = it->first;
      sqd.d   = it->second;
      lsqd.push_back( sqd );
   }

   editor_msg("blue", QString( "size of list <%1>" ).arg( lsqd.size() ) );
   lsqd.sort();

   distmap.clear( );

   unsigned int pos = 0;
   for ( list < sortable_qli_double >::iterator it = lsqd.begin();
         it != lsqd.end();
         it++ )
   {
      distmap[ it->lvi ] = it->d;
      pos++;
      if ( pos >= atoms )
      {
         break;
      }
   }

   lv->clearSelection();
   
   QTreeWidgetItemIterator it( lv );
   while ( (*it) ) 
   {
      QTreeWidgetItem *item = (*it);
      if ( distmap.count( item ) )
      {
         item->setSelected( true );
         editor_msg("blue", QString( "size setting for dist %1 key %2" ).arg( distmap[ item ] ).arg( key( item ) ) );
      }
      ++it;
   }

   clean_selection( lv );

   if ( lv == lv_csv )
   {
      emit csv_selection_changed();
   } else {
      emit csv2_selection_changed();
   }
   editor_msg( "blue", "sel nearest atoms done");
}

void US_Hydrodyn_Pdb_Tool::select_model( QTreeWidget *lv, QString model )
{
   lv->clearSelection();
   QTreeWidgetItemIterator it1( lv );
   while ( (*it1) ) 
   {
      QTreeWidgetItem *item1 = (*it1);
      if ( !US_Static::lvi_depth( item1 ) && item1->text( 0 ) == model )
      {
         item1->setSelected( true );
      }
      ++it1;
   }
   clean_selection( lv );
}

void US_Hydrodyn_Pdb_Tool::select_model( QTreeWidget *lv, const set < QString > & models )
{
   lv->clearSelection();
   QTreeWidgetItemIterator it1( lv );
   while ( (*it1) ) 
   {
      QTreeWidgetItem *item1 = (*it1);
      if ( !US_Static::lvi_depth( item1 ) && models.count( item1->text( 0 ) ) )
      {
         item1->setSelected( true );
      }
      ++it1;
   }
   clean_selection( lv );
}

void US_Hydrodyn_Pdb_Tool::select_chain( QTreeWidget *lv, QString chain )
{
   QStringList chains;
   chains << chain;
   return select_chain( lv, chains );
}

void US_Hydrodyn_Pdb_Tool::select_chain( QTreeWidget *lv, QStringList chains )
{
   lv->clearSelection();
   update_enables_csv();
   if ( !chains.size() )
   {
      return;
   }

   map < QString, bool > chains_to_select;
   for ( unsigned int i = 0; i < (unsigned int)chains.size(); i++ )
   {
      chains_to_select[ chains[ i ] ] = true;
   }

   QTreeWidgetItemIterator it1( lv );
   while ( (*it1) ) 
   {
      QTreeWidgetItem *item1 = (*it1);
      if ( US_Static::lvi_depth( item1 ) == 1 &&
           chains_to_select.count( item1->text( 0 ) ) )
      {
         item1->setSelected( true );
      }
      ++it1;
   }
   clean_selection( lv );
   if ( lv == lv_csv )
   {
      selection_since_count_csv1 = true;
      csv_sel_msg();
   } else {
      selection_since_count_csv2 = true;
      csv2_sel_msg();
   }
}

void US_Hydrodyn_Pdb_Tool::select_chain( QTreeWidget *lv )
{
   pdb_sel_count counts = count_selected( lv );
   if ( counts.chains <= 1 )
   {
      return;
   }
   QStringList chains = chain_set( lv );
   map < QString, bool > chains_to_select;

   bool ok;

   do {
      QString chain = US_Static::getItem(
                                            us_tr( "US-SOMO: PDB editor : Select chain" ) ,
                                            us_tr( "Select a chain or CANCEL when done" ),
                                            chains, 
                                            0, 
                                            false, 
                                            &ok,
                                            this );
      if ( ok )
      {
         chains_to_select[ chain ] = true;
      }
      QStringList new_list;
      for ( unsigned int i = 0; i < (unsigned int)chains.size(); i++ )
      {
         if ( !chains_to_select.count( chains[ i ] ) )
         {
            new_list << chains[ i ];
         }
      }
      chains = new_list;
   } while ( ok && chains.size() );

   if ( !chains.size() )
   {
      return;
   }

   // now select just these chains
   // first build map of original selections

   map < QTreeWidgetItem *, bool > selected;

   {
      QTreeWidgetItemIterator it1( lv );

      while ( (*it1) ) 
      {
         QTreeWidgetItem *item1 = (*it1);
         if ( is_selected( item1 ) )
         {
            selected[ item1 ] = true;
         }
         ++it1;
      }
   }

   lv->clearSelection();

   QTreeWidgetItemIterator it1( lv );
   while ( (*it1) ) 
   {
      QTreeWidgetItem *item1 = (*it1);
      if ( US_Static::lvi_depth( item1 ) == 1 &&
           selected.count( item1 ) &&
           chains_to_select.count( item1->text( 0 ) ) )
      {
         item1->setSelected( true );
      }
      ++it1;
   }
   clean_selection( lv );
   if ( lv == lv_csv )
   {
      selection_since_count_csv1 = true;
      csv_sel_msg();
   } else {
      selection_since_count_csv2 = true;
      csv2_sel_msg();
   }
}

void US_Hydrodyn_Pdb_Tool::distances( QTreeWidget *lv )
{
   static QString previous_selection;

   editor_msg( "blue", "compute pairwise distances");
   pdb_sel_count counts = count_selected( lv );
   bool ok;
   unsigned int atoms = ( unsigned int )US_Static::getInteger(
                                                                 us_tr( "US-SOMO: PDB editor : Pairwise Distances" ) ,
                                                                 QString( us_tr( "Enter the of number of minimum pairwise distances to find:" ) )
                                                                 ,
                                                                 1,
                                                                 1, 
                                                                 counts.atoms * counts.atoms,
                                                                 1,
                                                                 &ok, 
                                                                 this );

   if ( !ok ) 
   {
      return;
   }

   double dist_thresh = US_Static::getDouble(
                                                us_tr( "US-SOMO: PDB editor : Pairwise Distances" ) ,
                                                QString( us_tr( "Enter a maximum threshold distance or press CANCEL for no threshold limit:" ) )
                                                ,
                                                0,
                                                0, 
                                                1e99,
                                                4,
                                                &ok, 
                                                this );

   if ( !ok )
   {
      dist_thresh = 1e99;
   }

   QString restrict_atom;

   {
      QStringList qsl = atom_set( lv );
      bool ok;
      int use_pos = 0;
      if ( !previous_selection.isEmpty() )
      {
         for ( unsigned int pos = 0; pos < (unsigned int)qsl.size(); pos++ )
         {
            if ( qsl[ pos ] == previous_selection )
            {
               use_pos = ( int ) pos;
               break;
            }
         }
      }

      restrict_atom = US_Static::getItem(
                                            us_tr( "US-SOMO: PDB editor : Pairwise Distances" ) ,
                                            us_tr( "Restrict search to pairs with selected atom or CANCEL to compare all" ),
                                            qsl, 
                                            use_pos, 
                                            false, 
                                            &ok,
                                            this );
      if ( !ok )
      {
         // user pressed Cancel
         restrict_atom = "";
      } else {
         previous_selection = restrict_atom;
      }
   }

   if ( counts.models > 1 )
   {
      // go through each model and select individually
      // summary select all at the end
      // n will be from each model
      QStringList models = model_set( lv );
      QMessageBox::information( this, 
                                us_tr( "US-SOMO: PDB editor : Pairwise Distances" ) ,
                                QString( us_tr( "%1 models are selected, the operation will be performed on each model independently." ) ).arg( models.size() ) );

      map < QTreeWidgetItem *, bool > selected_items;
      list < sortable_qlipair_double > global_lsqd;
         
      for ( unsigned int i = 0; i < (unsigned int)models.size(); i++ )
      {
         editor_msg( "black", QString( us_tr( "Selecting model %1\n" ) ).arg( models[ i ] ) );
         qApp->processEvents();

         select_model( lv, models[ i ] );
         editor_msg( "black", QString( us_tr( "Processing model %1\n" ) ).arg( models[ i ] ) );
         qApp->processEvents();
         
         map < lvipair, double > distmap;

         QTreeWidgetItemIterator it1( lv );
         if ( dist_thresh != 1e99 )
         {
            editor_msg( "brown", QString( us_tr( "using threshold limit %1" ).arg( dist_thresh, 0, 'f', 3 ) ) );
            qApp->processEvents();
            unsigned int count = 0;
            while ( (*it1) ) 
            {
               QTreeWidgetItem *item1 = (*it1);
               if ( restrict_atom.isEmpty() || get_atom_name( item1 ) == restrict_atom )
               {
                  if ( !item1->childCount() && is_selected( item1 ) )
                  {
                     if ( !( ++count % 500 ) )
                     {
                        editor_msg( "black", QString( "processed %1" ).arg( count ) );
                        qApp->processEvents();
                     }

                     QTreeWidgetItemIterator it2( lv );
                     while ( (*it2) ) 
                     {
                        QTreeWidgetItem *item2 = (*it2);
                        lvipair lvp;
                        lvp.lvi1 = item2;
                        lvp.lvi2 = item1;
                        if ( item1 != item2 && !item2->childCount() && is_selected( item2 ) && !distmap.count( lvp ) )
                        {
                           double dx = fabs( item1->text( 3 ).toDouble() - item2->text( 3 ).toDouble() );
                           if ( dx <= dist_thresh )
                           {
                              double dy = fabs( item1->text( 4 ).toDouble() - item2->text( 4 ).toDouble() );
                              if ( dy <= dist_thresh )
                              {
                                 double dz = fabs( item1->text( 5 ).toDouble() - item2->text( 5 ).toDouble() );
                                 if ( dz <= dist_thresh )
                                 {
                                    double d = sqrt( dx * dx + dy * dy + dz * dz );
                                    if ( d <= dist_thresh )
                                    {
                                       lvp.lvi1 = item1;
                                       lvp.lvi2 = item2;
                                       if ( !distmap.count( lvp ) )
                                       {
                                          distmap[ lvp ] = d;
                                       } else {
                                          if ( distmap[ lvp ] > d )
                                          {
                                             distmap[ lvp ] = d;
                                          }
                                       }
                                    }
                                 }
                              }
                           }
                        
                        }
                        ++it2;
                     }
                  }
               }
               ++it1;
            }
         } else {
            unsigned int count = 0;
            while ( (*it1) ) 
            {
               QTreeWidgetItem *item1 = (*it1);
               if ( restrict_atom.isEmpty() || get_atom_name( item1 ) == restrict_atom )
               {
                  if ( !item1->childCount() && is_selected( item1 ) )
                  {
                     if ( !( ++count % 500 ) )
                     {
                        editor_msg( "black", QString( "processed %1" ).arg( count ) );
                        qApp->processEvents();
                     }
                     QTreeWidgetItemIterator it2( lv );
                     while ( (*it2) ) 
                     {
                        QTreeWidgetItem *item2 = (*it2);
                        lvipair lvp;
                        lvp.lvi1 = item2;
                        lvp.lvi2 = item1;
                        if ( item1 != item2 && !item2->childCount() && is_selected( item2 ) && !distmap.count( lvp ) )
                        {
                           double dx = item1->text( 3 ).toDouble() - item2->text( 3 ).toDouble();
                           double dy = item1->text( 4 ).toDouble() - item2->text( 4 ).toDouble();
                           double dz = item1->text( 5 ).toDouble() - item2->text( 5 ).toDouble();
                           double d = sqrt( dx * dx + dy * dy + dz * dz );
                           lvp.lvi1 = item1;
                           lvp.lvi2 = item2;
                           if ( !distmap.count( lvp ) )
                           {
                              distmap[ lvp ] = d;
                           } else {
                              if ( distmap[ lvp ] > d )
                              {
                                 distmap[ lvp ] = d;
                              }
                           }
                        
                        }
                        ++it2;
                     }
                  }
               }
               ++it1;
            }
         }

         
         list < sortable_qlipair_double > lsqd;
         
         for ( map < lvipair, double >::iterator it = distmap.begin(); 
               it != distmap.end(); 
               it++ ) 
         {
            sortable_qlipair_double sqd;
            sqd.lvip = it->first;
            sqd.d    = it->second;
            lsqd.push_back( sqd );
         }
         
         // editor_msg("blue", QString( "size of list <%1>" ).arg( lsqd.size() ) );
         lsqd.sort();
         
         distmap.clear( );
         
         unsigned int pos = 0;
         for ( list < sortable_qlipair_double >::iterator it = lsqd.begin();
               it != lsqd.end();
               it++ )
         {
            distmap[ it->lvip ] = it->d;
            selected_items[ it->lvip.lvi1 ] = true;
            selected_items[ it->lvip.lvi2 ] = true;
            editor_msg( "blue", 
                        QString( us_tr( "pw distance %1 %2 %3\n" ) )
                        .arg( key( it->lvip.lvi1 ) )
                        .arg( key( it->lvip.lvi2 ) )
                        .arg( it->d ) );
            pos++;
            global_lsqd.push_back( *it );
            if ( pos >= atoms )
            {
               break;
            }
         }
      }
      
      editor_msg( "black", QString( us_tr( "Global model pairwise distance summary\n" ) ) );

      global_lsqd.sort();
      map < lvipair, double > distmap;
         
      unsigned int pos = 0;
      for ( list < sortable_qlipair_double >::iterator it = global_lsqd.begin();
            it != global_lsqd.end();
            it++ )
      {
         distmap[ it->lvip ] = it->d;
         selected_items[ it->lvip.lvi1 ] = true;
         selected_items[ it->lvip.lvi2 ] = true;
         editor_msg( "blue", 
                     QString( us_tr( "pw distance %1 %2 %3\n" ) )
                     .arg( key( it->lvip.lvi1 ) )
                     .arg( key( it->lvip.lvi2 ) )
                     .arg( it->d ) );
         pos++;
      }
      
      lv->clearSelection();
      
      QTreeWidgetItemIterator it( lv );
      while ( (*it) ) 
      {
         QTreeWidgetItem *item = (*it);
         if ( selected_items.count( item ) )
         {
            item->setSelected( true );
            }
         ++it;
      }
      
      clean_selection( lv );
      
      if ( lv == lv_csv )
      {
         emit csv_selection_changed();
      } else {
         emit csv2_selection_changed();
      }
      editor_msg( "blue", QString( us_tr( "Pairwise distance report for %1 atoms done") ).arg( atoms ) );
      return;
   }

   map < lvipair, double > distmap;

   QTreeWidgetItemIterator it1( lv );
   if ( dist_thresh != 1e99 )
   {
      editor_msg( "brown", QString( us_tr( "using threshold limit %1" ).arg( dist_thresh, 0, 'f', 3 ) ) );
      qApp->processEvents();
      unsigned int count = 0;
      while ( (*it1) ) 
      {
         QTreeWidgetItem *item1 = (*it1);
         if ( restrict_atom.isEmpty() || get_atom_name( item1 ) == restrict_atom )
         {
            if ( !item1->childCount() && is_selected( item1 ) )
            {
               if ( !( ++count % 500 ) )
               {
                  editor_msg( "black", QString( "processed %1" ).arg( count ) );
                  qApp->processEvents();
               }
            
               QTreeWidgetItemIterator it2( lv );
               while ( (*it2) ) 
               {
                  QTreeWidgetItem *item2 = (*it2);
                  lvipair lvp;
                  lvp.lvi1 = item2;
                  lvp.lvi2 = item1;
                  if ( item1 != item2 && !item2->childCount() && is_selected( item2 ) && !distmap.count( lvp ) )
                  {
                     double dx = item1->text( 3 ).toDouble() - item2->text( 3 ).toDouble();
                     if ( dx <= dist_thresh )
                     {
                        double dy = item1->text( 4 ).toDouble() - item2->text( 4 ).toDouble();
                        if ( dy <= dist_thresh )
                        {
                           double dz = item1->text( 5 ).toDouble() - item2->text( 5 ).toDouble();
                           if ( dz <= dist_thresh )
                           {
                              double d = sqrt( dx * dx + dy * dy + dz * dz );
                              if ( d <= dist_thresh )
                              {
                                 lvp.lvi1 = item1;
                                 lvp.lvi2 = item2;
                                 if ( !distmap.count( lvp ) )
                                 {
                                    distmap[ lvp ] = d;
                                 } else {
                                    if ( distmap[ lvp ] > d )
                                    {
                                       distmap[ lvp ] = d;
                                    }
                                 }
                              }
                           }
                        }
                     }
                  }
                  ++it2;
               }
            }
         }
         ++it1;
      }
   } else {
      unsigned int count = 0;
      while ( (*it1) ) 
      {
         QTreeWidgetItem *item1 = (*it1);
         if ( restrict_atom.isEmpty() || get_atom_name( item1 ) == restrict_atom )
         {
            if ( !item1->childCount() && is_selected( item1 ) )
            {
               if ( !( ++count % 500 ) )
               {
                  editor_msg( "black", QString( "processed %1" ).arg( count ) );
                  qApp->processEvents();
               }
            
               QTreeWidgetItemIterator it2( lv );
               while ( (*it2) ) 
               {
                  QTreeWidgetItem *item2 = (*it2);
                  lvipair lvp;
                  lvp.lvi1 = item2;
                  lvp.lvi2 = item1;
                  if ( item1 != item2 && !item2->childCount() && is_selected( item2 ) && !distmap.count( lvp ) )
                  {
                     double dx = item1->text( 3 ).toDouble() - item2->text( 3 ).toDouble();
                     double dy = item1->text( 4 ).toDouble() - item2->text( 4 ).toDouble();
                     double dz = item1->text( 5 ).toDouble() - item2->text( 5 ).toDouble();
                     double d = sqrt( dx * dx + dy * dy + dz * dz );
                     lvp.lvi1 = item1;
                     lvp.lvi2 = item2;
                     if ( !distmap.count( lvp ) )
                     {
                        distmap[ lvp ] = d;
                     } else {
                        if ( distmap[ lvp ] > d )
                        {
                           distmap[ lvp ] = d;
                        }
                     }
                  
                  }
                  ++it2;
               }
            }
         }
         ++it1;
      }
   }

   list < sortable_qlipair_double > lsqd;

   for ( map < lvipair, double >::iterator it = distmap.begin(); 
         it != distmap.end(); 
         it++ ) 
   {
      sortable_qlipair_double sqd;
      sqd.lvip = it->first;
      sqd.d    = it->second;
      lsqd.push_back( sqd );
   }

   editor_msg("blue", QString( "size of list <%1>" ).arg( lsqd.size() ) );
   lsqd.sort();

   distmap.clear( );

   map < QTreeWidgetItem *, bool > selected_items;

   unsigned int pos = 0;
   for ( list < sortable_qlipair_double >::iterator it = lsqd.begin();
         it != lsqd.end();
         it++ )
   {
      distmap[ it->lvip ] = it->d;
      selected_items[ it->lvip.lvi1 ] = true;
      selected_items[ it->lvip.lvi2 ] = true;
      editor_msg( "blue", 
                  QString( us_tr( "pw distance %1 %2 %3\n" ) )
                  .arg( key( it->lvip.lvi1 ) )
                  .arg( key( it->lvip.lvi2 ) )
                  .arg( it->d ) );
      pos++;
      if ( pos >= atoms )
      {
         break;
      }
   }

   lv->clearSelection();
   
   QTreeWidgetItemIterator it( lv );
   while ( (*it) ) 
   {
      QTreeWidgetItem *item = (*it);
      if ( selected_items.count( item ) )
      {
         item->setSelected( true );
      }
      ++it;
   }

   clean_selection( lv );

   if ( lv == lv_csv )
   {
      emit csv_selection_changed();
   } else {
      emit csv2_selection_changed();
   }
   editor_msg( "blue", QString( us_tr( "Pairwise distance report for %1 atoms done") ).arg( atoms ) );
}

bool US_Hydrodyn_Pdb_Tool::sol2wat( QTreeWidget *lv, double use_radius, QString filepath, int frame, QString reportpath )
{
   // get threshold

   bool ok;
   double theo_hydroradius = pow( ( 3.0/( 4.0 * M_PI ) ) * ((US_Hydrodyn *)us_hydrodyn)->misc.hydrovol, 1.0 / 3.0 );
   double hydroradius = use_radius;

   // qDebug() << "sol2wat 1";
   if ( hydroradius == 0e0 ) {
      // non zero if traj
      hydration_summary         .clear();
      hydration_summary_res     .clear();
      hydration_summary_res_atom.clear();

      hydroradius = US_Static::getDouble(
                                         us_tr( "US-SOMO: PDB editor : SOL->WAT" ) ,
                                         QString( us_tr( "Enter a radius in Angstrom for water.\n"
                                                         "This will be used as a threshold to determine if the SOL is converted to WAT.\n"
                                                         "The radius will be added to the vdW radius of the nearest non-water and\n"
                                                         "if the distance between centers is less than or equal to the sum of the\n"
                                                         "radii, the water will be kept.\n"
                                                         ) )
                                         ,
                                         theo_hydroradius,
                                         0.0001,
                                         10,
                                         4,
                                         &ok, 
                                         this
                                         );

      if ( !ok ) {
         return false;
      }

      editor_msg( "brown", QString( us_tr( "SOL->WAT using radius %1 [A]" ).arg( hydroradius, 0, 'f', 3 ) ) );


      // editor_msg( "brown", QString( us_tr( "SOL->WAT water radius %1 [A]" ).arg( hydroradius, 0, 'f', 3 ) ) );
      qApp->processEvents();
   }

   // qDebug() << "sol2wat 2";
   csv tmp_csv;
   if ( lv == lv_csv )
   {
      tmp_csv = to_csv( lv_csv, csv1 );
   } else {
      tmp_csv = to_csv( lv_csv2, csv2[ csv2_pos ] );
   }
   // qDebug() << "sol2wat 3";
   QString report_header;
   report_header += QString( us_tr( "SOL->WAT using radius %1 [A]\n" ) ).arg( hydroradius, 0, 'f', 3 );
   report_header += QString( us_tr( "Name %1\n" ) ).arg( tmp_csv.name );
   map < int, QString > report_SOLs;
   
   vector < int > SOLs;
   set < int > SOL_set;
   vector < point > SOL_points;
   vector < int > compares;
   vector < point > compare_points;
   vector < double > compare_vdw;
   set < int > sol2wat_csv_index;

   point t;
   point t2;

   for ( int i = 0; i < (int) tmp_csv.data.size(); ++i ) {
      // QString model      = tmp_csv.data[ i ][ 0 ];
      // QString chain      = QString( tmp_csv.data[ i ][ 1 ] ).trimmed();
      QString residue    = QString( tmp_csv.data[ i ][ 2 ] ).trimmed();
      QString atom       = QString( tmp_csv.data[ i ][ 4 ] ).trimmed();

      t.axis[ 0 ] = tmp_csv.data[ i ][ 8 ].toFloat();
      t.axis[ 1 ] = tmp_csv.data[ i ][ 9 ].toFloat();
      t.axis[ 2 ] = tmp_csv.data[ i ][ 10 ].toFloat();

      if ( residue == "SOL" ) {
         SOLs.push_back( i );
         SOL_set.insert( i );
         SOL_points.push_back( t );
      } else {
         if ( residue != "WAT" ) {
            compares.push_back( i );
            compare_points.push_back( t );
            QString res_idx =
               QString( "%1|%2" )
               .arg( atom != "OXT" ? residue : "OXT" )
               .arg( atom );
            if ( ((US_Hydrodyn *)us_hydrodyn)->vdwf.count( res_idx ) ) {
               compare_vdw.push_back( ((US_Hydrodyn *)us_hydrodyn)->vdwf[ res_idx ].r );
            } else {
               compare_vdw.push_back( 0 );
               editor_msg( "red", QString( us_tr( "SOL->WAT: %1 missing vdW radius, using a value of zero" ).arg( res_idx ) ) );
            }
         }
      }
   }
         
   qApp->processEvents();

   int SOL_report_adds = 0;
   map < int, int > SOL_nonSOL_hits;
   double minimum_computed_radius = 1e99;
   int minimum_computed_radius_posi = 0;
   int minimum_computed_radius_posj = 0;

   // new csv detail report
   csv detail_csv;
   detail_csv.name     = QFileInfo( filepath ).baseName() + "_sol2wat_detail";
   detail_csv.filename = filepath +  "_sol2wat_detail.csv";

   detail_csv.header.push_back( "Solute residue name" );
   detail_csv.header.push_back( "Solute residue number" );
   detail_csv.header.push_back( "Solute atom name" );
   detail_csv.header.push_back( "Solute atom number" );
   detail_csv.header.push_back( "Solute position X" );
   detail_csv.header.push_back( "Solute position Y" );
   detail_csv.header.push_back( "Solute position Z" );
   detail_csv.header.push_back( "Solute vdW radius" );
   detail_csv.header.push_back( "Water rank" );
   detail_csv.header.push_back( "Water residue name" );
   detail_csv.header.push_back( "Water residue numbe" );
   detail_csv.header.push_back( "Water atom nam" );
   detail_csv.header.push_back( "Water atom numbe" );
   detail_csv.header.push_back( "Water position X" );
   detail_csv.header.push_back( "Water position Y" );
   detail_csv.header.push_back( "Water position Z" );
   detail_csv.header.push_back( "Distance" );
   detail_csv.header.push_back( "Water Computed Radius" );

   map < QString, map < QString, set < int > > > res_SOL_used; // map of residue names and SOLs used // rname, rnum, SOL
   
   map < QString, map < QString, double > > chosen_radius_for_SOLs; // map of residue number, atom number to chosen radius for SOL

   for ( int j = 0; j < (int) compares.size(); ++j ) {
      t2 = compare_points[ j ];
      double use_thresh = hydroradius + compare_vdw[ j ];
      double use_thresh2 = use_thresh * use_thresh;
      us_qdebug( QString( "atom %1 hydroradius %2 vdw radius %3 use thresh %4\n" )
              .arg( j )
              .arg( hydroradius )
              .arg( compare_vdw[ j ] )
              .arg( use_thresh )
              );
      
      map < double, int > paired_sols_by_distance;

      int csvj = compares[ j ];

      for ( int i = 0; i < (int) SOLs.size(); ++i ) {
         t = SOL_points[ i ];

         double dx = fabs( t2.axis[ 0 ] - t.axis[ 0 ] );
         if ( dx <= use_thresh ) {
            double dy = fabs( t2.axis[ 1 ] - t.axis[ 1 ] );
            if ( dy <= use_thresh ) {
               double dz = fabs( t2.axis[ 2 ] - t.axis[ 2 ] );
               if ( dz <= use_thresh ) {
                  double d2 = dx * dx + dy * dy + dz * dz;
                  if ( d2 <= use_thresh2 ) {
                     // us_qdebug( QString( "accepted SOL between %1 [%2,%3,%4] %5 [%6,%7,%8] distance %9" )
                     //         .arg( i )
                     //         .arg( t.axis[ 0 ] )
                     //         .arg( t.axis[ 1 ] )
                     //         .arg( t.axis[ 2 ] )
                     //         .arg( j )
                     //         .arg( t2.axis[ 0 ] )
                     //         .arg( t2.axis[ 1 ] )
                     //         .arg( t2.axis[ 2 ] )
                     //         .arg( sqrt( d2 ) )
                     //         );
                     int csvi = SOLs[ i ];
                     sol2wat_csv_index.insert( csvi );
                     SOL_nonSOL_hits[ csvi ]++;
                     {
                        double d = sqrt( d2 );
                        double this_computed_radius = d - compare_vdw[ j ];
                        if ( minimum_computed_radius > this_computed_radius ) {
                           minimum_computed_radius = this_computed_radius;
                           minimum_computed_radius_posi = csvi;
                           minimum_computed_radius_posj = csvj;
                        }

                        SOL_report_adds++;
                           
                        report_SOLs[ csvi ] +=
                           QString( "------------------------------------------------------------------------\n" )
                           + QString("")
                           .sprintf(     
                                    "ATOM  %5d%5s%4s %1s%4d    %8.3f%8.3f%8.3f%6.2f%6.2f          %2s\n",
                                    tmp_csv.data[ csvi ][ 5  ].toUInt(),
                                    tmp_csv.data[ csvi ][ 4  ].toLatin1().data(),
                                    tmp_csv.data[ csvi ][ 2  ].toLatin1().data(),
                                    tmp_csv.data[ csvi ][ 1  ].toLatin1().data(),
                                    tmp_csv.data[ csvi ][ 3  ].toUInt(),
                                    tmp_csv.data[ csvi ][ 8  ].toFloat(),
                                    tmp_csv.data[ csvi ][ 9  ].toFloat(),
                                    tmp_csv.data[ csvi ][ 10 ].toFloat(),
                                    tmp_csv.data[ csvi ][ 11 ].toFloat(),
                                    tmp_csv.data[ csvi ][ 12 ].toFloat(),
                                    tmp_csv.data[ csvi ][ 13 ].toLatin1().data()
                                         )
                           + QString("")
                           .sprintf(     
                                    "ATOM  %5d%5s%4s %1s%4d    %8.3f%8.3f%8.3f%6.2f%6.2f          %2s\n",
                                    tmp_csv.data[ csvj ][ 5  ].toUInt(),
                                    tmp_csv.data[ csvj ][ 4  ].toLatin1().data(),
                                    tmp_csv.data[ csvj ][ 2  ].toLatin1().data(),
                                    tmp_csv.data[ csvj ][ 1  ].toLatin1().data(),
                                    tmp_csv.data[ csvj ][ 3  ].toUInt(),
                                    tmp_csv.data[ csvj ][ 8  ].toFloat(),
                                    tmp_csv.data[ csvj ][ 9  ].toFloat(),
                                    tmp_csv.data[ csvj ][ 10 ].toFloat(),
                                    tmp_csv.data[ csvj ][ 11 ].toFloat(),
                                    tmp_csv.data[ csvj ][ 12 ].toFloat(),
                                    tmp_csv.data[ csvj ][ 13 ].toLatin1().data()
                                         )
                           + QString( "" )
                           .sprintf(
                                    "Distance                              : %8.4f\n"
                                    "vdW radius of non-SOL                 : %8.2f\n"
                                    "computed vdW radius of SOL if tangent : %8.4f\n",
                                    d,
                                    compare_vdw[ j ],
                                    d - compare_vdw[ j ]
                                    )
                           ;

                        // new detail info

                        paired_sols_by_distance[ d ] = i;
                     }
                  }
               }
            }
         }
      } // end of SOLs loop

      {
         // build up detail
         int rank = 1;
         for ( map < double, int >::iterator it = paired_sols_by_distance.begin();
               it != paired_sols_by_distance.end();
               ++it ) {
            int csvi = SOLs[ it->second ]; // the water
            vector < QString > tmp_data;
            // solute info
            tmp_data.push_back( tmp_csv.data[ csvj ][ 2 ] );
            tmp_data.push_back( tmp_csv.data[ csvj ][ 3 ] );
            tmp_data.push_back( tmp_csv.data[ csvj ][ 4 ] );
            tmp_data.push_back( tmp_csv.data[ csvj ][ 5 ] );
            tmp_data.push_back( tmp_csv.data[ csvj ][ 8 ] );
            tmp_data.push_back( tmp_csv.data[ csvj ][ 9 ] );
            tmp_data.push_back( tmp_csv.data[ csvj ][ 10 ] );

            // solute radius
            tmp_data.push_back( QString( "%1" ).arg( compare_vdw[ j ] ) );

            // rank
            tmp_data.push_back( QString( "%1" ).arg( rank++ ) );

            // water info

            tmp_data.push_back( tmp_csv.data[ csvi ][ 2 ] );
            tmp_data.push_back( tmp_csv.data[ csvi ][ 3 ] );
            tmp_data.push_back( tmp_csv.data[ csvi ][ 4 ] );
            tmp_data.push_back( tmp_csv.data[ csvi ][ 5 ] );
            tmp_data.push_back( tmp_csv.data[ csvi ][ 8 ] );
            tmp_data.push_back( tmp_csv.data[ csvi ][ 9 ] );
            tmp_data.push_back( tmp_csv.data[ csvi ][ 10 ] );

            // distance, computed radius

            tmp_data.push_back( QString( "%1" ).arg( it->first ) );
            tmp_data.push_back( QString( "%1" ).arg( it->first - compare_vdw[ j ] ) );

            detail_csv.data.push_back( tmp_data );

            // this is per frame
            hydration_info hi;
            hi.d = it->first;
            hi.r = it->first - compare_vdw[ j ];
            QString rname = tmp_csv.data[ csvj ][ 2 ];
            QString rnum  = tmp_csv.data[ csvj ][ 3 ];
            QString aname = tmp_csv.data[ csvj ][ 4 ];


            // pick minimum radius for SOL
            {
               QString wrnum = tmp_csv.data[ csvi ][ 3 ];
               QString wanum = tmp_csv.data[ csvi ][ 5 ];

               if (
                   chosen_radius_for_SOLs.count( wrnum ) &&
                   chosen_radius_for_SOLs[ wrnum ].count( wanum ) ) {
                  if ( chosen_radius_for_SOLs[ wrnum ][ wanum ] > hi.r ) {
                     chosen_radius_for_SOLs[ wrnum ][ wanum ] = hi.r;
                  }
               } else {
                  chosen_radius_for_SOLs[ wrnum ][ wanum ] = hi.r;
               }
            }

            hydration_summary[ csvj ][ frame ]                           .push_back( hi );
            if ( !res_SOL_used.count( rname ) ||
                 !res_SOL_used[ rname ].count( rnum ) ||
                 !res_SOL_used[ rname ][ rnum ].count( csvi ) ) {
               hydration_summary_res     [ rname ][ rnum ][ frame ]         .push_back( hi );
               res_SOL_used[ rname ][ rnum ].insert( csvi );
            }
                 
            hydration_summary_res_atom[ rname ][ aname ][ rnum ][ frame ].push_back( hi );
            if ( !hydration_header.count( csvj ) ) {
               hydration_header_info hhi;
               hhi.vdw     = compare_vdw[ j ];
               hhi.rname   = tmp_csv.data[ csvj ][ 2 ];
               hhi.rnum    = tmp_csv.data[ csvj ][ 3 ];
               hhi.aname   = tmp_csv.data[ csvj ][ 4 ];
               hhi.anum    = tmp_csv.data[ csvj ][ 5 ];
               hydration_header[ csvj ] = hhi;
            } else {
               if ( 
                   hydration_header[ csvj ].vdw     != compare_vdw[ j ] ||
                   hydration_header[ csvj ].rname   != tmp_csv.data[ csvj ][ 2 ] ||
                   hydration_header[ csvj ].rnum    != tmp_csv.data[ csvj ][ 3 ] ||
                   hydration_header[ csvj ].aname   != tmp_csv.data[ csvj ][ 4 ] ||
                   hydration_header[ csvj ].anum    != tmp_csv.data[ csvj ][ 5 ]
                    ) {
                  editor_msg( "red", us_tr( "Error: pdbs don't have identical structure" ) );
               }
            }
         }
      }
   }

   editor_msg( "blue", QString( us_tr( "Found %1 of %2 SOLs to Convert" ) ).arg( sol2wat_csv_index.size() ).arg( SOLs.size() ) );
   report_header += QString( us_tr( "Found %1 of %2 SOLs to Convert\n" ) ).arg( sol2wat_csv_index.size() ).arg( SOLs.size() );
   report_header += QString( us_tr( "Minimum computed vdW radius of SOL if tangent: %1 occurs between:\n" ).arg( minimum_computed_radius ) );
   report_header += "\n";

   report_header +=
      QString("")
      .sprintf(     
               "ATOM  %5d%5s%4s %1s%4d    %8.3f%8.3f%8.3f%6.2f%6.2f          %2s\n",
               tmp_csv.data[ minimum_computed_radius_posi ][ 5  ].toUInt(),
               tmp_csv.data[ minimum_computed_radius_posi ][ 4  ].toLatin1().data(),
               tmp_csv.data[ minimum_computed_radius_posi ][ 2  ].toLatin1().data(),
               tmp_csv.data[ minimum_computed_radius_posi ][ 1  ].toLatin1().data(),
               tmp_csv.data[ minimum_computed_radius_posi ][ 3  ].toUInt(),
               tmp_csv.data[ minimum_computed_radius_posi ][ 8  ].toFloat(),
               tmp_csv.data[ minimum_computed_radius_posi ][ 9  ].toFloat(),
               tmp_csv.data[ minimum_computed_radius_posi ][ 10 ].toFloat(),
               tmp_csv.data[ minimum_computed_radius_posi ][ 11 ].toFloat(),
               tmp_csv.data[ minimum_computed_radius_posi ][ 12 ].toFloat(),
               tmp_csv.data[ minimum_computed_radius_posi ][ 13 ].toLatin1().data()
                    )
      + QString("")
      .sprintf(     
               "ATOM  %5d%5s%4s %1s%4d    %8.3f%8.3f%8.3f%6.2f%6.2f          %2s\n",
               tmp_csv.data[ minimum_computed_radius_posj ][ 5  ].toUInt(),
               tmp_csv.data[ minimum_computed_radius_posj ][ 4  ].toLatin1().data(),
               tmp_csv.data[ minimum_computed_radius_posj ][ 2  ].toLatin1().data(),
               tmp_csv.data[ minimum_computed_radius_posj ][ 1  ].toLatin1().data(),
               tmp_csv.data[ minimum_computed_radius_posj ][ 3  ].toUInt(),
               tmp_csv.data[ minimum_computed_radius_posj ][ 8  ].toFloat(),
               tmp_csv.data[ minimum_computed_radius_posj ][ 9  ].toFloat(),
               tmp_csv.data[ minimum_computed_radius_posj ][ 10 ].toFloat(),
               tmp_csv.data[ minimum_computed_radius_posj ][ 11 ].toFloat(),
               tmp_csv.data[ minimum_computed_radius_posj ][ 12 ].toFloat(),
               tmp_csv.data[ minimum_computed_radius_posj ][ 13 ].toLatin1().data()
                    )
      ;
                             
   report_header += "\n";

   if ( (int) sol2wat_csv_index.size() == SOL_report_adds ) {
      report_header += QString( us_tr(  "Only one SOL atom per non SOL found.\n" ) );
   } else {
      report_header += QString( us_tr( "N.B.: Some SOL atoms were within the threshold of multiple non-SOL atoms.\n" ) );
      for ( map < int, int >::iterator it = SOL_nonSOL_hits.begin();
            it != SOL_nonSOL_hits.end();
            ++it ) {
         if ( it->second > 1 ) {
            report_header +=
               QString( "" )
               .sprintf(     
                        "ATOM  %5d%5s%4s %1s%4d is close to %d non-SOL atoms\n",
                        tmp_csv.data[ it->first ][ 5  ].toUInt(),
                        tmp_csv.data[ it->first ][ 4  ].toLatin1().data(),
                        tmp_csv.data[ it->first ][ 2  ].toLatin1().data(),
                        tmp_csv.data[ it->first ][ 1  ].toLatin1().data(),
                        tmp_csv.data[ it->first ][ 3  ].toUInt(),
                        it->second
                             )
               ;
         }
      }
   }
      
   // loop thru csv, keep non SOLs and marked SOLs as WATs

   csv new_csv;

   QString new_name = tmp_csv.name.replace( QRegExp( "\\.(pdb|PDB)$" ), "" );
   new_name += QString( "_WAT%1" ).arg( hydroradius ).replace( ".", "_" );

   new_csv.name     = new_name + ".pdb";
   new_csv.filename = new_name + ".pdb";

   new_csv.header.push_back("Model");
   new_csv.header.push_back("Chain");
   new_csv.header.push_back("Residue");
   new_csv.header.push_back("Residue Number");
   new_csv.header.push_back("Atom");
   new_csv.header.push_back("Atom Number");
   new_csv.header.push_back("Alt");
   new_csv.header.push_back("iC");
   new_csv.header.push_back("X");
   new_csv.header.push_back("Y");
   new_csv.header.push_back("Z");
   new_csv.header.push_back("Occ");
   new_csv.header.push_back("TF");
   new_csv.header.push_back("Ele");

   for ( int i = 0; i < (int) tmp_csv.data.size(); ++i ) {
      if ( !SOL_set.count( i ) ) {
         new_csv.data.push_back( tmp_csv.data[ i ] );
      } else {
         if ( sol2wat_csv_index.count( i ) ) {
            tmp_csv.data[ i ][ 2 ] = "WAT";
            if ( chosen_radius_for_SOLs.count( tmp_csv.data[ i ][ 3 ] ) &&
                 chosen_radius_for_SOLs[ tmp_csv.data[ i ][ 3 ] ].count( tmp_csv.data[ i ][ 5 ] ) ) {
               tmp_csv.data[ i ][ 12 ] = QString( "" ).sprintf( "%6.2f", chosen_radius_for_SOLs[ tmp_csv.data[ i ][ 3 ] ][ tmp_csv.data[ i ][ 5 ] ] );
               // QTextStream( stderr ) <<
               //    QString( us_tr( "WAT pdb Tf radius for WAT %1 OW %2 is %3 but chosen_radius is %4\n" ) )
               //    .arg( tmp_csv.data[ i ][ 3 ] )
               //    .arg( tmp_csv.data[ i ][ 5 ] )
               //    .arg( tmp_csv.data[ i ][ 12 ] )
               //    .arg( chosen_radius_for_SOLs[ tmp_csv.data[ i ][ 3 ] ][ tmp_csv.data[ i ][ 5 ] ] )
               //    ;
            } else {
               editor_msg( "red",
                           QString( us_tr( "Warning, missing radius for WAT %1 OW %2\n" ) )
                           .arg( tmp_csv.data[ i ][ 3 ] )
                           .arg( tmp_csv.data[ i ][ 5 ] )
                           );
               // QTextStream( stderr ) <<
               //    QString( us_tr( "Warning, missing radius for WAT %1 OW %2\n" ) )
               //    .arg( tmp_csv.data[ i ][ 3 ] )
               //    .arg( tmp_csv.data[ i ][ 5 ] )
               //    ;
            }
            new_csv.data.push_back( tmp_csv.data[ i ] );
         }
      }
   }

   // add keys
   csv_setup_keys( new_csv );

   // paste as lv2/csv2
   csv_clipboard = new_csv;
   csv2_paste_new();

   // get save file name for report
   {
      QString filename;
      if ( filepath.isEmpty() ) {
         QString use_dir = 
            ((US_Hydrodyn *)us_hydrodyn)->path_view_pdb.isEmpty() ?
            ((US_Hydrodyn *)us_hydrodyn)->somo_pdb_dir :
            ((US_Hydrodyn *)us_hydrodyn)->path_view_pdb;

         ((US_Hydrodyn *)us_hydrodyn)->select_from_directory_history( use_dir, this );

         QString fn = new_name.replace(QRegExp("\\.(pdb|PDB)$"),"") + "_sol2wat_report.txt";

         filename = QFileDialog::getSaveFileName( this , us_tr("Choose a filename to save the SOL->WAT report") , use_dir + "/" + fn , "TXT (*.txt *.TXT)" );

         if ( QFile::exists(filename) ) {
            filename = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( filename, 0, this );
         }

      } else {
         filename = reportpath + "/" + QFileInfo( filepath ).baseName() + "_sol2wat_detail.txt";
      }

      if ( !filename.isEmpty() ) {
      
         QFile f( filename );

         if ( !f.open( QIODevice::WriteOnly ) )
         {
            QMessageBox::warning( this, windowTitle(),
                                  QString(us_tr("Could not open %1 for writing!")).arg(filename) );
            return false;
         }

         QTextStream t( &f );

         t << report_header;

         for ( map < int, QString >::iterator it = report_SOLs.begin();
               it != report_SOLs.end();
               ++it ) {
            t << it->second;
         }
         f.close();
         ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( filename );
         editor_msg("black", QString("File %1 written\n").arg( filename ) );
      }
   }

   // get save file name for detail csv
   {
      QString filename;
      if ( filepath.isEmpty() ) {
         QString use_dir = 
            ((US_Hydrodyn *)us_hydrodyn)->path_view_pdb.isEmpty() ?
            ((US_Hydrodyn *)us_hydrodyn)->somo_pdb_dir :
            ((US_Hydrodyn *)us_hydrodyn)->path_view_pdb;

         ((US_Hydrodyn *)us_hydrodyn)->select_from_directory_history( use_dir, this );

         QString fn = new_name.replace(QRegExp("\\.(pdb|PDB)$"),"") + "_sol2wat_detail.csv";

         filename = QFileDialog::getSaveFileName( this , us_tr("Choose a filename to save the SOL->WAT detail csv") , use_dir + "/" + fn , "CSV (*.csv *.CSV)" );

         if ( QFile::exists(filename) ) {
            filename = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( filename, 0, this );
         }
      
      } else {
         filename = reportpath + "/" + QFileInfo( filepath ).baseName() + "_sol2wat_detail.csv";
      }

      if ( !filename.isEmpty() ) {
         QFile f( filename );

         if ( !f.open( QIODevice::WriteOnly ) )
         {
            QMessageBox::warning( this, windowTitle(),
                                  QString(us_tr("Could not open %1 for writing!")).arg(filename) );
            return false;
         }

         QTextStream t( &f );

         QString qs;
         for ( unsigned int i = 0; i < detail_csv.header.size(); i++ ) {
            qs += QString( "%1\"%2\"" ).arg( i ? "," : "" ).arg( detail_csv.header[ i ] );
         }
         t << qs << Qt::endl;
         for ( unsigned int i = 0; i < detail_csv.data.size(); ++i ) {
            qs = "";
            for ( unsigned int j = 0; j < detail_csv.data[i].size(); ++j ) {
               qs += QString( "%1%2" ).arg( j ? "," : "" ).arg( detail_csv.data[ i ][ j ] );
            }
            t << qs << Qt::endl;
         }
         f.close();
         ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( filename );
         editor_msg("black", QString("File %1 written\n").arg( filename ) );
      }
   }

   update_enables();
   return true;
}

void US_Hydrodyn_Pdb_Tool::compute_angle( QTreeWidget *lv )
{
   editor_msg( "blue", us_tr( "compute angle" ) );
   vector < QString > atom_names;
   vector < point > p;

   QTreeWidgetItemIterator it1( lv );
   while ( (*it1) ) 
   {
      QTreeWidgetItem *item1 = (*it1);
      if ( !item1->childCount() && is_selected( item1 ) )
      {
         atom_names.push_back( get_atom_name( item1 ) );
         point this_p;
         for ( unsigned int j = 0; j < 3; j++ )
         {
            this_p.axis[ j ] = item1->text( 3 + j ).toFloat();
         }
         p.push_back( this_p );
      }
      ++it1;
   }
   
   if ( atom_names.size() != 3 )
   {
      editor_msg( "red", 
                  QString( us_tr( "Error: compute angle: exactly 3 atoms must be selected, %1 are selected" ) )
                  .arg( atom_names.size() ) );
      return;
   }

   QString res;
   // base atom[ 0 ]
   {
      float a = acosf( ((US_Hydrodyn *)us_hydrodyn)->dot( ((US_Hydrodyn *)us_hydrodyn)->normal( ((US_Hydrodyn *)us_hydrodyn)->minus( p[ 1 ], p[ 0 ] ) ),
                                                          ((US_Hydrodyn *)us_hydrodyn)->normal( ((US_Hydrodyn *)us_hydrodyn)->minus( p[ 2 ], p[ 0 ] ) ) ) );
      
      res += 
         QString( us_tr( "Angle %1,%2,%3 = %4 or %5 degrees\n" ) )
         .arg( atom_names[ 1 ] )
         .arg( atom_names[ 0 ] )
         .arg( atom_names[ 2 ] )
         .arg( a )
         .arg( a * 180.0 / M_PI )
         ;
   }
   // base atom[ 1 ]
   {
      float a = acosf( ((US_Hydrodyn *)us_hydrodyn)->dot( ((US_Hydrodyn *)us_hydrodyn)->normal( ((US_Hydrodyn *)us_hydrodyn)->minus( p[ 0 ], p[ 1 ] ) ),
                                                          ((US_Hydrodyn *)us_hydrodyn)->normal( ((US_Hydrodyn *)us_hydrodyn)->minus( p[ 2 ], p[ 1 ] ) ) ) );
      res += 
         QString( us_tr( "Angle %1,%2,%3 = %4 or %5 degrees\n" ) )
         .arg( atom_names[ 0 ] )
         .arg( atom_names[ 1 ] )
         .arg( atom_names[ 2 ] )
         .arg( a )
         .arg( a * 180.0 / M_PI )
         ;
   }
   // base atom[ 2 ]
   {
      float a = acosf( ((US_Hydrodyn *)us_hydrodyn)->dot( ((US_Hydrodyn *)us_hydrodyn)->normal( ((US_Hydrodyn *)us_hydrodyn)->minus( p[ 0 ], p[ 2 ] ) ),
                                                          ((US_Hydrodyn *)us_hydrodyn)->normal( ((US_Hydrodyn *)us_hydrodyn)->minus( p[ 1 ], p[ 2 ] ) ) ) );
      res += 
         QString( us_tr( "Angle %1,%2,%3 = %4 or %5 degrees\n" ) )
         .arg( atom_names[ 0 ] )
         .arg( atom_names[ 2 ] )
         .arg( atom_names[ 1 ] )
         .arg( a )
         .arg( a * 180.0 / M_PI )
         ;
   }
   editor_msg( "black", res );
   editor_msg( "blue", us_tr( "compute angle done" ) );

}

QString US_Hydrodyn_Pdb_Tool::get_atom_name( QTreeWidgetItem *lvi )
{
   QString atom = 
      QString( "%1" ).arg( lvi->text( 0 ) )
      .replace( QRegExp( "^\\S+" ), "" )
      .replace( QRegExp( "\\S+$" ), "" )
      .trimmed();
   // cout << QString( "text0 is <%1> atom is <%2>\n" ).arg( lvi->text( 0 ) ).arg( atom );
   return atom;
}

QString US_Hydrodyn_Pdb_Tool::get_atom_number( QTreeWidgetItem *lvi )
{
   QString atom = 
      QString( "%1" ).arg( lvi->text( 0 ) )
      .trimmed()
      .replace( QRegExp( "^\\S+" ), "" )
      .trimmed();
   // cout << QString( "text0 is <%1> atom is <%2>\n" ).arg( lvi->text( 0 ) ).arg( atom );
   return atom;
}

QString US_Hydrodyn_Pdb_Tool::get_chain_id( QTreeWidgetItem *lvi )
{
   if ( US_Static::lvi_depth( lvi ) < 1 )
   {
      return "unknown";
   }
   while( US_Static::lvi_depth( lvi ) > 1 )
   {
      lvi = lvi->parent();
   }
   return lvi->text( 0 );
}

QString US_Hydrodyn_Pdb_Tool::get_model_id( QTreeWidgetItem *lvi )
{
   while( US_Static::lvi_depth( lvi ) > 0 )
   {
      lvi = lvi->parent();
   }
   return lvi->text( 0 );
}

QString US_Hydrodyn_Pdb_Tool::get_residue_name( QTreeWidgetItem *lvi )
{
   if ( US_Static::lvi_depth( lvi ) < 2 )
   {
      return "unknown";
   }
   while( US_Static::lvi_depth( lvi ) > 2 )
   {
      lvi = lvi->parent();
   }
   return QString( "%1" ).arg( lvi->text( 0 ) ).trimmed().replace( QRegExp( "\\s*\\d+$" ), "" );
}

QString US_Hydrodyn_Pdb_Tool::get_residue_number( QTreeWidgetItem *lvi )
{
   if ( US_Static::lvi_depth( lvi ) < 2 )
   {
      return "unknown";
   }
   while( US_Static::lvi_depth( lvi ) > 2 )
   {
      lvi = lvi->parent();
   }
   return QString( "%1" ).arg( lvi->text( 0 ) ).trimmed().replace( QRegExp( "^\\S+\\s*" ), "" ).trimmed();
}

QStringList US_Hydrodyn_Pdb_Tool::atom_set( QTreeWidget *lv )
{
   map < QString, bool > atoms;

   QTreeWidgetItemIterator it1( lv );
   while ( (*it1) ) 
   {
      QTreeWidgetItem *item1 = (*it1);
      if ( !item1->childCount() && is_selected( item1 ) )
      {
         atoms[ get_atom_name( item1 ) ] = true;
      }
      ++it1;
   }
   
   QStringList qsl;
   for ( map < QString, bool >::iterator it = atoms.begin();
         it != atoms.end();
         it++ )
   {
      qsl << it->first;
   }
   qsl.sort();
   return qsl;
}

QStringList US_Hydrodyn_Pdb_Tool::atom_sel_rasmol( QTreeWidget *lv )
{
   map < QString, bool > atoms;

   QTreeWidgetItemIterator it1( lv );
   QStringList qsl;

   while ( (*it1) ) 
   {
      QTreeWidgetItem *item1 = (*it1);
      if ( !item1->childCount() && is_selected( item1 ) )
      {
         qsl << QString( "select :%1 and atomno=%2\n" )
            .arg( get_chain_id( item1 ) )
            .arg( get_atom_number( item1 ) )
            ;
      }
      ++it1;
   }
   
   return qsl;
}

QStringList US_Hydrodyn_Pdb_Tool::model_set( QTreeWidget *lv )
{
   map < QString, bool > models;

   QTreeWidgetItemIterator it1( lv );
   while ( (*it1) ) 
   {
      QTreeWidgetItem *item1 = (*it1);
      if ( !US_Static::lvi_depth( item1 ) && is_selected( item1 ) )
      {
         models[ item1->text(0) ] = true;
      }
      ++it1;
   }
   
   QStringList qsl;
   for ( map < QString, bool >::iterator it = models.begin();
         it != models.end();
         it++ )
   {
      qsl << it->first;
   }
   qsl.sort();
   return qsl;
}

QStringList US_Hydrodyn_Pdb_Tool::chain_set( QTreeWidget *lv )
{
   map < QString, bool > chains;

   QTreeWidgetItemIterator it1( lv );
   while ( (*it1) ) 
   {
      QTreeWidgetItem *item1 = (*it1);
      if ( US_Static::lvi_depth( item1 ) == 1 && is_selected( item1 ) )
      {
         chains[ item1->text(0) ] = true;
      }
      ++it1;
   }
   
   QStringList qsl;
   for ( map < QString, bool >::iterator it = chains.begin();
         it != chains.end();
         it++ )
   {
      qsl << it->first;
   }
   qsl.sort();
   return qsl;
}

void US_Hydrodyn_Pdb_Tool::csv_sel_nearest_residues()
{
   sel_nearest_residues( lv_csv );
}


void US_Hydrodyn_Pdb_Tool::csv_clash_report()
{
   distances( lv_csv );
}

void US_Hydrodyn_Pdb_Tool::csv_angle()
{
   compute_angle( lv_csv );
}

void US_Hydrodyn_Pdb_Tool::csv_sol2wat()
{
   pdb_sel_count  counts            = count_selected( lv_csv );

   if ( !counts.SOLs ||
        QMessageBox::question(this, 
                              us_tr( "US-SOMO: PDB Editor : Sol2wat " ),
                              QString( us_tr( "What do you want to process?" ) ),
                              us_tr( "A &single file that is already loaded" ),
                              us_tr( "A &trajectory" ),
                              QString(),
                              0,
                              1
                              ) == 1 )
   {
      sol2wat_traj( lv_csv );
   } else {
      sol2wat( lv_csv );
   }
}

void US_Hydrodyn_Pdb_Tool::sol2wat_traj( QTreeWidget *lv ) {
   // select a directory
   
   QString use_dir = QDir::currentPath();
   bool not_ok = ((US_Hydrodyn  *)us_hydrodyn)->select_from_directory_history( use_dir, this );
   // raise();

   QStringList filenames;

   QString dirname = use_dir;
   if ( !not_ok ) {
      QDir up_one( use_dir );
      up_one.cdUp();
      dirname = QFileDialog::getExistingDirectory( this
                                                   ,"Add all .pdb files in directory"
                                                   ,up_one.canonicalPath()
                                                   ,QFileDialog::ShowDirsOnly );
   }
   if ( !dirname.isEmpty() ) {
      QDir dir( dirname );
      QStringList filters;
      filters << "*.pdb"; 
      filenames = dir.entryList( filters ).replaceInStrings( QRegExp( "^" ), dirname + "/" );
   } else {
      return;
   }

   if ( filenames.isEmpty() ) {
      return;
   }

   // optionally decimate

   if ( filenames.size() > 5 ) {
      bool ok;
      unsigned int decimate = ( unsigned int )US_Static::getInteger(
                                                                    "US-SOMO: PDB Editor : Sol2Wat", 
                                                                    QString( us_tr( "You have %1 files, enter the decimation value\n(1 = no decimation, 2 = every second, etc:" ) )
                                                                    .arg( filenames.size() )
                                                                    ,
                                                                    1,
                                                                    1, 
                                                                    filenames.size(),
                                                                    1,
                                                                    &ok, 
                                                                    this );
      if ( !ok ) {
         return;
      }

      if ( decimate > 1 ) {
         QStringList new_filenames;
         for ( int i = 0; i < (int) filenames.size(); i += decimate ) {
            new_filenames.push_back( filenames[ i ] );
         }
         filenames = new_filenames;
      }
   }
   
   // get parameters

   bool ok;
   double theo_hydroradius = pow( ( 3.0/( 4.0 * M_PI ) ) * ((US_Hydrodyn *)us_hydrodyn)->misc.hydrovol, 1.0 / 3.0 );
   
   double hydroradius = US_Static::getDouble(
                                             us_tr( "US-SOMO: PDB editor : SOL->WAT" ) ,
                                             QString( us_tr( "Enter a radius in Angstrom for water.\n"
                                                             "This will be used as a threshold to determine if the SOL is converted to WAT.\n"
                                                             "The radius will be added to the vdW radius of the nearest non-water and\n"
                                                             "if the distance between centers is less than or equal to the sum of the\n"
                                                             "radii, the water will be kept.\n"
                                                             ) )
                                             ,
                                             theo_hydroradius,
                                             0.0001,
                                             10,
                                             4,
                                             &ok, 
                                             this
                                             );

   if ( !ok ) {
      return;
   }

   // process all pdbs and save

   qDebug() << "now process all of them\n";
   hydration_summary         .clear();
   hydration_summary_res     .clear();
   hydration_summary_res_atom.clear();

   QString reportpath = QFileInfo( filenames[ 0 ] ).path() + QString( "/Hr_%1" ).arg( hydroradius ).replace( ".", "_" );
   qDebug() << "report dir path " << reportpath;
   {
      QDir tmp_dir;
      if ( !tmp_dir.mkpath( reportpath ) ) {
         editor_msg( "red", QString( us_tr( "Could not create report directory %1" ) ).arg( reportpath ) );
      }
   }

   QRegularExpression re( "_F_(\\d+).pdb$" );
   for ( int i = 0; i < (int) filenames.size(); ++i ) {
      QRegularExpressionMatch match = re.match( filenames[ i ] );
      if ( match.hasMatch() ) {
         qDebug() << QString( "file %1 has match value %2" ).arg( filenames[ i ] ).arg( match.captured( 1 ).toUInt() );

         load( lv, filenames[ i ] );
         lv->selectAll();
         pdb_sel_count  counts = count_selected( lv );
         if ( !counts.SOLs ) {
            editor_msg( "red", QString( us_tr( "Warning: no SOLs defined for file '%1', skipped" ) ).arg( filenames[ i ] ) );
         } else {
            if ( sol2wat( lv, hydroradius, filenames[ i ], match.captured( 1 ).toUInt(), reportpath ) ) {
               save_csv( lv_csv2, reportpath + "/" + QFileInfo( filenames[ i ] ).baseName() + "_Hr_" + QString( "%1" ).arg( hydroradius ).replace( ".", "_" ) + ".pdb" );
            }
         }
      }
   }      
   // summary report

   // map < int, map < int, vector < hydration_info > > > hydration_summary; // atom, frame, hydration_info
   // loop thru lv_csv, process all frames of each, produce summary report
   // global info later

   // find max rank (maximum number of waters)

   int max_rank = 0;

   for ( map < int, map < int, vector < hydration_info > > > ::iterator it1 = hydration_summary.begin();
         it1 != hydration_summary.end();
         ++it1 ) {
      int csvj = it1->first;
      if ( !hydration_header.count( csvj ) ) {
         editor_msg( "red", QString( us_tr( "Internal Error: no header info for '%1', quitting!" ) ).arg( csvj ) );
         return;
      }
   
      for ( map < int, vector < hydration_info > >::iterator it2 = hydration_summary[ csvj ].begin();
            it2 != hydration_summary[ csvj ].end();
            ++it2 ) {
         if ( max_rank < (int) it2->second.size() ) {
            max_rank = (int) it2->second.size();
         }
      }
   }

   qDebug() << QString( "max rank found %1" ).arg( max_rank );
   
   int max_rank_res = 0;

   for ( map < QString, map < QString, map < int, vector < hydration_info > > > >::iterator it1 = hydration_summary_res.begin();
         it1 != hydration_summary_res.end();
         ++it1 ) {
      QString rname = it1->first;
      for ( map < QString, map < int, vector < hydration_info > > >::iterator it2 = it1->second.begin();
            it2 != it1->second.end();
            ++it2 ) {
         for ( map < int, vector < hydration_info > >::iterator it3 = it2->second.begin();
               it3 != it2->second.end();
               ++it3 ) {
            if ( max_rank_res < (int) it3->second.size() ) {
               max_rank_res = (int) it3->second.size();
            }
         } // frame
      } // rnum
   } // rname

   qDebug() << QString( "max rank res found %1" ).arg( max_rank_res );

   int max_rank_res_atom = 0;

   for ( map < QString, map < QString, map < QString, map < int, vector < hydration_info > > > > >::iterator it1 = hydration_summary_res_atom.begin();
         it1 != hydration_summary_res_atom.end();
         ++it1 ) {
      QString rname = it1->first;
      for ( map < QString, map < QString, map < int, vector < hydration_info > > > >::iterator it2 = it1->second.begin();
            it2 != it1->second.end();
            ++it2 ) {
         for ( map < QString, map < int, vector < hydration_info > > >::iterator it3 = it2->second.begin();
               it3 != it2->second.end();
               ++it3 ) {
            for ( map < int, vector < hydration_info > >::iterator it4 = it3->second.begin();
                  it4 != it3->second.end();
                  ++it4 ) {
               if ( max_rank_res_atom < (int) it4->second.size() ) {
                  max_rank_res_atom = (int) it4->second.size();
               }
            } // frame
         } // rnum
      } // aname 
   } // rname

   qDebug() << QString( "max rank res atom found %1" ).arg( max_rank_res_atom );
   
   csv header_csv;
   header_csv.name     = "sol2wat_header";
   header_csv.filename = "sol2wat_header";
   header_csv.header.push_back( "PDB name" );
   header_csv.header.push_back( "Trajectory file" );
   header_csv.header.push_back( "Duration" );
   header_csv.header.push_back( "Frame interval" );
   header_csv.header.push_back( "Number of frames" );
   header_csv.header.push_back( "Cutoff" );
   header_csv.header.push_back( "Max number of water per solute atom" );

   csv header_res_csv;
   header_res_csv.name     = "sol2wat_header_res";
   header_res_csv.filename = "sol2wat_header_res";
   header_res_csv.header.push_back( "PDB name" );
   header_res_csv.header.push_back( "Trajectory file" );
   header_res_csv.header.push_back( "Duration" );
   header_res_csv.header.push_back( "Frame interval" );
   header_res_csv.header.push_back( "Number of frames" );
   header_res_csv.header.push_back( "Cutoff" );
   header_res_csv.header.push_back( "Max number of water per residue" );

   csv header_res_atom_csv;
   header_res_atom_csv.name     = "sol2wat_header_res_atom";
   header_res_atom_csv.filename = "sol2wat_header_res_atom";
   header_res_atom_csv.header.push_back( "PDB name" );
   header_res_atom_csv.header.push_back( "Trajectory file" );
   header_res_atom_csv.header.push_back( "Duration" );
   header_res_atom_csv.header.push_back( "Frame interval" );
   header_res_atom_csv.header.push_back( "Number of frames" );
   header_res_atom_csv.header.push_back( "Cutoff" );
   header_res_atom_csv.header.push_back( "Max number of water per solute atom" );
   
   QString base_name = QFileInfo( filenames[ 0 ] ).baseName().replace( QRegularExpression( "_F_\\d+$" ), "" );
                       
   // header info
   {
      vector < QString > tmp_data;
      tmp_data.push_back( QFileInfo( filenames[ 0 ] ).path() + "/" + base_name + ".gro" );
      tmp_data.push_back( QFileInfo( filenames[ 0 ] ).path() + "/" + base_name + ".xtc" );
      tmp_data.push_back( "?" );
      tmp_data.push_back( "?" );
      tmp_data.push_back( QString( "%1" ).arg( filenames.size() ) );
      tmp_data.push_back( QString( "%1" ).arg( hydroradius ) );
      tmp_data.push_back( QString( "%1" ).arg( max_rank ) );
      header_csv.data.push_back( tmp_data );
   }

   // header res info
   {
      vector < QString > tmp_data;
      tmp_data.push_back( QFileInfo( filenames[ 0 ] ).path() + "/" + base_name + ".gro" );
      tmp_data.push_back( QFileInfo( filenames[ 0 ] ).path() + "/" + base_name + ".xtc" );
      tmp_data.push_back( "?" );
      tmp_data.push_back( "?" );
      tmp_data.push_back( QString( "%1" ).arg( filenames.size() ) );
      tmp_data.push_back( QString( "%1" ).arg( hydroradius ) );
      tmp_data.push_back( QString( "%1" ).arg( max_rank_res ) );
      header_res_csv.data.push_back( tmp_data );
   }

   // header res atom info
   {
      vector < QString > tmp_data;
      tmp_data.push_back( QFileInfo( filenames[ 0 ] ).path() + "/" + base_name + ".gro" );
      tmp_data.push_back( QFileInfo( filenames[ 0 ] ).path() + "/" + base_name + ".xtc" );
      tmp_data.push_back( "?" );
      tmp_data.push_back( "?" );
      tmp_data.push_back( QString( "%1" ).arg( filenames.size() ) );
      tmp_data.push_back( QString( "%1" ).arg( hydroradius ) );
      tmp_data.push_back( QString( "%1" ).arg( max_rank_res_atom ) );
      header_res_atom_csv.data.push_back( tmp_data );
   }
   
   QString name_prefix =
      base_name
      + "_Hr_" + QString( "%1" ).arg( hydroradius ).replace( ".", "_" )
      ;
   
   QString summary_name           = name_prefix + "_summary";
   QString summary_res_name       = name_prefix + "_summary_res";
   QString summary_res_atom_name  = name_prefix + "_summary_res_atom";
   QString bin_name               = name_prefix + "_bin_r";
   QString bind_name              = name_prefix + "_bin_d";
   QString binda_name             = name_prefix + "_bin_d_a";

   csv bin_csv;
   bin_csv.name     = bin_name;
   bin_csv.filename = reportpath + "/" + bin_name + ".csv";
   bin_csv.header.push_back( "Radius" );
   
   for ( int i = 1; i <= max_rank; ++i ) {
      bin_csv.header.push_back( QString( "Rank %1 frequency" ).arg( i ) );
   }

   csv bind_csv;
   bind_csv.name     = bind_name;
   bind_csv.filename = reportpath + "/" + bind_name + ".csv";
   bind_csv.header.push_back( "Distance" );
   
   for ( int i = 1; i <= max_rank; ++i ) {
      bind_csv.header.push_back( QString( "Rank %1 frequency" ).arg( i ) );
   }

   csv binda_csv;
   binda_csv.name     = binda_name;
   binda_csv.filename = reportpath + "/" + binda_name + ".csv";
   binda_csv.header.push_back( "Distance" );
   QStringList binda_atoms;
   binda_atoms
      << "C"
      << "O"
      << "N"
      ;
   
   for ( int i = 0; i < binda_atoms.size(); ++i ) {
      binda_csv.header.push_back( QString( "Atom %1" ).arg( binda_atoms[ i ] ) );
   }
   
   csv summary_csv;
   csv summary_res_csv;
   csv summary_res_atom_csv;

   summary_csv.name     = summary_name;
   summary_csv.filename = reportpath + "/" + summary_name + ".csv";

   summary_csv.header.push_back( "Solute residue name" );
   summary_csv.header.push_back( "Solute residue number" );
   summary_csv.header.push_back( "Solute atom name" );
   summary_csv.header.push_back( "Solute atom number" );
   summary_csv.header.push_back( "Solute vdW radius" );

   summary_csv.header.push_back( "Minimum number of waters in cutoff" );
   summary_csv.header.push_back( "Maximum number of waters in cutoff" );
   summary_csv.header.push_back( "Average number of waters in cutoff" );
   summary_csv.header.push_back( "S.D. of number of waters in cutoff" );
   summary_csv.header.push_back( "Skew of number of waters in cutoff" );

   summary_res_csv.name     = summary_res_name;
   summary_res_csv.filename = reportpath + "/" + summary_res_name + ".csv";

   summary_res_csv.header.push_back( "Solute residue name" );
   summary_res_csv.header.push_back( "Number of occurrences" );

   summary_res_csv.header.push_back( "Minimum number of waters in cutoff" );
   summary_res_csv.header.push_back( "Maximum number of waters in cutoff" );
   summary_res_csv.header.push_back( "Average number of waters in cutoff" );
   summary_res_csv.header.push_back( "S.D. of number of waters in cutoff" );
   summary_res_csv.header.push_back( "Skew of number of waters in cutoff" );

   summary_res_atom_csv.name     = summary_res_atom_name;
   summary_res_atom_csv.filename = reportpath + "/" + summary_res_atom_name + ".csv";

   summary_res_atom_csv.header.push_back( "Solute residue name" );
   summary_res_atom_csv.header.push_back( "Solute atom name" );
   summary_res_atom_csv.header.push_back( "Number of occurrences" );

   summary_res_atom_csv.header.push_back( "Minimum number of waters in cutoff" );
   summary_res_atom_csv.header.push_back( "Maximum number of waters in cutoff" );
   summary_res_atom_csv.header.push_back( "Average number of waters in cutoff" );
   summary_res_atom_csv.header.push_back( "S.D. of number of waters in cutoff" );
   summary_res_atom_csv.header.push_back( "Skew of number of waters in cutoff" );
   
   for ( int i = 1; i <= max_rank; ++i ) {
      summary_csv.header.push_back( QString( "Minimum distance to rank %1 waters" ).arg( i ) );
      summary_csv.header.push_back( QString( "Maximum distance to rank %1 waters" ).arg( i ) );
      summary_csv.header.push_back( QString( "Average distance to rank %1 waters" ).arg( i ) );
      summary_csv.header.push_back( QString( "S.D. of distance to rank %1 waters" ).arg( i ) );
      summary_csv.header.push_back( QString( "Skew of distance to rank %1 waters" ).arg( i ) );

      summary_csv.header.push_back( QString( "Minimum radius of rank %1 waters" ).arg( i ) );
      summary_csv.header.push_back( QString( "Maximum radius of rank %1 waters" ).arg( i ) );
      summary_csv.header.push_back( QString( "Average radius of rank %1 waters" ).arg( i ) );
      summary_csv.header.push_back( QString( "S.D. of radius of rank %1 waters" ).arg( i ) );
      summary_csv.header.push_back( QString( "Skew of radius of rank %1 waters" ).arg( i ) );

   }

   for ( int i = 1; i <= max_rank_res; ++i ) {
      summary_res_csv.header.push_back( QString( "Minimum distance to rank %1 waters" ).arg( i ) );
      summary_res_csv.header.push_back( QString( "Maximum distance to rank %1 waters" ).arg( i ) );
      summary_res_csv.header.push_back( QString( "Average distance to rank %1 waters" ).arg( i ) );
      summary_res_csv.header.push_back( QString( "S.D. of distance to rank %1 waters" ).arg( i ) );
      summary_res_csv.header.push_back( QString( "Skew of distance to rank %1 waters" ).arg( i ) );

      summary_res_csv.header.push_back( QString( "Minimum radius of rank %1 waters" ).arg( i ) );
      summary_res_csv.header.push_back( QString( "Maximum radius of rank %1 waters" ).arg( i ) );
      summary_res_csv.header.push_back( QString( "Average radius of rank %1 waters" ).arg( i ) );
      summary_res_csv.header.push_back( QString( "S.D. of radius of rank %1 waters" ).arg( i ) );
      summary_res_csv.header.push_back( QString( "Skew of radius of rank %1 waters" ).arg( i ) );
   }

   for ( int i = 1; i <= max_rank_res_atom; ++i ) {
      summary_res_atom_csv.header.push_back( QString( "Minimum distance to rank %1 waters" ).arg( i ) );
      summary_res_atom_csv.header.push_back( QString( "Maximum distance to rank %1 waters" ).arg( i ) );
      summary_res_atom_csv.header.push_back( QString( "Average distance to rank %1 waters" ).arg( i ) );
      summary_res_atom_csv.header.push_back( QString( "S.D. of distance to rank %1 waters" ).arg( i ) );
      summary_res_atom_csv.header.push_back( QString( "Skew of distance to rank %1 waters" ).arg( i ) );

      summary_res_atom_csv.header.push_back( QString( "Minimum radius of rank %1 waters" ).arg( i ) );
      summary_res_atom_csv.header.push_back( QString( "Maximum radius of rank %1 waters" ).arg( i ) );
      summary_res_atom_csv.header.push_back( QString( "Average radius of rank %1 waters" ).arg( i ) );
      summary_res_atom_csv.header.push_back( QString( "S.D. of radius of rank %1 waters" ).arg( i ) );
      summary_res_atom_csv.header.push_back( QString( "Skew of radius of rank %1 waters" ).arg( i ) );
   }

   QStringList stat_headers;
   stat_headers
      << "w_min"
      << "w_max"
      << "w_avg"
      << "w_sd"
      << "w_skew"
      ;      

   for ( int rank = 0; rank < max_rank; ++rank ) {
      stat_headers
         << QString( "d_min %1" ).arg( rank )
         << QString( "d_max %1" ).arg( rank )
         << QString( "d_avg %1" ).arg( rank )
         << QString( "d_sd %1" ).arg( rank )
         << QString( "d_skew %1" ).arg( rank )
         << QString( "r_min %1" ).arg( rank )
         << QString( "r_max %1" ).arg( rank )
         << QString( "r_avg %1" ).arg( rank )
         << QString( "r_sd %1" ).arg( rank )
         << QString( "r_skew %1" ).arg( rank )
         ;
   }

   map < double, map < int, int > > bins2; // radius, rank, frequency
   map < double, map < int, int > > bins3; // radius, rank, frequency
   map < double, map < int, int > > binds2; // distance, rank, frequency
   map < double, map < int, int > > binds3; // distance, rank, frequency

   map < double, map < QString, int > > bindas2; // distance, atom, frequency
   map < double, map < QString, int > > bindas3; // distance, atom, frequency

   // by residue info
   
   // map < int, map < int, vector < hydration_info > > > hydration_summary_res; // residue#, frame, hydration_info
   // map < QString, map < QString, map < int, vector < hydration_info > > > >
   //                                                                             hydration_summary_res;
   //                                                                            // rname, rnum, frame, hydration_info

   for ( map < QString, map < QString, map < int, vector < hydration_info > > > >::iterator it1 = hydration_summary_res.begin();
         it1 != hydration_summary_res.end();
         ++it1 ) {
      QString rname = it1->first;
      vector < QString > tmp_data;

      tmp_data.push_back( rname );
      tmp_data.push_back( QString( "%1" ).arg( it1->second.size() ) );
      
      map < QString, US_Stat > us_stats;
      
      for ( map < QString, map < int, vector < hydration_info > > >::iterator it2 = it1->second.begin();
            it2 != it1->second.end();
            ++it2 ) {
         for ( map < int, vector < hydration_info > >::iterator it3 = it2->second.begin();
               it3 != it2->second.end();
               ++it3 ) {
            us_stats[ "w" ].add_point( (double) it3->second.size() );
            for ( int this_max_rank = 1; this_max_rank <= max_rank_res; ++this_max_rank ) {
               QString qsrank = QString( " %1" ).arg( this_max_rank );
               for ( int rank = 0; rank < (int) this_max_rank && rank < (int) it3->second.size(); ++rank ) {
                  us_stats[ "d" + qsrank ].add_point( it3->second[ rank ].d );
                  us_stats[ "r" + qsrank ].add_point( it3->second[ rank ].r );
               }
            } // this_max_rank
         } // frames
      } // rnum

      tmp_data.push_back( QString( "%1" ).arg( us_stats[ "w" ].min() ) );
      tmp_data.push_back( QString( "%1" ).arg( us_stats[ "w" ].max() ) );
      tmp_data.push_back( QString( "%1" ).arg( us_stats[ "w" ].avg() ) );
      tmp_data.push_back( QString( "%1" ).arg( us_stats[ "w" ].sd() ) );
      tmp_data.push_back( QString( "%1" ).arg( us_stats[ "w" ].skew() ) );

      for ( int rank = 1; rank <= max_rank_res; ++rank ) {
         QString qsrank = QString( " %1" ).arg( rank );
         tmp_data.push_back( QString( "%1" ).arg( us_stats[ "d" + qsrank ].min()  ) );
         tmp_data.push_back( QString( "%1" ).arg( us_stats[ "d" + qsrank ].max()  ) );
         tmp_data.push_back( QString( "%1" ).arg( us_stats[ "d" + qsrank ].avg()  ) );
         tmp_data.push_back( QString( "%1" ).arg( us_stats[ "d" + qsrank ].sd()   ) );
         tmp_data.push_back( QString( "%1" ).arg( us_stats[ "d" + qsrank ].skew() ) );

         tmp_data.push_back( QString( "%1" ).arg( us_stats[ "r" + qsrank ].min()  ) );
         tmp_data.push_back( QString( "%1" ).arg( us_stats[ "r" + qsrank ].max()  ) );
         tmp_data.push_back( QString( "%1" ).arg( us_stats[ "r" + qsrank ].avg()  ) );
         tmp_data.push_back( QString( "%1" ).arg( us_stats[ "r" + qsrank ].sd()   ) );
         tmp_data.push_back( QString( "%1" ).arg( us_stats[ "r" + qsrank ].skew() ) );
      }

      summary_res_csv.data.push_back( tmp_data );      
   } // rname

   // by residue atom info
   
   //   map < QString, map < QString, map < QString, map < int, vector < hydration_info > > > > >
   //                                                                               hydration_summary_res_atom;
   //                                                                               // rname, aname, rnum, frame, hydration_info


   for ( map < QString, map < QString, map < QString, map < int, vector < hydration_info > > > > >::iterator it1 = hydration_summary_res_atom.begin();
         it1 != hydration_summary_res_atom.end();
         ++it1 ) {
      QString rname = it1->first;
      for ( map < QString, map < QString, map < int, vector < hydration_info > > > >::iterator it2 = it1->second.begin();
            it2 != it1->second.end();
            ++it2 ) {
         QString aname = it2->first;

         vector < QString > tmp_data;

         tmp_data.push_back( rname );
         tmp_data.push_back( aname );
         tmp_data.push_back( QString( "%1" ).arg( it2->second.size() ) );
      
         map < QString, US_Stat > us_stats;
      
         for ( map < QString, map < int, vector < hydration_info > > >::iterator it3 = it2->second.begin();
               it3 != it2->second.end();
               ++it3 ) {
            for ( map < int, vector < hydration_info > >::iterator it4 = it3->second.begin();
                  it4 != it3->second.end();
                  ++it4 ) {
               us_stats[ "w" ].add_point( (double) it4->second.size() );
               for ( int this_max_rank = 1; this_max_rank <= max_rank_res_atom; ++this_max_rank ) {
                  QString qsrank = QString( " %1" ).arg( this_max_rank );
                  for ( int rank = 0; rank < (int) this_max_rank && rank < (int) it4->second.size(); ++rank ) {
                     us_stats[ "d" + qsrank ].add_point( it4->second[ rank ].d );
                     us_stats[ "r" + qsrank ].add_point( it4->second[ rank ].r );
                  }
               } // this_max_rank
            } // frames
         } // rnum

         tmp_data.push_back( QString( "%1" ).arg( us_stats[ "w" ].min() ) );
         tmp_data.push_back( QString( "%1" ).arg( us_stats[ "w" ].max() ) );
         tmp_data.push_back( QString( "%1" ).arg( us_stats[ "w" ].avg() ) );
         tmp_data.push_back( QString( "%1" ).arg( us_stats[ "w" ].sd() ) );
         tmp_data.push_back( QString( "%1" ).arg( us_stats[ "w" ].skew() ) );

         for ( int rank = 1; rank <= max_rank_res_atom; ++rank ) {
            QString qsrank = QString( " %1" ).arg( rank );
            tmp_data.push_back( QString( "%1" ).arg( us_stats[ "d" + qsrank ].min()  ) );
            tmp_data.push_back( QString( "%1" ).arg( us_stats[ "d" + qsrank ].max()  ) );
            tmp_data.push_back( QString( "%1" ).arg( us_stats[ "d" + qsrank ].avg()  ) );
            tmp_data.push_back( QString( "%1" ).arg( us_stats[ "d" + qsrank ].sd()   ) );
            tmp_data.push_back( QString( "%1" ).arg( us_stats[ "d" + qsrank ].skew() ) );

            tmp_data.push_back( QString( "%1" ).arg( us_stats[ "r" + qsrank ].min()  ) );
            tmp_data.push_back( QString( "%1" ).arg( us_stats[ "r" + qsrank ].max()  ) );
            tmp_data.push_back( QString( "%1" ).arg( us_stats[ "r" + qsrank ].avg()  ) );
            tmp_data.push_back( QString( "%1" ).arg( us_stats[ "r" + qsrank ].sd()   ) );
            tmp_data.push_back( QString( "%1" ).arg( us_stats[ "r" + qsrank ].skew() ) );
         }

         summary_res_atom_csv.data.push_back( tmp_data );      
      } // aname
   } // rname
   
   for ( map < int, map < int, vector < hydration_info > > > ::iterator it1 = hydration_summary.begin();
         it1 != hydration_summary.end();
         ++it1 ) {
      int csvj = it1->first;
      vector < QString > tmp_data;

      tmp_data.push_back( hydration_header[ csvj ].rname );
      tmp_data.push_back( hydration_header[ csvj ].rnum );
      tmp_data.push_back( hydration_header[ csvj ].aname );
      tmp_data.push_back( hydration_header[ csvj ].anum );
      tmp_data.push_back( QString( "%1" ).arg( hydration_header[ csvj ].vdw ) );

      QString rname = hydration_header[ csvj ].rname;
      QString aname = hydration_header[ csvj ].aname;
      QString atom  = aname.trimmed().left( 1 );

      // now compute avg, min, max, sd, skew of each

      map < QString, US_Stat > us_stats;

      // for each frame
      for ( map < int, vector < hydration_info > >::iterator it2 = hydration_summary[ csvj ].begin();
            it2 != hydration_summary[ csvj ].end();
            ++it2 ) {
         us_stats[ "w" ].add_point( (double) it2->second.size() );

         for ( int this_max_rank = 1; this_max_rank <= max_rank; ++this_max_rank ) {
            QString qsrank = QString( " %1" ).arg( this_max_rank );
            for ( int rank = 0; rank < (int) this_max_rank && rank < (int) it2->second.size(); ++rank ) {
               us_stats[ "d" + qsrank ].add_point( it2->second[ rank ].d );
               us_stats[ "r" + qsrank ].add_point( it2->second[ rank ].r );

               bins2  [ 0.01  * round( it2->second[ rank ].r * 100  ) ][ this_max_rank - 1 ]++;
               bins3  [ 0.001 * round( it2->second[ rank ].r * 1000 ) ][ this_max_rank - 1 ]++;
               binds2 [ 0.01  * round( it2->second[ rank ].d * 100  ) ][ this_max_rank - 1 ]++;
               binds3 [ 0.001 * round( it2->second[ rank ].d * 1000 ) ][ this_max_rank - 1 ]++;
            }
         } // this_max_rank

         // just rank 1 for atom distance binning
         if ( it2->second.size() ) {
            bindas2[ 0.01  * round( it2->second[ 0 ].d * 100  ) ][ atom ]++;
            bindas3[ 0.001 * round( it2->second[ 0 ].d * 1000 ) ][ atom ]++;
         }
      } // frame
      if ( us_stats[ "w" ].count() < (int) filenames.size() ) {
         vector < double > x( (int) filenames.size() - us_stats[ "w" ].count(), 0e0 );
         us_stats[ "w" ].add_points( x );
      }

      tmp_data.push_back( QString( "%1" ).arg( us_stats[ "w" ].min()  ) );
      tmp_data.push_back( QString( "%1" ).arg( us_stats[ "w" ].max()  ) );
      tmp_data.push_back( QString( "%1" ).arg( us_stats[ "w" ].avg()  ) );
      tmp_data.push_back( QString( "%1" ).arg( us_stats[ "w" ].sd()   ) );
      tmp_data.push_back( QString( "%1" ).arg( us_stats[ "w" ].skew() ) );

      for ( int rank = 1; rank <= max_rank; ++rank ) {
         QString qsrank = QString( " %1" ).arg( rank );

         tmp_data.push_back( QString( "%1" ).arg( us_stats[ "d" + qsrank ].min()  ) );
         tmp_data.push_back( QString( "%1" ).arg( us_stats[ "d" + qsrank ].max()  ) );
         tmp_data.push_back( QString( "%1" ).arg( us_stats[ "d" + qsrank ].avg()  ) );
         tmp_data.push_back( QString( "%1" ).arg( us_stats[ "d" + qsrank ].sd()   ) );
         tmp_data.push_back( QString( "%1" ).arg( us_stats[ "d" + qsrank ].skew() ) );

         tmp_data.push_back( QString( "%1" ).arg( us_stats[ "r" + qsrank ].min()  ) );
         tmp_data.push_back( QString( "%1" ).arg( us_stats[ "r" + qsrank ].max()  ) );
         tmp_data.push_back( QString( "%1" ).arg( us_stats[ "r" + qsrank ].avg()  ) );
         tmp_data.push_back( QString( "%1" ).arg( us_stats[ "r" + qsrank ].sd()   ) );
         tmp_data.push_back( QString( "%1" ).arg( us_stats[ "r" + qsrank ].skew() ) );
      }

      summary_csv.data.push_back( tmp_data );
   }

   {
      for ( map < double, map < int, int > >::iterator it = bins2.begin();
            it != bins2.end();
            ++it ) {
         vector < QString > tmp_data;
         tmp_data.push_back( QString( "%1" ).arg( it->first ) );
         for ( int rank = 0; rank < max_rank; ++rank ) {
            if ( it->second.count( rank ) ) {
               tmp_data.push_back( QString( "%1" ).arg( it->second[ rank ] ) );
            } else {
               tmp_data.push_back( "0" );
            }
         }
         bin_csv.data.push_back( tmp_data );
      }

      {
         vector < QString > tmp_data;
         bin_csv.data.push_back( tmp_data );
      }

      for ( map < double, map < int, int > >::iterator it = bins3.begin();
            it != bins3.end();
            ++it ) {
         vector < QString > tmp_data;
         tmp_data.push_back( QString( "%1" ).arg( it->first ) );
         for ( int rank = 0; rank < max_rank; ++rank ) {
            if ( it->second.count( rank ) ) {
               tmp_data.push_back( QString( "%1" ).arg( it->second[ rank ] ) );
            } else {
               tmp_data.push_back( "0" );
            }
         }
         bin_csv.data.push_back( tmp_data );
      }
   }

   {
      for ( map < double, map < int, int > >::iterator it = binds2.begin();
            it != binds2.end();
            ++it ) {
         vector < QString > tmp_data;
         tmp_data.push_back( QString( "%1" ).arg( it->first ) );
         for ( int rank = 0; rank < max_rank; ++rank ) {
            if ( it->second.count( rank ) ) {
               tmp_data.push_back( QString( "%1" ).arg( it->second[ rank ] ) );
            } else {
               tmp_data.push_back( "0" );
            }
         }
         bind_csv.data.push_back( tmp_data );
      }

      {
         vector < QString > tmp_data;
         bind_csv.data.push_back( tmp_data );
      }

      for ( map < double, map < int, int > >::iterator it = binds3.begin();
            it != binds3.end();
            ++it ) {
         vector < QString > tmp_data;
         tmp_data.push_back( QString( "%1" ).arg( it->first ) );
         for ( int rank = 0; rank < max_rank; ++rank ) {
            if ( it->second.count( rank ) ) {
               tmp_data.push_back( QString( "%1" ).arg( it->second[ rank ] ) );
            } else {
               tmp_data.push_back( "0" );
            }
         }
         bind_csv.data.push_back( tmp_data );
      }
   }

   {
      for ( map < double, map < QString, int > >::iterator it = bindas2.begin();
            it != bindas2.end();
            ++it ) {
         vector < QString > tmp_data;
         tmp_data.push_back( QString( "%1" ).arg( it->first ) );
         for ( int i = 0; i < (int) binda_atoms.size(); ++i ) {
            if ( it->second.count( binda_atoms[ i ] ) ) {
               tmp_data.push_back( QString( "%1" ).arg( it->second[ binda_atoms[ i ] ] ) );
            } else {
               tmp_data.push_back( "0" );
            }
         }
         binda_csv.data.push_back( tmp_data );
      }

      {
         vector < QString > tmp_data;
         binda_csv.data.push_back( tmp_data );
      }

      for ( map < double, map < QString, int > >::iterator it = bindas3.begin();
            it != bindas3.end();
            ++it ) {
         vector < QString > tmp_data;
         tmp_data.push_back( QString( "%1" ).arg( it->first ) );
         for ( int i = 0; i < (int) binda_atoms.size(); ++i ) {
            if ( it->second.count( binda_atoms[ i ] ) ) {
               tmp_data.push_back( QString( "%1" ).arg( it->second[ binda_atoms[ i ] ] ) );
            } else {
               tmp_data.push_back( "0" );
            }
         }
         binda_csv.data.push_back( tmp_data );
      }
   }
   
   if (
       !csv_write( header_csv, summary_csv ) ||
       !csv_write( header_res_csv, summary_res_csv ) ||
       !csv_write( header_res_atom_csv, summary_res_atom_csv ) ||
       !csv_write( bin_csv ) ||
       !csv_write( bind_csv ) ||
       !csv_write( binda_csv )
       ) {
      return;
   }

   editor_msg( "dark blue", QString( us_tr( "SOL2WAT done. All produced files in directory %1" ) ).arg( reportpath ) );
}

bool US_Hydrodyn_Pdb_Tool::csv_write( csv & header_csv, csv & detail_csv ) {
   QString filename = detail_csv.filename;
      
   QFile f( filename );

   if ( !f.open( QIODevice::WriteOnly ) )
   {
      QMessageBox::warning( this, windowTitle(),
                            QString(us_tr("Could not open %1 for writing!")).arg(filename) );
      return false;
   }

   QTextStream t( &f );

   QString qs;
   for ( unsigned int i = 0; i < header_csv.header.size(); i++ ) {
      qs += QString( "%1\"%2\"" ).arg( i ? "," : "" ).arg( header_csv.header[ i ] );
   }
   t << qs << Qt::endl;
   for ( unsigned int i = 0; i < header_csv.data.size(); ++i ) {
      qs = "";
      for ( unsigned int j = 0; j < header_csv.data[i].size(); ++j ) {
         qs += QString( "%1%2" ).arg( j ? "," : "" ).arg( header_csv.data[ i ][ j ] );
      }
      t << qs << Qt::endl;
   }

   t << Qt::endl;

   qs = "";
   for ( unsigned int i = 0; i < detail_csv.header.size(); i++ ) {
      qs += QString( "%1\"%2\"" ).arg( i ? "," : "" ).arg( detail_csv.header[ i ] );
   }
   t << qs << Qt::endl;
   for ( unsigned int i = 0; i < detail_csv.data.size(); ++i ) {
      qs = "";
      for ( unsigned int j = 0; j < detail_csv.data[i].size(); ++j ) {
         qs += QString( "%1%2" ).arg( j ? "," : "" ).arg( detail_csv.data[ i ][ j ] );
      }
      t << qs << Qt::endl;
   }
   f.close();
   ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( filename );
   editor_msg("black", QString("File %1 written\n").arg( filename ) );
   return true;
}
   
bool US_Hydrodyn_Pdb_Tool::csv_write( csv & detail_csv ) {
   QString filename = detail_csv.filename;
      
   QFile f( filename );

   if ( !f.open( QIODevice::WriteOnly ) )
   {
      QMessageBox::warning( this, windowTitle(),
                            QString(us_tr("Could not open %1 for writing!")).arg(filename) );
      return false;
   }

   QTextStream t( &f );

   QString qs;
   for ( unsigned int i = 0; i < detail_csv.header.size(); i++ ) {
      qs += QString( "%1\"%2\"" ).arg( i ? "," : "" ).arg( detail_csv.header[ i ] );
   }
   t << qs << Qt::endl;
   for ( unsigned int i = 0; i < detail_csv.data.size(); ++i ) {
      qs = "";
      for ( unsigned int j = 0; j < detail_csv.data[i].size(); ++j ) {
         qs += QString( "%1%2" ).arg( j ? "," : "" ).arg( detail_csv.data[ i ][ j ] );
      }
      t << qs << Qt::endl;
   }
   ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( filename );
   editor_msg("black", QString("File %1 written\n").arg( filename ) );
   return true;
}
   

void US_Hydrodyn_Pdb_Tool::csv_sel_msg()
{
   pdb_sel_count counts = count_selected( lv_csv );
   lbl_csv_sel_msg->setText( pdb_sel_count_msg( counts ) );
}

void US_Hydrodyn_Pdb_Tool::csv2_sel()
{
   sel( lv_csv2 );
   update_enables_csv();
}

void US_Hydrodyn_Pdb_Tool::csv2_sel_clear( )
{
   lv_csv2->clearSelection();
   update_enables_csv2();
}

void US_Hydrodyn_Pdb_Tool::csv2_sel_clean()
{
   clean_selection( lv_csv2 );
   update_enables_csv2();
}

void US_Hydrodyn_Pdb_Tool::csv2_sel_invert()
{
   invert_selection( lv_csv2 );
   update_enables_csv2();
}

void US_Hydrodyn_Pdb_Tool::csv2_sel_chain()
{
   select_chain( lv_csv2 );
   update_enables_csv2();
}

void US_Hydrodyn_Pdb_Tool::csv2_sel_nearest_atoms()
{
   sel_nearest_atoms( lv_csv2 );
}

void US_Hydrodyn_Pdb_Tool::csv2_sel_nearest_residues()
{
   sel_nearest_residues( lv_csv2 );
}

void US_Hydrodyn_Pdb_Tool::csv2_clash_report()
{
   distances( lv_csv2 );
}

void US_Hydrodyn_Pdb_Tool::csv2_angle()
{
   compute_angle( lv_csv2 );
}

void US_Hydrodyn_Pdb_Tool::csv2_sol2wat()
{
   pdb_sel_count  counts            = count_selected( lv_csv2 );
   if ( !counts.SOLs ||
        QMessageBox::question(this, 
                              us_tr( "US-SOMO: PDB Editor : Sol2wat " ),
                              QString( us_tr( "What do you want to process?" ) ),
                              us_tr( "A &single file that is already loaded" ),
                              us_tr( "A &trajectory" ),
                              QString(),
                              0,
                              1
                              ) == 1 )
   {
      sol2wat_traj( lv_csv2 );
   } else {
      sol2wat( lv_csv2 );
   }
}

void US_Hydrodyn_Pdb_Tool::csv2_sel_msg()
{
   pdb_sel_count counts = count_selected( lv_csv2 );
   lbl_csv2_sel_msg->setText( pdb_sel_count_msg( counts ) );
}

QString US_Hydrodyn_Pdb_Tool::pdb_sel_count_msg( pdb_sel_count &counts )
{
   QString qs = "";
   if ( counts.models > 1 )
   {
      qs += QString( "%1 models%2 " )
         .arg( counts.models )
         .arg( counts.model_partial ? "*" : "" );
   }
   if ( counts.chains > 1 )
   {
      qs += QString( "%1 chains%2 " )
         .arg( counts.chains )
         .arg( counts.chain_partial ? "*" : "" );
   }
   if ( counts.residues > 1 )
   {
      qs += QString( "%1 residues%2 " )
         .arg( counts.residues )
         .arg( counts.residue_partial ? "*" : "" );
   }
   qs += QString( "%1 atoms selected" ).arg( counts.atoms );
   return qs;
}

void US_Hydrodyn_Pdb_Tool::invert_selection( QTreeWidget *lv )
{
   // make sure atoms are selected
   clean_selection( lv );

   // turn off parents
   {
      QTreeWidgetItemIterator it( lv );
      while ( (*it) ) 
      {
         QTreeWidgetItem *item = (*it);
         if ( item->isSelected() && item->childCount() )
         {
            item->setSelected( false );
         }
         ++it;
      }
   }
   // invert atoms
   {
      QTreeWidgetItemIterator it( lv );
      while ( (*it) ) 
      {
         QTreeWidgetItem *item = (*it);
         if ( !item->childCount() )
         {
            item->setSelected( !item->isSelected() );
         }
         ++it;
      }
   }
   clean_selection( lv );
   if ( lv == lv_csv )
   {
      selection_since_count_csv1 = true;
      csv_sel_msg();
   } else {
      selection_since_count_csv2 = true;
      csv2_sel_msg();
   }
}

void US_Hydrodyn_Pdb_Tool::clean_selection( QTreeWidget *lv )
{
   // if a parent is selected, set selected on all children
   // & if all children are selected, set parents selected

   QTreeWidgetItemIterator it( lv );
   while ( (*it) ) 
   {
      QTreeWidgetItem *item = (*it);
      if ( !item->isSelected() )
      {
         if ( is_selected( item ) || all_children_selected( item ) )
         {
               item->setSelected( true );
         } 
      }
      ++it;
   }

   // lv->triggerUpdate();

   if ( lv == lv_csv )
   {
      selection_since_clean_csv1 = false;
   } else {
      selection_since_clean_csv2 = false;
   }
}

QString US_Hydrodyn_Pdb_Tool::list_keys( csv &csv1 )
{
   QString out;
   for ( map < QString, unsigned int >::iterator it = csv1.key.begin();
         it != csv1.key.end();
         it++ )
   {
      out += QString( "key: <%1> maps to <%2>\n" ).arg( it->first ).arg( it->second );
   }
   for ( map < QString, unsigned int >::iterator it = csv1.nd_key.begin();
         it != csv1.nd_key.end();
         it++ )
   {
      out += QString( "nd key: <%1> maps to <%2>\n" ).arg( it->first ).arg( it->second );
   }
   return out;
}

bool US_Hydrodyn_Pdb_Tool::no_dup_keys( csv &csv1, csv &csv2 )
{
   for ( map < QString, unsigned int >::iterator it = csv1.key.begin();
         it != csv1.key.end();
         it++ )
   {
      if ( csv2.key.count( it->first ) )
      {
         editor_msg("dark red",QString("dup key %1").arg(it->first));
         return false;
      }
   }
   for ( map < QString, unsigned int >::iterator it = csv1.nd_key.begin();
         it != csv1.nd_key.end();
         it++ )
   {
      if ( csv2.nd_key.count( it->first ) )
      {
         editor_msg("dark red",QString("dup key %1 (nd)").arg(it->first));
         return false;
      }
   }
   return true;
}

csv US_Hydrodyn_Pdb_Tool::merge_csvs( csv &csv1, csv &csv2 )
{
   // cout << "csv1 keys:\n";
   // cout << list_keys( csv1 );
   // cout << "csv2 keys:\n";
   // cout << list_keys( csv2 );

   // insert csv2 into csv1 at 
   // if ( !no_dup_keys( csv1, csv2 ) )
   // {
   // editor_msg("red","duplicate keys not yet supported");
   // return csv1;
   // }
      
   csv merged = csv1;
   editor_msg("blue",
              QString("current item key <%1>").arg( csv1.current_item_key ) );
   
   // first merge data

   // cout << QString( "current item key <%1> merged.key.count() %2\n" )
   // .arg( merged.current_item_key )
   // .arg( merged.key.count( merged.current_item_key ) )
   // ;

   merged.current_item_key = key_to_bottom_key( merged );

   QString dups_msg;

   if ( merged.current_item_key.isEmpty() ||
        !merged.key.count( merged.current_item_key ) )
   {
      // paste at end
      // cout << "paste at end\n";
      for ( unsigned int i = 0; i < (unsigned int)csv2.data.size(); i++ )
      {
         if ( merged.key.count( data_to_key( csv2.data[ i ] ) ) )
         {
            dups_msg += 
               QString( us_tr( "Warning: duplicate atoms not pasted <%1>\n") )
               .arg( data_to_key( csv2.data[ i ] ) );
         } else {
            merged.key[ data_to_key( csv2.data[ i ] ) ] = (unsigned int)merged.data.size();
            
            merged.data    .push_back  ( csv2.data    [ i ] );
            merged.visible .push_back  ( csv2.visible [ i ] );
            merged.selected.push_back  ( csv2.selected[ i ] );
            merged.open    .push_back  ( csv2.open    [ i ] );
         }
      }
   } else {
      // cout << "paste inbetween\n";
      unsigned int insert_after = merged.key[ merged.current_item_key ] + 1;
      merged.data    .resize( insert_after );
      merged.visible .resize( insert_after );
      merged.selected.resize( insert_after );
      merged.open    .resize( insert_after );
      for ( unsigned int i = 0; i < (unsigned int)csv2.data.size(); i++ )
      {
         if ( csv1.key.count( data_to_key( csv2.data[ i ] ) ) )
         {
            dups_msg += 
               QString( us_tr( "Warning: duplicate atom not pasted <%1>\n") )
               .arg( data_to_key( csv2.data[ i ] ) );
         } else {
            merged.key[ data_to_key( csv2.data[ i ] ) ] = (unsigned int)merged.data.size();
            merged.data    .push_back  ( csv2.data    [ i ] );
            merged.visible .push_back  ( csv2.visible [ i ] );
            merged.selected.push_back  ( csv2.selected[ i ] );
            merged.open    .push_back  ( csv2.open    [ i ] );
         }
      }
      for ( unsigned int i = insert_after; i < (unsigned int)csv1.data.size(); i++ )
      {
         merged.key[ data_to_key( csv1.data[ i ] ) ] = (unsigned int)merged.data.size();
         merged.data    .push_back  ( csv1.data    [ i ] );
         merged.visible .push_back  ( csv1.visible [ i ] );
         merged.selected.push_back  ( csv1.selected[ i ] );
         merged.open    .push_back  ( csv1.open    [ i ] );
      }
   }      
   
   // then add keys

   for ( map < QString, unsigned int >::iterator it = csv2.nd_key.begin();
         it != csv2.nd_key.end();
         it++ )
   {
      if ( !merged.nd_key.count( it->first ) )
      {
         merged.nd_key[ it->first ] = (unsigned int)merged.nd_selected.size();
         
#if QT_VERSION < 0x040000
         merged.nd_visible .push_back( csv2.nd_visible [ it->second ] );
#endif
         merged.nd_selected.push_back( csv2.nd_selected[ it->second ] );
         merged.nd_open    .push_back( csv2.nd_open    [ it->second ] );
      }
   }

   // cout << "after_merge keys:\n";
   // cout << list_keys( merged );
   if ( !dups_msg.isEmpty() )
   {
      editor_msg( "red", dups_msg );
   }
   return merged;
}

void US_Hydrodyn_Pdb_Tool::split_pdb()
{
   // read through a multi model pdb & create a file one per model

   pb_split_pdb->setEnabled( false );

   QString use_dir = ((US_Hydrodyn *)us_hydrodyn)->somo_pdb_dir;

   ((US_Hydrodyn *)us_hydrodyn)->select_from_directory_history( use_dir, this );

   QString filename = QFileDialog::getOpenFileName( this , windowTitle() , use_dir , "*.pdb *.PDB" );

   if ( filename.isEmpty() )
   {
      pb_split_pdb->setEnabled( true );
      return;
   }

   ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( filename );

   if ( !QFile::exists( filename ) )
   {
      QMessageBox::warning( this,
                            us_tr("Could not open file"),
                            QString( us_tr( "An error occured when trying to open file\n"
                                         "%1\n"
                                         "The file does not exist" ) )
                            .arg( filename )
                            );
      pb_split_pdb->setEnabled( true );
      return;
   }

   QFile f( filename );

   if ( !f.open( QIODevice::ReadOnly ) )
   {
      QMessageBox::warning( this,
                            us_tr("Could not open file"),
                            QString("An error occured when trying to open file\n"
                                    "%1\n"
                                    "Please check the permissions and try again\n")
                            .arg( filename )
                            );
      pb_split_pdb->setEnabled( true );
      return;
   }

   if ( QMessageBox::question(this, 
                              us_tr( "US-SOMO: PDB Editor : Split by model" ),
                              QString( us_tr( "How do you want to split the file %1?" ) ).arg( QFileInfo( filename ).fileName() ),
                              us_tr( "&Normally (by model)" ), 
                              us_tr( "Residues into models for saxs structure facture computation" ),
                              QString(),
                              0,
                              1
                              ) == 1 )
   {
      split_pdb_by_residue( f );
      pb_split_pdb->setEnabled( true );
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

   map    < QString, bool > model_names;
   vector < QString >       model_name_vector;
   unsigned int             max_model_name_len      = 0;
   QString                  model_header;
   bool                     dup_model_name_msg_done = false;
   unsigned int             end_count               = 0;
   bool                     found_model             = false;
   
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
         if ( !found_model && qs.contains( rx_save_header ) )
         {
            model_header += qs + "\n";
         }
         
         if ( qs.contains( rx_end ) )
         {
            end_count++;
         }

         if ( qs.contains( rx_model ) )
         {
            found_model = true;
            model_count++;
            // QStringList qsl = (qs.left(20).split( QRegExp("\\s+") , Qt::SkipEmptyParts ) );
            QStringList qsl;
            {
               QString qs2 = qs.left( 20 );
               qsl = (qs2 ).split( QRegExp("\\s+") , Qt::SkipEmptyParts );
            }
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
                  editor_msg( "red", us_tr( "Duplicate or missing model names found, -# extensions added" ) );
               }
            }
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

   bool no_model_directives = false;

   if ( model_count == 0 )
   {
      if ( end_count > 1 )
      {
         no_model_directives = true;
         model_count = end_count;
         for ( unsigned int i = 0; i < end_count; i++ )
         {
            QString model_name = QString("%1").arg( i + 1 );
            model_names[ model_name ] = true;
            model_name_vector.push_back ( model_name );
            if ( (unsigned int) model_name.length() > max_model_name_len )
            {
               max_model_name_len = model_name.length();
            }
         }
      } else {
         model_count = 1;
      }
   }

   editor_msg( "dark blue", QString( us_tr( "File %1 contains %2 models" ) ).arg( f.fileName() ).arg( model_count ) );

   if ( model_count == 1 )
   {
      QMessageBox::warning( this,
                            us_tr("US-SOMO: PDB Editor - Split"),
                            QString("The file"
                                    "%1\n"
                                    "Only appears to contain 1 model\n" )
                            .arg( f.fileName() )
                            );
      pb_split_pdb->setEnabled( true );
      return;
   }

   // ask how many to split into & then make them

   bool ok;
   int res = US_Static::getInteger(
                                      "US-SOMO: PDB Editor - Split",
                                      QString( us_tr( "File %1 contains %2 models\n"
                                                   "Enter the sequence increment value\n"
                                                   "Use 1 to make a pdb of each model, 2 for every other, etc.\n"
                                                   "Press Cancel to quit\n") )
                                      .arg( f.fileName() )
                                      .arg( model_count )
                                      , 
                                      1,
                                      1,
                                      model_count, 
                                      1, 
                                      &ok, 
                                      this 
                                      );
   if ( !ok ) {
      pb_split_pdb->setEnabled( true );
      return;
   } 


   QString ext = "X";
   while ( (unsigned int) ext.length() < max_model_name_len )
   {
      ext = "X" + ext;
   }
   ext = "-" + ext + ".pdb";

   QString fn = f.fileName().replace(QRegExp("\\.(pdb|PDB)$"),"") + ext;

   fn = QFileDialog::getSaveFileName( this , "Choose a name to save the files, the X's will be replaced by the model name" , fn , "PDB (*.pdb *.PDB)" );

   
   if ( fn.isEmpty() )
   {
      pb_split_pdb->setEnabled( true );
      return;
   }
   
   fn.replace(QRegExp(QString("%1$").arg(ext)), "" );

   if ( !f.open( QIODevice::ReadOnly ) )
   {
      QMessageBox::warning( this,
                            us_tr("Could not open file"),
                            QString("An error occured when trying to open file\n"
                                    "%1\n"
                                    "Please check the permissions and try again\n")
                            .arg( filename )
                            );
      pb_split_pdb->setEnabled( true );
      return;
   }

   bool overwriteForcedOn = false;

   if ( !((US_Hydrodyn *)us_hydrodyn)->overwrite )
   {
      switch ( QMessageBox::warning(this, 
                                    us_tr("US-SOMO: PDB Editor - overwrite question"),
                                    QString(us_tr("Please note:\n\n"
                                               "Overwriting of existing files currently off.\n"
                                               "This could block processing awaiting user input.\n"
                                               "What would you like to do?\n")),
                                    us_tr("&Stop"), 
                                    us_tr("&Turn on overwrite now"),
                                    us_tr("C&ontinue anyway"),
                                    0, // Stop == button 0
                                    0 // Escape == button 0
                                    ) )
      {
      case 0 : // stop
         pb_split_pdb->setEnabled( true );
         return;
         break;
      case 1 :
         ((US_Hydrodyn *)us_hydrodyn)->overwrite = true;
         ((US_Hydrodyn *)us_hydrodyn)->cb_overwrite->setChecked(true);
         overwriteForcedOn = true;
         break;
      case 2 : // continue
         break;
      }
   }

   QTextStream ts( &f );

   QString       model_lines;
   bool          in_model = no_model_directives;
   unsigned int  pos = 0;

   if ( !ts.atEnd() )
   {
      do 
      {
         QString qs = ts.readLine();
         if ( qs.contains( rx_model ) || qs.contains( rx_end ) || ts.atEnd() )
         {
            if ( model_lines.length() )
            {
               if ( !( pos % res ) )
               {
                  QString use_ext = model_name_vector[ pos ];
                  while ( (unsigned int) use_ext.length() < max_model_name_len )
                  {
                     use_ext = "0" + use_ext;
                  }
                  
                  QString use_fn = fn + "-" + use_ext + ".pdb";
                  
                  if ( QFile::exists( use_fn ) && 
                       !((US_Hydrodyn *)us_hydrodyn)->overwrite )
                  {
                     use_fn = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( use_fn, 0, this );
                  }
                  
                  QFile fn_out( use_fn );
                  
                  if ( !fn_out.open( QIODevice::WriteOnly ) )
                  {
                     QMessageBox::warning( this, "US-SOMO: PDB Editor : Split",
                                           QString(us_tr("Could not open %1 for writing!")).arg( use_fn ) );
                     pb_split_pdb->setEnabled( true );
                     return;
                  }
                  
                  QTextStream tso( &fn_out );
               
                  tso << QString("HEADER    split from %1: Model %2 of %3\n").arg( f.fileName() ).arg( pos + 1 ).arg( model_count );
                  tso << model_header;
                  tso << QString("").sprintf("MODEL  %7s\n", model_name_vector[ pos ].toLatin1().data() );
                  tso << model_lines;
                  tso << "ENDMDL\nEND\n";
                  
                  fn_out.close();
                  editor_msg( "dark blue", QString( us_tr( "File %1 written" ) ).arg( fn_out.fileName() ) );
                  qApp->processEvents();
               } else {
                  // editor_msg( "dark red", QString("model %1 skipped").arg( model_name_vector[ pos ] ) );
               }
               in_model = false;
               model_lines = "";
               pos++;
            }
            if ( qs.contains( rx_model ) ||
                 ( no_model_directives && qs.contains( rx_end ) ) )
            {
               in_model = true;
               model_lines = "";
            }
         } else {
            if ( in_model )
            {
               model_lines += qs + "\n";
            }
         }
      } while ( !ts.atEnd() );
   }
   f.close();

   if ( overwriteForcedOn )
   {
      ((US_Hydrodyn *)us_hydrodyn)->overwrite = false;
      ((US_Hydrodyn *)us_hydrodyn)->cb_overwrite->setChecked(false);
   }

   editor_msg( "dark blue", "Split done");
   pb_split_pdb->setEnabled( true );
   return;
}

void US_Hydrodyn_Pdb_Tool::hybrid_split()
{
   // read through a pdb
   // match hydrogens to atom
   // collect up into output pdb(s)

   QString use_dir = ((US_Hydrodyn *)us_hydrodyn)->somo_pdb_dir;

   ((US_Hydrodyn *)us_hydrodyn)->select_from_directory_history( use_dir, this );

   QString filename = QFileDialog::getOpenFileName( this , windowTitle() , use_dir , "*.pdb *.PDB" );

   if ( filename.isEmpty() )
   {
      return;
   }

   ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( filename );

   if ( !QFile::exists( filename ) )
   {
      QMessageBox::warning( this,
                            us_tr("US-SOMO: PDB Editor - Hybrid Extract"),
                            QString( us_tr( "An error occured when trying to open file\n"
                                         "%1\n"
                                         "The file does not exist" ) )
                            .arg( filename )
                            );
      return;
   }

   QFile f( filename );

   if ( !f.open( QIODevice::ReadOnly ) )
   {
      QMessageBox::warning( this,
                            us_tr("US-SOMO: PDB Editor - Hybrid Extract"),
                            QString("An error occured when trying to open file\n"
                                    "%1\n"
                                    "Please check the permissions and try again\n")
                            .arg( filename )
                            );
      return;
   }

   editor_msg( "dark blue", QString( us_tr( "Hybrid extract: processing file %1" ).arg( f.fileName() ) ) );
   
   QRegExp rx_atom("^ATOM");

   QTextStream ts( &f );
   unsigned int line_count = 0;

   map < QString, vector < QString > > atoms_with_hydrogens;
   map < QString, QString > this_residue;
   map < QString, QString > this_residue_hydrogens;

   QString last_residue     = "";
   QString last_residue_seq = "";

   while ( !ts.atEnd() )
   {
      QString qs = ts.readLine();
      line_count++;
      if ( line_count && !(line_count % 100000 ) )
      {
         editor_msg( "dark blue", QString( us_tr( "Lines read %1" ).arg( line_count ) ) );
         qApp->processEvents();
      }
      if ( qs.contains( rx_atom ) )
      {
         // cout << QString( "got atom <%1>\n" ).arg( qs );
         QString atom        = qs.mid( 12, 4 ).trimmed();
         QString atom_left   = atom.right( atom.length() - 1 );
         QString atom_hmap   = qs.mid( 12, 3 ).trimmed();
         atom_hmap           = atom_hmap.right( atom_hmap.length() - 1 );
         QString atom_hseq   = qs.mid( 15, 1 );
         if ( atom_hseq == " " && atom_hmap.contains( QRegExp( "^\\d$" ) ) )
         {
            atom_hseq = atom_hmap;
            atom_hmap = "";
         }

         if ( !this_residue.count( atom_hmap ) )
         {
            // cout << QString( "hmap does not contain <%1>, trying alternate\n" ).arg( atom_hmap );
            atom_hmap        = qs.mid( 12, 4 ).trimmed();
            atom_hmap        = atom_hmap.right( atom_hmap.length() - 1 );
            atom_hseq        = " ";
         }
         QString atom_hkey   = atom_hmap + ":" + atom_hseq;
         QString residue     = qs.mid( 17, 3 ).trimmed();
         QString residue_seq = qs.mid( 22, 4 ).trimmed();
         
         bool is_hydrogen = atom.left( 1 ) == "H";

         if ( !last_residue_seq.isEmpty() &&
              ( residue_seq != last_residue_seq ||
                residue     != last_residue ) )
         {
            // cout << QString( "store previous\n" );
            // match up hydrogens and push back to atoms with hydrogens
            map < QString, unsigned int > used_residues;
            for ( map < QString, QString >::iterator it = this_residue_hydrogens.begin();
                  it != this_residue_hydrogens.end();
                  it ++ )
            {
               QString href = it->first;
               href.replace( QRegExp( ":.$" ), "" );
               href.replace( QRegExp( "^\\d$" ), "" );
               // cout << QString( "href <%1>\n" ).arg( href );
               if ( !this_residue.count( href ) )
               {
                  editor_msg( "red", QString( us_tr( "Error: hydrogen ref <%1> but no atom with ref" ) ).arg( href ) );
                  return;
               }
               this_residue [ href ] += "\n" + it->second;
               if ( used_residues.count( href ) )
               {
                  used_residues[ href ]++;
               } else {
                  used_residues[ href ] = 1;
               }
            }
            for ( map < QString, unsigned int >::iterator it = used_residues.begin();
                  it != used_residues.end();
                  it ++ )
            {
               QString atom_hybrid = 
                  QString( "%1H%2" )
                  .arg( this_residue[ it->first ].mid( 77, 1 ) )
                  .arg( it->second );
               atoms_with_hydrogens[ atom_hybrid ].push_back( this_residue[ it->first ] );
            }
            this_residue          .clear( );
            this_residue_hydrogens.clear( );
         }
         last_residue          = residue;
         last_residue_seq      = residue_seq;
         
         if ( is_hydrogen )
         {
            // cout << QString( "is hydrogen key <%1>\n" ).arg( atom_hkey );
            this_residue_hydrogens[ atom_hkey ] = qs;
         } else {
            if ( atom != "C" && atom != "O" )
            {
               // cout << QString( "is not hydrogen key <%1>\n" ).arg( atom_left );
               this_residue          [ atom_left ] = qs;
            }
         }
      }
   }
   
   // flush last one
   if ( !last_residue_seq.isEmpty() )
   {
      // cout << QString( "store previous\n" );
      // match up hydrogens and push back to atoms with hydrogens
      map < QString, unsigned int > used_residues;
      for ( map < QString, QString >::iterator it = this_residue_hydrogens.begin();
            it != this_residue_hydrogens.end();
            it ++ )
      {
         QString href = it->first;
         href.replace( QRegExp( ":.$" ), "" );
         href.replace( QRegExp( "^\\d$" ), "" );
         // cout << QString( "href <%1>\n" ).arg( href );
         if ( !this_residue.count( href ) )
         {
            editor_msg( "red", QString( us_tr( "Error: hydrogen ref <%1> but no atom with ref" ) ).arg( href ) );
            return;
         }
         this_residue [ href ] += "\n" + it->second;
         if ( used_residues.count( href ) )
         {
            used_residues[ href ]++;
         } else {
            used_residues[ href ] = 1;
         }
      }
      for ( map < QString, unsigned int >::iterator it = used_residues.begin();
            it != used_residues.end();
            it ++ )
      {
         QString atom_hybrid = 
            QString( "%1H%2" )
            .arg( this_residue[ it->first ].mid( 77, 1 ) )
            .arg( it->second );
         atoms_with_hydrogens[ atom_hybrid ].push_back( this_residue[ it->first ] );
      }
      this_residue          .clear( );
      this_residue_hydrogens.clear( );
   }
   
   f.close();

   // write them out

   QString fbase = QFileInfo( filename ).path() + SLASH + QFileInfo( filename ).baseName();

   for ( map < QString, vector < QString > >::iterator it = atoms_with_hydrogens.begin();
         it != atoms_with_hydrogens.end();
         it++ )
   {

      QFile f_out( QString( "%1-%2.pdb" ).arg( fbase ).arg( it->first ) );
      if ( !f_out.open( QIODevice::WriteOnly ) )
      {
         QMessageBox::warning( this, 
                               us_tr( "US-SOMO: PDB editor : Hybrid Extract" ),
                               QString( us_tr( "Error: Can not open file %1 for writing" ) ).arg( f_out.fileName() ) );
         return;
      }

      editor_msg( "blue", QString( us_tr( "Creating %1 with %2 models" ) ).arg( f_out.fileName() ).arg( it->second.size() ) );

      // compute for remarks
      // average, sd dev pairwise distances

      vector < QString > atom_names;

      vector < double >  distance_sum;
      vector < double >  distance_sum2;
      unsigned int       count           = 0;

      double             gdistance_sum  = 0e0;
      double             gdistance_sum2 = 0e0;
      unsigned int       gcount         = 0;

      vector < double >  angle_sum;
      vector < double >  angle_sum2;
      unsigned int       angle_count    = 0;

      double             gangle_sum     = 0e0;
      double             gangle_sum2    = 0e0;
      unsigned int       gangle_count   = 0;

      QString            remark;

      for ( unsigned int i = 0; i < (unsigned int)it->second.size(); i++ )
      {
         QStringList lines = (it->second[ i ] ).split( "\n" , Qt::SkipEmptyParts );
         vector < QString > names;
         vector < point   > p;
         for ( unsigned int j = 0; j < (unsigned int)lines.size(); j++ )
         {
            names.push_back( lines[ j ].mid( 12, 4 ).trimmed().left( 1 ) );
            point this_p;
            this_p.axis[ 0 ] = lines[ j ].mid( 30 , 8 ).trimmed().toFloat();
            this_p.axis[ 1 ] = lines[ j ].mid( 38 , 8 ).trimmed().toFloat();
            this_p.axis[ 2 ] = lines[ j ].mid( 46 , 8 ).trimmed().toFloat();
            p.push_back( this_p );
         }

         if ( !i )
         {
            atom_names = names;
            distance_sum .resize( names.size() );
            distance_sum2.resize( names.size() );
            if ( names.size() > 2 )
            {
               angle_sum .resize( names.size() - 2 );
               angle_sum2.resize( names.size() - 2 );
            }
         } else {
            if ( atom_names != names )
            {
               editor_msg( "dark red", QString( us_tr( "WARNING: atom names inconsistance for %1" ) ).arg( it->first ) );
            }
         }

         // compute distances
         count++;

         for ( unsigned int j = 1; j < (unsigned int)names.size(); j++ )
         {
            double distance = 
               sqrt( ( p[ 0 ].axis[ 0 ] - p[ j ].axis[ 0 ] ) * ( p[ 0 ].axis[ 0 ] - p[ j ].axis[ 0 ] ) +
                     ( p[ 0 ].axis[ 1 ] - p[ j ].axis[ 1 ] ) * ( p[ 0 ].axis[ 1 ] - p[ j ].axis[ 1 ] ) +
                     ( p[ 0 ].axis[ 2 ] - p[ j ].axis[ 2 ] ) * ( p[ 0 ].axis[ 2 ] - p[ j ].axis[ 2 ] ) );
            if ( !i )
            {
               distance_sum [ j ] = distance;
               distance_sum2[ j ] = distance * distance;
            } else {
               distance_sum [ j ] += distance;
               distance_sum2[ j ] += distance * distance;
            }
            gcount++;
            gdistance_sum      += distance;
            gdistance_sum2     += distance * distance;
         }               

         // compute angles, if possible
         if ( names.size() > 2 )
         {
            // angle H1-X-H2
            angle_count++;
            for ( unsigned int j = 0; j < (unsigned int)names.size() - 2; j++ )
            {
               double angle =
                  acos( ((US_Hydrodyn *)us_hydrodyn)->dot( ((US_Hydrodyn *)us_hydrodyn)->normal( ((US_Hydrodyn *)us_hydrodyn)->minus( p[ 1 ], p[ 0 ] ) ),
                                                           ((US_Hydrodyn *)us_hydrodyn)->normal( ((US_Hydrodyn *)us_hydrodyn)->minus( p[ j + 2 ], p[ 0 ] ) ) ) );
               if ( !i )
               {
                  angle_sum [ j ] = angle;
                  angle_sum2[ j ] = angle * angle;
               } else {
                  angle_sum [ j ] += angle;
                  angle_sum2[ j ] += angle * angle;
               }
               gangle_count++;
               gangle_sum      += angle;
               gangle_sum2     += angle * angle;
            }
         }  
      }
      
      for ( unsigned int i = 1; i < (unsigned int)atom_names.size(); i++ )
      {
         remark += 
            QString( "REMARK     distance %1-%2%3 average of %4 pairs %5 A" )
            .arg( atom_names[ 0 ] )
            .arg( atom_names[ i ] )
            .arg( i )
            .arg( count )
            .arg( distance_sum[ i ] / (double) count );
         
         if ( count > 1 )
         {
            remark += 
               QString( ", standard deviation %1" )
               .arg( 
                    sqrt( 
                         ( count * distance_sum2[ i ] - distance_sum[ i ] * distance_sum[ i ] ) /
                         ( count * ( count - 1 ) ) 
                         ) );
         }
         remark += "\n";
      }

      if ( atom_names.size() > 2 )
      {
         remark += 
            QString( "REMARK     distance %1-H* average of %2 pairs %3 A" )
            .arg( atom_names[ 0 ] )
            .arg( gcount )
            .arg( gdistance_sum / (double) gcount );
         
         if ( gcount > 1 )
         {
            remark += 
               QString( ", standard deviation %1" )
               .arg( 
                    sqrt( 
                         ( gcount * gdistance_sum2 - gdistance_sum * gdistance_sum ) /
                         ( gcount * ( gcount - 1 ) ) 
                         ) );
         }
         remark += "\n";
      }

      for ( unsigned int i = 0; i < (unsigned int)atom_names.size() - 2; i++ )
      {
         remark += 
            QString( "REMARK     angle H1-%1-H%2 average of %3 pairs %4 degrees" )
            .arg( atom_names[ 0 ] )
            .arg( i + 2 )
            .arg( angle_count )
            .arg( ( angle_sum[ i ] / (double) angle_count ) * 180.0 / M_PI );

         if ( angle_count > 1 )
         {
            remark += 
               QString( ", standard deviation %1" )
               .arg( 
                    sqrt( 
                      ( angle_count * angle_sum2[ i ] - angle_sum[ i ] * angle_sum[ i ] ) /
                      ( angle_count * ( angle_count - 1 ) ) 
                      ) * ( 180.0 / M_PI ) );
         }
         remark += "\n";
      }
      
      if ( atom_names.size() > 3 )
      {
         remark += 
            QString( "REMARK     angle H1-%1-H* average of %2 pairs %3 degrees" )
            .arg( atom_names[ 0 ] )
            .arg( gangle_count )
            .arg( ( gangle_sum / (double) gangle_count ) * 180.0 / M_PI );

         if ( gangle_count > 1 )
         {
            remark += 
               QString( ", standard deviation %1" )
               .arg( 
                    sqrt( 
                         ( gangle_count * gangle_sum2 - gangle_sum * gangle_sum ) /
                         ( gangle_count * ( gangle_count - 1 ) ) 
                         ) * ( 180.0 / M_PI ) );
         }
         remark += "\n";
      }
         
      QTextStream ts_out( &f_out );
      ts_out << QString( "HEADER    US-SOMO PDB Editor hybrid extract from %1\n" ).arg( filename );
      ts_out << remark;
      for ( unsigned int i = 0; i < (unsigned int)it->second.size(); i++ )
      {
         ts_out <<
            QString( "MODEL     %1\n%2\nENDMDL\n" )
            .arg( QString( "" ).sprintf( "%4u", i + 1 ) )
            .arg( it->second[ i ] );
      }
      ts_out << "END\n";
      f_out.close();
   }

   editor_msg( "dark blue", "Hybrid extract done");
   return;
}

void US_Hydrodyn_Pdb_Tool::h_to_chainX()
{
   // read through a pdb
   // match hydrogens to atom
   // collect up into output pdb(s)

   QString use_dir = ((US_Hydrodyn *)us_hydrodyn)->somo_pdb_dir;

   ((US_Hydrodyn *)us_hydrodyn)->select_from_directory_history( use_dir, this );

   QString filename = QFileDialog::getOpenFileName( this , windowTitle() , use_dir , "*.pdb *.PDB" );

   if ( filename.isEmpty() )
   {
      return;
   }

   ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( filename );

   if ( !QFile::exists( filename ) )
   {
      QMessageBox::warning( this,
                            us_tr("US-SOMO: PDB Editor - H to chain X"),
                            QString( us_tr( "An error occured when trying to open file\n"
                                         "%1\n"
                                         "The file does not exist" ) )
                            .arg( filename )
                            );
      return;
   }

   QFile f( filename );

   if ( !f.open( QIODevice::ReadOnly ) )
   {
      QMessageBox::warning( this,
                            us_tr("US-SOMO: PDB Editor - H to chain X"),
                            QString("An error occured when trying to open file\n"
                                    "%1\n"
                                    "Please check the permissions and try again\n")
                            .arg( filename )
                            );
      return;
   }

   editor_msg( "dark blue", QString( us_tr( "H to Chain X: processing file %1" ).arg( f.fileName() ) ) );
   
   QRegExp rx_atom ("^(ATOM|HETATM)");
   QRegExp rx_model("^MODEL");
   QRegExp rx_end  ("^END");

   QTextStream ts( &f );
   unsigned int line_count = 0;

   QString     out;
   QStringList hydrogens;

   while ( !ts.atEnd() )
   {
      QString qs = ts.readLine();
      line_count++;
      if ( line_count && !(line_count % 100000 ) )
      {
         editor_msg( "dark blue", QString( us_tr( "Lines read %1" ).arg( line_count ) ) );
         qApp->processEvents();
      }
      if ( qs.contains( rx_atom ) )
      {
         QString atom        = qs.mid( 12, 4 ).trimmed();
         QString atom_left   = atom.right( atom.length() - 1 );
         if ( atom.left( 1 ) == "H" )
         {
            hydrogens << qs;
         } else {
            out += qs + "\n";
         }
      } else {
         if ( qs.contains( rx_end ) && hydrogens.size() )
         {
            // flush out hydrogens into this chain
            out += "TER\n";
            for ( unsigned int i = 0; i < (unsigned int)hydrogens.size(); i++ )
            {
               hydrogens[ i ]
                  .replace( 12, 4, " XH " )
                  .replace( 17, 3, "XHY" )
                  .replace( 21, 1, "X" )
                  .replace( 22, 4, QString( "" ).sprintf( "%4u",  i + 1 ) );
               out += hydrogens[ i ] + "\n";
            }
            out += "TER\n";
            hydrogens.clear( );
         }
         out += qs + "\n";
      }
   }

   f.close();

   if ( hydrogens.size() )
   {
      // flush out hydrogens into this chain
      out += "TER\n";
      for ( unsigned int i = 0; i < (unsigned int)hydrogens.size(); i++ )
      {
         hydrogens[ i ]
            .replace( 12, 4, " XH " )
            .replace( 17, 3, "XHY" )
            .replace( 21, 1, "X" );
         out += hydrogens[ i ] + "\n";
            }
      out += "TER\n";
      hydrogens.clear( );
   }

   QString fbase = QFileInfo( filename ).path() + SLASH + QFileInfo( filename ).baseName();
   QFile f_out( QString( "%1-HX.pdb" ).arg( fbase ) );

   if ( !f_out.open( QIODevice::WriteOnly ) )
   {
      QMessageBox::warning( this, 
                            us_tr( "US-SOMO: PDB editor : H to chain X" ),
                            QString( us_tr( "Error: Can not open file %1 for writing" ) ).arg( f_out.fileName() ) );
      return;
   }
   
   editor_msg( "blue", QString( us_tr( "Creating %1" ) ).arg( f_out.fileName() ) );

   QTextStream ts_out( &f_out );
   ts_out << out;
   f_out.close();
   
   editor_msg( "dark blue", "H to chain X done");
   return;
}

void US_Hydrodyn_Pdb_Tool::join_pdbs()
{
   QStringList join_files;
   QStringList files;
   map < QString, bool > already_listed;
   do 
   {
      QString use_dir = ((US_Hydrodyn *)us_hydrodyn)->somo_pdb_dir;
      ((US_Hydrodyn *)us_hydrodyn)->select_from_directory_history( use_dir, this );
      files = US_Pdb_Util::sort_pdbs( QFileDialog::getOpenFileNames( this , us_tr( "Select PDB files to join, Cancel when done" ) , use_dir , "PDB files (*.pdb *.PDB)" ) );

      for ( unsigned int i = 0; i < (unsigned int)files.size(); i++ )
      {
         if ( !already_listed.count( files[ i ] ) )
         {
            join_files << files[ i ];
            already_listed[ files[ i ] ] = true;
            ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( files[ i ] );
         }
      }
   } while ( files.size() );
   if ( !join_files.size() )
   {
      return;
   }

   if ( join_files.size() == 1 )
   {
      QMessageBox::warning( this, 
                            us_tr( "US-SOMO: PDB editor : Join" ),
                            us_tr( "Error: Only one file selected to join." ) );
      return;
   }

   
   

   QString save_file = QFileDialog::getSaveFileName( this , us_tr( "Choose a name to save the joined PDBs" ) , QFileInfo( join_files.back() ).path() , "PDB (*.pdb *.PDB)" );


   if ( save_file.isEmpty() )
   {
      return;
   }

   if ( !save_file.contains( QRegExp( "\\.pdb$", Qt::CaseInsensitive ) ) )
   {
      save_file += ".pdb";
   }

   if ( QFile::exists( save_file ) )
   {
      save_file = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( save_file, 0, this );
   }

   editor_msg( "dark blue", 
               QString( "Joining %1 as %2" )
               .arg( join_files.join( "\n" ) )
               .arg( save_file ) );

   // go through each file & join them as models
   // note: 

   QFile f_out( save_file );
   if ( !f_out.open( QIODevice::WriteOnly ) )
   {
      QMessageBox::warning( this, 
                            us_tr( "US-SOMO: PDB editor : Join" ),
                            QString( us_tr( "Error: Can not open file %1 for writing" ) ).arg( save_file ) );
      return;
   }

   QTextStream ts_out( &f_out );

   QRegExp rx_model("^MODEL");
   QRegExp rx_get_model("^MODEL\\s+(\\S+)");
   QRegExp rx_end("^END");
   QRegExp rx_atom("^(ATOM|HETATM|TER)");
   QRegExp rx_remarks("^REMARK");

   unsigned int model    = 0;

   ts_out << 
      QString( "HEADER    US-SOMO: join of %1 pdbs: %2 %3 etc\n" )
      .arg( join_files.size() )
      .arg( QFileInfo( join_files[ 0 ] ).fileName() )
      .arg( QFileInfo( join_files[ 1 ] ).fileName() );

   map < QString, bool > used_model;
   unsigned int actual_model_count = 0;

   for ( unsigned int i = 0; i < (unsigned int)join_files.size(); i++ )
   {
      editor_msg( "dark gray", QString( us_tr( "Processing %1" ) ).arg( join_files[ i ] ) );
      qApp->processEvents();

      bool  in_model = false;

      QFile f_in( join_files[ i ] );

      if ( !f_in.open( QIODevice::ReadOnly ) )
      {
         QMessageBox::warning( this, 
                               us_tr( "US-SOMO: PDB editor : Join" ) ,
                               QString( us_tr( "Error: Can not open file %1 for reading" ) ).arg( join_files[ i ] ) );
         f_out.close();
         f_out.remove();
         return;
      }

      QTextStream ts_in( &f_in );

      bool                  atom_or_model_found = false;
      // bool                  has_model_line      = false;
      QString               last_model          = "";

      QString               remarks;

      while ( !ts_in.atEnd() )
      {
         QString qs = ts_in.readLine();

         bool line_written = false;

         if ( rx_remarks.indexIn( qs ) != -1 )
         {
            if ( in_model )
            {
               ts_out << qs << Qt::endl;
            } else {
               remarks += qs + "\n";
            }
         }

         if ( rx_atom.indexIn( qs ) != -1 )
         {
            if ( !in_model )
            {
               if ( last_model.isEmpty() )
               {
                  ++model;
                  while( used_model.count( QString( "%1" ).arg( model ) ) )
                  {
                     model++;
                  }
                  used_model[ QString( "%1" ).arg( model ) ] = true;
                  ts_out << QString( "MODEL        %1\n" ).arg( model );
                  ts_out << remarks;
                  remarks = "";
               }
               actual_model_count++;
               in_model = true;
            }

            // has_model_line      = false;
            atom_or_model_found = true;
            ts_out << qs << "\n";
            line_written = true;
         }
         if ( rx_model.indexIn( qs ) != -1 )
         {
            atom_or_model_found = true;
            // has_model_line      = true;
            if ( rx_get_model.indexIn( qs ) != -1 )
            {
               last_model = rx_get_model.cap( 1 );
               if ( used_model.count( last_model ) )
               {
                  unsigned int ext = 0;
                  QString use_model = QString( "%1-%2" ).arg( last_model ).arg( ++ext );
                  while ( used_model.count( use_model ) )
                  {
                     use_model = QString( "%1-%2" ).arg( last_model ).arg( ++ext );
                  }
                  qs = "MODEL     " + use_model;
                  last_model = use_model;
               }
               used_model[ last_model ] = true;
            } else {
               last_model = "";
            }
            if ( in_model )
            {
               ts_out << remarks;
               remarks = "";
               ts_out << "ENDMDL\n";
               line_written = true;
               in_model = false;
            }
            
         } 
         if ( !line_written && atom_or_model_found && rx_end.indexIn( qs ) == -1 )
         {
            ts_out << qs << "\n";
         }
      }
      f_in.close();

      if ( in_model )
      {
         ts_out << "ENDMDL\n";
      }
   }
   ts_out << "END\n";
   f_out.close();
   QMessageBox::information( this, 
                             us_tr( "US-SOMO: PDB editor : Join" ) ,
                             QString( us_tr( "File %1 created with %2 models" ) ).arg( save_file ).arg( actual_model_count ) );
}

void US_Hydrodyn_Pdb_Tool::merge()
{
   if ( pdb_tool_merge_widget )
   {
      if ( pdb_tool_merge_window->isVisible() )
      {
         pdb_tool_merge_window->raise();
      }
      else
      {
         pdb_tool_merge_window->show();
      }
      return;
   }
   else
   {
      pdb_tool_merge_window = new US_Hydrodyn_Pdb_Tool_Merge( us_hydrodyn, this );
      pdb_tool_merge_window->show();
   }
}

void US_Hydrodyn_Pdb_Tool::renum_pdb()
{
   // read through a multi model pdb & create a file one per model

   pb_renum_pdb->setEnabled( false );

   QDir::setCurrent( ((US_Hydrodyn *)us_hydrodyn)->somo_pdb_dir );


   map < QString, QString > parameters;
   US_Hydrodyn_Pdb_Tool_Renum *uhptr = 
      new US_Hydrodyn_Pdb_Tool_Renum(
                                     us_hydrodyn,
                                     &parameters,
                                     this 
                                     );
   US_Hydrodyn::fixWinButtons( uhptr );
   uhptr->exec();
   delete uhptr;

   QString      filename            = 
      parameters.count( "inputfile"           ) ?
      parameters[ "inputfile"           ] : "";
   unsigned int startatom           = 
      parameters.count( "startatom"           ) ? 
      parameters[ "startatom"           ].toUInt() : 1;
   unsigned int startresidue        = 
      parameters.count( "startresidue"        ) ? 
      parameters[ "startresidue"        ].toUInt() : 1;
   bool         chainrestartatom    = 
      parameters.count( "chainrestartatom"    ) && parameters[ "chainrestartatom"    ].contains( QRegExp( "^(Y|y)" ) ) ? true : false;
   bool         chainrestartresidue  = 
      parameters.count( "chainrestartresidue" ) && parameters[ "chainrestartresidue" ].contains( QRegExp( "^(Y|y)" ) ) ? true : false;
   QString      usechainlist = 
      parameters.count( "usechainlist"        ) ? 
      parameters[ "usechainlist"        ] : "";
   bool         modelrestartatom    = 
      parameters.count( "modelrestartatom"    ) && parameters[ "modelrestartatom"    ].contains( QRegExp( "^(Y|y)" ) ) ? true : false;
   bool         modelrestartresidue = 
      parameters.count( "modelrestartresidue" ) && parameters[ "modelrestartresidue" ].contains( QRegExp( "^(Y|y)" ) ) ? true : false;
   bool         reseqatom           = 
      parameters.count( "reseqatom"           ) && parameters[ "reseqatom"           ].contains( QRegExp( "^(Y|y)" ) ) ? true : false;
   bool         reseqresidue        = 
      parameters.count( "reseqresidue"        ) && parameters[ "reseqresidue"        ].contains( QRegExp( "^(Y|y)" ) ) ? true : false;
   bool         striphydrogens      =
      parameters.count( "striphydrogens"      ) && parameters[ "striphydrogens"      ].contains( QRegExp( "^(Y|y)" ) ) ? true : false;
   bool         itassertemplate      =
      parameters.count( "itassertemplate"     ) && parameters[ "itassertemplate"     ].contains( QRegExp( "^(Y|y)" ) ) ? true : false;

   cout << QString( "usechainlist %1\n" ).arg( usechainlist );
   cout << QString( "reseqatom %1\n" ).arg( reseqatom );
   cout << QString( "startatom %1\n" ).arg( startatom );
   cout << QString( "startresidue %1\n" ).arg( startresidue );
   cout << QString( "chainrestartatom %1\n" ).arg( chainrestartatom );
   cout << QString( "modelrestartatom %1\n" ).arg( modelrestartatom );
   cout << QString( "reseqresidue %1\n" ).arg( reseqresidue );
   cout << QString( "chainrestartresidue %1\n" ).arg( chainrestartresidue );
   cout << QString( "modelrestartresidue %1\n" ).arg( modelrestartresidue );
   cout << QString( "striphydrogens %1\n" ).arg( striphydrogens );
   cout << QString( "itassertemplate %1\n" ).arg( itassertemplate );

   map < QString, bool > use_chain_map;
   for ( unsigned int i = 0; i < (unsigned int) usechainlist.length(); i++ )
   {
      use_chain_map[ QString( "%1" ).arg( usechainlist[ (int) i ] ) ] = true;
   }

   if ( filename.isEmpty() )
   {
      pb_renum_pdb->setEnabled( true );
      return;
   }

   if ( !QFile::exists( filename ) )
   {
      QMessageBox::warning( this,
                            us_tr("Could not open file"),
                            QString( us_tr( "An error occured when trying to open file\n"
                                         "%1\n"
                                         "The file does not exist" ) )
                            .arg( filename )
                            );
      pb_renum_pdb->setEnabled( true );
      return;
   }

   QFile f( filename );

   if ( !f.open( QIODevice::ReadOnly ) )
   {
      QMessageBox::warning( this,
                            us_tr("Could not open file"),
                            QString("An error occured when trying to open file\n"
                                    "%1\n"
                                    "Please check the permissions and try again\n")
                            .arg( filename )
                            );
      pb_renum_pdb->setEnabled( true );
      return;
   }

   QString foutname = QFileDialog::getSaveFileName( this , us_tr("Choose a filename to save the renumbered pdb") , QString() , "*.pdb *.PDB" );

   if( foutname.isEmpty() )
   {
      pb_renum_pdb->setEnabled( true );
      return;
   }
      
   if ( !foutname.contains( QRegExp( ".pdb$", Qt::CaseInsensitive ) ) )
   {
      foutname += ".pdb";
   }

   if ( foutname == filename )
   {
      QMessageBox::warning( this, "UltraScan",
                            QString(us_tr("The output file must not be the same as the inpufile %1!")).arg( foutname ) );
      pb_renum_pdb->setEnabled( true );
      return;
   }
      
   if ( QFile::exists( foutname ) )
   {
      foutname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( foutname, 0, this );
   }

   QFile fout( foutname );

   // read through & renumber
   if ( !fout.open( QIODevice::WriteOnly ) )
   {
      QMessageBox::warning( this, "UltraScan",
                            QString(us_tr("Could not open %1 for writing!")).arg( foutname ) );
      pb_renum_pdb->setEnabled( true );
      return;
   }
   
   QTextStream tsi( &f    );
   QTextStream tso( &fout );

   QRegExp rx_end ("^END");
   QRegExp rx_atom("^(ATOM|HETATM)");
   QRegExp rx_hetatm("^HETATM");

   unsigned int atomno    = startatom;
   unsigned int residueno = startresidue;

   QString last_residue_id = "";
   QString last_chain_id   = "";

   while( !tsi.atEnd() )
   {
      QString line = tsi.readLine();
      if ( rx_end.indexIn( line ) != -1 )
      {
         last_chain_id = "";
         last_residue_id = "";
         if ( modelrestartatom )
         {
            atomno = startatom;
         }
         if ( modelrestartresidue )
         {
            residueno = startresidue;
         }
      }
      if ( rx_atom.indexIn( line ) != -1 )
      {
         if ( striphydrogens && line.mid( 12, 2 ).contains( QRegExp( "^((\\d| )H|H)" ) ) )
         {
            continue;
         }
         QString chain_id   = line.mid( 21, 1 );
         QString residue_id = line.mid( 17, 10 );
         if ( chain_id != last_chain_id )
         {
            if ( chainrestartatom )
            {
               atomno = startatom;
               last_chain_id   = chain_id;
            }
            if ( chainrestartresidue )
            {
               residueno = startresidue;
               last_residue_id = residue_id;
            }
         }
         if ( !usechainlist.length() || use_chain_map.count( chain_id ) )
         {
            if ( reseqatom )
            {
               if ( atomno > 99999 )
               {
                  editor_msg( "dark red", us_tr( "Warning: more than 99,999 atoms, numbering restarted at 1" ) );
                  atomno = 1;
               }
               line = line.replace( 6, 5, QString( "" ).sprintf( "%5d", atomno ++ ) );
               if ( rx_hetatm.indexIn( line ) == -1 )
               {
                  line = line.replace( 5, 1, " " );
               } else {
                  line = line.replace( 5, 1, "M" );
               }
            }

            if ( reseqresidue )
            {
               if ( !last_residue_id.isEmpty() &&
                    last_residue_id != residue_id )
               {
                  residueno++;
                  if ( residueno > 9999 )
                  {
                     editor_msg( "dark red", us_tr( "Warning: more than 9,999 residues, numbering restarted at 1" ) );
                     residueno = 1;
                  }
               }
               line = line.replace( 22, 4, QString( "" ).sprintf( "%4d", residueno ) );
               last_residue_id = residue_id;
            }
         }
         if ( itassertemplate )
         {
            // Column 1 -30: Atom & Residue records of query sequence.
            // Column 31-54: Coordinates of atoms in query copied from corresponding atoms in template.
            // Column 55-59: Corresponding residue number in template based on alignment
            // Column 60-64: Corresponding residue name in template
            /* sequences, e.g.

ATOM   2001  CA  MET     1      41.116 -30.727   6.866  129 THR
ATOM   2002  CA  ALA     2      39.261 -27.408   6.496  130 ARG
ATOM   2003  CA  ALA     3      35.665 -27.370   7.726  131 THR
ATOM   2004  CA  ARG     4      32.662 -25.111   7.172  132 ARG
ATOM   2005  CA  GLY     5      29.121 -25.194   8.602  133 ARG
            */

            QString residue_seq  = line.mid( 22, 4 );
            QString residue_name = line.mid( 17, 3 );
            line = line.left( 55 ) + QString( "" ).sprintf( "%4d ", residue_seq.toUInt() ) + residue_name;
            // line = line.replace( 55, 4, residue_seq );
            // line = line.replace( 61, 4, residue_name );
         }
      }
      tso << line << Qt::endl;
   }

   f   .close();
   fout.close();

   editor_msg( "dark blue", QString( "Renumber done, written file %1" ).arg( foutname ) );
   pb_renum_pdb->setEnabled( true );
   return;
}

void US_Hydrodyn_Pdb_Tool::csv_clear( )
{
   lv_csv->clear( );
   csv new_csv;
   csv1 = new_csv;
   csv_to_lv( csv1, lv_csv );
   csv_undos.clear( );
   te_csv->clear( );
   update_enables_csv();
}

void US_Hydrodyn_Pdb_Tool::csv2_clear( )
{
   lv_csv2->clear( );
   csv new_csv;
   csv2.resize( 1 );
   csv2[ 0 ] = new_csv;
   csv_to_lv( csv2[ 0 ], lv_csv2 );
   csv2_undos.clear( );
   csv2_pos = 0;
   qwtw_wheel->setRange( 0.0, 0.0); qwtw_wheel->setSingleStep( 1 );
   lbl_pos_range->setText("1\nof\n1");
   csv2_redisplay( 0 );
   te_csv2->clear( );
   update_enables_csv2();
}

QString US_Hydrodyn_Pdb_Tool::key_to_bottom_key( csv &csv1 )
{
   // takes a key and returns the lowest level key
   if ( csv1.current_item_key.isEmpty() 
        || csv1.key.count( csv1.current_item_key ) )
   {
      // already lowest level
      return csv1.current_item_key;
   }

   // go through all the data rows and find highest # that matches
   // (ugh, better to maintain some sort of child ref? )

   QRegExp rx_ok( "^" + csv1.current_item_key );
   for ( int i = (int) csv1.data.size() - 1; i >= 0; i-- )
   {
      if ( rx_ok.indexIn( data_to_key( csv1.data[ i ] ) ) != -1 )
      {
         return data_to_key( csv1.data[ i ] );
      }
   }

   // couldn't find it
   editor_msg( "red", 
               QString( us_tr( "Internal error: could not find bottom for key <%1>" ) )
               .arg( csv1.current_item_key ) );
   return "";
}

void US_Hydrodyn_Pdb_Tool::csv_check()
{
   csv_msg( "black", us_tr( "Checking structure against residue file" ) );
   csv tmp_csv = to_csv( lv_csv, csv1, any_selected( lv_csv ) );
   vector < QString > error_keys;
   csv_msg( "dark blue", check_csv( tmp_csv, error_keys ) );
   if ( !errormsg.isEmpty() )
   {
      csv_msg( "red", errormsg );
   }
   csv_msg( "black", us_tr( "Finished checking structure" ) );
   if ( error_keys.size() )
   {
      select_these( lv_csv, error_keys );
      selection_since_count_csv1 = true;
   }
   update_enables_csv();
}

void US_Hydrodyn_Pdb_Tool::csv_sort()
{
   do_sort( lv_csv );
   update_enables_csv();
}

void US_Hydrodyn_Pdb_Tool::csv2_sort()
{
   do_sort( lv_csv2 );
   update_enables_csv2();
}

void US_Hydrodyn_Pdb_Tool::do_sort( QTreeWidget *lv )
{
   map < QString, QString > parameters;
   US_Hydrodyn_Pdb_Tool_Sort *uhpts = 
      new US_Hydrodyn_Pdb_Tool_Sort(
                                     us_hydrodyn,
                                     &parameters,
                                     this 
                                     );
   US_Hydrodyn::fixWinButtons( uhpts );
   uhpts->exec();
   delete uhpts;

   QString residuesa = 
      parameters.count( "residuesa" ) ? parameters[ "residuesa" ] : "";
   QString residuesb = 
      parameters.count( "residuesb" ) ? parameters[ "residuesb" ] : "";
   int reportcount = 
      parameters.count( "reportcount" ) ? parameters[ "reportcount" ].toInt() : 99999;
   bool order =  parameters.count( "order" );
   bool caonly =  parameters.count( "caonly" );

   if ( !residuesa.length() ||
        !residuesb.length() )
   {
      QString msg = us_tr( "Residues must be selected for sorting" );
      lv == lv_csv ?
         csv_msg( "red", msg ) : csv2_msg( "red", msg );
      return;
   }

   {
      QString msg = 
         QString( us_tr( "Residues a: %1\n" ) ).arg( residuesa )
         + QString( us_tr( "Residues b: %1\n" ) ).arg( residuesb )
         + QString( us_tr( "Report count: %1\n" ) ).arg( reportcount == 99999 ? QString( "\"All\"" ) : QString( "%1" ).arg( reportcount ) )
         + QString( us_tr( "Order: %1\n" ) ).arg( QString( us_tr( ( order ? "Maximum" : "Minimum" ) ) + " aggregate distance" ) )
         + QString( us_tr( "CA Only: %1\n" ) ).arg( caonly ? "True" : "False" )
         ;
      lv == lv_csv ?
         csv_msg( "blue", msg ) : csv2_msg( "blue", msg );
   }

   // validate residues, take selected models and compute aggregrate pairwise distance of residues and select the report count number of models
   // and report on the distances

   // expand residue strings

   set < QString > set_a;
   set < QString > set_b;
   if ( !US_Pdb_Util::range_to_set( set_a, residuesa ) ||
        !US_Pdb_Util::range_to_set( set_b, residuesb )
        )
   {
      QString msg = us_tr( "Invalid residue format" );
      lv == lv_csv ?
         csv_msg( "red", msg ) : csv2_msg( "red", msg );
      return;
   }      

   if ( !set_a.size() || !set_b.size() )
   {
      QString msg = us_tr( "Residues parsed empty" );
      lv == lv_csv ?
         csv_msg( "red", msg ) : csv2_msg( "red", msg );
      return;
   }      
      
   // us_qdebug( "set_a contains:" );
   // for ( set < QString >::iterator it = set_a.begin();
   //       it != set_a.end();
   //       it++ )
   // {
   //    us_qdebug( *it );
   // }
      
   // us_qdebug( "set_b contains:" );
   // for ( set < QString >::iterator it = set_b.begin();
   //       it != set_b.end();
   //       it++ )
   // {
   //    us_qdebug( *it );
   // }
   
   csv tmp_csv;

   if ( lv == lv_csv )
   {
      tmp_csv = to_csv( lv, csv1 );
   } else {
      tmp_csv = to_csv( lv, csv2[ csv2_pos ] );
   }

   // go through tmp_csv and find residues
   // error if not present

   QString last_model = "__first__";

   list < sortable_dist_name > model_distances;

   vector < point > pointsa;
   vector < point > pointsb;
   point t;

   for ( int i = 0; i < (int) tmp_csv.data.size(); ++i )
   {
      QString model      = tmp_csv.data[ i ][ 0 ];
      QString chain      = QString( tmp_csv.data[ i ][ 1 ] ).trimmed();
      QString residuen   = QString( tmp_csv.data[ i ][ 3 ] ).trimmed();

      if ( model != last_model )
      {
         if ( last_model != "__first__" )
         {
            // compute distance for last model
            sortable_dist_name sdn;
            sdn.name  = last_model;
            sdn.dist  = 0e0;
            sdn.pairs = 0;
            for ( int j = 0; j < (int) pointsa.size(); ++j )
            {
               for ( int k = 0; k < (int) pointsb.size(); ++k )
               {
                  sdn.pairs++;
                  sdn.dist += 
                     sqrt( 
                          ( ( pointsa[ j ].axis[ 0 ] - pointsb[ k ].axis[ 0 ] ) *
                            ( pointsa[ j ].axis[ 0 ] - pointsb[ k ].axis[ 0 ] ) )
                          +
                          ( ( pointsa[ j ].axis[ 1 ] - pointsb[ k ].axis[ 1 ] ) *
                            ( pointsa[ j ].axis[ 1 ] - pointsb[ k ].axis[ 1 ] ) )
                          +
                          ( ( pointsa[ j ].axis[ 2 ] - pointsb[ k ].axis[ 2 ] ) *
                            ( pointsa[ j ].axis[ 2 ] - pointsb[ k ].axis[ 2 ] ) )
                          );
               }
            }
            model_distances.push_back( sdn );
            pointsa.clear( );
            pointsb.clear( );
         }
         last_model = model;
      }
      QString check = QString( "%1~%2" ).arg( chain ).arg( residuen );
      // us_qdebug( QString( "check <%1> atom <%2>" ).arg( check ).arg( tmp_csv.data[ i ][ 4 ] ) );
      if ( !caonly || tmp_csv.data[ i ][ 4 ] == " CA " )
      { 
         if ( set_a.count( check ) )
         {
            t.axis[ 0 ] = tmp_csv.data[ i ][ 8 ].toFloat();
            t.axis[ 1 ] = tmp_csv.data[ i ][ 9 ].toFloat();
            t.axis[ 2 ] = tmp_csv.data[ i ][ 10 ].toFloat();
            pointsa.push_back( t );
         }
         if ( set_b.count( check ) )
         {
            t.axis[ 0 ] = tmp_csv.data[ i ][ 8 ].toFloat();
            t.axis[ 1 ] = tmp_csv.data[ i ][ 9 ].toFloat();
            t.axis[ 2 ] = tmp_csv.data[ i ][ 10 ].toFloat();
            pointsb.push_back( t );
         }
      }
   }

   if ( pointsa.size() && pointsb.size() )
   {
      // compute distance for last model
      sortable_dist_name sdn;
      sdn.name = last_model;
      sdn.dist = 0e0;
      for ( int j = 0; j < (int) pointsa.size(); ++j )
      {
         for ( int k = 0; k < (int) pointsb.size(); ++k )
         {
            sdn.dist += 
               sqrt( 
                    ( ( pointsa[ j ].axis[ 0 ] - pointsb[ k ].axis[ 0 ] ) *
                      ( pointsa[ j ].axis[ 0 ] - pointsb[ k ].axis[ 0 ] ) )
                    +
                    ( ( pointsa[ j ].axis[ 1 ] - pointsb[ k ].axis[ 1 ] ) *
                      ( pointsa[ j ].axis[ 1 ] - pointsb[ k ].axis[ 1 ] ) )
                    +
                    ( ( pointsa[ j ].axis[ 2 ] - pointsb[ k ].axis[ 2 ] ) *
                      ( pointsa[ j ].axis[ 2 ] - pointsb[ k ].axis[ 2 ] ) )
                    );
         }
      }
      model_distances.push_back( sdn );
   }

   model_distances.sort();

   if ( order )
   {
      model_distances.reverse();
   }

   if ( (int) model_distances.size() > reportcount )
   {
      model_distances.resize( reportcount );
   }

   set < QString > models;

   for ( list < sortable_dist_name >::iterator it = model_distances.begin();
         it != model_distances.end();
         it++ )
   {
      QString msg = QString( "model %1 distances %2 pairs %3" ).arg( it->name ).arg( it->dist ).arg( it->pairs );
      lv == lv_csv ?
         csv_msg( "dark blue", msg ) : csv2_msg( "dark blue", msg );
      models.insert( it->name );
   }
   select_model( lv, models );
   if ( lv == lv_csv )
   {
      selection_since_count_csv1 = true;
   } else {
      selection_since_count_csv2 = true;
   }
}

void US_Hydrodyn_Pdb_Tool::select_these( QTreeWidget *lv, vector < QString > &error_keys )
{
   lv->clearSelection();
   map < QString, bool > error_map;
   for ( unsigned int i = 0; i < (unsigned int)error_keys.size(); i++ )
   {
      error_map[ error_keys[ i ] ] = true;
      // cout << QString( "error map <%1>\n" ).arg( error_keys[ i ] );
   }

   {
      QTreeWidgetItemIterator it( lv );
      while ( (*it) ) 
      {
         QTreeWidgetItem *item = (*it);
         
         if ( error_map.count( key( item ) ) )
         {
            // cout << QString( "selecting <%1>\n" ).arg( item->text( 0 ) );
            item->setSelected( true  );
#if QT_VERSION < 0x040000
            item->setVisible ( true  );
#endif
            item->setExpanded    ( false );
            // open parents
            {
               QTreeWidgetItem *p = item;
               while ( ( p = p->parent() ) )
               {
                  p->setExpanded( true );
               }
            }
         } else {
            // cout << QString( "NOT selecting <%1>\n" ).arg( item->text( 0 ) );
         }
         ++it;
      }
   }
}

void US_Hydrodyn_Pdb_Tool::csv_find_alt()
{
   csv tmp_csv = to_csv( lv_csv, csv1, true );
   if ( !tmp_csv.data.size() )
   {
      return;
   }
   QStringList alt_residues;
   check_csv_for_alt( tmp_csv, alt_residues );
   if ( !alt_residues.size() )
   {
      csv_msg( "red" , us_tr( "No matching residues found" ) );
      return;
   }
   csv_msg( "blue", us_tr( "Matching residues: " ) + alt_residues.join( " " ) );
   
   bool self_included = false;
   // unsigned int self_pos;

   QStringList alt_residues_no_self;
   for ( unsigned int i = 0; i < (unsigned int)alt_residues.size(); i++ )
   {
      if ( tmp_csv.data[ 0 ][ 2 ].trimmed() == alt_residues[ i ] )
      {
         self_included = true;
         // self_pos = i;
      } else {
         alt_residues_no_self << alt_residues[ i ];
      }
   }

   if ( self_included && alt_residues.size() == 1 )
   {
      QMessageBox::information(
                               this,
                               us_tr( "US-SOMO: PDB Editor" ),
                               us_tr( "The only matching residue is the one already set" ),
                               QMessageBox::Ok );
      return;
   } 

   if ( self_included )
   {
      if ( QMessageBox::question(
                                 this,
                                 us_tr( "US-SOMO: PDB Editor" ),
                                 us_tr( QString( "The current residue is ok, but there %1\n"
                                               "Do you want to change the residue in the loaded pdb?" ) 
                                      .arg( alt_residues.size() > 2 ? "are alternatives" : "is an alternative" ) ),
                                 us_tr( "&Yes" ),
                                 us_tr( "&No" ),
                                 QString(), 0, 1 ) )
      {
         return;
      } else {
         if ( alt_residues.size() == 2 )
         {
            csv_undos.push_back( to_csv( lv_csv, csv1, false ) );
            csv_msg( "black", QString( us_tr( "Residue %1 replaced with %2" ) ).arg( tmp_csv.data[ 0 ][ 2 ] ).arg( alt_residues_no_self[ 0 ] ) );
            replace_selected_residues( lv_csv, csv1, tmp_csv.data[ 0 ][ 2 ], alt_residues_no_self[ 0 ] );
            update_enables_csv();
            return;
         }
      } 
   }

   bool ok;
   QString res = US_Static::getItem(
                                       us_tr( "US-SOMO: PDB Editor" ),
                                       us_tr( "Select a replacement residue\n"
                                           "or press Cancel\n" ),
                                       alt_residues_no_self, 
                                       0, 
                                       true,
                                       &ok,
                                       this );
   if ( ok ) {
      csv_undos.push_back( to_csv( lv_csv, csv1, false ) );
      replace_selected_residues( lv_csv, csv1, tmp_csv.data[ 0 ][ 2 ], res );
      csv_msg( "black", QString( us_tr( "Residue %1 replaced with %2" ) ).arg( tmp_csv.data[ 0 ][ 2 ] ).arg( res ) );
   }

   update_enables_csv();
}

void US_Hydrodyn_Pdb_Tool::csv2_check()
{
   csv2_msg( "black", us_tr( "Checking structure against residue file" ) );
   csv tmp_csv = to_csv( lv_csv2, csv2[ csv2_pos ], any_selected( lv_csv2 ) );
   vector < QString > error_keys;
   csv2_msg( "dark blue", check_csv( tmp_csv, error_keys ) );
   if ( !errormsg.isEmpty() )
   {
      csv2_msg( "red", errormsg );
   }
   csv2_msg( "black", us_tr( "Finished checking structure" ) );
   if ( error_keys.size() )
   {
      select_these( lv_csv2, error_keys );
      selection_since_count_csv2 = true;
   }
   update_enables_csv2();
}

void US_Hydrodyn_Pdb_Tool::csv2_find_alt()
{
   csv tmp_csv = to_csv( lv_csv2, csv2[ csv2_pos ], true );
   if ( !tmp_csv.data.size() )
   {
      return;
   }
   QStringList alt_residues;
   check_csv_for_alt( tmp_csv, alt_residues );
   if ( !alt_residues.size() )
   {
      csv2_msg( "red" , us_tr( "No matching residues found" ) );
      return;
   }
   csv2_msg( "blue", us_tr( "Matching residues: " ) + alt_residues.join( " " ) );
   
   bool self_included = false;
   // unsigned int self_pos;

   QStringList alt_residues_no_self;
   for ( unsigned int i = 0; i < (unsigned int)alt_residues.size(); i++ )
   {
      if ( tmp_csv.data[ 0 ][ 2 ].trimmed() == alt_residues[ i ] )
      {
         self_included = true;
         // self_pos = i;
      } else {
         alt_residues_no_self << alt_residues[ i ];
      }
   }

   if ( self_included && alt_residues.size() == 1 )
   {
      QMessageBox::information(
                               this,
                               us_tr( "US-SOMO: PDB Editor" ),
                               us_tr( "The only matching residue is the one already set" ),
                               QMessageBox::Ok );
      return;
   } 

   if ( self_included )
   {
      if ( QMessageBox::question(
                                 this,
                                 us_tr( "US-SOMO: PDB Editor" ),
                                 us_tr( QString( "The current residue is ok, but there %1\n"
                                               "Do you want to change the residue in the loaded pdb?" ) 
                                      .arg( alt_residues.size() > 2 ? "are alternatives" : "is an alternative" ) ),
                                 us_tr( "&Yes" ),
                                 us_tr( "&No" ),
                                 QString(), 0, 1 ) )
      {
         return;
      } else {
         if ( alt_residues.size() == 2 )
         {
            csv2_undos[ csv2_pos ].push_back( to_csv( lv_csv2, csv2[ csv2_pos ], false ) );
            replace_selected_residues( lv_csv2, csv2[ csv2_pos ], tmp_csv.data[ 0 ][ 2 ], alt_residues_no_self[ 0 ] );
            csv2_msg( "black", QString( us_tr( "Residue %1 replaced with %2" ) ).arg( tmp_csv.data[ 0 ][ 2 ] ).arg( alt_residues_no_self[ 0 ] ) );
            update_enables_csv2();
            return;
         }
      } 
   }

   bool ok;
   QString res = US_Static::getItem(
                                       us_tr( "US-SOMO: PDB Editor" ),
                                       us_tr( "Select a replacement residue\n"
                                           "or press Cancel\n" ),
                                       alt_residues_no_self, 
                                       0, 
                                       true,
                                       &ok,
                                       this );
   if ( ok ) {
      csv2_undos[ csv2_pos ].push_back( to_csv( lv_csv2, csv2[ csv2_pos ], false ) );
      replace_selected_residues( lv_csv2, csv2[ csv2_pos ], tmp_csv.data[ 0 ][ 2 ], res );
      csv2_msg( "black", QString( us_tr( "Residue %1 replaced with %2" ) ).arg( tmp_csv.data[ 0 ][ 2 ] ).arg( res ) );
   }

   update_enables_csv2();
}

void US_Hydrodyn_Pdb_Tool::replace_selected_residues( QTreeWidget *lv, csv &csv_use, QString from, QString to )
{
   QTreeWidgetItemIterator it( lv );
   while ( from.length() < 3 )
   {
      from += " ";
   }
   while ( to.length() < 3 )
   {
      to += " ";
   }

   while ( (*it) ) 
   {
      QTreeWidgetItem *item = (*it);
      if ( item->isSelected() &&
           // this  makes sure we are a residue
           US_Static::lvi_depth( item ) == 2 )
      {
         map < QTreeWidgetItem *, QString > previous_keys;

#if QT_VERSION < 0x040000
         if ( item->childCount() )
         {
            QTreeWidgetItem *myChild = item->firstChild();
            while ( myChild )
            {
               previous_keys[ myChild ] = key( myChild );
               myChild = myChild->nextSibling();
            }
         }
#else
         {
            int children = item->childCount();
            if ( children ) { 
               for ( int i = 0; i < children; ++i ) {
                  QTreeWidgetItem *myChild = item->child( i );
                  if ( myChild ) {
                     previous_keys[ myChild ] = key( myChild );
                  }
               }
            }
         }
#endif

         QString org_key = key( item );
         item->setText( 0,
                        QString( "%1" )
                        .arg( item->text( 0 ) )
                        .replace( QRegExp( QString( "^%1" ).arg( from ) ), to ) 
                        );
         if ( csv_use.nd_key.count( org_key ) )
         {
            csv_use.nd_key[ key( item ) ] = csv_use.nd_key[ org_key ];
            csv_use.nd_key.erase( org_key );
         } 


#if QT_VERSION < 0x040000
         if ( item->childCount() )
         {
            QTreeWidgetItem *myChild = item->firstChild();
            while ( myChild )
            {
               if ( csv_use.key.count( previous_keys[ myChild ] ) )
               {
                  if ( csv_use.data[ csv_use.key[ previous_keys[ myChild ] ] ].size() < 3 )
                  {
                     editor_msg( "red", QString( us_tr( "Internal error: insufficient data for %1" ) )
                                 .arg( previous_keys[ myChild ] ) );
                  } else {
                     csv_use.data[ csv_use.key[ previous_keys[ myChild ] ] ][ 2 ] = to;
                  }
                  csv_use.key[ key( myChild ) ] = csv_use.key[ previous_keys[ myChild ] ];
                  csv_use.key.erase( previous_keys[ myChild ] );
               } else {
                  cout << QString( "child org key:<%1> new_key:<%2>\n" ).arg( previous_keys[ myChild ] )
                     .arg( key( myChild ) );
                  editor_msg( "red", us_tr( "Internal error: expected key match 2" ) );
               }
               myChild = myChild->nextSibling();
            }
         }
#else
         {
            int children = item->childCount();
            if ( children ) { 
               for ( int i = 0; i < children; ++i ) {
                  QTreeWidgetItem *myChild = item->child( i );
                  if ( myChild ) {
                     if ( csv_use.key.count( previous_keys[ myChild ] ) )
                     {
                        if ( csv_use.data[ csv_use.key[ previous_keys[ myChild ] ] ].size() < 3 )
                        {
                           editor_msg( "red", QString( us_tr( "Internal error: insufficient data for %1" ) )
                                       .arg( previous_keys[ myChild ] ) );
                        } else {
                           csv_use.data[ csv_use.key[ previous_keys[ myChild ] ] ][ 2 ] = to;
                        }
                        csv_use.key[ key( myChild ) ] = csv_use.key[ previous_keys[ myChild ] ];
                        csv_use.key.erase( previous_keys[ myChild ] );
                     } else {
                        cout << QString( "child org key:<%1> new_key:<%2>\n" ).arg( previous_keys[ myChild ] )
                           .arg( key( myChild ) );
                        editor_msg( "red", us_tr( "Internal error: expected key match 2" ) );
                     }
                  }
               }
            }
         }
#endif

      }
      ++it;
   }

   return;
}

QString US_Hydrodyn_Pdb_Tool::check_csv_for_alt( csv &csv1, QStringList &alt_residues )
{
   alt_residues.clear( );
   if ( !usu->select_residue_file( ((US_Hydrodyn *)us_hydrodyn)->residue_filename ) )
   {
      errormsg = usu->errormsg;
      return "";
   }
   // try each residue name:
   QRegExp rx_skip(
                   "^("
                   "NPBR-OXT|"
                   "OXT-P|"
                   "PBR-G|"
                   "PBR-N|"
                   "PBR-NO-OXT|"
                   "PBR-OXT|"
                   "PBR-P"
                   ")$" );
                   

   map < QString, bool > residue_names;
   for ( unsigned int i = 0; i < (unsigned int)usu->residue_list.size(); i++ )
   {
      if ( rx_skip.indexIn( usu->residue_list[ i ].name ) == -1 )
      {
         residue_names[ usu->residue_list[ i ].name ] = true;
         // cout << QString( "residue_list: %1 %2\n" ).arg( i ).arg( usu->residue_list[ i ].name );
      }
   }

   for ( unsigned int i = 0; i < (unsigned int)csv1.data.size(); i++ )
   {
      if ( csv1.data[ i ].size() < 3 )
      {
         editor_msg( "red", us_tr( "Internal error: unexpected data length" ) );
         return "";
      }
   }

   usu->control_parameters[ "pdbmissingatoms" ] = "0";
   usu->control_parameters[ "pdbmissingresidues" ] = "0";
   usu->control_parameters[ "saxsfile"   ] =  ((US_Hydrodyn *)us_hydrodyn)->saxs_options.default_saxs_filename;
   usu->control_parameters[ "hybridfile" ] =  ((US_Hydrodyn *)us_hydrodyn)->saxs_options.default_hybrid_filename;
   usu->control_parameters[ "atomfile"   ] =  ((US_Hydrodyn *)us_hydrodyn)->saxs_options.default_atom_filename;

   vector < QString > error_keys;

   for ( map < QString, bool >::iterator it = residue_names.begin();
         it != residue_names.end();
         it++ )
   {
      csv tmp_csv = csv1;
      // cout << QString( "trying residue %1\n" ).arg( it->first );
      for ( unsigned int i = 0; i < (unsigned int)tmp_csv.data.size(); i++ )
      {
         tmp_csv.data[ i ][ 2 ] = it->first;
      }
      // now check it 

      QStringList qsl = csv_to_pdb_qsl( tmp_csv );
      // cout << QString( "read pdb qsl size %1\n" ).arg( qsl.size() );

      if ( !usu->read_pdb( qsl ) )
      {
         // cout << "can't read qsl: errormsg:" << usu->errormsg << endl;
         // errormsg = usu->errormsg;
         continue;
      }
      // cout << "read pdb:\n";
      // cout << usu->noticemsg << endl;
      // cout << QString( "model vector size %1\n" ).arg( usu->model_vector.size() );

      error_keys.clear( );
      
      for ( unsigned int i = 0; i < (unsigned int)usu->model_vector.size(); i++ )
      {
         usu->errormsg = "";
         if ( usu->check_for_missing_atoms( &usu->model_vector[ i ], qsl ) 
              && usu->errormsg.isEmpty() )
         {
            alt_residues << it->first;
         }         
      }
   }
   cout << "alt residues:" << alt_residues.join( " " ) << endl;
   return "";
}

QString US_Hydrodyn_Pdb_Tool::check_csv( csv & csv1, vector < QString > &error_keys )
{
   errormsg = "";
   QString qs;
   if ( !usu->select_residue_file( ((US_Hydrodyn *)us_hydrodyn)->residue_filename ) )
   {
      errormsg = usu->errormsg;
      return "";
   }
   if ( !usu->select_atom_file( ((US_Hydrodyn *)us_hydrodyn)->saxs_options.default_atom_filename ) )
   {
      errormsg = usu->errormsg;
      return "";
   }
   if ( !usu->select_hybrid_file( ((US_Hydrodyn *)us_hydrodyn)->saxs_options.default_hybrid_filename ) )
   {
      errormsg = usu->errormsg;
      return "";
   }
   if ( !usu->select_saxs_file( ((US_Hydrodyn *)us_hydrodyn)->saxs_options.default_saxs_filename ) )
   {
      errormsg = usu->errormsg;
      return "";
   }

   // if this is a very large pdb, it should probably go to a disk file
   QStringList qsl = csv_to_pdb_qsl( csv1 );
   // cout << QString( "read pdb qsl size %1\n" ).arg( qsl.size() );
   
   if ( !usu->read_pdb( qsl ) )
   {
      errormsg = usu->errormsg;
      return "";
   }
   // cout << "read pdb:\n";
   // cout << usu->noticemsg << endl;
   // cout << QString( "model vector size %1\n" ).arg( usu->model_vector.size() );

   errormsg += usu->noticemsg;

   usu->control_parameters[ "pdbmissingatoms" ] = QString( "%1" ).arg( ((US_Hydrodyn *)us_hydrodyn)->pdb_parse.missing_atoms );
   usu->control_parameters[ "pdbmissingresidues" ] = QString( "%1" ).arg( ((US_Hydrodyn *)us_hydrodyn)->pdb_parse.missing_residues );

   error_keys.clear( );

   for ( unsigned int i = 0; i < (unsigned int)usu->model_vector.size(); i++ )
   {
      if ( !usu->check_for_missing_atoms( &usu->model_vector[ i ], qsl ) )
      {
         errormsg += QString( us_tr( "Model %1: Errors with your PDB structure %2\n" ) )
            .arg( i + 1 ).arg( usu->errormsg );
      } else {
         qs += QString( us_tr( "Model %1: ok\n" ) ).arg( i + 1 );
      }         
      for ( map < QString, bool >::iterator it = usu->residue_errors.begin();
            it != usu->residue_errors.end();
            it++ )
      {
         // cout << "residue error key: " << it->first << endl;
         error_keys.push_back( QString( "%1~%2" ).arg( i + 1 ).arg( it->first ) );
      }
   }

   // for ( unsigned int i = 0; i < (unsigned int)error_keys.size(); i++ )
   // {
   // cout << "error key:" << error_keys[ i ] << endl;
   // }

   if ( error_keys.size() )
   {
      if ( QMessageBox::question(
                                  this,
                                  us_tr( "US-SOMO: PDB Editor" ),
                                  us_tr( "Errors were found\n"
                                      "Do you want to select the residues with errors?" ),
                                  us_tr( "&Yes" ),
                                  us_tr( "&No" ),
                                  QString(), 0, 1 ) )
      {
         // cout << "clearing error keys\n";
         error_keys.clear( );
      }
   }
      
   return qs;
}

double US_Hydrodyn_Pdb_Tool::minimum_pair_distance  ( QTreeWidget *lv,
                                                      QString    chain_1,
                                                      QString    chain_2,
                                                      QString  & key_1,
                                                      QString  & key_2 )
{
   vector < QTreeWidgetItem * > items_chain_1;
   vector < QTreeWidgetItem * > items_chain_2;

   {
      QTreeWidgetItemIterator it1( lv );
      while ( (*it1) ) 
      {
         QTreeWidgetItem *item1 = (*it1);
         if ( !item1->childCount() )
         {
            if ( chain_1 == get_chain_id( item1 ) )
            {
               items_chain_1.push_back( item1 );
            }
         }
         it1++;
      }
   }

   {
      QTreeWidgetItemIterator it2( lv );
      while ( (*it2) ) 
      {
         QTreeWidgetItem *item2 = (*it2);
         if ( !item2->childCount() )
         {
            if ( chain_2 == get_chain_id( item2 ) )
            {
               items_chain_2.push_back( item2 );
            }
         }
         it2++;
      }
   }

   lvipair min_lvp;
   min_lvp.lvi1 = (QTreeWidgetItem *)0;
   min_lvp.lvi2 = (QTreeWidgetItem *)0;

   double  min_distance = 9e99;

   for ( unsigned int i = 0; i < (unsigned int)items_chain_1.size(); i++ )
   {
      for ( unsigned int j = 0; j < (unsigned int)items_chain_2.size(); j++ )
      {
         lvipair lvp;
         lvp.lvi1 = items_chain_1[ i ];
         lvp.lvi2 = items_chain_2[ j ];
         double distance = pair_dist( lvp.lvi1, lvp.lvi2 );
         if ( ( !i && !j ) || min_distance > distance )
         {
            min_lvp      = lvp;
            min_distance = distance;
         }         
      }
   }
         
   key_1 = key( min_lvp.lvi1 );
   key_2 = key( min_lvp.lvi2 );
   update_enables();
   return min_distance;
}

void US_Hydrodyn_Pdb_Tool::split_pdb_by_residue( QFile &f )
{
   // split by residue, QFile f open;
   // build up vector of residues in 1st model

   QRegExp rx_model("^MODEL");
   QRegExp rx_end  ("^END");
   QRegExp rx_atom ("^("
                    "ATOM|"
                    "HETATM"
                    ")" );
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

   editor_msg( "dark blue", QString( us_tr( "Checking file %1" ).arg( f.fileName() ) ) );

   vector < QString >                       residue_atoms;
   vector < vector < QString > >            residues;
   vector < vector < vector < QString > > > chain_residues;
   
   QString last_resSeq           = "";
   QString last_chainId          = "";
   unsigned int max_chain_length = 0;

   QString                  model_header;

   bool skip_waters = 
      QMessageBox::question( this, 
                             us_tr( "US-SOMO: PDB Editor : Split by residue" ),
                             QString( us_tr( "Do you want to skip waters?" ) ),
                             us_tr( "&Yes" ), 
                             us_tr( "&No" ),
                             QString(),
                             1
                             ) == 0;

   bool waters_as_onezies = skip_waters ? false :
      ( QMessageBox::question( this, 
                             us_tr( "US-SOMO: PDB Editor : Split by residue" ),
                             QString( us_tr( "Do you want to keep the waters as singletons?" ) ),
                             us_tr( "&Yes" ), 
                             us_tr( "&No" ),
                             QString(),
                             0
                             ) == 0 );
      

   unsigned int water_count = 0;

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
            model_header += qs + "\n";
         }

         if ( qs.contains( rx_end ) )
         {
            break;
         }

         if ( qs.contains( rx_atom ) )
         {
            QString resName = qs.mid( 17 , 3 ).trimmed();
            bool is_water = ( resName == "HOH" || resName == "WAT" );
            if ( !skip_waters || !is_water )
            {
               if ( is_water && waters_as_onezies )
               {
                  water_count++;
                  if ( water_count > 1 )
                  {
                     continue;
                  }
               }

               QString resSeq  = qs.mid( 22 , 4 ).trimmed();
               QString chainId = qs.mid( 21 , 1 );
               if ( resSeq  != last_resSeq ||
                    chainId != last_chainId ||
                    ( waters_as_onezies && is_water ) )
               {
                  if ( residue_atoms.size() )
                  {
                     residues.push_back( residue_atoms );
                     residue_atoms.clear( );
                  }
                  last_resSeq = resSeq;
               
                  if ( chainId != last_chainId )
                  {
                     if ( residues.size() )
                     {
                        chain_residues.push_back( residues );
                        if ( max_chain_length < (unsigned int)residues.size() )
                        {
                           max_chain_length = (unsigned int)residues.size();
                        }
                        residues.clear( );
                     }
                     last_chainId = chainId;
                  }
               }               
               residue_atoms.push_back( qs );
            }
         }
      }
      if ( residue_atoms.size() )
      {
         residues.push_back( residue_atoms );
         chain_residues.push_back( residues );
         if ( max_chain_length < (unsigned int)residues.size() )
         {
            max_chain_length = (unsigned int)residues.size();
         }
         residues.clear( );
         residue_atoms.clear( );
      }
   }
   f.close();

   //    for ( unsigned int i = 0; i < ( unsigned int ) chain_residues.size(); i++ )
   //    {
   //       for ( unsigned int j = 0; j < ( unsigned int ) chain_residues[ i ].size(); j++ )
   //       {
   //          cout << QString( "split_by_residue(): chain %1 residue %2 atoms %2\n" )
   //             .arg( i )
   //             .arg( j )
   //             .arg( chain_residues[ i ][ j ].size() );
   //       }
   //    }

   unsigned int step_size   = 0;
   unsigned int window_size = 0;

   bool ok = false;
   do 
   {
      window_size = ( unsigned int )US_Static::getInteger(
                                                             us_tr( "US-SOMO: PDB Editor : Split by residue" ),
                                                             QString( us_tr( "The maximum chain length in residues is %1 in the 1st model in the PDB file %2\n"
                                                                          "Enter the number of resdues per output model:" ) )
                                                             .arg( max_chain_length )
                                                             .arg( f.fileName() ),
                                                             1,
                                                             1, 
                                                             max_chain_length / 2,
                                                             1,
                                                             &ok, 
                                                             this );
      if ( ok )
      {
         step_size = ( unsigned int )US_Static::getInteger(
                                                              us_tr( "US-SOMO: PDB Editor : Split by residue" ),
                                                              QString( us_tr( "The maximum chain length in residues is %1 in the 1st model in the PDB file %2\n"
                                                                           "The number of resdues per output model is %3\n"
                                                                           "Enter the step size:" ) )
                                                              .arg( max_chain_length )
                                                              .arg( f.fileName() )
                                                              .arg( window_size ),
                                                              1,
                                                              1, 
                                                              window_size,
                                                              1,
                                                              &ok, 
                                                              this );
      }
      if ( ok )
      {
         switch ( QMessageBox::question(
                                        this,
                                        us_tr( "US-SOMO: PDB Editor : Split by residue" ),
                                        QString( us_tr( "The maximum chain length in residues is %1 in the 1st model in the PDB file %2\n"
                                                     "The number of residues per model is %3\n"
                                                     "The step size is %4\n"
                                                     ) )
                                        .arg( max_chain_length )
                                        .arg( f.fileName() )
                                        .arg( window_size )
                                        .arg( step_size )
                                        ,
                                        us_tr( "&Re-enter values" ), 
                                        us_tr( "&Proceed" ),
                                        us_tr( "&Cancel" ),
                                        0, 
                                        2 ) 
                  )
         {
         case 0: // reenter
            ok = false;
            break;
         case 1:
            ok = true;
            break;
         case 2:
            return;
         }
      }
   } while( !ok );

   QString fn = f.fileName().replace(QRegExp("\\.(pdb|PDB)$"),"") + QString( "_ws%1_ss%2%3%4_sr.pdb" ).arg( window_size ).arg( step_size ).arg( skip_waters ? "_nw" : "" ).arg( waters_as_onezies ? "_ww" : "" );

   fn = QFileDialog::getSaveFileName( this , us_tr( "Choose a name to save the multiple model output file" ) , fn , "PDB (*.pdb *.PDB)" );

   
   if ( fn.isEmpty() )
   {
      pb_split_pdb->setEnabled( true );
      return;
   }
   
   if ( QFile::exists( fn ) && 
        !((US_Hydrodyn *)us_hydrodyn)->overwrite )
   {
      fn = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( fn, 0, this );
   }
                  
   QFile fn_out( fn );

   if ( !fn_out.open( QIODevice::WriteOnly ) )
   {
      QMessageBox::warning( this, "US-SOMO: PDB Editor : Split by residue",
                            QString(us_tr("Could not open %1 for writing!")).arg( fn ) );
      return;
   }

   unsigned int model_count = 1;
   
   QTextStream tso( &fn_out );

   tso << QString("HEADER    split by residue from %1: window size %2 step size %3\n")
      .arg( f.fileName() )
      .arg( window_size )
      .arg( step_size )
      ;

   tso << model_header;

   bool water_done = false;

   for ( unsigned int i = 0; i < ( unsigned int ) chain_residues.size(); i++ )
   {
      for ( int j = 1 - window_size; j < ( int ) chain_residues[ i ].size(); j += step_size )
      {
         // create a model
         QString qs_this_model;
         qs_this_model += QString( "MODEL     %1\n" ).arg( model_count );
         unsigned int model_lines = 0;
         for ( int k = j; k < j + ( int ) window_size && k < ( int ) chain_residues[ i ].size(); k++ )
         {
            if ( k >= 0 )
            {
               for ( unsigned int l = 0; l < ( unsigned int ) chain_residues[ i ][ k ].size(); l++ )
               {
                  if ( waters_as_onezies )
                  {
                     QString resName = chain_residues[ i ][ k ][ l ].mid( 17 , 3 ).trimmed();
                     if ( resName == "HOH" || resName == "WAT" )
                     {
                        if ( !water_done )
                        {
                           {
                              tso << QString( "REMARK Multiply water Iq by %1\n" ).arg( water_count * window_size );
                           }
                           water_done = true;
                        } else {
                           break;
                        }
                     }
                  }
                  
                  qs_this_model += chain_residues[ i ][ k ][ l ] + "\n";
                  model_lines++;
               }
            }
         }
         if ( model_lines )
         {
            tso << qs_this_model;
            tso << "TER\n";
            tso << "ENDMDL\n";
            model_count++;
         }
      }
   }

   tso << "END\n";

   fn_out.close();
   QMessageBox::information( this, 
                             us_tr( "US-SOMO: PDB Editor : Split by residue" ),
                             QString( us_tr( "Created file %1 with %2 models" ) )
                             .arg( fn_out.fileName() )
                             .arg( model_count ) );

   return;
}

void US_Hydrodyn_Pdb_Tool::hide_widgets( vector < QWidget * > w, bool hide )
{
   for ( unsigned int i = 0; i < ( unsigned int )w.size(); i++ )
   {
      hide ? w[ i ]->hide() : w[ i ]->show();
   }
}

void US_Hydrodyn_Pdb_Tool::hide_csv()
{
   hide_widgets( panel1_widgets, panel1_widgets[ 0 ]->isVisible() );
}

void US_Hydrodyn_Pdb_Tool::hide_csv2()
{
   hide_widgets( panel2_widgets, panel2_widgets[ 0 ]->isVisible() );
}

void US_Hydrodyn_Pdb_Tool::csv_bm()
{
   do_bm( lv_csv );
   update_enables_csv();
}

void US_Hydrodyn_Pdb_Tool::csv2_bm()
{
   do_bm( lv_csv2 );
   update_enables_csv2();
}

void US_Hydrodyn_Pdb_Tool::do_bm( QTreeWidget *lv )
{
   // 1st find atoms

   bool ok;
   double psv = US_Static::getDouble(
                                        us_tr( "US-SOMO: PDB editor : Bead model from atoms" )
                                        , QString( us_tr( "Enter an estimated PSV" ) )
                                        , 7e-1
                                        , 1e-3
                                        , 2e0
                                        , 3
                                        , &ok
                                        , this 
                                        );
   if ( !ok )
   {
      return;
   }

   double expansion = US_Static::getDouble(
                                              us_tr( "US-SOMO: PDB editor : Bead model from atoms" )
                                              , QString( us_tr( "Enter a vDW multiplier:" ) )
                                              , 1
                                              , 5e-1 
                                              , 5e1
                                              , 4
                                              , &ok
                                              , this 
                                              );

   if ( !ok )
   {
      return;
   }

   csv csv_to_save;
   if ( lv == lv_csv )
   {
      csv_to_save = to_csv( lv_csv, csv1 );
   } else {
      csv_to_save = to_csv( lv_csv2, csv2[ csv2_pos ] );
   }

   QStringList out;

   QTreeWidgetItemIterator it1( lv );
   while ( (*it1) ) 
   {
      QTreeWidgetItem *item1 = (*it1);
      if ( !item1->childCount() && is_selected( item1 ) )
      {
         //item1->text( 3 ).toFloat();
         //item1->text( 4 ).toFloat();
         //item1->text( 5 ).toFloat();
         QString atom = item1->text( 7 ).trimmed().toUpper();
         if ( !usu->atom_mw.count( atom ) )
         {
            QString msg = QString( us_tr( "Atom '%1' not found in loaded molecular weights (mw.json)" ) ).arg( atom );
            lv == lv_csv ?
               csv_msg( "red", msg ) : csv2_msg( "red", msg );
            return;
         }
         if ( !usu->atom_vdw.count( atom ) )
         {
            QString msg = QString( us_tr( "Atom '%1' not found in loaded molecular vdw radii (vdw.json)" ) ).arg( atom );
            lv == lv_csv ?
               csv_msg( "red", msg ) : csv2_msg( "red", msg );
            return;
         }
         out << 
            QString( "%1 %2 %3 %4 %5 1 %6 0" )
            .arg( item1->text( 3 ) )
            .arg( item1->text( 4 ) )
            .arg( item1->text( 5 ) )
            .arg( usu->atom_vdw[ atom ] * expansion )
            .arg( usu->atom_mw[ atom ] )
            .arg(
                 item1->parent()->text(0).trimmed().replace( QRegExp( "\\s+" ), ":" ) + 
                 ":" + 
                 item1->text( 0 ).trimmed().replace( QRegExp( "\\s+" ), ":" )
                 )
            ;
      }
      ++it1;
   }

   // take the atoms and ask for inflation & make a bead model
   QString use_dir;
   ((US_Hydrodyn *)us_hydrodyn)->select_from_directory_history( use_dir, this );

   QString filename = QFileDialog::getSaveFileName( this , us_tr("Choose a filename to save the bead model") , use_dir + QDir::separator() + QFileInfo( 
                                                                                           lv == lv_csv ?
                                                                                           csv1.name :
                                                                                           csv2[ csv2_pos ].name ).baseName() , "*.bead_model *.BEAD_MODEL" );


   if ( filename.isEmpty() )
   {
      return;
   }

   filename.replace( QRegExp("(_e\\d+_\\d+|)\\.bead_model$", Qt::CaseInsensitive ), "" );
   if ( expansion != 1e0 )
   {
      filename += QString( "_e%1" ).arg( expansion, 0, 'f', 4 ).replace( ".", "_" );
   }
   filename += ".bead_model";

   if ( QFile::exists(filename) )
   {
      filename = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( filename, 0, this );
   }

   QFile f(filename);

   if ( !f.open( QIODevice::WriteOnly ) )
   {
      QMessageBox::warning( this, windowTitle(),
                            QString( us_tr("Could not open %1 for writing!") ).arg(filename) );
      return;
   }

   QTextStream ts( &f );
   ts << QString( "%1 %2\n" ).arg( out.size() ).arg( psv, 0, 'f', 3 );
   ts << out.join("\n") << Qt::endl;
   f.close();
   {

      QString msg = QString( us_tr( "Created beam model output file %1" ) ).arg( f.fileName() );
      lv == lv_csv ?
         csv_msg( "blue", msg ) : csv2_msg( "blue", msg );
   }
   return;
}

