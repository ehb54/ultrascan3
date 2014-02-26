#ifndef US_EDEINT_H
#define US_EDEINT_H
#include "us_edit.h"


class US_EXTERN US_EditInterferenceEquil : public EditData_Win
{

   public:
      US_EditInterferenceEquil(QWidget *p = 0, const char *name = 0);
      ~US_EditInterferenceEquil();
      void help();
      void get_x(const QMouseEvent &e);
      unsigned int current_scan;
      float left_bracket;
   
   private slots:
      void setup_GUI();
};

#endif

