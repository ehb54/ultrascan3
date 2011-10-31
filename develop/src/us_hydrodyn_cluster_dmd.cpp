#include "../include/us_hydrodyn_cluster.h"
#include "../include/us_hydrodyn_cluster_dmd.h"

#define SLASH QDir::separator()

US_Hydrodyn_Cluster_Dmd::US_Hydrodyn_Cluster_Dmd(
                                               csv &csv1,
                                               void *us_hydrodyn, 
                                               QWidget *p, 
                                               const char *name
                                               ) : QDialog(p, name)
{
   this->csv1 = csv1;
   this->original_csv1 = &csv1;
   cluster_window = (void *)p;
   disable_updates = false;

   if ( !csv1.data.size() )
   {
      reset_csv();
   } 

   this->us_hydrodyn = us_hydrodyn;
   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("US-SOMO: Cluster DMD Setup"));

   setupGUI();

   sync_csv_with_selected();

   editor_msg("blue", "THIS WINDOW IS UNDER DEVELOPMENT." );

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
   if ( csv_height > 800 )
   {
      csv_height = 800;
   }
   if ( csv_width > 1000 )
   {
      csv_width = 1000;
   }

   // cout << QString("csv size %1 %2\n").arg(csv_height).arg(csv_width);

   setGeometry(global_Xpos, global_Ypos, csv_width, 400 + csv_height );
   update_enables();
}

US_Hydrodyn_Cluster_Dmd::~US_Hydrodyn_Cluster_Dmd()
{
}

void US_Hydrodyn_Cluster_Dmd::setupGUI()
{
   int minHeight1 = 30;
   int minHeight3 = 30;

   lbl_title = new QLabel(csv1.name.left(80), this);
   lbl_title->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   t_csv = new QTable(csv1.data.size(), csv1.header.size(), this);
   t_csv->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   // t_csv->setMinimumHeight(minHeight1 * 3);
   // t_csv->setMinimumWidth(minWidth1);
   t_csv->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   t_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));
   t_csv->setEnabled(true);
   t_csv->setSelectionMode( QTable::SingleRow );

   reload_csv();

   QValueList < unsigned int > column_widths;
   column_widths 
      << 120
      << 60
      << 95
      << 105
      << 102
      << 108
      << 88
      << 100
      << 108
      << 100
      << 200;
      
   for ( unsigned int i = 0; i < csv1.header.size(); i++ )
   {
      t_csv->horizontalHeader()->setLabel(i, csv1.header[i]);
      t_csv->setColumnWidth( i, column_widths[ i ] );
   }

   t_csv->setSorting(false);
   t_csv->setRowMovingEnabled(true);
   t_csv->setColumnMovingEnabled(false);
   t_csv->setReadOnly(false);

   t_csv->setColumnReadOnly( 0, true );

   // probably I'm not understanding something, but these next two lines don't seem to do anything
   t_csv->horizontalHeader()->adjustHeaderSize();
   t_csv->adjustSize();

   recompute_interval_from_points( 3 );
   recompute_interval_from_points( 7 );

   connect( t_csv, SIGNAL( valueChanged(int, int) ), SLOT( table_value( int, int ) ) );
   connect( t_csv, SIGNAL( selectionChanged() ), SLOT( update_enables() ) );
   connect( t_csv->verticalHeader(), SIGNAL( released( int ) ), SLOT( row_header_released( int ) ) );

   //   pb_select_all = new QPushButton(tr("Select all"), this);
   //   pb_select_all->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   //   pb_select_all->setMinimumHeight(minHeight1);
   //   pb_select_all->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   //   connect(pb_select_all, SIGNAL(clicked()), SLOT(select_all()));

   pb_copy = new QPushButton(tr("Copy values"), this);
   pb_copy->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_copy->setMinimumHeight(minHeight1);
   pb_copy->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_copy, SIGNAL(clicked()), SLOT(copy()));

   pb_paste = new QPushButton(tr("Paste values to selected"), this);
   pb_paste->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_paste->setMinimumHeight(minHeight1);
   pb_paste->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_paste, SIGNAL(clicked()), SLOT(paste()));

   pb_paste_all = new QPushButton(tr("Paste values to all"), this);
   pb_paste_all->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_paste_all->setMinimumHeight(minHeight1);
   pb_paste_all->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_paste_all, SIGNAL(clicked()), SLOT(paste_all()));

   pb_dup = new QPushButton(tr("Duplicate row"), this);
   pb_dup->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_dup->setMinimumHeight(minHeight1);
   pb_dup->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_dup, SIGNAL(clicked()), SLOT(dup()));

   pb_delete = new QPushButton(tr("Delete row"), this);
   pb_delete->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_delete->setMinimumHeight(minHeight1);
   pb_delete->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_delete, SIGNAL(clicked()), SLOT(delete_rows()));

   pb_load = new QPushButton(tr("Load"), this);
   pb_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_load->setMinimumHeight(minHeight1);
   pb_load->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_load, SIGNAL(clicked()), SLOT(load()));

   pb_reset = new QPushButton(tr("Reset"), this);
   pb_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_reset->setMinimumHeight(minHeight1);
   pb_reset->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect( pb_reset, SIGNAL( clicked() ), SLOT( reset() ) );

   pb_save_csv = new QPushButton(tr("Save"), this);
   pb_save_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_save_csv->setMinimumHeight(minHeight1);
   pb_save_csv->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_save_csv, SIGNAL(clicked()), SLOT(save_csv()));

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
   editor->setMinimumHeight( 50 );
   
   pb_cancel = new QPushButton(tr("Cancel"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_ok = new QPushButton( tr("Close"), this);
   pb_ok->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_ok->setMinimumHeight(minHeight1);
   pb_ok->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_ok, SIGNAL(clicked()), SLOT(ok()));

   // build layout

   QHBoxLayout *hbl_ctls = new QHBoxLayout(0);
   hbl_ctls->addSpacing( 4 );
   //   hbl_ctls->addWidget ( pb_select_all );
   //   hbl_ctls->addSpacing( 4 );
   hbl_ctls->addWidget ( pb_copy );
   hbl_ctls->addSpacing( 4 );
   hbl_ctls->addWidget ( pb_paste );
   hbl_ctls->addSpacing( 4 );
   hbl_ctls->addWidget ( pb_paste_all );
   hbl_ctls->addSpacing( 4 );
   hbl_ctls->addWidget ( pb_dup );
   hbl_ctls->addSpacing( 4 );
   hbl_ctls->addWidget ( pb_delete );
   hbl_ctls->addSpacing( 4 );

   QHBoxLayout *hbl_load_save = new QHBoxLayout(0);
   hbl_load_save->addSpacing( 4 );
   hbl_load_save->addWidget ( pb_load );
   hbl_load_save->addSpacing( 4 );
   hbl_load_save->addWidget ( pb_reset );
   hbl_load_save->addSpacing( 4 );
   hbl_load_save->addWidget ( pb_save_csv );
   hbl_load_save->addSpacing( 4 );

   QBoxLayout *vbl_editor_group = new QVBoxLayout(0);
   vbl_editor_group->addWidget( frame );
   vbl_editor_group->addWidget( editor );

   QHBoxLayout *hbl_bottom = new QHBoxLayout(0);
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_cancel );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_ok );
   hbl_bottom->addSpacing( 4 );


   QVBoxLayout *background = new QVBoxLayout(this);
   background->addSpacing( 4 );
   background->addWidget ( lbl_title );
   background->addSpacing( 4 );
   background->addWidget ( t_csv );
   background->addLayout ( hbl_ctls );
   background->addSpacing( 4 );
   background->addLayout ( hbl_load_save );
   background->addSpacing( 4 );
   background->addLayout ( vbl_editor_group );
   background->addSpacing( 4 );
   background->addLayout ( hbl_bottom );
   background->addSpacing( 4 );
}

void US_Hydrodyn_Cluster_Dmd::ok()
{
   *original_csv1 = current_csv();
   close();
}

void US_Hydrodyn_Cluster_Dmd::cancel()
{
   close();
}

void US_Hydrodyn_Cluster_Dmd::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_cluster_dmd.html");
}

void US_Hydrodyn_Cluster_Dmd::closeEvent(QCloseEvent *e)
{
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Cluster_Dmd::table_value( int /* row */, int col )
{
   if ( col == 3 || col == 4 )
   {
      recompute_interval_from_points( 3 );
   }
   if ( col == 5 )
   {
      recompute_points_from_interval( 3 );
   }

   if ( col == 7 || col == 8 )
   {
      recompute_interval_from_points( 7 );
   }
   if ( col == 9 )
   {
      recompute_points_from_interval( 7 );
   }

}

void US_Hydrodyn_Cluster_Dmd::clear_display()
{
   editor->clear();
   editor->append("\n\n");
}

void US_Hydrodyn_Cluster_Dmd::update_font()
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

void US_Hydrodyn_Cluster_Dmd::save()
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

csv US_Hydrodyn_Cluster_Dmd::current_csv()
{
   csv tmp_csv = csv1;
   
   for ( unsigned int i = 0; i < csv1.data.size(); i++ )
   {
      for ( unsigned int j = 0; j < csv1.data[i].size(); j++ )
      {
         if ( csv1.data[i][j] == "Y" || csv1.data[i][j] == "N" )
         {
            tmp_csv.data[i][j] = ((QCheckTableItem *)(t_csv->item( i, j )))->isChecked() ? "Y" : "N";
         } else {
            tmp_csv.data[i][j] = t_csv->text( i, j );
         }
         tmp_csv.num_data[i][j] = tmp_csv.data[i][j].toDouble();
      }
   }
   return tmp_csv;
}
  
void US_Hydrodyn_Cluster_Dmd::recompute_interval_from_points( unsigned int basecol )
{
   for ( unsigned int i = 0; i < (unsigned int)t_csv->numRows(); i++ )
   {
      t_csv->setText(
                     i, basecol + 2, 
                     ( 
                      t_csv->text(i, basecol + 1 ).toDouble() == 0e0 ?
                      ""
                      :
                      QString("%1")
                      .arg( t_csv->text(i, basecol ).toDouble() 
                             / ( t_csv->text( i, basecol + 1 ).toDouble() ) ) 
                      )
                     );
   }
}

void US_Hydrodyn_Cluster_Dmd::recompute_points_from_interval( unsigned int basecol )
{
   for ( unsigned int i = 0; i < (unsigned int)t_csv->numRows(); i++ )
   {
      t_csv->setText(
                     i, basecol + 1, 
                     ( 
                      t_csv->text( i, basecol + 2 ).toDouble() == 0e0 ?
                      ""
                      :
                      QString("%1")
                      .arg( (unsigned int)( t_csv->text( i, basecol ).toDouble()
                                            / t_csv->text( i, basecol + 2 ).toDouble() ) ) ) 
                     );
   }
}

void US_Hydrodyn_Cluster_Dmd::editor_msg( QString color, QString msg )
{
   QColor save_color = editor->color();
   editor->setColor(color);
   editor->append(msg);
   editor->setColor(save_color);
   editor->scrollToBottom();
}

void US_Hydrodyn_Cluster_Dmd::reset()
{
   reset_csv();
   reload_csv();
   update_enables();
}
   
void US_Hydrodyn_Cluster_Dmd::reset_csv()
{
   csv1.name = "Cluster DMD setup";

   csv1.header.clear();
   csv1.header_map.clear();
   csv1.data.clear();
   csv1.num_data.clear();
   csv1.prepended_names.clear();

   csv1.header.push_back("PDB file");                         // 0
   csv1.header.push_back("Active");                           // 1
   csv1.header.push_back("Relax temp\nkcal/mol/kB");          // 2
   csv1.header.push_back("Relax time\n* 50fs");               // 3
   csv1.header.push_back("Relax PDB\noutput\ntimestep");      // 4
   csv1.header.push_back("Relax PDB\noutput\ncount");         // 5
   csv1.header.push_back("Run temp\nkcal/mol/kB");            // 6
   csv1.header.push_back("Run time\n* 50fs)");                // 7
   csv1.header.push_back("Run PDB\noutput\ntimestep");        // 8
   csv1.header.push_back("Run PDB\noutput\ncount");           // 9
   csv1.header.push_back("Static range");                     // 10

   for ( unsigned int i = 0; i < ((US_Hydrodyn_Cluster *)cluster_window)->selected_files.size(); i++ )
   {
      vector < QString > tmp_data;

      // FIX THIS: read to see if fileName.dmd_info exists (?)

      tmp_data.push_back( QFileInfo( ((US_Hydrodyn_Cluster *)cluster_window)->selected_files[ i ] ).fileName() );
      tmp_data.push_back("Y");
      tmp_data.push_back("");
      tmp_data.push_back("");
      tmp_data.push_back("");
      tmp_data.push_back("");
      tmp_data.push_back("");
      tmp_data.push_back("");
      tmp_data.push_back("");
      tmp_data.push_back("");
      tmp_data.push_back("");

      csv1.prepended_names.push_back(tmp_data[0]);
      csv1.data.push_back(tmp_data);
   }

   for ( unsigned int i = 0; i < csv1.data.size(); i++ )
   {
      vector < double > tmp_num_data;
      for ( unsigned int j = 0; j < csv1.data[i].size(); j++ )
      {
         tmp_num_data.push_back(csv1.data[i][j].toDouble());
      }
      csv1.num_data.push_back(tmp_num_data);
   }
}

void US_Hydrodyn_Cluster_Dmd::copy()
{
   csv1 = current_csv();
   csv_copy = current_csv();
   csv_copy.data.clear();
   csv_copy.num_data.clear();
   csv_copy.prepended_names.clear();

   for ( int i = 0; i < t_csv->numRows(); i++ )
   {
      if ( t_csv->isRowSelected( i ) )
      {
         // editor_msg( "black", QString( "copying row %1" ).arg( i ) );
         csv_copy.data           .push_back( csv1.data           [ i ] );
         csv_copy.num_data       .push_back( csv1.num_data       [ i ] );
         csv_copy.prepended_names.push_back( csv1.prepended_names[ i ] );
      }
   }
   // editor_msg( "black", QString( "csv copy has %1 rows" ).arg( csv_copy.data.size() ) );

   // cout << "csv1 after copy():\n" << csv_to_qstring( csv1 ) << endl;
   // cout << "csv_copy after copy():\n" << csv_to_qstring( csv_copy ) << endl;

   update_enables();
}

void US_Hydrodyn_Cluster_Dmd::paste()
{
   csv1 = current_csv();
   unsigned int pos = 0;
   for ( int i = 0; i < t_csv->numRows(); i++ )
   {
      // editor_msg( "black", QString( "checking row %1" ).arg( i ) );
      if ( t_csv->isRowSelected( i ) )
      {
         // editor_msg( "black", QString( "pasting into row %1" ).arg( i ) );
         for ( unsigned int j = 1; j < csv_copy.data[ pos % csv_copy.data.size() ].size(); j++ )
         {
            // editor_msg( "black", QString( "setting data to row %1 from pos %2 %3 val %4 j %5" )
            // .arg( i )
            // .arg( pos )
            // .arg( pos % csv_copy.data.size() ) 
            // .arg( csv_copy.data    [ pos % csv_copy.data.size() ][ j ] )
            // .arg( j )
            // );
            csv1.data    [ i ][ j ] = csv_copy.data    [ pos % csv_copy.data.size() ][ j ];
            csv1.num_data[ i ][ j ] = csv_copy.num_data[ pos % csv_copy.data.size() ][ j ];
         }
         pos++;
      }
   }
   // cout << "csv1 after paste():\n" << csv_to_qstring( csv1 ) << endl;
   // cout << "csv_copy after paste():\n" << csv_to_qstring( csv_copy ) << endl;
   reload_csv();
   update_enables();
}

void US_Hydrodyn_Cluster_Dmd::paste_all()
{
   csv1 = current_csv();
   unsigned int pos = 0;
   for ( int i = 0; i < t_csv->numRows(); i++ )
   {
      for ( unsigned int j = 1; j < csv_copy.data[ pos % csv_copy.data.size() ].size(); j++ )
      {
         csv1.data    [ i ][ j ] = csv_copy.data    [ pos % csv_copy.data.size() ][ j ];
         csv1.num_data[ i ][ j ] = csv_copy.num_data[ pos % csv_copy.data.size() ][ j ];
      }
      pos++;
   }
   reload_csv();
   update_enables();
}

void US_Hydrodyn_Cluster_Dmd::dup()
{
   csv1 = current_csv();
   for ( int i = 0; i < t_csv->numRows(); i++ )
   {
      if ( t_csv->isRowSelected( i ) )
      {
         csv1.data           .push_back( csv1.data           [ i ] );
         csv1.num_data       .push_back( csv1.num_data       [ i ] );
         csv1.prepended_names.push_back( csv1.prepended_names[ i ] );
      }
   }
   reload_csv();
   update_enables();
}

void US_Hydrodyn_Cluster_Dmd::load()
{
}

void US_Hydrodyn_Cluster_Dmd::save_csv()
{
}

void US_Hydrodyn_Cluster_Dmd::update_enables()
{
   if ( !disable_updates )
   {
      // editor_msg( "black", "-------------" );
      disable_updates = true;
      unsigned int selected = 0;
      vector < int > selected_rows;
      for ( int i = 0; i < t_csv->numRows(); i++ )
      {
         if ( t_csv->isRowSelected( i ) )
         {
            selected++;
            selected_rows.push_back( i );
            // editor_msg( "black", QString( "row selected %1" ).arg( i ) );
         }
      }

      // t_csv->clearSelection();
      // for ( unsigned int i = 0; i < selected_rows.size(); i++ )
      // {
      // t_csv->selectRow( selected_rows[ i ] );
      // }

      pb_copy     ->setEnabled( selected == 1 );
      pb_dup      ->setEnabled( selected );
      pb_paste    ->setEnabled( selected && csv_copy.data.size() );
      pb_paste_all->setEnabled( csv_copy.data.size() );
      pb_delete   ->setEnabled( selected );
      pb_save_csv ->setEnabled( csv1.data.size() );
      disable_updates = false;
   }
}

void US_Hydrodyn_Cluster_Dmd::reload_csv()
{
   interval_starting_row = 0;
   t_csv->setNumRows( csv1.data.size() );
   for ( unsigned int i = 0; i < csv1.data.size(); i++ )
   {
      if ( csv1.data[ i ].size() < 3 || csv1.data[ i ][ 2 ].isEmpty() )
      {
         interval_starting_row = i + 1;
      }
      for ( unsigned int j = 0; j < csv1.data[i].size(); j++ )
      {
         if ( csv1.data[i][j] == "Y" || csv1.data[i][j] == "N" )
         {
            t_csv->setItem( i, j, new QCheckTableItem( t_csv, "" ) );
            ((QCheckTableItem *)(t_csv->item( i, j )))->setChecked( csv1.data[i][j] == "Y" );
         } else {
            t_csv->setText( i, j, csv1.data[i][j] );
         }
      }
   }
   // t_csv->clearSelection();
}

void US_Hydrodyn_Cluster_Dmd::delete_rows()
{
   csv csv_new = current_csv();
   csv_new.data.clear();
   csv_new.num_data.clear();
   csv_new.prepended_names.clear();
   
   for ( int i = 0; i < t_csv->numRows(); i++ )
   {
      if ( !t_csv->isRowSelected( i ) )
      {
         csv_new.data           .push_back( csv1.data           [ i ] );
         csv_new.num_data       .push_back( csv1.num_data       [ i ] );
         csv_new.prepended_names.push_back( csv1.prepended_names[ i ] );
      }
   }
   csv1 = csv_new;
   reload_csv();
   t_csv->clearSelection();
   update_enables();
}

void US_Hydrodyn_Cluster_Dmd::select_all()
{
   disable_updates = true;
   int selected = 0;
   vector < int > selected_rows;

   for ( int i = 0; i < t_csv->numRows(); i++ )
   {
      if ( t_csv->isRowSelected( i ) )
      {
         selected++;
         selected_rows.push_back( i );
      }
   }
   
   if ( selected != t_csv->numRows() )
   {
      // select all
      for ( int i = 0; i < t_csv->numRows(); i++ )
      {
         t_csv->selectRow( selected_rows[ i ] );
      }
   } else {
      t_csv->clearSelection();
   }
   disable_updates = false;
   update_enables();
}

QString US_Hydrodyn_Cluster_Dmd::csv_to_qstring( csv &from_csv )
{
   QString qs;

   for ( unsigned int i = 0; i < from_csv.header.size(); i++ )
   {
      qs += QString("%1\"%2\"").arg(i ? "," : "").arg(from_csv.header[i]);
   }
   qs += "\n";
   for ( unsigned int i = 0; i < from_csv.data.size(); i++ )
   {
      for ( unsigned int j = 0; j < from_csv.data[i].size(); j++ )
      {
         qs += QString("%1%2").arg(j ? "," : "").arg(from_csv.data[i][j]);
      }
      qs += "\n";
   }

   return qs;
}

void US_Hydrodyn_Cluster_Dmd::row_header_released( int row )
{
   // cout << QString( "row_header_released %1\n" ).arg( row );
   t_csv->clearSelection();
   QTableSelection qts( row, 0, row, 12 );
   
   t_csv->addSelection( qts );
   update_enables();
}

void US_Hydrodyn_Cluster_Dmd::sync_csv_with_selected()
{
   csv1 = current_csv();
   csv csv_new = csv1;
   csv_new.data.clear();
   csv_new.num_data.clear();
   csv_new.prepended_names.clear();

   map < QString, bool > selected_map;
   map < QString, bool > present_map;
   
   for ( unsigned int i = 0; i < ((US_Hydrodyn_Cluster *)cluster_window)->selected_files.size(); i++ )
   {
      selected_map[ QFileInfo( ((US_Hydrodyn_Cluster *)cluster_window)->selected_files[ i ] ).fileName() ] = true;
   }

   // add existing ones
   for ( unsigned int i = 0; i < csv1.prepended_names.size(); i++ )
   {
      if ( selected_map.count( csv1.prepended_names[ i ] ) )
      {
         present_map[ csv1.prepended_names[ i ] ] = true;
         csv_new.data           .push_back( csv1.data           [ i ] );
         csv_new.num_data       .push_back( csv1.num_data       [ i ] );
         csv_new.prepended_names.push_back( csv1.prepended_names[ i ] );
      }
   }

   // now go thru selected and find not present and add as new
   for ( map < QString, bool >::iterator it = selected_map.begin();
         it != selected_map.end();
         it++ )
   {
      if ( !present_map.count( it->first ) )
      {
         vector < QString > tmp_data;
         
         // FIX THIS: read to see if fileName.dmd_info exists (?)
         
         tmp_data.push_back( it->first );
         tmp_data.push_back( "Y" );
         tmp_data.push_back( "" );
         tmp_data.push_back( "" );
         tmp_data.push_back( "" );
         tmp_data.push_back( "" );
         tmp_data.push_back( "" );
         tmp_data.push_back( "" );
         tmp_data.push_back( "" );
         tmp_data.push_back( "" );
         tmp_data.push_back( "" );
         
         csv_new.prepended_names.push_back( tmp_data[ 0 ] );
         csv_new.data.push_back( tmp_data );
         
         vector < double > tmp_num_data;
         for ( unsigned int i = 0; i < tmp_data.size(); i++ )
         {
            tmp_num_data.push_back( tmp_data[ i ].toDouble() );
         }
         csv_new.num_data.push_back( tmp_num_data );
      }
   }
   csv1 = csv_new;
   reload_csv();
}

