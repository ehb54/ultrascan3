#ifndef US_DB_ADMIN_H
#define US_DB_ADMIN_H

#include <qpushbutton.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qdatastream.h>
#include "us_util.h"
//! An Interface for database adminstration.
/*!
  For database delete operation perimission.
*/
class US_EXTERN US_DB_Admin : public QFrame
{
  Q_OBJECT
  
public:
    US_DB_Admin(QString password, QWidget* p = 0, const char* name = "us_db_admin");
    ~US_DB_Admin();
    
    QLabel*      lbl_blank;     /*!< A raised Label shows 'Please input adminstrator password'. */
    QLabel*      lbl_psswd;     /*!< A Label shows 'Password:'. */
    QLineEdit*   le_psswd;      /*!< A LineEdit for inputting password. */
    QPushButton* pb_submit;     /*!< A PushButton connect to submit(). */
    QPushButton* pb_cancel;     /*!< A PushButton connect to quit(). */
    QString      password,      /*! <A string variable for password. */
          input_password,       /*! <A string variable for input password. */
          encrypted_password;   /*! <A string variable for encrypted password. */


    US_Config* USglobal;        /*!< A US_Config reference. */
          
public slots:
    
    void update_psswd(const QString &);
    void submit();
    void quit();
  
  /*!
    This signal is emitted in function submit().\n
    The argument is boolean variable <var>permission</var>.
  */
signals:
    void issue_pass( QString );
    void issue_permission( bool );
};

#endif


