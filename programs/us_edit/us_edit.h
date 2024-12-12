#ifndef US_EDIT_H
#define US_EDIT_H

#include <QApplication>
#include <QDomDocument>

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_plot.h"
#include "us_dataIO.h"
#include "us_db2.h"
#include "us_mwl_data.h"
#include "qwt_plot_marker.h"
//#include "us_license_t.h"
//#include "us_license.h"


class US_Edit : public US_Widgets
{
   Q_OBJECT

      public:
         US_Edit();
	 
	 // New constructor for automated read/upload/update
	 US_Edit(QString auto_mode);

	 US_Edit( QVector< US_DataIO::RawData > allData, QStringList  triples,
		  QString  workingDir, int currChInd, int plotind );

	 //void us_mode_passed  ( void );
	 //bool usmode;

	 bool us_edit_auto_mode;
	 bool us_edit_auto_mode_manual;
	 bool all_loaded;
	 bool is_spike_auto;
	  

      private:
	 US_Edit*              sdiag;
	 

      enum { MENISCUS, AIRGAP, RANGE, PLATEAU, BASELINE, FINISHED } step;

      class Edits
      {
         public:
            int scan;
            QList< QPointF > changes;
      };

      QList< Edits >                    changed_points;
      US_DataIO::RawData                data;
      US_DataIO::RawData*               edata;
      QList< US_DataIO::SpeedData >     sData;
      QVector< US_DataIO::RawData >     allData;
      QVector< US_DataIO::RawData* >    outData;

      US_MwlData         mwl_data;

      bool               changes_made;
      bool               spikes;
      bool               floatingData;
      bool               expIsVelo;
      bool               expIsEquil;
      bool               expIsDiff;
      bool               expIsOther;
      bool               all_edits;
      bool               men_1click;

      int                noise_order;
      int                triple_index;
      int                data_index;
      int                total_speeds;
      int                total_edits;
      int                dbg_level;

      double             meniscus;
      double             meniscus_left;
      double             airGap_left;
      double             airGap_right;
      double             range_left;
      double             range_right;
      double             baseline;

      int                scanExcl_begin_ind;
      int                scanExcl_end_ind;

      double             baseline_od;
      
      double             invert;
      double             plateau;
      double             gap_fringe;
      double             gap_thresh;
      double             bottom;

      QStringList        triple_info;
      QMap< QString, QStringList> editProfile;
      QMap< QString, QStringList> editProfile_scans_excl;
      QMap< QString, bool> automatic_meniscus;
      QMap< QString, QString> manual_edit_comments;

      QStringList        centparms_info;
      QMap< int, QStringList>  centerpieceParameters;
      QMap< QString, QString>  centerpiece_info;
      QVector<QString>         centerpiece_names;
      QMap <QString, QString>  cell_to_centerpiece;

      QString    AProfileGUID;
      double     aprofile_volume;
      QMap <QString, QStringList>  aprof_channel_to_parms;
      QMap <QString, QStringList>  aprof_channel_to_scans;
      QMap <int, QStringList>      aprofileParameters;
      

      QMap < QString, QString > details_at_editing_local;
      QMap < QString, QString > gmp_submitter_map;
  
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

      
      QChar              chlamb;

      QList< int >       sd_offs;        // Speed data offsets, ea. triple
      QList< int >       sd_knts;        // Speed data counts, ea. triple
      QList< int >       includes;       // Scan includes, current triple
      QList< double >    residuals;      // Noise residuals, current triple
      QVector< QString > editGUIDs;      // Edit GUIDs, ea. i/p triple
      QVector< QString > editIDs;        // Edit DB IDs, ea. i/p triple
      QVector< QString > editFnames;     // Edit file names, ea. i/p triple

      US_Help            showHelp;

      QIcon              check;

      QString            workingDir;
      QString            runID;
      QString            editLabel;
      QString            dataType;
      QString            expType;
      QStringList        files;
      QStringList        triples;
      
      QStringList        triples_all_optics;
      QStringList        channels_all;
      QMap < QString, bool > isSet_ref_wvl;

      QMap < QString, bool > channels_to_analyse;
      QMap < QString, QString > triples_skip_analysis;
      QMap < QString, QString > triple_to_edit;

      QVector < int > iwavl_edit_ref;
      QVector < int > iwavl_edit_ref_index;
      QVector < int > triple_plot_first_time;

      QMap < QString, bool > edited_triples_abde;
      
      QStringList        trip_rpms;
      QStringList        celchns;
      QStringList        rawc_wvlns;
      QStringList        expc_wvlns;
      QStringList        expc_radii;
                      
      QwtPlot*           data_plot;
      QwtPlotCurve*      raw_curve;
      QwtPlotCurve*      fit_curve;
      QwtPlotCurve*      v_line;
      QwtPlotCurve*      minimum_curve;
      QwtPlotGrid*       grid;
      QwtPlotMarker*     marker;
      US_PlotPicker*     pick;
      US_Plot*           plot;
      
      QLabel*            lb_edtrsp;
      QLabel*            lb_gaps;
      QLabel*            lb_rpms;
      QLabel*            lb_triple;
      QLabel*            lb_baseline;
      QLabel*            lb_plateau;
      QLabel*            lb_dataStart;
      QLabel*            lb_dataEnd;
      QLabel*            lb_meniscus;
      QLabel*            lb_airGap;

      QLineEdit*         le_status;
      QLineEdit*         le_investigator;
      QLineEdit*         le_info;
      QLineEdit*         le_meniscus;
      QLineEdit*         le_airGap;
      QLineEdit*         le_dataRange;
      QLineEdit*         le_dataStart;
      QLineEdit*         le_dataEnd;
      QLineEdit*         le_plateau;
      QLineEdit*         le_baseline;
      QLineEdit*         le_edtrsp;
                        
      QPushButton*       pb_details;
      QPushButton*       pb_report;
      QPushButton*       pb_exclude;
      QPushButton*       pb_excludeRange;
      QPushButton*       pb_exclusion;
      QPushButton*       pb_include;
      QPushButton*       pb_edit1;
      QPushButton*       pb_meniscus;
      QPushButton*       pb_airGap;
      QPushButton*       pb_dataRange;
      QPushButton*       pb_dataEnd;
      QPushButton*       pb_plateau;
      QPushButton*       pb_noise;
      QPushButton*       pb_spikes;
      QPushButton*       pb_invert;
      QPushButton*       pb_write;

      QPushButton*       pb_pass;
      QPushButton*       pb_emanual;
      
      QPushButton*       pb_residuals;
      QPushButton*       pb_priorEdits;
      QPushButton*       pb_reviewep;
      QPushButton*       pb_nexteqtr;

      QPushButton*       pb_nextChan;
      QPushButton*       pb_priorChan;

      QPushButton*       pb_float;
      QPushButton*       pb_undo;

      US_Disk_DB_Controls* disk_controls; //!< Radiobuttons for disk/db choice

      QComboBox*         cb_triple;
      QComboBox*         cb_rpms;
                        
      QwtCounter*        ct_from;
      QwtCounter*        ct_to;
      QwtCounter*        ct_noise;
      QwtCounter*        ct_gaps;

      // MWL Controls - only shown if multi-wavelength data
      QLabel*            lb_mwlctl;
      QLabel*            lb_ldelta;
      QLabel*            lb_lstart;
      QLabel*            lb_lend;
      QLabel*            lb_lplot;
      QLabel*            lb_odlim;

      QwtCounter*        ct_ldelta;
      QwtCounter*        ct_odlim;

      QLineEdit*         le_lxrng;
      QLineEdit*         le_ltrng;

      QComboBox*         cb_lplot;
      QComboBox*         cb_lstart;
      QComboBox*         cb_lend;

      QPushButton*       pb_larrow;
      QPushButton*       pb_rarrow;
      QPushButton*       pb_custom;
      QPushButton*       pb_incall;

      QRadioButton*      rb_lrange;
      QRadioButton*      rb_custom;
      QRadioButton*      rb_radius;
      QRadioButton*      rb_waveln;

      QCheckBox*         ck_writemwl;

      QGridLayout*       lo_lrange;
      QGridLayout*       lo_custom;
      QGridLayout*       lo_radius;
      QGridLayout*       lo_waveln;
      QGridLayout*       lo_writemwl;

      bool               isMwl;
      bool               xaxis_radius;
      bool               lsel_range;
      bool               lrng_bycell;

      int                dlambda;
      int                slambda;
      int                elambda;
      int                plotrec;
      int                plotndx;
      int                nwaveln;
      int                nwavelo;
      int                maxwavl;
      // int                nrpoint;
      int                ncelchn;
      int                ntriple;

      double             odlimit;

      US_DB2*            dbP;

      QVector< double >  expd_radii;
      QVector< int >     expi_wvlns;
      QVector< int >     rawi_wvlns;
      QVector< int >     toti_wvlns;

      QVector< QVector< int > >     wavelns_i;

      QVector< QVector< double > >  rdata;
      QVector< QVector< double > >  rdata_xvals;

      QString filename_runID_passed;
      QString filename_runID_auto;
      QString filename_runID_auto_base;
      QString idInv_auto;
      QString ProtocolName_auto;
      int     autoflowID_passed;
      int     autoflowStatusID;
      QString autoflow_expType;
      QString dataSource;
      bool    simulated_data;

      class DataDesc_auto   // Description of each data set in the list presented
      {
         public:
            QString label;           // Item label string
            QString runID;           // Run identifier string
            QString tripID;          // Triple identifier string
            QString filename;        // File name
            QString rawGUID;         // Raw data global identifier string
            QString date;            // Date/time last updated
            QString dcheck;          // Data checksum+size
            int     DB_id;           // Database ID number
            int     tripknt;         // Count of triples per run
            int     tripndx;         // Index of triple in run
      };

      QMap< QString, DataDesc_auto >    datamap;  // Map: labels,data-desc-objs

      bool          runType_combined_IP_RI;
      QMap < QString, int > runTypes_map;
      QString       type_to_process;
            
      
      // Private slots
      void set_pbColors      ( QPushButton* );
      void draw_vline        ( double );
      void next_step         ( void );
      void gap_check         ( void );

      void replot            ( void );
      void plot_current      ( void );
      void plot_all          ( void );
      void plot_range        ( void );
      void plot_last         ( void );
      void plot_current      ( int  );
      void plot_scan         ( void );
      void plot_mwl          ( void );
      void init_includes     ( void );
      
      void reset_excludes    ( void );
      void set_colors        ( const QList< int >& );

      double find_meniscus_auto ( void );
      double find_meniscus_interference_auto ( void );
            
      QVector<double> find_airGap_interference_auto ( void );
      QVector<double> airGap_values;
      
      int  scan_db_auto      ( void );
      void create_descs_auto ( QStringList&, QStringList&, int );
      void load_db_auto      ( QList< DataDesc_auto >& );

      void read_centerpiece_names_from_protocol( void );
      void read_aprofile_data_from_aprofile( void );
      void read_centerpiece_params( int );

      bool readProtocolCells_auto ( QXmlStreamReader& );
      bool readProtocolAProfile_auto ( QXmlStreamReader& );
      bool readAProfileBasicParms_auto ( QXmlStreamReader& );

      static bool bool_flag( const QString );
      
      void update_autoflow_record_atEditData( US_DB2*, QString& );
      int  create_autoflowAnalysis_record( US_DB2*, QString&,  QString& );
      void create_autoflowAnalysisStages_record( US_DB2*, int ); 
      QString compose_json( bool );

      void delete_autoflow_record ( void );

      void record_edit_status( QMap< QString, bool>, QString );
      
      bool isSet_to_analyse( QString, QString  );
      bool isSet_to_analyse_triple( QString, QString  );
      bool isSet_to_edit_triple( QString, QString );
      bool isSet_edit_info_for_channel( QString, QString );
      void set_data_over_lamda();
      void xaxis_wavl_wgts_on( bool );
      
   private slots:         
      void load              ( void );
      void load_auto         ( QMap < QString, QString > & );
      void load_manual_auto  ( void );
      
      void process_optics_auto ( void );
      
      QMap< QString, QString> read_autoflow_record( int );
      int read_autoflow_stages_record( int );
      void revert_autoflow_stages_record( int );
      
      bool isSaved_auto( void );
      
      void reset_editdata_panel ( void );
      
      void details           ( void );

      void new_triple        ( int  );
      void new_triple_auto   ( int  );
      
      void new_rpmval        ( int  );

      void focus_from        ( double );
      void focus_to          ( double );
      void focus             ( int, int );
      
      void exclude_range     ( void );
      void exclusion         ( void );
      void update_excludes   ( QList< int > );
      void finish_excludes   ( QList< int > );
      void cancel_excludes   ( void );
      void edit_scan         ( void );
      void update_scan       ( QList< QPointF > );
      void include           ( void );
      void apply_prior       ( void );
      void prior_equil       ( void );

      void sel_investigator  ( void );
      void set_fringe_tolerance( double ); 
      void set_meniscus      ( void );
      void set_airGap        ( void );
      void set_dataRange     ( void );
      void set_plateau       ( void );
      void mouse             ( const QwtDoublePoint& );

      void noise             ( void );
      void subtract_residuals( void );
      
      void remove_spikes     ( void );
      void remove_spikes_auto( void );

      void invert_values     ( void );
      
      void undo              ( void );
      void undo_auto         ( void );

      void floating          ( void );
      void write             ( void );
      void write_triple      ( void );
      void write_mwl         ( void );

      void setUnsetSaveBttn_abde( void );
  
      void manual_edit_auto  ( void );
      void write_auto        ( void );
      void write_triple_auto ( int );
      void write_mwl_auto    ( int );
  QString get_rawDataGUID( US_DB2*, QString, QString );
      
      void review_edits      ( void );
      
      void next_triple       ( void );
      
      void next_triple_auto  ( void );
      void prior_triple_auto ( void );

      bool all_edits_done    ( void );
      void update_disk_db    ( bool );
      void show_mwl_controls ( bool );
      void connect_mwl_ctrls ( bool );
      void ldelta_value      ( double );
      void lambda_start_value( int    );
      void lambda_end_value  ( int    );
      void reset_plot_lambdas( void   );
      void lambda_plot_value ( int    );
      void lselect_range_on  ( bool   );
      void lselect_custom_on ( bool   );
      void xaxis_radius_on   ( bool   );
      void xaxis_waveln_on   ( bool   );
      void lambda_plot_prev  ( void   );
      void lambda_plot_next  ( void   );
      void lambda_custom_list( void   );
      void lambda_new_list   ( QVector< int > );
      void lambda_include_all( void   );
      void od_radius_limit   ( double );
      void progress_load     ( QString );
      int  write_xml_file    ( QString&, QString&, QString&, QString& );
      int  write_edit_db     ( US_DB2*,
                               QString&, QString&, QString&, QString& );
      int  index_data        ( int = -1 );
      int  index_data_auto   ( int, int = -1 );
      
      int  like_edit_files   ( QString, QStringList&, US_DB2* );
      int  apply_edits       ( US_DataIO::EditValues parameters );
      int  lambdas_by_cell   ( int = -1 );
      double radius_indexed  ( const double );
                             
      QString      indent       ( int );

      QString      html_header  ( const QString, const QString );
      QString      html_header_auto ( const QString, const QString, const int );
            
      QString      run_details  ( void );
      QString      run_details_auto  ( int );
      
      QString      scan_info    ( void );
      QString      scan_info_auto    ( int );
      
      QString      table_row    ( const QString, const QString );
      QString      table_row    ( const QString, const QString,
                                  const QString );
      QString      table_row    ( const QString, const QString,
                                  const QString, const QString,
                                  const QString );
      QString      table_row    ( const QString, const QString,
                                  const QString, const QString,
                                  const QString, const QString );

      void create_report     ( QString& );
      void create_report_auto( QString&, int );
      
      void view_report       ( void );

      void save_report       ( const QString, const QString, const int );
      void save_report_auto  ( const QString, const QString, const int, const int );
      
      void reset             ( void );
      void reset_triple      ( void );
      void reset_outData     ( void );
      void close_edit        ( void );
      void pass_values       ( void );

      void update_triple_edit_params (  QMap < QString, QStringList > & );
      
      void help              ( void )
      { showHelp.show_help( "manual/us_edit.html" ); };
 
 signals:
      void data_loaded( void );
      void edit_complete_auto( QMap< QString, QString> & );
      void back_to_initAutoflow( void );

      void pass_edit_params( QMap< QString, QStringList> & );
      
      void process_next_optics( void );
};
#endif

