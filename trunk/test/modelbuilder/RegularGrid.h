#ifndef REGULARGRID_H
#define REGULARGRID_H

#include <QVector>
#include <QVector3D>
#include <QDebug>

class point; //forward declaration of point

class RegularGrid {
public:
    //define constructor, using regular s and ff0 differences

    RegularGrid(double s, double ff0) {
        sDifference = s;
        ff0Difference = ff0;
        sMin = -999;
        sMax = 999;
        ff0Min = -999;
        ff0Max = 999;

        grid = new QVector< QVector < QVector3D* >* >();
    }

    //define destructor

    ~RegularGrid() {
        delete grid;
    }

    QVector3D* get(int r, int c);
    void addRow(QVector<QVector3D*>* row);
    void setRMSDTarget(double toSet);
    double interpolate(point target);
    double getRMSDTarget();
    int getNumRows();
    int getNumCols();
    QVector< QVector < QVector3D* >* >* getGrid();

private:
    QVector< QVector < QVector3D* >* >* grid;
    double sDifference;
    double ff0Difference;
    double sMin;
    double sMax;
    double ff0Min;
    double ff0Max;
    double RMSDTarget;
};

#endif
