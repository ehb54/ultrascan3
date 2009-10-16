#ifndef US_EXPINFO_H
#define US_EXPINFO_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_help.h"

class US_EXTERN US_ExpInfo : public US_WidgetsDialog
{
  Q_OBJECT

   public:
      US_ExpInfo();

      class ExpInfo
      {
         public:
         int              investigator;
         QString          expType;
         int              rotor;
         QString          date;
         QString          label;
         QString          comments;
         ExpInfo();
         void clear( void );
      };

   signals:
      void update_expinfo_selection( US_ExpInfo::ExpInfo& );
      void cancel_expinfo_selection( void );

   private:
      US_Help              showHelp;
      QStringList          rotorTypes;

      QComboBox*           cb_rotor;
      QComboBox*           cb_expType;

      QLineEdit*           le_investigator;
      QLineEdit*           le_label;

      QTextEdit*           te_comment;

      bool rotorInfo       ( void );

  private slots:
      void reset           ( void );
      void accept          ( void );
      void cancel          ( void );
      void sel_investigator( void );
      void assign_investigator( int, const QString&, const QString& );
      void help            ( void )
        { showHelp.show_help( "manual/us_convert.html" ); };
};
#endif
