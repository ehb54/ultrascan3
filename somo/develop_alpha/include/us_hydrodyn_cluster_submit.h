#ifndef US_HYDRODYN_CLUSTER_SUBMIT_H
#define US_HYDRODYN_CLUSTER_SUBMIT_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <q3frame.h>
#include <qcheckbox.h>
#include <q3listview.h>
#include <q3process.h>
#include <q3cstring.h>
#include <qiodevice.h>
#include <q3progressbar.h>
//Added by qt3to4:
#include <QCloseEvent>

#include "us_util.h"
#include "us_hydrodyn_pdbdefs.h"
#include "us_hydrodyn_batch.h"

#include <q3http.h>
#include <q3ftp.h>

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
      Q3ListView     *lv_files;

      QLabel        *lbl_systems;
      Q3ListBox      *lb_systems;

      QPushButton   *pb_select_all;
      QPushButton   *pb_remove;
      QPushButton   *pb_submit;

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
      QString       tmp_dir;

      QString       cluster_id;
      QString       cluster_pw;
      QString       submit_url;
      QString       submit_url_host;
      QString       submit_url_port;
      QString       stage_url;
      QString       stage_url_path;
      QString       stage_path;

      QFile         *ftp_file;
      QString       ftp_url;
      QString       ftp_url_host;
      QString       ftp_url_port;

      QString       errormsg;
      bool          disable_updates;

      unsigned int  update_files( bool set_lv_files = true );

      bool          prepare_stage( QString file );
      bool          stage( QString file );

      void          *cluster_window;

      QStringList   last_stdout;
      QStringList   last_stderr;
      bool          submit_xml( QString file, QString &xml );
      bool          send_http_post( QString xml );

      bool          submit_active;
      bool          comm_active;
      Q3Http         submit_http;

      Q3Ftp          ftp;

      QString       current_xml;
      QString       current_xml_response;

      QString       current_http;
      QString       current_http_response;

      // here's the submit logic:
      // submit() builds a map of jobs, sets submit active
      // processing stages: stage, submit, move
      // any_to_process checks the map and updates status
      // system() commands run with signals, using emit()
#ifdef WIN32
# if !defined( QT4 )
  #pragma warning ( disable: 4251 )
# endif
#endif
      map < Q3ListViewItem *, QString > jobs;

      map < QString, QString >         selected_system;
#ifdef WIN32
# if !defined( QT4 )
  #pragma warning ( default: 4251 )
# endif
#endif
      QString       selected_system_name;

      Q3ListViewItem *next_to_process;

      bool          job_submit( QString file );
      void          process_list();
      bool          move_file( QString file );

      bool          system_cmd( QStringList cmd );

      bool          system_proc_active;
      Q3Process      *system_proc;

   private slots:

      void systems();

      void process_next();
      void process_prepare_stage();
      void process_stage();
      void process_submit();
      void process_move();

      void setupGUI();
   
      void update_enables();
      void select_all();
      void remove();
      void submit();

      void clear_display();
      void update_font();
      void save();

      void stop();
      void cancel();
      void help();

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

#endif
