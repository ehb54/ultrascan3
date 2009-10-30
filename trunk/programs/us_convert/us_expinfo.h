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
      US_ExpInfo( void );

   signals:
      void updateExpInfoSelection( US_Convert::ExperimentInfo& );
      void cancelExpInfoSelection( void );

   private:
      US_Help                showHelp;
      QStringList            rotorTypes;
                          
      QComboBox*             cb_rotor;
      QComboBox*             cb_expType;
                          
      QLineEdit*             le_investigator;
      QLineEdit*             le_label;
                          
      QTextEdit*             te_comment;
                          
      bool rotorInfo         ( void );

  private slots:
      void reset             ( void );
      void accept            ( void );
      void cancel            ( void );
      void selectInvestigator( void );
      void assignInvestigator( int, const QString&, const QString& );
      void help              ( void )
        { showHelp.show_help( "manual/us_convert.html" ); };
};
#endif
