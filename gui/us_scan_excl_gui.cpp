//! \file us_scan_excl_gui.cpp

#include "us_scan_excl_gui.h"
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


US_ScanExclGui::US_ScanExclGui( QStringList channels_desc, QList< int > scan_beg, QList< int > scan_end, int scanCount, int scanCount_int ) : US_Widgets()
{
  this->channels_desc = channels_desc;
  this->scan_beg      = scan_beg;
  this->scan_end      = scan_end;
  this->scanCount     = scanCount;
  this->scanCount_int = scanCount_int;
    
  setWindowTitle( tr( "Scan Exclusion Editor"));
  
  //setPalette( US_GuiSettings::frameColor() );
  setPalette( US_GuiSettings::normalColor() );
  main  = new QVBoxLayout( this );
  main->setSpacing( 2 );
  main->setContentsMargins( 2, 2, 2, 2 );

  //Top level parameters
  QLabel* bn_excl_scans     = us_banner( QString( tr( "Selection of the Scans Excluded from Beginnig and End of the Scan Sets: " ) ), 1 );
  bn_excl_scans->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
  main->addWidget( bn_excl_scans );

  // params    =  new QGridLayout();
  // params    ->setSpacing         ( 2 );
  // params    ->setContentsMargins ( 2, 2, 2, 2 );

  // QLabel* lb_channame       = us_label( tr( "Channel Description" ) );
  // QLabel* lb_begin          = us_label( tr( "Number of scans at the beginning of run:" ) );
  // QLabel* lb_end            = us_label( tr( "Number of scans at the end of run:" ) );

  

  // ////////////////////////////////////////////////////////////////////////////
  
  row = 0;
  // params->addWidget( lb_channame,       row,    0, 1, 2 );
  // params->addWidget( lb_begin,          row,    2, 1, 2 );
  // params->addWidget( lb_end,            row++,  4, 1, 2 );

  // topContainerWidget = new QWidget;
  // topContainerWidget->setLayout( params );
  // main->addWidget( topContainerWidget );


  //main->addLayout( params );

  //Main Table
  genL           = NULL;
  lower_buttons  = NULL;

  build_layout();

}


//build Layout
void US_ScanExclGui::build_layout ( void )
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

  maxScans_map.clear();
  
  genL        =  new QGridLayout();
  genL        ->setSpacing         ( 2 );
  genL        ->setContentsMargins ( 2, 2, 2, 2 );

  
  //add headers
  QLabel* lb_channame        = us_label( tr( "Channel Description" ) );
  QLabel* lb_begin           = us_label( tr( "Number of scans at\nthe beginning of run:" ) );
  QLabel* lb_end             = us_label( tr( "Number of scans at\nthe end of run:" ) );
  QLabel* lb_remaining_scans = us_label( tr( "Total number of \nremaining scans:" ) );

  genL->addWidget( lb_channame,        row,    0, 1, 2 );
  genL->addWidget( lb_begin,           row,    3, 1, 2 );
  genL->addWidget( lb_end,             row,    5, 1, 2 );
  genL->addWidget( lb_remaining_scans, row++,  7, 1, 2 );
  
  
  QLineEdit*   le_chan_desc; 
  QSpinBox*    sb_begin; 
  QSpinBox*    sb_end;
  QLineEdit*   le_remaining_scans; 

  pb_applyall = us_pushbutton( tr( "Apply to All" ) );
  
  qDebug() << "Sizes of channels_desc, scan_beg | end: " << channels_desc.size() << scan_beg.size() << scan_end.size();
  
  for ( int ii = 0; ii < channels_desc.size(); ii++ )
    {
      QString chan_desc = channels_desc[ ii ];

      qDebug() << "Build layout: chan_Desc -- " << chan_desc;
      qDebug() << "Build layout: scan_beg[ii], scan_end[ii]  -- " << scan_beg[ii] << scan_end[ii];

      le_chan_desc        = us_lineedit( chan_desc,  0, true  );
      sb_begin            = us_spinbox();
      sb_end              = us_spinbox();
      
      
      // sb_begin            ->setValue( (int)*scan_beg[ii] );
      // sb_end              ->setValue( (int)*scan_end[ii] );

      //set Maximum values based on type of Optics
      sb_begin            ->setMaximum( (int)( scanCount/2 - 5));
      sb_end              ->setMaximum( (int)( scanCount/2 - 5));

      //Calculate #remaining scans for a channel/triple
      int remaining_scans_num;
      QString max_scanCount;
      remaining_scans_num = scanCount - ( scan_beg[ii] + scan_end[ii] );
      maxScans_map[ QString::number( ii ) ]  = scanCount ;

      if( chan_desc.contains("Interf.") )
	{
	  sb_begin            ->setMaximum( (int)( scanCount_int/2 - 5));
	  sb_end              ->setMaximum( (int)( scanCount_int/2 - 5));

	  remaining_scans_num = scanCount_int - ( scan_beg[ii] + scan_end[ii] );
	  maxScans_map[ QString::number( ii ) ] = scanCount_int;
	}

      qDebug() << "Maximum range: sb_begin -- " << sb_begin->maximum();
      
      sb_begin            ->setValue( (int)scan_beg[ii] );
      sb_end              ->setValue( (int)scan_end[ii] );

      
      le_remaining_scans  = us_lineedit( QString::number( remaining_scans_num ),  0, true  );
	  
      QString stchan      =  QString::number( ii ) + ": ";
      le_chan_desc        -> setObjectName( stchan + "desc" );
      sb_begin            -> setObjectName( stchan + "begin" );
      sb_end              -> setObjectName( stchan + "end" );
      le_remaining_scans  -> setObjectName( stchan + "remain" );

      // if ( !chan_desc.contains("B:Interf.") )
      // 	{
      genL->addWidget( le_chan_desc,         row,    0, 1, 2 );
      genL->addWidget( sb_begin,             row,    3, 1, 2 );
      genL->addWidget( sb_end,               row,    5, 1, 2 );
      genL->addWidget( le_remaining_scans,   row,    7, 1, 2 );
      // }

      connect( sb_begin,   SIGNAL( valueChanged ( int ) ),
	       this,       SLOT  ( scan_excl_changed ( int ) ) );
      connect( sb_end,     SIGNAL( valueChanged ( int ) ),
	       this,       SLOT  ( scan_excl_changed ( int ) ) );
      
      QFont font   = le_chan_desc->property("font").value<QFont>();
      QFontMetrics fm(font);
      int pixelsWide = fm.width( le_chan_desc->text() );
      le_chan_desc->setMinimumWidth( pixelsWide*1.1 );
      le_chan_desc->adjustSize();
      
      if ( ii == 0 )
	{
	  genL->addWidget( pb_applyall, row++, 9, 1, 2 );
	  connect( pb_applyall, SIGNAL( clicked       ( ) ),
		   this,        SLOT(   applied_to_all( ) ) );
	}
      else
	row++;

      if ( chan_desc.contains("B:Interf.") )
	{
	  le_chan_desc->hide();
	  sb_begin    ->hide();
	  sb_end      ->hide();
	  le_remaining_scans ->hide();
	}
    }

  int ihgt        = le_chan_desc->height();
  QSpacerItem* spacer2 = new QSpacerItem( 20, 1*ihgt, QSizePolicy::Expanding);
  genL->setRowStretch( row, 1 );
  genL->addItem( spacer2,  row++,  0, 1, 1 );

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
  
  connect( pb_cancel, SIGNAL( clicked() ), this, SLOT( cancel_update() ) );
  connect( pb_accept, SIGNAL( clicked() ), SLOT( update_scans_excl() ) );

  lower_buttons->addWidget( pb_cancel );
  lower_buttons->addWidget( pb_accept );
   
  main->addLayout( lower_buttons );

  setMinimumSize( 850, 450 );

}

//Slot to update scan excl. counters
void US_ScanExclGui::scan_excl_changed( int )
{
  QObject* sobj       = sender();      
  QString oname       = sobj->objectName();

  bool isBegin        = false;
  QSpinBox * sb_widget_counterpart = NULL;
  QString objName_for_sb_widget_counterpart;
  
  QSpinBox * sb_widget = containerWidget->findChild<QSpinBox *>( oname );
  QString item_row = oname.split(":")[0] + QString(": ");
  QString row_num  = oname.split(":")[0];
  
  if ( oname.contains(": begin") )
    {
      isBegin = true;
      objName_for_sb_widget_counterpart = item_row + "end";
      sb_widget_counterpart = containerWidget->findChild<QSpinBox *>( objName_for_sb_widget_counterpart );
    }
  else
    {
      objName_for_sb_widget_counterpart = item_row + "begin";
      sb_widget_counterpart = containerWidget->findChild<QSpinBox *>( objName_for_sb_widget_counterpart );
    }
  
  qDebug() << "Clicked couner:        oname, value -- "
	   << oname
	   << sb_widget->value();
  qDebug() << "Counterpart's widget:  oname, value -- "
	   << objName_for_sb_widget_counterpart
	   << sb_widget_counterpart->value();

  //set # of remaining scans for the channel
  QString objName_for_le_remainScans = item_row + "remain";
  QLineEdit* le_remainScans = containerWidget->findChild<QLineEdit *>( objName_for_le_remainScans );
  int rem_scans_num = maxScans_map[ row_num ] - ( sb_widget->value() + sb_widget_counterpart->value() );

  le_remainScans ->setText( QString::number( rem_scans_num ));
}


//Apply to all channels
void US_ScanExclGui::applied_to_all( void )
{
  QSpinBox * sb_b = containerWidget->findChild< QSpinBox *>( "0: begin" );
  QSpinBox * sb_e = containerWidget->findChild< QSpinBox *>( "0: end" );
  
  int s_b_0 =  (int)sb_b->value();
  int s_e_0 =  (int)sb_e->value();
  
  for ( int ii = 0; ii < channels_desc.size(); ii++ )
    {
      QString stchan      =  QString::number( ii ) + ": ";

      QSpinBox * sb_b = containerWidget->findChild< QSpinBox *>( stchan + "begin" );
      sb_b -> setValue( (int)s_b_0 );

      QSpinBox * sb_e = containerWidget->findChild< QSpinBox *>( stchan + "end" );
      sb_e -> setValue( (int)s_e_0 );
    }
}

//save gui to parms
void US_ScanExclGui::gui_to_parms( void )
{
  QStringList scan_ranges_list;
  
  for ( int ii = 0; ii < channels_desc.size(); ii++ )
    {
      QString stchan      =  QString::number( ii ) + ": ";

      //scan begin
      QSpinBox * sb_b = containerWidget->findChild< QSpinBox *>( stchan + "begin" );
      // int sb_b_val = (int)sb_b->value();
      // scan_beg[ ii ] =  &sb_b_val;
      scan_beg[ ii ] = (int)sb_b->value();
      
      //scan end
      QSpinBox * sb_e = containerWidget->findChild< QSpinBox *>( stchan + "end" );
      // int sb_e_val = (int)sb_e->value();
      // scan_end[ ii ] =  &sb_e_val;
      scan_end[ ii ] = (int)sb_e->value();
	
      qDebug() << "In gui_to_parms: scna_beg, scan_end -- " << scan_beg[ ii ] << scan_end[ ii ];

      QString range_pair = QString::number( scan_beg[ ii ] ) + ":" + QString::number( scan_end[ ii ] ); 
      
      scan_ranges_list << range_pair;
    }
  
  
  emit update_aprofile_scans( scan_ranges_list );
}

//cancel
void US_ScanExclGui::cancel_update( void )
{
  close();
}

//update
void US_ScanExclGui::update_scans_excl( void ) 
{
  gui_to_parms( );
  close();
}
