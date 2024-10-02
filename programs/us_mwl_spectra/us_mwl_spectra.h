//! \file us_mwl_spectra.h
#ifndef US_MWL_SPECTRA_H
#define US_MWL_SPECTRA_H

#include "us_extern.h"
#include "us_mwls_pltctl.h"
#include "us_widgets.h"
#include "us_model_loader.h"
#include "us_help.h"
#include "us_plot.h"
#include "us_mwl_data.h"
#include "us_plot3d.h"
#include "us_model.h"

class US_MwlSpectra : public US_Widgets
{
  Q_OBJECT

  public:

     //! \brief  Generic constructor for the US_MwlSpectra() program.
     US_MwlSpectra();

  private:
     QPointer< US_MwlSPlotControl > p3d_ctld;    //!< Pointer to 3D control
     QPointer< US_Plot3D >          p3d_pltw;    //!< Pointer to 3D plot window
     QList< US_Model >              loadedmodels;      //!< model list
     QVector< QVector3D >           mdlxyz;      //!< Models 3D data vector
     QVector< QVector3D >           xyzdat;      //!< Normalized 3D data vector
     QVector< QVector3D >           p3dxyz;      //!< Plot-3D data vector
     QVector< QVector< double > >   concdat;     //!< All 2D plot data vectors
     QVector< double >              yvals3d;     //!< Y values for 3D plot
     int                            xtype;
     US_Disk_DB_Controls*           dkdb_cntrls; //!< Disk/DB controls

     QVector< double > pltxvals;    //!< Current plot's X (wvl) values
     QVector< double > pltyvals;    //!< Current plot's Y (conc) values
     QVector< double > sedcoes;     //!< Loaded s values (x 1e+13)
     QVector< int >    lambdas;     //!< Loaded wavelengths

     QStringList    mdescs;         //!< List of descriptions of models loaded for current
     QStringList    pfilts;         //!< List of prefilter names

     US_Help        showHelp;
     US_PlotPicker* picker;

     QLabel*        lb_pltrec;

     QLineEdit*     le_status;

     QComboBox*     cb_sstart;
     QComboBox*     cb_send;
     QComboBox*     cb_lstart;
     QComboBox*     cb_lend;
     QComboBox*     cb_pltrec;

     QCheckBox*     ck_srngsum;

     QwtCounter*    ct_smooth;
     QwtCounter*    ct_delay;

     QPushButton*   pb_prefilt;
     QPushButton*   pb_loaddis;
     QPushButton*   pb_reset;
     QPushButton*   pb_details;
     QPushButton*   pb_prev;
     QPushButton*   pb_next;
     QPushButton*   pb_svdata;
     QPushButton*   pb_plot2d;
     QPushButton*   pb_movie2d;
     QPushButton*   pb_plot3d;
     QPushButton*   pb_svplot;
     QPushButton*   pb_svmovie;

     QRadioButton*  rb_pltsw;
     QRadioButton*  rb_pltDw;
     QRadioButton*  rb_pltMW;
     QRadioButton*  rb_pltff0;
     QRadioButton*  rb_pltvb;
     QRadioButton*  rb_pltMWl;


     QwtPlot*       data_plot;
     QwtPlotGrid*   grid;

     QString        currentDir;
     QString        runID;
     QString        m_tpart;
     QString        m_apart;
     QString        mfilter;
     QString        xaxis;
     QString        xlegend;
     int            nsedcos;
     int            nlambda;
     int            npoint;
     int            nipoint;
     int            nnpoint;
     int            ntpoint;
     int            nc_max;
     int            nsmooth;
     int            dbg_level;
     int            lmb_start;
     int            lmb_end;
     int            recx;
     int            sedxp;
     int            lmbxs;
     int            lmbxe;
     int            sedxs;
     int            sedxe;
     int            klambda;
     int            ksedcos;
     int            krecs;
     int            kpoint;
     int            ktpoint;

     bool           have_rngs;

     double         x_min;
     double         x_max;
     double         wl_min;
     double         wl_max;
     double         co_min;
     double         co_max;
     double         cn_max;
     double         last_xmin;
     double         last_xmax;
     double         last_ymin;
     double         last_ymax;
     double         sed_start;
     double         sed_end;
     double         sed_plot;

  private slots:
     void   reset          ( void );
     void   select_prefilt ( void );
     void   load_distro    ( void );
     void   load_distro    ( const US_Model, const QString );
     void   plot_current   ( void );
     void   plot_titles    ( void );
     void   plot_all       ( void );
     void   enableControls ( void );
     void   runDetails     ( void );
     void   changeSedcoeff ( void );
     void   changeLambda   ( void );
     void   changeSmooth   ( void );
     void   changeRecord   ( void );
     void   show_2d_movie  ( void );
     void   plot_3d        ( void );
     void   save_plot      ( void );
     void   save_movie     ( void );
     void   resetAll       ( void );
     void   prevPlot       ( void );
     void   nextPlot       ( void );
     void   compute_ranges ( void );
     void   connect_ranges ( bool );
     void   update_disk_db ( bool );
     int    dvec_index     ( QVector< double >&, const double );
     bool   dvirt_equal    ( const double, const double );
     void   p3dctrl_closed ( void );
     void   sum_check      ( void );
     void   save_data      ( void );
     void   bld_stats      ( double, double, double, QVector< int >&,
                             QVector< double >&, QVector< double >&,
                             QVector< double >&, QVector< double >& );
     void   final_stats    ( QVector< int >&,    QVector< double >&,
                             QVector< double >&, QVector< double >&,
                             QVector< double >& );
     void   changedPlotX   ( bool );
     double comp_value     ( const US_Model::SimulationComponent*, int );
     void   help           ( void )
     { showHelp.show_help( "mwl_spectra.html" ); };
};
#endif

