#ifndef US_EDIT_H
#define US_EDIT_H

#include <QtGui>

#include "qwt_plot_marker.h"

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_plot.h"
#include "us_dataIO2.h"

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

      QList< Edits >                 changed_points;
      US_DataIO2::RawData            data;
      QList< US_DataIO2::SpeedData > sData;
      QVector< US_DataIO2::RawData > allData;

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

      QList< int >       sd_offs;    // speed data offsets, ea. triple
      QList< int >       sd_knts;    // speed data counts, ea. triple
      QList< int >       includes;
      QList< double >    residuals;
      QVector< QString > editGUIDs;
      QVector< QString > editIDs;

      US_Help            showHelp;

      QIcon              check;

      QString            workingDir;
      QString            runID;
      QString            editID;
      QString            dataType;
      QString            expType;
      QStringList        files;
      QStringList        triples;
      QStringList        trip_rpms;
      QStringList        celchns;
      QStringList        rlist_wvlns;
      QStringList        plist_wvlns;
      QStringList        plist_radii;
                      
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
      QPushButton*       pb_baseline;
      QPushButton*       pb_noise;
      QPushButton*       pb_spikes;
      QPushButton*       pb_invert;
      QPushButton*       pb_write;
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
      QLabel*            lb_lexclf;
      QLabel*            lb_lexclt;
      QLabel*            lb_odlim;

      QwtCounter*        ct_ldelta;
      QwtCounter*        ct_odlim;

      QLineEdit*         le_lxrng;
      QLineEdit*         le_ltrng;

      QComboBox*         cb_lplot;
      QComboBox*         cb_lstart;
      QComboBox*         cb_lend;
      QComboBox*         cb_lexclf;
      QComboBox*         cb_lexclt;

      QPushButton*       pb_larrow;
      QPushButton*       pb_rarrow;
      QPushButton*       pb_excrng;
      QPushButton*       pb_incall;

      QRadioButton*      rb_radius;
      QRadioButton*      rb_waveln;

      QGridLayout*       lo_radius;
      QGridLayout*       lo_waveln;

      bool               isMwl;
      bool               xaxis_radius;

      double             dlambda;
      double             slambda;
      double             elambda;
      double             plotrec;
      double             odlimit;
      double             excllfr;
      double             excllto;

      int                plotndx;
      int                exclfrx;
      int                excltox;
      int                nwaveln;
      int                nwavelo;
      int                nrpoint;
      int                ncelchn;
      int                ntriple;

      QList< double >    wl_excludes;
      QList< double >    plot_radii;
      QList< int >       plot_wvlns;

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
      void set_baseline      ( void );
      void mouse             ( const QwtDoublePoint& );

      void noise             ( void );
      void subtract_residuals( void );
      void remove_spikes     ( void );
      void invert_values     ( void );
      void undo              ( void );
      void floating          ( void );
      void write             ( void );
      void write_triple      ( void );
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
      void xaxis_radius_on   ( bool   );
      void xaxis_waveln_on   ( bool   );
      void lambda_plot_value ( int    );
      void lambda_plot_prev  (        );
      void lambda_plot_next  (        );
      void lambda_excl_from  ( int    );
      void lambda_excl_to    ( int    );
      void lambda_excl_range (        );
      void lambda_include_all(        );
      void od_radius_limit   ( double );
                             
      void reset             ( void );
      void reset_triple      ( void );
      void help              ( void )
      { showHelp.show_help( "manual/us_edit.html" ); };
};
#endif
