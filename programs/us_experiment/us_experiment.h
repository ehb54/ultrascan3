#ifndef US_EXPERIMENT_H
#define US_EXPERIMENT_H

#include <QApplication>

#include "us_project_gui.h"
#include "us_editor.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_investigator.h"
#include "us_util.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_select_runs.h"
#include "us_license_t.h"
#include "us_license.h"

class US_ExperGuiGeneral : public US_Widgets 
{
	Q_OBJECT

	public:

      US_ExperGuiGeneral();
      ~US_ExperGuiGeneral() {};

      void setPValue   ( const QString, QString& );
      QString getPValue( const QString );
      QString status   ( void );

   private:

      US_Disk_DB_Controls*  disk_controls; //!< Radiobuttons for disk/db choice

      QLineEdit* le_runid;
      QLineEdit* le_project;
      QLineEdit* le_investigator;

   private slots:

      void sel_project     ( void );
      void project_info    ( US_Project& );
      void sel_investigator( void );

};

class US_ExperGuiRotor : public US_Widgets 
{
	Q_OBJECT

	public:

      US_ExperGuiRotor();
      ~US_ExperGuiRotor() {};

      void setPValue   ( const QString, QString& );
      void setPValue   ( const QString, QStringList& );
      QString getPValue( const QString );
      QString status   ( void );

   private:

      QComboBox* cb_lab;
      QComboBox* cb_rotor;
      QComboBox* cb_calibr;
};

class US_ExperGuiSpeeds : public US_Widgets 
{
	Q_OBJECT

	public:

      US_ExperGuiSpeeds();
      ~US_ExperGuiSpeeds() {};

      void setPValue   ( const QString, QString& );
      QString getPValue( const QString );
      QString status   ( void );

};

class US_ExperGuiCells : public US_Widgets 
{
	Q_OBJECT

	public:

      US_ExperGuiCells();
      ~US_ExperGuiCells() {};

      void setPValue   ( const QString, QString& );
      QString getPValue( const QString );
      QString status   ( void );

};

class US_ExperGuiSolutions : public US_Widgets 
{
	Q_OBJECT

	public:

      US_ExperGuiSolutions();
      ~US_ExperGuiSolutions() {};

      void setPValue   ( const QString, QString& );
      QString getPValue( const QString );
      QString status   ( void );

};

class US_ExperGuiPhotoMult : public US_Widgets 
{
	Q_OBJECT

	public:

      US_ExperGuiPhotoMult();
      ~US_ExperGuiPhotoMult() {};

      void setPValue   ( const QString, QString& );
      QString getPValue( const QString );
      QString status   ( void );

};

class US_ExperGuiUpload : public US_Widgets 
{
	Q_OBJECT

	public:

      US_ExperGuiUpload();
      ~US_ExperGuiUpload() {};

      void setPValue   ( const QString, QString& );
      QString getPValue( const QString );
      QString status   ( void );

};

class US_Experiment : public US_Widgets 
{
	Q_OBJECT

	public:
      US_Experiment();


	private:

      QTabWidget*           tabWidget;

      US_ExperGuiGeneral*   epanGeneral;
      US_ExperGuiRotor*     epanRotor;
      US_ExperGuiSpeeds*    epanSpeeds;
      US_ExperGuiCells*     epanCells;
      US_ExperGuiSolutions* epanSolutions;
      US_ExperGuiPhotoMult* epanPhotoMult;
      US_ExperGuiUpload*    epanUpload;

      US_Help               showHelp;

      QLineEdit*            le_stat;

      int                   dbg_level;

private slots:

   void reset     ( void );
   void newPanel  ( int  );
   void statUpdate( void );
   void panelUp   ( void );
   void panelDown ( void );

	void help                 ( void )
      { showHelp.show_help( "manual/us_experiment.html" ); };
};
#endif

