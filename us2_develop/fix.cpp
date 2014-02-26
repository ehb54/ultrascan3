#include <iostream>
#include <fstream>
#include <qfile.h>
#include <qstring.h>
#include <qtextstream.h>
#include <vector>

using namespace std;

int main(int argc, char *argv[])
{
	QString program, str;
	vector <QString> line;
	unsigned int i;
	int pos;
	program = argv[0];
	if (argc > 1)
	{
		str = argv[1];
	}
	else
	{
		cout << "No input file given, please provide an UltraScan project file..." << endl;
		exit(-1);
	}
	QFile f(str);
	if (!f.exists())
	{
		cout << "The file " << str << "does not exist, please provide a valid UltraScan project file..." << endl;
		exit(-2);
	}
	f.open(IO_ReadWrite);
	QTextStream ts(&f);
	while (!ts.atEnd())
	{
		str = ts.readLine();
		line.push_back(str);
	}
	f.remove();
	f.close();
	f.open(IO_ReadWrite);
	for (i=0; i<line.size(); i++)
	{
		if (line[i].contains("TARGET") > 0)
		{
			ts << "\nunix:contains (UNAME,x86_64) {\n";
			pos = line[i].find("us", 0);
			str = line[i].right(line[i].length()-pos);
			ts << "\tTARGET\t\t= ../../bin64/" + str << endl;
			ts << "} else {\n";
			ts << "\tTARGET\t\t= ../../bin/" + str << endl;
			ts << "}\n\n";
		}
		else
		{
			ts << line[i] << endl;
		}
	}
	return (0);
}
