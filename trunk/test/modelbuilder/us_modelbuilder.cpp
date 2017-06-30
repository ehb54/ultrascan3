//! \file us_modelbuilder.cpp

#include "us_modelbuilder.h"
#include <QApplication>
#include <QDomDocument>
#include "us_math2.h"

using namespace std;
#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

//! \brief Main program for US_ModelBuilder. Loads translators and starts
//         the class US_ModelBuilder.

//global variable instance number
QString instanceNum;

int main(int argc, char* argv[])
{
   QApplication application(argc, argv);

#include "main1.inc"

   // License is OK.  Start up.

   //if argc == 0, set instance number to 0
   if(argc == 0)
      instanceNum = "0";
   else
      instanceNum = QString(argv[1]);

   US_ModelBuilder w;

   //Uncomment to enable GUI display (also uncomment the entire constructor below)
   //w.show(); //!< \memberof QWidget

   return application.exec(); //!< \memberof QApplication
}

// Constructor

US_ModelBuilder::US_ModelBuilder() : US_Widgets()
{

   //Enclosed lines setup and start the application, skipping GUI
   /////////////////////////////////////

   //set simparams to defualt values
   initalize_simulationParameters();

   //run simulation
   startSimulation();

   /////////////////////////////////////

   /*
   setWindowTitle(tr("Model Builder"));
   setPalette(US_GuiSettings::frameColor());

   QHBoxLayout* top = new QHBoxLayout(this);
   top->setSpacing(2);
   top->setContentsMargins(2, 2, 2, 2);

   QGroupBox* dataBox = new QGroupBox(tr("Simulation Parameters"));
   QGroupBox* controlBox = new QGroupBox(tr("Simulation Control"));
   QGroupBox* resultsBox = new QGroupBox(tr("Simulation Results"));

   // Start of global grid layout
   QGridLayout* specs = new QGridLayout();

   // Start of Grid Layout for results box
   QGridLayout* resultsSpecs = new QGridLayout();

   // Start of Grid Layout for control box
   QGridLayout* controlSpecs = new QGridLayout();

   // Start of Grid Layout for input box
   QGridLayout* inputSpecs = new QGridLayout();
   int s_row = 0;

   //prepare dataBox
   QLabel* lbl_sedimentationVal = us_label(tr("Initial Sed. Coeff."), -1);
   inputSpecs->addWidget(lbl_sedimentationVal, s_row, 0);

   le_sedimentationVal = us_lineedit("0.0");
   inputSpecs->addWidget(le_sedimentationVal, s_row, 1);

   QLabel* lbl_diffusionVal = us_label(tr("Initial Diff. Coeff."), -1);
   inputSpecs->addWidget(lbl_diffusionVal, ++s_row, 0);

   le_diffusionVal = us_lineedit("0.0");
   inputSpecs->addWidget(le_diffusionVal, s_row, 1);

   QLabel* lbl_RMSD = us_label(tr("Minimum RMSD"), -1);
   inputSpecs->addWidget(lbl_RMSD, ++s_row, 0);

   le_RMSD = us_lineedit("0.0");
   inputSpecs->addWidget(le_RMSD, s_row, 1);

   dataBox->setLayout(inputSpecs);

   //prepare controlBox
   parameterButton = new QPushButton(tr("Simulation Parameters"));
   controlSpecs->addWidget(parameterButton, 0, 0);
   connect(parameterButton, SIGNAL(clicked()), SLOT(collectParameters(void)));

   startButton = new QPushButton(tr("Start"));
   controlSpecs->addWidget(startButton, 0, 1);
   connect(startButton, SIGNAL(clicked()), SLOT(startSimulation(void)));
   //startButton -> setEnabled(false);

   //TODO: Add control for maximum s/d values to check, control for RMSD search tolerance

   controlBox->setLayout(controlSpecs);

   //prepare resultsBox
   //TODO: graph
   QBoxLayout* plot = new US_Plot(data_plot, tr(""), "x-axis", "y-axis");
   data_plot->setAutoDelete(true);
   data_plot->enableAxis(QwtPlot::xBottom, true);
   data_plot->enableAxis(QwtPlot::yLeft, true);
   data_plot->enableAxis(QwtPlot::yRight, false);
   data_plot->setMinimumSize(400, 200);
   data_plot->setAutoReplot(true);

   data_plot->setAxisTitle(0, "Frictional Ratio");
   data_plot->setAxisTitle(2, "Sedimentation Coefficient");

   QwtLegend legend;
   legend.setFrameStyle(QFrame::Box | QFrame::Sunken);
   data_plot->insertLegend(&legend, QwtPlot::BottomLegend);

   resultsSpecs->addLayout(plot, 0, 2, 23, 1);

   resultsBox->setLayout(resultsSpecs);


   //fill global gridLayout and apply
   specs->addWidget(dataBox, 0, 0);
   specs->addWidget(controlBox, 1, 0);
   specs->addWidget(resultsBox, 0, 1);
   top->addLayout(specs);
   this->setLayout(top);

   */

}

//this function allows the user to open simparams GUI

void US_ModelBuilder::collectParameters(void) {
   working_simparams = simparams;

   US_SimulationParametersGui* dialog = new US_SimulationParametersGui(working_simparams);

   connect(dialog, SIGNAL(complete()), SLOT(update_parameters()));

   dialog->exec();
}

//this function updates the global simparams variable with user data

void US_ModelBuilder::update_parameters(void)
{
   simparams = working_simparams;
}

//this function allows the user to begin the simulation
void US_ModelBuilder::startSimulation(void)
{

//   double sMin = 1e-13;
//   double sMax = 10e-13;

   double kMin = 1.0;
   double kMax = 4.0;

//   double dMin = 1e-7;
//   double dMax = 6e-7;

   double mMin = 5000;
   double mMax = 970000;

//   int dim = 20;

   //find out the hostname
   char tempHN[1024];
   tempHN[1023] = '\0';
   gethostname(tempHN, 1024);
   QString hostname = QString::fromLocal8Bit(tempHN);
   qDebug() << hostname;

   //SD grid generaton
   /*
   qDebug() << "Generating s-D grid...";

   QVector<QVector<US_Model::SimulationComponent> > sdGrid = generateSDGrid(sMin, sMax, dMin, dMax, dim, dim, 's');
   */

   //sk grid generation
   /*
   qDebug() << "Generating s-k grid...";

   QVector<QVector<US_Model::SimulationComponent> > skGrid = generateSKGrid(sMin, sMax, kMin, kMax, dim, dim, 's');
   */

   //MW grid generation
   /*
   qDebug() << "Generating mw-f/f0 grid...";

   QVector<QVector<US_Model::SimulationComponent> > mkGrid;
   for(int i = 0; i < dim; i++)
   {
      QVector<US_Model::SimulationComponent> column;

      for(int j = 0; j < dim; j++)
      {
         US_Model::SimulationComponent current;

         double mw = mMin + j * ((mMax - mMin) / dim);
         double k = kMin + i * ((kMax - kMin) / dim);

         double D = calculateDiffusionMK(mw, k);
         double s = calculateSedimentationMD(mw, D);

         current.s = s;
         current.D = D;
         current.f_f0 = k;
         current.mw = mw;

         //qDebug() << "s: " << s << " , D: " << D << " , f_f0: " << k;

         //do coefficient calculation, and proceed only if successful
         if(US_Model::calc_coefficients(current) && current.f_f0 <= 4.0 && current.f_f0 >= 1)
         {
            //qDebug() << "s: " << current.s << " , D: " << current.D << " , f_f0: " << current.f_f0 << endl;
            column.append(current);
         }
         else
         {
            //qDebug() << "Discarding invalid f/f0. Point: s, d, f/f0, mw: " << current.s << " , " << current.D << " , " << current.f_f0 << " , " << current.mw;
         }
      }

      mkGrid.append(column);
   }
   */

   qDebug() << "Beginning grid processing/output...";

   //testing and output for sd-type grids
   /*
   //SD grid output code
   QFile outfileSD("output/sdGrid_sp50_sMajor.tsv");
   outfileSD.open(QIODevice::ReadWrite);
   QTextStream outstreamSD(&outfileSD);
   outstreamSD << "#s \t D \t f/f0 \t MW \t RMSD" << endl;

   //do grid RMSD calc in paralell
   QVector<QVector<double> > calculatedSD = checkLineRMSDParalell(sdGrid, false);

   //write results
   for(int i = 0; i < calculatedSD.size(); i++)
   {
      QVector<double> current = calculatedSD.at(i);

      outstreamSD << current.at(0) << " \t" << current.at(1) << " \t" << current.at(2) << " \t" << current.at(3) << " \t" << current.at(4) << endl;
   }

   //close outfile
   outfileSD.close();
   */


   /*
   QVector<QPair<US_Model::SimulationComponent, double> > sdPairs = testRegularGrid(sdGrid);
   for(int i = 0; i < sdPairs.size(); i++)
   {
      US_Model::SimulationComponent current = sdPairs.at(i).first;
      outstreamSD << current.s << "\t" << current.D << "\t" << current.f_f0  << "\t" << current.mw << "\t" << sdPairs.at(i).second << endl;
   }

   //close outfile
   outfileSD.close();
   */

   //testing and output for SK-type grids
   /*
   //SK grid output code
   QFile outfileSK("output/skGrid_sp50_sMajor.tsv"); //change s/k - major in generation procedure above
   outfileSK.open(QIODevice::ReadWrite);
   QTextStream outstreamSK(&outfileSK);
   outstreamSK << "#s \t D \t f/f0 \t MW \t RMSD" << endl;

   //do grid RMSD calc in paralell
   QVector<QVector<double> > calculatedSK = checkLineRMSDParalell(skGrid, false);

   //write results
   for(int i = 0; i < calculatedSK.size(); i++)
   {
      QVector<double> current = calculatedSK.at(i);

      outstreamSK << current.at(0) << " \t" << current.at(1) << " \t" << current.at(2) << " \t" << current.at(3) << " \t" << current.at(4) << endl;
   }

   //close outfile
   outfileSK.close();
   */

   /*QVector<QPair<US_Model::SimulationComponent, double> > skPairs = testRegularGrid(skGrid);
   for(int i = 0; i < skPairs.size(); i++)
   {
      US_Model::SimulationComponent current = skPairs.at(i).first;
      outstreamSK << current.s << "\t" << current.D << "\t" << current.f_f0  << "\t" << current.mw << "\t" << skPairs.at(i).second << endl;
   }

   //close outfile
   outfileSK.close();
   */

   //Testing and output for MK-type grids
   /*
   //MK grid output code
   QFile outfileMK("output/kmGrid_sp50_kMajor.tsv");
   outfileMK.open(QIODevice::ReadWrite);
   QTextStream outstreamMK(&outfileMK);
   outstreamMK << "#s \t D \t f/f0 \t MW \t RMSD" << endl;

   //do grid RMSD calc in paralell
   QVector<QVector<double> > calculatedMK = checkLineRMSDParalell(mkGrid, true);

   //write results
   for(int i = 0; i < calculatedMK.size(); i++)
   {
      QVector<double> current = calculatedMK.at(i);

      outstreamMK << current.at(0) << " \t" << current.at(1) << " \t" << current.at(2) << " \t" << current.at(3) << " \t" << current.at(4) << endl;
   }

   //close outfile
   outfileMK.close();
   */

   /*QVector<QPair<US_Model::SimulationComponent, double> > mkPairs = testRegularGrid(mkGrid);
   for(int i = 0; i < mkPairs.size(); i++)
   {
      US_Model::SimulationComponent current = mkPairs.at(i).first;
      outstreamMK << current.s << "\t" << current.D << "\t" << current.f_f0  << "\t" << current.mw << "\t" << mkPairs.at(i).second << endl;
   }

   //close outfile
   outfileMK.close();

   */

   //Faxen grid generation and testing; must use constant-k lines
   /*
   QFile outfileFG("output/faxenGrid_kMajor.tsv");
   outfileFG.open(QIODevice::ReadWrite);
   QTextStream outstreamFG(&outfileFG);
   outstreamFG << "#s \t D \t f/f0 \t MW \t RMSD" << endl;

   QVector<QVector<US_Model::SimulationComponent> > faxen = createFaxenGrid(sMin,sMax, kMin, kMax, 35);

   //do grid RMSD calc in paralell
   QVector<QVector<double> > calculatedFG = checkLineRMSDParalell(faxen, true);

   //write results
   for(int i = 0; i < calculatedFG.size(); i++)
   {
      QVector<double> current = calculatedFG.at(i);

      outstreamFG << current.at(0) << " \t" << current.at(1) << " \t" << current.at(2) << " \t" << current.at(3) << " \t" << current.at(4) << endl;
   }

   outfileFG.close();
   */

   //Numerical grid generation along s-lines (s-major)
   /*
   QFile outfileN("output/numericalGrid_sp50_sMajor.tsv");
   outfileN.open(QIODevice::ReadWrite);
   QTextStream outstreamN(&outfileN);
   outstreamN << "#s \t D \t f/f0 \t MW \t RMSD" << endl;

   //target RMSD value
   double target = 1e-3;

   //double target = 0.0214; //k-major
   //double target = 1e-5; //s-major
   QVector<QVector<US_Model::SimulationComponent> > numerical = createNumericalGrid(target * 0.01, target, sMin, sMax, kMin, kMax, 's');

   //do grid RMSD calc in paralell
   QVector<QVector<double> > calculatedN = checkLineRMSDParalell(numerical, true);

   //write results
   for(int i = 0; i < calculatedN.size(); i++)
   {
      QVector<double> current = calculatedN.at(i);

      outstreamN << current.at(0) << " \t" << current.at(1) << " \t" << current.at(2) << " \t" << current.at(3) << " \t" << current.at(4) << endl;
   }

   outfileN.close();
   */

   //numerical grid generation along k-lines (k-major)
   /*
   QFile outfileN2("output/numericalGrid_sp50_kMajor.tsv");
   outfileN2.open(QIODevice::ReadWrite);
   QTextStream outstreamN2(&outfileN2);
   outstreamN2 << "#s \t D \t f/f0 \t MW \t RMSD" << endl;

   double target2 = 1e-3;

   QVector<QVector<US_Model::SimulationComponent> > numerical2 = createNumericalGrid(target2 * 0.01, target2, sMin, sMax, kMin, kMax, 'k');

   //do grid RMSD calc in paralell
   QVector<QVector<double> > calculatedN2 = checkLineRMSDParalell(numerical2, true);

   //write results
   for(int i = 0; i < calculatedN2.size(); i++)
   {
      QVector<double> current = calculatedN2.at(i);

      outstreamN2 << current.at(0) << " \t" << current.at(1) << " \t" << current.at(2) << " \t" << current.at(3) << " \t" << current.at(4) << endl;
   }

   outfileN2.close();
   */

   //Sk-grid gridsize testing
   /*
   qDebug() << "Generating sk grids for RMSD averaging";

   QFile outfileSKAverage("output/skGrid_Averages_sMajor.tsv");
   outfileSKAverage.open(QIODevice::ReadWrite);
   QTextStream outstreamSKAverage(&outfileSKAverage);
   outstreamSKAverage << "#PointCount \t AverageRMSD \t SampleStddev" << endl;

   //run grid generation for different dimensions
   for(int dimension = 20; dimension < 400; dimension += 10)
   {
      qDebug() << "Making grid for dimension: " << dimension;

      //make grid
      QVector<QVector<US_Model::SimulationComponent> > grid = generateSKGrid(sMin, sMax, kMin, kMax, dimension, dimension, 's');

      //get RMSD
      QVector<QVector<double> > results = checkLineRMSDParalell(grid, false);

      //stats - 3-length array of count, average, stddev
      QVector<double> stats = calculateGridStatistics(results);

      //output data to file
      outstreamSKAverage << stats.at(0) << " \t" << stats.at(1) << " \t" << stats.at(2) <<  endl;

   }

   outfileSKAverage.close();
   */

   //faxen gridsize testing
   /*
   qDebug() << "Generating Faxen grids for RMSD averaging";

   QFile outfileFGAverage("output/faxenGrid_Averages_kMajor.tsv");
   outfileFGAverage.open(QIODevice::ReadWrite);
   QTextStream outstreamFGAverage(&outfileFGAverage);
   outstreamFGAverage << "#PointCount \t AverageRMSD \t SampleStddev" << endl;

   //run grid generation for different dimensions
   for(int dimension = 10; dimension < 500; dimension += 10)
   {
      qDebug() << "Making grid for dimension: " << dimension;

      int numPoints = 0;
      double average = 0.0;
      double stddev = 0.0;

      //make grid
      QVector<QVector<US_Model::SimulationComponent> > grid = createFaxenGrid(sMin,sMax, kMin, kMax, dimension);

      //get RMSD
      QVector<QVector<double> > results = checkLineRMSDParalell(grid, false);

      //stats - 3-length array of count, average, stddev
      QVector<double> stats = calculateGridStatistics(results);

      //output data to file
      outstreamFGAverage << stats.at(0) << " \t" << stats.at(1) << " \t" << stats.at(2) <<  endl;
   }

   outfileFGAverage.close();
   */

   //sd gridsize testing
   /*
   qDebug() << "Generating sd grids for RMSD averaging";

   QFile outfileSDAverage("output/sdGrid_Averages_sMajor.tsv");
   outfileSDAverage.open(QIODevice::ReadWrite);
   QTextStream outstreamSDAverage(&outfileSDAverage);
   outstreamSDAverage << "#PointCount \t AverageRMSD \t SampleStddev" << endl;

   //run grid generation for different dimensions
   for(int dimension = 20; dimension < 400; dimension += 10)
   {
      qDebug() << "Making grid for dimension: " << dimension;

      //make grid
      QVector<QVector<US_Model::SimulationComponent> > grid = generateSDGrid(sMin, sMax, dMin, dMax, dimension, dimension, 's');

      //get RMSD
      QVector<QVector<double> > results = checkLineRMSDParalell(grid, false);

      //stats - 3-length array of count, average, stddev
      QVector<double> stats = calculateGridStatistics(results);

      //output data to file
      outstreamSDAverage << stats.at(0) << " \t" << stats.at(1) << " \t" << stats.at(2) <<  endl;

   }

   outfileSDAverage.close();
   */

   //MK grid gridsize testing
   //run only if we're on node 1
   if(hostname.compare("n1") == 0)
   {
      qDebug() << "n1: Generating mk grids for RMSD averaging";

      QFile outfileMKAverage("output/mkGrid_Averages_kMajor.tsv");
      outfileMKAverage.open(QIODevice::ReadWrite);
      QTextStream outstreamMKAverage(&outfileMKAverage);
      outstreamMKAverage << "#PointCount \t AverageRMSD \t SampleStddev" << endl;

      //generate grid at different sizes
      for(int dimension = 20; dimension < 400; dimension += 10)
      {
         qDebug() << "Making grid for dimension: " << dimension;

         //make grid
         QVector<QVector<US_Model::SimulationComponent> > grid = generateMKGrid(mMin, mMax, kMin, kMax, dimension, dimension, 'k');

         //get RMSD
         QVector<QVector<double> > results = checkLineRMSDParalell(grid, false);

         //stats - 3-length array of count, average, stddev
         QVector<double> stats = calculateGridStatistics(results);

         //output data to file
         outstreamMKAverage << stats.at(0) << " \t" << stats.at(1) << " \t" << stats.at(2) <<  endl;
      }

      outfileMKAverage.close();
   }

//   US_Model::SimulationComponent model1 = modelComponentFromSK(0.000000000000100096, 1);
//   US_Model::SimulationComponent model2 = modelComponentFromSK(0.000000000000100288, 1);

//   US_DataIO::RawData* data1 = simulateComponent(model1);
//   US_DataIO::RawData* data2 = simulateComponent(model2);

//   double rmsd = calculateScaledRMSD(data1, model1, data2, model2);
//   delete data1;
//   delete data2;

//   qDebug() << "RMSD: " << rmsd;

   //TODO: remove temporary exit
   qDebug() << "Exiting \"normally\"";
   exit(0);
}

//interpolates the RMSD of the point 'target' using the RMSDs of 4 surrounding points on a regular grid
double US_ModelBuilder::interpolatePoint(QVector3D target, QVector3D p11, QVector3D p12, QVector3D p21, QVector3D p22)
{
   //extract x and y values
   double x = target.x();
   double y = target.y();
   double x1 = p11.x();
   double y1 = p11.y();
   double x2 = p22.x();
   double y2 = p22.y();

   //interpolate on x-direction
   double xy1 = ((x2 - x) / (x2 - x1)) * p11.z() + ((x - x1) / (x2 - x1)) * p21.z();
   double xy2 = ((x2 - x) / (x2 - x1)) * p12.z() + ((x - x1) / (x2 - x1)) * p22.z();

   //calculate approximate interpolation - uses abbreviated formula, possibly replace in future
   return (((y2 - y) / (y2 - y1)) * xy1 + ((y - y1) / (y2 - y1)) * xy2);
}

//this function initalizes the simparams variable

void US_ModelBuilder::initalize_simulationParameters() {
   /*if (simparams.load_simparms("Resources/sp_defaultProfle.xml") != 0) {
        startButton->setEnabled(false);
        qDebug() << "Error: Simulation Parameters cannot be loaded. Manual selection required.";
    }*/

   US_SimulationParameters::SpeedProfile sp;
   QString rotor_calibr = "0";
   //double  rpm          = 15000.0; //normally set to 45000
   double rpm = 50000.0;

   // set up bottom start and rotor coefficients from hardware file
   simparams.setHardware( NULL, rotor_calibr, 0, 0 );

   // calculate bottom from rpm, channel bottom pos., rotor coefficients
   double bottom = US_AstfemMath::calc_bottom( rpm,
                                               simparams.bottom_position, simparams.rotorcoeffs );
   bottom        = (double)( qRound( bottom * 1000.0 ) ) * 0.001;

   simparams.mesh_radius.clear();
   simparams.speed_step .clear();

   //sp.duration_hours    = 5;
   //sp.duration_minutes  = 30.0;
   sp.duration_hours    = 6;
   sp.duration_minutes  = 0.0;
   sp.delay_hours       = 0;
   //sp.delay_minutes     = 20.0;
   sp.delay_minutes     = 2.0;
   sp.rotorspeed        = (int)rpm;
   //sp.scans             = 30;
   sp.scans             = 100;
   sp.acceleration      = 400;
   sp.acceleration_flag = true;

   simparams.speed_step << sp;

   simparams.simpoints         = 200;
   simparams.radial_resolution = 0.001;
   simparams.meshType          = US_SimulationParameters::ASTFEM;
   simparams.gridType          = US_SimulationParameters::MOVING;
   simparams.meniscus          = 5.8;
   simparams.bottom            = bottom;
   simparams.rnoise            = 0.0;
   simparams.tinoise           = 0.0;
   simparams.rinoise           = 0.0;
   simparams.band_volume       = 0.015;
   simparams.rotorCalID        = rotor_calibr;
   simparams.band_forming      = false;

   //set workingsimparams
   working_simparams = simparams;
}

//this function produces an initalized RawData sim object using given simparams
US_DataIO::RawData* US_ModelBuilder::init_simData(US_Model* system) {
   //instantiate object to be initalized
   US_DataIO::RawData* working_simdata = new US_DataIO::RawData();

   //init
   working_simdata->xvalues.clear();
   working_simdata->scanData.clear();

   working_simdata->type[0] = 'R';
   working_simdata->type[1] = 'A';

   working_simdata->cell = 1;
   working_simdata->channel = 'S';
   working_simdata->description = "Simulation";

   int points = qRound((simparams.bottom - simparams.meniscus) /
                       simparams.radial_resolution) + 1;

   working_simdata->xvalues.resize(points);

   for (int i = 0; i < points; i++) {
      working_simdata->xvalues[i] = simparams.meniscus
                                    + i * simparams.radial_resolution;
   }

   int total_scans = 0;

   for (int i = 0; i < simparams.speed_step.size(); i++) {
      total_scans += simparams.speed_step[i].scans;
   }

   working_simdata->scanData.resize(total_scans);
   int terpsize = (points + 7) / 8;

   for (int i = 0; i < total_scans; i++) {
      US_DataIO::Scan* scan = &working_simdata->scanData[i];

      scan->temperature = simparams.temperature;
      scan->rpm = simparams.speed_step[0].rotorspeed;
      scan->omega2t = 0.0;

      scan->wavelength = system->wavelength;
      scan->plateau = 0.0;
      scan->delta_r = simparams.radial_resolution;

      scan->rvalues.fill(0.0, points);
      scan->interpolated.fill(0, terpsize);
   }

   double w2t_sum = 0.0;
   double delay = simparams.speed_step[ 0 ].delay_hours * 3600.0
                  + simparams.speed_step[ 0 ].delay_minutes * 60.0;
   double current_time = 0.0;
   double duration;
   double increment = 0.0;
   int scan_number = 0;

   for (int ii = 0; ii < simparams.speed_step.size(); ii++) {
      US_SimulationParameters::SpeedProfile* sp = &simparams.speed_step[ii];
      double w2t = sq(sp->rotorspeed * M_PI / 30.0);

      delay = sp->delay_hours * 3600. + sp->delay_minutes * 60.;
      duration = sp->duration_hours * 3600. + sp->duration_minutes * 60.;
      increment = (duration - delay) / (double) (sp->scans - 1);
      double w2t_inc = increment * w2t;
      current_time += delay;
      w2t_sum = (ii == 0) ? (current_time * w2t) : w2t_sum;

      for (int jj = 0; jj < sp->scans; jj++) {
         US_DataIO::Scan* scan = &working_simdata->scanData[scan_number];
         scan->seconds = (double) qRound(current_time);
         scan->omega2t = w2t_sum;
         w2t_sum += w2t_inc;
         current_time += increment;
         scan_number++;
      }

      int j1 = scan_number - sp->scans;
      int j2 = scan_number - 1;

      sp->w2t_first = working_simdata->scanData[j1].omega2t;
      sp->w2t_last = working_simdata->scanData[j2].omega2t;
      sp->time_first = working_simdata->scanData[j1].seconds;
      sp->time_last = working_simdata->scanData[j2].seconds;
   }

   simparams.mesh_radius.clear();
   simparams.firstScanIsConcentration = false;

   //return initalized dataset
   return working_simdata;
}

//NOTE: Code from rewrite only below this point

// get the numerical distance between two SimulationComponents on the grid
double US_ModelBuilder::calculateDistance( US_Model::SimulationComponent first , US_Model::SimulationComponent second ) 
{
   return sqrt( pow(first.s - second.s , 2) ) + sqrt( pow(first.f_f0 - second.f_f0 , 2) );
}

// compute the value of f_f0 for a solute with specified s and D values
double US_ModelBuilder::calculateFrictionalRatioSD(double s, double D)
{
   //constants
   double R = 8.314e7; // already defined in US_Constants
   double T = 293.15;
   double vbar = 0.72;
   double eta = 0.0100914;
   double N = 6.022e23;
   double rho = 0.998234;

   //calculations
   double f = R * (T/ (N * D));
   double M = (s * N * f) / (1 - vbar * rho);
   double V = M * (vbar / N);
   double r_0= cbrt( (3 * V) / (4 * PI));
   double f_0 = 6 * PI * eta * r_0;

   return (f / f_0);
}

double US_ModelBuilder::calculateDiffusionSK(double s, double k)
{
   double R = 8.314e7;
   double T = 293.15;
   double vbar = 0.72;
   double eta = 0.0100914;
   double N = 6.022e23;
   double rho = 0.998234;

   double D;

   D = (R * T) / (N * k * 9.0 * eta * PI * pow((2.0 * s * k * vbar * eta)
                                                 / (1.0 - vbar * rho), 0.5));

   return D;
}

// calculates diffusion from molecular weight and f/f0
double US_ModelBuilder::calculateDiffusionMK(double M, double k)
{
   double R = 8.314e7;
   double T = 293.15;
   double vbar = 0.72;
   double eta = 0.0100914;
   double N = 6.022e23;

   double V = M * (vbar / N);
   double r_0= cbrt( (3 * V) / (4 * PI));
   double f_0 = 6 * PI * eta * r_0;
   double f = k * f_0;

   double D = (R * T) / (N * f);

   return D;
}

//calculates sedimentation coefficient from molecular weight and diffusion coefficient
double US_ModelBuilder::calculateSedimentationMD(double M, double D)
{
   double R = 8.314e7;
   double T = 293.15;
   double vbar = 0.72;
   double rho = 0.998234;

   double s = D * ((M * (1 - vbar * rho)) / (R * T));

   return s;
}

//Takes an ordered list of SCs, simulates each, and returns the list of resulting RMSD values from (0,1),(1,2), etc.
QVector<double> US_ModelBuilder::findListRMSD(QVector<US_Model::SimulationComponent> components)
{
   QVector<double> rmsdList;
   US_DataIO::RawData* firstPoint;
   US_DataIO::RawData* secondPoint;

   if(components.size() >= 2)
   {
      //get the first simulation
      firstPoint = simulateComponent(components.at(0));
      US_Model::SimulationComponent component1 = components.at(0);

      //iterate over list
      for (int i = 1; i < components.size(); i++)
      {
         //get second model RawData
         US_Model::SimulationComponent component2 = components.at(i);
         secondPoint = simulateComponent(component2);

         //calculate and store RMSD
         //rmsdList.append(calculate_RMSD(firstPoint, secondPoint));
         rmsdList.append(calculateScaledRMSD(firstPoint, component1, secondPoint, component2));

         //delete first RawData object
         delete firstPoint;

         //store second RawData object and s-value
         firstPoint = secondPoint;
         component1 = component2;
      }

      //works because at this point secondpoint and firstpoint have the same pointer
      delete firstPoint;

      return rmsdList;
   }

   else
   {
      qDebug() << "Error: less than 2 components in QVector";
      return rmsdList;
   }
}

//Creates a model from a single component and performs ASTFEM simulation to produce raw data
//TODO: remove dependence on non-updated method init_simData
US_DataIO::RawData* US_ModelBuilder::simulateComponent( US_Model::SimulationComponent component )
{
   //Make a model, and add the component
   US_Model model;
   model.components << component;

   //model.debug();

   //create the result object
   US_DataIO::RawData* simulation = init_simData(&model);

   //create the simulator
   US_Astfem_RSA* simulator = new US_Astfem_RSA(model, simparams);

   //run sim
   simulator->calculate(*simulation);
   
   //clean and return
   delete simulator;
   return simulation;
}

//For two simulations, calculate the RMSD between only useful scans
double US_ModelBuilder::calculateScaledRMSD(US_DataIO::RawData* simulation1, US_Model::SimulationComponent component1 , US_DataIO::RawData* simulation2 , US_Model::SimulationComponent component2)
{
   //TODO: check that all the params are the same between the simulatons (especially mensiscus position)

   //scan number check
   if(simulation1->scanCount() != simulation2->scanCount())
   {
      qDebug() << "Error: simulations do not match";
      return -999;
   }

   //Check that simparams has only one SpeedProfile
   if(simparams.speed_step.size() != 1)

   {
      qDebug() << "Error: simulations do not match";
      return -999;
   }

   //time check
   for(int i = 0; i < simulation1->scanCount(); i++)
   {
      if(simulation1->scanData.at(i).seconds != simulation2->scanData.at(i).seconds)
      {
         qDebug() << "Error: simulations do not match (time)";
         return -999;
      }
   }

   US_SimulationParameters::SpeedProfile sp = simparams.speed_step.at(0);

   //set constants
   double C01 = component1.signal_concentration;
   double C02 = component2.signal_concentration;
   double s1 = component1.s;
   double s2 = component2.s;
   double rpm = sp.rotorspeed;
   double omega2 = pow((rpm * PI) / 30.0, 2);
   double rMeniscus = simparams.meniscus;
   double rBottom = simparams.bottom;
   double left = rMeniscus + 0.025;
   //double right = rBottom - 0.1;

   double deltaT1 = log(rBottom / rMeniscus) / (omega2 * s1);
   double deltaT2 = log(rBottom / rMeniscus) / (omega2 * s2);
   double deltaTMin = min(deltaT1, deltaT2);

   //qDebug() << "delta T 1, delta T 2: " << deltaT1 << " , " << deltaT2;

   int startIndex = min(US_DataIO::index(simulation1, left) , US_DataIO::index(simulation2, left));

   int scanCount = 0;
   int pointCount = 0;
   double rmsd = 0.0;

   //qDebug() << "meniscus, bottom: " << rMeniscus << " , " << rBottom;

   bool pastDeltaT = false;

   /*
   QFile outfile("output/simulationdata.csv");
   outfile.open(QIODevice::ReadWrite);
   QTextStream outstream(&outfile);
   */

   //iterate over scans
   for(int i = 0; i < simulation1->scanCount() && !pastDeltaT; i++)
   {
      US_DataIO::Scan scan1 = simulation1->scanData.at(i);
      US_DataIO::Scan scan2 = simulation2->scanData.at(i);

      if(deltaTMin >= scan1.seconds && deltaTMin >= scan2.seconds)
      {
         scanCount++;
         int endIndex = min(scan1.rvalues.size(), scan2.rvalues.size());

         //find plateau concentrations
         double plateauConcentration1 = C01 * pow(M_E, -2.0 * s1 * omega2 * scan1.seconds);
         double plateauConcentration2 = C02 * pow(M_E, -2.0 * s1 * omega2 * scan2.seconds);

         //qDebug() << "plateau con1, con2: " << plateauConcentration1 << " , " << plateauConcentration2;

         double minPlateauConcentration = min(plateauConcentration1, plateauConcentration2);

         //do calculations for scan - run until values reach 1/2 of the minPlateauConcentration
         bool pastHalfPlateau = false;
         for(int j = startIndex; j < endIndex && !pastHalfPlateau; j++)
         {
            double val1 = simulation1->value(i, j);
            double val2 = simulation2->value(i, j);

            //outstream << val1 << " , " << val2 << " , " << scan1.seconds << " , " << minPlateauConcentration << endl;

            if(val1 * 2 < minPlateauConcentration && val2 * 2 < minPlateauConcentration)
            {
               pointCount++;
               rmsd += pow(abs(val1 - val2) , 2);
            }
            else
            {
               pastHalfPlateau = true;
            }
         }
      }
      else
      {
         //end of usable data reached; quit
         pastDeltaT = true;
      }
   }

   /*
   outfile.close();
   qDebug() << "scans, points: " << scanCount << " , " << pointCount;
   qDebug() << "raw difference-square sum: " << rmsd;*/

   //finalize rmsd calculations
   //rmsd /= pointCount;
   rmsd = sqrt(rmsd / pointCount);
   return rmsd;
}

//switches row-column orientation of a 2d simcomponent qvector; requires that the r-c dimensions be equal
QVector<QVector<US_Model::SimulationComponent> > US_ModelBuilder::switchRegularGridOrientation(QVector<QVector<US_Model::SimulationComponent> > regular)
{
   QVector<QVector<US_Model::SimulationComponent> > rotated;

   //check dimensions are equal
   for(int i = 0; i < regular.size(); i++)
   {
      if(regular.size() != regular.at(i).size())
      {
         qDebug() << "Grid orientation cannot proceed if dimensions are not all equivalent!";
         return rotated;
      }
   }

   //iterate
   for(int i = 0; i < regular.size(); i++)
   {
      //create new perpendicular vector
      QVector<US_Model::SimulationComponent> row;

      for(int j = 0; j < regular.size(); j++)
      {
         row.append(regular.at(j).at(i));
      }

      rotated.append(row);
   }

   return rotated;
}

//takes regularly spaced grid, tests RMSD in both directions
//Creates a paralell grid of simulation data that is populated as needed to ensure that each simulation is done only once
QVector<QPair<US_Model::SimulationComponent, double> > US_ModelBuilder::testRegularGrid(QVector<QVector<US_Model::SimulationComponent> > grid)
{
   QVector<QPair<US_Model::SimulationComponent, double> > results;

   //declare simulation data storage structures
   QVector<QVector<US_DataIO::RawData*> > simulationData;
   for(int i = 0; i < grid.size(); i++)
   {
      QVector<US_DataIO::RawData*> column(grid.at(i).size(), NULL);
      simulationData.append(column);
   }

   //do calculation for every point
   for(int i = 0; i < grid.size(); i++)
   {
      for(int j = 0; j < grid.at(i).size(); j++)
      {
         int numComparisons = 0;
         double rmsdSum = 0.0;
         US_Model::SimulationComponent current = grid.at(i).at(j);

         //simulate the current point if not already simulated
         if(simulationData.at(i).at(j) == NULL)
         {
            //remove the NULL pointer and replace with the simulation
            delete simulationData.at(i).at(j); //ensure we don't memory leak here
            simulationData[i].replace(j, simulateComponent(current));
         }

         //check above (i - 1)
         if((i - 1) >= 0 && (i - 1) < grid.size() && (j) >= 0 && (j < grid.at(i - 1).size()))
         {
            numComparisons++;

            //simulate if not already done
            if(simulationData.at(i - 1).at(j) == NULL)
            {
               delete simulationData.at(i - 1).at(j);
               simulationData[i - 1].replace(j, simulateComponent(grid.at(i - 1).at(j)));
            }

            //calculate rmsd between current point and point above
            rmsdSum += calculateScaledRMSD(simulationData.at(i).at(j), current,
                                           simulationData.at(i - 1).at(j), grid.at(i - 1).at(j));
         }

         //check right (j + 1)
         if((i) >= 0 && (i) < grid.size() && (j + 1) >= 0 && ((j + 1) < grid.at(i).size()))
         {
            numComparisons++;

            //simulate if not already done
            if(simulationData.at(i).at(j + 1) == NULL)
            {
               delete simulationData.at(i).at(j + 1);
               simulationData[i].replace(j + 1, simulateComponent(grid.at(i).at(j + 1)));
            }

            //calculate rmsd between current point and point to the right
            rmsdSum += calculateScaledRMSD(simulationData.at(i).at(j), current,
                                           simulationData.at(i).at(j + 1), grid.at(i).at(j + 1));
         }

         //check below (i + 1)
         if((i + 1) >= 0 && (i + 1) < grid.size() && (j) >= 0 && (j < grid.at(i + 1).size()))
         {
            numComparisons++;

            //simulate if not already done
            if(simulationData.at(i + 1).at(j) == NULL)
            {
               delete simulationData.at(i + 1).at(j);
               simulationData[i + 1].replace(j, simulateComponent(grid.at(i + 1).at(j)));
            }

            //calculate rmsd between current point and point above
            rmsdSum += calculateScaledRMSD(simulationData.at(i).at(j), current,
                                           simulationData.at(i + 1).at(j), grid.at(i + 1).at(j));
         }

         //check left (j - 1)
         if((i) >= 0 && (i) < grid.size() && (j - 1) >= 0 && ((j - 1) < grid.at(i).size()))
         {
            numComparisons++;

            //simulate if not already done
            if(simulationData.at(i).at(j - 1) == NULL)
            {
               delete simulationData.at(i).at(j - 1);
               simulationData[i].replace(j - 1, simulateComponent(grid.at(i).at(j - 1)));
            }

            //calculate rmsd between current point and point to the right
            rmsdSum += calculateScaledRMSD(simulationData.at(i).at(j), current,
                                           simulationData.at(i).at(j - 1), grid.at(i).at(j - 1));
         }

         //take average RMSD
         rmsdSum /= numComparisons;

         //add to results data
         QPair<US_Model::SimulationComponent, double> currentResult(current, rmsdSum);
         results.append(currentResult);
      }
   }

   //iterate through simulation data and delete everything
   for(int i = 0; i < simulationData.size(); i++)
   {
      for(int j = 0; j < simulationData.at(i).size(); j++)
      {
         delete simulationData.at(i).at(j);
      }
   }

   return results;
}

//Generates a Faxen grid from boundaries given
QVector<QVector<US_Model::SimulationComponent> > US_ModelBuilder::createFaxenGrid(double minS, double maxS, double minK, double maxK, int grids)
{
   QVector<QVector<US_Model::SimulationComponent> > grid;

   //scale
   minS /= 1e-13;
   maxS /= 1e-13;

   int M = grids;
   double mu_1 = minS * minK;
   double mu_2 = maxS * maxK;

   //list to store precoumputed xi_j values
   QVector<double> jValues;

   //first part of grid generation: calc xi_j
   for(int j = 0; j <= M; j++)
   {
      double xi_j = pow((1 - ((double) j) / M) * pow(mu_1, -0.25) + (((double) j ) / M) * pow(mu_2, -0.25), -4.0);

      jValues.append(xi_j);
   }

   //do calculations for all i satisfying specification condition
   for(int i = 0; i <= M; i++)
   {
      QVector<US_Model::SimulationComponent> column;

      double xi_i = pow((1 - (((double) i) / M)) * pow(mu_1, -0.25) + (((double) i ) / M) * pow(mu_2, -0.25), -4.0);

      //check that the value for xi_i is allowed
      if(xi_i <= minS * maxK)
      {
         //if the condition is satisfied, we have this column's y-value
         double y_i = xi_i / minS;

         //now we have to use all values of xi_j to find the x-values
         for(int j = 0; j < jValues.size(); j++)
         {
            //the value of xi
            double xi_j = jValues.at(j);

            //the coordinate
            double x_ij = xi_j / y_i;

            //add the point only if it's s-value falls in the bounds
            if(x_ij >= minS && x_ij <= maxS)
            {
               US_Model::SimulationComponent current;

               current.s = x_ij * 1e-13;
               current.f_f0 = y_i;
               current.D = calculateDiffusionSK(x_ij * 1e-13, y_i);

               //use calc_coefficients to populate the rest of the simcomponent
               if(US_Model::calc_coefficients(current))
               {
                  column.append(current);
               }
               else
               {
                  qDebug() << "calc_coefficients failed in Faxen generation. s, d, f/f0, mw: " << current.s << " , " << current.D << " , " << current.f_f0 << " , " << current.mw;
               }
            }
         }
      }

      //add the column if it isn't empty
      if(!column.isEmpty())
      {
         grid.append(column);
      }
   }

   return grid;
}

//creates a simulation component from an S and K value
US_Model::SimulationComponent US_ModelBuilder::modelComponentFromSK(double s, double k)
{
   US_Model::SimulationComponent newModel;

   newModel.s = s;
   newModel.f_f0 = k;
   newModel.D = calculateDiffusionSK(s, k);

   if(US_Model::calc_coefficients(newModel))
   {
      return newModel;
   }
   else
   {
      qDebug() << "calc_coefficients failed in SK model generation. s, f/f0: " << newModel.s << " , " << newModel.f_f0;
      return newModel;
   }
}

//produces a line of solutes with constant RMSD between neighbors
QVector<US_Model::SimulationComponent> US_ModelBuilder::calculateLine(double tolerance, double targetRMSD, QVector2D startCoord, QVector2D endCoord) 
{
   //extract start/end coordinates
   double sStart = startCoord.x();
   double kStart = startCoord.y();
   double sEnd = endCoord.x();
   double kEnd = endCoord.y();

   /*
   qDebug() << "s-start: " << sStart;
   qDebug() << "k-start: " << kStart;
   qDebug() << "s-end: " << sEnd;
   qDebug() << "k-end: " << kEnd;
   */

   QVector<US_Model::SimulationComponent> line;

   bool finished = false;

   US_Model::SimulationComponent start = modelComponentFromSK(sStart, kStart);
   US_Model::SimulationComponent end = modelComponentFromSK(sEnd, kEnd);

   double distanceLimit = calculateDistance(start, end);

   US_DataIO::RawData* terminusSim = simulateComponent(end);
   US_DataIO::RawData* currentPointSim;
   US_DataIO::RawData* testPointSim;

   US_Model::SimulationComponent testPoint;
   US_Model::SimulationComponent highPoint;
   US_Model::SimulationComponent lowPoint;

   //add start to line
   line.append(start);

   //outer loop over line
   do
   {
      //simulate current point
      US_Model::SimulationComponent currentComponent = line.last();
      currentPointSim = simulateComponent(currentComponent);

      //check that sufficient distance remains to find point
      if (calculateScaledRMSD(currentPointSim, currentComponent, terminusSim, end) > targetRMSD)
      {
         double testRMSD;

         //setup low and high points
         lowPoint = line.last();
         highPoint = end;

         int maxDepthInner = 200;
         int countInner = 0;

         //internal loop to find correct point
         do
         {
            countInner++;

            //calculate testPoint
            testPoint = modelComponentFromSK((lowPoint.s + highPoint.s) / 2.0, ((lowPoint.f_f0 + highPoint.f_f0) / 2.0));

            //qDebug() << "simulating component; counter: " << countInner;

            //simulate testpoint
            testPointSim = simulateComponent(testPoint);

            //calculate rmsd
            testRMSD = calculateScaledRMSD(currentPointSim, currentComponent, testPointSim, testPoint);

            //delete simulation
            delete testPointSim;

            //check if RMSD too low
            if (testRMSD < targetRMSD)
            {
               lowPoint = testPoint;
            }

            //check if too high
            else if (testRMSD > targetRMSD)
            {
               highPoint = testPoint;
            }

            //check if recursive depth exceeded
            if(countInner > maxDepthInner)
            {
               qDebug() << "Loop depth exceeded in inner loop for numerical grid gen!";
               finished = true;
            }


         } while (abs(targetRMSD - testRMSD) > tolerance && maxDepthInner > countInner);

         //check that point is valid to add
         if (calculateDistance(start, testPoint) > distanceLimit)
         {
            finished = true;
         }
         else
         {
            //qDebug() << "test RMSD value: " << testRMSD<< " s: " << testPoint.s << " ff0: " << testPoint.f_f0;
            line.append(testPoint);
         }

      }
      else
      {
         finished = true;
      }

      //delete temp variable
      delete currentPointSim;

      //qDebug() << "outer loop recurring";

   } while (!finished);

   //final delete of variables
   delete terminusSim;

   //return line
   return line;
}

// uses the constant RMSD line generator to generate a brute-force grid in the form specified by brookes100915a12.pdf
QVector<QVector<US_Model::SimulationComponent> > US_ModelBuilder::createNumericalGrid(double tolerance, double targetRMSD, double minS, double maxS, double minK, double maxK, QChar majorAxis)
{
   QVector<QVector<US_Model::SimulationComponent> > grid;

   QVector<US_Model::SimulationComponent> firstLineValues;

   //get f/f0-values to put s lines on
   if(majorAxis == 's')
   {
      qDebug() << "Making numerical s-major grid...";
      firstLineValues = calculateLine(tolerance, targetRMSD, QVector2D(minS, minK), QVector2D(maxS, minK)); //for s-major, constant k line first
   }
   else if(majorAxis == 'k')
   {
      qDebug() << "Making numerical k-major grid...";
      firstLineValues = calculateLine(tolerance, targetRMSD, QVector2D(minS, minK), QVector2D(minS, maxK)); //for k-major, constant s line first

   }
   else
   {
      qDebug() << "no major axis specified!";
      return grid;
   }

   //storage for concurrency tracker object
   QVector<QFuture<QVector<US_Model::SimulationComponent> > > futures;

   //make and add s-lines at all k-values
   for(int i = 0; i < firstLineValues.size(); i++)
   {
      double currentS = firstLineValues.at(i).s;
      double currentK = firstLineValues.at(i).f_f0;

      qDebug() << "currentS, currentK: " << currentS << " , " << currentK;

      //request thread for job, and store tracker
      if(majorAxis == 's')
      {
         futures.append(QtConcurrent::run(this, &US_ModelBuilder::calculateLine, tolerance, targetRMSD, QVector2D(currentS, minK) , QVector2D(currentS, maxK))); //for s-major
      }
      else if(majorAxis == 'k')
      {
         futures.append(QtConcurrent::run(this, &US_ModelBuilder::calculateLine, tolerance, targetRMSD, QVector2D(minS, currentK) , QVector2D(maxS, currentK))); //for k-major
      }

      //qDebug() << "Thread added to queue";

      //generate and add line - deprecated
      //grid.append(calculateLine(tolerance, targetRMSD, QVector2D(minS, currentK) , QVector2D(maxS, currentK)));
   }

   //qDebug() << "Waiting for threads to finish...";

   int numFutures = futures.size();

   //wait for threads to finish, and add to grid in correct order
   for(int i = 0; i < numFutures; i++)
   {
      //add result, blocking when calculation is incomplete (so that ordering is preserved)
      grid.append(futures.first().result());
      futures.remove(0);
      qDebug() << "Stored result " << (i + 1) << " of " << numFutures;
   }

   return grid;
}


//finds the RMSDs between adjacent points in each column of the 2d qvector grid and returns them as a qvector of sedimentation, diffusion, f/f0, MW, and RMSD
QVector<QVector<double> > US_ModelBuilder::checkLineRMSDParalell(QVector<QVector<US_Model::SimulationComponent> > grid, bool approximateToMidpoint)
{
   QVector<QVector<double> > values;
   QVector<QFuture<QVector<double> > > futures;

   //qDebug() << "Max thread count: " << QThreadPool::globalInstance()->maxThreadCount();

   //first loop to spawn threads
   for(int i = 0; i < grid.size(); i++)
   {
      QVector<US_Model::SimulationComponent> column = grid.at(i);

      //queue up the RMSD calculation
      futures.append(QtConcurrent::run(this, &US_ModelBuilder::findListRMSD, column));
   }

   //second loop to re-integrate threads and do calculations
   for(int i = 0; i < grid.size(); i++)
   {
      //once again, get our current col
      QVector<US_Model::SimulationComponent> column = grid.at(i);

      //fetch qFutures in the order that they were added, derefrerencing as we go to save memory
      QVector<double> vals = futures.at(0).result();
      futures.remove(0);

      //qDebug() << "Stored rowRMSD " << (i+1) << " of " << grid.size();

      for(int lcv = 1; lcv < column.size(); lcv++)
      {
         QVector<double> data;
         US_Model::SimulationComponent component1 = column.at(lcv - 1);
         US_Model::SimulationComponent component2 = column.at(lcv);
         double s;
         double f_f0;
         double D;
         double mw;

         //place points at average if apprx to mdpoint is enabled
         if(approximateToMidpoint)
         {
            s = (component1.s + component2.s) / 2;
            f_f0 = (component1.f_f0 + component2.f_f0) / 2;
            D = (component1.D + component2.D) / 2;
            mw = (component1.mw + component2.mw) / 2;
         }
         else //otherwise, just place at the first point
         {
            s = component1.s;
            f_f0 = component1.f_f0;
            D = component1.D;
            mw = component1.mw;
         }

         //save data
         data.append(s);
         data.append(D);
         data.append(f_f0);
         data.append(mw);
         data.append(vals.at(lcv - 1));
         values.append(data);
      }
   }

   return values;
}

//Generates a grid on the sk plane with the given params; 
//Note: Major axis is the axis along which we set constant lines; increments on outer QVector, other increments on internal QVector
QVector<QVector<US_Model::SimulationComponent> > US_ModelBuilder::generateSKGrid(double sMin, double sMax, double kMin, double kMax, int sDim, int kDim, QChar constantAxis)
{
   QVector<QVector<US_Model::SimulationComponent> > grid;

   //get rate of change in both dims
   double deltaS = (sMax - sMin) / sDim;
   double deltaK = (kMax - kMin) / kDim;
   double s = 0.0;
   double k = 0.0;

   //make grid for sMajor
   if(constantAxis == 's' || constantAxis == 'S')
   {
      for(int i = 0; i < sDim; i++)
      {
         QVector<US_Model::SimulationComponent> column;
         s = (deltaS * i) + sMin;

         for(int j = 0; j < kDim; j++)
         {
            k = (deltaK * j) + kMin;
            column.append(modelComponentFromSK(s, k));

         }

         grid.append(column);
      }
   }

   //grid for kMajor
   else if(constantAxis == 'k' || constantAxis == 'K')
   {
      for(int i = 0; i < kDim; i++)
      {
         QVector<US_Model::SimulationComponent> column;
         k = deltaK * i+ kMin;

         for(int j = 0; j < sDim; j++)
         {
            s = deltaS * j + sMin;
            column.append(modelComponentFromSK(s, k));
         }

         grid.append(column);
      }
   }

   //otherwise, error
   else
   {
      qDebug() << "ConstantAxis is neither s nor k!";
      return grid;
   }

   //end
   return grid;
}

//Generates a grid on the sd plane with the given params; 
QVector<QVector<US_Model::SimulationComponent> > US_ModelBuilder::generateSDGrid(double sMin, double sMax, double dMin, double dMax, int sDim, int dDim, QChar constantAxis)
{
   QVector<QVector<US_Model::SimulationComponent> > grid;

   //get rate of change in both dims
   double deltaS = (sMax - sMin) / sDim;
   double deltaD = (dMax - dMin) / dDim;
   double s = 0.0;
   double D = 0.0;

   //make grid for sMajor
   if(constantAxis == 's' || constantAxis == 'S')
   {
      for(int i = 0; i < sDim; i++)
      {
         QVector<US_Model::SimulationComponent> column;
         s = (deltaS * i) + sMin;

         for(int j = 0; j < dDim; j++)
         {
            D = (deltaD * j) + dMin;

            US_Model::SimulationComponent current;

            double k = calculateFrictionalRatioSD(s, D);

            current.s = s;
            current.D = D;
            current.f_f0 = k;

            //do coefficient calculation, and proceed only if successful
            if(US_Model::calc_coefficients(current) && current.f_f0 <= 4.0 && current.f_f0 >= 1)
            {
               //qDebug() << "s: " << current.s << " , D: " << current.D << " , f_f0: " << current.f_f0 << endl;
               column.append(current);
            }

         }
         grid.append(column);
      }
   }

   //grid for kMajor
   else if(constantAxis == 'd' || constantAxis == 'D')
   {
      for(int i = 0; i < dDim; i++)
      {
         QVector<US_Model::SimulationComponent> column;
         D = deltaD * i+ dMin;

         for(int j = 0; j < sDim; j++)
         {
            s = deltaS * j + sMin;

            US_Model::SimulationComponent current;

            double k = calculateFrictionalRatioSD(s, D);

            current.s = s;
            current.D = D;
            current.f_f0 = k;

            //do coefficient calculation, and proceed only if successful
            if(US_Model::calc_coefficients(current) && current.f_f0 <= 4.0 && current.f_f0 >= 1)
            {
               //qDebug() << "s: " << current.s << " , D: " << current.D << " , f_f0: " << current.f_f0 << endl;
               column.append(current);
            }

         }

         grid.append(column);
      }
   }

   //otherwise, error
   else
   {
      qDebug() << "ConstantAxis is neither s nor k!";
      return grid;
   }

   //end
   return grid;
}

//Generates a grid on the mk plane with the specifications passed
QVector<QVector<US_Model::SimulationComponent> > US_ModelBuilder::generateMKGrid(double mMin, double mMax, double kMin, double kMax, int mDim, int kDim, QChar constantAxis)
{
   QVector<QVector<US_Model::SimulationComponent> > grid;

   //get rate of change in both dims
   double deltaM = (mMax - mMin) / mDim;
   double deltaK = (kMax - kMin) / kDim;
   double m = 0.0;
   double k = 0.0;

   //make grid for sMajor
   if(constantAxis == 'm' || constantAxis == 'M')
   {
      for(int i = 0; i < mDim; i++)
      {
         QVector<US_Model::SimulationComponent> column;
         m = (deltaM * i) + mMin;

         for(int j = 0; j < kDim; j++)
         {
            k = (deltaK * j) + kMin;

            US_Model::SimulationComponent current;

            double D = calculateDiffusionMK(m, k);
            double s = calculateSedimentationMD(m, D);

            current.mw = m;
            current.s = s;
            current.D = D;
            current.f_f0 = k;

            //do coefficient calculation, and proceed only if successful
            if(US_Model::calc_coefficients(current) && current.f_f0 <= 4.0 && current.f_f0 >= 1)
            {
               //qDebug() << "s: " << current.s << " , D: " << current.D << " , f_f0: " << current.f_f0 << endl;
               column.append(current);
            }

         }
         grid.append(column);
      }
   }

   //grid for kMajor
   else if(constantAxis == 'k' || constantAxis == 'K')
   {
      for(int i = 0; i < kDim; i++)
      {
         QVector<US_Model::SimulationComponent> column;
         k = (deltaK * i) + kMin;

         for(int j = 0; j < mDim; j++)
         {
            m = (deltaM * j) + mMin;

            US_Model::SimulationComponent current;

            double D = calculateDiffusionMK(m, k);
            double s = calculateSedimentationMD(m, D);

            current.mw = m;
            current.s = s;
            current.D = D;
            current.f_f0 = k;

            //do coefficient calculation, and proceed only if successful
            if(US_Model::calc_coefficients(current) && current.f_f0 <= 4.0 && current.f_f0 >= 1)
            {
               //qDebug() << "s: " << current.s << " , D: " << current.D << " , f_f0: " << current.f_f0 << endl;
               column.append(current);
            }

         }

         grid.append(column);
      }
   }

   //otherwise, error
   else
   {
      qDebug() << "ConstantAxis is neither s nor k!";
      return grid;
   }

   //end
   return grid;
}

//calculates distribution statistics on a specified grid, calculating along inner QVectors
//Returns data in a size 3 qvector in the form: pointcount, average RMSD, standard deviation
QVector<double> US_ModelBuilder::calculateGridStatistics(QVector<QVector<double> > processedGrid)
{
   int numPoints = 0.0;
   double average = 0.0;
   double stddev = 0.0;
   QVector<double> data;

   //calculate average
   for(int i = 0; i < processedGrid.size(); i++)
   {
      average += processedGrid.at(i).at(4);
      numPoints++;
   }
   average = average / ((double) numPoints);

   //calculate standard deviation
   for(int i = 0; i < processedGrid.size(); i++)
   {
      stddev += sq(processedGrid.at(i).at(4) - average);
   }
   stddev = sqrt((1 / (numPoints - 1.0)) * stddev);

   data.append(numPoints);
   data.append(average);
   data.append(stddev);

   return data;
}

//Calculates RMSD levels for an irregular grid;
//Accomplished by simulating every solute, storing the simulations in the heap, and then finding RMSD relative to every point
//A point's RMSD is determined by taking the average RMSD of the 6 lowest values
//NOTE: UNDER NO CONDITIONS PASS A LARGE GRID IN HERE
//TODO: Optimize by selecting nearest 20 or so points in sk or sd grid for comparison instead of calculating RMSD for everything
//returns them as a qvector of sedimentation, diffusion, f/f0, MW, and RMSD
//QVector<std::tuple> US_ModelBuilder::calculateIrregularGridRMSD(QVector<QVector<US_Model::SimulationComponent> >)
//{
//   QVector<std::tuple> results;

//   return results;
//}

