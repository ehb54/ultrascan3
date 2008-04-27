#ifndef US_GA_GRIDCONTROL_H
#define US_GA_GRIDCONTROL_H

#include "us.h"
#include "us_util.h"
#include "us_mfem.h"
#include "us_editor.h"
#include "us_sa2dbase.h"
#include "us_ga_initialize.h"
#include <list>
#include <qwt_counter.h>
#include <qwt_plot.h>
#include <qwt_scldiv.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtextedit.h>
#include <qlistbox.h>
#include <qcheckbox.h>
#include <vector>
#include <iterator>

struct ga_data
{
    unsigned int genes, demes, generations, crossover, initial_solutes;
    unsigned int largest_oligomer;
    QString largest_oligomer_string;
    float mw_min, mw_max, ff0_min, ff0_max;
    unsigned int mutation, plague, elitism;
    unsigned int random_seed, migration_rate, monte_carlo;
    bool fit_meniscus;
    float regularization, meniscus_range;
    vector <struct bucket> solute;
    QString analysis_type;
};

class US_EXTERN US_Ga_GridControl : public QDialog
{
	Q_OBJECT
	
	public:
	
		US_Ga_GridControl(struct ga_data *, QWidget *p = 0, const char *name = 0);
		~US_Ga_GridControl();
		struct ga_data *GA_Params;
		US_Config *USglobal;	 /*!< A US_Config reference. */
		float ff0_max, ff0_min;
		QTextEdit *textwindow;
		QString id;
		bool minmax; //min = false, max = true
		unsigned int current_solute;
		vector <float> sval;
		vector <float> frequency;
		vector <float> ff0;
		vector <struct line> limits;
		list <Solute> distro_solute;
		
	private:
	
		int distro_type; // 0 = vhw, 1 = cofs, 2 = ASTFEM-FE, 3 = SA2D
		float k_range, s_range;
		QCheckBox  *cb_meniscus;
		QLabel *lbl_info1;
		QLabel *lbl_info2;
		QLabel *lbl_genes;
		QLabel *lbl_demes;
		QLabel *lbl_generations;
		QLabel *lbl_crossover;
		QLabel *lbl_initial_solutes;
		QLabel *lbl_mutation;
		QLabel *lbl_plague;
		QLabel *lbl_elitism;
		QLabel *lbl_random_seed;
		QLabel *lbl_regularization;
		QLabel *lbl_status;
		QLabel *lbl_ff0_min;
		QLabel *lbl_ff0_max;
		QLabel *lbl_meniscus_range;
		QwtCounter *cnt_genes;
		QwtCounter *cnt_demes;
		QwtCounter *cnt_generations;
		QwtCounter *cnt_crossover;
		QwtCounter *cnt_initial_solutes;
		QwtCounter *cnt_mutation;
		QwtCounter *cnt_plague;
		QwtCounter *cnt_elitism;
		QwtCounter *cnt_random_seed;
		QwtCounter *cnt_regularization;
		QwtCounter *cnt_ff0_min;
		QwtCounter *cnt_ff0_max;
		QwtCounter *cnt_meniscus_range;
		QwtPlot *plot;
		QPushButton *pb_help;
		QPushButton *pb_load_distro;
		QPushButton *pb_reset_peaks;
		QPushButton *pb_assign_peaks;
		QPushButton *pb_close;
		QPushButton *pb_reject;
		QListBox *lb_solutes;

	private slots:
	
		void update_k_range(double);
		void update_s_range(double);
		void update_genes(double);
		void update_demes(double);
		void update_generations(double);
		void update_crossover(double);
		void update_initial_solutes(double);
		void update_mutation(double);
		void update_plague(double);
		void update_elitism(double);
		void update_random_seed(double);
		void update_regularization(double);
		void update_ff0_max(double);
		void update_ff0_min(double);
		void update_meniscus_range(double);
		void set_meniscus();
		void load_distro();
		void calc_distro();
		void reset_peaks();
		void assign_peaks();
		void help();
		void reject();
		void accept();
		void edit_solute(int);
		void setup_GUI();
		void closeEvent(QCloseEvent *);
		void getMouseReleased(const QMouseEvent &);
		void getMousePressed(const QMouseEvent &);

};

#endif

