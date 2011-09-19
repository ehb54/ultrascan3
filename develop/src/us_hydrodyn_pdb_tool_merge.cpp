#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_pdb_tool_merge.h"

#define SLASH QDir::separator()

ostream& operator<<(ostream& out, const range_entry& c)
{
   out << "chain: " << c.chain << " " << c.start << "-" << c.end;
   return out;
}

US_Hydrodyn_Pdb_Tool_Merge::US_Hydrodyn_Pdb_Tool_Merge(
                                                       void *us_hydrodyn, 
                                                       void *pdb_tool_window, 
                                                       QWidget *p, 
                                                       const char *name
                                                       ) : QFrame(p, name)
{
   this->us_hydrodyn = us_hydrodyn;
   this->pdb_tool_window = pdb_tool_window;
   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("US-SOMO: PDB Editor Cut/Splice Control"));

   ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->pdb_tool_merge_widget = true;
   lv_csv_from = ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->lv_csv;
   lv_csv_to   = ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->lv_csv2;

   reset_csv_commands();

   setupGUI();
   running = false;

   update_enables();

   global_Xpos += 30;
   global_Ypos += 30;

   unsigned int csv_height = t_csv->rowHeight(0) + 30;
   unsigned int csv_width = t_csv->columnWidth(0) + 45;
   for ( int i = 0; i < t_csv->numRows(); i++ )
   {
      csv_height += t_csv->rowHeight(i);
   }
   for ( int i = 1; i < t_csv->numCols(); i++ )
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

   csv_commands.header_map.clear();
   csv_commands.data.clear();
   csv_commands.num_data.clear();
   csv_commands.prepended_names.clear();

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
   int minHeight1 = 30;
   int minHeight3 = 30;

   lbl_title = new QLabel(csv_commands.name.left(80), this);
   lbl_title->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   t_csv = new QTable(csv_commands.data.size(), csv_commands.header.size(), this);
   t_csv->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   // t_csv->setMinimumHeight(minHeight1 * 3);
   // t_csv->setMinimumWidth(minWidth1);
   t_csv->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   t_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));
   t_csv->setEnabled(true);

   for ( unsigned int i = 0; i < csv_commands.num_data.size(); i++ )
   {
      for ( unsigned int j = 0; j < csv_commands.num_data[i].size(); j++ )
      {
         if ( csv_commands.data[i][j] == "Y" || csv_commands.data[i][j] == "N" )
         {
            t_csv->setItem( i, j, new QCheckTableItem( t_csv, "" ) );
            ((QCheckTableItem *)(t_csv->item( i, j )))->setChecked( csv_commands.data[i][j] == "Y" );
         } else {
            t_csv->setText( i, j, csv_commands.data[i][j] );
         }
      }
   }

   for ( unsigned int i = 0; i < csv_commands.header.size(); i++ )
   {
      t_csv->horizontalHeader()->setLabel(i, csv_commands.header[i]);
   }

   t_csv->setSorting(false);
   t_csv->setRowMovingEnabled(true);
   t_csv->setColumnMovingEnabled(false);
   t_csv->setReadOnly(true);

   // t_csv->setColumnWidth(0, 200);
   // t_csv->setColumnReadOnly(0, true);
   // t_csv->setColumnReadOnly(t_csv->numCols() - 1, true);
   t_csv->setSelectionMode( QTable::SingleRow );

   // connect(t_csv, SIGNAL(valueChanged(int, int)), SLOT(table_value(int, int )));

   pb_sel_auto = new QPushButton(tr("Compute Guess"), this);
   pb_sel_auto->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_sel_auto->setMinimumHeight(minHeight1);
   pb_sel_auto->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_sel_auto, SIGNAL(clicked()), SLOT(sel_auto()));

   pb_sel_from_to_merge = new QPushButton(tr("From -> Merge"), this);
   pb_sel_from_to_merge->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_sel_from_to_merge->setMinimumHeight(minHeight1);
   pb_sel_from_to_merge->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_sel_from_to_merge, SIGNAL(clicked()), SLOT(sel_from_to_merge()));

   pb_sel_from_to_fit = new QPushButton(tr("From -> Fit"), this);
   pb_sel_from_to_fit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_sel_from_to_fit->setMinimumHeight(minHeight1);
   pb_sel_from_to_fit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_sel_from_to_fit, SIGNAL(clicked()), SLOT(sel_from_to_fit()));

   pb_sel_to_to_fit = new QPushButton(tr("To -> Fit"), this);
   pb_sel_to_to_fit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_sel_to_to_fit->setMinimumHeight(minHeight1);
   pb_sel_to_to_fit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_sel_to_to_fit, SIGNAL(clicked()), SLOT(sel_to_to_fit()));

   pb_sel_to_to_cut = new QPushButton(tr("To -> Cut"), this);
   pb_sel_to_to_cut->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_sel_to_to_cut->setMinimumHeight(minHeight1);
   pb_sel_to_to_cut->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_sel_to_to_cut, SIGNAL(clicked()), SLOT(sel_to_to_cut()));

   pb_clear = new QPushButton(tr("Clear"), this);
   pb_clear->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_clear->setMinimumHeight(minHeight1);
   pb_clear->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_clear, SIGNAL(clicked()), SLOT(clear()));

   pb_load = new QPushButton(tr("Load"), this);
   pb_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_load->setMinimumHeight(minHeight1);
   pb_load->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_load, SIGNAL(clicked()), SLOT(load()));

   pb_validate = new QPushButton(tr("Validate"), this);
   pb_validate->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_validate->setMinimumHeight(minHeight1);
   pb_validate->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_validate, SIGNAL(clicked()), SLOT(validate()));

   pb_csv_save = new QPushButton(tr("Save"), this);
   pb_csv_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_csv_save->setMinimumHeight(minHeight1);
   pb_csv_save->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_csv_save, SIGNAL(clicked()), SLOT(csv_save()));

   pb_chains_from = new QPushButton(tr("Chains From"), this);
   pb_chains_from->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_chains_from->setMinimumHeight(minHeight1);
   pb_chains_from->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_chains_from, SIGNAL(clicked()), SLOT(chains_from()));

   le_chains_from = new QLineEdit(this, "chains_from Line Edit");
   le_chains_from->setText("");
   le_chains_from->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   // le_chains_from->setMinimumWidth(150);
   le_chains_from->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_chains_from->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_chains_from->setReadOnly(true);

   pb_chains_to = new QPushButton(tr("Chains To"), this);
   pb_chains_to->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_chains_to->setMinimumHeight(minHeight1);
   pb_chains_to->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_chains_to, SIGNAL(clicked()), SLOT(chains_to()));

   le_chains_to = new QLineEdit(this, "chains_to Line Edit");
   le_chains_to->setText("");
   le_chains_to->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   // le_chains_to->setMinimumWidth(150);
   le_chains_to->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_chains_to->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_chains_to->setReadOnly(true);

   pb_target = new QPushButton(tr("Target"), this);
   pb_target->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_target->setMinimumHeight(minHeight1);
   pb_target->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_target, SIGNAL(clicked()), SLOT(target()));

   le_target = new QLineEdit(this, "target Line Edit");
   le_target->setText("");
   le_target->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   // le_target->setMinimumWidth(150);
   le_target->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_target->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_target->setReadOnly(true);

   progress = new QProgressBar(this, "Progress");
   progress->setMinimumHeight(minHeight1);
   progress->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   progress->reset();

   pb_start = new QPushButton(tr("Start"), this);
   pb_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_start->setMinimumHeight(minHeight1);
   pb_start->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_start, SIGNAL(clicked()), SLOT(start()));

   pb_stop = new QPushButton(tr("Stop"), this);
   pb_stop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_stop->setMinimumHeight(minHeight1);
   pb_stop->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_stop, SIGNAL(clicked()), SLOT(stop()));

   editor = new QTextEdit(this);
   editor->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   editor->setReadOnly(true);

   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight3);

   m = new QMenuBar(frame, "menu" );
   m->setMinimumHeight(minHeight1 - 5);
   m->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   QPopupMenu * file = new QPopupMenu(editor);
   m->insertItem( tr("&File"), file );
   file->insertItem( tr("Font"),  this, SLOT(update_font()),    ALT+Key_F );
   file->insertItem( tr("Save"),  this, SLOT(save()),    ALT+Key_S );
   file->insertItem( tr("Clear Display"), this, SLOT(clear_display()),   ALT+Key_X );
   editor->setWordWrap (QTextEdit::WidgetWidth);
   editor->setMinimumHeight(100);
   
   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_cancel = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   // build layout

   QGridLayout *gl_files = new QGridLayout(0);

   gl_files->addWidget( pb_chains_from, 0, 0 );
   gl_files->addWidget( le_chains_from, 0, 1 );
   gl_files->addWidget( pb_chains_to  , 1, 0 );
   gl_files->addWidget( le_chains_to  , 1, 1 );
   gl_files->addWidget( pb_target     , 2, 0 );
   gl_files->addWidget( le_target     , 2, 1 );

   QHBoxLayout *hbl_sel_cmds = new QHBoxLayout(0);
   hbl_sel_cmds->addWidget( pb_sel_auto );
   hbl_sel_cmds->addWidget( pb_sel_from_to_merge );
   hbl_sel_cmds->addWidget( pb_sel_from_to_fit );
   hbl_sel_cmds->addWidget( pb_sel_to_to_fit );
   hbl_sel_cmds->addWidget( pb_sel_to_to_cut );

   QHBoxLayout *hbl_load_save = new QHBoxLayout(0);
   hbl_load_save->addWidget( pb_clear );
   hbl_load_save->addWidget( pb_load );
   hbl_load_save->addWidget( pb_validate );
   hbl_load_save->addWidget( pb_csv_save );

   QHBoxLayout *hbl_controls = new QHBoxLayout(0);
   hbl_controls->addSpacing(4);
   hbl_controls->addWidget(pb_start);
   hbl_controls->addSpacing(4);
   hbl_controls->addWidget(pb_stop);
   hbl_controls->addSpacing(4);

   QHBoxLayout *hbl_bottom = new QHBoxLayout(0);
   hbl_bottom->addSpacing(4);
   hbl_bottom->addWidget(pb_help);
   hbl_bottom->addSpacing(4);
   hbl_bottom->addWidget(pb_cancel);
   hbl_bottom->addSpacing(4);

   QBoxLayout *vbl_editor_group = new QVBoxLayout(0);
   vbl_editor_group->addWidget(frame);
   vbl_editor_group->addWidget(editor);

   QVBoxLayout *background = new QVBoxLayout(this);
   background->addSpacing(4);
   background->addWidget(lbl_title);
   background->addSpacing(4);
   background->addWidget(t_csv);
   background->addSpacing(4);
   background->addLayout(hbl_sel_cmds);
   background->addSpacing(4);
   background->addLayout(hbl_load_save);
   background->addSpacing(4);
   background->addLayout(vbl_editor_group);
   background->addSpacing(4);
   background->addLayout(gl_files);
   background->addSpacing(4);
   background->addWidget(progress);
   background->addSpacing(4);
   background->addLayout(hbl_controls);
   background->addSpacing(4);
   background->addLayout(hbl_bottom);
   background->addSpacing(4);
}

void US_Hydrodyn_Pdb_Tool_Merge::cancel()
{
   close();
}

void US_Hydrodyn_Pdb_Tool_Merge::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_pdb_editor_merge.html");
}

void US_Hydrodyn_Pdb_Tool_Merge::closeEvent(QCloseEvent *e)
{
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Pdb_Tool_Merge::clear_display()
{
   editor->clear();
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


void US_Hydrodyn_Pdb_Tool_Merge::start()
{
   running = true;
   update_enables();
   // process
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
   bool           from_exists = QFile::exists( le_chains_from->text() );
   bool           to_exists   = QFile::exists( le_chains_to->text() );
   bool           any_from_selected  = ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->any_selected( lv_csv_from );
   bool           any_to_selected    = ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->any_selected( lv_csv_to );
   // pdb_sel_count  counts             = ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->count_selected( lv_csv_to );

   pb_clear            ->setEnabled( !running && t_csv->numRows() );
   pb_load             ->setEnabled( !running );
   pb_validate         ->setEnabled( !running && from_exists && to_exists && t_csv->numRows() );
   pb_csv_save         ->setEnabled( !running && t_csv->numRows() );

   pb_start            ->setEnabled( !running && from_exists && to_exists && t_csv->numRows() );
   pb_stop             ->setEnabled( running );
   pb_sel_auto         ->setEnabled( !running && from_exists && to_exists );

   pb_sel_auto         ->setEnabled( !running && from_exists && to_exists );
   pb_sel_from_to_merge->setEnabled( !running && any_from_selected );
   pb_sel_from_to_fit  ->setEnabled( !running && any_from_selected );
   pb_sel_to_to_fit    ->setEnabled( !running && any_to_selected );
   pb_sel_to_to_cut    ->setEnabled( !running && any_to_selected );
}

void US_Hydrodyn_Pdb_Tool_Merge::editor_msg( QString color, QString msg )
{
   QColor save_color = editor->color();
   editor->setColor(color);
   editor->append(msg);
   editor->setColor(save_color);
}

void US_Hydrodyn_Pdb_Tool_Merge::chains_from()
{
   QString filename;
   ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->load( lv_csv_from,
                                                    filename,
                                                    true );
   le_chains_from->setText( filename );
   update_enables();
}

void US_Hydrodyn_Pdb_Tool_Merge::chains_to()
{
   QString filename;
   ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->load( lv_csv_to,
                                                    filename,
                                                    true );
   le_chains_to->setText( filename );
   update_enables();
}

void US_Hydrodyn_Pdb_Tool_Merge::target()
{
   QString filename = QFileDialog::getSaveFileName( "", 
                                                    "PDB (*.pdb *.PDB)",
                                                    this,
                                                    "select target",
                                                    "Choose a name to save the result" );
   
   le_target->setText( filename );

   if ( filename.isEmpty() )
   {
      return;
   }
   update_enables();
}

void US_Hydrodyn_Pdb_Tool_Merge::sel_auto()
{
}

void US_Hydrodyn_Pdb_Tool_Merge::sel_to_range( QListView *lv, vector < range_entry > &ranges )
{
   csv csv1 = ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->to_csv( lv, 
                                                                 lv == lv_csv_from ?
                                                                 ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv1 :
                                                                 ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv2[ ((US_Hydrodyn_Pdb_Tool *)pdb_tool_window)->csv2_pos ],
                                                                 true );
   ranges.clear();

   range_entry range;

   map < QString, unsigned int > range_pos;

   for ( unsigned int i = 0; i < csv1.data.size(); i++ )
   {
      if ( csv1.data.size() < 14 )
      {
         editor_msg( "red", tr( "Internal error: Invalid selection csv" ) );
         return;
      }
      range.chain = csv1.data[ i ][ 1 ];
      unsigned int pos = csv1.data[ i ][ 3 ].toUInt();
      if ( range_pos.count( range.chain ) )
      {
         unsigned int rpos = range_pos[ range.chain ];
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
         range_pos[ range.chain ] = ranges.size();
         ranges.push_back( range );
      }
   }

   // for ( unsigned int i = 0; i < ranges.size(); i++ )
   // {
   // cout << ranges[ i ] << endl;
   // }
}

void US_Hydrodyn_Pdb_Tool_Merge::make_csv_chain_map()
{
   csv_chain_map.clear();
   for (int i = 0; i < t_csv->numRows(); i++ )
   {
      csv_chain_map[ t_csv->text( i, 0 ) ] = i;
   }
}

void US_Hydrodyn_Pdb_Tool_Merge::update_t_csv_range( vector < range_entry > &ranges,
                                                     unsigned int col_start,
                                                     unsigned int col_end )
{
   make_csv_chain_map();
   unsigned int pos;
   for ( unsigned int i = 0; i < ranges.size(); i++ )
   {
      if ( csv_chain_map.count( ranges[ i ].chain ) )
      {
         pos = csv_chain_map[ ranges[ i ].chain ];
      } else {
         pos = ( unsigned int ) t_csv->numRows();
         t_csv->setNumRows( pos + 1 );
         t_csv->setText( pos, 0, ranges[ i ].chain );
      }
      t_csv->setText( pos, col_start, QString("%1").arg( ranges[ i ].start ) );
      t_csv->setText( pos, col_end  , QString("%1").arg( ranges[ i ].end ) );
   }
}

void US_Hydrodyn_Pdb_Tool_Merge::sel_from_to_merge()
{
   vector < range_entry > ranges;
   sel_to_range( lv_csv_from, ranges );
   update_t_csv_range( ranges, 1, 2 );
   update_enables();
}

void US_Hydrodyn_Pdb_Tool_Merge::sel_from_to_fit()
{
   vector < range_entry > ranges;
   sel_to_range( lv_csv_from, ranges );
   update_t_csv_range( ranges, 3, 4 );
   update_enables();
}

void US_Hydrodyn_Pdb_Tool_Merge::sel_to_to_fit()
{
   vector < range_entry > ranges;
   sel_to_range( lv_csv_to, ranges );
   update_t_csv_range( ranges, 3, 4 );
   update_enables();
}

void US_Hydrodyn_Pdb_Tool_Merge::sel_to_to_cut()
{
   vector < range_entry > ranges;
   sel_to_range( lv_csv_to, ranges );
   update_t_csv_range( ranges, 5, 6 );
   update_enables();
}

void US_Hydrodyn_Pdb_Tool_Merge::clear()
{
   for ( int i = t_csv->numRows() - 1; i >= 0; i-- )
   {
      t_csv->removeRow( i );
   }
   update_enables();
}

void US_Hydrodyn_Pdb_Tool_Merge::load()
{
}

void US_Hydrodyn_Pdb_Tool_Merge::validate()
{
}

void US_Hydrodyn_Pdb_Tool_Merge::csv_save()
{
}

