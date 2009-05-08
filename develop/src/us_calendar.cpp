#include "../include/us_calendar.h"
#include <ctype.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <qevent.h>
#include <qscrbar.h>
#include <qtimer.h>
#include <qdatetm.h>
#include <qapp.h>

#define CALENDAR_WIDTH  260
#define TITLE_HEIGHT    40
#define ROW_HEIGHT      30
#define BLOCK_SIZE      26 // BLOCK_SIZE must be ROW_HEIGHT - 2 <- GAP

#define INITIAL_TIME 750
#define REPEAT_TIME 100


static char* months[]={"January", "February", "March", "April", "May", "June", 
                       "July", "August", "September", "October","November", "December" };
const char* holidayText[] = {
   "New Year's Day",
   "Martin Luther King Day",
   "Washington's Birthday",
   "Good Friday",
   "Easter",
   "Memorial Day",
   "Independence Day",
   "Labor Day",
   "Thanksgiving Day",
   "Christmas Day"
};

struct MonthDay NewYearsDay(int)
{
   struct MonthDay md;
   md.mon=0;
   md.mday=1;
   md.key=_NewYearsDay;
   return md;
}
struct MonthDay MartinLutherKingDay(int year)
{
   struct MonthDay md;
   /* Third monday in January */
#ifdef _DEBUG
   printf("Martin Luther King Day calculation for year %i:\n",year);
#endif
   if (year>1900) year-=1900;
   struct tm thistime;
   thistime.tm_year=year;
   thistime.tm_mon=0;
   thistime.tm_mday=1;
   thistime.tm_hour=thistime.tm_min=thistime.tm_sec=0;
   thistime.tm_wday=thistime.tm_yday=0;
   thistime.tm_isdst=-1;
   time_t time=mktime(&thistime);
   struct tm* ptr=localtime(&time);
   int dtfm=1-ptr->tm_wday;
   if (dtfm<0) dtfm+=7;
   md.mday=dtfm + 15;
   md.mon=0;
   md.key=_MartinLutherKingDay;
#ifdef _DEBUG
   printf("  RESULT month=%i day=%i\n",md.mon+1,md.mday);
#endif
   return md;
}
struct MonthDay WashingtonsBirthday(int year)
{
   struct MonthDay md;
   /* Third monday in February */
#ifdef _DEBUG
   printf("Washington's Birthday calculation for year %i:\n",year);
#endif
   if (year>1900) year-=1900;
   struct tm thistime;
   thistime.tm_year=year;
   thistime.tm_mon=1;
   thistime.tm_mday=1;
   thistime.tm_hour=thistime.tm_min=thistime.tm_sec=0;
   thistime.tm_wday=thistime.tm_yday=0;
   thistime.tm_isdst=-1;
   time_t time=mktime(&thistime);
   struct tm* ptr=localtime(&time);
   int dtfm=1-ptr->tm_wday;
   if (dtfm<0) dtfm+=7;
   md.mday=dtfm + 15;
   md.mon=1;
   md.key=_WashingtonsBirthday;
#ifdef _DEBUG
   printf("  RESULT month=%i day=%i\n",md.mon+1,md.mday);
#endif
   return md;
}

struct MonthDay GoodFriday(int year)
{
   struct MonthDay md;
#ifdef _DEBUG
   printf("GoodFriday calculation for year %i:\n",year);
#endif
   md=Easter(year);
   if (md.mday<3) {
      md.mon=2;
      md.mday=29+md.mday;
   } else {
      md.mday-=2;
   }
   md.key=_GoodFriday;
#ifdef _DEBUG
   printf("  RESULT month=%i day=%i\n",md.mon+1,md.mday);
#endif
   return md;
}
char* EasterTable[19][6] = {
   { "A5" , "A12", "A13", "A14", "A15", "A16"},
   { "M25", "A1" , "A2" , "A3" , "A4" , "A5" },
   { "A13", "M21", "M22", "M23", "M24", "M25"},
   { "A2" , "A9" , "A10", "A11", "A12", "A13"},
   { "M22", "M29", "M30", "M31", "A1" , "A2" },
   { "A10", "A17", "A18", "A18", "M21", "M22"},
   { "M30", "A6" , "A7" , "A8" , "A9" , "A10"},
   { "A18", "M26", "M27", "M28", "M29", "M30"},
   { "A7" , "A14", "A15", "A16", "A17", "A18"},
   { "M27", "A3" , "A4" , "A5" , "A6" , "A7" },
   { "A15", "M23", "M24", "M25", "M26", "M27"},
   { "A4" , "A11", "A12", "A13", "A14", "A15"},
   { "M24", "M31", "A1" , "A2" , "A3" , "A4" },
   { "A12", "A18", "M21", "M22", "M23", "M24"},
   { "A1" , "A8" , "A9" , "A10", "A11", "A12"},
   { "M21", "M28", "M29", "M30", "M31", "A1" },
   { "A9" , "A16", "A17", "A17", "A18", "M21"},
   { "M29", "A5" , "A6" , "A7" , "A8" , "A9" },
   { "A17", "M25", "M26", "M27", "M28", "M29"}
};

struct MonthDay Easter(int year)
{
   struct MonthDay md;
   char* lookup=NULL;
#ifdef _DEBUG
   printf("Easter calculation for year %i:\n",year);
#endif
   memset(&md,0,sizeof(struct MonthDay));
   /* STEP 1 */
   if (year >= 326 && year <= 1582) {
      lookup=EasterTable[year%19][0];
   } else if (year >= 1583 && year <= 1699) {
      lookup=EasterTable[year%19][1];
   } else if (year >= 1700 && year <= 1899) {
      lookup=EasterTable[year%19][2];
   } else if (year >= 1900 && year <= 2199) {
      lookup=EasterTable[year%19][3];
   } else if ((year >= 2200 && year <= 2299) || (year >= 2400 && year <= 2499)) {
      lookup=EasterTable[year%19][4];
   } else if ((year >= 2300 && year <= 2399) || (year >= 2500 && year <= 2599)) {
      lookup=EasterTable[year%19][5];
   }
#ifdef _DEBUG
   printf("  lookup=%s\n",lookup);
#endif
   if (lookup) {
      int res1,res2,res3;
      /* STEP 2 */
      int dd=atoi(lookup+1);
      res1=(*lookup=='M')*((dd-19)%7) + 
         (*lookup=='A')*((dd+5)%7);
#ifdef _DEBUG
      printf("  res1=%i\n",res1);
#endif
      int century=year/100;
      if (year<=1582) {
         res2=6-((century+1)%7);
      } else {
         if (century%4 != 0) {
            res2=5-((century-1)%4)*2;
         } else {
            res2=0;
         }
      }
#ifdef _DEBUG
      printf("  res2=%i\n",res2);
#endif
      int y=year%100;
      int skip=y/4;
      res3=(y+skip)%7;
#ifdef _DEBUG
      printf("  res3=%i\n",res3);
#endif
      /* STEP 3 */
      int dtns=7-((res1+res2+res3)%7);
      if (*lookup=='M' && dd+dtns<=31) {
         md.mon=2;
         md.mday=dd+dtns;
      } else {
         md.mon=3;
         md.mday=dd+dtns;
         if (md.mday>31) md.mday-=31;
      }
#ifdef _DEBUG
      printf("  RESULT month=%i day=%i\n",md.mon+1,md.mday);
#endif
   }
   md.key=_Easter;
   return md;
}

struct MonthDay MemorialDay(int year)
{
   struct MonthDay md;
   /* Last monday in May */
#ifdef _DEBUG
   printf("Memorial Day calculation for year %i:\n",year);
#endif
   if (year>1900) year-=1900;
   struct tm thistime;
   thistime.tm_year=year;
   thistime.tm_mon=4;
   thistime.tm_mday=31;
   thistime.tm_hour=thistime.tm_min=thistime.tm_sec=0;
   thistime.tm_wday=thistime.tm_yday=0;
   thistime.tm_isdst=-1;
   time_t time=mktime(&thistime);
   struct tm* ptr=localtime(&time);
   int dtlm=ptr->tm_wday-1;
   if (dtlm<0) dtlm+=7;
   md.mday=31 - dtlm;
   md.mon=4;
   md.key=_MemorialDay;
#ifdef _DEBUG
   printf("  RESULT month=%i day=%i\n",md.mon+1,md.mday);
#endif
   return md;
}
struct MonthDay IndependenceDay(int)
{
   struct MonthDay md;
   md.mon=6;
   md.mday=4;
   md.key=_IndependenceDay;
   return md;
}

struct MonthDay LaborDay(int year)
{
   struct MonthDay md;
   /* First monday in September */
#ifdef _DEBUG
   printf("Labor Day calculation for year %i:\n",year);
#endif
   if (year>1900) year-=1900;
   struct tm thistime;
   thistime.tm_year=year;
   thistime.tm_mon=8;
   thistime.tm_mday=1;
   thistime.tm_hour=thistime.tm_min=thistime.tm_sec=0;
   thistime.tm_wday=thistime.tm_yday=0;
   thistime.tm_isdst=-1;
   time_t time=mktime(&thistime);
   struct tm* ptr=localtime(&time);
   int dtfm=1-ptr->tm_wday;
   if (dtfm<0) dtfm+=7;
   md.mday=dtfm + 1;
   md.mon=8;
   md.key=_LaborDay;
#ifdef _DEBUG
   printf("  RESULT month=%i day=%i\n",md.mon+1,md.mday);
#endif
   return md;
}
struct MonthDay ThanksgivingDay(int year)
{
   struct MonthDay md;
   /* Fourth thursday in November */
#ifdef _DEBUG
   printf("Thanksgiving Day calculation for year %i:\n",year);
#endif
   if (year>1900) year-=1900;
   struct tm thistime;
   thistime.tm_year=year;
   thistime.tm_mon=10;
   thistime.tm_mday=1;
   thistime.tm_hour=thistime.tm_min=thistime.tm_sec=0;
   thistime.tm_wday=thistime.tm_yday=0;
   thistime.tm_isdst=-1;
   time_t time=mktime(&thistime);
   struct tm* ptr=localtime(&time);
   int dtft=4-ptr->tm_wday;
   if (dtft<0) dtft+=7;
   md.mday=dtft + 22;
   md.mon=10;
   md.key=_ThanksgivingDay;
#ifdef _DEBUG
   printf("  RESULT month=%i day=%i\n",md.mon+1,md.mday);
#endif
   return md;
}

struct MonthDay ChristmasDay(int)
{
   struct MonthDay md;
   md.mon=11;
   md.mday=25;
   md.key=_ChristmasDay;
   return md;
}





US_Calendar::US_Calendar(QWidget* p):QWidget(p)
{
   int xpos = border;
   int ypos = border;
   int buttonw = 130;
   int buttonh = 26;
   int labelh = 40;
   
   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   
   pb_cancel = new QPushButton(tr("Cancel"), this);
   pb_cancel->setAutoDefault(false);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_cancel->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_cancel->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_cancel, SIGNAL(clicked()), SLOT(quit()));
   
   xpos += buttonw + spacing;
   
   pb_accept = new QPushButton(tr("Accept"), this);
   pb_accept->setAutoDefault(false);
   pb_accept->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_accept->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_accept->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_accept, SIGNAL(clicked()), SLOT(accept()));
   
   xpos = border;
   ypos += buttonh + spacing;
   
   date_lbl = new QLabel(tr("Selected Date:"),this);
   date_lbl->setAlignment(AlignHCenter|AlignVCenter);
   date_lbl->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   date_lbl->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   date_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   date_lbl->setGeometry(xpos,ypos,buttonw,labelh);

   QDate date = QDate::currentDate();
   QString s;
   s.sprintf( "%d/%d/%d", date.month(), date.day(), date.year() );
   Display_Date = s;
   dateinput=new DateInput(this,s);
   dateinput->cal->show();
   dateinput->cal->move(xpos,ypos+labelh);
   QWidget::connect(dateinput,SIGNAL(classifyRequest(int,int,char*)),
                    this,SLOT(slotDayClassify(int,int,char*)));
   connect(dateinput,SIGNAL(dayLabel(int,int,int,char*)),
           this,SLOT(slotDayLabel(int,int,int,char*)));
   dateinput->setGeometry(xpos+buttonw,ypos,buttonw,labelh);
   dateinput->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize,QFont::Bold));
   
   dateinput->show();
   connect(dateinput, SIGNAL(DisplayChanged(QString)), SLOT(update_Date(QString)));
   
   ypos =338;
   xpos = 2 * border +2* buttonw + spacing;
   
   global_Xpos += 30;
   global_Ypos += 30;

   setMinimumSize(xpos, ypos);   
   setGeometry(global_Xpos, global_Ypos, xpos, ypos);

}
US_Calendar::~US_Calendar()
{
}

void US_Calendar::quit()
{
   close();
}
void US_Calendar::accept()
{
   emit(dateChanged(Display_Date));
   close();
}
void US_Calendar::update_Date(QString date)
{
   Display_Date = date;
}
void US_Calendar::slotDayClassify(int m, int y, char* dc)
{
   WorkTime wt(y);
   for (int i=0; i<wt.monthLength(m); i++) 
   {
      switch (wt.dayType(m,i)) 
      {
      case 0:
      default:
         dc[i]=NoColor;
         break;
      case 1:
         dc[i]=Color1;
         break;
      case 2:
         dc[i]=Color2;
         break;
      case 3:
         dc[i]=Color2;
      }
   }
}
void US_Calendar::slotDayLabel(int m, int d, int y, char* dl)
{
   WorkTime wt(y);
   wt.numberWorkDays();
   if (wt.isWorkDay(m,d-1))
      sprintf(dl,"Work day #%i of %i",wt.dayNumber(m,d-1),y);
   else if (wt.isHoliday(m,d-1))
      sprintf(dl,"%s",wt.dayText(m,d-1));
   else if (wt.isWeekend(m,d-1))
      sprintf(dl,"Weekend");
}

// Calendar

Calendar::Calendar(QWidget* p, DateInput* di)
   :QWidget(p)
{

   dateinput=di;
   USglobal = new US_Config();

   curyear=98;
   curmonth=9;
   curday=25;
   calrow=calcol=startdow=stopdow=numrows=-1;
   direction=initial=0;
   setMouseTracking(TRUE);
   setFixedWidth(CALENDAR_WIDTH);

   timer=new QTimer(this);
   CHECK_PTR(timer);
   connect(timer,SIGNAL(timeout()),SLOT(timerEvent()));

   tip=new CalendarTip(this,this);
   CHECK_PTR(tip);
}

void Calendar::show()
{
   // get curyear, curmonth, curday from string
   QString str=dateinput->getDate();
   dateinput->validator()->fixup(str);
   dateinput->setDate(str);
   char* tempstr=strdup(dateinput->getDate());
   curmonth=atoi(strtok(tempstr,"/"))-1;
   curday=atoi(strtok(NULL,"/"));
   curyear=atoi(strtok(NULL,"/"));
   if (curyear>=1900) curyear-=1900;
   free(tempstr);

   recalc();
   raise();
   QWidget::show();
}

void Calendar::hide()
{
   QWidget::hide();
}

const QDate Calendar::getQDate()
{
   QDate   retVal = myQDate;
   return (const QDate) retVal;
}

void Calendar::recalc(void)
{
   // Calculate some time stuff...
   struct tm tm1;
   tm1.tm_sec=0;
   tm1.tm_min=0;
   tm1.tm_hour=12;
   tm1.tm_mday=1;
   tm1.tm_mon=curmonth;
   tm1.tm_year=curyear;
   time_t temp=mktime(&tm1);
   struct tm* ptr=localtime(&temp);
   startdow=ptr->tm_wday;
   int nextmonth=curmonth+1;
   int nextyear=curyear;
   if (nextmonth>11) {
      nextmonth=0;
      nextyear++;
   }
   tm1.tm_sec=0;
   tm1.tm_min=0;
   tm1.tm_hour=12;
   tm1.tm_mday=1;
   tm1.tm_mon=nextmonth;
   tm1.tm_year=nextyear;
   temp=mktime(&tm1);
   temp-=3600*24;   // 1 day
   ptr=localtime(&temp);
   numdaysinmonth=ptr->tm_mday;
   stopdow=ptr->tm_wday;
   numrows=(numdaysinmonth+startdow-1)/7;

   // Get day coloring information, if present
   for (int i=0;i<31;i++)
      daycolor[i]=NoColor;
   emit(classifyRequest(curmonth,curyear,daycolor));

   resize(width(),numrows*ROW_HEIGHT+TITLE_HEIGHT*2+ROW_HEIGHT);
}

int Calendar::mouseLeftArrow(QMouseEvent* qme)
{
   if (qme->x()>=10 && qme->x()<=20 && 
       qme->y()>=(TITLE_HEIGHT-10)/2 && 
       qme->y()<=TITLE_HEIGHT-(TITLE_HEIGHT-10)/2 )
      return 1;
   return 0;
}

int Calendar::mouseRightArrow(QMouseEvent* qme)
{
   if (qme->x()>=width()-20 && qme->x()<=width()-10 && 
       qme->y()>=(TITLE_HEIGHT-10)/2 && 
       qme->y()<=TITLE_HEIGHT-(TITLE_HEIGHT-10)/2 )
      return 1;
   return 0;
}

void Calendar::paintEvent(QPaintEvent*)
{
   QPainter paint;
   paint.begin(this);
   QColorGroup g=colorGroup();
   paint.setPen(g.text());
   paint.fillRect(0,0,width(),height(),white);
   paint.fillRect(0,0,width(),TITLE_HEIGHT,cyan);
   paint.setPen(black);
   paint.drawLine(0,0,width()-1,0);
   paint.drawLine(0,0,0,height()-1);
   paint.drawLine(width()-1,0,width()-1,height()-1);
   paint.drawLine(0,height()-1,width()-1,height()-1);

   paint.setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+2, QFont::Bold));

   char txt[30];
   sprintf(txt,"%s, %i",months[curmonth],curyear+1900);
   paint.drawText(0,0,width(),TITLE_HEIGHT,AlignCenter,txt,strlen(txt));
   paint.setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   int w=width()/7;
   static char* dow[]={"Su","M","Tu","W","Th","F","Sa"};
   for (int j=0;j<7;j++) {
      paint.drawText(j*w+2,TITLE_HEIGHT,w,ROW_HEIGHT,AlignCenter,dow[j],
                     strlen(dow[j]));
   }
   for (int i=1;i<=numdaysinmonth;i++) {
      char txt[3];
      sprintf(txt,"%i",i);
      if ((int)daycolor[i-1] == (int)Color1) {
         paint.fillRect(((i+startdow-1)%7)*w+4,((i+startdow-1)/7)*ROW_HEIGHT+
                        TITLE_HEIGHT+ROW_HEIGHT+1,BLOCK_SIZE,BLOCK_SIZE,lightGray);
      }
      if ((int)daycolor[i-1] == (int)Color2) {
         paint.fillRect(((i+startdow-1)%7)*w+4,((i+startdow-1)/7)*ROW_HEIGHT+
                        TITLE_HEIGHT+ROW_HEIGHT+1,BLOCK_SIZE,BLOCK_SIZE,magenta);
      }
      if (i==curday) {
         paint.drawRect(((i+startdow-1)%7)*w+3,((i+startdow-1)/7)*ROW_HEIGHT+
                        TITLE_HEIGHT+ROW_HEIGHT,ROW_HEIGHT,ROW_HEIGHT);
      }
      paint.drawText(((i+startdow-1)%7)*w+3, ((i+startdow-1)/7)*ROW_HEIGHT+
                     TITLE_HEIGHT+ROW_HEIGHT,ROW_HEIGHT,ROW_HEIGHT,AlignCenter,txt,
                     strlen(txt));
   }
   QPointArray points(3);
   points.setPoints(3,10,TITLE_HEIGHT/2,20,TITLE_HEIGHT-(TITLE_HEIGHT-10)/2,
                    20,(TITLE_HEIGHT-10)/2);
   paint.setBrush(black);
   paint.drawPolygon(points,TRUE);
   points.setPoints(3,width()-10,TITLE_HEIGHT/2,width()-20,TITLE_HEIGHT-
                    (TITLE_HEIGHT-10)/2,width()-20,(TITLE_HEIGHT-10)/2);
   paint.drawPolygon(points,TRUE);
   paint.end();
}

void Calendar::mousePressEvent(QMouseEvent* qme)
{
   char str[20];
   if (mouseLeftArrow(qme)) {
      curmonth--;
      if (curmonth<0) {
         curmonth=11;
         curyear--;
      }
      direction=0;
      initial=1;
      timer->start(INITIAL_TIME);
      myQDate.setYMD(curyear+1900, curmonth+1, curday);
      sprintf(str,"%i/%i/%i",curmonth+1,curday,curyear+1900);
      dateinput->setDate(str);
      recalc();
      repaint();
   } else if (mouseRightArrow(qme)) {
      curmonth++;
      if (curmonth>11) {
         curmonth=0;
         curyear++;
      }
      direction=1;
      initial=1;
      timer->start(INITIAL_TIME);
      myQDate.setYMD(curyear+1900, curmonth+1, curday);
      sprintf(str,"%i/%i/%i",curmonth+1,curday,curyear+1900);
      dateinput->setDate(str);
      recalc();
      repaint();
   } else {
      int row=int(qme->y()>TITLE_HEIGHT+ROW_HEIGHT?(qme->y()-(TITLE_HEIGHT+
                                                              ROW_HEIGHT))/ROW_HEIGHT:-1);
      int col=int(qme->x()/(width()/7));
      if (!((row==0 && col<startdow) || (row>numrows) || (col>6) ||
            (row==numrows && col>stopdow)) && col>=0 && row>=0) {
         curday=row*7+col+1-startdow;
         char text[20];
         myQDate.setYMD(curyear+1900, curmonth+1, curday);
         sprintf(text,"%i/%i/%i",curmonth+1,curday,curyear+1900);
         dateinput->setDate(text);
         calrow=calcol=-1;
         repaint();
         show();
      }
   }
}
void Calendar::mouseMoveEvent(QMouseEvent* qme)
{
   int row=int(qme->y()>TITLE_HEIGHT+ROW_HEIGHT?(qme->y()-(TITLE_HEIGHT+
                                                           ROW_HEIGHT))/ROW_HEIGHT:-1);
   int col=int(qme->x()/(width()/7));
   if ((row==0 && col<startdow) || (row>numrows) || (col>6) ||
       (row==numrows && col>stopdow)) {
      row=col=-99;
   }
   QPainter paint;
   paint.begin(this);
   if (row!=calrow || col!=calcol) {
      if (curday == calrow*7+calcol+1-startdow) {
         paint.setPen(black);
      } else {
         paint.setPen(white);
      }
      paint.drawRect(calcol*(width()/7)+3,calrow*ROW_HEIGHT+TITLE_HEIGHT+
                     ROW_HEIGHT,ROW_HEIGHT,ROW_HEIGHT);
      if (row>=0 && col>=0) { 
         calrow=row;
         calcol=col;
         QColorGroup g=colorGroup();
         int x=calcol*(width()/7)+3;
         int y=calrow*ROW_HEIGHT+TITLE_HEIGHT+ROW_HEIGHT;
         paint.setPen(g.midlight());
         paint.drawLine(x,y,x,y+ROW_HEIGHT-1);
         paint.drawLine(x,y,x+ROW_HEIGHT-1,y);
         paint.setPen(g.dark());
         paint.drawLine(x+ROW_HEIGHT-1,y,x+ROW_HEIGHT-1,y+ROW_HEIGHT-1);
         paint.drawLine(x,y+ROW_HEIGHT-1,x+ROW_HEIGHT-1,y+ROW_HEIGHT-1);
      } else {
         calrow=calcol=-99;
      }
   }
   paint.end();
}

void Calendar::mouseReleaseEvent(QMouseEvent*)
{
   timer->stop();
}
void Calendar::timerEvent()
{
   if (initial) {
      initial=0;
      timer->start(REPEAT_TIME);
   }
   if (direction) {
      curmonth++;
      if (curmonth>11) {
         curmonth=0;
         curyear++;
      }
   } else {
      curmonth--;
      if (curmonth<0) {
         curmonth=11;
         curyear--;
      }
   }
   char str[20];
   sprintf(str,"%i/%i/%i",curmonth+1,curday,curyear+1900);
   dateinput->setDate(str);
   recalc();
   repaint();
}

void Calendar::getDayLabel(int m, int d, int y, char* t)
{
   // default to null if not connected...
   strcpy(t,"");
   emit(dayLabel(m,d,y,t));
}

// CalendarTip

CalendarTip::CalendarTip(QWidget* p, Calendar* c)
   :QToolTip(p)
{
   cal=c;
}
void CalendarTip::maybeTip(const QPoint& p)
{
   int row=int(p.y()>TITLE_HEIGHT+ROW_HEIGHT?(p.y()-(TITLE_HEIGHT+ROW_HEIGHT))/
               ROW_HEIGHT:-1);
   int col=int(p.x()/(cal->width()/7));
   int curday;

   if (!((row==0 && col<cal->startdow) || (row>cal->numrows) || (col>6) ||
         (row==cal->numrows && col>cal->stopdow)) && col>=0 && row>=0) {
      curday=row*7+col+1-cal->startdow;
      QRect rect(p.x()-5,p.y()-5,10,10);
      char tiptext[100],text[100];
      cal->getDayLabel(cal->curmonth,curday,cal->curyear+1900,text);
      if (strcmp(text,""))
         sprintf(tiptext,"%s %i, %i\n%s",months[cal->curmonth],curday,
                 cal->curyear+1900,text);
      else
         sprintf(tiptext,"%s %i, %i",months[cal->curmonth],curday,
                 cal->curyear+1900);
      tip(rect,tiptext);
   } 
}

// DateValidator

DateValidator::DateValidator(QWidget* p)
   :QValidator(p)
{
}
QValidator::State DateValidator::validate(QString& str, int&) const
{
   int tempmon=1;
   int tempday=1;
   int tempyear=1970;

   // Every character must be either a digit or a slash
   for (unsigned int i=0;i<strlen(str);i++)
      if (!str.at(i).isDigit() && str.at(i)!='/')
         return Invalid;

   // Must have exactly two slashes
   if(str.contains("/",false) != 2)
      return Valid;
      
   int firstslash = str.find("/",0,false);
   QString temp1 = str, temp2 = str, temp3;
   temp1.truncate(firstslash);
   temp2.remove(0,firstslash+1);
   tempmon =temp1.toInt();
   temp3 = temp2;
   int secondslash = temp2.find("/",0,false);
   temp2.truncate(secondslash);
   tempday = temp2.toInt();
   temp3.remove(0,secondslash+1);
   tempyear = temp3.toInt();
   
   // Month, day and year must be in an acceptable range
   if (tempmon>12 || tempday>31 || tempyear>2100)
      return Invalid;
   if (tempmon<1 || tempday<1 || tempyear<1970)
      return Valid;

   return Acceptable;
}

void DateValidator::fixup(QString& str)
{
   int tempmon=1;
   int tempday=1;
   int tempyear=1970;

   int firstslash = str.find("/",0,false);
   QString temp1 = str, temp2 = str, temp3;
   temp1.truncate(firstslash);
   temp2.remove(0,firstslash+1);
   tempmon =temp1.toInt();
   temp3 = temp2;
   int secondslash = temp2.find("/",0,false);
   temp2.truncate(secondslash);
   tempday = temp2.toInt();
   temp3.remove(0,secondslash+1);
   tempyear = temp3.toInt();
   
   if (tempmon<=0) tempmon=1;
   if (tempday<1) tempday=1;
   if (tempyear<1970) tempyear=1970;
   str.sprintf("%i/%i/%i",tempmon,tempday,tempyear);
}

// DateInput

DateInput::DateInput(QWidget* p, const char* txt)
   :QWidget(p)
{
   text=new QLineEdit(this);
   CHECK_PTR(text);
   //text->setValidator(new DateValidator(this));
   connect(text,SIGNAL(returnPressed()),SLOT(slotNewText()));

   cal=new Calendar(p,this);
   CHECK_PTR(cal);
   // cal->hide();
   cal->show();
   connect(cal,SIGNAL(classifyRequest(int,int,char*)),
           SLOT(slotClassifyRequest(int,int,char*)));
   connect(cal,SIGNAL(dayLabel(int,int,int,char*)),
           SLOT(slotDayLabel(int,int,int,char*)));

   setDate(txt);
}

DateInput::DateInput(QWidget* p, const QDate sdate)
   :QWidget(p)
{
   text=new QLineEdit(this);
   CHECK_PTR(text);
   //text->setValidator(new DateValidator(this));
   connect(text,SIGNAL(returnPressed()),SLOT(slotNewText()));

   cal=new Calendar(p,this);
   CHECK_PTR(cal);
   cal->show();
   connect(cal,SIGNAL(classifyRequest(int,int,char*)),
           SLOT(slotClassifyRequest(int,int,char*)));
   connect(cal,SIGNAL(dayLabel(int,int,int,char*)),
           SLOT(slotDayLabel(int,int,int,char*)));

   char txt[16];
   sprintf(txt, "%d/%d/%d", sdate.month(), sdate.day(), sdate.year());
   setDate(txt);
}


void DateInput::setDate(const char* str)
{
   text->setText(str);
   emit(DisplayChanged(str));
   emit(dateChanged());
}
void DateInput::setDate(const QDate newDate)
{
   char str[128];
   sprintf(str, "%d/%d/%d", newDate.month(), newDate.day(), newDate.year());

   emit(dateChanged());
}

const char* DateInput::getDate(void)
{
   return text->text();
}

/*const QDate DateInput::getQDate()
  {
  //  int     tmpPos = 0;
  char    *tmpStr = new char[1024];
  char    *part;
  int     Y = 1900, M = 1, D = 1;
  QDate   retVal;

  strcpy(tmpStr, text->text());
  part = strsep(&tmpStr, "/");
  if (part) {
  M = atoi(part);
  part = strsep(&tmpStr, "/");
  if (part) {
  D = atoi(part);
  part = strsep(&tmpStr, "/");
  if (part) Y = atoi(part);
  }
  }

  QString temstr = text->text();
  cout<<temstr<<endl;   
  retVal.setYMD(Y,M,D);
  return (const QDate) retVal;
  }
*/
void DateInput::slotClassifyRequest(int m,int y,char* c)
{
   emit(classifyRequest(m,y,c));
}

void DateInput::slotDayLabel(int m, int d, int y, char* t)
{
   emit(dayLabel(m,d,y,t));
}

void DateInput::slotNewText()
{
   QRect rect = geometry();
   cal->setGeometry(rect.x(),rect.y()+height(),rect.width(),rect.height());
   cal->show();
}

void DateInput::resizeEvent(QResizeEvent*)
{
   text->setGeometry(2,2,width(),height()-4);
}

void DateInput::mousePressEvent(QMouseEvent* qme)
{
   int x = qme->x();
   int y = qme->y();
   if (x > width()-20 && x < width()-4 && y > 5 && y < height()-5) 
   {
      QRect rect = geometry();
      cal->setGeometry(rect.x(),rect.y()+height(),rect.width(),rect.height());
      cal->show();
   }
}
// WorkTime

WorkTime::WorkTime(int y)
{
   year=y;
   if (year>1900) year-=1900;
   // Classify each day of the year
   for (int mon=0;mon<12;mon++) {
      struct tm temptime;
      temptime.tm_sec=temptime.tm_min=temptime.tm_hour=0;
      temptime.tm_mday=1;
      temptime.tm_mon=mon;
      temptime.tm_year=year;
      temptime.tm_wday=temptime.tm_yday=0;
      temptime.tm_isdst=-1;
      time_t temp=mktime(&temptime);
      struct tm* ptr=localtime(&temp);
      int dow=ptr->tm_wday;
      daysinmonth[mon]=numDaysInMonth(mon,year);
      for (int i=0;i<daysinmonth[mon];i++) {
         days[mon][i].dow=dow++;
         if (dow>6) dow=0;
         if (days[mon][i].dow==0 || days[mon][i].dow==6)
            days[mon][i].dtype=Weekend;
         else
            days[mon][i].dtype=WorkDay;
      }
   }
   // Assign the holidays
   // Check New Year's Day of this year and next year
   workHoliday(NewYearsDay(year+1900));
   workHoliday(MartinLutherKingDay(year+1900));
   workHoliday(WashingtonsBirthday(year+1900));
   workHoliday(GoodFriday(year+1900));
   workHoliday(MemorialDay(year+1900));
   workHoliday(IndependenceDay(year+1900));
   workHoliday(LaborDay(year+1900));
   workHoliday(ThanksgivingDay(year+1900));
   workHoliday(ChristmasDay(year+1900));
   if (days[11][30].dow == Fri) {
      days[11][30].dtype=HolidayObserved;
      days[11][30].key=_NewYearsDay;
   }
}
int WorkTime::monthLength(int mon)
{
   return daysinmonth[mon];
}

int WorkTime::dayType(int mon, int day)
{
   return days[mon][day].dtype;
}

int WorkTime::isWorkDay(int mon, int day)
{
   if (days[mon][day].dtype == WorkDay)
      return 1;
   return 0;
}

int WorkTime::isHoliday(int mon, int day)
{
   if (days[mon][day].dtype == Holiday || 
       days[mon][day].dtype == HolidayObserved)
      return 1;
   return 0;
}

int WorkTime::isWeekend(int mon, int day)
{
   if (days[mon][day].dtype == Weekend)
      return 1;
   return 0;
}

const char* WorkTime::dayText(int mon, int day)
{
   if (days[mon][day].dtype == Holiday)
      return holidayText[days[mon][day].key];
   if (days[mon][day].dtype == HolidayObserved) {
      sprintf(workstr,"%s (observed)",holidayText[days[mon][day].key]);
      return workstr;
   }
   return "";
}

int WorkTime::dayNumber(int mon, int day)
{
   if (days[mon][day].dtype == WorkDay)
      return days[mon][day].key+1;
   return -1;
}

void WorkTime::numberWorkDays(void)
{
   int count=0;
   for (int i=0;i<12;i++) {
      for (int j=0;j<daysinmonth[i];j++) {
         if (days[i][j].dtype == WorkDay)
            days[i][j].key=count++;
      }
   }
}

void WorkTime::dump(void)
{
   for (int mon=0;mon<12;mon++) {
      if (mon>0) printf("\n");
      for (int j=0;j<(int)days[mon][0].dow;j++)
         printf(" ");
      for (int j=0;j<daysinmonth[mon];j++) {
         if (days[mon][j].dtype==WorkDay)
            printf("T");
         if (days[mon][j].dtype==Weekend)
            printf("W");
         if (days[mon][j].dtype==Holiday)
            printf("H");
         if (days[mon][j].dtype==HolidayObserved)
            printf("O");
         if (days[mon][j].dow==6)
            printf("\n");
      }
   }
   printf("\n");
   printf("sizeof (DAY) = %d\n", (int) sizeof(DAY) );
}

int WorkTime::numDaysInMonth(int mon, int year)
{
   if (mon==0) return 31;   // January
   if (mon==1) {         // February
      if (year%4 == 0) {
         if (year%100 == 0) return 28;
         else return 29;
      }
      return 28;
   }
   if (mon==2) return 31;   // March
   if (mon==3) return 30;   // April
   if (mon==4) return 31;   // May
   if (mon==5) return 30;   // June
   if (mon==6) return 31;   // July
   if (mon==7) return 31;   // August
   if (mon==8) return 30;   // September
   if (mon==9) return 31;   // October
   if (mon==10) return 30;   // November
   if (mon==11) return 31;   // December
   return -1;  // error
}

void WorkTime::workHoliday(struct MonthDay md)
{
   int actday=md.mday;
   int actmon=md.mon;
   int extra=0;
   if (days[actmon][actday-1].dow == Sat) {
      /* if date is on a Saturday, holiday is on the previous Friday */
      extra=1;
      actday--;
      if (actday == 0) { 
         if (actmon>0) actday=daysinmonth[--actmon];
         else extra=0;
      }
   } else if (days[actmon][actday-1].dow == Sun) {
      /* if date is on a Sunday, holiday is on the next Monday */
      extra=1;
      actday++;
      if (actday > daysinmonth[actmon]) {
         if (actmon<11) {
            actday=1;
            actmon++;
         } else
            extra=0;
      }
   }
   if (days[md.mon][md.mday-1].dow != Sun && days[md.mon][md.mday-1].dow != Sat) {
      days[md.mon][md.mday-1].dtype=Holiday;
      days[md.mon][md.mday-1].key=md.key;
   }
   if (extra) {
      days[actmon][actday-1].dtype=HolidayObserved;
      days[actmon][actday-1].key=md.key;
   }
}


