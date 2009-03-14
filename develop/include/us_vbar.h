#ifndef US_VBAR_H
#define US_VBAR_H
#include "us_util.h"
#include "us_math.h"
#include "us_db_t.h"



//! A Struct for storing required info for peptide data in DB.
/*!
	Each element is according to an entry field in Database Table tblPeptide.
*/
struct US_VbarData
{
	int PepID;						/*!< An integer variable for tblPeptide entry. */
	QString PepFileName;			/*!< A string stored the value of Peptide filename.*/
	QString Description;			/*!< A string variable for peptide description. */
	QString Sequence;				/*!< A string variable for peptide sequence. */
	float vbar;						/*!< A floating point variable for the vbar. */
	float e280;						/*!< A floating point variable for the molar extinction. */
	int InvID;						/*!< An integer variable for tblInvestigators reference id. */
};

//! A Struct for exporting Peptide Data.
/*! It contains vbar, vbar20 and e280. */
struct peptideDetails
{
	int pepID;						/*!< An integer variable for tblPeptide entry. */
	float vbar;						/*!< A float variable. */
	float vbar20;					/*!< A float variable. */
	float e280;						/*!< A float variable. */
	QString sequence;				/*!< QString Peptide Sequence. */
	QString description;			/*!< A string variable for peptide description. */
};


class US_EXTERN US_Vbar : public US_DB_T
{
	Q_OBJECT
	public:
		US_Vbar(const float, int, QObject *parent=0, const char *name=0);
		US_Vbar(const float, QObject *parent=0, const char *name=0);
		~US_Vbar();
		US_DB_T *terminalDB;						/*!< An object to access the database in terminal mode.*/

		bool sequence_loaded;
		US_Config *USglobal;
		QString filename, res_file, description;
		struct US_VbarData vbar_info;			/*!< A struct US_VbarData for storing Peptide Data. */
		struct peptide pep;
		float temperature;

#ifdef WIN32
  #pragma warning( disable: 4251 )
#endif

		vector <int> item_PepID;							/*!< A Integer Array to store Peptide ID.*/
		vector <QString> item_Description;	/*!< A String Array to store Peptide description.*/
		vector <QString> display_Str;			/*!< A String Array to store peptide name dispaly.*/

#ifdef WIN32
      #pragma warning( default: 4251 )
#endif

	public slots:
		bool read_file(const QString &);
		bool result_output(const QString &);
		bool read_db();
		void select_vbar(int);
		bool retrieve_vbar(int);
		void setInvestigator(const int);
		struct peptideDetails export_vbar(int);
		struct peptideDetails export_DNA_vbar(int);

	signals:
		void valueChanged(float, float);
		void e280Changed(float);
/*!
	This signal is emitted whenever the PepID value is changed.\n
	The argument is integer <var>PepID</var>.
*/
		void idChanged(int PepID);
};

#endif

