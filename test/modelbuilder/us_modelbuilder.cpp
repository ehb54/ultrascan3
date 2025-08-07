//! \file us_modelbuilder.cpp

#include "us_modelbuilder.h"
#include <QApplication>
#include <QDomDocument>
#include "us_math2.h"

//using namespace std;
#ifndef DbgLv
#define DbgLv(a) \
   if (dbg_level >= a) \
   qDebug()
#endif

//! \brief Main program for US_ModelBuilder. Loads translators and starts
//         the class US_ModelBuilder.

//global variable instance number
QString instanceNum;

int main(int argc, char *argv[]) {
   QApplication application(argc, argv);

#include "main1.inc"

   // License is OK.  Start up.

   //if argc == 0, set instance number to 0
   if (argc == 0)
      instanceNum = "0";
   else
      instanceNum = QString(argv[ 1 ]);

   US_ModelBuilder w;

   //Uncomment to enable GUI display (also uncomment the entire constructor below)
   //w.show(); //!< \memberof QWidget

   return application.exec(); //!< \memberof QApplication
}

// Constructor

US_ModelBuilder::US_ModelBuilder() : US_Widgets() {
   //Enclosed lines setup and start the application, skipping GUI
   /////////////////////////////////////

   //set simparams to defualt values
   initSimparams();

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

   US_SimulationParametersGui *dialog = new US_SimulationParametersGui(working_simparams);

   connect(dialog, SIGNAL(complete()), SLOT(updateParameters()));

   dialog->exec();
}

//this function updates the global simparams variable with user data

void US_ModelBuilder::updateParameters(void) {
   simparams = working_simparams;
}

//this function allows the user to begin the simulation
void US_ModelBuilder::startSimulation(void) {
   /*
   double sMin = 1e-13;
   double sMax = 10e-13;

   double kMin = 1.0;
   double kMax = 4.0;

   double dMin = 1e-7;
   double dMax = 6e-7;

   double mMin = 5000;
   double mMax = 970000;

   int dim = 20;
   */

   //find out the hostname
   char tempHN[ 1024 ];
   tempHN[ 1023 ] = '\0';
   gethostname(tempHN, 1024);
   QString hostname = QString::fromLocal8Bit(tempHN);
   qDebug() << hostname;

   qDebug() << "Beginning grid processing/output...";

   //RMSD isoline testing
   /*
   double deltaS = (9.75e-13 - 1.25e-13) / 30;
   double deltaK = (3.75 - 1.25) / 5;
   double s = 0.0;
   double k = 0.0;
   */

   //special faxen grid run
   /*
   QVector<QVector<US_Model::SimulationComponent> > fg = generateFaxenGrid(1.1e-13, 9.9e-13, 1.2, 3.8, 270);
   //QVector<QVector<US_Model::SimulationComponent> > fg = generateFaxenGrid(1.1e-13, 9.99e-13, 1.05, 3.95, 270);
   QVector<US_Model::SimulationComponent> faxenPoints1;
   QVector<US_Model::SimulationComponent> faxenPoints2;
   for(int i = 0; i < fg.size(); i++)
   {
      for(int j = 0; j < fg.at(i).size(); j++)
      {
         US_Model::SimulationComponent current = fg.at(i).at(j);

         if(current.s >= 9.6e-13 && current.f_f0 >= 3)
         {
            faxenPoints1.append(current);
         }
         else if(current.s >= 1.2e-13 && current.s <= 1.5e-13 && current.f_f0 <= 1.5)
         {
            faxenPoints2.append(current);
         }
      }
   }

   //target RMSD levels
   QVector<double> rmsdTargets;
   rmsdTargets.append(1e-3);
   rmsdTargets.append(2e-3);

   runIsoRMSDTest(faxenPoints1, rmsdTargets, 10, "largeFaxen_UpperRight");
   runIsoRMSDTest(faxenPoints2, rmsdTargets, 10, "largeFaxen_LowerLeft");


   //special sk grid run

   QVector<QVector<US_Model::SimulationComponent> > sk = generateSKGrid(1.1e-13, 9.9e-13, 1.2, 3.8, 340, 62, 's');
   QVector<US_Model::SimulationComponent> skPoints1;
   QVector<US_Model::SimulationComponent> skPoints2;
   for(int i = 0; i < sk.size(); i++)
   {
      for(int j = 0; j < sk.at(i).size(); j++)
      {
         US_Model::SimulationComponent current = sk.at(i).at(j);

         if(current.s >= 9.6e-13 && current.f_f0 >= 3)
         {
            skPoints1.append(current);
         }
         else if(current.s >= 1.2e-13 && current.s <= 1.5e-13 && current.f_f0 <= 1.5)
         {
            skPoints2.append(current);
         }
      }
   }

   runIsoRMSDTest(skPoints1, rmsdTargets, 10, "largeSK_UpperRight");
   runIsoRMSDTest(skPoints2, rmsdTargets, 10, "largeSK_LowerLeft");
   */

   //run isormsd
   /*
   QVector<double> targets;
   targets.append(0.01);
   targets.append(0.0075);
   targets.append(0.005);
   targets.append(0.0025);
   targets.append(0.001);

   QVector<double> times;
   times.append(3.4);
   times.append(6.8);
   times.append(34);

   QVector<double> speeds;
   speeds.append(60000); //actual target 63460, but outside of bounds
   speeds.append(40135);
   speeds.append(29915);

   //foreach(double time, times)
   foreach(double speed, speeds)
   {
      //qDebug() << "running grids for time:" << time;

      //set duration
      //simparams.speed_step[0].duration_hours = (int) time;
      //simparams.speed_step[0].duration_minutes = (int) ((time - ((int) time)) * 60);

      //set speed
      qDebug() << "Running grids for speed:" << speed;
      simparams.speed_step[0].rotorspeed = speed;

      qDebug() << "\tMaking faxen grid";
      QVector<QVector<US_Model::SimulationComponent> > fg = generateFaxenGrid(1.1e-13, 9.9e-13, 1.2, 3.8, 26);
      QVector<US_Model::SimulationComponent> faxenPoints;
      for(int i = 0; i < fg.size(); i++)
      {
         faxenPoints.append(fg.at(i));
      }

      runIsoRMSDTest(faxenPoints, targets, 2, "Faxen_speed" + QString::number(speed));

      qDebug()<< "\tMaking sk grid";
      QVector<QVector<US_Model::SimulationComponent> > sk = generateSKGrid(1.1e-13, 9.9e-13, 1.2, 3.8, 35, 6, QChar('s'));
      QVector<US_Model::SimulationComponent> skPoints;
      for(int i = 0; i < sk.size(); i++)
      {
         skPoints.append(sk.at(i));
      }
      runIsoRMSDTest(skPoints, targets, 2, "skGrid_speed" + QString::number(speed));

      qDebug()<< "\tMaking sd grid";
      QVector<QVector<US_Model::SimulationComponent> > sd = generateSDGrid(1.1e-13, 9.9e-13, 8.55883e-08, 1.4469e-06, 46, 8, QChar('s'));
      QVector<US_Model::SimulationComponent> sdPoints;
      for(int i = 0; i < sd.size(); i++)
      {
         sdPoints.append(sd.at(i));
      }
      runIsoRMSDTest(sdPoints, targets, 2, "sdGrid_speed" + QString::number(speed));
      //qDebug() << "sdsize:" << sdPoints.size();

      qDebug()<< "\tMaking mk grid";
      QVector<QVector<US_Model::SimulationComponent> > mk = generateMKGrid(6587.98, 1.00235e+06, 1.2, 3.8, 32, 12, QChar('m'));
      QVector<US_Model::SimulationComponent> mkPoints;
      for(int i = 0; i < mk.size(); i++)
      {
         mkPoints.append(mk.at(i));
      }
      runIsoRMSDTest(mkPoints, targets, 2, "mkGrid_speed" + QString::number(speed));
   }
   */


   //isoRMSDs from JSON
   /*
   QFile jsonInputFile ("input/faxen.json");
   jsonInputFile.open(QIODevice::ReadOnly | QIODevice::Text);
   QString jsonString = QString(jsonInputFile.readAll());

   QJsonArray jsonSlopeArray = QJsonDocument::fromJson(jsonString.toUtf8()).array();

   QVector<double> targets;
   targets.append(0.01);
   targets.append(0.0075);
   targets.append(0.005);
   targets.append(0.0025);
   targets.append(0.001);

   QPair<QVector<QVector<QVector<US_Model::SimulationComponent> > >, QVector<US_Model::SimulationComponent> > sets = levelSetFromJson(jsonSlopeArray, targets);
   processJsonSlopes(sets.first, sets.second, targets, "jsonFaxenOut");
   */

   /*
   QVector<US_Model::SimulationComponent> testPoints(1);
   testPoints[0] = componentFromSK(1.6e-13, 1.4);
   QVector<double> testTargets;
   testTargets.append(0.01);
   testTargets.append(0.005);
   testTargets.append(0.001);

   runIsoRMSDTest(testPoints, testTargets, 60, "test");
   */

   //getLevelSets()

   /*
   for(int i = 4; i < 400; i += 2)
   {
      QVector<QVector<US_Model::SimulationComponent> > fg = generateFaxenGrid(1.25e-13, 9.75e-12, 1.1, 3.9, i);
      int count = 0;
      for(int j = 0; j < fg.size(); j++)
      {
         count += fg.at(j).size();
      }


      qDebug() << i << ", " << count;
   }
*/

   /*
   US_Model::SimulationComponent m1 = componentFromSK(1.1e-13, 1.2);
   US_Model::SimulationComponent m2 = componentFromSK(9.9e-13, 3.8);

   qDebug() << "D, mw:" << m1.D << "," << m1.mw;
   qDebug() << "D, mw:" << m2.D << "," << m2.mw;
   */

   //TODO: remove temporary exit
   qDebug() << "Exiting \"normally\"";
   exit(0);
}

//Runs isormsd tests, and outputs to specified directory
void US_ModelBuilder::runIsoRMSDTest(
   QVector<US_Model::SimulationComponent> points, QVector<double> rmsdTargets, double increment, QString outputname) {
   //check outfile directory existence
   QDir outputDirectory("output/isoRMSD_" + outputname);
   if (!outputDirectory.exists()) {
      outputDirectory.mkpath(".");
   }
   //iterate over RMSD directories
   for (int i = 0; i < rmsdTargets.size(); i++) {
      QDir outputRMSDDirectory("output/isoRMSD_" + outputname + "/RMSD_" + QString::number(rmsdTargets.at(i)));

      if (!outputRMSDDirectory.exists()) {
         outputRMSDDirectory.mkpath(".");
      }
   }

   //setup json objects to store slopes
   QJsonArray jsonSlopesArray;
   QJsonDocument jsonSlopes;

   //thread result storage
   QVector<QFuture<QPair<QVector<QVector<US_Model::SimulationComponent>>, QVector<QPair<double, double>>>>> futures;

   //setup threads
   for (int i = 0; i < points.size(); i++) {
      //queue up job
      futures.append(QtConcurrent::run(this, &US_ModelBuilder::getLevelSets, points.at(i), rmsdTargets, increment));
   }

   //get thread results in order
   for (int i = 0; i < points.size(); i++) {
      //status update every 10 results
      if ((i + 1) % 10 == 0) {
         qDebug() << "\t\tgetting result " << (i + 1) << " of " << points.size();
      }

      QPair<QVector<QVector<US_Model::SimulationComponent>>, QVector<QPair<double, double>>> results
         = futures.at(0).result();
      futures.remove(0);

      QVector<QVector<US_Model::SimulationComponent>> resultPoints = results.first;
      QVector<QPair<double, double>> slopes = results.second;

      //iterate over rmsd levels
      for (int j = 0; j < rmsdTargets.size(); j++) {
         if (resultPoints.size() != 0) {
            //output file
            QFile outfileIsoLines(
               "output/isoRMSD_" + outputname + "/RMSD_" + QString::number(rmsdTargets.at(j)) + "/" + outputname + "_"
               + QString::number(rmsdTargets.at(j)) + "_" + QString::number(points.at(i).s) + "s_"
               + QString::number(points.at(i).f_f0) + "k.tsv");
            outfileIsoLines.open(QFile::WriteOnly | QFile::Truncate);
            QTextStream outstreamIsoLines(&outfileIsoLines);
            outstreamIsoLines << "#Sed. Coeff. \t Diff. Coeff. \t Frict. Ratio \t Mol. Wt." << endl;

            //iterate over rmsd levels
            for (int k = 0; k < resultPoints.at(j).size(); k++) {
               US_Model::SimulationComponent currentComponent = resultPoints.at(j).at(k);
               outstreamIsoLines << currentComponent.s << " \t" << currentComponent.D << " \t" << currentComponent.f_f0
                                 << " \t" << currentComponent.mw << endl;
            }

            outfileIsoLines.close();
         }
      }

      //store slopes
      QJsonObject pointSlopes;
      QJsonArray pointSlopeArray;
      QJsonArray pointDegreeArray;

      pointSlopes.insert("s_value", QJsonValue(points.at(i).s));
      pointSlopes.insert("k_value", QJsonValue(points.at(i).f_f0));

      QPair<double, double> slope;
      foreach (slope, slopes) {
         pointSlopeArray.append(QJsonValue(slope.first));
         pointDegreeArray.append(QJsonValue(slope.second));
      }
      pointSlopes.insert("slopes", pointSlopeArray);
      pointSlopes.insert("degrees", pointDegreeArray);

      jsonSlopesArray.append(QJsonValue(pointSlopes));
   }

   //package and output slopes
   jsonSlopes.setArray(jsonSlopesArray);
   QByteArray jsonByteData = jsonSlopes.toJson();

   QFile outfileSlopes("output/isoRMSD_" + outputname + "/slopes.json");
   outfileSlopes.open(QFile::WriteOnly | QFile::Truncate);
   outfileSlopes.write(jsonByteData);
   outfileSlopes.close();
}

void US_ModelBuilder::processJsonSlopes(
   QVector<QVector<QVector<US_Model::SimulationComponent>>> sets, QVector<US_Model::SimulationComponent> centerpoints,
   QVector<double> targets, QString outputname) {
   //check output directory existence
   QDir outputDirectory("jsonOutput/isoRMSD_" + outputname);
   if (!outputDirectory.exists()) {
      outputDirectory.mkpath(".");
   }
   foreach (double target, targets) {
      QDir outputRMSDDirectory(outputDirectory.path() + "/RMSD_" + QString::number(target));

      if (!outputRMSDDirectory.exists()) {
         outputRMSDDirectory.mkpath(".");
      }
   }

   //iterate over centerpoints and sets
   for (int i = 0; i < centerpoints.size(); i++) {
      US_Model::SimulationComponent centerpoint = centerpoints.at(i);

      //iterate over rmsd targets
      for (int j = 0; j < targets.size(); j++) {
         //output
         QFile outfile(
            outputDirectory.path() + "/RMSD_" + QString::number(targets.at(j)) + "/" + outputname + "_"
            + QString::number(targets.at(j)) + "_" + QString::number(centerpoint.s) + "s_"
            + QString::number(centerpoint.f_f0) + "k.tsv");
         outfile.open(QFile::WriteOnly | QFile::Truncate);
         QTextStream outstream(&outfile);
         outstream << "#Sed. Coeff. \t Diff. Coeff. \t Frict. Ratio \t Mol. Wt." << endl;

         //iterate over target-RMSD levelset
         QVector<US_Model::SimulationComponent> currentSet = sets.at(i).at(j);
         foreach (US_Model::SimulationComponent component, currentSet) {
            outstream << component.s << " \t" << component.D << " \t" << component.f_f0 << " \t" << component.mw
                      << endl;
         }
      }
   }
}

//reads a QJsonObject containing stored slopes and uses them to project RMSD levels in each direction for every point with stored data
//NOTE: top level qvector has 1 index / centerpoint, mid level has 1 index / rmsd target, bottom has 1 index / projected point
QPair<QVector<QVector<QVector<US_Model::SimulationComponent>>>, QVector<US_Model::SimulationComponent>>
US_ModelBuilder::levelSetFromJson(QJsonArray slopeJson, QVector<double> targets) {
   QVector<QVector<QVector<US_Model::SimulationComponent>>> gridSets;
   QVector<US_Model::SimulationComponent> centerpoints;

   //loop over all points
   for (int i = 0; i < slopeJson.size(); i++) {
      //get current point data
      QJsonObject currentJsonValue = slopeJson.at(i).toObject();
      QJsonArray currentSlopes = currentJsonValue.value("slopes").toArray();
      QJsonArray currentDegrees = currentJsonValue.value("degrees").toArray();
      double s = currentJsonValue.value("s_value").toDouble();
      double k = currentJsonValue.value("k_value").toDouble();
      centerpoints.append(componentFromSK(s, k));

      QVector<QVector<US_Model::SimulationComponent>> levelSets;

      //loop over all targets
      foreach (double target, targets) {
         QVector<US_Model::SimulationComponent> levelSet;

         //loop over all slopes
         for (int j = 0; j < currentSlopes.size(); j++) {
            double slope = currentSlopes.at(j).toDouble();
            double degree = currentDegrees.at(j).toDouble();

            //project in direction and add to levelset
            double projectionDistance = target / slope;
            US_Model::SimulationComponent projected
               = componentFromSKPair(projectPolar(s * 1e13, k, projectionDistance, degree), true);

            //check that projection is legal
            if (projected.s >= 1e-13 && projected.f_f0 >= 1) {
               levelSet.append(projected);
            }
         }

         levelSets.append(levelSet);
      }

      gridSets.append(levelSets);
   }

   return QPair<QVector<QVector<QVector<US_Model::SimulationComponent>>>, QVector<US_Model::SimulationComponent>>(
      gridSets, centerpoints);
}

//interpolates the RMSD of the point 'target' using the RMSDs of 4 points on a regular grid
double US_ModelBuilder::interpolatePoint(QVector2D target, QVector3D p11, QVector3D p12, QVector3D p21, QVector3D p22) {
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
void US_ModelBuilder::initSimparams() {
   /*if (simparams.load_simparms("Resources/sp_defaultProfle.xml") != 0) {
        startButton->setEnabled(false);
        qDebug() << "Error: Simulation Parameters cannot be loaded. Manual selection required.";
    }*/

   US_SimulationParameters::SpeedProfile sp;
   QString rotor_calibr = "0";
   //double rpm = 50000.0;
   double rpm = 40000.0;

   // set up bottom start and rotor coefficients from hardware file
   simparams.setHardware(NULL, rotor_calibr, 0, 0);

   // calculate bottom from rpm, channel bottom pos., rotor coefficients
   double bottom = US_AstfemMath::calc_bottom(rpm, simparams.bottom_position, simparams.rotorcoeffs);
   bottom = ( double ) (qRound(bottom * 1000.0)) * 0.001;

   simparams.mesh_radius.clear();
   simparams.speed_step.clear();

   //sp.duration_hours    = 5;
   //sp.duration_minutes  = 30.0;
   sp.duration_hours = 6;
   sp.duration_minutes = 48.0;

   sp.delay_hours = 0;
   //sp.delay_minutes     = 20.0;
   sp.delay_minutes = 2.0;
   sp.rotorspeed = ( int ) rpm;
   //sp.scans             = 30;
   sp.scans = 100; //used to use 30
   sp.acceleration = 400;
   sp.acceleration_flag = true;

   simparams.speed_step << sp;

   simparams.simpoints = 200;
   simparams.radial_resolution = 0.001;
   simparams.meshType = US_SimulationParameters::ASTFEM;
   simparams.gridType = US_SimulationParameters::MOVING;
   simparams.meniscus = 5.8;
   simparams.bottom = bottom;
   simparams.rnoise = 0.0;
   simparams.tinoise = 0.0;
   simparams.rinoise = 0.0;
   simparams.band_volume = 0.015;
   simparams.rotorCalID = rotor_calibr;
   simparams.band_forming = false;

   //set workingsimparams
   working_simparams = simparams;
}

//this function produces an initalized RawData sim object using given simparams
US_DataIO::RawData *US_ModelBuilder::initRawData(US_Model *system) {
   //instantiate object to be initalized
   US_DataIO::RawData *working_simdata = new US_DataIO::RawData();

   //init
   working_simdata->xvalues.clear();
   working_simdata->scanData.clear();

   working_simdata->type[ 0 ] = 'R';
   working_simdata->type[ 1 ] = 'A';

   working_simdata->cell = 1;
   working_simdata->channel = 'S';
   working_simdata->description = "Simulation";

   int points = qRound((simparams.bottom - simparams.meniscus) / simparams.radial_resolution) + 1;

   working_simdata->xvalues.resize(points);

   for (int i = 0; i < points; i++) {
      working_simdata->xvalues[ i ] = simparams.meniscus + i * simparams.radial_resolution;
   }

   int total_scans = 0;

   for (int i = 0; i < simparams.speed_step.size(); i++) {
      total_scans += simparams.speed_step[ i ].scans;
   }

   working_simdata->scanData.resize(total_scans);
   int terpsize = (points + 7) / 8;

   for (int i = 0; i < total_scans; i++) {
      US_DataIO::Scan *scan = &working_simdata->scanData[ i ];

      scan->temperature = simparams.temperature;
      scan->rpm = simparams.speed_step[ 0 ].rotorspeed;
      scan->omega2t = 0.0;

      scan->wavelength = system->wavelength;
      scan->plateau = 0.0;
      scan->delta_r = simparams.radial_resolution;

      scan->rvalues.fill(0.0, points);
      scan->interpolated.fill(0, terpsize);
   }

   double w2t_sum = 0.0;
   double delay = simparams.speed_step[ 0 ].delay_hours * 3600.0 + simparams.speed_step[ 0 ].delay_minutes * 60.0;
   double current_time = 0.0;
   double duration;
   double increment = 0.0;
   int scan_number = 0;

   for (int ii = 0; ii < simparams.speed_step.size(); ii++) {
      US_SimulationParameters::SpeedProfile *sp = &simparams.speed_step[ ii ];
      double w2t = sq(sp->rotorspeed * M_PI / 30.0);

      delay = sp->delay_hours * 3600. + sp->delay_minutes * 60.;
      duration = sp->duration_hours * 3600. + sp->duration_minutes * 60.;
      increment = (duration - delay) / ( double ) (sp->scans - 1);
      double w2t_inc = increment * w2t;
      current_time += delay;
      w2t_sum = (ii == 0) ? (current_time * w2t) : w2t_sum;

      for (int jj = 0; jj < sp->scans; jj++) {
         US_DataIO::Scan *scan = &working_simdata->scanData[ scan_number ];
         scan->seconds = ( double ) qRound(current_time);
         scan->omega2t = w2t_sum;
         w2t_sum += w2t_inc;
         current_time += increment;
         scan_number++;
      }

      int j1 = scan_number - sp->scans;
      int j2 = scan_number - 1;

      sp->w2t_first = working_simdata->scanData[ j1 ].omega2t;
      sp->w2t_last = working_simdata->scanData[ j2 ].omega2t;
      sp->time_first = working_simdata->scanData[ j1 ].seconds;
      sp->time_last = working_simdata->scanData[ j2 ].seconds;
   }

   simparams.mesh_radius.clear();
   simparams.firstScanIsConcentration = false;

   //return initalized dataset
   return working_simdata;
}

//NOTE: Code from rewrite only below this point

// get the numerical distance between two SimulationComponents on the grid
double US_ModelBuilder::calculateDistance(
   US_Model::SimulationComponent first, US_Model::SimulationComponent second, bool scaleS) {
   if (scaleS) {
      return sqrt(pow((1e13 * (first.s - second.s)), 2) + pow(first.f_f0 - second.f_f0, 2));
   }
   else {
      return sqrt(pow((first.s - second.s), 2) + (pow(first.f_f0 - second.f_f0, 2)));
   }
}

// compute the value of f_f0 for a solute with specified s and D values
double US_ModelBuilder::calculateFrictionalRatioSD(double s, double D) {
   //constants
   double R = 8.314e7; // already defined in US_Constants
   double T = 293.15;
   double vbar = 0.72;
   double eta = 0.0100914;
   double N = 6.022e23;
   double rho = 0.998234;

   //calculations
   double f = R * (T / (N * D));
   double M = (s * N * f) / (1 - vbar * rho);
   double V = M * (vbar / N);
   double r_0 = cbrt((3 * V) / (4 * M_PI));
   double f_0 = 6 * M_PI * eta * r_0;

   return (f / f_0);
}

double US_ModelBuilder::calculateDiffusionSK(double s, double k) {
   double R = 8.314e7;
   double T = 293.15;
   double vbar = 0.72;
   double eta = 0.0100914;
   double N = 6.022e23;
   double rho = 0.998234;

   double D;

   D = (R * T) / (N * k * 9.0 * eta * M_PI * pow((2.0 * s * k * vbar * eta) / (1.0 - vbar * rho), 0.5));

   return D;
}

// calculates diffusion from molecular weight and f/f0
double US_ModelBuilder::calculateDiffusionMK(double M, double k) {
   double R = 8.314e7;
   double T = 293.15;
   double vbar = 0.72;
   double eta = 0.0100914;
   double N = 6.022e23;

   double V = M * (vbar / N);
   double r_0 = cbrt((3 * V) / (4 * M_PI));
   double f_0 = 6 * M_PI * eta * r_0;
   double f = k * f_0;

   double D = (R * T) / (N * f);

   return D;
}

//calculates sedimentation coefficient from molecular weight and diffusion coefficient
double US_ModelBuilder::calculateSedimentationMD(double M, double D) {
   double R = 8.314e7;
   double T = 293.15;
   double vbar = 0.72;
   double rho = 0.998234;

   double s = D * ((M * (1 - vbar * rho)) / (R * T));

   return s;
}

//Takes an ordered list of SCs, simulates each, and returns the list of resulting RMSD values from (0,1),(1,2), etc.
QVector<double> US_ModelBuilder::findListRMSD(QVector<US_Model::SimulationComponent> components) {
   QVector<double> rmsdList;
   US_DataIO::RawData *firstPoint;
   US_DataIO::RawData *secondPoint;

   if (components.size() >= 2) {
      //get the first simulation
      firstPoint = simulateComponent(components.at(0));
      US_Model::SimulationComponent component1 = components.at(0);

      //iterate over list
      for (int i = 1; i < components.size(); i++) {
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

   else {
      qDebug() << "Error: less than 2 components in QVector";
      return rmsdList;
   }
}

//Creates a model from a single component and performs ASTFEM simulation to produce raw data
//TODO: remove dependence on non-updated method init_simData
US_DataIO::RawData *US_ModelBuilder::simulateComponent(US_Model::SimulationComponent component) {
   //check that s, k values are sane
   if (component.s >= 1e-14 && component.f_f0 >= 1) {
      //Make a model, and add the component
      US_Model model;
      model.components << component;

      //model.debug();

      //create the result object
      US_DataIO::RawData *simulation = initRawData(&model);

      //create the simulator
      US_Astfem_RSA *simulator = new US_Astfem_RSA(model, simparams);

      //run sim
      simulator->calculate(*simulation);

      //clean and return
      delete simulator;
      return simulation;
   }
   else {
      //qDebug() << "Simulator called with invalid component params! Aborting...";
      //qDebug() << "s, k: " << component.s << ", " << component.f_f0;
      throw std::invalid_argument("Attempted to simulate invalid component.");
   }
}

//For two simulations, calculate the RMSD between only useful scans
double US_ModelBuilder::calculateScaledRMSD(
   US_DataIO::RawData *simulation1, US_Model::SimulationComponent component1, US_DataIO::RawData *simulation2,
   US_Model::SimulationComponent component2) {
   //TODO: check that all the params are the same between the simulatons (especially mensiscus position)

   //scan number check
   if (simulation1->scanCount() != simulation2->scanCount()) {
      qDebug() << "Error: simulations do not match";
      throw std::invalid_argument("Passed simulation scancounts do not match");
   }

   //Check that simparams has only one SpeedProfile
   if (simparams.speed_step.size() != 1)

   {
      qDebug() << "Error: simulations do not match";
      throw std::invalid_argument("Passed simulations have different speedprofiles");
   }

   //time check
   for (int i = 0; i < simulation1->scanCount(); i++) {
      if (simulation1->scanData.at(i).seconds != simulation2->scanData.at(i).seconds) {
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
   double omega2 = pow((rpm * M_PI) / 30.0, 2);
   double rMeniscus = simparams.meniscus;
   double rBottom = simparams.bottom;
   double left = rMeniscus + 0.025;
   //double right = rBottom - 0.1;

   double deltaT1 = log(rBottom / rMeniscus) / (omega2 * s1);
   double deltaT2 = log(rBottom / rMeniscus) / (omega2 * s2);
   double deltaTMin = min(deltaT1, deltaT2);

   //qDebug() << "delta T 1, delta T 2: " << deltaT1 << " , " << deltaT2;

   int startIndex = min(US_DataIO::index(simulation1, left), US_DataIO::index(simulation2, left));

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
   for (int i = 0; i < simulation1->scanCount() && !pastDeltaT; i++) {
      US_DataIO::Scan scan1 = simulation1->scanData.at(i);
      US_DataIO::Scan scan2 = simulation2->scanData.at(i);

      if (deltaTMin >= scan1.seconds && deltaTMin >= scan2.seconds) {
         scanCount++;
         int endIndex = min(scan1.rvalues.size(), scan2.rvalues.size());

         //find plateau concentrations
         double plateauConcentration1 = C01 * pow(M_E, -2.0 * s1 * omega2 * scan1.seconds);
         double plateauConcentration2 = C02 * pow(M_E, -2.0 * s1 * omega2 * scan2.seconds);

         //qDebug() << "plateau con1, con2: " << plateauConcentration1 << " , " << plateauConcentration2;

         double minPlateauConcentration = min(plateauConcentration1, plateauConcentration2);

         //do calculations for scan - run until values reach 1/2 of the minPlateauConcentration
         bool pastHalfPlateau = false;
         for (int j = startIndex; j < endIndex && !pastHalfPlateau; j++) {
            double val1 = simulation1->value(i, j);
            double val2 = simulation2->value(i, j);

            //outstream << val1 << " , " << val2 << " , " << scan1.seconds << " , " << minPlateauConcentration << endl;

            if (val1 * 2 < minPlateauConcentration && val2 * 2 < minPlateauConcentration) {
               pointCount++;
               rmsd += sq(val1 - val2);
            }
            else {
               pastHalfPlateau = true;
            }
         }
      }
      else {
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

//wrapper around calculateScaledRMSD using QPairs
double US_ModelBuilder::calculateScaledRMSD(
   QPair<US_Model::SimulationComponent, US_DataIO::RawData *> pair1,
   QPair<US_Model::SimulationComponent, US_DataIO::RawData *> pair2) {
   return calculateScaledRMSD(pair1.second, pair1.first, pair2.second, pair2.first);
}

//switches row-column orientation of a 2d simcomponent qvector; requires that the r-c dimensions be equal
QVector<QVector<US_Model::SimulationComponent>> US_ModelBuilder::switchRegularGridOrientation(
   QVector<QVector<US_Model::SimulationComponent>> regular) {
   QVector<QVector<US_Model::SimulationComponent>> rotated;

   //check dimensions are equal
   for (int i = 0; i < regular.size(); i++) {
      if (regular.size() != regular.at(i).size()) {
         qDebug() << "Grid orientation cannot proceed if dimensions are not all equivalent!";
         return rotated;
      }
   }

   //iterate
   for (int i = 0; i < regular.size(); i++) {
      //create new perpendicular vector
      QVector<US_Model::SimulationComponent> row;

      for (int j = 0; j < regular.size(); j++) {
         row.append(regular.at(j).at(i));
      }

      rotated.append(row);
   }

   return rotated;
}

//takes regularly spaced grid, tests RMSD in both directions
//Creates a paralell grid of simulation data that is populated as needed to ensure that each simulation is done only once
QVector<QPair<US_Model::SimulationComponent, double>> US_ModelBuilder::testRegularGrid(
   QVector<QVector<US_Model::SimulationComponent>> grid) {
   QVector<QPair<US_Model::SimulationComponent, double>> results;

   //declare simulation data storage structures
   QVector<QVector<US_DataIO::RawData *>> simulationData;
   for (int i = 0; i < grid.size(); i++) {
      QVector<US_DataIO::RawData *> column(grid.at(i).size(), NULL);
      simulationData.append(column);
   }

   //do calculation for every point
   for (int i = 0; i < grid.size(); i++) {
      for (int j = 0; j < grid.at(i).size(); j++) {
         int numComparisons = 0;
         double rmsdSum = 0.0;
         US_Model::SimulationComponent current = grid.at(i).at(j);

         //simulate the current point if not already simulated
         if (simulationData.at(i).at(j) == NULL) {
            //remove the NULL pointer and replace with the simulation
            delete simulationData.at(i).at(j); //ensure we don't memory leak here
            simulationData[ i ].replace(j, simulateComponent(current));
         }

         //check above (i - 1)
         if ((i - 1) >= 0 && (i - 1) < grid.size() && (j) >= 0 && (j < grid.at(i - 1).size())) {
            numComparisons++;

            //simulate if not already done
            if (simulationData.at(i - 1).at(j) == NULL) {
               delete simulationData.at(i - 1).at(j);
               simulationData[ i - 1 ].replace(j, simulateComponent(grid.at(i - 1).at(j)));
            }

            //calculate rmsd between current point and point above
            rmsdSum += calculateScaledRMSD(
               simulationData.at(i).at(j), current, simulationData.at(i - 1).at(j), grid.at(i - 1).at(j));
         }

         //check right (j + 1)
         if ((i) >= 0 && (i) < grid.size() && (j + 1) >= 0 && ((j + 1) < grid.at(i).size())) {
            numComparisons++;

            //simulate if not already done
            if (simulationData.at(i).at(j + 1) == NULL) {
               delete simulationData.at(i).at(j + 1);
               simulationData[ i ].replace(j + 1, simulateComponent(grid.at(i).at(j + 1)));
            }

            //calculate rmsd between current point and point to the right
            rmsdSum += calculateScaledRMSD(
               simulationData.at(i).at(j), current, simulationData.at(i).at(j + 1), grid.at(i).at(j + 1));
         }

         //check below (i + 1)
         if ((i + 1) >= 0 && (i + 1) < grid.size() && (j) >= 0 && (j < grid.at(i + 1).size())) {
            numComparisons++;

            //simulate if not already done
            if (simulationData.at(i + 1).at(j) == NULL) {
               delete simulationData.at(i + 1).at(j);
               simulationData[ i + 1 ].replace(j, simulateComponent(grid.at(i + 1).at(j)));
            }

            //calculate rmsd between current point and point above
            rmsdSum += calculateScaledRMSD(
               simulationData.at(i).at(j), current, simulationData.at(i + 1).at(j), grid.at(i + 1).at(j));
         }

         //check left (j - 1)
         if ((i) >= 0 && (i) < grid.size() && (j - 1) >= 0 && ((j - 1) < grid.at(i).size())) {
            numComparisons++;

            //simulate if not already done
            if (simulationData.at(i).at(j - 1) == NULL) {
               delete simulationData.at(i).at(j - 1);
               simulationData[ i ].replace(j - 1, simulateComponent(grid.at(i).at(j - 1)));
            }

            //calculate rmsd between current point and point to the right
            rmsdSum += calculateScaledRMSD(
               simulationData.at(i).at(j), current, simulationData.at(i).at(j - 1), grid.at(i).at(j - 1));
         }

         //take average RMSD
         rmsdSum /= numComparisons;

         //add to results data
         QPair<US_Model::SimulationComponent, double> currentResult(current, rmsdSum);
         results.append(currentResult);
      }
   }

   //iterate through simulation data and delete everything
   for (int i = 0; i < simulationData.size(); i++) {
      for (int j = 0; j < simulationData.at(i).size(); j++) {
         delete simulationData.at(i).at(j);
      }
   }

   return results;
}

//Calculates RMSD levels for an irregular grid;
//Accomplished by simulating every solute, storing the simulations in the heap, and then finding RMSD relative to every point
//A point's RMSD is determined by taking the average RMSD of the 'numNeighbors' lowest values
//NOTE: UNDER NO CONDITIONS PASS A LARGE GRID IN HERE
//TODO: Optimize by selecting nearest 20 or so points in sk or sd grid for comparison instead of calculating RMSD for everything
//returns them as a qvector of sedimentation, diffusion, f/f0, MW, and RMSD; this is done because std::tuple is unavailable on Alamo
QVector<QPair<US_Model::SimulationComponent, double>> US_ModelBuilder::calculateIrregularGridRMSD(
   QVector<QVector<US_Model::SimulationComponent>> grid, int numNeighbors) {
   QVector<QPair<US_Model::SimulationComponent, double>> results;

   //vector to store simulations for every point
   QVector<QPair<US_Model::SimulationComponent, US_DataIO::RawData *>> simulations;

   //paralell vector to store calcualted RMSDs
   QVector<QVector<double>> differences;

   QVector<QFuture<QVector<QPair<US_Model::SimulationComponent, US_DataIO::RawData *>>>> futures;

   //queue up all points for computation
   for (int i = 0; i < grid.size(); i++) {
      futures.append(QtConcurrent::run(this, &US_ModelBuilder::simulateBatch, grid.at(i)));
   }

   //get all points back from threads; block until finished
   int numFutures = futures.size();
   for (int i = 0; i < numFutures; i++) {
      simulations.append(futures.at(0).result());
      simulations.remove(0);
      qDebug() << "Got thread result " << i << " of " << numFutures;
   }

   //fill differences vector
   QVector<double> tempDiff;
   differences.fill(tempDiff, simulations.size());

   //Profiling
   QTime timer;
   qDebug() << "starting timer...";
   timer.start();

   //loop over all points
   //Lots of qvector juggling here -- TODO: check for bugs
   for (int i = 0; i < simulations.size(); i++) {
      //qDebug() << "on step " << i << " of " << simulations.size();
      QPair<US_Model::SimulationComponent, US_DataIO::RawData *> current = simulations.at(i);
      QVector<double> currentDifferences = differences[ i ];

      //iterate over all points after the current point
      for (int j = i + 1; j < simulations.size(); j++) {
         //calculate RMSD
         double rmsd = calculateScaledRMSD(current, simulations.at(j));

         //add to the current point's differences array and other point's differences array
         currentDifferences.append(rmsd);
         differences[ j ].append(rmsd);
      }

      //sort differences list, and store
      qSort(currentDifferences);

      //average numNeighbors nearest points to the current point, and store
      double average = 0.0;
      for (int j = 0; j < numNeighbors; j++) {
         average += currentDifferences.at(j);
      }
      average /= numNeighbors;

      QPair<US_Model::SimulationComponent, double> currentPair(current.first, average);
      //qDebug() << current.first.s << " , " << current.first.f_f0 << " , " << current.second;
      results.append(currentPair);
   }

   int time = timer.elapsed();
   qDebug() << "Singlethread RMSD calc time: " << time;

   //Delete every element of the simulations cache vector
   /*for(int i = 0; i < simulations.size(); i++)
   {
      delete simulations.at(i).second;
   }*/

   return results;
}

//Simulates every point in a list of components
QVector<QPair<US_Model::SimulationComponent, US_DataIO::RawData *>> US_ModelBuilder::simulateBatch(
   QVector<US_Model::SimulationComponent> components) {
   QVector<QPair<US_Model::SimulationComponent, US_DataIO::RawData *>> simulations;

   for (int i = 0; i < components.size(); i++) {
      QPair<US_Model::SimulationComponent, US_DataIO::RawData *>
         value(components.at(i), simulateComponent(components.at(i)));
      simulations.append(value);
   }

   return simulations;
}


//Generates a Faxen grid from boundaries given
QVector<QVector<US_Model::SimulationComponent>> US_ModelBuilder::generateFaxenGrid(
   double sMin, double sMax, double kMin, double kMax, int grids) {
   QVector<QVector<US_Model::SimulationComponent>> grid;

   //scale
   sMin /= 1e-13;
   sMax /= 1e-13;

   int M = grids;
   double mu_1 = sMin * kMin;
   double mu_2 = sMax * kMax;

   //list to store precoumputed xi_j values
   QVector<double> jValues;

   //first part of grid generation: calc xi_j
   for (int j = 0; j <= M; j++) {
      double xi_j = pow((1 - (( double ) j) / M) * pow(mu_1, -0.25) + ((( double ) j) / M) * pow(mu_2, -0.25), -4.0);

      jValues.append(xi_j);
   }

   //do calculations for all i satisfying specification condition
   for (int i = 0; i <= M; i++) {
      QVector<US_Model::SimulationComponent> column;

      double xi_i = pow((1 - ((( double ) i) / M)) * pow(mu_1, -0.25) + ((( double ) i) / M) * pow(mu_2, -0.25), -4.0);

      //check that the value for xi_i is allowed
      if (xi_i <= sMin * kMax) {
         //if the condition is satisfied, we have this column's y-value
         double y_i = xi_i / sMin;

         //now we have to use all values of xi_j to find the x-values
         for (int j = 0; j < jValues.size(); j++) {
            //the value of xi
            double xi_j = jValues.at(j);

            //the coordinate
            double x_ij = xi_j / y_i;

            //add the point only if it's s-value falls in the bounds
            if (x_ij >= sMin && x_ij <= sMax) {
               US_Model::SimulationComponent current;

               current.s = x_ij * 1e-13;
               current.f_f0 = y_i;
               current.D = calculateDiffusionSK(x_ij * 1e-13, y_i);

               //use calc_coefficients to populate the rest of the simcomponent
               if (US_Model::calc_coefficients(current)) {
                  column.append(current);
               }
               else {
                  qDebug() << "calc_coefficients failed in Faxen generation. s, d, f/f0, mw: " << current.s << " , "
                           << current.D << " , " << current.f_f0 << " , " << current.mw;
               }
            }
         }
      }

      //add the column if it isn't empty
      if (!column.isEmpty()) {
         grid.append(column);
      }
   }

   return grid;
}

//creates a simulation component from an S and K value
US_Model::SimulationComponent US_ModelBuilder::componentFromSK(double s, double k) {
   US_Model::SimulationComponent newModel;

   newModel.s = s;
   newModel.f_f0 = k;
   newModel.D = calculateDiffusionSK(s, k);

   if (US_Model::calc_coefficients(newModel)) {
      return newModel;
   }
   else {
      qDebug() << "calc_coefficients failed in SK model generation. s, f/f0: " << newModel.s << " , " << newModel.f_f0;
      return newModel;
   }
}

//produces a line of solutes with constant RMSD between neighbors
QVector<US_Model::SimulationComponent> US_ModelBuilder::calculateLine(
   double tolerance, double targetRMSD, QVector2D startCoord, QVector2D endCoord) {
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

   US_Model::SimulationComponent start = componentFromSK(sStart, kStart);
   US_Model::SimulationComponent end = componentFromSK(sEnd, kEnd);

   double distanceLimit = calculateDistance(start, end, false);

   US_DataIO::RawData *terminusSim = simulateComponent(end);
   US_DataIO::RawData *currentPointSim;
   US_DataIO::RawData *testPointSim;

   US_Model::SimulationComponent testPoint;
   US_Model::SimulationComponent highPoint;
   US_Model::SimulationComponent lowPoint;

   //add start to line
   line.append(start);

   //outer loop over line
   do {
      //simulate current point
      US_Model::SimulationComponent currentComponent = line.last();
      currentPointSim = simulateComponent(currentComponent);

      //check that sufficient distance remains to find point
      if (calculateScaledRMSD(currentPointSim, currentComponent, terminusSim, end) > targetRMSD) {
         double testRMSD;

         //setup low and high points
         lowPoint = line.last();
         highPoint = end;

         int maxDepthInner = 200;
         int countInner = 0;

         //internal loop to find correct point
         do {
            countInner++;

            //calculate testPoint
            testPoint = componentFromSK((lowPoint.s + highPoint.s) / 2.0, ((lowPoint.f_f0 + highPoint.f_f0) / 2.0));

            //qDebug() << "simulating component; counter: " << countInner;

            //simulate testpoint
            testPointSim = simulateComponent(testPoint);

            //calculate rmsd
            testRMSD = calculateScaledRMSD(currentPointSim, currentComponent, testPointSim, testPoint);

            //delete simulation
            delete testPointSim;

            //check if RMSD too low
            if (testRMSD < targetRMSD) {
               lowPoint = testPoint;
            }

            //check if too high
            else if (testRMSD > targetRMSD) {
               highPoint = testPoint;
            }

            //check if recursive depth exceeded
            if (countInner > maxDepthInner) {
               qDebug() << "Loop depth exceeded in inner loop for numerical grid gen!";
               finished = true;
            }


         } while (qAbs(targetRMSD - testRMSD) > tolerance && maxDepthInner > countInner);

         //check that point is valid to add
         if (calculateDistance(start, testPoint, false) > distanceLimit) {
            finished = true;
         }
         else {
            //qDebug() << "test RMSD value: " << testRMSD<< " s: " << testPoint.s << " ff0: " << testPoint.f_f0;
            line.append(testPoint);
         }
      }
      else {
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
QVector<QVector<US_Model::SimulationComponent>> US_ModelBuilder::createNumericalGrid(
   double tolerance, double targetRMSD, double minS, double maxS, double minK, double maxK, QChar majorAxis) {
   QVector<QVector<US_Model::SimulationComponent>> grid;

   QVector<US_Model::SimulationComponent> firstLineValues;

   //get f/f0-values to put s lines on
   if (majorAxis == 's') {
      qDebug() << "Making numerical s-major grid...";
      firstLineValues = calculateLine(
         tolerance, targetRMSD, QVector2D(minS, minK), QVector2D(maxS, minK)); //for s-major, constant k line first
   }
   else if (majorAxis == 'k') {
      qDebug() << "Making numerical k-major grid...";
      firstLineValues = calculateLine(
         tolerance, targetRMSD, QVector2D(minS, minK), QVector2D(minS, maxK)); //for k-major, constant s line first
   }
   else {
      qDebug() << "no major axis specified!";
      return grid;
   }

   //storage for concurrency tracker object
   QVector<QFuture<QVector<US_Model::SimulationComponent>>> futures;

   //make and add s-lines at all k-values
   for (int i = 0; i < firstLineValues.size(); i++) {
      double currentS = firstLineValues.at(i).s;
      double currentK = firstLineValues.at(i).f_f0;

      qDebug() << "currentS, currentK: " << currentS << " , " << currentK;

      //request thread for job, and store tracker
      if (majorAxis == 's') {
         futures.append(QtConcurrent::run(
            this, &US_ModelBuilder::calculateLine, tolerance, targetRMSD, QVector2D(currentS, minK),
            QVector2D(currentS, maxK))); //for s-major
      }
      else if (majorAxis == 'k') {
         futures.append(QtConcurrent::run(
            this, &US_ModelBuilder::calculateLine, tolerance, targetRMSD, QVector2D(minS, currentK),
            QVector2D(maxS, currentK))); //for k-major
      }

      //qDebug() << "Thread added to queue";

      //generate and add line - deprecated
      //grid.append(calculateLine(tolerance, targetRMSD, QVector2D(minS, currentK) , QVector2D(maxS, currentK)));
   }

   //qDebug() << "Waiting for threads to finish...";

   int numFutures = futures.size();

   //wait for threads to finish, and add to grid in correct order
   for (int i = 0; i < numFutures; i++) {
      //add result, blocking when calculation is incomplete (so that ordering is preserved)
      grid.append(futures.first().result());
      futures.remove(0);
      qDebug() << "Stored result " << (i + 1) << " of " << numFutures;
   }

   return grid;
}


//finds the RMSDs between adjacent points in each column of the 2d qvector grid and returns them as a qvector of sedimentation, diffusion, f/f0, MW, and RMSD
QVector<QVector<double>> US_ModelBuilder::checkLineRMSDParalell(
   QVector<QVector<US_Model::SimulationComponent>> grid, bool approximateToMidpoint) {
   QVector<QVector<double>> values;
   QVector<QFuture<QVector<double>>> futures;

   //qDebug() << "Max thread count: " << QThreadPool::globalInstance()->maxThreadCount();

   //first loop to spawn threads
   for (int i = 0; i < grid.size(); i++) {
      QVector<US_Model::SimulationComponent> column = grid.at(i);

      //queue up the RMSD calculation
      futures.append(QtConcurrent::run(this, &US_ModelBuilder::findListRMSD, column));
   }

   //second loop to re-integrate threads and do calculations
   for (int i = 0; i < grid.size(); i++) {
      //once again, get our current col
      QVector<US_Model::SimulationComponent> column = grid.at(i);

      //fetch qFutures in the order that they were added, derefrerencing as we go to save memory
      QVector<double> vals = futures.at(0).result();
      futures.remove(0);

      //qDebug() << "Stored rowRMSD " << (i+1) << " of " << grid.size();

      for (int lcv = 1; lcv < column.size(); lcv++) {
         QVector<double> data;
         US_Model::SimulationComponent component1 = column.at(lcv - 1);
         US_Model::SimulationComponent component2 = column.at(lcv);
         double s;
         double f_f0;
         double D;
         double mw;

         //place points at average if apprx to mdpoint is enabled
         if (approximateToMidpoint) {
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
QVector<QVector<US_Model::SimulationComponent>> US_ModelBuilder::generateSKGrid(
   double sMin, double sMax, double kMin, double kMax, int sDim, int kDim, QChar constantAxis) {
   QVector<QVector<US_Model::SimulationComponent>> grid;

   //get rate of change in both dims
   double deltaS = (sMax - sMin) / sDim;
   double deltaK = (kMax - kMin) / kDim;
   double s = 0.0;
   double k = 0.0;

   //make grid for sMajor
   if (constantAxis == 's' || constantAxis == 'S') {
      for (int i = 0; i < sDim; i++) {
         QVector<US_Model::SimulationComponent> column;
         s = (deltaS * i) + sMin;

         for (int j = 0; j < kDim; j++) {
            k = (deltaK * j) + kMin;
            column.append(componentFromSK(s, k));
         }

         grid.append(column);
      }
   }

   //grid for kMajor
   else if (constantAxis == 'k' || constantAxis == 'K') {
      for (int i = 0; i < kDim; i++) {
         QVector<US_Model::SimulationComponent> column;
         k = deltaK * i + kMin;

         for (int j = 0; j < sDim; j++) {
            s = deltaS * j + sMin;
            column.append(componentFromSK(s, k));
         }

         grid.append(column);
      }
   }

   //otherwise, error
   else {
      throw std::invalid_argument("ConstatnAxis passed to grid generator was invalid");
   }

   //end
   return grid;
}

//Generates a grid on the sd plane with the given params;
QVector<QVector<US_Model::SimulationComponent>> US_ModelBuilder::generateSDGrid(
   double sMin, double sMax, double dMin, double dMax, int sDim, int dDim, QChar constantAxis) {
   QVector<QVector<US_Model::SimulationComponent>> grid;

   //get rate of change in both dims
   double deltaS = (sMax - sMin) / sDim;
   double deltaD = (dMax - dMin) / dDim;
   double s = 0.0;
   double D = 0.0;

   //make grid for sMajor
   if (constantAxis == 's' || constantAxis == 'S') {
      for (int i = 0; i < sDim; i++) {
         QVector<US_Model::SimulationComponent> column;
         s = (deltaS * i) + sMin;

         for (int j = 0; j < dDim; j++) {
            D = (deltaD * j) + dMin;

            US_Model::SimulationComponent current;

            double k = calculateFrictionalRatioSD(s, D);

            current.s = s;
            current.D = D;
            current.f_f0 = k;

            //do coefficient calculation, and proceed only if successful
            if (US_Model::calc_coefficients(current) && current.f_f0 >= 1 && current.s <= 10e-13 && current.f_f0 <= 4) {
               //qDebug() << "s: " << current.s << " , D: " << current.D << " , f_f0: " << current.f_f0 << endl;
               column.append(current);
            }
         }
         grid.append(column);
      }
   }

   //grid for kMajor
   else if (constantAxis == 'd' || constantAxis == 'D') {
      for (int i = 0; i < dDim; i++) {
         QVector<US_Model::SimulationComponent> column;
         D = deltaD * i + dMin;

         for (int j = 0; j < sDim; j++) {
            s = deltaS * j + sMin;

            US_Model::SimulationComponent current;

            double k = calculateFrictionalRatioSD(s, D);

            current.s = s;
            current.D = D;
            current.f_f0 = k;

            //do coefficient calculation, and proceed only if successful
            if (US_Model::calc_coefficients(current) && current.f_f0 >= 1 && current.s < 10e-13) {
               //qDebug() << "s: " << current.s << " , D: " << current.D << " , f_f0: " << current.f_f0 << endl;
               column.append(current);
            }
         }

         grid.append(column);
      }
   }

   //otherwise, error
   else {
      throw std::invalid_argument("ConstatnAxis passed to grid generator was invalid");
   }

   //end
   return grid;
}

//Generates a grid on the mk plane with the specifications passed
QVector<QVector<US_Model::SimulationComponent>> US_ModelBuilder::generateMKGrid(
   double mMin, double mMax, double kMin, double kMax, int mDim, int kDim, QChar constantAxis) {
   QVector<QVector<US_Model::SimulationComponent>> grid;

   //get rate of change in both dims
   double deltaM = (mMax - mMin) / mDim;
   double deltaK = (kMax - kMin) / kDim;
   double m = 0.0;
   double k = 0.0;

   //make grid for sMajor
   if (constantAxis == 'm' || constantAxis == 'M') {
      for (int i = 0; i < mDim; i++) {
         QVector<US_Model::SimulationComponent> column;
         m = (deltaM * i) + mMin;

         for (int j = 0; j < kDim; j++) {
            k = (deltaK * j) + kMin;

            US_Model::SimulationComponent current;

            double D = calculateDiffusionMK(m, k);
            double s = calculateSedimentationMD(m, D);

            current.mw = m;
            current.s = s;
            current.D = D;
            current.f_f0 = k;

            //do coefficient calculation, and proceed only if successful
            if (US_Model::calc_coefficients(current) && current.f_f0 <= 4.0 && current.f_f0 >= 1 && current.s < 10e-13) {
               //qDebug() << "s: " << current.s << " , D: " << current.D << " , f_f0: " << current.f_f0 << endl;
               column.append(current);
            }
         }
         grid.append(column);
      }
   }

   //grid for kMajor
   else if (constantAxis == 'k' || constantAxis == 'K') {
      for (int i = 0; i < kDim; i++) {
         QVector<US_Model::SimulationComponent> column;
         k = (deltaK * i) + kMin;

         for (int j = 0; j < mDim; j++) {
            m = (deltaM * j) + mMin;

            US_Model::SimulationComponent current;

            double D = calculateDiffusionMK(m, k);
            double s = calculateSedimentationMD(m, D);

            current.mw = m;
            current.s = s;
            current.D = D;
            current.f_f0 = k;

            //do coefficient calculation, and proceed only if successful
            if (US_Model::calc_coefficients(current) && current.f_f0 <= 4.0 && current.f_f0 >= 1 && current.s < 10e-13) {
               //qDebug() << "s: " << current.s << " , D: " << current.D << " , f_f0: " << current.f_f0 << endl;
               column.append(current);
            }
         }

         grid.append(column);
      }
   }

   //otherwise, error
   else {
      throw std::invalid_argument("ConstatnAxis passed to grid generator was invalid");
   }

   //end
   return grid;
}

//calculates distribution statistics on a specified grid, calculating along inner QVectors
//Returns data in a size 3 qvector in the form: pointcount, average RMSD, standard deviation
QVector<double> US_ModelBuilder::calculateGridStatistics(QVector<QVector<double>> processedGrid) {
   int numPoints = 0.0;
   double average = 0.0;
   double stddev = 0.0;
   QVector<double> data;

   //calculate average
   for (int i = 0; i < processedGrid.size(); i++) {
      average += processedGrid.at(i).at(4);
      numPoints++;
   }
   average = average / (( double ) numPoints);

   //calculate standard deviation
   for (int i = 0; i < processedGrid.size(); i++) {
      stddev += sq(processedGrid.at(i).at(4) - average);
   }
   stddev = sqrt((1 / (numPoints - 1.0)) * stddev);

   data.append(numPoints);
   data.append(average);
   data.append(stddev);

   return data;
}

QPair<double, double> US_ModelBuilder::convertPolar(double degree) {
   double rad = degree * (M_PI / 180);
   double x = cos(rad);
   double y = sin(rad);

   QPair<double, double> vector(x, y);

   return vector;
}

//finds a point along a given direction vector from a source point in degrees with the specified distance
QPair<double, double> US_ModelBuilder::projectPolar(double originX, double originY, double distance, double degree) {
   double rad = degree * (M_PI / 180);
   QPair<double, double> newPoint(originX + distance * cos(rad), originY + distance * sin(rad));

   return newPoint;
}

//generates a simulation component from an SK pair
//can scale s-values down by 1e-13 if needed
US_Model::SimulationComponent US_ModelBuilder::componentFromSKPair(QPair<double, double> sk, bool scaleS) {
   if (scaleS) {
      return componentFromSK(sk.first * 1e-13, sk.second);
   }
   else {
      return componentFromSK(sk.first, sk.second);
   }
}

//Finds a solute point differing from the origin by a specified target RMSD in the polar direction given
//NOTE: tolerance, in this context does not directly refer to RMSD deviation from the target; use 0.01 as value for most use cases
US_Model::SimulationComponent US_ModelBuilder::findConstantRMSDPointInDirection(
   US_Model::SimulationComponent origin, US_DataIO::RawData *originSim, double target, double tolerance, double degree) {
   //constant tau value
   double tau = 2 / (1 + sqrt(5));

   //initial step setup
   QPair<double, double> direction = convertPolar(degree);
   double deltaS = ((1e-12 - 1e-13) / 64.0) * 1e13;
   double deltaK = ((4 - 1) / 64.0);
   //double delta = deltaS * direction.first + deltaK * direction.second; //scalar projection of default grid step
   //delta /= 25;

   //TODO: properly fix if we actually need this
   double delta = 2 * sqrt(sq(deltaS * direction.first) + sq(deltaK * direction.second));

   //NOTE: This assumes that delta is a valid, legal stepsize
   //this may not be true for very low RMSDs, and probably should be set dynamically

   //dynamic stepsize setting
   US_Model::SimulationComponent stepsizeTest
      = componentFromSKPair(projectPolar(origin.s * 1e13, origin.f_f0, delta, degree), true);
   US_DataIO::RawData *stepsizeTestSim = simulateComponent(stepsizeTest);
   double stepsizeTestDifference = qAbs(calculateScaledRMSD(originSim, origin, stepsizeTestSim, stepsizeTest) - target);
   delete stepsizeTestSim;

   //loop until difference less than initial value
   //qDebug() << "delta" << delta;
   //qDebug() << "difference: " << stepsizeTestDifference;
   while (stepsizeTestDifference > target) {
      //halve delta
      delta /= 2;
      //qDebug() << "new delta: " << delta;

      //redo calcs
      stepsizeTest = componentFromSKPair(projectPolar(origin.s * 1e13, origin.f_f0, delta, degree), true);
      stepsizeTestSim = simulateComponent(stepsizeTest);
      stepsizeTestDifference = qAbs(calculateScaledRMSD(originSim, origin, stepsizeTestSim, stepsizeTest) - target);
      delete stepsizeTestSim;
   }

   //iterate until we find our bracket
   US_Model::SimulationComponent lambda = componentFromSK(origin.s, origin.f_f0);
   US_DataIO::RawData *lambdaSim;
   QVector<US_Model::SimulationComponent> bracket;
   double lambdaDifference = target;
   double lastDifference = target;
   int count = 0;
   while (lambdaDifference <= lastDifference
          || bracket.size() < 3) //we assume this always runs; can guarantee if stepsize is dynamically set
   {
      count++;

      //reassign point
      bracket.append(lambda);
      lambda = componentFromSKPair(projectPolar(lambda.s * 1e13, lambda.f_f0, delta, degree), true);

      //check size on storage array
      if (bracket.size() > 3) {
         bracket.remove(0);
      }

      //recalculate
      lambdaSim = simulateComponent(lambda);
      lastDifference = lambdaDifference;
      lambdaDifference = qAbs(calculateScaledRMSD(originSim, origin, lambdaSim, lambda) - target);
      delete lambdaSim;

      //qDebug() << "bracket testpoint: " << lambda.s << " , " << lambda.f_f0 << " , " << lambdaDifference;
   }

   //qDebug() << "bracketing complete";

   //build bracket midpoint candidates
   US_Model::SimulationComponent bracketMidpoint = componentFromSKPair(
      projectPolar(
         origin.s * 1e13, origin.f_f0,
         (calculateDistance(origin, bracket.at(1), true) + calculateDistance(origin, bracket.at(2), true)) / 2.0,
         degree),
      true);
   US_DataIO::RawData *bracketMidpointSim = simulateComponent(bracketMidpoint);
   US_DataIO::RawData *bracketMedianSim = simulateComponent(bracket.at(1));

   //compare to find the lower function value
   if (
      qAbs(calculateScaledRMSD(originSim, origin, bracketMidpointSim, bracketMidpoint) - target)
      >= qAbs(calculateScaledRMSD(originSim, origin, bracketMedianSim, bracket.at(1)) - target)) {
      // median is smallest
      //now find which of k-1 or k+1 is closer
      if (calculateDistance(bracket.at(1), bracket.at(2), true) >= calculateDistance(bracket.at(1), bracket.at(0), true)) {
         //we want to discard k+1, keep k-1
         bracket.replace(2, bracketMidpoint);
      }
      else {
         //we want to discard k-1, keep k+1
         bracket.remove(0);
         bracket.insert(1, bracketMidpoint);
      }
   }

   else {
      //midpoint is smallest
      //now find which of k-1 or k+1 is closer
      if (calculateDistance(bracketMidpoint, bracket.at(2), true) >= calculateDistance(bracketMidpoint, bracket.at(0), true)) {
         //we want to discard k+1, keep k-1
         bracket.replace(2, bracketMidpoint);
      }
      else {
         //we want to discard k-1, keep k+1
         bracket.remove(0);
         bracket.insert(1, bracketMidpoint);
      }
   }

   delete bracketMidpointSim;
   delete bracketMedianSim;


   //qDebug() << "Starting golden section search";

   //calculate boundary differences for golden section search
   double alphaDistance = calculateDistance(origin, bracket.at(0), true);
   double betaDistance = calculateDistance(origin, bracket.at(2), true);

   US_Model::SimulationComponent x1 = componentFromSKPair(
      projectPolar(origin.s * 1e13, origin.f_f0, alphaDistance + (1 - tau) * (betaDistance - alphaDistance), degree),
      true);
   US_Model::SimulationComponent x2 = componentFromSKPair(
      projectPolar(origin.s * 1e13, origin.f_f0, alphaDistance + tau * (betaDistance - alphaDistance), degree), true);

   US_DataIO::RawData *stepSim1 = simulateComponent(x1);
   US_DataIO::RawData *stepSim2 = simulateComponent(x2);

   double stepDifference1 = qAbs(calculateScaledRMSD(originSim, origin, stepSim1, x1) - target);
   double stepDifference2 = qAbs(calculateScaledRMSD(originSim, origin, stepSim2, x2) - target);

   delete stepSim1;
   delete stepSim2;

   //do golden section search until bracket distance is small
   while (qAbs(betaDistance - alphaDistance) > tolerance) {
      //setup bracket for next iteration
      if (stepDifference1 < stepDifference2) {
         //f(step1) < f(step2), so adjust bracket to match
         bracket.replace(2, x2);
         x2 = x1;
         stepDifference2 = stepDifference1;
         betaDistance = calculateDistance(origin, bracket.at(2), true);
         x1 = componentFromSKPair(
            projectPolar(origin.s * 1e13, origin.f_f0, alphaDistance + (1 - tau) * (betaDistance - alphaDistance), degree),
            true);
         stepSim1 = simulateComponent(x1);
         stepDifference1 = qAbs(calculateScaledRMSD(originSim, origin, stepSim1, x1) - target);
         delete stepSim1;

         //qDebug() << x1.s << "," << x1.f_f0 << "," << stepDifference1;
      }
      else {
         //f(step1) > f(step2)
         bracket.replace(0, x1);
         x1 = x2;
         stepDifference1 = stepDifference2;
         alphaDistance = calculateDistance(origin, bracket.at(0), true);
         x2 = componentFromSKPair(
            projectPolar(origin.s * 1e13, origin.f_f0, alphaDistance + tau * (betaDistance - alphaDistance), degree),
            true);
         stepSim2 = simulateComponent(x2);
         stepDifference2 = qAbs(calculateScaledRMSD(originSim, origin, stepSim2, x2) - target);
         delete stepSim2;

         //qDebug() << x2.s << "," << x2.f_f0 << "," << stepDifference2;
      }
   }

   US_Model::SimulationComponent midpoint = componentFromSKPair(
      projectPolar(
         origin.s * 1e13, origin.f_f0,
         (calculateDistance(origin, bracket.at(0), true) + calculateDistance(origin, bracket.at(2), true)) / 2.0,
         degree),
      true);
   //US_DataIO::RawData* mdptSim = simulateComponent(midpoint);
   //double mdptDifference = qAbs(calculateScaledRMSD(originSim, origin, mdptSim, midpoint) - target);
   //delete mdptSim;
   //qDebug() << "result midpoint:";
   //qDebug() << midpoint.s << "," << midpoint.f_f0 << "," << mdptDifference;

   //qDebug() << "ident point: " << midpoint.s << "," << midpoint.f_f0;
   return midpoint;
}

QVector<US_Model::SimulationComponent> US_ModelBuilder::findConstantRMSDNeighbors(
   double s, double k, double degreeIncrement, double target) {
   QVector<US_Model::SimulationComponent> neighbors;

   //setup origin
   US_Model::SimulationComponent origin = componentFromSK(s, k);
   US_DataIO::RawData *originSim = simulateComponent(origin);

   //loop over all directions
   for (double deg = 0.0; deg < 360; deg += degreeIncrement) {
      US_Model::SimulationComponent result;

      try {
         //result = (findConstantRMSDPointInDirection(origin, originSim, target, 0.000001, deg));
         result = (findConstantRMSDPointInDirection(origin, originSim, target, 0.1, deg));
      } catch (std::invalid_argument) //catch invalid argument exception
      {
         //the point here tried to test a value that was illegal, cancelling the operation
      }

      neighbors.append(result);
   }

   delete originSim;
   return neighbors;
}

//Gets a list of sample points (in terms of distance from the origin point along the given line) and RMSDs relative to the specified origin in a given direction
QVector<QPair<double, double>> US_ModelBuilder::getSamplePoints(
   US_Model::SimulationComponent origin, US_DataIO::RawData *originSim, int numSamples, double degree) {
   //setup data structure
   QVector<QPair<double, double>> samples;
   samples.append(QPair<double, double>(0.0, 0.0));

   double deltaS = ((10 - 1) / 1536.0);
   double deltaK = ((4 - 1) / 64.0);
   double rad = degree * (M_PI / 180);
   double delta = (deltaS * deltaK) / sqrt(sq(deltaK) * sq(cos(rad)) + sq(deltaS) * sq(sin(rad)));

   //do backwards samples
   bool inBounds = true;
   for (int i = 1; i <= numSamples && inBounds; i++) {
      //get point projected in direction
      US_Model::SimulationComponent current
         = componentFromSKPair(projectPolar(origin.s * 1e13, origin.f_f0, delta * (i + 1), degree), true);

      //simulate to get rmsd
      try {
         US_DataIO::RawData *currentSim = simulateComponent(current);
         double currentRMSD = calculateScaledRMSD(originSim, origin, currentSim, current);
         delete currentSim;

         //qDebug() << current.s << "\t" << current.f_f0 << "\t" << currentRMSD;

         //make pair
         QPair<double, double> currentPair(calculateDistance(origin, current, true), currentRMSD);
         samples.append(currentPair);
      } catch (std::invalid_argument) //catch invalid argument exception
      {
         //we're no longer in bounds
         inBounds = false;
      }
   }

   return samples;
}

//gets RMSD level sets around the specified center point for the RMSD levels given
QPair<QVector<QVector<US_Model::SimulationComponent>>, QVector<QPair<double, double>>> US_ModelBuilder::getLevelSets(
   US_Model::SimulationComponent center, QVector<double> targets, double degreeIncrement) {
   //level set structure
   QVector<QVector<US_Model::SimulationComponent>> levelSets;

   //simulate center
   US_DataIO::RawData *centerSim = simulateComponent(center);

   //qvector to store fitted lines; 1 index for every degree increment
   //QVector<QPair<double, double> > lines;
   QVector<QPair<double, double>> slopes;

   //store last distance for each degree level
   //QVector<double> lastDistances;

   //directional loop
   for (double degree = 0; degree <= 360; degree += degreeIncrement) {
      //try to get samples
      try {
         //get samples in direction of linefit results
         QVector<QPair<double, double>> samples
            = getSamplePoints(center, centerSim, 3, degree); //change the number of points to make this go faster

         //store last distance in samples
         //lastDistances.append(samples.last().first);

         //linefitlevelSets
         QVector<double> regressionLine = linefitThroughOrigin(samples);

         //         qDebug() << "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n";
         //         qDebug() << "slope, intercept: " << slope << "," << intercept;
         //         qDebug() << "sigma, correlation: " << sigma << "," << correlation;
         //         qDebug() << "s, k, degree: " << center.s << "," << center.f_f0 << "," << degree;

         //check that correlation is high
         if (regressionLine.at(2) < 0.9) {
            qWarning() << "R^2 less than 0.9 at:";
            qWarning() << "\tR^2, correlation: " << regressionLine.at(2) << "," << regressionLine.at(3);
            qWarning() << "\ts, k, degree: " << center.s << "," << center.f_f0 << "," << degree;
         }

         //store line
         slopes.append(QPair<double, double>(regressionLine.first(), degree));
      } catch (std::invalid_argument) //catch invalid argument exception
      {
         qDebug() << "dropped point in sample search";
      }
   }

   //iterate over targets
   for (int i = 0; i < targets.size(); i++) {
      //make levelsets
      QVector<US_Model::SimulationComponent> levelSet;

      //compute target values
      for (double degree = 0; degree <= 360; degree += degreeIncrement) {
         //get the line for this degree level
         //QPair<double, double> currentLine = lines.at((int) (degree / degreeIncrement));
         double currentSlope = slopes.at(( int ) (degree / degreeIncrement)).first;

         //get current last distance
         //double currentLastSampleDistance = lastDistances.at((int) (degree / degreeIncrement));

         //calculate the distance to move
         double calculatedDistance = (targets.at(i)) / currentSlope;

         //project the point at the desired rmsd level
         US_Model::SimulationComponent projected
            = componentFromSKPair(projectPolar(center.s * 1e13, center.f_f0, calculatedDistance, degree), true);

         //check that projected point is legal
         if (projected.s >= 1e-13 && projected.f_f0 >= 1) {
            levelSet.append(projected);
         }

         /*
         //check that projection falls within sample space
         if(currentLastSampleDistance < calculatedDistance)
         {
            qWarning() << "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=";
            qWarning() << "Point out of range:";
            qWarning() << "degree, distance, sampledistance: " << degree << "," << calculatedDistance << "," << currentLastSampleDistance;
            qWarning() << "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=";
         }
         */
      }

      levelSets.append(levelSet);
   }

   delete centerSim;
   return QPair<QVector<QVector<US_Model::SimulationComponent>>, QVector<QPair<double, double>>>(levelSets, slopes);
}

//Performs an OLS regression through the origin (without intercept), returning a qvector containing in this order, the slope, coeff of det, and pearson corr. coeff
QVector<double> US_ModelBuilder::linefitThroughOrigin(QVector<QPair<double, double>> points) {
   double sumXY = 0.0;
   double sumXSquared = 0.0;
   double sumYSquared = 0.0;
   double sumX = 0.0;
   double sumY = 0.0;
   double meanY = 0.0;
   double regressionSumSquares = 0.0;
   double totalSumSquares = 0.0;

   QVector<double> regression(3);
   double slope = 0.0;
   double pearsonCorrelation = 0.0;

   //setup for regression
   for (int i = 0; i < points.size(); i++) {
      QPair<double, double> current = points.at(i);

      sumXY += current.first * current.second;
      sumXSquared += sq(current.first);
      sumYSquared += sq(current.second);
      sumX += current.first;
      sumY += current.second;
   }

   //calculate mean of y
   meanY = sumY / points.size();

   //calculate slope
   slope = (sumXY / sumXSquared);

   //calculate pearson corr. coeff.
   pearsonCorrelation = ((points.size() * sumXY) - (sumX * sumY))
                        / sqrt((points.size() * sumXSquared - sq(sumX)) * (points.size() * sumYSquared - sq(sumY)));

   //calculate R^2 (coeff. of determination)
   for (int i = 0; i < points.size(); i++) {
      QPair<double, double> current = points.at(i);

      regressionSumSquares += sq((current.first * slope) - meanY);
      totalSumSquares += sq(current.second - meanY);
   }

   regression[ 0 ] = slope; //store slope
   regression[ 1 ] = (regressionSumSquares / totalSumSquares);
   regression[ 2 ] = pearsonCorrelation;

   return regression;
}
