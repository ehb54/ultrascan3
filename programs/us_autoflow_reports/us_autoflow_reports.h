#ifndef US_REPORTS_AUTO_H
#define US_REPORTS_AUTO_H

#include "us_widgets.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "us_run_protocol.h"
#include "us_protocol_util.h"
#include "../us_analysis_profile/us_analysis_profile.h"
#include "us_solution.h"

class US_Reports_auto : public US_Widgets
{
   Q_OBJECT

      public:
         US_Reports_auto();
	 
	 QVBoxLayout* panel;
	 QGridLayout* genL;

	 QProgressDialog * progress_msg;
	 US_RunProtocol    currProto;

	 US_AnalysisProfileGui*     sdiag; 
	 US_AnaProfile              currAProf;
	 US_AnaProfile::AnaProf2DSA cAP2;
	 US_AnaProfile::AnaProfPCSA cAPp;
	 QStringList  chndescs;       
	 QStringList  chndescs_alt;
	 QMap< QString, QMap < QString, US_ReportGMP > > ch_reports;
	 QMap< QString, QList< double > > ch_wvls;

	 QMap <QString, bool >    ShowReportParts;
	 QMap <QString, QString > ShowSolutionParts;
	 QMap <QString, QString > ShowAnalysisGenParts;
	 QMap <QString, QString > ShowAnalysis2DSAParts;
	 QMap <QString, QString > ShowAnalysisPCSAParts;
	 int has_anagen_items;
	 int has_ana2dsa_items;
	 int has_anapcsa_items;
	 
      private:
	 QString html_general;
	 QString html_lab_rotor;
	 QString html_operator;
	 QString html_speed;
	 QString html_cells;
	 QString html_solutions;
	 QString html_optical;
	 QString html_ranges;
	 QString html_scan_count;
	 QString html_analysis_profile;
	 QString html_analysis_profile_2dsa;
	 QString html_analysis_profile_pcsa; 
	 
	 QString reportMask;
	 /* QStringList topLevelItems; */
	 /* QStringList topLevelItems_vals; */
	 
	 QStringList solutionItems;
	 QStringList solutionItems_vals;
	 
	 QStringList analysisItems;
	 
	 QStringList analysisGenItems;
	 QStringList analysisGenItems_vals;
	 
	 QStringList analysis2DSAItems;
	 QStringList analysis2DSAItems_vals;
	 
	 QStringList analysisPCSAItems;
	 QStringList analysisPCSAItems_vals;
	 
	 QPushButton*  pb_download_report;

	 QString    AProfileGUID;
	 QString    ProtocolName_auto;
	 int        invID;
	 QString    runID;
	 QString    filePath;

	 QString    current_date;
	 
	 QString    duration_str;
	 QString    delay_stage_str;
	 QString    total_time_str;

	 QString    delay_uvvis_str;
	 QString    scanint_uvvis_str;
	 QString    delay_int_str;
	 QString    scanint_int_str;

	 int        ncells_used;
	 int        nsol_channels;
	 int        nchan_optics;
	 int        nchan_ranges;

	 bool       has_uvvis;
	 bool       has_interference;
	 bool       has_fluorescense;
	 
	 QVector< QString >  Array_of_triples;
	 
	 void  read_protocol_data_triples( void );
	 bool  read_protoOptics( QXmlStreamReader&  );
	 bool  read_protoRanges( QXmlStreamReader&  );

	 void  get_current_date( void );
	 void  format_needed_params( void );
	 void  assemble_pdf( void );
	 void  add_solution_details( const QString, const QString, QString& );
	 void  add_ranges_details(  QString& );
	 void  inherit_protocol( US_RunProtocol* );
	 void  parse_mask_json ( void );
	 void  assemble_parts( QString& );

      private slots:
	void initPanel( QMap < QString, QString > & );
	void reset_report_panel ( void );
	void view_report ( void );
	
      signals:
	void reset_reports( void );
   
};
#endif
