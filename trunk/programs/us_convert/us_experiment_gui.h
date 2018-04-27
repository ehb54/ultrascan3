//! \file us_convert/us_experiment_gui.h
#ifndef US_EXPERIMENT_GUI_H
#define US_EXPERIMENT_GUI_H

#include "us_widgets_dialog.h"
#include "us_widgets.h"
#include "us_extern.h"
#include "us_help.h"
#include "us_convert.h"
#include "us_experiment.h"
#include "us_selectbox.h"
#include "us_project.h"
#include "us_rotor.h"

/*! \class US_ExperimentGui
           This class provides the ability to associate raw data with
           the relevant experiment parameters, such as the lab,
           instrument, and rotor on which the experiment was run. 
*/
           
class US_ExperimentGui : public US_WidgetsDialog
{
   Q_OBJECT

   public:

      /*! \brief Generic constructor for the US_ExperimentGui class. To 
                 instantiate the class a calling function must
                 provide a structure to contain all the data.
          \param signal_wanted A boolean value indicating whether the caller
                         wants a signal to be emitted
          \param dataIn  A reference to a structure that contains
                         previously selected experiment data, if any.
          \param select_db_disk Indicates whether the default search is on
                         the local disk or in the DB
      */
      US_ExperimentGui( bool,
                        US_Experiment&,
                        int );

      //! A null destructor. 
      ~US_ExperimentGui() {};

   signals:

      /*! \brief The signal that is emitted when the user chooses
                 to accept the current choices. This information is
                 passed back to the calling function.

          \param expInfo A reference to a structure that contains all
                         the current experiment data
      */
      void updateExpInfoSelection( US_Experiment& expInfo );

      /*! \brief The signal that is emitted when the user chooses
                 to cancel the current selection. In this case all
                 previously-entered experiment parameter associations
                 are erased.
      */
      void cancelExpInfoSelection( void );

      //! A signal to indicate that the current disk/db selection has changed.
      //! /param DB True if DB is the new selection
      void use_db( bool DB );

   private:
      US_Experiment&         expInfo;
      QVector< US_Rotor::Lab > labList;
      int                    currentLab;
      int                    currentInstrument;
      bool                   lab_changed;
      bool                   signal;

      US_Disk_DB_Controls*   disk_controls; //!< Radiobuttons for disk/db choice

      US_Help                showHelp;

      QStringList            experimentTypes;
      QComboBox*             cb_expType;

      US_SelectBox*          cb_instrument;
      US_SelectBox*          cb_operator;
                          
      QLineEdit*             le_investigator;
      QLineEdit*             le_runID;
      QLineEdit*             le_project;
      QLineEdit*             le_runTemp;
      QLineEdit*             le_label;
      QLineEdit*             le_rotorDesc;
      QTextEdit*             te_comment;
                          
      QListWidget*           lw_rotorSpeeds;

      QPushButton*           pb_project;
      QPushButton*           pb_rotor;
      QPushButton*           pb_accept;

  private slots:
      void reset             ( void );
      bool load              ( void );
      void reload            ( void );
      void syncHardware      ( void );
      void selectInvestigator( void );
      void assignInvestigator( int  );
      void getInvestigatorInfo( void );
      void source_changed     ( bool );
      void update_disk_db     ( bool );
      void selectProject     ( void );
      void assignProject     ( US_Project& );
      void cancelProject     ( void );
      void saveLabel         ( void );
      QComboBox* us_expTypeComboBox         ( void );
      void setInstrumentList ( void );
      void setOperatorList   ( void );
      void change_instrument ( int  );
      void selectRotor       ( void );
      void assignRotor       ( US_Rotor::Rotor&, US_Rotor::RotorCalibration& );
      void cancelRotor       ( void );
      void accept            ( void );
      void cancel            ( void );
      void connect_error     ( const QString& );
      void help              ( void )
        { showHelp.show_help( "manual/convert-experiment.html" ); };
};
#endif
