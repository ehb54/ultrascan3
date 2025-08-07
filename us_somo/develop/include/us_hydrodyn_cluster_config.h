#ifndef US_HYDRODYN_CLUSTER_CONFIG_H
#define US_HYDRODYN_CLUSTER_CONFIG_H

// QT defs:

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qstring.h>
// #include <q3frame.h>
#include <qcheckbox.h>
#include <qlistwidget.h>
// Added by qt3to4:
#include <QCloseEvent>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "us_hydrodyn_cluster.h"

// standard C and C++ defs:

using namespace std;

class US_EXTERN US_Hydrodyn_Cluster_Config : public QDialog {
  Q_OBJECT
  friend class US_Hydrodyn_Cluster;
  friend class US_Hydrodyn_Cluster_Config_Server;

 public:
  US_Hydrodyn_Cluster_Config(void *us_hydrodyn, QWidget *p = 0,
                             const char *name = 0);
  ~US_Hydrodyn_Cluster_Config();

 private:
  void *us_hydrodyn;

  US_Config *USglobal;

  QLabel *lbl_title;

  QLabel *lbl_cluster_id;
  QLineEdit *le_cluster_id;

  QLabel *lbl_cluster_pw;
  QLineEdit *le_cluster_pw;

  QLabel *lbl_cluster_pw2;
  QLineEdit *le_cluster_pw2;

  QLabel *lbl_cluster_email;
  QLineEdit *le_cluster_email;

  QLabel *lbl_submit_url;
  QLineEdit *le_submit_url;

  QLabel *lbl_manage_url;
  QLineEdit *le_manage_url;

  QPushButton *pb_check_user;
  QPushButton *pb_add_user;

  QLabel *lbl_systems;
  QListWidget *lb_systems;

  QPushButton *pb_edit;
  QPushButton *pb_add_new;
  QPushButton *pb_delete_system;

  QPushButton *pb_reset;
  QPushButton *pb_save_config;
  QPushButton *pb_help;
  QPushButton *pb_cancel;

  void *cluster_window;

  map<QString, map<QString, QString> > cluster_systems;

  bool comm_active;
  QString comm_mode;

  QNetworkAccessManager *http_access_manager;
  QNetworkRequest http_request;
  QNetworkReply *http_reply;
  void http_done(bool error);

  QString current_http;
  QString current_http_response;
  QString current_http_error;
  QString current_response_status;
  bool check_tried;
  bool check_not_ok;
  void update_enables();

 private slots:

  void setupGUI();

  void update_cluster_id(const QString &);
  void update_cluster_pw(const QString &);
  void update_cluster_pw2(const QString &);
  void update_cluster_email(const QString &);
  void update_manage_url(const QString &);

  void check_user();
  void add_user();

  void systems();

  void edit();
  void add_new();
  void delete_system();

  void reset();
  void save_config();
  void cancel();
  void help();

  void http_finished();
  void http_error(QNetworkReply::NetworkError code);
  void http_uploadProgress(qint64 done, qint64 total);
  void http_downloadProgress(qint64 done, qint64 total);

 protected slots:

  void closeEvent(QCloseEvent *);
};

#endif
