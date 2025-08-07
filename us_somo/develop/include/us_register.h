#ifndef US_REGISTER_H
#define US_REGISTER_H

// #include <q3textstream.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <qprocess.h>
#include <stdlib.h>

#include <iostream>
// #include <q3frame.h>
#include "us_extern.h"
#include "us_util.h"

using namespace std;

//! A Struct for storing required register info for new software user.
/*!
  Each element is according to an entry field in Database Table
  <tt>tblInvestigators</tt>.
*/
struct US_RegisterData {
  QString lastname;
  QString firstname;
  QString company;
  QString address;
  QString city;
  QString state;
  QString zip;
  QString phone;
  QString email;
  QString platform;
  QString os;
  QString version;
  QString license_type;
  QString license;
  QString expiration;
  QString code1;
  QString code2;
  QString code3;
  QString code4;
  QString code5;
};

//! An class for checking register information.
/*!
  When you first time to use this software after you install it in a new
  machine. This class will check your register information. To find whether you
  are registered, whether your license is valid, whether your license is
  expired. If some error occurs, Let you link to author to help you resolve it.
*/

class US_EXTERN US_Register : public QFrame {
  Q_OBJECT

 public:
  US_Register(QWidget* p = 0, const char* name = "us_register");
  ~US_Register();

  struct US_RegisterData register_list; /*!< A struct US_RegisterData for
                                             storing register information. */

  QProcess* proc; /*! <A new process reference. */

 private:
  US_Config* USConfig;

 private slots:
  void license_info(const QString&);
  void us_license();

 public slots:
  bool read();
};

#endif
