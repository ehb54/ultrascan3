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

      QLabel        *lbl_submit_url;
      QLineEdit     *le_submit_url;

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
      
   private slots:

      void setupGUI();

      void systems();

      void edit();
      void add_new();
      void delete_system();

      void save_config();
      void cancel();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
   
};

#endif
