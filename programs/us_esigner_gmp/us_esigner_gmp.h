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

     private:
        QMap< QString, QString > protocol_details;

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

     signals:  
       void accept_reviewers( QMap< QString, QString > & );
       void cancel_reviewers( QMap< QString, QString > & );
  
};
#endif
