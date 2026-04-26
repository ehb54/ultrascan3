#ifndef US_UHSHS_DATA_H
#define US_UHSHS_DATA_H

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