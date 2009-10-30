#ifndef US_CCWINFO_H
#define US_CCWINFO_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_help.h"
#include "us_convert.h"

class US_EXTERN US_TripleInfo : public US_WidgetsDialog
{
  Q_OBJECT

   public:
      US_TripleInfo( void );

   signals:
      void updateTripleInfoSelection( US_Convert::TripleInfo& );
      void cancelTripleInfoSelection( void );

   private:
      US_Help              showHelp;
      QStringList          centerpieceTypes;

      QComboBox*           cb_centerpiece;

      QPushButton*         pb_buffer;
      QPushButton*         pb_analyte;

      bool centerpieceInfo ( void );

      int  save_bufferID;
      int  save_analyteID;
   
  private slots:
      void reset           ( void );
      void accept          ( void );
      void cancel          ( void );
      void selectBuffer    ( void );
      void assignBuffer    ( int  );
      void selectAnalyte   ( void );
      void assignAnalyte   ( int  );
      void help            ( void )
        { showHelp.show_help( "manual/us_convert.html" ); };
};
#endif
