#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_saxs_buffer.h"
#include "../include/us_hydrodyn_saxs_buffer_conc.h"

US_Hydrodyn_Saxs_Buffer_Conc::US_Hydrodyn_Saxs_Buffer_Conc(
                                               csv &csv1,
                                               void *saxs_buffer_window,
                                               QWidget *p, 
                                               const char *name
                                               ) : QFrame(p, name)
{
   org_csv = &csv1;
   this->csv1 = csv1;
   this->saxs_buffer_window = saxs_buffer_window;
   ((US_Hydrodyn_Saxs_Buffer *)saxs_buffer_window)->conc_widget = true;
   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption( tr( "US-SOMO: SAXS Buffer: File Concentrations"));
   order_ascending = false;
   disable_updates = false;
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;

   unsigned int csv_height = t_csv->rowHeight(0) + 125;
   unsigned int csv_width = t_csv->columnWidth(0) + 60;
   for ( int i = 1; i < t_csv->numRows(); i++ )
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
   // cout << QString("csv size %1 %2\n").arg(csv_height).arg(csv_width);

   setGeometry(global_Xpos, global_Ypos, csv_width, 100 + csv_height );
   update_enables();
}

void US_Hydrodyn_Saxs_Buffer_Conc::refresh( csv &ref_csv )
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

   t_csv->setNumRows( csv1.data.size());
   for ( unsigned int i = 0; i < csv1.data.size(); i++ )
   {
      for ( unsigned int j = 0; j < csv1.data[i].size(); j++ )
      {
         t_csv->setText(i, j, csv1.data[i][j]);
      }
   }

   unsigned int csv_height = t_csv->rowHeight(0) + 125;
   unsigned int csv_width = t_csv->columnWidth(0) + 60;
   for ( int i = 1; i < t_csv->numRows(); i++ )
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

   setGeometry(
               geometry().left(),
               geometry().top(), 
               csv_width, 
               100 + csv_height );
}

US_Hydrodyn_Saxs_Buffer_Conc::~US_Hydrodyn_Saxs_Buffer_Conc()
{
}

void US_Hydrodyn_Saxs_Buffer_Conc::setupGUI()
{
   int minHeight1 = 30;

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

   for ( unsigned int i = 0; i < csv1.data.size(); i++ )
   {
      for ( unsigned int j = 0; j < csv1.data[i].size(); j++ )
      {
         t_csv->setText(i, j, csv1.data[i][j]);
      }
   }

   for ( unsigned int i = 0; i < csv1.header.size(); i++ )
   {
      t_csv->horizontalHeader()->setLabel(i, csv1.header[i]);
   }

   t_csv->setSorting(false);
   t_csv->setRowMovingEnabled(false);
   t_csv->setColumnMovingEnabled(false);
   t_csv->setColumnReadOnly( 0, true );
   t_csv->setColumnReadOnly( 1, false );
   t_csv->setColumnWidth(0, 330);
   t_csv->setColumnWidth(1, 150);
   
   t_csv->horizontalHeader()->setClickEnabled(true);
   connect(t_csv->horizontalHeader(), SIGNAL(clicked(int)), SLOT(sort_column(int)));
   
   // probably I'm not understanding something, but these next two lines don't seem to do anything
   t_csv->horizontalHeader()->adjustHeaderSize();
   t_csv->adjustSize();
   connect( t_csv, SIGNAL( selectionChanged() ), SLOT( update_enables() ) );
   connect( t_csv->verticalHeader(), SIGNAL( released( int ) ), SLOT( row_header_released( int ) ) );
   
   pb_load = new QPushButton(tr("Load"), this);
   pb_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_load->setMinimumHeight(minHeight1);
   pb_load->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_load, SIGNAL(clicked()), SLOT(load()));

   pb_save = new QPushButton(tr("Save to file"), this);
   pb_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_save->setMinimumHeight(minHeight1);
   pb_save->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_save, SIGNAL(clicked()), SLOT(save()));

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

   pb_set_ok = new QPushButton(tr("Save values"), this);
   pb_set_ok->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_set_ok->setMinimumHeight(minHeight1);
   pb_set_ok->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_set_ok, SIGNAL(clicked()), SLOT(set_ok()));

   // build layout

   QHBoxLayout *hbl_ctls = new QHBoxLayout(0);
   hbl_ctls->addSpacing( 4 );
   hbl_ctls->addWidget ( pb_copy );
   hbl_ctls->addSpacing( 4 );
   hbl_ctls->addWidget ( pb_paste );
   hbl_ctls->addSpacing( 4 );
   hbl_ctls->addWidget ( pb_paste_all );
   hbl_ctls->addSpacing( 4 );

   QHBoxLayout *hbl_load_save = new QHBoxLayout(0);
   hbl_load_save->addSpacing( 4 );
   hbl_load_save->addWidget ( pb_load );
   hbl_load_save->addSpacing( 4 );
   hbl_load_save->addWidget ( pb_save );
   hbl_load_save->addSpacing( 4 );

   QHBoxLayout *hbl_bottom = new QHBoxLayout;
   hbl_bottom->addSpacing(4);
   hbl_bottom->addWidget(pb_cancel);
   hbl_bottom->addSpacing(4);
   hbl_bottom->addWidget(pb_help);
   hbl_bottom->addSpacing(4);
   hbl_bottom->addWidget(pb_set_ok);
   hbl_bottom->addSpacing(4);

   QVBoxLayout *background = new QVBoxLayout(this);
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

void US_Hydrodyn_Saxs_Buffer_Conc::cancel()
{
   close();
}

void US_Hydrodyn_Saxs_Buffer_Conc::set_ok()
{
   *org_csv = current_csv();
   close();
}

void US_Hydrodyn_Saxs_Buffer_Conc::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_saxs_buffer_conc.html");
}

void US_Hydrodyn_Saxs_Buffer_Conc::closeEvent(QCloseEvent *e)
{
   ((US_Hydrodyn_Saxs_Buffer *)saxs_buffer_window)->conc_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Saxs_Buffer_Conc::sort_column( int section )
{
   t_csv->sortColumn(section, order_ascending, true);
   order_ascending = !order_ascending;
}

csv US_Hydrodyn_Saxs_Buffer_Conc::current_csv()
{
   csv tmp_csv = csv1;
   
   for ( unsigned int i = 0; i < csv1.data.size(); i++ )
   {
      for ( unsigned int j = 0; j < csv1.data[i].size(); j++ )
      {
         tmp_csv.data[i][j] = t_csv->text( i, j );
         tmp_csv.num_data[i][j] = tmp_csv.data[i][j].toDouble();
      }
   }
   return tmp_csv;
}

void US_Hydrodyn_Saxs_Buffer_Conc::copy()
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

void US_Hydrodyn_Saxs_Buffer_Conc::paste()
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

void US_Hydrodyn_Saxs_Buffer_Conc::paste_all()
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

void US_Hydrodyn_Saxs_Buffer_Conc::update_enables()
{
   if ( !disable_updates )
   {
      disable_updates = true;
      unsigned int selected = 0;
      vector < int > selected_rows;
      for ( int i = 0; i < t_csv->numRows(); i++ )
      {
         if ( t_csv->isRowSelected( i ) )
         {
            selected++;
            selected_rows.push_back( i );
         }
      }

      pb_copy     ->setEnabled( selected == 1 );
      pb_paste    ->setEnabled( selected && csv_copy.data.size() );
      pb_paste_all->setEnabled( csv_copy.data.size() );
      // pb_save     ->setEnabled( csv1.data.size() );
      pb_save     ->setEnabled( false );
      pb_load     ->setEnabled( false );
      disable_updates = false;
   }
}

void US_Hydrodyn_Saxs_Buffer_Conc::row_header_released( int row )
{
   // cout << QString( "row_header_released %1\n" ).arg( row );
   t_csv->clearSelection();
   QTableSelection qts( row, 0, row, 2 );
   
   t_csv->addSelection( qts );
   update_enables();
}

void US_Hydrodyn_Saxs_Buffer_Conc::reload_csv()
{
   t_csv->setNumRows( csv1.data.size() );
   for ( unsigned int i = 0; i < csv1.data.size(); i++ )
   {
      for ( unsigned int j = 0; j < csv1.data[i].size(); j++ )
      {
         t_csv->setText( i, j, csv1.data[i][j] );
      }
   }
   // t_csv->clearSelection();
}

void US_Hydrodyn_Saxs_Buffer_Conc::load()
{
}

void US_Hydrodyn_Saxs_Buffer_Conc::save()
{
}

