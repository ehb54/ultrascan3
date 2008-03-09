#ifndef US_COMBINE_H
#define US_COMBINE_H

#include "us_db_tbl_vbar.h"
#include "us_selectplot.h"
#include "us_printfilter.h"
#include "us_rundetails.h"

#include <qprinter.h>
#include <qwt_counter.h>
#include <qwt_plot.h>
#include <qbuttongroup.h>

struct distribution
{

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif

	vector <double> sval;
	vector <double> fraction;

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif

	QString id;
};

enum distribution_type
{
	INTEGRAL = 0, ENVELOPE = 1
};

class US_EXTERN US_Combine : public QFrame
{
	Q_OBJECT
	
	public:
	
		US_Combine(int, QWidget *p = 0, const char *name = 0);
		~US_Combine();
		struct runinfo run_inf;
		QString analysis_type;
		int first_cell;

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif

		vector <struct centerpieceInfo> cp_list;
		vector <struct rotorInfo> rotor_list;
		vector <struct distribution> distro;
		vector <bool> is_integral;

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif
    
    float ff0;
		float vbar;
		int prog_flag;
		QColor col[20];
		distribution_type distro_type;
		QString distribName;
		US_Config *USglobal;
		US_Pixmap *pm;
		QPushButton *pb_load;
		QPushButton *pb_details;
		QPushButton *pb_save;
		QPushButton *pb_print;
		QPushButton *pb_help;
		QPushButton *pb_close;
		QPushButton *pb_reset;
		QPushButton *pb_zoom_in;
		QPushButton *pb_zoom_out;
		QPushButton *pb_vbar;
		QwtCounter *ct_Xmin;
		QwtCounter *ct_Xmax;
		QwtCounter *ct_ff0;
		QLabel *banner0;
		QLabel *banner2;
		QLabel *banner3;
		QLabel *lbl_distribName;
		QLabel *lbl_ff0;
		QLabel *lbl_run_id1;
		QLineEdit *le_run_id;
		QLabel *lbl_cell_info1;
		QLabel *lbl_cell_info2;
		QTextEdit *lbl_cell_descr;
		QLabel *lbl_temperature1;
		QLabel *lbl_temperature2;
		QLabel *lbl_Xmin;
		QLabel *lbl_Xmax;
		QLabel *lbl_blank;
		QLabel *lbl_integral, *lbl_envelope;
		QCheckBox *cb_integral, *cb_envelope;
		QButton *b_sa2d, *b_ga;
		QButton *b_sa2d_mc, *b_ga_mc;
		QButton *b_fe, *b_cofs;
		QButton *b_sa2d_mw, *b_sa2d_mw_mc;
		QButton *b_ga_mw, *b_ga_mw_mc;
		QButtonGroup *bg_method_selection;
		QLineEdit *le_distribName;
		QLineEdit *le_vbar;
		QListBox *cell_select;
		QListBox *lambda_select;
		QwtPlot *analysis_plot;
		QwtSymbol *symbol;
		RunDetails_F *run_details;
		unsigned int i, j, k;
		unsigned int selected_cell;
		unsigned int selected_lambda;
		bool has_data[8];
		double max_s, min_s, orig_max_s, orig_min_s;
		uint *curve;
		QString fn;

	public slots:

		void show_cell(int);
		void show_lambda(int);
		void load_data();
		void details();
		void plotDistros();
		void quit();
		void set_colors();
		void print();
		void update_screen();
		void add_distribution(int);
		void reset();
		void help();
		void view();
		void update_distribName(const QString &);
		void update_vbar(const QString &);
		void zoom_in();
		void zoom_out();
		void save();
		void set_symbol();
		void update_Xmin(double);
		void update_Xmax(double);
		void update_ff0(double);
		void calc_limits();
		void enable_all();
		void disable_all();
		double convert(double);
		void read_vbar();
		void update_vbar_lbl(float, float);
		void select_integral();
		void select_envelope();
		void select_method(int);
		
	protected slots:
		void setup_GUI(int);
		void mousePressed(const QMouseEvent &);
//		void resizeEvent(QResizeEvent *e);
		void closeEvent(QCloseEvent *e);
	
};

#endif

