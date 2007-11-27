#include "../include/us_cpuload.h"
#include <qwt_math.h>

Lowpass::Lowpass()
{
    tsampl = 1.0;
    tconst = 0.0;
    recalc();
    reset(0.0);
}

double Lowpass::input(double v)
{
    val = c2 * v + c1 * val;
    return val;
}

void Lowpass::recalc()
{
    if (tconst > 0.00001)
       c1 = exp(-tsampl / tconst);
    else
       c1 = 0.0;

    c2 = 1.0 - c1;
}


void CpuloadWin::setTSampl(double sec)
{
    int i; 
    int ms = int(sec * 1000.0);

    if(cputmrID != 0) 
       killTimer(cputmrID);

    for(i=0;i<ThermoCnt;i++)
       lp[i].setTSampl(sec);

    cputmrID = startTimer(ms);
}

void CpuloadWin::setTConst(double sec)
{
    int i;
    for(i=0;i<ThermoCnt;i++)
       lp[i].setTConst(sec);
}

void CpuloadWin::read()
{
    static char buffer[20];
    int i;

    ifstream inp("/proc/stat");
    inp >> buffer;
    for(i=0;i<ThermoCnt;i++)
    {
	old[i] =  val[i];
	inp >> val[i];
    }

}


//---------------------------------------
// CpuloadWin::update()
//
//	read values from /proc/loadavg
//  and display them. Adjust thermometer scales
//  if necessary.
//---------------------------------------
void CpuloadWin::update()
{
    unsigned long delta[ThermoCnt];
    unsigned long sum = 0;
    double factor;
    int i;

    read();

    for(i=0;i<ThermoCnt;i++)
    {
	delta[i] = val[i] - old[i];
	sum += delta[i];
    }

    if (sum > 0)
       factor = 100.0 / double(sum);
    else 
       factor = 0.0;
    for(i=0;i<ThermoCnt;i++)
    {
	th[i]->setValue(lp[i].input(double(delta[i]) * factor));		
    }
}

//---------------------------------------
// CpuloadWin::timerEvent
//
// 	update thermometers
//---------------------------------------
void CpuloadWin::timerEvent(QTimerEvent *)
{
    update();
}

CpuloadWin::CpuloadWin(QWidget *p, const char* name)
: QFrame(p, name)
{
    int i;
    QColor cFill("MidnightBlue");
	USglobal = new US_Config();

    for(i=0;i<ThermoCnt;i++)
       lp[i].reset(0);
    
    for(i=0;i<ThermoCnt;i++)
    {
	th[i] = new QwtThermo(this,"");
	th[i]->setRange(0.0,100.0);
	lb[i] = new QLabel(this);
    }

    lb[CpuUser]->setText(tr("% User"));
    lb[CpuNice]->setText(tr("% Nice"));
    lb[CpuSystem]->setText(tr("% System"));
    lb[CpuIdle]->setText(tr(" % Idle"));

    for(i=0;i<ThermoCnt;i++)
    {
	th[i]->setGeometry(10 + i*60 ,30,50,100);
	lb[i]->setGeometry(10 + i*60, 130,50,20);
    }

    for(i=0;i<ThermoCnt;i++)
    {
	th[i]->setOrientation(QwtThermo::Vertical, QwtThermo::Left);
	th[i]->setRange(0.0,100.0);
	th[i]->setValue(0.0);
	th[i]->setBorderWidth(1);
	th[i]->setPipeWidth(4);
	th[i]->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	th[i]->setScaleMaxMajor(6);
	th[i]->setScaleMaxMinor(5);
	th[i]->setFillColor(cFill);

	lb[i]->setAlignment(AlignRight|AlignTop);
    }
    
    cputmrID = 0;
    read();
    setTConst(1.0);
    setTSampl(0.2);
    setFixedSize(10 + ThermoCnt*60 ,150);
}

CpuloadWin::~CpuloadWin()
{
    int i;
    for(i=0;i<ThermoCnt;i++)
    {
	delete th[i];
	delete lb[i];
    }
    if (cputmrID != 0) killTimer(cputmrID);
}



