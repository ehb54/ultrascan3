#ifndef US_EXPINFO_H
#define US_EXPINFO_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_help.h"
#include "us_convert.h"

class US_EXTERN US_ExpInfo : public US_WidgetsDialog
{
  Q_OBJECT

   public:
      US_ExpInfo( US_Convert::ExperimentInfo& );

   signals:
      void updateExpInfoSelection( US_Convert::ExperimentInfo& );
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
