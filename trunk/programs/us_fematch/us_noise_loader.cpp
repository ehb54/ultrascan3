//! \file us_noise_loader.cpp

#include "us_noise_loader.h"
#include "us_editor_gui.h"
#include "us_settings.h"
#include "us_gui_settings.h"

#include <qwt_legend.h>

// constructor:  residuals bitmap widget
US_NoiseLoader::US_NoiseLoader( US_DB2* a_db, QStringList& a_mieGUIDs,
   QStringList& a_nieGUIDs, US_Noise& a_ti_noise, US_Noise& a_ri_noise )
   : US_WidgetsDialog( 0, 0 ), db( a_db ), mieGUIDs( a_mieGUIDs ),
   nieGUIDs( a_nieGUIDs ), ti_noise( a_ti_noise ), ri_noise( a_ri_noise )
{
qDebug() << "NL: db=" << ( ( db == (US_DB2*)0 ) ? "(local)" : "(DB)" );
qDebug() << " mieGUIDs size" << mieGUIDs.size();
qDebug() << " nieGUIDs size" << nieGUIDs.size();
qDebug() << " ti_noise count" << ti_noise.count;
   setWindowTitle( tr( "Noise Vector Load" ) );
   setPalette( US_GuiSettings::frameColor() );

   mainLayout        = new QVBoxLayout( this );
   btnsLayout        = new QHBoxLayout();
   mainLayout->setSpacing        ( 2 );
   mainLayout->setContentsMargins( 2, 2, 2, 2 );

   QString hmsg     = tr( "Double-click on a tree noise entry to add it\n" )
                    + tr( "   to the list below of selected noise records.\n" )
                    + tr( "Double-click on a selected-noise list entry\n" )
                    + tr( "   to remove it from the selected list.\n\n" )
                    + tr( "You may select one TI and one RI noise record.\n\n" )
                    + tr( "Click the \"Load\" button to load selected noise\n" )
                    + tr( "    or the \"Cancel\" button to select no noise.\n" )
                    + tr( "Click \"Details\" for noise entry details." );
   //QLabel* lb_help  = us_label( hmsg );
   QLabel* lb_help  = us_banner( hmsg );
   lb_help->setAlignment(  Qt::AlignVCenter | Qt::AlignLeft );

   QLabel* lb_arrow = us_banner( "Selected Noise Vectors" );
   lb_arrow->setAlignment( Qt::AlignVCenter | Qt::AlignHCenter );

   QFont font( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() );
   QFontMetrics fm( font );
   int fontHeight = fm.lineSpacing();

   tv_noises        = new QTreeWidget( this );
   tv_noises->setFrameStyle( QFrame::NoFrame );
   tv_noises->setPalette( US_GuiSettings::frameColor() );
   tv_noises->setFont(    font );

   lw_selects       = us_listwidget();
   lw_selects->setMaximumHeight( fontHeight * 2 + 12 );

   mainLayout->addWidget( lb_help );
   mainLayout->addWidget( tv_noises );
   mainLayout->addWidget( lb_arrow );
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
   QString          twtitle  = QString( "Models-in-Edit/Noises-in-Model" );
   QString          mititle  = QString( "Loaded Model" );
   QList< QTreeWidgetItem* > items;
   tv_noises->setColumnCount( 1 );
   tv_noises->setHeaderLabel( twtitle );
   QTreeWidgetItem* twi_curr = 
      new QTreeWidgetItem( twi_null, QStringList( mititle ) );
   items.append( twi_curr );

   for ( int ii = 1; ii < mieGUIDs.size(); ii++ )
   {
      mititle    = QString().sprintf( "Model Sibling %4.4d", ii );
      twi_curr   = new QTreeWidgetItem( twi_null, QStringList( mititle ) );
qDebug() << "NL: ii mititle" << ii << mititle;
      items.append( twi_curr );
   }

   tv_noises->addTopLevelItems( items );

   for ( int ii = 0; ii < nieGUIDs.size(); ii++ )
   {
      QString nie   = nieGUIDs.at( ii );
      QString typ   = nie.section( ":", 1, 1 );
      QString mdx   = nie.section( ":", 2, 2 );
              nie   = typ + "_noise " + mdx;
      int     ndx   = mdx.toInt();
qDebug() << "NL:   ii ndx nie" << ii << ndx << nie;

      twi_curr      = new QTreeWidgetItem( QStringList( nie ) );
      items.at( ndx )->addChild( twi_curr );
   }

   if ( nieGUIDs.at( 0 ).section( ":", 2, 2 ).toInt() == 0 )
   {
      tv_noises->expandItem( items.at( 0 ) );
   }

   connect( tv_noises,  SIGNAL( itemDoubleClicked( QTreeWidgetItem*, int ) ),
            this,       SLOT(   itemSelect(        QTreeWidgetItem*, int ) ) );
   connect( lw_selects, SIGNAL( doubleClicked( const QModelIndex& ) ),
            this,       SLOT(   itemDeselect(  const QModelIndex& ) ) );
   connect( pb_detail,  SIGNAL( clicked()      ),
            this,       SLOT(   view_details() ) );
   connect( pb_cancel,  SIGNAL( clicked()      ),
            this,       SLOT(   cancelled()    ) );
   connect( pb_load,    SIGNAL( clicked()      ),
            this,       SLOT(   selected()     ) );
}

US_NoiseLoader::~US_NoiseLoader( )
{
qDebug() << "NL:DESTRUCT";
   if ( tv_noises )
      tv_noises->clear();
}

// move selected item to selects list
void US_NoiseLoader::itemSelect( QTreeWidgetItem* item, int )
{
   QString itemtext = item->text( 0 );
qDebug() << "NL: itemSelect" << itemtext;
   QString itemtype = itemtext.left( 2 );

   if ( itemtype != "ti"  &&  itemtype != "ri" )
      return;     // ignore select if it was not a noise line

   int nsels        = lw_selects->count();
   
   if ( nsels == 0 )
   {  // if no selects yet, just add item
      lw_selects->addItem( itemtext );
   }

   else
   {  // otherwise, replace anything of same type
      QStringList selects;
      QString     temptext;
      int ricount = 0;
      int ticount = 0;
      int secount = lw_selects->count();

      for ( int ii = 0; ii < secount; ii++ )
      {
         temptext = lw_selects->item( ii )->text();

         selects << temptext;

         if ( temptext.startsWith( "ti", Qt::CaseSensitive ) )
            ticount++;

         else
            ricount++;

      }

      if ( itemtype == "ti" )
      {  // item to add is "ti_*"
         if ( ticount == 0 )
         {  // there wasn't one before, so insert the new one
            lw_selects->insertItem( 0, itemtext );
         }

         else
         {  // there was one before, replace 1st in list (ti position)
            selects.replace( 0, itemtext );
            lw_selects->clear();
            lw_selects->addItems( selects );
         }
      }

      else
      {  // item to add is "ri_*"
         if ( ricount == 0 )
         {  // there wasn't one before, so add new one to end
            lw_selects->addItem( itemtext );
         }

         else
         {  // there was one before, replace 2nd in list (ti position)
            selects.replace( secount - 1, itemtext );
            lw_selects->clear();
            lw_selects->addItems( selects );
         }
      }
   }
}

// clear select list of item chosen
void US_NoiseLoader::itemDeselect( const QModelIndex& mx )
{
   int sx    = mx.row();
   int nrows = lw_selects->count();
qDebug() << "NL: itemDeselect row" << lw_selects->item( sx )->text();

   if ( nrows == 1 )
   {  // if there was only 1 item, list will now be clear
      lw_selects->clear();
   }

   else
   {  // otherwise, rebuild list with only other item
      QString keepitem = lw_selects->item( 1 - sx )->text();
      lw_selects->clear();
      lw_selects->addItem( keepitem );
   }

}

void US_NoiseLoader::close_all()
{
   if ( tv_noises )
   {
      tv_noises->clear();
   }

   tv_noises = (QTreeWidget*)0;

   this->close();
}

void US_NoiseLoader::cancelled()
{
   close_all();
}

void US_NoiseLoader::selected()
{
   bool isDB = ( db != (US_DB2*)0 );

   for ( int ii = 0; ii < lw_selects->count(); ii++ )
   {
      QString selntext = lw_selects->item( ii )->text();
      QString snumtext = selntext.section( " ", 1, 1 );
      QString styptext = selntext.left( 2 );
      QString noisGUID = "";

      for ( int jj = 0; jj < nieGUIDs.size(); jj++ )
      {
         QString nie   = nieGUIDs.at( jj );
         QString typ   = nie.section( ":", 1, 1 );
         QString mdx   = nie.section( ":", 2, 2 );
         if ( typ == styptext  &&   mdx == snumtext )
         {
            noisGUID      = nie.section( ":", 0, 0 );
            break;
         }
      }

      if ( styptext == "ti" )
         ti_noise.load( isDB, noisGUID, db );

      else
         ri_noise.load( isDB, noisGUID, db );

   }
qDebug() << "NL: ti_noise count" << ti_noise.count;
qDebug() << "NL: ri_noise count" << ri_noise.count;

   close_all();
}

void US_NoiseLoader::view_details()
{
qDebug() << "NL: view_details";
   US_Noise noise;
   bool     isDB   = ( db != (US_DB2*)0 );
   QString  mtxt;

   // build noise details text
   mtxt  = tr( "All models derive from the common loaded Edit.\n" )
         + tr( "All noise vector records derive from the loaded Model\n" )
         + tr( "  or from siblings of that model.\n" )
         + tr( "Details for noise vector records follow.\n" );

   for ( int ii = 0; ii < nieGUIDs.size(); ii++ )
   {  // add text for each noise vector entry
      QString nie      = nieGUIDs.at( ii );
      QString typ      = nie.section( ":", 1, 1 );
      QString mdx      = nie.section( ":", 2, 2 );
      QString noisGUID = nie.section( ":", 0, 0 );

      QString typedesc = tr( "Time-Invariant" );
      if ( typ == "ri" )
         typedesc      = tr( "Radius-Invariant" );

      noise.load( isDB, noisGUID, db );

      mtxt  += tr( "\nNoise record \"" ) + typ + "_noise " + mdx + "\":";

      if ( mdx.toInt() == 0 )
      {
         mtxt  += tr( "  (from Loaded Model):\n" );
      }

      else
      {
         mtxt  += tr( "  (from Model Sibling):\n" );
      }

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
   detaild->resize( 600, 500 );
   detaild->editor->e->setFont( QFont("monospace",US_GuiSettings::fontSize()) );
   detaild->editor->e->setText( mtxt );
   detaild->exec();
   qApp->processEvents();
}

