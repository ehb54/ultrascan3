#ifndef US_RTVIMAGE_H
#define US_RTVIMAGE_H

#include <qpushbutton.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qfiledialog.h>
#include <qtextstream.h>
#include <qfiledialog.h>
#include <qsqldatabase.h>
#include <qsqlquery.h>
#include <qsqlcursor.h>
#include <qlistbox.h>

#include "us_db_rtv_investigator.h"


class US_EXTERN US_DB_RtvImage : public US_DB_RtvInvestigator
{
	Q_OBJECT
	
	public:
		US_DB_RtvImage(QWidget *p=0, const char *name="us_rtvimage");
		~US_DB_RtvImage();
		
		
		int *item_ImageID;
		int ImageID;
	public slots:
		void show_image();
	protected slots:
		void checkImage();	
	   void check_image(int);
	   void select_image(int); 	
};

#endif


