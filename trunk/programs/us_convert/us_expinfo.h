//! \file us_expinfo.h
#ifndef US_EXPINFO_H
#define US_EXPINFO_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets.h"
#include "us_widgets_dialog.h"
#include "us_help.h"

/*! \class US_DBControl
           This is a base class that helps to provide the ability to 
           create QWidgets that can populate themselves from the US3
           database and then keep track of their contents. This
           is intended for widgets like combo boxes and list widgets
           that have a logical ID and an associated description to be
           displayed in the widget for selection.
*/
class US_EXTERN US_DBControl
{
   public:
      /*! \brief Generic constructor for the US_DBWidget class. To 
                 instantiate the class a calling function must
                 provide information about which US3 stored procedure
                 to use. 

                 The US3 procedure must be of a certain type. It must
                 return a multirow dataset containing the table
                 ID and the name or other information to display in the
                 widget.

          \param query  The number of US3 stored procedure to execute,
                        using the dbQueries enum below
      */
      US_DBControl             ( int = -1 );

      //! A null destructor. 
      ~US_DBControl            () {};

      enum dbQueries
      {
         SQL_PROJECTS    ,       //!< Query to get project descriptions
         SQL_LABS        ,       //!< Query to get lab names
         SQL_INSTRUMENTS ,       //!< Query to get instrument names
         SQL_PEOPLE      ,       //!< Query to get names of people
         SQL_ROTORS              //!< Query to get rotor names
      };

   protected:
      struct listInfo
      {
         QString        ID;
         QString        text;
      };

      QList< listInfo >        widgetList;

      QString                  dbError;
};

/*! \class US_DBComboBox
           This class provides the ability to create a combo box that
           can keep track of its contents. The ability to set the box
           to display the contents associated with the logical ID (the 
           ID from the database), and to retrieve the current logical 
           ID are provided.
*/
class US_EXTERN US_DBComboBox : public QComboBox, US_DBControl
{
   public:
      /*! \brief Generic constructor for the US_DBComboBox class. To 
                 instantiate the class a calling function must
                 provide information about which US3 stored procedure
                 to use. 

                 The US3 procedure must be of a certain type. It needs
                 to return a multirow dataset containing the table
                 ID and the name or other information to display in the
                 combo box.

          \param parent A reference to the parent dialog to which this 
                        US_DBComboBox belongs, or 0 if no parent.
          \param query  The number of the US3 stored procedure to execute,
                        using the dbQueries enum in US_DBControl.
      */
      US_DBComboBox            ( QWidget* parent = 0, int = -1 );

      //! A null destructor. 
      ~US_DBComboBox           () {};

      /*! \brief A function to set the current index of the combo box
                 to a logical ID, based on the ID of the record in the
                 database

          \param ID    The logical ID of the database record to set the
                       current index to
      */
      void setComboBoxIndex    ( int  );

      /*! \brief A function to retrieve the logical ID of the database
                 record to which the combo box is currently pointing
      */
      int  getComboBoxID       ( void );
};

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

      QStringList            experimentTypes;
                          
      US_DBComboBox*         cb_project;
      US_DBComboBox*         cb_lab;
      US_DBComboBox*         cb_instrument;
      US_DBComboBox*         cb_operator;
      US_DBComboBox*         cb_rotor;
      QListWidget*           lw_experiment;
      QComboBox*             cb_expType;
                          
      QLineEdit*             le_investigator;
      QLineEdit*             le_runTemp;
      QLineEdit*             le_label;
      QLineEdit*             le_centrifugeProtocol;

      QTextEdit*             te_comment;
                          
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

      QComboBox* us_expTypeComboBox   ( void );
      void setWidgetIndex             ( QListWidget*, QList< listInfo >&, int );

      void help              ( void )
        { showHelp.show_help( "manual/us_convert.html" ); };
};
#endif
