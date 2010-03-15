//! \file us_tripleinfo.h
#ifndef US_CCWINFO_H
#define US_CCWINFO_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_help.h"
#include "us_convert.h"

/*! \class US_TripleInfo
           This class provides the ability to associate a
           single US3 experiment cell/channel/wavelength combination
           with the relevant buffer and analyte data.
*/
class US_EXTERN US_TripleInfo : public US_WidgetsDialog
{
  Q_OBJECT

   public:

      /*! \brief Generic constructor for the US_TripleInfo class.
      */
      US_TripleInfo( void );

      //! A null destructor. 
      ~US_TripleInfo() {};

   signals:

      /*! \brief The signal that is emitted when the user chooses
                 to accept the current choices. This information is
                 passed back to the calling function.

          \param dataOut A reference to a structure that contains
                         buffer and analyte data for the current
                         cell/channel/wavelength combination
      */
      void updateTripleInfoSelection( US_Convert::TripleInfo& );

      /*! \brief The signal that is emitted when the user chooses
                 to cancel the current buffer and analyte selection.
      */
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
