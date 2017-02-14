#ifndef US_EXPERIMENT_H
#define US_EXPERIMENT_H

#include <QApplication>

#include "us_project_gui.h"
#include "us_editor.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_investigator.h"
#include "us_util.h"
#include "us_rotor.h"
#include "us_solution.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_hardware.h"
#include "us_select_runs.h"
#include "us_license_t.h"
#include "us_license.h"

class US_ExperGuiGeneral : public US_Widgets 
{
   Q_OBJECT

   public:

      US_ExperGuiGeneral( QWidget* );
      ~US_ExperGuiGeneral() {};

      void        initPanel( void );
      void        setPValue( const QString, QString& );
      void        setPValue( const QString, QStringList& );
      QString     getPValue( const QString );
      QStringList getPList ( const QString );
      QString     sibPValue( const QString, const QString );
      QStringList sibPList ( const QString, const QString );
      QString     status   ( void );
      void        help     ( void )
         { showHelp.show_help( "manual/experiment_general.html" ); };

      QStringList cpNames  ( void );
      bool        cpInfo   ( const QString, US_AbstractCenterpiece& );

   private:
      QWidget* mainw;
      US_Help  showHelp;

      US_Disk_DB_Controls*  disk_controls; //!< Radiobuttons for disk/db choice

      QLineEdit* le_runid;
      QLineEdit* le_project;
      QLineEdit* le_investigator;

      QList< US_AbstractCenterpiece >  acp_list;  // Full Centerpiece information
      QStringList                      cp_names;  // List of Centerpiece names
      int        dbg_level;

   private slots:

      void sel_project     ( void );
      void project_info    ( US_Project& );
      void sel_investigator( void );
      void centerpieceInfo ( void );

};

class US_ExperGuiRotor : public US_Widgets 
{
   Q_OBJECT

   public:

      US_ExperGuiRotor( QWidget* );
      ~US_ExperGuiRotor() {};

      void        initPanel( void );
      void        setPValue( const QString, QString& );
      void        setPValue( const QString, QStringList& );
      QString     getPValue( const QString );
      QStringList getPList ( const QString );
      QString     sibPValue( const QString, const QString );
      QStringList sibPList ( const QString, const QString );
      QString     status   ( void );
      void        help     ( void )
         { showHelp.show_help( "manual/experiment_rotor.html" ); };

   private:

      QWidget* mainw;
      US_Help  showHelp;
      QComboBox* cb_lab;
      QComboBox* cb_rotor;
      QComboBox* cb_calibr;

      QVector< US_Rotor::Lab >               labs;
      QVector< US_Rotor::Rotor >             rotors;
      QVector< US_Rotor::AbstractRotor >     arotors;
      QVector< US_Rotor::RotorCalibration >  calibs;

      QStringList sl_labs;
      QStringList sl_rotors;
      QStringList sl_arotors;
      QStringList sl_calibs;
      int         dbg_level;

   private slots:

      void changeLab  ( int );
      void changeRotor( int );
      void advRotor   ( void );
      void advRotorChanged( US_Rotor::Rotor&,
                            US_Rotor::RotorCalibration& );
};

class US_ExperGuiSpeeds : public US_Widgets 
{
   Q_OBJECT

   public:

      US_ExperGuiSpeeds( QWidget* );
      ~US_ExperGuiSpeeds() {};

      void        initPanel( void );
      void        setPValue( const QString, QString& );
      void        setPValue( const QString, QStringList& );
      QString     getPValue( const QString );
      QStringList getPList ( const QString );
      QString     sibPValue( const QString, const QString );
      QStringList sibPList ( const QString, const QString );
      QString     status   ( void );
      void        help     ( void )
         { showHelp.show_help( "manual/experiment_speeds.html" ); };

   private:

      QWidget*     mainw;
      US_Help      showHelp;

      QComboBox*   cb_prof;
      QwtCounter*  ct_speed;
      QwtCounter*  ct_accel;
      QwtCounter*  ct_count;
      QwtCounter*  ct_lenhr;
      QwtCounter*  ct_lenmin;
      QwtCounter*  ct_dlyhr;
      QwtCounter*  ct_dlymin;

      QVector< QString >  profdesc;
      QVector< double >   ssvals;

      int          dbg_level;
      int          nspeed;
      int          curssx;

   private slots:

      QString speedp_description( const int );
      void    ssChangeCount( double );
      void    ssChangeProfx( int    );
      void    ssChangeSpeed( double );
      void    ssChangeAccel( double );
      void    ssChangeDurhr( double );
      void    ssChangeDurmn( double );
      void    ssChangeDlyhr( double );
      void    ssChangeDlymn( double );
};

class US_ExperGuiCells : public US_Widgets 
{
   Q_OBJECT

   public:

      US_ExperGuiCells( QWidget* );
      ~US_ExperGuiCells() {};

      void        initPanel( void );
      void        setPValue( const QString, QString& );
      void        setPValue( const QString, QStringList& );
      QString     getPValue( const QString );
      QStringList getPList ( const QString );
      QString     sibPValue( const QString, const QString );
      QStringList sibPList ( const QString, const QString );
      QString     status   ( void );
      void        help     ( void )
         { showHelp.show_help( "manual/experiment_cells.html" ); };

   private:
      QWidget* mainw;
      US_Help  showHelp;
      QList< QLabel* >     cc_labls;  // Cell label GUI objects
      QList< QComboBox* >  cc_cenps;  // Centerpiece GUI objects
      QList< QComboBox* >  cc_winds;  // Windows GUI objects
      QStringList          cpnames;   // Centerpiece names
      int          dbg_level;

   private slots:
      void centerpieceChanged( int );
      void windowsChanged    ( int );

};

class US_ExperGuiSolutions : public US_Widgets 
{
   Q_OBJECT

   public:

      US_ExperGuiSolutions( QWidget* );
      ~US_ExperGuiSolutions() {};

      void        initPanel( void );
      void        setPValue( const QString, QString& );
      void        setPValue( const QString, QStringList& );
      QString     getPValue( const QString );
      QStringList getPList ( const QString );
      QString     sibPValue( const QString, const QString );
      QStringList sibPList ( const QString, const QString );
      QString     status   ( void );
      void        help     ( void )
         { showHelp.show_help( "manual/experiment_solutions.html" ); };

   private:

      QWidget* mainw;
      US_Help  showHelp;
      int          dbg_level;

      QVector< QLabel* >     cc_labls;
      QVector< QComboBox* >  cc_solus;
      QStringList            sonames;

      QMap< QString, QString >      solu_ids;
      QMap< QString, US_Solution >  solu_data;

   private slots:

      void manageSolutions( void );
      void detailSolutions( void );
      bool solutionID     ( const QString, QString& );
      bool solutionData   ( const QString, US_Solution& );
      int  allSolutions   ( void );
};


class US_ExperGuiPhotoMult : public US_Widgets 
{
   Q_OBJECT

   public:

      US_ExperGuiPhotoMult( QWidget* );
      ~US_ExperGuiPhotoMult() {};

      void        initPanel( void );
      void        setPValue( const QString, QString& );
      void        setPValue( const QString, QStringList& );
      QString     getPValue( const QString );
      QStringList getPList ( const QString );
      QString     sibPValue( const QString, const QString );
      QStringList sibPList ( const QString, const QString );
      QString     status   ( void );
      void        help     ( void )
         { showHelp.show_help( "manual/experiment_photomult.html" ); };

   private:

      QWidget* mainw;
      US_Help  showHelp;
      int          dbg_level;
};

class US_ExperGuiUpload : public US_Widgets 
{
   Q_OBJECT

   public:

      US_ExperGuiUpload( QWidget* );
      ~US_ExperGuiUpload() {};

      void        initPanel( void );
      void        setPValue( const QString, QString& );
      void        setPValue( const QString, QStringList& );
      QString     getPValue( const QString );
      QStringList getPList ( const QString );
      QString     sibPValue( const QString, const QString );
      QStringList sibPList ( const QString, const QString );
      QString     status   ( void );
      void        help     ( void )
         { showHelp.show_help( "manual/experiment_upload.html" ); };

   private:

      QWidget* mainw;
      US_Help  showHelp;
      int          dbg_level;

};

class US_Experiment : public US_Widgets 
{
   Q_OBJECT

   public:
      US_Experiment();

      QString     childPValue( const QString, const QString );
      QStringList childPList ( const QString, const QString );

   private:

      QTabWidget* tabWidget;

      US_ExperGuiGeneral*   epanGeneral;
      US_ExperGuiRotor*     epanRotor;
      US_ExperGuiSpeeds*    epanSpeeds;
      US_ExperGuiCells*     epanCells;
      US_ExperGuiSolutions* epanSolutions;
      US_ExperGuiPhotoMult* epanPhotoMult;
      US_ExperGuiUpload*    epanUpload;

      QLineEdit*  le_stat;

      int         dbg_level;
      int         curr_panx;

   private slots:

      void reset     ( void );
      void newPanel  ( int  );
      void statUpdate( void );
      void panelUp   ( void );
      void panelDown ( void );
      void help      ( void );
};
#endif

