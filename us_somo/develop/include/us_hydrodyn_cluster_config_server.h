#ifndef US_HYDRODYN_CLUSTER_CONFIG_SERVER_H
#define US_HYDRODYN_CLUSTER_CONFIG_SERVER_H

// QT defs:

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qstring.h>
//#include <q3frame.h>
#include <qcheckbox.h>
#include <qlistwidget.h>
//Added by qt3to4:
#include <QCloseEvent>

#include "us_hydrodyn_cluster.h"

//standard C and C++ defs:

using namespace std;

class US_EXTERN US_Hydrodyn_Cluster_Config_Server : public QDialog {
      Q_OBJECT
      friend class US_Hydrodyn_Cluster;

   public:
      US_Hydrodyn_Cluster_Config_Server(
         map<QString, QString> &system_map, QString server_name, void *us_hydrodyn, QWidget *p = 0,
         const char *name = 0);
      ~US_Hydrodyn_Cluster_Config_Server();

   private:
      void *us_hydrodyn;

      US_Config *USglobal;

      QLabel *lbl_title;

#ifdef WIN32
#if QT_VERSION < 0x040000
#pragma warning(disable : 4251)
#endif
#endif

      vector<QLabel *> lbls;
      vector<QLineEdit *> les;

      map<QString, QString> *system_map;
      map<QString, QString> our_system_map;

#ifdef WIN32
#if QT_VERSION < 0x040000
#pragma warning(default : 4251)
#endif
#endif

      QPushButton *pb_save_config;
      QPushButton *pb_help;
      QPushButton *pb_cancel;

      void *cluster_window;

      QString server_name;

   private slots:

      void setupGUI();

      void save_config();
      void cancel();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
};

#endif
