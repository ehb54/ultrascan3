#ifndef US_HYDRODYN_VDW_OVERLAP_H
#define US_HYDRODYN_VDW_OVERLAP_H

#include "us_hydrodyn_misc.h"

using namespace std;

class US_EXTERN US_Hydrodyn_Vdw_Overlap : public QFrame
{
   Q_OBJECT

   public:
      US_Hydrodyn_Vdw_Overlap(struct misc_options *, bool *, void *, QWidget *p = 0, const char *name = 0);
      ~US_Hydrodyn_Vdw_Overlap();

   public:

      struct misc_options *misc;
      bool *misc_widget;
                void *us_hydrodyn;

      US_Config *USglobal;
#ifndef NO_DB
      US_Vbar_DB *vbar_dlg;
#endif

      QLabel *lbl_info;

      QPushButton *pb_help;
      QPushButton *pb_cancel;

      QLabel    *lbl_vdw_ot_mult;
      QLineEdit *le_vdw_ot_mult;

      QLabel    *lbl_vdw_ot_dpct;
      QLineEdit *le_vdw_ot_dpct;

      QCheckBox *cb_vdw_ot_alt;
      QCheckBox *cb_vdw_saxs_water_beads;
      QCheckBox *cb_vdw_saxs_skip_pr0pair;

   private slots:

      void setupGUI();

      void update_vdw_ot_mult(const QString &);
      void update_vdw_ot_dpct(const QString &);

      void set_vdw_ot_alt();
      void set_vdw_saxs_water_beads();
      void set_vdw_saxs_skip_pr0pair();

      void cancel();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
};

#endif

