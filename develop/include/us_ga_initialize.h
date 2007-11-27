#ifndef US_GA_INITIALIZE_H
#define US_GA_INITIALIZE_H

#include "us.h"
#include "us_util.h"
#include "us_math.h"
#include "us_mfem.h"
#include "us_pixmap.h"
#include "us_femglobal.h"
#include "us_editor.h"
#include "us_sa2dbase.h"
#include "us_montecarlostats.h"
#include "us_pseudo3d_thread.h"

#include <list>
#include <qwt_counter.h>
#include <qwt_plot.h>
#include <qwt_scldiv.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qlayout.h>
#include <qprogressbar.h>
#include <qtextedit.h>
#include <qlistbox.h>
#include <qcheckbox.h>
#include <vector>
#include <iterator>



struct vertex
{
	double x, y;
};

class bucket
{
	public:
		float s;
		float s_min;
		float s_max;
		float ff0;
		float ff0_max;
		float ff0_min;
		float conc;
		int status; // 0 = full-sized bucket, 
						// 1 = this bucket is reduced from overlap
						// 2 = newly added bucket
		bucket() {};
		~bucket() {};
		bool operator==(const bucket& objIn) 
		{
			return (s == objIn.s && s_min == objIn.s_min && s_max == objIn.s_max 
				&& ff0 == objIn.ff0 && ff0_min == objIn.ff0_min && ff0_max == objIn.ff0_max 
				&& conc == objIn.conc && status == objIn.status);
		}
		bool operator!=(const bucket& objIn)
		{
			return (s != objIn.s || s_min != objIn.s_min || s_max != objIn.s_max 
				|| ff0 != objIn.ff0 || ff0_min != objIn.ff0_min || ff0_max != objIn.ff0_max 
				|| conc != objIn.conc || status != objIn.status);
		}
		bool operator < (const bucket& objIn) const
		{
			if (s < objIn.s)
			{
				return (true);
			}
			else if (s == objIn.s && ff0 < objIn.ff0)
			{
				return(true);
			}
			else
			{
				return(false);
			}
		}
};

struct line
{
	unsigned int line1;
	unsigned int line2;
};

class US_EXTERN US_GA_Initialize : public QFrame
{
	Q_OBJECT
	
	public:
	
		US_GA_Initialize(QWidget *p = 0, const char *name = 0);
		~US_GA_Initialize();
		vector <struct bucket> GA_Solute;
		vector <QColor> gradient;
		US_Config *USglobal;	 /*!< A US_Config reference. */
		float ff0_max, ff0_min, resolution, plot_fmax, plot_fmin, plot_smax, plot_smin;
		unsigned int x_resolution, y_resolution, x_pixel, y_pixel;
		QTextEdit *textwindow;
		QString id, cell_info;
		bool minmax; //min = false, max = true
		bool monte_carlo;
		unsigned int current_solute, dimension;
		vector <struct line> limits;
		vector <struct SimulationComponent> component;
		list <class Solute> distro_solute;
		vector <vector <struct SimulationComponent> > MC_solute;
		unsigned int initial_solutes, monte_carlo_iterations;
		struct vertex p1, p2;
		bool zoom, autolimit;
		
	private:
	
		int distro_type; // 0 = vhw, 1 = cofs, 2 = ASTFEM-FE, 3 = SA2D
		float k_range, s_range;
		QCheckBox  *cb_meniscus;
		QLabel *lbl_info1;
		QLabel *lbl_info2;
		QLabel *lbl_info3;
		QLabel *lbl_initial_solutes;
		QLabel *lbl_status;
		QLabel *lbl_ff0_min;
		QLabel *lbl_ff0_max;
		QLabel *lbl_k_range;
		QLabel *lbl_s_range;
		QLabel *lbl_resolution;
		QLabel *lbl_meniscus_range;
		QLabel *lbl_x_resolution;
		QLabel *lbl_y_resolution;
		QLabel *lbl_x_pixel;
		QLabel *lbl_y_pixel;
		QLabel *lbl_autolimit;
		QLabel *lbl_plot_smin;
		QLabel *lbl_plot_smax;
		QLabel *lbl_plot_fmin;
		QLabel *lbl_plot_fmax;
		QwtCounter *cnt_initial_solutes;
		QwtCounter *cnt_ff0_min;
		QwtCounter *cnt_ff0_max;
		QwtCounter *cnt_plot_fmin;
		QwtCounter *cnt_plot_fmax;
		QwtCounter *cnt_plot_smin;
		QwtCounter *cnt_plot_smax;
		QwtCounter *cnt_k_range;
		QwtCounter *cnt_s_range;
		QwtCounter *cnt_resolution;
		QwtCounter *cnt_x_resolution;
		QwtCounter *cnt_y_resolution;
		QwtCounter *cnt_x_pixel;
		QwtCounter *cnt_y_pixel;
		QwtPlot *plot;
		QPushButton *pb_help;
		QPushButton *pb_color;
		QPushButton *pb_replot3d;
		QPushButton *pb_print;
		QPushButton *pb_load_distro;
		QPushButton *pb_reset_peaks;
		QPushButton *pb_assign_peaks;
		QPushButton *pb_close;
		QPushButton *pb_shrink;
		QPushButton *pb_draw;
		QPushButton *pb_save;
		QCheckBox *cb_1dim;
		QCheckBox *cb_2dim;
		QCheckBox *cb_3dim;
		QCheckBox *cb_autolimit;
		QListBox *lb_solutes;
		QProgressBar *progress;

	private slots:
	
		void update_k_range(double);
		void update_s_range(double);
		void update_resolution(double);
		void update_initial_solutes(double);
		void update_ff0_max(double);
		void update_ff0_min(double);
		void update_plot_smax(double);
		void update_plot_smin(double);
		void update_plot_fmax(double);
		void update_plot_fmin(double);
		void update_x_resolution(double);
		void update_y_resolution(double);
		void update_x_pixel(double);
		void update_y_pixel(double);
		void plot_1dim();
		void plot_2dim();
		void plot_3dim();
		void select_3dim();
		void select_1dim();
		void select_2dim();
		void select_autolimit();
		void load_distro();
		void load_color();
		void calc_distro();
		void reset_peaks_replot();
		void reset_peaks();
		void assign_peaks();
		void help();
		void save();
		void print();
		void draw();
		void plot_buckets();
		void shrink();
		void edit_solute(int);
		void highlight_solute(int);
		void highlight_solute(int, bool);
		void setup_GUI();
		void closeEvent(QCloseEvent *);
		void getMouseReleased(const QMouseEvent &);
		void getMousePressed(const QMouseEvent &);
		QString calc_stats(struct MonteCarloStats *, const vector <double>, QString);

};

#endif

