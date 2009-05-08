#ifndef US_DB_T_H
#define US_DB_T_H

#include <qsqldatabase.h>
#include <qsqlquery.h>
#include <qsqlcursor.h>
#include <qapplication.h>
#include   <qlayout.h>

#include "us_database.h"
//#include "us_db_admin.h"


//! A Struct for storing DB login info.
/*! 
   The basic information needed to log into a database
*/
struct US_DatabaseLogin
{
   QString driver, user, database, password, host;
};

//! A Struct for storing required info for Experimental Data in DB.
/*! 
   Each element is according to an entry field in Database Table tblExpData.
*/
struct US_ExpData
{
   int ExpdataID;            /*!< Table ID of experimental data. */
   QString Path;            /*!< From where to load the data. */
   int Invid;               /*!< Table ID of investigator. */
   int Edit_type;            /*!< Edit type as defined in US_Editdb */
   QString Date;            /*!< Date of experiment. */
   QString Runid;            /*!< Name of run (ex: 102v). */
   QString Cell[8];         /*!< Cell contents (1-8), cell's name. */
   int CellID[8];            /*!< Cell(1-8), cell table ID. */
   int cell_position;      /*!< cell 1-8; */
   float Temperature;      /*!< Average temperature. */
   float Duration;         /*!< Length of experiments (seconds). */
   int Rotor;               /*!< Rotor serial number. */
   QString Description;      /*!< Description of experiment. */
   int wavelength[8][3];   /*!< Wavelength 1-3 of cell. */
   int scans[8][3];         /*!< How many scans for each wavelength. */
   int centerpiece[8];      /*!< Which centerpiece was used (serial number). */
};

//! A Struct for storing required info for ExpData Result in DB.
/*! 
   Each element is according to an entry field in Database Table ExpResult.
*/
struct US_ExpResult
{
   int expRstID;            /*!< ExpdataID. */
   int invID;               /*!< Investigator ID. */
   QString date;            /*!< Date of experiment. */
   QString description;      /*!< Description of experiment. */
   int CellID[8];            /*!< Cell(1-8), cell table ID. */
   QString Cell[8];         /*!< Cell(1-8). cell description. */
   int centerpiece[8];      /*!< Centerpiece for Cell(1-8) */
   int EDid[8][4];
   int VHWid[8][4];
   int FEid[8][4];
   int CSid[8][4];
   int SMid[8][4];
   int TDid[8][4];
   int RDid[8][4];
   unsigned int wavelength[8][3];      // up to 3 wavelengths per cell
   unsigned int scans[8][3];            // how many scans are there for each lambda in each cell?
};

//! A basic class for Database implement.
/*!
   This basic class build basic functions for inherited class using, like database connect and disconnect. 
*/
class US_EXTERN US_DB_T : public QObject
{
   Q_OBJECT
   
   public:
      US_DB_T (QObject *p=0, const char *name="us_db");
      ~US_DB_T();
      
      struct US_LoginData login_list;      /*!< A struct US_LoginData for storing DB login information. */
      US_Config *USglobal;            /*!< A reference of US_Config. */   
      QSqlDatabase *defaultDB;            /*!< A reference of QSqlDatabase. */
      bool DB_flag;                        /*!< A bool variable for choosing use database or not. */
      int db_connect();
      int db_connect(struct US_DatabaseLogin);
      int get_newID(QString table, QString key);
      QString make_tempFile(QString, QString);
      void remove_temp_dir(QString);
      bool read_blob(QString, QSqlCursor, QString);
      bool write_blob(QString, QSqlRecord *, QString);
      void clean_db(QString, QString, QString, QString, int);
      QString show_investigator(int);
      QString show_buffer(int);
      QString show_peptide(int);
      QString show_DNA(int);
      QString show_image(int);
      QString show_request(int);
      QString show_sample(int);
      void close_db();
      
   signals:
   
      void newMessage(QString, int); // error message, return code
};

#endif


