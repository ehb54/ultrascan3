#include <QApplication>
#include "us_spectrum.h"

int main (int argc, char* argv[])
{
	QApplication application (argc, argv);

	US_Spectrum w;
	w.show();
	return application.exec();
}

US_Spectrum::US_Spectrum() : US_Widgets()
{
	//Vector for basis wavelength profiles
	v_basis.clear();

	//Push Buttons for US_Spectrum GUI
	pb_load_target = us_pushbutton(tr("Load Target Spectrum"));
	connect(pb_load_target, SIGNAL(clicked()), SLOT(load_target()));
	pb_load_basis = us_pushbutton(tr("Load Basis Spectrum"));
   pb_load_basis->setEnabled(false);
	connect(pb_load_basis, SIGNAL(clicked()), SLOT(load_basis()));
	pb_update = us_pushbutton(tr("Update Extinction Scaling"));	
	pb_difference = us_pushbutton(tr("Difference Spectrum"));
	pb_delete = us_pushbutton(tr("Delete Current Basis Scan"));
	pb_load_fit = us_pushbutton(tr("Load Fit"));
	pb_extrapolate = us_pushbutton(tr("Extrapolate Extinction Profile"));
	pb_overlap = us_pushbutton(tr("Find Extinction Profile Overlap"));
	pb_fit = us_pushbutton(tr("Fit Data"));
	pb_help = us_pushbutton(tr("Help"));
	pb_reset_basis = us_pushbutton(tr("Reset Basis Spectra"));
	pb_save= us_pushbutton(tr("Save Fit"));
	pb_print_fit = us_pushbutton(tr("Print Fit"));
	pb_print_residuals = us_pushbutton(tr("Print Residuals"));
	pb_close = us_pushbutton(tr("Close"));
	
	//List Widgets
	lw_target = us_listwidget();
	lw_basis = us_listwidget();

	//Label Widgets
	lbl_scaling = us_banner(tr("(Double-click to Edit Scaling"));
	lbl_wavelength = us_label(tr("Wavelength:"));
	lbl_extinction = us_label(tr("Extinction Coeff.:"));

	//Line Edit Widgets
	le_wavelength = us_lineedit("", 1, true);
	le_extinction = us_lineedit("", 1, true);
	le_rmsd = us_lineedit("RMSD:", 1, false);

   data_plot = new QwtPlot();
   plotLayout1 = new US_Plot(data_plot, tr(""), tr("Wavelength(nm)"), tr("Extinction"));
   data_plot->setCanvasBackground(Qt::black);
   data_plot->setTitle("Wavelength Spectrum Fit");
	data_plot->setMinimumSize(700,200);
	data_plot->clear();
	
   residuals_plot = new QwtPlot();
   plotLayout2 = new US_Plot(residuals_plot, tr(""), tr("Wavelength(nm)"), tr("Extinction"));
   residuals_plot->setCanvasBackground(Qt::black);
   residuals_plot->setTitle("Fitting Residuals");
	residuals_plot->setMinimumSize(700, 200);

	QGridLayout* plotGrid;
	plotGrid =  new QGridLayout();
	plotGrid->addLayout(plotLayout1, 0, 0);
	plotGrid->addLayout(plotLayout2, 1, 0);
	
	QGridLayout* subgl1;
	subgl1 = new QGridLayout();
	subgl1->addWidget(lbl_wavelength, 0,0);
	subgl1->addWidget(le_wavelength, 0,1);
	subgl1->addWidget(lbl_extinction, 1, 0);
	subgl1->addWidget(le_extinction, 1, 1);

	QGridLayout* subgl2;
	subgl2 = new QGridLayout();
	subgl2->addWidget(pb_print_residuals, 0, 0);
	subgl2->addWidget(pb_print_fit, 0, 1);
   subgl2->addWidget(pb_load_fit, 1, 0);
   subgl2->addWidget(pb_save, 1, 1);
   subgl2->addWidget(pb_help, 2, 0);
   subgl2->addWidget(pb_close, 2, 1);

	QGridLayout* gl1;
	gl1 = new QGridLayout();
	gl1->addWidget(pb_load_target, 0, 0);
	gl1->addWidget(lw_target, 1, 0);
	gl1->addWidget(pb_load_basis, 2, 0);
   gl1->addWidget(lw_basis, 3, 0);
	gl1->addWidget(lbl_scaling, 4, 0);
	gl1->addLayout(subgl1, 5, 0);
	gl1->addWidget(pb_update, 6, 0);
	gl1->addWidget(pb_delete, 7, 0);
   gl1->addWidget(pb_reset_basis, 8, 0);
   gl1->addWidget(pb_overlap, 9, 0);
   gl1->addWidget(pb_extrapolate, 10, 0);
   gl1->addWidget(pb_difference, 11, 0);
   gl1->addWidget(pb_fit, 12, 0);
   gl1->addWidget(le_rmsd, 13, 0);
	gl1->addLayout(subgl2, 14, 0);
	
	QGridLayout *mainLayout;
	mainLayout = new QGridLayout(this);
	mainLayout->setSpacing(2);
	mainLayout->setContentsMargins(2,2,2,2);
	mainLayout->addLayout(gl1, 0, 0);
	mainLayout->addLayout(plotGrid, 0, 1);
}

void US_Spectrum::load_basis()
{
   QStringList files;
	int row = 0;
   QVector  <double> v_x_values;
   QVector <double> v_y_values;
	
	int basisIndex = 0;
	struct WavelengthProfile temp_wp;
   QFileDialog dialog (this);
   dialog.setNameFilter(tr("Text (*.res)"));
   dialog.setFileMode(QFileDialog::ExistingFiles);
   dialog.setViewMode(QFileDialog::Detail);
   dialog.setDirectory("/home/minji/ultrascan/results");
   if(dialog.exec())
   {
      files = dialog.selectedFiles();
   }
   for (QStringList::const_iterator  it=files.begin(); it!=files.end(); ++it)
	{
		basisIndex = 0;
		load_gaussian_profile(temp_wp, *it);
		v_basis.push_back(temp_wp);
		QFileInfo fi;
		fi.setFile(*it);
		lw_basis->insertItem(row,fi.baseName());	
   	v_x_values.clear();
   	v_y_values.clear();

   	double temp_y;
   //plot the points for the target spectrum
      for(unsigned int k = 0; k < temp_wp.lambda_max - temp_wp.lambda_min; k++)
      {
         temp_y = 0;
         v_x_values.push_back(temp_wp.lambda_min + k);
         for(int j = 0; j < temp_wp.gaussians.size(); j++)
         {
            temp_y += temp_wp.gaussians.at(j).amplitude * exp(-(pow(v_x_values.at(k) - temp_wp.gaussians.at(j).mean, 2.0) / (2.0 * pow(temp_wp.gaussians.at(j).sigma, 2.0))));
  		   }
     	   temp_y *= temp_wp.amplitude;                      
     	   v_y_values.push_back(temp_y);
     	}
		QwtPlotCurve* c;
   	QPen p;
  		p.setColor(Qt::green);
   	p.setWidth(3);
   	c = us_curve(data_plot, v_basis.at(basisIndex).filename);
   	c->setPen(p);
   	c->setData(v_x_values, v_y_values);
		basisIndex++;
	}
	data_plot->replot();
}

void US_Spectrum::load_target()
{
   QFileDialog dialog (this);
   dialog.setNameFilter(tr("Text (*.res)"));
   dialog.setFileMode(QFileDialog::ExistingFiles);
   dialog.setViewMode(QFileDialog::Detail);
   dialog.setDirectory("/home/minji/ultrascan/results");
	us_grid(data_plot);

	if(dialog.exec())
	{	
		QString fileName = dialog.selectedFiles().first();
		load_gaussian_profile(target, fileName);
		QFileInfo fi;
		fi.setFile(fileName);
		lw_target->insertItem(0, fi.baseName()); 
	}	
	
	QVector	<double> v_x_values;
	v_x_values.clear();
	QVector <double> v_y_values;
	v_y_values.clear();
	
	double temp_y;
	//plot the points for the target spectrum
	for(unsigned int k = 0; k < target.lambda_max - target.lambda_min; k++)
	{
		temp_y = 0;
		v_x_values.push_back(target.lambda_min + k);
      for(int j = 0; j <target.gaussians.size(); j++)
		{
			temp_y += target.gaussians.at(j).amplitude *
           exp(-(pow(v_x_values.at(k) - target.gaussians.at(j).mean, 2.0)
           / (2.0 * pow(target.gaussians.at(j).sigma, 2.0))));
		}
		temp_y *= target.amplitude;
		v_y_values.push_back(temp_y);
	}
	
	QwtPlotCurve* c;
	QPen p;
	p.setColor(Qt::yellow);
	p.setWidth(3);
	c = us_curve(data_plot, target.filename);
	c->setPen(p);
	c->setData(v_x_values, v_y_values);
	data_plot->replot();
   pb_load_basis->setEnabled(true);
}


void US_Spectrum:: load_gaussian_profile(struct WavelengthProfile &profile, const QString &fileName)
{
   QString line;
   QString str1;
   struct Gaussian temp_gauss;
	//number of Gaussians to make the curve
	int order;

	//gets information from a file 
	QFile f(fileName);
	QFileInfo fi;

	fi.setFile(fileName);

	if(f.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QTextStream ts(&f);
		ts.readLine();
		ts.readLine();	
		line = ts.readLine();
		str1 = line.split(":").last();
		order = str1.toInt();
		ts.readLine();
		line = ts.readLine();
		
		//receive scaling parameters from the file
		profile.lambda_scale  = line.split(" ")[3].toInt();
		profile.scale = line.split(" ")[10].toFloat();
		ts.readLine();
		ts.readLine();
		for(int k = 0; k < order; k++)
		{
			ts.readLine();
			ts.readLine();
			temp_gauss.mean = ts.readLine().split(" ")[2].toDouble();
			temp_gauss.amplitude = ts.readLine().split(" ")[3].toDouble();
			temp_gauss.sigma = ts.readLine().split(" ")[2].toDouble();
			profile.gaussians.push_back(temp_gauss);
		}
		find_amplitude(profile);		
		profile.filename = fileName;
		f.close();
	}	

	//Find where the maximum and minimum wavelength in the file is	
	QString fName = fi.dir().path() + "/" + fi.baseName() + ".extinction.dat";
	QFile file(fName);
	if(file.open(QIODevice::ReadOnly))
	{
		QTextStream ts2(&file);
		ts2.readLine();
		profile.lambda_min = ts2.readLine().split("\t")[0].toInt();
		while(!ts2.atEnd())
			str1 = ts2.readLine();
		profile.lambda_max = str1.split("\t")[0].toInt(); 
		file.close();
	}
	else
	{
		QMessageBox mb;
		mb.setWindowTitle(tr("Attention:"));
		mb.setText("Could not read the wavelength data file:\n" + fName);
	}
}
void US_Spectrum::find_amplitude(struct WavelengthProfile &profile)
{
   profile.amplitude = 0;
   for (int j=0; j<profile.gaussians.size(); j++)
   {
      profile.amplitude += profile.gaussians.at(j).amplitude *
         exp(-(pow(profile.lambda_scale - profile.gaussians.at(j).mean, 2.0)
               / (2.0 * pow(profile.gaussians.at(j).sigma, 2.0))));
   }
   profile.amplitude = 1.0/profile.amplitude;
   profile.amplitude *= profile.scale;
}
