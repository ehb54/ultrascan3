#ifndef US_MODELBUILDER_H
#define US_MODELBUILDER_H

//#include <QtGui>
#include <QApplication>
#include <QDomDocument>
#include <QVector>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QPair>
#include <QTime>
#if QT_VERSION > 0x050000
#include <QtConcurrent/QtConcurrent>
#else
#include <qtconcurrentrun.h>
#endif
#include <QThread>
#include <qwt_legend.h>
#include <cmath>
#include <stdexcept>
#include <unistd.h> //for getting hostname
#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_plot.h"
#include "us_astfem_rsa.h"
#include "us_model.h"
#include "us_editor.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_investigator.h"
#include "us_run_details2.h"
#include "us_util.h"
#include "us_load_auc.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_constants.h"
#include "us_dataIO.h"
#include "us_plot3d_xyz.h"
#include "../../programs/us_astfem_sim/us_simulationparameters.h"
#include "../../programs/us_astfem_sim/us_astfem_sim.h"

class US_ModelBuilder : public US_Widgets
{
   Q_OBJECT

public:
   US_ModelBuilder();
   //~US_ModelBuilder();

private:

//   US_Help showHelp;
//   QLineEdit* le_RMSD;
//   QLineEdit* le_sedimentationVal;
//   QLineEdit* le_diffusionVal;
//   QPushButton* startButton;
//   QPushButton* parameterButton;
//   QwtPlot* data_plot;
   US_SimulationParameters working_simparams;
   US_SimulationParameters simparams;

private slots:
   void collectParameters(void);
   void startSimulation(void);
   void update_parameters(void);
   void initalize_simulationParameters
   (void);

//   void help(void)
//   {
//      showHelp.show_help("manual/us_buoyancy.html");
//   }

   US_DataIO::RawData* init_simData(US_Model* system);

   double interpolatePoint(QVector3D target, QVector3D p11, QVector3D p12, QVector3D p21, QVector3D p22);

   //Declarations from rewrite below:

   double calculateDistance( US_Model::SimulationComponent first , US_Model::SimulationComponent second, bool scaleS);
   US_DataIO::RawData* simulateComponent	( US_Model::SimulationComponent component );
   QVector<QPair<US_Model::SimulationComponent, US_DataIO::RawData*> > simulateBatch(QVector<US_Model::SimulationComponent> components);

   //Calculation functions, from other solute properties
   double calculateFrictionalRatioSD( double s , double D );
   double calculateDiffusionSK( double s , double k);
   double calculateDiffusionMK( double M, double k);
   double calculateSedimentationMD( double , double D);
   US_Model::SimulationComponent componentFromSK(double s, double k);

   //RMSD calculation functions and associated utilities
   double calculateScaledRMSD(US_DataIO::RawData* simulation1, US_Model::SimulationComponent component1 , US_DataIO::RawData* simulation2 , US_Model::SimulationComponent component2);
   double calculateScaledRMSD(QPair<US_Model::SimulationComponent, US_DataIO::RawData*> pair1, QPair<US_Model::SimulationComponent, US_DataIO::RawData*> pair2);
   QVector<double> findListRMSD(QVector<US_Model::SimulationComponent> components);
   QVector<QVector<double> > checkLineRMSDParalell(QVector<QVector<US_Model::SimulationComponent> > grid, bool approximateToMidpoint);
   QVector<double> calculateGridStatistics(QVector<QVector<double> > processedGrid);
   QVector<QPair<US_Model::SimulationComponent, double> > testRegularGrid(QVector<QVector<US_Model::SimulationComponent> > grid);
   QVector<QVector<US_Model::SimulationComponent> > switchRegularGridOrientation(QVector<QVector<US_Model::SimulationComponent> > regular);

   QVector<QPair<US_Model::SimulationComponent, double> > calculateIrregularGridRMSD(QVector<QVector<US_Model::SimulationComponent> > grid, int numNeighbors);

   //Grid generation functions for particular grid-types and planes
   QVector<QVector<US_Model::SimulationComponent> > createFaxenGrid(double minS, double maxS, double minK, double maxK, int grids);
   QVector<QVector<US_Model::SimulationComponent> > generateSKGrid(double sMin, double sMax, double kMin, double kMax, int sDim, int kDim, QChar constantAxis);
   QVector<QVector<US_Model::SimulationComponent> > generateSDGrid(double sMin, double sMax, double dMin, double dMax, int sDim, int dDim, QChar constantAxis);
   QVector<QVector<US_Model::SimulationComponent> > generateMKGrid(double mMin, double mMax, double kMin, double kMax, int mDim, int kDim, QChar constantAxis);

   //Same for numerical grids
   QVector<US_Model::SimulationComponent> calculateLine(double tolerance, double targetRMSD, QVector2D startCoord, QVector2D endCoord);
   QVector<QVector<US_Model::SimulationComponent> > createNumericalGrid(double tolerance, double targetRMSD, double minS, double maxS, double minK, double maxK, QChar majorAxis);

   //trig component
   QPair<double, double> convertPolar(double degree);
   US_Model::SimulationComponent findConstantRMSDPointInDirection(US_DataIO::RawData* originSim, US_Model::SimulationComponent origin, double target, double tolerance, double degree);
   QVector<US_Model::SimulationComponent> findConstantRMSDNeighbors(double s, double k, double degreeIncrement, double target);
   QPair<double, double> projectPolar(double originX, double originY, double distance, double degree);
   US_Model::SimulationComponent componentFromSKPair(QPair<double, double> sk, bool scaleS);

};

#endif

