// compile with:
// g++ multiline.cpp -I$QTDIR/include -L$QTDIR/lib -lqt-mt -o multiline
#include <qstring.h>
#include <iostream>
#include <qfile.h>
#include <qtextstream.h>
#include <vector>
using namespace std;

int main(int argc, char **argv)
{
	QString infile, str;
	vector <QString> line;
	line.clear();
	if (argc < 2)
	{
		cout << "Usage: multiline Input file...\n";
		return -1;
	}
	else
	{
		infile = argv[1];
	}
	QFile f(infile);
	if (f.open(IO_ReadOnly))
	{
		QTextStream ts(&f);
		while (!ts.atEnd())
		{
			line.push_back(ts.readLine());
		}
		f.close();
	}
	f.remove();
	f.open(IO_WriteOnly);
	QTextStream ts(&f);

	unsigned int val;
	for (unsigned int i=0; i<line.size(); i++)
	{
		if (line[i] == "unix:contains(UNAME,Darwin) {")
		{
			cout << "line contains search string...\n";
			val = i+4;
		}
	}
	for (unsigned int i=0; i<val; i++)
	{
		ts << line[i] << endl;
		cout << "i=" << i << ": " << line[i] << endl; 
	}
	ts << "unix:contains (UNAME,Darwin) {\n\tDEFINES += OSX\n\tUNAME = $$system(uname -p)\n\tmessage (\"Configuring for the Darwin Macintosh OS-X operating system...\")\n}\n";
	for (unsigned int i=val; i<line.size(); i++)
	{
		ts << line[i] << endl;
	}
}
