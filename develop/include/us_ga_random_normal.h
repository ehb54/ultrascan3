#ifndef US_GA_RANDOM_NORMAL_H
#define US_GA_RANDOM_NORMAL_H

typedef struct {
  double x, y;
} double_pair;

double_pair random_normal(double mean1, double sd1, double mean2, double sd2);

#endif
