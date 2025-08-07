//! \file us_adv_analysis_pc.h
#ifndef US_ADV_ANALYSIS_H
#define US_ADV_ANALYSIS_H

#include "us_extern.h"
#include "us_help.h"
#include "us_pcsa_process.h"
#include "us_plot.h"
#include "us_widgets_dialog.h"

#ifndef DbgLv
#define DbgLv(a) \
   if (dbg_level >= a) \
   qDebug()
#endif

//! \brief A class to provide a window for advanced analysis controls

class US_AdvAnalysisPc : public US_WidgetsDialog {
      Q_OBJECT

   public:
      //! \brief US_AdvAnalysisPc constructor
      //! \param p_mrs   Pointer to model records vector
      //! \param nth     Number of threads to use
      //! \param ds0     Pointer to first dataset record
      //! \param p       Pointer to the parent of this widget
      US_AdvAnalysisPc(QVector<US_ModelRecord> *, const int, US_SolveSim::DataSet *, QWidget *p = 0);

      //! \brief Return flag of advanced analysis state and possibly MC models
      //! \param p_mrecsmc  Pointer for return of MC model records if appropo
      //! \return           Flag with or'd state (1=new-bfm, 2=new-mrs, 4=mc)
      int advanced_results(QVector<US_ModelRecord> *);

   private:
      QVector<US_ModelRecord> *p_mrecs;
      QVector<US_ModelRecord> mrecs0;
      QVector<US_ModelRecord> mrecs;
      QVector<US_ModelRecord> mrecs_mc;

      US_ModelRecord mrec0;
      US_ModelRecord mrec;

      US_SolveSim::DataSet *dset0;

      US_DataIO::EditedData *edata;
      US_DataIO::EditedData wdata;
      US_DataIO::RawData sdata1;

      QVector<double> sigmas;

      US_Model model;

      QVector<US_SolveSim::DataSet> wkdsets;

      static const int msk_bfnew = 1;
      static const int msk_mrnew = 2;
      static const int msk_mcarl = 4;

      int nthr;
      int mciters;
      int kciters;
      int ksiters;
      int nmrecs;
      int ctype;
      int nisols;
      int ncsols;
      int dbg_level;

      bool bfm0_exists;
      bool mrs0_exists;
      bool bfm_new;
      bool mrs_new;
      bool mc_done;
      bool mc_running;

      QString store_dir;

      QHBoxLayout *mainLayout;
      QGridLayout *finmodelLayout;
      QGridLayout *mreclistLayout;

      QWidget *parentw;

      QLabel *lb_sigmpar1;
      QLabel *lb_sigmpar2;
      QLabel *lb_y_strpt;
      QLabel *lb_y_endpt;

      QLineEdit *le_x_lower;
      QLineEdit *le_x_upper;
      QLineEdit *le_y_lower;
      QLineEdit *le_y_upper;
      QLineEdit *le_sigmpar1;
      QLineEdit *le_sigmpar2;
      QLineEdit *le_y_strpt;
      QLineEdit *le_y_endpt;
      QLineEdit *le_mciters;
      QLineEdit *le_crpoints;

      QPushButton *pb_loadmrs;
      QPushButton *pb_storemrs;
      QPushButton *pb_loadbfm;
      QPushButton *pb_storebfm;
      QPushButton *pb_resetbfm;
      QPushButton *pb_resetmrs;
      QPushButton *pb_buildbfm;
      QPushButton *pb_mciters;
      QPushButton *pb_help;
      QPushButton *pb_cancel;
      QPushButton *pb_accept;

      QComboBox *cb_curvtype;

      QTextEdit *te_bfmstat;
      QTextEdit *te_mrecstat;

   protected:
      US_Help showHelp;
      QProgressBar *b_progress;

   private slots:
      void curvtypeChanged(int);
#if 0
      void slowerChanged   ( double );
      void supperChanged   ( double );
      void sipar1Changed   ( double );
      void sipar2Changed   ( double );
      void klowerChanged   ( double );
      void kupperChanged   ( double );
      void pointsChanged   ( double );
#endif
      void mciterChanged(double);
      void load_mrecs(void);
      void store_mrecs(void);
      void load_bfm(void);
      void store_bfm(void);
      void reset_bfm(void);
      void reset_mrecs(void);
      void build_bfm(void);
      void start_montecarlo(void);
      void set_gaussians(US_SolveSim::Simulation &);
      void apply_gaussians(void);
      void process_job(WorkerThreadPc *);
      void montecarlo_done(void);
      void under_construct(QString);
      void curve_isolutes(US_ModelRecord &);
      void bfm_model(void);
      void stat_mrecs(const QString, bool = false, int = 0);
      void stat_bfm(const QString, bool = false, int = 0);
      void show_stat(QTextEdit *, const QString, bool = false, int = 0);
      void set_fittings(QVector<US_ModelRecord> &);
      bool mrecs_required(const QString);
      bool bfm_incompat(const QString);

      void select(void);
      void cancel(void);
      void test_db_mrecs(void);

      void help(void) { showHelp.show_help("pcsa_advanced.html"); };
};
#endif
