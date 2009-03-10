//! \file us_investigator.h
#ifndef US_INVESTIGATOR_H
#define US_INVESTIGATOR_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_help.h"

//! An Interface for investigator information.

/*! An interface for personal information.
    This interface can be found in "Database->Commit Data
    to DB->Investigator" and other places.  The program will save
    investigator information to database table: tblInvestigators.    
*/

class US_EXTERN US_Investigator : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      US_Investigator( bool = false, QWidget* = 0, Qt::WindowFlags = 0 );

   signals:
      void investigator_accepted( int, const QString&, const QString& );

   private:
      bool signal_wanted;

      //! A Struct for storing investigator data in the DB.

      /*! Each element corresponds to an entry field in DB table
          tblInvestigators.
      */

      struct US_InvestigatorData
      {
         int     invID;     /*!< The uniqe ID in the DB for the  entry. */
         QString lastName;     
         QString firstName;    
         QString address;      
         QString city;         
         QString state;        
         QString zip;          
         QString phone;        
         QString email;        
         QString display;      
      } info;

      QList< struct US_InvestigatorData > investigators;

      QLineEdit*   le_invID;
      QLineEdit*   le_fname;
      QLineEdit*   le_lname;
      QLineEdit*   le_address;
      QLineEdit*   le_city;
      QLineEdit*   le_state;
      QLineEdit*   le_zip;
      QLineEdit*   le_phone;
      QLineEdit*   le_email;

      QListWidget* lw_names;

      QPushButton* pb_delete;
      QPushButton* pb_update;

      US_Help      showHelp;

      bool         check_fields( void );
      bool         changed     ( void );

   private slots:
      void queryDB     ( void );
      void add         ( void );
      void update      ( void );
      void del         ( void );
      void reset       ( void );
      void close       ( void );
      void limit_names ( const QString& );
      void get_inv_data( QListWidgetItem* );

      void help        ( void )
      { showHelp.show_help( "manual/tbl_investigator.html" ); };

};
#endif
