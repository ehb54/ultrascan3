#ifndef US_HYDRODYN_CLUSTER_CONFIG_H
#define US_HYDRODYN_CLUSTER_CONFIG_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qlistbox.h>
#include <qhttp.h>

#include "us_hydrodyn_cluster.h"

//standard C and C++ defs:

using namespace std;

class US_EXTERN US_Hydrodyn_Cluster_Config : public QDialog
{
   Q_OBJECT
      friend class US_Hydrodyn_Cluster;
      friend class US_Hydrodyn_Cluster_Config_Server;

   public:
     US_Hydrodyn_Cluster_Config(
                               void *us_hydrodyn,
                               QWidget *p = 0, 
                               const char *name = 0
                               );
      ~US_Hydrodyn_Cluster_Config();
      
    private:
      void          *us_hydrodyn;
      
      US_Config     *USglobal;

      QLabel        *lbl_title;

      QLabel        *lbl_cluster_id;
      QLineEdit     *le_cluster_id;

      QLabel        *lbl_cluster_pw;
      QLineEdit     *le_cluster_pw;

      QLabel        *lbl_cluster_pw2;
      QLineEdit     *le_cluster_pw2;

      QLabel        *lbl_cluster_email;
      QLineEdit     *le_cluster_email;

      QLabel        *lbl_submit_url;
      QLineEdit     *le_submit_url;

      QLabel        *lbl_manage_url;
      QLineEdit     *le_manage_url;

      QPushButton   *pb_check_user;
      QPushButton   *pb_add_user;

      QLabel        *lbl_systems;
      QListBox      *lb_systems;

      QPushButton   *pb_edit;
      QPushButton   *pb_add_new;
      QPushButton   *pb_delete_system;
      
      QPushButton   *pb_save_config;
      QPushButton   *pb_help;
      QPushButton   *pb_cancel;

      void          *cluster_window;

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif
      map < QString, map < QString, QString > > cluster_systems;
#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif

      bool          comm_active;
      QString       comm_mode;
      QHttp         submit_http;
      QString       current_http;
      QString       current_http_response;
      QString       current_response_status;
      bool          check_tried;
      bool          check_not_ok;
      void          update_enables();
      
   private slots:

      void setupGUI();

      void update_cluster_id   ( const QString & );
      void update_cluster_pw   ( const QString & );
      void update_cluster_pw2  ( const QString & );
      void update_cluster_email( const QString & );
      void update_manage_url   ( const QString & );

      void check_user();
      void add_user();

      void systems();

      void edit();
      void add_new();
      void delete_system();

      void save_config();
      void cancel();
      void help();

      void http_stateChanged ( int state );
      void http_responseHeaderReceived ( const QHttpResponseHeader & resp );
      void http_readyRead ( const QHttpResponseHeader & resp );
      void http_dataSendProgress ( int done, int total );
      void http_dataReadProgress ( int done, int total );
      void http_requestStarted ( int id );
      void http_requestFinished ( int id, bool error );
      void http_done ( bool error );

   protected slots:

      void closeEvent(QCloseEvent *);
   
};

#endif
