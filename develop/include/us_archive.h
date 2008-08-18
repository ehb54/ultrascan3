#ifndef US_ARCHIVE_H
#define US_ARCHIVE_H

#include <qlabel.h>
#include <qstringlist.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qtextedit.h>

#include "us_util.h"
#include "us_widgets.h"

class US_EXTERN US_Archive : public US_Widgets
{
	Q_OBJECT

	public:	
		US_Archive   ( QWidget* p = 0, const char* name = 0 );
		~US_Archive  ( void ){};

	private:
		int          run_type;
		bool         ultrascan_flag;
		bool         reports_flag;
		bool         delete_flag;

		QString      filename;
		QString      projectName;
		
		QTextEdit*   mle;

		QPushButton* pb_view;
		QPushButton* pb_create;
		QPushButton* pb_extract;
		QPushButton* pb_select_equildata_create;
		QPushButton* pb_select_velocdata_create;
		QPushButton* pb_select_montecarlo_create;
		QPushButton* pb_select_eqproject_create;
		QPushButton* pb_help;
		QPushButton* pb_close;
		
		QLabel*      lbl_name1;
		QLabel*      lbl_name2;
		QLabel*      lbl_create_name1;
		QLabel*      lbl_create_name2;
		QLabel*      lbl_ultrascan;
		QLabel*      lbl_reports;
		QLabel*      lbl_delete;

		QLabel*      banner1;
		QLabel*      banner2;
		QLabel*      banner3;
		QLabel*      banner4;

		QCheckBox*   cb_ultrascan;
		QCheckBox*   cb_reports;
		QCheckBox*   cb_delete;
		
  private slots:
		void view                  ( void );
		void viewtargz             ( const QString& );
		void extract_archive       ( void );
		void extract               ( const QString&, const QString&, const QString&,
												         const bool = false );
                               
		void quit                  ( void );
		void help                  ( void );
                               
		void set_ultrascan         ( void );
		void set_delete            ( void );
		void set_reports           ( void );
		
		void select_velocdata      ( void );
		void select_equildata      ( void );
		void select_eqilproj       ( void );
		void select_montecarlo     ( void );
		void select_create_archive ( const int );
		void create_archive        ( QStringList* = 0 );
		void move_file             ( const QString&, const QString& );
		bool create_tar            ( const QString&, const QString&, QStringList& );
		
		void clean_temp_dir        ( void );
		void disable_buttons       ( void );
		void enable_buttons        ( void );
                               
	protected slots:             
		void setup_GUI             ( void );
		void closeEvent            ( QCloseEvent* );
};

#endif

