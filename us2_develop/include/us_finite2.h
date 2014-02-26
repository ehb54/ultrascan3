#ifndef US_FINITE2_H
#define US_FINITE2_H

#include "us_util.h"

#include <qfiledialog.h>
#include <qlistbox.h>
#include <qwt_counter.h>

class US_EXTERN US_SetModel : public QDialog
{

   Q_OBJECT

   public:

      US_SetModel(unsigned int *temp_components, int *temp_model, 
      QWidget *p = 0, const char *name = 0);
      ~US_SetModel();
      US_Config *USglobal;
      unsigned int *components;
      int *model;
      QLabel *lbl_model;
      QListBox *lb_model;
      QLabel *lbl_comp;
      QwtCounter *ct_comp;
      QPushButton *pb_cancel;
      QPushButton *pb_help;
      QPushButton *pb_ok;

   private slots:

      void assign_component(double);
      void select_model(int);
      void help();
};
#endif

