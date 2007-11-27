#ifndef US_CPULOAD_H
#define US_CPULOAD_H
#include <qapp.h>
#include <qfont.h>
#include <qwt_thermo.h>
#include <qlabel.h>
#include <iostream>
#include <fstream>
#include <qwt_math.h>
#include <qwt_counter.h>
#include <qpopmenu.h>
#include <qmenubar.h>
#include <qdialog.h>
#include "us_util.h"

//
// A simple first-order lowpass used for averaging
//
class Lowpass
{
public:
    Lowpass();
    
    void setTConst(double t){tconst = t; recalc();}
    void setTSampl(double t){tsampl = t; recalc();}
    void reset(double v) { val = v; }
    double input(double v); 
    double value() {return val;}
    
private:

    void recalc();
    double val;
    double tsampl;
    double tconst;
    double c1;
    double c2;
    
};

//
//	The main window
//
class US_EXTERN CpuloadWin : public QFrame
{
    Q_OBJECT
   public:

    enum { CpuUser, CpuNice, CpuSystem, CpuIdle, ThermoCnt };
    QwtThermo *th[ThermoCnt];
    QLabel  *lb[ThermoCnt];
    Lowpass lp[ThermoCnt];

    unsigned long val[ThermoCnt];
    unsigned long old[ThermoCnt];
    
    int dynscale;
    int cputmrID;
    
public:
		US_Config *USglobal;

    CpuloadWin(QWidget *p = 0, const char *name = 0);
    ~CpuloadWin();
    void start();
    void update();
    void read();
    int setTimer(int ms);

protected:

    void timerEvent(QTimerEvent *e);

    public slots:

    void setTSampl(double sec);
    void setTConst(double sec);
};
#endif

