#include "../include/us_sysload.h"
#include <fstream>
using namespace std;

//---------------------------------------
// SysloadWin::start()
//
//  initialize thermometers,
//  start timer
//---------------------------------------
void SysloadWin::start()
{
    update();
    tmrID = startTimer(2000);
}


//---------------------------------------
// SysloadWin::update()
//
//	read values from /proc/loadavg
//  and display them. Adjust thermometer scales
//  if necessary.
//---------------------------------------
void SysloadWin::update()
{
    double v1, v2, v3;
    static double vmin = 0.1;
    double vmax, vnew;
    
    ifstream inp("/proc/loadavg");
    inp >> v1 >> v2 >> v3;

    // adjust and synchronize ranges
    vmax = qwtCeil125(qwtMax(qwtMax(qwtMax(v1,v2),v3),vmin));

    if ( (vmax > t1->maxValue()) 
	|| (vmax < 0.25 * t1->maxValue()) )
    {
	vnew = vmax;
	t1->setRange(0.0, vnew);
	t2->setRange(0.0, vnew);
	t3->setRange(0.0, vnew);
    }

    // set values
    t1->setValue(v1);
    t2->setValue(v2);
    t3->setValue(v3);
    
}

//---------------------------------------
// SysloadWin::timerEvent
//
// 	update thermomoters
//---------------------------------------
void SysloadWin::timerEvent(QTimerEvent *)
{
    update();
}


SysloadWin::SysloadWin()
: QWidget()
{
//    QColor cFill("DarkGreen");

    t1 = new QwtThermo(this,"");
    t2 = new QwtThermo(this,"");
    t3 = new QwtThermo(this,"");
    l1 = new QLabel(tr("1 min"), this);
    l2 = new QLabel(tr("10 min."), this);
    l3 = new QLabel(tr("15 min."), this);

    t1->setGeometry(10,10,50,100);
    t2->setGeometry(70,10,50,100);
    t3->setGeometry(130,10,50,100);
    l1->setGeometry(10,115,50,10);
    l2->setGeometry(70,115,50,10);
    l3->setGeometry(130,115,50,10);

    t1->setOrientation(QwtThermo::Vertical, QwtThermo::Right);
    t1->setRange(0.0,1.0);
    t1->setValue(1.0);
    t1->setBorderWidth(1);
    t1->setPipeWidth(4);
    t1->setFont(QFont("Helvetica",10));
    t1->setScaleMaxMajor(6);
    t1->setFillColor(QColor("DarkRed"));

    t2->setOrientation(QwtThermo::Vertical, QwtThermo::Right);
    t2->setRange(0.0,1.0);
    t2->setValue(1.0);
    t2->setBorderWidth(1);
    t2->setPipeWidth(4);
    t2->setFont(QFont("Helvetica",10));
    t2->setScaleMaxMajor(6);
    t2->setFillColor(QColor("DarkGreen"));

    t3->setOrientation(QwtThermo::Vertical, QwtThermo::Right);
    t3->setRange(0.0,1.0);
    t3->setValue(1.0);
    t3->setBorderWidth(1);
    t3->setPipeWidth(4);
    t3->setFont(QFont("Helvetica",10));
    t3->setScaleMaxMajor(6);
    t3->setFillColor(QColor("DarkBlue"));

    l1->setText(tr("1 min."));
    l2->setText(tr("5 min."));
    l3->setText(tr("15 min."));

    l1->setAlignment(AlignLeft|AlignVCenter);
    l2->setAlignment(AlignLeft|AlignVCenter);
    l3->setAlignment(AlignLeft|AlignVCenter);
    
    tmrID = 0;

    setFixedSize(185,130);

}


SysloadWin::~SysloadWin()
{
    delete t1;
    delete t2;
    delete t3;
    delete l1;
    delete l2;
    delete l3;

    if (tmrID != 0) killTimer(tmrID);
}

/* Sample invocation:

int main (int argc, char **argv)
{
    QApplication a(argc, argv);

    SysloadWin w;
    a.setMainWidget(&w);

    w.setCaption("load average");
    w.start();
    w.show();
    
    return a.exec();
}
*/



