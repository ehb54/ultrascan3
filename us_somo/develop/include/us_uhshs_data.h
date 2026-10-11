#ifndef US_UHSHS_DATA_H
#define US_UHSHS_DATA_H

// Shared by us_hydrodyn_{dad,mals,mals_saxs,saxs_hplc}_simulate.h, which each
// used to carry their own identical copy of this struct.

#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>

struct uhshs_data {
   QLabel     * lbl_name;

   QLabel     * lbl_i_mult;
   QLineEdit  * le_i_mult;

   QLabel     * lbl_center;
   QLineEdit  * le_center;

   QLabel     * lbl_width;
   QLineEdit  * le_width;

   QCheckBox  * cb_alpha;
   QLineEdit  * le_alpha;
};

#endif
