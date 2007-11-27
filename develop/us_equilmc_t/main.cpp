#include "../include/us_equilmc_t.h"

int main (int argc, char **argv)
{
	QString infile;
	if(argc < 2)
	{
		cout << "Usage: us_equilmc_t <path/input_file>\n\n";
		return(-1);
	}
	else
	{
		infile = argv[1];
	}
	QApplication a(argc, argv);
  	US_EquilMC_T *us_equilmc;
   us_equilmc = new US_EquilMC_T();
//	a.setMainWidget(us_equilmc);
	us_equilmc->read_init(infile);
	us_equilmc->monte_carlo();
	us_equilmc->monte_carlo_iterate();
//	return a.exec();
}
