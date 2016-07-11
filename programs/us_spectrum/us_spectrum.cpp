#include <QApplication>
#include "us_spectrum.h"
#include <math.h>
#if QT_VERSION < 0x050000
#define setSamples(a,b,c) setData(a,b,c)
#define setSymbol(a) setSymbol(*a)
#endif

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

	solution_curve = NULL;

	//Constantly keeps track of the number of basis vectors
	basisIndex = 0;

	//Push Buttons for US_Spectrum GUI
	pb_load_target = us_pushbutton(tr("Load Target Spectrum"));
	connect(pb_load_target, SIGNAL(clicked()), SLOT(load_target()));
	pb_load_basis = us_pushbutton(tr("Load Basis Spectrum"));
   pb_load_basis->setEnabled(false);
	connect(pb_load_basis, SIGNAL(clicked()), SLOT(load_basis()));
	pb_find_extinction = us_pushbutton(tr("Find Corresponding Extin. Coeff."));
	connect(pb_find_extinction, SIGNAL(clicked()), SLOT(findExtinction()));
	pb_delete = us_pushbutton(tr("Delete Current Basis Scan"));
	connect(pb_delete, SIGNAL(clicked()), SLOT(deleteCurrent()));
	pb_load_fit = us_pushbutton(tr("Load Fit"));
	connect(pb_load_fit, SIGNAL(clicked()), SLOT(load()));
	pb_overlap = us_pushbutton(tr("Find Extinction Profile Overlap"));
	connect(pb_overlap, SIGNAL(clicked()), SLOT(overlap()));
	pb_overlap->setEnabled(false);
	pb_fit = us_pushbutton(tr("Fit Data"));
	connect(pb_fit, SIGNAL(clicked()), SLOT(fit()));
	pb_fit->setEnabled(false);
	pb_find_angles = us_pushbutton(tr("Fit Angles"));
	connect(pb_find_angles, SIGNAL(clicked()), SLOT(findAngles()));
	pb_help = us_pushbutton(tr("Help"));
	pb_reset_basis = us_pushbutton(tr("Reset Basis Spectra"));
	connect(pb_reset_basis, SIGNAL(clicked()), SLOT(resetBasis()));
	pb_save= us_pushbutton(tr("Save Fit"));
	connect(pb_save, SIGNAL(clicked()), SLOT(save()));
	pb_close = us_pushbutton(tr("Close"));
	connect(pb_close, SIGNAL(clicked()), SLOT(close()));

	//List Widgets
	lw_target = us_listwidget();
	lw_basis = us_listwidget();

	//Label Widgets
	lbl_wavelength = us_label(tr("Wavelength:"));
	lbl_extinction = us_label(tr("Extinction Coeff.:"));

	//Line Edit Widgets
	le_angle = us_lineedit("", 1, true);
	le_wavelength = us_lineedit("", 1, false);
	le_extinction = us_lineedit("", 1, true);
	le_rmsd = us_lineedit("RMSD:", 1, false);

	cb_spectrum_type = new QComboBox();
	QStringList spectrum_types;
	spectrum_types << "target" << "basis" << "fitted";
	cb_spectrum_type->addItems(spectrum_types);
	cb_angle_one = new QComboBox();
	cb_angle_two = new QComboBox();

   data_plot = new QwtPlot();
   plotLayout1 = new US_Plot(data_plot, tr(""), tr("Wavelength(nm)"), tr("Extinction"));
   data_plot->setCanvasBackground(Qt::black);
   data_plot->setTitle("Wavelength Spectrum Fit");
	data_plot->setMinimumSize(700,200);
	data_plot->detachItems();
	
   residuals_plot = new QwtPlot();
   plotLayout2 = new US_Plot(residuals_plot, tr(""), tr("Wavelength(nm)"), tr("Extinction"));
   residuals_plot->setCanvasBackground(Qt::black);
   residuals_plot->setTitle("Fitting Residuals");
	residuals_plot->setMinimumSize(700, 200);

   pick = new US_PlotPicker( data_plot );
   pick->setRubberBand( QwtPicker::VLineRubberBand );
   connect( pick, SIGNAL( moved    ( const QwtDoublePoint& ) ),
                  SLOT  ( new_value( const QwtDoublePoint& ) ) );
   connect( pick, SIGNAL( mouseDown( const QwtDoublePoint& ) ),
                  SLOT  ( new_value( const QwtDoublePoint& ) ) );

	QGridLayout* plotGrid;
	plotGrid =  new QGridLayout();
	plotGrid->addLayout(plotLayout1, 0, 0);
	plotGrid->addLayout(plotLayout2, 1, 0);
	
	QGridLayout* angles_layout;
	angles_layout = new QGridLayout();
	angles_layout->addWidget(cb_angle_one, 0, 0);
	angles_layout->addWidget(cb_angle_two, 0, 1);
	angles_layout->addWidget(le_angle, 1, 0);
	angles_layout->addWidget(pb_find_angles, 1, 1);

	QGridLayout* subgl1;
	subgl1 = new QGridLayout();
	subgl1->addWidget(lbl_wavelength, 0,0);
	subgl1->addWidget(le_wavelength, 0,1);
	subgl1->addWidget(lbl_extinction, 1, 0);
	subgl1->addWidget(le_extinction, 1, 1);

	QGridLayout* subgl2;
	subgl2 = new QGridLayout();
   subgl2->addWidget(pb_load_fit, 0, 0);
   subgl2->addWidget(pb_save, 0, 1);
   subgl2->addWidget(pb_help, 1, 0);
   subgl2->addWidget(pb_close, 1, 1);

	QGridLayout* gl1;
	gl1 = new QGridLayout();
	gl1->addWidget(pb_load_target, 0, 0);
	gl1->addWidget(lw_target, 1, 0);
	gl1->addWidget(pb_load_basis, 2, 0);
   gl1->addWidget(lw_basis, 3, 0);
	gl1->addWidget(cb_spectrum_type, 4, 0);
	gl1->addLayout(subgl1, 5, 0);
	gl1->addWidget(pb_find_extinction, 6, 0);
	gl1->addWidget(pb_delete, 7, 0);
   gl1->addWidget(pb_reset_basis, 8, 0);
   gl1->addWidget(pb_overlap, 9, 0);
   gl1->addWidget(pb_fit, 11, 0);
   gl1->addWidget(le_rmsd, 12, 0);
	gl1->addLayout(angles_layout, 13, 0);
	gl1->addLayout(subgl2, 14, 0);
	
	QGridLayout *mainLayout;
	mainLayout = new QGridLayout(this);
	mainLayout->setSpacing(2);
	mainLayout->setContentsMargins(2,2,2,2);
	mainLayout->addLayout(gl1, 0, 0);
	mainLayout->addLayout(plotGrid, 0, 1);
}

//loads basis spectra according to user specification
void US_Spectrum::load_basis()
{
   QStringList files;
	
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
      QFileInfo fi;
      fi.setFile(*it);
		load_gaussian_profile(temp_wp, *it);
		temp_wp.filenameBasis = fi.baseName();
		v_basis.push_back(temp_wp);
	}
	plot_basis();
	pb_overlap->setEnabled(true);
	pb_fit->setEnabled(true);
}

//Takes the information in the basis vector to plot all of the curves for the basis spectrums 
void US_Spectrum::plot_basis()
{
	QStringList names;
   QVector  <double> v_x_values;

	for(int m = basisIndex; m < v_basis.size(); m++)
	{
		names.append(v_basis.at(m).filenameBasis);   
   	v_x_values.clear();
		lw_basis->insertItem(0, v_basis.at(m).filenameBasis);

   	//plot the points for the target spectrum
      for(unsigned int k = 0; k < v_basis.at(m).lambda_max - v_basis.at(m).lambda_min; k++)
      {
         v_x_values.push_back(v_basis.at(m).lambda_min + k);
     	}
      double* xx = (double*)v_x_values.data();
      double* yy = (double*)v_basis.at(m).extinction.data();
      int     nn = v_x_values.size();
		QwtPlotCurve* c;
   	QPen p;
  		p.setColor(Qt::green);
   	p.setWidth(3);
   	c = us_curve(data_plot, v_basis.at(m).filename);
   	c->setPen(p);
   	c->setSamples( xx, yy, nn );
		v_basis[basisIndex].matchingCurve = c;
		basisIndex++;
	}
	cb_angle_one->addItems(names);
	cb_angle_two->addItems(names);	
	data_plot->replot();
}

//brings in the target spectrum according to user specification
void US_Spectrum::load_target()
{
   QFileDialog dialog (this);
	QString fileName = "";
   dialog.setNameFilter(tr("Text (*.res)"));
   dialog.setFileMode(QFileDialog::ExistingFiles);
   dialog.setViewMode(QFileDialog::Detail);
   dialog.setDirectory("/home/minji/ultrascan/results");
	us_grid(data_plot);
	
	//reset for a new target spectrum to be loaded
	if(lw_target->count() > 0)
	{
		lw_target->clear();
		w_target.gaussians.clear();
		w_target.extinction.clear();
		w_target.matchingCurve->detach();
	}
	if(dialog.exec())
	{
			fileName = dialog.selectedFiles().first();
			load_gaussian_profile(w_target, fileName);
			QFileInfo fi;
			fi.setFile(fileName);
      	w_target.filenameBasis = fi.baseName();
	}
   if(fileName.contains(".res"))
   {
		plot_target();
   }

}

void US_Spectrum:: plot_target()
{	
	QVector	<double> v_x_values;
	v_x_values.clear();
	
	//plot the points for the target spectrum
	for(unsigned int k = 0; k < w_target.lambda_max - w_target.lambda_min; k++)
	{
		v_x_values.push_back(w_target.lambda_min + k);
	}
	
   double* xx = (double*)v_x_values.data();
   double* yy = (double*)w_target.extinction.data();
   int     nn = v_x_values.size();
	QwtPlotCurve* c;
	QPen p;
	p.setColor(Qt::yellow);
	p.setWidth(3);
	c = us_curve(data_plot, w_target.filename);
	c->setPen(p);
	c->setSamples( xx, yy, nn );
	w_target.matchingCurve = c;
	data_plot->replot();
   lw_target->insertItem(0, w_target.filenameBasis);
   pb_load_basis->setEnabled(true);
}

//reads in gaussian information for each spectrum
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
		str1 = ts2.readLine();
		profile.lambda_min = str1.split("\t")[0].toInt();
		profile.extinction.push_back(str1.split("\t")[1].toDouble());
		while(!ts2.atEnd())
		{
			str1 = ts2.readLine();
			profile.extinction.push_back(str1.split("\t")[1].toDouble());
		}
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

//Find the appropriate amplitude to scale the spectrum's curve at
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

void US_Spectrum::new_value(const QwtDoublePoint& p)
{
	unsigned int specified_wavelength = 0;
	specified_wavelength = (unsigned int)p.x();
	le_wavelength->setText(QString::number(specified_wavelength));	

   if(cb_spectrum_type->currentText().compare("basis") == 0)
   {
      int basisIndex = 0;
      for(int m = 0; m < v_basis.size(); m++)
      {
			if(lw_basis->currentItem() == NULL)
				return;
         if(v_basis.at(m).filenameBasis.compare(lw_basis->currentItem()->text()) == 0)
         {
            basisIndex = m;
         }
      }
      if(specified_wavelength < v_basis.at(basisIndex).lambda_max && specified_wavelength > v_basis.at(basisIndex).lambda_min)
         le_extinction->setText(QString::number(v_basis.at(basisIndex).extinction.at(specified_wavelength - v_basis.at(basisIndex).lambda_min)));
   }
   else if (cb_spectrum_type->currentText().compare("target") == 0)
   {
		if(specified_wavelength < w_target.lambda_max && specified_wavelength > w_target.lambda_min)
    		le_extinction->setText(QString::number(w_target.extinction.at(specified_wavelength - w_target.lambda_min)));   
	}
   else
   {
      if(specified_wavelength < w_solution.lambda_max && specified_wavelength > w_solution.lambda_min)
         le_extinction->setText(QString::number(w_solution.extinction.at(specified_wavelength - w_solution.lambda_min)));

   }
}
void US_Spectrum::findExtinction()
{
	unsigned int specified_wavelength = 0;	
	specified_wavelength = atoi(le_wavelength->text().toStdString().c_str());
	if(cb_spectrum_type->currentText().compare("basis") == 0)
	{
		int basisIndex = 0;
		for(int m = 0; m < v_basis.size(); m++)
   	{
      	if(v_basis.at(m).filenameBasis.compare(lw_basis->currentItem()->text()) == 0)
   		{
				basisIndex = m;
			}
		}
		if(specified_wavelength < v_basis.at(basisIndex).lambda_max && specified_wavelength > v_basis.at(basisIndex).lambda_min)
			le_extinction->setText(QString::number(v_basis.at(basisIndex).extinction.at(specified_wavelength - v_basis.at(basisIndex).lambda_min))); 	
	}
	else if (cb_spectrum_type->currentText().compare("target") == 0)
	{
		if(specified_wavelength < w_target.lambda_max && specified_wavelength > w_target.lambda_min)
         le_extinction->setText(QString::number( w_target.extinction.at(specified_wavelength - w_target.lambda_min)));
	}
	else
	{
		if(specified_wavelength < w_solution.lambda_max && specified_wavelength > w_solution.lambda_min)
         le_extinction->setText(QString::number(w_solution.extinction.at(specified_wavelength - w_solution.lambda_min)));

	}	
}
void US_Spectrum::fit()
{
   unsigned int min_lambda = w_target.lambda_min;
   unsigned int max_lambda = w_target.lambda_max;
   unsigned int points, order, i, j, k, counter=0;
   double *nnls_a, *nnls_b, *nnls_x, nnls_rnorm, *nnls_wp, *nnls_zzp, *x, *y;
   float fval = 0.0;
   QVector <float> residuals, solution, b;
   QPen pen;
   residuals.clear();
   solution.clear();
   b.clear();
   int *nnls_indexp;
   QString str = "Please note:\n\n" 
      "The target and basic spectra have different limits.\n" 
      "These vectors need to be congruent before you can fit\n" 
      "the data. You can correct the problem by first running\n" 
      "\"Find Extinction Profile Overlap\" (preferred), or by\n" 
      "running \"Extrapolate Extinction Profile\" (possibly imprecise).";
   
	for (i=0; i< (unsigned int) v_basis.size(); i++)
   {
      if(v_basis[i].lambda_min != min_lambda || v_basis[i].lambda_max != max_lambda)
      {
         QMessageBox::warning(this, tr("UltraScan Warning"), str,
    		QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
         return;
      }
   }

   points = w_target.lambda_max - w_target.lambda_min + 1;
   x = new double [points];
   y = new double [points];
   order = v_basis.size(); // no baseline necessary with gaussians
   nnls_a = new double [points * order]; // contains the model functions, end-to-end
   nnls_b = new double [points]; // contains the experimental data
   nnls_zzp = new double [points]; // pre-allocated working space for nnls
   nnls_x = new double [order]; // the solution vector, pre-allocated for nnls
   nnls_wp = new double [order]; // pre-allocated working space for nnls, On exit, wp[] will contain the dual solution vector, wp[i]=0.0 for all i in set p and wp[i]<=0.0 for all i in set z. 

	nnls_indexp = new int [order];
	//find_amplitude(target);
   for (i=0; i<points; i++)
   {
      x[i] = w_target.lambda_min + i;
      nnls_b[i] = 0.0;
      for (j=0; j < (unsigned int)w_target.gaussians.size(); j++)
      {
         nnls_b[i] += w_target.gaussians[j].amplitude *
            exp(-(pow(x[i] - w_target.gaussians[j].mean, 2.0)
                  / (2.0 * pow(w_target.gaussians[j].sigma, 2.0))));
      }
      nnls_b[i] *= w_target.amplitude;
      b.push_back((float) nnls_b[i]);
   }
   counter = 0;
   for (k=0; k<order; k++)
   {		
		//find amplitude(basis[k])
  		for(i = 0; i<points; i++)
      {
         x[i] = v_basis[k].lambda_min + i;
         nnls_a[counter] = 0.0;
         for (j=0; j< (unsigned int) v_basis[k].gaussians.size(); j++)
         {
            nnls_a[counter] += v_basis[k].gaussians[j].amplitude *
               exp(-(pow(x[i] - v_basis[k].gaussians[j].mean, 2.0)
                     / (2.0 * pow(v_basis[k].gaussians[j].sigma, 2.0))));
         }
         nnls_a[counter] *= v_basis[k].amplitude;
         counter ++;
      }
   }
   US_Math2::nnls(nnls_a, points, points, order, nnls_b, nnls_x, &nnls_rnorm, nnls_wp, nnls_zzp, nnls_indexp);
	
	QVector <float> results;
   results.clear();
   fval = 0.0;
   for (i=0; i< (unsigned int) v_basis.size(); i++)
   {
      fval += nnls_x[i];
   }
   for (i=0; i< (unsigned int) v_basis.size(); i++)
   {
      results.push_back(100.0 * nnls_x[i]/fval);
      str.sprintf((v_basis[i].filenameBasis +": %3.2f%% (%6.4e)").toLocal8Bit().data(), results[i], nnls_x[i]);
		lw_basis->item((int)i)->setText(str);
      v_basis[i].nnls_factor = nnls_x[i];
      v_basis[i].nnls_percentage = results[i];
   }

   for (i=0; i<points; i++)
   {
      solution.push_back(0.0);
   }
   for (k=0; k<order; k++)
   {
      //find_amplitude(basis[k]);
      for (i=0; i<points; i++)
      {
         x[i] = v_basis[k].lambda_min + i;
      
      	for (j=0; j< (unsigned int) v_basis[k].gaussians.size(); j++)
         {
      	   solution[i] += (v_basis[k].gaussians[j].amplitude *                                         exp(-(pow(x[i] - v_basis[k].gaussians[j].mean, 2.0) / (2.0 * pow(v_basis[k].gaussians[j].sigma, 2.0))))										) * v_basis[k].amplitude * nnls_x[k];
         }
      }
    }
   for (i=0; i<points; i++)
   {
      residuals.push_back(solution[i] - b[i]);
      y[i] = solution[i];
	}
   
	residuals_plot->detachItems();                                                 
   QwtPlotCurve *resid_curve = us_curve(residuals_plot, "Residuals");
   QwtPlotCurve *target_curve = us_curve(residuals_plot,"Mean");
   if (solution_curve != NULL)
   {
      solution_curve->detach();
   }
   solution_curve = us_curve(data_plot, "Solution");

   resid_curve->setStyle(QwtPlotCurve::Lines);
   target_curve->setStyle(QwtPlotCurve::Lines);
   solution_curve->setStyle(QwtPlotCurve::Lines);

   solution_curve->setSamples(x, y, points);
   pen.setColor(Qt::magenta);
   pen.setWidth(3);
   solution_curve->setPen(pen);
	//Update w_solution's profile
	w_solution.matchingCurve = solution_curve;
   data_plot->replot();

	for(unsigned int j = 0; j < points; j++)
	{
		w_solution.extinction.push_back(y[j]);
	}
	w_solution.lambda_min = w_target.lambda_min;
	w_solution.lambda_max = w_target.lambda_max;
   fval = 0.0;
   for (i=0; i<points; i++)
   {
      y[i] = residuals[i];
      fval += pow(residuals[i], (float) 2.0);
   }
	fval /= points;
   le_rmsd->setText(str.sprintf("RMSD: %3.2e", pow(fval, (float) 0.5)));
   resid_curve->setSamples(x, y, points);
   pen.setColor(Qt::yellow);
   pen.setWidth(2);
   resid_curve->setPen(pen);
   residuals_plot->replot();

   x[1] = x[points - 1];
   y[0] = 0.0;
   y[1] = 0.0;
   target_curve->setSamples(x, y, 2);
   pen.setColor(Qt::red);
   pen.setWidth(3);
   target_curve->setPen(pen);
   residuals_plot->replot();
   pb_save->setEnabled(true);
   delete [] x;
   delete [] y;
}

void US_Spectrum::deleteCurrent()
{
	int deleteIndex = 0;
	basisIndex--;

	for(int m = 0; m < v_basis.size(); m++)
	{
		if(v_basis.at(m).filenameBasis.compare(lw_basis->currentItem()->text()) == 0)
			deleteIndex = m;
	}	
	v_basis[deleteIndex].matchingCurve->detach();
	data_plot->replot();
	v_basis.remove(deleteIndex);
	delete lw_basis->currentItem();
}
void US_Spectrum::resetBasis()
{
	basisIndex = 0;
	cb_angle_one->clear();
	cb_angle_two->clear();
	le_angle->clear();
	for(int k = 0; k < v_basis.size(); k++)
	{
		v_basis[k].matchingCurve->detach();
	}
   v_basis.clear();
	//delete the solution curve
	if(solution_curve != NULL)
	{	
		solution_curve->detach();
		solution_curve = NULL;
	}
	//clear the residuals plot
	residuals_plot->detachItems();
	residuals_plot->replot();
	data_plot->replot();
	lw_basis->clear();
	le_rmsd->clear();
	le_rmsd->setText(QString("RMSD"));
}

void US_Spectrum::overlap()
{
	unsigned int highest_lambda_min = 0, lowest_lambda_max = 100000;

	//Find the highest minimum wavelength and largest maximum wavelength to
	//determine where to plot	
	for(int m = 0; m < v_basis.size(); m++)
	{
		if(v_basis.at(m).lambda_min > highest_lambda_min)
			highest_lambda_min = v_basis.at(m).lambda_min;
      if(v_basis.at(m).lambda_max < lowest_lambda_max)
         lowest_lambda_max = v_basis.at(m).lambda_max;
	}
	
	if(highest_lambda_min < w_target.lambda_min)
		highest_lambda_min = w_target.lambda_min;
	else
		w_target.lambda_min = highest_lambda_min;
	
	if(lowest_lambda_max > w_target.lambda_max)
		lowest_lambda_max = w_target.lambda_max;
	else
		w_target.lambda_max = lowest_lambda_max;

	lw_target->clear();
	w_target.matchingCurve->detach();
	
	//change lambda_min to match the highest of all the profiles
	//change lambda_max to match the lowest of all the profiles
	for(int k = 0; k < v_basis.size(); k++)
	{
		if(v_basis.at(k).lambda_max >lowest_lambda_max)
			v_basis[k].lambda_max = lowest_lambda_max;
		if(v_basis.at(k).lambda_min < highest_lambda_min)
			v_basis[k].lambda_min = highest_lambda_min;
		v_basis[k].matchingCurve->detach();	
	}

	//Clear components of the basis so replotting will work properly
	lw_basis->clear();
	basisIndex = 0;

	plot_basis();
	plot_target();
}

void US_Spectrum::findAngles()
{
	QString firstProf = cb_angle_one->currentText();
	QString secondProf = cb_angle_two->currentText();
	int indexOne = 0;
	int indexTwo = 0;
	double dotproduct = 0.0, vlength_one = 0.0, vlength_two = 0.0, angle;

	//Find the two basis vectors that the user selected
	for(int k = 0; k < v_basis.size(); k++)
	{	
		if(firstProf.compare(v_basis[k].filenameBasis) == 0)
			indexOne = k;
		if(secondProf.compare(v_basis[k].filenameBasis) == 0)
			indexTwo = k;
	}

	//Calculate the angle measure between the two 
	for(int i = 0; i < v_basis[indexOne].extinction.size(); i++)
	{
		dotproduct += v_basis[indexOne].extinction[i] * v_basis[indexTwo].extinction[i];
		vlength_one += pow(v_basis[indexOne].extinction[i], 2);
		vlength_two += pow(v_basis[indexTwo].extinction[i], 2);
	}
	angle = dotproduct/(pow(vlength_one, 0.5) * pow(vlength_two, 0.5));
	angle =180 * ((acos(angle))/M_PI);
	le_angle->setText(QString::number(angle));	
}

void US_Spectrum::save()
{
	QString filename = QFileDialog::getSaveFileName(this, "Save File", "/home/minji/ultrascan/results", "*.spectrum_fit");
	if(filename.isEmpty())
		return;

	//Ensures that user did not add their own extension	
	if(!(filename.lastIndexOf(".", -1) == -1))
	{
		filename = filename.left(1 + filename.lastIndexOf(".", -1));
	}
	
	filename = filename + ".spectrum_fit";
	QFile f (filename); 	
	if(f.open(QIODevice::WriteOnly))
	{
		QDataStream ds(&f);
		ds << w_target.amplitude;
      ds << w_target.filename;
		ds << w_target.filenameBasis;
      ds << w_target.lambda_min;
      ds << w_target.lambda_max;
      ds << w_target.lambda_scale;
      ds << w_target.scale;
      ds << w_target.gaussians.size();
      for (int i=0; i< w_target.gaussians.size(); i++)
      {
         ds << w_target.gaussians[i].amplitude;
         ds << w_target.gaussians[i].sigma;
         ds << w_target.gaussians[i].mean;
      }
      ds << v_basis.size();
      for (int j=0; j< v_basis.size(); j++)
      {
         ds << v_basis[j].amplitude;
         ds << v_basis[j].filename;
			ds << v_basis[j].filenameBasis;
         ds << v_basis[j].lambda_min;
         ds << v_basis[j].lambda_max;
         ds << v_basis[j].lambda_scale;
         ds << v_basis[j].scale;
         ds << v_basis[j].nnls_factor;
         ds << v_basis[j].nnls_percentage;
         ds << v_basis[j].gaussians.size();
         for (int i=0; i< v_basis[j].gaussians.size(); i++)
         {
            ds << v_basis[j].gaussians[i].amplitude;
            ds << v_basis[j].gaussians[i].sigma;
            ds << v_basis[j].gaussians[i].mean;
         }
		}
		f.close();
	}
}
	
void US_Spectrum::load()
{
	//Empty anything that may still contain information about previous curves
	resetBasis();
   lw_target->clear();
   if(lw_target->count() > 0)
	{
		w_target.matchingCurve->detach();
		w_target.gaussians.clear();
	}
	
	//Load the file that the user selected
   QString filename = QFileDialog::getOpenFileName(this, "Load File", "/home/minji/ultrascan/results", "*.spectrum_fit");
   if(filename.isEmpty())
      return;
	
	//holds the number of gaussians
	int tempSize = 0;

	//reload information for the target and basis curves
   QFile f (filename);
   if(f.open(QIODevice::ReadOnly))
   {
      QDataStream ds(&f);
      ds >> w_target.amplitude;
      ds >> w_target.filename;
		ds >> w_target.filenameBasis;
      ds >> w_target.lambda_min;
      ds >> w_target.lambda_max;
      ds >> w_target.lambda_scale;
      ds >> w_target.scale;
      ds >> tempSize;
     	w_target.gaussians.resize(tempSize);
 
		for (int i=0; i< w_target.gaussians.size(); i++)
      {
         ds >> w_target.gaussians[i].amplitude;
         ds >> w_target.gaussians[i].sigma;
         ds >> w_target.gaussians[i].mean;
      }

      ds >> tempSize;
		v_basis.resize(tempSize);

      for (int j=0; j< v_basis.size(); j++)
      {
         ds >> v_basis[j].amplitude;
         ds >> v_basis[j].filename;
			ds >> v_basis[j].filenameBasis;
         ds >> v_basis[j].lambda_min;
         ds >> v_basis[j].lambda_max;
         ds >> v_basis[j].lambda_scale;
         ds >> v_basis[j].scale;
         ds >> v_basis[j].nnls_factor;
         ds >> v_basis[j].nnls_percentage;
         ds >> tempSize;
			v_basis[j].gaussians.resize(tempSize);

         for (int i=0; i< v_basis[j].gaussians.size(); i++)
         {
            ds >> v_basis[j].gaussians[i].amplitude;
            ds >> v_basis[j].gaussians[i].sigma;
            ds >> v_basis[j].gaussians[i].mean;
         }
      }
      f.close();
   }
	plot_basis();
	plot_target();
	fit();
}
