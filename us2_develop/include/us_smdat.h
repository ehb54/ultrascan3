#ifndef US_SMDAT_H
#define US_SMDAT_H


#include "us_editor.h"
#include "us_util.h"
#include "us_pixmap.h"
#include "us_dtactrl.h"

#include <qprogbar.h>

class US_EXTERN sm_dat_W : public Data_Control_W
{
   Q_OBJECT
   
   public:
   
      sm_dat_W(QWidget *p = 0, const char *name = 0);
      ~sm_dat_W();
      US_Editor *e;

   private:

      US_Pixmap *pm;
      double *smp;
      double *sms;
      double avg;
      double *plot1_x;
      double *plot1_y;
      double *plot2_x;
      double *plot2_y;
      double *plot3_x;
      double *plot3_y;
      uint plot1;
      uint plot2;
      uint plot3;
      unsigned int comp, divisions, count, exclude;

   private slots:

      void setup_GUI();
      void write_sm();
      void write_res();

// re-implemented Functions:

      int plot_analysis();
      int setups();
      void view();
      void help();
      void save();
      void second_plot();
};

#endif

