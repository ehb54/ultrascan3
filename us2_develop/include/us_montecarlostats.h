#ifndef US_MONTECARLOSTATS_H
#define US_MONTECARLOSTATS_H

#include "us_util.h"

#include <qlabel.h>
#include <qpushbutton.h>


struct lines
{
   vector <float> value;
};

struct MonteCarloStats
{
   float high;               // the maximum value of the distribution
   float low;               // the minimum value of the distribution
   float mean;               // the average value of the distribution
   float median;            // the half-way value in the distribution
   float mode1;            // the lower limit of the bin with the most datapoints in the distribution
   float mode2;            // the upper limit of the bin with the most datapoints in the distribution
   float mode_center;      // mode center
   double kurtosis;         // the kurtosis of the distribution
   double skew;            // the skew of the distribution
   float correlation;      // the correlation coefficient of the distribution
   double std_deviation;      // the standard deviation of the distribution
   double std_error;         // the standard error of the distribution
   double variance;         // the variance of the distribution
   float area;               // area under the distribution
   unsigned int points;      // the number of points in the distribution
   float conf95low;         // 95 % confidence limit on the low side
   float conf95high;         // 95 % confidence limit on the high side
   float conf99low;         // 99 % confidence limit on the low side
   float conf99high;         // 99 % confidence limit on the high side
   QString parameter_name;
};

struct MonteCarlo
{
   int experiment;   //0=velocity, 1=equilibrium
   int data_type; // 0=gaussian, 1=bootstrap, 2=mixed 
   unsigned int iterations;
   unsigned int parameters;
   int random_seed;
   QString filename;
   bool append;
   bool status;      // true = run, false = stop
   int addNoise;
   int percent_bootstrap;
   int percent_gaussian;
   int rule;
   float varianceThreshold;
   float parameterNoise;
   vector <QString> parameter;
   QString run_id;
   QString fitName;
   float mw_upperLimit, mw_lowerLimit; // needed for fixed MW distribution results
};

class US_MonteCarloStats_W : public QFrame
{

   Q_OBJECT

   public:

      US_MonteCarloStats_W(struct MonteCarloStats *temp_stats, 
      bool *temp_stats_widget, QWidget *p = 0, const char *name = 0);
      ~US_MonteCarloStats_W();
      bool *stats_widget;
      struct MonteCarloStats *stats;
      US_Config *USglobal;
      QLabel *lbl_banner1;
      QLabel *lbl_high1;
      QLabel *lbl_high2;
      QLabel *lbl_low1;
      QLabel *lbl_low2;
      QLabel *lbl_mean1;
      QLabel *lbl_mean2;
      QLabel *lbl_median1;
      QLabel *lbl_median2;
      QLabel *lbl_skew1;
      QLabel *lbl_skew2;
      QLabel *lbl_kurtosis1;
      QLabel *lbl_kurtosis2;
      QLabel *lbl_mode_low1;
      QLabel *lbl_mode_low2;
      QLabel *lbl_mode1;
      QLabel *lbl_mode_high1;
      QLabel *lbl_mode_high2;
      QLabel *lbl_mode2;
      QLabel *lbl_limits1;
      QLabel *lbl_limits2;
      QLabel *lbl_std_dev1;
      QLabel *lbl_std_dev2;
      QLabel *lbl_std_err1;
      QLabel *lbl_std_err2;
      QLabel *lbl_variance1;
      QLabel *lbl_variance2;
      QLabel *lbl_correlation1;
      QLabel *lbl_correlation2;
      QLabel *lbl_area1;
      QLabel *lbl_area2;
      QLabel *lbl_points1;
      QLabel *lbl_points2;
      QLabel *lbl_95conf1;
      QLabel *lbl_95conf2;
      QLabel *lbl_99conf1;
      QLabel *lbl_99conf2;
      QPushButton *pb_help;
      QPushButton *pb_quit;

   public slots:
      void update_labels();
      void help();
      void quit();
   
   protected slots:
      void closeEvent(QCloseEvent *e);
};

#endif

