#ifndef US_BUFFER_H
#define US_BUFFER_H

#include "us_util.h"
#include "us_db_t.h"


//! A struct for storing the buffer template file in $ULTRASCAN/etc/buffer.dat
/*!
	Each element contains a component from the buffer.dat file.
*/
struct BufferIngredient
{
	QString name;					/*!<A string for buffer component's name. */
	QString unit;					/*!<A string for buffer component value's unit. */
	float dens_coeff[6];			/*!<A float array  for density coefficient. */
	float visc_coeff[6];			/*!<A float array for viscosity coefficient. */
	QString range;					/*!<A string for buffer component's range. */
	float partial_concentration;	/*!< A float variable for the partial concentration. */
};


//! A struct for exporting Buffer Data.

struct BufferData
{
	vector <struct BufferIngredient>
	component;								/*!< A vector of ingredient structure. */
	float refractive_index;				/*!< A float variable for the refractive index of the buffer. */
	float density;							/*!< A float variable for the density of the buffer. */
	float viscosity;						/*!< A float variable for the viscosity of the buffer. */
	QString description;					/*!< A string variable for the buffer description. */
	QString data;							/*!< A string variable for the buffer component field in the database. */
	int bufferID;							/*!< A integer variable to hold the buffer's DB ID, or -1 if from harddrive. */
	int investigatorID;					/*!< A integer variable to hold the investigator's ID of this buffer, or -1 if from harddrive. */
};

//! An Interface for Buffer data with Database.
/*!
	This interface can be found in "Database->Commit Data to DB->Buffer".
	You can use this interface to pickup your buffer values.
	When you Save Buffer to HD, the program will save your buffer data to hard drive.
	When you Backup Buffer to DB, the program will save your buffer data to database table: tblBuffer.
	Also you can use this interface to load your buffer data records from hard drive or DB.
*/
class US_EXTERN US_Buffer : public US_DB_T
{
	Q_OBJECT
	public:
		US_Buffer(int temp_invID, QObject *parent=0, const char *name="us_buffer_db");
		US_Buffer(QObject *parent=0, const char *name="us_buffer_db");
		~US_Buffer();

		struct BufferData Buffer;		/*!< A BufferData structure for the currently active Buffer Data. */
		vector <struct BufferIngredient> component_list;	/*!< A BufferIngredient vector structure for all components in template list (stored in $ULTRASCAN/etc/buffer.dat)). */

	public slots:

		struct BufferData export_buffer(int id);
		struct BufferData read_buffer(QString);
		void buf_init();
		bool read_template_file();
		void setInvestigator(const int);
		void recalc_density();
		void recalc_viscosity();

	signals:
/*!
	This signal is emitted in function add_component(), reset() and accept().\n
	The argument is float variable <var>density</var> and <var>viscosity</var>.
*/
		void valueChanged(float density, float viscosity, float refractive_index);
		void valueChanged(float density, float viscosity);
};

#endif

