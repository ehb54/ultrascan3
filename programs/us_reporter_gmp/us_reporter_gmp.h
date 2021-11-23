#ifndef US_REPORTER_GMP_H
#define US_REPORTER_GMP_H

#include "us_widgets.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "us_run_protocol.h"
#include "us_protocol_util.h"
#include "../us_analysis_profile/us_analysis_profile.h"
#include "us_solution.h"
#include "us_help.h"
#include "us_extern.h"
#include "us_select_item.h"

class US_ReporterGMP : public US_Widgets
{
   Q_OBJECT

      public:
         US_ReporterGMP();
	 
	 /* QVBoxLayout* panel; */
	 /* QGridLayout* genL; */

	 QTreeWidget     *genTree;
	 QTreeWidget     *perChanTree;
	 
	 QProgressDialog * progress_msg;
	 US_RunProtocol    currProto;

	 QString ap_xml;

	 US_AnalysisProfileGui*     sdiag; 
	 US_AnaProfile              currAProf;
	 US_AnaProfile::AnaProf2DSA cAP2;
	 US_AnaProfile::AnaProfPCSA cAPp;
	 QStringList  chndescs;       
	 QStringList  chndescs_alt;
	 QMap< QString, QMap < QString, US_ReportGMP > > ch_reports;
	 QMap< QString, QList< double > > ch_wvls;
	 QString      reportMask;

	 QMap <QString, bool >    ShowReportParts;
	 QMap <QString, QString > ShowSolutionParts;
	 QMap <QString, QString > ShowAnalysisGenParts;
	 QMap <QString, QString > ShowAnalysis2DSAParts;
	 QMap <QString, QString > ShowAnalysisPCSAParts;
	 int has_anagen_items;
	 int has_ana2dsa_items;
	 int has_anapcsa_items;
	 
      private:

	 //General report Mask
	 QJsonObject json;
	 QMap<QString, QTreeWidgetItem *> topItem;
	 QMap<QString, QTreeWidgetItem *> solutionItem;
	 QMap<QString, QTreeWidgetItem *> analysisItem;
	 QMap<QString, QTreeWidgetItem *> analysisGenItem;
	 QMap<QString, QTreeWidgetItem *> analysis2DSAItem;
	 QMap<QString, QTreeWidgetItem *> analysisPCSAItem;
	 QStringList topLevelItems;
	 
	 QStringList solutionItems;
	 QStringList solutionItems_vals;
	 
	 QStringList analysisItems;
	 
	 QStringList analysisGenItems;
	 QStringList analysisGenItems_vals;
	 
	 QStringList analysis2DSAItems;
	 QStringList analysis2DSAItems_vals;
	 
	 QStringList analysisPCSAItems;
	 QStringList analysisPCSAItems_vals;
	 //End of general report mask
	 
	 QList< QStringList >  autoflowdata;
	 US_SelectItem* pdiag_autoflow;

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
	 
	 US_Help       showHelp;

	 QPushButton*  pb_download_report;
	 QPushButton*  pb_gen_report  ;
	 QPushButton*  pb_view_report ;
	 QPushButton*  pb_reset_trees ;
	 QPushButton*  pb_help;
	 QPushButton*  pb_close;
	 QLineEdit*    le_loaded_run;

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
	 
	 void  get_current_date( void );
	 void  format_needed_params( void );
	 void  assemble_pdf( void );
	 void  add_solution_details( const QString, const QString, QString& );
	 void  parse_mask_json ( void );
	 void  assemble_parts( QString& );
	 int   list_all_autoflow_records( QList< QStringList >&  );
	 QMap < QString, QString > read_autoflow_record( int );

	 void read_protocol_and_reportMasks( void );
	 void parse_gen_mask_json ( void );
	 void build_genTree ( void );

      private slots:
	void reset_report_panel ( void );
	void view_report ( void );
	void load_gmp_run ( void );
	void changedItem_gen ( QTreeWidgetItem*, int );
	
	
	void help          ( void )
	{ showHelp.show_help( "reporter_gmp.html" ); };
	
      signals:
	void reset_reports( void );
   
};
#endif
