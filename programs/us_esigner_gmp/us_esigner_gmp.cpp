#include <QPrinter>
#include <QPdfWriter>
#include <QPainter>

#include "us_esigner_gmp.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "us_protocol_util.h"
#include "us_run_protocol.h"
#include "us_math2.h"
#include "us_constants.h"
#include "us_solution_vals.h"
#include "us_tar.h"

#define MIN_NTC 25

// Constructor
US_eSignaturesGMP::US_eSignaturesGMP() : US_Widgets()
{
  auto_mode  = false;
  
  setWindowTitle( tr( "GMP e-Signatures"));
  //setPalette( US_GuiSettings::frameColor() );
  setPalette( US_GuiSettings::normalColor() );

  //main V-layout
  QVBoxLayout* mainLayout     = new QVBoxLayout( this );
  mainLayout->setSpacing        ( 2 );
  mainLayout->setContentsMargins( 2, 2, 2, 2 );

  //for setting global reviewers: people, permits
  QGridLayout* revGlobalGrid  = new QGridLayout();
  revGlobalGrid->setSpacing     ( 2 );
  revGlobalGrid->setContentsMargins( 2, 2, 2, 2 );

  QLabel* bn_revGlobal     = us_banner( tr( "Assign Global Reviewers from the List of Investigators:" ), 1 );
  QFontMetrics m (bn_revGlobal -> font()) ;
  int RowHeight = m.lineSpacing() ;
  bn_revGlobal -> setFixedHeight  (1.5 * RowHeight);

  lb_inv_search     = us_label( tr( "Investigator Search:" ) );
  le_inv_search     = us_lineedit();
  lw_inv_list       = us_listwidget();
  lw_inv_list       ->setSelectionMode( QAbstractItemView::ExtendedSelection );
  
  QLabel* lb_inv_smry     = us_label( tr( "Investigator Information:" ), 1 );
  te_inv_smry      = us_textedit();
  te_inv_smry      ->setTextColor( Qt::blue );
  te_inv_smry      ->setFont( QFont( US_Widgets::fixedFont().family(),
                                    US_GuiSettings::fontSize() - 1) );
  us_setReadOnly( te_inv_smry, true );

  lb_grev_search     = us_label( tr( "Global Reviewer Search:" ) );
  le_grev_search     = us_lineedit();
  lw_grev_list       = us_listwidget();
  lw_grev_list       ->setSelectionMode( QAbstractItemView::ExtendedSelection );
  
  QLabel* lb_grev_smry     = us_label( tr( "Reviewer Information:" ), 1 );
  te_grev_smry      = us_textedit();
  te_grev_smry      ->setTextColor( Qt::blue );
  te_grev_smry      ->setFont( QFont( US_Widgets::fixedFont().family(),
                                    US_GuiSettings::fontSize() - 1) );
  us_setReadOnly( te_grev_smry, true );

  pb_set_global_rev    = us_pushbutton( tr( "Set Investigator as Global Reviewer" ) );
  pb_unset_global_rev  = us_pushbutton( tr( "Remove from List of Global Reviewers" ) );

  pb_set_global_rev   ->setEnabled( false );
  pb_unset_global_rev ->setEnabled( false );

  int row = 0;
  revGlobalGrid -> addWidget( bn_revGlobal,             row++,    0, 1,  20 );
  
  revGlobalGrid -> addWidget( lb_inv_search,   row,      0, 1,  2  );
  revGlobalGrid -> addWidget( le_inv_search,   row,      2, 1,  3  );

  revGlobalGrid -> addWidget( lb_inv_smry,     row,      5, 1,  5  );
  
  revGlobalGrid -> addWidget( lb_grev_search,  row,      10, 1, 2  );
  revGlobalGrid -> addWidget( le_grev_search,  row,      12, 1, 3  );

  revGlobalGrid -> addWidget( lb_grev_smry,    row++,    15, 1, 5  );
  
  revGlobalGrid -> addWidget( lw_inv_list,     row,      0,  3,  5  );
  revGlobalGrid -> addWidget( te_inv_smry,     row,      5,  3,  5  );
  revGlobalGrid -> addWidget( lw_grev_list,    row,      10, 3,  5  );
  revGlobalGrid -> addWidget( te_grev_smry,    row++,    15, 3,  5  );

  row += 5;
  revGlobalGrid -> addWidget( pb_set_global_rev,       row,       0,  1,  10  );
  revGlobalGrid -> addWidget( pb_unset_global_rev,     row++,     10, 1,  10  );

  connect( le_inv_search, SIGNAL( textChanged( const QString& ) ), 
	   SLOT  ( limit_inv_names( const QString& ) ) );
  connect( lw_inv_list, SIGNAL( itemClicked( QListWidgetItem* ) ), 
	   SLOT  ( get_inv_data  ( QListWidgetItem* ) ) );

  connect( le_grev_search, SIGNAL( textChanged( const QString& ) ), 
	   SLOT  ( limit_grev_names( const QString& ) ) );
  connect( lw_grev_list, SIGNAL( itemClicked( QListWidgetItem* ) ), 
	   SLOT  ( get_grev_data  ( QListWidgetItem* ) ) );

  connect( pb_set_global_rev,   SIGNAL( clicked() ), SLOT ( set_greviewer() ) );
  connect( pb_unset_global_rev, SIGNAL( clicked() ), SLOT ( unset_greviewer() ) );
 
  //for setting oper, revs. for selected GMP Run
  QGridLayout*  revOperGMPRunGrid  = new QGridLayout();
  revOperGMPRunGrid->setSpacing     ( 2 );
  revOperGMPRunGrid->setContentsMargins( 1, 1, 1, 1 );

  QLabel* bn_revOperGMP     = us_banner( tr( "Assign Operator and Reviewer(s) for GMP Run:" ), 1 );
  bn_revOperGMP -> setFixedHeight  (1.5 * RowHeight);

  pb_selRun_operRev_set = us_pushbutton( tr( "Select GMP Run" ) );
  pb_set_operRev        = us_pushbutton( tr( "Assign/Change Operators and Reviewers" ) );
  pb_set_operRev -> setEnabled( false );

  pb_add_oper      = us_pushbutton( tr( "Add" ) );
  pb_add_oper       -> setEnabled( false );
  pb_remove_oper   = us_pushbutton( tr( "Remove Last" ) );
  pb_remove_oper   -> setEnabled( false );
  pb_add_rev       = us_pushbutton( tr( "Add" ) );
  pb_remove_rev    = us_pushbutton( tr( "Remove Last" ) );
  
  QLabel* lb_run_name       = us_label( "Run Name:" );
  QLabel* lb_optima_name    = us_label( "Optima:" );
  QLabel* lb_operator_names = us_label( "Assigned Operators:" );
  QLabel* lb_reviewer_names = us_label( "Assigned Reviewers:" );
  
  QLabel* lb_choose_oper      = us_label( "Choose Operator:" );
  QLabel* lb_choose_rev       = us_label( "Choose Reviewer:" );
  QLabel* lb_opers_to_assign  = us_label( "Operators to Assign:" );
  QLabel* lb_revs_to_assign   = us_label( "Reviewers to Assign:" );
  
  le_run_name       = us_lineedit( tr(""), 0, true );
  le_optima_name    = us_lineedit( tr(""), 0, true );


  te_operator_names    = us_textedit();
  //te_operator_names    ->setTextColor( Qt::blue );
  te_operator_names    -> setFixedHeight  ( RowHeight * 2 );
  te_operator_names    ->setFont( QFont( US_Widgets::fixedFont().family(),
					 US_GuiSettings::fontSize() - 1) );
  us_setReadOnly( te_operator_names, true );

  te_reviewer_names    = us_textedit();
  //te_reviewer_names    ->setTextColor( Qt::blue );
  te_reviewer_names    -> setFixedHeight  ( RowHeight * 3 );
  te_reviewer_names    ->setFont( QFont( US_Widgets::fixedFont().family(),
					 US_GuiSettings::fontSize() - 1) );
  us_setReadOnly( te_reviewer_names, true );


  te_opers_to_assign    = us_textedit();
  //te_opers_to_assign    ->setTextColor( Qt::blue );
  te_opers_to_assign    -> setFixedHeight  ( RowHeight * 2 );
  te_opers_to_assign    ->setFont( QFont( US_Widgets::fixedFont().family(),
					 US_GuiSettings::fontSize() - 1) );
  us_setReadOnly( te_opers_to_assign, true );

  te_revs_to_assign    = us_textedit();
  //te_revs_to_assign    ->setTextColor( Qt::blue );
  te_revs_to_assign    -> setFixedHeight  ( RowHeight * 3 );
  te_revs_to_assign    ->setFont( QFont( US_Widgets::fixedFont().family(),
					 US_GuiSettings::fontSize() - 1) );
  us_setReadOnly( te_revs_to_assign, true );
  
  
  cb_choose_operator   = new QComboBox( this );
  cb_choose_rev        = new QComboBox( this );
   
  row = 0;
  revOperGMPRunGrid -> addWidget( bn_revOperGMP,          row++,    0,  1,  14 );
  
  revOperGMPRunGrid -> addWidget( pb_selRun_operRev_set,  row++,    0,  1,  6 );

  revOperGMPRunGrid -> addWidget( lb_run_name,            row,      0,  1,  2 );
  revOperGMPRunGrid -> addWidget( le_run_name,            row,      2,  1,  4 );
  revOperGMPRunGrid -> addWidget( lb_choose_oper,         row,      7,  1,  2 );
  revOperGMPRunGrid -> addWidget( cb_choose_operator,     row,      9,  1,  3 );
  revOperGMPRunGrid -> addWidget( pb_add_oper,            row++,    12, 1,  2 );

  revOperGMPRunGrid -> addWidget( lb_optima_name,         row,      0,  1,  2 );
  revOperGMPRunGrid -> addWidget( le_optima_name,         row,      2,  1,  4 );
  revOperGMPRunGrid -> addWidget( lb_choose_rev,          row,      7,  1,  2 );
  revOperGMPRunGrid -> addWidget( cb_choose_rev,          row,      9,  1,  3 );
  revOperGMPRunGrid -> addWidget( pb_add_rev,             row++,    12, 1,  2 );
  
  revOperGMPRunGrid -> addWidget( lb_operator_names,      row,      0, 1,  2 );
  revOperGMPRunGrid -> addWidget( te_operator_names,      row,      2, 1,  4 );
  revOperGMPRunGrid -> addWidget( lb_opers_to_assign,     row,      7,  1,  2 );
  revOperGMPRunGrid -> addWidget( te_opers_to_assign,     row,      9,  1,  3 );
  revOperGMPRunGrid -> addWidget( pb_remove_oper,         row++,    12, 1,  2 );

  revOperGMPRunGrid -> addWidget( lb_reviewer_names,      row,      0,  1,  2 );
  revOperGMPRunGrid -> addWidget( te_reviewer_names,      row,      2,  1,  4 );
  revOperGMPRunGrid -> addWidget( lb_revs_to_assign,      row,      7,  1,  2 );
  revOperGMPRunGrid -> addWidget( te_revs_to_assign,      row,      9,  1,  3 );
  revOperGMPRunGrid -> addWidget( pb_remove_rev,          row++,    12, 1,  2 );

  revOperGMPRunGrid -> addWidget( pb_set_operRev,         row++,    7,  1,  6 );

  connect( pb_selRun_operRev_set,   SIGNAL( clicked() ), SLOT ( selectGMPRun() ) );
  connect( pb_set_operRev, SIGNAL( clicked() ), SLOT ( assignOperRevs() ) );
  connect( pb_add_oper, SIGNAL( clicked() ), SLOT ( addOpertoList() ) );
  connect( pb_remove_oper, SIGNAL( clicked() ), SLOT ( removeOperfromList() ) );
  connect( pb_add_rev, SIGNAL( clicked() ), SLOT ( addRevtoList() ) );
  connect( pb_remove_rev, SIGNAL( clicked() ), SLOT ( removeRevfromList() ) );

  
  //for eSigning selected GMP Run's Report with Assigned Operator && Reviewers
  QGridLayout* eSignGMPRunGrid     = new QGridLayout();
  eSignGMPRunGrid->setSpacing        ( 2 );
  eSignGMPRunGrid->setContentsMargins( 1, 1, 1, 1 );

  QLabel* bn_eSignGMP     = us_banner( tr( "Manage e-Signatures for GMP Run:" ), 1 );
  bn_eSignGMP -> setFixedHeight  (1.5 * RowHeight);

  pb_loadreport_db          =  us_pushbutton( tr( "Load GMP Report from DB (.PDF) with Assigned Reviewers" ) );
  pb_view_report_db         =  us_pushbutton( tr( "Review Downloaded Report" ) );
  pb_esign_report           =  us_pushbutton( tr( "e-Sign Report" ) );

  pb_view_report_db  -> setEnabled( false );
  pb_esign_report    -> setEnabled( false );

  pb_esign_report    -> setStyleSheet( tr("QPushButton::enabled {background: #556B2F; color: lightgray; }"
					  "QPushButton::disabled {background: #90EE90; color: black}" ));

  QLabel* lb_loaded_run_db  = us_label( tr( "Loaded GMP Report:" ) );
  le_loaded_run_db          = us_lineedit( tr(""), 0, true );
  
  //Filename path
  QLabel*      lb_fpath_info = us_label( tr( "Report File \nLocation:" ) );
  te_fpath_info =  us_textedit();
  te_fpath_info -> setFixedHeight  ( RowHeight * 3 );
  te_fpath_info -> setText( tr( "" ) );
  us_setReadOnly( te_fpath_info, true );

  //Status
  QLabel*      lb_eSign_status = us_label( tr( "e-Signing Status:" ) );
  le_eSign_status              = us_lineedit( tr(""), 0, true );
  pb_view_eSigns  =  us_pushbutton( tr( "View e-Signatures" ) );
  pb_view_eSigns  -> setEnabled( false );
  
  row = 0;
  eSignGMPRunGrid -> addWidget( bn_eSignGMP,        row++,    0,  1,  14 );

  eSignGMPRunGrid -> addWidget( pb_loadreport_db,   row,      0,  1,  7);
  eSignGMPRunGrid -> addWidget( lb_eSign_status,    row,      8,  1,  3);
  eSignGMPRunGrid -> addWidget( le_eSign_status,    row++,    11, 1,  3);
  
  eSignGMPRunGrid -> addWidget( lb_loaded_run_db,   row,      0, 1,  2);
  eSignGMPRunGrid -> addWidget( le_loaded_run_db,   row,      2, 1,  5);

  eSignGMPRunGrid -> addWidget( pb_view_report_db,  row,      8,   1,  3 );
  eSignGMPRunGrid -> addWidget( pb_view_eSigns,     row++,    11,  1,  3 );

  eSignGMPRunGrid -> addWidget( lb_fpath_info,      row,      0, 1,  2);
  eSignGMPRunGrid -> addWidget( te_fpath_info,      row,      2, 1,  5);

  eSignGMPRunGrid -> addWidget( pb_esign_report,    row++,    9,  1,  4 );
  
  connect( pb_loadreport_db,  SIGNAL( clicked() ), SLOT ( loadGMPReportDB_assigned() ) );
  connect( pb_view_report_db, SIGNAL( clicked() ), SLOT ( view_report_db() ) );
  connect( pb_esign_report,   SIGNAL( clicked() ), SLOT ( esign_report() ) );
  connect( pb_view_eSigns,    SIGNAL( clicked() ), SLOT ( view_eSignatures() ) );
  
  //Setting top-level Layouts:
  mainLayout -> addLayout( revGlobalGrid );
  mainLayout -> addLayout( revOperGMPRunGrid );
  mainLayout -> addLayout( eSignGMPRunGrid );

  mainLayout -> addStretch();

  //initialize investigators, global reviewers
  init_invs();
  init_grevs();
			 
  resize( 1200, 700 );
  adjustSize();
}


//For autoflow: constructor
US_eSignaturesGMP::US_eSignaturesGMP( QString a_mode ) : US_Widgets()
{
  auto_mode  = true;

  setWindowTitle( tr( "GMP e-Signatures"));
  setPalette( US_GuiSettings::frameColor() );

  // primary layouts
  QHBoxLayout* mainLayout     = new QHBoxLayout( this );
  mainLayout->setSpacing        ( 2 );
  mainLayout->setContentsMargins( 2, 2, 2, 2 );

  resize( 1000, 700 );
  adjustSize();
}


//For the end of 1. EXP: defined by admin
US_eSignaturesGMP::US_eSignaturesGMP( QMap< QString, QString > & protocol_details ) : US_Widgets()
{
  this->protocol_details = protocol_details;
  
  setWindowTitle( tr( "GMP e-Signatures"));
  setPalette( US_GuiSettings::frameColor() );

  // primary layouts
  QHBoxLayout* mainLayout     = new QHBoxLayout( this );
  mainLayout->setSpacing        ( 2 );
  mainLayout->setContentsMargins( 2, 2, 2, 2 );

  resize( 1000, 700 );
  adjustSize();
}


//init investigators list
void US_eSignaturesGMP::init_invs( void )
{
  investigators. clear();
  
  US_Passwd   pw;
  QString     masterPW  = pw.getPasswd();
  US_DB2      db( masterPW );  // New constructor

  if ( db.lastErrno() != US_DB2::OK )
    {
      // Error message here
      QMessageBox::information( this,
				tr( "DB Connection Problem" ),
				tr( "There was an error connecting to the database:\n" ) 
				+ db.lastError() );
      return;
    }

  lw_inv_list-> clear();

  QStringList query;
  query << "get_people_inv" << "%" + le_inv_search->text() + "%";
  qDebug() << "init_invs(), query --  " << query;
  db.query( query );

  US_InvestigatorData data;
  int inv = US_Settings::us_inv_ID();
  int lev = US_Settings::us_inv_level();
  
  while ( db.next() )
    {
      data.invID     = db.value( 0 ).toInt();
      data.lastName  = db.value( 1 ).toString();
      data.firstName = db.value( 2 ).toString();
      data.ulev      = db.value( 4 ).toInt();

      // qDebug() << "init_invs(): lname, fname, ulev -- "
      // 	       <<  data.lastName << data.firstName << data.ulev;
	
      if ( lev < 3  &&  inv != data.invID )
	continue;

      //Filter by userlevel: cannot be UL=1
      if ( data.ulev < 2 )
	continue;

      investigators << data;
      
      lw_inv_list-> addItem( new QListWidgetItem( 
						 "InvID: (" + QString::number( data.invID ) + "), " + 
						 data.lastName + ", " + data.firstName ) );
    }
}


void US_eSignaturesGMP::limit_inv_names( const QString& s )
{
  
  lw_inv_list->clear();
  
  for ( int i = 0; i < investigators.size(); i++ )
   {
     if ( investigators[ i ].lastName.contains( 
					       QRegExp( ".*" + s + ".*", Qt::CaseInsensitive ) ) ||
	  investigators[ i ].firstName.contains(
						QRegExp( ".*" + s + ".*", Qt::CaseInsensitive ) ) )
       lw_inv_list->addItem( new QListWidgetItem(
						 "InvID: (" + QString::number( investigators[ i ].invID ) + "), " +
						 investigators[ i ].lastName + ", " + 
						 investigators[ i ].firstName ) );
   }
}

void US_eSignaturesGMP::limit_grev_names( const QString& s )
{
  lw_grev_list->clear();
  
  for ( int i = 0; i < g_reviewers.size(); i++ )
   {
     if ( g_reviewers[ i ].lastName.contains( 
					       QRegExp( ".*" + s + ".*", Qt::CaseInsensitive ) ) ||
	  g_reviewers[ i ].firstName.contains(
						QRegExp( ".*" + s + ".*", Qt::CaseInsensitive ) ) )
       lw_grev_list->addItem( new QListWidgetItem(
						 "InvID: (" + QString::number( g_reviewers[ i ].invID ) + "), " +
						 g_reviewers[ i ].lastName + ", " + 
						 g_reviewers[ i ].firstName ) );
   }
}


void US_eSignaturesGMP::get_inv_data( QListWidgetItem* item )
{
   QString entry = item->text();
   
   int     left  = entry.indexOf( '(' ) + 1;
   int     right = entry.indexOf( ')' );
   QString invID = entry.mid( left, right - left );

   US_Passwd   pw;
   QString     masterPW  = pw.getPasswd();
   US_DB2      db( masterPW ); 

   if ( db.lastErrno() != US_DB2::OK )
   {
      QMessageBox::information( this,
         tr( "DB Connection Problem" ),
         tr( "There was an error connecting to the database:\n" ) 
             + db.lastError() );
      return;
   } 

   QStringList query;
   query << "get_person_info" << invID; 
      
   db.query( query );
   db.next();

   info.invID        = invID.toInt();
   info.firstName    = db.value( 0 ).toString();
   info.lastName     = db.value( 1 ).toString();
   info.address      = db.value( 2 ).toString();
   info.city         = db.value( 3 ).toString();
   info.state        = db.value( 4 ).toString();
   info.zip          = db.value( 5 ).toString();
   info.phone        = db.value( 6 ).toString();
   info.organization = db.value( 7 ).toString();
   info.email        = db.value( 8 ).toString();
   info.invGuid      = db.value( 9 ).toString();
   info.ulev         = db.value( 10 ).toInt();
   info.gmpReviewer  = db.value( 11 ).toInt();

   te_inv_smry->setText( get_inv_or_grev_smry( info, "Investigator") );


   pb_set_global_rev -> setEnabled( true );
}

// Message string for investigator summary
QString US_eSignaturesGMP::get_inv_or_grev_smry( US_InvestigatorData p_info, QString p_type )
{
  QString smry;
  QStringList mlines;

  mlines << QString( tr( "== Selected %1 ==\n" )). arg( p_type );
    
  mlines << "Last Name:\n "       +  p_info.lastName; 
  mlines << "First Name:\n "      +  p_info.firstName;
  mlines << "User Level:\n "      +  QString::number( p_info.ulev ) ;
  QString grev_set = (p_info.gmpReviewer) ? "YES" : "NO" ;
  mlines << "GMP Reviewer:\n "    +  grev_set;
  mlines << "Email:\n "           +  p_info.email       ; 
  mlines << "Organization:\n "    +  p_info.organization; 

  // mlines << "Investigator ID:\n\t" +  p_info.invID ;
  // mlines << "Address:\t"         +  p_info.address     ;
  // mlines << "City:\t"            +  p_info.city        ; 
  // mlines << "State:\t"           +  p_info.state       ; 
  // mlines << "Zip:\t"             +  p_info.zip         ; 
  // mlines << "Phone:\t"           +  p_info.phone       ; 
  
  for ( int ii = 0; ii < mlines.count(); ii++ )
    smry         += mlines[ ii ] + "\n";
   

  return smry;
}


//init global reviewers list
void US_eSignaturesGMP::init_grevs( void )
{
  g_reviewers. clear();
  lw_grev_list   -> clear();
  cb_choose_rev  -> clear();
  
  US_Passwd   pw;
  QString     masterPW  = pw.getPasswd();
  US_DB2      db( masterPW );  // New constructor

  if ( db.lastErrno() != US_DB2::OK )
    {
      // Error message here
      QMessageBox::information( this,
				tr( "DB Connection Problem" ),
				tr( "There was an error connecting to the database:\n" ) 
				+ db.lastError() );
      return;
    }
  
  QStringList query;
  query << "get_people_grev" << "%" + le_grev_search->text() + "%";
  qDebug() << "init_invs(), query --  " << query;
  db.query( query );

  US_InvestigatorData data;
  int inv = US_Settings::us_inv_ID();
  int lev = US_Settings::us_inv_level();
  
  while ( db.next() )
    {
      data.invID     = db.value( 0 ).toInt();
      data.lastName  = db.value( 1 ).toString();
      data.firstName = db.value( 2 ).toString();
            
      if ( lev < 3  &&  inv != data.invID )
	continue;

      g_reviewers << data;
      //populate lists
      lw_grev_list-> addItem( new QListWidgetItem( 
						  "InvID: (" + QString::number( data.invID ) + "), " + 
						  data.lastName + ", " + data.firstName ) );
      cb_choose_rev->addItem( QString::number( data.invID ) + ". " + 
			      data.lastName + ", " + data.firstName );
      
    }
}


void US_eSignaturesGMP::get_grev_data( QListWidgetItem* item )
{
   QString entry = item->text();
   
   int     left  = entry.indexOf( '(' ) + 1;
   int     right = entry.indexOf( ')' );
   QString invID = entry.mid( left, right - left );

   US_Passwd   pw;
   QString     masterPW  = pw.getPasswd();
   US_DB2      db( masterPW ); 

   if ( db.lastErrno() != US_DB2::OK )
   {
      QMessageBox::information( this,
         tr( "DB Connection Problem" ),
         tr( "There was an error connecting to the database:\n" ) 
             + db.lastError() );
      return;
   } 

   QStringList query;
   query << "get_person_info" << invID; 
      
   db.query( query );
   db.next();

   info_grev.invID        = invID.toInt();
   info_grev.firstName    = db.value( 0 ).toString();
   info_grev.lastName     = db.value( 1 ).toString();
   info_grev.address      = db.value( 2 ).toString();
   info_grev.city         = db.value( 3 ).toString();
   info_grev.state        = db.value( 4 ).toString();
   info_grev.zip          = db.value( 5 ).toString();
   info_grev.phone        = db.value( 6 ).toString();
   info_grev.organization = db.value( 7 ).toString();
   info_grev.email        = db.value( 8 ).toString();
   info_grev.invGuid      = db.value( 9 ).toString();
   info_grev.ulev         = db.value( 10 ).toInt();
   info_grev.gmpReviewer  = db.value( 11 ).toInt();

   te_grev_smry->setText( get_inv_or_grev_smry( info_grev, "Reviewer") );

   pb_unset_global_rev -> setEnabled( true );
}

void US_eSignaturesGMP::set_greviewer()
{

  QString entry = lw_inv_list->currentItem()->text();
  qDebug() << "Set gRev: -- " << entry;

  int     left  = entry.indexOf( '(' ) + 1;
  int     right = entry.indexOf( ')' );
  QString invID = entry.mid( left, right - left );

  US_Passwd   pw;
  QString     masterPW  = pw.getPasswd();
  US_DB2      db( masterPW ); 
  
   if ( db.lastErrno() != US_DB2::OK )
     {
       QMessageBox::information( this,
				 tr( "DB Connection Problem" ),
				 tr( "There was an error connecting to the database:\n" ) 
				 + db.lastError() );
       return;
     }
   
   QStringList query;
   query << "set_person_grev_status" << invID;

   db.query( query );
   db.next();

   //update both inv && grev lw_lists
   lw_inv_list -> clear();
   lw_grev_list-> clear();

   le_inv_search   ->setText("");
   le_grev_search  ->setText("");
   
   te_inv_smry -> clear();
   te_grev_smry-> clear();
   
   init_invs();
   init_grevs();
}

void US_eSignaturesGMP::unset_greviewer()
{
  QString entry = lw_grev_list->currentItem()->text();
  qDebug() << "[UN]Set gRev: -- " << entry;
  
  int     left  = entry.indexOf( '(' ) + 1;
  int     right = entry.indexOf( ')' );
  QString invID = entry.mid( left, right - left );
  
  US_Passwd   pw;
  QString     masterPW  = pw.getPasswd();
  US_DB2      db( masterPW ); 
  
  if ( db.lastErrno() != US_DB2::OK )
    {
      QMessageBox::information( this,
				tr( "DB Connection Problem" ),
				tr( "There was an error connecting to the database:\n" ) 
				+ db.lastError() );
      return;
    }
  
   QStringList query;
   query << "unset_person_grev_status" << invID;

   db.query( query );
   db.next();
  
   //update both inv && grev lw_lists
   lw_inv_list -> clear();
   lw_grev_list-> clear();

   le_inv_search   ->setText("");
   le_grev_search  ->setText("");
   
   te_inv_smry -> clear();
   te_grev_smry-> clear();
    
   init_invs();
   init_grevs();

   ///Debug
   for(int i=0; i<investigators.size(); ++i )
     qDebug() << "inv after unsetting -- " << investigators[i].lastName;

}


void US_eSignaturesGMP::selectGMPRun( void )
{
  list_all_autoflow_records( autoflowdata  );

  QString pdtitle( tr( "Select GMP Run" ) );
  QStringList hdrs;
  int         prx;
  hdrs << "ID"
       << "Run Name"
       << "Optima Name"
       << "Created"
       << "Run Status"
       << "Stage"
       << "GMP";
  
  QString autoflow_btn = "AUTOFLOW_GMP_REPORT";

  pdiag_autoflow = new US_SelectItem( autoflowdata, hdrs, pdtitle, &prx, autoflow_btn, -2 );

  QString autoflow_id_selected("");
  if ( pdiag_autoflow->exec() == QDialog::Accepted )
    {
      autoflow_id_selected  = autoflowdata[ prx ][ 0 ];

      //reset Gui && internal structures
      reset_set_revOper_panel();
    }
  else
    return;

  // Get detailed info on the autoflow record
  int autoflowID = autoflow_id_selected.toInt();
  gmp_run_details = read_autoflow_record( autoflowID );

  set_revOper_panel_gui();

  //Enable button to change/set assigned oper(s) / rev(s) 
  pb_add_oper    -> setEnabled( true );
  pb_remove_oper -> setEnabled( true );
}


void US_eSignaturesGMP::reset_set_revOper_panel( void )
{
  le_run_name        -> clear();
  le_optima_name     -> clear();
  te_operator_names  -> clear();
  te_reviewer_names  -> clear();
  cb_choose_operator -> clear();

  te_opers_to_assign -> clear();
  te_revs_to_assign  -> clear();

  //main QMap for the loaded GMP run
  gmp_run_details   .clear();
  eSign_details     .clear();
  isEsignRecord     = false;
  
  pb_set_operRev -> setEnabled( false );
}

void US_eSignaturesGMP::set_revOper_panel_gui( void )
{
  QString full_runname = gmp_run_details[ "filename" ];
  QString FullRunName_auto = gmp_run_details[ "experimentName" ] + "-run" + gmp_run_details[ "runID" ];
  if ( full_runname.contains(",") && full_runname.contains("IP") && full_runname.contains("RI") )
    FullRunName_auto += " (combined RI+IP)";
    
  QString OptimaName = gmp_run_details["OptimaName"]; 
  le_run_name        -> setText( FullRunName_auto );
  le_optima_name     -> setText( OptimaName );

  //Set Operators for Optima:
  QStringList sl_operators = read_operators( gmp_run_details[ "OptimaID" ] );
  cb_choose_operator -> addItems( sl_operators );

  //Read autoflowGMPReportEsign record by autoflowID:
  eSign_details = read_autoflowGMPReportEsign_record( gmp_run_details[ "autoflowID" ] );
  
  //&& Set defined Operator/Reviewers (if any)
  if ( !eSign_details. contains("operatorListJson") )
    te_operator_names  -> setText( "NOT SET" );
  else
    {
      QJsonDocument jsonDocOperList = QJsonDocument::fromJson( eSign_details[ "operatorListJson" ] .toUtf8() );
      te_operator_names -> setText( get_assigned_oper_revs( jsonDocOperList ) );

      
    }
  
  if ( !eSign_details. contains("reviewersListJson") )
    te_reviewer_names  -> setText( "NOT SET" );
  else
    {
      QJsonDocument jsonDocRevList  = QJsonDocument::fromJson( eSign_details[ "reviewersListJson" ] .toUtf8() );
      te_reviewer_names  -> setText( get_assigned_oper_revs( jsonDocRevList ) );
	
    }
}

//form a string of opers/revs out of jsonDoc
QString US_eSignaturesGMP::get_assigned_oper_revs( QJsonDocument jsonDoc )
{
  QString smry;
  QStringList assigned_list;
  
  if ( !jsonDoc. isArray() )
    {
      qDebug() << "jsonDoc not a JSON, and/or not an JSON Array!";
      return smry;
    }
  
  QJsonArray jsonDoc_array  = jsonDoc.array();
  for (int i = 0; i < jsonDoc_array.size(); ++i )
    assigned_list << jsonDoc_array[i].toString();
  
  for ( int ii = 0; ii < assigned_list.count(); ii++ )
    {
      smry += assigned_list[ ii ];
      if ( ii != assigned_list.count() -1 )
	smry += "\n";
    }
  
  return smry;
}

// Query autoflow (history) table for records
int US_eSignaturesGMP::list_all_autoflow_records( QList< QStringList >& autoflowdata )
{
  int nrecs        = 0;   
  autoflowdata.clear();

  QStringList qry;
  
  US_Passwd pw;
  US_DB2* db = new US_DB2( pw.getPasswd() );
  
  if ( db->lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "LIMS DB Connection Problem" ),
			    tr( "Could not connect to database \n" ) + db->lastError() );

      return nrecs;
    }
  
  //Check user level && ID
  QStringList defaultDB = US_Settings::defaultDB();
  QString user_guid   = defaultDB.at( 9 );
  
  //get personID from personGUID
  qry.clear();
  qry << QString( "get_personID_from_GUID" ) << user_guid;
  db->query( qry );
  
  int user_id = 0;
  
  if ( db->next() )
    user_id = db->value( 0 ).toInt();


  //deal with autoflow descriptions
  qry. clear();
  qry << "get_autoflow_desc";
  db->query( qry );

  while ( db->next() )
    {
      QStringList autoflowentry;
      QString id                 = db->value( 0 ).toString();
      QString runname            = db->value( 5 ).toString();
      QString status             = db->value( 8 ).toString();
      QString optimaname         = db->value( 10 ).toString();
      
      QDateTime time_started     = db->value( 11 ).toDateTime().toUTC();
      QString invID              = db->value( 12 ).toString();

      QDateTime time_created     = db->value( 13 ).toDateTime().toUTC();
      QString gmpRun             = db->value( 14 ).toString();
      QString full_runname       = db->value( 15 ).toString();

      QString operatorID         = db->value( 16 ).toString();
      QString devRecord          = db->value( 18 ).toString();

      QDateTime local(QDateTime::currentDateTime());

      if ( devRecord == "Processed" )
      	continue;
      
      //process runname: if combined, correct for nicer appearance
      if ( full_runname.contains(",") && full_runname.contains("IP") && full_runname.contains("RI") )
	{
	  QString full_runname_edited  = full_runname.split(",")[0];
	  full_runname_edited.chop(3);

	  full_runname = full_runname_edited + " (combined RI+IP) ";
	  runname += " (combined RI+IP) ";
	}
      
      autoflowentry << id << runname << optimaname  << time_created.toString(); // << time_started.toString(); // << local.toString( Qt::ISODate );

      if ( time_started.toString().isEmpty() )
	autoflowentry << QString( tr( "NOT STARTED" ) );
      else
	{
	  if ( status == "LIVE_UPDATE" )
	    autoflowentry << QString( tr( "RUNNING" ) );
	  if ( status == "EDITING" || status == "EDIT_DATA" || status == "ANALYSIS" || status == "REPORT" )
	    autoflowentry << QString( tr( "COMPLETED" ) );
	    //autoflowentry << time_started.toString();
	}

      if ( status == "EDITING" )
	status = "LIMS_IMPORT";
      
      autoflowentry << status << gmpRun;

      //Check user level && GUID; if <3, check if the user is operator || investigator
      if ( US_Settings::us_inv_level() < 3 )
	{
	  qDebug() << "User level low: " << US_Settings::us_inv_level();
	  qDebug() << "user_id, operatorID.toInt(), invID.toInt() -- " << user_id << operatorID.toInt() << invID.toInt();

	  //if ( user_id && ( user_id == operatorID.toInt() || user_id == invID.toInt() ) )
	  if ( user_id && user_id == invID.toInt() )
	    {//Do we allow operator as defined in autoflow record to also see reports?? 
	    
	      autoflowdata  << autoflowentry;
	      nrecs++;
	    }
	}
      else
	{
	  autoflowdata  << autoflowentry;
	  nrecs++;
	}
      
    }

  return nrecs;
}


// Query autoflow record
QMap< QString, QString>  US_eSignaturesGMP::read_autoflow_record( int autoflowID  )
{
   // Check DB connection
   US_Passwd pw;
   QString masterpw = pw.getPasswd();
   US_DB2* db = new US_DB2( masterpw );

   QMap <QString, QString> run_info;
   
   if ( db->lastErrno() != US_DB2::OK )
     {
       QMessageBox::warning( this, tr( "Connection Problem" ),
			     tr( "Read protocol: Could not connect to database \n" ) + db->lastError() );
       return run_info;
     }

   QStringList qry;
   qry << "read_autoflow_record"
       << QString::number( autoflowID );
   
   db->query( qry );

   if ( db->lastErrno() == US_DB2::OK )      // Autoflow record exists
     {
       while ( db->next() )
	 {
	   run_info[ "protocolName" ]   = db->value( 0 ).toString();
	   run_info[ "CellChNumber" ]   = db->value( 1 ).toString();
	   run_info[ "TripleNumber" ]   = db->value( 2 ).toString();
	   run_info[ "duration" ]       = db->value( 3 ).toString();
	   run_info[ "experimentName" ] = db->value( 4 ).toString();
	   run_info[ "experimentId" ]   = db->value( 5 ).toString();
	   run_info[ "runID" ]          = db->value( 6 ).toString();
	   run_info[ "status" ]         = db->value( 7 ).toString();
           run_info[ "dataPath" ]       = db->value( 8 ).toString();   
	   run_info[ "OptimaName" ]     = db->value( 9 ).toString();
	   run_info[ "runStarted" ]     = db->value( 10 ).toString();
	   run_info[ "invID_passed" ]   = db->value( 11 ).toString();

	   run_info[ "correctRadii" ]   = db->value( 13 ).toString();
	   run_info[ "expAborted" ]     = db->value( 14 ).toString();
	   run_info[ "label" ]          = db->value( 15 ).toString();
	   run_info[ "gmpRun" ]         = db->value( 16 ).toString();

	   run_info[ "filename" ]       = db->value( 17 ).toString();
	   run_info[ "aprofileguid" ]   = db->value( 18 ).toString();

	   run_info[ "analysisIDs" ]    = db->value( 19 ).toString();
	   run_info[ "intensityID" ]    = db->value( 20 ).toString();

	   run_info[ "statusID" ]       = db->value( 21 ).toString();
	   	   
	 }
     }

   run_info[ "autoflowID" ]  = QString::number( autoflowID );
   
   qry. clear();
   QString xmlstr( "" );
   US_ProtocolUtil::read_record_auto(  run_info[ "protocolName" ],
				       run_info[ "invID_passed" ].toInt(),
				       &xmlstr, NULL, db );
   QXmlStreamReader xmli( xmlstr );
   US_RunProtocol currProto;
   currProto. fromXml( xmli );

   qDebug() << "Instrument ID from protocol -- " << currProto.rpRotor. instID;
   run_info[ "OptimaID" ] = QString::number( currProto.rpRotor. instID );
   
   return run_info;
}



// get operators from instrumentID
QStringList US_eSignaturesGMP::read_operators( QString optima_id )
{
  QStringList instr_opers;
  
  US_Passwd pw;
  US_DB2* db = new US_DB2( pw.getPasswd() );
  
  if ( db->lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "LIMS DB Connection Problem" ),
			    tr( "Could not connect to database \n" ) + db->lastError() );

      return instr_opers;
    }

  qDebug() << "OptimaID -- " << optima_id;
  
  //Get operators for this instrID:
  QStringList qry;
  qry << "get_operator_names" << optima_id;
  db->query( qry );

  if ( db->lastErrno() == US_DB2::OK )
    {
      while ( db->next() )
	{
	  int ID    = db->value( 0 ).toString().toInt();
	  //QString GUID  = db->value( 1 ).toString();
	  QString lname = db->value( 2 ).toString();
	  QString fname = db->value( 3 ).toString();
	  
	  instr_opers << QString::number(ID) + ". " + lname + ", " + fname;
	}
    }

  return instr_opers;
}


//read eSign GMP record for assigned oper(s) && rev(s) && status
QMap< QString, QString> US_eSignaturesGMP::read_autoflowGMPReportEsign_record( QString aID)
{
  QMap< QString, QString> eSign_record;
  
  US_Passwd pw;
  US_DB2* db = new US_DB2( pw.getPasswd() );
  
  if ( db->lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "LIMS DB Connection Problem" ),
			    tr( "Could not connect to database \n" ) + db->lastError() );

      return eSign_record;
    }

  QStringList qry;
  qry << "get_gmp_review_info_by_autoflowID" << aID;
  qDebug() << "read eSing rec, qry -- " << qry;
  
  db->query( qry );

  if ( db->lastErrno() == US_DB2::OK )      // e-Sign record exists
    {
      while ( db->next() )
	{
	  eSign_record[ "ID" ]                   = db->value( 0 ).toString(); 
	  eSign_record[ "autoflowID" ]           = db->value( 1 ).toString();
	  eSign_record[ "autoflowName" ]         = db->value( 2 ).toString();
	  eSign_record[ "operatorListJson" ]     = db->value( 3 ).toString();
	  eSign_record[ "reviewersListJson" ]    = db->value( 4 ).toString();
	  eSign_record[ "eSignStatusJson" ]      = db->value( 5 ).toString();
	  eSign_record[ "eSignStatusAll" ]       = db->value( 6 ).toString();
	  eSign_record[ "createUpdateLogJson" ]  = db->value( 7 ).toString();
	  
	  eSign_record[ "isEsignRecord" ]        = QString("YES");
	  isEsignRecord = true;
	}
    }
  else
    {
      //No record, so no oper/revs assigned!
      qDebug() << "No e-Sign GMP record exists!!";

      eSign_record[ "isEsignRecord" ]        = QString("NO");
      isEsignRecord = false;
      eSign_record. clear();

      // //TEST ---------------------------
      // eSign_record[ "operatorListJson" ]  = QString( tr( "[\"Operator 1\",\"Operator 2\",\"Operator 3\"]" ));
      // eSign_record[ "reviewersListJson" ] = QString( tr( "[\"Reviewer 1\",\"Reviewer 2\",\"Reviewer 3\"]" ));
      // //END TEST ------------------------
    }

  return eSign_record;
}

//Set/Unset  pb_set_operRev -> setEnabled( false );
void US_eSignaturesGMP::setUnsetPb_operRev( void )
{
  QString e_operList = te_opers_to_assign->toPlainText();
  QString e_revList  = te_revs_to_assign->toPlainText();

  if ( e_operList.isEmpty() || e_revList.isEmpty() )
    pb_set_operRev -> setEnabled( false );
  else
    pb_set_operRev -> setEnabled( true );
}

//Add operator to list 
void US_eSignaturesGMP::addOpertoList( void )
{
  QString c_oper = cb_choose_operator->currentText();
  
  //check if selected item already in the list:
  QString e_operList = te_opers_to_assign->toPlainText();
  if ( e_operList. contains( c_oper ) )
    {
      QMessageBox::information( this, tr( "Cannot add Operator" ),
				tr( "<font color='red'><b>ATTENTION:</b> </font> Selected operator: <br><br>"
				    "<font ><b>%1</b><br><br>"
				    "is already in the list of operators.<br>"
				    "Please choose other operator.")
				.arg( c_oper ) );
      return;
    }
  
  te_opers_to_assign->append( c_oper );
  setUnsetPb_operRev();
}

//Remove operator from list 
void US_eSignaturesGMP::removeOperfromList( void )
{
  te_opers_to_assign->setFocus();
  QTextCursor storeCursorPos = te_opers_to_assign->textCursor();
  te_opers_to_assign->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
  te_opers_to_assign->moveCursor(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
  te_opers_to_assign->moveCursor(QTextCursor::End, QTextCursor::KeepAnchor);
  te_opers_to_assign->textCursor().removeSelectedText();
  te_opers_to_assign->textCursor().deletePreviousChar();
  te_opers_to_assign->setTextCursor(storeCursorPos);

  setUnsetPb_operRev();
}

//Add reviewer to list 
void US_eSignaturesGMP::addRevtoList( void )
{
  QString c_rev = cb_choose_rev->currentText();

  //check if selected item already in the list:
  QString e_revList = te_revs_to_assign->toPlainText();
  if ( e_revList. contains( c_rev ) )
    {
      QMessageBox::information( this, tr( "Cannot add Reviewer" ),
				tr( "<font color='red'><b>ATTENTION:</b> </font> Selected reviewer: <br><br>"
				    "<font ><b>%1</b><br><br>"
				    "is already in the list of reviewers.<br>"
				    "Please choose other reviewer.")
				.arg( c_rev ) );
      return;
    }
  
  te_revs_to_assign->append( c_rev );

  setUnsetPb_operRev();
}

//Remove reviewer from list 
void US_eSignaturesGMP::removeRevfromList( void )
{
  te_revs_to_assign->setFocus();
  QTextCursor storeCursorPos = te_revs_to_assign->textCursor();
  te_revs_to_assign->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
  te_revs_to_assign->moveCursor(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
  te_revs_to_assign->moveCursor(QTextCursor::End, QTextCursor::KeepAnchor);
  te_revs_to_assign->textCursor().removeSelectedText();
  te_revs_to_assign->textCursor().deletePreviousChar();
  te_revs_to_assign->setTextCursor(storeCursorPos);

  qDebug() << "Revs to ASSIGN: " << te_revs_to_assign->toPlainText();

  setUnsetPb_operRev();
}

//Assign operators & reviewers for the current GMP run:
void US_eSignaturesGMP::assignOperRevs( void )
{
  /** 
      0. FIRST, check if e-Signature process already BEGAN !!!!         <-- 1st thing to check
        -- if YES, STOP!!!!
	      ** if (isEsignRecord) BUT eSignStatusJson indicates it began
	      ** above && || autoflowStatus indicates 7. e-Signature stage in the signing mode  
	-- if NOT, PROCEED: 
	      ** if (!isEsignRecord)
	      ** if (isEsignRecord) BUT eSignStatusJson HAS NOT began: at least 1 signature put!!!
  ***/		    

  if ( is_eSignProcessBegan() )
    {
      QMessageBox::information( this, tr( "Operator(s), Reviewer(s) CANNOT be Assigned" ),
				tr( "<font color='red'><b>ATTENTION:</b> </font> Operator(s), Reviewer(s)"
				    "cannot be set/changed for the currently uploaded GMP run: <br><br>"
				    "<font ><b>%1</b><br><br>"
				    "E-Signing process has already began.")
				.arg( le_run_name->text() ) );
      return;
    }

  //save existign operators && reviewers:
  QString exsisting_oper_list = te_operator_names->toPlainText();
  QString exsisting_rev_list  = te_reviewer_names->toPlainText();

  //Set new opers && revs in the te areas
  QString oper_list = te_opers_to_assign->toPlainText();
  QString rev_list = te_revs_to_assign->toPlainText();

  //Compose JSON arrays: QString( tr( "[\"Operator 1\",\"Operator 2\",\"Operator 3\"]" ));
                                     
  QString operListJsonArray = "[";
  QString revListJsonArray  = "[";
  QStringList oper_listList = oper_list.split("\n");
  QStringList rev_listList  = rev_list.split("\n");
  QStringList oper_rev_joinedList;

  for (int i=0; i<oper_listList.size(); ++i )
    {
      oper_rev_joinedList << oper_listList[i]; 
      operListJsonArray += "\"" + oper_listList[i] + "\",";
    }
      
  for (int i=0; i<rev_listList.size(); ++i )
    {
      oper_rev_joinedList << rev_listList[i]; 
      revListJsonArray += "\"" + rev_listList[i] + "\",";
    }
  
  operListJsonArray.chop(1);
  revListJsonArray.chop(1);
  operListJsonArray += "]";
  revListJsonArray  += "]";

  qDebug() << "operListJsonArray -- " << operListJsonArray;
  qDebug() << "revListJsonArray -- "  << revListJsonArray;

  //Minimum structure of eSignStatusJson field:
  QString eSignStatusJson = "{\"to_sign\":[";
  for (int i=0; i<oper_rev_joinedList.size(); ++i )
    {
      eSignStatusJson += "\"" + oper_rev_joinedList[i] + "\",";
    }
  eSignStatusJson. chop(1);
  eSignStatusJson += "]}";
  
  qDebug() << "operRevToSignJsonObject -- "  << eSignStatusJson;

  //DB 
  US_Passwd pw;
  US_DB2* db = new US_DB2( pw.getPasswd() );
  
  if ( db->lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "LIMS DB Connection Problem" ),
			    tr( "Could not connect to database \n" ) + db->lastError() );

      return;
    }
  
  QStringList qry;
  
  
  //Minimum structure of logJson when record created from scratch:
  /** 
      { "Created by": [{ "Person": "12. Savelyev, Alexey", "timeDate": "timestamp", "Comment": "Created frist time" }],
        "Updated by": [{ ... }]  <=== later by admin, e.g. if oper(s), rev(s) are updated
      }
  **/
  QString logJsonFirstTime = "{\"Created by\":[{\"Person\":";

  qry.clear();
  qry <<  QString( "get_user_info" );
  db -> query( qry );
  db -> next();
  int u_ID        = db->value( 0 ).toInt();
  QString u_fname = db->value( 1 ).toString();
  QString u_lname = db->value( 2 ).toString();

  QDateTime date = QDateTime::currentDateTime();
  QString current_date = date.toString("MM-dd-yyyy hh:mm:ss");

  logJsonFirstTime += "\"" + QString::number(u_ID) + ". " + u_lname + ", " + u_fname +  "\",";
  logJsonFirstTime += "\"timeDate\":\"" + current_date +  "\",";
  logJsonFirstTime += "\"Comment\": \"Created first time\"";

  logJsonFirstTime += "}]}";
  qDebug() << "logJsonFirstTimeJsonObject -- "  << logJsonFirstTime;

   
  /**  NEXT, 2 scenarios:
     --- check if eSign record in Db exists;
      1. if YES, update as ADMIN 
        -- update_gmp_review_record_by_admin << p_eSignID       INT,
					     << p_autoflowID    INT,
					     << p_operListJson  TEXT,
                                             << p_revListJson   TEXT,
					     << p_logJson       TEXT
     2. if NOT, create new one: 
        --  new_gmp_review_record  <<	 p_autoflowID   INT,
				   <<	 p_autoflowName TEXT,
				   <<	 p_operListJson TEXT,
				   <<	 p_revListJson  TEXT,
				   <<    p_eSignStatusJson TEXT,
				   <<    p_logJson      TEXT )
			 
     FINALLY. Update autolfow run's gmpReviewID with p_eSignID [if 3. "NO"]
      
   **/

  /***********************************************************************/


  if ( !isEsignRecord ) //No eSignature Record exists, so create new one, with minimal status, createupdatelog
    {
      //Update te fileds
      te_operator_names -> setText( oper_list );
      te_reviewer_names -> setText( rev_list );
      
      int eSignID_returned = 0;
      
      qry. clear();
      qry << "new_gmp_review_record"
      	  << gmp_run_details[ "autoflowID" ]
      	  << gmp_run_details[ "protocolName" ]
      	  << operListJsonArray
      	  << revListJsonArray
      	  << eSignStatusJson       
      	  << logJsonFirstTime;     

      qDebug() << "new_gmp_review_record qry -- " << qry;
      db->statusQuery( qry );
      eSignID_returned = db->lastInsertID();

      if ( eSignID_returned == 0 )
      	{
      	  QMessageBox::warning( this, tr( "New eSign Record Problem" ),
      				tr( "autoflowGMPRecordEsign: There was a problem with creating a new record! \n" ) );
      	  return;
      	}

      //Update primary autolfow record with the new generated eSignID:
      qry. clear();
      qry << "update_autoflow_with_gmpReviewID"
      	  <<  gmp_run_details[ "autoflowID" ]
      	  <<  QString::number( eSignID_returned );

      qDebug() << "update_autoflow_with_gmpReviewID qry -- " << qry;
      db->query( qry );
    }
    
   else //exists eSign record: so update as ADMIN
    {
      qDebug() << "e-Signature record exists: will be updated by ADMIN!!";
      
      qDebug() << "Old Operators -- " << exsisting_oper_list;
      qDebug() << "New Operators -- " << oper_list;
      qDebug() << "Old Reviewers -- " << exsisting_rev_list;
      qDebug() << "New Reviewers -- " << rev_list;
      
      //check if at least one of tehe operator OR reviewer lists has been changed 
      if ( exsisting_oper_list == oper_list && exsisting_rev_list == rev_list )
	{
	  qDebug() << "Operators and Reviewers are the same...";
	  QMessageBox::information( this, tr( "Operator(s), Reviewer(s) NOT CHANGED" ),
				    tr( "Existing and to-be assigned "
					"Operator(s) and Reviewer(s) " 
					"lists are the same.<br><br>"
					"Nothing will be changed..."));
	  
	  return;
	}
      
      QStringList exsisting_oper_listList = exsisting_oper_list.split("\n");
      QStringList exsisting_rev_listList  = exsisting_rev_list .split("\n");

      QMessageBox msg_rev;
      msg_rev.setWindowTitle(tr("Operator(s), Reviewer(s) Change"));
      msg_rev.setText( tr( "<font color='red'><b>ATTENTION:</b> </font><br>"
			   "You are about to change assigned operator(s) and/or reviewer(s): <br><br>"
			   "Old Operators: <font ><b>%1</b><br>"
			   "New Operators: <font ><b>%2</b><br><br>"
			   "Old Reviewers: <font ><b>%3</b><br>"
			   "New Reviewers: <font ><b>%4</b><br><br>")
		       .arg( exsisting_oper_listList.join(",") )
		       .arg( oper_listList.join(",") )
		       .arg( exsisting_rev_listList.join(",") )
		       .arg( rev_listList.join(",") )
		       );
      
      msg_rev.setInformativeText( QString( tr( "Do you want to Proceed?" )));
      
      QPushButton *Accept    = msg_rev.addButton(tr("Proceed"), QMessageBox::YesRole);
      QPushButton *Cancel    = msg_rev.addButton(tr("Cancel"), QMessageBox::RejectRole);
      
      msg_rev.setIcon(QMessageBox::Question);
      msg_rev.exec();
      
      if (msg_rev.clickedButton() == Accept)
	{
	  //Update te fileds
	  te_operator_names -> setText( oper_list );
	  te_reviewer_names -> setText( rev_list );
	  
	  //Minimum structure JSON for logJsonUpdateTime:
	  QString logJsonUpdateTime = compose_updated_admin_logJson( u_ID, u_fname, u_lname );
	  qDebug() << "logJsonUpdateTimeJsonObject -- "  << logJsonUpdateTime;
	  
	  qry. clear();
	  qry << "update_gmp_review_record_by_admin"
	      << eSign_details[ "ID" ]
	      << gmp_run_details[ "autoflowID" ]
	      << operListJsonArray
	      << revListJsonArray
	      << logJsonUpdateTime;

	  qDebug() << "update_gmp_review_record_by_admin, qry -- " << qry;
	  db->query( qry );
	}
      else
	{
	  qDebug() << "Canceling oper(s)/rev(s) change...";
	  return;
	}
    }

  /**************************************************/

}

//Check if e-Signing process for the GMP run started
bool US_eSignaturesGMP::is_eSignProcessBegan( void )
{
  bool isBegan = false;
  eSign_details .clear();
  eSign_details = read_autoflowGMPReportEsign_record( gmp_run_details[ "autoflowID" ] );

  if ( !isEsignRecord )
    return isBegan;

  QString operatorListJson  = eSign_details[ "operatorListJson" ];
  QString reviewersListJson = eSign_details[ "reviewersListJson" ];
  QString eSignStatusJson   = eSign_details[ "eSignStatusJson" ];

  /****
     Proposed JSON Struncture of eSignStatusJson:
     { 
        "to_sign": ["Rev1","Rev2"],       <===== ORIGINALLY, full combined list of Oper(s) && Rev(s)
	                                         i.e. ["Oper1","Oper2",..., "Rev1","Rev2",..]
	"signed" : [
	              { "Oper1": 
	                      { 
			        "Comment"  : "Explanation",
			        "timeDate" : "timestamp"
			      }},
		      { "Oper2" :
		              {
			        Same ...
			      }}
		    ]
     }

     So, to understand if e-Signing started, investigate eSignStatusJson[ "signed" ] JSON...
   ****/
  QJsonDocument jsonDoc = QJsonDocument::fromJson( eSignStatusJson.toUtf8() );
  if (!jsonDoc.isObject())
    {
      qDebug() << "is_eSignProcessBegan(): eSignStatusJson: NOT a JSON Doc !!";
      return isBegan;
    }
  
  const QJsonValue &to_esign = jsonDoc.object().value("to_sign");
  const QJsonValue &esigned  = jsonDoc.object().value("signed");

  QJsonArray to_esign_array  = to_esign .toArray();
  QJsonArray esigned_array   = esigned  .toArray();

  //to_sign:
  if ( to_esign.isUndefined() )
    qDebug() << "All signatures have been collected; noone left to e-sign !!";

  //signed
  if ( esigned.isUndefined() || esigned_array.size() == 0 || !esigned_array.size() )
    {
      qDebug() << "Nothing has been e-Signed yet !!! Oper(s), Rev(s) CAN BE changed/assigned!!!";
      return false;
    }
  else
    {
      //DEBUG
      for (int i=0; i < esigned_array.size(); ++i )
	{
	  foreach(const QString& key, esigned_array[i].toObject().keys())
	    {
	      QJsonObject newObj = esigned_array[i].toObject().value(key).toObject();
	      
	      qDebug() << "E-Signed - " << key << ": Comment, timeDate -- "
		       << newObj["Comment"]   .toString()
		       << newObj["timeDate"]  .toString();
	    }
	}
      // END DEBUG: There is/are e-Signee(s) already; 
      isBegan = true;
    }
  return isBegan;
}


//append eSign log Json by admin upon opers/rev Update:
QString US_eSignaturesGMP::compose_updated_admin_logJson( int u_ID, QString u_fname, QString u_lname )
{
  QString e_logJson   = eSign_details[ "createUpdateLogJson" ];

  QJsonDocument jsonDoc = QJsonDocument::fromJson( e_logJson.toUtf8() );
  if (!jsonDoc.isObject())
    {
      qDebug() << "compose_updated_admin_logJson(): createUpdateLogJson: NOT a JSON Doc !!";
      return e_logJson;
    }

  //Appended portion
  QString logJsonUpdateTime = ",\"Updated by\":[{\"Person\":";

  QDateTime date = QDateTime::currentDateTime();
  QString current_date = date.toString("MM-dd-yyyy hh:mm:ss");

  logJsonUpdateTime += "\"" + QString::number(u_ID) + ". " + u_lname + ", " + u_fname +  "\",";
  logJsonUpdateTime += "\"timeDate\":\"" + current_date +  "\",";
  logJsonUpdateTime += "\"Comment\": \"Updated Operator, Reviewer lists\"";

  logJsonUpdateTime += "}]}";

  //Combined JSON
  e_logJson.chop(1); //remove trailing '}'
  QString composedJson = e_logJson + logJsonUpdateTime;

  return composedJson;
}

//Load GMP Report form Db with assigned operator(s) && reviewer(s)
void US_eSignaturesGMP::loadGMPReportDB_assigned( void )
{
  US_Passwd pw;
  US_DB2 db( pw.getPasswd() );
  
  if ( db.lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "LIMS DB Connection Problem" ),
			    tr( "Could not connect to database \n" ) + db.lastError() );
      return;
    }
  
  list_all_gmp_reports_db( gmpReportsDBdata, &db );

  QString pdtitle( tr( "Select GMP Report" ) );
  QStringList hdrs;
  int         prx;
  
  hdrs << "ID"
       << "Run Name"
    //<< "Protocol Name"
       << "Created"
       << "Filename (.pdf)"
       << "GMP Run ID";
         
  QString autoflow_btn = "AUTOFLOW_GMP_REPORT";

  pdiag_autoflow_db = new US_SelectItem( gmpReportsDBdata, hdrs, pdtitle, &prx, autoflow_btn, -2 );

  QString gmpReport_id_selected("");
  QString gmpReport_runname_selected("");
  QString gmpReport_runname_selected_c("");
  QString gmpReport_filename_pdf ("");
        
  if ( pdiag_autoflow_db->exec() == QDialog::Accepted )
    {
      gmpReport_id_selected        = gmpReportsDBdata[ prx ][ 0 ];
      gmpReport_runname_selected_c = gmpReportsDBdata[ prx ][ 1 ];
      gmpReport_filename_pdf       = gmpReportsDBdata[ prx ][ 3 ];
      gmpRunID_eSign               = gmpReportsDBdata[ prx ][ 4 ];

      pb_view_report_db -> setEnabled( false );
      pb_view_eSigns    -> setEnabled( false );
      te_fpath_info     -> setText( "" );
      le_loaded_run_db  -> setText( "" );
      le_eSign_status   -> setText( "" );
    }
  else
    return;

  
  //read 'data' .tar.gz for autoflowGMPReport record:
  if ( gmpReport_runname_selected_c.  contains("combined") )
    {
      gmpReport_runname_selected = gmpReport_runname_selected_c.split("(")[0];
      gmpReport_runname_selected. simplified();
    }
  else
    gmpReport_runname_selected = gmpReport_runname_selected_c;
  
  QString subDirName = gmpReport_runname_selected + "_GMP_DB_Esign";
  mkdir( US_Settings::reportDir(), subDirName );
  QString dirName     = US_Settings::reportDir() + "/" + subDirName;

  //Clean folder (if exists) where .tar.gz to be unpacked
  QStringList f_exts = QStringList() <<  "*.*";
  QString i_folder = dirName + "/" + gmpReport_runname_selected;
  remove_files_by_mask( i_folder, f_exts );

  QString GMPReportfname = "GMP_Report_from_DB.tar";
  QString GMPReportfpath = dirName + "/" + GMPReportfname;
  
  int db_read = db.readBlobFromDB( GMPReportfpath,
				   "download_gmpReportData",
				   gmpReport_id_selected.toInt() );

  if ( db_read == US_DB2::DBERROR )
    {
      QMessageBox::warning(this, "Error", "Error processing file:\n"
			   + GMPReportfpath + "\n" + db.lastError() +
			   "\n" + "Could not open file or no data \n");

      return;
    }
  else if ( db_read != US_DB2::OK )
    {
      QMessageBox::warning(this, "Error", "returned processing file:\n" +
			   GMPReportfpath + "\n" + db.lastError() + "\n");

      return;
    }
  
  // <--- TESTING: tried .tar.gz - NOT compatible (even for different Linux distros...) ****
  // //Un-tar using system TAR && enable View Report btn:
  // QProcess *process = new QProcess(this);
  // process->setWorkingDirectory( dirName );
  // process->start("tar", QStringList() << "-zxvf" << GMPReportfname );
  // END TESTING ****************************************************************************
  
  // // Using .tar (NOT gzip: .tgz or tar.gz !!!)
  QProcess *process = new QProcess(this);
  process->setWorkingDirectory( dirName );
  process->start("tar", QStringList() << "-xvf" << GMPReportfname );
    
  filePath_db = dirName + "/" + gmpReport_runname_selected + "/" + gmpReport_filename_pdf;
  qDebug() << "Extracted .PDF GMP Report filepath -- " << filePath_db;

  //Gui fields
  le_loaded_run_db  -> setText( gmpReport_runname_selected_c );
  pb_view_report_db -> setEnabled( true );
  pb_esign_report   -> setEnabled( true );
  te_fpath_info     -> setText( filePath_db );

  //Check, the status of e-Signing: NOT signed, Partially, Completed
  QString eSign_status = check_eSign_status_for_gmpReport();

  if ( eSign_status == "PARTIALLY COMPLETED" || eSign_status == "COMPLETED" )
    write_download_eSignatures_DB( filePath_db, "download_gmpReportEsignData" );
  

  //Inform user of the PDF location
  QMessageBox msgBox;
  msgBox.setText(tr("Report PDF Ready!"));
  msgBox.setInformativeText(tr( "Report was downloaded form DB in .PDF format and saved at: \n%1\n\n"
				"When this dialog is closed, the report can be re-opened by clicking \'View Downloaded Report\' button on the left.")
			    .arg( filePath_db ) );
  
  msgBox.setWindowTitle(tr("GMP Report Downloaded"));
  QPushButton *Open      = msgBox.addButton(tr("View Report"), QMessageBox::YesRole);
  //QPushButton *Cancel  = msgBox.addButton(tr("Ignore Data"), QMessageBox::RejectRole);
  
  msgBox.setIcon(QMessageBox::Information);
  msgBox.exec();
  
  if (msgBox.clickedButton() == Open)
    {
      view_report_db();
    }  

  qDebug() << "After download GMP Report, gmpRunID_eSign -- " << gmpRunID_eSign;
}


// Get .pdf GMP reports with assigned reviewers:
int US_eSignaturesGMP::list_all_gmp_reports_db( QList< QStringList >& gmpReportsDBdata, US_DB2* db)
{
  int nrecs        = 0;   
  gmpReportsDBdata.clear();

  QStringList qry;
  qry << "get_autoflowGMPReport_desc";
  db->query( qry );

  while ( db->next() )
    {
      QStringList gmpreportentry;
      QString id                     = db->value( 0 ).toString();
      QString autoflowHistoryID      = db->value( 1 ).toString();
      QString autoflowHistoryName    = db->value( 2 ).toString();
      QString protocolName           = db->value( 3 ).toString();
      QDateTime time_created         = db->value( 4 ).toDateTime().toUTC();
      QString filenamePdf            = db->value( 5 ).toString();

      //check if report has assigned operator(s) & reviewer(s)
      QMap< QString, QString > eSign = read_autoflowGMPReportEsign_record( autoflowHistoryID );
      QString operatorListJson  = eSign[ "operatorListJson" ];
      QString reviewersListJson = eSign[ "reviewersListJson" ];
      QString eSignStatusJson   = eSign[ "eSignStatusJson" ];

      qDebug() << "In listing GMP Reports with assigned reviewers -- ";
      qDebug() << "operatorListJson, reviewersListJson -- "
	       << operatorListJson << ",   "
	       << reviewersListJson;

      QJsonDocument jsonDocRevList  = QJsonDocument::fromJson( reviewersListJson.toUtf8() );
      QJsonDocument jsonDocOperList = QJsonDocument::fromJson( operatorListJson .toUtf8() );
  
      if ( jsonDocRevList. isArray() && jsonDocOperList. isArray()
	   && !operatorListJson.isEmpty() && !reviewersListJson.isEmpty() )
      	{
	  gmpreportentry << id << autoflowHistoryName // << protocolName
			 << time_created.toString()
			 << filenamePdf
			 << autoflowHistoryID;
	  gmpReportsDBdata << gmpreportentry;
	  nrecs++;
	}
    }

  return nrecs;
}


//Check eSign status for GMP Report
QString US_eSignaturesGMP::check_eSign_status_for_gmpReport()
{
  QMap< QString, QString > eSign = read_autoflowGMPReportEsign_record( gmpRunID_eSign );
  QString eSignStatusJson   = eSign[ "eSignStatusJson" ];
  QString eSignStatusAll    = eSign[ "eSignStatusAll" ];
  eSignID_global            = eSign[ "ID" ];

  QJsonDocument jsonDocEsign = QJsonDocument::fromJson( eSignStatusJson.toUtf8() );
  if (!jsonDocEsign.isObject())
    {
      qDebug() << "to_eSign(): ERROR: eSignStatusJson: NOT a JSON Doc !!";
      return QString("");
    }
  
  const QJsonValue &to_esign = jsonDocEsign.object().value("to_sign");
  const QJsonValue &esigned  = jsonDocEsign.object().value("signed");

  QJsonArray to_esign_array  = to_esign .toArray();
  QJsonArray esigned_array   = esigned  .toArray();

  //Palette of eStatus:
  QPalette orig_pal = le_eSign_status->palette();
  QPalette *new_palette = new QPalette();
  new_palette->setColor(QPalette::Base, orig_pal.color(QPalette::Base));
  

  //to_sign:
  if ( to_esign.isUndefined() || to_esign_array.size() == 0
       || !to_esign_array.size() || eSignStatusAll == "YES" )
    {
      qDebug() << "check_eSign_status(): All signatures have been collected; none left to e-sign !!";
      
      le_eSign_status -> setText( "COMPLETED" );
      new_palette->setColor(QPalette::Text,Qt::green);
      le_eSign_status->setPalette(*new_palette);
      pb_view_eSigns   -> setEnabled( true );

      return QString("COMPLETED");
    }

  //signed:
  if ( esigned.isUndefined() || esigned_array.size() == 0 || !esigned_array.size() )
    {
      qDebug() << "check_eSign_Status(): Nothing has been e-Signed yet !!!";

      le_eSign_status -> setText( "NOT STARTED" );
      new_palette->setColor(QPalette::Text,Qt::red);
      le_eSign_status->setPalette(*new_palette);
      pb_view_eSigns   -> setEnabled( true );
      
      return QString("NOT STARTED");
    }
  else
    {
      qDebug() << "check_eSign_status(): Some parties have e-Signed already !!!";
      //DEBUG
      QStringList eSignees_current;
      for (int i=0; i < esigned_array.size(); ++i )
	{
	  foreach(const QString& key, esigned_array[i].toObject().keys())
	    {
	      QJsonObject newObj = esigned_array[i].toObject().value(key).toObject();
	      
	      qDebug() << "E-Signed - " << key << ": Comment, timeDate -- "
		       << newObj["Comment"]   .toString()
		       << newObj["timeDate"]  .toString();

	      QString current_reviewer = key;
	      QString current_reviewer_id = current_reviewer. section( ".", 0, 0 );

	      eSignees_current << key;
	    }
	}
      //END DEBUG:
      qDebug() << "check_eSign_status(): so far, e-signed by: " << eSignees_current;

      le_eSign_status -> setText( "PARTIALLY COMPLETED" );
      new_palette->setColor(QPalette::Text,Qt::blue);
      le_eSign_status->setPalette(*new_palette);
      pb_view_eSigns   -> setEnabled( true );
      
      return QString("PARTIALLY COMPLETED");
    }
}



//remove files by extension from dirPath
void US_eSignaturesGMP::remove_files_by_mask( QString dirPath, QStringList file_exts )
{
  QDir dir( dirPath );
  dir.setNameFilters(file_exts);
  dir.setFilter(QDir::Files);
  foreach( QString dirFile, dir.entryList() ) 
    {
      dir.remove(dirFile);
    }
}

//view report DB
void US_eSignaturesGMP::view_report_db ( )
{
  qDebug() << "Opening PDF (for downloaded from DB) at -- " << filePath_db;

  QFileInfo check_file( filePath_db );
  if (check_file.exists() && check_file.isFile())
    {
      //Open with OS's applicaiton settings ?
      QDesktopServices::openUrl(QUrl( filePath_db ));
    }
  else
    {
      QMessageBox::warning( this, tr( "Error: Cannot Open .PDF File" ),
			    tr( "%1 \n\n"
				"No such file or directory...") .arg( filePath_db ) );
    }
}

//view current e-Signatures
void US_eSignaturesGMP::view_eSignatures ( )
{
  QString fpath = filePath_db;
  fpath. replace( filePath_db.section('/', -1), "" );
  QString filePath_to_eSigns = fpath + "eSignatures.pdf";

  qDebug() << "Opening PDF for e-Signatures (either downloaded OR generated) at -- " << filePath_to_eSigns;
  
  QFileInfo check_file( filePath_to_eSigns );
  if (check_file.exists() && check_file.isFile())
    {
      //Open with OS's applicaiton settings ?
      QDesktopServices::openUrl(QUrl( filePath_to_eSigns ));
    }
  else
    {
      QMessageBox::warning( this, tr( "Error: Cannot Open .PDF File" ),
			    tr( "%1 \n\n"
				"No such file or directory...") .arg( filePath_to_eSigns ) );
    }
}

// Create a subdirectory if need be
bool US_eSignaturesGMP::mkdir( const QString& baseDir, const QString& subdir )
{
   QDir folder( baseDir );

   if ( folder.exists( subdir ) ) return true;

   if ( folder.mkdir( subdir ) ) return true;

   QMessageBox::warning( this,
      tr( "File error" ),
      tr( "Could not create the directory:\n" ) + baseDir + "/" + subdir );

   return false;
}


//eSign GMP Report
void US_eSignaturesGMP::esign_report( void )
{
  //first, check if you (as logged in user) are among listed operators && reviewers
  US_Passwd pw;
  US_DB2 db( pw.getPasswd() );
  
  if ( db.lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "LIMS DB Connection Problem" ),
			    tr( "Could not connect to database \n" ) + db.lastError() );
      return;
    }

  
  QStringList qry;
  //Check user level && ID
  qry <<  QString( "get_user_info" );
  db.query( qry );
  db.next();
  int u_ID        = db.value( 0 ).toInt();
  QString u_fname = db.value( 1 ).toString();
  QString u_lname = db.value( 2 ).toString();
          
  qDebug() << "eSign: user_id, gmpRunID_eSign -- "
	   << u_ID << gmpRunID_eSign;
  
  QMap< QString, QString > eSign = read_autoflowGMPReportEsign_record( gmpRunID_eSign );
  QString operatorListJson  = eSign[ "operatorListJson" ];
  QString reviewersListJson = eSign[ "reviewersListJson" ];
  QString eSignStatusJson   = eSign[ "eSignStatusJson" ];
  QString eSignStatusAll    = eSign[ "eSignStatusAll" ];
  QString eSignID           = eSign[ "ID" ];

  QJsonDocument jsonDocOperList = QJsonDocument::fromJson( operatorListJson .toUtf8() );
  QJsonDocument jsonDocRevList  = QJsonDocument::fromJson( reviewersListJson.toUtf8() );
  
  qDebug() << "eSign: operatorListJson, reviewersListJson,  eSignStatusAll -- "
	   << operatorListJson << reviewersListJson << eSignStatusAll;

  QJsonDocument jsonDocEsign = QJsonDocument::fromJson( eSignStatusJson.toUtf8() );
  if (!jsonDocEsign.isObject())
    {
      qDebug() << "to_eSign(): ERROR: eSignStatusJson: NOT a JSON Doc !!";
      return;
    }
  
  const QJsonValue &to_esign = jsonDocEsign.object().value("to_sign");
  const QJsonValue &esigned  = jsonDocEsign.object().value("signed");

  QJsonArray to_esign_array  = to_esign .toArray();
  QJsonArray esigned_array   = esigned  .toArray();

  //to_sign:
  if ( to_esign.isUndefined() || to_esign_array.size() == 0
       || !to_esign_array.size() || eSignStatusAll == "YES" )
    {
      qDebug() << "to_eSign(): All signatures have been collected; noone left to e-sign !!";

      QMessageBox::information( this, tr( "GMP Report e-Signed" ),
				tr( "<font color='red'><b>ATTENTION:</b> </font>" 
				    "All parties e-Signed current GMP Report!" ));
				       
      return;
    }

  //Check if the person among reviewers, OR if the person already e-signed:
  //to_sign section:
  bool yesToSign = false;
  for (int i=0; i < to_esign_array.size(); ++i )
    {
      QString current_reviewer = to_esign_array[i].toString();
      QString current_reviewer_id = current_reviewer. section( ".", 0, 0 );

      if ( u_ID == current_reviewer_id.toInt() )
	{
	  yesToSign = true;
	  break;
	}
    }

  //signed section:
  bool yesWasSigned = false;
  if ( esigned.isUndefined() || esigned_array.size() == 0 || !esigned_array.size() )
      qDebug() << "to_eSign(): Nothing has been e-Signed yet !!!";
  else
    {
      qDebug() << "to_eSign(): Some parties have e-Signed already !!!";
      for (int i=0; i < esigned_array.size(); ++i )
	{
	  foreach(const QString& key, esigned_array[i].toObject().keys())
	    {
	      QJsonObject newObj = esigned_array[i].toObject().value(key).toObject();
	      
	      qDebug() << "E-Signed - " << key << ": Comment, timeDate -- "
		       << newObj["Comment"]   .toString()
		       << newObj["timeDate"]  .toString();

	      QString current_reviewer = key;
	      QString current_reviewer_id = current_reviewer. section( ".", 0, 0 );

	      if ( u_ID == current_reviewer_id.toInt() )
		{
		  qDebug() << "Current user, " << key << ", has already e-Signed!";
		  yesWasSigned = true;
		  break;
		}
	    }
	}
    }
  //END of verifying user in toSign, has already e-Signed  ///////////

  QString msg_current_user;
  if ( !yesToSign )
    {
      if ( !yesWasSigned )
	{
	  //inform user he is NOT either operator || assigned reviewer
	  msg_current_user = tr("<font color='red'><b>ATTENTION:</b> </font>" 
				"You cannot e-Sign!<br><br>"
				"You are not among assigned operator(s), or reviewer(s)<br>"
				"for downloaded GMP Report.");
	}
      else
	{
	  msg_current_user = tr("<font color='red'><b>ATTENTION:</b> </font>" 
				"You cannot e-Sign!<br><br>"
				"You have already e-Signed current GMP Report.");
	}
      
      QMessageBox::information( this, tr( "Not Eligible for e-Signing" ),
				msg_current_user );
      
      return;
    }

  //QDialog for an e-Signee's Comment:
  QString user_esignee = u_lname + ", " + u_fname;
  bool ok;
  QString msg = QString(tr("e-Sign: comment by <b>%1</b>").arg( user_esignee ) );
  QString default_text = QString(tr("e-Sign Comment: "));
  QString comment_t    = QInputDialog::getText( this,
						tr( "e-Sign Comment" ),
						msg, QLineEdit::Normal, default_text, &ok );
  
  if ( !ok )
    {
      qDebug() << "e-Signee refused to comment...";
      return;
    }

  //OK, eSign downloaded GMP Report /////////////////////////////////////////////////////////////////////////
  //BEFORE writing, check if writing has been initiated, or completed from different session:
  int status_esign_unique = 0;
  qry. clear();
  qry << "autoflow_esigning_status"
      << gmpRunID_eSign;

  qDebug() << "Checking eSining autoflowStages -- " << qry;
  status_esign_unique = db.statusQuery( qry );

  qDebug() << "status_esign_unique -- " << status_esign_unique ;
  
  if ( !status_esign_unique )
    {
      QMessageBox::information( this,
				tr( "The e-Signature record can NOT be updated" ),
				tr( "The e-Signature record is currenlty being updated" 
				    "by other reviewer(s) and/or operator(s).\n\n"
				    "Please try to e-Sign again later.") );
      return;
    }
  ///////////////////////////////////////////////////////////////////////////////////////////

  //Compose/Update eSignStatusJson && eSignStatusAll:
  QString eSignStatusAll_updated;
  QString eSignStatusJson_updated = compose_updated_eSign_Json( u_ID, u_fname, u_lname, to_esign_array,
								esigned_array, comment_t, eSignStatusAll_updated );
  
  qry.clear();
  qry << "update_gmp_review_record_by_esigner"
      << eSignID
      << gmpRunID_eSign
      << eSignStatusJson_updated
      << eSignStatusAll_updated;

  qDebug() << "e-Sign(): qry -- " << qry;

  db.query( qry );

  //generate .PDF with e-Signatires page && write to Db
  write_pdf_eSignatures( filePath_db, eSignStatusJson_updated, operatorListJson, reviewersListJson );

  //Revert 'esigning' stage status back to DEFAULT for other reviewers to be able to e-sign:
  qry. clear();
  qry << "autoflow_esigning_status_revert"
      << gmpRunID_eSign;
  db.query( qry );
  
  //merge original && e-Signatures page??:
  /*** 
       do we need to do this at all?
       do we need to do this ONLY when ALL eSignatures collected??
       OR
       do we store eSignatures .PDF separately, in autoflowGMPReportEsign table??
   ***/

  //update le_esing_status:
  QString eSign_status = check_eSign_status_for_gmpReport();
  qDebug() << "Current eSigning Status -- " << eSign_status;
  
  //concluding msg:
  QString msg_f = QString( tr("<font color='red'><b>SUCCESS:</b> </font><br><br>"
			      "Operator | Reviewer, <br>"
			      "<b>%1,</b><br>"
			      "successfully e-Signed GMP report.")
			   .arg( user_esignee ) );
			
  if ( eSignStatusAll_updated == "YES" )
    msg_f += tr("<br><br>All parties e-signed the report!");
  
  QMessageBox::information( this, tr( "Successful e-Signing" ),
			    msg_f );
}


//Compose /Update eSignStatusJson:
QString US_eSignaturesGMP::compose_updated_eSign_Json( int u_ID, QString u_fname, QString u_lname,
						       QJsonArray to_esign_array,  QJsonArray esigned_array,
						       QString comment_esignee, QString& eSignStatusAll_updated )
{
  QString statusJson = "{";

  // "to_sign" section:
  QString to_sign_str = "\"to_sign\":[";
  QString current_esignee;
  int esignees_left = 0;
    
  for (int i=0; i < to_esign_array.size(); ++i )
    {
      QString current_reviewer = to_esign_array[i].toString();
      QString current_reviewer_id = current_reviewer. section( ".", 0, 0 );

      qDebug() << "Curr reviewer: " << current_reviewer;

      if ( u_ID == current_reviewer_id.toInt() )
	{
	  current_esignee = "{\"" + current_reviewer  + "\":";
	  continue;
	}

      ++esignees_left;
      to_sign_str += "\"" + current_reviewer + "\",";
    }
  
  
  //update global e-Sign status:
  if ( !esignees_left )
    {
      eSignStatusAll_updated = "YES";
    }
  else
    {
      eSignStatusAll_updated = "NO";
      to_sign_str.chop(1);
    }
  to_sign_str += "],";
  
  
  // "esigned" section":
  QString esigned_str = "\"signed\":[";

  //Comment for current e-Signee:
  current_esignee += "{\"Comment\":\"" + comment_esignee + "\",";

  //TimeDate fro current e-signee:
  QDateTime date = QDateTime::currentDateTime();
  QString timedate_esignee = date.toString("MM-dd-yyyy hh:mm:ss");
  current_esignee += "\"timeDate\":\"" + timedate_esignee + "\"}}";
  
  if (  esigned_array.size() == 0 || !esigned_array.size() )
    {
      //1st time e-Signing:
      esigned_str += current_esignee;
    }
  else
    {
      QString existing_esignees;
      for (int i=0; i < esigned_array.size(); ++i )
	{
	  foreach(const QString& key, esigned_array[i].toObject().keys())
	    {
	      QJsonObject newObj = esigned_array[i].toObject().value(key).toObject();
	      
	      qDebug() << "Updating E-Signed section - " << key << ": Comment, timeDate -- "
		       << newObj["Comment"]   .toString()
		       << newObj["timeDate"]  .toString();

	      
	      existing_esignees += "{\"" + key + "\":{\"Comment\":\"" + newObj["Comment"]  .toString() + "\",";
	      existing_esignees +=                  "\"timeDate\":\"" + newObj["timeDate"] .toString() + "\"}},";
	    }
	}
      esigned_str += existing_esignees;
      esigned_str += current_esignee;
    }
  esigned_str += "]";

  //Compose final statusJson:
  statusJson += to_sign_str;
  statusJson += esigned_str;
  statusJson += "}";
  
  return statusJson;
}


//write .PDF eSignatures page
void US_eSignaturesGMP::write_pdf_eSignatures( QString filePath, QString eSignStatusJson_upd,
					       QString operList, QString revList )
{
  QString fpath = filePath;
  fpath. replace( filePath.section('/', -1), "" );
  qDebug() << "Writing .PDF: basename -- " << fpath;

  //html string
  QString html_assembled = tr( "<h2 align=left>Electronic Signatures:</h2>" );

  //Parse JSON
  QJsonDocument jsonDocEsign = QJsonDocument::fromJson( eSignStatusJson_upd.toUtf8() );
  if (!jsonDocEsign.isObject())
    {
      qDebug() << "write_pdf_eSigns(): ERROR: eSignStatusJson_upd: NOT a JSON Doc !!";
      return;
    }
  
  const QJsonValue &to_esign = jsonDocEsign.object().value("to_sign");
  const QJsonValue &esigned  = jsonDocEsign.object().value("signed");

  QJsonArray to_esign_array  = to_esign .toArray();
  QJsonArray esigned_array   = esigned  .toArray();

  int eSigner_counter = 0;
  for (int i=0; i < esigned_array.size(); ++i )
    {
      foreach(const QString& key, esigned_array[i].toObject().keys())
	{
	  QJsonObject newObj = esigned_array[i].toObject().value(key).toObject();
	  ++eSigner_counter;

	  //role
	  QString eSigner_role;
	  if ( operList. contains( key ) )
	    eSigner_role = "Operator";
	  else if ( revList. contains( key ) )
	    eSigner_role = "Reviewer"; 
	  
	  qDebug() << "write_pdf_eSign() -- " << key
		   << ": Comment, timeDate, eSigner_role -- "
		   << newObj["Comment"]   .toString()
		   << newObj["timeDate"]  .toString()
		   << eSigner_role;

	  QString eSigner = (key. split(".")[1]).trimmed();
	  //eSigner
	  html_assembled += tr( "<h3 style=\"margin-left:10px\">%1. %2</h3>" )
	    .arg( QString::number( eSigner_counter ) )
	    .arg( eSigner );
	  
	  //details
	  html_assembled += tr(
			       "<table style=\"margin-left:30px\">"
			       "<tr><td>Role:        </td> <td> %1 </td></tr>"
			       "<tr><td>Comment:     </td> <td> %2 </td></tr>"
			       "<tr><td>e-Signed at: </td> <td> %3 </td></tr>"
			       "</table>"
			       )
	    .arg( eSigner_role )                                      //1
	    .arg( newObj["Comment"]   .toString() )                   //2
	    .arg( newObj["timeDate"]  .toString() )                   //3
	    ;
	}
    }
  
  
  //write .PDF   
  QTextDocument document;
  document.setHtml( html_assembled );
  
  QPrinter printer(QPrinter::PrinterResolution);
  printer.setOutputFormat(QPrinter::PdfFormat);
  printer.setPaperSize(QPrinter::Letter);
  
  filePath_eSign  = fpath + "eSignatures.pdf";
  
  printer.setOutputFileName( filePath_eSign );
  printer.setFullPage(true);
  printer.setPageMargins(0, 0, 0, 0, QPrinter::Millimeter);
  
  document.print(&printer);
  qApp->processEvents();

  //Write to autoflowGMPReportEsign table as blob
  write_download_eSignatures_DB( filePath_db, "upload_gmpReportEsignData" );
  qApp->processEvents();
  
}

//write blob of eSignatures page
void US_eSignaturesGMP::write_download_eSignatures_DB( QString filePath, QString proc_name )
{
  QString fpath = filePath;
  fpath. replace( filePath.section('/', -1), "" );
  qDebug() << "Writing basename -- " << fpath;

  QString final_fpath = fpath + "eSignatures.pdf";
   
  qDebug() << "write_eSignatures_DB():: Writing Blob of final_fpath, eSignID -- "
	   << final_fpath << eSignID_global.toInt();
  
  US_Passwd pw;
  US_DB2    db( pw.getPasswd() );
  
  if ( db.lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Connection Problem" ),
			    tr( "Could not connect to database \n" ) +  db.lastError() );
      return;
    }

  QStringList qry;
  
  int writeStatus;

  if ( proc_name. contains("upload") )
    writeStatus = db.writeBlobToDB( final_fpath,
				    proc_name,
				    eSignID_global.toInt() );
  if ( proc_name. contains("download") )
    writeStatus = db.readBlobFromDB( final_fpath,
				     proc_name,
				     eSignID_global.toInt() );
  
  
  if ( writeStatus == US_DB2::DBERROR )
    {
      QMessageBox::warning(this, "Error", "Error processing file:\n"
			   + filePath_eSign + "\n" + db.lastError() +
			   "\n" + "Could not open file or no data \n");
    }
  
  else if ( writeStatus != US_DB2::OK )
    {
      QMessageBox::warning(this, "Error", "returned processing file:\n" +
			   filePath_eSign + "\n" + db.lastError() + "\n");
    }
}
