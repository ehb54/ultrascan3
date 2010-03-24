//! \file us_expinfo.h
#ifndef US_EXPINFO_H
#define US_EXPINFO_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_help.h"

/*! \class US_ExpInfo
           This class provides the ability to associate raw data with
           the relevant experiment parameters, such as the lab,
           instrument, and rotor on which the experiment was run. 
*/
           
class US_EXTERN US_ExpInfo : public US_WidgetsDialog
{
   Q_OBJECT

   public:

      //! \brief  Class that contains information about relevant 
      //!         cell/channel/wavelength combinations
      class TripleInfo
      {
         public:
         int              tripleID;           //!< The ID of this c/c/w combination
         int              centerpiece;        //!< The ID of the centerpiece that was used
         int              bufferID;           //!< The ID of the buffer that was associated
         int              analyteID;          //!< The ID of the analyte that was associated
         TripleInfo();                        //!< A generic constructor
      };

      //! \brief  Class that contains information about the hardware and other
      //!         associations
      class ExperimentInfo
      {
         public:
         int              invID;              //!< The personID of the investigator
         QString          lastName;           //!< The last name of the investigator
         QString          firstName;          //!< The first name of the investigator
         int              expID;              //!< The ID of the experiment itself
         int              projectID;          //!< The project this experiment is associated with
         int              labID;              //!< The lab in which the experiment was conducted
         int              instrumentID;       //!< The identifier of the ultra-centrifuge
         int              operatorID;         //!< The personID of the person who operated the centrifuge
         int              rotorID;            //!< The rotor that was used
         QString          expType;            //!< The type of experiment
         QString          runTemp;            //!< The run temperature
         QString          label;              //!< The experiment label, or identifying information
         QString          comments;           //!< Comments that were associated with the experiment
         QString          centrifugeProtocol; //!< The governing centrifuge protocol
         QString          date;               //!< The date the record was entered or last updated
         QList< TripleInfo > triples;         //!< Information about the cell/channel/wavelength info
         ExperimentInfo();                    //!< A generic constructor
         ExperimentInfo&  operator=( const ExperimentInfo& ); //!< An overloaded assignment operator
         void updateDB( void );               //!< Function to update the database with changes
         void clear( void );                  //!< Function to reset all class variables to defaults
      };

      /*! \brief Generic constructor for the US_ExpInfo class. To 
                 instantiate the class a calling function must
                 provide a structure to contain all the data.

          \param dataIn  A reference to a structure that contains
                         previously selected experiment data, if any.
      */
      US_ExpInfo( ExperimentInfo& );

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
      ExperimentInfo&        expInfo;

      US_Help                showHelp;

      // A list structure to contain hardware and other choices
      struct listInfo
      {
         QString             ID;
         QString             text;
      };
      QList< listInfo >      experimentList;
      QList< listInfo >      projectList;
      QList< listInfo >      labList;
      QList< listInfo >      instrumentList;
      QList< listInfo >      operatorList;
      QList< listInfo >      rotorList;

      QStringList            experimentTypes;
                          
      QComboBox*             cb_project;
      QComboBox*             cb_expType;
      QComboBox*             cb_lab;
      QComboBox*             cb_instrument;
      QComboBox*             cb_operator;
      QComboBox*             cb_rotor;
                          
      QLineEdit*             le_investigator;
      QLineEdit*             le_runTemp;
      QLineEdit*             le_label;
      QLineEdit*             le_centrifugeProtocol;

      QTextEdit*             te_comment;
                          
      QListWidget*           lw_experiment;
                          
      QPushButton*           pb_accept;
      QPushButton*           pb_newExperiment;

  private slots:
      void reset             ( void );
      void accept            ( void );
      void cancel            ( void );
      bool getExperimentDesc ( void );
      void selectInvestigator( void );
      void assignInvestigator( int, const QString&, const QString& );
      void selectExperiment  ( QListWidgetItem* );
      void updateExperiment  ( ExperimentInfo&  );
      void newExperiment     ( void );
      void connect_error     ( const QString& );

      QComboBox* us_projectComboBox   ( void );
      QComboBox* us_labComboBox       ( void );
      QComboBox* us_instrumentComboBox( void );
      QComboBox* us_operatorComboBox  ( void );
      QComboBox* us_rotorComboBox     ( void );
      QComboBox* us_expTypeComboBox   ( void );
      void setComboBoxIndex           ( QComboBox*,   QList< listInfo >&, int );
      void setWidgetIndex             ( QListWidget*, QList< listInfo >&, int );

      void help              ( void )
        { showHelp.show_help( "manual/us_convert.html" ); };
};
#endif
