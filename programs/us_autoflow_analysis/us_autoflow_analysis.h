#ifndef US_ANALYSIS_AUTO_H
#define US_ANALYSIS_AUTO_H

#include "us_widgets.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "../us_fit_meniscus/us_fit_meniscus.h"

class US_Analysis_auto : public US_Widgets
{
   Q_OBJECT

      public:
         US_Analysis_auto();
	 
	 QTreeWidget     *treeWidget;
	 QMap<QString, QTreeWidgetItem *> topItem;
	 QMap<QString, QTreeWidgetItem *> childItem_2DSA;
	 QMap<QString, QTreeWidgetItem *> childItem_2DSA_FM;
	 QMap<QString, QTreeWidgetItem *> childItem_2DSA_FITMEN;
	 QMap<QString, QTreeWidgetItem *> childItem_2DSA_IT;
	 QMap<QString, QTreeWidgetItem *> childItem_2DSA_MC;

	 QMap<QString, QGroupBox *> groupbox_2DSA;
	 QMap<QString, QGroupBox *> groupbox_2DSA_FM;
	 QMap<QString, QGroupBox *> groupbox_2DSA_FITMEN;
	 QMap<QString, QGroupBox *> groupbox_2DSA_IT;
	 QMap<QString, QGroupBox *> groupbox_2DSA_MC;

	 QTimer * timer_update;
	 QTimer * timer_end_process;

	 bool in_gui_update;
	 bool in_reload_end_process;

	 void reset_analysis_panel_public ( void );

	 US_FitMeniscus* FitMen; 
	 
      private:
	 QPushButton*  pb_show_all;
	 QPushButton*  pb_hide_all;
	 
	 QString    AProfileGUID;
	 QString    ProtocolName_auto;
	 int        invID;
	 QString    analysisIDs;
	 QMap <QString, QString >    investigator_details;
	 QString     defaultDB;
	 QString    FileName;
	 
	 QVector< QString >  Array_of_triples;

	 QMap < QString, QMap< QString, QString > > Array_of_analysis;
	 QMap < QString, QStringList > Channel_to_requestIDs;

	 QStringList channels_all;

	 QMap < QString, bool > Manual_update;
	 QMap < QString, bool > History_read;
	 QMap < QString, bool > Completed_triples;
	 QMap < QString, bool > Failed_triples;

	 QMap < QString, QString > read_autoflowAnalysis_record( US_DB2*, const QString& );
	 QMap < QString, QString > read_autoflowAnalysisHistory_record( US_DB2*, const QString& );

	 QMap < QString, QString > get_investigator_info ( US_DB2*, const QString& );

	 void update_autoflowAnalysis_status_at_fitmen ( US_DB2*, const QStringList& );
	 
	 void  read_protocol_data_triples( void );
	 bool  read_protoOptics( QXmlStreamReader&  );
	 bool  read_protoRanges( QXmlStreamReader&  );
	 
	 void  read_aprofile_data_from_aprofile( void );
	 bool  readAProfile ( QXmlStreamReader& );
	 bool  readAProfile_2DSA ( QXmlStreamReader& );
	 static bool bool_flag( const QString );

	 QGroupBox *createGroup( QString &);

	 //2DSA
	 bool        job1run;         //!< Run 1 (2dsa) run flag
	 bool        job2run;         //!< Run 2 (2dsa_fm) run flag
	 bool        job3run;         //!< Run 3 (fitmen) run flag
	 bool        job4run;         //!< Run 4 (2dsa_it) run flag
	 bool        job5run;         //!< Run 5 (2dsa_mc) run flag
	 bool        job3auto;        //!< Fit-Meniscus auto-run flag
	 QString     job1nois;        //!< 2DSA noise type
	 QString     job2nois;        //!< 2DSA-FM noise type
	 QString     job4nois;        //!< 2DSA-IT noise type

      private slots:
	void initPanel( QMap < QString, QString > & );
	void show_all( void );
	void hide_all( void );
	void gui_update ( void );

	void gui_update_temp ( void );

	void reset_analysis_panel ( void );
	void end_process( void );
	void reset_auto     ( void );

	void update_autoflowAnalysis_statuses( QMap < QString, QString > & );
     
      signals:
	void analysis_update_process_stopped( void );
	void close_analysissetup_msg( void ); 
   
};
#endif
