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

US_ReportGenGui::US_ReportGenGui( ) : US_Widgets()
{
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

  // connect( treeWidget, SIGNAL( itemChanged( QTreeWidgetItem*, int ) ),
  // 	   this,       SLOT(   changedItem( QTreeWidgetItem*, int ) ) );

  
  topLevelItems.clear();
  solutionItems.clear();
  topLevelItems << "General Settings"
		<< "Lab/Rotor Parameters"
		<< "Optima Machine Operator"
		<< "Speed Parameters"
		<< "Cell Centerpiece Usage"
		<< "Solutions for Channels"
		<< "Optics"
		<< "Ranges"
		<< "Scan Counts and Intervals for Optics"
		<< "Analysis Profile"
    ;

  solutionItems << "Analyte Information"
		<< "Buffer Information"
    ;

  analysisItems << "General Settings"
		<< "2DSA Controls"
		<< "PCSA Controls"
    ;

  analysisGenItems << "Channel General Settings"
		   << "Report Parameters (per-triple)"
		   << "Report Item Parameters (per-triple)"
    ;

  analysis2DSAItems << "Job Flow Summary"
		    << "Per-Channel Profiles"
    ;

  analysisPCSAItems << "Job Flow Summary"
		    << "Per-Channel Profiles"
    ;
  
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

      //add children
      if( topItemName.contains("Solutions") )
	{
	  for ( int is=0; is<solutionItems.size(); ++is )
	    {
	      QString solutionItemName = solutionItems[ is ];
	      solutionItemNameList.clear();
	      solutionItemNameList << "" << indent.repeated( 2 ) + solutionItemName;
	      solutionItem [ solutionItemName ] = new QTreeWidgetItem( topItem [ topItemName ], solutionItemNameList, wiubase);

	      solutionItem [ solutionItemName ] ->setCheckState( 0, Qt::Checked );
	    }
	}

      if( topItemName.contains("Analysis Profile") )
	{
	  for ( int ia=0; ia < analysisItems.size(); ++ia )
	    {
	      QString analysisItemName = analysisItems[ ia ];
	      analysisItemNameList.clear();
	      analysisItemNameList << "" << indent.repeated( 2 ) + analysisItemName;
	      analysisItem [ analysisItemName ] = new QTreeWidgetItem( topItem [ topItemName ], analysisItemNameList, wiubase);

	      analysisItem [ analysisItemName ] ->setCheckState( 0, Qt::Checked );

	      //General analysis
	      if( analysisItemName.contains("General") )
		{
		  for ( int iag=0; iag < analysisGenItems.size(); ++iag )
		    {
		      QString analysisGenItemName = analysisGenItems[ iag ];
		      analysisGenItemNameList.clear();
		      analysisGenItemNameList << "" << indent.repeated( 3 ) + analysisGenItemName;
		      analysisGenItem [ analysisGenItemName ] = new QTreeWidgetItem( analysisItem [ analysisItemName ], analysisGenItemNameList, wiubase);
		      
		      analysisGenItem [ analysisGenItemName ] ->setCheckState( 0, Qt::Checked );
		    }
		}
	      //2DSA analysis
	      if( analysisItemName.contains("2DSA") )
		{
		  for ( int ia2=0; ia2 < analysis2DSAItems.size(); ++ia2 )
		    {
		      QString analysis2DSAItemName = analysis2DSAItems[ ia2 ];
		      analysis2DSAItemNameList.clear();
		      analysis2DSAItemNameList << "" << indent.repeated( 3 ) + analysis2DSAItemName;
		      analysis2DSAItem [ analysis2DSAItemName ] = new QTreeWidgetItem( analysisItem [ analysisItemName ], analysis2DSAItemNameList, wiubase);
		      
		      analysis2DSAItem [ analysis2DSAItemName ] ->setCheckState( 0, Qt::Checked );
		    }
		}
	      //PCSA analysis
	      if( analysisItemName.contains("PCSA") )
		{
		  for ( int iap=0; iap < analysisPCSAItems.size(); ++iap )
		    {
		      QString analysisPCSAItemName = analysisPCSAItems[ iap ];
		      analysisPCSAItemNameList.clear();
		      analysisPCSAItemNameList << "" << indent.repeated( 3 ) + analysisPCSAItemName;
		      analysisPCSAItem [ analysisPCSAItemName ] = new QTreeWidgetItem( analysisItem [ analysisItemName ], analysisPCSAItemNameList, wiubase);
		      
		      analysisPCSAItem [ analysisPCSAItemName ] ->setCheckState( 0, Qt::Checked );
		    }
		}
	    }
	}
      
      topItem [ topItemName ] ->setCheckState( 0, Qt::Checked );
    }
  
  treeWidget->expandAll();    
  treeWidget->resizeColumnToContents( 0 );
  treeWidget->resizeColumnToContents( 1 );

  
  
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

void US_ReportGenGui::gui_to_parms( void )
{

  
  //emit update_details( report_details );
}

void US_ReportGenGui::cancel_selection( void )
{
  
}

void US_ReportGenGui::update_selection( void )
{
  gui_to_parms( );
  close();
}
