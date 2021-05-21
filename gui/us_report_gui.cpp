//! \file us_report_gui.cpp

#include "us_report_gui.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_investigator.h"
//#include "us_report_gmp.h"


#if QT_VERSION < 0x050000
#define setSymbol(a)      setSymbol(*a)
#define setSamples(a,b,c) setData(a,b,c)
#endif


// Report GUI 
US_ReportGui::US_ReportGui( US_ReportGMP *tmp_report ) : US_Widgets()
{
  this->report               = tmp_report;
  this->report_copy_original = *report;

  setWindowTitle( tr( "Channel Report Editor"));
  
  setPalette( US_GuiSettings::frameColor() );
  main  = new QVBoxLayout( this );
  main->setSpacing( 2 );
  main->setContentsMargins( 2, 2, 2, 2 );

  //Top level parameters
  QLabel* bn_report     = us_banner( QString( tr( "Report Parameters for channel: %1" ) ).arg( report->channel_name ), 1 );
  bn_report->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
  main->addWidget( bn_report );

  params    =  new QGridLayout();
  params    ->setSpacing         ( 2 );
  params    ->setContentsMargins ( 2, 2, 2, 2 );

  QLabel* lb_tot_conc       = us_label( tr( "Total Concentration" ) );
  QLabel* lb_rmsd_limit     = us_label( tr( "RMSD (upper limit)" ) );
  QLabel* lb_av_intensity   = us_label( tr( "Average Intensity" ) );

  le_tot_conc      = us_lineedit( QString::number(report->tot_conc),   0, false  );
  le_rmsd_limit    = us_lineedit( QString::number(report->rmsd_limit), 0, false  );
  le_av_intensity  = us_lineedit( QString::number(report->av_intensity),  0, false  );

  row = 0;
  params->addWidget( lb_tot_conc,       row,    0, 1, 2 );
  params->addWidget( le_tot_conc,       row,    3, 1, 2 );
  params->addWidget( lb_rmsd_limit,     row,    5, 1, 2 );
  params->addWidget( le_rmsd_limit,     row,    7, 1, 2 );
  params->addWidget( lb_av_intensity,   row,    9, 1, 2 );
  params->addWidget( le_av_intensity,   row++,    13, 1, 2 );
 
  // int ihgt        = le_tot_conc->height();
  // QSpacerItem* spacer1 = new QSpacerItem( 20, 0.75*ihgt, QSizePolicy::Expanding );
  // params->setRowStretch( row, 1 );
  // params->addItem( spacer1,  row++,  0, 1, 1 );

  main->addLayout( params );
  
  //Banner for Table
  QLabel* bn_report_t     = us_banner( tr( "Report Parameters: Type | Method" ) );
  bn_report_t->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
  main->addWidget( bn_report_t );

  //Main Table
  genL           = NULL;
  addRem_buttons = NULL;
  lower_buttons  = NULL;
   
  build_report_layout();

}

//Layout build
void US_ReportGui::build_report_layout( void )
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
  
  int r_item_num = report->reportItems.size();
  qDebug() << "Size of report class -- " << report->reportItems.size();

  genL        =  new QGridLayout();
  genL        ->setSpacing         ( 2 );
  genL        ->setContentsMargins ( 2, 2, 2, 2 );

  //Table Header
  QLabel* lb_type    = us_label( tr( "Type" ) );
  QLabel* lb_method  = us_label( tr( "Method" ) );
  QLabel* lb_low     = us_label( tr( "Range Low" ) );
  QLabel* lb_high    = us_label( tr( "Range High" ) );
  QLabel* lb_intval  = us_label( tr( "Integration Value" ) );
  QLabel* lb_tol     = us_label( tr( "Tolerance (%)" ) );
  QLabel* lb_total   = us_label( tr( "Fraction of Total (%)" ) );
    
  row = 0;
  genL->addWidget( lb_type,   row,    0, 1, 2 );
  genL->addWidget( lb_method, row,    3, 1, 2 );
  genL->addWidget( lb_low,    row,    5, 1, 2 );
  genL->addWidget( lb_high,   row,    7, 1, 2 );
  genL->addWidget( lb_intval, row,    9, 1, 2 );
  genL->addWidget( lb_tol,    row,    11, 1, 2 );
  genL->addWidget( lb_total,  row++,    13, 1, 2 );
  //End of table header
  
  QComboBox* cb_type;  
  QComboBox* cb_method;
  QStringList sl_types;
  QStringList sl_methods;
  sl_types     << QString("S") << QString("D") << QString("f/f0") << QString("MW");
  sl_methods   << QString("2DSA-IT") << QString("PCSA-SL") << QString("2DSA-MC") << QString("raw");
  qDebug() << "Begin ReportItems iteration -- ";
   
  QLineEdit* le_low;    
  QLineEdit* le_high;   
  QLineEdit* le_intval; 
  QLineEdit* le_tol;    
  QLineEdit* le_total;
    

  for ( int ii = 0; ii < r_item_num; ii++ )
    {
      US_ReportGMP::ReportItem curr_item = report->reportItems[ ii ];

      //type ComboBox
      cb_type   =  us_comboBox();
      cb_type   -> clear();
      cb_type   -> addItems( sl_types );
      //need to set index corr. to type in ReportItem
      int type_ind = cb_type->findText( curr_item.type );
      cb_type->setCurrentIndex( type_ind );

      //method ComboBox
      cb_method = us_comboBox();      
      cb_method->clear();
      cb_method->addItems( sl_methods );
      //need to set index corr. to method in ReportItem
      int method_ind = cb_method->findText( curr_item.method );
      cb_method->setCurrentIndex( method_ind );

      le_low        = us_lineedit( QString::number(curr_item.range_low),  0, false  );
      le_high       = us_lineedit( QString::number(curr_item.range_high), 0, false  );
      le_intval     = us_lineedit( QString::number(curr_item.integration_val), 0, false  );
      le_tol        = us_lineedit( QString::number(curr_item.tolerance), 0, false  );
      le_total      = us_lineedit( QString::number(curr_item.total_percent), 0, false  );

      genL->addWidget( cb_type,   row,    0, 1, 2 );
      genL->addWidget( cb_method, row,    3, 1, 2 );
      genL->addWidget( le_low,    row,    5, 1, 2 );
      genL->addWidget( le_high,   row,    7, 1, 2 );
      genL->addWidget( le_intval, row,    9, 1, 2 );
      genL->addWidget( le_tol,    row,    11, 1, 2 );
      genL->addWidget( le_total,  row++,  13, 1, 2 );
    }
  
  int ihgt        = le_low->height();
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

  //main->addLayout( genL );

  //Build | Re-build Add/Remove buttons layout
  qDebug() << "Building Add|Remove Buttons Layout -- ";
  //Clean layout first:
  if ( addRem_buttons != NULL && addRem_buttons->layout() != NULL )
    {
      QLayoutItem* item;
      while ( ( item = addRem_buttons->layout()->takeAt( 0 ) ) != NULL )
	{
	  delete item->widget();
	  delete item;
	}
      delete addRem_buttons;
    }
  
  addRem_buttons     = new QGridLayout();
  
  pb_addRow     = us_pushbutton( tr( "Add New Row" ) );
  connect( pb_addRow, SIGNAL( clicked() ), this, SLOT( add_row()  ) );

  pb_removeRow  = us_pushbutton( tr( "Remove Last Row" ) );
  connect( pb_removeRow, SIGNAL( clicked() ), this, SLOT( remove_row()  ) );
  if (  report->reportItems.size() < 2 )
    pb_removeRow->setEnabled( false );
  
  addRem_buttons->addWidget( pb_removeRow, row,  11, 1, 2 );
  addRem_buttons->addWidget( pb_addRow,    row,  13, 1, 2 );

  main->addLayout( addRem_buttons );
   
  
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
  connect( pb_accept, SIGNAL( clicked() ), SLOT( update_report() ) );

  lower_buttons->addWidget( pb_cancel );
  lower_buttons->addWidget( pb_accept );
   
  main->addLayout( lower_buttons );

  setMinimumSize( 850, 450 );
  //adjustSize();

}

//Slot to update report
void US_ReportGui::update_report( void )
{
  
}

//Slot to cancel any updates on report
void US_ReportGui::cancel_update( void )
{
  
  QMessageBox msgBox;
  msgBox.setIcon(QMessageBox::Critical);
  msgBox.setWindowTitle(tr("Cancel Updates"));
  
  //QString msg_text      = QString("Attention! Do you want to cancel all report updates for channel %1?").arg( channel_name );
  QString msg_text      = QString("Attention! Do you want to cancel all report updates for the current channel?");
  msgBox.setText( msg_text );
    
  QPushButton *Accept    = msgBox.addButton(tr("Yes"), QMessageBox::YesRole);
  QPushButton *Cancel    = msgBox.addButton(tr("No"), QMessageBox::RejectRole);
  msgBox.exec();
  
  if ( msgBox.clickedButton() == Accept )
    {
      //Send copy of the report to restore to original
      qDebug() << "Sizes: report CHANGED,  report_original -- " << report->reportItems.size() << report_copy_original.reportItems.size();
      report = &( report_copy_original );
      qDebug() << "Sizes: report RESTORED, report_original -- " << report->reportItems.size() << report_copy_original.reportItems.size();
      qDebug() << "Report_copy_original.channel_name -- " << report_copy_original.channel_name;
      emit cancel_changes( report_copy_original );
      close();
    }
  else if (msgBox.clickedButton() == Cancel)
    return;
}


//Slot to add row
void US_ReportGui::add_row( void )
{
  US_ReportGMP::ReportItem initItem;
  
  initItem.type             = QString("S");
  initItem.method           = QString("2DSA-IT");
  initItem.range_low        = 3.2;
  initItem.range_high       = 3.7;
  initItem.integration_val  = 0.57;
  initItem.tolerance        = 10; 
  initItem.total_percent    = 0.58;
  
  report->reportItems.push_back( initItem );

  //re-build genL layout
  build_report_layout( );
}

//Slot to remove rows
void US_ReportGui::remove_row( void )
{
  report->reportItems.removeLast();
  
  //re-build genL layout
  build_report_layout( );
}


