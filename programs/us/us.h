//! \file us.h
#if QT_VERSION > 0x050000
#include <QtWidgets>
#else
#include <QtGui>
#endif

#include "us_global.h"
#include "us_help.h"
#include "us_geturl.h"

//! \brief UltraScan initial screen

class US_Win : public QMainWindow
{
  Q_OBJECT

public:

  /*! \brief Launch Qt Assistant and set the page to show.
  
      \param parent  A pointer to the parent widget.  This normally can be 
                     left as NULL.
      \param flags   An enum value that describes the window properties. 
                     This normally can be left as 0 (Qt::Widget).
  */
  US_Win( QWidget* = 0, Qt::WindowFlags = 0 );
  
  //! Destructor for US_Win.  Resets position in global memory. 
  ~US_Win();

private:
  US_Global  g;
  QLabel*    bigframe;
  QLabel*    smallframe;
  QLabel*    splash_shadow;
  US_Help    showhelp;
  QDateTime  ln_time;                // Last notice-check time
  
  void addMenu( int, const QString&, QMenu* );
  void splash ( void );
  void logo   ( int );
  void help   ( int );
  void launch ( int );
  void apply_prefs ( void );

  struct procData
  {
    QProcess* proc;
    QString   name;
    int       index;
  };

  QList<procData*> procs;

  US_GetUrl notices_get_url;

private slots:
  void onIndexTriggered( int index );
  void closeSplash     ( void );
  void terminated      ( int, QProcess::ExitStatus );
  void closeEvent      ( QCloseEvent* );
  void closeProcs      ( void );
  void notices_ready   ();
};

//////////////


/*! \brief A subclass of QAction to allow sending an integer when a
           menu item is selected.
*/
class US_Action : public QAction
{
  Q_OBJECT

public:

  /*! \brief Create a new QAction and connect the action to the 
             private slot onTriggered.
      \param i      The index number to send when triggered.
      \param text   The menu label to be displayed.
      \param parent A pointer to the parent menu.
  */
  US_Action( int, const QString&, QObject* );

signals:
  //! A signal to send with the index of the menu selected.
  void indexTriggered( int );

private slots:
  //! A function to accept a normal QAction signal and emit \ref indexTriggered.
  // \param bool The parameter sent by the QAction object is ignored.
  void onTriggered( bool );

private:
  int  index;
};

