#ifndef US_PSEUDO_ABSORBANCE_H
#define US_PSEUDO_ABSORBANCE_H

#include <us_widgets.h>
#include <us_add_refScan.h>
#include <us_convert_scan.h>

class US_PseudoAbsorbance : public US_Widgets
{
    Q_OBJECT
public:
    US_PseudoAbsorbance();
private slots:
    void select_addRef(void);
    void select_cnvtSc(void);
    void turn_on(QPushButton*,bool);
private:
    US_AddRefScan *addRefScan;
    US_ConvertScan *convertScan;
    QPushButton *pb_addRef;
    QPushButton *pb_cnvtSc;
    QVBoxLayout *main_lyt;
    QString curr_prgm;
    QLabel* hline;


};

#endif // US_PSEUDO_ABSORBANCE_H
