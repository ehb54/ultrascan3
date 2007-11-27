#include "../include/us_meminfo.h"

void MeminfoWin::start()
{
    update();
    memtmrID = startTimer(2000);
}


void MeminfoWin::update()
{
    double mtotal, mused, mcached, mshared, mbuffers, mfree;
    double stotal, sused, sfree;
    static char buffer[81];
    QString text;	
    ifstream inp("/proc/meminfo");

    inp.getline(buffer, 80);
    inp >> buffer >> mtotal >> mused >> mfree >> mshared >> mbuffers
       >> mcached;
    inp >> buffer >> stotal >> sused >> sfree;

    mtotal *= 0.01;
    stotal *= 0.01;

    mused /= mtotal;
    mshared /= mtotal;
    mbuffers /= mtotal;
    mcached /= mtotal;
    sused /= stotal;
    
    // set values
    th[MemUsed]->setValue(mused);
    th[MemShared]->setValue(mshared);
    th[MemBuffers]->setValue(mbuffers);
    th[MemCached]->setValue(mcached);
    th[SwapUsed]->setValue(sused);

    text.setNum(mused, 'g', 3);
    text = QString(tr("Used: ")) + text + QString(" %"); 
    lb[MemUsed]->setText(text);
    
    text.setNum(mshared, 'g', 3);
    text = QString(tr("Shared: ")) + text + QString(" %"); 
    lb[MemShared]->setText(text);

    text.setNum(mbuffers, 'g', 3);
    text = QString(tr("Used: ")) + text + QString(" %"); 
    lb[MemBuffers]->setText(text);

    text.setNum(mcached, 'g', 3);
    text = QString(tr("Cached: ")) + text + QString(" %"); 
    lb[MemCached]->setText(text);

    text.setNum(sused, 'g', 3);
    text = QString(tr("Swap Used: ")) + text + QString(" %"); 
    lb[SwapUsed]->setText(text);
}


void MeminfoWin::timerEvent(QTimerEvent *)
{
    update();
}



MeminfoWin::MeminfoWin()
: QWidget()
{
	USglobal = new US_Config();

    int i;

    QFont fnThermo(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1);
    QFont fnLabel(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1);
    QColor cFill("DarkMagenta");

    //
    // initialize members
    //
    memtmrID = 0;

    //
    // create widgets
    //
    for(i=0;i<ThermoCnt;i++)		
       th[i] = new QwtThermo(this,"");
    
    lb[MemUsed] = new QLabel(tr("Used"),this);
    lb[MemShared] = new QLabel(tr("Shared"),this);
    lb[MemCached] = new QLabel(tr("Cached"),this);
    lb[MemBuffers] = new QLabel(tr("Buffers"),this);
    lb[SwapUsed] = new QLabel(tr("Used Swap Space"),this);

    //
    // place widgets
    //
    for (i=0; i<ThermoCnt; i++)
    {
	lb[i]->setGeometry(0,i*65,130,20);
	th[i]->setGeometry(0,20 + i * 65 ,130,45);
    }

    //
    // configure thermometers
    //
    for (i=0;i<ThermoCnt;i++)
    {
	th[i]->setOrientation(QwtThermo::Horizontal, QwtThermo::Bottom);
	th[i]->setRange(0.0,100.0);
	th[i]->setValue(0.0);
	th[i]->setFont(fnThermo);
	th[i]->setPipeWidth(6);
	th[i]->setScaleMaxMajor(6);
	th[i]->setScaleMaxMinor(5);
	th[i]->setMargin(10);
	th[i]->setFillColor(cFill);
    }

    //
    // configure labels
    //
    for (i=0;i<ThermoCnt;i++)
    {
	lb[i]->setFont(fnLabel);
	lb[i]->setAlignment(AlignCenter);
    }
    
    setCaption(tr("Memory Usage"));
    setFixedSize(130,325);
}


MeminfoWin::~MeminfoWin()
{
    int i;

    for(i=0;i<ThermoCnt;i++)
    {	
	delete th[i];
	delete lb[i];
    }   
    if (memtmrID != 0) killTimer(memtmrID);
}



