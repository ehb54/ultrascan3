#ifndef US_EDIT_H
#define US_EDIT_H

#include <qprogbar.h>
#include <qwt_math.h>
#include <qwt_counter.h>
#include <qtimer.h>
#include <qcombobox.h>
#include <qdatetime.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qstring.h>
#include <qpoint.h>
#include <qdir.h>
#include <qlayout.h>
#include <vector>

#include "us_util.h"
#include "us_rundetails.h"
#include "us_edscan.h"
#include "us_extern.h"
#include "us_math.h"
#include "us_select_channel.h"
#include "us_exclude_profile.h"

struct absscan
{
	unsigned int time;
	vector <float> rad;
	vector <float> abs;
};


class US_EXTERN EditData_Win : public QFrame
{
	Q_OBJECT

	public:
		QLabel *lbl_instructions1;
		QLabel *lbl_instructions2;
		QLabel *lbl_status1;
		QLabel *lbl_details;
		QLabel *lbl_run_id1;
		QLabel *lbl_run_id2;
		QLabel *lbl_current_cell1;
		QLabel *lbl_current_cell2;
		QLabel *lbl_directory;
		QLabel *lbl_rotor;
		QLabel *lbl_centerpiece;
		QComboBox *cmbb_centerpiece;
		QComboBox *cmbb_rotor;
		QPushButton *pb_select_dir;
		QPushButton *pb_details;
		QPushButton *pb_run_id;
		QPushButton *pb_accept;
		QPushButton *pb_editscan;
		QPushButton *pb_exsingle;
		QPushButton *pb_exrange;
		QPushButton *pb_invert;
		QPushButton *pb_sub_baseline;
		QPushButton *pb_exclude_profile;
		QPushButton *pb_spikes;
		QPushButton *pb_close;
		QPushButton *pb_help;
		QwtCounter *cnt_exsingle;
		QwtCounter *cnt_exrange;
		QwtCounter *cnt_edit;
		QProgressBar *progress;
		QwtPlot *edit_plt;
		RunDetails_F *edit_details;

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif

		vector <struct rotorInfo> rotor_list;
		vector <struct centerpieceInfo> cp_list;
		vector <struct absscan> oldscan;

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif

		struct rotorInfo rotor;
		struct centerpieceInfo centerpiece;

		US_Config *USglobal;
		unsigned int step;
		unsigned int current_channel;
		bool database; 					// if true, we are running the us_editdb module
		bool save_button;					// if true, pb_accept will be used
												// to save a subset of equilibrium data
		int edit_type; 					// 1=absorbance velocity
												// 2=absorbance equilibrium
												// 3=interference velocity
												// 4=interference equilibrium
												// 5=fluorescence velocity
												// 6=fluorescence equilibrium
		QString edited_channel;			// which channel was edited in a fluorescence expt.
		void step_check();
		QDir *data_dir;
		QPoint p1;							// stores position when mouse is pressed in plot window
		bool zoomflag;
		QString str, str1, str2;
		QString extension, extension1, extension2;
		uint t_rpm;							// temporary variable for rpm
		unsigned int dataset_counter;	// counts only datasets containing data, not empty ones
		double **radius;					// 2-d array[scans][points] for radius
		double **absorbance;				// 2-d array[scans][points] for absorbance
		double **temp_radius;			// temporary 2-d array[scans][points] for radius of equilibrium data
		double **temp_absorbance;		// temporary 2-d array[scans][points] for absorbance of equilibrium data
		unsigned int active_set[24];	// contains information about which datasets have data
		unsigned int *points;			// datapoints in each scan
		unsigned int *temp_points;		// temporary datapoints in each scan for 6 channels
		unsigned int count1;
		unsigned int count2;
		int index1, index2;
		char trashcan[100];
		unsigned int *curve;
		int cell, current_centerpiece, current_rotor;							//
		int lambda;
		float current_R;					// current radius returned by MouseEvent
		float limit_left;
		float limit_right;
		double xmin;
		double xmax;
		double ymin;
		double ymax;
		unsigned int exclude_single;
		unsigned int exclude_range;
		unsigned int edit_scan;
		edit_single_Win *edit_win;
		struct runinfo run_inf;

		QString dir_str;
		QString run_id;
		QPushButton *pb_zoom;
		EditData_Win(QWidget *p = 0, const char *name = 0);
		~EditData_Win();

	public slots:

		void quitthis();
		void details();
		void directory();
		void fluorescence_directory();
		void newdir(const QString &);
		void edit_all();
		void invert();
		virtual void load_dataset();
		virtual void plot_dataset();
		virtual void get_x(const QMouseEvent &e);
		virtual void help();
		virtual void sub_baseline();
		void set_range1(double);
		void set_range2(double);
		void set_editscan(double);
		void exSingle();
		void exRange();
		void remove_spikes();
		void edSingle();
		void write_equil_file();
		void set_zoom();
		void exclude_profile();
		void getMousePressed(const QMouseEvent &e);
		void update_rotor(int);
		void update_centerpiece(int);

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif

		void update_exclude_profile(vector <bool>);
		void final_exclude_profile(vector <bool>);

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif

	protected slots:
//		virtual void resizeEvent(QResizeEvent *e);
//		void mouseMoved(const QMouseEvent &e);
		virtual void closeEvent(QCloseEvent *);
		virtual void cleanup_dataset();

	signals:
		void explain(const QString &);
		void clicked();
		void channel_selected(const QString &);
		void absorbance_changed();
};

#endif

