//! \file us_xpnhost.h
#ifndef US_NEWXPNHOSTDB_H
#define US_NEWXPNHOSTDB_H

#include <QtCore>

#include "us_db2.h"
#include "us_widgets.h"
#include "us_widgets_dialog.h"
#include "us_xpnhost_db.h"

/*! \brief A class to allow the user to set, modify or delete
    parameters for database access.
*/

class US_NewXpnHostDB : public US_Widgets {
  Q_OBJECT

 public:
  //!  Construct the window to manage database parameters.
  /* US_NewXpnHostDB( QWidget* w = 0, Qt::WindowFlags flags = 0 ); */
  /* US_NewXpnHostDB( QWidget* w = 0, Qt::WindowFlags flags = 0, QMap<QString,
   * QString> ); */
  US_NewXpnHostDB();
  US_NewXpnHostDB(QMap<QString, QString> instrument);

 private:
  // US_XpnHostDB::Instrument   currentInstrument;

  /* QList<QStringList> dblist; */
  /* QString            uuid; */
  /* QMap< QString, QString > conn_stat; */

  QMap<QString, QString> instrumentedit;

  QPushButton* pb_save;
  QPushButton* pb_cancel;
  QPushButton* pb_loadchromo;
  QPushButton* pb_testconn;

  QLabel* host;
  QLabel* port;
  QLabel* msgPort;

  QLabel* name;
  QLabel* user;
  QLabel* pasw;
  QLabel* bn_chromoab;
  QLabel* lb_radcalwvl;

  QLineEdit* le_description;
  QLineEdit* le_serialNumber;
  QLineEdit* le_dbname;
  QLineEdit* le_host;
  QLineEdit* le_port;
  QLineEdit* le_msgPort;

  QLineEdit* le_name;
  QLineEdit* le_user;
  QLineEdit* le_pasw;

  QwtCounter* ct_radcalwvl;
  QLineEdit* le_chromofile;

  QComboBox* cb_os1;
  QComboBox* cb_os2;
  QComboBox* cb_os3;

  QComboBox* cb_type;

  bool use_db;
  bool nonOptima_selected;

  bool update_instrument;
  QString ChromoArrayList;
  QMap<double, double> ChromoArray;

  QVector<double> corr_lambda;
  QVector<double> corr_value;
  QVector<double> corr_lambda_current;
  QVector<double> corr_value_current;

 private slots:
  void save_new(void);
  void cancel(void);
  void fillGui(void);
  void desc_changed(QString);
  void load_chromo(void);
  void readingChromoArrayFile(const QString&);
  void readingChromoArrayDB(void);
  void shiftChromoArray(double);
  void changeType(int);
  bool test_connectivity(void);
  /* void reset       ( void ); */
  /* void save_default( void ); */
  /* void deleteDB    ( void ); */
  /* bool test_connect( void ); */

 signals:
  void accepted(QMap<QString, QString>& newInstrument);
  void editnew_cancelled(void);
};
#endif
