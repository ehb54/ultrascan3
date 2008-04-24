#ifndef US_SIMULATIONPARAMETERS_H
#define US_SIMULATIONPARAMETERS_H

#include <iostream>

#include <qlayout.h>
#include <qframe.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qapp.h>
#include <qmessagebox.h>
#include <qlineedit.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qlabel.h>
#include <qtextstream.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qbuttongroup.h>

#include <qwt_counter.h>
#include "us_util.h"
#include "us_femglobal.h"

extern int global_Xpos;
extern int global_Ypos;

class US_EXTERN US_SimulationParameters : public QDialog
{
	Q_OBJECT
	public:
		US_SimulationParameters(struct SimulationParameters *, QWidget *parent=0, const char *name=0);
		~US_SimulationParameters();

		US_Config *USglobal;
		struct SimulationParameters *simparams;
		unsigned int current_speed_step;

	private:
	
		struct SimulationParameters backup_simparams;
		QButtonGroup *bg_centerpiece_selection;
		QCheckBox *cb_acceleration_flag;
		QCheckBox *cb_standard;
		QCheckBox *cb_band;
		QComboBox *cmb_speeds;
		QComboBox *cmb_mesh;
		QComboBox *cmb_moving;

		QLabel *lbl_selected_speed;
		QLabel *lbl_number_of_speeds;
		QLabel *lbl_title;
		QLabel *lbl_duration_hours;
		QLabel *lbl_lamella;
		QLabel *lbl_duration_minutes;
		QLabel *lbl_delay_hours;
		QLabel *lbl_delay_minutes;
		QLabel *lbl_rotorspeed;
		QLabel *lbl_acceleration;
		QLabel *lbl_acceleration_flag;
		QLabel *lbl_simpoints;
		QLabel *lbl_radial_resolution;
		QLabel *lbl_meniscus;
		QLabel *lbl_bottom;
		QLabel *lbl_scans;
		QLabel *lbl_rnoise;
		QLabel *lbl_inoise;
		QLabel *lbl_rinoise;

		QwtCounter *cnt_duration_hours;
		QwtCounter *cnt_duration_minutes;
		QwtCounter *cnt_delay_hours;
		QwtCounter *cnt_delay_minutes;
		QwtCounter *cnt_rotorspeed;
		QwtCounter *cnt_acceleration;
		QwtCounter *cnt_simpoints;
		QwtCounter *cnt_lamella;
		QwtCounter *cnt_radial_resolution;
		QwtCounter *cnt_meniscus;
		QwtCounter *cnt_bottom;
		QwtCounter *cnt_scans;
		QwtCounter *cnt_rnoise;
		QwtCounter *cnt_inoise;
		QwtCounter *cnt_rinoise;
		QwtCounter *cnt_selected_speed;
		QwtCounter *cnt_number_of_speeds;
		
		QPushButton *pb_cancel;
		QPushButton *pb_ok;
		QPushButton *pb_help;
		QPushButton *pb_save;
		QPushButton *pb_load;

	protected slots:
		void closeEvent(QCloseEvent *e);
	
	private slots:

		void setupGUI();
		void update_duration_hours(double);
		void update_duration_minutes(double);
		void update_delay_hours(double);
		void update_delay_minutes(double);
		void update_rotorspeed(double);
		void update_simpoints(double);
		void update_radial_resolution(double);
		void update_meniscus(double);
		void update_acceleration(double);
		void update_number_of_speeds(double);
		void update_bottom(double);
		void update_scans(double);
		void update_rnoise(double);
		void update_inoise(double);
		void update_rinoise(double);
		void select_speed_profile(double);
		void select_lamella(double);
		void select_speed_profile(int);
		void load();
		void acceleration_flag();
		void save();
		void revert();
		void check_delay();
		void help();
		void update_combobox();
		void update_mesh(int);
		void update_moving(int);
		void select_centerpiece(int);
		void save(const QString &);
		void printError(const int &);
		void check_params();
};

#endif

