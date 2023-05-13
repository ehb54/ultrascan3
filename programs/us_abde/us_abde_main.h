#ifndef US_ABDE_MAIN_H
#define US_ABDE_MAIN_H

#include <us_widgets.h>
#include "../us_buoyancy/us_buoyancy.h"
#include "us_norm_profile.h"

class US_ABDE_MW : public US_Widgets
{
    Q_OBJECT
public:
    US_ABDE_MW();

private slots:
    void sel_fitter(void);
    void sel_normalizer(void);
    void turn_on(QPushButton*,bool);

private:
    US_Norm_Profile *normalizer;
    US_Buoyancy *fitter;
    QPushButton *pb_fitter;
    QPushButton *pb_normalizer;
    QVBoxLayout *main_lyt;
    int prog_state;
    QLabel* hline;

};

#endif // US_ABDE_MAIN_H
