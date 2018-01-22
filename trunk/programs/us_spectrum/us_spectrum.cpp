#include <QApplication>
#include "us_spectrum.h"
#include "us_gui_util.h"
#include "us_settings.h"
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
   pb_load_basis = us_pushbutton(tr("Add Basis Spectrum"));
   pb_load_basis->setEnabled(false);
   connect(pb_load_basis, SIGNAL(clicked()), SLOT(load_basis()));
     
   pb_delete = us_pushbutton(tr("Delete Current Basis Scan"));
   connect(pb_delete, SIGNAL(clicked()), SLOT(deleteCurrent()));
   pb_delete->hide();  // Spurious 
   
  

   pb_overlap = us_pushbutton(tr("Find Extinction Profile Overlap"));
   connect(pb_overlap, SIGNAL(clicked()), SLOT(overlap()));
   pb_overlap->setEnabled(false);

   pb_fit = us_pushbutton(tr("Fit Data"));
   connect(pb_fit, SIGNAL(clicked()), SLOT(fit()));
   pb_fit->setEnabled(false);
   
   pb_find_angles = us_pushbutton(tr("Find Angle between Basis Vectors"));
   connect(pb_find_angles, SIGNAL(clicked()), SLOT(findAngles()));
   pb_find_angles->setEnabled(false);

   pb_help = us_pushbutton(tr("Help"));
   pb_reset_basis = us_pushbutton(tr("Reset Basis Spectra / Reset Fit Results"));
   connect(pb_reset_basis, SIGNAL(clicked()), SLOT(resetBasis()));
   pb_reset_basis->setEnabled(false);

   pb_load_fit = us_pushbutton(tr("Load Fit"));
   connect(pb_load_fit, SIGNAL(clicked()), SLOT(load()));
   pb_load_fit->hide();
   
   pb_save= us_pushbutton(tr("Save Fit"));
   connect(pb_save, SIGNAL(clicked()), SLOT(save()));
   pb_load_fit->setEnabled(false);
   pb_save->setEnabled(false);
   
   pb_close = us_pushbutton(tr("Close"));
   connect(pb_close, SIGNAL(clicked()), SLOT(close()));

   //List Widgets
   lw_target = us_listwidget();
   lw_basis = us_listwidget();
   connect(lw_basis, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(deleteBasisCurve()));


   lbl_wvlinfo = us_banner(tr("Target/Basis Spectra Information"));
   lbl_correlation = us_banner(tr("Basis Vectors Correlation"));
   lbl_fit = us_banner(tr("Perform Fit"));
   lbl_load_save = us_banner(tr("Save Fitting Results"));
   lbl_rmsd = us_label(tr("RMSD: "));
   lbl_angle = us_label(tr("Angle (Deg.): "));
   le_rmsd = us_lineedit("", 1, true);

   //Do we need this ? (finding extinc. coeff.)
   cb_spectrum_type = new QComboBox();
   QStringList spectrum_types;
   spectrum_types << "target" << "basis" << "fitted";
   cb_spectrum_type->addItems(spectrum_types);
   lbl_wavelength = us_label(tr("Wavelength:"));
   lbl_extinction = us_label(tr("Extinction Coeff.:"));
   le_wavelength = us_lineedit("", 1, false);
   le_extinction = us_lineedit("", 1, true);
   pb_find_extinction = us_pushbutton(tr("Find Corresponding Extin. Coeff."));
   connect(pb_find_extinction, SIGNAL(clicked()), SLOT(findExtinction()));
   
   pb_find_extinction->hide();
   cb_spectrum_type->hide();
   lbl_wavelength->hide();
   lbl_extinction->hide();
   le_wavelength->hide();
   le_extinction->hide();
   /////////////////////

   cb_angle_one = new QComboBox();
   cb_angle_two = new QComboBox();
   le_angle = us_lineedit("", 1, true); 

   data_plot = new QwtPlot();
   plotLayout1 = new US_Plot(data_plot, tr(""), tr("Wavelength(nm)"), tr("Extinction"));
   data_plot->setCanvasBackground(Qt::black);
   data_plot->setTitle("Wavelength Spectrum Fit");
   data_plot->setMinimumSize(600,200);
   
   residuals_plot = new QwtPlot();
   plotLayout2 = new US_Plot(residuals_plot, tr(""), tr("Wavelength(nm)"), tr("Extinction"));
   residuals_plot->setCanvasBackground(Qt::black);
   residuals_plot->setTitle("Fitting Residuals");
   residuals_plot->setMinimumSize(600, 200);

   pick = new US_PlotPicker( data_plot );
   pick->setRubberBand( QwtPicker::VLineRubberBand );
   // connect( pick, SIGNAL( moved    ( const QwtDoublePoint& ) ),
   // 	    SLOT  ( new_value( const QwtDoublePoint& ) ) );
   // connect( pick, SIGNAL( moved    ( const QPointF& ) ),
   // 	    SLOT  ( new_value( const QwtDoublePoint& ) ) ); 
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
   // angles_layout->addWidget(le_angle, 1, 0);
   // angles_layout->addWidget(pb_find_angles, 2, 0);

   QGridLayout* angles_layout_res;
   angles_layout_res = new QGridLayout();
   angles_layout_res->addWidget(lbl_angle, 0, 0);
   angles_layout_res->addWidget(le_angle, 0, 1);

   QGridLayout* subgl1;
   subgl1 = new QGridLayout();
   subgl1->addWidget(lbl_wavelength, 0,0);
   subgl1->addWidget(le_wavelength, 0,1);
   subgl1->addWidget(lbl_extinction, 1, 0);
   subgl1->addWidget(le_extinction, 1, 1);

   QGridLayout* subgl_rmsd;
   subgl_rmsd = new QGridLayout();
   subgl_rmsd->addWidget(lbl_rmsd, 0, 0);
   subgl_rmsd->addWidget(le_rmsd, 0, 1);

   QGridLayout* subgl2;
   subgl2 = new QGridLayout();
   // subgl2->addWidget(pb_load_fit, 0, 0);
   // subgl2->addWidget(pb_save, 0, 1);
   // subgl2->addWidget(pb_help, 1, 0);
   // subgl2->addWidget(pb_close, 1, 1);
   subgl2->addWidget(pb_save, 0, 0);
   subgl2->addWidget(pb_help, 1, 0);
   subgl2->addWidget(pb_close, 2, 0); 

   QGridLayout* gl1;
   gl1 = new QGridLayout();
   int row = 0;
   gl1->addWidget(lbl_wvlinfo, row++, 0);
   gl1->addWidget(pb_load_target, row++, 0);
   gl1->addWidget(lw_target, row++, 0);
   gl1->addWidget(pb_load_basis, row++, 0);
   gl1->addWidget(lw_basis, row++, 0);
   gl1->addWidget(pb_reset_basis, row++, 0);
   
   /* Do we need this ??? */
   //gl1->addWidget(cb_spectrum_type, row++, 0);
   //gl1->addLayout(subgl1, row++, 0);
   //gl1->addWidget(pb_find_extinction, row++, 0);
   
   //gl1->addWidget(pb_delete, row++, 0);
   
   gl1->addWidget(lbl_fit, row++, 0);
   gl1->addWidget(pb_overlap, row++, 0);
   gl1->addWidget(pb_fit, row++, 0);
   gl1->addLayout(subgl_rmsd, row++, 0);
 
   //gl1->addLayout(subgl1, row++, 0);

   gl1->addWidget(lbl_correlation, row++, 0);
   gl1->addLayout(angles_layout, row++, 0);
   gl1->addWidget(pb_find_angles, row++, 0);
   gl1->addLayout(angles_layout_res, row++, 0);
   
   gl1->addWidget(lbl_load_save, row++, 0);
   gl1->addLayout(subgl2, row++, 0);
   
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
   
   //struct WavelengthProfile temp_wp;
   QFileDialog dialog (this);

   dialog.setNameFilter(tr("Text files (*.[Tt][Xx][Tt] *.[Cc][Ss][Vv] *.[Dd][Aa][Tt] *.[Ww][Aa]* *.[Dd][Ss][Pp]);;All files (*)"));
   //dialog.setNameFilter(tr("Text files (*.[Rr][Ee][Ss]);;All files (*)"));
   dialog.setFileMode(QFileDialog::ExistingFiles);
   dialog.setViewMode(QFileDialog::Detail);
   
   QString work_dir_data  = US_Settings::dataDir();
   
   current_path = current_path.isEmpty() ? work_dir_data : current_path;
   dialog.setDirectory(current_path);  

   if(dialog.exec())
   {
     QDir d = dialog.directory();
     current_path = d.absolutePath();
     files = dialog.selectedFiles();
     
     for (QStringList::const_iterator  it=files.begin(); it!=files.end(); ++it)
       {
	 QFileInfo fi;
	 fi.setFile(*it);
	 
	 struct WavelengthProfile temp_wp;
	 load_spectra(temp_wp, *it);
	 temp_wp.filenameBasis = fi.baseName();
	 //basis_names.append(fi.baseName());
	 v_basis.push_back(temp_wp);

	 cb_angle_one->addItem(fi.baseName());
	 cb_angle_two->addItem(fi.baseName());
       }
   }
 
   plot_basis();
   
   pb_reset_basis->setEnabled(true);
   pb_overlap->setEnabled(true);
   pb_fit->setEnabled(true);
   pb_find_angles->setEnabled(true);
   
   // overlap();
}

//Takes the information in the basis vector to plot all of the curves for the basis spectrums 
void US_Spectrum::plot_basis()
{
  //QStringList names;

   for(int m = basisIndex; m < v_basis.size(); m++)
   {
     //names.append(v_basis.at(m).filenameBasis);   
      lw_basis->insertItem(0, v_basis.at(m).filenameBasis);

      double* xx = (double*)v_basis.at(m).wvl.data();
      double* yy = (double*)v_basis.at(m).extinction.data();
      int     nn = v_basis.at(m).wvl.size();
      qDebug() << "Basis " << m << " size: " << nn;
      qDebug() << "Basis " << m << " x[0]/y[0]: " << xx[0] << "/" << yy[0];
      
      QwtPlotCurve* c;
      QwtSymbol *s = new QwtSymbol;
      s->setStyle(QwtSymbol::Ellipse);
      s->setPen(QPen(Qt::green));
      s->setBrush(QBrush(Qt::blue));
      s->setSize(3);
   
      // QPen p;
      // p.setColor(Qt::green);
      // p.setWidth(3);
      c = us_curve(data_plot, v_basis.at(m).filename);
      c->setSymbol(s);
      c->setStyle(QwtPlotCurve::NoCurve);

      // c->setPen(p);
      c->setSamples( xx, yy, nn );
      v_basis[basisIndex].matchingCurve = c;
      basisIndex++;
   }
   // cb_angle_one->addItems(names);
   // cb_angle_two->addItems(names);
   data_plot->replot();
}

//brings in the target spectrum according to user specification
void US_Spectrum::load_target()
{
   QFileDialog dialog (this);
   QString fileName = "";

   dialog.setNameFilter(tr("Text files (*.[Tt][Xx][Tt] *.[Cc][Ss][Vv] *.[Dd][Aa][Tt] *.[Ww][Aa]* *.[Dd][Ss][Pp]);;All files (*)"));
   dialog.setFileMode(QFileDialog::ExistingFile);
   dialog.setViewMode(QFileDialog::Detail);
   
   QString work_dir_data  = US_Settings::dataDir();
   dialog.setDirectory(work_dir_data);
   
   us_grid(data_plot);
   
   //reset for a new target spectrum to be loaded
   if(lw_target->count() > 0)
   {
      lw_target->clear();
      w_target.extinction.clear();
      w_target.wvl.clear();
      w_target.filenameBasis.clear();
      w_target.matchingCurve->detach();
      pb_load_basis->setEnabled(false);
      resetBasis();
   }
   if(dialog.exec())
   {
      fileName = dialog.selectedFiles().first();
      qDebug() << "filename: " << fileName;
      load_spectra(w_target, fileName);
      QFileInfo fi;
      fi.setFile(fileName);
      w_target.filenameBasis = fi.baseName();
      //lw_target->insertItem(0, w_target.filenameBasis);
   }
   
   plot_target();
   if ( lw_target->count() > 0 )
     pb_load_basis->setEnabled(true);
}

void US_Spectrum:: plot_target()
{
  //dataPlotClear( data_plot );
  //us_grid(data_plot);
   double* xx = (double*)w_target.wvl.data();
   double* yy = (double*)w_target.extinction.data();
   int     nn = w_target.wvl.size();
   
   qDebug() << "Target " << " size: " << nn;
   qDebug() << "Target " << " x[0]/y[0]: " << xx[0] << "/" << yy[0];
   
   QwtPlotCurve* c;
   
   QwtSymbol *s = new QwtSymbol;
   s->setStyle(QwtSymbol::Ellipse);
   s->setPen(QPen(Qt::yellow));
   s->setBrush(QBrush(Qt::blue));
   s->setSize(3);
   
   // QPen p;
   // p.setColor(Qt::yellow);
   // p.setWidth(3);
   c = us_curve(data_plot, w_target.filename);
   c->setSymbol(s);
   c->setStyle(QwtPlotCurve::NoCurve);
   //c->setPen(p);
   c->setSamples( xx, yy, nn );
   w_target.matchingCurve = c;
   data_plot->replot();
   if( !w_target.filenameBasis.isEmpty() )
     lw_target->insertItem(0, w_target.filenameBasis);
   //pb_load_basis->setEnabled(true);
}

//read spectrum
void US_Spectrum:: load_spectra(struct WavelengthProfile &profile, const QString &fileName)
{
   QString line;
   QString str1;
   double temp_x, temp_y;

   QVector <QString> strList;

   QFile f(fileName);
   QFileInfo fi;

   strList.clear();
   //   strList.resize(0);
   if(f.open(QIODevice::ReadOnly | QIODevice::Text))
   {
     QTextStream ts2(&f); 
     ts2.readLine();      // description
     ts2.readLine();
     QRegExp separator("(\\s+|,)");
     
     while(!ts2.atEnd())
	{
	  str1 = ts2.readLine();
	  str1 = str1.simplified();
	  
	  strList.push_back(str1);
	}
      f.close();
      
      qSort( strList );
      // qDebug() << "strLIST_size: " << strList.size();
      for ( int i = 0; i < strList.size(); i++)
	{
	  //qDebug() << strList[i]; 
	  temp_x = strList[i].split(QRegExp(separator), QString::SkipEmptyParts)[0].toDouble();
	  temp_y = strList[i].split(QRegExp(separator), QString::SkipEmptyParts)[1].toDouble();
	  
	  profile.extinction.push_back(temp_y);
	  profile.wvl.push_back(temp_x);
	}
      profile.lambda_min = profile.wvl[0];
      profile.lambda_max = profile.wvl.last();

      //qDebug() << "min/max: " << profile.lambda_min << "/" << profile.lambda_max;
   }
   else
   {
      QMessageBox mb;
      mb.setWindowTitle(tr("Attention:"));
      mb.setText("Could not read the wavelength data file:\n" + fileName);
   }
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
   unsigned int points, order, i, k, counter=0;
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
      "the data. You can correct the problem by running\n" 
      "\"Find Extinction Profile Overlap\".";
   
   for (i=0; i< (unsigned int) v_basis.size(); i++)
   {
     if(v_basis[i].lambda_min != min_lambda || v_basis[i].lambda_max != max_lambda || v_basis[i].wvl.size() != w_target.wvl.size())
      {
          QMessageBox::warning(this, tr("UltraScan Warning"), str,
          QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
          return;
      }
   }

   //points = w_target.lambda_max - w_target.lambda_min + 1;
   
   points = w_target.wvl.size();
   x = new double [points];
   y = new double [points];

   order = v_basis.size(); // no baseline necessary with gaussians
   nnls_a = new double [points * order]; // contains the model functions, end-to-end
   nnls_b = new double [points]; // contains the experimental data
   nnls_zzp = new double [points]; // pre-allocated working space for nnls
   nnls_x = new double [order]; // the solution vector, pre-allocated for nnls
   nnls_wp = new double [order]; // pre-allocated working space for nnls, On exit, wp[] will contain the dual solution vector, wp[i]=0.0 for all i in set p and wp[i]<=0.0 for all i in set z. 

   nnls_indexp = new int [order];
   //extinction
   for (i=0; i<points; i++)
   {
     x[i] = w_target.wvl[i];
     
     nnls_b[i] = w_target.extinction[i];
     b.push_back((float) nnls_b[i]);
   }
   
  
   counter = 0;
   //basis
   for (k=0; k<order; k++)
   {
     for(i = 0; i<points; i++)
      {
	nnls_a[counter] = v_basis[k].extinction[i];
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

   // Solution
   for (k=0; k<order; k++)
   {
     for (i=0; i<points; i++)
      {
      	solution[i] += v_basis[k].extinction[i] * nnls_x[k];
      }
    }


   for (i=0; i<points; i++)
   {
      residuals.push_back(solution[i] - b[i]);
      y[i] = solution[i];
   }

   dataPlotClear( residuals_plot );
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
     w_solution.wvl.push_back(x[j]);
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
   le_rmsd->setText(str.sprintf(" %3.2e", pow(fval, (float) 0.5)));
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

// Delete upon double click
bool US_Spectrum::deleteBasisCurve(void)
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
  
  if(v_basis.size() <= 1)
     {
       resetBasis();
       return(true);
     }
 
  QString selectedName = lw_basis->currentItem()->text();
  for(int k = 0; k < v_basis.size(); k++)
    {
      if(selectedName.contains(v_basis.at(k).filenameBasis))
	{
	  v_basis[k].matchingCurve->detach();
	  v_basis.remove(k);
	  cb_angle_one->removeItem(k);
	  cb_angle_two->removeItem(k);
	  delete lw_basis->currentItem();
	}
    }
  data_plot->replot();

  
  
  // v_basis[deleteIndex].matchingCurve->detach();
  // data_plot->replot();
  // v_basis.remove(deleteIndex);
  // delete lw_basis->currentItem();
  
  return(true);
}
//////////////////////////////////////////////////

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
   dataPlotClear( residuals_plot );
   residuals_plot->replot();
   data_plot->replot();
   lw_basis->clear();
   le_rmsd->clear();
   //le_rmsd->setText(QString("RMSD"));

   pb_overlap->setEnabled(false);
   pb_fit->setEnabled(false);
   pb_find_angles->setEnabled(false);
   pb_reset_basis->setEnabled(false);
   pb_save->setEnabled(false);
}

void US_Spectrum::overlap()
{
  unsigned int highest_lambda_min, lowest_lambda_max, largest, smallest;
  int exists_target, exists_basis;

   QVector<int> lambdaMins, lambdaMaxs;

   lambdaMins.push_back(w_target.lambda_min);
   lambdaMaxs.push_back(w_target.lambda_max);
   for(int m = 0; m < v_basis.size(); m++)
     {
       lambdaMins.push_back(v_basis.at(m).lambda_min);
       lambdaMaxs.push_back(v_basis.at(m).lambda_max);
     }

   qSort(lambdaMins);
   qSort(lambdaMaxs);
   
   highest_lambda_min = lambdaMins.last();
   lowest_lambda_max  = lambdaMaxs[0];
   
   smallest = lambdaMins[0];
   largest  = lambdaMaxs.last();
   
   qDebug() << "Mins: " << lambdaMins << ", highest_lambda_min: " << highest_lambda_min;
   qDebug() << "Maxs: " << lambdaMaxs << ", lowest_lambda_max : " << lowest_lambda_max ; 
   
   //we need a vector of all wavelengths which will be decimated based on overlap with target/basis
   int allpoints = largest - smallest + 1;
   QVector<int> allRange;
   for (int i=0; i < allpoints; i++)
     {
       allRange.push_back(smallest + i);
     }

   qDebug() << "AllRange size_0: " << allRange.size();
   for (int i=0; i < allRange.size(); i++)
     {
       //target
       exists_target = 0;
       for(int j = 0; j < w_target.wvl.size(); j++)
	 {
	   if (allRange[i] == w_target.wvl.at(j))
	     exists_target = 1;
	 }
       if ( !exists_target )
	 {
	   allRange.remove(i);
	   --i;
	 }
     }
   qDebug() << "AllRange size_1: " << allRange.size();
   
   for (int i=0; i < allRange.size(); i++)
     {
       //basis
       for(int j = 0; j < v_basis.size(); j++)
	 {
	   exists_basis = 0;
	   for(int m = 0; m < v_basis.at(j).wvl.size(); m++)
	     {    
	       if (allRange[i] == v_basis.at(j).wvl.at(m) )
		 exists_basis = 1;
	     }
	   if ( !exists_basis )
	     {
	       allRange.remove(i);
	       --i;
	     }
	 }
     }
   qDebug() << "AllRange size_2: " << allRange.size();  

   
   // basis
   for(int i = 0; i < v_basis.size(); i++)
     {
       for(int j = 0; j < v_basis.at(i).wvl.size(); j++)
	 {
           // try decimating basis based on values in updated allRange
	   exists_basis = 0;
	   for (int m = 0; m < allRange.size(); m++)
	     {
	       if(v_basis.at(i).wvl.at(j) == allRange[m])
		 exists_basis = 1;
	     }
	   if ( !exists_basis )
	     {
	       v_basis[i].wvl.remove(j);
	       v_basis[i].extinction.remove(j);
	       --j;
	     }
	 }
	   
	   // if(v_basis.at(i).wvl.at(j) >= lowest_lambda_max)
	   //   {
	   //     v_basis[i].wvl.remove(j, v_basis.at(i).wvl.size() - j);
	   //     v_basis[i].extinction.remove(j, v_basis.at(i).wvl.size() - j);
	   //     break;
	   //   }
	  	   
	   // if(v_basis.at(i).wvl.at(j) < highest_lambda_min)
	   //   {
	   //     qDebug() << "i wvl: " << i << " " << v_basis.at(i).wvl.at(j);  
	   //     v_basis[i].wvl.remove(0, j + 1);
	   //     v_basis[i].extinction.remove(0, j + 1);
	   //   }
           //}
     }

   //target
   for(int j = 0; j < w_target.wvl.size(); j++)
     {
       // try decimating target based on values in updated allRange
       exists_target = 0;
       for (int m = 0; m < allRange.size(); m++)
	 {
	   if(w_target.wvl.at(j) == allRange[m])
	     exists_target = 1;
	 }
       if ( !exists_target )
	 {
	   w_target.wvl.remove(j);
	   w_target.extinction.remove(j);
	   --j;
	 }

       // if(w_target.wvl.at(j) >= lowest_lambda_max)
       // 	 {
       // 	   w_target.wvl.remove(j, w_target.wvl.size() - j);
       // 	   w_target.extinction.remove(j, w_target.wvl.size() - j);
       // 	   break;
       // 	 }
       
       // if(w_target.wvl.at(j) < highest_lambda_min)
       // 	 {
       // 	   w_target.wvl.remove(0, j + 1);
       // 	   w_target.extinction.remove(0, j + 1);  
       // 	 }
     }

   w_target.lambda_min = highest_lambda_min;
   w_target.lambda_max = lowest_lambda_max;

   lw_target->clear();
   w_target.matchingCurve->detach();
   for(int k = 0; k < v_basis.size(); k++)
     {
       v_basis[k].lambda_min = highest_lambda_min;
       v_basis[k].lambda_max = lowest_lambda_max;
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
   
    QString str = "Please note:\n\n" 
      "Selected basic spectra have different limits.\n" 
      "These vectors need to be congruent before \n" 
      "computing correlation among them. \n\n" 
      "Please run \"Find Extinction Profile Overlap\"\n"
      "first to resolve the issue.";

   // If Basis vectros are of different dimensions
   if ( v_basis[indexOne].extinction.size() != v_basis[indexTwo].extinction.size() ) 
     {
       QMessageBox::warning(this, tr("UltraScan Warning"), str,
			    QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
       return;
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
  //QString filename = QFileDialog::getSaveFileName(this, "Save File", "/home/minji/ultrascan/results", "*.spectrum_fit");
  QString filename = QFileDialog::getSaveFileName(this, "Save File", US_Settings::resultDir(), "*.spectrum_fit.dat");
  if(filename.isEmpty())
      return;

   //Ensures that user did not add their own extension
   if(!(filename.lastIndexOf(".", -1) == -1))
   {
      filename = filename.left(1 + filename.lastIndexOf(".", -1));
   }
   
   filename = filename + ".spectrum_fit.dat";
   QFile f (filename);
   if(f.open(QIODevice::WriteOnly | QIODevice::Text))
   {
      QTextStream ts(&f);
      ts << tr("\"Wavelength\"\t\"Extinction\"\n");
      for(int i = 0; i < w_solution.wvl.size(); i++)
      {
         ts << w_solution.wvl[i] << "\t";
         ts << w_solution.extinction[i] << "\n";
      }
      f.close();
   }

   /*
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
      //ds << w_target.gaussians.size();
      // for (int i=0; i< w_target.gaussians.size(); i++)
      // {
      //    ds << w_target.gaussians[i].amplitude;
      //    ds << w_target.gaussians[i].sigma;
      //    ds << w_target.gaussians[i].mean;
      // }
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
         //ds << v_basis[j].gaussians.size();
         // for (int i=0; i< v_basis[j].gaussians.size(); i++)
         // {
         //    ds << v_basis[j].gaussians[i].amplitude;
         //    ds << v_basis[j].gaussians[i].sigma;
         //    ds << v_basis[j].gaussians[i].mean;
         // }
      }
      f.close();
   }
   */
}
   
void US_Spectrum::load()
{
   //Empty anything that may still contain information about previous curves
   resetBasis();
   lw_target->clear();
   if(lw_target->count() > 0)
   {
      w_target.matchingCurve->detach();
      //w_target.gaussians.clear();
   }
   
   //Load the file that the user selected
   //QString filename = QFileDialog::getOpenFileName(this, "Load File", "/home/minji/ultrascan/results", "*.spectrum_fit");
   
   QString filename = QFileDialog::getOpenFileName(this, "Load File", US_Settings::resultDir(), "*.spectrum_fit");
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
      // w_target.gaussians.resize(tempSize);
 
      // for (int i=0; i< w_target.gaussians.size(); i++)
      // {
      //    ds >> w_target.gaussians[i].amplitude;
      //    ds >> w_target.gaussians[i].sigma;
      //    ds >> w_target.gaussians[i].mean;
      // }

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
         // v_basis[j].gaussians.resize(tempSize);

         // for (int i=0; i< v_basis[j].gaussians.size(); i++)
         // {
         //    ds >> v_basis[j].gaussians[i].amplitude;
         //    ds >> v_basis[j].gaussians[i].sigma;
         //    ds >> v_basis[j].gaussians[i].mean;
         // }
      }
      f.close();
   }
   plot_basis();
   plot_target();
   fit();
}
