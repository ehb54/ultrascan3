//! \file us_expinfo.h
#ifndef US_EXPINFO_H
#define US_EXPINFO_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets.h"
#include "us_widgets_dialog.h"
#include "us_help.h"
#include "us_selectbox.h"

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
         QString          bufferDesc;         //!< The corresponding buffer description
         int              analyteID;          //!< The ID of the analyte that was associated
         QString          analyteDesc;        //!< The corresponding analyte description
         char             guid[16];           //!< The GUID of this triple
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
         QString          runID;              //!< The run ID
         int              labID;              //!< The lab in which the experiment was conducted
         int              instrumentID;       //!< The identifier of the ultra-centrifuge
         int              operatorID;         //!< The personID of the person who operated the centrifuge
         int              rotorID;            //!< The rotor that was used
         QString          expType;            //!< The type of experiment
         QString          opticalSystem;      //!< The type of optical system used
         QList< double >  rpms;               //!< A list of rotor speeds observed during the experiment
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
          \param editing A way to distinguish between a new db instance
                         or editing a previously existing one
      */
      US_ExpInfo( ExperimentInfo&, bool editing = false );

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
      bool                   editing;
      bool                   cb_changed;

      US_Help                showHelp;

      QStringList            experimentTypes;
      QComboBox*             cb_expType;

      US_SelectBox*          cb_project;
      US_SelectBox*          cb_lab;
      US_SelectBox*          cb_instrument;
      US_SelectBox*          cb_operator;
      US_SelectBox*          cb_rotor;
                          
      QLineEdit*             le_investigator;
      QLineEdit*             le_runID;
      QLineEdit*             le_runTemp;
      QLineEdit*             le_label;
      QTextEdit*             te_comment;
                          
      QListWidget*           lw_rotorSpeeds;

      QPushButton*           pb_accept;

  private slots:
      void reset             ( void );
      bool load              ( void );
      void reload            ( void );
      void selectInvestigator( void );
      void assignInvestigator( int, const QString&, const QString& );
      QComboBox* us_expTypeComboBox         ( void );
      void runIDChanged      ( void );
      int  checkRunID        ( void );
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
