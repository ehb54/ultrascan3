#ifndef SELECT_PROGRAM_H
#define SELECT_PROGRAM_H

#include <us_widgets_dialog.h>

enum indent{S_REF_SCAN, S_ABS};

class selectProgram : public US_WidgetsDialog
{
    Q_OBJECT
public:
    selectProgram(int& st);

private:
    QPushButton* pb_refScan;
    QPushButton* pb_pAbs;
    QPushButton* pb_close;
    QPushButton* pb_open;
    QColor color;
    int *choice;
    void turn_on(QPushButton *button, bool status);

private slots:
    void exit(void);
    void select_refScan(void);
    void select_pAbs(void);
};

#endif
