#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
//Added by qt3to4:
#include <Q3BoxLayout>
#include <QLabel>
#include <QCloseEvent>
#include <Q3GridLayout>
#include <Q3TextStream>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <Q3Frame>
#include <Q3PopupMenu>

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

#define SLASH QDir::separator()

class sortable_qli_double {
public:
   double         d;
   Q3ListViewItem  *lvi;
   bool operator < (const sortable_qli_double& objIn) const
   {
      return ( d < objIn.d );
   }
};

class lvipair
{
public:
   Q3ListViewItem *lvi1;
   Q3ListViewItem *lvi2;
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

double US_Hydrodyn_Pdb_Tool::pair_dist( Q3ListViewItem *item1, Q3ListViewItem *item2 )
{
   double dx = item1->text( 3 ).toDouble() - item2->text( 3 ).toDouble();
   double dy = item1->text( 4 ).toDouble() - item2->text( 4 ).toDouble();
   double dz = item1->text( 5 ).toDouble() - item2->text( 5 ).toDouble();

   return sqrt( dx * dx + dy * dy + dz * dz );
}

US_Hydrodyn_Pdb_Tool::US_Hydrodyn_Pdb_Tool(
                                           csv csv1,
                                           void *us_hydrodyn, 
                                           QWidget *p, 
                                           const char *name
                                           ) : Q3Frame(p, name)
{
   this->csv1 = csv1;
   this->us_hydrodyn = us_hydrodyn;
   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setCaption( tr("US-SOMO: PDB editor") );
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
      // qDebug( "no mw.json load" );
      bm_active = false;
   }

   if ( 
       !usu->load_vdw_json( USglobal->config_list.system_dir + 
                            QDir::separator() + "etc" +
                            QDir::separator() + "vdw.json" ) )
   {
      // qDebug( "no vdw.json load" );
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
   lbl_title->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Raised);
   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette( PALET_FRAME );
   AUTFBACK( lbl_title );
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   // left pane

   editor = new Q3TextEdit(this);
   editor->setPalette( PALET_NORMAL );
   AUTFBACK( editor );
   editor->setReadOnly(true);

   Q3Frame *frame = new Q3Frame(this);
   frame->setMinimumHeight(minHeight1);

   m = new QMenuBar(frame, "menu" );
   m->setMinimumHeight(minHeight1 - 5);
   m->setPalette( PALET_NORMAL );
   AUTFBACK( m );
   Q3PopupMenu * file = new Q3PopupMenu(editor);
   m->insertItem( tr("&File"), file );
   file->insertItem( tr("Font"),  this, SLOT(update_font()),    Qt::ALT+Qt::Key_F );
   file->insertItem( tr("Save"),  this, SLOT(save()),    Qt::ALT+Qt::Key_S );
   file->insertItem( tr("Clear Display"), this, SLOT(clear_display()),   Qt::ALT+Qt::Key_X );
   editor->setWordWrap (Q3TextEdit::WidgetWidth);
   editor->setMinimumHeight(300);

   pb_split_pdb = new QPushButton(tr("Split"), this);
   pb_split_pdb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_split_pdb->setMinimumHeight(minHeight1);
   pb_split_pdb->setPalette( PALET_PUSHB );
   connect(pb_split_pdb, SIGNAL(clicked()), SLOT(split_pdb()));

   pb_join_pdbs = new QPushButton(tr("Join"), this);
   pb_join_pdbs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_join_pdbs->setMinimumHeight(minHeight1);
   pb_join_pdbs->setPalette( PALET_PUSHB );
   pb_join_pdbs->setEnabled( true );
   connect(pb_join_pdbs, SIGNAL(clicked()), SLOT(join_pdbs()));

   pb_merge = new QPushButton(tr("Cut/Splice"), this);
   pb_merge->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_merge->setMinimumHeight(minHeight1);
   pb_merge->setPalette( PALET_PUSHB );
   connect(pb_merge, SIGNAL(clicked()), SLOT(merge()));

   pb_renum_pdb = new QPushButton(tr("Renumber"), this);
   pb_renum_pdb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_renum_pdb->setMinimumHeight(minHeight1);
   pb_renum_pdb->setPalette( PALET_PUSHB );
   connect(pb_renum_pdb, SIGNAL(clicked()), SLOT(renum_pdb()));

   pb_hybrid_split = new QPushButton(tr("Hybrid extract"), this);
   pb_hybrid_split->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_hybrid_split->setMinimumHeight(minHeight1);
   pb_hybrid_split->setPalette( PALET_PUSHB );
   connect(pb_hybrid_split, SIGNAL(clicked()), SLOT(hybrid_split()));

   pb_h_to_chainX = new QPushButton(tr("H to chain X"), this);
   pb_h_to_chainX->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_h_to_chainX->setMinimumHeight(minHeight1);
   pb_h_to_chainX->setPalette( PALET_PUSHB );
   connect(pb_h_to_chainX, SIGNAL(clicked()), SLOT(h_to_chainX()));

   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_cancel = new QPushButton(tr("Close"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   // center pane

   lbl_csv = new mQLabel(tr("Panel 1"), this);
   lbl_csv->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_csv->setMinimumHeight(minHeight1);
   lbl_csv->setPalette( PALET_LABEL );
   AUTFBACK( lbl_csv );
   lbl_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   connect( lbl_csv, SIGNAL( pressed() ), SLOT( hide_csv() ) );

   lv_csv = new Q3ListView( this );
   lv_csv->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Raised);
   lv_csv->setPalette( PALET_EDIT );
   AUTFBACK( lv_csv );
   lv_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lv_csv->setEnabled(true);
   lv_csv->setMinimumWidth( 175 );

   lv_csv->addColumn( "Models" );
   for ( unsigned int i = 6; i < (unsigned int)csv1.header.size() - 2; i++ )
   {
      lv_csv->addColumn( csv1.header[i] );
   }
   lv_csv->addColumn( csv1.header.back() );

   lv_csv->setSorting        ( -1 );
   lv_csv->setRootIsDecorated( true );
   lv_csv->setSelectionMode  ( Q3ListView::Multi );
   connect(lv_csv, SIGNAL(selectionChanged()), SLOT(csv_selection_changed()));

   panel1_widgets.push_back( lv_csv );

   lbl_csv->setText( csv1.name.isEmpty() ? tr( "Panel 1" ) : csv1.name );
   csv_to_lv( csv1, lv_csv );

   te_csv = new Q3TextEdit(this);
   te_csv->setPalette( PALET_NORMAL );
   AUTFBACK( te_csv );
   te_csv->setReadOnly(true);
   te_csv->setMaximumHeight( minHeight1 * 4 );
   
   panel1_widgets.push_back( te_csv );

   pb_csv_load_1 = new QPushButton(tr("Load"), this);
   pb_csv_load_1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_load_1->setMinimumHeight(minHeight1);
   pb_csv_load_1->setPalette( PALET_PUSHB );
   connect(pb_csv_load_1, SIGNAL(clicked()), SLOT(csv_load_1()));

   panel1_widgets.push_back( pb_csv_load_1 );

   pb_csv_load = new QPushButton(tr("Load All"), this);
   pb_csv_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_load->setMinimumHeight(minHeight1);
   pb_csv_load->setPalette( PALET_PUSHB );
   connect(pb_csv_load, SIGNAL(clicked()), SLOT(csv_load()));

   panel1_widgets.push_back( pb_csv_load );

   pb_csv_visualize = new QPushButton(tr("Visualize"), this);
   pb_csv_visualize->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_visualize->setMinimumHeight(minHeight1);
   pb_csv_visualize->setPalette( PALET_PUSHB );
   connect(pb_csv_visualize, SIGNAL(clicked()), SLOT(csv_visualize()));

   panel1_widgets.push_back( pb_csv_visualize );

   pb_csv_save = new QPushButton(tr("Save"), this);
   pb_csv_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_save->setMinimumHeight(minHeight1);
   pb_csv_save->setPalette( PALET_PUSHB );
   connect(pb_csv_save, SIGNAL(clicked()), SLOT(csv_save()));

   panel1_widgets.push_back( pb_csv_save );

   pb_csv_undo = new QPushButton(tr("Undo"), this);
   pb_csv_undo->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_undo->setMinimumHeight(minHeight1);
   pb_csv_undo->setPalette( PALET_PUSHB );
   connect(pb_csv_undo, SIGNAL(clicked()), SLOT(csv_undo()));

   panel1_widgets.push_back( pb_csv_undo );

   pb_csv_clear = new QPushButton(tr("Clear"), this);
   pb_csv_clear->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_clear->setMinimumHeight(minHeight1);
   pb_csv_clear->setPalette( PALET_PUSHB );
   connect(pb_csv_clear, SIGNAL(clicked()), SLOT(csv_clear()));

   panel1_widgets.push_back( pb_csv_clear );

   pb_csv_cut = new QPushButton(tr("Cut"), this);
   pb_csv_cut->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_cut->setMinimumHeight(minHeight1);
   pb_csv_cut->setPalette( PALET_PUSHB );
   connect(pb_csv_cut, SIGNAL(clicked()), SLOT(csv_cut()));

   panel1_widgets.push_back( pb_csv_cut );

   pb_csv_copy = new QPushButton(tr("Copy"), this);
   pb_csv_copy->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_copy->setMinimumHeight(minHeight1);
   pb_csv_copy->setPalette( PALET_PUSHB );
   connect(pb_csv_copy, SIGNAL(clicked()), SLOT(csv_copy()));

   panel1_widgets.push_back( pb_csv_copy );

   pb_csv_paste = new QPushButton(tr("Paste"), this);
   pb_csv_paste->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_paste->setMinimumHeight(minHeight1);
   pb_csv_paste->setPalette( PALET_PUSHB );
   connect(pb_csv_paste, SIGNAL(clicked()), SLOT(csv_paste()));

   panel1_widgets.push_back( pb_csv_paste );

   pb_csv_paste_new = new QPushButton(tr("Paste as new"), this);
   pb_csv_paste_new->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_paste_new->setMinimumHeight(minHeight1);
   pb_csv_paste_new->setPalette( PALET_PUSHB );
   connect(pb_csv_paste_new, SIGNAL(clicked()), SLOT(csv_paste_new()));

   panel1_widgets.push_back( pb_csv_paste_new );

   pb_csv_merge = new QPushButton(tr("Merge"), this);
   pb_csv_merge->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_merge->setMinimumHeight(minHeight1);
   pb_csv_merge->setPalette( PALET_PUSHB );
   connect(pb_csv_merge, SIGNAL(clicked()), SLOT(csv_merge()));

   panel1_widgets.push_back( pb_csv_merge );

   pb_csv_angle = new QPushButton(tr("Angle"), this);
   pb_csv_angle->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_angle->setMinimumHeight(minHeight1);
   pb_csv_angle->setPalette( PALET_PUSHB );
   connect(pb_csv_angle, SIGNAL(clicked()), SLOT(csv_angle()));

   panel1_widgets.push_back( pb_csv_angle );

   pb_csv_reseq = new QPushButton(tr("Reseq"), this);
   pb_csv_reseq->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_reseq->setMinimumHeight(minHeight1);
   pb_csv_reseq->setPalette( PALET_PUSHB );
   connect(pb_csv_reseq, SIGNAL(clicked()), SLOT(csv_reseq()));

   panel1_widgets.push_back( pb_csv_reseq );

   pb_csv_check = new QPushButton(tr("Check"), this);
   pb_csv_check->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_check->setMinimumHeight(minHeight1);
   pb_csv_check->setPalette( PALET_PUSHB );
   connect(pb_csv_check, SIGNAL(clicked()), SLOT(csv_check()));

   panel1_widgets.push_back( pb_csv_check );

   pb_csv_sort = new QPushButton(tr("Sort"), this);
   pb_csv_sort->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_sort->setMinimumHeight(minHeight1);
   pb_csv_sort->setPalette( PALET_PUSHB );
   connect(pb_csv_sort, SIGNAL(clicked()), SLOT(csv_sort()));

   panel1_widgets.push_back( pb_csv_sort );

   pb_csv_find_alt = new QPushButton(tr("Find alternate matching residues"), this);
   pb_csv_find_alt->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_find_alt->setMinimumHeight(minHeight1);
   pb_csv_find_alt->setPalette( PALET_PUSHB );
   connect(pb_csv_find_alt, SIGNAL(clicked()), SLOT(csv_find_alt()));

   panel1_widgets.push_back( pb_csv_find_alt );

   pb_csv_bm = new QPushButton(tr("BM"), this);
   pb_csv_bm->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_bm->setMinimumHeight(minHeight1);
   pb_csv_bm->setPalette( PALET_PUSHB );
   connect(pb_csv_bm, SIGNAL(clicked()), SLOT(csv_bm()));

   panel1_widgets.push_back( pb_csv_bm );

   pb_csv_clash_report = new QPushButton(tr("PW distance"), this);
   pb_csv_clash_report->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_clash_report->setMinimumHeight(minHeight1);
   pb_csv_clash_report->setPalette( PALET_PUSHB );
   connect(pb_csv_clash_report, SIGNAL(clicked()), SLOT(csv_clash_report()));

   panel1_widgets.push_back( pb_csv_clash_report );

   pb_csv_sel = new QPushButton(tr("Select"), this);
   pb_csv_sel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_sel->setMinimumHeight(minHeight1);
   pb_csv_sel->setPalette( PALET_PUSHB );
   connect(pb_csv_sel, SIGNAL(clicked()), SLOT(csv_sel()));

   panel1_widgets.push_back( pb_csv_sel );

   pb_csv_sel_clear = new QPushButton(tr("Clear"), this);
   pb_csv_sel_clear->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_sel_clear->setMinimumHeight(minHeight1);
   pb_csv_sel_clear->setPalette( PALET_PUSHB );
   connect(pb_csv_sel_clear, SIGNAL(clicked()), SLOT(csv_sel_clear()));

   panel1_widgets.push_back( pb_csv_sel_clear );

   pb_csv_sel_clean = new QPushButton(tr("Clean"), this);
   pb_csv_sel_clean->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_sel_clean->setMinimumHeight(minHeight1);
   pb_csv_sel_clean->setPalette( PALET_PUSHB );
   connect(pb_csv_sel_clean, SIGNAL(clicked()), SLOT(csv_sel_clean()));

   panel1_widgets.push_back( pb_csv_sel_clean );

   pb_csv_sel_invert = new QPushButton(tr("Invert"), this);
   pb_csv_sel_invert->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_sel_invert->setMinimumHeight(minHeight1);
   pb_csv_sel_invert->setPalette( PALET_PUSHB );
   connect(pb_csv_sel_invert, SIGNAL(clicked()), SLOT(csv_sel_invert()));

   panel1_widgets.push_back( pb_csv_sel_invert );

   pb_csv_sel_chain = new QPushButton(tr("Chain"), this);
   pb_csv_sel_chain->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_sel_chain->setMinimumHeight(minHeight1);
   pb_csv_sel_chain->setPalette( PALET_PUSHB );
   connect(pb_csv_sel_chain, SIGNAL(clicked()), SLOT(csv_sel_chain()));

   panel1_widgets.push_back( pb_csv_sel_chain );

   pb_csv_sel_nearest_atoms = new QPushButton(tr("Nearest Atoms"), this);
   pb_csv_sel_nearest_atoms->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv_sel_nearest_atoms->setMinimumHeight(minHeight1);
   pb_csv_sel_nearest_atoms->setPalette( PALET_PUSHB );
   connect(pb_csv_sel_nearest_atoms, SIGNAL(clicked()), SLOT(csv_sel_nearest_atoms()));

   panel1_widgets.push_back( pb_csv_sel_nearest_atoms );

   pb_csv_sel_nearest_residues = new QPushButton(tr("Nearest Residues"), this);
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

   lbl_csv2 = new mQLabel(tr("Panel 2"), this);
   lbl_csv2->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_csv2->setMinimumHeight(minHeight1);
   lbl_csv2->setPalette( PALET_LABEL );
   AUTFBACK( lbl_csv2 );
   lbl_csv2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   connect( lbl_csv2, SIGNAL( pressed() ), SLOT( hide_csv2() ) );

   lv_csv2 = new Q3ListView( this );
   lv_csv2->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Raised);
   lv_csv2->setPalette( PALET_EDIT );
   AUTFBACK( lv_csv2 );
   lv_csv2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lv_csv2->setEnabled(true);
   lv_csv2->setMinimumWidth( 175 );

   lv_csv2->addColumn( "Models" );
   for ( unsigned int i = 6; i < (unsigned int)csv1.header.size(); i++ )
   {
      lv_csv2->addColumn( csv1.header[i] );
   }

   lv_csv2->setSorting        ( -1 );
   lv_csv2->setRootIsDecorated( true );
   lv_csv2->setSelectionMode  ( Q3ListView::Multi );
   connect(lv_csv2, SIGNAL(selectionChanged()), SLOT(csv2_selection_changed()));

   panel2_widgets.push_back( lv_csv2 );

   te_csv2 = new Q3TextEdit(this);
   te_csv2->setPalette( PALET_NORMAL );
   AUTFBACK( te_csv2 );
   te_csv2->setReadOnly(true);
   te_csv2->setMaximumHeight( minHeight1 * 4 );

   panel2_widgets.push_back( te_csv2 );

   pb_csv2_load_1 = new QPushButton(tr("Load"), this);
   pb_csv2_load_1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_load_1->setMinimumHeight(minHeight1);
   pb_csv2_load_1->setPalette( PALET_PUSHB );
   connect(pb_csv2_load_1, SIGNAL(clicked()), SLOT(csv2_load_1()));

   panel2_widgets.push_back( pb_csv2_load_1 );

   pb_csv2_load = new QPushButton(tr("Load All"), this);
   pb_csv2_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_load->setMinimumHeight(minHeight1);
   pb_csv2_load->setPalette( PALET_PUSHB );
   connect(pb_csv2_load, SIGNAL(clicked()), SLOT(csv2_load()));

   panel2_widgets.push_back( pb_csv2_load );

   pb_csv2_visualize = new QPushButton(tr("Visualize"), this);
   pb_csv2_visualize->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_visualize->setMinimumHeight(minHeight1);
   pb_csv2_visualize->setPalette( PALET_PUSHB );
   connect(pb_csv2_visualize, SIGNAL(clicked()), SLOT(csv2_visualize()));

   panel2_widgets.push_back( pb_csv2_visualize );

   pb_csv2_dup = new QPushButton(tr("Dup"), this);
   pb_csv2_dup->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_dup->setMinimumHeight(minHeight1);
   pb_csv2_dup->setPalette( PALET_PUSHB );
   connect(pb_csv2_dup, SIGNAL(clicked()), SLOT(csv2_dup()));

   panel2_widgets.push_back( pb_csv2_dup );

   pb_csv2_save = new QPushButton(tr("Save"), this);
   pb_csv2_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_save->setMinimumHeight(minHeight1);
   pb_csv2_save->setPalette( PALET_PUSHB );
   connect(pb_csv2_save, SIGNAL(clicked()), SLOT(csv2_save()));

   panel2_widgets.push_back( pb_csv2_save );

   pb_csv2_undo = new QPushButton(tr("Undo"), this);
   pb_csv2_undo->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_undo->setMinimumHeight(minHeight1);
   pb_csv2_undo->setPalette( PALET_PUSHB );
   connect(pb_csv2_undo, SIGNAL(clicked()), SLOT(csv2_undo()));

   panel2_widgets.push_back( pb_csv2_undo );

   pb_csv2_clear = new QPushButton(tr("Clear"), this);
   pb_csv2_clear->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_clear->setMinimumHeight(minHeight1);
   pb_csv2_clear->setPalette( PALET_PUSHB );
   connect(pb_csv2_clear, SIGNAL(clicked()), SLOT(csv2_clear()));

   panel2_widgets.push_back( pb_csv2_clear );

   pb_csv2_cut = new QPushButton(tr("Cut"), this);
   pb_csv2_cut->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_cut->setMinimumHeight(minHeight1);
   pb_csv2_cut->setPalette( PALET_PUSHB );
   connect(pb_csv2_cut, SIGNAL(clicked()), SLOT(csv2_cut()));

   panel2_widgets.push_back( pb_csv2_cut );

   pb_csv2_copy = new QPushButton(tr("Copy"), this);
   pb_csv2_copy->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_copy->setMinimumHeight(minHeight1);
   pb_csv2_copy->setPalette( PALET_PUSHB );
   connect(pb_csv2_copy, SIGNAL(clicked()), SLOT(csv2_copy()));

   panel2_widgets.push_back( pb_csv2_copy );

   pb_csv2_paste = new QPushButton(tr("Paste"), this);
   pb_csv2_paste->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_paste->setMinimumHeight(minHeight1);
   pb_csv2_paste->setPalette( PALET_PUSHB );
   connect(pb_csv2_paste, SIGNAL(clicked()), SLOT(csv2_paste()));

   panel2_widgets.push_back( pb_csv2_paste );

   pb_csv2_paste_new = new QPushButton(tr("Paste as new"), this);
   pb_csv2_paste_new->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_paste_new->setMinimumHeight(minHeight1);
   pb_csv2_paste_new->setPalette( PALET_PUSHB );
   connect(pb_csv2_paste_new, SIGNAL(clicked()), SLOT(csv2_paste_new()));

   panel2_widgets.push_back( pb_csv2_paste_new );

   pb_csv2_merge = new QPushButton(tr("Merge"), this);
   pb_csv2_merge->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_merge->setMinimumHeight(minHeight1);
   pb_csv2_merge->setPalette( PALET_PUSHB );
   connect(pb_csv2_merge, SIGNAL(clicked()), SLOT(csv2_merge()));

   panel2_widgets.push_back( pb_csv2_merge );

   pb_csv2_angle = new QPushButton(tr("Angle"), this);
   pb_csv2_angle->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_angle->setMinimumHeight(minHeight1);
   pb_csv2_angle->setPalette( PALET_PUSHB );
   connect(pb_csv2_angle, SIGNAL(clicked()), SLOT(csv2_angle()));

   panel2_widgets.push_back( pb_csv2_angle );

   pb_csv2_reseq = new QPushButton(tr("Reseq"), this);
   pb_csv2_reseq->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_reseq->setMinimumHeight(minHeight1);
   pb_csv2_reseq->setPalette( PALET_PUSHB );
   connect(pb_csv2_reseq, SIGNAL(clicked()), SLOT(csv2_reseq()));

   panel2_widgets.push_back( pb_csv2_reseq );

   pb_csv2_check = new QPushButton(tr("Check"), this);
   pb_csv2_check->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_check->setMinimumHeight(minHeight1);
   pb_csv2_check->setPalette( PALET_PUSHB );
   connect(pb_csv2_check, SIGNAL(clicked()), SLOT(csv2_check()));

   panel2_widgets.push_back( pb_csv2_check );

   pb_csv2_sort = new QPushButton(tr("Sort"), this);
   pb_csv2_sort->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_sort->setMinimumHeight(minHeight1);
   pb_csv2_sort->setPalette( PALET_PUSHB );
   connect(pb_csv2_sort, SIGNAL(clicked()), SLOT(csv2_sort()));

   panel2_widgets.push_back( pb_csv2_sort );

   pb_csv2_find_alt = new QPushButton(tr("Find alternate matching residues"), this);
   pb_csv2_find_alt->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_find_alt->setMinimumHeight(minHeight1);
   pb_csv2_find_alt->setPalette( PALET_PUSHB );
   connect(pb_csv2_find_alt, SIGNAL(clicked()), SLOT(csv2_find_alt()));

   panel2_widgets.push_back( pb_csv2_find_alt );

   pb_csv2_bm = new QPushButton(tr("BM"), this);
   pb_csv2_bm->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_bm->setMinimumHeight(minHeight1);
   pb_csv2_bm->setPalette( PALET_PUSHB );
   connect(pb_csv2_bm, SIGNAL(clicked()), SLOT(csv2_bm()));

   panel2_widgets.push_back( pb_csv2_bm );

   pb_csv2_clash_report = new QPushButton(tr("PW Distance"), this);
   pb_csv2_clash_report->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_clash_report->setMinimumHeight(minHeight1);
   pb_csv2_clash_report->setPalette( PALET_PUSHB );
   connect(pb_csv2_clash_report, SIGNAL(clicked()), SLOT(csv2_clash_report()));

   panel2_widgets.push_back( pb_csv2_clash_report );

   pb_csv2_sel = new QPushButton(tr("Select"), this);
   pb_csv2_sel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_sel->setMinimumHeight(minHeight1);
   pb_csv2_sel->setPalette( PALET_PUSHB );
   connect(pb_csv2_sel, SIGNAL(clicked()), SLOT(csv2_sel()));

   panel2_widgets.push_back( pb_csv2_sel );

   pb_csv2_sel_clear = new QPushButton(tr("Clear"), this);
   pb_csv2_sel_clear->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_sel_clear->setMinimumHeight(minHeight1);
   pb_csv2_sel_clear->setPalette( PALET_PUSHB );
   connect(pb_csv2_sel_clear, SIGNAL(clicked()), SLOT(csv2_sel_clear()));

   panel2_widgets.push_back( pb_csv2_sel_clear );

   pb_csv2_sel_clean = new QPushButton(tr("Clean"), this);
   pb_csv2_sel_clean->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_sel_clean->setMinimumHeight(minHeight1);
   pb_csv2_sel_clean->setPalette( PALET_PUSHB );
   connect(pb_csv2_sel_clean, SIGNAL(clicked()), SLOT(csv2_sel_clean()));

   panel2_widgets.push_back( pb_csv2_sel_clean );

   pb_csv2_sel_invert = new QPushButton(tr("Invert"), this);
   pb_csv2_sel_invert->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_sel_invert->setMinimumHeight(minHeight1);
   pb_csv2_sel_invert->setPalette( PALET_PUSHB );
   connect(pb_csv2_sel_invert, SIGNAL(clicked()), SLOT(csv2_sel_invert()));

   panel2_widgets.push_back( pb_csv2_sel_invert );

   pb_csv2_sel_chain = new QPushButton(tr("Chain"), this);
   pb_csv2_sel_chain->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_sel_chain->setMinimumHeight(minHeight1);
   pb_csv2_sel_chain->setPalette( PALET_PUSHB );
   connect(pb_csv2_sel_chain, SIGNAL(clicked()), SLOT(csv2_sel_chain()));

   panel2_widgets.push_back( pb_csv2_sel_chain );

   pb_csv2_sel_nearest_atoms = new QPushButton(tr("Nearest Atoms"), this);
   pb_csv2_sel_nearest_atoms->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_csv2_sel_nearest_atoms->setMinimumHeight(minHeight1);
   pb_csv2_sel_nearest_atoms->setPalette( PALET_PUSHB );
   connect(pb_csv2_sel_nearest_atoms, SIGNAL(clicked()), SLOT(csv2_sel_nearest_atoms()));

   panel2_widgets.push_back( pb_csv2_sel_nearest_atoms );

   pb_csv2_sel_nearest_residues = new QPushButton(tr("Nearest Residues"), this);
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
   qwtw_wheel->setRange        ( 0.0, 0.0, 1 );
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
   Q3GridLayout *gl_panes = new Q3GridLayout( 0, 0, 0, 1, 1 );

   // left pane
   Q3BoxLayout *vbl_editor_group = new Q3VBoxLayout;
   vbl_editor_group->addWidget(frame);
   vbl_editor_group->addWidget(editor);

   gl_panes->addLayout( vbl_editor_group, 0, 0 );

   Q3BoxLayout *hbl_left_buttons_row_1 = new Q3HBoxLayout;
   hbl_left_buttons_row_1->addWidget( pb_split_pdb );
   hbl_left_buttons_row_1->addSpacing( 2 );
   hbl_left_buttons_row_1->addWidget( pb_join_pdbs );
   
   Q3BoxLayout *hbl_left_buttons_row_2 = new Q3HBoxLayout;
   hbl_left_buttons_row_2->addWidget( pb_merge );
   hbl_left_buttons_row_2->addWidget( pb_renum_pdb );

   Q3BoxLayout *hbl_left_buttons_row_3 = new Q3HBoxLayout;
   hbl_left_buttons_row_3->addWidget( pb_hybrid_split );
   hbl_left_buttons_row_3->addWidget( pb_h_to_chainX );

   Q3BoxLayout *hbl_left_buttons_row_4 = new Q3HBoxLayout;
   hbl_left_buttons_row_4->addWidget( pb_help );
   hbl_left_buttons_row_4->addSpacing( 2 );
   hbl_left_buttons_row_4->addWidget( pb_cancel );

   Q3BoxLayout *vbl_left_buttons = new Q3VBoxLayout;
   vbl_left_buttons->addLayout( hbl_left_buttons_row_1 );
   vbl_left_buttons->addLayout( hbl_left_buttons_row_2 );
   vbl_left_buttons->addLayout( hbl_left_buttons_row_3 );
   vbl_left_buttons->addLayout( hbl_left_buttons_row_4 );

   gl_panes->addLayout( vbl_left_buttons, 1, 0 );

   // center pane
   Q3BoxLayout *vbl_center_top = new Q3VBoxLayout;
   vbl_center_top->addWidget( lbl_csv );
   vbl_center_top->addWidget( lv_csv );
   vbl_center_top->addWidget( lbl_csv_sel_msg );
   vbl_center_top->addWidget( te_csv );

   gl_panes->addLayout( vbl_center_top, 0, 1 );

   Q3BoxLayout *hbl_center_buttons_row_1 = new Q3HBoxLayout;
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
   
   Q3BoxLayout *hbl_center_buttons_row_2 = new Q3HBoxLayout;
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

   Q3BoxLayout *hbl_center_buttons_row_3 = new Q3HBoxLayout;
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

   Q3BoxLayout *hbl_center_buttons_row_4 = new Q3HBoxLayout;
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

   Q3BoxLayout *vbl_center_buttons = new Q3VBoxLayout;
   vbl_center_buttons->addLayout( hbl_center_buttons_row_1 );
   vbl_center_buttons->addLayout( hbl_center_buttons_row_2 );
   vbl_center_buttons->addLayout( hbl_center_buttons_row_3 );
   vbl_center_buttons->addLayout( hbl_center_buttons_row_4 );

   gl_panes->addLayout( vbl_center_buttons, 1, 1 );

   // right pane
   Q3BoxLayout *vbl_wheel = new Q3VBoxLayout;
   vbl_wheel->addWidget( qwtw_wheel );
   vbl_wheel->addWidget( lbl_pos_range );

   Q3BoxLayout *hbl_csv2_wheel = new Q3HBoxLayout;
   hbl_csv2_wheel->addWidget( lv_csv2 );
   hbl_csv2_wheel->addLayout( vbl_wheel );

   Q3BoxLayout *vbl_right_top = new Q3VBoxLayout;
   vbl_right_top->addWidget( lbl_csv2 );
   vbl_right_top->addLayout( hbl_csv2_wheel );
   vbl_right_top->addWidget( lbl_csv2_sel_msg );
   vbl_right_top->addWidget( te_csv2 );

   gl_panes->addLayout( vbl_right_top, 0, 2 );

   Q3BoxLayout *hbl_right_buttons_row_1 = new Q3HBoxLayout;
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
   
   Q3BoxLayout *hbl_right_buttons_row_2 = new Q3HBoxLayout;
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

   Q3BoxLayout *hbl_right_buttons_row_3 = new Q3HBoxLayout;
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

   Q3BoxLayout *hbl_right_buttons_row_4 = new Q3HBoxLayout;
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

   Q3BoxLayout *vbl_right_buttons = new Q3VBoxLayout;
   vbl_right_buttons->addLayout( hbl_right_buttons_row_1 );
   vbl_right_buttons->addLayout( hbl_right_buttons_row_2 );
   vbl_right_buttons->addLayout( hbl_right_buttons_row_3 );
   vbl_right_buttons->addLayout( hbl_right_buttons_row_4 );

   gl_panes->addLayout( vbl_right_buttons, 1, 2 );

   Q3VBoxLayout *background = new Q3VBoxLayout(this);
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
   online_help->show_help("manual/somo_pdb_editor.html");
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
   editor->clear();
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
   fn = Q3FileDialog::getSaveFileName( use_dir, "*.pdb *.PDB", this, "save dialog", caption() + tr( ": Save" )  );
   if(!fn.isEmpty() )
   {
      ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( fn );
      QString text = editor->text();
      QFile f( fn );
      if ( !f.open( QIODevice::WriteOnly | QIODevice::Text) )
      {
         return;
      }
      Q3TextStream t( &f );
      t << text;
      f.close();
      editor->setModified( false );
      setCaption( fn );
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
   QColor save_color = editor->color();
   editor->setColor(color);
   editor->append(msg);
   editor->setColor(save_color);
}

void US_Hydrodyn_Pdb_Tool::csv_msg( QString color, QString msg )
{
   QColor save_color = te_csv->color();
   te_csv->setColor(color);
   te_csv->append(msg);
   te_csv->setColor(save_color);
}

void US_Hydrodyn_Pdb_Tool::csv2_msg( QString color, QString msg )
{
   QColor save_color = te_csv2->color();
   te_csv2->setColor(color);
   te_csv2->append(msg);
   te_csv2->setColor(save_color);
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

   Q3ListViewItemIterator it( lv_csv );
   while ( it.current() ) 
   {
      if ( lv_csv->isSelected( it.current() ) )
      {
         delete it.current();
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
                                    Q3ListView * /* lv           */, 
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

void US_Hydrodyn_Pdb_Tool::save_csv( Q3ListView *lv )
{
   QString use_dir = 
      ((US_Hydrodyn *)us_hydrodyn)->path_view_pdb.isEmpty() ?
      ((US_Hydrodyn *)us_hydrodyn)->somo_pdb_dir :
      ((US_Hydrodyn *)us_hydrodyn)->path_view_pdb;

   ((US_Hydrodyn *)us_hydrodyn)->select_from_directory_history( use_dir, this );

   QString filename = Q3FileDialog::getSaveFileName(
                                                   use_dir,
                                                   "*.pdb *.PDB",
                                                   this,
                                                   "save file dialog",
                                                   tr("Choose a filename to save the pdb") );
   if ( filename.isEmpty() )
   {
      return;
   }

   if ( !filename.contains(QRegExp(".pdb$",false)) )
   {
      filename += ".pdb";
   }

   if ( QFile::exists(filename) )
   {
      filename = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( filename, 0, this );
   }

   QFile f(filename);

   if ( !f.open( QIODevice::WriteOnly ) )
   {
      QMessageBox::warning( this, caption(),
                            QString(tr("Could not open %1 for writing!")).arg(filename) );
      return;
   }

   Q3TextStream t( &f );

   csv csv_to_save;
   if ( lv == lv_csv )
   {
      csv_to_save = to_csv( lv_csv, csv1 );
   } else {
      csv_to_save = to_csv( lv_csv2, csv2[ csv2_pos ] );
   }

   t << csv_to_pdb( csv_to_save );
   
   f.close();
   if ( lv == lv_csv )
   {
      csv_msg("black", QString("File %1 written\n").arg( filename ) );
   } else {
      csv2_msg("black", QString("File %1 written\n").arg( filename ) );
   }
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
   Q3ListViewItemIterator it( lv_csv2 );
   while ( it.current() ) {
      if ( lv_csv2->isSelected( it.current() ) )
      {
         delete it.current();
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
   lv_csv2->clear();
   update_enables_csv2();

   qwtw_wheel->setRange ( csv2.size() - 0.5,
                          -0.5,
                          csv2.size() * csv2.size() * 0.1  < 1.0
                          ? 
                          csv2.size() * csv2.size() * 0.05
                          :
                          1.0 );
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

pdb_sel_count US_Hydrodyn_Pdb_Tool::count_selected( Q3ListView *lv )
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
      csv_selected_element_counts.clear();
   } else {
      csv2_selected_element_counts.clear();
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

   Q3ListViewItemIterator it( lv );
   while ( it.current() ) 
   {
      Q3ListViewItem *item = it.current();
      if ( item->childCount() )
      {
         if ( is_selected( item ) || child_selected( item ) )
         {
            switch ( item->depth() )
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
                  editor_msg( "red", QString( tr( "Internal error: missing key %1" ) ).arg( key( item ) ) );
               } else {
                  csv_selected_element_counts [ csv1.data[ csv1.key[ key( item ) ] ][ 13 ] ]++;
               }
            } else {
               if ( !csv2[ csv2_pos].key.count( key( item ) ) )
               {
                  editor_msg( "red", QString( tr( "Internal error: missing key %1" ) ).arg( key( item ) ) );
               } else {
                  csv2_selected_element_counts[ csv2[ csv2_pos ].data[ csv2[ csv2_pos ].key[ key( item ) ] ][ 13 ] ]++;
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
   return counts;
}

bool US_Hydrodyn_Pdb_Tool::any_selected( Q3ListView *lv )
{
   if ( lv == lv_csv && !selection_since_count_csv1 )
   {
      return last_count_csv1.atoms != 0;
   }
   if ( lv == lv_csv2 && !selection_since_count_csv2 )
   {
      return last_count_csv2.atoms != 0;
   }

   Q3ListViewItemIterator it( lv );
   while ( it.current() ) {
      // QListViewItem *item = it.current();
      if ( it.current()->isSelected() )
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

csv US_Hydrodyn_Pdb_Tool::to_csv( Q3ListView *lv, csv &ref_csv, bool only_selected )
{
   Q3ListViewItemIterator it( lv );
   csv csv1;
   csv1.header        = ref_csv.header;
   csv1.name          = ref_csv.name;
   csv1.filename      = ref_csv.filename;
   csv1.title_text    = ref_csv.title_text;
   csv1.header_text   = ref_csv.header_text;

   while ( it.current() ) {
      Q3ListViewItem *item = it.current();
      if ( !only_selected || is_selected( item ) || child_selected( item ) )
      {
         if ( !item->childCount() )
         {
            if ( !ref_csv.key.count( key( item ) ) )
            {
               editor_msg("red", QString( tr("Error: Can not find reference key %1 in %2\n") ).arg( key( item ) ).arg( ref_csv.name ) );
            } else {
               csv1.data.push_back( ref_csv.data[ ref_csv.key[ key( item ) ] ] );
               csv1.visible.push_back( item->isVisible() );
               csv1.selected.push_back( item->isSelected() );
               csv1.open.push_back( item->isOpen() );
               if ( item == lv->currentItem() )
               {
                  csv1.current_item_key = key( item );
               }
            }
         } else {
            csv1.nd_key[ key( item ) ] = csv1.nd_visible.size();
            csv1.nd_visible.push_back( item->isVisible() );
            csv1.nd_selected.push_back( item->isSelected() );
            csv1.nd_open.push_back( item->isOpen() );
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

bool US_Hydrodyn_Pdb_Tool::is_selected( Q3ListViewItem *lvi )
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

bool US_Hydrodyn_Pdb_Tool::child_selected( Q3ListViewItem *lvi )
{
   if ( lvi->isSelected() )
   {
      return true;
   }

   if ( lvi->childCount() )
   {
      Q3ListViewItem *myChild = lvi->firstChild();
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

   return false;
}


bool US_Hydrodyn_Pdb_Tool::all_children_selected( Q3ListViewItem *lvi )
{
   if ( lvi->childCount() )
   {
      Q3ListViewItem *myChild = lvi->firstChild();
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

   return true;
}

QString US_Hydrodyn_Pdb_Tool::key( Q3ListViewItem *lvi )
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
   csv1.key.clear();

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
      
void US_Hydrodyn_Pdb_Tool::csv_to_lv( csv &csv1, Q3ListView *lv )
{
   lv->clear();

   map < QString, Q3ListViewItem * >   models;
   map < QString, Q3ListViewItem * >   model_chains;
   map < QString, Q3ListViewItem * >   model_chain_residues;
   map < QString, Q3ListViewItem * >   model_chain_residue_atoms;

   QString last_model   = "none";
   QString last_chain   = "none";
   QString last_residue = "none";

   Q3ListViewItem *current = ( Q3ListViewItem * )0;

   for ( int i = (int)csv1.data.size() - 1; i >= 0; i-- )
   {
      if ( csv1.data[ i ].size() < 6 )
      {
         editor_msg( "red", 
                     QString( tr( "Error: data row %1 too few tokens (%2), skipping" ) )
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
                        QString( tr( "Error: data row %1, model %2 duplicated , skipping" ) )
                        .arg( i + 1 )
                        .arg( model ) );
            continue;
         }
         models[ model ] = new Q3ListViewItem( lv, model );
         if ( csv1.current_item_key == model )
         {
            current = models[ model ];
         }
         if ( csv1.nd_key.count( model ) )
         {
            models[ model ]->setVisible ( csv1.nd_visible [ csv1.nd_key[ model ] ] );
            models[ model ]->setSelected( csv1.nd_selected[ csv1.nd_key[ model ] ] );
            models[ model ]->setOpen    ( csv1.nd_open    [ csv1.nd_key[ model ] ] );
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
                        QString( tr( "Error: data row %1, model~chain %2 duplicated , skipping" ) )
                        .arg( i + 1 )
                        .arg( model_chain ) );
            continue;
         }
         if ( !models.count( model ) )
         {
            editor_msg( "red", 
                        QString( tr( "Error: data row %1, model %2 not found int models! , skipping" ) )
                        .arg( i + 1 )
                        .arg( model ) );
            continue;
         }
            
         model_chains[ model_chain ] = new Q3ListViewItem( models[ model ], chain );
         if ( csv1.current_item_key == model_chain )
         {
            current = model_chains[ model_chain ];
         }
         if ( csv1.nd_key.count( model_chain ) )
         {
            model_chains[ model_chain ]->setVisible ( csv1.nd_visible [ csv1.nd_key[ model_chain ] ] );
            model_chains[ model_chain ]->setSelected( csv1.nd_selected[ csv1.nd_key[ model_chain ] ] );
            model_chains[ model_chain ]->setOpen    ( csv1.nd_open    [ csv1.nd_key[ model_chain ] ] );
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
                        QString( tr( "Error: data row %1, model~chain~residue %2 duplicated , skipping" ) )
                        .arg( i + 1 )
                        .arg( model_chain_residue ) );
            continue;
         }
         if ( !model_chains.count( model_chain ) )
         {
            editor_msg( "red", 
                        QString( tr( "Error: data row %1, model~chain %2 not found int models! , skipping" ) )
                        .arg( i + 1 )
                        .arg( model_chain ) );
            continue;
         }
         model_chain_residues[ model_chain_residue ] = new Q3ListViewItem( model_chains[ model_chain ], residue );
         if ( csv1.current_item_key == model_chain_residue )
         {
            current = model_chain_residues[ model_chain_residue ];
         }
         if ( csv1.nd_key.count( model_chain_residue ) )
         {
            model_chain_residues[ model_chain_residue ]->setVisible ( csv1.nd_visible [ csv1.nd_key[ model_chain_residue ] ] );
            model_chain_residues[ model_chain_residue ]->setSelected( csv1.nd_selected[ csv1.nd_key[ model_chain_residue ] ] );
            model_chain_residues[ model_chain_residue ]->setOpen    ( csv1.nd_open    [ csv1.nd_key[ model_chain_residue ] ] );
         } else {
            // editor_msg("red", QString( "Error: model_chain_residue %1 visi/sel/open not key not found" ).arg( model_chain_residue) );
         }
      }
      if ( !model_chain_residues.count( model_chain_residue ) )
      {
         editor_msg( "red", 
                     QString( tr( "Error: data row %1, model~chain~residue %2 not found int models! , skipping" ) )
                     .arg( i + 1 )
                     .arg( model_chain_residue ) );
         continue;
      }
      model_chain_residue_atoms[ model_chain_residue_atom ] = 
         new Q3ListViewItem( model_chain_residues[ model_chain_residue ], 
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
      if ( csv1.current_item_key == model_chain_residue_atom )
      {
         current = model_chain_residue_atoms[ model_chain_residue_atom ];
      }
      if ( csv1.key.count( model_chain_residue_atom ) )
      {
         model_chain_residue_atoms[ model_chain_residue_atom ]->setVisible ( 
                                                                            (unsigned int)csv1.visible.size() > csv1.key[ model_chain_residue_atom ]  ?
                                                                            csv1.visible [ csv1.key[ model_chain_residue_atom ] ] : false 
                                                                             );
         model_chain_residue_atoms[ model_chain_residue_atom ]->setSelected( 
                                                                            (unsigned int)csv1.selected.size() > csv1.key[ model_chain_residue_atom ] ?
                                                                            csv1.selected[ csv1.key[ model_chain_residue_atom ] ] : false
                                                                             );
         model_chain_residue_atoms[ model_chain_residue_atom ]->setOpen    ( 
                                                                            (unsigned int)csv1.open.size() > csv1.key[ model_chain_residue_atom ] ?
                                                                            csv1.open    [ csv1.key[ model_chain_residue_atom ] ] : false
                                                                             );
      } else {
         // editor_msg("red", QString( "Error: model_chain_residue_atom %1 visi/sel/open not key not found" ).arg( model_chain_residue_atom ) );
      }
   }
   lv->setCurrentItem( current );

   if ( lv == lv_csv )
   {
      lbl_csv->setText( csv1.name.isEmpty() ? tr( "Panel 1" ) : csv1.name );
      selection_since_count_csv1 = true;
   } else {
      lbl_csv2->setText( (unsigned int)csv2.size() > csv2_pos ? csv2[ csv2_pos ].name : tr( "Panel 2" ) );
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
                  csv1.data[ i ][ 4  ].ascii(),
                  csv1.data[ i ][ 2  ].ascii(),
                  csv1.data[ i ][ 1  ].ascii(),
                  csv1.data[ i ][ 3  ].toUInt(),
                  csv1.data[ i ][ 8  ].toFloat(),
                  csv1.data[ i ][ 9  ].toFloat(),
                  csv1.data[ i ][ 10 ].toFloat(),
                  csv1.data[ i ][ 11 ].toFloat(),
                  csv1.data[ i ][ 12 ].toFloat(),
                  csv1.data[ i ][ 13 ].ascii()
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
                  csv1.data[ i ][ 4  ].ascii(),
                  csv1.data[ i ][ 2  ].ascii(),
                  csv1.data[ i ][ 1  ].ascii(),
                  csv1.data[ i ][ 3  ].toUInt(),
                  csv1.data[ i ][ 8  ].toFloat(),
                  csv1.data[ i ][ 9  ].toFloat(),
                  csv1.data[ i ][ 10 ].toFloat(),
                  csv1.data[ i ][ 11 ].toFloat(),
                  csv1.data[ i ][ 12 ].toFloat(),
                  csv1.data[ i ][ 13 ].ascii()
                  );
   }
   if ( !only_atoms )
   {
      qsl << "ENDMDL\nEND\n";
   }

   return qsl;
}

void US_Hydrodyn_Pdb_Tool::visualize( Q3ListView *lv )
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
                            QString(tr("Could not open %1 for writing!")).arg(filename) );
      return;
   }

   {
      Q3TextStream t( &f );
      t << QString( "TITLE %1\n" ).arg( tmp_csv.name );
      t << csv_to_pdb( tmp_csv );
      f.close();
   }

   if ( !f_spt.open( QIODevice::WriteOnly ) )
   {
      QMessageBox::warning( this, "UltraScan",
                            QString(tr("Could not open %1 for writing!")).arg(spt_filename) );
      return;
   }

   {
      Q3TextStream t( &f_spt );
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

   Q3Process *rasmol = new Q3Process( this );

   QStringList argument;
#if !defined(WIN32) && !defined(MAC)
   argument.append("xterm");
   argument.append("-e");
#endif
#if defined(BIN64)
   argument.append(USglobal->config_list.system_dir + SLASH + "bin64" + SLASH + "rasmol");
#else
   argument.append(USglobal->config_list.system_dir + SLASH + "bin" + SLASH + "rasmol");
#endif
   argument.append( "-script" );
   argument.append( QFileInfo( spt_filename ).fileName() );
   rasmol->setWorkingDirectory(QFileInfo( filename ).dirPath());
   rasmol->setArguments(argument);
   if ( !rasmol->start() )
   {
      QMessageBox::message(tr("Please note:"), tr("There was a problem starting RASMOL\n"
                                                  "Please check to make sure RASMOL is properly installed..."));
   }

   //    QStringList qsl = atom_sel_rasmol( lv );
   //    cout << QString( "atom set %1\n" ).arg( qsl.size() );
   //    for ( int i = 0; i < (int) qsl.size(); i++ )
   //    {
   //       cout << QString( "%1: %2\n" ).arg( i ).arg( qsl[ i ] );
   //    }
   //    cout << "after atom set\n";
   //    cout << flush;
}

void US_Hydrodyn_Pdb_Tool::load( Q3ListView *lv, QString &filename, bool only_first_nmr )
{
   QString use_dir = ((US_Hydrodyn *)us_hydrodyn)->somo_pdb_dir;

   if ( filename.isEmpty() )
   {
      ((US_Hydrodyn *)us_hydrodyn)->select_from_directory_history( use_dir, this );

      filename = Q3FileDialog::getOpenFileName(use_dir, "*.pdb *.PDB", this);
   }

   if ( filename.isEmpty() )
   {
      return;
   }

   ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( filename );

   if ( !QFile::exists( filename ) )
   {
      QMessageBox::warning( this,
                            tr("Could not open file"),
                            QString( tr( "An error occured when trying to open file\n"
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
                            tr("Could not open file"),
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

   Q3TextStream ts( &f );

   unsigned int model           = 0;
   unsigned int line_count      = 0;
   bool         last_was_ENDMDL = true;

   while ( !ts.atEnd() )
   {
      QString qs = ts.readLine();
      line_count++;

      QString left6 = qs.left( 6 ).upper();
      QString left5 = qs.left( 5 ).upper();

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
      data.push_back( qs.mid( 76 , 2 ).stripWhiteSpace() );
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

void US_Hydrodyn_Pdb_Tool::load_from_qsl( Q3ListView *lv, QStringList &pdb_text, QString title )
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

      QString left6 = qs.left( 6 ).upper();
      QString left5 = qs.left( 5 ).upper();

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
      data.push_back( qs.mid( 76 , 2 ).stripWhiteSpace() );
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

void US_Hydrodyn_Pdb_Tool::csv_sel_clear()
{
   lv_csv->selectAll( false );
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

void US_Hydrodyn_Pdb_Tool::sel_nearest_atoms( Q3ListView *lv )
{
   editor_msg( "blue", "sel nearest atoms");
   pdb_sel_count counts = count_selected( lv );
   bool ok;
   unsigned int atoms = ( unsigned int )QInputDialog::getInteger(
                                                                 "US-SOMO: PDB Editor", 
                                                                 QString( tr( "Enter the of nearest atoms to find (maximum %1):" ) )
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

   map < Q3ListViewItem *, double > distmap;

   Q3ListViewItemIterator it1( lv );
   while ( it1.current() ) 
   {
      Q3ListViewItem *item1 = it1.current();
      if ( !item1->childCount() && is_selected( item1 ) )
      {
         Q3ListViewItemIterator it2( lv );
         while ( it2.current() ) 
         {
            Q3ListViewItem *item2 = it2.current();
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

   for ( map < Q3ListViewItem *, double >::iterator it = distmap.begin(); 
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

   distmap.clear();

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

   lv->selectAll( false );
   
   Q3ListViewItemIterator it( lv );
   while ( it.current() ) 
   {
      Q3ListViewItem *item = it.current();
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

void US_Hydrodyn_Pdb_Tool::select_model( Q3ListView *lv, QString model )
{
   lv->selectAll( false );
   Q3ListViewItemIterator it1( lv );
   while ( it1.current() ) 
   {
      Q3ListViewItem *item1 = it1.current();
      if ( !item1->depth() && item1->text( 0 ) == model )
      {
         item1->setSelected( true );
      }
      ++it1;
   }
   clean_selection( lv );
}

void US_Hydrodyn_Pdb_Tool::select_model( Q3ListView *lv, const set < QString > & models )
{
   lv->selectAll( false );
   Q3ListViewItemIterator it1( lv );
   while ( it1.current() ) 
   {
      Q3ListViewItem *item1 = it1.current();
      if ( !item1->depth() && models.count( item1->text( 0 ) ) )
      {
         item1->setSelected( true );
      }
      ++it1;
   }
   clean_selection( lv );
}

void US_Hydrodyn_Pdb_Tool::select_chain( Q3ListView *lv, QString chain )
{
   QStringList chains;
   chains << chain;
   return select_chain( lv, chains );
}

void US_Hydrodyn_Pdb_Tool::select_chain( Q3ListView *lv, QStringList chains )
{
   lv->selectAll( false );
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

   Q3ListViewItemIterator it1( lv );
   while ( it1.current() ) 
   {
      Q3ListViewItem *item1 = it1.current();
      if ( item1->depth() == 1 &&
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

void US_Hydrodyn_Pdb_Tool::select_chain( Q3ListView *lv )
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
      QString chain = QInputDialog::getItem(
                                            tr( "US-SOMO: PDB editor : Select chain" ) ,
                                            tr( "Select a chain or CANCEL when done" ),
                                            chains, 
                                            0, 
                                            FALSE, 
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

   map < Q3ListViewItem *, bool > selected;

   {
      Q3ListViewItemIterator it1( lv );

      while ( it1.current() ) 
      {
         Q3ListViewItem *item1 = it1.current();
         if ( is_selected( item1 ) )
         {
            selected[ item1 ] = true;
         }
         ++it1;
      }
   }

   lv->selectAll( false );

   Q3ListViewItemIterator it1( lv );
   while ( it1.current() ) 
   {
      Q3ListViewItem *item1 = it1.current();
      if ( item1->depth() == 1 &&
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

void US_Hydrodyn_Pdb_Tool::distances( Q3ListView *lv )
{
   static QString previous_selection;

   editor_msg( "blue", "compute pairwise distances");
   pdb_sel_count counts = count_selected( lv );
   bool ok;
   unsigned int atoms = ( unsigned int )QInputDialog::getInteger(
                                                                 tr( "US-SOMO: PDB editor : Pairwise Distances" ) ,
                                                                 QString( tr( "Enter the of number of minimum pairwise distances to find:" ) )
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

   double dist_thresh = QInputDialog::getDouble(
                                                tr( "US-SOMO: PDB editor : Pairwise Distances" ) ,
                                                QString( tr( "Enter a maximum threshold distance or press CANCEL for no threshold limit:" ) )
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

      restrict_atom = QInputDialog::getItem(
                                            tr( "US-SOMO: PDB editor : Pairwise Distances" ) ,
                                            tr( "Restrict search to pairs with selected atom or CANCEL to compare all" ),
                                            qsl, 
                                            use_pos, 
                                            FALSE, 
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
                                tr( "US-SOMO: PDB editor : Pairwise Distances" ) ,
                                QString( tr( "%1 models are selected, the operation will be performed on each model independently." ) ).arg( models.size() ) );

      map < Q3ListViewItem *, bool > selected_items;
      list < sortable_qlipair_double > global_lsqd;
         
      for ( unsigned int i = 0; i < (unsigned int)models.size(); i++ )
      {
         editor_msg( "black", QString( tr( "Selecting model %1\n" ) ).arg( models[ i ] ) );
         qApp->processEvents();

         select_model( lv, models[ i ] );
         editor_msg( "black", QString( tr( "Processing model %1\n" ) ).arg( models[ i ] ) );
         qApp->processEvents();
         
         map < lvipair, double > distmap;

         Q3ListViewItemIterator it1( lv );
         if ( dist_thresh != 1e99 )
         {
            editor_msg( "brown", QString( tr( "using threshold limit %1" ).arg( dist_thresh, 0, 'f', 3 ) ) );
            qApp->processEvents();
            unsigned int count = 0;
            while ( it1.current() ) 
            {
               Q3ListViewItem *item1 = it1.current();
               if ( restrict_atom.isEmpty() || get_atom_name( item1 ) == restrict_atom )
               {
                  if ( !item1->childCount() && is_selected( item1 ) )
                  {
                     if ( !( ++count % 500 ) )
                     {
                        editor_msg( "black", QString( "processed %1" ).arg( count ) );
                        qApp->processEvents();
                     }

                     Q3ListViewItemIterator it2( lv );
                     while ( it2.current() ) 
                     {
                        Q3ListViewItem *item2 = it2.current();
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
            while ( it1.current() ) 
            {
               Q3ListViewItem *item1 = it1.current();
               if ( restrict_atom.isEmpty() || get_atom_name( item1 ) == restrict_atom )
               {
                  if ( !item1->childCount() && is_selected( item1 ) )
                  {
                     if ( !( ++count % 500 ) )
                     {
                        editor_msg( "black", QString( "processed %1" ).arg( count ) );
                        qApp->processEvents();
                     }
                     Q3ListViewItemIterator it2( lv );
                     while ( it2.current() ) 
                     {
                        Q3ListViewItem *item2 = it2.current();
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
         
         distmap.clear();
         
         unsigned int pos = 0;
         for ( list < sortable_qlipair_double >::iterator it = lsqd.begin();
               it != lsqd.end();
               it++ )
         {
            distmap[ it->lvip ] = it->d;
            selected_items[ it->lvip.lvi1 ] = true;
            selected_items[ it->lvip.lvi2 ] = true;
            editor_msg( "blue", 
                        QString( tr( "pw distance %1 %2 %3\n" ) )
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
      
      editor_msg( "black", QString( tr( "Global model pairwise distance summary\n" ) ) );

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
                     QString( tr( "pw distance %1 %2 %3\n" ) )
                     .arg( key( it->lvip.lvi1 ) )
                     .arg( key( it->lvip.lvi2 ) )
                     .arg( it->d ) );
         pos++;
      }
      
      lv->selectAll( false );
      
      Q3ListViewItemIterator it( lv );
      while ( it.current() ) 
      {
         Q3ListViewItem *item = it.current();
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
      editor_msg( "blue", QString( tr( "Pairwise distance report for %1 atoms done") ).arg( atoms ) );
      return;
   }

   map < lvipair, double > distmap;

   Q3ListViewItemIterator it1( lv );
   if ( dist_thresh != 1e99 )
   {
      editor_msg( "brown", QString( tr( "using threshold limit %1" ).arg( dist_thresh, 0, 'f', 3 ) ) );
      qApp->processEvents();
      unsigned int count = 0;
      while ( it1.current() ) 
      {
         Q3ListViewItem *item1 = it1.current();
         if ( restrict_atom.isEmpty() || get_atom_name( item1 ) == restrict_atom )
         {
            if ( !item1->childCount() && is_selected( item1 ) )
            {
               if ( !( ++count % 500 ) )
               {
                  editor_msg( "black", QString( "processed %1" ).arg( count ) );
                  qApp->processEvents();
               }
            
               Q3ListViewItemIterator it2( lv );
               while ( it2.current() ) 
               {
                  Q3ListViewItem *item2 = it2.current();
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
      while ( it1.current() ) 
      {
         Q3ListViewItem *item1 = it1.current();
         if ( restrict_atom.isEmpty() || get_atom_name( item1 ) == restrict_atom )
         {
            if ( !item1->childCount() && is_selected( item1 ) )
            {
               if ( !( ++count % 500 ) )
               {
                  editor_msg( "black", QString( "processed %1" ).arg( count ) );
                  qApp->processEvents();
               }
            
               Q3ListViewItemIterator it2( lv );
               while ( it2.current() ) 
               {
                  Q3ListViewItem *item2 = it2.current();
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

   distmap.clear();

   map < Q3ListViewItem *, bool > selected_items;

   unsigned int pos = 0;
   for ( list < sortable_qlipair_double >::iterator it = lsqd.begin();
         it != lsqd.end();
         it++ )
   {
      distmap[ it->lvip ] = it->d;
      selected_items[ it->lvip.lvi1 ] = true;
      selected_items[ it->lvip.lvi2 ] = true;
      editor_msg( "blue", 
                  QString( tr( "pw distance %1 %2 %3\n" ) )
                  .arg( key( it->lvip.lvi1 ) )
                  .arg( key( it->lvip.lvi2 ) )
                  .arg( it->d ) );
      pos++;
      if ( pos >= atoms )
      {
         break;
      }
   }

   lv->selectAll( false );
   
   Q3ListViewItemIterator it( lv );
   while ( it.current() ) 
   {
      Q3ListViewItem *item = it.current();
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
   editor_msg( "blue", QString( tr( "Pairwise distance report for %1 atoms done") ).arg( atoms ) );
}

void US_Hydrodyn_Pdb_Tool::compute_angle( Q3ListView *lv )
{
   editor_msg( "blue", tr( "compute angle" ) );
   vector < QString > atom_names;
   vector < point > p;

   Q3ListViewItemIterator it1( lv );
   while ( it1.current() ) 
   {
      Q3ListViewItem *item1 = it1.current();
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
                  QString( tr( "Error: compute angle: exactly 3 atoms must be selected, %1 are selected" ) )
                  .arg( atom_names.size() ) );
      return;
   }

   QString res;
   // base atom[ 0 ]
   {
      float a = acosf( ((US_Hydrodyn *)us_hydrodyn)->dot( ((US_Hydrodyn *)us_hydrodyn)->normal( ((US_Hydrodyn *)us_hydrodyn)->minus( p[ 1 ], p[ 0 ] ) ),
                                                          ((US_Hydrodyn *)us_hydrodyn)->normal( ((US_Hydrodyn *)us_hydrodyn)->minus( p[ 2 ], p[ 0 ] ) ) ) );
      
      res += 
         QString( tr( "Angle %1,%2,%3 = %4 or %5 degrees\n" ) )
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
         QString( tr( "Angle %1,%2,%3 = %4 or %5 degrees\n" ) )
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
         QString( tr( "Angle %1,%2,%3 = %4 or %5 degrees\n" ) )
         .arg( atom_names[ 0 ] )
         .arg( atom_names[ 2 ] )
         .arg( atom_names[ 1 ] )
         .arg( a )
         .arg( a * 180.0 / M_PI )
         ;
   }
   editor_msg( "black", res );
   editor_msg( "blue", tr( "compute angle done" ) );

}

QString US_Hydrodyn_Pdb_Tool::get_atom_name( Q3ListViewItem *lvi )
{
   QString atom = 
      QString( "%1" ).arg( lvi->text( 0 ) )
      .replace( QRegExp( "^\\S+" ), "" )
      .replace( QRegExp( "\\S+$" ), "" )
      .stripWhiteSpace();
   // cout << QString( "text0 is <%1> atom is <%2>\n" ).arg( lvi->text( 0 ) ).arg( atom );
   return atom;
}

QString US_Hydrodyn_Pdb_Tool::get_atom_number( Q3ListViewItem *lvi )
{
   QString atom = 
      QString( "%1" ).arg( lvi->text( 0 ) )
      .stripWhiteSpace()
      .replace( QRegExp( "^\\S+" ), "" )
      .stripWhiteSpace();
   // cout << QString( "text0 is <%1> atom is <%2>\n" ).arg( lvi->text( 0 ) ).arg( atom );
   return atom;
}

QString US_Hydrodyn_Pdb_Tool::get_chain_id( Q3ListViewItem *lvi )
{
   if ( lvi->depth() < 1 )
   {
      return "unknown";
   }
   while( lvi->depth() > 1 )
   {
      lvi = lvi->parent();
   }
   return lvi->text( 0 );
}

QString US_Hydrodyn_Pdb_Tool::get_model_id( Q3ListViewItem *lvi )
{
   while( lvi->depth() > 0 )
   {
      lvi = lvi->parent();
   }
   return lvi->text( 0 );
}

QString US_Hydrodyn_Pdb_Tool::get_residue_name( Q3ListViewItem *lvi )
{
   if ( lvi->depth() < 2 )
   {
      return "unknown";
   }
   while( lvi->depth() > 2 )
   {
      lvi = lvi->parent();
   }
   return QString( "%1" ).arg( lvi->text( 0 ) ).stripWhiteSpace().replace( QRegExp( "\\s*\\d+$" ), "" );
}

QString US_Hydrodyn_Pdb_Tool::get_residue_number( Q3ListViewItem *lvi )
{
   if ( lvi->depth() < 2 )
   {
      return "unknown";
   }
   while( lvi->depth() > 2 )
   {
      lvi = lvi->parent();
   }
   return QString( "%1" ).arg( lvi->text( 0 ) ).stripWhiteSpace().replace( QRegExp( "^\\S+\\s*" ), "" ).stripWhiteSpace();
}

QStringList US_Hydrodyn_Pdb_Tool::atom_set( Q3ListView *lv )
{
   map < QString, bool > atoms;

   Q3ListViewItemIterator it1( lv );
   while ( it1.current() ) 
   {
      Q3ListViewItem *item1 = it1.current();
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

QStringList US_Hydrodyn_Pdb_Tool::atom_sel_rasmol( Q3ListView *lv )
{
   map < QString, bool > atoms;

   Q3ListViewItemIterator it1( lv );
   QStringList qsl;

   while ( it1.current() ) 
   {
      Q3ListViewItem *item1 = it1.current();
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

QStringList US_Hydrodyn_Pdb_Tool::model_set( Q3ListView *lv )
{
   map < QString, bool > models;

   Q3ListViewItemIterator it1( lv );
   while ( it1.current() ) 
   {
      Q3ListViewItem *item1 = it1.current();
      if ( !item1->depth() && is_selected( item1 ) )
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

QStringList US_Hydrodyn_Pdb_Tool::chain_set( Q3ListView *lv )
{
   map < QString, bool > chains;

   Q3ListViewItemIterator it1( lv );
   while ( it1.current() ) 
   {
      Q3ListViewItem *item1 = it1.current();
      if ( item1->depth() == 1 && is_selected( item1 ) )
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

void US_Hydrodyn_Pdb_Tool::csv2_sel_clear()
{
   lv_csv2->selectAll( false );
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

void US_Hydrodyn_Pdb_Tool::invert_selection( Q3ListView *lv )
{
   // make sure atoms are selected
   clean_selection( lv );

   // turn off parents
   {
      Q3ListViewItemIterator it( lv );
      while ( it.current() ) 
      {
         Q3ListViewItem *item = it.current();
         if ( item->isSelected() && item->childCount() )
         {
            item->setSelected( false );
         }
         ++it;
      }
   }
   // invert atoms
   {
      Q3ListViewItemIterator it( lv );
      while ( it.current() ) 
      {
         Q3ListViewItem *item = it.current();
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

void US_Hydrodyn_Pdb_Tool::clean_selection( Q3ListView *lv )
{
   // if a parent is selected, set selected on all children
   // & if all children are selected, set parents selected

   Q3ListViewItemIterator it( lv );
   while ( it.current() ) 
   {
      Q3ListViewItem *item = it.current();
      if ( !item->isSelected() )
      {
         if ( is_selected( item ) || all_children_selected( item ) )
         {
               item->setSelected( true );
         } 
      }
      ++it;
   }

   lv->triggerUpdate();

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
               QString( tr( "Warning: duplicate atoms not pasted <%1>\n") )
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
               QString( tr( "Warning: duplicate atom not pasted <%1>\n") )
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
         merged.nd_key[ it->first ] = (unsigned int)merged.nd_visible.size();
         
         merged.nd_visible .push_back( csv2.nd_visible [ it->second ] );
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

   QString filename = Q3FileDialog::getOpenFileName(use_dir, "*.pdb *.PDB", this);

   if ( filename.isEmpty() )
   {
      pb_split_pdb->setEnabled( true );
      return;
   }

   ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( filename );

   if ( !QFile::exists( filename ) )
   {
      QMessageBox::warning( this,
                            tr("Could not open file"),
                            QString( tr( "An error occured when trying to open file\n"
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
                            tr("Could not open file"),
                            QString("An error occured when trying to open file\n"
                                    "%1\n"
                                    "Please check the permissions and try again\n")
                            .arg( filename )
                            );
      pb_split_pdb->setEnabled( true );
      return;
   }

   if ( QMessageBox::question(this, 
                              tr( "US-SOMO: PDB Editor : Split by model" ),
                              QString( tr( "How do you want to split the file %1?" ) ).arg( QFileInfo( filename ).fileName() ),
                              tr( "&Normally (by model)" ), 
                              tr( "Residues into models for saxs structure facture computation" ),
                              QString::null,
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

   editor_msg( "dark blue", QString( tr( "Checking file %1" ).arg( f.name() ) ) );

   map    < QString, bool > model_names;
   vector < QString >       model_name_vector;
   unsigned int             max_model_name_len      = 0;
   QString                  model_header;
   bool                     dup_model_name_msg_done = false;
   unsigned int             end_count               = 0;
   bool                     found_model             = false;
   
   {
      Q3TextStream ts( &f );
      unsigned int line_count = 0;
   
      while ( !ts.atEnd() )
      {
         QString qs = ts.readLine();
         line_count++;
         if ( line_count && !(line_count % 100000 ) )
         {
            editor_msg( "dark blue", QString( tr( "Lines read %1" ).arg( line_count ) ) );
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
            QStringList qsl = QStringList::split( QRegExp("\\s+"), qs.left(20) );
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
                  editor_msg( "red", tr( "Duplicate or missing model names found, -# extensions added" ) );
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

   editor_msg( "dark blue", QString( tr( "File %1 contains %2 models" ) ).arg( f.name() ).arg( model_count ) );

   if ( model_count == 1 )
   {
      QMessageBox::warning( this,
                            tr("US-SOMO: PDB Editor - Split"),
                            QString("The file"
                                    "%1\n"
                                    "Only appears to contain 1 model\n" )
                            .arg( f.name() )
                            );
      pb_split_pdb->setEnabled( true );
      return;
   }

   // ask how many to split into & then make them

   bool ok;
   int res = QInputDialog::getInteger(
                                      "US-SOMO: PDB Editor - Split",
                                      QString( tr( "File %1 contains %2 models\n"
                                                   "Enter the sequence increment value\n"
                                                   "Use 1 to make a pdb of each model, 2 for every other, etc.\n"
                                                   "Press Cancel to quit\n") )
                                      .arg( f.name() )
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

   QString fn = f.name().replace(QRegExp("\\.(pdb|PDB)$"),"") + ext;

   fn = Q3FileDialog::getSaveFileName( fn, 
                                      "PDB (*.pdb *.PDB)",
                                      this,
                                      "save the models",
                                      "Choose a name to save the files, the X's will be replaced by the model name" );
   
   if ( fn.isEmpty() )
   {
      pb_split_pdb->setEnabled( true );
      return;
   }
   
   fn.replace(QRegExp(QString("%1$").arg(ext)), "" );

   if ( !f.open( QIODevice::ReadOnly ) )
   {
      QMessageBox::warning( this,
                            tr("Could not open file"),
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
                                    tr("US-SOMO: PDB Editor - overwrite question"),
                                    QString(tr("Please note:\n\n"
                                               "Overwriting of existing files currently off.\n"
                                               "This could block processing awaiting user input.\n"
                                               "What would you like to do?\n")),
                                    tr("&Stop"), 
                                    tr("&Turn on overwrite now"),
                                    tr("C&ontinue anyway"),
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

   Q3TextStream ts( &f );

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
                                           QString(tr("Could not open %1 for writing!")).arg( use_fn ) );
                     pb_split_pdb->setEnabled( true );
                     return;
                  }
                  
                  Q3TextStream tso( &fn_out );
               
                  tso << QString("HEADER    split from %1: Model %2 of %3\n").arg( f.name() ).arg( pos + 1 ).arg( model_count );
                  tso << model_header;
                  tso << QString("").sprintf("MODEL  %7s\n", model_name_vector[ pos ].ascii() );
                  tso << model_lines;
                  tso << "ENDMDL\nEND\n";
                  
                  fn_out.close();
                  editor_msg( "dark blue", QString( tr( "File %1 written" ) ).arg( fn_out.name() ) );
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

   QString filename = Q3FileDialog::getOpenFileName(use_dir, "*.pdb *.PDB", this);

   if ( filename.isEmpty() )
   {
      return;
   }

   ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( filename );

   if ( !QFile::exists( filename ) )
   {
      QMessageBox::warning( this,
                            tr("US-SOMO: PDB Editor - Hybrid Extract"),
                            QString( tr( "An error occured when trying to open file\n"
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
                            tr("US-SOMO: PDB Editor - Hybrid Extract"),
                            QString("An error occured when trying to open file\n"
                                    "%1\n"
                                    "Please check the permissions and try again\n")
                            .arg( filename )
                            );
      return;
   }

   editor_msg( "dark blue", QString( tr( "Hybrid extract: processing file %1" ).arg( f.name() ) ) );
   
   QRegExp rx_atom("^ATOM");

   Q3TextStream ts( &f );
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
         editor_msg( "dark blue", QString( tr( "Lines read %1" ).arg( line_count ) ) );
         qApp->processEvents();
      }
      if ( qs.contains( rx_atom ) )
      {
         // cout << QString( "got atom <%1>\n" ).arg( qs );
         QString atom        = qs.mid( 12, 4 ).stripWhiteSpace();
         QString atom_left   = atom.right( atom.length() - 1 );
         QString atom_hmap   = qs.mid( 12, 3 ).stripWhiteSpace();
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
            atom_hmap        = qs.mid( 12, 4 ).stripWhiteSpace();
            atom_hmap        = atom_hmap.right( atom_hmap.length() - 1 );
            atom_hseq        = " ";
         }
         QString atom_hkey   = atom_hmap + ":" + atom_hseq;
         QString residue     = qs.mid( 17, 3 ).stripWhiteSpace();
         QString residue_seq = qs.mid( 22, 4 ).stripWhiteSpace();
         
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
                  editor_msg( "red", QString( tr( "Error: hydrogen ref <%1> but no atom with ref" ) ).arg( href ) );
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
            this_residue          .clear();
            this_residue_hydrogens.clear();
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
            editor_msg( "red", QString( tr( "Error: hydrogen ref <%1> but no atom with ref" ) ).arg( href ) );
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
      this_residue          .clear();
      this_residue_hydrogens.clear();
   }
   
   f.close();

   // write them out

   QString fbase = QFileInfo( filename ).dirPath() + SLASH + QFileInfo( filename ).baseName();

   for ( map < QString, vector < QString > >::iterator it = atoms_with_hydrogens.begin();
         it != atoms_with_hydrogens.end();
         it++ )
   {

      QFile f_out( QString( "%1-%2.pdb" ).arg( fbase ).arg( it->first ) );
      if ( !f_out.open( QIODevice::WriteOnly ) )
      {
         QMessageBox::warning( this, 
                               tr( "US-SOMO: PDB editor : Hybrid Extract" ),
                               QString( tr( "Error: Can not open file %1 for writing" ) ).arg( f_out.name() ) );
         return;
      }

      editor_msg( "blue", QString( tr( "Creating %1 with %2 models" ) ).arg( f_out.name() ).arg( it->second.size() ) );

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
         QStringList lines = QStringList::split( "\n", it->second[ i ] );
         vector < QString > names;
         vector < point   > p;
         for ( unsigned int j = 0; j < (unsigned int)lines.size(); j++ )
         {
            names.push_back( lines[ j ].mid( 12, 4 ).stripWhiteSpace().left( 1 ) );
            point this_p;
            this_p.axis[ 0 ] = lines[ j ].mid( 30 , 8 ).stripWhiteSpace().toFloat();
            this_p.axis[ 1 ] = lines[ j ].mid( 38 , 8 ).stripWhiteSpace().toFloat();
            this_p.axis[ 2 ] = lines[ j ].mid( 46 , 8 ).stripWhiteSpace().toFloat();
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
               editor_msg( "dark red", QString( tr( "WARNING: atom names inconsistance for %1" ) ).arg( it->first ) );
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
         
      Q3TextStream ts_out( &f_out );
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

   QString filename = Q3FileDialog::getOpenFileName(use_dir, "*.pdb *.PDB", this);

   if ( filename.isEmpty() )
   {
      return;
   }

   ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( filename );

   if ( !QFile::exists( filename ) )
   {
      QMessageBox::warning( this,
                            tr("US-SOMO: PDB Editor - H to chain X"),
                            QString( tr( "An error occured when trying to open file\n"
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
                            tr("US-SOMO: PDB Editor - H to chain X"),
                            QString("An error occured when trying to open file\n"
                                    "%1\n"
                                    "Please check the permissions and try again\n")
                            .arg( filename )
                            );
      return;
   }

   editor_msg( "dark blue", QString( tr( "H to Chain X: processing file %1" ).arg( f.name() ) ) );
   
   QRegExp rx_atom ("^(ATOM|HETATM)");
   QRegExp rx_model("^MODEL");
   QRegExp rx_end  ("^END");

   Q3TextStream ts( &f );
   unsigned int line_count = 0;

   QString     out;
   QStringList hydrogens;

   while ( !ts.atEnd() )
   {
      QString qs = ts.readLine();
      line_count++;
      if ( line_count && !(line_count % 100000 ) )
      {
         editor_msg( "dark blue", QString( tr( "Lines read %1" ).arg( line_count ) ) );
         qApp->processEvents();
      }
      if ( qs.contains( rx_atom ) )
      {
         QString atom        = qs.mid( 12, 4 ).stripWhiteSpace();
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
            hydrogens.clear();
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
      hydrogens.clear();
   }

   QString fbase = QFileInfo( filename ).dirPath() + SLASH + QFileInfo( filename ).baseName();
   QFile f_out( QString( "%1-HX.pdb" ).arg( fbase ) );

   if ( !f_out.open( QIODevice::WriteOnly ) )
   {
      QMessageBox::warning( this, 
                            tr( "US-SOMO: PDB editor : H to chain X" ),
                            QString( tr( "Error: Can not open file %1 for writing" ) ).arg( f_out.name() ) );
      return;
   }
   
   editor_msg( "blue", QString( tr( "Creating %1" ) ).arg( f_out.name() ) );

   Q3TextStream ts_out( &f_out );
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
      files = US_Pdb_Util::sort_pdbs( Q3FileDialog::getOpenFileNames(
                                                                    "PDB files (*.pdb *.PDB)"
                                                                    , use_dir
                                                                    , this
                                                                    , tr( "US-SOMO: PDB editor : Select PDBs to join" ) 
                                                                    , tr( "Select PDB files to join, Cancel when done" )
                                                                    ) );
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
                            tr( "US-SOMO: PDB editor : Join" ),
                            tr( "Error: Only one file selected to join." ) );
      return;
   }

   
   

   QString save_file = Q3FileDialog::getSaveFileName( QFileInfo( join_files.back() ).dirPath() 
                                                     , "PDB (*.pdb *.PDB)"
                                                     , this
                                                     , tr("US-SOMO: PDB editor : Select PDBs to join" ) 
                                                     , tr( "Choose a name to save the joined PDBs" )
                                                     );

   if ( save_file.isEmpty() )
   {
      return;
   }

   if ( !save_file.contains( QRegExp( "\\.pdb$", false ) ) )
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
                            tr( "US-SOMO: PDB editor : Join" ),
                            QString( tr( "Error: Can not open file %1 for writing" ) ).arg( save_file ) );
      return;
   }

   Q3TextStream ts_out( &f_out );

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
      editor_msg( "dark gray", QString( tr( "Processing %1" ) ).arg( join_files[ i ] ) );
      qApp->processEvents();

      bool  in_model = false;

      QFile f_in( join_files[ i ] );

      if ( !f_in.open( QIODevice::ReadOnly ) )
      {
         QMessageBox::warning( this, 
                               tr( "US-SOMO: PDB editor : Join" ) ,
                               QString( tr( "Error: Can not open file %1 for reading" ) ).arg( join_files[ i ] ) );
         f_out.close();
         f_out.remove();
         return;
      }

      Q3TextStream ts_in( &f_in );

      bool                  atom_or_model_found = false;
      // bool                  has_model_line      = false;
      QString               last_model          = "";

      QString               remarks;

      while ( !ts_in.atEnd() )
      {
         QString qs = ts_in.readLine();

         bool line_written = false;

         if ( rx_remarks.search( qs ) != -1 )
         {
            if ( in_model )
            {
               ts_out << qs << endl;
            } else {
               remarks += qs + "\n";
            }
         }

         if ( rx_atom.search( qs ) != -1 )
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
         if ( rx_model.search( qs ) != -1 )
         {
            atom_or_model_found = true;
            // has_model_line      = true;
            if ( rx_get_model.search( qs ) != -1 )
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
         if ( !line_written && atom_or_model_found && rx_end.search( qs ) == -1 )
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
                             tr( "US-SOMO: PDB editor : Join" ) ,
                             QString( tr( "File %1 created with %2 models" ) ).arg( save_file ).arg( actual_model_count ) );
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
                            tr("Could not open file"),
                            QString( tr( "An error occured when trying to open file\n"
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
                            tr("Could not open file"),
                            QString("An error occured when trying to open file\n"
                                    "%1\n"
                                    "Please check the permissions and try again\n")
                            .arg( filename )
                            );
      pb_renum_pdb->setEnabled( true );
      return;
   }

   QString foutname = Q3FileDialog::getSaveFileName(QString::null, 
                                                   "*.pdb *.PDB",
                                                   this, 
                                                   "save file dialog", 
                                                   tr("Choose a filename to save the renumbered pdb") );
   if( foutname.isEmpty() )
   {
      pb_renum_pdb->setEnabled( true );
      return;
   }
      
   if ( !foutname.contains( QRegExp( ".pdb$", false ) ) )
   {
      foutname += ".pdb";
   }

   if ( foutname == filename )
   {
      QMessageBox::warning( this, "UltraScan",
                            QString(tr("The output file must not be the same as the inpufile %1!")).arg( foutname ) );
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
                            QString(tr("Could not open %1 for writing!")).arg( foutname ) );
      pb_renum_pdb->setEnabled( true );
      return;
   }
   
   Q3TextStream tsi( &f    );
   Q3TextStream tso( &fout );

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
      if ( rx_end.search( line ) != -1 )
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
      if ( rx_atom.search( line ) != -1 )
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
                  editor_msg( "dark red", tr( "Warning: more than 99,999 atoms, numbering restarted at 1" ) );
                  atomno = 1;
               }
               line = line.replace( 6, 5, QString( "" ).sprintf( "%5d", atomno ++ ) );
               if ( rx_hetatm.search( line ) == -1 )
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
                     editor_msg( "dark red", tr( "Warning: more than 9,999 residues, numbering restarted at 1" ) );
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
      tso << line << endl;
   }

   f   .close();
   fout.close();

   editor_msg( "dark blue", QString( "Renumber done, written file %1" ).arg( foutname ) );
   pb_renum_pdb->setEnabled( true );
   return;
}

void US_Hydrodyn_Pdb_Tool::csv_clear()
{
   lv_csv->clear();
   csv new_csv;
   csv1 = new_csv;
   csv_to_lv( csv1, lv_csv );
   csv_undos.clear();
   te_csv->clear();
   update_enables_csv();
}

void US_Hydrodyn_Pdb_Tool::csv2_clear()
{
   lv_csv2->clear();
   csv new_csv;
   csv2.resize( 1 );
   csv2[ 0 ] = new_csv;
   csv_to_lv( csv2[ 0 ], lv_csv2 );
   csv2_undos.clear();
   csv2_pos = 0;
   qwtw_wheel->setRange( 0.0, 0.0, 1 );
   lbl_pos_range->setText("1\nof\n1");
   csv2_redisplay( 0 );
   te_csv2->clear();
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
      if ( rx_ok.search( data_to_key( csv1.data[ i ] ) ) != -1 )
      {
         return data_to_key( csv1.data[ i ] );
      }
   }

   // couldn't find it
   editor_msg( "red", 
               QString( tr( "Internal error: could not find bottom for key <%1>" ) )
               .arg( csv1.current_item_key ) );
   return "";
}

void US_Hydrodyn_Pdb_Tool::csv_check()
{
   csv_msg( "black", tr( "Checking structure against residue file" ) );
   csv tmp_csv = to_csv( lv_csv, csv1, any_selected( lv_csv ) );
   vector < QString > error_keys;
   csv_msg( "dark blue", check_csv( tmp_csv, error_keys ) );
   if ( !errormsg.isEmpty() )
   {
      csv_msg( "red", errormsg );
   }
   csv_msg( "black", tr( "Finished checking structure" ) );
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

void US_Hydrodyn_Pdb_Tool::do_sort( Q3ListView *lv )
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
      QString msg = tr( "Residues must be selected for sorting" );
      lv == lv_csv ?
         csv_msg( "red", msg ) : csv2_msg( "red", msg );
      return;
   }

   {
      QString msg = 
         QString( tr( "Residues a: %1\n" ) ).arg( residuesa )
         + QString( tr( "Residues b: %1\n" ) ).arg( residuesb )
         + QString( tr( "Report count: %1\n" ) ).arg( reportcount == 99999 ? QString( "\"All\"" ) : QString( "%1" ).arg( reportcount ) )
         + QString( tr( "Order: %1\n" ) ).arg( QString( tr( ( order ? "Maximum" : "Minimum" ) ) + " aggregate distance" ) )
         + QString( tr( "CA Only: %1\n" ) ).arg( caonly ? "True" : "False" )
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
      QString msg = tr( "Invalid residue format" );
      lv == lv_csv ?
         csv_msg( "red", msg ) : csv2_msg( "red", msg );
      return;
   }      

   if ( !set_a.size() || !set_b.size() )
   {
      QString msg = tr( "Residues parsed empty" );
      lv == lv_csv ?
         csv_msg( "red", msg ) : csv2_msg( "red", msg );
      return;
   }      
      
   // qDebug( "set_a contains:" );
   // for ( set < QString >::iterator it = set_a.begin();
   //       it != set_a.end();
   //       it++ )
   // {
   //    qDebug( *it );
   // }
      
   // qDebug( "set_b contains:" );
   // for ( set < QString >::iterator it = set_b.begin();
   //       it != set_b.end();
   //       it++ )
   // {
   //    qDebug( *it );
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
      QString chain      = QString( tmp_csv.data[ i ][ 1 ] ).stripWhiteSpace();
      QString residuen   = QString( tmp_csv.data[ i ][ 3 ] ).stripWhiteSpace();

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
            pointsa.clear();
            pointsb.clear();
         }
         last_model = model;
      }
      QString check = QString( "%1~%2" ).arg( chain ).arg( residuen );
      // qDebug( QString( "check <%1> atom <%2>" ).arg( check ).arg( tmp_csv.data[ i ][ 4 ] ) );
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

void US_Hydrodyn_Pdb_Tool::select_these( Q3ListView *lv, vector < QString > &error_keys )
{
   lv->selectAll( false );
   map < QString, bool > error_map;
   for ( unsigned int i = 0; i < (unsigned int)error_keys.size(); i++ )
   {
      error_map[ error_keys[ i ] ] = true;
      // cout << QString( "error map <%1>\n" ).arg( error_keys[ i ] );
   }

   {
      Q3ListViewItemIterator it( lv );
      while ( it.current() ) 
      {
         Q3ListViewItem *item = it.current();
         
         if ( error_map.count( key( item ) ) )
         {
            // cout << QString( "selecting <%1>\n" ).arg( item->text( 0 ) );
            item->setSelected( true  );
            item->setVisible ( true  );
            item->setOpen    ( false );
            // open parents
            {
               Q3ListViewItem *p = item;
               while ( ( p = p->parent() ) )
               {
                  p->setOpen( true );
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
      csv_msg( "red" , tr( "No matching residues found" ) );
      return;
   }
   csv_msg( "blue", tr( "Matching residues: " ) + alt_residues.join( " " ) );
   
   bool self_included = false;
   // unsigned int self_pos;

   QStringList alt_residues_no_self;
   for ( unsigned int i = 0; i < (unsigned int)alt_residues.size(); i++ )
   {
      if ( tmp_csv.data[ 0 ][ 2 ].stripWhiteSpace() == alt_residues[ i ] )
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
                               tr( "US-SOMO: PDB Editor" ),
                               tr( "The only matching residue is the one already set" ),
                               QMessageBox::Ok );
      return;
   } 

   if ( self_included )
   {
      if ( QMessageBox::question(
                                 this,
                                 tr( "US-SOMO: PDB Editor" ),
                                 tr ( QString( "The current residue is ok, but there %1\n"
                                               "Do you want to change the residue in the loaded pdb?" ) 
                                      .arg( alt_residues.size() > 2 ? "are alternatives" : "is an alternative" ) ),
                                 tr( "&Yes" ),
                                 tr( "&No" ),
                                 QString::null, 0, 1 ) )
      {
         return;
      } else {
         if ( alt_residues.size() == 2 )
         {
            csv_undos.push_back( to_csv( lv_csv, csv1, false ) );
            csv_msg( "black", QString( tr( "Residue %1 replaced with %2" ) ).arg( tmp_csv.data[ 0 ][ 2 ] ).arg( alt_residues_no_self[ 0 ] ) );
            replace_selected_residues( lv_csv, csv1, tmp_csv.data[ 0 ][ 2 ], alt_residues_no_self[ 0 ] );
            update_enables_csv();
            return;
         }
      } 
   }

   bool ok;
   QString res = QInputDialog::getItem(
                                       tr( "US-SOMO: PDB Editor" ),
                                       tr( "Select a replacement residue\n"
                                           "or press Cancel\n" ),
                                       alt_residues_no_self, 
                                       0, 
                                       TRUE,
                                       &ok,
                                       this );
   if ( ok ) {
      csv_undos.push_back( to_csv( lv_csv, csv1, false ) );
      replace_selected_residues( lv_csv, csv1, tmp_csv.data[ 0 ][ 2 ], res );
      csv_msg( "black", QString( tr( "Residue %1 replaced with %2" ) ).arg( tmp_csv.data[ 0 ][ 2 ] ).arg( res ) );
   }

   update_enables_csv();
}

void US_Hydrodyn_Pdb_Tool::csv2_check()
{
   csv2_msg( "black", tr( "Checking structure against residue file" ) );
   csv tmp_csv = to_csv( lv_csv2, csv2[ csv2_pos ], any_selected( lv_csv2 ) );
   vector < QString > error_keys;
   csv2_msg( "dark blue", check_csv( tmp_csv, error_keys ) );
   if ( !errormsg.isEmpty() )
   {
      csv2_msg( "red", errormsg );
   }
   csv2_msg( "black", tr( "Finished checking structure" ) );
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
      csv2_msg( "red" , tr( "No matching residues found" ) );
      return;
   }
   csv2_msg( "blue", tr( "Matching residues: " ) + alt_residues.join( " " ) );
   
   bool self_included = false;
   // unsigned int self_pos;

   QStringList alt_residues_no_self;
   for ( unsigned int i = 0; i < (unsigned int)alt_residues.size(); i++ )
   {
      if ( tmp_csv.data[ 0 ][ 2 ].stripWhiteSpace() == alt_residues[ i ] )
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
                               tr( "US-SOMO: PDB Editor" ),
                               tr( "The only matching residue is the one already set" ),
                               QMessageBox::Ok );
      return;
   } 

   if ( self_included )
   {
      if ( QMessageBox::question(
                                 this,
                                 tr( "US-SOMO: PDB Editor" ),
                                 tr ( QString( "The current residue is ok, but there %1\n"
                                               "Do you want to change the residue in the loaded pdb?" ) 
                                      .arg( alt_residues.size() > 2 ? "are alternatives" : "is an alternative" ) ),
                                 tr( "&Yes" ),
                                 tr( "&No" ),
                                 QString::null, 0, 1 ) )
      {
         return;
      } else {
         if ( alt_residues.size() == 2 )
         {
            csv2_undos[ csv2_pos ].push_back( to_csv( lv_csv2, csv2[ csv2_pos ], false ) );
            replace_selected_residues( lv_csv2, csv2[ csv2_pos ], tmp_csv.data[ 0 ][ 2 ], alt_residues_no_self[ 0 ] );
            csv2_msg( "black", QString( tr( "Residue %1 replaced with %2" ) ).arg( tmp_csv.data[ 0 ][ 2 ] ).arg( alt_residues_no_self[ 0 ] ) );
            update_enables_csv2();
            return;
         }
      } 
   }

   bool ok;
   QString res = QInputDialog::getItem(
                                       tr( "US-SOMO: PDB Editor" ),
                                       tr( "Select a replacement residue\n"
                                           "or press Cancel\n" ),
                                       alt_residues_no_self, 
                                       0, 
                                       TRUE,
                                       &ok,
                                       this );
   if ( ok ) {
      csv2_undos[ csv2_pos ].push_back( to_csv( lv_csv2, csv2[ csv2_pos ], false ) );
      replace_selected_residues( lv_csv2, csv2[ csv2_pos ], tmp_csv.data[ 0 ][ 2 ], res );
      csv2_msg( "black", QString( tr( "Residue %1 replaced with %2" ) ).arg( tmp_csv.data[ 0 ][ 2 ] ).arg( res ) );
   }

   update_enables_csv2();
}

void US_Hydrodyn_Pdb_Tool::replace_selected_residues( Q3ListView *lv, csv &csv_use, QString from, QString to )
{
   Q3ListViewItemIterator it( lv );
   while ( from.length() < 3 )
   {
      from += " ";
   }
   while ( to.length() < 3 )
   {
      to += " ";
   }

   while ( it.current() ) 
   {
      Q3ListViewItem *item = it.current();
      if ( item->isSelected() &&
           // this  makes sure we are a residue
           item->depth() == 2 )
      {
         map < Q3ListViewItem *, QString > previous_keys;

         if ( item->childCount() )
         {
            Q3ListViewItem *myChild = item->firstChild();
            while ( myChild )
            {
               previous_keys[ myChild ] = key( myChild );
               myChild = myChild->nextSibling();
            }
         }

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

         if ( item->childCount() )
         {
            Q3ListViewItem *myChild = item->firstChild();
            while ( myChild )
            {
               if ( csv_use.key.count( previous_keys[ myChild ] ) )
               {
                  if ( csv_use.data[ csv_use.key[ previous_keys[ myChild ] ] ].size() < 3 )
                  {
                     editor_msg( "red", QString( tr( "Internal error: insufficient data for %1" ) )
                                 .arg( previous_keys[ myChild ] ) );
                  } else {
                     csv_use.data[ csv_use.key[ previous_keys[ myChild ] ] ][ 2 ] = to;
                  }
                  csv_use.key[ key( myChild ) ] = csv_use.key[ previous_keys[ myChild ] ];
                  csv_use.key.erase( previous_keys[ myChild ] );
               } else {
                  cout << QString( "child org key:<%1> new_key:<%2>\n" ).arg( previous_keys[ myChild ] )
                     .arg( key( myChild ) );
                  editor_msg( "red", tr( "Internal error: expected key match 2" ) );
               }
               myChild = myChild->nextSibling();
            }
         }

      }
      ++it;
   }

   return;
}

QString US_Hydrodyn_Pdb_Tool::check_csv_for_alt( csv &csv1, QStringList &alt_residues )
{
   alt_residues.clear();
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
      if ( rx_skip.search( usu->residue_list[ i ].name ) == -1 )
      {
         residue_names[ usu->residue_list[ i ].name ] = true;
         // cout << QString( "residue_list: %1 %2\n" ).arg( i ).arg( usu->residue_list[ i ].name );
      }
   }

   for ( unsigned int i = 0; i < (unsigned int)csv1.data.size(); i++ )
   {
      if ( csv1.data[ i ].size() < 3 )
      {
         editor_msg( "red", tr( "Internal error: unexpected data length" ) );
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

      error_keys.clear();
      
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

   error_keys.clear();

   for ( unsigned int i = 0; i < (unsigned int)usu->model_vector.size(); i++ )
   {
      if ( !usu->check_for_missing_atoms( &usu->model_vector[ i ], qsl ) )
      {
         errormsg += QString( tr( "Model %1: Errors with your PDB structure %2\n" ) )
            .arg( i + 1 ).arg( usu->errormsg );
      } else {
         qs += QString( tr( "Model %1: ok\n" ) ).arg( i + 1 );
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
                                  tr( "US-SOMO: PDB Editor" ),
                                  tr( "Errors were found\n"
                                      "Do you want to select the residues with errors?" ),
                                  tr( "&Yes" ),
                                  tr( "&No" ),
                                  QString::null, 0, 1 ) )
      {
         // cout << "clearing error keys\n";
         error_keys.clear();
      }
   }
      
   return qs;
}

double US_Hydrodyn_Pdb_Tool::minimum_pair_distance  ( Q3ListView *lv,
                                                      QString    chain_1,
                                                      QString    chain_2,
                                                      QString  & key_1,
                                                      QString  & key_2 )
{
   vector < Q3ListViewItem * > items_chain_1;
   vector < Q3ListViewItem * > items_chain_2;

   {
      Q3ListViewItemIterator it1( lv );
      while ( it1.current() ) 
      {
         Q3ListViewItem *item1 = it1.current();
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
      Q3ListViewItemIterator it2( lv );
      while ( it2.current() ) 
      {
         Q3ListViewItem *item2 = it2.current();
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

   editor_msg( "dark blue", QString( tr( "Checking file %1" ).arg( f.name() ) ) );

   vector < QString >                       residue_atoms;
   vector < vector < QString > >            residues;
   vector < vector < vector < QString > > > chain_residues;
   
   QString last_resSeq           = "";
   QString last_chainId          = "";
   unsigned int max_chain_length = 0;

   QString                  model_header;

   bool skip_waters = 
      QMessageBox::question( this, 
                             tr( "US-SOMO: PDB Editor : Split by residue" ),
                             QString( tr( "Do you want to skip waters?" ) ),
                             tr( "&Yes" ), 
                             tr( "&No" ),
                             QString::null,
                             1
                             ) == 0;

   bool waters_as_onezies = skip_waters ? false :
      ( QMessageBox::question( this, 
                             tr( "US-SOMO: PDB Editor : Split by residue" ),
                             QString( tr( "Do you want to keep the waters as singletons?" ) ),
                             tr( "&Yes" ), 
                             tr( "&No" ),
                             QString::null,
                             0
                             ) == 0 );
      

   unsigned int water_count = 0;

   {
      Q3TextStream ts( &f );
      unsigned int line_count = 0;
   
      while ( !ts.atEnd() )
      {
         QString qs = ts.readLine();
         line_count++;
         if ( line_count && !(line_count % 100000 ) )
         {
            editor_msg( "dark blue", QString( tr( "Lines read %1" ).arg( line_count ) ) );
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
            QString resName = qs.mid( 17 , 3 ).stripWhiteSpace();
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

               QString resSeq  = qs.mid( 22 , 4 ).stripWhiteSpace();
               QString chainId = qs.mid( 21 , 1 );
               if ( resSeq  != last_resSeq ||
                    chainId != last_chainId ||
                    ( waters_as_onezies && is_water ) )
               {
                  if ( residue_atoms.size() )
                  {
                     residues.push_back( residue_atoms );
                     residue_atoms.clear();
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
                        residues.clear();
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
         residues.clear();
         residue_atoms.clear();
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
      window_size = ( unsigned int )QInputDialog::getInteger(
                                                             tr( "US-SOMO: PDB Editor : Split by residue" ),
                                                             QString( tr( "The maximum chain length in residues is %1 in the 1st model in the PDB file %2\n"
                                                                          "Enter the number of resdues per output model:" ) )
                                                             .arg( max_chain_length )
                                                             .arg( f.name() ),
                                                             1,
                                                             1, 
                                                             max_chain_length / 2,
                                                             1,
                                                             &ok, 
                                                             this );
      if ( ok )
      {
         step_size = ( unsigned int )QInputDialog::getInteger(
                                                              tr( "US-SOMO: PDB Editor : Split by residue" ),
                                                              QString( tr( "The maximum chain length in residues is %1 in the 1st model in the PDB file %2\n"
                                                                           "The number of resdues per output model is %3\n"
                                                                           "Enter the step size:" ) )
                                                              .arg( max_chain_length )
                                                              .arg( f.name() )
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
                                        tr( "US-SOMO: PDB Editor : Split by residue" ),
                                        QString( tr( "The maximum chain length in residues is %1 in the 1st model in the PDB file %2\n"
                                                     "The number of residues per model is %3\n"
                                                     "The step size is %4\n"
                                                     ) )
                                        .arg( max_chain_length )
                                        .arg( f.name() )
                                        .arg( window_size )
                                        .arg( step_size )
                                        ,
                                        tr( "&Re-enter values" ), 
                                        tr( "&Proceed" ),
                                        tr( "&Cancel" ),
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

   QString fn = f.name().replace(QRegExp("\\.(pdb|PDB)$"),"") + QString( "_ws%1_ss%2%3%4_sr.pdb" ).arg( window_size ).arg( step_size ).arg( skip_waters ? "_nw" : "" ).arg( waters_as_onezies ? "_ww" : "" );

   fn = Q3FileDialog::getSaveFileName( fn, 
                                      "PDB (*.pdb *.PDB)",
                                      this,
                                      "save the models",
                                      tr( "Choose a name to save the multiple model output file" ) );
   
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
                            QString(tr("Could not open %1 for writing!")).arg( fn ) );
      return;
   }

   unsigned int model_count = 1;
   
   Q3TextStream tso( &fn_out );

   tso << QString("HEADER    split by residue from %1: window size %2 step size %3\n")
      .arg( f.name() )
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
                     QString resName = chain_residues[ i ][ k ][ l ].mid( 17 , 3 ).stripWhiteSpace();
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
                             tr( "US-SOMO: PDB Editor : Split by residue" ),
                             QString( tr( "Created file %1 with %2 models" ) )
                             .arg( fn_out.name() )
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

void US_Hydrodyn_Pdb_Tool::do_bm( Q3ListView *lv )
{
   // 1st find atoms

   bool ok;
   double psv = QInputDialog::getDouble(
                                        tr( "US-SOMO: PDB editor : Bead model from atoms" )
                                        , QString( tr( "Enter an estimated PSV" ) )
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

   double expansion = QInputDialog::getDouble(
                                              tr( "US-SOMO: PDB editor : Bead model from atoms" )
                                              , QString( tr( "Enter a vDW multiplier:" ) )
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

   Q3ListViewItemIterator it1( lv );
   while ( it1.current() ) 
   {
      Q3ListViewItem *item1 = it1.current();
      if ( !item1->childCount() && is_selected( item1 ) )
      {
         //item1->text( 3 ).toFloat();
         //item1->text( 4 ).toFloat();
         //item1->text( 5 ).toFloat();
         QString atom = item1->text( 7 ).stripWhiteSpace();
         if ( !usu->atom_mw.count( atom ) )
         {
            QString msg = QString( tr( "Atom '%1' not found in loaded molecular weights (mw.json)" ) ).arg( atom );
            lv == lv_csv ?
               csv_msg( "red", msg ) : csv2_msg( "red", msg );
            return;
         }
         if ( !usu->atom_vdw.count( atom ) )
         {
            QString msg = QString( tr( "Atom '%1' not found in loaded molecular vdw radii (vdw.json)" ) ).arg( atom );
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
            .arg( item1->parent()->text(0).stripWhiteSpace().replace( QRegExp( "\\s+" ), ":" ) + 
                  ":" + 
                  item1->text( 0 ).stripWhiteSpace().replace( QRegExp( "\\s+" ), ":" ) )
            ;
      }
      ++it1;
   }

   // take the atoms and ask for inflation & make a bead model
   QString use_dir;
   ((US_Hydrodyn *)us_hydrodyn)->select_from_directory_history( use_dir, this );

   QString filename = Q3FileDialog::getSaveFileName(
                                                   use_dir + QDir::separator() + QFileInfo( 
                                                                                           lv == lv_csv ?
                                                                                           csv1.name :
                                                                                           csv2[ csv2_pos ].name ).baseName()
                                                   ,
                                                   "*.bead_model *.BEAD_MODEL",
                                                   this,
                                                   "save file dialog",
                                                   tr("Choose a filename to save the bead model") );

   if ( filename.isEmpty() )
   {
      return;
   }

   filename.replace( QRegExp("(_e\\d+_\\d+|)\\.bead_model$",false), "" );
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
      QMessageBox::warning( this, caption(),
                            QString( tr("Could not open %1 for writing!") ).arg(filename) );
      return;
   }

   Q3TextStream ts( &f );
   ts << QString( "%1 %2\n" ).arg( out.size() ).arg( psv, 0, 'f', 3 );
   ts << out.join("\n") << endl;
   f.close();
   {

      QString msg = QString( tr( "Created beam model output file %1" ) ).arg( f.name() );
      lv == lv_csv ?
         csv_msg( "blue", msg ) : csv2_msg( "blue", msg );
   }
   return;
}

