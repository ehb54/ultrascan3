#ifndef US_MEMINFO_H
#define US_MEMINFO_H
#include <qapp.h>
#include <qfont.h>
#include <qwt_thermo.h>
#include <qlabel.h>
#include <iostream>
#include <fstream>
#include <qwt_math.h>
#include "us_util.h"

class US_EXTERN MeminfoWin : public QWidget 
{
public:

    enum {MemUsed, MemShared, MemBuffers, MemCached, SwapUsed, ThermoCnt };
    QwtThermo *th[ThermoCnt];
    QLabel  *lb[ThermoCnt];
    int memtmrID;
   US_Config *USglobal;
    
public:

    MeminfoWin();
    ~MeminfoWin();
    void start();
    void update();
    
protected:

    void timerEvent(QTimerEvent *e);
    
};
#endif

