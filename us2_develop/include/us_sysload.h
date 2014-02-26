#ifndef US_SYSLOAD_H
#define US_SYSLOAD_H
#include <qapp.h>
#include <qfont.h>
#include <qwt_thermo.h>
#include <qlabel.h>
#include <iostream>
#include <fstream>
#include <qwt_math.h>
#include "us_extern.h"

class US_EXTERN SysloadWin : public QWidget 
{
public:

    QwtThermo *t1;
    QwtThermo *t2;
    QwtThermo *t3;
    QLabel  *l1;
    QLabel  *l2;
    QLabel  *l3;
    int tmrID;
    SysloadWin();
    ~SysloadWin();
    void start();
    void update();

protected:

    void timerEvent(QTimerEvent *e);
    
};
#endif

