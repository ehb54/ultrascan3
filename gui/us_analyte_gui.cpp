//! \file us_analyte_gui.cpp
#include "us_analyte_gui.h"
#include "us_passwd.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_investigator.h"
#include "us_editor_gui.h"
#include "us_table.h"
#include "us_util.h"
#include "us_db2.h"
#include "us_constants.h"
#include "us_editor.h"
#include "us_math2.h"
#include "us_eprofile.h"

#include "us_new_spectrum.h"
#include "us_edit_spectrum.h"

#if QT_VERSION < 0x050000
#define setSymbol(a)      setSymbol(*a)
#define setSamples(a,b,c) setData(a,b,c)
#endif

/*  Class US_SequenceEditor */
US_SequenceEditor::US_SequenceEditor( const QString& sequence ) 
   : US_WidgetsDialog( 0, 0 )
{
   setWindowTitle( tr( "Sequence Management" ) );
   setPalette( US_GuiSettings::frameColor() );
   setAttribute( Qt::WA_DeleteOnClose );

   QGridLayout* main = new QGridLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   QFont font = QFont( "monospace", US_GuiSettings::fontSize() );
   edit = new US_Editor( US_Editor::LOAD, false );
   edit->e->setAcceptRichText( false );
   edit->e->setText( sequence );
   edit->e->setFont( font );

   QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
   QPushButton* pb_accept = us_pushbutton( tr( "Accept" ) );

   main->addWidget( edit,      0, 0, 5, 2 );
   main->addWidget( pb_cancel, 5, 0 );
   main->addWidget( pb_accept, 5, 1 );

   connect( pb_cancel, SIGNAL( clicked() ), SLOT( close() ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( accept() ) );

   QFontMetrics fm( font );
   resize( fm.width( 'W' ) * 80, fm.height() * 20 );
}

void US_SequenceEditor::accept( void ) 
{
   emit sequenceChanged( edit->e->toPlainText() );
   close();
}

//! Tab for selection of an analyte
US_AnalyteMgrSelect::US_AnalyteMgrSelect( int *invID, int *select_db_disk,
      US_Analyte *tmp_analyte ) : US_Widgets()
{
   analyte    = tmp_analyte;
   personID   = invID;
   db_or_disk = select_db_disk;
   from_db    = ( (*db_or_disk) == 1 );
   dbg_level  = US_Settings::us_debug();

   setPalette( US_GuiSettings::frameColor() );
   QGridLayout* main = new QGridLayout( this );
   main->setSpacing( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );

   pb_cancel   = us_pushbutton( tr( "Cancel" ) );
   pb_accept   = us_pushbutton( tr( "Accept" ) );
   pb_spectrum = us_pushbutton( tr( "View Spectrum" ) );
   pb_sequence = us_pushbutton( tr( "View Sequence" ) );
   pb_delete   = us_pushbutton( tr( "Delete Analyte" ) );
   pb_info     = us_pushbutton( tr( "Analyte Details" ) );
   pb_help     = us_pushbutton( tr( "Help" ) );

   QGridLayout* protein = us_radiobutton( tr( "Protein" ), rb_protein, true );
   QGridLayout* dna     = us_radiobutton( tr( "DNA"     ), rb_dna   );
   QGridLayout* rna     = us_radiobutton( tr( "RNA"     ), rb_rna   );
   QGridLayout* carbo   = us_radiobutton(
                               tr( "Carbohydrate/Other" ), rb_carbo );
   QHBoxLayout* radios  = new QHBoxLayout;
   radios->addLayout( protein );
   radios->addLayout( dna     );
   radios->addLayout( rna     );
   radios->addLayout( carbo   );
   QButtonGroup* typeButtons = new QButtonGroup( this );
   typeButtons->addButton( rb_protein, US_Analyte::PROTEIN      );
   typeButtons->addButton( rb_dna,     US_Analyte::DNA          );
   typeButtons->addButton( rb_rna,     US_Analyte::RNA          );
   typeButtons->addButton( rb_carbo,   US_Analyte::CARBOHYDRATE );

   QLabel* bn_select    = us_banner( tr( "Select an analyte to use" ) );
   bn_calcmw            = us_banner( tr( "Calculate Molecular Weight" ) );
   bn_ratnuc            = us_banner( tr( "Counterion molar ratio/nucleotide" ) );

   lb_search            = us_label( tr( "Search:" ) );
   lb_molecwt           = us_label( tr( "MW (<small>Daltons</small>):" ) );
   lb_vbar20            = us_label( tr( "VBar (<small>cm<sup>3</sup>/g at 20" )
                                    + DEGC + "</small>):" );
   lb_residue           = us_label( tr( "Residue count:" ) );
   lb_e280              = us_label( tr( "E280 ( <small>OD/(mol*cm)</small> ):" ) );
   lb_strand            = us_label( tr( "Strand:" ) );
   lb_3prime            = us_label( tr( "Three prime:" ) );
   lb_5prime            = us_label( tr( "Five prime:" ) );
   lb_sodium            = us_label( tr( "Sodium, Na+" ) );
   lb_potassium         = us_label( tr( "Potassium, K+" ) );
   lb_lithium           = us_label( tr( "Lithium, Li+" ) );
   lb_magnesium         = us_label( tr( "Magnesium, Mg+" ) );
   lb_calcium           = us_label( tr( "Calcium, Ca+" ) );

   QGridLayout* gfbox   = us_checkbox( tr( "Gradient-Forming" ), ck_gradform );

   le_search            = us_lineedit();
   le_molecwt           = us_lineedit( "", -1, true );
   le_vbar20            = us_lineedit( "", -1, true );
   le_residue           = us_lineedit( "", -1, true );
   le_e280              = us_lineedit( "", -1, true );
   le_strand            = us_lineedit( tr( "Double Stranded" ), -1, true );
   le_3prime            = us_lineedit( tr( "Hydroxyl" ),        -1, true );
   le_5prime            = us_lineedit( tr( "Phospate" ),        -1, true );
   le_sodium            = us_lineedit( "1.00", -1, true );
   le_potassium         = us_lineedit( "0.00", -1, true );
   le_lithium           = us_lineedit( "0.34", -1, true );
   le_magnesium         = us_lineedit( "0.00", -1, true );
   le_calcium           = us_lineedit( "0.00", -1, true );

   lw_analyte_list      = us_listwidget();
   lw_analyte_list->setSelectionMode( QAbstractItemView::ExtendedSelection );
   te_analyte_smry      = us_textedit();
   te_analyte_smry->setTextColor( Qt::blue );
   te_analyte_smry->setFont( QFont( US_Widgets::fixedFont().family(),
                                    US_GuiSettings::fontSize() ) );
   us_setReadOnly( te_analyte_smry, true );

   pb_accept  ->setEnabled( false );
   pb_spectrum->setEnabled( false );
   pb_sequence->setEnabled( false );
   pb_info    ->setEnabled( false );
   pb_delete  ->setEnabled( false );
   bn_select  ->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   bn_calcmw  ->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   bn_ratnuc  ->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );

   int row = 0;
   main->addWidget( bn_select,       row++,  0, 1, 12 );
   main->addLayout( radios,          row++,  0, 1, 12 );
   main->addWidget( lb_search,       row,    0, 1,  1 );
   main->addWidget( le_search,       row,    1, 1,  5 );
   main->addWidget( pb_help,         row,    6, 1,  2 );
   main->addWidget( pb_cancel,       row,    8, 1,  2 );
   main->addWidget( pb_accept,       row++, 10, 1,  2 );
   main->addWidget( lw_analyte_list, row,    0, 8,  6 );
   main->addWidget( pb_spectrum,     row,    6, 1,  3 );
   main->addWidget( pb_sequence,     row++,  9, 1,  3 );
   main->addWidget( pb_info,         row,    6, 1,  3 );
   main->addWidget( pb_delete,       row++,  9, 1,  3 );
   main->addWidget( te_analyte_smry, row,    6, 6,  6 );
   row += 6;
   main->addWidget( bn_calcmw,       row,    0, 1,  4 );
   main->addWidget( bn_ratnuc,       row++,  4, 1,  8 );
   main->addWidget( lb_strand,       row,    0, 1,  2 );
   main->addWidget( le_strand,       row,    2, 1,  2 );
   main->addWidget( lb_sodium,       row,    4, 1,  3 );
   main->addWidget( le_sodium,       row,    7, 1,  1 );
   main->addWidget( lb_potassium,    row,    8, 1,  3 );
   main->addWidget( le_potassium,    row++, 11, 1,  1 );
   main->addWidget( lb_3prime,       row,    0, 1,  2 );
   main->addWidget( le_3prime,       row,    2, 1,  2 );
   main->addWidget( lb_lithium,      row,    4, 1,  3 );
   main->addWidget( le_lithium,      row,    7, 1,  1 );
   main->addWidget( lb_magnesium,    row,    8, 1,  3 );
   main->addWidget( le_magnesium,    row++, 11, 1,  1 );
   main->addWidget( lb_5prime,       row,    0, 1,  2 );
   main->addWidget( le_5prime,       row,    2, 1,  2 );
   main->addWidget( lb_calcium,      row,    4, 1,  3 );
   main->addWidget( le_calcium,      row++,  7, 1,  1 );
   main->addLayout( gfbox,           row++,  0, 1,  6 );
   main->addWidget( lb_molecwt,      row,    0, 1,  4 );
   main->addWidget( le_molecwt,      row,    4, 1,  2 );
   main->addWidget( lb_vbar20,       row,    6, 1,  4 );
   main->addWidget( le_vbar20,       row++, 10, 1,  2 );
   main->addWidget( lb_residue,      row,    0, 1,  4 );
   main->addWidget( le_residue,      row,    4, 1,  2 );
   main->addWidget( lb_e280,         row,    6, 1,  4 );
   main->addWidget( le_e280,         row++, 10, 1,  2 );
   
   for ( int ii = 0; ii < 12; ii++ )  main->setColumnStretch( ii, 99 );
   for ( int ii = 0; ii < 12; ii++ )  main->setRowStretch( ii, 99 );

   connect( typeButtons,     SIGNAL( buttonClicked   ( int )       ),
            this,            SLOT  ( set_analyte_type( int )       ) );
   connect( le_search,       SIGNAL( textChanged( const QString& ) ),
            this,            SLOT  ( search     ( const QString& ) ) );
   connect( pb_cancel,       SIGNAL( clicked() ),
            this,            SLOT  ( reject()  ) );
   connect( pb_accept,       SIGNAL( clicked()        ),
            this,            SLOT  ( accept_analyte() ) );
   connect( pb_info,         SIGNAL( clicked()        ),
            this,            SLOT  ( info_analyte()   ) );
   connect( pb_help,         SIGNAL( clicked() ),
            this,            SLOT  ( help()    ) );
   connect( lw_analyte_list, SIGNAL( itemSelectionChanged() ),
            this,            SLOT  ( select_analyte()        ) );
   connect( pb_spectrum,     SIGNAL( clicked()  ),
            this,            SLOT  ( spectrum() ) );
   connect( pb_sequence,     SIGNAL( clicked()  ),
            this,            SLOT  ( sequence() ) );
   connect( pb_delete,       SIGNAL( clicked()        ),
            this,            SLOT  ( delete_analyte() ) );

   init_analyte();
   set_analyte_type( 0 );
}

void US_AnalyteMgrSelect::select_analyte()
{
   if ( lw_analyte_list->currentRow() < 0 )
   {
DbgLv(1) << "AnaS:  selana: ROW<0";
      reset();
      return;
   }

DbgLv(1) << "AnaS:  selana: ROW" << lw_analyte_list->currentRow();
   QList< QListWidgetItem* > selItems  = lw_analyte_list->selectedItems();
   int nsel  = selItems.count();
   ms_analytes.clear();
DbgLv(1) << "AnaS:  aAcc : nsel" << nsel;

   if ( nsel == 1 )
   {
      QListWidgetItem *item = lw_analyte_list->currentItem();

      select_analyte( item );

      ms_analytes << *analyte;
   }
   else if ( nsel > 1 )
   {

      for ( int ii = 0; ii < nsel; ii++ )
      {
DbgLv(1) << "AnaS:  aAcc :   ii" << ii;
         QListWidgetItem* item = selItems[ ii ];

         select_analyte( item );

         ms_analytes << *analyte;
      }
   }
}

// Select the analyte for the currently selected list item
void US_AnalyteMgrSelect::select_analyte( QListWidgetItem *tmp_item )
{
   QListWidgetItem *item = tmp_item;
   if ( from_db )  read_from_db  ( item );
   else            read_from_disk( item );
DbgLv(1) << "AnaS:   ana-descr " << analyte->description;
DbgLv(1) << "AnaS:   ana-type  " << analyte->type;
DbgLv(1) << "AnaS:   ana-mw    " << analyte->mw;
DbgLv(1) << "AnaS:   ana-vbar20" << analyte->vbar20;
DbgLv(1) << "AnaS:   ana-seqsiz" << analyte->sequence.size();
DbgLv(1) << "AnaS:   ana-extsiz" << analyte->extinction.keys().size();

   // Write values to screen
   US_Math2::Peptide pp;
   double tempera       = 20.0;
   US_Math2::calc_vbar( pp, analyte->sequence, tempera );
   analyte->mw          = ( analyte->mw == 0.0 ) ? pp.mw : analyte->mw;
   int rescount         = pp.residues;
   int extcount         = analyte->extinction.keys().count();

   qDebug() << "Select ANALYTE 1:: -- extcount, extMAP -- " << extcount << analyte->extinction;
   qDebug() << "Select ANALYTE: type 1::  -- " << analyte->type;

   //QString e280         = ( extcount == 0 ) ? tr( "(none)" ) : QString::number( pp.e280 );
   QString e280         = "";
   
   if ( extcount == 0 ) {
     //e280 = tr( "(none)" );
     e280 = QString::number( pp.e280 );
     
     //ALEXEY: should we skip this for non-PROTEINS ??
     // It uses incorrect logic for proteins in ::calc_vbar() when DNA/RNA contains Cytosin (mistakes for Cysteine)
     if ( analyte->type == US_Analyte::PROTEIN )
       {
	 analyte->extinction[ 280.0 ] = pp.e280;
	 
	 qDebug() << "Select ANALYTE:: -- e280: " << pp.e280;
       }
     
   }
   else
     {
       //ALEXEY: add this ONLY for proteins
       if ( analyte->type == US_Analyte::PROTEIN )
	 e280 = QString::number( analyte->extinction[ 280.0 ] );
     }

   qDebug() << "Select ANALYTE 2:: -- extcount, extMAP -- " << extcount << analyte->extinction;
   qDebug() << "Select ANALYTE: type 2::  -- " << analyte->type;
   
   le_molecwt   ->setText( QString::number( analyte->mw ) );
   le_vbar20    ->setText( QString::number( analyte->vbar20 ) );
   le_residue   ->setText( QString::number( rescount ) );
   le_e280      ->setText( e280 );

   if ( analyte->type == US_Analyte::CARBOHYDRATE )
   {
      ck_gradform->setChecked( analyte->grad_form );
   }

   else if ( analyte->type != US_Analyte::PROTEIN )
     //if ( analyte->type == US_Analyte::DNA || analyte->type == US_Analyte::RNA ) //ALEXEY: bug fixed
   {
      QString strand    = "";
      strand            = analyte->doubleStranded ? tr( "Double Stranded" ) : strand;
      strand            = analyte->complement     ? tr( "Complement Only" ) : strand;
      QString _3prime   = analyte->_3prime ? tr( "Hydroxyl" ) : tr( "Phospate" );
      QString _5prime   = analyte->_5prime ? tr( "Hydroxyl" ) : tr( "Phospate" );

      le_strand   ->setText( strand  );
      le_3prime   ->setText( _3prime );
      le_5prime   ->setText( _5prime );
      le_sodium   ->setText( QString::number( analyte->sodium    ) );
      le_potassium->setText( QString::number( analyte->potassium ) );
      le_lithium  ->setText( QString::number( analyte->lithium   ) );
      le_magnesium->setText( QString::number( analyte->magnesium ) );
      le_calcium  ->setText( QString::number( analyte->calcium   ) );

      //ALEXEY: shouldn't we reset analyte->extinction[ 280.0 ] && entire analyte_extinciton to 0 / empty .clear()?;

      qDebug() << "Select ANALYTE 3:: -- extcount, extMAP -- " << extcount << analyte->extinction;

   }

   qDebug() << "Select ANALYTE 4:: -- extcount, extMAP -- " << extcount << analyte->extinction;
   qDebug() << "Analyte->type: " << analyte->type;
   te_analyte_smry->setText( analyte_smry( analyte ) );
   qDebug() << "Select ANALYTE 5:: -- extcount, extMAP -- " << extcount << analyte->extinction;
   
   pb_accept  ->setEnabled ( true );
   pb_delete  ->setEnabled ( true );
   pb_info    ->setEnabled ( true );
   pb_spectrum->setEnabled ( true );
   pb_sequence->setEnabled ( true );
}

// Read analyte information from the local disk
void US_AnalyteMgrSelect::read_from_disk( QListWidgetItem* item )
{
   int row = lw_analyte_list->row( item );
   int ana = analyte_metadata[ row ].index;
DbgLv(1) << "row: " << row << " ana: "  << ana;
   analyte->extinction.clear();

   QString analyGUID = analyte_metadata[ row ].guid;
DbgLv(1) << "AnaS-rddk-item  row" << row << "anaGUID" << analyGUID;
   analyte->load( false, analyGUID, NULL );
}

// Read analyte information from the database (by analyte list item)
void US_AnalyteMgrSelect::read_from_db( QListWidgetItem* item )
{
   int row           = lw_analyte_list->row( item );
   QString analyGUID = analyte_metadata[ row ].guid;
DbgLv(1) << "AnaS-rddb-item  row" << row << "anaGUID" << analyGUID;
   read_from_db( analyGUID );
}

// Read analyte information from the database (by analyte GUID)
void US_AnalyteMgrSelect::read_from_db( const QString& analyGUID )
{
DbgLv(1) << "agS: rddb: anaID" << analyGUID;
   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );

   // Get the analyte data from the database
   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   analyte->extinction.clear();
DbgLv(1) << "AnaS-rddb-anid  anaGUID" << analyGUID;
   analyte->load( true, analyGUID, &db );
}

// Accept the currently selected analyte
void US_AnalyteMgrSelect::accept_analyte( void )
{
   int nsel   = ms_analytes.count();
DbgLv(1) << "AnaS: aa:  nsel" << nsel;
   if ( nsel == 1 )
   {
      emit analyteAccepted();
   }
   else if ( nsel > 1 )
   {
      emit analytesAccepted( ms_analytes );
   }
}

// Initialize analyte information, often after re-entry to the Select tab
void US_AnalyteMgrSelect::init_analyte( void )
{
DbgLv(1) << "agS: init_a";
   from_db       = ( (*db_or_disk) == 1 );
DbgLv(1) << "agS: init_a  from_db" << from_db << "dkordk" << *db_or_disk;
   QString aguid = analyte->analyteGUID;
   QString anaid = analyte->analyteID;
   int idAna     = anaid.toInt();
DbgLv(1) << "agS-initb: db_or_disk" << *db_or_disk << "from_db" << from_db;
DbgLv(1) << "agS-initb:   anaID anaGUID" << anaid << aguid << idAna;

   if ( ( from_db  &&  idAna < 0 )  ||
        ( !from_db  &&  aguid.isEmpty() ) )
   {
      lw_analyte_list->setCurrentRow( -1 );
      idAna         = -1;
      anaid         = "";
      aguid         = "";
   }

   query();
DbgLv(1) << "agS-initb:   post-q:anaID anaGUID" << anaid << aguid;

   if ( ! aguid.isEmpty() )
   {  // There is a selected analyte, select a list item
      if ( from_db ) // DB access
      {
         // Search for analyteID
         for ( int ii = 0; ii < analyte_metadata.size(); ii++ )
         {
            if ( anaid == analyte_metadata[ ii ].analyteID )
            {
DbgLv(1) << "agS-initb:  ii" << ii << "match anaID"<< analyte->analyteID;
               lw_analyte_list->setCurrentRow( ii );
               QListWidgetItem* item = lw_analyte_list->item( ii );
               select_analyte( item );
               break;
            }
         }
      }

      else            // Disk access
      {
         // Search for GUID
         for ( int ii = 0; ii < analyte_metadata.size(); ii++ )
         {
            if ( aguid == analyte_metadata[ ii ].guid )
            {
               lw_analyte_list->setCurrentRow( ii );
               QListWidgetItem* item = lw_analyte_list->item( ii );
               select_analyte( item );
               break;
            }
         }
      }
   }
   else
   {  // There is no selected analyte, de-select a list item
      lw_analyte_list->setCurrentRow( -1 );

      reset();
   }
}

// If called with string initialized, this function
//  will search analytes matching search string
void US_AnalyteMgrSelect::search( QString const& text )
{
DbgLv(1) << "agS: search : text" << text;
 
   //ALEXEY: modified separator to something random which is unlikely to be encountered in analyte description:
   // caused problem with GUI listing...
   QString sep = ";:;:m6rr";
   QStringList sortdesc;

   lw_analyte_list->clear();
   analyte_metadata.clear();
   sortdesc        .clear();

   int dsize   = descriptions.size();    

   for ( int ii = 0; ii < dsize; ii++ )
   {  // get list of filtered-description + index strings
      if ( descriptions[ ii ].contains(
         QRegExp( ".*" + text + ".*", Qt::CaseInsensitive ) )  &&
         ! descriptions[ ii].isEmpty() )
      {
         sortdesc << descriptions[ ii ] + sep + QString::number( ii );
      }
   }

   // sort the descriptions
   sortdesc.sort();
DbgLv(1) << "agS-search:  descsize" << dsize
 << "sortsize" << sortdesc.size();

   for ( int jj = 0; jj < sortdesc.size(); jj++ )
   {  // build list of sorted meta data and ListWidget entries
      int ii      = sortdesc[ jj ].section( sep, 1, 1 ).toInt();

      if ( ii < 0  ||  ii >= dsize )
      {
DbgLv(1) << "agS-search:  *ERROR* ii" << ii << "jj" << jj
 << "sdesc" << sortdesc[jj].section(sep,0,0);
         continue;
      }
      AnalyteInfo info;
      info.index       = ii;
      info.description = descriptions[ ii ];
      info.guid        = GUIDs       [ ii ];
      info.analyteID   = analyteIDs  [ ii ];

      analyte_metadata << info;

      lw_analyte_list->addItem( info.description );
   }
}

// Verify path to local analyte files and create directory if need be
bool US_AnalyteMgrSelect::analyte_path( QString& path )
{
   QDir dir;
   path = US_Settings::dataDir() + "/analytes";

   if ( ! dir.exists( path ) )
   {
      if ( ! dir.mkpath( path ) )
      {
         QMessageBox::critical( this,
            tr( "Bad Analyte Path" ),
            tr( "Could not create default directory for analytes\n" ) + path );
         return false;
      }
   }

   return true;
}

// Display analyte sequence
void US_AnalyteMgrSelect::sequence( void )
{
  QString inf_text;
  QString big_line( "" );
   
  QFont tfont( QFont( US_Widgets::fixedFont().family(),
		      US_GuiSettings::fontSize() -1  ) );
  QFontMetrics fmet( tfont );
  
  QString seqsmry = analyte->sequence;
  
  int nlines = int(seqsmry.count() / 70);
  big_line += QString( 70, ' ' );

  // Build and show the analyte sequence
   int iwid     = fmet.width( big_line ) + 40;
   int ihgt     = fmet.lineSpacing() * qMin( 22, nlines ) + 80;

   US_Editor* ana_info = new US_Editor( US_Editor::DEFAULT, true,
                                        QString(), this );
   ana_info->setWindowTitle( tr( "Selected Analyte Sequence Information" ) );
   ana_info->move( pos() + QPoint( 200, 200 ) );
   ana_info->resize( iwid, ihgt );
   ana_info->e->setFont( tfont );
   if ( seqsmry.count() != 0 )
     {
       ana_info->e->setText( seqsmry );
       ana_info->show();
     }
   else
     {
       QMessageBox::information( this,
				 tr( "WARNING" ),
				 tr( "Analyte does not have sequence!" ) );
     }
}

// Display detailed information on selected analyte
void US_AnalyteMgrSelect::info_analyte( void )
{
DbgLv(1) << "AnalyteID for INFO: " << analyte->analyteGUID;

   US_Math2::Peptide p;
   US_Math2::calc_vbar( p, analyte->sequence, 20.0 );
   analyte->mw         = ( analyte->mw == 0.0 ) ? p.mw : analyte->mw;
       
      // Absorbing residues
   
DbgLv(1) << "W: " << p.w;
   int cys = int(p.c);
   int hao = int(p.j);
   int orn = int(p.o);
   int trp = int(p.w);
   int tyr = int(p.y);
   int all_abs = 0;
   all_abs = cys + hao + orn + trp + tyr;
DbgLv(1) << "Tot AAs: " << all_abs;
   QString absorbing_residues = tr( "(empty)" );
   if (all_abs > 0)
     {
       absorbing_residues = "";
       if ( cys > 0)
	 absorbing_residues += QString().sprintf( "%d", cys ) + " " + "Cys"  + ", ";
       if ( hao > 0)
	 absorbing_residues += QString().sprintf( "%d", hao ) + " " + "Hao"  + ", ";
       if ( orn > 0)
	 absorbing_residues += QString().sprintf( "%d", orn ) + " " + "Orn"  + ", ";
       if ( trp > 0)
	 absorbing_residues += QString().sprintf( "%d", trp ) + " " + "Trp"  + ", ";
       if ( tyr > 0)
	 absorbing_residues += QString().sprintf( "%d", tyr ) + " " + "Tyr"  + ", ";

       absorbing_residues += QString().sprintf( "%d", all_abs ) + " tot";
     }
   

   QStringList lines;
   QString inf_text;
   QString big_line( "" );
   int mxlch    = 0;
   int nspec    = analyte->extinction.keys().count();
   //qDebug() << "#NSPEC:  " << nspec;
   QFont tfont( QFont( US_Widgets::fixedFont().family(),
                       US_GuiSettings::fontSize() - 1 ) );
   QFontMetrics fmet( tfont );
   
   QString atype   = tr( "Other" );
   atype           = analyte->type == ( US_Analyte::PROTEIN )
                                  ? tr( "Protein" ) : atype;
   atype           = analyte->type == ( US_Analyte::DNA ) ? tr( "DNA" ) : atype;
   atype           = analyte->type == ( US_Analyte::RNA ) ? tr( "RNA" ) : atype;

   int seqlen      = analyte->sequence.length();
   QString seqsmry = analyte->sequence;
   int total       = 0;
   if ( seqlen == 0 )
      seqsmry         = tr( "(empty)" );
   else
   {
      seqsmry         = seqsmry.toLower()
                               .remove( QRegExp( "[\\s0-9]" ) );
      seqlen          = seqsmry.length();
      if ( seqlen > 25 )
      {
         seqsmry        = QString( seqsmry ).left( 10 ) + " ... "
                        + QString( seqsmry ).mid( seqlen - 10 );
      }
      seqsmry          += "\n  ";

      for ( int ii = 0; ii < 26; ii++ )
      {
         QString letter  = QString( QChar( 'a' + ii ) );
         int lcount      = analyte->sequence.count( letter );
         total          += lcount;
         if ( lcount > 0 )
         {
            seqsmry     += QString().sprintf( "%d", lcount )
                           + " " + letter.toUpper() + ", ";
	    //seqsmry          += "\n  ";
         }
      }
      seqsmry     += QString().sprintf( "%d", total ) + " tot";
   }

   // Compose analyte information lines
   lines << tr( "Detailed information on the selected Analyte" );
   lines << "";
   lines << tr( "Description:              " ) + analyte->description;
   lines << tr( "Type:                     " ) + atype;
   lines << tr( "Molecular Weight:         " ) + QString::number(
                                                    analyte->mw ) + " D";;
   lines << tr( "Vbar (20 deg. C):         " ) + QString::number(
                                                    analyte->vbar20 );
   lines << tr( "Sequence length:          " ) + QString::number(seqlen);
   lines << tr( "Database ID (-1==HD):     " ) + analyte->analyteID;
   lines << tr( "Global Identifier:        " ) + analyte->analyteGUID;
   //   lines << tr( "Inputting Investigator:   " ) + analyte->person;
   lines << tr( "Spectrum pairs Count:     " ) + QString::number( nspec );
   //lines << tr( "Components Count:         " ) + QString::number( ncomp );
   lines << "";

   // Sequence Composition 
   lines << tr( "Sequence summary:" );
   lines << "  " + seqsmry;
   lines << "  " + tr( "AAs absorbing at 280 nm: " ) +  absorbing_residues;
   lines << "";
   
     
   // Compose the section for any extinction spectrum
   QString stitle  = tr( "Extinction Spectrum:      " );
   QString spline  = stitle;
   QList< double >  keys = analyte->extinction.keys();
   for ( int ii = 0; ii < nspec; ii++ )
   {
      double waveln   = keys[ ii ];
      double extinc   = analyte->extinction[ waveln ];
      
      QString spair;    
      if (extinc < 0)
	spair = QString::number( waveln ) + " /" +
	        QString::number( extinc, 'f', 4) + "  ";
      else 
	spair = QString::number( waveln ) + " / " +
	        QString::number( extinc, 'f', 4) + "  ";
      spline         += spair;

      if ( ( ii % 4 ) == 3  ||  ( ii + 1 ) == nspec )
      {
         lines << spline;
         spline          = stitle;
      }
   }
   if ( nspec > 0 )  lines << "";

   // Create a single text string of info lines
   int nlines   = lines.count();

   for ( int ii = 0; ii < nlines; ii++ )
   {
      QString cur_line = lines[ ii ];
      int nlchr        = cur_line.length();

      if ( nlchr > mxlch )
      {
         mxlch         = nlchr;
         big_line      = cur_line;
      }

      inf_text     += cur_line + "\n";
   }

   // Build and show the analyte details dialog
   int iwid     = fmet.width( big_line ) + 40;
   int ihgt     = fmet.lineSpacing() * qMin( 22, nlines ) + 80;

   US_Editor* ana_info = new US_Editor( US_Editor::DEFAULT, true,
                                        QString(), this );
   ana_info->setWindowTitle( tr( "Detailed Selected Analyte Information" ) );
   ana_info->move( pos() + QPoint( 200, 200 ) );
   ana_info->resize( iwid, ihgt );
   ana_info->e->setFont( tfont );
   ana_info->e->setText( inf_text );
   ana_info->show();
}

// Display a spectrum dialog for list/manage
void US_AnalyteMgrSelect::spectrum( void )
{
DbgLv(1) << "AnaS: spectrum : extinc" << analyte->extinction;
  
  if (analyte->extinction.isEmpty())
    {
      QMessageBox::information( this,
      tr( "WARNING" ),
      tr( "Analyte does not have spectrum data!" ) );
    }
  else
    {
      US_ViewSpectrum *w = new US_ViewSpectrum(analyte->extinction);
      w->setParent(this, Qt::Window);
      w->show();
    }
}


// Remove a selected analyte
void US_AnalyteMgrSelect::delete_analyte( void )
{
#if 0
   if ( analyte->GUID.size() == 0 || lw_analyte->currentRow() < 0 )
   {
      QMessageBox::information( this,
            tr( "Attention" ),
            tr( "First select the analyte to be deleted." ) );
      return;
   }
#endif

   int response = QMessageBox::question( this,
            tr( "Confirmation" ),
            tr( "Delete this analyte?\n"
                "Click 'OK' to proceed..." ),
            QMessageBox::Ok, QMessageBox::Cancel );

   if ( response != QMessageBox::Ok ) return;

   if ( from_db )
      delete_db();
   else
      delete_disk();

   reset();
   query();
}

// Delete an analyte from local disk
void US_AnalyteMgrSelect::delete_disk( void )
{
#if 0
   QString bufGUID = buffer->GUID;
   QString path;
   if ( ! buffer_path( path ) ) return;

   bool    newFile;
   QString filename = US_Analyte::get_filename( path, bufGUID );

   if ( buffer_in_use( bufGUID ) )
   {
      QMessageBox::warning( this,
         tr( "Analyte Not Deleted" ),
         tr( "The buffer could not be deleted,\n"
             "since it is in use in one or more solutions." ) );
      return;
   }

   if ( ! newFile )
   {
      QFile f( filename );
      f.remove();
   }
#endif
}

// Determine if a given analyte GUID is in use
bool US_AnalyteMgrSelect::analyte_in_use( QString& analyteGUID )
{
   bool in_use = false;
   QString soldir = US_Settings::dataDir() + "/solutions/";
   QStringList sfilt( "S*.xml" );
   QStringList snames = QDir( soldir )
      .entryList( sfilt, QDir::Files, QDir::Name );

   for ( int ii = 0;  ii < snames.size(); ii++ )
   {
      QString sfname = soldir + snames.at( ii );
      QFile sfile( sfname );

      if ( ! sfile.open( QIODevice::ReadOnly | QIODevice::Text ) ) continue;

      QXmlStreamReader xml( &sfile );

      while ( ! xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement()  &&  xml.name() == "analyte" )
         {
            QXmlStreamAttributes atts = xml.attributes();

            if ( atts.value( "guid" ).toString() == analyteGUID )
            {
               in_use = true;
               break;
            }
         }
      }

      sfile.close();

      if ( in_use )  break;
   }

   return in_use;
}

// Delete an analyte from the database (if not in use by another table type)
void US_AnalyteMgrSelect::delete_db( void )
{
   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );

   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   QStringList q( "get_analyteID" );
   q << analyte->analyteGUID;

   db.query( q );

   int status = db.lastErrno();

   if (  status == US_DB2::OK )
   {
      db.next();
      QString analyteID = db.value( 0 ).toString();

      q[ 0 ] = "delete_analyte";
      q[ 1 ] = analyteID;
      status = db.statusQuery( q );
   }

   if ( status == US_DB2::ANALY_IN_USE )
   {
      QMessageBox::warning( this,
         tr( "Analyte Not Deleted" ),
         tr( "This analyte could not be deleted since\n"
             "it is in use in one or more solutions." ) );
      return;
   }

   if ( status != US_DB2::OK )
   {
      QMessageBox::warning( this,
         tr( "Attention" ),
         tr( "Delete failed.\n\n" ) + db.lastError() );
   }
}

// Reread all analyte information from DB or local disk
void US_AnalyteMgrSelect::query( void )
{
DbgLv(1) << "agS-query: from_db" << from_db;
   if ( from_db )
   {
      read_db();
   }
   else
   {
      read_analyte();
   }
}

// Read analyte information from local disk
void US_AnalyteMgrSelect::read_analyte( void )
{
   QString path;
   if ( ! analyte_path( path ) ) return;

   QString atype_set  = "PROTEIN";
   atype_set          = rb_dna  ->isChecked() ? "DNA"          : atype_set;
   atype_set          = rb_rna  ->isChecked() ? "RNA"          : atype_set;
   atype_set          = rb_carbo->isChecked() ? "CARBOHYDRATE" : atype_set;

   filenames   .clear();
   descriptions.clear();
   GUIDs       .clear();
   analyteIDs  .clear();
   le_search->  clear();
   le_search->  setReadOnly( true );

   /*
   *   bool desc_set = le_description->text().size() > 0;
   *   pb_save  ->setEnabled( desc_set );
   *   pb_update->setEnabled( desc_set );
   *   pb_del   ->setEnabled( false );
   */

   QDir ff( path );
   QStringList filter( "A*.xml" );
   QStringList f_names = ff.entryList( filter, QDir::Files, QDir::Name );

   for ( int ii = 0; ii < f_names.size(); ii++ )
   {
      QFile a_file( path + "/" + f_names[ ii ] );

      if ( ! a_file.open( QIODevice::ReadOnly | QIODevice::Text) ) continue;

      QXmlStreamReader xml( &a_file );

      while ( ! xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement() )
         {
            if ( xml.name() == "analyte" )
            {
               QXmlStreamAttributes aa = xml.attributes();
               QString atype = aa.value( "type" ).toString();

               if ( atype != atype_set )  continue;

               descriptions << aa.value( "description" ).toString();
               GUIDs        << aa.value( "analyteGUID" ).toString();
               filenames    << path + "/" + f_names[ ii ];
               analyteIDs   << "";
               break;
            }
         }
      }
   }

   lw_analyte_list->clear();

   if ( descriptions.size() == 0 )
      lw_analyte_list->addItem( "No analyte files found." );
   else
   {
      le_search->setReadOnly( false );
      search();
   }
}

// Read analyte information from the database
void US_AnalyteMgrSelect::read_db( void )
{
   US_Passwd pw;
   US_DB2 db( pw.getPasswd() );

DbgLv(1) << "agS-rddb: lastErr" << db.lastError();
DbgLv(1) << "agS-rddb: errNo OK" << db.lastErrno() << US_DB2::OK;
   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   US_Analyte::analyte_t atype_set  = US_Analyte::PROTEIN;
DbgLv(1) << "agS-rddb: ==A1==";
   atype_set      = rb_dna  ->isChecked() ? US_Analyte::DNA          : atype_set;
DbgLv(1) << "agS-rddb: ==A2==";
   atype_set      = rb_rna  ->isChecked() ? US_Analyte::RNA          : atype_set;
DbgLv(1) << "agS-rddb: ==A3==";
   atype_set      = rb_carbo->isChecked() ? US_Analyte::CARBOHYDRATE : atype_set;
DbgLv(1) << "agS-rddb: atype_set" << atype_set;

   analyteIDs  .clear();
DbgLv(1) << "agS-rddb: ==A==";
   descriptions.clear();
DbgLv(1) << "agS-rddb: ==B==";
   GUIDs       .clear();
DbgLv(1) << "agS-rddb: ==C==";
   le_search->  clear();
DbgLv(1) << "agS-rddb: ==D==";
   le_search->  setText( "" );
DbgLv(1) << "agS-rddb: ==E==";
   le_search->  setReadOnly( true );
DbgLv(1) << "agS-rddb: ==F==";

DbgLv(1) << "agS-rddb: personID" << *personID;
   QString IDperson = QString::number( *personID );
   QStringList q;
   q << "get_analyte_desc" << IDperson;
   db.query( q );

   while ( db.next() )
   {
      QString a_type   = db.value( 2 ).toString();
DbgLv(1) << "agS-rddb:   a_type" << a_type;
      US_Analyte::analyte_t current = US_Analyte::PROTEIN;

      if      ( a_type == "Protein" )  current = US_Analyte::PROTEIN;
      else if ( a_type == "DNA"     )  current = US_Analyte::DNA;
      else if ( a_type == "RNA"     )  current = US_Analyte::RNA;
      else if ( a_type == "Other"   )  current = US_Analyte::CARBOHYDRATE;
DbgLv(1) << "agS-rddb:    current atype_set" << current << atype_set;

      if ( current != atype_set )      continue;

      QString anaID    = db.value( 0 ).toString();
      QString descr    = db.value( 1 ).toString();
DbgLv(1) << "agS-rddb:     aID descr" << anaID << descr;
      analyteIDs   << anaID;
      descriptions << descr;
      GUIDs        << "";
   }

   for ( int ii = 0; ii < analyteIDs.count(); ii++ )
   {
      q.clear();
      q << "get_analyte_info" << analyteIDs[ ii ];
      db.query( q );
      db.next();
      GUIDs[ ii ]      = db.value( 0 ).toString();
   }

   lw_analyte_list->clear();

   if ( descriptions.size() == 0 )
   {
      lw_analyte_list->addItem( "No analyte files found." );
   }
   else
   {
      le_search->setReadOnly( false );
      search();
   }

}

// Report a database connection error
void US_AnalyteMgrSelect::connect_error( const QString& error )
{
   QMessageBox::warning( this, tr( "Connection Problem" ),
                        tr( "Could not connect to database \n" ) + error );
}

// Accept a selected analyte for return to the caller
void US_AnalyteMgrSelect::accept( void )
{
}

// Reject the selected analyte and return to caller with no change
void US_AnalyteMgrSelect::reject( void )
{
   emit selectionCanceled();
}

// Reset to no analyte selected
void US_AnalyteMgrSelect::reset()
{
   lw_analyte_list->setCurrentRow( -1 );
   pb_accept  ->setEnabled ( false );
   pb_delete  ->setEnabled ( false );
   pb_info    ->setEnabled ( false );
   pb_spectrum->setEnabled ( false );
   pb_sequence->setEnabled ( false );

   te_analyte_smry->setText( "" );
}

void US_AnalyteMgrSelect::set_analyte_type( int type )
{
DbgLv(1) << "agN: sAtype" << type;
#if 0
   if ( inReset ) return;
#endif

   bool visProt         = true;
   bool visAll          = true;
   bool visDRna         = true;
   bool visCarb         = true;
DbgLv(1) << "agS: sAtype: type" << type;
 
 switch ( type )
   {
      case US_Analyte::PROTEIN:
         visDRna              = false;
         visCarb              = false;
	 break;

      case US_Analyte::DNA:
      case US_Analyte::RNA:
         visProt              = false;
         visCarb              = false;
         break;

      case US_Analyte::CARBOHYDRATE:
         visProt              = false;
         visDRna              = false;
         break;
   }
   bn_calcmw   ->setVisible( visDRna );
   bn_ratnuc   ->setVisible( visDRna );
   lb_search   ->setVisible( visAll  );
   lb_molecwt  ->setVisible( visAll  );
   lb_vbar20   ->setVisible( visAll  );
   lb_residue  ->setVisible( visProt );
   lb_e280     ->setVisible( visProt );
   lb_strand   ->setVisible( visDRna );
   lb_3prime   ->setVisible( visDRna );
   lb_5prime   ->setVisible( visDRna );
   lb_sodium   ->setVisible( visDRna );
   lb_potassium->setVisible( visDRna );
   lb_lithium  ->setVisible( visDRna );
   lb_magnesium->setVisible( visDRna );
   lb_calcium  ->setVisible( visDRna );
   ck_gradform ->setVisible( visCarb );
   le_search   ->setVisible( visAll  ); 
   le_molecwt  ->setVisible( visAll  ); 
   le_vbar20   ->setVisible( visAll  ); 
   le_residue  ->setVisible( visProt ); 
   le_e280     ->setVisible( visProt ); 
   le_strand   ->setVisible( visDRna );
   le_3prime   ->setVisible( visDRna );
   le_5prime   ->setVisible( visDRna );
   le_sodium   ->setVisible( visDRna );
   le_potassium->setVisible( visDRna );
   le_lithium  ->setVisible( visDRna );
   le_magnesium->setVisible( visDRna );
   le_calcium  ->setVisible( visDRna );

#if 0
   US_Analyte a;  // Create a blank analyte
   analyte            = a;
   analyte.type       = (US_Analyte::analyte_t) type;
   saved_analyte.type = analyte.type;
   reset();
   list();
#endif
   reset();
   query();
   search( "" );
}

// Message string for analyte information
QString US_AnalyteMgrSelect::analyte_info( US_Analyte* ana )
{
DbgLv(1) << "agN: aInfo" << ana->description;
   QString amsg = tr( "=== Analyte Information Details ===\n\n" );

   return amsg;
}

// Message string for analyte summary
QString US_AnalyteMgrSelect::analyte_smry( US_Analyte* ana )
{
DbgLv(1) << "AnalyteID for SMRY: " << analyte->analyteGUID;

DbgLv(1) << "agN: aInfo   descr" << ana->description << "type" << ana->type;
   // Get base analyte value strings
   QString atype   = tr( "Other" );
   atype           = ana->type == ( US_Analyte::PROTEIN )
                                  ? tr( "Protein" ) : atype;
   atype           = ana->type == ( US_Analyte::DNA ) ? tr( "DNA" ) : atype;
   atype           = ana->type == ( US_Analyte::RNA ) ? tr( "RNA" ) : atype;
   int seqlen      = ana->sequence.length();
   int extknt      = ana->extinction.keys().count();
   US_Math2::Peptide pp;
   US_Math2::calc_vbar( pp, ana->sequence, 20.0 );
   ana->mw         = ( ana->mw == 0.0 ) ? pp.mw : ana->mw;

   qDebug() << "In Summary 1: analyte->extinciton: " <<  ana->extinction;
     
   // Absorbing residues
   
DbgLv(1) << "W: " << pp.w;
   int cys = int(pp.c);
   int hao = int(pp.j);
   int orn = int(pp.o);
   int trp = int(pp.w);
   int tyr = int(pp.y);
   int all_abs = 0;
   all_abs = cys + hao + orn + trp + tyr;
DbgLv(1) << "Tot AAs: " << all_abs;
   QString absorbing_residues = tr( "(empty)" );
   if (all_abs > 0)
     {
       absorbing_residues = "";
       if ( cys > 0)
	 absorbing_residues += QString().sprintf( "%d", cys ) + " " + "Cys"  + ", ";
       if ( hao > 0)
	 absorbing_residues += QString().sprintf( "%d", hao ) + " " + "Hao"  + ", ";
       if ( orn > 0)
	 absorbing_residues += QString().sprintf( "%d", orn ) + " " + "Orn"  + ", ";
       if ( trp > 0)
	 absorbing_residues += QString().sprintf( "%d", trp ) + " " + "Trp"  + ", ";
       if ( tyr > 0)
	 absorbing_residues += QString().sprintf( "%d", tyr ) + " " + "Tyr"  + ", ";

       absorbing_residues += QString().sprintf( "%d", all_abs ) + " tot";
     }
   
DbgLv(1) << "AA absorbibg String: " << absorbing_residues;

   qDebug() << "In Summary 2: analyte->extinciton: " <<  ana->extinction;
 
   // Compose the sequence summary string
   int total       = 0;
   QString seqsmry = ana->sequence;
   if ( seqlen == 0 )
      seqsmry         = tr( "(empty)" );
   else
   {
      seqsmry         = seqsmry.toLower()
                               .remove( QRegExp( "[\\s0-9]" ) );
      seqlen          = seqsmry.length();
      if ( seqlen > 25 )
      {
         seqsmry        = QString( seqsmry ).left( 10 ) + " ... "
                        + QString( seqsmry ).mid( seqlen - 10 );
      }
      seqsmry          += "\n  ";

      for ( int ii = 0; ii < 26; ii++ )
      {
         QString letter  = QString( QChar( 'a' + ii ) );
         int lcount      = ana->sequence.count( letter );
         total          += lcount;
         if ( lcount > 0 )
         {
            seqsmry     += QString().sprintf( "%d", lcount )
                           + " " + letter.toUpper() + ", ";
         }
      }
      seqsmry     += QString().sprintf( "%d", total ) + " tot";
   }

   qDebug() << "In Summary 3: analyte->extinciton: " <<  ana->extinction;
   
   // Compose the summary string for extinction pairs
   QString extsmry;
   QList< double > ekeys = ana->extinction.keys();
   double lambda;
   if ( extknt == 0 )
      extsmry         = tr( "(empty)" );
   else if ( extknt == 1 )
   {
      lambda          = ekeys[ 0 ];
      extsmry         = QString::number( lambda ) + " / "
                        + QString::number( ana->extinction[ lambda ] );
   }

   qDebug() << "In Summary 4: analyte->extinciton: " <<  ana->extinction;
   
   // Compose the summary message lines
   QStringList mlines;
   QString     amsg;
   mlines << tr( "=== Selected Analyte ===" );
   mlines << "  " + ana->description;
   mlines << tr( "Type:                    " ) + atype;
   mlines << tr( "Molecular Weight:        " ) +  QString::number( ana->mw )
                                        +  " D";
   mlines << tr( "Vbar (20 deg. C):        " ) +  QString::number( ana->vbar20 );
   mlines << tr( "Sequence length:         " ) +  QString::number( seqlen );
   // mlines << tr( "Extinction count:  " ) +  QString::number( extknt );
   mlines << tr( "Sequence summary:" );
   mlines << "  " + seqsmry;
   // mlines << tr( "Extinction " ) + ( ( extknt == 1 ) ? tr( "pair" )
   //                                                   : tr( "pairs" ) )
   //           + tr( "  (lambda/coeff.):" );
   // mlines << "  " + extsmry;

   qDebug() << "In Summary 4a: analyte->extinciton: " <<  ana->extinction;
   //ALEXEY: do not include absorbing residues information for non-protein analytes ??

   qDebug() << "Ana->type :: " << ana->type << analyte->type ;
   if ( ana->type == 0 )
     {
       mlines << tr( "AAs absorbing at 280 nm: " ) +  absorbing_residues;
       
       qDebug() << "In Summary 4b: analyte->extinciton: " <<  ana->extinction;
       
       //mlines << tr( "E280:                    " ) +  QString::number( analyte->extinction[ 280.0 ]);
       //ALEXEY: bug fix for showing analyte's extinciton at 280.0 when it doesn't exist
       if( analyte->extinction.contains( 280.0 ))
	 {
	   mlines << tr( "E280:                    " ) +  QString::number( analyte->extinction[ 280.0 ]);
	 }
       else
	 mlines << tr( "E280:                    " ) +  QString::number( 0 );
       
       qDebug() << "In Summary 4c: analyte->extinciton: " <<  ana->extinction;
       
       mlines << tr( "Extinction count:        " ) +  QString::number( extknt );
     }
   
   for ( int ii = 0; ii < mlines.count(); ii++ )
   {  // Create a single string from the lines
      amsg         += mlines[ ii ] + "\n";
   }

   qDebug() << "In Summary 5: analyte->extinciton: " <<  ana->extinction;
   
   return amsg;
}

// New Analyte panel
US_AnalyteMgrNew::US_AnalyteMgrNew( int *invID, int *select_db_disk,
				    US_Analyte *tmp_analyte, double temperature, bool signal ) : US_Widgets()
{
DbgLv(1) << "agN: id dbdk ana" << invID << select_db_disk << tmp_analyte;
   analyte    = tmp_analyte;
   personID   = invID;
   db_or_disk = select_db_disk;
   from_db    = ( (*db_or_disk) == 1 );
   dbg_level  = US_Settings::us_debug();

   setPalette( US_GuiSettings::frameColor() );
   QGridLayout* main = new QGridLayout( this );
   main->setSpacing( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );

   QGridLayout* protein = us_radiobutton( tr( "Protein" ), rb_protein, true );
   QGridLayout* dna     = us_radiobutton( tr( "DNA"     ), rb_dna   );
   QGridLayout* rna     = us_radiobutton( tr( "RNA"     ), rb_rna   );
   QGridLayout* carbo   = us_radiobutton(
                               tr( "Carbohydrate/Other" ), rb_carbo );
   QHBoxLayout* radios  = new QHBoxLayout;
   radios->addLayout( protein );
   radios->addLayout( dna     );
   radios->addLayout( rna     );
   radios->addLayout( carbo   );
   QButtonGroup* typeButtons = new QButtonGroup( this );
   typeButtons->addButton( rb_protein, US_Analyte::PROTEIN      );
   typeButtons->addButton( rb_dna,     US_Analyte::DNA          );
   typeButtons->addButton( rb_rna,     US_Analyte::RNA          );
   typeButtons->addButton( rb_carbo,   US_Analyte::CARBOHYDRATE );

   QLabel* bn_newanalyte    = us_banner( tr( "Create New Analyte" ) );

   bn_newanalyte  ->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   
   pb_sequence = us_pushbutton( tr( "Enter Sequence" ) );
   pb_spectrum = us_pushbutton( tr( "Manage Spectrum" ) );

   pb_cancel   = us_pushbutton( tr( "Cancel" ) );
   connect( pb_cancel,   SIGNAL( clicked()     ),
            this,        SLOT  ( newCanceled() ) );

   pb_reset   = us_pushbutton( tr( "Reset" ) );
   connect( pb_reset,   SIGNAL( clicked()     ),
            this,        SLOT  ( reset() ) );
   
   //pb_save     = us_pushbutton( tr( "Accept" ), false);
   //connect( pb_save, SIGNAL( clicked() ), SLOT( save() ) );
   
   pb_accept     = us_pushbutton( tr( "Accept" ), false);
   connect( pb_accept,   SIGNAL( clicked()     ),
            this,        SLOT  ( newAccepted() ) );
   
   lb_descrip   = us_label( tr( "Analyte Name:" ) );
   le_descrip   = us_lineedit( "New Analyte", 0, false );
   connect( le_descrip, SIGNAL( editingFinished   () ), 
                            SLOT  ( new_description() ) );

   // Start Protein widget /////////////////////////////////////////////////

    protein_widget = new QWidget( this );
    QGridLayout* protein_info   = new QGridLayout( protein_widget );
    protein_info->setSpacing        ( 2 );
    protein_info->setContentsMargins( 2, 2, 2, 2 );

    int prow = 0;

    QLabel* lb_protein_mw = us_label( tr( "MW <small>(Daltons)</small>:" ) );
    protein_info->addWidget( lb_protein_mw, prow, 0 );

    le_protein_mw = us_lineedit( "0", 0, false );
    protein_info->addWidget( le_protein_mw, prow, 1 );

    QLabel* lb_protein_vbar20 = us_label( 
          tr( "VBar <small>(cm<sup>3</sup>/g at 20" ) + DEGC + ")</small>:" );
    protein_info->addWidget( lb_protein_vbar20, prow, 2 );

    le_protein_vbar20 = us_lineedit(  "0.0000", 0, false );
    connect( le_protein_vbar20, SIGNAL( textChanged  ( const QString& ) ),
                                 SLOT  ( value_changed( const QString& ) ) );
    protein_info->addWidget( le_protein_vbar20, prow++, 3 );

    signal_tmp = signal;
DbgLv(1) << "Temp. in New: " << temperature;
DbgLv(1) << "Signal in New: " << signal_tmp;
   
    QLabel* lb_protein_temp = us_label( 
          tr( "Temperature <small>(" ) + DEGC + ")</small>:" );
    protein_info->addWidget( lb_protein_temp, prow, 0 );

    le_protein_temp = us_lineedit( QString::number( temperature, 'f', 1 ) );
  
    if ( signal_tmp )
    {
       us_setReadOnly( le_protein_temp, true );
    }
    else
    {
       connect( le_protein_temp, SIGNAL( textChanged ( const QString& ) ), 
                                 SLOT  ( temp_changed( const QString& ) ) );
    }

    protein_info->addWidget( le_protein_temp, prow, 1 );

    QLabel* lb_protein_vbar = us_label( 
          tr( "VBar <small>(cm<sup>3</sup>/g at T " ) + DEGC + ")</small>:" );

    protein_info->addWidget( lb_protein_vbar, prow, 2 );

    le_protein_vbar = us_lineedit( "0.0000", 0, true );
    protein_info->addWidget( le_protein_vbar, prow++, 3 );
    
    QLabel* lb_protein_residues = us_label( tr( "Residue count:" ) );
    protein_info->addWidget( lb_protein_residues, prow, 0 );

    le_protein_residues = us_lineedit( "0", 0, true );
    protein_info->addWidget( le_protein_residues, prow, 1 );
      
    QLabel* lb_protein_e280     = us_label(
          tr( "E280 <small>(OD/(mol*cm))</small>:" ) );
    protein_info->addWidget( lb_protein_e280, prow, 2 );
    
    //le_protein_e280 = us_lineedit( "0", 0, true );
    le_protein_e280 = us_lineedit( "0", 0, false );
    connect( le_protein_e280, SIGNAL( textChanged  ( const QString& ) ),
	                      SLOT  ( value_changed_e280( const QString& ) ) );

    protein_info->addWidget( le_protein_e280, prow++, 3 );
    QSpacerItem* spacer1 = new QSpacerItem( 20, 0 );
    protein_info->addItem( spacer1, prow, 0, 1, 4 );
    protein_info->setRowStretch( prow, 100 );
    
    // END Protein widget ///////////////////////////////////////////////

   // Start DNA/RNA widget //////////////
   dna_widget              = new QWidget( this );
   QGridLayout* dna_layout = new QGridLayout( dna_widget );
   dna_layout->setSpacing        ( 2 );
   dna_layout->setContentsMargins( 2, 2, 2, 2 );

   //QPalette p = US_GuiSettings::labelColor();
   QPalette p = US_GuiSettings::frameColor();
   //qDebug() << "Palette: " << p;
   
   QGroupBox*    gb_double = new QGroupBox( tr( "Calculate MW" ) );
   //p.setColor(QPalette::Dark, Qt::white);
   gb_double->setAutoFillBackground(true);
   gb_double->setPalette( p );
   QGridLayout*  grid1     = new QGridLayout;   
   grid1->setSpacing        ( 2 );
   grid1->setContentsMargins( 2, 2, 2, 2 );
   
   QGridLayout* box1 = us_checkbox( tr( "Double Stranded" ), ck_stranded, true );
   QGridLayout* box2 = us_checkbox( tr( "Complement Only" ), ck_mw_only );
   grid1->addLayout( box1, 0, 0 );
   grid1->addLayout( box2, 1, 0 );
   connect( ck_stranded, SIGNAL( toggled        ( bool ) ), 
	    SLOT  ( update_stranded( bool ) ) );
   connect( ck_mw_only , SIGNAL( toggled        ( bool ) ), 
	    SLOT  ( update_mw_only ( bool ) ) );
 
   QVBoxLayout* stretch1 = new QVBoxLayout;
   stretch1->addStretch();
   grid1->addLayout( stretch1, 2, 0 );

   gb_double->setLayout( grid1 ); 
   dna_layout->addWidget( gb_double, 0, 0 );

   QGroupBox*    gb_three_prime = new QGroupBox( tr( "Three prime" ) );
   gb_three_prime->setAutoFillBackground(true);
   gb_three_prime->setPalette( p );
   QGridLayout*  grid2          = new QGridLayout;   
   grid2->setSpacing        ( 2 );
   grid2->setContentsMargins( 2, 2, 2, 2 );

   QGridLayout* box3 = us_radiobutton( tr( "Hydroxyl" ), rb_3_hydroxyl, true );
   QGridLayout* box4 = us_radiobutton( tr( "Phosphate" ), rb_3_phosphate );

   grid2->addLayout( box3, 0, 0 );
   grid2->addLayout( box4, 0, 1 );
   gb_three_prime->setLayout( grid2 ); 
   connect( rb_3_hydroxyl, SIGNAL( toggled          ( bool ) ), 
	    SLOT  ( update_nucleotide( bool ) ) );

   dna_layout->addWidget( gb_three_prime, 1, 0 );

   QGroupBox*    gb_five_prime = new QGroupBox( tr( "Five prime" ) );
   gb_five_prime->setAutoFillBackground(true);
   gb_five_prime->setPalette( p );
   QGridLayout*  grid3         = new QGridLayout;   
   grid3->setSpacing        ( 2 );
   grid3->setContentsMargins( 2, 2, 2, 2 );

   QGridLayout* box5 = us_radiobutton( tr( "Hydroxyl" ), rb_5_hydroxyl, true );
   QGridLayout* box6 = us_radiobutton( tr( "Phosphate" ), rb_5_phosphate );

   grid3->addLayout( box5, 0, 0 );
   grid3->addLayout( box6, 0, 1 );
  
   gb_five_prime->setLayout( grid3 ); 
   connect( rb_5_hydroxyl, SIGNAL( toggled          ( bool ) ), 
	    SLOT  ( update_nucleotide( bool ) ) );

   dna_layout->addWidget( gb_five_prime, 2, 0 );

   QGridLayout* ratios = new QGridLayout;

   QLabel* lb_ratios = us_banner( tr( "Counterion molar ratio/nucletide" ) );
   ratios->addWidget( lb_ratios, 0, 0, 1, 3 );

   QLabel* lb_sodium = us_label( tr( "Sodium, Na+" ) );
   ratios->addWidget( lb_sodium, 1, 0 );

   ct_sodium = us_counter( 2, 0.0, 1.0, 0.0 );
   ct_sodium->setSingleStep( 0.01 );
   connect( ct_sodium, SIGNAL( valueChanged     ( double ) ),
                        SLOT  ( update_nucleotide( double ) ) );
   ratios->addWidget( ct_sodium, 1, 1, 1, 2 );

   QLabel* lb_potassium = us_label( tr( "Potassium, K+" ) );
   ratios->addWidget( lb_potassium, 2, 0 );

   ct_potassium = us_counter( 2, 0.0, 1.0, 0.0 );
   ct_potassium->setSingleStep( 0.01 );
   connect( ct_potassium, SIGNAL( valueChanged     ( double ) ),
	                 SLOT  ( update_nucleotide( double ) ) );
   ratios->addWidget( ct_potassium, 2, 1, 1, 2 );

   QLabel* lb_lithium = us_label( tr( "Lithium, Li+" ) );
   ratios->addWidget( lb_lithium, 3, 0 );

   ct_lithium = us_counter( 2, 0.0, 1.0, 0.0 );
   ct_lithium->setSingleStep( 0.01 );
   connect( ct_lithium, SIGNAL( valueChanged     ( double ) ),
                         SLOT  ( update_nucleotide( double ) ) );
   ratios->addWidget( ct_lithium, 3, 1, 1, 2 );

   QLabel* lb_magnesium = us_label( tr( "Magnesium, Mg+" ) );
   ratios->addWidget( lb_magnesium, 4, 0 );

   ct_magnesium = us_counter( 2, 0.0, 1.0, 0.0 );
   ct_magnesium->setSingleStep( 0.01 );
   connect( ct_magnesium, SIGNAL( valueChanged     ( double ) ),
                           SLOT  ( update_nucleotide( double ) ) );
   ratios->addWidget( ct_magnesium, 4, 1, 1, 2 );

   QLabel* lb_calcium = us_label( tr( "Calcium, Ca+" ) );
   ratios->addWidget( lb_calcium, 5, 0 );

   ct_calcium = us_counter( 2, 0.0, 1.0, 0.0 );
   ct_calcium->setSingleStep( 0.01 );
   connect( ct_calcium, SIGNAL( valueChanged     ( double ) ),
                         SLOT  ( update_nucleotide( double ) ) );
   ratios->addWidget( ct_calcium, 5, 1 );

   dna_layout->addLayout( ratios, 0, 1, 4, 2 );

   QGridLayout* nucle_data = new QGridLayout;
   QLabel* lb_nucle_mw = us_label( tr( "MW <small>(Daltons)</small>:" ) );
   nucle_data->addWidget( lb_nucle_mw, 0, 0 );

   le_nucle_mw = us_lineedit( "", -2, true );
   nucle_data->addWidget( le_nucle_mw, 0, 1, 1, 3 );

   QLabel* lb_nucle_vbar = us_label( 
         tr( "VBar<small>(cm<sup>3</sup>/g)</small>:" ) );
   nucle_data->addWidget( lb_nucle_vbar, 1, 0 );

   le_nucle_vbar = us_lineedit( "0.5500" );
   connect( le_nucle_vbar, SIGNAL( textChanged  ( const QString& ) ),
                            SLOT  ( value_changed( const QString& ) ) );
   nucle_data->addWidget( le_nucle_vbar, 1, 1, 1, 3 );

   dna_layout->addLayout( nucle_data, 4, 0, 2, 3 );
     
   // End DNA/RNA widget ///////////////////////////////////////////////

   // Start carbohydrates widget ///////////////////////////////////////

   carbs_widget = new QWidget( this );
   
   QGridLayout* carbs_info   = new QGridLayout( carbs_widget );
   carbs_info->setSpacing        ( 2 );
   carbs_info->setContentsMargins( 2, 2, 2, 2 );

   QGridLayout* box7 = us_checkbox( tr( "Gradient-Forming" ),
                                    ck_grad_form, false );
   carbs_info->addLayout( box7,          0, 0, 1, 4 );

   QLabel* lb_carbs_mw = us_label( tr( "MW <small>(Daltons)</small>:" ) );
   carbs_info->addWidget( lb_carbs_mw,   1, 0 );

   le_carbs_mw = us_lineedit( "" );
   carbs_info->addWidget( le_carbs_mw,   1, 1 );

   QLabel* lb_carbs_vbar = us_label( 
         tr( "VBar<small>(cm<sup>3</sup>/g)</small>:" ) );
   carbs_info->addWidget( lb_carbs_vbar, 1, 2 );

   le_carbs_vbar = us_lineedit( "" );
   carbs_info->addWidget( le_carbs_vbar, 1, 3 );

   // End Carbohydrates widget //////////////////////////////////////////


   int row = 0;
   main->addWidget( bn_newanalyte,   row++,  0, 1, 12 );
   main->addLayout( radios,          row++,  0, 1, 12 );
   main->addWidget( lb_descrip,      row,    0, 1,  4 );
   main->addWidget( le_descrip,      row++,  4, 1,  8 );
   main->addWidget( pb_sequence,     row,    6, 1,  3 );
   main->addWidget( pb_spectrum,     row++,  9, 1,  3 );
   main->addWidget( pb_cancel,       row,    6, 1,  2 );
   main->addWidget( pb_reset,        row,    8, 1,  2 );
   main->addWidget( pb_accept,       row++, 10, 1,  2 );
   row += 5;
  
   main->addWidget( protein_widget, row, 0, 1, 12 ); 
   main->addWidget( dna_widget, row, 0, 1, 12 ); 
   main->addWidget( carbs_widget, row, 0, 2, 12 ); 

   //for ( int ii = 0; ii < 12; ii++ )  main->setColumnStretch( ii, 99 );
   //for ( int ii = 0; ii < 12; ii++ )  main->setRowStretch( ii, 99 );
   main->setRowStretch( 5, 5 );

   connect( typeButtons,     SIGNAL( buttonClicked   ( int )       ),
            this,            SLOT  ( set_analyte_type( int )       ) );
   connect( pb_sequence,     SIGNAL( clicked() ),
	    this,            SLOT  ( manage_sequence()    ) );
   //connect( pb_spectrum,     SIGNAL( clicked() ),
   // 	    this,            SLOT  ( manage_spectrum()    ) );
   connect( pb_spectrum,     SIGNAL( clicked() ),
    	    this,            SLOT  ( spectrum_class()    ) );

   init_analyte();
   set_analyte_type( 0 );

}


// Display a spectrum dialog for list/manage
void US_AnalyteMgrNew::spectrum_class( void )
{
  US_NewSpectrum *w = new US_NewSpectrum("ANALYTE", le_descrip->text(), le_protein_e280->text(), analyte);

  connect( w,     SIGNAL( change_prot_e280( void ) ),
           this,  SLOT ( change_prot_e280( void ) ) );

  w->setParent(this, Qt::Window);
  w->setWindowModality(Qt::WindowModal);
  w->setAttribute(Qt::WA_DeleteOnClose);
  w->show(); 
}

// Display a spectrum dialog for list/manage
void US_AnalyteMgrNew::change_prot_e280( void )
{
  le_protein_e280->setText( QString::number(analyte->extinction[280.0]) );
}

// Slot for Entering/Editing Sequence ////
void US_AnalyteMgrNew::manage_sequence( void )
{
   US_SequenceEditor* edit = new US_SequenceEditor( analyte->sequence );
   connect( edit, SIGNAL( sequenceChanged( QString ) ), 
                  SLOT  ( update_sequence( QString ) ) );
   edit->exec();

}

void US_AnalyteMgrNew::update_sequence( QString seq )
{
   seq = seq.toLower().remove( QRegExp( "[\\s0-9]" ) );
   QString check = seq;

   if ( seq == analyte->sequence ) return;

   switch ( analyte->type )
   {
      case US_Analyte::PROTEIN:
         seq.remove( QRegExp( "[^a-z\\+\\?\\@]" ) );
         break;

      case US_Analyte::DNA:
         seq.remove( QRegExp( "[^acgt]" ) );
         break;

      case US_Analyte::RNA:
         seq.remove( QRegExp( "[^acgu]" ) );
         break;

      case US_Analyte::CARBOHYDRATE:
         break;
   }

   if ( check != seq )
   {
      int response = QMessageBox::question( this,
         tr( "Attention" ), 
         tr( "There are invalid characters in the sequence!\n"
             "Invalid characters will be removed\n"
             "Do you want to continue?" ), 
         QMessageBox::Yes, QMessageBox::No );

      if ( response == QMessageBox::No ) return;
   }

   // Reformat the sequence
   const int  gsize = 10;
   const int  lsize = 6;

   // Groups of gsize nucleotides
   int     segments = ( seq.size() + gsize - 1 ) / gsize;
   int     p        = 0;
   QString s;

   for ( int i = 0; i < segments; i++ )
   {
      QString t;

      if ( i % lsize == 0 )
         s += t.sprintf( "%04i ", i * gsize + 1 );
     
      s += seq.mid( p, gsize );
      p += gsize;
      
      if ( i % lsize == lsize - 1 )
         s.append( "\n" );
      else
         s.append( " " );
   }

   analyte->sequence = s;

   switch ( analyte->type )
   {
      case US_Analyte::PROTEIN:
      {
         US_Math2::Peptide p;
         double temperature = le_protein_temp->text().toDouble();
         US_Math2::calc_vbar( p, analyte->sequence, temperature );

         le_protein_mw      ->setText( QString::number( (int) p.mw ) );
         le_protein_vbar20  ->setText( QString::number( p.vbar20, 'f', 4 ) );
         le_protein_vbar    ->setText( QString::number( p.vbar  , 'f', 4 ) );
         le_protein_residues->setText( QString::number( p.residues ) );
         le_protein_e280    ->setText( QString::number( p.e280     ) );

         analyte->mw     = p.mw;
         analyte->vbar20 = p.vbar20;

         // If spectrum is empty, set to 280.0/e280
#if 0
         QString spectrum_type = cmb_optics->currentText();

         if ( spectrum_type == tr( "Absorbance" ) )
         {
#endif
	   if ( analyte->extinction.count() == 0 )
               analyte->extinction  [ 280.0 ] = p.e280;
#if 0
         }
         else if ( spectrum_type == tr( "Interference" ) )
         {
            if ( analyte->refraction.count() == 0 )
               analyte->refraction  [ 280.0 ] = p.e280;
         }
         else
         {
            if ( analyte->fluorescence.count() == 0 )
               analyte->fluorescence[ 280.0 ] = p.e280;
         }
         break;
#endif
	 pb_spectrum->setEnabled(true);
	 break;
      }

      case US_Analyte::DNA:
      case US_Analyte::RNA:
	qDebug() << "DNA/RNA extrinction BEFORE:-- " << analyte->extinction.count();
	update_nucleotide();
	qDebug() << "DNA/RNA extrinction AFTER: -- " << analyte->extinction.count();
	break;

      case US_Analyte::CARBOHYDRATE:
	 le_protein_mw  ->setText( QString::number( (int) analyte->mw ) );
         le_protein_vbar20->setText( QString::number( analyte->vbar20, 'f', 4 ) );
         break;
   }

   bool can_accept = ( !le_descrip->text().isEmpty() );
   pb_accept  ->setEnabled( can_accept );

//   pb_save->setEnabled( true );
//   pb_more->setEnabled( true );
}


void US_AnalyteMgrNew::value_changed_e280( const QString& )
{
  double val_vbar20 =  le_protein_e280->text().toDouble();
  analyte->extinction[280.0] = val_vbar20;
DbgLv(1) << "New e280: " << analyte->extinction[280.0];
} 

void US_AnalyteMgrNew::value_changed( const QString& )
{
   // This only is activated by changes to vbar20
   // (either protein or dna/rna) but vbar is not saved.
   temp_changed( le_protein_temp->text() );
}

void US_AnalyteMgrNew::temp_changed( const QString& text )
{
   double temperature = text.toDouble();
   double vbar20      = le_protein_vbar20->text().toDouble();
   double vbar        = vbar20 + 4.25e-4 * ( temperature - 20.0 );
   le_protein_vbar->setText( QString::number( vbar, 'f', 4 ) );
}

void US_AnalyteMgrNew::update_stranded( bool checked )
{
  // if ( inReset ) return;
   if ( checked ) ck_mw_only->setChecked( false );
   update_nucleotide();
}

void US_AnalyteMgrNew::update_mw_only( bool checked )
{
  //if ( inReset ) return;
   if ( checked ) ck_stranded->setChecked( false );
   update_nucleotide();
}

// Update Nucleotides ////////////////////
void US_AnalyteMgrNew::update_nucleotide( bool /* value */ )
{
  //if ( inReset ) return;
   update_nucleotide();
}

void US_AnalyteMgrNew::update_nucleotide( double /* value */ )
{
  //if ( inReset ) return;
   update_nucleotide();
}

void US_AnalyteMgrNew::update_nucleotide( void )
{
  //if ( inReset ) return;

   parse_dna();

   bool isDNA              = rb_dna       ->isChecked();
   analyte->doubleStranded = ck_stranded  ->isChecked();
   analyte->complement     = ck_mw_only   ->isChecked();
   analyte->_3prime        = rb_3_hydroxyl->isChecked();
   analyte->_5prime        = rb_5_hydroxyl->isChecked();

   analyte->sodium    = ct_sodium   ->value();
   analyte->potassium = ct_potassium->value();
   analyte->lithium   = ct_lithium  ->value();
   analyte->magnesium = ct_magnesium->value();
   analyte->calcium   = ct_calcium  ->value();

   double MW = 0;
   uint   total = A + G + C + T + U;
   
   if ( analyte->doubleStranded ) total *= 2;
   
   const double mw_A = 313.209;
   const double mw_C = 289.184;
   const double mw_G = 329.208;
   const double mw_T = 304.196;
   const double mw_U = 274.170;
   
   if ( isDNA )
   {
      if ( analyte->doubleStranded )
      {
         MW += A * mw_A;
         MW += G * mw_G;
         MW += C * mw_C;
         MW += T * mw_T;
         MW += A * mw_T;
         MW += G * mw_C;
         MW += C * mw_G;
         MW += T * mw_A;
      }

      if ( analyte->complement )
      {
         MW += A * mw_T;
         MW += G * mw_C;
         MW += C * mw_G;
         MW += T * mw_A;
      }

      if ( ! analyte->complement && ! analyte->doubleStranded )
      {
         MW += A * mw_A;
         MW += G * mw_G;
         MW += C * mw_C;
         MW += T * mw_T;
      }
   }
   else /* RNA */
   {
      if ( analyte->doubleStranded )
      {
         MW += A * ( mw_A + 15.999 );
         MW += G * ( mw_G + 15.999 );
         MW += C * ( mw_C + 15.999 );
         MW += U * ( mw_U + 15.999 );
         MW += A * ( mw_U + 15.999 );
         MW += G * ( mw_C + 15.999 );
         MW += C * ( mw_G + 15.999 );
         MW += U * ( mw_A + 15.999 );
      }

      if ( analyte->complement )
      {
         MW += A * ( mw_U + 15.999 );
         MW += G * ( mw_C + 15.999 );
         MW += C * ( mw_G + 15.999 );
         MW += U * ( mw_A + 15.999 );
      }

      if ( ! analyte->complement && ! analyte->doubleStranded )
      {
         MW += A * ( mw_A + 15.999 );
         MW += G * ( mw_G + 15.999 );
         MW += C * ( mw_C + 15.999 );
         MW += U * ( mw_U + 15.999 );
      }
   }
   
   MW += analyte->sodium    * total * 22.99;
   MW += analyte->potassium * total * 39.1;
   MW += analyte->lithium   * total * 6.94;
   MW += analyte->magnesium * total * 24.305;
   MW += analyte->calcium   * total * 40.08;
   
   if ( analyte->_3prime )
   {
      MW += 17.01;
      if ( analyte->doubleStranded )  MW += 17.01; 
   }
   else // we have phosphate
   {
      MW += 94.87;
      if ( analyte->doubleStranded ) MW += 94.87;
   }

   if ( analyte->_5prime )
   {
      MW -=  77.96;
      if ( analyte->doubleStranded )  MW -= 77.96; 
   }

   if ( analyte->sequence.isEmpty() ) MW = 0;

   QString s;

   if ( analyte->doubleStranded )
   {
      s.sprintf(" %2.5e kD (%d A, %d G, %d C, %d U, %d T, %d bp)",
            MW / 1000.0, A, G, C, U, T, total / 2);
   }
   else
   {
     s.sprintf(" %2.5e kD (%d A, %d G, %d C, %d U, %d T, %d bases)",
            MW / 1000.0, A, G, C, U, T, total );
   }
   
   le_nucle_mw->setText( s );

   if ( rb_dna->isChecked() )
   {

      if ( MW > 0 && T == 0 && U > 0)
         QMessageBox::question( this, 
            tr( "Attention:" ),
            tr( "Are you sure?\n"
                "There don't appear to be any thymine residues present,\n"
                "instead there are uracil residues in this sequence." ) );
   }
   else // DNA */
   {
      if ( MW > 0 && T > 0 && U == 0 )
         QMessageBox::question( this,
            tr( "Attention:" ),
            tr( "Are you sure?\n"
                "There don't appear to be any uracil residues present,\n"
                "instead there are thymine residues in this sequence." ) );
   }
}

void US_AnalyteMgrNew::parse_dna( void )
{
   A = analyte->sequence.count( "a" );
   C = analyte->sequence.count( "c" );
   G = analyte->sequence.count( "g" );
   T = analyte->sequence.count( "t" );
   U = analyte->sequence.count( "u" );
}

// Slot for change to New panel
void US_AnalyteMgrNew::init_analyte( void )
{
DbgLv(1) << "agN: init_a";
   from_db       = ( (*db_or_disk) == 1 );
DbgLv(1) << "agN: init_a  from_db" << from_db << "dkordk" << *db_or_disk;

   analyte->analyteID         = "";
   analyte->analyteGUID       = "";
   analyte->description       = "";
   analyte->sequence          = "";
   analyte->extinction.clear();

   reset();
}

void US_AnalyteMgrNew::reset()
{
   QStringList mw;
   if ( rb_protein->isChecked() ) 
     {
       analyte->type = US_Analyte::PROTEIN;
       le_descrip->setText("New Protein Analyte");
       le_protein_mw->setText("0");
       le_protein_vbar20->setText("0.0000");
       le_protein_vbar  ->setText("0.0000");
       if ( ! signal_tmp ) le_protein_temp->setText( "20.0" );
       le_protein_residues ->setText("0");
       le_protein_e280 ->setText("0");
       analyte->description = le_descrip->text();
       analyte->mw          = le_protein_mw->text().toDouble();
       analyte->vbar20      = le_protein_vbar20->text().toDouble();

       pb_spectrum->setEnabled( false );
     }
   
   if ( rb_dna    ->isChecked() )
     {
       analyte->type = US_Analyte::DNA;
       le_descrip->setText("New DNA Analyte");
       ck_stranded->setChecked( true );
       ck_mw_only ->setChecked( false );
       rb_3_hydroxyl->setChecked( true );
       rb_5_hydroxyl->setChecked( true );
       ct_sodium          ->setValue( 0.0 );
       ct_potassium       ->setValue( 0.0 );
       ct_lithium         ->setValue( 0.0 );
       ct_magnesium       ->setValue( 0.0 );
       ct_calcium         ->setValue( 0.0 );
       
       update_nucleotide();
       le_nucle_vbar->setText("0.5500");
       mw = le_nucle_mw->text().split( " ", QString::SkipEmptyParts );
       analyte->description = le_descrip->text();
       analyte->mw          = mw[ 0 ].toDouble() * 1000.0;
       analyte->vbar20      = le_nucle_vbar->text().toDouble();

       pb_spectrum->setEnabled( true );
     }
   
   if ( rb_rna    ->isChecked() ) 
     {
       analyte->type = US_Analyte::RNA;
       le_descrip->setText("New RNA Analyte");
       ck_stranded->setChecked( true );
       ck_mw_only ->setChecked( false );
       rb_3_hydroxyl->setChecked( true );
       rb_5_hydroxyl->setChecked( true );
       ct_sodium          ->setValue( 0.0 );
       ct_potassium       ->setValue( 0.0 );
       ct_lithium         ->setValue( 0.0 );
       ct_magnesium       ->setValue( 0.0 );
       ct_calcium         ->setValue( 0.0 );

       update_nucleotide();
       le_nucle_vbar->setText("0.5500");
       mw = le_nucle_mw->text().split( " ", QString::SkipEmptyParts );
       analyte->description = le_descrip->text();
       analyte->mw          = mw[ 0 ].toDouble() * 1000.0;
       analyte->vbar20      = le_nucle_vbar->text().toDouble();

       pb_spectrum->setEnabled( true );
     }
   
   if ( rb_carbo  ->isChecked() ) 
     {
       analyte->type = US_Analyte::CARBOHYDRATE;
       le_descrip->setText("New Carbohydrate Analyte");
       ck_grad_form       ->setChecked( false );
       le_carbs_mw        ->clear();
       le_carbs_vbar      ->clear();
       analyte->description = le_descrip->text();
       analyte->mw          = le_carbs_mw  ->text().toDouble();
       analyte->vbar20      = le_carbs_vbar->text().toDouble();
       analyte->grad_form   = ck_grad_form ->isChecked();

       pb_spectrum->setEnabled( true );
     }

   pb_accept          ->setEnabled( false );

}

void US_AnalyteMgrNew::set_analyte_type( int type )
{
DbgLv(1) << "agN: sAtype" << type;
#if 0
   if ( inReset ) return;
#endif

   bool visProt         = true;
   bool visAll          = true;
   bool visDRna         = true;
   bool visCarb         = true;
DbgLv(1) << "agS: sAtype: type" << type;

 anatype = type;
 
 reset();
 
 switch ( type )
   {
      case US_Analyte::PROTEIN:
         visDRna              = false;
         visCarb              = false;
	 break;

      case US_Analyte::DNA:
	 visProt              = false;
         visCarb              = false;
         break;

      case US_Analyte::RNA:
         visProt              = false;
         visCarb              = false;
         break;

      case US_Analyte::CARBOHYDRATE:
         visProt              = false;
         visDRna              = false;
         break;
   }
   lb_descrip ->setVisible( visAll );
   le_descrip ->setVisible( visAll );
   protein_widget->setVisible( visProt );
   dna_widget->setVisible( visDRna );
   carbs_widget->setVisible( visCarb ); 
   

#if 0
   US_Analyte a;  // Create a blank analyte
   analyte            = a;
   analyte.type       = (US_Analyte::analyte_t) type;
   saved_analyte.type = analyte.type;
   reset();
   list();
#endif
   //reset();
   //query();
   //search( "" );
}


// Slot to capture new analyte description
void US_AnalyteMgrNew::new_description()
{
DbgLv(1) << "AnaN:SL: new_description()";
   analyte->description = le_descrip->text();
DbgLv(1) << "AnaN:SL: new_desc:" << analyte->description;
DbgLv(1) << "AnaN:SL: anatype"  << anatype;

 bool can_accept = false;
 
 switch ( anatype )
   {
      case US_Analyte::PROTEIN:
	can_accept = ( !le_descrip->text().isEmpty()  &&
                       !le_protein_mw->text().isEmpty()  &&
                       !le_protein_vbar20 ->text().isEmpty() );
	break;

      case US_Analyte::DNA:
      case US_Analyte::RNA:
         can_accept = ( !le_descrip->text().isEmpty()  &&
                       !le_protein_mw->text().isEmpty()  &&
                       !le_protein_vbar20 ->text().isEmpty() );
         break;

      case US_Analyte::CARBOHYDRATE:
         can_accept = ( !le_descrip->text().isEmpty()  &&
                       !le_protein_mw->text().isEmpty()  &&
                       !le_protein_vbar20 ->text().isEmpty() );
         break;
   }

 pb_accept  ->setEnabled( can_accept );

#if 0
   bool can_accept = ( !le_descrip->text().isEmpty()  &&
                       !le_protein_mw->text().isEmpty()  &&
                       !le_protein_vbar20 ->text().isEmpty() );
   pb_accept  ->setEnabled( can_accept );
#endif
}

//void US_AnalyteMgrNew::set_acceptbutton( int type )
//{
  
//}

// Slot for entry of concentration to complete add-component
void US_AnalyteMgrNew::add_component()
{
DbgLv(1) << "AnaN:SL: add_component()";
#if 0
   double concen   = le_concen->text().toDouble();

   le_concen->setText( "" );

   if ( concen == 0.0 )
      return;

   // Get selected component
   QListWidgetItem* item    = lw_allcomps->currentItem();
   QString compID  = QString::number( item->type() );
DbgLv(1) << "AnaN:SL: adco:" << item->text();

   // Verify that concentration is within a valid range
   QString prange = "range";
   QString runit  = "runit";
   double rlow    = prange.section( "-", 0, 0 ).toDouble();
   double rhigh   = prange.section( "-", 1, 1 ).toDouble();

   if ( runit != "runit" )
   { // Change limits to component (not range) units
      double rfact   = 1.0;

      if ( runit == "M" )
      { // Range in M, Concentration in mM; so multiple by 1000
         rfact          = 1000.0;
      }
      else
      { // Range in mM, Concentration mM; so divide by 1000
         rfact          = 0.001;
      }

      rlow          *= rfact;
      rhigh         *= rfact;
DbgLv(1) << "AnaN:SL: adco:  runit" << runit
 << "rlow rhigh" << rlow << rhigh;
   }

   if ( concen < rlow  ||  concen > rhigh )
   {
      QMessageBox::critical( this,
         tr( "Concentration Out of Range" ),
         tr( "Entered concentration %1 out of component range:\n"
             "  %2 - %3 " ).arg( concen ).arg( rlow ).arg(rhigh )
            + "mM" );
      return;
   }

   recalc_density();
   recalc_viscosity();

   bool can_accept = ( !le_descrip->text().isEmpty()  &&
                       !le_density->text().isEmpty()  &&
                       !le_viscos ->text().isEmpty() );
   pb_accept->setEnabled( can_accept );
#endif
}

// Slot for select of analyte component
void US_AnalyteMgrNew::select_bcomp( )
{
#if 0
   QListWidgetItem* item    = lw_allcomps->currentItem();
   QString compID  = QString::number( item->type() );
   //int bcx         = lw_allcomps->currentRow();
DbgLv(1) << "AnaN:SL: select_bcomp()" << item->text();
   QString bctext  = item->text();
   QString lbtext  = tr( "Please enter the concentration of\n " );
   lb_bselect->setText( lbtext );
#endif
}

// Slot for double-click of analyte component to remove
void US_AnalyteMgrNew::remove_bcomp( QListWidgetItem* item )
{
DbgLv(1) << "AnaN:SL: remove_bcomp()" << item->text();
}

// Slot to recalculate density based on new component
void US_AnalyteMgrNew::recalc_density( void )
{
#if 0
   int bcsize        = buffer->component.size();
   if ( bcsize < 1 )
      return;

   buffer->density   = DENS_20W;
DbgLv(1) << "AnaN:SL: recalc_density()" << buffer->component[bcsize-1].name;

   // Iterate over all components in this buffer
   for ( int ii = 0; ii < bcsize; ii++ )
   {

      double c1 = buffer->concentration[ ii ];
      c1        = ( bc->unit == "mM" ) ? ( c1 / 1000.0 ) : c1;
      double c2 = c1 * c1;      // c1^2
      double c3 = c2 * c1;      // c1^3
      double c4 = c3 * c1;      // c1^4

      if ( c1 > 0.0 )
      {
         buffer->density += ( bc->dens_coeff[ 0 ]
                            + bc->dens_coeff[ 1 ] * 1.0e-3 * sqrt( c1 )
                            + bc->dens_coeff[ 2 ] * 1.0e-2 * c1
                            + bc->dens_coeff[ 3 ] * 1.0e-3 * c2
                            + bc->dens_coeff[ 4 ] * 1.0e-4 * c3
                            + bc->dens_coeff[ 5 ] * 1.0e-6 * c4 
                            - DENS_20W );
      }
   }

   le_density->setText( QString::number( buffer->density ) );
#endif
}

// Slot to recalculate viscosity based on new component
void US_AnalyteMgrNew::recalc_viscosity( void )
{
#if 0
   int bcsize        = buffer->component.size();
   if ( bcsize < 1 )
      return;

   buffer->viscosity = VISC_20W;
DbgLv(1) << "AnaN:SL: recalc_viscosity()" << buffer->component[bcsize-1].name;

   // Iterate over all components in this buffer
   for ( int ii = 0; ii < bcsize; ii++ )
   {

      double c1 = buffer->concentration[ ii ];
      c1        = ( bc->unit == "mM" ) ? ( c1 / 1000.0 ) : c1;
      double c2 = c1 * c1;      // c1^2
      double c3 = c2 * c1;      // c1^3
      double c4 = c3 * c1;      // c1^4

      if ( c1 > 0.0 )
      {
         buffer->viscosity += ( bc->visc_coeff[ 0 ]
                              + bc->visc_coeff[ 1 ] * 1.0e-3 * sqrt( c1 )
                              + bc->visc_coeff[ 2 ] * 1.0e-2 * c1
                              + bc->visc_coeff[ 3 ] * 1.0e-3 * c2
                              + bc->visc_coeff[ 4 ] * 1.0e-4 * c3
                              + bc->visc_coeff[ 5 ] * 1.0e-6 * c4 
                              - VISC_20W );
      }
   }

   le_viscos->setText( QString::number( buffer->viscosity ) );
#endif
}

// Slot for manually changed density
void US_AnalyteMgrNew::density( void )
{
#if 0
   buffer->density    = le_density->text().toDouble();
DbgLv(1) << "AnaN:SL: density()" << buffer->density;

   bool can_accept = ( !le_descrip->text().isEmpty()  &&
                       !le_density->text().isEmpty()  &&
                       !le_viscos ->text().isEmpty() );
   pb_accept  ->setEnabled( can_accept );
#endif
}

// Slot for manually changed pH
void US_AnalyteMgrNew::ph( void )
{
#if 0
   buffer->pH         = le_ph->text().toDouble();
DbgLv(1) << "AnaN:SL: ph()" << buffer->pH;
#endif
}

// Slot for manually changed compressibility
void US_AnalyteMgrNew::compressibility( void )
{
#if 0
   buffer->compressibility = le_compress->text().toDouble();
DbgLv(1) << "AnaN:SL: compressibility()" << buffer->compressibility;
#endif
}

// Slot for manually changed viscosity
void US_AnalyteMgrNew::viscosity( void )
{
#if 0
   buffer->viscosity  = le_viscos->text().toDouble();
DbgLv(1) << "AnaN:SL: viscosity()" << buffer->viscosity;

   bool can_accept = ( !le_descrip->text().isEmpty()  &&
                       !le_density->text().isEmpty()  &&
                       !le_viscos ->text().isEmpty() );
   pb_accept  ->setEnabled( can_accept );
#endif
}

// Slot for manually changed density
void US_AnalyteMgrNew::manual_flag( bool is_on )
{
DbgLv(1) << "agN: manf is_on" << is_on;
#if 0
DbgLv(1) << "AnaN:SL: manual_flag()" << is_on;
   us_setReadOnly( le_density, ! is_on );
   us_setReadOnly( le_viscos,  ! is_on );
#endif
}

// Display a spectrum dialog for list/manage
void US_AnalyteMgrNew::spectrum()
{
DbgLv(1) << "AnaN:SL: spectrum()";
QMessageBox::information( this,
 tr( "INCOMPLETE" ),
 tr( "A new Spectrum dialog is under development." ) );
}

// Slot to cancel edited analyte
void US_AnalyteMgrNew::newCanceled()
{
DbgLv(1) << "AnaN:SL: newCanceled()";
#if 0
   analyte->person       = "";
   analyte->analyteID     = "-1";
   analyte->GUID         = "";
   analyte->description  = "";
   analyte->component    .clear();
   analyte->componentIDs .clear();
   analyte->concentration.clear();
#endif

   emit newAnaCanceled();
}

// Slot to accept edited analyte
void US_AnalyteMgrNew::newAccepted()
{
DbgLv(1) << "AnaN:SL: newAccepted()";
   analyte->analyteGUID   = US_Util::new_guid();

DbgLv(1) << "AnaN: Analyte Type: " << analyte->type;

   if ( ! data_ok() ) return;

   if ( analyte->type == US_Analyte::DNA ||
        analyte->type == US_Analyte::RNA  )
   {
      // Strip trailing items from the mw text box.
      QStringList mw = le_nucle_mw->text().split( " ", QString::SkipEmptyParts );

      if ( mw.empty() )
      {
         QMessageBox::warning( this,
            tr( "Analyte Error" ),
            tr( "Molecular weight is empty.  Define a sequence." ) );
         return;
      }

      analyte->mw     = mw[ 0 ].toDouble() * 1000.0;
      analyte->vbar20 = le_nucle_vbar->text().toDouble();
   }

   else if ( analyte->type == US_Analyte::CARBOHYDRATE )
   {
      analyte->mw        = le_carbs_mw  ->text().toDouble();
      analyte->vbar20    = le_carbs_vbar->text().toDouble();
      analyte->grad_form = ck_grad_form ->isChecked();
   }

   else
   {
      analyte->mw     = le_protein_mw    ->text().toDouble();
      analyte->vbar20 = le_protein_vbar20->text().toDouble();
   }

   qDebug() << "In Accept new analyte 1: -- " << analyte->extinction.count();
   
   verify_vbar();

   qDebug() << "In Accept new analyte 2: -- " << analyte->extinction.count();

   if ( from_db )
   { // Add analyte to database
      write_db  ();
DbgLv(1) << "AnaN:SL:  newAcc: DB  id" << analyte->analyteID;
   }

   else
   { // Add analyte to local disk
DbgLv(1) << "AnaN:SL:  newAcc: Disk";
      write_disk();
   }

   qDebug() << "In Accept new analyte 3: -- " << analyte->extinction.count();
   
   emit newAnaAccepted();
}

// Write new analyte to database
void US_AnalyteMgrNew::write_db()
{
DbgLv(1) << "AnaN:SL: write_db()  anaID" << analyte->analyteID;
   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );
   analyte->new_or_changed_spectrum = true;
   int idAna    = analyte->write( true, "", &db );
   analyte->analyteID = QString::number( idAna );

   if ( idAna < 0 )
   {

      QString msg  = tr( "(Return Code = %1 ) " ).arg( idAna )
                     + db.lastError();
      QMessageBox::critical( this,
          tr( "DB Analyte Write Error" ),
          tr( "Error updating analyte in the database:\n" )
          + msg );
   }
}

// Write new analyte to local disk file
void US_AnalyteMgrNew::write_disk()
{
DbgLv(1) << "AnaN:SL: write_disk()  bufID" << analyte->analyteGUID;
   QString path     = US_Settings::dataDir() + "/analytes";

   //bool    newFile;
   QString filename = US_Analyte::get_filename( path, analyte->analyteGUID );

   analyte->write( false, filename );
}


bool US_AnalyteMgrNew::data_ok( void )
{
   // Check to see if a sequence is entered
   if ( analyte->sequence.isEmpty() )
   {
      QMessageBox question( QMessageBox::Question,
            tr( "Attention" ),
            tr( "There is no sequence defined.\n\n" 
                "Continue?" ), 
            QMessageBox::No,
            this );

      question.addButton( tr( "Continue" ), QMessageBox::YesRole );

      if ( question.exec() == QMessageBox::No )
         return false;
   }
   
   analyte->description = le_descrip->text().remove( '|' );

   if ( analyte->description.isEmpty() )
   {
      QMessageBox question( QMessageBox::Question,
            tr( "Attention" ),
            tr( "There is no description for this analyte.\n\n" 
                "Continue?" ), 
            QMessageBox::No,
            this );

      question.addButton( tr( "Continue" ), QMessageBox::YesRole );

      if ( question.exec() == QMessageBox::No )
         return false;

   }

   double vbar = le_protein_vbar->text().toDouble();

   if ( analyte->type == US_Analyte::PROTEIN && ( vbar <= 0.0  || vbar > 2.0 ) )
   {
      QMessageBox::information( this,
         tr( "Attention" ), 
         tr( "The vbar entry (%1) is not a reasonable value." )
         .arg( vbar ) );
      return false;
   }

   double mwvl   = le_protein_mw->text().toDouble();
   if ( analyte->type == US_Analyte::DNA ||
        analyte->type == US_Analyte::RNA  )
      mwvl   = le_nucle_mw->text().section( " ", 1, 1 ).toDouble();
   else if ( analyte->type == US_Analyte::CARBOHYDRATE )
      mwvl   = le_carbs_mw->text().toDouble();

   if ( mwvl <= 0.0 )
   {
      QMessageBox::information( this,
         tr( "Attention" ), 
         tr( "The Molecular Weight entry (%1) is not a reasonable value." )
         .arg( mwvl ) );
      return false;
   }

   return true;
}

void US_AnalyteMgrNew::verify_vbar()
{
   if ( analyte->type == US_Analyte::PROTEIN )
   {
      US_Math2::Peptide p;
      double temperature = le_protein_temp->text().toDouble();
      US_Math2::calc_vbar( p, analyte->sequence, temperature );

      double mwval    = le_protein_mw->text().toDouble();

      if ( p.mw == 0.0 )
         p.mw            = mwval;

      else if ( mwval != 0.0  &&  qAbs( mwval - p.mw ) > 1.0 )
      {
         QString msg  = tr(
               "There is a difference between<br/>"
               "the Molecular Weight value that you specified and<br/>"
               "the one calculated from the protein sequence.<br/> <br/>"
               "Do you wish to accept the specified value?<ul>"
               "<li><b>Yes</b> to use %1 (the specified);</li>"
               "<li><b>No </b> to use %2 (the calculated).</li></ul>" )
                            .arg( mwval ).arg( p.mw );

         QMessageBox msgBox     ( this );
         msgBox.setWindowTitle  ( tr( "Analyte MW Difference" ) );
         msgBox.setTextFormat   ( Qt::RichText );
         msgBox.setText         ( msg );
         msgBox.addButton       ( QMessageBox::No  );
         msgBox.addButton       ( QMessageBox::Yes );
         msgBox.setDefaultButton( QMessageBox::Yes );

         if ( msgBox.exec() == QMessageBox::No )
            mwval        = p.mw;
      }

      else if ( mwval == 0.0 )
         mwval        = p.mw;

      le_protein_mw->setText( QString::number( (int) mwval ) );
      analyte->mw      = mwval;
      double pvbar    = p.vbar20;
      double vbar20   = le_protein_vbar20->text().toDouble();

      if ( qAbs( vbar20 - pvbar ) > 1e-4 )
      {
         QString msg  = tr(
               "There is a difference between<br/>"
               "the vbar20 value that you specified and<br/>"
               "the one calculated from the protein sequence.<br/> <br/>"
               "Do you wish to accept the specified value?<ul>"
               "<li><b>Yes</b> to use %1 (the specified);</li>"
               "<li><b>No </b> to use %2 (the calculated).</li></ul>" )
                            .arg( vbar20 ).arg( pvbar );

         QMessageBox msgBox( this );
         msgBox.setWindowTitle( tr( "Analyte Vbar Difference" ) );
         msgBox.setTextFormat ( Qt::RichText );
         msgBox.setText       ( msg );
         msgBox.addButton     ( QMessageBox::No  );
         msgBox.addButton     ( QMessageBox::Yes );
         msgBox.setDefaultButton( QMessageBox::Yes );

         if ( msgBox.exec() == QMessageBox::No )
            vbar20       = pvbar;

         analyte->vbar20  = vbar20;
         le_protein_vbar20->setText( QString::number( vbar20 ) );
      }
   }
}

// Edit Existing Analyte panel
US_AnalyteMgrEdit::US_AnalyteMgrEdit( int *invID, int *select_db_disk,
      US_Analyte *tmp_analyte ) : US_Widgets()
{
   analyte      = tmp_analyte;
   orig_analyte = *analyte;
   personID     = invID;
   db_or_disk   = select_db_disk;
   from_db      = ( (*db_or_disk) == 1 );
   dbg_level    = US_Settings::us_debug();

   QGridLayout* main = new QGridLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   QPushButton* pb_cancel   = us_pushbutton( tr( "Cancel" ) );
   pb_accept                = us_pushbutton( tr( "Accept" ) );
   QPushButton* pb_spectrum = us_pushbutton( tr( "Manage Spectrum" ) );
   QPushButton* pb_help     = us_pushbutton( tr( "Help" ) );
   QLabel* bn_modana        = us_banner( tr( "Edit an existing analyte" ) );
   QLabel* lb_descrip       = us_label( tr( "Description:" ) );
   le_descrip    = us_lineedit( analyte->description );

   bn_modana->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   //bn_spacer->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   pb_accept->setEnabled( false );
   //us_setReadOnly( le_descrip, false );
   us_setReadOnly( le_descrip, true );

   int row = 0;
   main->addWidget( bn_modana,       row++, 0, 1, 8 );
   main->addWidget( lb_descrip,      row,   0, 1, 1 );
   main->addWidget( le_descrip,      row++, 1, 1, 7 );
   main->addWidget( pb_spectrum,     row,   0, 1, 2 );
   main->addWidget( pb_help,         row,   2, 1, 2 );
   main->addWidget( pb_cancel,       row,   4, 1, 2 );
   main->addWidget( pb_accept,       row++, 6, 1, 2 );

   QLabel *empty = us_banner ("");
   main->addWidget( empty,           row,   0, 6, 8 );

   connect( pb_help,     SIGNAL( clicked()  ),
            this,        SLOT  ( help()     ) );
   connect( pb_cancel,   SIGNAL( clicked()      ),
            this,        SLOT  ( editCanceled() ) );
   connect( pb_accept,   SIGNAL( clicked()      ),
            this,        SLOT  ( editAccepted() ) );
   // connect( pb_spectrum, SIGNAL( clicked()  ),
   //          this,        SLOT  ( spectrum() ) );
   connect( pb_spectrum, SIGNAL( clicked()  ),
            this,        SLOT  ( spectrum_class() ) );
   connect( le_descrip, SIGNAL( editingFinished   () ), 
	                SLOT  ( description() ) );
}

// Slot for manually changed description
void US_AnalyteMgrEdit::description()
{
  if ( analyte->description != le_descrip->text() )
    {
      analyte->description = le_descrip->text();
      pb_accept->setEnabled( !le_descrip->text().isEmpty() );
    }
}

// Initialize analyte settings, possibly after re-entry to Edit panel
void US_AnalyteMgrEdit::spectrum_class( void )
{
  QString ifexists;
  if (analyte->extinction.isEmpty())
    ifexists = "NEW";
  else
    ifexists = "EXISTS";
  
  US_EditSpectrum *w = new US_EditSpectrum("ANALYTE", ifexists, le_descrip->text(), "1.000", analyte);
  
  connect( w,     SIGNAL( change_spectrum( void ) ),
	   this,  SLOT ( change_spectrum( void ) ) );
  connect( w,     SIGNAL( accept_enable( void ) ),
	   this,  SLOT ( accept_enable( void ) ) );
  
  w->setParent(this, Qt::Window);
  w->setWindowModality(Qt::WindowModal);
  w->setAttribute(Qt::WA_DeleteOnClose);
  w->show(); 
}

void US_AnalyteMgrEdit::change_spectrum( void )
{
  emit editAnaAccepted();
}

void US_AnalyteMgrEdit::accept_enable( void )
{
  pb_accept->setEnabled( !le_descrip->text().isEmpty() );
}

// Initialize analyte settings, possibly after re-entry to Edit panel
void US_AnalyteMgrEdit::init_analyte( void )
{
DbgLv(1) << "agE: init_a";
   from_db       = ( (*db_or_disk) == 1 );
DbgLv(1) << "agE: init_a  from_db" << from_db << "dkordk" << *db_or_disk;
   orig_analyte  = *analyte;

   le_descrip ->setText( analyte->description );
   pb_accept  ->setEnabled( false );
}

// Slot to cancel edited analyte
void US_AnalyteMgrEdit::editCanceled()
{
DbgLv(1) << "AnaE:SL: editCanceled()  origAnaGUID" << orig_analyte.analyteGUID;
   *analyte     = orig_analyte;
   emit editAnaCanceled();
}

// Slot to accept edited analyte
void US_AnalyteMgrEdit::editAccepted()
{

  qDebug() << "In EditSpectrum 1: analyte-extinction -- " << analyte->extinction;
DbgLv(1) << "AnaE:SL: editAccepted()  anaGUID" << analyte->analyteGUID;
   if ( from_db )
   { // Update analyte in database
      write_db  ();
DbgLv(1) << "AnaE:SL:  edtAcc: DB  id" << analyte->analyteID;
   }

   else
   { // Update analyte on local disk
DbgLv(1) << "AnaE:SL:  edtAcc: Disk";
      write_disk();
   }

   emit editAnaAccepted();
}

// Write updated analyte to database
void US_AnalyteMgrEdit::write_db()
{
DbgLv(1) << "AnaE:SL: write_db()  bufID" << analyte->analyteID;
   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );
   int idAna    = analyte->write( true, "", &db );

   if ( idAna < 0 )
   {

      QString msg  = tr( "(Return Code = %1 ) " ).arg( idAna )
                     + db.lastError();
      QMessageBox::critical( this,
          tr( "DB Analyte Write Error" ),
          tr( "Error updating analyte in the database:\n" )
          + msg );
   }
}

// Write updated analyte to local disk file
void US_AnalyteMgrEdit::write_disk()
{
DbgLv(1) << "AnaE:SL: write_disk()  GUID" << analyte->analyteGUID;
   QString path     = US_Settings::dataDir() + "/analytes";

   //bool    newFile;
   QString filename = US_Analyte::get_filename( path, analyte->analyteGUID );

   analyte->write( false, filename );
}


// Settings panel
US_AnalyteMgrSettings::US_AnalyteMgrSettings( int *invID, int *select_db_disk )
   : US_Widgets()
{
   personID   = invID;
   db_or_disk = select_db_disk;
   from_db    = ( (*db_or_disk) == 1 );
   dbg_level  = US_Settings::us_debug();

   QGridLayout* main = new QGridLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   QStringList DB = US_Settings::defaultDB();
   if ( DB.isEmpty() ) DB << "Undefined";

   QLabel* lb_DB  = us_banner( tr( "Database: " ) + DB.at( 0 ) );
   QPushButton* pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
   QPushButton* pb_help         = us_pushbutton( tr( "Help" ) );

   if ( US_Settings::us_inv_level() < 3 )
      pb_investigator->setEnabled( false );

   QString number  = ( (*personID) > 0 )
      ? QString::number( US_Settings::us_inv_ID() ) + ": "
      : "";

   le_investigator = us_lineedit( number + US_Settings::us_inv_name() );
   int idb_or_disk = from_db ? US_Disk_DB_Controls::DB
                             : US_Disk_DB_Controls::Disk;
   disk_controls   = new US_Disk_DB_Controls( idb_or_disk );
   QLabel *empty   = us_banner ("");

   lb_DB      ->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   us_setReadOnly( le_investigator, true );


   int row = 0;
   main->addWidget( lb_DB,           row++, 0, 1, 4 );
   main->addWidget( pb_investigator, row,   0, 1, 1 );
   main->addWidget( le_investigator, row++, 1, 1, 3 );
   main->addLayout( disk_controls,   row,   0, 1, 3 );
   main->addWidget( pb_help,         row++, 3, 1, 1 );
   main->addWidget( empty,           row,   0, 6, 4 );

   connect( disk_controls,   SIGNAL( changed   ( bool ) ),
            this,            SLOT  ( db_changed( bool ) ) );
   connect( pb_investigator, SIGNAL( clicked()          ),
            this,            SLOT(   sel_investigator() ) );
   connect( pb_help,         SIGNAL( clicked() ),
            this,            SLOT  ( help()    ) );
}

// Select a new investigator
void US_AnalyteMgrSettings::sel_investigator( void )
{
   US_Investigator* inv_dialog = new US_Investigator( true, (*personID) );

   connect( inv_dialog,
            SIGNAL( investigator_accepted( int ) ),
            SLOT  ( assign_investigator  ( int ) ) );

   inv_dialog->exec();
}

// Note and notify of change in db/disk source
void US_AnalyteMgrSettings::db_changed( bool db )
{
   emit use_db( db );
   // calling class needs to query DB/disk when signal is emitted
   qApp->processEvents();
}

// Assign an investigator after a change
void US_AnalyteMgrSettings::assign_investigator( int invID )
{
   (*personID) = invID;

   QString number = ( (*personID) > 0 )
   ? QString::number( invID ) + ": "
   : "";

   le_investigator->setText( number + US_Settings::us_inv_name() );
   emit investigator_changed( invID );
}

#if 0
// Get buffer components from DB and write to */etc/bufferComponents.xml
void US_AnalyteMgrSettings::synch_components( void )
{
   US_Passwd pw;
   qApp->processEvents();
DbgLv(1) << "setB:synchc read to HD from DB";

   // Read components from DB, write to HD
   component_list.clear();
DbgLv(1) << "setB:synchc   synch complete:  components:"
 << component_list.size();

   le_syncstat->setText( tr( "Local buffer components synchronized." ) );
}
#endif


// Main Analyte window with panels
US_AnalyteGui::US_AnalyteGui( bool           signal,
                              const QString& GUID,
                              int            accessf, 
                              double         temper ) 
   : US_WidgetsDialog( 0, 0 ),
     signal( signal ), guid( GUID ), temperature( temper )
{
   personID     = US_Settings::us_inv_ID();
   analyte      = US_Analyte();
   orig_analyte = analyte;
   disk_or_db   = ( accessf == US_Disk_DB_Controls::Default )
                  ?  US_Settings::default_data_location()
                  : accessf;
   dbg_level    = US_Settings::us_debug();

DbgLv(1) << "Temperature: " << temperature;
DbgLv(1) << "Signal: " << signal;

   setWindowTitle( tr( "Analyte Management" ) );
   setPalette( US_GuiSettings::frameColor() );
   this->setMinimumSize( 640, 480 );

   QGridLayout* main = new QGridLayout(this);
   main->setSpacing(2);
   main->setContentsMargins(2, 2, 2, 2);

   tabWidget   = us_tabwidget();
   selectTab   = new US_AnalyteMgrSelect  ( &personID, &disk_or_db, &analyte );
   newTab      = new US_AnalyteMgrNew     ( &personID, &disk_or_db, &analyte, temperature, signal);
   editTab     = new US_AnalyteMgrEdit    ( &personID, &disk_or_db, &analyte );
   settingsTab = new US_AnalyteMgrSettings( &personID, &disk_or_db );
   tabWidget -> addTab( selectTab,   tr( "Select Analyte" ) );
   tabWidget -> addTab( newTab,      tr( "Enter New Analyte" ) );
   tabWidget -> addTab( editTab,     tr( "Edit Existing Analyte" ) );
   tabWidget -> addTab( settingsTab, tr( "Settings" ) );

   main->addWidget( tabWidget );

   connect( tabWidget,   SIGNAL( currentChanged(       int  ) ),
            this,        SLOT (  checkTab(             int  ) ) );
   connect( selectTab,   SIGNAL( analyteAccepted(      void ) ),
            this,        SLOT (  analyteAccepted(      void ) ) );
   connect( selectTab,   SIGNAL( selectionCanceled(    void ) ),
            this,        SLOT (  analyteRejected(      void ) ) );
   connect( selectTab,   SIGNAL( analytesAccepted( QList< US_Analyte >& ) ),
            this,        SLOT (  analytesAccepted( QList< US_Analyte >& ) ) );
   connect( newTab,      SIGNAL( newAnaAccepted(       void ) ),
            this,        SLOT (  newAnaAccepted(       void ) ) );
   connect( newTab,      SIGNAL( newAnaCanceled(       void ) ),
            this,        SLOT (  newAnaCanceled(       void ) ) );
   connect( editTab,     SIGNAL( editAnaAccepted(      void ) ),
            this,        SLOT (  editAnaAccepted(      void ) ) );
   connect( editTab,     SIGNAL( editAnaCanceled(      void ) ),
            this,        SLOT (  editAnaCanceled(      void ) ) );
   connect( settingsTab, SIGNAL( use_db(               bool ) ),
            this,        SLOT (  update_disk_or_db(    bool ) ) );
   connect( settingsTab, SIGNAL( investigator_changed( int  ) ),
            this,        SLOT (  update_personID(      int  ) ) );
}

void US_AnalyteGui::value_changed( const QString& )
{
   // This only is activated by changes to vbar20
   // (either protein or dna/rna) but vbar is not saved.
#if 0
   temp_changed( le_protein_temp->text() );
#endif
}

// React to a change in panel
void US_AnalyteGui::checkTab( int currentTab )
{
DbgLv(1) << "ckTab: currTab" << currentTab;
   // Need to re-read the database or disk or to make other adjustments
   // in case relevant changes were made elsewhere
   if ( currentTab == 0 )
   {
DbgLv(1) << "ckTab:   selectTab  init_analyte";
      selectTab  ->init_analyte();
   }
   else if ( currentTab == 1 )
   {
DbgLv(1) << "ckTab:   newTab     init_analyte";
      newTab     ->init_analyte();
   }
   else if ( currentTab == 2 )
   {
DbgLv(1) << "ckTab:   editTab    init_analyte";
      editTab    ->init_analyte();
   }
}

// Make global setting for change in db/disk in Settings panel
void US_AnalyteGui::update_disk_or_db( bool choice )
{
   (choice) ? (disk_or_db = 1 ) : (disk_or_db = 0 );
}

// Global person ID after Settings panel change
void US_AnalyteGui::update_personID( int ID )
{
   personID = ID;
}

// Slot for Edit panel analyte-accepted
void US_AnalyteGui::editAnaAccepted( void )
{
DbgLv(1) << "main: editAnaAccepted";
   tabWidget->setCurrentIndex( 0 );
}

// Slot for Edit panel analyte-changes-rejected
void US_AnalyteGui::editAnaCanceled( void )
{
DbgLv(1) << "main: editAnaCanceled";
   tabWidget->setCurrentIndex( 0 );
}

// Slot for New panel analyte accepted
void US_AnalyteGui::newAnaAccepted( void )
{
DbgLv(1) << "main: newAnaAccepted";
   tabWidget->setCurrentIndex( 0 );
}

// Slot for New panel analyte add rejected
void US_AnalyteGui::newAnaCanceled( void )
{
DbgLv(1) << "main: newAnaCanceled";
   tabWidget->setCurrentIndex( 0 );
}

// Exit and signal caller that changes and selected were accepted
void US_AnalyteGui::analyteAccepted( void )
{
#if 0
   valueChanged      ( analyte.density, analyte.viscosity );
#endif
DbgLv(1) << "AnaM: aAcc:";
#if 1
   emit valueChanged  ( analyte );
   emit valueAnalyteID( analyte.analyteID );
#endif

   accept();
}

// Exit and signal caller that changes and selected were accepted
void US_AnalyteGui::analytesAccepted( QList< US_Analyte >& analytes )
{
DbgLv(1) << "AnaM: aaAcc:";
   for ( int ii = 0; ii < analytes.count(); ii++ )
   {
      emit valueChanged( analytes[ ii ] );
   }

   accept();
}

// Exit and signal caller that analyte selection/changes were rejected
void US_AnalyteGui::analyteRejected( void )
{
   analyte   = orig_analyte;

   reject();
}

