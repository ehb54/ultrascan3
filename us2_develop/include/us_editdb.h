#ifndef US_EDITDB_H
#define US_EDITDB_H
#include <qlayout.h>
#include "us_edit.h"

class US_EXTERN US_Edit_DB : public EditData_Win
{
   Q_OBJECT

   public:
      US_Edit_DB(QWidget *p = 0, const char *name = 0);
      ~US_Edit_DB();
      
      struct experiment_type exp_type;
   
      QCheckBox *cb_data_if;
      QCheckBox *cb_data_uv;
      QCheckBox *cb_data_fl;
      QCheckBox *cb_data_equil;
      QCheckBox *cb_data_veloc;
      QCheckBox *cb_data_diff;
      QCheckBox *cb_data_wv;
      QCheckBox *cb_data_intensity;   
      
   public slots:
      void help();

      void plot_dataset();
      void review();
      void load_dataset();
      void update_data_uv();
      void update_data_if();
      void update_data_fl();
      void update_data_veloc();
      void update_data_equil();
      void update_data_diff();
      void update_data_intensity();
      void update_data_wv();
      void set_plot_axes();
   
   protected slots:
      void setup_GUI();
   
   signals:
   
      void variablesUpdated();

};

#endif
