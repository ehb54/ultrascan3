#ifndef US_CCWINFO_H
#define US_CCWINFO_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_help.h"

class US_EXTERN US_CCWInfo : public US_WidgetsDialog
{
  Q_OBJECT

   public:
      US_CCWInfo( int invID = 0 );

      class CCWInfo
      {
         public:
         int              investigator;
         int              centerpiece;
         int              bufferID;
         int              analyteID;
         CCWInfo();
         void clear( void );
      };

   signals:
      void update_ccwinfo_selection( US_CCWInfo::CCWInfo& );
      void cancel_ccwinfo_selection( void );

   private:
      US_Help              showHelp;
      QStringList          centerpieceTypes;

      QLineEdit*           le_investigator;
      QComboBox*           cb_centerpiece;

      QPushButton*         pb_buffer;
      QPushButton*         pb_analyte;

      bool centerpieceInfo ( void );

      int  save_investigator;
      int  save_bufferID;
      int  save_analyteID;
   
  private slots:
      void reset           ( void );
      void accept          ( void );
      void cancel          ( void );
      void sel_investigator( void );
      void assign_investigator( int, const QString&, const QString& );
      void sel_buffer      ( void );
      void assign_buffer   ( int  );
      void sel_analyte     ( void );
      void assign_analyte  ( int  );
      void help            ( void )
        { showHelp.show_help( "manual/us_convert.html" ); };
};
#endif
