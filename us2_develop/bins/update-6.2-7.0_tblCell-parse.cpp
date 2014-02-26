#include <qstring.h>
#include <vector>
#include <iostream>
#include <qfile.h>
#include <qtextstream.h>

using namespace std;

QString getToken(QString *str, const QString &separator);

int main()
{
	QFile f("tblCell.dll");
	int count=0;
	QString s1,s3;
	vector <QString> s2;
	f.open(IO_ReadOnly);
	QTextStream ts(&f);
	while (!ts.atEnd())
	{
		s1 = ts.readLine();
		count ++;
		s2.clear();
		while (s1.length() > 0)
		{
			s3 = getToken(&s1, ",");
			if(s3.left(1) == "'")
			{
				while (s3.right(1) != "'")
				{
					s3 += getToken(&s1, ",");
				}
				s2.push_back(s3);
			}
			else
			{
				s2.push_back(s3);
			}
		}
//		if (s2.size() > 25)
//		{
//			cout << "Line number: " << count << ", size: " << s2.size() << endl;
//		}
		
		cout << "SET FOREIGN_KEY_CHECKS=0;\n";
		for (unsigned int i=0; i<s2.size(); i++)
		{
			if (i==11 || i==13 || i==14 || i==16 || i==17 || i==19 || i==20 || i==22)
			{
				cout << s2[i] << ",NULL,NULL,";
			}
			else if (i==24)
			{
				cout << s2[i];
			}
			else
			{
				cout << s2[i] << ",";
			}
		}
		cout << endl;
	}
	cout << "SET FOREIGN_KEY_CHECKS=1;\n";
}

QString getToken(QString *str, const QString &separator)
{
   int pos;
   QString token;
   pos = str->find(separator, 0, false);
   if (pos < 0)
   {
      if (str->length() > 0)
      {
         token = (*str);
         (*str) = "";
         return (token);
      }
      else
      {
         return((QString) "");
      }
   }
   while (pos == 0)
   {
      (*str) = str->mid(pos + 1, str->length());
      pos = str->find(separator, 0, false);
      if (pos < 0)
      {
         if (str->length() > 0)
         {
            token = (*str);
            (*str) = "";
            return (token);
         }
         else
         {
            return((QString) "");
         }
      }
   }
   token = str->left(pos);
   (*str) = str->mid(pos + 1, str->length());
   return(token);
}

