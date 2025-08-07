#ifndef US_ADMIN_H
#define US_ADMIN_H

#include <qdatastream.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
//Added by qt3to4:
#include <QFrame>
#include "us_util.h"
//! An Interface for database adminstration.
/*!
   For database delete operation perimission.
*/
class US_EXTERN US_Admin : public QFrame {
      Q_OBJECT

   public:
      US_Admin(QWidget *p = 0, const char *name = "us_admin");
      ~US_Admin();

      QLabel *lbl_blank; /*!< A raised Label shows 'Change Adminstrator Password'. */
      QLabel *lbl_psswd1; /*!< A Label shows 'Enter New Password:'. */
      QLabel *lbl_psswd2; /*!< A Label shows 'Verify New Password:'. */
      QLineEdit *le_psswd1; /*!< A LineEdit for inputting new password. */
      QLineEdit *le_psswd2; /*!< A LineEdit for reinputting new password. */
      QPushButton *pb_help; /*!< A PushButton connect to help(). */
      QPushButton *pb_save; /*!< A PushButton connect to save(). */
      QPushButton *pb_cancel; /*!< A PushButton connect to quit(). */
      QString password1, /*! <A string variable for password. */
         password2; /*! <A string variable for verifying password. */
      US_Config *USglobal; /*!< A US_Config reference. */

   public slots:

      void update_psswd1(const QString &);
      void update_psswd2(const QString &);
      void help();
      void save();
      void quit();
};

#endif
