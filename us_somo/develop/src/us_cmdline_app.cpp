#include "../include/us_cmdline_app.h"

US_Cmdline_App::US_Cmdline_App(QApplication* qa, QString dir, QStringList args,
                               QStringList app_text, QStringList response,
                               QString* error_msg, int timer_delay_ms
                               // QStringList  * stdout,
                               // QStringList  * stderr
) {
  this->qa = qa;
  this->dir = dir;
  this->args = args;
  this->app_text = app_text;
  this->response = response;
  this->error_msg = error_msg;
  this->timer_delay_ms = timer_delay_ms;
  // this->stdout         = stdout;
  // this->stderr         = stderr;

  *error_msg = "";

  if (!args.size() || args[0].isEmpty()) {
    *error_msg += QString("Error: an application name must be specified\n");
    qa->quit();
    return;
  }

  if (app_text.size() != response.size()) {
    *error_msg += QString("Error: query/response size mismatch %1 vs %2\n")
                      .arg(app_text.size())
                      .arg(response.size());
    qa->quit();
    return;
  }

  // process = new QProcess( this );

  if (!dir.isEmpty()) {
    process.setWorkingDirectory(dir);
  }

  connect(&process, SIGNAL(readyReadStandardOutput()), this,
          SLOT(readFromStdout()));
  connect(&process, SIGNAL(readyReadStandardError()), this,
          SLOT(readFromStderr()));
  connect(&process, SIGNAL(processExited()), this, SLOT(processExited()));
  connect(&process, SIGNAL(started()), this, SLOT(started()));

  connect(&timer, SIGNAL(timeout()), this, SLOT(timeout()));

#if QT_VERSION < 0x040000
  process.setArguments(args);
#else
  QString prog = args.front();
  args.pop_front();
#endif

  query_response_pos = 0;
  run_to_end = false;

#if QT_VERSION < 0x040000
  if (!process.start())
#else
  process.start(prog, args);
  if (!process.waitForStarted())
#endif
  {
    // us_qdebug( "error starting" );
    *error_msg += QString("Error: could not start process: %1\n").arg(args[0]);
    process.kill();
    qa->quit();
    qa->processEvents();
    return;
  }
  // us_qdebug( QString( "starting process: %1" ).arg( args[ 0 ] ) );
}

US_Cmdline_App::~US_Cmdline_App() { process.kill(); }

void US_Cmdline_App::timeout() {
  *error_msg += QString("Error: out of responses to queries (timeout)\n");
  // us_qdebug( "timeout" );
  process.kill();
}

void US_Cmdline_App::readFromStdout() {
  timer.stop();
  // us_qdebug( "readFromStdout()" );
  QString qs;
  QString text;
#if QT_VERSION < 0x040000
  do {
    qs = process.readLineStdout();
    // if ( stdout )
    // {
    //    *stdout << qs;
    // }
    text += qs + "\n";
  } while (qs != QString());

  do {
    QString read = process.readStdout();
    // if ( stdout )
    // {
    //    *stdout << qs;
    // }
    qs = QString("%1").arg(read);
    text += qs;
  } while (qs.length());
#else
  text = QString(process.readAllStandardOutput());
#endif

  // us_qdebug( QString( "received <%1>" ).arg( text ) );

  if (!run_to_end && app_text.size()) {
    // if not at first entry, read data to find match
    int previous_pos = query_response_pos;
    while ((int)app_text.size() > query_response_pos &&
           !text.contains(app_text[query_response_pos]) && query_response_pos) {
      query_response_pos++;
    }
    if (query_response_pos >= (int)app_text.size()) {
      query_response_pos = previous_pos;
      if (timer_delay_ms) {
        // us_qdebug( QString( "starting timer for %1 seconds" ).arg( ( double
        // )timer_delay_ms / 1000e0 ) );
        timer.start(timer_delay_ms);
      } else {
        *error_msg += QString("Error: out of responses to queries\n");
        process.kill();
        qa->quit();
      }
      return;
    }

    // do we have a match?
    if ((int)app_text.size() > query_response_pos &&
        text.contains(app_text[query_response_pos])) {
      // us_qdebug( QString( "received <%1> from application" ).arg( app_text[
      // query_response_pos ] ) );
      if (response[query_response_pos] != "___run___") {
        if (response[query_response_pos].left(2).contains("__")) {
          *error_msg +=
              QString("Error: undefined response <%1> to query <%2>\n")
                  .arg(response[query_response_pos])
                  .arg(app_text[query_response_pos]);
          process.kill();
          qa->quit();
          return;
        }
        // us_qdebug( QString( "sent     <%1> to application"   ).arg( response[
        // query_response_pos ] ) );
#if QT_VERSION < 0x040000
        process.writeToStdin(response[query_response_pos] + "\n");
#else
        {
          QByteArray qba =
              QString(response[query_response_pos] + "\n").toUtf8();
          process.write(qba.constData(), qba.size());
        }
#endif
        query_response_pos++;
      } else {
        // us_qdebug( "now run to end of application" );
        run_to_end = true;
      }
    }
  }
}

void US_Cmdline_App::readFromStderr() {
  // us_qdebug( "readFromStderr()" );
  // if ( stderr )
  // {
  //    while ( process.canReadLineStderr() )
  //    {
  //       *stderr << process.readLineStderr();
  //    }
  // }
}

void US_Cmdline_App::finished(int, QProcess::ExitStatus) {
  // us_qdebug( "finished( int, QProcess::ExitStatus )" );

#if QT_VERSION < 0x040000
  if (!process.normalExit())
#else
  if (process.exitCode() != QProcess::NormalExit)
#endif
  {
    *error_msg += "Error: process did not exit normally\n";
  }

  if (process.exitStatus()) {
    *error_msg +=
        QString("Error: exit status non-zero: %1\n").arg(process.exitStatus());
  }

  qa->quit();
}

void US_Cmdline_App::started() {
  // us_qdebug( "started()" );
}
