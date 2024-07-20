#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_mals_saxs.h"
#include "../include/us_hydrodyn_mals_saxs_conc.h"
#include "../include/us_hydrodyn_mals_saxs_conc_load.h"
//Added by qt3to4:
#include <QTextStream>
#include <QHBoxLayout>
#include <QCloseEvent>
#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>

US_Hydrodyn_Mals_Saxs_Conc::US_Hydrodyn_Mals_Saxs_Conc(
                                                           csv &csv1,
                                                           void *mals_saxs_window,
                                                           QWidget *p, 
                                                           const char *
                                                           ) : QFrame( p )
{
   org_csv = &csv1;
   this->csv1 = csv1;
   this->mals_saxs_window = mals_saxs_window;
   ((US_Hydrodyn_Mals_Saxs *)mals_saxs_window)->conc_widget = true;
   us_hydrodyn = ((US_Hydrodyn_Mals_Saxs *)mals_saxs_window)->us_hydrodyn;
   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle( us_tr( "US-SOMO: MALS_SAXS: File Concentrations"));
   order_ascending = false;
   disable_updates = false;
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;

   unsigned int csv_height = t_csv->rowHeight(0) + 125;
   unsigned int csv_width = t_csv->columnWidth(0) + 60;
   for ( int i = 1; i < t_csv->rowCount(); i++ )
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
   // cout << QString("csv size %1 %2\n").arg(csv_height).arg(csv_width);

   setGeometry(global_Xpos, global_Ypos, csv_width, 100 + csv_height );
   update_enables();
}

void US_Hydrodyn_Mals_Saxs_Conc::refresh( csv &ref_csv )
{
   csv new_csv = ref_csv;
   csv our_csv = current_csv();

   map < QString, unsigned int > our_files;
   for ( unsigned int i = 0; i < our_csv.data.size(); i++ )
   {
      our_files[ our_csv.data[ i ][ 0 ] ] = i;
   }

   // copy data values over
   for ( unsigned int i = 0; i < new_csv.data.size(); i++ )
   {
      if ( our_files.count( new_csv.data[ i ][ 0 ] ) )
      {
         new_csv.data[ i ][ 1 ] = our_csv.data[ our_files[ new_csv.data[ i ][ 0 ] ] ][ 1 ];
      }
   }

   csv1 = new_csv;

   t_csv->setRowCount( csv1.data.size());
   for ( unsigned int i = 0; i < csv1.data.size(); i++ )
   {
      for ( unsigned int j = 0; j < csv1.data[i].size(); j++ )
      {
         t_csv->setItem(i, j, new QTableWidgetItem( csv1.data[i][j]) );
      }
   }

   unsigned int csv_height = t_csv->rowHeight(0) + 125;
   unsigned int csv_width = t_csv->columnWidth(0) + 60;
   for ( int i = 1; i < t_csv->rowCount(); i++ )
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

   setGeometry(
               geometry().left(),
               geometry().top(), 
               csv_width, 
               100 + csv_height );
}

US_Hydrodyn_Mals_Saxs_Conc::~US_Hydrodyn_Mals_Saxs_Conc()
{
}

void US_Hydrodyn_Mals_Saxs_Conc::setupGUI()
{
   int minHeight1 = 30;

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

   for ( unsigned int i = 0; i < csv1.data.size(); i++ )
   {
      for ( unsigned int j = 0; j < csv1.data[i].size(); j++ )
      {
         t_csv->setItem(i, j, new QTableWidgetItem( csv1.data[i][j]) );
      }
   }

   for ( unsigned int i = 0; i < csv1.header.size(); i++ )
   {
      t_csv->setHorizontalHeaderItem(i, new QTableWidgetItem( csv1.header[i]));
   }

   t_csv->setSortingEnabled(false);
    t_csv->verticalHeader()->setSectionsMovable(false);
    t_csv->horizontalHeader()->setSectionsMovable(false);
   { for ( int i = 0; i < t_csv->rowCount(); ++i ) { t_csv->item( i,  0 )->setFlags( t_csv->item( i,  0 )->flags() ^ Qt::ItemIsEditable ); } };
   { for ( int i = 0; i < t_csv->rowCount(); ++i ) { t_csv->item( i,  1 )->setFlags( t_csv->item( i,  1 )->flags() | Qt::ItemIsEditable ); } };
   t_csv->setColumnWidth(0, 330);
   t_csv->setColumnWidth(1, 150);
   
    t_csv->horizontalHeader()->setSectionsClickable( true );
#if QT_VERSION < 0x040000   
   connect(t_csv->horizontalHeader(), SIGNAL(clicked(int)), SLOT(sort_column(int)));
#else
   connect(t_csv->horizontalHeader(), SIGNAL(sectionClicked(int)), SLOT(sort_column(int)));
#endif
   
   // probably I'm not understanding something, but these next two lines don't seem to do anything
   // t_csv->horizontalHeader()->adjustHeaderSize();
   t_csv->adjustSize();
   connect( t_csv, SIGNAL( itemSelectionChanged() ), SLOT( update_enables() ) );
   connect( t_csv->verticalHeader(), SIGNAL( released( int ) ), SLOT( row_header_released( int ) ) );
   
   pb_load = new QPushButton(us_tr("Load"), this);
   pb_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_load->setMinimumHeight(minHeight1);
   pb_load->setPalette( PALET_PUSHB );
   connect(pb_load, SIGNAL(clicked()), SLOT(load()));

   pb_save = new QPushButton(us_tr("Save to file"), this);
   pb_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_save->setMinimumHeight(minHeight1);
   pb_save->setPalette( PALET_PUSHB );
   connect(pb_save, SIGNAL(clicked()), SLOT(save()));

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

   pb_set_ok = new QPushButton(us_tr("Save values"), this);
   pb_set_ok->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_set_ok->setMinimumHeight(minHeight1);
   pb_set_ok->setPalette( PALET_PUSHB );
   connect(pb_set_ok, SIGNAL(clicked()), SLOT(set_ok()));

   // build layout

   QHBoxLayout * hbl_ctls = new QHBoxLayout(); hbl_ctls->setContentsMargins( 0, 0, 0, 0 ); hbl_ctls->setSpacing( 0 );
   hbl_ctls->addSpacing( 4 );
   hbl_ctls->addWidget ( pb_copy );
   hbl_ctls->addSpacing( 4 );
   hbl_ctls->addWidget ( pb_paste );
   hbl_ctls->addSpacing( 4 );
   hbl_ctls->addWidget ( pb_paste_all );
   hbl_ctls->addSpacing( 4 );

   QHBoxLayout * hbl_load_save = new QHBoxLayout(); hbl_load_save->setContentsMargins( 0, 0, 0, 0 ); hbl_load_save->setSpacing( 0 );
   hbl_load_save->addSpacing( 4 );
   hbl_load_save->addWidget ( pb_load );
   hbl_load_save->addSpacing( 4 );
   hbl_load_save->addWidget ( pb_save );
   hbl_load_save->addSpacing( 4 );

   QHBoxLayout * hbl_bottom = new QHBoxLayout; hbl_bottom->setContentsMargins( 0, 0, 0, 0 ); hbl_bottom->setSpacing( 0 );
   hbl_bottom->addSpacing(4);
   hbl_bottom->addWidget(pb_cancel);
   hbl_bottom->addSpacing(4);
   hbl_bottom->addWidget(pb_help);
   hbl_bottom->addSpacing(4);
   hbl_bottom->addWidget(pb_set_ok);
   hbl_bottom->addSpacing(4);

   QVBoxLayout * background = new QVBoxLayout(this); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 );
   background->addSpacing(4);
   background->addWidget(lbl_title);
   background->addSpacing(4);
   background->addWidget(t_csv);
   background->addSpacing(4);
   background->addLayout(hbl_ctls);
   background->addSpacing(4);
   background->addLayout(hbl_load_save);
   background->addSpacing(4);
   background->addLayout(hbl_bottom);
   background->addSpacing(4);
}

void US_Hydrodyn_Mals_Saxs_Conc::cancel()
{
   close();
}

void US_Hydrodyn_Mals_Saxs_Conc::set_ok()
{
   *org_csv = current_csv();
   ((US_Hydrodyn_Mals_Saxs *)mals_saxs_window)->update_enables();
   close();
}

void US_Hydrodyn_Mals_Saxs_Conc::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo/somo_mals_saxs_conc.html");
}

void US_Hydrodyn_Mals_Saxs_Conc::closeEvent(QCloseEvent *e)
{
   ((US_Hydrodyn_Mals_Saxs *)mals_saxs_window)->conc_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Mals_Saxs_Conc::sort_column( int section )
{
   t_csv->sortByColumn(section,  order_ascending ? Qt::AscendingOrder : Qt::DescendingOrder );
   order_ascending = !order_ascending;
}

csv US_Hydrodyn_Mals_Saxs_Conc::current_csv()
{
   csv tmp_csv = csv1;
   
   for ( unsigned int i = 0; i < csv1.data.size(); i++ )
   {
      for ( unsigned int j = 0; j < csv1.data[i].size(); j++ )
      {
         tmp_csv.data[i][j] = t_csv->item( i, j )->text();
         tmp_csv.num_data[i][j] = tmp_csv.data[i][j].toDouble();
      }
   }
   return tmp_csv;
}

void US_Hydrodyn_Mals_Saxs_Conc::copy()
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

   // cout << "csv1 after copy():\n" << csv_to_qstring( csv1 ) << endl;
   // cout << "csv_copy after copy():\n" << csv_to_qstring( csv_copy ) << endl;

   update_enables();
}

void US_Hydrodyn_Mals_Saxs_Conc::paste()
{
   csv1 = current_csv();
   unsigned int pos = 0;
   for ( int i = 0; i < t_csv->rowCount(); i++ )
   {
      // editor_msg( "black", QString( "checking row %1" ).arg( i ) );
      if ( t_csv->item( i , 0 )->isSelected() )
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

void US_Hydrodyn_Mals_Saxs_Conc::paste_all()
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

void US_Hydrodyn_Mals_Saxs_Conc::update_enables()
{
   if ( !disable_updates )
   {
      disable_updates = true;
      unsigned int selected = 0;
      vector < int > selected_rows;
      for ( int i = 0; i < t_csv->rowCount(); i++ )
      {
         if ( t_csv->item( i , 0 )->isSelected() )
         {
            selected++;
            selected_rows.push_back( i );
         }
      }

      pb_copy     ->setEnabled( selected == 1 );
      pb_paste    ->setEnabled( selected && csv_copy.data.size() );
      pb_paste_all->setEnabled( csv_copy.data.size() );
      pb_save     ->setEnabled( csv1.data.size() );
      pb_load     ->setEnabled( true );
      disable_updates = false;
   }
}

void US_Hydrodyn_Mals_Saxs_Conc::row_header_released( int row )
{
   // cout << QString( "row_header_released %1\n" ).arg( row );
   t_csv->clearSelection();
   QTableWidgetSelectionRange qts( row, 0, row, 2 );
   
   t_csv->setRangeSelected( qts, true );
   update_enables();
}

void US_Hydrodyn_Mals_Saxs_Conc::reload_csv()
{
   t_csv->setRowCount( csv1.data.size() );
   for ( unsigned int i = 0; i < csv1.data.size(); i++ )
   {
      for ( unsigned int j = 0; j < csv1.data[i].size(); j++ )
      {
         t_csv->setItem( i, j, new QTableWidgetItem( csv1.data[i][j] ) );
      }
   }
   // t_csv->clearSelection();
}

void US_Hydrodyn_Mals_Saxs_Conc::load()
{

   QString use_dir = QDir::currentPath();

   ((US_Hydrodyn *)us_hydrodyn)->select_from_directory_history( use_dir, this );
   raise();

   QString fname = QFileDialog::getOpenFileName( this , windowTitle() , use_dir , "Concentration files (*.sbc *.SBC);;"
                                                "Text files (*.txt *.TXT);;"
                                                "All Files (*)" );

   if ( fname.isEmpty() )
   {
      return;
   }

   if ( !QFile::exists( fname ) )
   {
      QMessageBox::warning( this, 
                            us_tr( "US-SOMO: MALS_SAXS Subtraction Utility: Concentrations" ),
                            QString( us_tr( "File %1 does not exist" ) ).arg( fname ) );
      return;
   }

   QFile f( fname );

   if ( !f.open( QIODevice::ReadOnly ) )
   {
      QMessageBox::warning( this, 
                            us_tr( "US-SOMO: MALS_SAXS Subtraction Utility: Concentrations" ),
                            QString( us_tr( "Can not open file %1 for reading" ) ).arg( fname ) );
      return;
   }

   csv1 = current_csv();
   
   map < QString, unsigned int > our_files;
   for ( unsigned int i = 0; i < csv1.data.size(); i++ )
   {
      our_files[ csv1.data[ i ][ 0 ] ] = i;
   }
      
   QTextStream ts( &f );

   QStringList qsl_lines;
      
   while ( !ts.atEnd() )
   {
      QString qs = ts.readLine();
      qsl_lines << qs;
   }
   f.close();
      
   if ( QFileInfo( fname ).suffix().contains( QRegExp( "^(sbc|SBC)$" ) ) )
   {
      for ( unsigned int i = 0; i < (unsigned int) qsl_lines.size(); i++ ) 
      {
         QString qs = qsl_lines[ i ];
         QStringList qsl = csv_parse_line( qs );
         if ( qsl.size() > 1 &&
              our_files.count( qsl[ 0 ] ) )
         {
            csv1.data[ our_files[ qsl[ 0 ] ] ][ 1 ] = qsl[ 1 ];
         }
      }
   } else {
      US_Hydrodyn_Mals_Saxs_Conc_Load *hbscl = 
         new US_Hydrodyn_Mals_Saxs_Conc_Load(
                                               qsl_lines,
                                               csv1,
                                               this );
      US_Hydrodyn::fixWinButtons( hbscl );
      if ( hbscl->disp_csv.data.size() )
      {
         hbscl->exec();
      } else {
         QMessageBox::warning( this, 
                               us_tr( "US-SOMO: MALS_SAXS: File Concentrations"),
                               us_tr( "The file does not seem to contain data the this program understands" ) );
         delete hbscl;
         return;
      }
      delete hbscl;
   }

   reload_csv();
   update_enables();
}

void US_Hydrodyn_Mals_Saxs_Conc::save()
{
   QString use_dir = QDir::currentPath();

   ((US_Hydrodyn *)us_hydrodyn)->select_from_directory_history( use_dir, this );
   raise();

   QString fname = QFileDialog::getSaveFileName( this , us_tr("Choose a filename to save the concentrations") , use_dir , "*.sbc *.SBC" );

   if ( fname.isEmpty() )
   {
      return;
   }

   if ( !fname.contains(QRegExp(".sbc$", Qt::CaseInsensitive )) )
   {
      fname += ".sbc";
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
                            us_tr( "US-SOMO: MALS_SAXS Subtraction Utility: Concentrations" ),
                            QString( us_tr( "Can not open file %1 for writing" ) ).arg( fname ) );
      return;
   }

   ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( fname );

   csv tmp_csv = current_csv();

   QTextStream ts( &f );
   ts << csv_to_qstring( tmp_csv );
   f.close();
   QMessageBox::information( this, 
                             us_tr( "US-SOMO: MALS_SAXS Subtraction Utility: Concentrations" ),
                             QString( us_tr( "File %1 saved" ) ).arg( fname ) );
}

QString US_Hydrodyn_Mals_Saxs_Conc::csv_to_qstring( csv from_csv )
{
   QString qs;

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

QStringList US_Hydrodyn_Mals_Saxs_Conc::csv_parse_line( QString qs )
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
