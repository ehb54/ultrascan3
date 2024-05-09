#ifndef US_SPECTRUM_H
#define US_SPECTRUM_H

#include "us_widgets.h"
#include "us_plot.h"
#include "us_math2.h"
#include <math.h>

/* struct Gaussian
{
	double mean, amplitude, sigma;
};
*/

struct WavelengthProfile
{
  //QVector <struct Gaussian> gaussians;
	QVector <double> extinction;
        QVector <double> wvl;
	QwtPlotCurve* matchingCurve;
	unsigned int lambda_scale, lambda_min, lambda_max;
	float scale, amplitude;
	QString filename, filenameBasis;
	float nnls_factor, nnls_percentage;
};

class US_Spectrum : public US_Widgets
{
	Q_OBJECT

	public:
		US_Spectrum();
		int basisIndex; 
		QString current_path;

	private:
		QwtPlot 		*data_plot, *residuals_plot;
		US_Plot		*plotLayout1, *plotLayout2;
		struct WavelengthProfile w_target;
		QVector <struct WavelengthProfile> v_basis;
		QwtPlotCurve *solution_curve;
		QwtPlotPicker* pick;
		struct WavelengthProfile w_solution;
	
		QPushButton *pb_load_target;
		QPushButton *pb_load_basis;
		QPushButton *pb_load_fit;
      QPushButton *pb_fit;
		QPushButton	*pb_find_angles;
      QPushButton *pb_help;
      QPushButton *pb_save;
      QPushButton *pb_overlap;
      QPushButton *pb_close;
      QPushButton *pb_reset_basis;
      QPushButton *pb_delete;
		QPushButton	*pb_find_angle;
		QPushButton *pb_find_extinction;
		
		//QStringList basis_names;
		
		QListWidget *lw_target;
		QListWidget *lw_basis;
		QLabel		*lbl_wavelength;
		QLabel		*lbl_extinction;
		QLineEdit	*le_angle;
		QLineEdit	*le_wavelength;
		QLineEdit	*le_extinction;
		QLineEdit	*le_rmsd;

		QLabel          *lbl_wvlinfo; 
		QLabel          *lbl_correlation; 
		QLabel          *lbl_fit; 
		QLabel          *lbl_rmsd;
		QLabel          *lbl_angle;
		QLabel          *lbl_load_save;

		QComboBox	*cb_angle_one;
		QComboBox	*cb_angle_two;
		QComboBox	*cb_spectrum_type;
	private slots: 
		void	load_basis();
		void	plot_basis();
		void	load_target();
		void	plot_target();
		void	new_value(const QwtDoublePoint&);
		void	findExtinction();
		//void 	find_amplitude(struct WavelengthProfile&);
		void	fit();
		void	deleteCurrent();
		bool	deleteBasisCurve( void );
		void	resetBasis();
		void	findAngles();
		void 	save();
		void	load();
		void	overlap();
};

#endif
