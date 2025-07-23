//! \us_failed_gmp_run_gui.cpp

#include "us_failed_gmp_run_gui.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_passwd.h"
#include "us_help.h"
#include "us_crypto.h"
#include "us_db2.h"

US_FailedRunGui::US_FailedRunGui( QMap <QString, QString> run_details ) : US_Widgets()
{

  //Get needed run params
  this->autoflowID = run_details[ "autoflowID" ];
  
  // Frame layout
   setPalette( US_GuiSettings::frameColor() );

   setWindowTitle( "Failed GMP Run Information" );
   setAttribute( Qt::WA_DeleteOnClose );

   QBoxLayout* topbox = new QVBoxLayout( this );
   topbox->setSpacing( 2 );

   int row = 0;
   QGridLayout* details = new QGridLayout();

   // Row 0
   QLabel* banner_type   = us_banner( tr( "Failed Run:" ) );
   //banner_type->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   details->addWidget( banner_type,      row++, 0, 1, 4 );

   //Row 0a
   QLabel* name         = us_label( tr( "Run Name:" ) );
   le_name              = us_lineedit( run_details[ "protocolName" ], 1, true );
   details->addWidget( name,           row,   0, 1, 1 );
   details->addWidget( le_name, row++, 1, 1, 3 );

   QLabel* banner_type1   = us_banner( tr( "Set Additional Information:" ) );
   //banner_type->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   details->addWidget( banner_type1,      row++, 0, 1, 4 );

   // Row 1
   QLabel* lb_stage     = us_label( tr( "Failing Stage:" ) );
   
   cb_stage             = us_comboBox();
   QStringList istages;
   istages << "Not Specified"
	   << "3. IMPORT"
	   << "4. EDIT"
	   << "5. ANALYSIS";
   cb_stage->addItems( istages );
   details->addWidget( lb_stage,        row,   0, 1, 2 );
   details->addWidget( cb_stage,        row++, 2, 1, 2 );
   cb_stage->setCurrentIndex( 0 );

   // Row 2
   QLabel* desc         = us_label( tr( "Reason for Failure:" ) );
   le_description       = us_lineedit( "", 0 );
   le_description->setPlaceholderText("optional");
   details->addWidget( desc,           row,   0, 1, 2 );
   details->addWidget( le_description, row++, 2, 1, 2 );

   topbox->addLayout( details );


   //Pushbuttons
   row = 0;
   QGridLayout* buttons = new QGridLayout();

   pb_cancel = us_pushbutton( tr( "Cancel" ) );
   //pb_cancel->setEnabled( true );
   connect( pb_cancel,      SIGNAL( clicked()  ),
	    this,           SLOT  ( cancel() ) );
   buttons->addWidget( pb_cancel, row, 0, 1, 1 );

   pb_save = us_pushbutton( tr( "Submit Failed Run Info" ) );
   pb_save->setEnabled( true );
   connect( pb_save, SIGNAL( clicked( ) ), this, SLOT( save_new( ) ) );
   buttons->addWidget( pb_save, row++, 1, 1, 1 );
   //pb_save->setEnabled(false);

   topbox->addLayout( buttons );

   setMinimumSize( 450 , 200 );
   adjustSize();

}

// Save info on failed run
void US_FailedRunGui::save_new( void )
{
  QString f_stage  = cb_stage       -> currentText();
  QString f_reason = le_description -> text();

  f_reason.simplified();

  if ( f_reason.isEmpty() )
    f_reason = "Not Specified";

  qDebug() << "Setting Info for FAILED run: stage, reason -- " << f_stage << f_reason;

  int autoflowFailedID = 0;
  
  US_Passwd pw;
  US_DB2* db = new US_DB2( pw.getPasswd() );
   
  QStringList qry;
  qry << "new_autoflow_failed_record"
      << autoflowID
      << f_stage
      << f_reason;

  autoflowFailedID = db->functionQuery( qry );

  if ( !autoflowFailedID )
    {
      QMessageBox::warning( this, tr( "AutoflowFailed Record Problem" ),
			    tr( "autoflowFailed: There was a problem with creating a record in autoflowFailed table \n" ) );
      
      return;
    }

  //update 'failed' in autoflow:
  qry.clear();
  qry << "update_autoflow_failedID"
      << autoflowID
      << QString::number( autoflowFailedID );
  
  qDebug() << "Query for update_autoflow_failedID -- " << qry;
    
  int status = db->statusQuery( qry );
  
  if ( status == US_DB2::NO_AUTOFLOW_RECORD )
    {
      QMessageBox::warning( this,
			    tr( "Autoflow's failedID field Not Updated" ),
			    tr( "No autoflow record\n"
				"associated with this experiment." ) );
      return;
    }

  //close
  close();

  //
  emit failed_status_set();
}


// Cancel
void US_FailedRunGui::cancel( void )
{
  close();
}
