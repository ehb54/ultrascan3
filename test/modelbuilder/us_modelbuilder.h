#ifndef US_MODELBUILDER_H
#define US_MODELBUILDER_H

#include <QtGui>
#include <QApplication>
#include <QDomDocument>
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
#include <cmath>
#include "us_util.h"
#include "us_load_auc.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_constants.h"
#include "us_dataIO.h"
#include "us_plot3d_xyz.h"
#include "../../programs/us_astfem_sim/us_simulationparameters.h"
#include "../../programs/us_astfem_sim/us_astfem_sim.h"
#include <QVector>
#include <QFile>
#include <QFileInfo>
#include <QDirIterator>
#include <QTextStream>
#if QT_VERSION > 0x050000
#include <QtConcurrent/QtConcurrent>
#else
#include <qtconcurrentrun.h>
#endif
#include <QThread>

#include <qwt_legend.h>
#include "RegularGrid.h"
#include "points2.h"

class US_ModelBuilder : public US_Widgets {
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

    void help(void) {
        showHelp.show_help("manual/us_buoyancy.html");
    };

    US_Model* get_model(double s_value, double d_value, double k_value);

    US_DataIO::RawData* init_simData(US_Model* system);

    US_DataIO::RawData* perform_calculation(QVector2D* p);

    double calculate_RMSD(US_DataIO::RawData* simdata_1, US_DataIO::RawData* simdata_2);

    double calculate_diffusion(double s_val, double k_val);

    QVector2D* getNextPoint(QVector2D start, QVector2D end, double target, double tolerance, int maxIterations);

    QVector<QVector2D*>* calculate_line
    (QVector<double>* params, QVector2D* initial, QVector2D* end);

    /*QVector<QVector<QVector2D*> >* identify_boundaryPoints
    (QVector2D* initial_QVector2D);*/

    QVector<QVector<QVector2D*> *>* readAnalyticalPoints
    (QString filename);

    QVector<QVector3D>* getBatchRMSD
    (QVector<QVector2D*>* QVector2Ds);

    QVector<QVector<QVector3D> *>* testSpeeds
    (QVector<QVector<QVector2D*> *>* QVector2Ds, QVector<int>* speeds);

    QVector<QVector<QVector3D>* >* testGrid
    (QVector<QVector<QVector2D*> *>* QVector2Ds);

    QVector<QVector<QVector2D*> *>* generateRegularGrid
    (double sStart, double sEnd, double kStart, double kEnd, int QVector2DQuantity);

    double calculateDistance(QVector2D* point1, QVector2D* point2);

    QVector2D* calculateMidpoint
    (QVector2D* point1, QVector2D* point2);

    QVector<QVector<QVector2D*>* >* gridFromSide
    (double targetRMSD, double toleranceRMSD);

    QVector<QVector2D*>* approximate_frictional_line
    (QVector<double>* params);

    QVector<QVector<QVector2D *>* >* generateFaxenGrid
    (double sRangeStart, double sRangeEnd, double ff0RangeStart, double ff0RangeEnd, int numPoints);

    RegularGrid* testRegularGrid(QVector<QVector<QVector2D*> *>* regularGrid);

    //QVector3D interpolateRepulsion(QVector3D point, QVector<QVector3D*> repulsors);

    double interpolatePoint(QVector3D target, QVector3D p11, QVector3D p12, QVector3D p21, QVector3D p22);
    
    QVector<QVector3D*>* testIrregularGrid(QVector<QVector2D*>* input);
    
    QVector<QVector2D*>* findNearestNeighbors(int index, QVector<QVector2D*>* all, int numToFind);
    
    QVector<QVector2D*>* readIrregularGrid(QString filename);
};

#endif

