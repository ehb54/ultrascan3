//! \file us_cfa_viewer.h
#ifndef US_CFA_VIEWER_H
#define US_CFA_VIEWER_H

#include <QtGui>

#include "us_extern.h"
#include "us_cfa_data.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_plot.h"
#include "us_dataIO.h"

class US_CfaDataViewer : public US_Widgets
{
  Q_OBJECT

  public:

     //! \brief  Generic constructor for the US_CfaDataViewer() program.
     US_CfaDataViewer();

  private:
     QVector< US_DataIO::RawData >   allData;    //!< All AUC raw data

     QVector< double > radii;       //!< Loaded radii
     QVector< int >    lambdas;     //!< Loaded wavelengths

     QList< int >      excludes;    //!< List of scans to exclude

     US_CfaData*    cfa_data;       //!< Raw CFA (.sqlite) data loaded

     QStringList    cfa_fnames;     //!< List of names of files loaded
     QStringList    cellchans;      //!< List of cell/channel values present
     QStringList    triples;        //!< List of triple values present

     US_Help        showHelp;
     US_PlotPicker* picker;

     QLabel*        lb_pltrec;

     QLineEdit*     le_runID;
     QLineEdit*     le_dir;
     QLineEdit*     le_lrange;
     QLineEdit*     le_status;

     QComboBox*     cb_scntype;
     QComboBox*     cb_cellchn;
     QComboBox*     cb_rstart;
     QComboBox*     cb_rend;
     QComboBox*     cb_pltrec;

     QwtCounter*    ct_recavg;
     QwtCounter*    ct_from;
     QwtCounter*    ct_to;

     QCheckBox*     ck_autoscy;

     QPushButton*   pb_loadCfa;
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

     QwtPlot*       data_plot;
     QwtPlotGrid*   grid;

     QString        currentDir;
     QString        runID;
     QString        prectype;
     QString        ptype_mw;
     QString        ptype_tr;

     int            sctype;
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

     bool           have_rngs;
     bool           isMWL;
     bool           isRaw;
     bool           haveData;
     bool           haveTmst;

     double         last_xmin;
     double         last_xmax;
     double         last_ymin;
     double         last_ymax;
     double         rad_start;
     double         rad_end;

  private slots:
     void   reset          ( void );
     void   load_cfa_raw   ( void );
     void   load_auc_cfa   ( void );
     void   plot_current   ( void );
     void   plot_titles    ( void );
     void   plot_all       ( void );
     void   enableControls ( void );
     void   runDetails     ( void );
     void   export_auc     ( void );
     void   changeRadius   ( void );
     void   changeCellCh   ( void );
     void   changeRecord   ( void );
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
     void   help           ( void )
     { showHelp.show_help( "cfa_viewer.html" ); };
};
#endif

