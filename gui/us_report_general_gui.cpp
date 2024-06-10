//! \file us_report_general_gui.cpp

#include "us_report_general_gui.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_investigator.h"
#include "us_images.h"


#if QT_VERSION < 0x050000
#define setSymbol(a)      setSymbol(*a)
#define setSamples(a,b,c) setData(a,b,c)
#define QRegularExpression(a)  QRegExp(a)
#endif

US_ReportGenGui::US_ReportGenGui( QString reportMask ) : US_Widgets()
{
  this->reportMask = reportMask;
  abde_mode = false;
  
  QJsonDocument jsonDoc = QJsonDocument::fromJson( reportMask.toUtf8() );
  json = jsonDoc.object();

  topLevelItems = json.keys();
  
  foreach(const QString& key, json.keys())
    {
      QJsonValue value = json.value(key);
      qDebug() << "Key = " << key << ", Value = " << value;//.toString();
      
      if ( key.contains("Solutions") || key.contains("Analysis") )
	{
	   QJsonArray json_array = value.toArray();
	   for (int i=0; i < json_array.size(); ++i )
	     {
	       foreach(const QString& array_key, json_array[i].toObject().keys())
		 {
		   if (  key.contains("Solutions") )
		     {
		       solutionItems      << array_key;
		       solutionItems_vals << json_array[i].toObject().value(array_key).toString(); 
		     }
		   if (  key.contains("Analysis") )
		     {
		       QJsonObject newObj = json_array[i].toObject().value(array_key).toObject();
		       analysisItems << array_key;

		       foreach ( const QString& n_key, newObj.keys() )
			 {
			   if ( array_key.contains("General") )
			     {
			       analysisGenItems << n_key;
			       analysisGenItems_vals << newObj.value( n_key ).toString();
			     }
			   if ( array_key.contains("2DSA") )
			     {
			       analysis2DSAItems << n_key;
			       analysis2DSAItems_vals << newObj.value( n_key ).toString();
			     }
			   if ( array_key.contains("PCSA") ) 
			     {
			       analysisPCSAItems << n_key;
			       analysisPCSAItems_vals << newObj.value( n_key ).toString();
			     }
			 }
		     }
		 }
	     }
	}
    }

  qDebug() << "solutionItems: " << solutionItems;
  qDebug() << "solutionItems_vals: " << solutionItems_vals;

  qDebug() << "analysisItems: " << analysisItems;
  
  qDebug() << "analysisGenItems: " << analysisGenItems;
  qDebug() << "analysisGenItems_vals: " << analysisGenItems_vals;

  qDebug() << "analysis2DSAItems: " << analysis2DSAItems;
  qDebug() << "analysis2DSAItems_vals: " << analysis2DSAItems_vals;

  qDebug() << "analysisPCSAItems: " << analysisPCSAItems;
  qDebug() << "analysisPCSAItems_vals: " << analysisPCSAItems_vals;

  //
  setWindowTitle( tr( "Report General Settings"));

  //setPalette( US_GuiSettings::frameColor() );
  setPalette( US_GuiSettings::normalColor() );
  main  = new QVBoxLayout( this );
  main->setSpacing( 2 );
  main->setContentsMargins( 2, 2, 2, 2 );

  //Top level parameters
  QLabel* bn_detail     = us_banner( QString( tr( "Define Report's Representation of the Protocol Settings: " ) ), 1 );
  bn_detail->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
  main->addWidget( bn_detail );


  row = 0;
  genL           = NULL;
  lower_buttons  = NULL;

  build_layout();
}


//ABDE
void US_ReportGenGui::abde_mode_passed( void )
{
  abde_mode = true;
  build_layout();
}

void US_ReportGenGui::build_layout( void )
{
  qDebug() << "Building Tabular Layout -- ";
  //Clean genL layout first:
  if ( genL != NULL && genL->layout() != NULL )
    {
      QLayoutItem* item;
      while ( ( item = genL->layout()->takeAt( 0 ) ) != NULL )
	{
	  delete item->widget();
	  delete item;
	}
      delete genL;
      delete scrollArea;
    }
  //End cleaning layout
  
  genL        =  new QGridLayout();
  genL        ->setSpacing         ( 2 );
  genL        ->setContentsMargins ( 2, 2, 2, 2 );

  QFont sfont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() );

  // TreeWidget
  treeWidget = new QTreeWidget();
  QStringList theads;
  theads << "Selected" << "Protocol Settings";
  treeWidget->setHeaderLabels( theads );
  treeWidget->setFont( QFont( US_Widgets::fixedFont().family(),
			      US_GuiSettings::fontSize() + 1 ) );
  genL->addWidget(treeWidget);

  treeWidget->setStyleSheet( "QTreeWidget { font: bold; font-size: " + QString::number(sfont.pointSize() ) + "pt;}  QTreeView { alternate-background-color: yellow;} QTreeView::item:hover { border: black;  border-radius:1px;  background-color: rgba(0,128,255,95);}");

  QString indent( "  " );
  QStringList topItemNameList, solutionItemNameList, analysisItemNameList,
              analysisGenItemNameList, analysis2DSAItemNameList, analysisPCSAItemNameList;
  int wiubase = (int)QTreeWidgetItem::UserType;

  for ( int i=0; i<topLevelItems.size(); ++i )
    {
      QString topItemName = topLevelItems[i];
      topItemNameList.clear();
      topItemNameList << "" << indent + topItemName;
      topItem [ topItemName ] = new QTreeWidgetItem( treeWidget, topItemNameList, wiubase );

      //Solutions: add 1-level children
      if( topItemName.contains("Solutions") )
	{
	  int checked_childs = 0;
	  for ( int is=0; is<solutionItems.size(); ++is )
	    {
	      QString solutionItemName = solutionItems[ is ];
	      solutionItemNameList.clear();
	      solutionItemNameList << "" << indent.repeated( 2 ) + solutionItemName;
	      solutionItem [ solutionItemName ] = new QTreeWidgetItem( topItem [ topItemName ], solutionItemNameList, wiubase);

	      if ( solutionItems_vals[ is ].toInt() )
		{
		  solutionItem [ solutionItemName ] ->setCheckState( 0, Qt::Checked );
		  ++checked_childs;
		}
	      else
		solutionItem [ solutionItemName ] ->setCheckState( 0, Qt::Unchecked );
	    }
	  if ( checked_childs )
	    topItem [ topItemName ] ->setCheckState( 0, Qt::Checked );
	  else
	    topItem [ topItemName ] ->setCheckState( 0, Qt::Unchecked );
	  
	}
      
      //Analysis Profile: add 2-levelchildren
      else if ( topItemName.contains("Analysis Profile") )
	{
	  int checked_childs = 0;
	  for ( int ia=0; ia < analysisItems.size(); ++ia )
	    {
	      QString analysisItemName = analysisItems[ ia ];
	      analysisItemNameList.clear();
	      analysisItemNameList << "" << indent.repeated( 2 ) + analysisItemName;
	      analysisItem [ analysisItemName ] = new QTreeWidgetItem( topItem [ topItemName ], analysisItemNameList, wiubase);
	      
	      //General analysis
	      if( analysisItemName.contains("General") )
		{
		  int checked_gen = 0;
		  for ( int iag=0; iag < analysisGenItems.size(); ++iag )
		    {
		      QString analysisGenItemName = analysisGenItems[ iag ];
		      analysisGenItemNameList.clear();
		      analysisGenItemNameList << "" << indent.repeated( 3 ) + analysisGenItemName;
		      analysisGenItem [ analysisGenItemName ] = new QTreeWidgetItem( analysisItem [ analysisItemName ], analysisGenItemNameList, wiubase);

		      if ( analysisGenItems_vals[ iag ].toInt() )
			{
			  analysisGenItem [ analysisGenItemName ] ->setCheckState( 0, Qt::Checked );
			  ++checked_gen;
			}
		      else
			analysisGenItem [ analysisGenItemName ] ->setCheckState( 0, Qt::Unchecked );
		    }

		  if ( checked_gen )
		    {
		      analysisItem [ analysisItemName ] ->setCheckState( 0, Qt::Checked );
		      ++checked_childs;
		    }
		  else
		    analysisItem [ analysisItemName ] ->setCheckState( 0, Qt::Unchecked );
		}
	      //2DSA analysis
	      if( analysisItemName.contains("2DSA")  )
		{
		  int checked_2dsa = 0;
		  for ( int ia2=0; ia2 < analysis2DSAItems.size(); ++ia2 )
		    {
		      QString analysis2DSAItemName = analysis2DSAItems[ ia2 ];
		      analysis2DSAItemNameList.clear();
		      analysis2DSAItemNameList << "" << indent.repeated( 3 ) + analysis2DSAItemName;
		      analysis2DSAItem [ analysis2DSAItemName ] = new QTreeWidgetItem( analysisItem [ analysisItemName ], analysis2DSAItemNameList, wiubase);

		      if ( analysis2DSAItems_vals[ ia2 ].toInt() )
			{
			  analysis2DSAItem [ analysis2DSAItemName ] ->setCheckState( 0, Qt::Checked );
			  ++checked_2dsa;
			}
		      else
			analysis2DSAItem [ analysis2DSAItemName ] ->setCheckState( 0, Qt::Unchecked );
		    }

		  if ( checked_2dsa )
		    {
		      analysisItem [ analysisItemName ] ->setCheckState( 0, Qt::Checked );
		      ++checked_childs;
		    }
		  else
		    analysisItem [ analysisItemName ] ->setCheckState( 0, Qt::Unchecked );
		}

	      //PCSA analysis
	      if( analysisItemName.contains("PCSA")  )
		{
		  int checked_pcsa = 0;
		  for ( int iap=0; iap < analysisPCSAItems.size(); ++iap )
		    {
		      QString analysisPCSAItemName = analysisPCSAItems[ iap ];
		      analysisPCSAItemNameList.clear();
		      analysisPCSAItemNameList << "" << indent.repeated( 3 ) + analysisPCSAItemName;
		      analysisPCSAItem [ analysisPCSAItemName ] = new QTreeWidgetItem( analysisItem [ analysisItemName ], analysisPCSAItemNameList, wiubase);

		      if ( analysisPCSAItems_vals[ iap ].toInt() )
			{
			  analysisPCSAItem [ analysisPCSAItemName ] ->setCheckState( 0, Qt::Checked );
			  ++checked_pcsa;
			}
		      else
			analysisPCSAItem [ analysisPCSAItemName ] ->setCheckState( 0, Qt::Unchecked );
		    }

		  if ( checked_pcsa )
		    {
		      analysisItem [ analysisItemName ] ->setCheckState( 0, Qt::Checked );
		      ++checked_childs;
		    }
		  else
		    analysisItem [ analysisItemName ] ->setCheckState( 0, Qt::Unchecked );

		}
	    }
	  if ( checked_childs )
	    topItem [ topItemName ] ->setCheckState( 0, Qt::Checked );
	  else
	    topItem [ topItemName ] ->setCheckState( 0, Qt::Unchecked );
	}

      //set checked/unchecked for top-level item
      else
	{
	  if ( json.value( topItemName ).toString().toInt() )
	    topItem [ topItemName ] ->setCheckState( 0, Qt::Checked );
	  else
	    topItem [ topItemName ] ->setCheckState( 0, Qt::Unchecked );
	}
    }

  //ABDE
  if ( abde_mode )
    {
      for ( const auto& i : analysisItem.keys() )
	{
	  if ( i.contains("2DSA") ||i.contains("PCSA")  )
	    analysisItem[ i ]-> setHidden( true );
	}
      
      //2DSA
      for ( const auto& i : analysis2DSAItem.keys() )
	analysis2DSAItem [ i ]-> setHidden( true );
      
      //PCSA
      for ( const auto& i : analysisPCSAItem.keys() )
	analysisPCSAItem [ i ]-> setHidden( true );
    }
		  
  
  
  treeWidget->expandAll();    
  treeWidget->resizeColumnToContents( 0 );
  treeWidget->resizeColumnToContents( 1 );

  connect( treeWidget, SIGNAL( itemChanged( QTreeWidgetItem*, int ) ),
  	   this,       SLOT(   changedItem( QTreeWidgetItem*, int ) ) );

  
  // int ihgt        = le_chan_desc->height();
  // QSpacerItem* spacer2 = new QSpacerItem( 20, 1*ihgt, QSizePolicy::Expanding);
  // genL->setRowStretch( row, 1 );
  // genL->addItem( spacer2,  row++,  0, 1, 1 );

  //add Scroll Area to genL
  scrollArea      = new QScrollArea;
  containerWidget = new QWidget;
  containerWidget->setLayout( genL );
  scrollArea     ->setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded );
  scrollArea     ->setWidgetResizable( true );
  scrollArea     ->setWidget( containerWidget );
  main->addWidget( scrollArea );

  
  //Build | Re-build Lower buttons layout
  qDebug() << "Building Lower Buttons Layout -- ";
  //Clean genL layout first:
  if ( lower_buttons != NULL && lower_buttons->layout() != NULL )
    {
      QLayoutItem* item;
      while ( ( item = lower_buttons->layout()->takeAt( 0 ) ) != NULL )
	{
	  delete item->widget();
	  delete item;
	}
      delete lower_buttons;
    }
  //End cleaning layout
  lower_buttons     = new QHBoxLayout();

  pb_cancel   = us_pushbutton( tr( "Cancel" ) );
  pb_accept   = us_pushbutton( tr( "Accept" ) );
  
  connect( pb_cancel, SIGNAL( clicked() ), this, SLOT( cancel_selection() ) );
  connect( pb_accept, SIGNAL( clicked() ), SLOT( update_selection() ) );

  lower_buttons->addWidget( pb_cancel );
  lower_buttons->addWidget( pb_accept );
   
  main->addLayout( lower_buttons );

  setMinimumSize( 850, 450 );
  
}

//get Item's children
void US_ReportGenGui::get_item_childs( QList< QTreeWidgetItem* > & children_list, QTreeWidgetItem* item)
{
  children_list << item;
  
  int children = item->childCount();
  for ( int i = 0; i < children; ++i )
    get_item_childs( children_list, item->child(i) ); 
}

//What to check/uncheck upon change in items status
void US_ReportGenGui::changedItem( QTreeWidgetItem* item, int col )
{
  //we deal with col 0 only
  if ( col != 0  ) 
    return;

  //disconnect
  item -> treeWidget() -> disconnect();

  //go over children: recursive search for all children down the tree
  int children = item->childCount();
  QList< QTreeWidgetItem* > children_list;

  for( int i = 0; i < children; ++i )
    get_item_childs( children_list, item->child(i) ); 

  for ( int i = 0; i < children_list.size(); ++i )
    children_list[ i ] -> setCheckState( 0, (Qt::CheckState) item->checkState(0) );

  //Go over parents
  QTreeWidgetItem* parent_item = item->parent();
  QList< QTreeWidgetItem* > parents_list;
  QTreeWidgetItem* current_p_item = new QTreeWidgetItem;
  
  while ( parent_item )
    {
      parents_list << parent_item;
      current_p_item = parent_item;
      parent_item  = current_p_item -> parent();
    }
  
  for( int j = 0; j < parents_list.size(); ++j )
    {
      int checked_children = 0;
      int parent_item_children = parents_list[ j ] ->childCount();
      for( int jj = 0; jj < parent_item_children; ++jj )
	{
	  if ( int( parents_list[ j ]->child( jj )->checkState(0) ) )
	    ++checked_children;
	}
      
      if ( checked_children )
	parents_list[ j ]->setCheckState( 0, Qt::Checked );
      else
	parents_list[ j ]->setCheckState( 0, Qt::Unchecked );
    }

  //reconnect
  connect( item -> treeWidget(), SIGNAL( itemChanged( QTreeWidgetItem*, int ) ),
	   this,       SLOT(   changedItem( QTreeWidgetItem*, int ) ) );
}

/*
//What to check/uncheck upon change in items status
void US_ReportGenGui::changedItem( QTreeWidgetItem* item, int col )
{
  if ( col == 0  ) //we deal with col 0 only...
    {
      //qDebug() << "Changed item name0" << item->text( 1 );

      //if has (nested) children items
      int children_lev1 = item->childCount();
      if ( children_lev1 )
	{
	  treeWidget -> disconnect();

	  for( int i = 0; i < children_lev1; ++i )
	    {
	      item->child(i)->setCheckState( 0, (Qt::CheckState) item->checkState(0) );

	      int children_lev2 = item->child(i)->childCount();
	      if ( children_lev2 )
		{
		  for( int ii = 0; ii < children_lev2; ++ii )
		    {
		      item->child(i)->child(ii)->setCheckState( 0, (Qt::CheckState) item->child(i)->checkState(0) );
		    }
		}
	    }
	  
	  connect( treeWidget, SIGNAL( itemChanged( QTreeWidgetItem*, int ) ),
	       this,       SLOT(   changedItem( QTreeWidgetItem*, int ) ) );
	}
      
           
      //qDebug() << "Changed item name1 " << item->text( 1 );
      	
      //if has parent item
      QTreeWidgetItem* parent_item = item->parent();

      //qDebug() << "Changed item name2: " << item->text( 1 );
      
      if ( parent_item )
	{
	  //qDebug() << "Changed item name3: " << item->text( 1 );
	  treeWidget -> disconnect();
	  
	  //qDebug() << " Current item, " << item->text( 1 ) << ", has a parent: " << parent_item->text( 1 );
	    
	  int checked_children = 0;
	  int parent_item_children = parent_item ->childCount();
	  for( int i = 0; i < parent_item_children; ++i )
	    {
	      if ( int( parent_item->child( i )->checkState(0) ) )
		++checked_children;
	    }
	  if ( checked_children )
	    parent_item->setCheckState( 0, Qt::Checked );
	  else
	    parent_item->setCheckState( 0, Qt::Unchecked );

	  connect( treeWidget, SIGNAL( itemChanged( QTreeWidgetItem*, int ) ),
  	           this,       SLOT(   changedItem( QTreeWidgetItem*, int ) ) );
	}
    }
}
*/

//Save selection to JSON string & emit signal
void US_ReportGenGui::gui_to_parms( void )
{
  //tree-to-json: genTree && json-to-genMask structure
  QString mask_edited = tree_to_json ( topItem );
  //parse_edited_gen_mask_json( mask_edited, genMask_edited );  // <-- not here, will be in the analysis program

  emit update_details( mask_edited );
  
  // //DEBUG
  // exit(1);
}

//
void US_ReportGenGui::get_children_to_json( QString& mask_edited, QTreeWidgetItem* item )
{
  for( int i = 0; i < item->childCount(); ++i )
    {
      mask_edited += "\"" + item->child(i)->text(1).trimmed() + "\":";
      
      int children_lev2 = item->child(i)->childCount();
      if ( !children_lev2 )
	{
	  mask_edited += "\"" + QString::number( int(item->child(i)->checkState(0)) ) + "\"";
	  if ( i != item->childCount()-1 )
	    mask_edited += ",";
	}
      else
	{
	  mask_edited += "{";
	  get_children_to_json( mask_edited, item->child(i) );
	  mask_edited += "},";
	}
    }

  //ALEXEY: <-- little trick to enable super-fast recursive over arbitrary tree:))
  mask_edited.replace(",},","},");  
}


//transform arbitraty-level tree to JSON
QString US_ReportGenGui::tree_to_json( QMap < QString, QTreeWidgetItem * > topLevItems )
{
  QString mask_edited;
  mask_edited += "{";
  
  QMap < QString, QTreeWidgetItem * >::iterator top;
  for ( top = topLevItems.begin(); top != topLevItems.end(); ++top )
    {
      mask_edited += "\"" + top.key().trimmed() + "\":";
      int children_lev1 = top.value()->childCount();
      if ( !children_lev1 )
	{
	  mask_edited += "\"" + QString::number( int(top.value()->checkState(0)) ) + "\",";
	}
      else
	{
	  mask_edited += "[{";
	  
	  //here we need to generalize for any tree nestedness: recursive
	  get_children_to_json( mask_edited, top.value() );

	  mask_edited += "}],";
	}
    }

  //ALEXEY: <-- little trick to enable super-fast recursive over arbitrary tree:))
  mask_edited.replace(",}],","}],"); 
  QString to_replace = "}],";
  QString new_substr = "}]";
  mask_edited.replace( mask_edited.lastIndexOf( to_replace ), to_replace.size(), new_substr );

  mask_edited += "}";

  qDebug() << "Edited Mask: " << mask_edited;

  return mask_edited;
}


/*
//Save selection to JSON string & emit signal
void US_ReportGenGui::gui_to_parms( void )
{
  QString mask_edited;
  mask_edited += "{";
  
  QMap < QString, QTreeWidgetItem * >::iterator top;
  for ( top = topItem.begin(); top != topItem.end(); ++top )
    {
      qDebug() << "Top item " << top.key() << " is " <<  int(top.value()->checkState(0)) << "\n";

      mask_edited += "\"" + top.key().trimmed() + "\":";
      
      int children_lev1 = top.value()->childCount();
      if ( !children_lev1 )
	{
	  mask_edited += "\"" + QString::number( int(top.value()->checkState(0)) ) + "\",";
	}
      else
	{
	  mask_edited += "[";
	  for( int i = 0; i < top.value()->childCount(); ++i )
	    {
	      qDebug() << "\tThe child's " << top.value()->child(i)->text(1) << ", state is: " << top.value()->child(i)->checkState(0);

	      mask_edited += "{\"" + top.value()->child(i)->text(1).trimmed() + "\":";

	      int children_lev2 = top.value()->child(i)->childCount();
	      if ( !children_lev2 )
		{
		  mask_edited += "\"" + QString::number( int(top.value()->child(i)->checkState(0)) ) + "\"}";
		  if ( i != top.value()->childCount()-1 )
		    mask_edited += ",";
		}
	      else
		{
		  mask_edited += "{";
		  for( int ii = 0; ii < top.value()->child(i)->childCount(); ++ii )
		    {
		      qDebug() << "\t\tThe child's " << top.value()->child(i)->child(ii)->text(1) << ", state is: " << top.value()->child(i)->child(ii)->checkState(0);

		      mask_edited += "\"" + top.value()->child(i)->child(ii)->text(1).trimmed() + "\":";
		      int children_lev3 = top.value()->child(i)->child(ii)->childCount();
		      if ( !children_lev3 )
			{
			  mask_edited += "\"" + QString::number( int(top.value()->child(i)->child(ii)->checkState(0)) ) + "\"";
			  if ( ii != top.value()->child(i)->childCount()-1 )
			    mask_edited += ",";
			}
		      else
			{
			  //Here 3th level of nestedness may be considered if needed... and so on...
			}
		      
		    }
		  mask_edited += "}}";
		  if ( i != top.value()->childCount()-1 )
		    mask_edited += ",";
		}
	    }
	  mask_edited += "],";
	}
    }
  mask_edited.chop(1);
  mask_edited += "}";


  qDebug() << "Edited Mask: " << mask_edited; 
  
  emit update_details( mask_edited );
}
*/


void US_ReportGenGui::cancel_selection( void )
{
  close();
}

void US_ReportGenGui::update_selection( void )
{
  gui_to_parms( );
  close();
}
