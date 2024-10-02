//! \file us_mwlr_viewer.h
#ifndef US_MWLR_VIEWER_H
#define US_MWLR_VIEWER_H

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_plot.h"
#include "us_mwl_data.h"
#include "us_plot3d.h"
#include "us_mwl_pltctrl.h"
#include "us_dataIO.h"

class US_MwlRawViewer : public US_Widgets
{
  Q_OBJECT

  public:

     //! \brief  Generic constructor for the US_MwlRawViewer() program.
     US_MwlRawViewer();

  private:
     QPointer< US_MwlPlotControl >   p3d_ctld;   //!< Pointer to 3D control
     QPointer< US_Plot3D >           p3d_pltw;   //!< Pointer to 3D plot window

     QVector< double >               curr_adata; //!< Current averaged data

     QVector< QVector< double > >    curr_cdata; //!< Current avg. comp. data
     QVector< QVector< double > >    prev_cdata; //!< Previous avg. comp. data

     QVector< QVector3D >            xyzdat;     //!< Current 3D data vector
     QVector< QVector< QVector3D > > xyzdats;    //!< All 3D plot data vectors
     QVector< double >               yvals3d;    //!< Y values for 3D plot

     QVector< US_DataIO::RawData >   allData;    //!< All AUC raw data

     QVector< double > pltxvals;    //!< Current plot's X (wvl/rad) values
     QVector< double > radii;       //!< Loaded radii
     QVector< int >    lambdas;     //!< Loaded wavelengths
     QVector< int >    curr_recxs;  //!< Current avg. component rec. indexes
     QVector< int >    prev_recxs;  //!< Previous avg. component rec. indexes

     QList< int >      excludes;    //!< List of scans to exclude

     US_MwlData     mwl_data;       //!< Raw MWL (.mwrs) data loaded

     QStringList    mwl_fnames;     //!< List of names of files loaded
     QStringList    cellchans;      //!< List of cell/channel values present

     US_Help        showHelp;
     US_PlotPicker* picker;

     QLabel*        lb_pltrec;

     QLineEdit*     le_runID;
     QLineEdit*     le_dir;
     QLineEdit*     le_status;

     QComboBox*     cb_cellchn;
     QComboBox*     cb_rstart;
     QComboBox*     cb_rend;
     QComboBox*     cb_lstart;
     QComboBox*     cb_lend;
     QComboBox*     cb_pltrec;

     QwtCounter*    ct_recavg;
     QwtCounter*    ct_from;
     QwtCounter*    ct_to;

     QCheckBox*     ck_xwavlen;
     QCheckBox*     ck_hcolorc;

     QPushButton*   pb_loadMwl;
     QPushButton*   pb_loadAUC;
     QPushButton*   pb_reset;
     QPushButton*   pb_details;
     QPushButton*   pb_prev;
     QPushButton*   pb_next;
     QPushButton*   pb_exclude;
     QPushButton*   pb_include;
     QPushButton*   pb_plot2d;
     QPushButton*   pb_movie2d;
     QPushButton*   pb_plot3d;
     QPushButton*   pb_movie3d;
     QPushButton*   pb_svplot;
     QPushButton*   pb_svmovie;

     QwtPlot*       data_plot;
     QwtPlotGrid*   grid;

     QString        currentDir;
     QString        runID;

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

     bool           is_wrecs;
     bool           have_rngs;

     double         last_xmin;
     double         last_xmax;
     double         last_ymin;
     double         last_ymax;
     double         rad_start;
     double         rad_end;

  private slots:
     void   reset          ( void );
     void   load_mwl_raw   ( void );
     void   load_auc_mwl   ( void );
     void   plot_current   ( void );
     void   plot_titles    ( void );
     void   plot_all       ( void );
     void   build_cmp_data ( void );
     void   build_avg_data ( void );
     void   build_rec_data ( const int, QVector< double >& );
     void   enableControls ( void );
     void   runDetails     ( void );
     void   changeCellCh   ( void );
     void   changeRadius   ( void );
     void   changeLambda   ( void );
     void   changeAverage  ( void );
     void   changeRecord   ( void );
     void   changeRectype  ( bool );
     void   show_2d_movie  ( void );
     void   plot_3d        ( void );
     void   show_3d_movie  ( void );
     void   save_plot      ( void );
     void   save_movie     ( void );
     void   save_2d_movie  ( void );
     void   save_3d_movie  ( void );
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
     int    build_xyz_data ( QVector< QVector3D >&, int = -1 );
     void   p3dctrl_closed ( void );
     int    live_scan      ( int* = 0, int* = 0, int* = 0 );
     void   help           ( void )
     { showHelp.show_help( "mwlr_viewer.html" ); };
};
#endif

