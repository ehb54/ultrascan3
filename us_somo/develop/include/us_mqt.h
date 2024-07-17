#ifndef US_MQT_H
#define US_MQT_H

#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qthread.h>
#include <qlistwidget.h>
//#include <q3grid.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <QMouseEvent>
#include <QFocusEvent>
#include <vector>
#include <set>
#include <qcolor.h>
#include <math.h>
#include <qwt_plot.h>
#include <qprogressbar.h>
#include <qtextedit.h>

class mQTextEdit : public QTextEdit
{
   Q_OBJECT

   public:

     mQTextEdit ( QWidget *parent = 0 );
     ~mQTextEdit();

     void set_cli_progress( bool & );

   public slots:

     void append        ( const QString & text );
     void set_cli_prefix( QString prefix );

   private:
     bool *        cli_progress;
     QString       cli_prefix;
};

class mQProgressBar : public QProgressBar
{
   Q_OBJECT

   public:

     mQProgressBar ( QWidget *parent = 0 );
     ~mQProgressBar();

     void set_cli_progress( bool & );
     void set_cli_prefix  ( QString prefix );

   public slots:
      
     void setValue        ( int value );

   private:
     bool *        cli_progress;
     QString       cli_prefix;
};

class mQwtPlot : public QwtPlot
{
   Q_OBJECT

   public:

      mQwtPlot ( QWidget *parent = 0 );
      ~mQwtPlot();

   signals:
      void resized();

   protected:
      virtual void resizeEvent ( QResizeEvent *e );
};

class mQLabel : public QLabel
{
   Q_OBJECT

   public:

      mQLabel ( QWidget *parent = 0 );
      mQLabel ( const QString & text, QWidget *parent = 0 );
      ~mQLabel();

   signals:
      void pressed();
      void resized();

   protected:
      virtual void mousePressEvent ( QMouseEvent *e );
      virtual void resizeEvent ( QResizeEvent *e );
};

class mQLineEdit : public QLineEdit
{
   Q_OBJECT

   public:

      mQLineEdit ( QWidget *parent = 0 );
      ~mQLineEdit();

   signals:
      void focussed(bool hasFocus);
      void pressed();

   protected:
      virtual void focusInEvent ( QFocusEvent *e );
      virtual void focusOutEvent ( QFocusEvent *e );
      virtual void mousePressEvent ( QMouseEvent *e );
};

class mQPushButton : public QPushButton
{
   Q_OBJECT

   public:

      mQPushButton ( QWidget *parent = 0 );
      mQPushButton ( const QString & text, QWidget *parent = 0 );
      ~mQPushButton();

   signals:
      void doubleClicked();

   protected:
      virtual void mouseDoubleClickEvent ( QMouseEvent *e );
};

class mQThread : public QThread
{
 public:
   static void sleep(unsigned long secs);
   static void msleep(unsigned long msecs);
   static void usleep(unsigned long usecs);
};

class ShowHide
{
 public:
   static void hide_widgets( const std::set < QWidget *> & widgets, 
                             bool hide = true, 
                             QWidget * do_resize = (QWidget *) 0 );
   static void hide_widgets( const std::vector < QWidget *> & widgets, 
                             bool hide = true, 
                             QWidget * do_resize = (QWidget *) 0 );
   static void hide_widgets( const std::vector < std::vector < QWidget *> > & widgets, 
                             int row,
                             bool hide = true, 
                             QWidget * do_resize = (QWidget *) 0 );
   static void only_widgets( const std::vector < std::vector < QWidget *> > & widgets, 
                             int row,
                             bool hide = true, 
                             QWidget * do_resize = (QWidget *) 0 );
   static void hide_widgets( const std::set < QWidget *> & widgets, 
                             const std::set < QWidget *> & always_hide_widgets,
                             bool hide = true, 
                             QWidget * do_resize = (QWidget *) 0 );
   static void hide_widgets( const std::vector < QWidget *> & widgets, 
                             const std::set < QWidget *> & always_hide_widgets,
                             bool hide = true,
                             QWidget * do_resize = (QWidget *) 0 );
   static void hide_widgets( const std::vector < std::vector < QWidget *> > & widgets, 
                             int row,
                             const std::set < QWidget *> & always_hide_widgets,
                             bool hide = true, 
                             QWidget * do_resize = (QWidget *) 0 );
   static void only_widgets( const std::vector < std::vector < QWidget *> > & widgets, 
                             int row,
                             const std::set < QWidget *> & always_hide_widgets,
                             bool hide = true, 
                             QWidget * do_resize = (QWidget *) 0 );
};

class MQT
{
 public:
   static QStringList get_lb_qsl( QListWidget * lb, bool only_selected = false );
};

// plot colors

class PC
{
 public:

   PC( QColor bgc );
   
   void color_rotate();
   QColor color( int i );
   
 private:
   std::vector < QColor >                  plot_colors;
   void  push_back_color_if_ok( QColor bg, QColor set );
};


#endif
