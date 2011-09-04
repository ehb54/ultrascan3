#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"

#define SLASH QDir::separator()

US_Hydrodyn_Pdb_Tool::US_Hydrodyn_Pdb_Tool(
                                               csv csv1,
                                               void *us_hydrodyn, 
                                               QWidget *p, 
                                               const char *name
                                               ) : QFrame(p, name)
{
   this->csv1 = csv1;
   this->us_hydrodyn = us_hydrodyn;
   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("PDB viewer"));
   setupGUI();
   this->csv1.visible.resize( this->csv1.data.size() );
   this->csv1.selected.resize( this->csv1.selected.size() );
   this->csv1.open.resize( this->csv1.open.size() );
   csv_setup_keys( this->csv1 );
   selection_since_count_csv1 = true;
   selection_since_count_csv2 = true;
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
   editor->setMaximumWidth( editor->width() * 4 );

   setGeometry(global_Xpos, global_Ypos, 0, 0 ); // csv_width, 100 + csv_height );
}

US_Hydrodyn_Pdb_Tool::~US_Hydrodyn_Pdb_Tool()
{
}

void US_Hydrodyn_Pdb_Tool::setupGUI()
{
   int minHeight1 = 30;

   lbl_title = new QLabel("PDB Editor", this);
   lbl_title->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   // left pane

   editor = new QTextEdit(this);
   editor->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   editor->setReadOnly(true);

   QFrame *frame = new QFrame(this);
   frame->setMinimumHeight(minHeight1);

   m = new QMenuBar(frame, "menu" );
   m->setMinimumHeight(minHeight1 - 5);
   m->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   QPopupMenu * file = new QPopupMenu(editor);
   m->insertItem( tr("&File"), file );
   file->insertItem( tr("Font"),  this, SLOT(update_font()),    ALT+Key_F );
   file->insertItem( tr("Save"),  this, SLOT(save()),    ALT+Key_S );
   file->insertItem( tr("Clear Display"), this, SLOT(clear_display()),   ALT+Key_X );
   editor->setWordWrap (QTextEdit::WidgetWidth);
   editor->setMinimumHeight(300);

   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_cancel = new QPushButton(tr("Close"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   // center pane

   lbl_csv = new QLabel("", this);
   lbl_csv->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_csv->setMinimumHeight(minHeight1);
   lbl_csv->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lv_csv = new QListView( this );
   lv_csv->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lv_csv->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   lv_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));
   lv_csv->setEnabled(true);
   lv_csv->setMinimumWidth( 300 );

   lv_csv->addColumn( "Models" );
   for ( unsigned int i = 6; i < csv1.header.size() - 1; i++ )
   {
      lv_csv->addColumn( csv1.header[i] );
   }

   lv_csv->setSorting        ( -1 );
   lv_csv->setRootIsDecorated( true );
   lv_csv->setSelectionMode  ( QListView::Multi );
   connect(lv_csv, SIGNAL(selectionChanged()), SLOT(csv_selection_changed()));

   lbl_csv->setText( csv1.name );
   csv_to_lv( csv1, lv_csv );

   te_csv = new QTextEdit(this);
   te_csv->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   te_csv->setReadOnly(true);
   te_csv->setMaximumHeight( minHeight1 * 4 );
   
   pb_csv_load = new QPushButton(tr("Load"), this);
   pb_csv_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_csv_load->setMinimumHeight(minHeight1);
   pb_csv_load->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_csv_load, SIGNAL(clicked()), SLOT(csv_load()));

   pb_csv_visualize = new QPushButton(tr("Visualize"), this);
   pb_csv_visualize->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_csv_visualize->setMinimumHeight(minHeight1);
   pb_csv_visualize->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_csv_visualize, SIGNAL(clicked()), SLOT(csv_visualize()));

   pb_csv_save = new QPushButton(tr("Save"), this);
   pb_csv_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_csv_save->setMinimumHeight(minHeight1);
   pb_csv_save->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_csv_save, SIGNAL(clicked()), SLOT(csv_save()));

   pb_csv_undo = new QPushButton(tr("Undo"), this);
   pb_csv_undo->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_csv_undo->setMinimumHeight(minHeight1);
   pb_csv_undo->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_csv_undo, SIGNAL(clicked()), SLOT(csv_undo()));

   pb_csv_cut = new QPushButton(tr("Cut"), this);
   pb_csv_cut->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_csv_cut->setMinimumHeight(minHeight1);
   pb_csv_cut->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_csv_cut, SIGNAL(clicked()), SLOT(csv_cut()));

   pb_csv_copy = new QPushButton(tr("Copy"), this);
   pb_csv_copy->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_csv_copy->setMinimumHeight(minHeight1);
   pb_csv_copy->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_csv_copy, SIGNAL(clicked()), SLOT(csv_copy()));

   pb_csv_paste = new QPushButton(tr("Paste"), this);
   pb_csv_paste->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_csv_paste->setMinimumHeight(minHeight1);
   pb_csv_paste->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_csv_paste, SIGNAL(clicked()), SLOT(csv_paste()));

   pb_csv_merge = new QPushButton(tr("Merge"), this);
   pb_csv_merge->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_csv_merge->setMinimumHeight(minHeight1);
   pb_csv_merge->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_csv_merge, SIGNAL(clicked()), SLOT(csv_merge()));

   pb_csv_reseq = new QPushButton(tr("Reseq"), this);
   pb_csv_reseq->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_csv_reseq->setMinimumHeight(minHeight1);
   pb_csv_reseq->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_csv_reseq, SIGNAL(clicked()), SLOT(csv_reseq()));

   // right pane

   lbl_csv2 = new QLabel("", this);
   lbl_csv2->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_csv2->setMinimumHeight(minHeight1);
   lbl_csv2->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_csv2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lv_csv2 = new QListView( this );
   lv_csv2->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lv_csv2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   lv_csv2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));
   lv_csv2->setEnabled(true);
   lv_csv2->setMinimumWidth( 300 );

   lv_csv2->addColumn( "Models" );
   for ( unsigned int i = 6; i < csv1.header.size() - 1; i++ )
   {
      lv_csv2->addColumn( csv1.header[i] );
   }

   lv_csv2->setSorting        ( -1 );
   lv_csv2->setRootIsDecorated( true );
   lv_csv2->setSelectionMode  ( QListView::Multi );
   connect(lv_csv2, SIGNAL(selectionChanged()), SLOT(csv2_selection_changed()));

   te_csv2 = new QTextEdit(this);
   te_csv2->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   te_csv2->setReadOnly(true);
   te_csv2->setMaximumHeight( minHeight1 * 4 );

   pb_csv2_load = new QPushButton(tr("Load"), this);
   pb_csv2_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_csv2_load->setMinimumHeight(minHeight1);
   pb_csv2_load->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_csv2_load, SIGNAL(clicked()), SLOT(csv2_load()));

   pb_csv2_visualize = new QPushButton(tr("Visualize"), this);
   pb_csv2_visualize->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_csv2_visualize->setMinimumHeight(minHeight1);
   pb_csv2_visualize->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_csv2_visualize, SIGNAL(clicked()), SLOT(csv2_visualize()));

   pb_csv2_dup = new QPushButton(tr("Dup"), this);
   pb_csv2_dup->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_csv2_dup->setMinimumHeight(minHeight1);
   pb_csv2_dup->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_csv2_dup, SIGNAL(clicked()), SLOT(csv2_dup()));

   pb_csv2_save = new QPushButton(tr("Save"), this);
   pb_csv2_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_csv2_save->setMinimumHeight(minHeight1);
   pb_csv2_save->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_csv2_save, SIGNAL(clicked()), SLOT(csv2_save()));

   pb_csv2_undo = new QPushButton(tr("Undo"), this);
   pb_csv2_undo->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_csv2_undo->setMinimumHeight(minHeight1);
   pb_csv2_undo->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_csv2_undo, SIGNAL(clicked()), SLOT(csv2_undo()));

   pb_csv2_cut = new QPushButton(tr("Cut"), this);
   pb_csv2_cut->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_csv2_cut->setMinimumHeight(minHeight1);
   pb_csv2_cut->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_csv2_cut, SIGNAL(clicked()), SLOT(csv2_cut()));

   pb_csv2_copy = new QPushButton(tr("Copy"), this);
   pb_csv2_copy->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_csv2_copy->setMinimumHeight(minHeight1);
   pb_csv2_copy->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_csv2_copy, SIGNAL(clicked()), SLOT(csv2_copy()));

   pb_csv2_paste = new QPushButton(tr("Paste"), this);
   pb_csv2_paste->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_csv2_paste->setMinimumHeight(minHeight1);
   pb_csv2_paste->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_csv2_paste, SIGNAL(clicked()), SLOT(csv2_paste()));

   pb_csv2_merge = new QPushButton(tr("Merge"), this);
   pb_csv2_merge->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_csv2_merge->setMinimumHeight(minHeight1);
   pb_csv2_merge->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_csv2_merge, SIGNAL(clicked()), SLOT(csv2_merge()));

   pb_csv2_reseq = new QPushButton(tr("Reseq"), this);
   pb_csv2_reseq->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_csv2_reseq->setMinimumHeight(minHeight1);
   pb_csv2_reseq->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_csv2_reseq, SIGNAL(clicked()), SLOT(csv2_reseq()));

   qwtw_wheel = new QwtWheel( this );
   qwtw_wheel->setOrientation  ( Qt::Vertical );
   qwtw_wheel->setMass         ( 1.0 );
   qwtw_wheel->setRange        ( 0.0, 0.0, 1 );
   // qwtw_wheel->setMinimumHeight( minHeight1 );
   connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );

   lbl_pos_range = new QLabel("0\nof\n0", this);
   lbl_pos_range->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
   lbl_pos_range->setMaximumHeight( minHeight1 * 2 + minHeight1 / 2 );
   lbl_pos_range->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_pos_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   // build layout
   QGridLayout *gl_panes = new QGridLayout( 0, 0, 0, 1, 1 );

   // left pane
   QBoxLayout *vbl_editor_group = new QVBoxLayout;
   vbl_editor_group->addWidget(frame);
   vbl_editor_group->addWidget(editor);

   gl_panes->addLayout( vbl_editor_group, 0, 0 );

   QBoxLayout *hbl_left_buttons_row_1 = new QHBoxLayout;
   hbl_left_buttons_row_1->addSpacing( 2 );
   
   QBoxLayout *hbl_left_buttons_row_2 = new QHBoxLayout;
   hbl_left_buttons_row_2->addWidget( pb_help );
   hbl_left_buttons_row_2->addSpacing( 2 );
   hbl_left_buttons_row_2->addWidget( pb_cancel );

   QBoxLayout *vbl_left_buttons = new QVBoxLayout;
   vbl_left_buttons->addLayout( hbl_left_buttons_row_1 );
   vbl_left_buttons->addLayout( hbl_left_buttons_row_2 );

   gl_panes->addLayout( vbl_left_buttons, 1, 0 );

   // center pane
   QBoxLayout *vbl_center_top = new QVBoxLayout;
   vbl_center_top->addWidget( lbl_csv );
   vbl_center_top->addWidget( lv_csv );
   vbl_center_top->addWidget( te_csv );

   gl_panes->addLayout( vbl_center_top, 0, 1 );

   QBoxLayout *hbl_center_buttons_row_1 = new QHBoxLayout;
   hbl_center_buttons_row_1->addWidget( pb_csv_load );
   hbl_center_buttons_row_1->addSpacing( 2 );
   hbl_center_buttons_row_1->addWidget( pb_csv_visualize );
   hbl_center_buttons_row_1->addSpacing( 2 );
   hbl_center_buttons_row_1->addWidget( pb_csv_save );
   hbl_center_buttons_row_1->addSpacing( 2 );
   hbl_center_buttons_row_1->addWidget( pb_csv_undo );
   
   QBoxLayout *hbl_center_buttons_row_2 = new QHBoxLayout;
   hbl_center_buttons_row_2->addWidget( pb_csv_cut );
   hbl_center_buttons_row_2->addSpacing( 2 );
   hbl_center_buttons_row_2->addWidget( pb_csv_copy );
   hbl_center_buttons_row_2->addSpacing( 2 );
   hbl_center_buttons_row_2->addWidget( pb_csv_paste );
   hbl_center_buttons_row_2->addSpacing( 2 );
   hbl_center_buttons_row_2->addWidget( pb_csv_merge );
   hbl_center_buttons_row_2->addSpacing( 2 );
   hbl_center_buttons_row_2->addWidget( pb_csv_reseq );

   QBoxLayout *vbl_center_buttons = new QVBoxLayout;
   vbl_center_buttons->addLayout( hbl_center_buttons_row_1 );
   vbl_center_buttons->addLayout( hbl_center_buttons_row_2 );

   gl_panes->addLayout( vbl_center_buttons, 1, 1 );

   // right pane
   QBoxLayout *vbl_wheel = new QVBoxLayout;
   vbl_wheel->addWidget( qwtw_wheel );
   vbl_wheel->addWidget( lbl_pos_range );

   QBoxLayout *hbl_csv2_wheel = new QHBoxLayout;
   hbl_csv2_wheel->addWidget( lv_csv2 );
   hbl_csv2_wheel->addLayout( vbl_wheel );

   QBoxLayout *vbl_right_top = new QVBoxLayout;
   vbl_right_top->addWidget( lbl_csv2 );
   vbl_right_top->addLayout( hbl_csv2_wheel );
   vbl_right_top->addWidget( te_csv2 );

   gl_panes->addLayout( vbl_right_top, 0, 2 );

   QBoxLayout *hbl_right_buttons_row_1 = new QHBoxLayout;
   hbl_right_buttons_row_1->addWidget( pb_csv2_load );
   hbl_right_buttons_row_1->addSpacing( 2 );
   hbl_right_buttons_row_1->addWidget( pb_csv2_visualize );
   hbl_right_buttons_row_1->addSpacing( 2 );
   hbl_right_buttons_row_1->addWidget( pb_csv2_dup );
   hbl_right_buttons_row_1->addSpacing( 2 );
   hbl_right_buttons_row_1->addWidget( pb_csv2_save );
   hbl_right_buttons_row_1->addSpacing( 2 );
   hbl_right_buttons_row_1->addWidget( pb_csv2_undo );
   
   QBoxLayout *hbl_right_buttons_row_2 = new QHBoxLayout;
   hbl_right_buttons_row_2->addWidget( pb_csv2_cut );
   hbl_right_buttons_row_2->addSpacing( 2 );
   hbl_right_buttons_row_2->addWidget( pb_csv2_copy );
   hbl_right_buttons_row_2->addSpacing( 2 );
   hbl_right_buttons_row_2->addWidget( pb_csv2_paste );
   hbl_right_buttons_row_2->addSpacing( 2 );
   hbl_right_buttons_row_2->addWidget( pb_csv2_merge );
   hbl_right_buttons_row_2->addSpacing( 2 );
   hbl_right_buttons_row_2->addWidget( pb_csv2_reseq );

   QBoxLayout *vbl_right_buttons = new QVBoxLayout;
   vbl_right_buttons->addLayout( hbl_right_buttons_row_1 );
   vbl_right_buttons->addLayout( hbl_right_buttons_row_2 );

   gl_panes->addLayout( vbl_right_buttons, 1, 2 );

   QVBoxLayout *background = new QVBoxLayout(this);
   background->addSpacing( 2 );
   background->addWidget ( lbl_title );
   background->addSpacing( 2 );
   background->addLayout ( gl_panes );
}

void US_Hydrodyn_Pdb_Tool::cancel()
{
   close();
}

void US_Hydrodyn_Pdb_Tool::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_pdb_tool.html");
}

void US_Hydrodyn_Pdb_Tool::closeEvent(QCloseEvent *e)
{
   global_Xpos -= 30;
   global_Ypos -= 30;
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
   QString fn;
   fn = QFileDialog::getSaveFileName(QString::null, QString::null,this );
   if(!fn.isEmpty() )
   {
      QString text = editor->text();
      QFile f( fn );
      if ( !f.open( IO_WriteOnly | IO_Translate) )
      {
         return;
      }
      QTextStream t( &f );
      t << text;
      f.close();
      editor->setModified( false );
      setCaption( fn );
   }
}

void US_Hydrodyn_Pdb_Tool::update_enables_csv()
{
   bool any_csv_selected  = any_selected( lv_csv );
   bool any_csv2_selected = any_selected( lv_csv2 );

   pb_csv_load      ->setEnabled( false );
   pb_csv_visualize ->setEnabled( csv1.data.size() );
   pb_csv_save      ->setEnabled( csv1.data.size() );
   pb_csv_undo      ->setEnabled( csv_undos.size() );
   pb_csv_cut       ->setEnabled( any_csv_selected );
   pb_csv_copy      ->setEnabled( any_csv_selected );
   pb_csv_paste     ->setEnabled( csv_clipboard.data.size() );
   pb_csv_merge     ->setEnabled( any_csv_selected && merge_ok() );
   pb_csv2_merge    ->setEnabled( any_csv2_selected && merge_ok() );
   pb_csv_reseq     ->setEnabled( csv1.data.size() );
}

void US_Hydrodyn_Pdb_Tool::update_enables_csv2()
{
   bool any_csv_selected  = any_selected( lv_csv );
   bool any_csv2_selected = any_selected( lv_csv2 );

   pb_csv2_load      ->setEnabled( false );
   pb_csv2_visualize ->setEnabled( csv2.data.size() );
   pb_csv2_dup       ->setEnabled( csv1.data.size() );
   pb_csv2_save      ->setEnabled( csv2.data.size() );
   pb_csv2_undo      ->setEnabled( csv2_undos.size() );
   pb_csv2_cut       ->setEnabled( any_csv2_selected );
   pb_csv2_copy      ->setEnabled( any_csv2_selected );
   pb_csv2_paste     ->setEnabled( csv_clipboard.data.size() );
   pb_csv_merge      ->setEnabled( any_csv_selected && merge_ok() );
   pb_csv2_merge     ->setEnabled( any_csv2_selected && merge_ok() );
   pb_csv2_reseq     ->setEnabled( csv2.data.size() );
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

   QListViewItemIterator it( lv_csv );
   while ( it.current() ) {
      if ( lv_csv->isSelected( it.current() ) )
      {
         delete it.current();
      }
      ++it;
   }
   selection_since_count_csv1 = true;
   update_enables();
}

void US_Hydrodyn_Pdb_Tool::csv_copy()
{
   csv_clipboard = to_csv( lv_csv, csv1, true );
   csv_clipboard.name = "selection of " + csv_clipboard.name;
   update_enables();
}

void US_Hydrodyn_Pdb_Tool::csv_paste()
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

void US_Hydrodyn_Pdb_Tool::csv_visualize()
{
   visualize( lv_csv );
}

void US_Hydrodyn_Pdb_Tool::csv2_load()
{
}

void US_Hydrodyn_Pdb_Tool::csv2_dup()
{
   csv2_undos.push_back( to_csv( lv_csv2, csv2, false ) );
   csv2 = to_csv( lv_csv, csv1 );
   csv2.name = "duplicate of " + csv2.name;
   csv_to_lv( csv2, lv_csv2 );
   update_enables_csv2();
}

void US_Hydrodyn_Pdb_Tool::csv2_save()
{
   save_csv( lv_csv2 );
}

void US_Hydrodyn_Pdb_Tool::save_csv( QListView *lv )
{
   QString use_dir = 
      ((US_Hydrodyn *)us_hydrodyn)->path_view_pdb.isEmpty() ?
      ((US_Hydrodyn *)us_hydrodyn)->somo_pdb_dir :
      ((US_Hydrodyn *)us_hydrodyn)->path_view_pdb;

   QString filename = QFileDialog::getSaveFileName(
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
      filename = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(filename);
   }

   QFile f(filename);

   if ( !f.open( IO_WriteOnly ) )
   {
      QMessageBox::warning( this, "UltraScan",
                            QString(tr("Could not open %1 for writing!")).arg(filename) );
      return;
   }

   QTextStream t( &f );

   csv csv_to_save;
   if ( lv == lv_csv )
   {
      csv_to_save = to_csv( lv_csv, csv1 );
   } else {
      csv_to_save = to_csv( lv_csv2, csv2 );
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
   if ( !csv2_undos.size() )
   {
      editor_msg("red", "Internal error: undo called with empty undo buffer" );
      return;
   }
   csv2 = csv2_undos[ csv2_undos.size() - 1 ];
   csv2_undos.pop_back();
   csv_to_lv( csv2, lv_csv2 );
   update_enables_csv2();
}

void US_Hydrodyn_Pdb_Tool::csv2_cut()
{
   // lv_csv2_undo.push_back( *lv_csv2 );
   csv2_undos.push_back( to_csv( lv_csv2, csv2, false ) );
   csv2_copy();
   if ( !csv2.name.contains(QRegExp("^edit of")) )
   {
      csv2.name = "edit of " + csv2.name;
      lbl_csv2->setText( csv2.name );
   }
   QListViewItemIterator it( lv_csv2 );
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
   csv_clipboard = to_csv( lv_csv2, csv2, true );
   csv_clipboard.name = "selection of " + csv_clipboard.name;
   update_enables();
}

void US_Hydrodyn_Pdb_Tool::csv2_paste()
{
   csv2_undos.push_back( to_csv( lv_csv2, csv2, false ) );
   csv2 = csv_clipboard;
   csv_to_lv( csv2, lv_csv2 );
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

void US_Hydrodyn_Pdb_Tool::adjust_wheel( double )
{
}

unsigned int US_Hydrodyn_Pdb_Tool::count_selected( QListView *lv )
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
   unsigned int number_selected = 0;
   if ( lv_is_csv )
   {
      csv_selected_element_counts.clear();
   } else {
      csv2_selected_element_counts.clear();
   }
   
   QListViewItemIterator it( lv );
   while ( it.current() ) {
      QListViewItem *item = it.current();
      if ( !item->childCount() && is_selected( item ) )
      {
         number_selected++;
         if ( lv_is_csv )
         {
            csv_selected_element_counts [ csv1.data[ csv1.key[ key( item ) ] ][ 13 ] ]++;
         } else {
            csv2_selected_element_counts[ csv2.data[ csv2.key[ key( item ) ] ][ 13 ] ]++;
         }
      }
      ++it;
   }
   if ( lv_is_csv )
   {
      selection_since_count_csv1 = false;
      last_count_csv1 = number_selected;
   } else {
      selection_since_count_csv2 = false;
      last_count_csv2 = number_selected;
   }
   return number_selected;
}

bool US_Hydrodyn_Pdb_Tool::any_selected( QListView *lv )
{
   if ( lv == lv_csv && !selection_since_count_csv1 )
   {
      return last_count_csv1 != 0;
   }
   if ( lv == lv_csv2 && !selection_since_count_csv2 )
   {
      return last_count_csv2 != 0;
   }

   QListViewItemIterator it( lv );
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
   // csv_msg("black", QString("selection changed # selected %1").arg( count_selected( lv_csv ) ) );
   selection_since_count_csv1 = true;
   update_enables_csv();
}

void US_Hydrodyn_Pdb_Tool::csv2_selection_changed()
{
   // csv2_msg("black", QString("selection changed # selected %1").arg( count_selected( lv_csv2 ) ) );
   selection_since_count_csv2 = true;
   update_enables_csv2();
}

csv US_Hydrodyn_Pdb_Tool::to_csv( QListView *lv, csv &ref_csv, bool only_selected )
{
   QListViewItemIterator it( lv );
   csv csv1;
   csv1.header        = ref_csv.header;
   csv1.name          = ref_csv.name;
   csv1.filename      = ref_csv.filename;
   csv1.title_text    = ref_csv.title_text;
   csv1.header_text   = ref_csv.header_text;

   while ( it.current() ) {
      QListViewItem *item = it.current();
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

bool US_Hydrodyn_Pdb_Tool::is_selected( QListViewItem *lvi )
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

bool US_Hydrodyn_Pdb_Tool::child_selected( QListViewItem *lvi )
{
   if ( lvi->isSelected() )
   {
      return true;
   }

   if ( lvi->childCount() )
   {
      QListViewItem *myChild = lvi->firstChild();
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

QString US_Hydrodyn_Pdb_Tool::key( QListViewItem *lvi )
{
   QString key = lvi->text( 0 );
   while ( lvi->parent() )
   {
      lvi = lvi->parent();
      key = lvi->text( 0 ) + "~" + key;
   }
   return key;
}

void US_Hydrodyn_Pdb_Tool::csv_setup_keys( csv &csv1 )
{
   csv1.key.clear();

   for ( unsigned int i = 0; i < csv1.data.size(); i++ )
   {
      csv1.key[ 
               QString( "%1~%2~%3~%4" )
               .arg( csv1.data[ i ][ 0 ] )
               .arg( csv1.data[ i ][ 1 ] )
               .arg( csv1.data[ i ][ 2 ] + " " + csv1.data[ i ][ 3 ] )
               .arg( csv1.data[ i ][ 4 ] + " " + csv1.data[ i ][ 5 ] ) ] = i;
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
      
void US_Hydrodyn_Pdb_Tool::csv_to_lv( csv &csv1, QListView *lv )
{
   lv->clear();

   map < QString, QListViewItem * >   models;
   map < QString, QListViewItem * >   model_chains;
   map < QString, QListViewItem * >   model_chain_residues;
   map < QString, QListViewItem * >   model_chain_residue_atoms;

   QString last_model   = "none";
   QString last_chain   = "none";
   QString last_residue = "none";

   QListViewItem *current = ( QListViewItem * )0;

   for ( int i = csv1.data.size() - 1; i >= 0; i-- )
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
         models[ model ] = new QListViewItem( lv, model );
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
            
         model_chains[ model_chain ] = new QListViewItem( models[ model ], chain );
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
         model_chain_residues[ model_chain_residue ] = new QListViewItem( model_chains[ model_chain ], residue );
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
         new QListViewItem( model_chain_residues[ model_chain_residue ], 
                            atom,
                            csv1.data[ i ][ 6 ],
                            csv1.data[ i ][ 7 ],
                            csv1.data[ i ][ 8 ],
                            csv1.data[ i ][ 9 ],
                            csv1.data[ i ][ 10 ],
                            csv1.data[ i ][ 11 ],
                            csv1.data[ i ][ 12 ]
                            );
      if ( csv1.current_item_key == model_chain_residue_atom )
      {
         current = model_chain_residue_atoms[ model_chain_residue_atom ];
      }
      if ( csv1.key.count( model_chain_residue_atom ) )
      {
         model_chain_residue_atoms[ model_chain_residue_atom ]->setVisible ( csv1.visible [ csv1.key[ model_chain_residue_atom ] ] );
         model_chain_residue_atoms[ model_chain_residue_atom ]->setSelected( csv1.selected[ csv1.key[ model_chain_residue_atom ] ] );
         model_chain_residue_atoms[ model_chain_residue_atom ]->setOpen    ( csv1.open    [ csv1.key[ model_chain_residue_atom ] ] );
      } else {
         // editor_msg("red", QString( "Error: model_chain_residue_atom %1 visi/sel/open not key not found" ).arg( model_chain_residue_atom ) );
      }
   }
   lv->setCurrentItem( current );

   if ( lv == lv_csv )
   {
      lbl_csv->setText( csv1.name );
      selection_since_count_csv1 = true;
   } else {
      lbl_csv2->setText( csv1.name );
      selection_since_count_csv2 = true;
   }
}

bool US_Hydrodyn_Pdb_Tool::merge_ok()
{
   unsigned int sel1 = count_selected( lv_csv );
   unsigned int sel2 = count_selected( lv_csv2 );
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

QString US_Hydrodyn_Pdb_Tool::csv_to_pdb( csv &csv1 )
{
   QString s;

   for ( unsigned int i = 0; i < csv1.header_text.size(); i++ )
   {
      s += QString( "HEADER   %1" + csv1.header_text[ i ] + "\n" )
         .arg( i ? QString("%1").arg(i + 1) : " " );
   }

   for ( unsigned int i = 0; i < csv1.title_text.size(); i++ )
   {
      s += QString( "TITLE    %1" + csv1.title_text[ i ] + "\n" )
         .arg( i ? QString("%1").arg(i + 1) : " " );
   }

   QString last_model = "none";

   for ( unsigned int i = 0; i < csv1.data.size(); i++ )
   {
      QString model      = csv1.data[ i ][ 0 ];
      
      if ( last_model != model )
      {
         if ( last_model != "none" )
         {
            s += "ENDMDL\n";
         }
         last_model = model;
         s += "MODEL " + model + "\n";
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
   s += "ENDMDL\nEND\n";

   return s;
}

void US_Hydrodyn_Pdb_Tool::visualize( QListView *lv )
{
   csv tmp_csv;

   if ( lv == lv_csv )
   {
      tmp_csv = to_csv( lv, csv1 );
   } else {
      tmp_csv = to_csv( lv, csv2 );
   }

   QString use_dir = ((US_Hydrodyn *)us_hydrodyn)->somo_tmp_dir + QDir::separator();

   unsigned int pos = 0;
   QString filename;

   do {
      filename = QString("%1temp%2.pdb").arg( use_dir ).arg( pos );
      pos++;
   } while( QFile::exists( filename ) );

   QFile f( filename );

   if ( !f.open( IO_WriteOnly ) )
   {
      QMessageBox::warning( this, "UltraScan",
                            QString(tr("Could not open %1 for writing!")).arg(filename) );
      return;
   }

   QTextStream t( &f );
   t << csv_to_pdb( tmp_csv );
   f.close();

   QProcess *rasmol = new QProcess( this );

   QStringList argument;
#if !defined(WIN32)
   argument.append("xterm");
   argument.append("-e");
#endif
#if defined(BIN64)
   argument.append(USglobal->config_list.system_dir + SLASH + "bin64" + SLASH + "rasmol");
#else
   argument.append(USglobal->config_list.system_dir + SLASH + "bin" + SLASH + "rasmol");
#endif
   argument.append( QFileInfo( filename ).fileName() );
   rasmol->setWorkingDirectory(QFileInfo( filename ).dirPath());
   rasmol->setArguments(argument);
   if ( !rasmol->start() )
   {
      QMessageBox::message(tr("Please note:"), tr("There was a problem starting RASMOL\n"
                                                  "Please check to make sure RASMOL is properly installed..."));
   }
}
