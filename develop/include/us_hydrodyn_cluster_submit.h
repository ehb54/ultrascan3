#ifndef US_HYDRODYN_CLUSTER_SUBMIT_H
#define US_HYDRODYN_CLUSTER_SUBMIT_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qlistbox.h>
#include <qsocket.h>

#include "us_util.h"
#include "us_hydrodyn_pdbdefs.h"
#include "us_hydrodyn_batch.h"

//standard C and C++ defs:

using namespace std;

class US_EXTERN US_Hydrodyn_Cluster_Submit : public QDialog
{
   Q_OBJECT
      friend class US_Hydrodyn_Cluster;

   public:
     US_Hydrodyn_Cluster_Submit(
                               void *us_hydrodyn,
                               QWidget *p = 0, 
                               const char *name = 0
                               );
      ~US_Hydrodyn_Cluster_Submit();
      
    private:
      void          *us_hydrodyn;
      
      US_Config     *USglobal;

      QLabel        *lbl_title;

      QLabel        *lbl_files;
      QListBox      *lb_files;

      QPushButton   *pb_select_all;
      QPushButton   *pb_submit;

      QFont         ft;
      QTextEdit     *editor;
      QMenuBar      *m;

      QPushButton   *pb_stop;
      QPushButton   *pb_help;
      QPushButton   *pb_cancel;
      
      QStringList   files;

      void          editor_msg( QString color, QString msg );

      QString       pkg_dir;
      QString       submitted_dir;
      QString       tmp_dir;

      QString       cluster_id;
      QString       submit_url;
      QString       submit_url_host;
      QString       submit_url_port;
      QString       stage_url;
      QString       stage_url_path;
      QString       stage_path;

      QString       errormsg;
      bool          disable_updates;

      unsigned int  update_files( bool set_lb_files = true );

      bool          stage( QStringList files );
      bool          stage( QString     file  );
      void          *cluster_window;

      bool          run_in_tmp( QString cmd );
      QStringList   last_stdout;
      QStringList   last_stderr;
      bool          submit_xml( QString file, QString &xml );
      bool          submit_jobs( QStringList files );
      bool          send_xml( QString xml );

      bool          submit_active;
      bool          comm_active;
      QSocket       submit_socket;

      QString       current_xml;
      QString       current_xml_response;

   private slots:

      void setupGUI();
   
      void update_enables();
      void select_all();
      void submit();

      void clear_display();
      void update_font();
      void save();

      void stop();
      void cancel();
      void help();

      void socket_error( int );
      void socket_connected();
      void socket_readyRead();
      void socket_connectionClosed();
      void socket_delayedCloseFinished();

   protected slots:

      void closeEvent(QCloseEvent *);
   
};

#endif
