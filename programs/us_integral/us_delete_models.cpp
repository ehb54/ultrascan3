//! \file us_delete_models.cpp

#include "us_delete_models.h"
#include "us_gui_settings.h"

// Constructor:  remove-distributions dialog widget
US_DeleteModels::US_DeleteModels( QVector< DisSys >& adistros,
    QWidget* p ) : US_WidgetsDialog( p, Qt::WindowFlags() ), distros( adistros )
{
   setObjectName( "US_DeleteModels" );
   setPalette( US_GuiSettings::frameColor() );
   setFont( QFont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() ) );

   // Lay out the GUI
   setWindowTitle( tr( "Remove Model Distributions" ) );

   mainLayout      = new QGridLayout( this );

   mainLayout->setSpacing        ( 2 );
   mainLayout->setContentsMargins( 2, 2, 2, 2 );

   QLabel* lb_mtitle = us_banner( tr( "Select Models to Remove"         ) );
   lw_distrs         = us_listwidget();

   pb_restore   = us_pushbutton( tr( "Restore" ) );
   pb_remove    = us_pushbutton( tr( "Remove"  ) );
   pb_help      = us_pushbutton( tr( "Help"    ) );
   pb_cancel    = us_pushbutton( tr( "Cancel"  ) );
   pb_accept    = us_pushbutton( tr( "Accept"  ) );

   te_status    = us_textedit();

   int row      = 0;
   mainLayout ->addWidget( lb_mtitle,  row++, 0, 1, 5 );
   mainLayout ->addWidget( lw_distrs,  row,   0, 9, 5 );
   row         += 9;
   mainLayout ->addWidget( pb_remove,  row,   0, 1, 1 );
   mainLayout ->addWidget( pb_restore, row,   1, 1, 1 );
   mainLayout ->addWidget( pb_help,    row,   2, 1, 1 );
   mainLayout ->addWidget( pb_cancel,  row,   3, 1, 1 );
   mainLayout ->addWidget( pb_accept,  row++, 4, 1, 1 );
   mainLayout ->addWidget( te_status,  row,   0, 2, 5 );
   row         += 2;

   connect( pb_remove,  SIGNAL( clicked()  ),
            this,       SLOT(   remove()   ) );
   connect( pb_restore, SIGNAL( clicked()  ),
            this,       SLOT(   restore()  ) );
   connect( pb_help,    SIGNAL( clicked()  ),
            this,       SLOT(   help()     ) );
   connect( pb_cancel,  SIGNAL( clicked()  ),
            this,       SLOT(   reject()   ) );
   connect( pb_accept,  SIGNAL( clicked()  ),
            this,       SLOT(   accepted() ) );

   connect( lw_distrs,  SIGNAL( itemSelectionChanged() ),
            this,       SLOT(   selectionsChanged()    ) );

   // Build list of original model descriptions
   nd_orig      = distros.count();
   int maxdlen  = 0;

   for ( int jj = 0; jj < nd_orig; jj++ )
   {
      QString runid  = distros[ jj ].run_name;
      QString analy  = distros[ jj ].analys_name;
      QString method = distros[ jj ].method;
      QString edid   = analy.section( "_",  0, -4 );
      QString anid   = analy.section( "_", -3, -3 );
      QString iter   = analy.section( "_", -2, -1 );
      QString mdesc  = runid + "." + edid + "_" + anid + "_" + method
         + "_" + iter;

      mdesc_orig << mdesc;

      maxdlen        = qMax( maxdlen, mdesc.length() );
   }

   // Set up the list widget with the original models
   restore();

   QFont font( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() );
   QFontMetrics fm( font );
   int fhigh = fm.lineSpacing();
   int fwide = fm.width( QChar( '6' ) );
   int lhigh = fhigh * 10 + 12;
   int lwide = fwide * ( maxdlen + 2 );

   lw_distrs->setMinimumHeight( lhigh );
   lw_distrs->resize( lwide, lhigh );
   lw_distrs->setSelectionMode( QAbstractItemView::ExtendedSelection );

   te_status->setMaximumHeight( fhigh * 2 + 12 );
   us_setReadOnly( te_status, true );
   te_status->setTextColor( Qt::blue );
   te_status->setText(
      tr( "The list shows all the original %1 distributions.\n"
          "No models are currently selected for removal." ).arg( nd_orig ) );

   adjustSize();
   int wwide = qMax( 500, lwide );
   int whigh = size().height();
   resize( wwide, whigh );
   qApp->processEvents();
}

// Private slot to react to a change in selections
void US_DeleteModels::selectionsChanged()
{
   // Get the list of selected items
   QList< QListWidgetItem* > selitems = lw_distrs->selectedItems();
   nd_selected = selitems.count();

   if ( nd_removed == 0 )
   {  // No list entries have thus-far been removed
      if ( nd_selected == 0 )
      {  // No items removed or selected
         te_status->setText(
            tr( "The list shows all the original %1 distributions.\n"
                "No models are currently selected for removal." )
            .arg( nd_orig ) );
      }

      else if ( nd_selected == 1 )
      {  // No items removed, but 1 selected
         te_status->setText(
            tr( "The list shows all the original %1 distributions.\n"
                "One model is currently selected for removal." )
            .arg( nd_orig ) );
      }

      else
      {  // No items removed, but several selected
         te_status->setText(
            tr( "The list shows all the original %1 distributions.\n"
                "%2 models are currently selected for removal." )
            .arg( nd_orig ).arg( nd_selected ) );
      }
   }

   else
   {  // Some items were previously removed
      if ( nd_selected == 0 )
      {  // Removals were done, but currently no items selected
         te_status->setText(
            tr( "The list shows distributions with %1 removed.\n"
                "No more models are currently selected for removal." )
            .arg( nd_removed ) );
      }

      else if ( nd_selected == 1 )
      {  // Removals were done, and currently one more item is selected
         te_status->setText(
            tr( "The list shows distributions with %1 removed.\n"
                "One more model is currently selected for removal." )
            .arg( nd_removed ) );
      }

      else
      {  // Removals were done, and currently some more items are selected
         te_status->setText(
            tr( "The list shows distributions with %1 removed.\n"
                "%2 more models are currently selected for removal." )
            .arg( nd_removed ).arg( nd_selected ) );
      }
   }

   // Enable buttons according to the present state of selection/removal
   pb_remove ->setEnabled( nd_selected > 0 );
   pb_restore->setEnabled( nd_removed  > 0 );
   pb_accept ->setEnabled( nd_removed  > 0 );
}

// Private slot to remove selected distributions from the displayed list
void US_DeleteModels::remove()
{
   // Get the list-widget items currently selected
   QList< QListWidgetItem* > selitems = lw_distrs->selectedItems();
   nd_selected  = selitems.count();
   nd_removed  += nd_selected;

   for ( int jj = 0; jj < nd_selected; jj++ )
   {  // Get a selected item and remove its description from the current list
      QListWidgetItem* item = selitems[ jj ];
      QString          mdesc = item->text();

      mdesc_list.removeOne( mdesc );
   }

   // Re-do the displayed list and enable buttons according to present state
   lw_distrs->clear();
   lw_distrs->addItems( mdesc_list );
   pb_remove ->setEnabled( false );
   pb_restore->setEnabled( true  );
   pb_accept ->setEnabled( true  );
   nd_selected  = 0;
}

// Private slot to restore the original list of distributions
void US_DeleteModels::restore()
{
   mdesc_list  = mdesc_orig;
   nd_orig     = mdesc_orig.count();
   nd_removed  = 0;
   nd_selected = 0;

   lw_distrs->clear();
   lw_distrs->addItems( mdesc_orig );
   te_status->setText(
      tr( "The list shows all the original %1 distributions.\n"
          "No models are currently selected for removal." ).arg( nd_orig ) );

   pb_remove ->setEnabled( false );
   pb_restore->setEnabled( false );
   pb_accept ->setEnabled( false );
}

// Private slot to do the actual removal of distributions and close
void US_DeleteModels::accepted()
{
   if ( nd_selected > 0 )
   {  // Accept attempt with selections and no Remove clicked
      if ( QMessageBox::Yes == QMessageBox::warning( this,
              tr( "Outstanding Selections" ),
              tr( "You have selected distributions,\n"
                  "but did not click on the Remove button.\n"
                  "Do you want to remove the selected distributions?" ),
              QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes ) )
      {  // If "Yes" to above, add current selections to the remove list
         remove();
      }
   }

   for ( int jj = nd_orig - 1; jj >= 0; jj-- )
   {  // Remove all entries in the original that are not in the current list
      QString mdesc = mdesc_orig[ jj ];

      if ( ! mdesc_list.contains( mdesc ) )
      {  // This original list item is to be removed from the passed list
         distros.removeAt( jj );
      }
   }

   accept();
}

