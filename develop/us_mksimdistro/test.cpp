// compile with: g++ -L/usr/lib/qt/lib -lqt-mt test.cpp

#include <iostream>
#include <math.h>
#include <qstring.h>
#include <qfile.h>
#include <qtextstream.h>

using namespace std;
/*
Inputs needed:

number of components
starting molecular weight
model name/file name
model type (isodesmic/self-associating/individual)
for isodesmic/self-associating: how many components, partial concentration
for individual: partial concentration, molecular weight

*/
float rod_ff0(float);
void isodesmic(float, float, int, QString, int);
void self_associating(float, float, int, QString, int);

int main()
{
	QString str1;
	float vbar=0.65, start_mw=15000;
	int array=10;
	int conc_model=0;	// 0 = equal amounts for each component
						// 1 = gaussian concentration distribution
	QString name="test";
	isodesmic(start_mw, vbar, array, name, conc_model); // create a isodesmic system of spheres
//	self_associating(start_mw, vbar, array, name, conc_model); // create a self-associating system of spheres


/*
		p = 1.0/(3.0*pow(2.0*M_PI, 0.5)) * exp(-pow(((array/2.0)-i)/4.0, 2.0)/2.0);
		ts << p << endl;
		d = 8.314e7 * 293.3/(6.22e23 * f0);
		s = d * mw * (1 - vbar)/(8.314e7*293.3);
		p = 1.0/(3.0*pow(2.0*M_PI, 0.5)) * exp(-pow(((array/2.0)-i)/4.0, 2.0)/2.0);
		ts << p << endl;
		f = rod_ff0(ax) * f0;
		d = 8.314e7 * 293.3/(6.22e23 * f);
		s = d * mw * (1 - vbar)/(8.314e7*293.3);
		ts << s << endl;
		ts << d << endl;
		ts << "0" << "\n" << "0" << endl;
		cout << p << "\t" << s << "\t" << d << "\t" << mw << endl;
		ax += (120.0-16.0)/array;
*/
	return (0);
}

float rod_ff0(float p)
{
	return(pow((double) (2.0/3.0), (double) (1.0/3.0)) * pow((double)p, (double) (2.0/3.0)))/(log(2.0 * p) - 0.3);
}

void self_associating(float start_mw, float vbar, int array, QString name, int conc_model)
{
// this function produces the partial concentration of each component 
// with a gaussian concentration distribution. If the array variable is an odd integer,
// the center of the distribution will have the highest concentration.
	float s, d, mw, vol, f, f0, r, p;
	QFile file(name +".mod");
	file.open(IO_WriteOnly);
	QTextStream ts(&file);
	ts << name.latin1() << endl;
	ts << "0" << endl;
	ts << array << endl;
	mw = start_mw;
	for (int i=0; i<array; i++)
	{
		vol =  vbar * mw/6.22e23; // volume of molecule in ccm
		r = pow((double) (0.75*vol/M_PI), (double) (1.0/3.0)); // radius of sphere with the same volume
		f0 = 6 * M_PI * 0.01 * r; // frictional coefficient of sphere with the same volume
		if (conc_model == 0)
		{
			p = 1.0/array;
		}
		if (conc_model == 1)
		{
			p = 1.0/(3.0*pow(2.0*M_PI, 0.5)) * exp(-pow(((array/2.0)-i)/4.0, 2.0)/2.0);
		}
		ts << p << endl;
		d = 8.314e7 * 293.3/(6.22e23 * f0);
		s = d * mw * (1 - vbar)/(8.314e7*293.3);
		ts << s << endl;
		ts << d << endl;
		ts << "0" << "\n" << "0" << endl;
		cout << p << "\t" << s << "\t" << d << "\t" << mw << endl;
		mw += mw;
	}
}

void isodesmic(float start_mw, float vbar, int array, QString name, int conc_model)
{
	float s, d, mw, vol, f, f0, r, p;
	QFile file2(name +".dat");
	file2.open(IO_WriteOnly);
	QTextStream ts2(&file2);
	QFile file(name +".mod");
	file.open(IO_WriteOnly);
	QTextStream ts(&file);
	ts << name.latin1() << endl;
	ts << "0" << endl;
	ts << array << endl;
	mw = start_mw;
	for (int i=0; i<array; i++)
	{
		vol =  vbar * mw/6.22e23; // volume of molecule in ccm
		r = pow((double) (0.75*vol/M_PI), (double) (1.0/3.0)); // radius of sphere with the same volume
		f0 = 6 * M_PI * 0.01 * r; // frictional coefficient of sphere with the same volume
		if (conc_model == 0)
		{
			p = 10 + (100.0/array);
		}
		if (conc_model == 1)
		{
			p = 1.0/(3.0*pow(2.0*M_PI, 0.5)) * exp(-pow(((array/2.0)-i)/4.0, 2.0)/2.0);
		}
		ts << p << endl;

		d = 8.314e7 * 293.3/(6.22e23 * f0);
		s = d * mw * (1 - vbar)/(8.314e7*293.3);
		ts2 << s*1e13 << " " ;
		ts2 << p*i << endl;
		ts << s << endl;
		ts << d << endl;
		ts << "0" << "\n" << "0" << endl;
		cout << p << "\t" << s << "\t" << d << "\t" << mw << endl;
		mw += start_mw;
	}
}
