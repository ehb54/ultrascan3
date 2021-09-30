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
	 	 
      private:
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

      private slots:
	void initPanel( QMap < QString, QString > & );
	void reset_report_panel ( void );
	void view_report ( void );
	
      signals:
	void reset_reports( void );
   
};
#endif
