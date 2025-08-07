#ifndef US_H
#define US_H

#include "us3_defines.h"

#ifdef WIN32
#include "Windows.h"
#endif

#include <qcolor.h>
#include <qpalette.h>
#include <vector>

using namespace std;

struct XY_data {
      vector<float> X;
      vector<float> Y;
};

struct XY_data_UI {
      vector<unsigned int> X;
      vector<unsigned int> Y;
};

struct XYZ_data {
      vector<float> X;
      vector<float> Y;
      vector<float> Z;
};

// define structure variable type us_colors:

//#define square(value) ((value) * (value))

const int spacing = 2;
const int border = 4;
const int column1 = 140; // name
const int column2 = 100; // value
const int column3 = 100; // bounds
const int column4 = 70; // unit
const int column5 = 40; // float?
const int column6 = 50; // bounds-lcd
const int span = column1 + column2 + column3 + column4 + 2 * column5 + column6 + 6 * spacing;


#endif
