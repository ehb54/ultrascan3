#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_saxs_buffer.h"
#include "../include/us_hydrodyn_saxs_buffer_conc.h"
#include "../include/us_hydrodyn_saxs_buffer_conc_load.h"
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <QCloseEvent>
#include <Q3Frame>
#include <QLabel>
#include <Q3VBoxLayout>

US_Hydrodyn_Saxs_Buffer_Conc_Load::US_Hydrodyn_Saxs_Buffer_Conc_Load(
                                                                     QStringList &qsl_text,
                                                                     csv &csv1,
                                                                     QWidget *p, 
                                                                     const char *name
                                                                     ) : QDialog(p, name)
{
   this->text = &qsl_text;
   this->csv1 = &csv1;

   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setCaption( tr( "US-SOMO: SAXS Buffer: File Concentration Loader"));

   disable_updates = false;

   // try to break text up

   vector < QStringList > lines;
   QRegExp rx_split( "(,|)\\s+" );

   disp_csv.name = this->csv1->name;

   // remove rows that are not all numeric
   QRegExp rx_numeric( "^(-|\\+|)(\\d*(|\\.)(\\d+))(|(e|E)(-|\\+|)\\d+)$" );

   for ( unsigned int i = 0; i < (unsigned int) qsl_text.size(); i++ )
   {
      QString qs = qsl_text[ i ].stripWhiteSpace();
      QStringList line = QStringList::split( rx_split, qs );
      if ( line.size() > 1 )
      {
         lines.push_back( line );
         vector < QString > tmp_data;
         bool all_numeric = true;
         for ( unsigned int j = 0; j < (unsigned int) line.size(); j++ )
         {
            if ( rx_numeric.search( line[ j ].stripWhiteSpace() ) != -1 )
            {
               tmp_data.push_back( line[ j ].stripWhiteSpace() );
            } else {
               all_numeric = false;
               break;
            }
         }

         if ( all_numeric )
         {
            disp_csv.data.push_back( tmp_data );
            if ( disp_csv.header.size() < tmp_data.size() )
            {
               disp_csv.header.resize( tmp_data.size() );
            }
         }
      }
   }
   
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

US_Hydrodyn_Saxs_Buffer_Conc_Load::~US_Hydrodyn_Saxs_Buffer_Conc_Load()
{
}

void US_Hydrodyn_Saxs_Buffer_Conc_Load::setupGUI()
{
   int minHeight1 = 30;

   lbl_title = new QLabel(disp_csv.name.left(80), this);
   lbl_title->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Raised);
   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette( PALET_FRAME );
   AUTFBACK( lbl_title );
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   // cout << csv_to_qstring( disp_csv );

   t_csv = new Q3Table(disp_csv.data.size(), disp_csv.header.size(), this);
   t_csv->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Raised);
   t_csv->setPalette( PALET_EDIT );
   AUTFBACK( t_csv );
   t_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));
   t_csv->setEnabled(true);
   // t_csv->setSelectionMode( QTable::SingleRow );

   for ( unsigned int i = 0; i < disp_csv.data.size(); i++ )
   {
      for ( unsigned int j = 0; j < disp_csv.data[ i ].size(); j++ )
      {
         t_csv->setText( i, j, disp_csv.data[ i ][ j ]);
      }
   }

   t_csv->setSorting            ( false );
   t_csv->setRowMovingEnabled   ( true );
   t_csv->setColumnMovingEnabled( false );
   
   t_csv->adjustSize();
   connect( t_csv, SIGNAL( selectionChanged() ), SLOT( update_enables() ) );
   connect( t_csv->horizontalHeader(), SIGNAL( released( int ) ), SLOT( col_header_released( int ) ) );
   
   pb_del_row = new QPushButton(tr("Delete Rows"), this);
   pb_del_row->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_del_row->setMinimumHeight(minHeight1);
   pb_del_row->setPalette( PALET_PUSHB );
   connect(pb_del_row, SIGNAL(clicked()), SLOT(del_row()));

   pb_set_name = new QPushButton(tr("Set column for frame timecode"), this);
   pb_set_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_set_name->setMinimumHeight(minHeight1);
   pb_set_name->setPalette( PALET_PUSHB );
   connect(pb_set_name, SIGNAL(clicked()), SLOT(set_name()));

   lbl_name = new QLabel("", this);
   lbl_name->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_name->setMinimumHeight(minHeight1);
   lbl_name->setPalette( PALET_LABEL );
   AUTFBACK( lbl_name );
   lbl_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   pb_adjust = new QPushButton(tr("Adjust frame timecode"), this);
   pb_adjust->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_adjust->setMinimumHeight(minHeight1);
   pb_adjust->setPalette( PALET_PUSHB );
   connect(pb_adjust, SIGNAL(clicked()), SLOT(adjust()));

   pb_set_conc = new QPushButton(tr("Set column for concentration"), this);
   pb_set_conc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_set_conc->setMinimumHeight(minHeight1);
   pb_set_conc->setPalette( PALET_PUSHB );
   connect(pb_set_conc, SIGNAL(clicked()), SLOT(set_conc()));

   lbl_conc = new QLabel("", this);
   lbl_conc->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_conc->setMinimumHeight(minHeight1);
   lbl_conc->setPalette( PALET_LABEL );
   AUTFBACK( lbl_conc );
   lbl_conc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   pb_trial = new QPushButton(tr("Trial set"), this);
   pb_trial->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_trial->setMinimumHeight(minHeight1);
   pb_trial->setPalette( PALET_PUSHB );
   connect(pb_trial, SIGNAL(clicked()), SLOT(trial()));

   lbl_trial = new QLabel("", this);
   lbl_trial->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_trial->setMinimumHeight(minHeight1);
   lbl_trial->setPalette( PALET_LABEL );
   AUTFBACK( lbl_trial );
   lbl_trial->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   pb_cancel = new QPushButton(tr("Cancel"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_set_ok = new QPushButton(tr("Save values"), this);
   pb_set_ok->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_set_ok->setMinimumHeight(minHeight1);
   pb_set_ok->setPalette( PALET_PUSHB );
   connect(pb_set_ok, SIGNAL(clicked()), SLOT(set_ok()));

   // build layout
   Q3HBoxLayout *hbl_name = new Q3HBoxLayout;
   hbl_name->addSpacing( 4 );
   hbl_name->addWidget ( pb_set_name );
   hbl_name->addSpacing( 4 );
   hbl_name->addWidget ( lbl_name );
   hbl_name->addSpacing( 4 );

   Q3HBoxLayout *hbl_conc = new Q3HBoxLayout;
   hbl_conc->addSpacing( 4 );
   hbl_conc->addWidget ( pb_set_conc );
   hbl_conc->addSpacing( 4 );
   hbl_conc->addWidget ( lbl_conc );
   hbl_conc->addSpacing( 4 );

   Q3HBoxLayout *hbl_trial = new Q3HBoxLayout;
   hbl_trial->addSpacing( 4 );
   hbl_trial->addWidget ( pb_trial );
   hbl_trial->addSpacing( 4 );
   hbl_trial->addWidget ( lbl_trial );
   hbl_trial->addSpacing( 4 );

   Q3HBoxLayout *hbl_bottom = new Q3HBoxLayout;
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_cancel );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_set_ok );
   hbl_bottom->addSpacing( 4 );

   Q3VBoxLayout *background = new Q3VBoxLayout( this );
   background->addSpacing( 4 );
   background->addWidget ( lbl_title );
   background->addSpacing( 4 );
   background->addWidget ( t_csv );
   background->addSpacing( 4 );
   background->addWidget ( pb_del_row );
   background->addSpacing( 4 );
   background->addLayout ( hbl_name );
   background->addSpacing( 4 );
   background->addWidget ( pb_adjust );
   background->addSpacing( 4 );
   background->addLayout ( hbl_conc );
   background->addSpacing( 4 );
   background->addLayout ( hbl_trial );
   background->addSpacing( 4 );
   background->addLayout ( hbl_bottom );
   background->addSpacing( 4 );
}

void US_Hydrodyn_Saxs_Buffer_Conc_Load::cancel()
{
   close();
}

void US_Hydrodyn_Saxs_Buffer_Conc_Load::set_ok()
{
   // set *csv1
   trial();
   *csv1 = trial_csv;
   close();
}

void US_Hydrodyn_Saxs_Buffer_Conc_Load::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_saxs_buffer_conc_load.html");
}

void US_Hydrodyn_Saxs_Buffer_Conc_Load::closeEvent(QCloseEvent *e)
{
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

csv US_Hydrodyn_Saxs_Buffer_Conc_Load::current_csv()
{
   csv tmp_csv = disp_csv;
   
   for ( unsigned int i = 0; i < (unsigned int) disp_csv.data.size(); i++ )
   {
      for ( unsigned int j = 0; j < (unsigned int) disp_csv.data[i].size(); j++ )
      {
         tmp_csv.data[i][j] = t_csv->text( i, j );
      }
   }
   return tmp_csv;
}

void US_Hydrodyn_Saxs_Buffer_Conc_Load::update_enables()
{
   if ( !disable_updates )
   {
      disable_updates = true;
      unsigned int selected_rows = 0;
      unsigned int selected_cols = 0;
      for ( int i = 0; i < t_csv->numRows(); i++ )
      {
         if ( t_csv->isRowSelected( i ) )
         {
            selected_rows++;
         }
      }

      for ( int i = 0; i < t_csv->numCols(); i++ )
      {
         if ( t_csv->isColumnSelected( i ) )
         {
            selected_cols++;
         }
      }

      pb_del_row   ->setEnabled( selected_rows );
      pb_set_name  ->setEnabled( selected_cols == 1 );
      pb_adjust    ->setEnabled( !lbl_name->text().isEmpty() );
      pb_set_conc  ->setEnabled( selected_cols == 1 );
      pb_set_ok    ->setEnabled( !lbl_name->text().isEmpty() &&
                                 !lbl_conc->text().isEmpty() &&
                                 lbl_trial->text().contains( tr( "Match ok" ) ) );
      pb_trial     ->setEnabled( !lbl_name->text().isEmpty() && 
                                 !lbl_conc->text().isEmpty() );

      disable_updates = false;
   }
}

void US_Hydrodyn_Saxs_Buffer_Conc_Load::del_row()
{
   csv csv_new = current_csv();
   csv_new.header.clear();
   csv_new.data.clear();
   csv_new.num_data.clear();
   csv_new.prepended_names.clear();
   
   for ( int i = 0; i < t_csv->numRows(); i++ )
   {
      if ( !t_csv->isRowSelected( i ) )
      {
         csv_new.data.push_back( disp_csv.data[ i ] );
         if ( csv_new.header.size() < disp_csv.data[ i ].size() )
         {
            csv_new.header.resize( disp_csv.data[ i ].size() );
         }
      }
   }
   t_csv->clearSelection();
   disp_csv = csv_new;
   reload_csv();

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

   update_enables();
}

void US_Hydrodyn_Saxs_Buffer_Conc_Load::set_name()
{
   for ( int i = 0; i < t_csv->numCols(); i++ )
   {
      if ( t_csv->isColumnSelected( i ) )
      {
         lbl_name->setText( QString( "%1" ).arg( i + 1 ) );
      }
   }
   t_csv->clearSelection();
   update_enables();
}

void US_Hydrodyn_Saxs_Buffer_Conc_Load::set_conc()
{
   for ( int i = 0; i < t_csv->numCols(); i++ )
   {
      if ( t_csv->isColumnSelected( i ) )
      {
         lbl_conc->setText( QString( "%1" ).arg( i + 1 ) );
      }
   }
   t_csv->clearSelection();
   update_enables();
}

QString US_Hydrodyn_Saxs_Buffer_Conc_Load::csv_to_qstring( csv from_csv )
{
   QString qs;

   qs += QString( "# header size %1\n" ).arg( from_csv.header.size() );
   qs += QString( "# data size %1\n" ).arg( from_csv.data.size() );
   
   for ( unsigned int i = 0; i < (unsigned int) from_csv.data.size(); i++ )
   {
      for ( unsigned int j = 0; j < (unsigned int) from_csv.data[i].size(); j++ )
      {
         qs += QString("%1%2").arg(j ? "," : "").arg(from_csv.data[i][j]);
      }
      qs += "\n";
   }
   
   return qs;
}

void US_Hydrodyn_Saxs_Buffer_Conc_Load::reload_csv()
{
   // cout << csv_to_qstring( disp_csv );
   t_csv->setNumRows( disp_csv.data.size() );
   t_csv->setNumCols( disp_csv.header.size() );
   for ( unsigned int i = 0; i < (unsigned int) disp_csv.data.size(); i++ )
   {
      for ( unsigned int j = 0; j < (unsigned int) disp_csv.data[ i ].size(); j++ )
      {
         t_csv->setText( i, j, disp_csv.data[ i ][ j ]);
      }
   }
}

void US_Hydrodyn_Saxs_Buffer_Conc_Load::trial()
{
   // go through original csv & try to find matches
   map < QString, QString > concs;

   for ( unsigned int i = 0; i < (unsigned int) csv1->data.size(); i++ )
   {
      if ( csv1->data[ i ].size() > 1 )
      {
         concs[ csv1->data[ i ][ 0 ] ] =  csv1->data[ i ][ 1 ];
      }
   }


   csv tmp_csv = current_csv();

   map < QString, QString > loaded_concs;

   for ( unsigned int i = 0; i < (unsigned int) tmp_csv.data.size(); i++ )
   {
      if ( tmp_csv.data[ i ].size() > lbl_name->text().toUInt() - 1 &&
           tmp_csv.data[ i ].size() > lbl_conc->text().toUInt() - 1 )
      {
         loaded_concs[ tmp_csv.data[ i ][ lbl_name->text().toUInt() - 1 ] ] =
            tmp_csv.data[ i ][ lbl_conc->text().toUInt() - 1 ];
      }
   }

   // now, see if we can figure out the time sequence codes
   // how many numeric fields are present and collect them all up

   QRegExp rx_nondigit( "\\D+" );

   map < QString, QString > new_concs;

   for ( map < QString, QString >::iterator it = concs.begin();
         it != concs.end();
         it++ )
   {
      QStringList qsl = QStringList::split( rx_nondigit, it->first );
      // cout << QString( "ref <%1>  qsl: <%2>\n" )
      // .arg( it->first )
      // .arg( qsl.join( ":" ) );
      for ( int i = (int) qsl.size() - 1; i >= 0; i-- )
      {
         if ( loaded_concs.count( qsl[ i ] ) )
         {
            new_concs[ it->first ] = loaded_concs[ qsl[ i ] ];
            break;
         }
      }
   }
   
   unsigned int found     = new_concs.size();
   unsigned int not_found = concs.size() - new_concs.size();

   trial_csv = *csv1;

   for ( unsigned int i = 0; i < (unsigned int) trial_csv.data.size(); i++ )
   {
      if ( trial_csv.data[ i ].size() > 1 &&
           new_concs.count( trial_csv.data[ i ][ 0 ] ) )
      {
         trial_csv.data[ i ][ 1 ] = new_concs[ trial_csv.data[ i ][ 0 ] ];
      }
   }
   lbl_trial->setText( ( found > 0 ?
                         tr( "Match ok" ) : tr( "No matches found" ) ) +
                       QString( tr( " %1 files matched, %2 not matched" ).arg( found ).arg( not_found ) ) );
   update_enables();
}

void US_Hydrodyn_Saxs_Buffer_Conc_Load::adjust()
{
   // ask & add or subtract
   bool ok;
   int res = QInputDialog::getInteger(
                                      tr( "US-SOMO: SAXS Buffer: File Concentration Loader: Adjust timecode" ),
                                      tr( "Enter a timecode correction:" ), 
                                      0, 
                                      -1000, 
                                      +1000,
                                      1,
                                      &ok, 
                                      this 
                                      );
   if ( ok && res ) 
   {
      // user entered something and pressed OK
      for ( int i = 0; i < t_csv->numRows(); i++ )
      {
         t_csv->setText( i, 0, 
                         QString( "%1" ).
                         arg( t_csv->text( i, 0 ).toInt() + res ) );
      }
   }
}

void US_Hydrodyn_Saxs_Buffer_Conc_Load::col_header_released( int col )
{
   t_csv->clearSelection();
   Q3TableSelection qts( 0, col, t_csv->numRows(), col );
   
   t_csv->addSelection( qts );
   update_enables();
}
