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
    investigator information to database table 'people'.    
*/

class US_EXTERN US_Investigator : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      US_Investigator( bool = false, QWidget* = 0, Qt::WindowFlags = 0 );

      //! \brief A class for storing investigator data in the DB.

      /*! Each element corresponds to an entry field in DB table
          'people'.
      */
      class US_InvestigatorData
      {
         public:
         int     invID;        //!< The uniqe ID in the DB for the entry.
         QString lastName;     //!< Last Name
         QString firstName;    //!< First Name   
         QString address;      //!< Investigator's address
         QString city;         //!< Investigator's city
         QString state;        //!< Investigator's state
         QString zip;          //!< Investigator's zip code
         QString phone;        //!< Investigator's phone number
         QString email;        //!< Investigator's email address
         QString organization; //!< Investigator's organization
      };

   signals:
      //! \brief A signal that returns data to the invoking program
      //! \param investigatorID The index in the database for this person
      //! \param lname The last name of the selected investigator
      //! \param fname The first name of the selected investigator
      void investigator_accepted( int investigatorID, 
            const QString& lname, const QString& fname );

   private:
      bool                         signal_wanted;
      US_InvestigatorData          info;
      QList< US_InvestigatorData > investigators;

      QLineEdit*   le_search;
      QLineEdit*   le_invID;
      QLineEdit*   le_fname;
      QLineEdit*   le_lname;
      QLineEdit*   le_address;
      QLineEdit*   le_city;
      QLineEdit*   le_state;
      QLineEdit*   le_zip;
      QLineEdit*   le_phone;
      QLineEdit*   le_email;
      QLineEdit*   le_org;

      QListWidget* lw_names;

      QPushButton* pb_update;

      US_Help      showHelp;

      bool         check_fields( void );
      bool         changed     ( void );

   private slots:
      void queryDB     ( void );
      void update      ( void );
      void reset       ( void );
      void close       ( void );
      void limit_names ( const QString& );
      void get_inv_data( QListWidgetItem* );

      void help        ( void )
      { showHelp.show_help( "tbl_investigator.html" ); };

};
#endif
