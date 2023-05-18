#include <QPrinter>
#include <QPdfWriter>
#include <QPainter>

#include "us_esigner_gmp.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "us_protocol_util.h"
#include "us_math2.h"
#include "us_constants.h"
#include "us_solution_vals.h"
#include "us_tar.h"

#define MIN_NTC   25

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
                                    US_GuiSettings::fontSize() ) );
  us_setReadOnly( te_inv_smry, true );

  lb_grev_search     = us_label( tr( "Global Reviewer Search:" ) );
  le_grev_search     = us_lineedit();
  lw_grev_list       = us_listwidget();
  lw_grev_list       ->setSelectionMode( QAbstractItemView::ExtendedSelection );
  
  QLabel* lb_grev_smry     = us_label( tr( "Reviewer Information:" ), 1 );
  te_grev_smry      = us_textedit();
  te_grev_smry      ->setTextColor( Qt::blue );
  te_grev_smry      ->setFont( QFont( US_Widgets::fixedFont().family(),
                                    US_GuiSettings::fontSize() ) );
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
 

  //for setting oper, revs. for selected GMP Run
  QGridLayout*  revOperGMPRunGrid  = new QGridLayout();
  revOperGMPRunGrid->setSpacing     ( 2 );
  revOperGMPRunGrid->setContentsMargins( 1, 1, 1, 1 );

  QLabel* bn_revOperGMP     = us_banner( tr( "Assign Operator and Reviewer(s) for GMP Run:" ), 1 );
  bn_revOperGMP -> setFixedHeight  (1.5 * RowHeight);

  pb_selRun_operRev_set = us_pushbutton( tr( "Select GMP Run" ) );
  pb_set_operRev        = us_pushbutton( tr( "Change/Set Operators and Reviewers" ) );

  QLabel* lb_run_name       = us_label( "Run Name:" );
  QLabel* lb_optima_name    = us_label( "Optima:" );
  QLabel* lb_operator_names = us_label( "Assigned Operator:" );
  QLabel* lb_reviewer_names = us_label( "Assigned Reviewers:" );
  
  QLabel* lb_choose_oper      = us_label( "Choose Operator:" );
  QLabel* lb_choose_rev1      = us_label( "Choose Reviewer 1:" );
  QLabel* lb_choose_rev2      = us_label( "Choose Reviewer 2:" );

  le_run_name       = us_lineedit( tr(""), 0, true );
  le_optima_name    = us_lineedit( tr(""), 0, true );
  le_operator_names = us_lineedit( tr(""), 0, true );
  le_reviewer_names = us_lineedit( tr(""), 0, true );

  cb_choose_operator   = new QComboBox( this );
  cb_choose_rev1       = new QComboBox( this );
  cb_choose_rev2       = new QComboBox( this ); 

  
  row = 0;
  revOperGMPRunGrid -> addWidget( bn_revOperGMP,          row++,    0, 1,  14 );
  
  revOperGMPRunGrid -> addWidget( pb_selRun_operRev_set,  row++,    0, 1,  6 );
  revOperGMPRunGrid -> addWidget( lb_run_name,            row,      0, 1,  3 );
  revOperGMPRunGrid -> addWidget( le_run_name,            row,      3, 1,  4 );

  revOperGMPRunGrid -> addWidget( lb_choose_oper,         row,      8,  1,  3 );
  revOperGMPRunGrid -> addWidget( cb_choose_operator,     row++,    11, 1,  3 );

  revOperGMPRunGrid -> addWidget( lb_optima_name,         row,      0, 1,  3 );
  revOperGMPRunGrid -> addWidget( le_optima_name,         row,      3, 1,  4 );

  revOperGMPRunGrid -> addWidget( lb_choose_rev1,         row,      8,  1,  3 );
  revOperGMPRunGrid -> addWidget( cb_choose_rev1,         row++,    11, 1,  3 );

  revOperGMPRunGrid -> addWidget( lb_operator_names,      row,      0, 1,  3 );
  revOperGMPRunGrid -> addWidget( le_operator_names,      row,      3, 1,  4 );

  revOperGMPRunGrid -> addWidget( lb_choose_rev2,         row,      8,  1,  3 );
  revOperGMPRunGrid -> addWidget( cb_choose_rev2,         row++,    11, 1,  3 );

  revOperGMPRunGrid -> addWidget( lb_reviewer_names,      row,      0, 1,  3 );
  revOperGMPRunGrid -> addWidget( le_reviewer_names,      row,      3, 1,  4 );

  revOperGMPRunGrid -> addWidget( pb_set_operRev,         row++,    8, 1,  6 );
    
  //for eSigning selected GMP Run
  QGridLayout* eSignGMPRunGrid     = new QGridLayout();
  eSignGMPRunGrid->setSpacing        ( 2 );
  eSignGMPRunGrid->setContentsMargins( 1, 1, 1, 1 );

  QLabel* bn_eSignGMP     = us_banner( tr( "Manage e-Signatures for GMP Run:" ), 1 );
  bn_eSignGMP -> setFixedHeight  (1.5 * RowHeight);

  pb_loadreport_db          =  us_pushbutton( tr( "Load GMP Report from DB (.PDF)" ) );
  pb_view_report_db         =  us_pushbutton( tr( "Review Downloaded Report" ) );
  pb_esign_report           =  us_pushbutton( tr( "e-Sign Report" ) );

  pb_view_report_db  -> setEnabled( false );
  pb_esign_report    -> setEnabled( false );

  pb_esign_report -> setStyleSheet( "QPushButton::enabled {background: #556B2F; color: lightgray; } QPushButton::disabled {background: #90EE90; color: black}");

  QLabel* lb_loaded_run_db  = us_label( tr( "Loaded GMP Report for Run:" ) );
  le_loaded_run_db          = us_lineedit( tr(""), 0, true );
  
  //Filename path
  QLabel*      lb_fpath_info = us_label( tr( "Report File \nLocation:" ) );
  te_fpath_info =  us_textedit();
  te_fpath_info -> setFixedHeight  ( RowHeight * 3 );
  te_fpath_info -> setText( tr( "" ) );
  us_setReadOnly( te_fpath_info, true );
  
  row = 0;
  eSignGMPRunGrid -> addWidget( bn_eSignGMP,        row++,    0, 1,  14 );

  eSignGMPRunGrid -> addWidget( pb_loadreport_db,   row++,      0, 1,  6);
  
  eSignGMPRunGrid -> addWidget( lb_loaded_run_db,   row,      0, 1,  3);
  eSignGMPRunGrid -> addWidget( le_loaded_run_db,   row,      3, 1,  4);

  eSignGMPRunGrid -> addWidget( pb_view_report_db , row++,    8,  1,  4 );

  eSignGMPRunGrid -> addWidget( lb_fpath_info,      row,      0, 1,  3);
  eSignGMPRunGrid -> addWidget( te_fpath_info,      row,      3, 1,  4);

  eSignGMPRunGrid -> addWidget( pb_esign_report,    row++,    8,  1,  4 );
  

  //Setting top-level Layouts:
  mainLayout -> addLayout( revGlobalGrid );
  mainLayout -> addLayout( revOperGMPRunGrid );
  mainLayout -> addLayout( eSignGMPRunGrid );

  mainLayout -> addStretch();

  
			 
  resize( 1000, 700 );
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
}
