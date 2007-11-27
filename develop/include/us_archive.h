#ifndef US_ARCHIVE_H
#define US_ARCHIVE_H

#include <qlabel.h>
#include <string>
#include <qfont.h>
#include <qfiledialog.h>
#include <qwt_plot.h>
#include <qtextedit.h>
#include <qmessagebox.h>
#include <qapp.h>
#include <qcheckbox.h>

#include "us_dtactrl.h"
#ifdef HAS_VFS
#include <sys/vfs.h>
#endif
#include <errno.h>




extern int global_Xpos;
extern int global_Ypos;

class US_EXTERN US_Archive : public QFrame
{
	Q_OBJECT

	private:

		US_Config *USglobal;
		Data_Control_W *data_control;
		QString filename, projectName, view_filename, extract_filename, fn;
		QString current_data_dir, raw_file, result_file, report_file;
		QFile view_file;
#ifdef HAS_VFS
		struct statfs file_info;
#endif
		int run_type, v_step, c_step, e_step;
		bool ultrascan_flag, reports_flag, delete_flag, data_ctrl_flag;
		QTextEdit *mle;
		QPushButton *pb_view;
		QPushButton *pb_help;
		QPushButton *pb_extract;
		QPushButton *pb_select_equildata_create;
		QPushButton *pb_select_velocdata_create;
		QPushButton *pb_select_montecarlo_create;
		QPushButton *pb_select_eqproject_create;
		QPushButton *pb_create;
		QPushButton *pb_close;
		QLabel *lbl_name1;
		QLabel *lbl_name2;
		QLabel *lbl_create_name1;
		QLabel *lbl_create_name2;
		QLabel *lbl_ultrascan;
		QCheckBox *cb_ultrascan;
		QLabel *lbl_reports;
		QCheckBox *cb_reports;
		QLabel *lbl_delete;
		QCheckBox *cb_delete;
		QLabel *banner1;
		QLabel *banner2;
		QLabel *banner3;
		QLabel *banner4;
		
		QProcess *view_proc, *create_proc, *extract_proc;
		bool continue_flag;
	public:	
		US_Archive(QWidget *p = 0, const char *name = 0);
		~US_Archive();

	public slots:
		void view();
		void view(const QString &);
		void readView();
		void endView();
		void quit();
		void help();
		void set_ultrascan();
		void set_delete();
		void set_reports();
		void extract_archive();
		void create_archive();
		void readCreate();
	   void endCreateProcess();
		void readExtract();
	   void endExtractProcess();
		void select_velocdata_create_archive();
		void select_equildata_create_archive();
		void select_montecarlo_create_archive();
		void select_eqproject_create_archive();
		void clean_temp_dir();
		void disable_buttons();
		void enable_buttons();

	protected slots:
		void setup_GUI();
		void closeEvent(QCloseEvent *e);
//		void resizeEvent(QResizeEvent *e);
};

#endif

