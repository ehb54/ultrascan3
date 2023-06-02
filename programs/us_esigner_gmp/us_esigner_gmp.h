#ifndef US_ESIGNER_GMP_H
#define US_ESIGNER_GMP_H

#include "us_widgets.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "us_run_protocol.h"
#include "us_protocol_util.h"
#include "us_solution.h"
#include "us_help.h"
#include "us_extern.h"
#include "us_select_item.h"

class US_eSignaturesGMP : public US_Widgets
{
   Q_OBJECT

      public:
         US_eSignaturesGMP();
	 US_eSignaturesGMP( QString );
         US_eSignaturesGMP( QMap <QString, QString>& );

         bool auto_mode;

         class US_InvestigatorData
	 {
           public:
	   int     invID;        //!< The uniqe ID in the DB for the entry.
	   int     ulev;         //!< User level
	   int     gmpReviewer;  //!< Is user set a reviewer (0 | 1 )
	   QString invGuid;      //!< Investigator's global ID
	   QString lastName;     //!< Last Name
	   QString firstName;    //!< First Name   
	   QString address;      //!< Investigator's address
	   QString city;         //!< Investigator's city
	   QString state;        //!< Investigator's state
	   QString zip;          //!< Investigator's zip code
	   QString phone;        //!< Investigator's phone number
	   QString email;        //!< Investigator's email address
	   QString organization; //!< Investigator's organization
	 };
  

     private:
        QMap< QString, QString > gmp_run_details;
        QMap< QString, QString > eSign_details;
        QMap< QString, QString > protocol_details;
        QList< US_InvestigatorData > investigators;
        QList< US_InvestigatorData > g_reviewers;
        US_InvestigatorData          info;
        US_InvestigatorData          info_grev;

  //set global Revs. from the list of investigators
        QListWidget*  lw_inv_list;
        QTextEdit*    te_inv_smry;
	QLabel*       lb_inv_search;
        QLineEdit*    le_inv_search;

        QListWidget*  lw_grev_list;
        QTextEdit*    te_grev_smry;
        QLabel*       lb_grev_search;
        QLineEdit*    le_grev_search;  
    
        QPushButton*  pb_set_global_rev;
	QPushButton*  pb_unset_global_rev;			  

  //Set Oper/Revs for selected GMP Run from eligible operators & global reviewers
        QPushButton*  pb_selRun_operRev_set;
        QPushButton*  pb_set_operRev;
  
	QPushButton*  pb_add_oper;
        QPushButton*  pb_remove_oper;
	QPushButton*  pb_add_rev;
        QPushButton*  pb_remove_rev;
    
	QLineEdit*    le_run_name;
        QLineEdit*    le_optima_name;
  
        QTextEdit*    te_operator_names;
        QTextEdit*    te_reviewer_names;
        QTextEdit*    te_opers_to_assign;
        QTextEdit*    te_revs_to_assign;

        QComboBox*    cb_choose_operator;
        QComboBox*    cb_choose_rev;

        QList< QStringList >  autoflowdata;
        US_SelectItem* pdiag_autoflow;
        
        bool      isEsignRecord;
        
  //Download GMP Report form DB && Review, e-Sign, && upload back
       QPushButton* pb_loadreport_db;			   
       QPushButton* pb_view_report_db;
       QPushButton* pb_esign_report;
       QPushButton* pb_view_eSigns;
  
       QLineEdit*   le_loaded_run_db;
       QTextEdit*   te_fpath_info;
       QLineEdit*   le_eSign_status;
  
       US_SelectItem* pdiag_autoflow_db;
       QList< QStringList >  gmpReportsDBdata;
       QString    filePath_db;
       QString    filePath_eSign;
       QString    gmpRunID_eSign;
       QString    eSignID_global;				
			     
				 
     public slots:

     private slots:
       void init_invs     ( void );
       void init_grevs     ( void );
       void limit_inv_names( const QString& );
       void limit_grev_names( const QString& );
       void get_inv_data( QListWidgetItem* );
       void get_grev_data( QListWidgetItem* );
       QString get_inv_or_grev_smry( US_InvestigatorData, QString );
       void set_greviewer( void );
       void unset_greviewer( void );
       void selectGMPRun( void );
       void reset_set_revOper_panel( void );
       void set_revOper_panel_gui( void );
       int list_all_autoflow_records( QList< QStringList >&  );
       QMap < QString, QString > read_autoflow_record( int );
       QStringList read_operators( QString );
       QMap< QString, QString> read_autoflowGMPReportEsign_record( QString );
       QString get_assigned_oper_revs( QJsonDocument );
       void assignOperRevs( void );
       void addOpertoList( void );
       void removeOperfromList( void );
       void addRevtoList( void );
       void removeRevfromList( void );
       bool is_eSignProcessBegan( void );
       void setUnsetPb_operRev( void );
       QString compose_updated_admin_logJson( int, QString, QString );
       void loadGMPReportDB_assigned( void );
       int  list_all_gmp_reports_db( QList< QStringList >&, US_DB2* );
       void  remove_files_by_mask( QString, QStringList );
       void view_report_db ( void );
       void view_eSignatures ( void );
       bool    mkdir         ( const QString&, const QString& );
       void esign_report( void );
       QString compose_updated_eSign_Json( int, QString, QString,  QJsonArray, QJsonArray,
					   QString, QString& );
       void write_pdf_eSignatures( QString, QString, QString, QString );
       QString check_eSign_status_for_gmpReport( void );
       void write_download_eSignatures_DB( QString, QString );
       
  
     signals:  
       void accept_reviewers( QMap< QString, QString > & );
       void cancel_reviewers( QMap< QString, QString > & );
  
  
};
#endif
