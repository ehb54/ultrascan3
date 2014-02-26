#ifndef US_HYDRODYN_CLUSTER_STATUS_H
#define US_HYDRODYN_CLUSTER_STATUS_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <q3frame.h>
#include <q3listview.h>
#include <q3socket.h>
#include <q3cstring.h>
#include <qiodevice.h>
//Added by qt3to4:
#include <QCloseEvent>

#include "us_util.h"
#include "us_hydrodyn_pdbdefs.h"
#include "us_hydrodyn_batch.h"

#include <q3http.h>
#include <q3ftp.h>

#ifdef WIN32
# if !defined( QT4 )
  #pragma warning ( disable: 4251 )
# endif
#endif

//standard C and C++ defs:

using namespace std;

class US_EXTERN US_Hydrodyn_Cluster_Status : public QDialog
{
   Q_OBJECT
      friend class US_Hydrodyn_Cluster;

   public:
     US_Hydrodyn_Cluster_Status(
                               void *us_hydrodyn,
                               QWidget *p = 0, 
                               const char *name = 0
                               );
      ~US_Hydrodyn_Cluster_Status();
      
    private:
      void          *us_hydrodyn;
      
      US_Config     *USglobal;

      QLabel        *lbl_title;

      Q3ListView     *lv_files;

      QPushButton   *pb_refresh;
      QPushButton   *pb_remove;
      QPushButton   *pb_retrieve_selected;
      QPushButton   *pb_retrieve;

      Q3ProgressBar  *progress;

      QFont         ft;
      Q3TextEdit     *editor;
      QMenuBar      *m;

      QPushButton   *pb_stop;
      QPushButton   *pb_help;
      QPushButton   *pb_cancel;
      
      QStringList   files;

      void          editor_msg( QString color, QString msg );

      QString       pkg_dir;
      QString       submitted_dir;
      QString       completed_dir;
      QString       tmp_dir;

      QString       cluster_id;
      QString       cluster_pw;
      QString       submit_url;
      QString       submit_url_host;
      QString       submit_url_port;
      QString       stage_url;
      QString       stage_url_path;
      QString       stage_path;

      QString       errormsg;
      bool          disable_updates;

      void          *cluster_window;

      bool          comm_active;
      QString       comm_mode;
      Q3Http         submit_http;

      Q3Ftp          ftp;

      QFile         *ftp_file;
      QString       ftp_url;
      QString       ftp_url_host;
      QString       ftp_url_port;

      unsigned int  update_files( bool set_lv_files = true );
      bool          send_http_get( QString file );
      QString       current_http;
      QString       current_http_response;

      map < Q3ListViewItem *, QString > jobs;
      map < QString, QString >         job_hostname;

      Q3ListViewItem *next_to_process;
      void          get_status();
      void          cancel_selected();
      void          complete_remove();
      void          complete_retrieve();

      bool          system_cmd( QStringList cmd );
      bool          system_proc_active;
      bool          processing_active;
      bool          stopFlag;
      Q3Process      *system_proc;
      bool          schedule_retrieve( QString file );

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

      void http_stateChanged ( int state );
      void http_responseHeaderReceived ( const Q3HttpResponseHeader & resp );
      void http_readyRead ( const Q3HttpResponseHeader & resp );
      void http_dataSendProgress ( int done, int total );
      void http_dataReadProgress ( int done, int total );
      void http_requestStarted ( int id );
      void http_requestFinished ( int id, bool error );
      void http_done ( bool error );

      void ftp_stateChanged ( int state );
      // void ftp_listInfo ( const QUrlInfo & i );
      // void ftp_readyRead ();
      // void ftp_dataTransferProgress ( int done, int total );
      // void ftp_rawCommandReply ( int replyCode, const QString & detail );
      void ftp_commandStarted ( int id );
      void ftp_commandFinished ( int id, bool error );
      void ftp_done ( bool error );

      void system_proc_readFromStdout();
      void system_proc_readFromStderr();
      void system_proc_processExited();
      void system_proc_launchFinished();

   protected slots:

      void closeEvent(QCloseEvent *);
   
};

#ifdef WIN32
# if !defined( QT4 )
  #pragma warning ( default: 4251 )
# endif
#endif

#endif
