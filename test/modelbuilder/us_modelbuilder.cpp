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

int main(int argc, char* argv[]) {
    QApplication application(argc, argv);
    
    #include "main1.inc"
    
    // License is OK.  Start up.

    US_ModelBuilder w;
    w.show(); //!< \memberof QWidget
    return application.exec(); //!< \memberof QApplication
}

// Constructor

US_ModelBuilder::US_ModelBuilder() : US_Widgets() {
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

    data_plot->setAxisScale(0, 1, 3.5);
    data_plot->setAxisScale(2, -7e-19, 1e16);
    //data_plot->setAxisScale(2, 0, 15);
    
    QwtLegend legend;
    legend.setFrameStyle(QFrame::Box | QFrame::Sunken);
    data_plot->insertLegend(&legend, QwtPlot::BottomLegend);

    resultsSpecs->addLayout(plot, 0, 2, 23, 1);

    resultsBox->setLayout(resultsSpecs);

    //set simparams to defualt values
    initalize_simulationParameters();

    //fill global gridLayout and apply
    specs->addWidget(dataBox, 0, 0);
    specs->addWidget(controlBox, 1, 0);
    specs->addWidget(resultsBox, 0, 1);
    top->addLayout(specs);
    this->setLayout(top);
}

//this function allows the user to open simparams GUI

void US_ModelBuilder::collectParameters(void) {
    working_simparams = simparams;

    US_SimulationParametersGui* dialog = new US_SimulationParametersGui(working_simparams);

    connect(dialog, SIGNAL(complete()), SLOT(update_parameters()));

    dialog->exec();
}

//this function updates the global simparams variable with user data

void US_ModelBuilder::update_parameters(void) {
    simparams = working_simparams;
    startButton -> setEnabled(true);
}

//this function allows the user to begin the simulation

void US_ModelBuilder::startSimulation(void) {
    //lock input fields
    le_sedimentationVal->setDisabled(true);
    le_diffusionVal->setDisabled(true);
    le_RMSD->setDisabled(true);
    startButton->setDisabled(true);
    parameterButton->setDisabled(true);

    //QVector<QVector2D> toPrint = generateFaxenGrid(3.5e-13, 5e-13, 1, 4, 500);
    //QVector<QVector<QVector2D*>* >* grid = readAnalyticalPoints("input/faxenGeneratorOutputFullsize.tsv");
    //QVector<QVector<QVector2D*>* >* grid = gridFromSide(0.0005, 0.00005);
    //QVector<QVector<QVector2D*>* >* grid = generateRegularGrid(3.5e-13, 5e-13, 1, 4, 2000);
    //QVector<QVector<QVector2D*>* >* grid = generateRegularGrid(1e-13, 1e-12, 1, 4, 8000);
    //QVector<QVector3D*>* grid = testRegularGrid(generateRegularGrid(1e-13, 1e-12, 1, 4, 8000));
    //QVector<QVector2D> faxenGrid = generateFaxenGrid(1e-13, 3.5e-13, 1, 4, 500);

    qDebug() << "Generating regular grid";
    QVector<QVector<QVector2D*>* >* raw = generateRegularGrid(1e-13, 1e-12, 1, 4, 100);

    qDebug() << "Calculating RMSDs for regular grid.";
    RegularGrid* rg = testRegularGrid(raw);

    //set RMSD Target
    rg->setRMSDTarget(0.2);

    //QVector< QVector < QVector3D* >* >* gridPoints = rg->getGrid();
  
    /*
    //scale up s-values to whole number range
    for(int y = 0; y < gridPoints->size(); y++) {
            for(int x = 0; x < gridPoints->at(y)->size(); x++) {
                    //do upscalaing
                    gridPoints->at(y)->at(x)->setX(gridPoints->at(y)->at(x)->x() * 1e13);
            }
    }*/

    qDebug() << "Grid generated. Creating annealing object...";
    //double pts[] = {rg->getGrid()->size() - 1, rg->getGrid()->at(0)->size() - 1}; //number of points to place on grid
    double pts[] = {35, 35};
    
    //grid gr(rg, pts, 2e-7);
    grid gr(rg, pts, 2e-7, 8); //only consider 8 nearest neighbors
    
    qDebug() << "Object created. Running annealing process..";
    gr.run(25, false, data_plot);

    qDebug() << "Annealing finished. Writing to file...";
    gr.write_pgrid("annealedGrid.out");

    //QFile outfile("output/testFaxenGeneratorOutput.tsv");
    //QFile outfile("output/leftSideGrid_overall.tsv");
    //QFile outfile("output/regularGridRMSDs_comparitive.tsv");
    //QFile outfile("output/localFaxenRMSDResultsAll.tsv");
    //QFile outfile("output/newRegularGridSurface.tsv");
    QFile outfile("output/interpolationSurface.tsv");

    outfile.open(QIODevice::ReadWrite);
    QTextStream outstream(&outfile);

    /*
    //interpolation validation code
    qDebug() << "Doing interpolation validation";
	
    QVector<QVector<QVector2D*>* >* testGrid = generateRegularGrid(-1.25e-14, 1.1125e-12, .5, 4.5, 10000);
    for(int y = 0; y < testGrid->size(); y++)
    {	
            for(int x = 0; x < testGrid->at(y)->size(); x++)
            {
                    QVector2D* current = testGrid->at(y)->at(x);
                    point* currentPt = new point(current);
                    double y = rg->interpolate(*currentPt);
                    outstream << current->x() << "\t" << current->y() << "\t" << y << endl;
                    delete currentPt;
            }
    }*/

    outfile.close();

    //qDebug() << "z-value: " << interpolatePoint(QVector3D(14.5, 20.2, 0), 
    //	QVector3D(14,20,91), QVector3D(14,21,162), QVector3D(15,20,210), QVector3D(15,21,95));

    qDebug() << "job finished";

    //re-enable parameter button, start button
    parameterButton->setDisabled(false);
    startButton->setDisabled(false);
}

//function to generate a regularly spaced grid using given initial and end points

QVector<QVector<QVector2D*> *>* US_ModelBuilder::generateRegularGrid(double sStart, double sEnd, double kStart, double kEnd, int pointQuantity) {
    //find integer approximation of square root of number of points
    int side = (int) qCeil((double) qSqrt(pointQuantity));

    //calculate regular s_value distance
    double sSpacing = (sEnd - sStart) / side;

    //calculate regular f/f0 difference
    double kSpacing = (kEnd - kStart) / side;

    //working variables used to construct point objects
    double currentK = kStart;
    double currentS = sStart;

    qDebug() << "size: " << pointQuantity << ",sidelen: " << side << ", sSpacing: " << sSpacing << ", kSpacing: " << kSpacing;

    //2D QVector to store all points in
    QVector<QVector<QVector2D*> *>* regularGrid = new QVector<QVector<QVector2D*> *>();

    //create all points in double for loop
    for (int x = 0; x < side; x++) {
        //initalize current row of grid 2D Vector
        regularGrid->append(new QVector<QVector2D*>);

        for (int y = 0; y < side; y++) {
            //add new point
            regularGrid->at(x)->append(new QVector2D(currentS, currentK));

            //increment s_value
            currentS += sSpacing;
        }

        //increment current f/f0
        currentK += kSpacing;

        //reset current s_value
        currentS = sStart;
    }

    int count = 0;
    for (int i = 0; i < regularGrid->size(); i++)
        count += regularGrid->at(i)->size();

    qDebug() << "final size: " << count;
    //regularGrid->at(regularGrid->size() - 1)->at(regularGrid->at(regularGrid->size() - 1)->size() -1)->print();

    //return the regulargrid
    return regularGrid;
}

RegularGrid* US_ModelBuilder::testRegularGrid(QVector<QVector<QVector2D*> *>* regularGrid) {
    qDebug() << "Rows in regularGrid: " + QString::number(regularGrid->size());

    //calculate s and ff0 differences from regularGrid
    double sDifference = abs(regularGrid->at(0)->at(0)->x() - regularGrid->at(0)->at(1)->x());
    double ff0Difference = abs(regularGrid->at(0)->at(0)->y() - regularGrid->at(1)->at(0)->y());

    //local variable instantiation
    //QVector<QVector3D*> results = new QVector<QVector3D*>();
    RegularGrid* results = new RegularGrid(sDifference, ff0Difference);

    QVector< QVector< QVector2D* >* >* rotatedRegularGrid = new QVector<QVector<QVector2D*> *>();
    QVector< QVector< QVector3D >* >* horizontalResults;
    QVector< QVector< QVector3D >* >* verticalResults;

    //rotate regular grid to allow RMSD spacing calculations along ff0 axis
    for (int x = 0; x < regularGrid->at(0)->size(); x++) {
        QVector<QVector2D*>* currentLine = new QVector<QVector2D*>();

        for (int y = 0; y < regularGrid->size(); y++) {
            currentLine->append(regularGrid->at(y)->at(x));
        }

        rotatedRegularGrid->append(currentLine);
    }

    //qDebug() << "testing grid 1";

    //test regularGrid and rotatedRegularGrid
    horizontalResults = testGrid(regularGrid);
    verticalResults = testGrid(rotatedRegularGrid);

    //qDebug() << "mapping grids";

    //iterate over regulargrid, and map results to correct points
    for (int y = 1; y < regularGrid->size() - 2; y++) {
        //qDebug() << "\tcreating vector for row " << y;

        //temporary vector to store row data
        QVector<QVector3D*>* row = new QVector<QVector3D*>();

        //qDebug() << "\tvector created";

        for (int x = 1; x < regularGrid->at(y)->size() - 2; x++) {
            //qDebug() << "\t\tfetch point";

            QVector2D* currentPoint = regularGrid->at(y)->at(x);

            //qDebug() << "\t\tcalculate point at x: " << x << " of " << regularGrid->at(y)->size() << " , y: " << y << " of " << regularGrid->size();

            double left = horizontalResults->at(y)->at(x - 1).z(); //qDebug() << "\t\t\tfetched left: " << left;
            double right = horizontalResults->at(y)->at(x + 1).z(); //qDebug() << "\t\t\tfetched right: " << right;
            double up = verticalResults->at(x)->at(y - 1).z(); //qDebug() << "\t\t\tfetched up: " << up;
            double down = verticalResults->at(x)->at(y + 1).z(); //qDebug() << "\t\t\tfetched down: " << down;

            //qDebug() << "\t\tAppend point to row";

            //calculate true RMSD and store
            row->append(new QVector3D(currentPoint->x(), currentPoint->y(),
                    qSqrt(pow(left, 2) + qPow(right, 2) + qPow(up, 2) + qPow(down, 2))));
        }

        qDebug() << "\tadd row to RegularGrid";

        //add to grid
        results->addRow(row);
    }

    return results;
}

/*QVector<QVector2D*> US_ModelBuilder::findBoundaryPoints(QVector<QVector3D*>)
{
        //local fields
        double minS = std::numeric_limits<double>::min();
        double minff0 = std::numeric_limits<double>::min();
        double maxS = std::numeric_limits<double>::max();
        double maxff0 = std::numeric_limits<double>::max();
	
        //find bottom left corner
        while
}*/


double US_ModelBuilder::interpolatePoint(QVector3D target, QVector3D p11, QVector3D p12, QVector3D p21, QVector3D p22) {
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

QVector<QVector<QVector2D*> *>* US_ModelBuilder::generateFaxenGrid(double sRangeStart, double sRangeEnd, double ff0RangeStart, double ff0RangeEnd, int numPoints) {
    qDebug() << "S_Value_0: " << sRangeStart << " S_Value_Last: " << sRangeEnd;
    qDebug() << "ff0_0: " << ff0RangeStart << " ff0_Last: " << ff0RangeEnd;

    //rescale s-range
    sRangeStart /= 1e-13;
    sRangeEnd /= 1e-13;

    //declare data storage structures
    QVector<QVector<QVector2D*>* >* faxenGrid = new QVector<QVector<QVector2D*>* >();
    QVector<QVector2D> jValues;

    //calculate values of mu
    double mu1 = sRangeStart * ff0RangeStart;
    double mu2 = sRangeEnd * sRangeEnd;

    //perform calculation for every point to be created
    for (int j = 0; j <= numPoints; j++) // numpoints equivalent of 'M' in specificaton, j iterates between 0 and M inclusive
    {
        //do computation for Ej value
        double Ej = qPow((1.0 - ((double) j) / ((double) numPoints)) * qPow(mu1, -0.25)
                + (((double) j) / ((double) numPoints)) * qPow(mu2, -0.25), -4.0);

        //save j and computed value
        jValues.append(QVector2D(j, Ej));
    }

    for (int i = 0; i <= numPoints; i++) // presumes that i should range between 0 and M also - TODO: Verify!
    {
        //do computation for Ei value
        double Ei = qPow((1.0 - ((double) i) / ((double) numPoints)) * qPow(mu1, -0.25)
                + (((double) i) / ((double) numPoints)) * qPow(mu2, -0.25), -4.0);

        //variable to indicate if current ff0 vector has been created
        bool fCreated = false;

        //only proceed if Ei valid
        if (Ei <= sRangeStart * ff0RangeEnd) {
            //calculate y-value
            double yi = Ei / sRangeStart;

            //do point calc for all values of j

            foreach(QVector2D currentJ, jValues) {
                //calculate point x-coord
                double xji = currentJ.y() / yi;

                //check that point falls in desired s-range
                if (xji >= sRangeStart && xji <= sRangeEnd) {
                    //rescale s-value
                    xji *= 1e-13;

                    //create data structure for current ff0 line
                    if (!fCreated) {
                        fCreated = true; //flip boolean
                        faxenGrid->append(new QVector<QVector2D *>());
                    }

                    faxenGrid->last()->append(new QVector2D(xji, yi));
                }
            }
        }
    }

    return faxenGrid;
}

//function to perform RMSD tests on a single dataset under globally set conditions

QVector<QVector<QVector3D>* >* US_ModelBuilder::testGrid(QVector<QVector<QVector2D*> *>* points) {
    //create qvector output
    QVector<QVector<QVector3D>* >* calculated_points = new QVector<QVector<QVector3D>* >();

    //data structures
    //QVector<QFuture<QVector<QVector3D>*> >* rmsd_futures = new QVector<QFuture<QVector<QVector3D>*> >();

    for (int i = 0; i < points->size(); i++) {
        //run calculation
        calculated_points->append(US_ModelBuilder::getBatchRMSD(points->at(i)));
    }

    /*for(int i = 1; i <= points->size(); i++)
    {
		
	
            //create new thread
            rmsd_futures->append(QtConcurrent::run(this, &US_ModelBuilder::getBatchRMSD, points->at(i - 1)));
	
            //qDebug()<<"thread created";
		
            //stop and wait for threads to complete every nth execution
            if(i % 3 == 0)
            {
                    //get results from all threads
                    for(int k = 0; k < rmsd_futures->size(); k++)
                            calculated_points->append(rmsd_futures->at(k).result());
			
                    //remove all contents
                    rmsd_futures->clear();
			
                    qDebug()<<"waited for threads to complete";
            }
    }*/

    //append last set of future results
    /*for(int k = 0; k < rmsd_futures->size(); k++)
            calculated_points->append(rmsd_futures->at(k).result());*/

    //remove all contents
    //rmsd_futures->clear();

    //return
    return calculated_points;
}

//function to perform RMSD tests on a dataset for a specified set of speeds

QVector<QVector<QVector3D> *>* US_ModelBuilder::testSpeeds(QVector<QVector<QVector2D*> *>* points, QVector<int>* speeds) {
    //create QVector to hold lists of RMSD and associated points
    QVector<QVector<QVector3D>* >* calculated_points = new QVector<QVector<QVector3D>* >();

    //create variables for use in loop
    QVector<QVector3D>* current_rmsd_batch = new QVector<QVector3D>();
    QVector<QFuture<QVector<QVector3D>*> >* rmsd_futures = new QVector<QFuture<QVector<QVector3D>*> >();

    //TODO: make constant omega2t for highest RPM (constant for now)
    double constant_omega2t = qPow((PI / 30) * 60000, 2) * 4800;

    qDebug() << "constant omega2t: " << constant_omega2t;

    //test dataset at each speed
    for (int i = 0; i < speeds->size(); i++) {
        //local double containing current speed
        int current_speed = speeds->at(i);

        //new time calculation system
        double current_duration = (constant_omega2t / qPow((current_speed * (PI / 30)), 2)) / 3600;

        //old time setting system
        //double current_duration = qPow(((double) current_speed) / ((double) original_speed), -2) * original_duration;
        simparams.speed_step[0].duration_hours = (int) current_duration; //removes decimal places; only gives hour value
        simparams.speed_step[0].duration_minutes = (int) ((current_duration - ((int) current_duration)) * 60);

        //set simparams SpeedProfile
        simparams.speed_step[0].rotorspeed = current_speed;
        simparams.speed_step[0].avg_speed = simparams.speed_step[0].rotorspeed;

        qDebug() << "speed: " << current_speed << ", duration (hrs): " << current_duration;
        //create lists for handling datasets
        current_rmsd_batch = new QVector<QVector3D>();
        rmsd_futures = new QVector<QFuture<QVector<QVector3D>*> >();

        for (int i = 1; i <= points->size(); i++) {
            //create new thread
            rmsd_futures->append(QtConcurrent::run(this, &US_ModelBuilder::getBatchRMSD, points->at(i - 1)));

            //stop and wait for threads to complete every 15th execution
            if (i % 15 == 0) {
                //get results from all threads
                for (int k = 0; k < rmsd_futures->size(); k++)
                    *current_rmsd_batch += *rmsd_futures->at(k).result();

                //remove all contents
                rmsd_futures->clear();
            }
        }

        //append last set of future results
        for (int k = 0; k < rmsd_futures->size(); k++)
            *current_rmsd_batch += *rmsd_futures->at(k).result();

        calculated_points->append(current_rmsd_batch);

        //delete futures list and rmsd batch
        delete rmsd_futures;
        delete current_rmsd_batch;
    }

    //restore original rotorspeed
    initalize_simulationParameters();

    //return points Vector
    return calculated_points;
}

US_DataIO::RawData* US_ModelBuilder::perform_calculation(QVector2D* p) {
    //temporary variables
    double s = p->x();
    double k = p->y();

    //create rawdata object to be used
    double D = calculate_diffusion(s, k);

    //qDebug() << "s, d, k" << s << D << k;

    US_Model* temp_model = get_model(s, D, k);
    US_DataIO::RawData* simulation = init_simData(temp_model);

    US_Astfem_RSA* simulator = new US_Astfem_RSA(*temp_model, simparams);

    //run sim
    simulator->calculate(*simulation);

    //erase simulator and model objects
    delete simulator;
    delete temp_model;

    //return RawData
    return simulation;
}



//TODO: arguments for selecting side to start on

QVector<QVector<QVector2D*>* >* US_ModelBuilder::gridFromSide(double targetRMSD, double toleranceRMSD) {
    int pointCount = 0;

    //create parameter qvector
    QVector<double>* parameters = new QVector<double>();
    parameters->append(toleranceRMSD);
    parameters->append(targetRMSD);

    //create endpoint qvector
    QVector2D startPoint(1e-13, 1);
    QVector2D endPoint(1e-13, 4);

    //disabled code for parallelization
    /*
    //create qFuture output qvector
    QVector<QFuture<QVector<QVector2D*>* > >* threadResults = new QVector<QFuture<QVector<QVector2D*>* > >();
	
    //vector of parameters vector
    QVector<QVector<double>*>* paramSets = new QVector<QVector<double>*>();
     */

    //create output qVector
    QVector<QVector<QVector2D*>* >* lines = new QVector<QVector<QVector2D*>* >();

    //calculate starting line
    QVector<QVector2D*>* startLine = calculate_line(parameters, &startPoint, &endPoint);

    //linear iteration over startline
    for (int i = 0; i < startLine->size(); i++) {
        QVector<double>* tparams = new QVector<double>();

        tparams->append(targetRMSD);
        tparams->append(startLine->at(i)->y());

        //tparams->append(1e-13);
        //tparams->append(1e-12 - 1e-13);
        tparams->append(3.5e-13);
        tparams->append(5e-13 - 3.5e-13);
        //tparams->append(5e-13);
        //tparams->append(7e-13 - 5e-13);

        qDebug() << "processing f/f0: " << startLine->at(i)->y() << " line";
        //QVector2D start(1e-13, startLine->at(i)->y());
        QVector2D start(3.5e-13, startLine->at(i)->y());
        //QVector2D end(1e-12, startLine->at(i)->y());
        QVector2D end(5e-13, startLine->at(i)->y());

        //lines->append(approximate_frictional_line(tparams));
        lines->append(calculate_line(parameters, &start, &end));
        pointCount += lines->last()->size();

        delete tparams;
    }

    qDebug() << pointCount << " points generated.";

    //delete line
    delete startLine;

    //return 
    return lines;
}

QVector<QVector2D*>* US_ModelBuilder::approximate_frictional_line(QVector<double>* params) //limit is the svalue maximum
{
    //unpack variables
    double rmsdTarget = params->at(0);
    double ff0 = params->at(1);
    double startingS = params->at(2);
    double limit = params->at(3);

    //create startpoint variables
    QVector2D* start = new QVector2D(startingS, ff0);
    US_DataIO::RawData* initialSim;
    US_DataIO::RawData* midSim;
    US_DataIO::RawData* endSim;

    //output vector of points
    QVector<QVector2D*>* line = new QVector<QVector2D*>();

    //add initial point to list
    line->append(start);
    initialSim = perform_calculation(start);

    QVector2D* tempNext1;
    QVector2D* tempNext2;
    double lastDifference = 5e-14;

    //do-while loop to conduct linear approximation
    do {
        //create temporary point for line-drawing to current start
        tempNext1 = new QVector2D(line->last()->x() + lastDifference, ff0);
        tempNext2 = new QVector2D(line->last()->x() + (2 * lastDifference), ff0);

        //do simulations for current start and next
        midSim = perform_calculation(tempNext1);
        endSim = perform_calculation(tempNext2);

        //calculate slope
        double slope = std::abs(calculate_RMSD(initialSim, endSim) - calculate_RMSD(initialSim, midSim)) / lastDifference;

        //calc point
        line->append(new QVector2D(line->last()->x() + (rmsdTarget / slope), ff0));

        //calculate and store distance
        lastDifference = calculateDistance(line->at(line->size() - 2), line->last());

        //erase temp contents
        delete tempNext1;
        delete tempNext2;
        delete midSim;
        delete endSim;
    } while (calculateDistance(start, line->last()) <= limit);

    //delete startpoint
    delete start;
    delete initialSim;

    //return line
    return line;
}

//suspected cause: error in endpoint selection

QVector<QVector2D*>* US_ModelBuilder::calculate_line(QVector<double>* params, QVector2D* initial, QVector2D* end) {
    //retrieve packaged data
    double toleranceRMSD = params->at(0);
    double targetRMSD = params->at(1);

    //declaration and init of variables
    QVector<QVector2D*>* line = new QVector<QVector2D*>();

    double distanceLimit = calculateDistance(initial, end);
    bool finished = false;

    US_DataIO::RawData* terminusSim = perform_calculation(end);
    US_DataIO::RawData* currentPointSim;
    US_DataIO::RawData* testPointSim;

    QVector2D* testPoint;
    QVector2D* highPoint;
    QVector2D* lowPoint;

    //add start to line
    line->append(initial);

    //outer loop over line
    do {
        //simulate current point
        currentPointSim = perform_calculation(line->last());

        //check that sufficient distance remains to find point
        if (calculate_RMSD(currentPointSim, terminusSim) > targetRMSD) {
            //local double to contain rmsd value
            double testRMSD;

            //setup low and high points
            lowPoint = new QVector2D(line->last()->x(), line->last()->y());
            highPoint = new QVector2D(end->x(), end->y());

            //internal loop to find correct point
            do {
                //calculate testPoint
                testPoint = calculateMidpoint(lowPoint, highPoint);

                //simulate testpoint
                testPointSim = perform_calculation(testPoint);

                //calculate rmsd
                testRMSD = calculate_RMSD(currentPointSim, testPointSim);

                //delete simulation
                delete testPointSim;

                //check if RMSD too low
                if (testRMSD < targetRMSD) {
                    delete lowPoint;
                    lowPoint = new QVector2D(testPoint->x(), testPoint->y());
                }                    //check if too high
                else if (testRMSD > targetRMSD) {
                    delete highPoint;
                    highPoint = new QVector2D(testPoint->x(), testPoint->y());
                }

                //check if testPoint will not be added - seperate if-structure
                if (std::abs(targetRMSD - testRMSD) <= toleranceRMSD && calculateDistance(initial, testPoint) > distanceLimit) {
                    delete testPoint;
                }
            } while (std::abs(targetRMSD - testRMSD) > toleranceRMSD);

            //delete temporary variables;
            delete lowPoint;
            delete highPoint;

            //check that point is valid to add
            if (calculateDistance(initial, testPoint) > distanceLimit) {
                finished = true;
            } else {
                line->append(testPoint);
            }
        } else {
            finished = true;
        }

        //delete temp variable
        delete currentPointSim;
    } while (!finished);

    //final delete of variables
    delete terminusSim;

    //return line 
    return line;
}

QVector2D* US_ModelBuilder::getNextPoint(QVector2D start, QVector2D end, double target, double tolerance, int maxIterations) {
    //TODO: replace
    maxIterations = 500;

    //variable declartion and instantiation
    int iterationCount = 0;
    QVector2D initial(start.x(), start.y());

    do {
        //get midpoint
        QVector2D* middle = calculateMidpoint(&initial, &end);

        //simulate initial and endpoints
        US_DataIO::RawData* initialSim = perform_calculation(&initial);
        US_DataIO::RawData* middleSim = perform_calculation(middle);

        //calculate RMSD
        double currentRMSD = calculate_RMSD(initialSim, middleSim);

        //delete simulations
        delete initialSim;
        delete middleSim;

        //check if middle is close enough
        if (std::abs(currentRMSD - target) <= tolerance)
            return middle;
            //otherwise, re-delimit search
        else if (currentRMSD < target) {
            initial.setX(middle->x());
            initial.setY(middle->y());
        } else if (currentRMSD > target) {
            end.setX(middle->x());
            end.setY(middle->y());
        }

        //increment counter
        iterationCount++;
    } while (iterationCount <= maxIterations);

    //if no value returned, loop must have ended due to hitting cap
    return NULL;
}

double US_ModelBuilder::calculate_diffusion(double s_val, double k_val) {

    double R = 8.314e7; // already defined in US_Constants
    double T = 293.15;
    double vbar = 0.72;
    double eta = 0.01;
    double N = 6.022e23;
    double rho = 1.0;
    double D;

    D = (R * T) / (N * k_val * 9.0 * eta * M_PI * pow((2.0 * s_val * k_val * vbar * eta)
            / (1.0 - vbar * rho), 0.5));
    //cout << "inside---- D: " << D << ", s: " << s_val << ", k: " << k_val << endl;

    return D;
}

//this function initalizes the simparams variable

void US_ModelBuilder::initalize_simulationParameters() {
    if (simparams.load_simparms("Resources/sp_defaultProfle.xml") != 0) {
        startButton->setEnabled(false);
        qDebug() << "Error: Simulation Parameters cannot be loaded. Manual selection required.";
    }
    working_simparams = simparams;
}

//this function determines the RMSD between two seperate RawData simulated datasets
//Precondition: simdata_1 and simdata_2 have the same number of scans and same number of datapoints per scan

double US_ModelBuilder::calculate_RMSD(US_DataIO::RawData* simdata_1, US_DataIO::RawData* simdata_2) {
    //double value to store RMSD
    double rmsd = 0.0;
    int num_scans = 0;
    int num_points = 0;
    int total_points = 0;

    //check that # scans and # points are all equal
    if (simdata_1->scanCount() != simdata_2->scanCount()) {
        //TODO: error message
        return -1;
        qDebug() << "RMSD Comparison failed";
    } else {
        //take scancount from simdata_1 b/c both are equivalent
        num_scans = simdata_1->scanCount();
    }

    //check all point qtys and ensure that they are the same
    if (simdata_1->pointCount() != simdata_2->pointCount())
        return -1;

    //get the number of points in set 1
    num_points = simdata_1->pointCount();

    //qDebug() << "NumScans, NumPoints: " << num_scans << " " << num_points;

    //iterate through scans and compare points values
    for (int x = 0; x < num_scans; x++)
        for (int y = 3; y < num_points - 50; y++) {
            rmsd += sq(simdata_1->value(x, y) - simdata_2->value(x, y));
            total_points++;
        }

    //perform other critical caluclations
    rmsd /= (double) total_points;
    rmsd = qSqrt(rmsd);

    //qDebug() << "" << rmsd;

    //return rmsd
    return rmsd;
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

//this function returns a hardcoded default model with specified s- and d- values

US_Model* US_ModelBuilder::get_model(double s_value, double d_value, double k_value) {
    //instantiate model
    US_Model* current_model = new US_Model();

    //init model
    US_Model::SimulationComponent* temp_component = new US_Model::SimulationComponent();

    current_model->components.push_back(*temp_component);
    current_model->components[0].s = s_value;
    current_model->components[0].D = d_value;
    current_model->components[0].f_f0 = k_value;

    current_model->update_coefficients();

    current_model->components[0].s = s_value;
    current_model->components[0].D = d_value;
    current_model->components[0].f_f0 = k_value;


    //current_model->debug();

    return current_model;
}

//Function to identify RMSD differences between every adjacent point in a QVector of Points

QVector<QVector3D>* US_ModelBuilder::getBatchRMSD(QVector<QVector2D*>* points) {
    //create list to store all output variables
    QVector<QVector3D>* RMSD_list = new QVector<QVector3D>();

    //create RawData objects
    US_DataIO::RawData* firstPoint = NULL;
    US_DataIO::RawData* secondPoint = NULL;

    //iterate over list
    for (int i = 0; i < points->size() - 1; i++) {
        if (points->at(i)->y() != 0 && points->at(i + 1)->y() != 0) {
            //get first model RawData if not already known
            if (firstPoint == NULL) {
                firstPoint = perform_calculation(points->at(i));
            }

            //get second model RawData
            secondPoint = perform_calculation(points->at(i + 1));

            //calculate and store RMSD
            RMSD_list->append(QVector3D(points->at(i)->x(), points->at(i)->y(), calculate_RMSD(firstPoint, secondPoint)));

            //delete first RawData object
            delete firstPoint;

            //store second RawData object
            firstPoint = secondPoint;
        }

    }


    //delete final instance of secondPoint
    delete secondPoint;

    //return the list
    return RMSD_list;
}

//Function to read from Faxen Solution datafile and create 2D QVector of S_Val, ff_0_Val Points

QVector<QVector<QVector2D*> *>* US_ModelBuilder::readAnalyticalPoints(QString filename) {
    //create file object to read from
    QFile input_file(filename);

    //create 2d QVector to contain all rows of points
    QVector<QVector<QVector2D*> *>* points_list = new QVector<QVector<QVector2D*> *>();

    //open file, and proceed if successful
    if (input_file.open(QIODevice::ReadOnly)) {
        //internal iterative reader
        QTextStream iterator(&input_file);
        QString current_line;
        QStringList current_line_contents;
        QVector<QVector2D*>* current_points;

        //iterate over input file
        while (!iterator.atEnd()) {
            //create local list of points
            current_points = new QVector<QVector2D*>();

            //get line from file
            current_line = iterator.readLine();

            //store current line as string array
            //current_line_contents = current_line.split(QRegExp("\\s"));
            current_line_contents = current_line.split(QRegExp("\\t"));

            //check if current line is not a comment
            if (QString::compare(current_line_contents.first(), "%") != 0 && !current_line.isEmpty()) {
                //local point list
                current_points = new QVector<QVector2D*>();

                //remove initial empty strings
                while (current_line_contents.first().isEmpty())
                    current_line_contents.removeFirst();

                //parse and store f/f0 value, removing from list
                double current_ff0 = current_line_contents.first().toDouble();
                current_line_contents.removeFirst();

                //create point objects for all subsequent pairs
                QString current_s;

                //create points for each non-empty d value found
                foreach(current_s, current_line_contents)
                if (!current_s.isEmpty())
                    //TODO: REMOVE THIS LINE
                    //if(current_s.toDouble() >= 5e-13 && current_s.toDouble() <= 7e-13)
                    current_points->append(new QVector2D(current_s.toDouble(), current_ff0));

                //add current points list
                points_list->append(current_points);
            }
        }
    } else {
        qDebug() << "Error reading analytical solution points.";
    }

    //close input
    input_file.close();

    /*int count = 0;
    for(int i = 0; i < points_list->size(); i++)
            count += points_list->at(i)->size();
		
    qDebug() << "size: " << count;*/

    //return list of points
    return points_list;
}

double US_ModelBuilder::calculateDistance(QVector2D* point1, QVector2D* point2) {
    double dist = 0.0;

    dist = qSqrt((pow(point2->x() - point1->x(), 2) + qPow(point2->y() - point1->y(), 2)));

    return dist;

}

QVector2D* US_ModelBuilder::calculateMidpoint(QVector2D* point1, QVector2D* point2) {
    QVector2D* mdpt = new QVector2D((point1->x() + point2->x()) / 2, (point1->y() + point2->y()) / 2);

    return mdpt;
}

/*
QVector<QVector<QVector2D*> >* US_ModelBuilder::identify_boundaryPoints(QVector2D* initial_point) {

    //variables for use in calculation loops
    //top_side->append(initial_point);
    //right_side->append(initial_point);
    //bottom_side->append(new QVector2D(initial_point->x(), gui_data->at(1)));
    //left_side->append(new QVector2D(gui_data->at(0), initial_point->y()));

    
    //variables to store search parameters
    //TODO: fetch values from GUI - store in array
    QVector<double>* gui_data = new QVector <double>;
    gui_data->append(0.00001); // RMSD tolerance
    gui_data->append(0.0005); // RMSD minimum

    //Create endpoint lists
    QVector<QVector2D*>* point_param_top = new QVector<QVector2D*>;
    point_param_top->insert(0, initial_point);
    point_param_top->insert(1, new QVector2D(gui_data->at(0), initial_point->y()));
	
    QVector<QVector2D*>* point_param_right = new QVector<QVector2D*>;
    point_param_right->insert(0, initial_point); //startpt
    point_param_right->insert(1, new QVector2D(initial_point->x(), gui_data->at(1))); //endpt

    QVector<QVector2D*>* point_param_bottom = new QVector<QVector2D*>;
    point_param_bottom->insert(0, new QVector2D(initial_point->x(), gui_data->at(1)));
    point_param_bottom->insert(1, new QVector2D(gui_data->at(0), gui_data->at(1)));

    QVector<QVector2D*>* point_param_left = new QVector<QVector2D*>;
    point_param_left->insert(0, new QVector2D(gui_data->at(0), initial_point->y()));
    point_param_left->insert(1, new QVector2D(gui_data->at(0), gui_data->at(1)));
	
    //Threads for all 4 calculations
    QFuture<QVector <QVector2D*>*> top_future = QtConcurrent::run(this, &US_ModelBuilder::calculate_line, 
            gui_data, point_param_top);
	
    QFuture<QVector <QVector2D*>*> right_future = QtConcurrent::run(this, &US_ModelBuilder::calculate_line, 
            gui_data, point_param_right);

    QFuture<QVector <QVector2D*>*> bottom_future = QtConcurrent::run(this, &US_ModelBuilder::calculate_line, 
            gui_data, point_param_bottom);

    QFuture<QVector <QVector2D*>*> left_future = QtConcurrent::run(this, &US_ModelBuilder::calculate_line, 
            gui_data, point_param_left);
		
    qDebug() << "all calcs started";
	
    //create return variable and get results from threads
    QVector<QVector<QVector2D*> >* sides = new QVector<QVector<QVector2D*> >;
    sides->append(*top_future.result());
    sides->append(*right_future.result());
    sides->append(*bottom_future.result());
    sides->append(*left_future.result());

    //debug
    qDebug() << "Outer level loop complete.";
    qDebug() << "Size of top side:" << sides->at(0).size();
    qDebug() << "Size of right side:" << sides->at(1).size();
    qDebug() << "Size of bottom side:" << sides->at(2).size();
    qDebug() << "Size of left side:" << sides->at(3).size();
    
    return NULL;
}*/
