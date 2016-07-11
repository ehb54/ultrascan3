//! \file us_extinction.cpp
//
#include <QApplication>
#include "us_extinction.h"
#include "us_license_t.h"
#include "us_license.h"
#if QT_VERSION < 0x050000
#define setSamples(a,b,c) setData(a,b,c)
#define setSymbol(a) setSymbol(*a)
#endif
//
//! \brief Main program for US_EXTINCTION. Loads translators and starts
//         the class US_EXTINCTION

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.

   US_Extinction  w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

// Constructor
US_Extinction::US_Extinction() : US_Widgets()
{
	disk_controls = new US_Disk_DB_Controls(0);
	//default values for limits on the graph
	lambdaLimitLeft = 200.0;
   lambdaLimitRight = 1500.0;
 	lambda_min = 1000;
	lambda_max = -1000;
	odCutoff = 3.0;
	
	//length of cuvette
	pathlength = (float) 1.2;
	
	//number of Gaussians to fit
	order = 15;

	//whether data has been fitted
	fitted = false;

	//Connect all the buttons on the GUI to their proper functions
	v_wavelength.clear();
	pb_addWavelength = us_pushbutton( tr( "Add Wavelength Scanfile") );
	connect( pb_addWavelength, SIGNAL( clicked()), SLOT(add_wavelength()));
  	pb_reset = us_pushbutton( tr( "Reset Scanlist") );
        connect( pb_reset, SIGNAL( clicked()), SLOT(reset_scanlist()));
	pb_update = us_pushbutton( tr( "Update Data Plot"));
	connect( pb_update, SIGNAL( clicked()), SLOT(update_data()));
	pb_perform = us_pushbutton( tr( "Perform Global Fit") );
	connect( pb_perform, SIGNAL( clicked()), SLOT(perform_global()));
	pb_calculate = us_pushbutton( tr( "Calculate E280 from Peptide File") );
	connect( pb_calculate, SIGNAL( clicked()), SLOT(calculateE280()));
	pb_save = us_pushbutton( tr( "Save") );
	connect( pb_save, SIGNAL( clicked()), SLOT(save()));
	pb_view = us_pushbutton( tr( "View Result File") );
	connect( pb_view, SIGNAL( clicked()), SLOT(view_result()));
	pb_help = us_pushbutton( tr( "Help") );
	connect( pb_help, SIGNAL( clicked()), SLOT(help()));
	pb_close = us_pushbutton( tr( "Close") );
	connect( pb_close, SIGNAL( clicked()), SLOT(close()));
	
	lbl_peptide = us_banner(tr("Peptide Information"));
	lbl_wvinfo = us_banner(tr("Wavelength Information:"));
	lbl_associate = us_label(tr("Associate with Run:"));
	lbl_gaussians = us_label(tr("# of Gaussians: "));
	lbl_cutoff = us_label(tr("OD Cutoff:"));
   lbl_lambda1 = us_label(tr("Lower Lambda Limit"));
   lbl_lambda2 = us_label(tr("Upper Lambda Limit "));
	lbl_pathlength = us_label(tr("Pathlength:"));
	lbl_coefficient = us_label(tr("Extinction Coeff.:"));
	lw_file_names = us_listwidget();
	connect(lw_file_names, SIGNAL(itemSelectionChanged()), SLOT(listToCurve()));
	connect(lw_file_names, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(deleteCurve()));
	
	le_associate = us_lineedit("Simulation",1, false);
	le_odCutoff = us_lineedit("3.000", 1, false);
	le_lambdaLimitLeft = us_lineedit("200.0", 1, false);
   le_lambdaLimitRight = us_lineedit("1500.0",1, false);
   le_pathlength = us_lineedit("1.2000", 1, false);
	le_coefficient = us_lineedit("1.0000",1, false);

	ct_gaussian = us_counter(2, 1, 50, 15);
	ct_gaussian->setSingleStep(1);
	ct_gaussian->setEnabled(true);
	connect(ct_gaussian, SIGNAL(valueChanged(double)), SLOT(update_order(double)));

	ct_coefficient = us_counter(2, 200, 1500, 280);
	ct_coefficient->setSingleStep(1);
	ct_coefficient->setEnabled(true);

	data_plot = new QwtPlot();
	changedCurve = NULL;
	plotLayout = new US_Plot(data_plot, tr(""), tr("Wavelength(nm)"), tr("Optical Density"));
	data_plot->setCanvasBackground(Qt::black);
	data_plot->setTitle("Absorbance and Extinction Profile");
	data_plot->setMinimumSize(560, 240);
	data_plot->enableAxis(1, true);
	data_plot->setAxisTitle(1, "Extinction OD/(mol*cm)");
	
	QGridLayout* gl1;
	gl1 = new QGridLayout();
	gl1->addWidget(lbl_associate, 0, 0);
	gl1->addWidget(le_associate, 0, 1);	
		
	QGridLayout* gl2;
	gl2 = new QGridLayout();
	gl2->addWidget(lbl_gaussians, 0, 0);
	gl2->addWidget(ct_gaussian, 0, 1);
        gl2->addWidget(lbl_cutoff, 1, 0);
        gl2->addWidget(le_odCutoff, 1, 1);
        gl2->addWidget(lbl_lambda1, 2, 0);
	gl2->addWidget(ct_gaussian, 0, 1);
   gl2->addWidget(lbl_cutoff, 1, 0);
   gl2->addWidget(le_odCutoff, 1, 1);
   gl2->addWidget(lbl_lambda1, 2, 0);
   gl2->addWidget(le_lambdaLimitLeft, 2, 1);
   gl2->addWidget(lbl_lambda2, 3, 0);
   gl2->addWidget(le_lambdaLimitRight, 3, 1);
   gl2->addWidget(lbl_pathlength, 4, 0);
   gl2->addWidget(le_pathlength, 4, 1);

	QGridLayout* gl3;
	gl3 = new QGridLayout();
	gl3->addWidget(lbl_coefficient, 0, 0);
	gl3->addWidget(ct_coefficient, 0, 1);
	gl3->addWidget(le_coefficient, 1, 1);

	QGridLayout* submain;
	submain = new QGridLayout();
	submain-> setColumnStretch(0,1);
	submain->addWidget(lbl_wvinfo, 0,0);
	submain->addWidget(pb_addWavelength, 1, 0);
	submain->addWidget(lw_file_names, 2, 0);
	submain->addLayout(gl1, 3, 0);
	submain->addWidget(pb_reset, 4, 0);
   submain->addWidget(pb_update, 5, 0);
   submain->addWidget(pb_perform, 6, 0);
	submain->addLayout(gl2, 7, 0);
   submain->addWidget(lbl_peptide, 8, 0);
	submain->addWidget(pb_calculate, 9, 0);
	submain->addLayout(gl3, 10, 0);
   submain->addWidget(pb_save, 11, 0);
   submain->addWidget(pb_view, 12, 0);
   submain->addWidget(pb_help, 14, 0);
   submain->addWidget(pb_close, 15, 0);
	
	QGridLayout* main;
	main = new QGridLayout(this);
   main->setSpacing(2);
   main->setContentsMargins(2,2,2,2);
	main->addLayout(submain, 0, 0);
	main->addLayout(plotLayout, 0, 1);
}

void US_Extinction::add_wavelength(void)
{
	QStringList files;
	QFile f;

	QFileDialog dialog (this);
   dialog.setNameFilter(tr("Text (*.txt *.csv *.dat *.wa)"));
	dialog.setFileMode(QFileDialog::ExistingFiles);
   dialog.setViewMode(QFileDialog::Detail);
	dialog.setDirectory("/home/minji/ultrascan/data/SpectralData");
	if(dialog.exec())
	{
		files = dialog.selectedFiles();
		reading(files);
	}
}
void US_Extinction::reading(QStringList sl)
{
	QString fileName, str, str1, extension;
	int position;
	for (QStringList::const_iterator  it=sl.begin(); it!=sl.end(); ++it)
	{
		fileName = *it;
		position = fileName.lastIndexOf(".");
		str = fileName.mid(position+2, 1);
		if(str.compare ("i") == 0 || str.compare ("I") == 0)
		{
			lambdaLimitLeft = 200.0;
        	odCutoff = 1.0e5;
         le_lambdaLimitLeft->setText(str.sprintf(" %2.3f", lambdaLimitLeft));
        	le_odCutoff->setText(str.sprintf(" %2.3e", odCutoff));
         data_plot->setTitle(tr("Intensity Profile"));
		}
		if(!fileName.isEmpty())
     	{
         		filenames.push_back(fileName);
        			loadScan(fileName);
      }
	} 
	le_lambdaLimitLeft->setText(str1.sprintf(" %2.1f", lambda_min));
	le_lambdaLimitRight->setText(str1.sprintf(" %2.1f", lambda_max));
	plot();
}
bool US_Extinction::isComment(const QString &str)
{
	QString teststr = str, str1, str2;
	QStringList holder;
	teststr = teststr.simplified();
	teststr = teststr.replace("\"", " ");
   teststr = teststr.replace(",", " ");
	holder = teststr.split(" ");
	str1 = holder.at(0);
	str2 = holder.at(1);
	if(str1.toFloat() && str2.toFloat())
	{
		return(false);
	}
	else
	{
		return(true);
	}
}
bool US_Extinction::loadScan(const QString &fileName)
{
	QString str1;
	QStringList strl;
	float temp_x, temp_y;
	WavelengthScan wls;
	QFile f(fileName);
	wls.v_readings.clear();
	QFileInfo fi(fileName);
	wls.filePath = fi.filePath();
	wls.fileName = fi.fileName();
	//reads in files until the end of the file
	if(f.open(QIODevice::ReadOnly | QIODevice::Text))
	{		
      int row = 0;
		QTextStream ts(&f);
		wls.description = ts.readLine();
		while(!ts.atEnd())
		{
			bool flag1 = true;
	
			 //true while text information is not a number
			if(flag1 && !ts.atEnd())
			{
				str1 = ts.readLine();
			}
			else
			{
				flag1 = false;
			}
			str1 = str1.simplified();
			str1 = str1.replace("\"", " ");
			str1 = str1.replace(",", " ");
			strl = str1.split(" ");
			temp_x = strl.at(0).toFloat();
			temp_y = strl.at(1).toFloat();
			if(temp_x >= lambdaLimitLeft && temp_y <= odCutoff && temp_x <= lambdaLimitRight)
			{	
				Reading r = {temp_x, temp_y};
				wls.v_readings.push_back(r);
				lambda_max = max(temp_x, lambda_max);
				lambda_min = min (temp_x, lambda_min);		
			}
		}
		f.close();
		if(wls.v_readings.size() < 10)
		{
			//QMessageBox msg1 = US_LongMessageBox("Ultrascan Error", "This wavelength doesn't have\n enough usable points - scan not loaded", this);
		}
		if(wls.v_readings.at(0).lambda > wls.v_readings.at(wls.v_readings.size() - 1).lambda)
		{//we need to reverse the order of entries
			WavelengthScan wls2;
			wls2.v_readings.clear();
			for(int i=(wls.v_readings.size() - (unsigned int) 1); i >=0; i--)
			{
				Reading temp = {wls.v_readings.at(i).lambda, wls.v_readings.at(i).od};
				wls2.v_readings.push_back(temp);
			}
			wls.v_readings.clear();
			for(unsigned int i = 0; i < (unsigned int)wls2.v_readings.size(); i++)
			{
				Reading temp2 = {wls2.v_readings.at(i).lambda, wls.v_readings.at(i).od};
				wls.v_readings.push_back(temp2);
			}
		}
		v_wavelength.push_back(wls);
		str1.sprintf("Scan %d: ", v_wavelength.size());
		str1 += wls.fileName + ", ";
		str1 += wls.description;
		lw_file_names->insertItem(row, str1);
		row++;
		str1 = "";
	}
	else
	{
		//QMessageBox msg2 = us_longmessagebox("Ultrascan Error:", "The wavelength file\n" + fileName + "\ncannot be read.\n Please check to make sure that you have\n read access to this file.", this);
	}
	return(true);
}
void US_Extinction::plot()
{
	QVector<QVector<double> > x_plot, y_plot;
	x_plot.clear();
	y_plot.clear();
	v_curve.clear();
	QString str, title;
	double xmax = 0.0;
	double xmin = 10000.0;
	if(v_wavelength.size() == 0)
	{	
		return;
	}
	data_plot->detachItems();
	us_grid(data_plot);
	
	for(int i = 0; i < v_wavelength.size(); i++)
	{
		xmax = max(xmax, (double) v_wavelength.at(i).v_readings.at(v_wavelength.at(i).v_readings.size() - 1).lambda);
		xmin = min(xmin, (double) v_wavelength.at(i).v_readings.at(0).lambda);
	}

	//Put data into vectors
	for(int j = 0; j < v_wavelength.size(); j++)
	{
		QVector<double> xtemp;
		QVector<double> ytemp;
		xtemp.clear();
		ytemp.clear();
		for(int i = 0; i < v_wavelength.at(j).v_readings.size(); i++)
		{
			xtemp.push_back((double)v_wavelength.at(j).v_readings.at(i).lambda);
			ytemp.push_back((double)v_wavelength.at(j).v_readings.at(i).od);
		}	
		x_plot.push_back(xtemp);
		y_plot.push_back(ytemp);
	}
	for(int m = 0; m < x_plot.size(); m++)
	{
		QwtPlotCurve* c;
		QwtSymbol *s = new QwtSymbol;
		s->setStyle(QwtSymbol::Ellipse);
		s->setPen(QPen(Qt::blue));
		s->setBrush(QBrush(Qt::yellow));
		s->setSize(10);
		title = v_wavelength.at(m).fileName;
		c = us_curve(data_plot, title);
		c->setSymbol(s);
		c->setPen(QPen(Qt::green));
      double* xx = (double*)x_plot.at(m).data();
      double* yy = (double*)y_plot.at(m).data();
      c->setSamples( xx, yy, x_plot.at(m).size() );
		v_curve.push_back(c);
	}
	if(fitted)
	{
		calc_extinction();
      QwtPlotCurve* fit_curve;
		fit_curve = us_curve(data_plot, "Extinction");
      fit_curve->setPen(QPen(Qt::red, 2, Qt::SolidLine));
      double* ll = (double*)lambda.data();
      double* ee = (double*)extinction.data();
      fit_curve->setSamples( ll, ee, lambda.size());
		fit_curve->setYAxis(QwtPlot::yRight);
	}

	data_plot->replot();
	return;
}
void US_Extinction::reset_scanlist(void)
{
	v_wavelength.clear();
	lw_file_names->clear();
	filenames.clear();
	changedCurve = NULL;
	fitted = false;
	v_curve.clear();
	data_plot->detachItems();
	data_plot->replot();
}
void US_Extinction::update_data(void)
{
	fitted = false;
	pathlength = le_pathlength->text().toFloat();
	float minimum = le_lambdaLimitLeft->text().toFloat(), maximum = le_lambdaLimitRight->text().toFloat(), odLimit = le_odCutoff->text().toFloat();
	//Deletes points that are over the cutoffs set by the user
	for(int i = 0; i < v_wavelength.size(); i++)
	{
		for(int j = 0; j < v_wavelength.at(i).v_readings.size(); j++)
		{
			if(v_wavelength.at(i).v_readings.at(j).lambda > maximum)
				v_wavelength[i].v_readings.remove(j, v_wavelength.at(i).v_readings.size() - j);					
			if(v_wavelength.at(i).v_readings.at(j).lambda == minimum)
				v_wavelength[i].v_readings.remove(0, j + 1);
			if(v_wavelength.at(i).v_readings.at(j).od > odLimit)
			{
				v_wavelength[i].v_readings.remove(j);
				j--;
			}
		}
	}
	plot();
}

void US_Extinction::listToCurve(void)
{
	QString selectedName = lw_file_names->currentItem()->text();
	QwtPlotCurve* c_select;
	c_select = NULL;
	QwtSymbol *s_old = new QwtSymbol;
   s_old->setStyle(QwtSymbol::Ellipse);
   s_old->setPen(QPen(Qt::blue));
   s_old->setBrush(QBrush(Qt::yellow));
   s_old->setSize(10);
	QwtSymbol *s_new = new QwtSymbol;
   s_new->setStyle(QwtSymbol::Triangle);
   s_new->setPen(QPen(Qt::black));
   s_new->setBrush(QBrush(Qt::red));
   s_new->setSize(10);

	foreach(QwtPlotCurve* c, v_curve)
	{
		if(selectedName.contains(c->title().text()))
		{
			c_select = c;
		}
	}
	if(c_select == NULL)
	{
		return;
	}
	c_select->setSymbol(s_new);
	c_select->setPen(QPen(Qt::cyan));
	if(changedCurve != NULL)	
	{
		changedCurve->setPen(QPen(Qt::green));
		changedCurve->setSymbol(s_old);
	}
	changedCurve = c_select;
	data_plot->replot();
}

bool US_Extinction::deleteCurve(void)
{
	QMessageBox mBox;
	mBox.setText(tr("Are you sure you want to delete the curve you double-clicked?"));
	mBox.addButton(tr("Yes"), QMessageBox::AcceptRole);
	QPushButton *cancelButton = mBox.addButton(tr("Cancel"), QMessageBox::RejectRole);

	mBox.exec();

	if (mBox.clickedButton() == cancelButton)
	{
		return(false);
	}
	if(v_wavelength.size() <= 1)
	{	
		reset_scanlist();
		return(true);
	}
	QString selectedName = lw_file_names->currentItem()->text();
	for(int k = 0; k < v_wavelength.size(); k++)
	{
		if(selectedName.contains(v_wavelength.at(k).description))
		{
			v_wavelength.remove(k);
		}
	}
	changedCurve = NULL;
	plot();
	delete lw_file_names->currentItem();
	return(true);
}

void US_Extinction::perform_global(void)
{
	if (v_wavelength.size() < 2)
   {
      QMessageBox message;
		message.setWindowTitle(tr("Ultrascan Error:"));
		message.setText(tr("You will need at least 2 scans\nto perform a global fit.\n\nPlease add more scans before attempting\na global fit."));
		message.exec();
      return;
   }
   fitting_widget = false;
   parameters = order * 3 + v_wavelength.size();
   fitparameters = new double [parameters];
   for (int i=0; i<v_wavelength.size(); i++)
   {
      fitparameters[i] = 0.3;
   }
   float lambda_step = (lambda_max - lambda_min)/(order+1); // create "order" peaks evenly distributed over the range
   for (unsigned int i=0; i<order; i++)
	{
		fitparameters[v_wavelength.size() + (i * 3) ] = 1;
      // spread out the peaks
      fitparameters[v_wavelength.size() + (i * 3) + 1] = lambda_min + lambda_step * i;
      fitparameters[v_wavelength.size() + (i * 3) + 2] = 10;
   }
	//opens the fitting GUI
   fitter = new US_ExtinctFitter(&v_wavelength, fitparameters, order, parameters,
                                    projectName, &fitting_widget);
   fitter->show();
	fitted = true;
	//causes the fitted line to plot after the fitting widget is closed
   connect(fitter, SIGNAL(fittingWidgetClosed()), SLOT(plot()));
   //data_plot->enableOutline(true);
}
void US_Extinction::calc_extinction()
{
	//specified scale for the fitted curve
	selected_wavelength = (float) ct_coefficient->value();
	extinction_coefficient = le_coefficient->text().toDouble();
   unsigned int i, j;
   if (v_wavelength.empty())
   {  
      return;
   }
   if (!lambda.empty())
   { 
      lambda.clear();
      extinction.clear();
   }
   float od_wavelength=0, od;
   xmax = -1.0;
   xmin = 1e6;

   for (int i=0; i< v_wavelength.size(); i++)
   {
      xmax = max(xmax, v_wavelength.at(i).v_readings.at(v_wavelength.at(i).v_readings.size()-1).lambda);
      xmin = min(xmin, v_wavelength.at(i).v_readings.at(0).lambda);
   }
   maxrange = (unsigned int) (xmax - xmin + 0.5);
   maxrange += 1;
   for (i=0; i<maxrange; i++)
   {
      lambda.push_back(xmin + i);
      od = 0.0;
      for (j=0; j<order; j++)
      {
         od += exp(fitparameters[v_wavelength.size() + (3 * j)]
                   - (pow((lambda[i] - fitparameters[v_wavelength.size() + (3 * j) + 1]), 2)
                      / ( 2 * pow(fitparameters[v_wavelength.size() + (3 * j) + 2], 2))));
      }
      extinction.push_back(od);
      if((unsigned int) lambda[i] == selected_wavelength)
      {
         od_wavelength = od;
      }
   }
	if(od_wavelength != 0)
	{
   	for (int i=0; i<extinction.size(); i++)
   	{
      	extinction[i] = extinction_coefficient * (extinction[i]/od_wavelength);
   	}
	}
	delete[] fitparameters;
}

//Changes number of Gaussians used to create the best fit curve
void US_Extinction::update_order(double new_order)
{
	order = (int) new_order;
}
void US_Extinction::calculateE280(void)
{
   int dbdisk = ( disk_controls->db() ) ? US_Disk_DB_Controls::DB
                                        : US_Disk_DB_Controls::Disk;

   US_AnalyteGui* analyte_dialog = new US_AnalyteGui( true, QString(), dbdisk );

   connect( analyte_dialog, SIGNAL( valueChanged  ( US_Analyte ) ),
            this,           SLOT  ( accessAnalyteExtinc( US_Analyte ) ) );

   analyte_dialog->exec();
   qApp->processEvents();
}

//Access that analyte's extinction value at the specified wavelength
void US_Extinction::accessAnalyteExtinc(US_Analyte data)
{
	int wavelengthNum = ct_coefficient->value();
	le_coefficient->setText(QString::number(data.extinction[wavelengthNum]));	
	currentAnalyte = data;
}
void US_Extinction::save(void)
{
	if(!fitted)
	{
   	QMessageBox mBox;
		mBox.setWindowTitle(tr("Ultrascan Error:"));
   	mBox.setText(tr("You have not yet performed the global fit. There is no data to save."));
   	mBox.exec();
		return;
	}

   QString filename = QFileDialog::getSaveFileName(this, "Save File", "/home/minji/ultrascan/results/", "*.extinction.dat");
   if(filename.isEmpty())
      return;
	
	//Takes off any extension the user might have added
	if(!(filename.lastIndexOf(".", -1) == -1))
   {
      filename = filename.left(1 + filename.lastIndexOf(".", -1));
   }

	filename_one =  filename + ".extinction.dat";
   QFile f (filename_one);
   if(f.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		QTextStream ts(&f);
		ts << tr("\"Wavelength\"\t\"Extinction\"\n");
		for(int i = 0; i < lambda.size(); i++)
		{
			ts << lambda[i] << "\t";
			ts << extinction[i] << "\n";
		}
		f.close();
	}
	
   filename_two = filename + ".extinction.res";
	QFile f2(filename_two);
   if(f2.open(QIODevice::WriteOnly | QIODevice::Text))
   {
      QTextStream ts(&f2);
      ts << tr("Results for global wavelength scan/extinction coefficient fit:\n\n");
      ts << tr("Number of Gaussian terms: ") << order << "\n\n";
      ts << tr("Extinction coeffcient at ") << selected_wavelength << tr(" nm used for normalization of Data: ") << extinction_coefficient << "\n\n";
      ts << tr("Parameters for each Gaussian term:\n");
      for (unsigned int i=0; i<order; i++)
      {
         ts << "\n" << (i+1) << tr(". Gaussian:\n");
         ts << tr("Peak position: ") << fitparameters[v_wavelength.size() + (i * 3) + 1] << " nm\n";
         ts << tr("Amplitude of peak: ") << exp(fitparameters[v_wavelength.size() + (i * 3)]) << tr(" extinction\n");
         ts << tr("Peak width: ") << fitparameters[v_wavelength.size() + (i * 3) + 2] << " nm\n";
      }
      ts << "\n";
      ts << "Equation to be multiplied by the relative concentration of each scan (printed below)\n";
      ts << "to re-generate the curve for each scan:\n\n";
      ts << "Concentration = Relative_Conc * SUM amplitude_i * exp((-(lambda - position_i)^2) / (2*width_i^2))\n";
      ts << "\n";
      for (int i=0; i< v_wavelength.size(); i++)
      {
         ts << tr("Relative Concentration of Scan ") << (i+1) << ": " << fitparameters[i] << "\n";
      }
      ts << "\n";
      ts << "/* A small C++ program to calculate the concentration curve\n";
      ts << "   compile with \"g++ myprog.cpp\"\n";
      ts << "   run with \"./a.out relative_concentration wavelength_start wavelength_end\"\n";
      ts << "*/\n";
      ts << "#include <iostream>\n";
      ts << "#include <math.h>\n";
      ts << "#include <cstdlib>\n";
      ts << "using namespace std;\n";
      ts << "int main (int argc, char **argv)\n";
      ts << "{\n";
      ts << "  float p[" << order << "], w[" << order << "], a[" << order << "], c=0.0, rel_conc;\n";
      ts << "  int start, end;\n";
      for (unsigned int i=0; i<order; i++)
      {
         ts << "   a[" << i << "] = " << exp(fitparameters[v_wavelength.size() + (i * 3)]) << ";\n";
         ts << "   p[" << i << "] = " << fitparameters[v_wavelength.size() + (i * 3) + 1] << ";\n";
         ts << "   w[" << i << "] = " << fitparameters[v_wavelength.size() + (i * 3) + 2] << ";\n";
      }
      ts << "  if(argc < 4)\n";
      ts << "  {\n";
      ts << "     cout << \"\\nUsage: a,out relative_concentration wavelength_start wavelength_end\\n\";\n";
      ts << "     return(-1);\n";
      ts << "  }\n";
      ts << "  else\n";
      ts << "  {\n";
      ts << "     rel_conc = atof(argv[1]);\n";
      ts << "     start = atoi(argv[2]);\n";
      ts << "     end = atoi(argv[3]);\n";
      ts << "  }\n";
      ts << "  for (int j=start; j<end; j++)\n";
      ts << "  {\n";
      ts << "     c = 0.0;\n";
      ts << "     for (int i=0; i<" << order << "; i++)\n";
      ts << "     {\n";
      ts << "        c += rel_conc * a[i] * exp(-pow(j-p[i], 2.0)/(2.0*w[i]*w[i]));\n";
      ts << "     }\n";
      ts << "     cout << j << \", \" << c << endl;\n";
      ts << "  }\n";
      ts << "  return 0;\n";
      ts << "}\n\n\n";
      f2.close();
		//currentAnalyte.setSpectrum(db);
   }

}
void US_Extinction::view_result(void)
{
	save();
   QTextEdit *e1;
   e1 = new QTextEdit();
   //e1->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   e1->setGeometry(30, 30, 685, 600);
   //e1->load(filename_one);
   e1->show();

}
void US_Extinction::help(void)
{

}

