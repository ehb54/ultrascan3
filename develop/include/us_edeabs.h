#ifndef US_EDEABS_H
#define US_EDEABS_H
#include "us_edit.h"


class US_EXTERN EditAbsEquil_Win : public EditData_Win
{

   public:
      EditAbsEquil_Win(QWidget *p = 0, const char *name = 0);
      ~EditAbsEquil_Win();
      void help();
      void get_x(const QMouseEvent &e);
   
   private slots:
      void setup_GUI();
};

#endif

