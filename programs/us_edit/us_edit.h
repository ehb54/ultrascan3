#ifndef US_EDIT_H
#define US_EDIT_H

#include <QtGui>

#include "qwt_plot_marker.h"

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_plot.h"
#include "us_dataIO.h"
#include "us_db2.h"
#include "us_mwl_data.h"

class US_Edit : public US_Widgets
{
	Q_OBJECT

	public:
		US_Edit();

	private:

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
      double             invert;
      double             plateau;

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

      QLineEdit*         le_investigator;
      QLineEdit*         le_info;
      QLineEdit*         le_meniscus;
      QLineEdit*         le_airGap;
      QLineEdit*         le_dataRange;
      QLineEdit*         le_plateau;
      QLineEdit*         le_baseline;
      QLineEdit*         le_edtrsp;
                        
      QPushButton*       pb_details;
      QPushButton*       pb_exclude;
      QPushButton*       pb_excludeRange;
      QPushButton*       pb_exclusion;
      QPushButton*       pb_include;
      QPushButton*       pb_edit1;
      QPushButton*       pb_meniscus;
      QPushButton*       pb_airGap;
      QPushButton*       pb_dataRange;
      QPushButton*       pb_plateau;
      QPushButton*       pb_noise;
      QPushButton*       pb_spikes;
      QPushButton*       pb_invert;
      QPushButton*       pb_write;
      QPushButton*       pb_writemwl;
      QPushButton*       pb_residuals;
      QPushButton*       pb_priorEdits;
      QPushButton*       pb_reviewep;
      QPushButton*       pb_nexttrip;
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

      QGridLayout*       lo_lrange;
      QGridLayout*       lo_custom;
      QGridLayout*       lo_radius;
      QGridLayout*       lo_waveln;

      bool               isMwl;
      bool               xaxis_radius;
      bool               lsel_range;

      int                dlambda;
      int                slambda;
      int                elambda;
      int                plotrec;
      int                plotndx;
      int                nwaveln;
      int                nwavelo;
      int                nrpoint;
      int                ncelchn;
      int                ntriple;

      double             odlimit;

      US_DB2*            dbP;

      QVector< double >  expd_radii;
      QVector< int >     expi_wvlns;
      QVector< int >     rawi_wvlns;

      QVector< QVector< double > >  rdata;

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
                          
	private slots:         
      void load              ( void );
      void details           ( void );
      void new_triple        ( int  );
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
      void invert_values     ( void );
      void undo              ( void );
      void floating          ( void );
      void write             ( void );
      void write_triple      ( void );
      void write_mwl         ( void );
      void review_edits      ( void );
      void next_triple       ( void );
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
      void lambda_plot_prev  (        );
      void lambda_plot_next  (        );
      void lambda_custom_list(        );
      void lambda_new_list   ( QVector< int > );
      void lambda_include_all(        );
      void od_radius_limit   ( double );
      void progress_load     ( QString );
      int  write_xml_file    ( QString&, QString&, QString&, QString& );
      int  write_edit_db     ( US_DB2*,
                               QString&, QString&, QString&, QString& );
      int  index_data        ( int = -1 );
      int  like_edit_files   ( QString, QStringList&, US_DB2* );
      int  apply_edits       ( US_DataIO::EditValues parameters );
                             
      void reset             ( void );
      void reset_triple      ( void );
      void help              ( void )
      { showHelp.show_help( "manual/us_edit.html" ); };
};
#endif

