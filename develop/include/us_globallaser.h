#ifndef US_GLOBALLASER_H
#define US_GLOBALLASER_H

#include <qapp.h>
#include <qstyle.h>
#include <qprinter.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qwt_plot.h>
#include <qwt_counter.h>
#include <qwt_symbol.h>
#include "us_globalequil.h"
#include "us_db_laser.h"

class US_EXTERN US_GlobalLaser_Detail : public US_DB_Laser 
{
	Q_OBJECT
	
	public:
		US_GlobalLaser_Detail(struct LaserData temp_list, QWidget *p = 0, const char *name = "laser_detail");
		~US_GlobalLaser_Detail();
	struct LaserData ls_list;
	QLabel *lbl_invest;
	QLabel *lbl_buff;
	QLabel *lbl_pep;
	QLabel *lbl_dna;

	 protected slots:
		void parameter_detail();
		void channel_detail();
		void result_detail();	
		virtual void resizeEvent(QResizeEvent *e);
};

class US_EXTERN US_GlobalLaser : public US_GlobalEquil
{
	Q_OBJECT
	
	public:
	
		US_GlobalLaser(QWidget *p=0, const char *name="us_globallaser");
		~US_GlobalLaser();
		
		QPushButton *pb_load_data;

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif

		vector <struct LaserData> ls_info;

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif

		struct LaserData currentData;
		US_DB_Laser *us_laser_db;
		US_GlobalLaser_Detail *ls_detail;			
	protected slots:
		void load_data();
		void load_plot(struct LaserData);
		void update_data(struct LaserData);
		void select_ls(int);
		void show_detail();
		virtual void resizeEvent(QResizeEvent *e);
		
};

#endif

