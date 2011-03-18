//! \file us_experiment_gui.h
#ifndef US_EXPERIMENT_GUI_H
#define US_EXPERIMENT_GUI_H

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_help.h"
#include "us_experiment.h"
#include "us_selectbox.h"
#include "us_project.h"
#include "us_rotor.h"

/*! \class US_ExperimentGui
           This class provides the ability to associate raw data with
           the relevant experiment parameters, such as the lab,
           instrument, and rotor on which the experiment was run. 
*/
           
class US_EXTERN US_ExperimentGui : public US_WidgetsDialog
{
   Q_OBJECT

   public:

      /*! \brief Generic constructor for the US_ExperimentGui class. To 
                 instantiate the class a calling function must
                 provide a structure to contain all the data.
          \param dataIn  A reference to a structure that contains
                         previously selected experiment data, if any.
      */
      US_ExperimentGui( US_Experiment& );

      //! A null destructor. 
      ~US_ExperimentGui() {};

   signals:

      /*! \brief The signal that is emitted when the user chooses
                 to accept the current choices. This information is
                 passed back to the calling function.

          \param dataOut A reference to a structure that contains all
                         the current experiment data
      */
      void updateExpInfoSelection( US_Experiment& );

      /*! \brief The signal that is emitted when the user chooses
                 to cancel the current selection. In this case all
                 previously-entered experiment parameter associations
                 are erased.
      */
      void cancelExpInfoSelection( void );

   private:
      US_Experiment&         expInfo;
      bool                   lab_changed;

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
      void assignInvestigator( int, const QString&, const QString& );
      void getInvestigatorInfo( void );
      void selectProject     ( void );
      void assignProject     ( US_Project& );
      void cancelProject     ( void );
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
