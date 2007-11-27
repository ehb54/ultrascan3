#ifndef US_EQUILU_H
#define US_EQUILU_H

#include "us_util.h"
#include "us_math.h"
#include "us_db_tbl_buffer.h"
#include "us_db_tbl_vbar.h"

#include <qwt_counter.h>
#include <qradiobutton.h>
#include <qtimer.h>

extern int global_Xpos;
extern int global_Ypos;

class US_EXTERN US_EquilSpeed : public QFrame
{
	Q_OBJECT
	
	public:
		US_EquilSpeed( QWidget *parent=0, const char *name=0 );
		~US_EquilSpeed();
		US_Config *USglobal;

	private:
		float density_w20, density_tb;
		float vbar, vbar20;
		float temperature;
		float bottom;
		float meniscus;
		float sigma;
		float ratio;
		float mw;
		float volume;
		float correction;
		float buoyancyb;
		float buoyancyw;
		bool alu_centerpiece;
		bool an50;
		bool vbar_from_seq;
		int blink1, blink2;
		uint rpm;
		QLabel *banner1;
		QLabel *banner2;
		QLabel *banner3;
		QLabel *lbl_mw;
		QLabel *lbl_lcd1;
		QLabel *lbl_lcd2;
		QLabel *lbl_lcd3;
		QLabel *lbl_lcd4;
		QLabel *lbl_overspeed1;
		QLabel *lbl_overspeed2;
		QLabel *lbl_volume;
		QLabel *lbl_temperature;
		QLabel *lbl_meniscus1;
		QLabel *lbl_meniscus2;
		QLabel *lbl_bottom1;
		QLabel *lbl_bottom2;
		QLabel *lbl_column1;
		QLabel *lbl_column2;
		QLabel *lbl_sigma;
		QLabel *lbl_rpm1;
		QLabel *lbl_rpm2;
		QLabel *lbl_intpoints1;
		QLabel *lbl_intpoints2;
		QLabel *lbl_abspoints1;
		QLabel *lbl_abspoints2;
		QLabel *lbl_ratio1;
		QLabel *lbl_ratio2;
		QLabel *lbl_rotor;
		QLabel *lbl_blank1;
		QLabel *lbl_blank2;
		QLabel *lbl_centerpiece;
		QwtCounter *ct_mw;
		QwtCounter *ct_sigma;
		QLineEdit *le_density;
		QLineEdit *le_vbar;
		QLineEdit *le_volume;
		QLineEdit *le_temperature;
		QRadioButton *bt_an50;
		QRadioButton *bt_an60;
		QRadioButton *bt_aluminum;
		QRadioButton *bt_epon;
		QPushButton *pb_help;
		QPushButton *pb_close;
		QPushButton *pb_density;
		QPushButton *pb_vbar;
		QTimer *flash_time1;
		QTimer *flash_time2;
		US_Vbar_DB *vbar_dlg;
		US_Buffer_DB *buffer_dlg;

	private slots:
		void quit();
		void help();
		void read_vbar();
		void update_vbar(const QString &);
		void update_density(const QString &);
		void update_vbar_lbl(float, float);
		void update_buffer_lbl(float, float);
		void get_buffer();
		void update_mw(double);
		void update_volume(const QString &);
		void update_temperature(const QString &);
		void update_sigma(double);
		void calc_correction();
		void animate1();
		void animate2();
		void calc();
		void valid();
		void update_epon();
		void update_aluminum();
		void update_an50();
		void update_an60();

	protected slots:
		void setup_GUI();
		void closeEvent(QCloseEvent *e);
};

#endif

