#ifndef US_SPECTRUM_H
#define US_SPECTRUM_H

#include "us_widgets.h"
#include "us_plot.h"

struct Gaussian
{
	double mean, amplitude, sigma;
};

struct WavelengthProfile
{
	QVector <struct Gaussian> gaussians;
	unsigned int lambda_scale, lambda_min, lambda_max;
	float scale, amplitude;
	QString filename, filenameBasis;
};

class US_Spectrum : public US_Widgets
{
	Q_OBJECT

	public:
		US_Spectrum();

	private:
		QwtPlot 		*data_plot, *residuals_plot;
		US_Plot		*plotLayout1, *plotLayout2;
		struct WavelengthProfile target;
		QVector <struct WavelengthProfile> v_basis;

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
		
		QListWidget *lw_target;
		QListWidget *lw_basis;
		QLabel		*lbl_wavelength;
		QLabel		*lbl_extinction;
		QLabel		*lbl_scaling;	
		QLineEdit	*le_wavelength;
		QLineEdit	*le_extinction;
		QLineEdit	*le_rmsd;

	private slots: 
		void	load_basis();
		void	load_target();
		void	load_gaussian_profile(struct WavelengthProfile&, const QString&);
		void 	find_amplitude(struct WavelengthProfile&);
};

#endif
