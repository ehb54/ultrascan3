#ifndef US_POINTS2_H
#define US_POINTS2_H

#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <QVector2D>
#include "RegularGrid.h"
#include "us_plot.h"

// #define THREADS 8

#ifdef THREADS
#include <pthread.h>
#endif

using namespace std;

#define DIM 2

class point {
public:
    vector < double > x;
    int id;

    point();
    point(double v, int = -1);
    point(vector < double >, int = -1);
    point(double *, int = -1);
    point(QVector2D *, int = -1);
    static point point_min(const point&, const point&);
    static point point_max(const point&, const point&);
    static double mag2(const point&);
    static double r2(const point&, const point&);
    static double r3(const point&, const point&);
    static double r6(const point&, const point&);
    point operator+(const point&);
    point operator-(const point&);
    point operator*(const double &);
    point operator/(const double &);
    bool operator<(const point &) const;
    bool operator==(const point &) const;
    bool operator!=(const point &) const;
    //friend ostream& operator<<(ostream&, const point&);
    //friend ostream& operator<<(ostream&, const set < point > &);
};

class grid {
public:
    /*grid( 
         point     start                     // starting corner
         ,point     end                      // ending corner
         ,point     pts                      // points in each direction for initial grid
         //,double    (*charge)( point )       // provided function which returns the charge of a point
         ,double    deltat                   // delta t
         ,int       neighbour_points   = 0   // maximum nearest neighbour points, 0 for no nearest neighbour 
         ,int       neighbour_steps    = 0   // number of delta t steps before recomputing neighbour 
          );*/

    grid(RegularGrid* rg, point pts, double deltat, int neighbour_points = 0, int neighbour_steps = 0);
    //grid( RegularGrid* rg ,double deltat ,int neighbour_points = 0,int neighbour_steps = 0 );
    void run(int steps, bool do_write = false, QwtPlot* grid_display = NULL);

    bool write_pgrid(string file);
    bool in_bounds(point p);
    double charge(point p);
    double* get_dim_values(int dim);

private:

    point F(point);

    //double                         (*charge)(point);
    double deltat;
    int neighbour_points;
    int neighbour_steps;

    point pmin;
    point pmax;
    point pwidth;

    set < point > pgrid;
    set < point > pgrid_mobile;
    set < point > corners;
    set < point > edges;
    map < point, set < point > > neighbours;

    RegularGrid* calculatedGrid;

    void recompute_neighbours(double mult = 0);
    void trim_edges_not_neighbours();
};

#endif
