#include <qfile.h>
#include <qstring.h>
#include <qtextstream.h>

#include <vector>
#include <iostream>
#include <set>

using namespace std;

struct scan
{
	vector <float> x, model, raw, residual;
};

main()
{
	QString s, file;
	QFile fin, fout;
	set <float> xval;
	unsigned int i, j,k;
	struct scan temp_scan;
	vector <struct scan> allscans;
	allscans.clear();
	k=0;
	
	for (i=1; i<=34; i++)
	{
		temp_scan.x.clear();
		temp_scan.model.clear();
		temp_scan.raw.clear();
		temp_scan.residual.clear();
		fin.setName(s.sprintf("PAH2-equil1-4.%d.dat", i));
		if (fin.open(IO_ReadOnly))
		{
			QTextStream ts(&fin);
			while (!ts.eof())
			{
				ts.readLine();
				ts >> s;
				temp_scan.x.push_back(s.toFloat());
				ts >> s;
				temp_scan.model.push_back(s.toFloat());
				ts >> s;
				temp_scan.raw.push_back(s.toFloat());
				ts >> s;
				temp_scan.residual.push_back(s.toFloat());
			}
		}
		fin.close();
		allscans.push_back(temp_scan);
		cout << k << endl;
		k++;
	}
	
	for (i=0; i<allscans.size(); i++)
	{
		for (j=0; j<allscans[i].x.size(); j++)
		{
			xval.insert(allscans[i].x[j]);
		} 
	}
	set <float>::iterator it;
	fout.setName("model.dat");
	if (fout.open(IO_WriteOnly))
	{
		QTextStream ts(&fout);
		for (it=xval.begin(); it!=xval.end();  it++)
		{
			ts << *it << ",";
			for (i=0; i<allscans.size(); i++)
			{
				k=0;
				while(k < allscans[i].x.size()-1 && allscans[i].x[k] < *it)
				{
					k++;
				}
				if (allscans[i].x[k] == *it)
				{
					ts << allscans[i].model[k] << ",";
				}
				else
				{
					ts << "       ,";
				}
			}
			ts << endl;
		}
		fout.close();
	}
	fout.setName("residual.dat");
	if (fout.open(IO_WriteOnly))
	{
		QTextStream ts(&fout);
		for (it=xval.begin(); it!=xval.end();  it++)
		{
			ts << *it << ",";
			for (i=0; i<allscans.size(); i++)
			{
				k=0;
				while(k < allscans[i].x.size()-1 && allscans[i].x[k] < *it)
				{
					k++;
				}
				if (allscans[i].x[k] == *it)
				{
					ts << allscans[i].residual[k] << ",";
				}
				else
				{
					ts << "       ,";
				}
			}
			ts << endl;
		}
		fout.close();
	}
	fout.setName("raw.dat");
	if (fout.open(IO_WriteOnly))
	{
		QTextStream ts(&fout);
		for (it=xval.begin(); it!=xval.end();  it++)
		{
			ts << *it << ",";
			for (i=0; i<allscans.size(); i++)
			{
				k=0;
				while(k < allscans[i].x.size()-1 && allscans[i].x[k] < *it)
				{
					k++;
				}
				if (allscans[i].x[k] == *it)
				{
					ts << allscans[i].raw[k] << ",";
				}
				else
				{
					ts << "       ,";
				}
			}
			ts << endl;
		}
		fout.close();
	}
}
