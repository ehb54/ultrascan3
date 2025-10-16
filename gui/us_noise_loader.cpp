//! \file us_noise_loader.cpp

#include "us_noise_loader.h"
#include "us_editor_gui.h"
#include "us_settings.h"
#include "us_gui_settings.h"

#include <qwt_legend.h>

// constructor:  load loader dialog
US_NoiseLoader::US_NoiseLoader( IUS_DB2* db, QStringList& mieGUIDs,
   QStringList& nieGUIDs, US_Noise& ti_noise, US_Noise& ri_noise,
   US_DataIO::EditedData* edata )
   : US_WidgetsDialog( 0, 0 ), db( db ), mieGUIDs( mieGUIDs ),
   nieGUIDs( nieGUIDs ), ti_noise( ti_noise ), ri_noise( ri_noise )
{
   setWindowTitle( ( db == (US_DB2*) 0 ?
                    tr( "Noise Vector Local Disk Load" ) :
                    tr( "Noise Vector Database Load" ) ) );
   setPalette( US_GuiSettings::frameColor() );

   mainLayout        = new QVBoxLayout( this );
   btnsLayout        = new QHBoxLayout();
   mainLayout->setSpacing        ( 2 );
   mainLayout->setContentsMargins( 2, 2, 2, 2 );

   QString hmsg     = tr( "You may select multiple noise records.\n\n" )
                    + tr( "Click on a tree noise entry to add it\n" )
                    + tr( "   to the list below of selected noise records.\n" )
                    + tr( "Click on a top-level entry to add all\n" )
                    + tr( "   of its children to the selected list.\n" )
                    + tr( "Ctrl-click on multiple noise entries\n" )
                    + tr( "   to add them to the selected list.\n\n" )
                    + tr( "Click the \"Load\" button to load selected noise\n" )
                    + tr( "    or the \"Cancel\" button to select no noise.\n" )
                    + tr( "Click \"Details\" for noise entry details." );

   QLabel* lb_noises  = us_banner( hmsg );
   lb_noises->setAlignment(  Qt::AlignVCenter | Qt::AlignLeft );

   QLabel* lb_selects = us_banner( "Selected Noise Vectors" );
   lb_selects->setAlignment( Qt::AlignVCenter | Qt::AlignHCenter );

   QFont font( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() );
   QFontMetrics fm( font );
   int fontHeight = fm.lineSpacing();

   tw_noises        = new QTreeWidget( this );
   tw_noises->setFrameStyle( QFrame::NoFrame );
   tw_noises->setPalette( US_GuiSettings::editColor() );
   tw_noises->setFont(    font );
   tw_noises->setSelectionMode( QAbstractItemView::ExtendedSelection );

   lw_selects       = us_listwidget();
   lw_selects->setMaximumHeight( fontHeight * 3 + 12 );

   mainLayout->addWidget( lb_noises );
   mainLayout->addWidget( tw_noises );
   mainLayout->addWidget( lb_selects );
   mainLayout->addWidget( lw_selects );

   QPushButton* pb_detail = us_pushbutton( tr( "Details" )   );
   QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
   QPushButton* pb_load   = us_pushbutton( tr( "Load" )   );
   btnsLayout->addWidget( pb_detail );
   btnsLayout->addWidget( pb_cancel );
   btnsLayout->addWidget( pb_load   );
   mainLayout->addLayout( btnsLayout );

   ti_noise.count   = 0;
   ri_noise.count   = 0;

   // populate the tree widget
   QTreeWidgetItem* twi_null = (QTreeWidgetItem*)0;
   QString          twtitle  = QString( "Models-in-Edit / Noises-in-Model" );
   QString          mititle  = QString( "Latest/Loaded Model" );
   QList< QTreeWidgetItem* > items;
   tw_noises->setColumnCount( 1 );
   tw_noises->setHeaderLabel( twtitle );

   QTreeWidgetItem* twi_curr = 
      new QTreeWidgetItem( twi_null, QStringList( mititle ) );
   items.append( twi_curr );      // first top-level item in tree

   for ( int ii = 1; ii < mieGUIDs.size(); ii++ )
   {  // complete models list for top level
      mititle    = QString::asprintf( "Model Sibling %4.4d", ii );
      twi_curr   = new QTreeWidgetItem( twi_null, QStringList( mititle ) );
      items.append( twi_curr );   // other top-level items are model siblings
   }

   tw_noises->addTopLevelItems( items );  // put the top level in tree GUI

   for ( int ii = 0; ii < nieGUIDs.size(); ii++ )
   {  // review noises-in-edit, adding children
      QString nie   = nieGUIDs.at( ii );         // noiseGUID:type:index in edit
      QString typ   = nie.section( ":", 1, 1 );  // type part ("ti" or "ri")
      QString mdx   = nie.section( ":", 2, 2 );  // model index part ("0001")
              nie   = typ + "_noise " + mdx;     // list name ("ti_noise 0003")
      int     ndx   = mdx.toInt();               // integral model index

      twi_curr      = new QTreeWidgetItem( QStringList( nie ) );
      items.at( ndx )->addChild( twi_curr );     // add as child of model "ndx"
   }

   if ( nieGUIDs.at( 0 ).section( ":", 2, 2 ).toInt() == 0 )
   {  // expand the loaded model tree if present
      tw_noises->expandItem( items.at( 0 ) );
   }

   connect( tw_noises,  SIGNAL( itemSelectionChanged() ),
            this,       SLOT(   itemsSelected()        ) );
   connect( pb_detail,  SIGNAL( clicked()      ),
            this,       SLOT(   view_details() ) );
   connect( pb_cancel,  SIGNAL( clicked()      ),
            this,       SLOT(   cancelled()    ) );
   connect( pb_load,    SIGNAL( clicked()      ),
            this,       SLOT(   selected()     ) );

   // Compute the OD range of the edited data
   if ( edata != NULL )
   {
      int nscans    = edata->scanCount();
      int npoints   = edata->pointCount();
      double datmin = edata->value( 0, 0 );
      double datmax = datmin;

      for ( int ii = 0; ii < nscans; ii++ )
      {
         for ( int jj = 0; jj < npoints; jj++ )
         {
            double datval = edata->value( ii, jj );
            datmin        = qMin( datmin, datval );
            datmax        = qMax( datmax, datval );
         }
      }

      darange = datmax - datmin;
   }

   else
      darange = 0.0;
}

void US_NoiseLoader::itemsSelected( void )
{
   QList< QTreeWidgetItem* > selitems = tw_noises->selectedItems();
   int nsels = selitems.size();
   lw_selects->clear();

   if ( nsels == 0 )
      return;

   for ( int ii = 0; ii < nsels; ii++ )
   {
      QTreeWidgetItem* item = selitems[ ii ];
      QString itemtext = item->text( 0 );

      if ( itemtext.contains( "_noise" ) )
      {  // Select an individual noise
         lw_selects->addItem( itemtext );
      }

      else if ( itemtext.contains( "Model" ) )
      {  // Select all the noises associated with a model
         for ( int jj = 0; jj < item->childCount(); jj++ )
         {
            lw_selects->addItem( item->child( jj )->text( 0 ) );
         }
      }
   }
}

// close with check of need to clear noises
void US_NoiseLoader::close_all()
{
   if ( tw_noises )
   {
      tw_noises->clear();
   }

   tw_noises = (QTreeWidget*)0;

   this->close();
}

// close out after Cancel clicked
void US_NoiseLoader::cancelled()
{
   close_all();
}

// close out after Load clicked: load noise(s)
void US_NoiseLoader::selected()
{
   bool isDB = ( db != (US_DB2*)0 );  // flag DB or Local
   int  nti  = 0;
   int  nri  = 0;
   US_Noise t2_noise;

   for ( int ii = 0; ii < lw_selects->count(); ii++ )
   {  // browse select list
      QString selntext = lw_selects->item( ii )->text();
      QString snumtext = selntext.section( " ", 1, 1 );
      QString styptext = selntext.left( 2 );
      QString noisGUID = "";

      for ( int jj = 0; jj < nieGUIDs.size(); jj++ )
      {  // examine noises-in-edit list
         QString nie   = nieGUIDs.at( jj );
         QString typ   = nie.section( ":", 1, 1 );
         QString mdx   = nie.section( ":", 2, 2 );

         if ( typ == styptext  &&   mdx == snumtext )
         {  // found the selected noise: break with its GUID
            noisGUID      = nie.section( ":", 0, 0 );
            break;
         }
      }

      if ( styptext == "ti" )
      {
         if ( nti == 0 )
            ti_noise.load( isDB, noisGUID, db );    // load ti noise

         else
         {
            t2_noise.load( isDB, noisGUID, db );
            ti_noise.sum_noise( t2_noise, true );   // sum multiple ti noises
         }

         nti++;
      }

      else
      {
         if ( nri == 0 )
            ri_noise.load( isDB, noisGUID, db );    // load ri noise

         else
         {
            t2_noise.load( isDB, noisGUID, db );
            ri_noise.sum_noise( t2_noise, true );   // sum multiple ri noises
         }

         nri++;
      }
   }

   // Verify that noise(s) values not beyond reasonable data range
   if ( darange > 0.0 )
   {
      int nurnois    = 0;

      if ( nti > 0 )
      {
         double noimin  = ti_noise.values[ 0 ];
         double noimax  = noimin;

         for ( int ii = 0; ii < ti_noise.values.size(); ii++ )
         {
            double noival  = ti_noise.values[ ii ];
            noimin         = qMin( noimin, noival );
            noimax         = qMax( noimax, noival );
         }

         if ( ( noimax - noimin ) > darange )  nurnois++;
      }

      if ( nri > 0 )
      {
         double noimin  = ri_noise.values[ 0 ];
         double noimax  = noimin;

         for ( int ii = 0; ii < ri_noise.values.size(); ii++ )
         {
            double noival  = ri_noise.values[ ii ];
            noimin         = qMin( noimin, noival );
            noimax         = qMax( noimax, noival );
         }

         if ( ( noimax - noimin ) > darange )  nurnois++;
      }

      if ( nurnois > 0 )
      {
         QString msg = tr(
            "Noise values exceed the range of experimental data.<br/>"
            "Do you still want to honor the current noise selection(s)?<br/>"
            "<ul><li><b>Yes</b> to proceed, anyway, with selected noise;</li>"
            "<li><b>No </b> to retry with a new noise selection></li></ul>" );
         QMessageBox msgBox     ( this );
         msgBox.setWindowTitle  ( tr( "Noise Value Problems" ) );
         msgBox.setTextFormat   ( Qt::RichText );
         msgBox.setText         ( msg );
         msgBox.addButton       ( QMessageBox::No  );
         msgBox.addButton       ( QMessageBox::Yes );
         msgBox.setDefaultButton( QMessageBox::Yes );

         if ( msgBox.exec() == QMessageBox::No )
         {
            ti_noise.values.clear();
            ri_noise.values.clear();
            return;
         }
      }
   }

   close_all();
}

// top up editor dialog with current noise details
void US_NoiseLoader::view_details()
{
   US_Noise noise;
   bool     isDB   = ( db != (US_DB2*)0 );
   QString  mtxt;

   // build noise details text
   mtxt  = tr( "All models derive from the common loaded Edit.\n"
               "All noise vector records derive from the latest/loaded model\n"
               "  or from common-edit siblings of that model " )
         + ( isDB ? tr( "in the database.\n" ) : tr( "on local disk.\n" ) )
         + tr( "Details for noise vector records follow.\n" );

   for ( int ii = 0; ii < nieGUIDs.size(); ii++ )
   {  // add text for each noise vector entry
      QString nie      = nieGUIDs.at( ii );
      QString typ      = nie.section( ":", 1, 1 );
      QString mdx      = nie.section( ":", 2, 2 );
      QString noisGUID = nie.section( ":", 0, 0 );
      QString typedesc;

      if ( typ == "ti" )
         typedesc      = tr( "Time-Invariant" );

      else
         typedesc      = tr( "Radially-Invariant" );

      noise.load( isDB, noisGUID, db );  // load noise to get details

      mtxt  += tr( "\nNoise record \"" ) + typ + "_noise " + mdx + "\"";

      if ( mdx.toInt() == 0 )
         mtxt  += tr( "  (from Latest/Loaded Model):\n" );

      else
         mtxt  += tr( "  (from Model Sibling):\n" );

      mtxt  += tr( "  Type:           " ) + typedesc          + "\n";
      mtxt  += tr( "  Description:    " ) + noise.description + "\n";
      mtxt  += tr( "  Noise GUID:     " ) + noise.noiseGUID   + "\n";
      mtxt  += tr( "  Model GUID:     " ) + noise.modelGUID   + "\n";
      mtxt  += tr( "  Values Count:   " )
         + QString::number( noise.count ) + "\n";

      if ( typ == "ti" )
      {
         mtxt  += tr( "  Minimum Radius: " )
            + QString::number( noise.minradius ) + "\n";
         mtxt  += tr( "  Maximum Radius: " )
            + QString::number( noise.maxradius ) + "\n";
      }

   }

   // display details text
   US_EditorGui* detaild = new US_EditorGui();
   detaild->setWindowTitle( tr( "Noise Vector Details" ) );
   detaild->move( this->pos() + QPoint( 200, 200 ) );
   detaild->resize( 720, 560 );
   detaild->editor->e->setFont( QFont("monospace",US_GuiSettings::fontSize()) );
   detaild->editor->e->setText( mtxt );
   detaild->exec();
   qApp->processEvents();
}

