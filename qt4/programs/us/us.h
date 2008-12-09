#include <QtGui>

#include "us_global.h"
#include "us_help.h"

class us_win : public QMainWindow
{
  Q_OBJECT

public:

  us_win( QWidget* parent = 0, Qt::WindowFlags flags = 0 );
  ~us_win();

private:
  US_Global  g;
  QLabel*    bigframe;
  QLabel*    smallframe;
  QLabel*    splash_shadow;
  US_Help    showhelp;
  
  void addMenu( int, const QString&, QMenu* );
  void splash ( void );
  void logo   ( int );
  void help   ( int );
  void launch ( int );

  struct procData
  {
    QProcess* proc;
    QString   name;
    int       index;
  };

  QList<procData*> procs;

private slots:
  void onIndexTriggered( int index );
  void closeSplash     ( void );
  void terminated      ( int, QProcess::ExitStatus );
  void closeEvent      ( QCloseEvent* );
  void closeProcs      ( void );
};

//////////////
class us_action : public QAction
{
  Q_OBJECT

public:
  us_action( int, const QString&, QObject* );

signals:
  void indexTriggered( int );

private slots:
  void onTriggered( bool );

private:
  int  index;
};

