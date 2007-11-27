//
// C++ Implementation: us_dma60/
//
// Description:
//
//
// Author: Borries Demeler <demeler@biochem.uthscsa.edu>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef US_DMA60_H
#define US_DMA60_H

#include <qwidget.h>
#include <qlistbox.h>
#include <qsqldatabase.h>
#include <qsqlquery.h>
#include <qsqlcursor.h>
#include <qfiledlg.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qstring.h>
#include <math.h>
#include <vector>
#include "us_editor.h"
#include "us_db.h"
#include "us_db_tbl_investigator.h"

using namespace std;

//! An Interface for Measuring density and vbar using the Anton Paar DMA 60 instrument
/*!
	This program can be used to calibrate the instrument, record calibration data, and
	calculate densities and vbar values for solutes in measured and known solutions
*/

struct measurement
{
	double temperature;
	double density;
	double pressure;
	QString units;
	double T_water;
	double T_air;
	double T_sample;
	double k;
	double concentration;
	QString description;
	QString dateTime;
};

struct dma_constant
{
	double k;
	double temperature;
	double pressure;
	double T_water;
	double T_air;
	QString dateTime;
};


class US_EXTERN US_DMA60 : public QWidget
{
	Q_OBJECT
	public:
		US_DMA60(QWidget *parent=0, const char *name=0);
		~US_DMA60();
		US_Config *USglobal;
		QLabel *lbl_banner1;
		QLabel *lbl_banner2;
		QLabel *lbl_banner3;
		QLabel *lbl_banner4;
		QLabel *lbl_banner5;
		QLabel *lbl_banner6;
		QLabel *lbl_temperature;
		QLabel *lbl_pressure;
		QLabel *lbl_T_air;
		QLabel *lbl_T_water;
		QLabel *lbl_k;
		QLabel *lbl_description;
		QLabel *lbl_units;
		QLabel *lbl_concentration;
		QLabel *lbl_T_sample;
		QLabel *lbl_sample_density;
		QLabel *lbl_conc_limit_low;
		QLabel *lbl_conc_limit_high;
		QLabel *lbl_steps;
		QLabel *lbl_select_conc;
		QLabel *lbl_select_density;
		QLineEdit *le_temperature;
		QLineEdit *le_pressure;
		QLineEdit *le_T_air;
		QLineEdit *le_T_water;
		QLineEdit *le_k;
		QLineEdit *le_description;
		QLineEdit *le_units;
		QLineEdit *le_concentration;
		QLineEdit *le_T_sample;
		QLineEdit *le_sample_density;
		QLineEdit *le_conc_limit_low;
		QLineEdit *le_conc_limit_high;
		QLineEdit *le_steps;
		QLineEdit *le_select_conc;
		QLineEdit *le_select_density;
		QLineEdit *le_extrapolation;
		QListBox *lb_k;
		QListBox *lb_measurement;
		QListBox *lb_extrapolation;
		QPushButton *pb_calibrate;
		QPushButton *pb_save_k;
		QPushButton *pb_load_k;
		QPushButton *pb_measurement;
		QPushButton *pb_save_measurement;
		QPushButton *pb_load_measurement;
		QPushButton *pb_extrapolate_single_value;
		QPushButton *pb_extrapolate;
		QPushButton *pb_help;
		QPushButton *pb_close;
		QPushButton *pb_reset_k;
		QPushButton *pb_reset_measurement;


		int InvID;
		unsigned int steps, current_k, current_measurement;
		double temperature, constant, T_air, T_water, T_sample, pressure, concentration;
		float conc_limit_low, conc_limit_high, select_conc, select_density;
		bool k_changed, density_changed; // keep track of it if need to be saved
		vector <struct dma_constant> dma;
		vector <struct measurement> recording;
		QString description, units;

	protected slots:
		void update_temperature(const QString &);
		void update_pressure(const QString &);
		void update_T_air(const QString &);
		void update_T_water(const QString &);
		void update_description(const QString &);
		void update_units(const QString &);
		void update_concentration(const QString &);
		void update_T_sample(const QString &);
		void update_conc_limit_low(const QString &);
		void update_conc_limit_high(const QString &);
		void update_steps(const QString &);
		void update_select_conc(const QString &);
		void update_select_density(const QString &);
		void extrapolate_single_value();
		void calibrate();
		void save_k();
		void load_k();
		void reset_k();
		void select_k(int);
		void measurement();
		void save_measurement();
		void load_measurement();
		void reset_measurement();
		void select_measurement(int);
		void extrapolate();
		void help();
		void quit();
};

#endif
