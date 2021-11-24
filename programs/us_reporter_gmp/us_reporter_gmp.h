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
	 	 
	 QTreeWidget     * genTree;
	 QTreeWidget     * perChanTree;
	 
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
	 QMap< QString, QMap < QString, US_ReportGMP > > ch_reports_internal;
	 QMap< QString, QList< double > > ch_wvls;
	 // QString      reportMask;

	 struct GenReportMaskStructure
	 {
	   QMap <QString, bool >    ShowReportParts;
	   QMap <QString, QString > ShowSolutionParts;
	   QMap <QString, QString > ShowAnalysisGenParts;
	   QMap <QString, QString > ShowAnalysis2DSAParts;
	   QMap <QString, QString > ShowAnalysisPCSAParts;
	   int has_anagen_items;
	   int has_ana2dsa_items;
	   int has_anapcsa_items;
	 };

	 GenReportMaskStructure genMask_edited;

	 struct PerChanReportMaskStructure
	 {
	   
	 };

	 PerChanReportMaskStructure perChanMask_edited;
	 
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

	 //perChan Report masks
	 QMap<QString, QTreeWidgetItem *> chanItem;
	 QMap<QString, QTreeWidgetItem *> tripleItem;
	 QMap<QString, QTreeWidgetItem *> tripleMaskItem;
	 
	 
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
	 QPushButton*  pb_select_all ;
	 QPushButton*  pb_unselect_all ;
	 QPushButton*  pb_expand_all ;
	 QPushButton*  pb_collapse_all ;
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
	 void  assemble_parts( QString& );
	 int   list_all_autoflow_records( QList< QStringList >&  );
	 QMap < QString, QString > read_autoflow_record( int );

	 void read_protocol_and_reportMasks( void );
	 void parse_gen_mask_json ( const QString  );
	 void build_genTree ( void );
	 void build_perChanTree ( void ) ;
	 void gui_to_parms ( void ) ;
	 QString tree_to_json ( QMap < QString, QTreeWidgetItem * > );
	 void parse_edited_gen_mask_json( const QString, GenReportMaskStructure &  );
	 void parse_edited_perChan_mask_json( const QString, PerChanReportMaskStructure &  );
	 
      private slots:
	void reset_report_panel ( void );
	void view_report ( void );
	void load_gmp_run ( void );
	void generate_report( void );
	void changedItem_gen    ( QTreeWidgetItem*, int );
	void changedItem_triple ( QTreeWidgetItem*, int );
	void select_all( void );
	void unselect_all( void );
	void expand_all( void );
	void collapse_all( void );
			
	void help          ( void )
	{ showHelp.show_help( "reporter_gmp.html" ); };
	
      signals:
	void reset_reports( void );
   
};
#endif
