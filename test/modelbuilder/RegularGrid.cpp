//! \file RegularGrid.cpp

#include "RegularGrid.h"
#include "points2.h"

QVector3D* RegularGrid::get(int r, int c) {
    return grid->at(r)->at(c);
}

void RegularGrid::addRow(QVector<QVector3D*>* row) {
    //add new row
    grid->append(row);

    //re-check max and min values
    for (int i = 0; i < row->size(); i++) {
        if (row->at(i)->x() > sMax)
            sMax = row->at(i)->x();
        else if (row->at(i)->x() < sMin)
            sMin = row->at(i)->x();

        if (row->at(i)->y() > ff0Max)
            ff0Max = row->at(i)->y();
        else if (row->at(i)->y() < ff0Min)
            ff0Min = row->at(i)->y();
    }
}

double RegularGrid::interpolate(point target) {
    //extract x and y values
    double x = target.x[0];
    double y = target.x[1];

    int numAdjacent = 0;

    double x1;
    double y1;
    double x2;
    double y2;

    bool outOfBounds = false;

    int rowIterations = 0; //row ahead of point
    int colIterations = 0; //column above point

    //find neighbors
    while (rowIterations < grid->size() && (grid->at(0)->at(0)->y() + ff0Difference * rowIterations) < y)
        rowIterations++;
    while (colIterations < grid->at(0)->size() && (grid->at(0)->at(0)->x() + sDifference * colIterations) < x)
        colIterations++;

    QVector3D* p11 = NULL;
    QVector3D* p21 = NULL;
    QVector3D* p12 = NULL;
    QVector3D* p22 = NULL;

    //check that target falls within boundaries, and fetch points if safe
    if (rowIterations - 1 >= 0 && rowIterations - 1 < grid->size() && colIterations - 1 >= 0 && colIterations - 1 < grid->at(rowIterations - 1)->size()) {
        p11 = grid->at(rowIterations - 1)->at(colIterations - 1);
        numAdjacent++;
    } else
        outOfBounds = true;

    if (rowIterations - 1 >= 0 && rowIterations - 1 < grid->size() && colIterations >= 0 && colIterations < grid->at(rowIterations - 1)->size()) {
        p21 = grid->at(rowIterations - 1)->at(colIterations);
        numAdjacent++;
    } else
        outOfBounds = true;

    if (rowIterations >= 0 && rowIterations < grid->size() && colIterations - 1 >= 0 && colIterations - 1 < grid->at(rowIterations)->size()) {
        p12 = grid->at(rowIterations)->at(colIterations - 1);
        numAdjacent++;
    } else
        outOfBounds = true;

    if (rowIterations >= 0 && rowIterations < grid->size() && colIterations >= 0 && colIterations < grid->at(rowIterations)->size()) {
        p22 = grid->at(rowIterations)->at(colIterations);
        numAdjacent++;
    } else
        outOfBounds = true;

    if (!outOfBounds) {
        //qDebug() << "Particle normally positioned";

        x1 = p11->x();
        y1 = p11->y();
        x2 = p22->x();
        y2 = p22->y();

        //interpolate on x-direction
        double xy1 = ((x2 - x) / (x2 - x1)) * p11->z() + ((x - x1) / (x2 - x1)) * p21->z();
        double xy2 = ((x2 - x) / (x2 - x1)) * p12->z() + ((x - x1) / (x2 - x1)) * p22->z();

        //calculate approximate interpolation - uses abbreviated formula, possibly replace in future
        return (((y2 - y) / (y2 - y1)) * xy1 + ((y - y1) / (y2 - y1)) * xy2);
    } else {
        //qDebug() << "out of bounds case, numAdjacent: " << numAdjacent;

        if (numAdjacent == 1) //case for situations when target is outside a corner of the grid
        {
            if (p11 != NULL)
                return p11->z();
            else if (p21 != NULL)
                return p21->z();
            else if (p12 != NULL)
                return p12->z();
            else if (p22 != NULL)
                return p22->z();
        }
            //case for situations in which target is outside a side of the grid
        else if (numAdjacent == 2) {
            /*qDebug() << "rowIterations: " << rowIterations << "\t colIterations: " << colIterations;
			
            if(p11 != NULL)
                    qDebug() << "p11: " << *p11;
            if(p21 != NULL)
                    qDebug() << "p21: " << *p21;
            if(p12 != NULL)
                    qDebug() << "p12: " << *p12;
            if(p22 != NULL)
                    qDebug() << "p22: " << *p22;*/

            //target is above top of grid
            if (rowIterations == 0) {
                //qDebug() << "\ttop";
                return (p12->z() + (p22->z() - p12->z()) * ((x - p12->x()) / (p22->x() - p12->x())));
            }
                //target is to left of grid
            else if (colIterations == 0) {
                //qDebug() << "\tleft";
                return (p21->z() + (p22->z() - p21->z()) * ((y - p21->y()) / (p22->y() - p21->y())));
            }
                //target is below grid
            else if (rowIterations == grid->size()) {
                //qDebug() << "\tbelow";
                return (p11->z() + (p21->z() - p11->z()) * ((x - p11->x()) / (p21->x() - p11->x())));
            }
                //target is to right of grid
            else if (colIterations == grid->at(0)->size()) {
                //qDebug() << "\tright";
                return (p11->z() + (p12->z() - p11->z()) * ((y - p11->y()) / (p12->y() - p11->y())));
            } else
                qDebug() << "Warning: numAdjacent is 2, but target position relative to grid not found";

        }
        else {
            qDebug() << "Warning: target out of bounds, but interpolation identified 3 neighbors.";
        }
    }
}

/*double RegularGrid::interpolate(point target)
{
        //extract x and y values
        double x = target.x[0];
        double y = target.x[1];
	
        double x1;
        double y1;
        double x2;
        double y2;
	
        int rowIterations = 0; //row ahead of point
        int colIterations = 0; //column above point
	
        qDebug() << "locating neighbors part 1/3";
	
        //find neighbors
        while(rowIterations < grid->size() && (grid->at(0)->at(0)->y() + ff0Difference * rowIterations) < y)
                rowIterations++;
        while(colIterations < grid->at(0)->size() && (grid->at(0)->at(0)->x() + sDifference * colIterations) < x)
                colIterations++;
	
        qDebug() << "locating neighbors part 2/3";
	
        qDebug() << "\trowIterations: " << rowIterations;
        qDebug() << "\tcolIterations: " << colIterations;
        qDebug() << "\trowSize: " << grid->size();
        qDebug() << "\tcolSize: " << grid->at(0)->size();
	
        qDebug() << "\t\t\Neighbor subpart 1/4";
        QVector3D* p11 = grid->at(rowIterations - 1)->at(colIterations - 1); qDebug() << "\t\t\Neighbor subpart 2/4";
        QVector3D* p21 = grid->at(rowIterations - 1)->at(colIterations); qDebug() << "\t\t\Neighbor subpart 3/4";
        QVector3D* p12 = grid->at(rowIterations)->at(colIterations - 1); qDebug() << "\t\t\Neighbor subpart 4/4";
        QVector3D* p22 = grid->at(rowIterations)->at(colIterations);
	
        qDebug() << "locating neighbors part 3/3";
	
        x1 = p11->x();
        y1 = p11->y();
        x2 = p22->x();
        y2 = p22->y();
	
        qDebug() << "Doing calculation set 1/2";
	
        //interpolate on x-direction
        double xy1 = ((x2 - x)/(x2 - x1)) * p11->z() + ((x - x1)/(x2 - x1)) * p21->z();
        double xy2 = ((x2 - x)/(x2 - x1)) * p12->z() + ((x - x1)/(x2 - x1)) * p22->z();
	
        qDebug() << "Doing calculation set 2/2 and returning";
	
        //calculate approximate interpolation - uses abbreviated formula, possibly replace in future
        return (((y2 - y)/(y2 - y1)) * xy1 + ((y - y1)/(y2 - y1)) * xy2);
}*/

void RegularGrid::setRMSDTarget(double toSet) {
    RMSDTarget = toSet;
}

double RegularGrid::getRMSDTarget() {
    return RMSDTarget;
}

int RegularGrid::getNumRows() {
    return grid->size();
}

int RegularGrid::getNumCols() {
    return grid->at(0)->size();
}

QVector< QVector < QVector3D* >* >* RegularGrid::getGrid() {
    return grid;
}
