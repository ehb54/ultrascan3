#ifndef US_HYDRODYN_GRID_H
#define US_HYDRODYN_GRID_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qwt_counter.h>
#include "us_util.h"
#include "us_hydrodyn_overlap.h"

//standard C and C++ defs:

#include <iostream>

using namespace std;

struct grid_options
{
   bool cubic;    // apply cubic grid
   bool hydrate;    // true: hydrate model
   bool center;    // true: center of mass
                  // false: center of cubelet
   bool tangency;   // true: Expand beads to tangency
   double cube_side; 
   bool enable_asa;   // true:-> run asa buried/exposed code
};

class US_EXTERN US_Hydrodyn_Grid : public QFrame
{
   Q_OBJECT

   public:
      US_Hydrodyn_Grid(struct overlap_reduction *,
                       struct overlap_reduction *,
                       struct overlap_reduction *,
                       struct grid_options *,
                       double *, 
                       bool *, 
                       void *, 
                       QWidget *p = 0, 
                       const char *name = 0);
      ~US_Hydrodyn_Grid();

   public:
      
      struct grid_options *grid;
      struct overlap_reduction *grid_exposed_overlap;
      struct overlap_reduction *grid_buried_overlap;
      struct overlap_reduction *grid_overlap;
      bool *grid_widget;
      bool overlap_widget;
      double *overlap_tolerance;
      US_Hydrodyn_Overlap *overlap_window;
      
      QwtCounter *cnt_cube_side;
      QButtonGroup *bg_center;
      QCheckBox *cb_cubic;
      QCheckBox *cb_hydrate;
      QCheckBox *cb_center_mass;
      QCheckBox *cb_center_cubelet;
      QCheckBox *cb_tangency;
      QCheckBox *cb_enable_asa;
      void *us_hydrodyn;
      
      US_Config *USglobal;
      
      QLabel *lbl_info;
      QLabel *lbl_cube_side;

      QPushButton *pb_overlaps;
      QPushButton *pb_help;
      QPushButton *pb_cancel;

   private slots:
      
      void setupGUI();
      void update_cube_side(double);
      void select_center(int);
      void set_hydrate();
      void set_tangency();
      void set_enable_asa();
      void set_cubic();
      void overlaps();
      void cancel();
      void help();
   
   protected slots:

      void closeEvent(QCloseEvent *);

};



#endif

