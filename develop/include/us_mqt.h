#ifndef US_MQT_H
#define US_MQT_H

#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qthread.h>
#include <q3listbox.h>
#include <q3grid.h>
//Added by qt3to4:
#include <QResizeEvent>
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
      void resized();

   protected:
      virtual void mousePressEvent ( QMouseEvent *e );
      virtual void resizeEvent ( QResizeEvent *e );
};

class mQGrid : public Q3Grid
{
   Q_OBJECT

   public:

      mQGrid( int n, QWidget* parent=0, const char* name=0, Qt::WFlags f = 0 );
      mQGrid( int n, Qt::Orientation orient, QWidget* parent=0, const char* name=0, Qt::WFlags f = 0 );

      ~mQGrid();

   signals:
      void resized();

   protected:
      virtual void resizeEvent ( QResizeEvent *e );
};

class mQLineEdit : public QLineEdit
{
   Q_OBJECT

   public:

      mQLineEdit ( QWidget *parent = 0 , const char * name = 0 );
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
