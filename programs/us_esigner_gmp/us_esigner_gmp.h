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
        QMap< QString, QString > protocol_details;
        QList< US_InvestigatorData > investigators;
        US_InvestigatorData          info;

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
				    
	QLineEdit*   le_run_name;
        QLineEdit*   le_optima_name;
        QLineEdit*   le_operator_names;
        QLineEdit*   le_reviewer_names;

        QComboBox*   cb_choose_operator;
	QComboBox*   cb_choose_rev1;
	QComboBox*   cb_choose_rev2;

  //Download GMP Report form DB && Review, e-Sign, && upload back
       QPushButton* pb_loadreport_db;			   
       QPushButton* pb_view_report_db;
       QPushButton* pb_esign_report;

       QLineEdit*   le_loaded_run_db;

       QTextEdit*   te_fpath_info;			    

     public slots:

     private slots:
       void init_invs     ( void );
       void init_grevs     ( void );
       void limit_inv_names( const QString& );
       void get_inv_data( QListWidgetItem* );
       QString get_inv_or_grev_smry( US_InvestigatorData, QString );

     signals:  
       void accept_reviewers( QMap< QString, QString > & );
       void cancel_reviewers( QMap< QString, QString > & );
  
};
#endif
