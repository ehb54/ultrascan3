#ifndef US_ANALYSIS_AUTO_H
#define US_ANALYSIS_AUTO_H

#include "us_widgets.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "../us_fit_meniscus/us_fit_meniscus.h"
#include "../us_fematch/us_fematch.h"


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
	 QMap<QString, QTreeWidgetItem *> childItem_PCSA;

	 QMap<QString, QGroupBox *> groupbox_2DSA;
	 QMap<QString, QGroupBox *> groupbox_2DSA_FM;
	 QMap<QString, QGroupBox *> groupbox_2DSA_FITMEN;
	 QMap<QString, QGroupBox *> groupbox_2DSA_IT;
	 QMap<QString, QGroupBox *> groupbox_2DSA_MC;
	 QMap<QString, QGroupBox *> groupbox_PCSA;

	 QTimer * timer_update;
	 QTimer * timer_end_process;

	 bool in_gui_update;
	 bool in_reload_end_process;

	 void reset_analysis_panel_public ( void );

	 US_FitMeniscus* FitMen;

	 QPointer< US_ResidPlotFem >    resplotd;


	 US_DataIO::EditedData*      aa_editdata();
	 US_DataIO::RawData*         aa_simdata();
	 QList< int >*               aa_excllist();
	 US_Model*                   aa_model();
	 US_Noise*                   aa_ti_noise();
	 US_Noise*                   aa_ri_noise();
	 QPointer< US_ResidsBitmap > aa_resbmap();
	 QString                     aa_tripleInfo();

	 QMessageBox * msg_sim;
	 QProgressDialog * progress_msg;
	 
      private:
	 QVector< US_DataIO::RawData    > rawData;
	 QVector< US_DataIO::EditedData > editedData;

	 QVector< SP_SPEEDPROFILE >     speed_steps;
	 
	 US_DataIO::EditedData*      edata;
	 US_DataIO::RawData*         rdata;
	 US_DataIO::RawData*         sdata;
	 US_DataIO::RawData          wsdata;
	 
	 QPointer< US_ResidsBitmap >    rbmapd;

	 QString tripleInfo;
	 
	 int eID_global;
	 
	 US_Model                    model;
	 US_Model                    model_loaded;
	 US_Model                    model_used;
	 
	 US_Noise                    ri_noise;
	 US_Noise                    ti_noise;
	 QList< int >                excludedScans;
	 US_Solution                 solution_rec;

	 US_Math2::SolutionData      solution;
	 QVector< QVector< double > > resids;
	 
	 US_SimulationParameters     simparams;
	 QList< US_DataIO::RawData >   tsimdats;
	 QList< US_Model >             tmodels;
	 QVector< int >                kcomps;

	 int           thrdone;
	 
	 double       density;
	 double       viscosity;
	 double       vbar;
	 double       compress;

	 QString      svbar_global;
	 
	 bool         manual;
	 bool          dataLoaded;
	 bool          haveSim;
	 bool          dataLatest;
	 bool          buffLoaded;
	 bool          cnstvb;
	 bool          cnstff;
	 bool          exp_steps;
	 bool          dat_steps;
	 bool          is_dmga_mc;

	 QMap< QString, QString >    adv_vals;

	 int           dbg_level;
	 int           nthread;
	 int           scanCount;

	 QPoint        rpd_pos;

	 QPushButton*  pb_show_all;
	 QPushButton*  pb_hide_all;

	 bool         all_processed;
	 
	 QString    AProfileGUID;
	 QString    ProtocolName_auto;
	 int        invID;
	 QString    analysisIDs;
	 QMap <QString, QString >    investigator_details;
	 QString     defaultDB;
	 QString    FileName;

	 int        sim_msg_pos_x;
	 int        sim_msg_pos_y;
	 
	 QVector< QString >  Array_of_triples;

	 QMap < QString, QMap< QString, QString > > Array_of_analysis;
	 QMap < QString, QMap< QString, QString > > Array_of_analysis_by_requestID;
	 QMap < QString, QStringList > Channel_to_requestIDs;

	 QStringList channels_all;

	 QMap < QString, bool > Manual_update;
	 QMap < QString, bool > History_read;
	 QMap < QString, bool > Completed_triples;
	 QMap < QString, bool > Failed_triples;
	 QMap < QString, bool > Canceled_triples;
	 QMap < QString, bool > Process_2dsafm;

	 QMap < QString, QString > read_autoflowAnalysis_record( US_DB2*, const QString& );
	 QMap < QString, QString > read_autoflowAnalysisHistory_record( US_DB2*, const QString& );

	 QMap < QString, QString > get_investigator_info ( US_DB2*, const QString& );

	 void update_autoflowAnalysis_status_at_fitmen ( US_DB2*, const QStringList& );

	 void update_autoflowAnalysis_uponDeletion ( US_DB2*, const QString& );
	 void update_autoflowAnalysis_uponDeletion_other_wvl ( US_DB2*, const QStringList& );

	 bool loadData( QMap < QString, QString > & );
	 bool loadModel( QMap < QString, QString > & );
	 bool loadNoises( QMap < QString, QString > & );

	 void simulateModel( void );
	 void adjustModel( void );
	 
	 QGroupBox *createGroup( QString &);

	 QSignalMapper  *signalMapper;
	 QSignalMapper  *signalMapper_overlay;
	 
	 //2DSA
	 bool        job1run;         //!< Run 1 (2dsa) run flag
	 bool        job2run;         //!< Run 2 (2dsa_fm) run flag
	 bool        job3run;         //!< Run 3 (fitmen) run flag
	 bool        job4run;         //!< Run 4 (2dsa_it) run flag
	 bool        job5run;         //!< Run 5 (2dsa_mc) run flag
	 bool        job3auto;        //!< Fit-Meniscus auto-run flag
	 bool        job6run_pcsa;
	 QString     job1nois;        //!< 2DSA noise type
	 QString     job2nois;        //!< 2DSA-FM noise type
	 QString     job4nois;        //!< 2DSA-IT noise type

      public slots:
	void    thread_progress( int, int );
	void    thread_complete( int );
	void    resplot_done( void );
	void    update_progress( int );
	
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

	void delete_job  ( QString );
	void show_overlay( QString );

	void show_results   ( void );
	void calc_residuals( void );
	double  interp_sval( double, double*, double*,  int );
	void plotres(   void );

	
	
      signals:
	void analysis_update_process_stopped( void );
	void close_analysissetup_msg( void ); 
   
};
#endif