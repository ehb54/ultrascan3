#ifndef US_LASER_H
#define US_LASER_H

#include <qcombobox.h>
#include <qfiledialog.h>
#include <qtextstream.h>
#include <qwt_plot.h>
#include "us_util.h"
#include "us_db.h"
struct LaserData
{
   QString sampleID;
   QString operatorID;
   QString date;
   QString time;
   int LaserID;
   int InvID;
   int BuffID;
   int PepID;
   int DNAID;
   float baseline;
   float angle;
   float lambda;
   float temperature;
   float viscosity;
   float liquid;
   float real;
   float imaginary;
   float first_channel;
   float mode1;
   float mode2;
   
   double run_number;
   double count_A;
   double count_B;
   double total;
   double first_delay;
   double channels;
   double ext_channels;
   double calculated;
   double measured;
   double last_delay;
   double FH_delay;
   double S_time;
   double H_channel;
   double M_time;
   double M_channel;
   double L_time;
   double L_channel;
   double FM_channel;
   double LM_channel;
   vector <double> dl_time;
   vector <double> correlation;
};


class US_EXTERN US_Laser_Param : public QFrame
{
   Q_OBJECT
   
   public:
      US_Laser_Param(struct LaserData data_list, QWidget *p = 0, const char *name = "us_laser_param");
      ~US_Laser_Param();
      
      int xpos, ypos, buttonw, buttonh, border, spacing;

      QLabel *lbl_angle1;
      QLabel *lbl_lambda1;
      QLabel *lbl_temperature1;
      QLabel *lbl_viscosity1;
      QLabel *lbl_liquid1;
      QLabel *lbl_real1;
      QLabel *lbl_imaginary1;
      QLabel *lbl_angle2;
      QLabel *lbl_lambda2;
      QLabel *lbl_temperature2;
      QLabel *lbl_viscosity2;
      QLabel *lbl_liquid2;
      QLabel *lbl_real2;
      QLabel *lbl_imaginary2;
      US_Config *USglobal;


};

class US_EXTERN US_Laser_Channel : public QFrame
{
   Q_OBJECT
   
   public:
      US_Laser_Channel(struct LaserData data_list, QWidget *p = 0, const char *name = "us_laser_channel");
      ~US_Laser_Channel();

      int xpos, ypos, buttonw, buttonh, border, spacing;
      QLabel *lbl_first1;
      QLabel *lbl_baseline1;
      QLabel *lbl_mode11;
      QLabel *lbl_mode21;
      QLabel *lbl_channels1;
      QLabel *lbl_extended1;
      QLabel *lbl_Fdelay1;
      QLabel *lbl_Ldelay1;
      QLabel *lbl_FH_delay1;
      QLabel *lbl_FMchannel1;
      QLabel *lbl_LMchannel1;
      QLabel *lbl_Hchannel1;
      QLabel *lbl_Mchannel1;
      QLabel *lbl_Lchannel1;
      QLabel *lbl_Stime1;
      QLabel *lbl_Mtime1;
      QLabel *lbl_Ltime1;
      
      QLabel *lbl_first2;
      QLabel *lbl_baseline2;
      QLabel *lbl_mode12;
      QLabel *lbl_mode22;
      QLabel *lbl_channels2;
      QLabel *lbl_extended2;
      QLabel *lbl_Fdelay2;
      QLabel *lbl_Ldelay2;
      QLabel *lbl_FH_delay2;
      QLabel *lbl_FMchannel2;
      QLabel *lbl_LMchannel2;
      QLabel *lbl_Hchannel2;
      QLabel *lbl_Mchannel2;
      QLabel *lbl_Lchannel2;
      QLabel *lbl_Stime2;
      QLabel *lbl_Mtime2;
      QLabel *lbl_Ltime2;

      US_Config *USglobal;


};

class US_EXTERN US_Laser_Result : public QFrame
{
   Q_OBJECT
   
   public:
      US_Laser_Result(struct LaserData data_list, QWidget *p = 0, const char *name = "us_laser_result");
      ~US_Laser_Result();
   
      int xpos, ypos, buttonw, buttonh, border, spacing;
      QLabel *lbl_countA1;
      QLabel *lbl_countB1;
      QLabel *lbl_total1;
      QLabel *lbl_calculated1;
      QLabel *lbl_measured1;
      QLabel *lbl_countA2;
      QLabel *lbl_countB2;
      QLabel *lbl_total2;
      QLabel *lbl_calculated2;
      QLabel *lbl_measured2;
      US_Config *USglobal;

};


class US_EXTERN US_Laser : public US_DB
{
   Q_OBJECT
   
   public:
      US_Laser(QWidget *p = 0, const char *name = "us_laser");
      ~US_Laser();
      
      struct LaserData data_list;
   //   double *delay_time, *contents;
      int xpos, ypos, buttonw, buttonh, border, spacing;
      QString fn;
      bool load_flag, from_HD;
      QPushButton *pb_load;
      QPushButton *pb_param;
      QPushButton *pb_channel;
      QPushButton *pb_result;
      QPushButton *pb_help;
      QPushButton *pb_print;
      QPushButton *pb_close;
      
      QLabel *lbl_sample;
      QLabel *lbl_operator;
      QLabel *lbl_date1;
      QLabel *lbl_date2;
      QLabel *lbl_time1;
      QLabel *lbl_time2;
         
   // for display value
      QLineEdit *le_sample;
      QLineEdit *le_operator;
                  
      QwtPlot *data_plot;
      US_Config *USglobal;
      
   public slots:
      void load();
      void load_interface(struct LaserData);
   private slots:
      void get_parameters();
      void get_channels();
      void get_results();
      void help();
      void print();
      void quit();
   
   protected slots:
      void resizeEvent(QResizeEvent *e);
   
   signals:
   
      void dataChanged(struct LaserData);

};

#endif

