#ifndef US_MONTECARLO_H
#define US_MONTECARLO_H

#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qwt_plot.h>
#include <qpoint.h>
#include <qwt_counter.h>
#include <qfiledialog.h>
#include <qapplication.h>
#include <qlistbox.h>
#include "us_util.h"
#include "us_pixmap.h"
#ifdef UNIX
#include "us_beowulf.h"
#endif
#include "us_montecarlostats.h"
#include "us_math.h"
#include "us_selectplot.h"
#include "us_printfilter.h"



class US_MonteCarlo : public QFrame
{

	Q_OBJECT

	public:

		US_MonteCarlo(struct MonteCarlo *temp_mc, 
		bool *temp_mc_widget, QWidget *p = 0, const char *name = 0);
		~US_MonteCarlo();
		struct MonteCarlo *mc;
		struct MonteCarloStats stats;
		bool *mc_widget;
		bool stats_widget;
		bool beowulf_widget;
		bool file_read;
		bool file_ok;
		bool from_public;
		bool dont_show;
		bool print_plot;
		QString plot_status;
		vector <float> amplitude, mw;
		unsigned int bins, entries;
		vector <struct lines> parameter_value;
		vector <unsigned int> parameter_bin;
		int border, spacing, plot_start, column1, column2, current_item;
		QString htmlDir;
		US_Config *USglobal;
#ifdef UNIX
                US_Beowulf *beowulf_W;
#endif
		US_MonteCarloStats_W  *stats_window;
		US_Pixmap *pm;
		QPoint p1, p2;
		QLabel *lbl_banner1;
		QLabel *lbl_new;
		QLabel *lbl_append;
		QLabel *lbl_banner2;
		QLabel *lbl_iterations;
		QLabel *lbl_current_iteration1;
		QLabel *lbl_current_iteration2;
		QLabel *lbl_gaussian;
		QLabel *lbl_bootstrap;
		QLabel *lbl_mixed;
		QLabel *lbl_guesses;
		QLabel *lbl_random;
		QLabel *lbl_lastFit;
		QLabel *lbl_originalFit;
		QLabel *lbl_percent_bootstrap;
		QLabel *lbl_percent_gaussian;
		QLabel *lbl_seed;
		QLabel *lbl_percent;
		QLabel *lbl_banner3;
		QLabel *lbl_mean1;
		QLabel *lbl_mean2;
		QLabel *lbl_bins;
		QLabel *lbl_entries1;
		QLabel *lbl_entries2;
		QLabel *lbl_status;
		QLabel *lbl_ignoreVariance;
		QLineEdit *le_filename;
		QLineEdit *le_noiseLevel;
		QLineEdit *le_iterations;
		QLineEdit *le_seed;
		QLineEdit *le_bins;
		QLineEdit *le_ignoreVariance;
		QListBox *lb_parameters;
		QListBox *lb_rules;
		QCheckBox *cb_new;
		QCheckBox *cb_append;
		QCheckBox *cb_gaussian;
		QCheckBox *cb_bootstrap;
		QCheckBox *cb_mixed;
		QCheckBox *cb_random;
		QCheckBox *cb_lastFit;
		QCheckBox *cb_originalFit;
		QwtCounter *cnt_percent_bootstrap;
		QwtCounter *cnt_percent_gaussian;
		QPushButton *pb_filename;
		QPushButton *pb_beowulf;
		QPushButton *pb_update;
		QPushButton *pb_statistics;
		QPushButton *pb_print;
		QPushButton *pb_save;
		QPushButton *pb_start;
		QPushButton *pb_stop;
		QPushButton *pb_help;
		QPushButton *pb_quit;
		QProgressBar *progress;
		QwtPlot *histogram_plot;

	signals:
	
		void iterate();
		
	public slots:
		void update_iteration();
      void beowulf();


	protected slots:
		void set_new();
		void set_append();
		void select_file();
		void update_file(const QString &);
		void set_gaussian();
		void set_bootstrap();
		void set_mixed();
		void set_random();
		void set_originalFit();
		void set_lastFit();
		void update_noiseLevel(const QString &);
		void update_ignoreVariance(const QString &);
		void update_percent_bootstrap(double);
		void update_percent_gaussian(double);
		void select_seed(const QString &);
		void update_parameter();
		void update_rule(int);
		void show_parameter();
		void show_parameter(int);
		void statistics();
		void update_total_iterations(const QString &);
		void update_bins(const QString &);
		void print();
		bool createHtmlDir();
		void start();
		void stop();
		void quit();
		void file_error();
		void plotMousePressed(const QMouseEvent &);
		void plotMouseReleased(const QMouseEvent &);
		virtual void help();
		virtual void save();
		void closeEvent(QCloseEvent *e);
		void resizeEvent(QResizeEvent *e);
};

#endif

