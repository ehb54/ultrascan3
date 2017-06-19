//! \file us_buffer_gui.cpp
#include "us_buffer_gui.h"
#include "us_gui_settings.h"
#include "us_settings.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "us_constants.h"
#include "us_investigator.h"
#include "us_table.h"
#include "us_editor.h"
#include "us_util.h"
#include "us_math2.h"
#include "us_eprofile.h"

#if QT_VERSION < 0x050000
#define setSamples(a,b,c) setData(a,b,c)
#define setSymbol(a) setSymbol(*a)
#endif

//! Tab for selection of a buffer
US_BufferGuiSelect::US_BufferGuiSelect( int *invID, int *select_db_disk,
      US_Buffer *tmp_buffer ) : US_Widgets()
{
   buffer     = tmp_buffer;
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
   pb_delete   = us_pushbutton( tr( "Delete Buffer" ) );
   pb_info     = us_pushbutton( tr( "Buffer Details" ) );
   pb_help     = us_pushbutton( tr( "Help" ) );

   QLabel* bn_select     = us_banner( tr( "Select a buffer to use" ) );
   QLabel* lb_search     = us_label( tr( "Search:" ) );
   QLabel* lb_density    = us_label( tr( "Density (20" ) + DEGC
                                   + tr( ", g/cm<sup>3</sup>):" ) );
   QLabel* lb_ph         = us_label( tr( "pH:" ) );
   QLabel* lb_viscosity  = us_label( tr( "Viscosity (20" ) + DEGC
                                   + tr( ", cP):" ) );
   QLabel* lb_compressib = us_label( tr( "Compressibility:" ) );

   le_search             = us_lineedit();
   le_density            = us_lineedit();
   le_ph                 = us_lineedit( "7.0000" );
   le_viscosity          = us_lineedit();
   le_compressib         = us_lineedit();

   lw_buffer_list        = us_listwidget();
   lw_buffer_comps       = us_listwidget();

   pb_accept->setEnabled( false );
   pb_delete->setEnabled( false );
   pb_info  ->setEnabled( false );
   bn_select->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   us_setReadOnly( le_density,     true );
   us_setReadOnly( le_ph,          true );
   us_setReadOnly( le_viscosity,   true );
   us_setReadOnly( le_compressib,  true );
   lw_buffer_comps->setSelectionMode( QAbstractItemView::NoSelection );


   int row = 0;
   main->addWidget( bn_select,       row++, 0, 1, 5 );
   main->addWidget( lb_search,       row,   0, 1, 1 );
   main->addWidget( le_search,       row,   1, 1, 2 );
   main->addWidget( pb_cancel,       row,   3, 1, 1 );
   main->addWidget( pb_accept,       row++, 4, 1, 1 );
   main->addWidget( lw_buffer_list,  row,   0, 6, 3 );
   main->addWidget( pb_spectrum,     row,   3, 1, 1 );
   main->addWidget( pb_delete,       row++, 4, 1, 1 );
   main->addWidget( pb_info,         row,   3, 1, 1 );
   main->addWidget( pb_help,         row++, 4, 1, 1 );
   main->addWidget( lw_buffer_comps, row,   3, 4, 2 );
   row += 5;
   main->addWidget( lb_density,      row,   0, 1, 2 );
   main->addWidget( le_density,      row,   2, 1, 1 );
   main->addWidget( lb_ph,           row,   3, 1, 1 );
   main->addWidget( le_ph,           row++, 4, 1, 1 );
   main->addWidget( lb_viscosity,    row,   0, 1, 2 );
   main->addWidget( le_viscosity,    row,   2, 1, 1 );
   main->addWidget( lb_compressib,   row,   3, 1, 1);
   main->addWidget( le_compressib,   row,   4, 1, 1);

   connect( le_search,      SIGNAL( textChanged( const QString& ) ),
            this,           SLOT  ( search     ( const QString& ) ) );
   connect( pb_cancel,      SIGNAL( clicked() ),
            this,           SLOT  ( reject()  ) );
   connect( pb_accept,      SIGNAL( clicked()       ),
            this,           SLOT  ( accept_buffer() ) );
   connect( pb_info,        SIGNAL( clicked()       ),
            this,           SLOT  ( info_buffer()   ) );
   connect( pb_help,        SIGNAL( clicked() ),
            this,           SLOT  ( help()    ) );
   connect( lw_buffer_list, SIGNAL( itemSelectionChanged() ),
            this,           SLOT  ( select_buffer()        ) );
   connect( pb_spectrum,    SIGNAL( clicked()  ),
            this,           SLOT  ( spectrum() ) );
   connect( pb_delete,      SIGNAL( clicked()       ),
            this,           SLOT  ( delete_buffer() ) );

   US_BufferComponent::getAllFromHD( component_list );

   init_buffer();
}

void US_BufferGuiSelect::select_buffer()
{
   if ( lw_buffer_list->currentRow() < 0 )
   {
DbgLv(1) << "BufS:  selbuf: ROW<0";
      reset();
      return;
   }
else
DbgLv(1) << "BufS:  selbuf: ROW" << lw_buffer_list->currentRow();

   QListWidgetItem *item = lw_buffer_list->currentItem();
   if ( from_db ) read_from_db  ( item );
   else           read_from_disk( item );

   // Write values to screen
   le_density   ->setText( QString::number( buffer->density,    'f', 6 ) );
   le_viscosity ->setText( QString::number( buffer->viscosity,  'f', 5 ) );
   le_ph        ->setText( QString::number( buffer->pH,         'f', 4 ) );
   le_compressib->setText( QString::number( buffer->compressibility, 'e', 4 ) );

   lw_buffer_comps->clear();

DbgLv(1) << "BufS: desc" << buffer->description
 << "compsize" << buffer->componentIDs.size();
DbgLv(0) << "MwDa: dbg_level" << dbg_level;
   for ( int ii = 0; ii < buffer->componentIDs.size(); ii++ )
   {
      show_component( buffer->componentIDs [ ii ],
                      buffer->concentration[ ii ] );
   }

   pb_accept->setEnabled ( true );
   pb_delete->setEnabled ( true );
   pb_info  ->setEnabled ( true );
}

// Select the buffer for the currently selected list item
void US_BufferGuiSelect::select_buffer( QListWidgetItem *tmp_item )
{
   QListWidgetItem *item = tmp_item;
   if ( from_db )  read_from_db  ( item );
   else            read_from_disk( item );

   // Write values to screen
   le_density   ->setText( QString::number( buffer->density,    'f', 6 ) );
   le_viscosity ->setText( QString::number( buffer->viscosity,  'f', 5 ) );
   le_ph        ->setText( QString::number( buffer->pH,         'f', 4 ) );
   le_compressib->setText( QString::number( buffer->compressibility, 'e', 4 ) );

   lw_buffer_comps->clear();

DbgLv(1) << "BufS-item: desc" << buffer->description
 << "compsize" << buffer->componentIDs.size();
   for ( int ii = 0; ii < buffer->componentIDs.size(); ii++ )
   {
      show_component( buffer->componentIDs [ ii ],
                      buffer->concentration[ ii ] );
   }

   pb_accept->setEnabled ( true );
   pb_delete->setEnabled ( true );
}

// Show each component for the current buffer
void US_BufferGuiSelect::show_component( const QString& componentID, double conc )
{
   QString name  = component_list[ componentID ].name;
   QString unit  = component_list[ componentID ].unit;

   QString sconc = QString::number( conc, 'f', 1 );

DbgLv(1) << "shco: name conc unit" << name << sconc << unit
 << "id" << componentID;
   lw_buffer_comps->addItem( name + " (" + sconc + " " + unit + ")" );
}

// Read buffer information from the local disk
void US_BufferGuiSelect::read_from_disk( QListWidgetItem* item )
{
   int row = lw_buffer_list->row( item );
   int buf = buffer_metadata[ row ].index;
DbgLv(1) << "row: " << row << " buf: "  << buf;
   buffer->extinction.clear();

   if ( ! buffer->readFromDisk( filenames[ buf ] ) )
      qDebug() << "read failed";

   buffer->component.clear();

   for ( int ii = 0; ii < buffer->componentIDs.size(); ii++ )
   {
      QString index = buffer->componentIDs[ ii ];
      buffer->component << component_list[ index ];
   }
}

// Read buffer information from the database (by buffer list item)
void US_BufferGuiSelect::read_from_db( QListWidgetItem* item )
{
   int row = lw_buffer_list->row( item );
   QString bufferID = buffer_metadata[ row ].bufferID;
DbgLv(1) << "BufS-rddb-item  row" << row << "bufID" << bufferID;
   read_from_db( bufferID );
}

// Read buffer information from the database (by buffer ID)
void US_BufferGuiSelect::read_from_db( const QString& bufferID )
{
   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );

   // Get the buffer data from the database
   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   buffer->extinction.clear();
DbgLv(1) << "BufS-rddb-bfid  bufID" << bufferID;
   buffer->readFromDB( &db, bufferID );
if(buffer->component.size()>0)
DbgLv(1) << "BufS-rddb-bfid   comp0 nm,rng,un,id" << buffer->component[0].name
 << buffer->component[0].range << buffer->component[0].unit
 << buffer->component[0].componentID;
}

// Accept the currently selected buffer
void US_BufferGuiSelect::accept_buffer( void )
{
   emit bufferAccepted();
}

// Initialize buffer information, often after re-entry to the Select tab
void US_BufferGuiSelect::init_buffer( void )
{
   from_db       = ( (*db_or_disk) == 1 );
   QString bguid = buffer->GUID;
   QString bufid = buffer->bufferID;
   int idBuf     = bufid.toInt();
DbgLv(1) << "BufS-initb: db_or_disk" << *db_or_disk << "from_db" << from_db;
DbgLv(1) << "BufS-initb:   bufID bufGUID" << bufid << bguid << idBuf;
DbgLv(1) << "BufS-initb:   bufpH" << buffer->pH;

   if ( ( from_db  &&  idBuf < 0 )  ||
        ( !from_db  &&  bguid.isEmpty() ) )
   {
      lw_buffer_list->setCurrentRow( -1 );
   }

   query();
DbgLv(1) << "BufS-initb:   post-q:bufID bufGUID" << bufid << bguid;

   if ( ! bguid.isEmpty() )
   {  // There is a selected buffer, select a list item
      if ( from_db ) // DB access
      {
         // Search for bufferID
         for ( int ii = 0; ii < buffer_metadata.size(); ii++ )
         {
            if ( bufid == buffer_metadata[ ii ].bufferID )
            {
DbgLv(1) << "BufS-initb:  ii" << ii << "match bufID"<< buffer->bufferID;
               lw_buffer_list->setCurrentRow( ii );
               QListWidgetItem* item = lw_buffer_list->item( ii );
               select_buffer( item );
               break;
            }
         }
      }

      else            // Disk access
      {
         // Search for GUID
         for ( int ii = 0; ii < buffer_metadata.size(); ii++ )
         {
            if ( bguid == buffer_metadata[ ii ].guid )
            {
               lw_buffer_list->setCurrentRow( ii );
               QListWidgetItem* item = lw_buffer_list->item( ii );
               select_buffer( item );
               break;
            }
         }
      }
   }
   else
   {  // There is no selected buffer, de-select a list item
      lw_buffer_list->setCurrentRow( -1 );

      reset();
   }
}

// If called with string initialized, this function
//  will search buffers matching search string
void US_BufferGuiSelect::search( QString const& text )
{
   QString sep = ";";
   QStringList sortdesc;
   lw_buffer_list  ->clear();
   buffer_metadata.clear();
   sortdesc       .clear();
   int dsize   = descriptions.size();    

   for ( int ii = 0; ii < descriptions.size(); ii++ )
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
DbgLv(1) << "BufS-search:  descsize" << dsize
 << "sortsize" << sortdesc.size();

   for ( int jj = 0; jj < sortdesc.size(); jj++ )
   {  // build list of sorted meta data and ListWidget entries
      int ii      = sortdesc[ jj ].section( sep, 1, 1 ).toInt();

      if ( ii < 0  ||  ii >= dsize )
      {
DbgLv(1) << "BufS-search:  *ERROR* ii" << ii << "jj" << jj
 << "sdesc" << sortdesc[jj].section(sep,0,0);
         continue;
      }
      BufferInfo info;
      info.index       = ii;
      info.description = descriptions[ ii ];
      info.guid        = GUIDs       [ ii ];
      info.bufferID    = bufferIDs   [ ii ];

      buffer_metadata << info;

      lw_buffer_list->addItem( info.description );
   }
}

// Verify path to local buffer files and create directory if need be
bool US_BufferGuiSelect::buffer_path( QString& path )
{
   QDir dir;
   path = US_Settings::dataDir() + "/buffers";

   if ( ! dir.exists( path ) )
   {
      if ( ! dir.mkpath( path ) )
      {
         QMessageBox::critical( this,
            tr( "Bad Buffer Path" ),
            tr( "Could not create default directory for buffers\n" ) + path );
         return false;
      }
   }

   return true;
}

// Display detailed information on selected buffer
void US_BufferGuiSelect::info_buffer( void )
{
   QStringList lines;
   QString inf_text;
   QString big_line( "" );
   int mxlch    = 0;
   int nspec    = buffer->extinction.keys().count();
   qDebug() << "#NSPEC:  " << nspec;
   int ncomp    = buffer->component.count();
   QFont tfont( QFont( US_Widgets::fixedFont().family(),
                       US_GuiSettings::fontSize() - 1 ) );
   QFontMetrics fmet( tfont );

   // Compose buffer information lines
   lines << tr( "Detailed information on the selected buffer" );
   lines << "";
   lines << tr( "Description:              " ) + buffer->description;
   lines << tr( "Density:                  " ) + QString::number(
                                                    buffer->density );
   lines << tr( "Viscosity:                " ) + QString::number(
                                                    buffer->viscosity );
   lines << tr( "pH:                       " ) + QString::number(
                                                    buffer->pH );
   lines << tr( "Compressibility:          " ) + QString::number(
                                                    buffer->compressibility );
   lines << tr( "Manual density,viscosity: " ) + ( buffer->manual
                                                   ? tr( "ON" ) : tr( "off" ) );
   lines << tr( "Database ID (-1==HD):     " ) + buffer->bufferID;
   lines << tr( "Global Identifier:        " ) + buffer->GUID;
   lines << tr( "Inputting Investigator:   " ) + buffer->person;
   lines << tr( "Spectrum pairs Count:     " ) + QString::number( nspec );
   lines << tr( "Components Count:         " ) + QString::number( ncomp );
   lines << "";

   // Compose the sections on each component
   for ( int ii = 0; ii < ncomp; ii++ )
   {
      US_BufferComponent bcomp = buffer->component[ ii ];
      QString compx   = tr( "%1 of %2" ).arg( ii + 1 ).arg( ncomp );
      QString dcoeffs = QString::number( bcomp.dens_coeff[ 0 ] ) + " " +
                        QString::number( bcomp.dens_coeff[ 1 ] ) + " " +
                        QString::number( bcomp.dens_coeff[ 2 ] ) + " " +
                        QString::number( bcomp.dens_coeff[ 3 ] ) + " " +
                        QString::number( bcomp.dens_coeff[ 4 ] ) + " " +
                        QString::number( bcomp.dens_coeff[ 5 ] );
      QString vcoeffs = QString::number( bcomp.visc_coeff[ 0 ] ) + " " +
                        QString::number( bcomp.visc_coeff[ 1 ] ) + " " +
                        QString::number( bcomp.visc_coeff[ 2 ] ) + " " +
                        QString::number( bcomp.visc_coeff[ 3 ] ) + " " +
                        QString::number( bcomp.visc_coeff[ 4 ] ) + " " +
                        QString::number( bcomp.visc_coeff[ 5 ] );
      QString sgradf  = bcomp.grad_form ? tr( "YES" ) : tr( "no" );

      lines << "  " + tr( "Component index:          " ) + compx;
      lines << "  " + tr( "Component ID:             " ) + bcomp.componentID;
      lines << "  " + tr( "Name:                     " ) + bcomp.name;
      lines << "  " + tr( "Unit:                     " ) + bcomp.unit;
      lines << "  " + tr( "Density Coefficients:     " ) + dcoeffs;
      lines << "  " + tr( "Viscosity Coefficients:   " ) + vcoeffs;
      lines << "  " + tr( "Range:                    " ) + bcomp.range;
      lines << "  " + tr( "Gradient Forming:         " ) + sgradf;
      lines << "";
   }

   // Compose the section for any extinction spectrum
   QString stitle  = tr( "  Extinction Spectrum:      " );
   QString spline  = stitle;
   QList< double >  keys = buffer->extinction.keys();
DbgLv(1) << "BufS-info: keys" << keys;
DbgLv(1) << "BufS-info values" << buffer->extinction.values();
   for ( int ii = 0; ii < nspec; ii++ )
   {
      double waveln   = keys[ ii ];
      double extinc   = buffer->extinction[ waveln ];
DbgLv(1) << "BufS-info:  ii" << ii << "waveln extinc" << waveln << extinc;
      QString spair   = QString::number( waveln ) + " / " +
                        QString::number( extinc ) + "  ";
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

   // Build and show the buffer details dialog
   int iwid     = fmet.width( big_line ) + 40;
   int ihgt     = fmet.lineSpacing() * qMin( 22, nlines ) + 80;

   US_Editor* buf_info = new US_Editor( US_Editor::DEFAULT, true,
                                        QString(), this );
   buf_info->setWindowTitle( tr( "Detailed Selected Buffer Information" ) );
   buf_info->move( pos() + QPoint( 200, 200 ) );
   buf_info->resize( iwid, ihgt );
   buf_info->e->setFont( tfont );
   buf_info->e->setText( inf_text );
   buf_info->show();
}

// Display a spectrum dialog for list/manage
void US_BufferGuiSelect::spectrum( void )
{
  qDebug() << buffer->extinction;
  
  if (buffer->extinction.isEmpty())
    {
      QMessageBox::information( this,
      tr( "WARNING" ),
      tr( "Buffer does not have spectrum data!" ) );
    }
  else
    {
      US_ViewSpectrum *w = new US_ViewSpectrum(buffer->extinction);
      w->setParent(this, Qt::Window);
      w->show();
    }
}

// Remove a selected buffer
void US_BufferGuiSelect::delete_buffer( void )
{
   if ( buffer->GUID.size() == 0 || lw_buffer_list->currentRow() < 0 )
   {
      QMessageBox::information( this,
            tr( "Attention" ),
            tr( "First select the buffer to be deleted." ) );
      return;
   }

   int response = QMessageBox::question( this,
            tr( "Confirmation" ),
            tr( "Delete this buffer?\n"
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

// Delete a buffer from local disk
void US_BufferGuiSelect::delete_disk( void )
{
   QString bufGUID = buffer->GUID;
   QString path;
   if ( ! buffer_path( path ) ) return;

   bool    newFile;
   QString filename = US_Buffer::get_filename( path, bufGUID, newFile );

   if ( buffer_in_use( bufGUID ) )
   {
      QMessageBox::warning( this,
         tr( "Buffer Not Deleted" ),
         tr( "The buffer could not be deleted,\n"
             "since it is in use in one or more solutions." ) );
      return;
   }

   if ( ! newFile )
   {
      QFile f( filename );
      f.remove();
   }
}

// Determine if a given buffer GUID is in use
bool US_BufferGuiSelect::buffer_in_use( QString& bufferGUID )
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

         if ( xml.isStartElement()  &&  xml.name() == "buffer" )
         {
            QXmlStreamAttributes atts = xml.attributes();

            if ( atts.value( "guid" ).toString() == bufferGUID )
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

// Delete a buffer from the database (if not in use by another table type)
void US_BufferGuiSelect::delete_db( void )
{
   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );

   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   QStringList q( "get_bufferID" );
   q << buffer->GUID;

   db.query( q );

   int status = db.lastErrno();

   if (  status == US_DB2::OK )
   {
      db.next();
      QString bufferID = db.value( 0 ).toString();

      q[ 0 ] = "delete_buffer";
      q[ 1 ] = bufferID;
      status = db.statusQuery( q );

      QString compType("Buffer");
   }

   if ( status == US_DB2::BUFFR_IN_USE )
   {
      QMessageBox::warning( this,
         tr( "Buffer Not Deleted" ),
         tr( "This buffer could not be deleted since\n"
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

// Reread all buffer information from DB or local disk
void US_BufferGuiSelect::query( void )
{
   if ( from_db )
   {
      read_db();
   }
   else
   {
      read_buffer();
   }
}

// Read buffer information from local disk
void US_BufferGuiSelect::read_buffer( void )
{
   QString path;
   if ( ! buffer_path( path ) ) return;

   filenames   .clear();
   descriptions.clear();
   GUIDs       .clear();
   bufferIDs   .clear();
   le_search->  clear();
   le_search->setReadOnly( true );

   /*
   *   bool desc_set = le_description->text().size() > 0;
   *   pb_save  ->setEnabled( desc_set );
   *   pb_update->setEnabled( desc_set );
   *   pb_del   ->setEnabled( false );
   */

   QDir f( path );
   QStringList filter( "B*.xml" );
   QStringList f_names = f.entryList( filter, QDir::Files, QDir::Name );

   for ( int ii = 0; ii < f_names.size(); ii++ )
   {
      QFile b_file( path + "/" + f_names[ ii ] );

      if ( ! b_file.open( QIODevice::ReadOnly | QIODevice::Text) ) continue;

      QXmlStreamReader xml( &b_file );

      while ( ! xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement() )
         {
            if ( xml.name() == "buffer" )
            {
               QXmlStreamAttributes a = xml.attributes();
               descriptions << a.value( "description" ).toString();
               GUIDs        << a.value( "guid"        ).toString();
               filenames    << path + "/" + f_names[ ii ];
               bufferIDs    << "";
               break;
            }
         }
      }
   }

   lw_buffer_list->clear();

   if ( descriptions.size() == 0 )
      lw_buffer_list->addItem( "No buffer files found." );
   else
   {
      le_search->setReadOnly( false );
      search();
   }
}

// Read buffer information from the database
void US_BufferGuiSelect::read_db( void )
{
   US_Passwd pw;
   US_DB2 db( pw.getPasswd() );

   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   bufferIDs   .clear();
   descriptions.clear();
   GUIDs       .clear();
   le_search->  clear();
   le_search->setText( "" );
   le_search->setReadOnly( true );

   QStringList q;
   q << "get_buffer_desc" << QString::number( *personID );
   db.query( q );

   while ( db.next() )
   {
      bufferIDs    << db.value( 0 ).toString();
      descriptions << db.value( 1 ).toString();
      GUIDs        << "";
   }

   lw_buffer_list->clear();

   if ( descriptions.size() == 0 )
   {
      lw_buffer_list->addItem( "No buffer files found." );
   }
   else
   {
      le_search->setReadOnly( false );
      search();
   }

}

// Report a database connection error
void US_BufferGuiSelect::connect_error( const QString& error )
{
   QMessageBox::warning( this, tr( "Connection Problem" ),
                        tr( "Could not connect to database \n" ) + error );
}

// Accept a selected buffer for return to the caller
void US_BufferGuiSelect::accept( void )
{
}

// Reject the selected buffer and return to caller with no change
void US_BufferGuiSelect::reject( void )
{
   emit selectionCanceled();
}

// Reset to no buffer selected
void US_BufferGuiSelect::reset()
{
   lw_buffer_comps->clear();
   lw_buffer_list ->setCurrentRow( -1 );
   le_density     ->setText( "" );
   le_viscosity   ->setText( "" );
   le_ph          ->setText( "" );
   le_compressib  ->setText( "" );
   pb_accept      ->setEnabled( false );
   pb_delete      ->setEnabled( false );
   pb_info        ->setEnabled( false );
}


// New Buffer panel
US_BufferGuiNew::US_BufferGuiNew( int *invID, int *select_db_disk,
      US_Buffer *tmp_buffer ) : US_Widgets()
{
   buffer     = tmp_buffer;
   personID   = invID;
   db_or_disk = select_db_disk;
   from_db    = ( (*db_or_disk) == 1 );
   dbg_level  = US_Settings::us_debug();

   // Read all buffer components from the
   //  $ULTRASCAN3/etc/bufferComponents xml file:
   US_BufferComponent::getAllFromHD( component_list );

   QGridLayout* main = new QGridLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   QPushButton* pb_cancel   = us_pushbutton( tr( "Cancel" ) );
   pb_accept                = us_pushbutton( tr( "Accept" ) );
   pb_spectrum              = us_pushbutton( tr( "Enter Spectrum" ) );
   //QPushButton* pb_spectrum = us_pushbutton( tr( "Enter Spectrum" ) );
   QPushButton* pb_help     = us_pushbutton( tr( "Help" ) );

   QGridLayout* lo_manual   = us_checkbox(
         tr( "Manual unadjusted Density and Viscosity" ), ck_manual );

   QLabel* bn_newbuf   = us_banner( tr( "Specify a new buffer to add" ), -1 );
   QLabel* lb_descrip  = us_label( tr( "Description:" ) );
   lb_bselect          = us_label( tr( "Please enter the concentration of\n"
                                       " (component) in mM:" ) );
   QLabel* bn_allcomps = us_banner( tr( "Click on item to select" ), -2 );
   QLabel* bn_bufcomps = us_banner( tr( "Double-click an item to remove" ), -2);
   QLabel* lb_density  = us_label( tr( "Density (20" ) + DEGC
                                 + tr( ", g/cm<sup>3</sup>):" ) );
   QLabel* lb_viscos   = us_label( tr( "Viscosity (20" ) + DEGC
                                 + tr( ", cP):" ) );
   QLabel* lb_ph       = us_label( tr( "pH:" ) );
   QLabel* lb_compress = us_label( tr( "Compressibility:" ) );

   le_descrip          = us_lineedit( "" );
   le_concen           = us_lineedit( "" );
   le_density          = us_lineedit( "" );
   le_viscos           = us_lineedit( "" );
   le_ph               = us_lineedit( "7.0000" );
   le_compress         = us_lineedit( "0.0000e+0" );

   lw_allcomps         = us_listwidget();
   lw_bufcomps         = us_listwidget();


   QPalette upal       = lb_bselect->palette();
   upal.setColor( QPalette::WindowText, Qt::red );
   lb_bselect ->setPalette( upal );

   bn_newbuf  ->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   bn_allcomps->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   bn_bufcomps->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );

   pb_accept  ->setEnabled       ( false );
   lw_allcomps->setSortingEnabled( true );
   lw_bufcomps->setSortingEnabled( true );

   us_setReadOnly( le_density,  true  );
   us_setReadOnly( le_viscos,   true  );
   us_setReadOnly( le_ph,       false );
   us_setReadOnly( le_compress, false );


   int row = 0;
   main->addWidget( bn_newbuf,       row++, 0, 1, 8 );
   main->addWidget( lb_descrip,      row,   0, 1, 1 );
   main->addWidget( le_descrip,      row++, 1, 1, 7 );
   main->addWidget( pb_cancel,       row,   4, 1, 2 );
   main->addWidget( pb_accept,       row++, 6, 1, 2 );
   main->addWidget( lb_bselect,      row,   0, 2, 4 );
   main->addWidget( pb_spectrum,     row,   4, 1, 2 );
   main->addWidget( pb_help,         row++, 6, 1, 2 );
   main->addWidget( le_concen,       row++, 4, 1, 4 );
   main->addWidget( bn_allcomps,     row,   0, 1, 4 );
   main->addWidget( bn_bufcomps,     row++, 4, 1, 4 );
   main->addWidget( lw_allcomps,     row,   0, 5, 4 );
   main->addWidget( lw_bufcomps,     row,   4, 5, 4 );
   row    += 5;
   main->addLayout( lo_manual,       row++, 0, 1, 8 );
   main->addWidget( lb_density,      row,   0, 1, 2 );
   main->addWidget( le_density,      row,   2, 1, 2 );
   main->addWidget( lb_ph,           row,   4, 1, 2 );
   main->addWidget( le_ph,           row++, 6, 1, 2 );
   main->addWidget( lb_viscos,       row,   0, 1, 2 );
   main->addWidget( le_viscos,       row,   2, 1, 2 );
   main->addWidget( lb_compress,     row,   4, 1, 2 );
   main->addWidget( le_compress,     row,   6, 1, 2 );


   QStringList keys = component_list.keys();
   qSort( keys );

   for ( int ii = 0; ii < keys.size(); ii++ )
   {
      QString key     = keys[ ii ];
      US_BufferComponent bcomp = component_list[ key ];

      // Insert the buffer component with it's key
      QString sitem = bcomp.name + " (" + bcomp.range + ")";
      new QListWidgetItem( sitem, lw_allcomps, key.toInt() );
   }

   connect( le_descrip,  SIGNAL( editingFinished() ),
            this,        SLOT  ( new_description() ) );
   connect( le_concen,   SIGNAL( editingFinished() ),
            this,        SLOT  ( add_component()   ) );
   connect( lw_allcomps, SIGNAL( itemSelectionChanged() ),
            this,        SLOT  ( select_bcomp()         ) );
   connect( lw_bufcomps, SIGNAL( itemDoubleClicked( QListWidgetItem* ) ),
            this,        SLOT  ( remove_bcomp(      QListWidgetItem* ) ) );
   connect( le_density,  SIGNAL( editingFinished()   ), 
            this,        SLOT  ( density()           ) );
   connect( le_viscos,   SIGNAL( editingFinished()   ), 
            this,        SLOT  ( viscosity()         ) );
   connect( le_ph,       SIGNAL( editingFinished()   ), 
            this,        SLOT  ( ph()                ) );
   connect( le_compress, SIGNAL( editingFinished()   ), 
            this,        SLOT  ( compressibility()   ) );
   connect( ck_manual,   SIGNAL( toggled    ( bool ) ),
            this,        SLOT  ( manual_flag( bool ) ) );
   //connect( pb_spectrum, SIGNAL( clicked()   ),
   //         this,        SLOT  ( spectrum()  ) );

   connect( pb_spectrum, SIGNAL( clicked()   ),
            this,        SLOT  ( spectrum_class()  ) );

   connect( pb_help,     SIGNAL( clicked()   ),
            this,        SLOT  ( help()      ) );
   connect( pb_cancel,   SIGNAL( clicked()     ),
            this,        SLOT  ( newCanceled() ) );
   connect( pb_accept,   SIGNAL( clicked()     ),
            this,        SLOT  ( newAccepted() ) );
}

// Slot for change to New panel
void US_BufferGuiNew::init_buffer( void )
{
   from_db    = ( (*db_or_disk) == 1 );

   // In case we just re-synced in Settings panel,
   //   reread components and recompose list widget
   US_BufferComponent::getAllFromHD( component_list );
DbgLv(1) << "BufN:SL: init_buffer  comps" << component_list.size();
   QStringList keys = component_list.keys();
   qSort( keys );
   lw_allcomps->clear();

   for ( int ii = 0; ii < keys.size(); ii++ )
   {
      QString key              = keys[ ii ];
      US_BufferComponent bcomp = component_list[ key ];

      QString sitem = bcomp.name + " (" + bcomp.range + ")";

      // Insert the buffer component with it's key
      new QListWidgetItem( sitem, lw_allcomps, key.toInt() );
   }
DbgLv(1) << "BufN:SL: init_buffer   lw_allcomps rebuilt";

   // Coming (back) into New panel, all should be cleared
   lw_bufcomps->clear();
   le_descrip ->setText( "" );
   le_concen  ->setText( "" );
   le_density ->setText( "" );
   le_viscos  ->setText( "" );
   le_ph      ->setText( "7.0000" );
   le_compress->setText( "0.0000e+0" );
   pb_accept  ->setEnabled( false );
   pb_spectrum->setEnabled( false );

   buffer->person       = "";
   buffer->bufferID     = "";
   buffer->GUID         = "";
   buffer->description  = "";
   buffer->component    .clear();
   buffer->componentIDs .clear();
   buffer->concentration.clear();
   buffer->extinction.clear();
}

// Slot to capture new buffer description
void US_BufferGuiNew::new_description()
{
DbgLv(1) << "BufN:SL: new_description()";
   buffer->description = le_descrip->text();
DbgLv(1) << "BufN:SL: new_desc:" << buffer->description;

   bool can_accept = ( !le_descrip->text().isEmpty()  &&
                       !le_density->text().isEmpty()  &&
                       !le_viscos ->text().isEmpty() );
   pb_accept  ->setEnabled( can_accept );
   pb_spectrum  ->setEnabled( can_accept );
}

// Slot for entry of concentration to complete add-component
void US_BufferGuiNew::add_component()
{
DbgLv(1) << "BufN:SL: add_component()";
   double concen   = le_concen->text().toDouble();

   le_concen->setText( "" );

   if ( concen == 0.0 )
      return;

   // Get selected component
   QListWidgetItem* item    = lw_allcomps->currentItem();
   QString compID  = QString::number( item->type() );
   US_BufferComponent bcomp = component_list[ compID ];
DbgLv(1) << "BufN:SL: adco:" << item->text();

   // Verify that concentration is within a valid range
   QString prange = QString( bcomp.range ).section( " ", 0, 0 );
   QString runit  = QString( bcomp.range ).section( " ", 1, 1 ).simplified();
   double rlow    = prange.section( "-", 0, 0 ).toDouble();
   double rhigh   = prange.section( "-", 1, 1 ).toDouble();

   if ( bcomp.unit != runit )
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
DbgLv(1) << "BufN:SL: adco:  runit" << runit << "cunit" << bcomp.unit
 << "rlow rhigh" << rlow << rhigh;
   }

   if ( concen < rlow  ||  concen > rhigh )
   {
      QMessageBox::critical( this,
         tr( "Concentration Out of Range" ),
         tr( "Entered concentration %1 out of component range:\n"
             "  %2 - %3 " ).arg( concen ).arg( rlow ).arg(rhigh )
            + bcomp.unit );
      return;
   }

   // Check if ingredient already exists
   for ( int ii = 0; ii < buffer->component.size(); ii++ )
   {
      if ( bcomp.name == buffer->component[ ii ].name )
      {  // Already a component: just change concentration
         buffer->concentration[ ii ] = concen;
         QString entext  = bcomp.name + "  ("
                           + QString::number( concen )
                           + " " + bcomp.unit + ")";
         lw_bufcomps->item( ii )->setText( entext );

         return;
      }
   }

DbgLv(1) << "BufN:SL: adco:  cname" << bcomp.name << "crange" << bcomp.range
 << "cunit" << bcomp.unit;
   QString entext  = bcomp.name + "  ("
                     + QString::number( concen )
                     + " " + bcomp.unit + ")";
   lw_bufcomps->addItem( entext );

   buffer->concentration << concen;
   buffer->component     << bcomp;
   buffer->componentIDs  << compID;
DbgLv(1) << "BufN:SL: adco:   concen" << concen << "newitem" << entext;

   recalc_density();
   recalc_viscosity();

   bool can_accept = ( !le_descrip->text().isEmpty()  &&
                       !le_density->text().isEmpty()  &&
                       !le_viscos ->text().isEmpty() );
   pb_accept->setEnabled( can_accept );
   pb_spectrum->setEnabled( can_accept );
}

// Slot for select of buffer component
void US_BufferGuiNew::select_bcomp( )
{
   QListWidgetItem* item    = lw_allcomps->currentItem();
   QString compID  = QString::number( item->type() );
   US_BufferComponent bcomp = component_list[ compID ];
   //int bcx         = lw_allcomps->currentRow();
DbgLv(1) << "BufN:SL: select_bcomp()" << item->text();
   QString bctext  = item->text();
DbgLv(1) << "BufN:SL:  selb:  cname" << bcomp.name << "crange" << bcomp.range
 << "cunit" << bcomp.unit;
   QString lbtext  = tr( "Please enter the concentration of\n " )
                     + bcomp.name + tr( " in " ) + bcomp.unit + ":";
   lb_bselect->setText( lbtext );
}

// Slot for double-click of buffer component to remove
void US_BufferGuiNew::remove_bcomp( QListWidgetItem* item )
{
DbgLv(1) << "BufN:SL: remove_bcomp()" << item->text();
}

// Slot to recalculate density based on new component
void US_BufferGuiNew::recalc_density( void )
{
   int bcsize        = buffer->component.size();
   if ( bcsize < 1 )
      return;

   buffer->density   = DENS_20W;
DbgLv(1) << "BufN:SL: recalc_density()" << buffer->component[bcsize-1].name;

   // Iterate over all components in this buffer
   for ( int ii = 0; ii < bcsize; ii++ )
   {
      US_BufferComponent* bc = &buffer->component[ ii ];

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
}

// Slot to recalculate viscosity based on new component
void US_BufferGuiNew::recalc_viscosity( void )
{
   int bcsize        = buffer->component.size();
   if ( bcsize < 1 )
      return;

   buffer->viscosity = VISC_20W;
DbgLv(1) << "BufN:SL: recalc_viscosity()" << buffer->component[bcsize-1].name;

   // Iterate over all components in this buffer
   for ( int ii = 0; ii < bcsize; ii++ )
   {
      US_BufferComponent* bc = &buffer->component[ ii ];

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
}

// Slot for manually changed density
void US_BufferGuiNew::density( void )
{
   buffer->density    = le_density->text().toDouble();
DbgLv(1) << "BufN:SL: density()" << buffer->density;

   bool can_accept = ( !le_descrip->text().isEmpty()  &&
                       !le_density->text().isEmpty()  &&
                       !le_viscos ->text().isEmpty() );
   pb_accept  ->setEnabled( can_accept );
   pb_spectrum  ->setEnabled( can_accept );
}

// Slot for manually changed pH
void US_BufferGuiNew::ph( void )
{
   buffer->pH         = le_ph->text().toDouble();
DbgLv(1) << "BufN:SL: ph()" << buffer->pH;
}

// Slot for manually changed compressibility
void US_BufferGuiNew::compressibility( void )
{
   buffer->compressibility = le_compress->text().toDouble();
DbgLv(1) << "BufN:SL: compressibility()" << buffer->compressibility;
}

// Slot for manually changed viscosity
void US_BufferGuiNew::viscosity( void )
{
   buffer->viscosity  = le_viscos->text().toDouble();
DbgLv(1) << "BufN:SL: viscosity()" << buffer->viscosity;

   bool can_accept = ( !le_descrip->text().isEmpty()  &&
                       !le_density->text().isEmpty()  &&
                       !le_viscos ->text().isEmpty() );
   pb_accept  ->setEnabled( can_accept );
   pb_spectrum  ->setEnabled( can_accept );
}

// Slot for manually changed density
void US_BufferGuiNew::manual_flag( bool is_on )
{
DbgLv(1) << "BufN:SL: manual_flag()" << is_on;
   us_setReadOnly( le_density, ! is_on );
   us_setReadOnly( le_viscos,  ! is_on );
}

// Display a spectrum dialog for list/manage
void US_BufferGuiNew::spectrum_class( void )
{
  US_NewSpectrum *w = new US_NewSpectrum("BUFFER", le_descrip->text(), "", buffer);
 
  w->setParent(this, Qt::Window);
  w->setWindowModality(Qt::WindowModal);
  w->setAttribute(Qt::WA_DeleteOnClose);
  w->show(); 
}


// Slot to cancel edited buffer
void US_BufferGuiNew::newCanceled()
{

  
DbgLv(1) << "BufN:SL: newCanceled()";
   buffer->person       = "";
   buffer->bufferID     = "-1";
   buffer->GUID         = "";
   buffer->description  = "";
   buffer->component    .clear();
   buffer->componentIDs .clear();
   buffer->concentration.clear();

   emit newBufCanceled();
}

// Slot to accept edited buffer
void US_BufferGuiNew::newAccepted()
{
DbgLv(1) << "BufN:SL: newAccepted()";
   buffer->GUID         = US_Util::new_guid();

   
   if ( from_db )
   { // Add buffer to database
      write_db  ();
      DbgLv(1) << "BufN:SL:  newAcc: DB  id" << buffer->bufferID;
      qDebug() << "BufferID: " << buffer->bufferID;
   }

   else
   { // Add buffer to local disk
DbgLv(1) << "BufN:SL:  newAcc: Disk";
      write_disk();
   }

   emit newBufAccepted();
}

// Write new buffer to database
void US_BufferGuiNew::write_db()
{
DbgLv(1) << "BufN:SL: write_db()  bufID" << buffer->bufferID;
   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );
   int idBuf    = buffer->saveToDB( &db, "1" );
   buffer->bufferID = QString::number( idBuf );

   if ( idBuf < 0 )
   {

      QString msg  = tr( "(Return Code = %1 ) " ).arg( idBuf )
                     + db.lastError();
      QMessageBox::critical( this,
          tr( "DB Buffer Write Error" ),
          tr( "Error updating buffer in the database:\n" )
          + msg );
   }
   
   qDebug() << "BufferID: " << buffer->bufferID << ", Description: " << buffer->description << ", Extintcion key #: " << buffer->extinction.keys().count();
   
}

// Write new buffer to local disk file
void US_BufferGuiNew::write_disk()
{
DbgLv(1) << "BufN:SL: write_disk()  bufID" << buffer->GUID;
   QString path     = US_Settings::dataDir() + "/buffers";

   bool    newFile;
   QString filename = US_Buffer::get_filename( path, buffer->GUID, newFile );

   buffer->writeToDisk( filename );
DbgLv(1) << "BufN:SL: write_disk()    newFile" << newFile;
}


// Edit Existing Buffer panel
US_BufferGuiEdit::US_BufferGuiEdit( int *invID, int *select_db_disk,
      US_Buffer *tmp_buffer ) : US_Widgets()
{
  
   buffer      = tmp_buffer;
   orig_buffer = *buffer;
   personID    = invID;
   db_or_disk  = select_db_disk;
   from_db     = ( (*db_or_disk) == 1 );
   dbg_level   = US_Settings::us_debug();

   QGridLayout* main = new QGridLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   QPushButton* pb_cancel   = us_pushbutton( tr( "Cancel" ) );
   pb_accept                = us_pushbutton( tr( "Accept" ) );
   QPushButton* pb_spectrum = us_pushbutton( tr( "Manage Spectrum" ) );
   QPushButton* pb_help     = us_pushbutton( tr( "Help" ) );
   QLabel* bn_modbuf        = us_banner( tr( "Edit an existing buffer" ) );
   QLabel* lb_descrip       = us_label( tr( "Description:" ) );
   QLabel* lb_ph            = us_label( tr( "pH:" ) );
   //QLabel* bn_spacer        = us_banner( "" );
   le_descrip    = us_lineedit( buffer->description );
   le_ph         = us_lineedit( QString::number( buffer->pH, 'f', 4 ) );

   bn_modbuf->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   //bn_spacer->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   pb_accept->setEnabled( false );
   us_setReadOnly( le_descrip, true );

   int row = 0;
   main->addWidget( bn_modbuf,       row++, 0, 1, 8 );
   main->addWidget( lb_descrip,      row,   0, 1, 1 );
   main->addWidget( le_descrip,      row++, 1, 1, 7 );
   main->addWidget( pb_cancel,       row,   4, 1, 2 );
   main->addWidget( pb_accept,       row++, 6, 1, 2 );
   main->addWidget( lb_ph,           row,   0, 1, 1 );
   main->addWidget( le_ph,           row,   1, 1, 3 );
   main->addWidget( pb_spectrum,     row,   4, 1, 2 );
   main->addWidget( pb_help,         row++, 6, 1, 2 );

   QLabel *empty = us_banner ("");
   main->addWidget( empty,           row,   0, 6, 8 );


   connect( le_ph,       SIGNAL( editingFinished() ), 
            this,        SLOT  ( ph             () ) );
   // connect( pb_spectrum, SIGNAL( clicked()  ),
   //          this,        SLOT  ( spectrum() ) );
   connect( pb_spectrum, SIGNAL( clicked()  ),
            this,        SLOT  ( spectrum_class() ) );
   connect( pb_help,     SIGNAL( clicked()  ),
            this,        SLOT  ( help()     ) );
   connect( pb_cancel,   SIGNAL( clicked()      ),
            this,        SLOT  ( editCanceled() ) );
   connect( pb_accept,   SIGNAL( clicked()      ),
            this,        SLOT  ( editAccepted() ) );
}

// Initialize buffer settings, possibly after re-entry to Edit panel
void US_BufferGuiEdit::init_buffer( void )
{
   from_db     = ( (*db_or_disk) == 1 );
   orig_buffer = *buffer;
DbgLv(1) << "BufE: init: bufGUIDs" << buffer->GUID << orig_buffer.GUID;

   le_descrip ->setText( buffer->description );
   le_ph      ->setText( QString::number( buffer->pH ) );
   pb_accept  ->setEnabled( false );

   edit_buffer_description = le_descrip->text();
}

// Slot for manually changed pH
void US_BufferGuiEdit::ph()
{
   buffer->pH         = le_ph->text().toDouble();
DbgLv(1) << "BufE:SL:ph()" << buffer->pH;

   pb_accept->setEnabled( !le_descrip->text().isEmpty() );
}


// Initialize analyte settings, possibly after re-entry to Edit panel
void US_BufferGuiEdit::spectrum_class( void )
{
  QString ifexists;
  if (buffer->extinction.isEmpty())
    ifexists = "NEW";
  else
    ifexists = "EXISTS";
  
  US_EditSpectrum *w = new US_EditSpectrum("BUFFER", ifexists, le_descrip->text(), "1.000", buffer);
  
  connect( w,     SIGNAL( change_spectrum( void ) ),
	   this,  SLOT ( change_spectrum( void ) ) );
  connect( w,     SIGNAL( accept_enable( void ) ),
	   this,  SLOT ( accept_enable( void ) ) );
  
  w->setParent(this, Qt::Window);
  w->setWindowModality(Qt::WindowModal);
  w->setAttribute(Qt::WA_DeleteOnClose);
  w->show(); 
}

void US_BufferGuiEdit::change_spectrum( void )
{
  emit editBufAccepted();
}

void US_BufferGuiEdit::accept_enable( void )
{
  pb_accept->setEnabled( !le_descrip->text().isEmpty() );
}


// Slot to cancel edited buffer
void US_BufferGuiEdit::editCanceled()
{
DbgLv(1) << "BufE:SL: editCanceled()  origbufGUID" << orig_buffer.GUID;
   *buffer     = orig_buffer;
   emit editBufCanceled();
}

// Slot to accept edited buffer
void US_BufferGuiEdit::editAccepted()
{
DbgLv(1) << "BufE:SL: editAccepted()  bufGUID" << buffer->GUID;
   if ( from_db )
   { // Update buffer in database
      write_db  ();
DbgLv(1) << "BufE:SL:  edtAcc: DB  id" << buffer->bufferID;
   }

   else
   { // Update buffer on local disk
DbgLv(1) << "BufE:SL:  edtAcc: Disk";
      write_disk();
   }

   emit editBufAccepted();
}

// Write updated buffer to database
void US_BufferGuiEdit::write_db()
{
DbgLv(1) << "BufE:SL: write_db()  bufID" << buffer->bufferID;
   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );
   int idBuf    = buffer->saveToDB( &db, "1" );

   if ( idBuf < 0 )
   {

      QString msg  = tr( "(Return Code = %1 ) " ).arg( idBuf )
                     + db.lastError();
      QMessageBox::critical( this,
          tr( "DB Buffer Write Error" ),
          tr( "Error updating buffer in the database:\n" )
          + msg );
   }
}

// Write updated buffer to local disk file
void US_BufferGuiEdit::write_disk()
{
DbgLv(1) << "BufE:SL: write_disk()  GUID" << buffer->GUID;
   QString path     = US_Settings::dataDir() + "/buffers";

   bool    newFile;
   QString filename = US_Buffer::get_filename( path, buffer->GUID, newFile );

   buffer->writeToDisk( filename );
DbgLv(1) << "BufE:SL: write_disk()    newFile" << newFile;
}


// Settings panel
US_BufferGuiSettings::US_BufferGuiSettings( int *invID, int *select_db_disk )
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
   QPushButton* pb_synchdb      = us_pushbutton( tr( "Synchronize buffer"
                                                     " components with DB" ) );
   QPushButton* pb_help         = us_pushbutton( tr( "Help" ) );

   if ( US_Settings::us_inv_level() < 3 )
      pb_investigator->setEnabled( false );

   QString number  = ( (*personID) > 0 )
      ? QString::number( US_Settings::us_inv_ID() ) + ": "
      : "";

   le_investigator = us_lineedit( number + US_Settings::us_inv_name() );
   le_syncstat     = us_lineedit();
   int idb_or_disk = from_db ? US_Disk_DB_Controls::DB
                             : US_Disk_DB_Controls::Disk;
   disk_controls   = new US_Disk_DB_Controls( idb_or_disk );
   QLabel *empty   = us_banner ("");

   lb_DB      ->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   us_setReadOnly( le_investigator, true );
   us_setReadOnly( le_syncstat,     true );
   QPalette stpal  = le_syncstat->palette();
   stpal.setColor( QPalette::Text, Qt::blue );
   le_syncstat->setPalette( stpal );


   int row = 0;
   main->addWidget( lb_DB,           row++, 0, 1, 4 );
   main->addWidget( pb_investigator, row,   0, 1, 1 );
   main->addWidget( le_investigator, row++, 1, 1, 3 );
   main->addLayout( disk_controls,   row,   0, 1, 3 );
   main->addWidget( pb_help,         row++, 3, 1, 1 );
   main->addWidget( pb_synchdb,      row,   0, 1, 2 );
   main->addWidget( le_syncstat,     row++, 2, 1, 2 );
   main->addWidget( empty,           row,   0, 6, 4 );

   connect( disk_controls,   SIGNAL( changed       ( bool ) ),
            this,            SLOT  ( source_changed( bool ) ) );
   connect( pb_investigator, SIGNAL( clicked() ),
            this,            SLOT(   sel_investigator() ) );
   connect( pb_synchdb,      SIGNAL( clicked() ),
            this,            SLOT(   synch_components() ) );
   connect( pb_help,         SIGNAL( clicked() ),
            this,            SLOT  ( help()    ) );
}

// Select a new investigator
void US_BufferGuiSettings::sel_investigator( void )
{
   US_Investigator* inv_dialog = new US_Investigator( true, (*personID) );

   connect( inv_dialog,
            SIGNAL( investigator_accepted( int ) ),
            SLOT  ( assign_investigator  ( int ) ) );

   inv_dialog->exec();
}

// Note and notify of change in db/disk source
void US_BufferGuiSettings::source_changed( bool db )
{
   emit use_db( db );
   // calling class needs to query DB/disk when signal is emitted
   qApp->processEvents();
}

// Assign an investigator after a change
void US_BufferGuiSettings::assign_investigator( int invID )
{
   (*personID) = invID;

   QString number = ( (*personID) > 0 )
   ? QString::number( invID ) + ": "
   : "";

   le_investigator->setText( number + US_Settings::us_inv_name() );
   emit investigator_changed( invID );
}

// Get buffer components from DB and write to */etc/bufferComponents.xml
void US_BufferGuiSettings::synch_components( void )
{
   US_Passwd pw;
   QMap< QString, US_BufferComponent > component_list;
   qApp->processEvents();
DbgLv(1) << "setB:synchc read to HD from DB";

   // Read components from DB, write to HD
   component_list.clear();
   US_BufferComponent::getAllFromDB( pw.getPasswd(), component_list );
   US_BufferComponent::putAllToHD  ( component_list );
DbgLv(1) << "setB:synchc   synch complete:  components:"
 << component_list.size();

   le_syncstat->setText( tr( "Local buffer components synchronized." ) );
}


// Main Buffer window with panels
US_BufferGui::US_BufferGui( bool signal_wanted, const US_Buffer& buf,
      int select_db_disk) : US_WidgetsDialog( 0, 0 ),
      signal( signal_wanted ), buffer( buf )
{
   personID    = US_Settings::us_inv_ID();
   buffer      = buf;
   orig_buffer = buf;
   disk_or_db  = ( select_db_disk == US_Disk_DB_Controls::Default )
                 ?  US_Settings::default_data_location()
                 : select_db_disk;
   dbg_level  = US_Settings::us_debug();

   setWindowTitle( tr( "Buffer Management" ) );
   setPalette( US_GuiSettings::frameColor() );
   this->setMinimumSize( 640, 480 );

   QGridLayout* main = new QGridLayout(this);
   main->setSpacing(2);
   main->setContentsMargins(2, 2, 2, 2);

   tabWidget   = us_tabwidget();
   selectTab   = new US_BufferGuiSelect  ( &personID, &disk_or_db, &buffer );
   newTab      = new US_BufferGuiNew     ( &personID, &disk_or_db, &buffer );
   editTab     = new US_BufferGuiEdit    ( &personID, &disk_or_db, &buffer );
   settingsTab = new US_BufferGuiSettings( &personID, &disk_or_db );
   tabWidget -> addTab( selectTab,   tr( "Select Buffer" ) );
   tabWidget -> addTab( newTab,      tr( "Enter New Buffer" ) );
   tabWidget -> addTab( editTab,     tr( "Edit Existing Buffer" ) );
   tabWidget -> addTab( settingsTab, tr( "Settings" ) );

   main->addWidget( tabWidget );

   connect( tabWidget,   SIGNAL( currentChanged(       int  ) ),
            this,        SLOT (  checkTab(             int  ) ) );
   connect( selectTab,   SIGNAL( bufferAccepted(       void ) ),
            this,        SLOT (  bufferAccepted(       void ) ) );
   connect( selectTab,   SIGNAL( selectionCanceled(    void ) ),
            this,        SLOT (  bufferRejected(       void ) ) );
   connect( newTab,      SIGNAL( newBufAccepted(       void ) ),
            this,        SLOT (  newBufAccepted(       void ) ) );
   connect( newTab,      SIGNAL( newBufCanceled(       void ) ),
            this,        SLOT (  newBufCanceled(       void ) ) );
   connect( editTab,     SIGNAL( editBufAccepted(      void ) ),
            this,        SLOT (  editBufAccepted(      void ) ) );
   connect( editTab,     SIGNAL( editBufCanceled(      void ) ),
            this,        SLOT (  editBufCanceled(      void ) ) );
   connect( settingsTab, SIGNAL( use_db(               bool ) ),
            this,        SLOT (  update_disk_or_db(    bool ) ) );
   connect( settingsTab, SIGNAL( investigator_changed( int  ) ),
            this,        SLOT (  update_personID(      int  ) ) );
   
}

// React to a change in panel
void US_BufferGui::checkTab( int currentTab )
{
   // Need to re-read the database or disk or to make other adjustments
   // in case relevant changes were made elsewhere
   if ( currentTab == 0 )
   {
     selectTab  ->init_buffer();
   }
   else if ( currentTab == 1 )
     {
     newTab     ->init_buffer();
   }
   else if ( currentTab == 2 )
   {
     // Check if buffer is selected in the Select Buffer
     editTab    ->init_buffer();
     if (editTab->edit_buffer_description == "")
       {
	 QMessageBox::information( this,
	 tr( "WARNING" ),
	 tr( "No Buffer selected! Please select Buffer from the list" ) );
 
	 emit editBufAccepted();
	 
       }
   }
}

// Make global setting for change in db/disk in Settings panel
void US_BufferGui::update_disk_or_db( bool choice )
{
   (choice) ? (disk_or_db = 1 ) : (disk_or_db = 0 );
}

// Global person ID after Settings panel change
void US_BufferGui::update_personID( int ID )
{
   personID = ID;
}

// Slot for Edit panel buffer-accepted
void US_BufferGui::editBufAccepted( void )
{
DbgLv(1) << "main: editBufAccepted  bGUID" << buffer.GUID;
   tabWidget->setCurrentIndex( 0 );
}

// Slot for Edit panel buffer-changes-rejected
void US_BufferGui::editBufCanceled( void )
{
DbgLv(1) << "main: editBufCanceled  bGUID" << buffer.GUID;
   tabWidget->setCurrentIndex( 0 );
}

// Slot for New panel buffer accepted
void US_BufferGui::newBufAccepted( void )
{
DbgLv(1) << "main: newBufAccepted  bGUID" << buffer.GUID;
   tabWidget->setCurrentIndex( 0 );
}

// Slot for New panel buffer add rejected
void US_BufferGui::newBufCanceled( void )
{
DbgLv(1) << "main: newBufCanceled  bGUID" << buffer.GUID;
   tabWidget->setCurrentIndex( 0 );
}

// Exit and signal caller that changes and selected were accepted
void US_BufferGui::bufferAccepted( void )
{
   valueChanged      ( buffer.density, buffer.viscosity );
   emit valueChanged ( buffer );
   emit valueBufferID( buffer.bufferID );
   accept();
}

// Exit and signal caller that buffer selection/changes were rejected
void US_BufferGui::bufferRejected( void )
{
   buffer   = orig_buffer;
   reject();
}

