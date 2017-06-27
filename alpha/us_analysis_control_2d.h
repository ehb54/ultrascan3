//! \file us_analysis_control_2d.h
#ifndef US_ANALYSIS_CTL_H
#define US_ANALYSIS_CTL_H
#include <QApplication>
#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_2dsa_process.h"
#include "us_worker_calcnorm.h"
#include "us_plot.h"
#include "us_help.h"
#include "us_plot.h"
#include "us_colorgradIO.h"
#include "us_spectrodata.h"
#include "qwt_plot_marker.h"
#include "qwt_plot_spectrogram.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_zoomer.h"
#include "qwt_plot_panner.h"
#include "qwt_scale_widget.h"
#include "qwt_scale_draw.h"
#include "qwt_color_map.h"
#include "us_show_norm.h"

#define PA_TMDIS_MS 0

#ifndef SS_DATASET
#define SS_DATASET US_SolveSim::DataSet
#endif
//---------------------------------------
//!< \brief Distribution structure
/*typedef struct distro_sys
{
   QList< S_Solute >   sk_distro;
   QList< S_Solute >   xy_distro;
   QList< S_Solute >   sk_distro_zp;
   QList< S_Solute >   xy_distro_zp;
   QwtLinearColorMap*  colormap;
   QString             run_name;
   QString             analys_name;
   QString             method;
   QString             cmapname;
   QString             editGUID;
   int                 distro_type;
   int                 plot_x;
   int                 plot_y;
   bool                monte_carlo;
} DisSys;

//! \brief Less-than function for sorting distributions
bool distro_lessthan( const S_Solute&, const S_Solute& );
class US_RemoveDistros : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! \brief US_RemoveDistros constructor
      //! \param adistros Pointer to model distributions list
      //! \param p        Pointer to the parent of this widget
      US_RemoveDistros( QList< DisSys >&, QWidget* p = 0 );

   private:
      QList< DisSys >& distros;    // Reference to model distributions list

      int           nd_orig;       // Number of distributions in original
      int           nd_removed;    // Number of total distributions removed
      int           nd_selected;   // Number of currently selected distributions

      QGridLayout*  mainLayout;    // 

      QStringList   mdesc_orig;    // List of descriptions from original
      QStringList   mdesc_list;    // List of descriptions now listed

      QListWidget*  lw_distrs;     // List widget of distributions

      QTextEdit*    te_status;     // Status text box

      QPushButton*  pb_restore;    // Restore button
      QPushButton*  pb_remove;     // Remove  button
      QPushButton*  pb_help;       // Help    button
      QPushButton*  pb_cancel;     // Cancel  button
      QPushButton*  pb_accept;     // Accept  button

   protected:
      US_Help       showHelp;

   private slots:

      void selectionsChanged( void );
      void remove           ( void );
      void restore          ( void );
      void accepted         ( void );
      void help             ( void )
      { showHelp.show_help( "pseudo3d_comb_rmv.html" ); };
};
*/
//! Class for displaying models in pseudo-3D
/*class US_show_norm : public US_Widgets
{
   Q_OBJECT

   public:
      US_show_norm();
      //( QList< SS_DATASET* >&, bool&, QWidget* p = 0 );
      // US_show_norm( QList< SS_DATASET* >&, bool&, QWidget* p = 0 );
   private:
      //QList< SS_DATASET* >&            dsets;
      enum attr_type { ATTR_S, ATTR_K, ATTR_W, ATTR_V, ATTR_D, ATTR_F };

      QLabel*       lb_plt_smin;
      QLabel*       lb_plt_smax;
      QLabel*       lb_plt_kmin;
      QLabel*       lb_plt_kmax;

      QTextEdit*    te_distr_info;

      QLineEdit*    le_cmap_name;
      QLineEdit*    le_prefilt;

      US_Help       showHelp;
 
      QwtCounter*   ct_resolu;
      QwtCounter*   ct_xreso;
      QwtCounter*   ct_yreso;
      QwtCounter*   ct_zfloor;
      QwtCounter*   ct_plt_kmin;     
      QwtCounter*   ct_plt_kmax;     
      QwtCounter*   ct_plt_smin;     
      QwtCounter*   ct_plt_smax;     
      QwtCounter*   ct_plt_dlay;     
      QwtCounter*   ct_curr_distr;

      QwtPlot*      data_plot;

      QwtPlotPicker* pick;

      QwtLinearColorMap* colormap;

      US_Disk_DB_Controls* dkdb_cntrls;

      QPushButton*  pb_pltall;
      QPushButton*  pb_stopplt;
      QPushButton*  pb_refresh;
      QPushButton*  pb_reset;
      QPushButton*  pb_prefilt;
      QPushButton*  pb_lddistr;
      QPushButton*  pb_ldcolor;
      QPushButton*  pb_help;
      QPushButton*  pb_close;
      QPushButton*  pb_rmvdist;

      QCheckBox*    ck_autosxy;
      QCheckBox*    ck_autoscz;
      QCheckBox*    ck_conloop;
      QCheckBox*    ck_plot_sk;
      QCheckBox*    ck_plot_wk;
      QCheckBox*    ck_plot_sv;
      QCheckBox*    ck_plot_wv;
      QCheckBox*    ck_zpcent;
      QCheckBox*    ck_savepl;
      QCheckBox*    ck_locsave;

      QRadioButton* rb_x_s;
      QRadioButton* rb_x_ff0;
      QRadioButton* rb_x_mw;
      QRadioButton* rb_x_vbar;
      QRadioButton* rb_x_D;
      QRadioButton* rb_x_f;
      QRadioButton* rb_y_s;
      QRadioButton* rb_y_ff0;
      QRadioButton* rb_y_mw;
      QRadioButton* rb_y_vbar;
      QRadioButton* rb_y_D;
      QRadioButton* rb_y_f;

      QButtonGroup* bg_x_axis;
      QButtonGroup* bg_y_axis;

      QList< DisSys > system;

      double        resolu;
      double        plt_smin;
      double        plt_smax;
      double        plt_kmin;
      double        plt_kmax;
      double        plt_zmin;
      double        plt_zmax;
      double        plt_zmin_zp;
      double        plt_zmax_zp;
      double        plt_zmin_co;
      double        plt_zmax_co;
      double        s_range;
      double        k_range;
      double        xreso;
      double        yreso;
      double        zfloor;

      int           curr_distr;
      int           init_solutes;
      int           mc_iters;
      int           patm_id;
      int           patm_dlay;
      int           dbg_level;
      int           plot_x;
      int           plot_y;

      bool          cnst_vbar;
      bool          auto_sxy;
      bool          auto_scz;
      bool          cont_loop;
      bool          looping;
      bool          need_save;
      bool          runsel;
      bool          latest;
      bool          zpcent;

      QString       xa_title;
      QString       ya_title;
      QString       cmapname;
      QString       mfilter;

      QStringList   pfilts;

   private slots:

      void update_resolu(     double );
      void update_xreso(      double );
      void update_yreso(      double );
      void update_zfloor(     double );
      void update_curr_distr( double );
      void update_plot_smin(  double );
      void update_plot_smax(  double );
      void update_plot_kmin(  double );
      void update_plot_kmax(  double );
      void plot_data(      int );
      void plot_data(      void );
      void select_autosxy( void );
      void select_autoscz( void );
      void select_conloop( void );
      void update_disk_db( bool );
      void select_prefilt( void );
      void load_distro(    void );
      void load_distro(    US_Model, QString );
      void load_color(     void );
      void plotall(     void );
      void stop(        void );
      void reset(       void );
      void set_limits(  void );
      void sort_distro( QList< S_Solute >&, bool );
      void remove_distro( void );
      void select_x_axis( int  );
      void select_y_axis( int  );
      void build_xy_distro( void );
      //void show_norm (void );
      QString anno_title  ( int );
      QwtLinearColorMap* ColorMapCopy( QwtLinearColorMap* );

      void help       ( void )
      { showHelp.show_help( "show_norm.html" ); };

   protected:
      virtual void timerEvent( QTimerEvent *e );
};*/
//#endif

//--------------------------------------
//! \brief A class to provide a window with 2DSA analysis controls
 
class US_AnalysisControl2D : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! \brief US_AnalysisControl2D constructor
      //! \param dsets   Pointer to the experiment data
      //! \param loadDB  Flag for whether loads are from DB
      //! \param p       Pointer to the parent of this widget
      US_AnalysisControl2D( QList< SS_DATASET* >&, bool&, QWidget* p = 0 );
      enum attr_type { ATTR_S, ATTR_K, ATTR_W, ATTR_V, ATTR_D, ATTR_F };
      US_Model  model2 ;
      //void calculate_norms( US_Model& ) ;
      void set_comp_attr     ( US_Model::SimulationComponent&,
                             US_Solute&, int );

   public slots:
      void update_progress (  int  );
      void completed_process( int  );
      void progress_message(  QString, bool = true );
      void reset_steps(       int,     int );
      void norm_progress(     int  );
      void norm_complete( WorkerThreadCalcNorm* );

   private:
      QList< SS_DATASET* >&            dsets;
      US_Model                         cusmodel;

      bool&         loadDB;

      int           dbg_level;
      int           ncsteps;
      int           nctotal;
      int           grtype;
      int           baserss;
      int           memneed;
      int           normstep;
      int           kthrdr;
      int           nsolutes;

      QHBoxLayout*  mainLayout;
      QGridLayout*  controlsLayout;
      QGridLayout*  optimizeLayout;

      US_DataIO::EditedData*           edata;
      US_DataIO::RawData*              sdata;
      US_DataIO::RawData*              rdata;
      US_Model*                        model;
      US_Noise*                        ri_noise;
      US_Noise*                        ti_noise;
      QVector< double >                normvA;
      US_SimulationParameters*         sparms;
      QPointer< QTextEdit    >         mw_stattext;
      int*                             mw_baserss;
      QPointer< US_show_norm >         analcd1;

      QWidget*                         parentw;
      US_2dsaProcess*                  processor;

      QLabel*       lb_lolimitk;
      QLabel*       lb_uplimitk;
      QLabel*       lb_nstepsk;
      QLabel*       lb_constff0;

      QwtCounter*   ct_lolimits;
      QwtCounter*   ct_uplimits;
      QwtCounter*   ct_nstepss;
      QwtCounter*   ct_lolimitk;
      QwtCounter*   ct_uplimitk;
      QwtCounter*   ct_nstepsk;
      QwtCounter*   ct_thrdcnt;
      QwtCounter*   ct_menisrng;
      QwtCounter*   ct_menispts;
      QwtCounter*   ct_repetloc;
      QwtCounter*   ct_scfactor;
      QwtCounter*   ct_scfact2;
      QwtCounter*   ct_repetran;
      QwtCounter*   ct_stddevia;
      QwtCounter*   ct_coaldist;
      QwtCounter*   ct_nbrclips;
      QwtCounter*   ct_mciters;
      QwtCounter*   ct_iters;
      QwtCounter*   ct_tol;
      QwtCounter*   ct_constff0;

      QCheckBox*    ck_tinoise;
      QCheckBox*    ck_rinoise;
      QCheckBox*    ck_autoplt;
      QCheckBox*    ck_unifgr;
      QCheckBox*    ck_custgr;
      QCheckBox*    ck_menisc;
      QCheckBox*    ck_ranlgr;
      QCheckBox*    ck_soluco;
      QCheckBox*    ck_clipcs;
      QCheckBox*    ck_mcarlo;
      QCheckBox*    ck_iters;
      QCheckBox*    ck_varvbar;
      QCheckBox*    ck_norm;


      QLineEdit*    le_estmemory;
      QLineEdit*    le_iteration;
      QLineEdit*    le_oldvari;
      QLineEdit*    le_newvari;
      QLineEdit*    le_improve;
      QLineEdit*    le_gridreps;

      QTextEdit*    te_status;

      QPushButton*  pb_strtfit;
      QPushButton*  pb_stopfit;
      QPushButton*  pb_plot;
      QPushButton*  pb_save;
      QPushButton*  pb_ldmodel;

   protected:
      US_Help       showHelp;
      QProgressBar* b_progress;

   private slots:
      void optimize_options( void );
      void calculate_norms ( void );
      void uncheck_optimize( int  );
      void checkUniGrid (    bool );
      void checkCusGrid (    bool );
      void checkMeniscus(    bool );
      void checkIterate (    bool );
      void checkLocalUni(    bool );
      void checkRandLoc (    bool );
      void checkSoluCoal(    bool );
      void checkClipLow (    bool );
      void checkMonteCar(    bool );
      void checkRegular (    bool );
      void checkVaryVbar(    bool );
      void grid_change(      void );
      void slim_change(      void );
      void klim_change(      void );
      void kstep_change(     void );
      void start(            void );
      void stop_fit(         void );
      void plot(             void );
      void save(             void );
      void close_all(        void );
      void advanced(         void );
      void load_model(       void );
      int  memory_check(     void );
      //void show_norm   (     void );
      void help     ( void )
      { showHelp.show_help( "2dsa_analys.html" ); };
};
#endif

