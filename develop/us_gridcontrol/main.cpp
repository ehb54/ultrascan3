#include "../include/us_gridcontrol.h"

int main (int argc, char **argv)
{
	QString infile;
	QString gridopt;
	if(argc < 2)
	{
		cout << "Usage: us_gridcontrol <path/input_file>\n\n";
		return(-1);
	}
	else
	{
		infile = argv[1];
		if(argc > 2) {
		    gridopt = argv[2];
		}
	}
	QApplication a(argc, argv);
  	US_GridControl *gridcontrol;
	gridcontrol = new US_GridControl(infile, gridopt);
//	gridcontrol->read_init(infile);
	return a.exec();
//	return(0);
}
