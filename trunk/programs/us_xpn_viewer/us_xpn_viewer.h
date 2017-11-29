//! \file us_xpn_viewer.h
#ifndef US_XPN_VIEWER_H
#define US_XPN_VIEWER_H

#include <QtGui>

#include "us_extern.h"
#include "us_xpn_data.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_plot.h"
#include "us_dataIO.h"

class US_XpnDataViewer : public US_Widgets
{
  Q_OBJECT

  public:

     //! \brief  Generic constructor for the US_XpnDataViewer() program.
     US_XpnDataViewer();

  private:
     QVector< US_DataIO::RawData >   allData;    //!< All AUC raw data

     QVector< double > r_radii;     //!< Loaded radii
     QVector< int >    lambdas;     //!< Loaded wavelengths

     QList< int >      excludes;    //!< List of scans to exclude

     QList< QColor >   mcolors;     //!< Map colors for scan curves

     US_XpnData*    xpn_data;       //!< Raw XPN (.postres) data loaded

     QStringList    runInfo;        //!< List of run information strings
     QStringList    xpn_fnames;     //!< List of names of files loaded
     QStringList    cellchans;      //!< List of cell/channel values present
     QStringList    triples;        //!< List of triple values present

     US_Help        showHelp;
     US_PlotPicker* picker;

     QLabel*        lb_pltrec;

     QLineEdit*     le_dir;
     QLineEdit*     le_dbhost;
     QLineEdit*     le_runID;
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

     QwtPlot*       data_plot;
     QwtPlotGrid*   grid;
     US_Plot*       plot;

     QString        currentDir;
     QString        runID;
     QString        runType;
     QString        prectype;
     QString        ptype_mw;
     QString        ptype_tr;
     QString        xpnhost;
     QString        xpnport;
     QString        xpndesc;
     QString        xpnname;
     QString        xpnuser;
     QString        xpnpasw;

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

     bool           have_rngs;
     bool           isMWL;
     bool           isRaw;
     bool           haveData;
     bool           haveTmst;
     bool           in_reload;

     double         last_xmin;
     double         last_xmax;
     double         last_ymin;
     double         last_ymax;
     double         rad_start;
     double         rad_end;

  private slots:
     void   reset          ( void );
     void   load_xpn_raw   ( void );
     void   load_auc_xpn   ( void );
     void   plot_current   ( void );
     void   plot_titles    ( void );
     void   plot_all       ( void );
     void   enableControls ( void );
     void   runDetails     ( void );
     void   export_auc     ( void );
     void   changeRadius   ( void );
     void   changeCellCh   ( void );
     void   changeRecord   ( void );
     void   changeOptics   ( void );
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
};
#endif

