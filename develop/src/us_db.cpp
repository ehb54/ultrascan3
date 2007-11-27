#include "../include/us_db.h"

//! Constructor
/*! 
	Constractor a basic class <var>US_DB</var>, 
	with <var>p</var> as a parent and <var>us_db</var> as object name. 
*/ 
US_DB::US_DB(QWidget *p, const char *name) :QDialog(p, name)
{
	USglobal = new US_Config();
	returnval = 0;
	errorMessage = "";
	terminalDB = new US_DB_T();
	connect(terminalDB, SIGNAL (newMessage(QString, int)), this, SLOT(newMessage(QString, int)));
	db_connect();

}
//!deconstructor
US_DB::~US_DB()
{
}

/*! 
	Read DB file and make a connect with database.
	\return 0, No Error.\n
	\return -1, No DB file exists.\n
	\return -2, Database mode unvailable.\n
	\return -3, DB file info is incorrect.
*/
int US_DB::db_connect()
{
	returnval = terminalDB->db_connect();
	DB_flag = terminalDB->DB_flag;
	login_list.driver = terminalDB->login_list.driver;
	login_list.username = terminalDB->login_list.username;
	login_list.password = terminalDB->login_list.password;
	login_list.dbname = terminalDB->login_list.dbname;
	login_list.host = terminalDB->login_list.host;
	login_list.description = terminalDB->login_list.description;
	return returnval;
}

// this overloaded function is provided to bypass the login information and allow 
// multiple MySQL databases to be used without additional authentication from the us_gridcontrol
// supercomputer interface 
int US_DB::db_connect(struct US_DatabaseLogin DB_Login)
{
	returnval = terminalDB->db_connect(DB_Login);
	DB_flag = terminalDB->DB_flag;
	login_list.driver = terminalDB->login_list.driver;
	login_list.username = terminalDB->login_list.username;
	login_list.password = terminalDB->login_list.password;
	login_list.dbname = terminalDB->login_list.dbname;
	login_list.host = terminalDB->login_list.host;
	login_list.description = terminalDB->login_list.description;
	return returnval;
}

void US_DB::newMessage(QString str, int val)
{
	returnval = val;
	QMessageBox::warning(this, tr("UltraScan Warning"), str,
	QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
}

/*!
	Return DB table Max entry number+1 as new entry ID.
	If table is empty, return 1.
	\param table DB table name.
	\param key Table's primary key.
*/
int US_DB::get_newID(QString table, QString key)
{
	return terminalDB->get_newID(table, key);
}

QString US_DB::make_tempFile(QString dirName, QString fileName)
{
	return terminalDB->make_tempFile(dirName, fileName);
}

void US_DB::remove_temp_dir(QString dirName)
{
	terminalDB->remove_temp_dir(dirName);
}

bool US_DB::read_blob(QString field, QSqlCursor cur, QString filename)
{
	return terminalDB->read_blob(field, cur, filename);
}

bool US_DB::write_blob(QString filename, QSqlRecord *buffer, QString fieldname)
{
	return terminalDB->write_blob(filename, buffer, fieldname);
}

void US_DB::clean_db(QString table, QString key, QString sub_table, QString sub_key, int value)
{
	terminalDB->clean_db(table, key, sub_table, sub_key, value);
}

QString US_DB::show_investigator(int InvID)
{
	return terminalDB->show_investigator(InvID);
}

QString US_DB::show_buffer(int BuffID)
{
	return terminalDB->show_buffer(BuffID);
}

QString US_DB::show_peptide(int PepID)
{
	return terminalDB->show_peptide(PepID);
}

QString US_DB::show_DNA(int DNAID)
{
	return terminalDB->show_DNA(DNAID);
}

QString US_DB::show_image(int ImageID)
{
	return terminalDB->show_image(ImageID);
}

QString US_DB::show_request(int RequestID)
{
	return terminalDB->show_request(RequestID);
}

QString US_DB::show_sample(int SampleID)
{
	return terminalDB->show_sample(SampleID);
}

/*!
	Closes the database connection, freeing any resources acquired.
*/
void US_DB::close_db()
{	
	terminalDB->close_db();
	DB_flag = terminalDB->DB_flag;
}

