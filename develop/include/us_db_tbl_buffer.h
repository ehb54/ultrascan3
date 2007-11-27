#ifndef US_DB_TBL_BUFFER_H
#define US_DB_TBL_BUFFER_H

#include <qsqldatabase.h>
#include <qsqlquery.h>
#include <qsqlcursor.h>
#include <qsqlfield.h>
#include <qmemarray.h>
#include <qiodevice.h>
#include <math.h>
#include <qlistbox.h>
#include <qfiledlg.h>

#include "us_editor.h"
#include "us_buffer.h"
#include "us_db.h"
#include "us_math.h"
#include "us_db_tbl_investigator.h"

//! An Interface for Buffer data with Database.
/*!
	This interface can be found in "Database->Commit Data to DB->Buffer".
	You can use this interface to pickup your buffer values.
	When you Save Buffer to HD, the program will save your buffer data to hard drive.
	When you Backup Buffer to DB, the program will save your buffer data to database table: tblBuffer.
	Also you can use this interface to load your buffer data records from hard drive or DB.
*/
class US_EXTERN US_Buffer_DB : public US_DB
{
	Q_OBJECT
	public:
		US_Buffer_DB(bool from_cell, int temp_invID, QWidget *parent=0, const char *name="us_buffer_db");
		~US_Buffer_DB();

		struct BufferData Buffer;		/*!< A BufferData structure for the currently active Buffer Data. */
		vector <struct BufferData> db_list;		/*!< A vector of a BufferData structure for the contents of the database. */
		vector <struct BufferIngredient> component_list;	/*!< A BufferIngredient vector structure for all components in template list (stored in $ULTRASCAN/etc/buffer.dat)). */
		float partial_concentration;			/*!< A variable to keep track of the current partial concentration. */
		bool cell_flag;							/*!< A flag to indicate if the class is called from the us_db_tbl_cell module. */
		QPushButton *pb_load;					/*!< A PushButton connect to read_buffer(). */
		QPushButton *pb_save;					/*!< A PushButton connect to save_buffer(). */
		QPushButton *pb_load_db;				/*!< A PushButton connect to read_db(). */
		QPushButton *pb_save_db;				/*!< A PushButton connect to save_db(). */
		QPushButton *pb_del_db;					/*!< A PushButton connect to del_db(). */
		QPushButton *pb_update_db;				/*!< A PushButton connect to update_db(). */
		QPushButton *pb_help;					/*!< A PushButton connect to help(). */
		QPushButton *pb_accept;					/*!< A PushButton connect to accept(). */
		QPushButton *pb_reset;					/*!< A PushButton connect to clear(). */
		QPushButton	*pb_investigator;			/*!< A PushButton connect to sel_investigator(). */
		QListBox *lb_buffer_db;					/*!< A ListBox containing a list of database buffer entries. */
		QListBox *lb_ingredients;				/*!< A ListBox containing all buffer components. */
		QListBox *lb_current_buffer;			/*!< A ListBox showing the currently selected buffer omponents. */
		QLabel *lbl_investigator;				/*!< A Label shows selected investigator Information. */
		QLabel *lbl_density;						/*!< A Label shows 'Density (20ºC)'. */
		QLineEdit *le_density;					/*!< A lineedit to enter or show the buffer's density value, default set to 0.9982g/ccm. */
		QLabel *lbl_viscosity;					/*!< A Label shows 'Viscosity (20ºC)'. */
		QLineEdit *le_viscosity;				/*!< A lineedit to enter or show the viscosity value, default set to 1.002cp. */
		QLabel *lbl_refractive_index;			/*!< A Label showing 'Refractive Index (20ºC)'. */
		QLineEdit *le_refractive_index;		/*!< A LineEdit for entering the refractive index value, default set to 0.0. */
		QLabel *lbl_description;				/*!< A Label shows 'Buffer Description:'. */
		QLabel *lbl_buffer1;						/*!< A Label shows 'Please select a Buffer Component:'. */
		QLabel *lbl_buffer2;						/*!< A Label shows selected buffer component. */
		QLabel *lbl_unit;							/*!< A Label shows buffer component's unit (ex: mM). */
		QLabel *lbl_banner1;						/*!< A Label shows 'Doubleclick on buffer data to select:'. */
		QLabel *lbl_banner2;						/*!< A Label shows 'Click on item to select'. */
		QLabel *lbl_banner3;						/*!< A Label shows 'Doubleclick on item to remove'. */
		QLineEdit *le_concentration;			/*!< A LineEdit to enter the concentration of a buffer component */
		QLineEdit *le_description;				/*!< A LineEdit for inputting the buffer's description. */

	public slots:

		bool get_buffer(int);

	protected slots:

		void buf_init();
		bool read_template_file();
		void setInvestigator(const int);
		void recalc_density();
		void recalc_viscosity();
		void setup_GUI();
	//pushbuttons:
		void read_buffer();
		void save_buffer();
		void read_db();
		void save_db();
		void check_permission();
		void del_db(bool);
		void help();
		void reset();
		void accept();
		void update_db();
		void sel_investigator();
		void update_investigator_lbl(QString, int );
	//ListBoxs:
		void select_buff(int item);
		void add_component();
		void remove_component(int);
		void list_component(int);
		void update_concentration(const QString &);
		void update_description(const QString &);
		void update_density(const QString &);
		void update_viscosity(const QString &);
		void update_refractive_index(const QString &);
	//other:
	//	void resizeEvent(QResizeEvent *e);
		void closeEvent(QCloseEvent *e);

	signals:
/*!
	This signal is emitted whenever the widget is called by US_Cell_DB.\n
	The argument is integer <var>BuffID</var>.
*/
		void IDChanged(int BuffID);
/*!
	This signal is emitted in function add_component(), reset() and accept().\n
	The argument is float variable <var>density</var> and <var>viscosity</var>.
*/
		void valueChanged(float density, float viscosity, float refractive_index);
		void valueChanged(float density, float viscosity);
};

#endif

