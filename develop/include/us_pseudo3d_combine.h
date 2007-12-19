#ifndef US_PSEUDO3D_COMBINE_H
#define US_PSEUDO3D_COMBINE_H

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
#include <qlineedit.h>
#include <qlayout.h>
#include <qprogressbar.h>
#include <qtextedit.h>
#include <qlistbox.h>
#include <qcheckbox.h>
#include <vector>
#include <iterator>

struct distro_system
{
	list <Solute> s_distro;
	list <Solute> mw_distro;
	vector <QColor> gradient;
	QString run_name;
	QString cell;
	QString wavelength;
	QString method;
	unsigned int distro_type;
	bool monte_carlo;
};

class US_EXTERN US_Pseudo3D_Combine : public QFrame
{
	Q_OBJECT

	public:

		US_Pseudo3D_Combine(QWidget *p = 0, const char *name = 0);
		~US_Pseudo3D_Combine();
		vector <struct distro_system> system;
		vector <QColor> current_gradient;
		US_Config *USglobal;	 /*!< A US_Config reference. */
		float resolution, plot_fmax, plot_fmin, plot_smax, plot_smin;
		unsigned int x_resolution, y_resolution, x_pixel, y_pixel;
		bool minmax; //min = false, max = true
		unsigned int current_distro;
		unsigned int initial_solutes, monte_carlo_iterations, dimension;
		bool zoom, autolimit, plot_s;

	private:

		float k_range, s_range;

		QLabel *lbl_info1;
		QLabel *lbl_resolution;
		QLabel *lbl_x_resolution;
		QLabel *lbl_y_resolution;
		QLabel *lbl_x_pixel;
		QLabel *lbl_y_pixel;
		QLabel *lbl_autolimit;
		QLabel *lbl_plot_smin;
		QLabel *lbl_plot_smax;
		QLabel *lbl_plot_fmin;
		QLabel *lbl_plot_fmax;
		QLabel *lbl_current_distro;
		QLineEdit *le_distro_info;
		QwtCounter *cnt_plot_fmin;
		QwtCounter *cnt_plot_fmax;
		QwtCounter *cnt_plot_smin;
		QwtCounter *cnt_plot_smax;
		QwtCounter *cnt_resolution;
		QwtCounter *cnt_x_resolution;
		QwtCounter *cnt_y_resolution;
		QwtCounter *cnt_x_pixel;
		QwtCounter *cnt_y_pixel;
		QwtCounter *cnt_current_distro;
		QwtPlot *plot;

		QPushButton *pb_help;
		QPushButton *pb_color;
		QPushButton *pb_replot3d;
		QPushButton *pb_print;
		QPushButton *pb_load_distro;
		QPushButton *pb_close;
		QPushButton *pb_save;
		QPushButton *pb_reset;
		QCheckBox *cb_autolimit;
		QCheckBox *cb_plot_s;
		QCheckBox *cb_plot_mw;
		QProgressBar *progress;

	private slots:

		void update_plot_smax(double);
		void update_plot_smin(double);
		void update_plot_fmax(double);
		void update_plot_fmin(double);
		void update_resolution(double);
		void update_x_resolution(double);
		void update_y_resolution(double);
		void update_x_pixel(double);
		void update_y_pixel(double);
		void update_current_distro(double);
		void plot_3dim();
		void select_autolimit();
		void select_plot_s();
		void select_plot_mw();
		void load_distro();
		void load_distro(const QString &);
		void load_color();
		void help();
		void save();
		void print();
		void reset();
		void set_limits(); // resets the plot limits in the counters
		void setup_GUI();
		void closeEvent(QCloseEvent *);
};

#endif

