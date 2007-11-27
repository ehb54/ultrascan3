#include <iostream.h>
#include <math.h>

int main()
{
	float f1, f2, f3, r1, r2, r3;
	f1 = 10.53925;
	f2 = 2.209063e-01;
	f3 = -1.859808e-01;
	r1 = 1.0/exp(f1 + f2);
	r2 = 1.0/exp(f1 + f3);
	r3 = 1.0/exp(f1);
	cout << "Kd + limit: " << r1 << endl;
	cout << "Kd - limit: " << r2 << endl;
	cout << "Kd : " << r3 << endl;
	cout << "+95 CI: " << r1 - r3 << endl; 
	cout << "-95 CI: " << r2 - r1 << endl; 
	return (0);
}
