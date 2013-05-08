#ifndef US_MQLABEL_H
#define US_MQLABEL_H

#include <qlabel.h>

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

#endif
