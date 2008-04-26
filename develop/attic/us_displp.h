#ifndef US_DISPLP_H
#define US_DISPLP_H

#include <qwidget.h>
#include <qframe.h>
#include <qpushbt.h>
#include <qcombo.h>
#include <qlistbox.h>
#include <string.h>
#include <qdialog.h>
#include <qfont.h>
#include <qlayout.h>
#include "us_util.h"
#include "us_simdlg.h"
#include "us_math.h"

class DisplayPanel : public QFrame
{
	Q_OBJECT

	public:
		QLabel *simsettings;
		QLabel *speed_lb1;
		QLabel *speed_lb2;
		QLabel *speed_lb3;
		QLabel *speed_lb4;
		QLabel *meniscus_lb1;
		QLabel *meniscus_lb2;
		QLabel *bottom_lb1;
		QLabel *bottom_lb2;
		QLabel *time_lb1;
		QLabel *time_lb2;
		QLabel *time_lb3;
		QLabel *time_lb4;
		QLabel *scans_lb1;
		QLabel *scans_lb2;
		QLabel *scans_lb3;
		QLabel *scans_lb4;
		QLabel *rnoise_lb1;
		QLabel *rnoise_lb2;
		QLabel *inoise_lb1;
		QLabel *inoise_lb2;
		QLabel *delta_r_lb1;
		QLabel *delta_r_lb2;
		QLabel *delta_t_lb1;
		QLabel *delta_t_lb2;
		QLabel *points_lb1;
		QLabel *points_lb2;
		QLabel *modelsettings;
		QLabel *model_lb1;
		QLabel *model_lb2;
		QLabel *comp_lb1;
		QLabel *comp_lb2;
		QLabel *component_id;
		QLabel *conc_lb1;
		QLabel *conc_lb2;
		QLabel *eq_lb1;
		QLabel *eq_lb2;
		QLabel *sed_lb1;
		QLabel *sed_lb2;
		QLabel *totalc_lb1;
		QLabel *totalc_lb2;
		QLabel *diff_lb1;
		QLabel *diff_lb2;
		QLabel *sigma_lb1;
		QLabel *sigma_lb2;
		QLabel *delta_lb1;
		QLabel *delta_lb2;
		QwtCounter *comp_ct;
		QPushButton *update_b;
		QPushButton *close_b;
		DisplayPanel(unsigned int *, int *, vector <struct component>, 
						struct simulation_parameters *, QWidget *p = 0, const char *name = 0);
		~DisplayPanel();
		US_Config *USglobal;

	public slots:
		void set_current(double);
		void update_panel();
		void close_panel();

	private:	
		int *model;
		unsigned int *comp, current;
		vector <struct component> components;
		struct simulation_parameters *sp;
		
	protected slots:
		void setup_GUI();
		void closeEvent(QCloseEvent *e);
};
#endif

