#ifndef US_AUDITTRAIL_GMP_H
#define US_AUDITTRAIL_GMP_H

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

class US_auditTrailGMP : public US_Widgets
{
   Q_OBJECT

      public:
         US_auditTrailGMP();

   
     private:
       QVBoxLayout* topLayout_auto;
       QPushButton* pb_loadreport_db;
       QPushButton* pb_viewAPDF;
  
       QVBoxLayout* mainLayout_auto;
       QGridLayout* loadedRunGrid;
       QGridLayout* eSignersGrid;
       QGridLayout* userInteractions;
       QGridLayout* lowerButtons;

  QVBoxLayout* leftLayout;				 
  QVBoxLayout* rghtLayout;
  QWidget*     mainWidget_auto;
       
      
       QVector < QGridLayout* > gridLayoutVector;

       QTreeWidget* eSignTree;
       QTreeWidget* uInteractionsTree;
  
       US_SelectItem* pdiag_autoflow_db;
       QString autoflowID_passed;
  QString gmpRunName_passed;
       int autoflowStatusID;
       QList< QStringList >  gmpReportsDBdata;
       bool       isEsignRecord;
       QString    gmpRunID_eSign;
       QString    AProfileGUID;
       QMap<QString, QString> eSign_details_auto;
       bool p_2dsa_run_fitmen;					
       bool p_2dsa_auto_fitmen;

       int RowHeight;
       QString html_assembled;
       QString html_assembled_esigs;
       QString filePath_pdf;
			    
			   
     public slots:

     private slots:
       void loadGMPReport( void );
       void printAPDF( void );
       void viewAPDF( void );
       bool    mkdir         ( const QString&, const QString& );
       int  list_all_gmp_reports_db( QList< QStringList >&, US_DB2* );
       QMap< QString, QString> read_autoflowGMPReportEsign_record( QString );
       void initPanel_auto( QMap < QString, QString > &  );
       void reset_panel ( void );
       QGroupBox * createGroup_eSign( QString );
       QVector<QGroupBox *> createGroup_stages( QString, QString );
       void display_reviewers_auto( int&, QMap< QString, QString>, QString, QGridLayout*);
       QMap < QString, QString > read_autoflow_record( int );
       QLineEdit* check_eSign_status_for_gmpReport_auto( QString, QMap< QString, QString> );
       void  read_autoflowStatus_record( QString&,  QString&,  QString&,  QString&,
					 QString&,  QString&,  QString&,  QString&, QString&,
					 QString&,  QString&,  QString&,  QString&, QString&,
					 QString&,  QString& );
       QMap< QString, QMap< QString, QString > >  parse_autoflowStatus_json( const QString, const QString  );
       QMap < QString, QString > parse_autoflowStatus_analysis_json( QString );
       void read_reportLists_from_aprofile( QStringList &, QStringList & );
       bool readReportLists( QXmlStreamReader&, QMap< QString, QString> &, QMap< QString, QString> & );
       bool read_2dsa_settings( QXmlStreamReader& );

       QStringList buildDroppedTriplesList ( US_DB2*, QMap <QString, QString> );
       void assemble_GMP_init( QMap< QString, QMap < QString, QString > >, QString );
       void assemble_GMP_live_update( QMap< QString, QMap < QString, QString > >, QString, QString );
       void assemble_GMP_import( QMap< QString, QMap < QString, QString > >, QString, QString );
       void assemble_GMP_editing( QMap< QString, QMap < QString, QString > >, QString, QString );
       void assemble_GMP_analysis_fitmen( QMap < QString, QString > );
       void assemble_GMP_analysis_cancelled( QMap < QString, QString >, QString );
  void assemble_esigs( QMap<QString, QString>); 
  void initHTML( void );
  void closeHTML( void );

  
     signals:  
  
  
  
};
#endif
