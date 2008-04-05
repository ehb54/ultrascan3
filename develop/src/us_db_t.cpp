#include "../include/us_db_t.h"

//! Constructor
/*! 
	Constructor a basic class <var>US_DB_T</var>, 
	with <var>p</var> as a parent and <var>usdbconf.bin</var> as object name. 
*/ 
US_DB_T::US_DB_T(QObject *p, const char *name) : QObject(p, name)
{
	USglobal = new US_Config();
	DB_flag = false;
}
//!destructor
US_DB_T::~US_DB_T()
{
	delete USglobal;
	close_db();
}

/*! 
	Read DB file and make a connection with database.
	\return 0, No Error.\n
	\return -1, No DB file exists.\n
	\return -2, Database mode unvailable.\n
	\return -3, DB file info is incorrect.
*/
int US_DB_T::db_connect()
{
	QString dbfile = US_Config::get_home_dir() + USDB;
	QFile f( dbfile );

	if (f.exists())
	{
		f.open(IO_ReadOnly);
		QDataStream ds (&f);
		double num[64];
		int code[64];
		int i,length;
		
			ds >> length;
			for(i=0; i<length; i++)
			{
				ds >> num[i];
				code[i] = (int)(num[i]+0.5);
				login_list.dbname[i] = QChar(code[i]);
			}
			ds >> length;
			for(i=0; i<length; i++)
			{
				ds >> num[i];
				code[i] = (int)(num[i]+0.5);
				login_list.username[i] = QChar(code[i]);
			}
			ds >> length;
			for(i=0; i<length; i++)
			{
				ds >> num[i];
				code[i] = (int)(num[i]+0.5);
				login_list.password[i] = QChar(code[i]);
			}
			ds >> length;
			for(i=0; i<length; i++)
			{
				ds >> num[i];
				code[i] = (int)(num[i]+0.5);
				login_list.host[i] = QChar(code[i]);
			}
			ds >> length;
			for(i=0; i<length; i++)
			{
				ds >> num[i];
				code[i] = (int)(num[i]+0.5);
				login_list.driver[i] = QChar(code[i]);
			}
		f.close();				

	}
	else
	{															
		QString str;
		str = tr("The data file with your login info could not be found.\n"
					"Please configure you database preferences in:\n\n"
					"File:Configuration:Database Preferences\n\n"
					"before attempting to use the database functions..");
		emit newMessage(str, -1);								
		DB_flag = false;
		return -1;
	}
	defaultDB = QSqlDatabase::addDatabase(login_list.driver);
	defaultDB->setDatabaseName( login_list.dbname);
   defaultDB->setUserName(login_list.username);
   defaultDB->setPassword(login_list.password);
   defaultDB->setHostName(login_list.host);
   if (! defaultDB->open()) 
	{
   	QString str;
		str = tr("UltraScan was unable to initialize the database functionality of this module.\n"
					"Several conditions could cause this error. To avoid this error, please check\n"
					"the following items:\n\n"
					"1. The driver \"" + login_list.driver + "\" selected in your database setup\n"
					"(File:Configuration:Database Preferences) is not available on your system.\n"
					"To correct this, please select the correct setup in your Database Configuration\n"
					"before attempting to use the database functions (also check suggestion #3).\n\n"
					"2. The configured database is only available through the network and the network\n"
					"capability is currently not available. To correct this, connect your computer\n"
					"to the Internet and check firewall settings on both the client and server.\n\n"
					"3. Your QT library does not provide support for the driver selected in your\n"
					"setup (\"" + login_list.driver + "\"). To correct this, recompile your QT library\n"
					"with the proper SQL driver support or download a correctly configured version.\n\n" 
					"4. The selected database engine is not running or does not contain the selected\n"
					"database, or the database ports are blocked by a firewall.\n\n"
					"Some modules will work without database support and you can continue to run the\n"
					"program without database support. Error message produced by QT:\n\n"
					"Failed to open database: " + defaultDB->lastError().driverText() + "\n"
					+ defaultDB->lastError().databaseText());
		emit newMessage(str, -2);								
		cerr << str << "\n\nThe module exited with return code -2\n";
		DB_flag = false;
      return -2;
   }
	DB_flag = true;
	return 0;
}

// this overloaded function is provided to bypass the login information and allow 
// multiple MySQL databases to be used without additional authentication from the us_gridcontrol
// supercomputer interface 
int US_DB_T::db_connect(struct US_DatabaseLogin DB_Login)
{
	defaultDB = QSqlDatabase::addDatabase(DB_Login.driver);
	defaultDB->setDatabaseName(DB_Login.database);
   defaultDB->setUserName(DB_Login.user);
   defaultDB->setPassword(DB_Login.password);
   defaultDB->setHostName(DB_Login.host);
   if (! defaultDB->open()) 
	{
   	QString str;
		str = tr("UltraScan was unable to initialize the database functionality of this module. Several conditions could\n"
					"cause this error. To avoid this error, please check the following items:\n\n"
					"1. The driver \"" + login_list.driver + "\" selected in your database setup (File:Configuration:Database Preferences)\n"
					"is not available on your system. To correct this, please select the correct setup in your Database Configuration before\n"
					"attempting to use the database functions.\n\n"
					"2. The configured database is only available through the network and the network capability is currently\n"
					"not available. To correct this, connect your computer to the Internet.\n\n"
					"3. Your QT library does not provide support for the driver selected in your setup (\"" + login_list.driver + "\")\n\n"
					"To correct this, recompile your QT library with the proper SQL driver support or download a correctly configured version.\n\n" 
					"4. The selected database engine is not running or doesn't contain the selected database\n\n"
					"Some modules will work without database support and you can continue to run the program without database support.\n\n"
					"Failed to open database: " + defaultDB->lastError().driverText() + ", " + defaultDB->lastError().databaseText());
		emit newMessage(str, -2);								
		cerr << str << "\n\nThe module exited with return code -2\n";
		DB_flag = false;
      return -2;
   }
	DB_flag = true;
	return 0;
}

/*!
	Return DB table Max entry number+1 as new entry ID.
	If table is empty, return 1.
	\param table DB table name.
	\param key Table's primary key.
*/
int US_DB_T::get_newID(QString table, QString key)
{
	int i;
	QString str = "SELECT " + key + " FROM " + table;
	QSqlQuery query( str );
                       
	if ( query.isActive() ) 
	{
   	query.last();
		i = query.value(0).toInt();
		return (i+1);
	}
	else
	{
		return 1;
	}	
}

QString US_DB_T::make_tempFile(QString dirName, QString fileName)
{
	QDir temp_dir;
	//QString str = USglobal->config_list.html_dir + "/temp/";
	temp_dir.setPath(dirName);
	if(!temp_dir.exists())
	{
		temp_dir.mkdir(dirName);
	}
	QString path = dirName +"/"+ fileName;
	return path;
}

void US_DB_T::remove_temp_dir(QString dirName)
{
	QDir temp_dir;
	QString str = USglobal->config_list.html_dir + "/" + dirName;
	temp_dir.setPath(str);
	if(temp_dir.exists())
	{
		QStringList entries;
		temp_dir.setNameFilter("*.*");
		entries = temp_dir.entryList();
		for (QStringList::Iterator it = entries.begin(); it != entries.end(); ++it ) 
		{
				temp_dir.remove((*it).latin1());
		}
		temp_dir.cdUp();
		temp_dir.rmdir(dirName);		
	}
}

bool US_DB_T::read_blob(QString field, QSqlCursor cur, QString filename)
{
	bool flag = false;
	int byte;
	int size = cur.value(field).toByteArray().size();
//	cerr<<filename<<" : "<<size<<endl;
	if(size>0)
	{
		QByteArray da(size);
		da = cur.value(field).toByteArray();
		QFile fw(filename);
		fw.open(IO_WriteOnly);
		byte = fw.writeBlock(da.data(),size);
		fw.close();
	   flag = true;
	}
	return flag;
}

bool US_DB_T::write_blob(QString filename, QSqlRecord *buffer, QString fieldname)
{
	bool flag = false;
	QFile f(filename);
	if(f.exists())
	{
		f.open(IO_ReadOnly);
		unsigned int SIZE = f.size();
//		cerr<<filename<<" : "<<SIZE<<endl;
		if(SIZE>50000000)
		{
			emit newMessage(tr("The file's size is too large to store\n"), -3);
			return false;
		}
		QByteArray byteFile(f.size());
		f.readBlock(byteFile.data(), f.size());
		buffer->setValue(fieldname, byteFile);
		f.close();
		flag = true;
	}
	return flag;
}

void US_DB_T::clean_db(QString table, QString key, QString sub_table, QString sub_key, int value)
{
	QString str, str1;
	int wl[3];
	/*if(value>0)
	{
		QSqlCursor cur(table);
		str.sprintf(key + "=%d", value);
		cur.select(str);
		if(cur.next() && sub_table !="")
		{
			for(int i=0; i<3; i++)
			{	
				str1.sprintf(sub_key + "_WL%d", i+1);
				wl[i] = cur.value(str1).toInt();
				if(wl[i]>0)
				{
					QSqlCursor sub_cur(sub_table);
					sub_cur.setMode( QSqlCursor::Delete);
					QSqlIndex sub_filter = sub_cur.index(sub_key);
					sub_cur.setValue(sub_key, wl[i]);
					sub_cur.select(sub_filter);
					sub_cur.primeDelete();
					sub_cur.del();
				}
			}
		}
		cur.setMode( QSqlCursor::Delete);
		QSqlIndex filter = cur.index(key);
		cur.setValue(key, value);
		cur.select(filter);
		cur.primeDelete();
		cur.del();
	}
	*/
	for(int i=0; i<3; i++)
	{
		wl[i] = 0;
	}
	if(value>0)
	{
		QSqlCursor cur(table);
		str.sprintf(key + "=%d", value);
		cur.select(str);
		if(cur.next() && sub_table !="")
		{
			for(int i=0; i<3; i++)
			{	
				str1.sprintf(sub_key + "_WL%d", i+1);
				wl[i] = cur.value(str1).toInt();
			}
		}
		str.sprintf("DELETE FROM " + table + " WHERE " + key + "=%d;", value);
		cerr<<str<<endl;
		QSqlQuery del;
		del.exec(str);

		for(int i=0; i<3; i++)
		{	
			if(wl[i]>0)
			{
				str1.sprintf("DELETE FROM " + sub_table + " WHERE " + sub_key + "=%d;", wl[i]);
				cerr<<str1<<endl;
				del.exec(str1);
			}
		}	
	}
}

QString US_DB_T::show_investigator(int InvID)
{
	QString str, show, firstname, lastname;
	str.sprintf("SELECT FirstName, LastName FROM tblInvestigators WHERE InvID = %d",InvID);
	QSqlQuery query(str);
	if(query.isActive())
	{
		if(query.next())
		{
			firstname = query.value(0).toString();
			lastname = query.value(1).toString();
			show = "InvID (" + QString::number( InvID ) + "), "+ firstname + " "+ lastname;
		}
	}
	return show;
}

QString US_DB_T::show_buffer(int BuffID)
{
	QString str, show, description;
	str.sprintf("SELECT BufferDescription FROM tblBuffer WHERE BuffID = %d",BuffID);
	QSqlQuery query(str);
	if(query.isActive())
	{
		if(query.next())
		{
			description = query.value(0).toString();
			show = "BuffID (" + QString::number( BuffID ) + "), "+ description;
		}
	}
	return show;
}

QString US_DB_T::show_peptide(int PepID)
{
	QString str, show, description;
	str.sprintf("SELECT Description FROM tblPeptide WHERE PepID = %d",PepID);
	QSqlQuery query(str);
	if(query.isActive())
	{
		if(query.next())
		{
			description = query.value(0).toString();
			show = "PepID (" + QString::number( PepID ) + "), "+ description;
		}
	}
	return show;
}

QString US_DB_T::show_DNA(int DNAID)
{
	QString str, show, description;
	str.sprintf("SELECT Description FROM tblDNA WHERE DNAID = %d",DNAID);
	QSqlQuery query(str);
	if(query.isActive())
	{
		if(query.next())
		{
			description = query.value(0).toString();
			show = "DNAID (" + QString::number( DNAID ) + "), "+ description;
		}
	}
	return show;
}

QString US_DB_T::show_image(int ImageID)
{
	QString str, show, description;
	str.sprintf("SELECT Description FROM tblImage WHERE GelID = %d",ImageID);
	QSqlQuery query(str);
	if(query.isActive())
	{
		if(query.next())
		{
			description = query.value(0).toString();
			show = "ImageID (" + QString::number( ImageID ) + "), "+ description;
		}
	}
	return show;
}

QString US_DB_T::show_request(int RequestID)
{
	QString str, show, description;
	str.sprintf("SELECT ProjectDescription FROM tblRequest WHERE RequestID = %d",RequestID);
	QSqlQuery query(str);
	if(query.isActive())
	{
		if(query.next())
		{
			description = query.value(0).toString();
			show = "(" + QString::number( RequestID ) + "), "+ description;
		}
	}
	return show;
}

QString US_DB_T::show_sample(int SampleID)
{
	QString str, show, description;
	str.sprintf("SELECT Description FROM tblSample WHERE SampleID = %d",SampleID);
	QSqlQuery query(str);
	if(query.isActive())
	{
		if(query.next())
		{
			description = query.value(0).toString();
			show = "(" + QString::number( SampleID ) + "), "+ description;
		}
	}
	return show;
}

/*!
	Closes the database connection, freeing any resources acquired.
*/
void US_DB_T::close_db()
{	
	if(DB_flag)
	{
		if ( defaultDB->isOpen() )
		{
			defaultDB->close();
			DB_flag = false;
		}
	}	
}

