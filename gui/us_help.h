#ifndef US_HELP_H
#define US_HELP_H

#include <QtGui>

class US_Help : public QWidget
{
  Q_OBJECT

  public:
    US_Help( QWidget* parent = 0 ) : QWidget( parent ) {};
    
    ~US_Help(){};

    void show_URL      ( const QString& );
    void show_help     ( const QString& );

  private:
    QProcess* proc;
    QString   URL;
    int       stderrSize;
    int       trials;

    void openBrowser();

  private slots:
    void captureStdout( void );
    void captureStderr( void );
    void endProcess( int, QProcess::ExitStatus );
};
#endif

