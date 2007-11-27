#ifndef US_SPECTRUM_H
#define US_SPECTRUM_H

#include <iostream>
#include <vector>

#include <qwidget.h>
#include <qframe.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <string.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qlayout.h>
#include <qlistbox.h>

#include <qwt_plot.h>
#include <qwt_counter.h>

#include "us_util.h"
#include "us_extinctfitter.h"

struct Gaussian
{
	double mean;
	double amplitude;
	double sigma;
};

struct WavelengthProfile
{
	vector <struct Gaussian> gaussian;
	float scale;
	unsigned int lambda_scale;
	unsigned int lambda_min;
	unsigned int lambda_max;
	QString filename;
	int curve_number;
// use this amplitude to scale the final distribution for any 
// desired extinction coefficient at any wavelength
	float amplitude;
	float nnls_factor;
	float nnls_percentage;
};

class US_EXTERN US_Spectrum : public QFrame
{
	Q_OBJECT

	public:
		US_Spectrum(QWidget *p = 0, const char *name = 0);
		~US_Spectrum();
		US_Config *USglobal;
		
	private:
		QwtPlot *data_plot, *residuals_plot;
		vector <struct WavelengthProfile> basis;
		struct WavelengthProfile target;
		vector <unsigned int> curve;
		vector <unsigned int> lambda_min;
		vector <unsigned int> lambda_max;
		unsigned int min, max, lambda_scale;
		int current_scan;
		float scale;
		int solution_curve;
		QPushButton *pb_load_target;
		QPushButton *pb_load_basis;
		QPushButton *pb_load_fit;
		QPushButton *pb_fit;
		QPushButton *pb_help;
		QPushButton *pb_save;
		QPushButton *pb_extrapolate;
		QPushButton *pb_overlap;
		QPushButton *pb_close;
		QPushButton *pb_reset_basis;
		QPushButton *pb_delete;
		QPushButton *pb_update;
		QPushButton *pb_difference;
		QPushButton *pb_print_fit;
		QPushButton *pb_print_residuals;

		QLabel *lbl_info;
		QLabel *lbl_wavelength;
		QLabel *lbl_extinction;
		
		QListBox *lb_basis;
		QListBox *lb_target;

		QLineEdit *le_wavelength;
		QLineEdit *le_extinction;
		QLineEdit *le_rmsd;

	protected slots:
		void closeEvent(QCloseEvent *e);
		
	private slots:
		void setup_GUI();
		void printError(const int &);
		void select_basis(const QMouseEvent &);
		void delete_basis(int);
		void load_basis();
		void load_target();
		void edit_basis(int);
		void edit_target(int);
		void load_fit();
		void load_gaussian_profile(struct WavelengthProfile &, const QString &);
		void reset_basis();
		void fit();
		void help();
		void update_scale();
		void delete_scan();
		void save();
		void extrapolate();
		void overlap();
		void difference();
		void reset_edit_gui();
		void find_amplitude(struct WavelengthProfile &);
		void print_residuals();
		void print_fit();
};

#endif
