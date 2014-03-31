#ifndef US_MQT_H
#define US_MQT_H

#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qthread.h>
#include <q3listbox.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QFocusEvent>
#include <vector>
#include <qcolor.h>
#include <math.h>

class mQLabel : public QLabel
{
   Q_OBJECT

   public:

      mQLabel ( QWidget *parent = 0 , const char * name = 0 );
      mQLabel ( const QString & text, QWidget *parent = 0 , const char * name = 0 );
      ~mQLabel();

   signals:
      void pressed();

   protected:
      virtual void mousePressEvent ( QMouseEvent *e );
};

class mQLineEdit : public QLineEdit
{
   Q_OBJECT

   public:

      mQLineEdit ( QWidget *parent = 0 , const char * name = 0 );
      ~mQLineEdit();

   signals:
      void focussed(bool hasFocus);

   protected:
      virtual void focusInEvent ( QFocusEvent *e );
      virtual void focusOutEvent ( QFocusEvent *e );
};

class mQPushButton : public QPushButton
{
   Q_OBJECT

   public:

      mQPushButton ( QWidget *parent = 0 , const char * name = 0 );
      mQPushButton ( const QString & text, QWidget *parent = 0 , const char * name = 0 );
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
   static void hide_widgets( std::vector < QWidget *> widgets, bool hide = true, QWidget * do_resize = (QWidget *) 0 );
};

class MQT
{
 public:
   static QStringList get_lb_qsl( Q3ListBox * lb, bool only_selected = false );
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
