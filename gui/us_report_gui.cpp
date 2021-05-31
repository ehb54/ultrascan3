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
#define QRegularExpression(a)  QRegExp(a)
#endif


// Report GUI 
US_ReportGui::US_ReportGui( US_ReportGMP *tmp_report ) : US_Widgets()
{
  this->report               = tmp_report;
  this->report_copy_original = *report;

  setWindowTitle( tr( "Channel Report Editor"));
  
  //setPalette( US_GuiSettings::frameColor() );
  setPalette( US_GuiSettings::normalColor() );
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
  QLabel* lb_duration       = us_label( tr( "Experiment Duration" ) );
  QLabel* lb_wvl            = us_label( tr( "Wavelength" ) );
  QHBoxLayout* lo_duratlay  = us_ddhhmmsslay( 0, 1,0,0,1, &sb_durat_dd, &sb_durat_hh, &sb_durat_mm, &sb_durat_ss ); // ALEXEY 0 - visible, 1 - hidden

  le_tot_conc      = us_lineedit( QString::number(report->tot_conc),     0, false  );
  le_rmsd_limit    = us_lineedit( QString::number(report->rmsd_limit),   0, false  );
  le_av_intensity  = us_lineedit( QString::number(report->av_intensity), 0, false  );

  le_tot_conc      -> setObjectName( "tot_conc" );
  le_rmsd_limit    -> setObjectName( "rmsd" );
  le_av_intensity  -> setObjectName( "av_intensity" );

  //set connecitons btw textChanged() and slot
  connect( le_tot_conc,   SIGNAL( textChanged ( const QString& ) ),
	   this,          SLOT  ( verify_text ( const QString& ) ) );
  connect( le_rmsd_limit, SIGNAL( textChanged ( const QString& ) ),
	   this,          SLOT  ( verify_text ( const QString& ) ) );
  connect( le_av_intensity, SIGNAL( textChanged ( const QString& ) ),
	   this,          SLOT  ( verify_text ( const QString& ) ) );
   

  QList< int > dhms_dur;
  double exp_dur = report->experiment_duration;
  US_RunProtocol::timeToList( exp_dur, dhms_dur );
  sb_durat_dd ->setValue( (int)dhms_dur[ 0 ] );
  sb_durat_hh ->setValue( (int)dhms_dur[ 1 ] );
  sb_durat_mm ->setValue( (int)dhms_dur[ 2 ] );
  sb_durat_ss ->setValue( (int)dhms_dur[ 3 ] );

  le_wvl      = us_lineedit( QString::number(report->wavelength),  0, true  );
  
  row = 0;
  params->addWidget( lb_tot_conc,       row,    0, 1, 2 );
  params->addWidget( le_tot_conc,       row,    3, 1, 2 );
  params->addWidget( lb_rmsd_limit,     row,    5, 1, 2 );
  params->addWidget( le_rmsd_limit,     row,    7, 1, 2 );
  params->addWidget( lb_av_intensity,   row,    9, 1, 2 );
  params->addWidget( le_av_intensity,   row++,  13, 1, 2 );

  params->addWidget( lb_duration,       row,    0, 1, 2 );
  params->addLayout( lo_duratlay,       row,    3, 1, 2 );
  params->addWidget( lb_wvl,            row,    9, 1, 2 );
  params->addWidget( le_wvl,            row++,  13, 1, 2 );
  
 
  // int ihgt        = le_tot_conc->height();
  // QSpacerItem* spacer1 = new QSpacerItem( 20, 0.75*ihgt, QSizePolicy::Expanding );
  // params->setRowStretch( row, 1 );
  // params->addItem( spacer1,  row++,  0, 1, 1 );

  topContainerWidget = new QWidget;
  topContainerWidget->setLayout( params );
  main->addWidget( topContainerWidget );
  
  //main->addLayout( params );

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
  sl_types     << QString("s") << QString("D") << QString("f/f0") << QString("MW");
  sl_methods   << QString("2DSA-IT") << QString("PCSA-SL/DS/IS") << QString("2DSA-MC") << QString("raw");
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

      //set Object Name based on row number
      QString stchan      =  QString::number( ii ) + ": ";
      cb_type      -> setObjectName( stchan + "type" );
      cb_method    -> setObjectName( stchan + "method" );
      le_low       -> setObjectName( stchan + "low" );
      le_high      -> setObjectName( stchan + "high" );
      le_intval    -> setObjectName( stchan + "intval" );
      le_tol       -> setObjectName( stchan + "tol" );
      le_total     -> setObjectName( stchan + "total" );

      //set connecitons btw textChanged() and slot
      connect( le_low, SIGNAL( textChanged ( const QString& ) ),
	       this,   SLOT  ( verify_text ( const QString& ) ) );
      connect( le_high, SIGNAL( textChanged ( const QString& ) ),
	       this,   SLOT  ( verify_text ( const QString& ) ) );
      connect( le_intval, SIGNAL( textChanged ( const QString& ) ),
	       this,   SLOT  ( verify_text ( const QString& ) ) );
      connect( le_tol, SIGNAL( textChanged ( const QString& ) ),
	       this,   SLOT  ( verify_text ( const QString& ) ) );
      connect( le_total, SIGNAL( textChanged ( const QString& ) ),
	       this,   SLOT  ( verify_text ( const QString& ) ) );
      
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
  //Check for errors
  int syntax_errors = check_syntax();
  if ( syntax_errors != 0 )
    {
      QString mtitle_error    = tr( "Error" );
      QString message_error   = QString( tr( "There are %1 syntax errors!"
					     "\n\nPlease fix them (red fields) before accepting..." ))
	                        .arg( QString::number( syntax_errors ) );
      QMessageBox::critical( this, mtitle_error, message_error );
      return;
    }

  //save Gui params to reportGMP structure
  gui_to_report();

  close();
}


//Verify text
void US_ReportGui::verify_text( const QString& text )
{
  QObject* sobj       = sender();      // Sender object
  QString oname       = sobj->objectName();
  
  qDebug() << "QLineEdit oname -- " << oname;
  QLineEdit * curr_widget = NULL;

  if ( oname.contains(":") )
    curr_widget = containerWidget->findChild<QLineEdit *>( oname );
  else
    curr_widget = topContainerWidget->findChild<QLineEdit *>( oname );
  // else if ( oname == "tot_conc" )
  //   curr_widget = le_tot_conc;
  // else if ( oname == "rmsd" )
  //   curr_widget = le_rmsd_limit;
  // else if ( oname == "av_intensity" )
  //   curr_widget = le_av_intensity;

  QRegExp rx_double("\\d*\\.?\\d+");

  if ( curr_widget != NULL )
    {      
      if ( !rx_double.exactMatch( text ) )
	{
	  QPalette *palette = new QPalette();
	  palette->setColor(QPalette::Text,Qt::white);
	  palette->setColor(QPalette::Base,Qt::red);
	  curr_widget->setPalette(*palette);

	  isErrorField[ oname ] = true;
	}
      else
	{
	  QPalette *palette = new QPalette();
	  palette->setColor(QPalette::Text,Qt::black);
	  palette->setColor(QPalette::Base,Qt::white);
	  curr_widget->setPalette(*palette);

	  isErrorField[ oname ] = false;
	}
    }
}

//Transfer GUI params to US_ReportGMP structure
void US_ReportGui::gui_to_report( void )
{
  //Main params
  report->tot_conc            = le_tot_conc     ->text().toDouble();
  report->rmsd_limit          = le_rmsd_limit   ->text().toDouble();
  report->av_intensity        = le_av_intensity ->text().toDouble();

  QList< int > dhms_dur;
  int t_day        = (double)sb_durat_dd->value();
  int t_hour       = (double)sb_durat_hh->value();
  int t_minute     = (double)sb_durat_mm->value();
  int t_second     = (double)sb_durat_ss->value();

  dhms_dur << t_day << t_hour << t_minute << t_second;
  
  double exp_dur = 0;
  US_RunProtocol::timeFromList( exp_dur, dhms_dur );
  report->experiment_duration = exp_dur;

  //ReportItems
  for ( int ii = 0; ii < report->reportItems.size(); ii++ )
    {
      QString stchan      =  QString::number( ii ) + ": ";
      
      //type
      QComboBox * cb_type    = containerWidget->findChild<QComboBox *>( stchan + "type" );
      qDebug() << "ii, cb_type->currentText()" << ii << cb_type->currentText();
      report->reportItems[ ii ].type = cb_type->currentText();
      
      //method
      QComboBox * cb_method    = containerWidget->findChild<QComboBox *>( stchan + "method" );
      qDebug() << "ii, cb_method->currentText()" << ii << cb_method->currentText();
      report->reportItems[ ii ].method = cb_method->currentText();
      
      //range_low
      QLineEdit * le_low    = containerWidget->findChild<QLineEdit *>( stchan + "low" );
      qDebug() << "ii, le_low->text()" << ii << le_low->text();
      report->reportItems[ ii ].range_low = le_low->text().toDouble();

      //range_high
      QLineEdit * le_high    = containerWidget->findChild<QLineEdit *>( stchan + "high" );
      qDebug() << "ii, le_high->text()" << ii << le_high->text();
      report->reportItems[ ii ].range_high = le_high->text().toDouble();

      //integration value
      QLineEdit * le_intval  = containerWidget->findChild<QLineEdit *>( stchan + "intval" );
      qDebug() << "ii, le_intval->text()" << ii << le_intval->text();
      report->reportItems[ ii ].integration_val = le_intval->text().toDouble();
      
      //tolerance
      QLineEdit * le_tol  = containerWidget->findChild<QLineEdit *>( stchan + "tol" );
      qDebug() << "ii, le_tol->text()" << ii << le_tol->text();
      report->reportItems[ ii ].tolerance = le_tol->text().toDouble();
      
      //percent
      QLineEdit * le_total  = containerWidget->findChild<QLineEdit *>( stchan + "total" );
      qDebug() << "ii, le_total->text()" << ii << le_total->text();
      report->reportItems[ ii ].total_percent = le_total->text().toDouble();
    }
}

//Slot to cancel any updates on channel's report
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
  //Check if there are exsiting syntax errors:
  int syntax_errors = check_syntax();
  if ( syntax_errors != 0 )
    {
      QString mtitle_error    = tr( "Error" );
      QString message_error   = QString( tr( "There are %1 syntax errors!"
					     "\n\nPlease fix them (red fields) before adding new row..." ))
	                        .arg( QString::number( syntax_errors ) );
      QMessageBox::critical( this, mtitle_error, message_error );
      return;
    }

  //If no errors, save Gui params to reportGMP structure
  gui_to_report();

  //Add plain ReportItem
  US_ReportGMP::ReportItem initItem;
  
  initItem.type             = QString("s");
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
  //Check if there are exsiting syntax errors:
  int syntax_errors = check_syntax();
  if ( syntax_errors != 0 )
    {
      QString mtitle_error    = tr( "Error" );
      QString message_error   = QString( tr( "There are %1 syntax errors!"
					     "\n\nPlease fix them (red fields) before removing row..." ))
	                        .arg( QString::number( syntax_errors ) );
      QMessageBox::critical( this, mtitle_error, message_error );
      return;
    }

  //If no errors, remove last row
  report->reportItems.removeLast();

  //save Gui params to reportGMP structure
  gui_to_report();
  
  //re-build genL layout
  build_report_layout( );
}

//Check systax errors in the fields
int US_ReportGui::check_syntax( void )
{
  int syntax_errors = 0;
  
  QMap<QString, bool>::iterator ri;
  for ( ri = isErrorField.begin(); ri != isErrorField.end(); ++ri )
    {
      if ( ri.value() )
	{
	  qDebug() << "Error for object name -- " << ri.key();
	  ++syntax_errors;
	}
    }

  return syntax_errors;
}
