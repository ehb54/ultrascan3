// Compile with: g++ write_portal.cpp -lqt -o write_portal
#include <iostream.h>
#include <fstream.h>
#include <string.h>
#include <stdlib.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qstring.h>

int main( int argc, char *argv[])
{
	QString infile="webpage.dat", outfile="webpage.html", str1, str2;
	QFile file_in(infile);
	file_in.open(IO_ReadWrite);
	QTextStream ts_in(&file_in);
	while (!ts_in.eof())
	{
		for (int i=0; i<2; i++)   	// throw away the first 2 tokens
		{
			ts_in >> line;
		}
		ts_in >> line;			// save the 3rd
		sum += line.toLong();
		count ++;
		ts_in.readLine();		// read the rest of the line and throw away.
	}
	dsize = double (sum);
	str.sprintf("%6.4e", dsize);
	cout << "\nThis backup contains " << count << " files consuming ";
	cout << str << " bytes of storage\n" ;
	str.sprintf("%6.4f", dsize/1e3);
	cout << "(= " << str << " KB; = ";
	str.sprintf("%6.5f", dsize/1e6);
	cout << str << " MB; = ";
	str.sprintf("%6.7f", dsize/1e9);
	cout << str << " GB)\n\n ";
}
