//! \file us_xpn_viewer_gui.h
#ifndef US_XPN_VIEWERG_H
#define US_XPN_VIEWERG_H

#include <QtGui>
#include <qwt_dial.h>
#include <qpainter.h>
#include <qwt_dial_needle.h>
#include <qwt_round_scale_draw.h>
#include <qwt_wheel.h>
#include <qwt_thermo.h>
#include <qwt_scale_engine.h>
#include <qwt_transform.h>
#include <qwt_color_map.h>

#include "us_extern.h"
#include "us_xpn_data.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_plot.h"
#include "us_dataIO.h"
//#include "us_license_t.h"
//#include "us_license.h"
#include "us_xpn_run_raw.h"
#include "us_xpn_run_auc.h"

#include "us_link_ssl.h"
#include "us_sleep.h"

class SpeedoMeter: public QwtDial
{

  Q_OBJECT
 
public:
    SpeedoMeter( QWidget *parent = NULL );

    void setLabel( const QString & );
    QString label() const;

protected:
    virtual void drawScaleContents( QPainter *painter,
        const QPointF &center, double radius ) const;

private:
    QString d_label;
};



class DialBox: public QWidget
{
  Q_OBJECT
 public:
  DialBox( QWidget *parent );
  void setSpeed( double );
  
 private Q_SLOTS:
    void setNum( double v );
    
 private:
    SpeedoMeter *createDial( void ) const;
    SpeedoMeter *d_dial;
    QLabel  *d_label;
};




class WheelBox: public QWidget
{
    Q_OBJECT
public:
    WheelBox( Qt::Orientation, double, double, QString, bool, QWidget *parent = NULL );
    void setTemp( double, QString);
    
private Q_SLOTS:
  void setNum( double, QString );

private:
  QWidget *createBox( Qt::Orientation, double minv, double maxv, bool log );
private:
    QwtThermo *d_thermo;
    QLabel *d_label;
};


class US_XpnDataViewer : public US_Widgets
{
  Q_OBJECT

  public:

     //! \brief  Generic constructor for the US_XpnDataViewer() program.
     US_XpnDataViewer();

     // New constructor for automated read/upload/update
     US_XpnDataViewer(QString auto_mode);

     void   reset_liveupdate_panel_public ( void );
     
  private:
     bool auto_mode_bool;
     bool inExport;

     int autoflowID_passed;
     int autoflowStatusID;
     
     QVector< US_DataIO::RawData >   allData;    //!< All AUC raw data

     QStringList opsys_auto;
     int optndx_auto;
     
     QVector< double > r_radii;     //!< Loaded radii
     QVector< int >    lambdas;     //!< Loaded wavelengths

     QList< int >      excludes;    //!< List of scans to exclude

     QList< QColor >   mcolors;     //!< Map colors for scan curves

     QVector<double> rpm_data;
     QVector<double> temp_data;
     QVector<double> time_data;

     Link *          link;
     
     SpeedoMeter*   rpm_speed;
     DialBox*       rpm_box;
     WheelBox*      temperature_box;
     WheelBox*      vacuum_box;

     QString        temp_label;
     QString        vacuum_label;
     
     US_XpnData*    xpn_data;       //!< Raw XPN (.postres) data loaded

     QStringList    runInfo;        //!< List of run information strings
     QStringList    xpn_fnames;     //!< List of names of files loaded
     QStringList    cellchans;      //!< List of cell/channel values present
     QStringList    triples;        //!< List of triple values present

     US_Help        showHelp;
     US_PlotPicker* picker;

     US_PlotPicker* picker_temp;
     US_PlotPicker* picker_rpm;

     QLabel*        lb_pltrec;
     
     QLineEdit*     le_elapsed;
     QLineEdit*     le_remaining;
     QLineEdit*     le_running;
     
     QLineEdit*     le_dir;
     QLineEdit*     le_dbhost;
     QComboBox*     cb_optima;           // New
     QStringList    sl_optimas;          // New
     QLineEdit*     le_optima_connected; //New
     
     US_LineEdit_RE*     le_runID;
     QLineEdit*     le_lrange;
     QLineEdit*     le_status;
     QLineEdit*     le_colmap;

     QComboBox*     cb_cellchn;
     QComboBox*     cb_rstart;
     QComboBox*     cb_rend;
     QComboBox*     cb_pltrec;
     QComboBox*     cb_optsys;

     QwtCounter*    ct_recavg;
     QwtCounter*    ct_rinterv;
     QwtCounter*    ct_from;
     QwtCounter*    ct_to;

     QCheckBox*     ck_autoscy;
     QCheckBox*     ck_autorld;

     QPushButton*   pb_loadXpn;
     QPushButton*   pb_loadAUC;
     QPushButton*   pb_reset;
     QPushButton*   pb_details;
     QPushButton*   pb_prev;
     QPushButton*   pb_next;
     QPushButton*   pb_exclude;
     QPushButton*   pb_include;
     QPushButton*   pb_plot2d;
     QPushButton*   pb_movie2d;
     QPushButton*   pb_saveauc;
     QPushButton*   pb_showtmst;
     QPushButton*   pb_reload;
     QPushButton*   pb_colmap;

     QLineEdit*     le_stage;
     QPushButton*   pb_skip_stage;
     QPushButton*   pb_stop;

     QLineEdit*     le_totscans;
     QLineEdit*     le_omega2T;

     QwtPlot*       data_plot;
     QwtPlotGrid*   grid;
     US_Plot*       plot;

     QwtPlot*       data_plot_temp;
     QwtPlotGrid*   grid_temp;
     US_Plot*       plot_temp;
     
     QwtPlot*       data_plot_rpm;
     QwtPlotGrid*   grid_rpm;
     US_Plot*       plot_rpm;

     QwtPlotCurve* curv_rpm;
     QwtPlotCurve* curv_temp;
     
     QString        currentDir;
     QString        runID;
     QString        runType;
     QString        prectype;
     QString        ptype_mw;
     QString        ptype_tr;
     QString        xpnhost;
     QString        xpnport;

     QString        xpnmsgPort;
     
     QString        xpndesc;
     QString        xpnname;
     QString        xpnuser;
     QString        xpnpasw;
     QMap<QString, QString> currentInstrument;      //New
 
     QList< QMap<QString, QString> > instruments;   //New

     int            nscan;
     int            ncell;
     int            nchan;
     int            ncellch;
     int            ntriple;
     int            nlambda;
     int            npoint;
     int            ntpoint;
     int            navgrec;
     int            dbg_level;
     int            ccx;
     int            lmb_start;
     int            lmb_end;
     int            recx;
     int            lmbxs;
     int            lmbxe;
     int            radxs;
     int            radxe;
     int            klambda;
     int            kradii;
     int            kscan;
     int            krecs;
     int            kpoint;
     int            ktpoint;
     int            trpxs;
     int            k3dscan;
     int            k3dlamb;
     int            k3drads;
     int            k3dsize;
     int            rlt_id;
     int            rlt_dlay;
     int            mcknt;

     int   counter_mins;

     bool           have_rngs;
     bool           isMWL;
     bool           isRaw;
     bool           haveData;
     bool           haveTmst;
     bool           in_reload;

     bool           in_reload_auto;
     bool           finishing_live_update;
     
     bool           in_reload_data_init;
     bool           in_reload_all_data;
     bool           in_reload_all_data_set_gui;
     bool           in_reload_check_sysdata;
     bool           in_reload_end_processes;
     bool           in_reload_end_process_all_data_avail;
     
     double         last_xmin;
     double         last_xmax;
     double         last_ymin;
     double         last_ymax;
     double         rad_start;
     double         rad_end;

     QTimer*        timer_data_init;
     QTimer*        timer_data_reload;
     QTimer*        timer_all_data_avail;
     QTimer*        timer_check_sysdata;

     QTimer*        timer_end_processes;
     QTimer*        timer_end_process_all_data_avail;

     QElapsedTimer*  elapsed_timer;
     
     QTimer*        timer_sys_thread;
     QThread*       sys_thread;
     
     QMessageBox*   msg_data_avail;
     QString        RunID_to_retrieve;
     QString        ExpID_to_use;
     QString        ProtocolName;
     QString        RunName;
     
     QString        CellChNumber;
     QString        TripleNumber;

     QMap< QString, QString > CellChNumber_map;
     QMap< QString, QString > TripleNumber_map;

     bool           combinedOptics;
     
     QString        OptimaName;          //New
     QString        TotalDuration;       //New
     QString        invID_passed;
     QString        correctRadii;
     QString        expAborted;
     QString        runID_passed;

     QMap < QString, QString > details_at_live_update; 

     bool           experimentAborted;
     bool           experimentAborted_remotely;
     bool           opticsFailed;
     QString        opticsFailed_type;
  
     int            ElapsedTimeOffset;

     void           timeToList( int&, QList< int >& );
	 
  private slots:
     void   reset          ( void );
     void   reset_auto     ( void );
     void   load_xpn_raw   ( void );

     //Sys_server
     void   stop_optima( void );
     void   skip_optima_stage( void );
  void   record_live_update_status( QString, QString);
  
     //QMap <QString, QString> returnSelectedInstrument( US_DB2* = 0 );
     bool   load_xpn_raw_auto   ( void );
     void   check_for_data ( QMap < QString, QString > & );
     void   check_for_sysdata ( void );

     void   reset_liveupdate_panel ( void );
     
     //void   retrieve_xpn_raw_auto ( QString & );
     void   retrieve_xpn_raw_auto ( void );

     void   end_processes ( void );
     void   end_process_all_data_avail ( void );
     
     void   reloadData_auto     ( void );
  int    CheckExpComplete_auto( QString &, bool & );
     void   update_autoflow_runId_timeStarted( void );
     int    read_timeElapsed_offset( void );
     void   delete_autoflow_record( void );
     void   updateautoflow_record_atLiveUpdate( void );

     int    read_autoflow_stages_record( int );
     void   revert_autoflow_stages_record( int );

     //void   expStatFive( void );
     //void   expStatThree( void );
     
     void   load_auc_xpn   ( void );
     void   plot_current   ( void );
     void   plot_titles    ( void );
     void   plot_all       ( void );
     void   enableControls ( void );
     void   runDetails     ( void );
     
     void   export_auc     ( void );
     void   export_auc_auto( bool& );

     void   changeRadius   ( void );
     void   changeCellCh   ( void );
     void   changeRecord   ( void );

     void   changeOptics   ( void );
     void   changeOptics_auto   ( void );
     
     
     void   changeReload   ( void );
     void   changeInterval ( void );
     void   resetAll       ( void );
     void   prevPlot       ( void );
     void   nextPlot       ( void );
     void   compute_ranges ( void );
     void   connect_ranges ( bool );
     void   exclude_from   ( double );
     void   exclude_to     ( double );
     void   exclude_scans  ( void );
     void   include_scans  ( void );
     int    dvec_index     ( QVector< double >&, const double );
     void   showTimeState  ( void );
     void   status_report  ( QString );
     void   reloadData     ( void );
     void   timerEvent     ( QTimerEvent *e );
     void   selectColorMap ( void );
     void   correct_radii  ( void );
     void   currentRectf   ( QRectF );
     void   help           ( void )
     { showHelp.show_help( "xpn_viewer.html" ); };

     void read_optima_machines( US_DB2* = 0 );         //New 
     void changeOptima ( int );                        //New
     void selectOptimaByName_auto( QString );          //New
     void test_optima_connection( void );              //New
     bool check_sysdata_connection( );                 //New
     //bool check_sysdata_connection( Link* );            //New
     
   signals:
     //void experiment_complete_auto( QString &, QString &, QString &, QString & );
     void experiment_complete_auto( QMap < QString, QString > &);
     
     void return_to_experiment( QString & );
     void close_program( void );
     void liveupdate_processes_stopped( void );
     void aborted_back_to_initAutoflow( void );
};
#endif

