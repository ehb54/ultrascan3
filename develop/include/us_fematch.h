#ifndef US_FEMATCH_H
#define US_FEMATCH_H

#include "us_dtactrl.h"
#include "us_extern.h"
#include "us_mfem.h"
#include "us_resplot.h"
#include "us_util.h"
#include "us_plotdata.h"
#include "us_3dsolutes.h"
#include "../3dplot/mesh2mainwindow.h"
#include "us_femglobal.h"
#include "us_astfem_rsa.h"

#include <qcheckbox.h>
#include <qwt_symbol.h>
#include <cerrno>
using namespace std;

class  US_EXTERN US_FeMatch_W : public Data_Control_W
{
	Q_OBJECT

	public:

		US_FeMatch_W(QWidget *p = 0, const char *name = 0);
		~US_FeMatch_W();

	private:

		int plot2;
		unsigned int monte_carlo_iterations;
		bool window_3d_flag;
		SA2d_control_variables sa2d_ctrl_vars;
		QString analysis_type;
		QLabel *lbl_s;
		QLabel *lbl_D;
		QLabel *lbl_C;
		QLabel *lbl_s2;
		QLabel *lbl_D2;
		QLabel *lbl_C2;
		QLabel *lbl_variance;
		QLabel *lbl_variance2;
		QLabel *lbl_MW_ff01;
		QLabel *lbl_component;
		QLineEdit *le_baseline;
		QLabel *lbl_MW_ff02;
		QPushButton *pb_fit;
		QPushButton *pb_loadModel;
		QwtCounter *cnt_component;
		US_ResidualPlot *resplot;
		US_Pixmap *pm;
		US_3d_Solutes *us_3d_solutes;
		float rmsd, baseline;
		int model;
		unsigned int components, current_component;
		vector <struct ModelSystem> msv;
		vector <struct mfem_data> simdata;
		struct SimulationParameters sp;

#ifdef WIN32
		    #pragma warning ( disable: 4251 )
#endif

		vector <double> ri_noise, ti_noise;
		vector <double> s_distribution;
		vector <double> D_distribution;
		vector <double> partial_concentration;
		vector <double> s20w;
		vector <double> D20w;
		vector <double> sigma;
		vector <double> delta;
		vector <double> mw;
		vector <double> f_f0;
		vector <double> fv;

#ifdef WIN32
		    #pragma warning ( default: 4251 )
#endif

		struct mfem_data residuals, fem_model;

	private slots:

		void setup_GUI();
		void update_component(double);
		void update_s(const QString &);
		void update_D(const QString &);
		void update_C(const QString &);
		void update_baseline(const QString &);
		void cofs_GUI();
		void enableButtons();
		void update_distribution();
		void clear_data(struct mfem_data *);
		void updateParameters();
		void fit();
		float calc_residuals();
		float calc_residuals_ra();
		void second_plot(int);
		void write_cofs();
		void write_res();
		void calc_distros();
		void load_model();
		void load_model(const QString &);
		void create_modelsystems();
		void update_labels();
		void clearDisplay();

// re-implemented Functions:

		void view();
		void help();
		void save();
		void second_plot();
};


#include <qthread.h>
#include <qwaitcondition.h>
#include <qmutex.h>

class fematch_thr_t : public QThread
{
 public:
  fematch_thr_t(int);
  void fematch_thr_setup(QProgressBar *,
			 struct mfem_data *,
			 struct mfem_data *,
			 struct mfem_data *,
			 struct mfem_initial *,
			 US_MovingFEM *,
			 struct runinfo *,
			 vector <double> *,
			 vector <double> *,
			 unsigned int ,
			 unsigned int ,
			 vector <double> *,
			 double ,
			 unsigned int ,
			 unsigned int ,
			 unsigned int
			 );
  void fematch_thr_shutdown();
  void fematch_thr_wait();
  virtual void run();

 private:
  QProgressBar *progress;
  struct mfem_data *fem_data;
  struct mfem_data *experiment;
  struct mfem_data *residuals;
  struct mfem_initial *initCVector;
  US_MovingFEM *mfem;
  struct runinfo *run_inf;
  vector <double> *s_distribution;
  vector <double> *D_distribution;
  unsigned int selected_cell;
  unsigned int selected_lambda;
  vector <double> *partial_concentration;
  double bottom;
  unsigned int points;
  unsigned int j_start;
  unsigned int j_end;

  int thread;
  unsigned int i;
  unsigned int j;
  QMutex work_mutex;
  int work_to_do;
  QWaitCondition cond_work_to_do;
  int work_done;
  QWaitCondition cond_work_done;
  int work_to_do_waiters;
  int work_done_waiters;
};

#endif

