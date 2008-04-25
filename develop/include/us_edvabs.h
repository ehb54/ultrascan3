#ifndef US_EDVABS_H
#define US_EDVABS_H
#include "us_edit.h"

struct absscan
{
	vector <float> rad;
	vector <float> abs;
};

class US_EXTERN EditAbsVeloc_Win : public EditData_Win
{

	Q_OBJECT
	
	public:
		EditAbsVeloc_Win(QWidget *p = 0, const char *name = 0);
		~EditAbsVeloc_Win();
		
		double *absorbance_integral, *residuals;

		QPushButton *pb_subtract;
		QPushButton *pb_continue;
		QwtCounter *ct_order;
		QLabel *lbl_subtract_ri_noise;
		bool residuals_defined;
		unsigned int order;

#ifdef WIN32
		  #pragma warning ( disable: 4251 )
#endif

		vector <struct absscan> oldscan;

#ifdef WIN32
		  #pragma warning ( default: 4251 )
#endif


	public slots:
	
		void help();
		void get_x(const QMouseEvent &e);
		void setup_GUI();
		void next_step();
		void subtract_residuals();
		void fit_ls(double);
};

#endif

