#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_cluster.h"
#include "../include/us_hydrodyn_cluster_dmd.h"
#include "../include/us_hydrodyn.h"
//Added by qt3to4:
#include <QTextStream>
#include <QHBoxLayout>
 //#include <Q3ValueList>
#include <QLabel>
#include <QFrame>
 //#include <Q3PopupMenu>
#include <QVBoxLayout>
#include <QBoxLayout>
#include <QCloseEvent>

#define SLASH QDir::separator()

#define TSO QTextStream( stdout )

US_Hydrodyn_Cluster_Dmd::US_Hydrodyn_Cluster_Dmd(
                                               csv &csv1,
                                               void *us_hydrodyn, 
                                               QWidget *p, 
                                               const char *
                                               ) : QDialog( p )
{
   this->csv1 = csv1;
   this->original_csv1 = &csv1;
   cluster_window = (void *)p;
   disable_updates = false;
   gui_setup       = false;


   this->us_hydrodyn = us_hydrodyn;
   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle(us_tr("US-SOMO: Cluster DMD Setup"));

   if ( !csv1.data.size() )
   {
      init_csv();
   } 

   setupGUI();

   // reset_csv();
   editor_msg("blue", "THIS WINDOW IS UNDER DEVELOPMENT." );

   sync_csv_with_selected();

   // check pdbs
   {
      ((US_Hydrodyn *)us_hydrodyn)->dmd_failed_validation = false;
      ((US_Hydrodyn *)us_hydrodyn)->dmd_all_pdb_prepare_reports.clear();
      US_Saxs_Util *us = ((US_Hydrodyn *)us_hydrodyn)->saxs_util;

      for ( int i = 0; i < (int)((US_Hydrodyn_Cluster *)cluster_window)->selected_files.size(); i++ ) {
         QString full_filename = ((US_Hydrodyn_Cluster *)cluster_window)->selected_files[ i ];
         QString filename      = QFileInfo( full_filename ).fileName();
         if ( !us->dmd_pdb_prepare( full_filename ) ) {
            ((US_Hydrodyn *)us_hydrodyn)->dmd_failed_validation = true;
            qDebug() << "dmd_pdb_prepare() : errors : " << us->errormsg << Qt::endl;
            ((US_Hydrodyn *)us_hydrodyn)->dmd_all_pdb_prepare_reports[ filename ] = us->dmd_pdb_prepare_reports;
            ((US_Hydrodyn *)us_hydrodyn)->dmd_all_pdb_prepare_reports[ filename ][ "errors" ] << us->errormsg;
            if ( !us->noticemsg.isEmpty() ) {
               ((US_Hydrodyn *)us_hydrodyn)->dmd_all_pdb_prepare_reports[ filename ][ "notice" ] << us->noticemsg;
            }
         } else {
            ((US_Hydrodyn *)us_hydrodyn)->dmd_all_pdb_prepare_reports[ filename ] = us->dmd_pdb_prepare_reports;
            dmd_chain          [ filename ] = us->dmd_chain;
            dmd_res_link       [ filename ] = us->dmd_res_link;
            dmd_chain_is_hetatm[ filename ] = us->dmd_chain_is_hetatm;
         }            
      }
      if ( ((US_Hydrodyn *)us_hydrodyn)->dmd_failed_validation ) {
         QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection);
         return;
      }
      for ( auto it = ((US_Hydrodyn *)us_hydrodyn)->dmd_all_pdb_prepare_reports.begin();
            it != ((US_Hydrodyn *)us_hydrodyn)->dmd_all_pdb_prepare_reports.end();
            ++it ) {
         editor_msg( "black", "" );
         editor_msg( "black", "Reports for " + it->first + ":" );
         for ( auto it2 = it->second.begin();
               it2 !=  it->second.end();
               ++it2 ) {
            if ( it2->second.size() ) {
               editor_msg( "black", it2->first + ":" );
               editor_msg( "black", it2->second.join("\n") );
            }
         }
         editor_msg( "black", "" );
      }            
   }

   for ( map < QString, bool >::iterator it = selected_map.begin();
         it != selected_map.end();
         it++ ) {
      if ( !setup_residues( it->first ) ) {
         continue;
      }
      residue_summary( it->first );
   }
   
   dmd_dir = ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "cluster" + SLASH + "dmd";
   QDir dir1( dmd_dir );
   if ( !dir1.exists() )
   {
      editor_msg( "black", QString( us_tr( "Created directory %1" ) ).arg( dmd_dir ) );
      dir1.mkdir( dmd_dir );
   }

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
   if ( csv_height > 800 )
   {
      csv_height = 800;
   }
   if ( csv_width > 1000 )
   {
      csv_width = 1000;
   }

   // TSO << QString("csv size %1 %2\n").arg(csv_height).arg(csv_width);

   setGeometry(global_Xpos, global_Ypos, csv_width, 400 + csv_height );
   update_enables();
}

US_Hydrodyn_Cluster_Dmd::~US_Hydrodyn_Cluster_Dmd()
{
}

void US_Hydrodyn_Cluster_Dmd::setupGUI()
{
   int minHeight1 = 30;
#if QT_VERSION < 0x040000 || !defined(Q_OS_MAC)
   int minHeight3 = 30;
#endif

   lbl_title = new QLabel(csv1.name.left(80), this);
   lbl_title->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette( PALET_FRAME );
   AUTFBACK( lbl_title );
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   t_csv = new QTableWidget(csv1.data.size(), csv1.header.size(), this);
   t_csv->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   // t_csv->setMinimumHeight(minHeight1 * 3);
   // t_csv->setMinimumWidth(minWidth1);
   t_csv->setPalette( PALET_EDIT );
   AUTFBACK( t_csv );
   t_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));
   t_csv->setEnabled(true);
   t_csv->setSelectionMode( QAbstractItemView::SingleSelection );t_csv->setSelectionBehavior( QAbstractItemView::SelectRows );

   reload_csv();

#if QT_VERSION < 0x040000      
 //   Q3ValueList < unsigned int > column_widths;
#else
   QList < unsigned int > column_widths;
#endif

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
      << 200
      << 200;
      
   for ( unsigned int i = 0; i < (unsigned int)csv1.header.size(); i++ )
   {
      t_csv->setHorizontalHeaderItem(i, new QTableWidgetItem( csv1.header[i]));
      t_csv->setColumnWidth( i, column_widths[ i ] );
   }

   t_csv->setSortingEnabled(false);
    t_csv->verticalHeader()->setSectionsMovable(true);
    t_csv->horizontalHeader()->setSectionsMovable(false);
   //  t_csv->setReadOnly(false);

   { for ( int i = 0; i < t_csv->rowCount(); ++i ) { t_csv->item( i,  0 )->setFlags( t_csv->item( i,  0 )->flags() ^ Qt::ItemIsEditable ); } };
   { for ( int i = 0; i < t_csv->rowCount(); ++i ) { t_csv->item( i,  11 )->setFlags( t_csv->item( i,  11 )->flags() ^ Qt::ItemIsEditable ); } };

   // probably I'm not understanding something, but these next two lines don't seem to do anything
   // t_csv->horizontalHeader()->adjustHeaderSize();
   t_csv->adjustSize();

   recompute_interval_from_points( 3 );
   recompute_interval_from_points( 7 );

   connect( t_csv, SIGNAL( cellChanged(int, int) ), SLOT( table_value( int, int ) ) );
   connect( t_csv, SIGNAL( itemSelectionChanged() ), SLOT( update_enables() ) );
   connect( t_csv->verticalHeader(), SIGNAL( sectionPressed( int ) ), SLOT( row_header_released( int ) ) );

   //   pb_select_all = new QPushButton(us_tr("Select all"), this);
   //   pb_select_all->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   //   pb_select_all->setMinimumHeight(minHeight1);
   //   pb_select_all->setPalette( PALET_PUSHB );
   //   connect(pb_select_all, SIGNAL(clicked()), SLOT(select_all()));

   pb_copy = new QPushButton(us_tr("Copy values"), this);
   pb_copy->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_copy->setMinimumHeight(minHeight1);
   pb_copy->setPalette( PALET_PUSHB );
   connect(pb_copy, SIGNAL(clicked()), SLOT(copy()));

   pb_paste = new QPushButton(us_tr("Paste values to selected"), this);
   pb_paste->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_paste->setMinimumHeight(minHeight1);
   pb_paste->setPalette( PALET_PUSHB );
   connect(pb_paste, SIGNAL(clicked()), SLOT(paste()));

   pb_paste_all = new QPushButton(us_tr("Paste values to all"), this);
   pb_paste_all->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_paste_all->setMinimumHeight(minHeight1);
   pb_paste_all->setPalette( PALET_PUSHB );
   connect(pb_paste_all, SIGNAL(clicked()), SLOT(paste_all()));

   pb_dup = new QPushButton(us_tr("Duplicate row"), this);
   pb_dup->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_dup->setMinimumHeight(minHeight1);
   pb_dup->setPalette( PALET_PUSHB );
   connect(pb_dup, SIGNAL(clicked()), SLOT(dup()));

   pb_delete = new QPushButton(us_tr("Delete row"), this);
   pb_delete->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_delete->setMinimumHeight(minHeight1);
   pb_delete->setPalette( PALET_PUSHB );
   connect(pb_delete, SIGNAL(clicked()), SLOT(delete_rows()));

   pb_load = new QPushButton(us_tr("Load"), this);
   pb_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_load->setMinimumHeight(minHeight1);
   pb_load->setPalette( PALET_PUSHB );
   connect(pb_load, SIGNAL(clicked()), SLOT(load()));

   pb_reset = new QPushButton(us_tr("Reset"), this);
   pb_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_reset->setMinimumHeight(minHeight1);
   pb_reset->setPalette( PALET_PUSHB );
   connect( pb_reset, SIGNAL( clicked() ), SLOT( reset() ) );

   pb_save_csv = new QPushButton(us_tr("Save"), this);
   pb_save_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_save_csv->setMinimumHeight(minHeight1);
   pb_save_csv->setPalette( PALET_PUSHB );
   connect(pb_save_csv, SIGNAL(clicked()), SLOT(save_csv()));

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
   editor->setMinimumHeight( 50 );
   
   pb_cancel = new QPushButton(us_tr("Cancel"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_help = new QPushButton(us_tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_ok = new QPushButton( us_tr("Close"), this);
   pb_ok->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_ok->setMinimumHeight(minHeight1);
   pb_ok->setPalette( PALET_PUSHB );
   connect(pb_ok, SIGNAL(clicked()), SLOT(ok()));

   // build layout

   QHBoxLayout * hbl_ctls = new QHBoxLayout(); hbl_ctls->setContentsMargins( 0, 0, 0, 0 ); hbl_ctls->setSpacing( 0 );
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

   QHBoxLayout * hbl_load_save = new QHBoxLayout(); hbl_load_save->setContentsMargins( 0, 0, 0, 0 ); hbl_load_save->setSpacing( 0 );
   hbl_load_save->addSpacing( 4 );
   hbl_load_save->addWidget ( pb_load );
   hbl_load_save->addSpacing( 4 );
   hbl_load_save->addWidget ( pb_reset );
   hbl_load_save->addSpacing( 4 );
   hbl_load_save->addWidget ( pb_save_csv );
   hbl_load_save->addSpacing( 4 );

   QBoxLayout * vbl_editor_group = new QVBoxLayout(0); vbl_editor_group->setContentsMargins( 0, 0, 0, 0 ); vbl_editor_group->setSpacing( 0 );
#if QT_VERSION < 0x040000 || !defined(Q_OS_MAC)
   vbl_editor_group->addWidget( frame );
#endif
   vbl_editor_group->addWidget( editor );

   QHBoxLayout * hbl_bottom = new QHBoxLayout(); hbl_bottom->setContentsMargins( 0, 0, 0, 0 ); hbl_bottom->setSpacing( 0 );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_cancel );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_ok );
   hbl_bottom->addSpacing( 4 );


   QVBoxLayout * background = new QVBoxLayout(this); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 );
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
   online_help->show_help("manual/somo/somo_cluster_dmd.html");
}

void US_Hydrodyn_Cluster_Dmd::closeEvent(QCloseEvent *e)
{
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Cluster_Dmd::table_value( int row, int col )
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
   if ( col == 10 )
   {
      convert_static_range( row );
   }
}

void US_Hydrodyn_Cluster_Dmd::clear_display()
{
   editor->clear( );
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

csv US_Hydrodyn_Cluster_Dmd::current_csv()
{
   csv tmp_csv = csv1;
   
   for ( unsigned int i = 0; i < (unsigned int)csv1.data.size(); i++ )
   {
      for ( unsigned int j = 0; j < (unsigned int)csv1.data[i].size(); j++ )
      {
         if ( csv1.data[i][j] == "Y" || csv1.data[i][j] == "N" )
         {
            tmp_csv.data[i][j] = ((QCheckBox *)(t_csv->cellWidget( i, j )))->isChecked() ? "Y" : "N";
         } else {
            tmp_csv.data[i][j] = t_csv->item( i, j )->text();
         }
         tmp_csv.num_data[i][j] = tmp_csv.data[i][j].toDouble();
      }
   }
   return tmp_csv;
}
  
void US_Hydrodyn_Cluster_Dmd::recompute_interval_from_points( unsigned int basecol )
{
   if ( (unsigned int) t_csv->columnCount() <=  basecol + 1 ) {
      qDebug() << "US_Hydrodyn_Cluster_Dmd::recompute_interval_from_points() insufficient columns";
      return;
   }

   for ( unsigned int i = 0; i < (unsigned int)t_csv->rowCount(); i++ )
   {
      QString toset =
                      t_csv->item(i, basecol + 1 )->text().toDouble() == 0e0 ?
                      ""
                      :
                      QString("%1")
                      .arg( t_csv->item(i, basecol )->text().toDouble() 
                             / ( t_csv->item( i, basecol + 1 )->text().toDouble() ) )
         ;
      disconnect( t_csv, SIGNAL( cellChanged(int, int) ), 0, 0 );
      t_csv->setItem( i, basecol + 2, new QTableWidgetItem( toset ) );
      connect( t_csv, SIGNAL( cellChanged(int, int) ), SLOT( table_value( int, int ) ) );
   }
}

void US_Hydrodyn_Cluster_Dmd::recompute_points_from_interval( unsigned int basecol )
{
   if ( (unsigned int) t_csv->columnCount() <= basecol + 2 ) {
      qDebug() << "US_Hydrodyn_Cluster_Dmd::recompute_points_from_interval() insufficient columns";
      return;
   }
   for ( unsigned int i = 0; i < (unsigned int)t_csv->rowCount(); i++ )
   {
      QString toset =
                      t_csv->item( i, basecol + 2 )->text().toDouble() == 0e0 ?
                      ""
                      :
                      QString("%1")
                      .arg( (unsigned int)( t_csv->item( i, basecol )->text().toDouble()
                                            / t_csv->item( i, basecol + 2 )->text().toDouble() ) )
         ;
      disconnect( t_csv, SIGNAL( cellChanged(int, int) ), 0, 0 );
      t_csv->setItem( i, basecol + 1, new QTableWidgetItem( toset ) );
      connect( t_csv, SIGNAL( cellChanged(int, int) ), SLOT( table_value( int, int ) ) );
   }
}

void US_Hydrodyn_Cluster_Dmd::editor_msg( QString color, QString msg )
{
   QColor save_color = editor->textColor();
   editor->setTextColor(color);
   editor->append(msg);
   editor->setTextColor(save_color);
   editor->verticalScrollBar()->setValue(editor->verticalScrollBar()->maximum());
}

void US_Hydrodyn_Cluster_Dmd::reset()
{
   reset_csv();
   reload_csv();
   update_enables();
}
   
void US_Hydrodyn_Cluster_Dmd::init_csv()
{
   csv1.name = "Cluster DMD setup";

   csv1.header.clear( );
   csv1.header_map.clear( );
   csv1.data.clear( );
   csv1.num_data.clear( );
   csv1.prepended_names.clear( );

   csv1.header.push_back("PDB file");                         // 0
   csv1.header.push_back("Active");                           // 1
   csv1.header.push_back("Relax temp\nkcal/mol/kB");          // 2
   csv1.header.push_back("Relax time\n* 50fs");               // 3
   csv1.header.push_back("Relax PDB\noutput\ntimestep");      // 4
   csv1.header.push_back("Relax PDB\noutput\ncount");         // 5
   csv1.header.push_back("Run temp\nkcal/mol/kB");            // 6
   csv1.header.push_back("Run time\n* 50fs");                 // 7
   csv1.header.push_back("Run PDB\noutput\ntimestep");        // 8
   csv1.header.push_back("Run PDB\noutput\ncount");           // 9
   csv1.header.push_back("Static range");                     // 10
   csv1.header.push_back("Static range (native)");            // 11
}

void US_Hydrodyn_Cluster_Dmd::reset_csv()
{
   init_csv();

   full_filenames.clear( );
   residues_chain          .clear( );
   residues_chain_map      .clear( );
   residues_number         .clear( );
   residues_range_start    .clear( );
   residues_range_end      .clear( );
   residues_range_chain    .clear( );
   residues_range_chain_pos.clear( );

   for ( unsigned int i = 0; i < (unsigned int)((US_Hydrodyn_Cluster *)cluster_window)->selected_files.size(); i++ )
   {
      vector < QString > tmp_data;

      // FIX THIS: read to see if fileName.dmd_info exists (?)

      QString full_filename = ((US_Hydrodyn_Cluster *)cluster_window)->selected_files[ i ];
      QString filename      = QFileInfo( full_filename ).fileName();

      if ( !full_filenames.count( filename ) )
      {
         full_filenames[ filename ] = full_filename;
      } else {
         if ( full_filenames[ filename  ] != full_filename )
         {
             editor_msg( "dark red", QString( us_tr( "Warning: The same file name has been referenced in multiple file locations.\n"
                                                  "%1 vs %2\n"
                                                  "This will cause problems.  Duplicate reference skipped.\n" ) )
                         .arg( full_filenames[ filename ] )
                         .arg( full_filename )
                         );
             continue;
         }
      }                                             

      if ( !setup_residues( filename ) )
      {
         continue;
      }
      residue_summary( filename );
      
      tmp_data.push_back( filename );
      tmp_data.push_back("Y");
      tmp_data.push_back(".7");
      tmp_data.push_back("100");
      tmp_data.push_back("50");
      tmp_data.push_back("2");
      tmp_data.push_back(".5");
      tmp_data.push_back("10000");
      tmp_data.push_back("200");
      tmp_data.push_back("50");
      tmp_data.push_back("");
      tmp_data.push_back("");

      csv1.prepended_names.push_back(tmp_data[0]);
      csv1.data.push_back(tmp_data);
   }

   for ( unsigned int i = 0; i < (unsigned int)csv1.data.size(); i++ )
   {
      vector < double > tmp_num_data;
      for ( unsigned int j = 0; j < (unsigned int)csv1.data[i].size(); j++ )
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
   csv_copy.data.clear( );
   csv_copy.num_data.clear( );
   csv_copy.prepended_names.clear( );

   for ( int i = 0; i < t_csv->rowCount(); i++ )
   {
      if ( t_csv->item( i , 0 )->isSelected() )
      {
         // editor_msg( "black", QString( "copying row %1" ).arg( i ) );
         csv_copy.data           .push_back( csv1.data           [ i ] );
         csv_copy.num_data       .push_back( csv1.num_data       [ i ] );
         csv_copy.prepended_names.push_back( csv1.prepended_names[ i ] );
      }
   }
   // editor_msg( "black", QString( "csv copy has %1 rows" ).arg( csv_copy.data.size() ) );

   // TSO << "csv1 after copy():\n" << csv_to_qstring( csv1 ) << Qt::endl;
   // TSO << "csv_copy after copy():\n" << csv_to_qstring( csv_copy ) << Qt::endl;

   update_enables();
}

void US_Hydrodyn_Cluster_Dmd::paste()
{
   csv1 = current_csv();
   unsigned int pos = 0;
   for ( int i = 0; i < t_csv->rowCount(); i++ )
   {
      // editor_msg( "black", QString( "checking row %1" ).arg( i ) );
      if ( t_csv->item( i , 0 )->isSelected() )
      {
         // editor_msg( "black", QString( "pasting into row %1" ).arg( i ) );
         for ( unsigned int j = 1; j < (unsigned int)csv_copy.data[ pos % csv_copy.data.size() ].size(); j++ )
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
   // TSO << "csv1 after paste():\n" << csv_to_qstring( csv1 ) << Qt::endl;
   // TSO << "csv_copy after paste():\n" << csv_to_qstring( csv_copy ) << Qt::endl;
   reload_csv();
   update_enables();
}

void US_Hydrodyn_Cluster_Dmd::paste_all()
{
   csv1 = current_csv();
   unsigned int pos = 0;
   for ( int i = 0; i < t_csv->rowCount(); i++ )
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
   for ( int i = 0; i < t_csv->rowCount(); i++ )
   {
      if ( t_csv->item( i , 0 )->isSelected() )
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
   QString use_dir = dmd_dir;

   QString fname = QFileDialog::getOpenFileName( this , windowTitle() , use_dir , "DMD parameter files (*.dmd *.dmd)" );

   if ( fname.isEmpty() )
   {
      return;
   }

   if ( !QFile::exists( fname ) )
   {
      QMessageBox::warning( this, 
                            us_tr( "US-SOMO : Cluster DMD Setup : Load" ),
                            QString( us_tr( "File %1 does not exist" ) ).arg( fname ) );
      return;
   }

   QFile f( fname );

   if ( !f.open( QIODevice::ReadOnly ) )
   {
      QMessageBox::warning( this, 
                            us_tr( "US-SOMO : Cluster DMD Setup : Load" ),
                            QString( us_tr( "Can not open file %1 for reading" ) ).arg( fname ) );
      return;
   }

   csv1 = current_csv();
   
   map < QString, list < unsigned int > > our_files;
   QStringList                            qsl_our_files;
   for ( unsigned int i = 0; i < (unsigned int)csv1.data.size(); i++ )
   {
      if ( !our_files.count( csv1.data[ i ][ 0 ] ) )
      {
         qsl_our_files << csv1.data[ i ][ 0 ];
      }
      our_files[ csv1.data[ i ][ 0 ] ].push_back( i );
   }
      
   QTextStream ts( &f );

   QStringList qsl_lines;

   ts.readLine(); // skip header
   while ( !ts.atEnd() )
   {
      QString qs = ts.readLine();
      qsl_lines << qs;
   }
   f.close();
      
   map < QString, bool > names_not_present;

   for ( unsigned int i = 0; i < (unsigned int)qsl_lines.size(); i++ ) 
   {
      QString     qs  = qsl_lines[ i ];
      QStringList qsl = csv_parse_line( qs );
      if ( qsl.size() > 1 && !our_files.count( qsl[ 0 ] ) )
      {
         names_not_present[ qsl[ 0 ] ] = true;
      }
   }

   map < QString, QString > rename_map;
   for ( map < QString, bool >::iterator it = names_not_present.begin();
         it != names_not_present.end();
         it++ )
   {
      bool ok;
      QString res = US_Static::getItem(
                                          us_tr( "US-SOMO : Cluster DMD Setup : Load : Missing name" ),
                                          QString( us_tr(
                                                      "%1 is found in the DMD file but is not a currently selected PDB.\n"
                                                      "Select a remapping or cancel to ignore:" ) )
                                          .arg( it->first )
                                          , 
                                          qsl_our_files,
                                          0,
                                          false, 
                                          &ok,
                                          this );
      if ( ok ) {
         rename_map[ it->first ] = res;
      } 
   }

   // merge with current data
   csv csv_new = csv1;

   for ( unsigned int i = 0; i < (unsigned int)qsl_lines.size(); i++ ) 
   {
      QString     qs  = qsl_lines[ i ];
      QStringList qsl = csv_parse_line( qs );

      // if 
      if ( qsl.size() > 1 )
      {
         if ( rename_map.count( qsl[ 0 ] ) )
         {
            qsl[ 0 ] = rename_map[ qsl[ 0 ] ];
         }
         if ( our_files.count( qsl[ 0 ] ) && our_files[ qsl[ 0 ] ].size() )
         {
            for ( unsigned int j = 1; j < (unsigned int)qsl.size(); j++ )
            {
               csv_new.data    [ our_files[ qsl[ 0 ] ].front() ][ j ] = qsl[ j ];
               csv_new.num_data[ our_files[ qsl[ 0 ] ].front() ][ j ] = qsl[ j ].toDouble();
            }
            our_files[ qsl[ 0 ] ].pop_front();
         } else {
            vector < QString > tmp_data;
            for ( unsigned int j = 0; j < (unsigned int)qsl.size(); j++ )
            {
               tmp_data.push_back( qsl[ j ] );
            }
            csv_new.prepended_names.push_back( tmp_data[ 0 ] );
            csv_new.data.push_back( tmp_data );
            vector < double > tmp_num_data;
            for ( unsigned int i = 0; i < (unsigned int)tmp_data.size(); i++ )
            {
               tmp_num_data.push_back( tmp_data[ i ].toDouble() );
            }
            csv_new.num_data.push_back( tmp_num_data );
         }
      }
   }
   csv1 = csv_new;

   reload_csv();
   sync_csv_with_selected();
   update_enables();
}

void US_Hydrodyn_Cluster_Dmd::save_csv()
{
   QString use_dir = dmd_dir;

   QString fname = QFileDialog::getSaveFileName( this , us_tr("Choose a filename to save the dmd run parameters") , use_dir , "*.dmd *.dmd" );

   if ( fname.isEmpty() )
   {
      return;
   }

   if ( !fname.contains( QRegExp("\\.dmd$", Qt::CaseInsensitive ) ) )
   {
      fname += ".dmd";
   }
   
   if ( QFile::exists( fname ) )
   {
      fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( fname, 0, this );
      raise();
   }

   QFile f( fname );

   if ( !f.open( QIODevice::WriteOnly ) )
   {
      QMessageBox::warning( this, 
                            us_tr( "US-SOMO : Cluster DMD Setup : Save" ),
                            QString( us_tr( "Can not open file %1 for writing" ) ).arg( fname ) );
      return;
   }

   csv tmp_csv = current_csv();

   QTextStream ts( &f );
   ts << csv_to_qstring( tmp_csv );
   f.close();
   QMessageBox::information( this, 
                             us_tr( "US-SOMO : Cluster DMD Setup : Save" ),
                             QString( us_tr( "File %1 saved" ) ).arg( fname ) );
}

void US_Hydrodyn_Cluster_Dmd::update_enables()
{
   if ( !disable_updates )
   {
      // editor_msg( "black", "-------------" );
      disable_updates = true;
      unsigned int selected = 0;
      vector < int > selected_rows;
      for ( int i = 0; i < t_csv->rowCount(); i++ )
      {
         if ( t_csv->item( i , 0 )->isSelected() )
         {
            selected++;
            selected_rows.push_back( i );
            // editor_msg( "black", QString( "row selected %1" ).arg( i ) );
         }
      }

      // t_csv->clearSelection();
      // for ( unsigned int i = 0; i < (unsigned int)selected_rows.size(); i++ )
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
   disconnect( t_csv, SIGNAL( cellChanged(int, int) ), 0, 0 );
   t_csv->setRowCount( csv1.data.size() );

   for ( unsigned int i = 0; i < (unsigned int)csv1.data.size(); i++ )
   {
      if ( csv1.data[ i ].size() < 3 || csv1.data[ i ][ 2 ].isEmpty() )
      {
         interval_starting_row = i + 1;
      }
      for ( unsigned int j = 0; j < (unsigned int)csv1.data[i].size(); j++ )
      {
         if ( csv1.data[i][j] == "Y" || csv1.data[i][j] == "N" )
         {
            t_csv->setCellWidget( i, j, new QCheckBox() );
            ((QCheckBox *)(t_csv->cellWidget( i, j )))->setChecked( csv1.data[i][j] == "Y" );
         } else {
            t_csv->setItem( i, j, new QTableWidgetItem( csv1.data[i][j] ) );
         }
      }
   }
   connect( t_csv, SIGNAL( cellChanged(int, int) ), SLOT( table_value( int, int ) ) );
   recompute_interval_from_points( 3 );
   recompute_interval_from_points( 7 );
   // t_csv->clearSelection();
}

void US_Hydrodyn_Cluster_Dmd::delete_rows()
{
   csv csv_new = current_csv();
   csv_new.data.clear( );
   csv_new.num_data.clear( );
   csv_new.prepended_names.clear( );
   
   for ( int i = 0; i < t_csv->rowCount(); i++ )
   {
      if ( !t_csv->item( i , 0 )->isSelected() )
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

   for ( int i = 0; i < t_csv->rowCount(); i++ )
   {
      if ( t_csv->item( i , 0 )->isSelected() )
      {
         selected++;
         selected_rows.push_back( i );
      }
   }
   
   if ( selected != t_csv->rowCount() )
   {
      // select all
      for ( int i = 0; i < t_csv->rowCount(); i++ )
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

   for ( unsigned int i = 0; i < (unsigned int)from_csv.header.size(); i++ )
   {
      qs += QString("%1\"%2\"").arg(i ? "," : "").arg(from_csv.header[i]).replace("\n", " ");
   }
   qs += "\n";
   for ( unsigned int i = 0; i < (unsigned int)from_csv.data.size(); i++ )
   {
      for ( unsigned int j = 0; j < (unsigned int)from_csv.data[i].size(); j++ )
      {
         qs += 
            QString("%1%2")
            .arg( j ? "," : "" )
            .arg( from_csv.data[i][j] == QString( "%1" ).arg( from_csv.data[i][j].toDouble() ) ?
                  from_csv.data[i][j] :
                  QString( "\"%1\"" ).arg( from_csv.data[i][j] ) )
            ;
      }
      qs += "\n";
   }

   return qs;
}

void US_Hydrodyn_Cluster_Dmd::row_header_released( int row )
{
   // qDebug() << QString( "row_header_released %1\n" ).arg( row );
   t_csv->clearSelection();
   QTableWidgetSelectionRange qts( row, 0, row, 12 );
   
   t_csv->setRangeSelected( qts, true );
   update_enables();
}

void US_Hydrodyn_Cluster_Dmd::sync_csv_with_selected()
{
   csv1 = current_csv();
   csv csv_new = csv1;
   csv_new.data.clear( );
   csv_new.num_data.clear( );
   csv_new.prepended_names.clear( );

   selected_map.clear();
   map < QString, bool > present_map;
   
   full_filenames.clear( );
   residues_chain          .clear( );
   residues_chain_map      .clear( );
   residues_number         .clear( );
   residues_range_start    .clear( );
   residues_range_end      .clear( );
   residues_range_chain    .clear( );
   residues_range_chain_pos.clear( );

   for ( unsigned int i = 0; i < (unsigned int)((US_Hydrodyn_Cluster *)cluster_window)->selected_files.size(); i++ )
   {
      selected_map[ QFileInfo( ((US_Hydrodyn_Cluster *)cluster_window)->selected_files[ i ] ).fileName() ] = true;

      QString full_filename = ((US_Hydrodyn_Cluster *)cluster_window)->selected_files[ i ];
      QString filename      = QFileInfo( full_filename ).fileName();

      if ( !full_filenames.count( filename ) )
      {
         full_filenames[ filename ] = full_filename;
      } else {
         if ( full_filenames[ filename  ] != full_filename )
         {
             editor_msg( "red", QString( us_tr( "WARNING: The same file name has been referenced in multiple file locations.\n"
                                             "%1 vs %2\n"
                                             "This will cause problems.  Duplicate reference skipped.\n" ) )
                         .arg( full_filenames[ filename ] )
                         .arg( full_filename )
                         );
             continue;
         }
      }                                             
   }

   // add existing ones that are selected
   for ( unsigned int i = 0; i < (unsigned int)csv1.prepended_names.size(); i++ )
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
      if ( !setup_residues( it->first ) )
      {
         continue;
      }
      // residue_summary( it->first );

      if ( !present_map.count( it->first ) )
      {
         vector < QString > tmp_data;
                  
         tmp_data.push_back( it->first );
         tmp_data.push_back( "Y" );
         tmp_data.push_back(".7");
         tmp_data.push_back("100");
         tmp_data.push_back("50");
         tmp_data.push_back("2");
         tmp_data.push_back(".5");
         tmp_data.push_back("10000");
         tmp_data.push_back("200");
         tmp_data.push_back("50");
         tmp_data.push_back("");
         tmp_data.push_back("");
         
         csv_new.prepended_names.push_back( tmp_data[ 0 ] );
         csv_new.data.push_back( tmp_data );
         
         vector < double > tmp_num_data;
         for ( unsigned int i = 0; i < (unsigned int)tmp_data.size(); i++ )
         {
            tmp_num_data.push_back( tmp_data[ i ].toDouble() );
         }
         csv_new.num_data.push_back( tmp_num_data );
      }
   }
   csv1 = csv_new;
   reload_csv();
}


QStringList US_Hydrodyn_Cluster_Dmd::csv_parse_line( QString qs )
{
   // TSO << QString("csv_parse_line:\ninital string <%1>\n").arg(qs);
   QStringList qsl;
   if ( qs.isEmpty() )
   {
      // TSO << QString("csv_parse_line: empty\n");
      return qsl;
   }
   if ( !qs.contains(",") )
   {
      // TSO << QString("csv_parse_line: one token\n");
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
   // TSO << QString("csv_parse_line results:\n<%1>\n").arg(qsl.join(">\n<"));
   return qsl;
}

bool US_Hydrodyn_Cluster_Dmd::setup_residues( QString filename )
{
   if ( !full_filenames.count( filename ) )
   {
      editor_msg( "red", QString( us_tr( "Internal Error: filename %1 has not been cached" ) ).arg( filename ) );
      return false;
   }

   QString full_filename = full_filenames[ filename ];

   QFile f( full_filename );
   if ( !f.exists() )
   {
      editor_msg( "red", QString( us_tr( "Error: file %1 does not exist" ) ).arg( full_filename ) );
      return false;
   }

   if ( !f.open( QIODevice::ReadOnly ) )
   {
      editor_msg( "red", QString( us_tr( "Error: file %1 can not be opened.\nCheck permissions" ) ).arg( full_filename ) );
      return false;
   }

   QTextStream ts( &f );

   QRegExp rx_end ("^END");
   QRegExp rx_atom("^(ATOM|HETATM)");
   QRegExp rx_hetatm("^HETATM");

   map < QString, bool > dup_chain;

   QString last_key = "";
   
   residues_chain          [ filename ].clear( );
   residues_chain_map      [ filename ].clear( );
   residues_number         [ filename ].clear( );
   residues_range_chain    [ filename ].clear( );
   residues_range_chain_pos[ filename ].clear( );

   for ( map < QString, vector < unsigned int > >::iterator it = residues_range_start.begin();
         it != residues_range_start.end();
         it++ )
   {
      if ( it->first.contains( QRegExp( QString( "^%1:" ).arg( filename ) ) ) )
      {
         it->second.clear( );
      }
   }

   while( !ts.atEnd() )
   {
      QString line = ts.readLine();

      if ( rx_end.indexIn( line ) != -1 )
      {
         break;
      }
      if ( rx_atom.indexIn( line ) != -1 )
      {
         QString      chain_id   = line.mid( 21, 1 );
         unsigned int residue_no = line.mid( 22, 4 ).trimmed().toUInt();
         QString      this_key   = chain_id + line.mid( 22, 4 ).trimmed();
         // TSO << QString( "line <%1> chain id <%2> key <%3>\n" ).arg( line.left(30) ).arg( chain_id ).arg( this_key );

         if ( last_key.isEmpty() ||
              this_key != last_key )
         {
            if ( dup_chain.count( this_key ) )
            {
               editor_msg( "red", QString( us_tr( "Error: repeated residue or chain %1 in file %2" ) )
                           .arg( this_key ) 
                           .arg( full_filename ) 
                           );
               f.close();
               return false;
            }
            last_key = this_key;
            residues_chain    [ filename ].push_back( chain_id   );
            residues_number   [ filename ].push_back( residue_no );
            residues_chain_map[ filename ][ chain_id ] = true;
         }
      }
   }
   f.close();

   if ( !residues_chain.count( filename ) )
   {
      editor_msg( "red", QString( us_tr( "Error: no chains or residues found file %1" ) )
                  .arg( full_filename ) 
                  );
      return false;
   }

   if ( residues_chain[ filename ].size() == 1 )
   {
      editor_msg( "red", QString( us_tr( "Error: only one residue %1:%2 found in file %2" ) )
                  .arg( residues_chain [ filename ][ 0 ] ) 
                  .arg( residues_number[ filename ][ 0 ] ) 
                  .arg( full_filename ) 
                  );
      return false;
   }

   unsigned int pos              = 0;
   QString      last_chain       = "";

   for ( unsigned int i = 0; i < (unsigned int)residues_chain[ filename ].size(); i++ )
   {
      QString key           = filename + ":" + residues_chain[ filename ][ i ];
      // TSO << "in residues_chain: " << key << Qt::endl;
      unsigned int this_pos = residues_number[ filename ][ i ];
      if ( last_chain != residues_chain[ filename ][ i ] )
      {
         pos = this_pos;
         // residues_range_start[ key      ].clear( );
         // residues_range_end  [ key      ].clear( );
         residues_range_start[ key      ].push_back( this_pos );
         residues_range_end  [ key      ].push_back( this_pos );
         residues_range_chain    [ filename ].push_back( key );
         residues_range_chain_pos[ filename ].push_back( residues_range_start[ key ].size() - 1 );
         // TSO << "in residues_chain - pushback: " << key << " chain " << residues_chain[ filename ][ i ] << Qt::endl;
         last_chain = residues_chain[ filename ][ i ];
         continue;
      }
      pos++;
      if ( pos != this_pos )
      {
         // break in chain
         pos = this_pos;
         residues_range_start    [ key      ].push_back( this_pos );
         residues_range_end      [ key      ].push_back( this_pos );
         residues_range_chain    [ filename ].push_back( key );
         residues_range_chain_pos[ filename ].push_back( residues_range_start[ key ].size() - 1 );
         continue;
      }
      residues_range_end  [ key ].back() = pos;
   }

   return true;
}

void US_Hydrodyn_Cluster_Dmd::residue_summary( QString filename )
{
   // TSO << "residue summary: " << filename << Qt::endl;
   if ( !residues_chain.count( filename ) )
   {
      editor_msg( "red", QString( us_tr( "Internal Error: filename %1 not prepared for summary" ) ).arg( filename ) );
      return;
   }
      
   unsigned int pos = 0;
   for ( unsigned int i = 0; i < (unsigned int)residues_range_chain[ filename ].size(); i++ )
   {
      QString key = residues_range_chain[ filename ][ i ];
      QString chain_id = key.right( 1 );
      // TSO << "key " << key << Qt::endl;
      if ( !residues_range_start.count( key ) )
      {
         editor_msg( "red", QString( us_tr( "Internal Error: filename %1 range %2 not prepared for summary" ) ).arg( filename ).arg( key ) );
         return;
      }
      if ( !residues_range_end.count( key ) )
      {
         editor_msg( "red", QString( us_tr( "Internal Error: filename %1 range %2 unbalanced for summary" ) ).arg( filename ).arg( key ) );
         return;
      }
      if ( !residues_range_start[ key ].size() )
      {
         editor_msg( "red", QString( us_tr( "Internal Error: filename %1 range %2 empty start key" ) ).arg( filename ).arg( key ) );
         return;
      }
      if ( !residues_range_end[ key ].size() )
      {
         editor_msg( "red", QString( us_tr( "Internal Error: filename %1 range %2 empty end key" ) ).arg( filename ).arg( key ) );
         return;
      }
      if ( residues_range_end[ key ].size() != residues_range_end[ key ].size() )
      {
         editor_msg( "red", QString( us_tr( "Internal Error: filename %1 range %2 start end mismatch" ) ).arg( filename ).arg( key ) );
         return;
      }
      // for ( unsigned int j = 0; j < residues_range_start[ key ].size(); j++ )
      // {
      pos++;

      int resseq_start = (int) residues_range_start[ key ][ residues_range_chain_pos[ filename ][ i ] ];
      int resseq_end   = (int) residues_range_end  [ key ][ residues_range_chain_pos[ filename ][ i ] ];
      
      bool is_hetatm =
         dmd_chain_is_hetatm[ filename ].count( chain_id ) &&
         dmd_chain_is_hetatm[ filename ][ chain_id ].count( resseq_start ) > 0;
      if ( is_hetatm ) {
         bool ok = true;
         if ( !dmd_chain   .count( filename ) ||
              !dmd_res_link.count( filename ) ) {
            editor_msg(
                       "dark red",
                       QString( us_tr( "Error: Internal. dmd_native_range() Could not find %1 in maps" ) )
                       .arg( filename )
                       );
            ok = false;
         }

         if ( !dmd_chain   [ filename ].count( chain_id ) ||
              !dmd_res_link[ filename ].count( chain_id ) ) {
            editor_msg(
                       "dark red",
                       QString( us_tr( "Error: Internal. dmd_native_range() Could not find %1 chain %2 in maps" ) )
                       .arg( filename )
                       .arg( chain_id )
                       );
            ok = false;
         }

         int dmd_chain_start = ok ? dmd_chain[ filename ][ chain_id ][ resseq_start ] : -1;
         int dmd_chain_end   = ok ? dmd_chain[ filename ][ chain_id ][ resseq_end   ] : -1;

         editor_msg( "dark blue", QString( us_tr( "%1: Chain %2 [%3] residue range: %4 - %5\n" ) )
                     .arg( filename )
                     .arg( chain_id )
                     .arg( dmd_chain_start == dmd_chain_end ?
                           QString( "%1" ).arg( dmd_chain_start ) :
                           QString( "%1-%2" ).arg( dmd_chain_start ).arg( dmd_chain_end ) )
                     .arg( residues_range_start[ key ][ residues_range_chain_pos[ filename ][ i ] ] )
                     .arg( residues_range_end  [ key ][ residues_range_chain_pos[ filename ][ i ] ] )
                     );
      } else {
         editor_msg( "dark blue", QString( us_tr( "%1: Chain %2 [%3] residue range: %4 - %5\n" ) )
                     .arg( filename )
                     .arg( chain_id )
                     .arg( pos )
                     .arg( residues_range_start[ key ][ residues_range_chain_pos[ filename ][ i ] ] )
                     .arg( residues_range_end  [ key ][ residues_range_chain_pos[ filename ][ i ] ] )
                     );
      }
         
      // }
   }
}

bool US_Hydrodyn_Cluster_Dmd::convert_static_range( int row )
{
   if ( (unsigned int) t_csv->columnCount() <= 11 ) {
      qDebug() << "US_Hydrodyn_Cluster_Dmd::convert_static_range() insufficient columns";
      return true;
   }

   QString filename     = t_csv->item( row, 0  )->text();
   QString static_range = t_csv->item( row, 10 )->text().trimmed();
   if ( static_range.isEmpty() )
   {
      t_csv->setItem( row, 11, new QTableWidgetItem( "" ) );
      return true;
   }

   QStringList qsl;
   {
      QRegExp rx = QRegExp( "\\s*(\\s|,|;)+\\s*" );
      qsl = (static_range ).split( rx , Qt::SkipEmptyParts );
   }

   TSO << QString( "convert_static_range qsl.size() %1\n" ).arg( qsl.size() );

   QRegExp rx ( "^(|.):(\\d+)(-(\\d+)|)$" );

   QString native_range = "";

   bool has_errors = false;
   for ( unsigned int i = 0; i < (unsigned int)qsl.size(); i++ )
   {
      qsl[ i ] = qsl[ i ].trimmed();
      if ( qsl[ i ].isEmpty() )
      {
         continue;
      }
      if ( rx.indexIn( qsl[ i ] ) == -1 )
      {
         editor_msg( "red",
                     QString( us_tr( "Error: Static range \"%1\" has an invalid format.\n"
                                  "The format must be \"Chain:residue_number\" or \"Chain:start_residue_number-end_residue_residue_number\"\n" ) )
                     .arg( qsl[ i ] ) 
                     );
         has_errors = true;
         continue;
      }
      QString      chain_id      = rx.cap( 1 );
      unsigned int start_residue = rx.cap( 2 ).toUInt();
      unsigned int end_residue   = rx.cap( 4 ).toUInt();
      if ( rx.cap( 4 ).isEmpty() )
      {
         end_residue = start_residue;
      }
      if ( start_residue > end_residue )
      {
         editor_msg( "red",
                     QString( us_tr( "Error: Static range \"%1\" has an invalid format.\n"
                                  "The end residue number is less that the start residue number\n" ) )
                     .arg( qsl[ i ] ) 
                     );
         has_errors = true;
         continue;
      }
         
      if ( chain_id.isEmpty() )
      {
         chain_id = " ";
      }
      //      editor_msg( "dark gray", QString( "chain %1 residues: [%2-%3]" )
      //                  .arg( chain_id )
      //                  .arg( start_residue )
      //                  .arg( end_residue ) );

      if ( !residues_chain_map.count( filename ) ||
           !residues_chain_map[ filename ].count( chain_id ) )
      {
         editor_msg( "red",
                     QString( us_tr( "Error: No matching chain found for static range \"%1\" chain %2." ) )
                     .arg( qsl[ i ] ) 
                     .arg( chain_id ) 
                     );
         has_errors = true;
         continue;
      }
      // now intersect the selected range with each of the recognized ranges
      // offset and append to native_range
      map < unsigned int, unsigned int > used_bits;
      for ( unsigned int j = start_residue; j <= end_residue; j++ )
      {
         used_bits[ j ] = 0;
      }
      unsigned int pos = 0;
      for ( unsigned int j = 0; j < (unsigned int)residues_range_chain[ filename ].size(); j++ )
      {
         QString key = residues_range_chain[ filename ][ j ];
         pos++;
         if ( chain_id !=  key.right( 1 ) )
         {
            // pos += residues_range_start[ key ].size();
            continue;
         }
         unsigned int chain_pos        = residues_range_chain_pos[ filename ][ j         ];
         unsigned int this_chain_start = residues_range_start    [ key      ][ chain_pos ];
         // unsigned int this_chain_base  = this_chain_start;
         unsigned int this_chain_end   = residues_range_end      [ key      ][ chain_pos ];
         if ( this_chain_start < start_residue )
         {
            this_chain_start = start_residue;
         }
         if ( this_chain_end > end_residue )
         {
            this_chain_end = end_residue;
         }
         if ( this_chain_start <= this_chain_end )
         {
            {
               QString dmd_static;
               if ( !dmd_native_range(
                                      filename,
                                      chain_id,
                                      (int) this_chain_start,
                                      (int) this_chain_end,
                                      dmd_static
                                      ) ) {
                  continue;
               }
               native_range += ( native_range.isEmpty() ? "" : "," ) + dmd_static;
            }
            
            // native_range += QString( "%1%2.%3.*" )
            //    .arg( native_range.isEmpty() ? "" : "," )
            //    .arg( pos )
            //    .arg( this_chain_start < this_chain_end ? 
            //          QString( "%1-%2" )
            //          .arg( this_chain_start - this_chain_base + 1 )
            //          .arg( this_chain_end   - this_chain_base + 1 ) :
            //          QString( "%1" ).arg( this_chain_start - this_chain_base + 1 ) )
            //    ;
            for ( unsigned int k = this_chain_start; k <= this_chain_end; k++ )
            {
               used_bits[ k ]++;
            }
         }
      }
      for ( unsigned int j = start_residue; j <= end_residue; j++ )
      {
         if ( used_bits[ j ] == 1 )
         {
            continue;
         }
         if ( used_bits[ j ] == 0 )
         {
            editor_msg( "dark red",
                        QString( us_tr( "Warning: Unused static residue %1:%2 from static entry %3" ) )
                        .arg( chain_id ) 
                        .arg( j ) 
                        .arg( qsl[ i ] ) 
                        );
            continue;
         }
         editor_msg( "dark red",
                     QString( us_tr( "Warning: Multiply used static residue %1:%2 from static entry %3" ) )
                     .arg( chain_id ) 
                     .arg( j ) 
                     .arg( qsl[ i ] ) 
                     );
      }
   }
   if ( has_errors )
   {
      t_csv->setItem( row, 11, new QTableWidgetItem( "" ) );
      return false;
   }
   t_csv->setItem( row, 11, new QTableWidgetItem( native_range ) );
   return true;
}

bool US_Hydrodyn_Cluster_Dmd::dmd_native_range( const QString & filename,
                                                const QString & chainid,
                                                const int resseq_start,
                                                const int resseq_end,
                                                QString & dmd_static ) {
   dmd_static = "";

   if ( !dmd_chain   .count( filename ) ||
        !dmd_res_link.count( filename ) ) {
      editor_msg(
                 "dark red",
                 QString( us_tr( "Error: Internal. dmd_native_range() Could not find %1 in maps" ) )
                 .arg( filename )
                 );
      return false;
   }

   if ( !dmd_chain   [ filename ].count( chainid ) ||
        !dmd_res_link[ filename ].count( chainid ) ) {
      editor_msg(
                 "dark red",
                 QString( us_tr( "Error: Internal. dmd_native_range() Could not find %1 chain %2 in maps" ) )
                 .arg( filename )
                 .arg( chainid )
                 );
      return false;
   }

   if ( !dmd_chain   [ filename ][ chainid ].count( resseq_start ) ||
        !dmd_res_link[ filename ][ chainid ].count( resseq_start ) ) {
      editor_msg(
                 "dark red",
                 QString( us_tr( "Error: Internal. dmd_native_range() Could not find %1 chain %2 residue %3 (start) in maps" ) )
                 .arg( filename )
                 .arg( chainid )
                 .arg( resseq_start )
                 );
      return false;
   }

   if ( !dmd_chain   [ filename ][ chainid ].count( resseq_end ) ||
        !dmd_res_link[ filename ][ chainid ].count( resseq_end ) ) {
      editor_msg(
                 "dark red",
                 QString( us_tr( "Error: Internal. dmd_native_range() Could not find %1 chain %2 residue %3 (end) in maps" ) )
                 .arg( filename )
                 .arg( chainid )
                 .arg( resseq_end )
                 );
      return false;
   }

   int dmd_chain_start = dmd_chain   [ filename ][ chainid ][ resseq_start ];
   int dmd_chain_end   = dmd_chain   [ filename ][ chainid ][ resseq_end   ];
   int dmd_res_start   = dmd_res_link[ filename ][ chainid ][ resseq_start ];
   int dmd_res_end     = dmd_res_link[ filename ][ chainid ][ resseq_end   ];
   bool is_hetatm      =
      dmd_chain_is_hetatm[ filename ].count( chainid ) &&
      dmd_chain_is_hetatm[ filename ][ chainid ].count( resseq_start ) > 0;

   if ( dmd_chain_start == dmd_chain_end ) {
      dmd_static += QString( "%1" ).arg( dmd_chain_start );
   } else {
      dmd_static += QString( "%1-%2" ).arg( dmd_chain_start ).arg( dmd_chain_end );
   }
   dmd_static += ".";
   if ( is_hetatm ) {
      dmd_static += "1";
   } else {
      if ( dmd_res_start == dmd_res_end ) {
         dmd_static += QString( "%1" ).arg( dmd_res_start );
      } else {
         dmd_static += QString( "%1-%2" ).arg( dmd_res_start ).arg( dmd_res_end );
      }
   }
   dmd_static += ".*";
   return true;
}
