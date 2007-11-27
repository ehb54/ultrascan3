#include "../include/us_velocdudmc_t.h"

int main (int argc, char **argv)
{
	QString infile;
	if(argc < 2)
	{
		cout << "Usage: us_velocdudmc_t <path/input_file>\n\n";
		return(-1);
	}
	else
	{
		infile = argv[1];
	}
	QApplication a(argc, argv);
  	US_VelocDud_T *us_velocdudmc;
   us_velocdudmc = new US_VelocDud_T();
	a.setMainWidget(us_velocdudmc);
	us_velocdudmc->read_init(infile);
	us_velocdudmc->monte_carlo();
	return a.exec();
}
