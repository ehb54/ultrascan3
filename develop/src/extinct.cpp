#include <iostream>

using namespace std;

main()
{
	double extinction=8960, Ka=55.299, Kd, pathlength=1.2;

	Kd = 1.0/(Ka/pathlength);
	Kd /= extinction;
	cout << Kd << endl;
}
