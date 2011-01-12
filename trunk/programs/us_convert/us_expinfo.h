//! \file us_expinfo.h
#ifndef US_EXPINFO_H
#define US_EXPINFO_H

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_help.h"
#include "us_selectbox.h"
#include "us_project.h"

/*! \class US_ExpInfo
           This class provides the ability to associate raw data with
           the relevant experiment parameters, such as the lab,
           instrument, and rotor on which the experiment was run. 
*/
           
class US_EXTERN US_ExpInfo : public US_WidgetsDialog
{
   Q_OBJECT

   public:

      //! \brief  Class that contains information about all the labs, instruments,
      //!         operators, rotors, and rotor calibrations
      class HardwareInfo
      {
         public:
         struct CalibrationInfo
         {
            int              calibrationID;      //!< The ID of the rotor calibration
            double           rotorCoeff1;        //!< The first rotor stretch calibration coefficient
            double           rotorCoeff2;        //!< The second rotor stretch coefficient
         };

         struct RotorInfo
         {
            int              rotorID;            //!< The rotor that was used
            QString          rotorGUID;          //!< The GUID of the rotor
            QString          rotorSerial;        //!< The serial number of the rotor
            QList< CalibrationInfo > calibrationInfo; //!< All available calibration profiles for this rotor
         };

         struct OperatorInfo
         {
            int              operatorID;         //!< The personID of the person who operated the centrifuge
            QString          operatorGUID;       //!< The GUID of the operator
         };

         struct InstrumentInfo
         {
            int              instrumentID;       //!< The identifier of the ultra-centrifuge
            QString          instrumentSerial;   //!< The serial number of the instrument
            QList< RotorInfo > rotorInfo;        //!< All available rotors 
            QList< OperatorInfo > operatorInfo;  //!< Who can operate the instrument
         };

         struct LabInfo
         {
            int              labID;              //!< The lab in which the experiment was conducted
            QString          labGUID;            //!< The GUID of the lab
            QList< InstrumentInfo > instrumentInfo; //!< All the instruments in the lab
         };
      };

      //! \brief  Class that contains information about the hardware and other
      //!         associations
      class ExperimentInfo
      {
         public:
         int              invID;              //!< The personID of the investigator
         QString          invGUID;            //!< The GUID of the investigator
         
         QString          name;               //!< The name of the investigator
         int              expID;              //!< The ID of the experiment itself
         QString          expGUID;            //!< The GUID of the experiment
         int              projectID;          //!< The project this experiment is associated with
         QString          projectGUID;        //!< The GUID of the project
         QString          projectDesc;        //!< A short description of the project
         QString          runID;              //!< The run ID
         int              labID;              //!< The lab in which the experiment was conducted
         QString          labGUID;            //!< The GUID of the lab
         int              instrumentID;       //!< The identifier of the ultra-centrifuge
         QString          instrumentSerial;   //!< The serial number of the instrument
         int              operatorID;         //!< The personID of the person who operated the centrifuge
         QString          operatorGUID;       //!< The GUID of the operator
         int              rotorID;            //!< The rotor that was used
         QString          rotorGUID;          //!< The GUID of the rotor
         QString          rotorSerial;        //!< The serial number of the rotor
         int              calibrationID;      //!< The ID of the rotor calibration
         double           rotorCoeff1;        //!< The first rotor stretch calibration coefficient
         double           rotorCoeff2;        //!< The second rotor stretch coefficient
         QString          expType;            //!< The type of experiment
         QByteArray       opticalSystem;      //!< The type of optical system used
         QList< double >  rpms;               //!< A list of rotor speeds observed during the experiment
         QString          runTemp;            //!< The run temperature
         QString          label;              //!< The experiment label, or identifying information
         QString          comments;           //!< Comments that were associated with the experiment
         QString          centrifugeProtocol; //!< The governing centrifuge protocol
         QString          date;               //!< The date the record was entered or last updated
         bool             syncOK;             //!< The user has connected with the db
         ExperimentInfo();                    //!< A generic constructor
         ExperimentInfo&  operator=( const ExperimentInfo& ); //!< An overloaded assignment operator
         void updateDB( void );               //!< Function to update the database with changes
         void clear( void );                  //!< Function to reset all class variables to defaults
         void show ( void );                  // Temporary function to display current exp info
      };

      /*! \brief Generic constructor for the US_ExpInfo class. To 
                 instantiate the class a calling function must
                 provide a structure to contain all the data.
          \param dataIn  A reference to a structure that contains
                         previously selected experiment data, if any.
      */
      US_ExpInfo( ExperimentInfo&  );

      //! A null destructor. 
      ~US_ExpInfo() {};

   signals:

      /*! \brief The signal that is emitted when the user chooses
                 to accept the current choices. This information is
                 passed back to the calling function.

          \param dataOut A reference to a structure that contains all
                         the current experiment data
      */
      void updateExpInfoSelection( US_ExpInfo::ExperimentInfo& );

      /*! \brief The signal that is emitted when the user chooses
                 to cancel the current selection. In this case all
                 previously-entered experiment parameter associations
                 are erased.
      */
      void cancelExpInfoSelection( void );

   private:
      HardwareInfo           hwInfo;
      ExperimentInfo&        expInfo;
      bool                   cb_changed;

      US_Help                showHelp;

      QStringList            experimentTypes;
      QComboBox*             cb_expType;

      US_SelectBox*          cb_lab;
      US_SelectBox*          cb_instrument;
      US_SelectBox*          cb_operator;
      US_SelectBox*          cb_rotor;
                          
      QLineEdit*             le_investigator;
      QLineEdit*             le_runID;
      QLineEdit*             le_project;
      QLineEdit*             le_runTemp;
      QLineEdit*             le_label;
      QTextEdit*             te_comment;
                          
      QListWidget*           lw_rotorSpeeds;

      QPushButton*           pb_project;
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
      void setRotorList      ( void );
      void change_lab        ( int  );
      void change_instrument ( int  );
      void accept            ( void );
      void cancel            ( void );
      void connect_error     ( const QString& );
      void help              ( void )
        { showHelp.show_help( "manual/us_convert.html" ); };
};
#endif
