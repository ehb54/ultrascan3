#ifndef US_EDEFLO_H
#define US_EDEFLO_H
#include "us_edit.h"


class US_EXTERN EditFloEquil_Win : public EditData_Win
{

	public:
		EditFloEquil_Win(QWidget *p = 0, const char *name = 0);
		~EditFloEquil_Win();
		void help();
		void get_x(const QMouseEvent &e);
	
	private slots:
		void setup_GUI();
};

#endif

