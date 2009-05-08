#ifndef US_EDVFLO_H
#define US_EDVFLO_H
#include "us_edit.h"


class US_EXTERN EditFloVeloc_Win : public EditData_Win
{

   public:
      EditFloVeloc_Win(QWidget *p = 0, const char *name = 0);
      ~EditFloVeloc_Win();
   void help();
   void get_x(const QMouseEvent &e);
   
   private slots:
      void setup_GUI();
};

#endif

