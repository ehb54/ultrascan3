#ifndef US_ESIGNER_GMP_H
#define US_ESIGNER_GMP_H

#include <QPrinter>

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
         US_eSignaturesGMP( QStringList );
         US_eSignaturesGMP( QMap <QString, QString>& );

         bool auto_mode;
         bool auto_separate_status;
         bool assign_revs_sep;
         bool reassign_revs_sep;

         class US_InvestigatorData
	 {
           public:
	   int     invID;        //!< The uniqe ID in the DB for the entry.
	   int     ulev;         //!< User level
	   QString gmpReviewerRole; //!<Reviewer | Approver> 
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
        QMap< QString, QString > eSign_details_auto;
        QMap< QString, QString > it_details;
        QStringList reassign;
        QList< US_InvestigatorData > investigators;
        QList< US_InvestigatorData > g_reviewers;
        QList< US_InvestigatorData > g_apprs;
        US_InvestigatorData          info;
        US_InvestigatorData          info_grev;
        US_InvestigatorData          info_gappr;

  //set global Revs. from the list of investigators
        QListWidget*  lw_inv_list;
        QTextEdit*    te_inv_smry;
	QLabel*       lb_inv_search;
        QLineEdit*    le_inv_search;

        QListWidget*  lw_grev_list;
        QTextEdit*    te_grev_smry;
        QLabel*       lb_grev_search;
        QLineEdit*    le_grev_search;
  
        QListWidget*  lw_gappr_list;
        QTextEdit*    te_gappr_smry;
        QLabel*       lb_gappr_search;
        QLineEdit*    le_gappr_search; 
    
        QPushButton*  pb_set_global_rev;
        QPushButton*  pb_set_global_appr;
	QPushButton*  pb_unset_global_rev;			  
        QPushButton*  pb_unset_global_appr;
  
  //Set Oper/Revs for selected GMP Run from eligible operators & global reviewers
        QPushButton*  pb_selRun_operRev_set;
        QPushButton*  pb_set_operRev;
  
	QPushButton*  pb_add_oper;
        QPushButton*  pb_remove_oper;
	QPushButton*  pb_add_rev;
        QPushButton*  pb_remove_rev;
        QPushButton*  pb_add_appr;
        QPushButton*  pb_remove_appr;
    
	QLineEdit*    le_run_name;
        QLineEdit*    le_optima_name;
  
        QTextEdit*    te_operator_names;
        QTextEdit*    te_reviewer_names;
        QTextEdit*    te_appr_names;
        QTextEdit*    te_opers_to_assign;
        QTextEdit*    te_revs_to_assign;
        QTextEdit*    te_apprs_to_assign;

        QComboBox*    cb_choose_operator;
        QComboBox*    cb_choose_rev;
        QComboBox*    cb_choose_appr;

        QList< QStringList >  autoflowdata;
        US_SelectItem* pdiag_autoflow;
        
        bool      isEsignRecord;
        
  //Download GMP Report form DB && Review, e-Sign, && upload back
       QPushButton* pb_loadreport_db;			   
       QPushButton* pb_view_report_db;
       QPushButton* pb_esign_report;
       QPushButton* pb_view_eSigns;
       QPushButton* pb_help;
       QPushButton* pb_cancel;
  
       QLineEdit*   le_loaded_run_db;
       QTextEdit*   te_fpath_info;
       QLineEdit*   le_eSign_status;
  
       US_SelectItem* pdiag_autoflow_db;
       QList< QStringList >  gmpReportsDBdata;
       QString    folderRunName;
       QString    filePath_db;
       QString    filePath_db_html;
       QString    filePath_eSign;
       QString    gmpRunID_eSign;
       QString    eSignID_global;

       QVBoxLayout* topLayout_auto;
  //QHBoxLayout* mainLayout_auto;
  QVBoxLayout* mainLayout_auto;
       QGridLayout* eSignersGrid_auto;
       QGridLayout* eSignActionsGrid_auto;
       QVBoxLayout* leftLayout;				 
       QVBoxLayout* rghtLayout;
       QWidget*     mainWidget_auto;
       QHBoxLayout* lowerButtons;
       QVBoxLayout* spacerLayout;
  
       QString autoflowID_passed;
       QString autoflowGMPReport_id_selected;
  QMap<QString, QString> operators_info;
  QMap<QString, QString> reviewers_info;
  QMap<QString, QString> approvers_info;				       
			      
     public slots:

     private slots:
       void initPanel_auto( QMap < QString, QString > &  );
       void display_reviewers_auto( int&, QMap< QString, QString>, QString );
       void init_invs      ( void );
       void init_grevs     ( void );
       void init_gapprs    ( void );
       void limit_inv_names( const QString& );
       void limit_grev_names( const QString& );
       void limit_gappr_names( const QString& );
       void get_inv_data( QListWidgetItem* );
       void get_grev_data( QListWidgetItem* );
       void get_gappr_data( QListWidgetItem* );
       QString get_inv_or_grev_smry( US_InvestigatorData, QString );
       void set_greviewer( void );
       void set_gappr( void );
       void unset_greviewer( void );
       void unset_gappr( void );
       void selectGMPRun( void );
       void selectGMPRun_sa( void );
       void reset_set_revOper_panel( void );
       void set_revOper_panel_gui( void );
       void set_revOper_panel_gui_sa( void );
       int list_all_autoflow_records( QList< QStringList >& , QString );
       QMap < QString, QString > read_autoflow_record( int, QString );
       QStringList read_operators( QString );
       QMap< QString, QString> read_autoflowGMPReportEsign_record( QString );
       QString get_assigned_oper_revs( QJsonDocument );
       QString get_assigned_oper_revs_sa( QString, QJsonDocument, QMap<QString, QString> );
       void assignOperRevs( void );
       void assignOperRevs_sa( void );
       void addOpertoList( void );
       void removeOperfromList( void );
       void addRevtoList( void );
       void removeRevfromList( void );
       void addApprtoList( void );
       void removeApprfromList( void );
       bool is_eSignProcessBegan( void );
       void disableSetUnsetGButtons( void );
       void setUnsetPb_operRev( void );
       void setUnset_AddRemove_RevAppr_bttn( QString );
       QString compose_updated_admin_logJson( int, QString, QString );
       
       void loadGMPReportDB_assigned( void );
       void loadGMPReportDB_assigned_auto( QString );
       void loadGMPReportDB_assigned_separate( void );
       void compose_updated_ora_list( QString&, QString&, QString&, QString, QString );
  
       int  list_all_gmp_reports_db( QList< QStringList >&, US_DB2* );
       void  remove_files_by_mask( QString, QStringList );
       void view_report_db ( void );
       void view_eSignatures ( void );
       bool    mkdir         ( const QString&, const QString& );
       void esign_report( void );
       QString compose_updated_eSign_Json( int, QString, QString,  QJsonArray, QJsonArray,
					   QString, QString& );
       QString compose_updated_eSign_Json_sa( int, QString, QString, QString, QJsonArray);
					      
  
       QString write_pdf_eSignatures( QString, QString, QString, QString, QString );
       QString check_eSign_status_for_gmpReport( void );
       QString check_revs_esign_status_sa( QString, QMap< QString, QString> );
       QLineEdit* check_eSign_status_for_gmpReport_auto( QString, QMap< QString, QString> );
       void write_download_eSignatures_DB( QString, QString );

       void reset_esign_panel ( void );

       void paintPage(QPrinter& printer, int pageNumber, int pageCount,
       		     QPainter* painter, QTextDocument* doc,
		      const QRectF& textRect, qreal footerHeight, QMap< QString, QMap< QString, QString>>);
       void printDocument(QPrinter& printer, QTextDocument* doc, QMap< QString, QMap< QString, QString>> );
                        //QWidget* parentWidget);
       double mmToPixels(QPrinter& printer, int mm);

       QMap< QString, QMap< QString, QString>> json_to_qmap( QString );
       QString get_role_by_name( QString );
       
  
     signals:  
       void accept_reviewers( QMap< QString, QString > & );
       void cancel_reviewers( QMap< QString, QString > & );
  
  
};
#endif
