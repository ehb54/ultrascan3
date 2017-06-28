#ifndef US_MODELBUILDER_H
#define US_MODELBUILDER_H

#include <QtGui>
#include <QApplication>
#include <QDomDocument>
#include <QHostInfo>
#include <QVector>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#if QT_VERSION > 0x050000
#include <QtConcurrent/QtConcurrent>
#else
#include <qtconcurrentrun.h>
#endif
#include <QThread>
#include <qwt_legend.h>
#include <cmath>
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

   US_Help showHelp;
   QLineEdit* le_RMSD;
   QLineEdit* le_sedimentationVal;
   QLineEdit* le_diffusionVal;
   QPushButton* startButton;
   QPushButton* parameterButton;
   QwtPlot* data_plot;
   US_SimulationParameters working_simparams;
   US_SimulationParameters simparams;

private slots:
   void collectParameters(void);
   void startSimulation(void);
   void update_parameters(void);
   void initalize_simulationParameters
   (void);

   void help(void)
   {
      showHelp.show_help("manual/us_buoyancy.html");
   };

   US_DataIO::RawData* init_simData(US_Model* system);

   double interpolatePoint(QVector3D target, QVector3D p11, QVector3D p12, QVector3D p21, QVector3D p22);

   //Declarations from rewrite below:

   double calculateDistance( US_Model::SimulationComponent first , US_Model::SimulationComponent second );
   US_DataIO::RawData* simulateComponent	( US_Model::SimulationComponent component );
   QVector<double> findListRMSD(QVector<US_Model::SimulationComponent> components);
   double calculateFrictionalRatioSD( double s , double D );
   double calculateDiffusionSK( double s , double k);
   double calculateDiffusionMK( double M, double k);
   double calculateSedimentationMD( double , double D);
   double calculateScaledRMSD(US_DataIO::RawData* simulation1, US_Model::SimulationComponent component1 , US_DataIO::RawData* simulation2 , US_Model::SimulationComponent component2);
   QVector<QVector<US_Model::SimulationComponent> > switchRegularGridOrientation(QVector<QVector<US_Model::SimulationComponent> > regular);
   QVector<QPair<US_Model::SimulationComponent, double> > testRegularGrid(QVector<QVector<US_Model::SimulationComponent> > grid);
   QVector<QVector<US_Model::SimulationComponent> > createFaxenGrid(double minS, double maxS, double minK, double maxK, int grids);
   US_Model::SimulationComponent modelComponentFromSK(double s, double k);
   QVector<US_Model::SimulationComponent> calculateLine(double tolerance, double targetRMSD, QVector2D startCoord, QVector2D endCoord);
   QVector<QVector<US_Model::SimulationComponent> > createNumericalGrid(double tolerance, double targetRMSD, double minS, double maxS, double minK, double maxK, QChar majorAxis);
   QVector<QVector<double> > checkLineRMSDParalell(QVector<QVector<US_Model::SimulationComponent> > grid, bool approximateToMidpoint);
   QVector<QVector<US_Model::SimulationComponent> > generateSKGrid(double sMin, double sMax, double kMin, double kMax, int sDim, int kDim, QChar constantAxis);
   QVector<QVector<US_Model::SimulationComponent> > generateSDGrid(double sMin, double sMax, double dMin, double dMax, int sDim, int dDim, QChar constantAxis);
   QVector<QVector<US_Model::SimulationComponent> > generateMKGrid(double mMin, double mMax, double kMin, double kMax, int mDim, int kDim, QChar constantAxis);
   QVector<double> calculateGridStatistics(QVector<QVector<double> > processedGrid);
};

#endif

