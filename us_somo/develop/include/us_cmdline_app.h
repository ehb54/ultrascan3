#ifndef US_CMDLINE_APP
#define US_CMDLINE_APP

#include <iostream>

#include "qapplication.h"
#include "qobject.h"
#include "qprocess.h"
#include "qstring.h"
#include "qstringlist.h"
#include "qtimer.h"

using namespace std;

// US_Cmdline_App
// runs a program (specified as args[ 0 ])
// when text matches line of app_text,
// response is set to value in response at matching position
// order is sequential, so if a question is repeated, it must
// also be repeated in app_text
// lines of app_text will be skipped looking for next response
// first line of app_text will always wait
// response of ___run___ will simply continue the process until termination

// this is designed to be run under nonGUI mode
// and to support various external applications
// that exist only in binary format requiring command line &
// possibly some sequence of query/response

class US_Cmdline_App : public QObject {
  Q_OBJECT

 public:
  US_Cmdline_App(
      QApplication* qa, QString dir, QStringList args, QStringList app_text,
      QStringList response, QString* error_msg, int timer_delay_ms = 0
      //                     QStringList  * stdout         = (QStringList *)0,
      //                     QStringList  * stderr         = (QStringList *)0
  );
  ~US_Cmdline_App();
  QString* error_msg;

 private slots:
  void readFromStdout();
  void readFromStderr();
  void finished(int, QProcess::ExitStatus);
  void started();

  void timeout();

 private:
  QProcess process;
  QApplication* qa;
  QString dir;
  QStringList args;
  QStringList app_text;
  QStringList response;
  // QStringList  * stdout;
  // QStringList  * stderr;

  int query_response_pos;
  bool run_to_end;

  QTimer timer;
  int timer_delay_ms;
};

#endif

/* --------------------  example usage snippets -----------------------------

   // try a quick little app
   {
      int argc = 0;

      QApplication app( argc, argv, false );
      QStringList args;
      args << "ls" << "-l";
      QStringList empty;
      QString error_msg;
      US_Cmdline_App cla(
                         &app,
                         "",
                         args,
                         empty,
                         empty,
                         &error_msg
                         );
      if ( error_msg.isEmpty() )
      {
         app.exec();
      }
      cout << "back from app\n";
      cout << QString( "app's error msg: <%1>\n" ).arg( error_msg );
   }

   // something more sophisticated
   {
      int argc = 0;

      QApplication app( argc, argv, false );
      QStringList args;
      args << "./test.sh";

      QStringList app_text;
      app_text
         << "please respond"
         << "how about this"
         << "what about this"
         << "and j is"
         ;

      QStringList response;
      response
         << "hey joe"
         << "crazy now"
         << "crazy now2"
         << "___run___"
         ;

      QString error_msg;
      US_Cmdline_App cla(
                         &app,
                         "",
                         args,
                         app_text,
                         response,
                         &error_msg,
                         2000
                         );
      cout << "cla made\n" << flush;

      if ( error_msg.isEmpty() )
      {
         app.exec();
      }
      cout << "back from app\n";
      cout << QString( "app's error msg: <%1>\n" ).arg( error_msg );
   }

   // now test incomplete lines
   {
      int argc = 0;
      cout << "start test.pl\n";

      QApplication app( argc, argv, false );
      QStringList args;
      args
         // stdbuf is used to adjust applications buffering in linux
         // perl seems to do its own buffering regardless (?)
         << "stdbuf"
         << "-o0"
         << "-i0"
         << "./test.pl"
         ;

      QStringList app_text;
      app_text
         << "please respond"
         << "how about this"
         << "what about this"
         << ""
         ;

      QStringList response;
      response
         << "waka"
         << "ja now"
         << "jawaka"
         << "___run___"
         ;

      QString error_msg;
      US_Cmdline_App cla(
                         &app,
                         "",
                         args,
                         app_text,
                         response,
                         &error_msg
                         );
      if ( error_msg.isEmpty() )
      {
         app.exec();
      }
      cout << "back from app\n";
      cout << QString( "app's error msg: <%1>\n" ).arg( error_msg );
   }

   // now dammin test
   {
      int argc = 0;
      cout << "start dammin\n";

      QApplication app( argc, argv, false );
      QStringList args;
      args
         // stdbuf is used to adjust applications buffering in linux
         // perl seems to do its own buffering regardless (?)
         << "dammin"
         ;

      QStringList app_text;
      app_text
         << "[E]xpert"
         << "Log file name"
         << "Input data, GNOM output file name"
         << "Enter project description"
         << "Angular units in the input file:"
         << "Portion of the curve to be fitted"
         << "Initial DAM: type S for sphere [default]"
         << "Symmetry: P1"
         << "Expected particle shape: <P>rolate, <O>blate"
         << "====  Simulated annealing procedure started  ===="
         ;

      QStringList response;
      response
         << "F"
         << "lyzexp"
         << "lyzexp"
         << "my project"
         << "1"
         << ""
         << ""
         << ""
         << ""
         << "___run___"
         ;

      QString error_msg;
      US_Cmdline_App cla(
                         &app,
                         "",
                         args,
                         app_text,
                         response,
                         &error_msg,
                         2000
                         );
      if ( error_msg.isEmpty() )
      {
         app.exec();
      }
      cout << "back from app\n";
      cout << QString( "app's error msg: <%1>\n" ).arg( error_msg );
   }

   ----------------------------------------------------------------------------------
 */
