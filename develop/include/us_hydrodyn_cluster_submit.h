#ifndef US_HYDRODYN_CLUSTER_SUBMIT_H
#define US_HYDRODYN_CLUSTER_SUBMIT_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qlistview.h>
#include <qsocket.h>
#include <qprocess.h>
#include <qcstring.h>
#include <qiodevice.h>

#include "us_util.h"
#include "us_hydrodyn_pdbdefs.h"
#include "us_hydrodyn_batch.h"

#include <qhttp.h>

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
      QListView     *lv_files;

      QPushButton   *pb_select_all;
      QPushButton   *pb_remove;
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

      unsigned int  update_files( bool set_lv_files = true );

      bool          prepare_stage( QString file );
      bool          stage( QString file );

      void          *cluster_window;

      QStringList   last_stdout;
      QStringList   last_stderr;
      bool          submit_xml( QString file, QString &xml );
      bool          send_xml( QString xml );

      bool          submit_active;
      bool          comm_active;
      QSocket       submit_socket;
      QHttp         submit_http;

      QString       current_xml;
      QString       current_xml_response;

      // here's the new submit logic:
      // submit() builds a vector of jobs, sets submit active
      // processing stages: stage, submit, move
      // any_to_process checks the vector and updates status
      // system() commands run with signals, using emit()
#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif
      map < QListViewItem *, QString > jobs;
#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif
      QListViewItem *next_to_process;

      bool          job_submit( QString file );
      void          process_list();
      bool          move_file( QString file );

      bool          system_cmd( QStringList cmd );

      bool          system_proc_active;
      QProcess      *system_proc;

   private slots:

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

      void socket_error( int );
      void socket_connected();
      void socket_readyRead();
      void socket_connectionClosed();
      void socket_delayedCloseFinished();

      void system_proc_readFromStdout();
      void system_proc_readFromStderr();
      void system_proc_processExited();
      void system_proc_launchFinished();

   protected slots:

      void closeEvent(QCloseEvent *);
   
};

#endif
