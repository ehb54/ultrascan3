#ifndef US_HYDRODYN_CLUSTER_STATUS_H
#define US_HYDRODYN_CLUSTER_STATUS_H

// QT defs:

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qstring.h>
// #include <q3frame.h>
#include <qtcpsocket.h>
#include <qtreewidget.h>
// #include <q3cstring.h>
#include <qiodevice.h>
// Added by qt3to4:
#include <QCloseEvent>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "us_hydrodyn_batch.h"
#include "us_hydrodyn_pdbdefs.h"
#include "us_util.h"

#ifdef WIN32
#if QT_VERSION < 0x040000
#pragma warning(disable : 4251)
#endif
#endif

// standard C and C++ defs:

using namespace std;

class US_EXTERN US_Hydrodyn_Cluster_Status : public QDialog {
  Q_OBJECT
  friend class US_Hydrodyn_Cluster;

 public:
  US_Hydrodyn_Cluster_Status(void *us_hydrodyn, QWidget *p = 0,
                             const char *name = 0);
  ~US_Hydrodyn_Cluster_Status();

 private:
  void *us_hydrodyn;

  US_Config *USglobal;

  QLabel *lbl_title;

  QTreeWidget *lv_files;

  QPushButton *pb_refresh;
  QPushButton *pb_remove;
  QPushButton *pb_retrieve_selected;
  QPushButton *pb_retrieve;

  QProgressBar *progress;

  QFont ft;
  QTextEdit *editor;
  QMenuBar *m;

  QPushButton *pb_stop;
  QPushButton *pb_help;
  QPushButton *pb_cancel;

  QStringList files;

  void editor_msg(QString color, QString msg);

  QString pkg_dir;
  QString submitted_dir;
  QString completed_dir;
  QString tmp_dir;

  QString cluster_id;
  QString cluster_pw;

  QString manage_url;
  QString manage_url_host;
  QString manage_url_port;

  // QString       submit_url;
  // QString       submit_url_host;
  // QString       submit_url_port;
  // QString       stage_url;
  // QString       stage_url_path;
  // QString       stage_path;

  QString errormsg;
  bool disable_updates;

  void *cluster_window;

  bool comm_active;
  QString comm_mode;

  QNetworkAccessManager *http_access_manager;
  QNetworkRequest http_request;
  QNetworkReply *http_reply;

  void http_done(bool error);
  void http_retrieve_done(bool error);

  QFile *retrieve_file;
  QString retrieve_file_name;
  QDataStream *retrieve_datastream;

  unsigned int update_files(bool set_lv_files = true);
  bool send_http_get(QString file);
  QString current_http;
  QString current_http_response;
  QString current_http_error;

  map<QTreeWidgetItem *, QString> jobs;
  map<QString, QString> job_hostname;

  QTreeWidgetItem *next_to_process;
  void get_status();
  void cancel_selected();
  void complete_remove();
  void complete_retrieve();

  bool system_cmd(QStringList cmd);
  bool system_proc_active;
  bool processing_active;
  bool stopFlag;
  QProcess *system_proc;
  bool schedule_retrieve(QString file);

 private slots:

  void setupGUI();

  void update_enables();

  void refresh();
  void remove();
  void retrieve();
  void retrieve_selected();

  void clear_display();
  void update_font();
  void save();

  void stop();
  void cancel();
  void help();

  void next_status();
  void get_next_status();
  void get_next_retrieve();

  void http_finished();
  void http_error(QNetworkReply::NetworkError code);
  void http_uploadProgress(qint64 done, qint64 total);
  void http_downloadProgress(qint64 done, qint64 total);

  void http_retrieve_finished();
  void http_retrieve_error(QNetworkReply::NetworkError code);
  void http_retrieve_uploadProgress(qint64 done, qint64 total);
  void http_retrieve_downloadProgress(qint64 done, qint64 total);

  void system_proc_readFromStdout();
  void system_proc_readFromStderr();
  void system_proc_finished(int, QProcess::ExitStatus);
  void system_proc_started();

 protected slots:

  void closeEvent(QCloseEvent *);
};

#ifdef WIN32
#if QT_VERSION < 0x040000
#pragma warning(default : 4251)
#endif
#endif

#endif
