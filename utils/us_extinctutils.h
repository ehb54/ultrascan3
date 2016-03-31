#ifndef US_EXTINCTUTILS_H
#define US_EXTINCTUTILS_H

//Structures needed for each file
struct Reading
{
   float lambda, od;
};

struct WavelengthScan
{
   QVector<Reading> v_readings;
   QString fileName, filePath;
   QString description;
	int pos, neg, runs;
};


#endif
