#ifndef US_EDVINT_H
#define US_EDVINT_H
#include "us_edit.h"


class US_EXTERN EditIntVeloc_Win : public EditData_Win
{

	Q_OBJECT
	
	public:
		EditIntVeloc_Win(QWidget *p = 0, const char *name = 0);
		~EditIntVeloc_Win();
		US_ZoomWin *zoom_win;
		QPushButton *pb_subtract;
		QPushButton *pb_movie;
		QPushButton *pb_continue;
		QwtCounter *ct_order;
		QwtCounter *ct_fringe_tolerance;
		QLabel *lbl_fringe_tolerance;
		double r1;
		double r2;
		double od1;
		double od2;
		double *residuals;
		int right_index, left_index;
		unsigned int order;
		double *absorbance_integral, fringe_tolerance;
		bool residuals_defined;
		
public slots:
		void movie();
		void next_step();
		void help();
		void get_x(const QMouseEvent &e);
		void sub_baseline();
		float calc_sum(int, int);
		void load_base();
		void load_base(const char *);
		void subtract_residuals();
		void fit_ls(double);
		void calc_integral();
		void setFringeTolerance(double);
		
protected slots:
		void setup_GUI();
	//	void resizeEvent(QResizeEvent *e);
};

#endif

