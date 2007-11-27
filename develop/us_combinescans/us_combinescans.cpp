#include <iostream>
#include <vector>
#include <algorithm>
#include <qstring.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qstringlist.h>
#include <qdir.h>
#include <qapp.h>

using namespace std;

struct xyplot
{
	vector <float> x;
	vector <float> y;
};

int main(int argc, char **argv)
{
	QString str, run_id;
	unsigned int i, j, k;
	float offset, fval;
	QFile f;
	if(argc < 3)
	{
		cout << "\nUsage: us_combinescans run_id offset<float>\n\n";
		return(-1);
	}
	else
	{
		run_id = argv[1];
		str = argv[2];
		offset = str.toFloat();
	}
	vector <struct xyplot> residuals;
	vector <struct xyplot> model;
	vector <struct xyplot> raw;
	raw.clear();
	residuals.clear();
	model.clear();
	QDir currentDir;
	currentDir.setPath(currentDir.currentDirPath());
	QStringList slist;
	slist = currentDir.entryList(run_id + "*.dat");
	residuals.resize(slist.size());
	model.resize(slist.size());
	raw.resize(slist.size());
	i = 0;
	for (QStringList::Iterator it = slist.begin(); it != slist.end(); it++)
	{
		f.setName(*it);
		if(f.open(IO_ReadOnly))
		{
			QTextStream ts(&f);
			ts.readLine();
			residuals[i].x.clear();
			model[i].x.clear();
			raw[i].x.clear();
			model[i].y.clear();
			raw[i].y.clear();
			residuals[i].y.clear();
			while (!ts.atEnd())
			{
				ts >> fval;
				residuals[i].x.push_back(fval);
				model[i].x.push_back(fval);
				raw[i].x.push_back(fval);
				ts >> fval;
				model[i].y.push_back(fval);
				ts >> fval;
				raw[i].y.push_back(fval);
				ts >> fval;
				residuals[i].y.push_back(fval);
			}
			f.close();
		}
		i++;
	}
	f.setName("residuals_combined.dat");
	if (f.open(IO_WriteOnly|IO_Translate))
	{
		QTextStream ts(&f);
		ts.width(14);
		ts.flags(0x1000);
		j=0;
		k=0;
		for (i=0; i<raw.size(); i++)
		{
			k = max((unsigned int) raw[i].x.size(), (unsigned int) k);
		}
		for (i=0; i<k; i++)
		{
			for (j=0; j<raw.size(); j++)
			{
				if (i < raw[j].x.size())
				{
					if(j != raw.size()-1)
					{
						ts << raw[j].x[i] << "," << (residuals[j].y[i]+(j*offset)) << ",";
					}
					else
					{
						ts << raw[j].x[i] << "," << (residuals[j].y[i]+(j*offset)) << endl;
					}
				}
				else
				{
					if (j != raw.size()-1)
					{
						ts << ",,";
					}
					else
					{
						ts << endl;
					}
				}
			}
		}
		f.close();
	}
	f.setName("residuals_baselines.dat");
	if (f.open(IO_WriteOnly|IO_Translate))
	{
		QTextStream ts(&f);
		for (i=0; i<raw.size()-1; i++)
		{
			ts << raw[i].x[0] << "," << i * offset << ",";
		}
		ts << raw[i].x[0] << "," << i * offset << endl;
		for (i=0; i<raw.size()-1; i++)
		{
			ts << raw[i].x[raw[i].x.size()-1] << "," << i * offset << ",";
		}
		ts << raw[i].x[raw[i].x.size()-1] << "," << i * offset << endl;
	}
	f.close();
	f.setName("raw_data.dat");
	if (f.open(IO_WriteOnly|IO_Translate))
	{
		QTextStream ts(&f);
		ts.width(14);
		ts.flags(0x1000);
		j=0;
		k=0;
		for (i=0; i<raw.size(); i++)
		{
			k = max((unsigned int) raw[i].x.size(), (unsigned int) k);
		}
		for (i=0; i<k; i++)
		{
			for (j=0; j<raw.size(); j++)
			{
				if (i < raw[j].x.size())
				{
					if(j != raw.size()-1)
					{
						ts << raw[j].x[i] << "," << raw[j].y[i] << ",";
					}
					else
					{
						ts << raw[j].x[i] << "," << raw[j].y[i] << endl;
					}
				}
				else
				{
					if (j != raw.size()-1)
					{
						ts << ",,";
					}
					else
					{
						ts << endl;
					}
				}
			}
		}
		f.close();
	}
	f.setName("model_data.dat");
	if (f.open(IO_WriteOnly|IO_Translate))
	{
		QTextStream ts(&f);
		ts.width(14);
		ts.flags(0x1000);
		j=0;
		k=0;
		for (i=0; i<raw.size(); i++)
		{
			k = max((unsigned int) raw[i].x.size(), (unsigned int) k);
		}
		for (i=0; i<k; i++)
		{
			for (j=0; j<raw.size(); j++)
			{
				if (i < raw[j].x.size())
				{
					if(j != raw.size()-1)
					{
						ts << raw[j].x[i] << "," << model[j].y[i] << ",";
					}
					else
					{
						ts << raw[j].x[i] << "," << model[j].y[i] << endl;
					}
				}
				else
				{
					if (j != raw.size()-1)
					{
						ts << ",,";
					}
					else
					{
						ts << endl;
					}
				}
			}
		}
		f.close();
	}
	return (0);
}
