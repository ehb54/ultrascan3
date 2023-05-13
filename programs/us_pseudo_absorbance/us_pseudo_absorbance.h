#ifndef US_PSEUDO_ABSORBANCE_H
#define US_PSEUDO_ABSORBANCE_H

#include <us_widgets.h>
#include "us_add_refScan.h"
#include "us_convert_scan.h"
#include "us_remove_ri.h"

class US_PseudoAbsorbance : public US_Widgets
{
    Q_OBJECT
public:
    US_PseudoAbsorbance();
private slots:
    void select_addRef(void);
    void select_cnvtSc(void);
    void select_rmRi(void);
    void turn_on(QPushButton*,bool);

private:
    enum programs{ADDREFSCAN, CONVERT, REMOVERI};
    US_AddRefScan *addRefScan;
    US_ConvertScan *convertScan;
    US_RemoveRI *removeRi;
    QPushButton *pb_addRef;
    QPushButton *pb_cnvtSc;
    QPushButton *pb_rmri;
    QVBoxLayout *main_lyt;
    int program_state;
    QLabel* hline;

    void new_program(int);
    template<class T> bool cls_program(T*);
};

#endif // US_PSEUDO_ABSORBANCE_H
