#ifndef __CALENDAR_H
#define __CALENDAR_H

#include <qlineedit.h>
#include <qlistbox.h>
#include <qtooltip.h>
#include <qvalidator.h>
#include <qwidget.h>
#include <qdatetm.h>
#include <qpainter.h>
#include <qdrawutil.h>
#include <qstyle.h>
#include "us_util.h"

class Calendar;
class CalendarTip;
class DateInput;
class QMouseEvent;
class QPaintEvent;
class QResizeEvent;

typedef enum { NoColor, Color1, Color2, Color3 } DayColors;

// Max chars in holiday string
#define MAXHOLIDAY 50

typedef enum { _NewYearsDay, _MartinLutherKingDay, _WashingtonsBirthday,
  _GoodFriday, _Easter, _MemorialDay, _IndependenceDay, _LaborDay,
  _ThanksgivingDay, _ChristmasDay } HolidayType ;

extern const char* holidayText[];

struct MonthDay {
  short mon;      /* Month 0-11 */
  short mday;     /* Day of month 1-31 */
  short key;      /* Holiday key */
};

struct MonthDay NewYearsDay(int year);
struct MonthDay MartinLutherKingDay(int year);
struct MonthDay WashingtonsBirthday(int year);
struct MonthDay GoodFriday(int year);
struct MonthDay Easter(int year);   /* valid years 326 to 2599 A.D. */
struct MonthDay MemorialDay(int year);
struct MonthDay IndependenceDay(int year);
struct MonthDay LaborDay(int year);
struct MonthDay ThanksgivingDay(int year);
struct MonthDay ChristmasDay(int year);

typedef enum {Sun, Mon, Tue, Wed, Thu, Fri, Sat} DOW;
typedef enum {WorkDay, Weekend, Holiday, HolidayObserved} DTYPE;

struct DAY {
  unsigned int dow:4;
  unsigned int dtype:4;
  unsigned char key;
};


class US_EXTERN US_Calendar : public QWidget 
{
  Q_OBJECT
public:

   US_Calendar(QWidget *parent=0);
   ~US_Calendar();
    US_Config *USglobal;
   QPushButton *pb_cancel;
   QPushButton *pb_accept;
   QLabel *date_lbl;
   
   QString Display_Date;
   
signals:
   void dateChanged(QString);
   
public slots:
  void slotDayClassify(int,int,char*);
  void slotDayLabel(int,int,int,char*);
private slots:
   void quit();
   void accept();
   void update_Date(QString);
private:
  DateInput* dateinput;
};

class WorkTime 
{
public:
  WorkTime(int year);
  int monthLength(int mon);
  int dayType(int mon, int day);
  int isWorkDay(int mon, int day);
  int isWeekend(int mon, int day);
  int isHoliday(int mon, int day);
  const char* dayText(int mon, int day);
  int dayNumber(int mon, int day);
  void numberWorkDays(void);
  void dump(void);
private:
  int numDaysInMonth(int mon, int year);
  void workHoliday(struct MonthDay);

  int year;
  int daysinmonth[12];
  DAY days[12][31];
  char workstr[MAXHOLIDAY];
};



class Calendar : public QWidget 
{
  Q_OBJECT
public:
  Calendar(QWidget*, DateInput*);
  void show();
  void hide();
  const QDate getQDate();

   US_Config *USglobal;
signals:
  void classifyRequest(int mon, int yr, char*);
  void dayLabel(int, int, int, char*);
protected:
  void paintEvent(QPaintEvent*);
  void mousePressEvent(QMouseEvent*);
  void mouseMoveEvent(QMouseEvent*);
  void mouseReleaseEvent(QMouseEvent*);
protected slots:
  void timerEvent();
private:
  void getDayLabel(int, int, int, char*);
  void recalc(void);
  int mouseLeftArrow(QMouseEvent*);
  int mouseRightArrow(QMouseEvent*);

  int initial,direction;
  int curyear,curmonth,curday;
  int calrow,calcol,startdow,stopdow,numrows;
  int numdaysinmonth;
  char daycolor[31];
  QTimer* timer;
  DateInput* dateinput;
  CalendarTip* tip;

  QDate myQDate;

  friend class CalendarTip;
};

class CalendarTip : public QToolTip {
public:
  CalendarTip(QWidget*,Calendar*);
protected:
  void maybeTip(const QPoint&);
private:
  Calendar* cal;
};

class DateValidator : public QValidator {
public:
  DateValidator(QWidget*);
  State validate(QString&,int&) const;
  void fixup(QString&);
};

class DateInput : public QWidget {
  Q_OBJECT
public:
  DateInput(QWidget*,const char*);
  DateInput(QWidget*, const QDate);
  Calendar* cal;
 QLineEdit* text;
  const char* getDate(void);
  const QDate getQDate();
  DateValidator* validator(void) {return dv;}
  void setDate(const char*);
  void setDate(const QDate);
signals:
  void classifyRequest(int,int,char*);
  void dayLabel(int, int, int, char*);
  void dateChanged();
  void DisplayChanged(QString);
protected slots:
  void slotDayLabel(int, int, int, char*);
  void slotClassifyRequest(int,int,char*);
  void slotNewText();
protected:
  void resizeEvent(QResizeEvent*);
  void mousePressEvent(QMouseEvent*);
private:
  DateValidator* dv;
};

#endif



