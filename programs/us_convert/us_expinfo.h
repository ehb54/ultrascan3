//! \file us_expinfo.h
#ifndef US_EXPINFO_H
#define US_EXPINFO_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_help.h"
#include "us_convert.h"

/*! \class US_ExpInfo
           This class provides the ability to associate raw data with
           the relevant experiment parameters, such as the lab,
           instrument, and rotor on which the experiment was run. 
*/
           
class US_EXTERN US_ExpInfo : public US_WidgetsDialog
{
   Q_OBJECT

   public:

      /*! \brief Generic constructor for the US_ExpInfo class. To 
                 instantiate the class a calling function must
                 provide a structure to contain all the data.

          \param dataIn  A reference to a structure that contains
                         previously selected experiment data, if any.
      */
      US_ExpInfo( US_Convert::ExperimentInfo& );

      //! A null destructor. 
      ~US_ExpInfo() {};

   signals:

      /*! \brief The signal that is emitted when the user chooses
                 to accept the current choices. This information is
                 passed back to the calling function.

          \param dataOut A reference to a structure that contains all
                         the current experiment data
      */
      void updateExpInfoSelection( US_Convert::ExperimentInfo& );

      /*! \brief The signal that is emitted when the user chooses
                 to cancel the current selection. In this case all
                 previously-entered experiment parameter associations
                 are erased.
      */
      void cancelExpInfoSelection( void );

   private:
      US_Convert::ExperimentInfo& expInfo;

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
