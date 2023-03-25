#include "../include/us_math.h"

#undef DEBUG

unsigned long square(int num)
{
   return (long) num * num;
}

float square(float num)
{
   return num * num;
}

double square(double num)
{
   return num * num;
}

double linefit(double **x, double **y, double *slope, double *intercept, double *sigma,
               double *correlation, int arraysize)
{
   double sumx=0, sumy=0, sumxy=0, sumx_sq=0, sumy_sq=0, sumchi_sq=0, average;
   int i;
   for (i=0; i<arraysize; i++)
   {
      sumy += (*y)[i];
   }
   average = sumy/arraysize;
   for (i=0; i<arraysize; i++)
   {
      sumx += (*x)[i];
      sumchi_sq += ((*y)[i] - average) * ((*y)[i] - average);
      sumxy += (*x)[i] * (*y)[i];
      sumx_sq += (*x)[i] * (*x)[i];
      sumy_sq += (*y)[i] * (*y)[i];
   }
   *slope = (arraysize * sumxy - sumx * sumy)/(arraysize * sumx_sq - sumx * sumx);
   *intercept = (sumy - *slope * sumx)/arraysize;
   *correlation = (arraysize * sumxy - sumx * sumy)/(sqrt(arraysize * sumx_sq
                                                          - sumx * sumx) * sqrt(arraysize *sumy_sq - sumy * sumy));
   *sigma = sqrt(sumchi_sq)/arraysize;
   return(average);
}

float linefit(float **x, float **y, float *slope, float *intercept, float *sigma,
              float *correlation, int arraysize)
{
   float sumx=0, sumy=0, sumxy=0, sumx_sq=0, sumy_sq=0, sumchi_sq=0, average;
   int i;
   for (i=0; i<arraysize; i++)
   {
      sumy += (*y)[i];
   }
   average = sumy/arraysize;
   for (i=0; i<arraysize; i++)
   {
      sumx += (*x)[i];
      sumchi_sq += ((*y)[i] - average) * ((*y)[i] - average);
      sumxy += (*x)[i] * (*y)[i];
      sumx_sq += (*x)[i] * (*x)[i];
      sumy_sq += (*y)[i] * (*y)[i];
   }
   *slope = (arraysize * sumxy - sumx * sumy)/(arraysize * sumx_sq - sumx * sumx);
   *intercept = (sumy - *slope * sumx)/arraysize;
   *correlation = (arraysize * sumxy - sumx * sumy)/(sqrt(arraysize * sumx_sq
                                                          - sumx * sumx) * sqrt(arraysize *sumy_sq - sumy * sumy));
   *sigma = sqrt(sumchi_sq)/arraysize;
   return(average);
}
/*
  int max (int val1, int val2)
  {
  if (val1 <= val2)
  return val2;
  else
  return val1;
  }

  unsigned int max (unsigned int val1, unsigned int val2)
  {
  if (val1 <= val2)
  return val2;
  else
  return val1;
  }

  float max (float val1, float val2)
  {
  if (val1 <= val2)
  return val2;
  else
  return val1;
  }

  double max (double val1, double val2)
  {
  if (val1 <= val2)
  return val2;
  else
  return val1;
  }

  int min (int val1, int val2)
  {
  if (val1 >= val2)
  return val2;
  else
  return val1;
  }

  unsigned int min (unsigned int val1, unsigned int val2)
  {
  if (val1 >= val2)
  return val2;
  else
  return val1;
  }

  float min (float val1, float val2)
  {
  if (val1 >= val2)
  return val2;
  else
  return val1;
  }

  double min (double val1, double val2)
  {
  if (val1 >= val2)
  return val2;
  else
  return val1;
  }
*/
float box_muller(float m, float s)      /* normal random variate generator */
{                                       /* mean m, standard deviation s */
   float x1, x2, w, y1;
   static float y2;
   static int use_last = 0;

   if (use_last)                   /* use value from previous call */
   {
      y1 = y2;
      use_last = 0;
   }
   else
   {
      do
      {
         x1 = 2.0 * ranf() - 1.0;
         x2 = 2.0 * ranf() - 1.0;
         w = x1 * x1 + x2 * x2;
      } while ( w >= 1.0 );
      w = sqrt( (-2.0 * log( w ) ) / w );
      y1 = x1 * w;
      y2 = x2 * w;
      use_last = 1;
   }
   return( m + y1 * s );
}

float ranf()
{
   int N=1;
   float temp=0.0;
   temp = (((float) rand() / ((float) RAND_MAX + 1) * N));
   return(temp);
}

int us_randomize()
{
   QTime t;
   t = QTime::currentTime();
   int seed, now = t.msec() + t.second() * 1000 + t.minute() * 60000 + t.hour() * 3600000;
   seed = now;
#ifdef UNIX
   seed -= (int)getpid();
#endif
   srand(seed);   //randomize the random function
   return (seed);
}

float random_range(float min_val, float max_val)
{
   float tmp = min_val + (max_val - min_val) * (rand() / (RAND_MAX + min_val));
   return (tmp);
}

float standard_distribution(float x)
{
   float result;
   result = (exp((-0.5) * pow((double) x, (double) 2.0))) / sqrt((double) (2 * M_PI));
   return result;
}

float us_normal_distribution(float sigma, float mean, float x)
{
   float result;
   result = exp(-(pow((double) ((x - mean)/sigma), (double) 2.0))) / sqrt((double) (2 * M_PI * sigma));
   return result;
}
//
// overloaded function find_index(f|r):
// finds the subscript of an array entry that matches most closely the value
// of the testvalue testx.
// To enhance efficiency, find_indexf starts searching at the beginning of the
// array (forward), while find_indexr starts searching from the end (reverse).
// The function willl check the array bounds and return (-1) if the array
// bounds where exceeded (i.e., the searched value testx was not found in the array)
//
int find_indexf(double testx, double **xarray, int maxcount)
{
   int i=0;
   maxcount --;
   while (((*xarray)[i] < testx) && (i<=maxcount))
   {
      i++;
      if (i > maxcount)
      {
         return(-1);
      }
   }
   if (fabs((*xarray)[i]-testx) > fabs((*xarray)[i-1]-testx))
   {
      return (i-1);
   }
   else
   {
      return (i);
   }
}

int find_indexf(float testx, float **xarray, int maxcount)
{
   int i=0;
   maxcount --;
   while (((*xarray)[i] < testx) && (i<=maxcount))
   {
      i++;
      if (i > maxcount)
      {
         return(-1);
      }

   }
   if (fabs((*xarray)[i]-testx) > fabs((*xarray)[i-1]-testx))
   {
      return (i-1);
   }
   else
   {
      return (i);
   }
}

int find_indexf(int testx, int **xarray, int maxcount)
{
   int i=0;
   maxcount --;
   while (((*xarray)[i] < testx) && (i<=maxcount))
   {
      i++;
      if (i > maxcount)
      {
         return(-1);
      }
   }
   return (i-1);
}

int find_indexr(double testx, double **xarray, int maxcount)
{
   int i=maxcount-1;
   while (((*xarray)[i] > testx) && (i>=0))
   {
      i--;
      if (i < 0)
      {
         return(-1);
      }
   }
   if (fabs((*xarray)[i]-testx) > fabs((*xarray)[i+1]-testx))
   {
      return (i+1);
   }
   else
   {
      return (i);
   }
}

int find_indexr(float testx, float **xarray, int maxcount)
{
   int i=maxcount-1;
   while (((*xarray)[i] > testx) && (i>=0))
   {
      i--;
      if (i < 0)
      {
         return(-1);
      }
   }
   if (fabs((*xarray)[i]-testx) > fabs((*xarray)[i+1]-testx))
   {
      return (i+1);
   }
   else
   {
      return (i);
   }
}

int find_indexr(int testx, int **xarray, int maxcount)
{
   int i=maxcount-1;
   while (((*xarray)[i] > testx) && (i>=0))
   {
      i--;
      if (i < 0)
      {
         return(-1);
      }
   }
   return (i);
}
//
// overloaded function get_limits
// find the smallest and largest entries in a 1-dimensional array
//
void get_1d_limits(double **array, double *lower, double *upper, int count, int start_count)
{
   *upper = (*array)[0];
   *lower = (*array)[0];
   for (int i=start_count+1; i<count; i++)
   {
      *upper = max(*upper, (*array)[i]);
      *lower = min(*lower, (*array)[i]);
   }
}

void get_1d_limits(float **array, float *lower, float *upper, int count, int start_count)
{
   *upper = (*array)[0];
   *lower = (*array)[0];

   for (int i=start_count+1; i<count; i++)
   {
      *upper = max(*upper, (*array)[i]);
      *lower = min(*lower, (*array)[i]);
   }
}

void get_1d_limits(int **array, int *lower, int *upper, int count, int start_count)
{
   *upper = (*array)[0];
   *lower = (*array)[0];
   for (int i=start_count+1; i<count; i++)
   {
      *upper = max(*upper, (*array)[i]);
      *lower = min(*lower, (*array)[i]);
   }
}

//
// overloaded function get_limits
// find the smallest and largest entries in a 2-dimensional array
//
void get_2d_limits(double ***array, double *lower, double *upper, int count1, int count2, int start_count1,
                   int start_count2)
{
   *upper = (*array)[start_count1][start_count2];
   *lower = (*array)[start_count1][start_count2];
   for (int i=start_count1; i<count1; i++)
   {
      for (int j=start_count2; j<count2; j++)
      {
         *upper = max(*upper, (*array)[i][j]);
         *lower = min(*lower, (*array)[i][j]);
      }
   }
}

void get_2d_limits(float ***array, float *lower, float *upper, int count1, int count2, int start_count1,
                   int start_count2)
{
   *upper = (*array)[start_count1][start_count2];
   *lower = (*array)[start_count1][start_count2];
   for (int i=start_count1; i<count1; i++)
   {
      for (int j=start_count2; j<count2; j++)
      {
         *upper = max(*upper, (*array)[i][j]);
         *lower = min(*lower, (*array)[i][j]);
      }
   }
}

void get_2d_limits(int ***array, int *lower, int *upper, int count1, int count2, int start_count1,
                   int start_count2)
{
   *upper = (*array)[start_count1][start_count2];
   *lower = (*array)[start_count1][start_count2];
   for (int i=start_count1; i<count1; i++)
   {
      for (int j=start_count2; j<count2; j++)
      {
         *upper = max(*upper, (*array)[i][j]);
         *lower = min(*lower, (*array)[i][j]);
      }
   }
}

void gaussian_smoothing(float **array, unsigned int smooth, unsigned int points)
{
   unsigned int i, j, k;
   float sum, *temp_array;
   temp_array = new float [points];

   for (i=0; i<points; i++)
   {
      temp_array[i] = (*array)[i];
   }
   if (smooth > 1.0) // we'll apply a normalized Gaussian smoothing kernel that goes out to 2 standard deviations
   {
      float *x_weights, *y_weights;
      float increment = 2.0/(smooth);
      float sum_y;
      int position;
      x_weights       = new float [smooth];
      y_weights       = new float [smooth];
      x_weights[0] = 0.0;
      y_weights[0] = us_normal_distribution(1.0, 0.0, x_weights[0]); // standard deviation = 1.0, Mean = 0;
      for (i=1; i<smooth; i++) //we only calculate half a Gaussian, since the other side is symmetric
      {
         x_weights[i] = x_weights[i-1] + increment;
         y_weights[i] = us_normal_distribution(1, 0, x_weights[i]);
      }

      // first, take care of the left border, using an "appearing frame" algorithm,
      // starting with half a frame visible:

      for(j=0; j<smooth; j++)   // loop over all border point centers
      {
         sum = 0.0;
         sum_y = 0.0;
         position = 0;
         for(k=j; k<j+smooth; k++)
         {
            sum += temp_array[k] * y_weights[position];   // sum the weighted points right of center, including center
            sum_y += y_weights[position];
            position ++;
         }
         position = 0;
         for (int m=j-1; m>=0; m--)
         {
            position ++;
            sum += temp_array[m] * y_weights[position];    // sum all applicable points on the left of center
            sum_y += y_weights[position];
         }
         (*array)[j] = (sum/sum_y);   // normalize by the sum of all weights that were used
      }

      // Now deal with all non-border points:

      for(j=smooth; j<points-smooth; j++)   // loop over all non-border points
      {
         sum = 0.0;
         sum_y = 0.0;
         position = 0;
         for(k=j; k<j+smooth; k++)
         {
            //cout << "k: " << k << ", position: " << position << ", y_weights: " <<  y_weights[position] << ", points: " << points << endl;
            sum += temp_array[k] * y_weights[position];   // sum the weighted points right of center, including center
            sum_y += y_weights[position];
            position ++;
         }
         position = 0;
         for (int m = (int) j-1; m >= (int) (j-smooth+1); m--)
         {
            position ++;
            sum += temp_array[m] * y_weights[position];    // sum all applicable points on the left of center
            sum_y += y_weights[position];
         }
         (*array)[j] = sum/sum_y;   // normalize by the sum of all weights that were used
      }

      // Now deal with all points from the right border, using a "disappearing frame" algorithm,
      // starting with a full frame minus 1 point visible:

      for(j=points-smooth; j<points; j++)   // loop over all right-border points
      {
         sum = 0.0;
         sum_y = 0.0;
         position = 0;
         for (int m= (int)j-1; m>=(int) (j-smooth+1); m--)
         {
            position ++;
            sum += temp_array[m] * y_weights[position];    // sum all points on the left of center
            sum_y += y_weights[position];
         }
         position = 0;
         for(k=j; k<points; k++)
         {
            sum += temp_array[k] * y_weights[position];   // sum the weighted points in the disappearing frame
            sum_y += y_weights[position];               // right of center, including center
            position ++;
         }
         (*array)[j] = sum/sum_y;   // normalize by the sum of all weights that were used
      }
      delete[] x_weights;
      delete[] y_weights;
   }
   delete [] temp_array;
}

double inverse_error_function(double test_val, double step)
{
   double result=0, y=0;
   y += step;

   while (result < test_val)
   {
      result += (2/pow(M_PI, (double) 0.5)) * exp(-1 * pow((double) y, (double) 2)) * step;
      y += step;
   }
   return(y);
}

void calc_vbar(struct peptide *pep, QString *sequence, float *temperature)
{
   pep->vbar_sum = 0.0;
   pep->residues = 0;
   pep->mw = 0;
   pep->weight = 0;
   pep->e280 = 0;
   pep->r = sequence->count("r", Qt::CaseInsensitive);
   pep->l = sequence->count("l", Qt::CaseInsensitive);
   pep->k = sequence->count("k", Qt::CaseInsensitive);
   pep->a = sequence->count("a", Qt::CaseInsensitive);
   pep->v = sequence->count("v", Qt::CaseInsensitive);
   pep->g = sequence->count("g", Qt::CaseInsensitive);
   pep->e = sequence->count("e", Qt::CaseInsensitive);
   pep->n = sequence->count("n", Qt::CaseInsensitive);
   pep->i = sequence->count("i", Qt::CaseInsensitive);
   pep->j = sequence->count("j", Qt::CaseInsensitive);
   pep->o = sequence->count("o", Qt::CaseInsensitive);
   pep->q = sequence->count("q", Qt::CaseInsensitive);
   pep->y = sequence->count("y", Qt::CaseInsensitive);
   pep->p = sequence->count("p", Qt::CaseInsensitive);
   pep->s = sequence->count("s", Qt::CaseInsensitive);
   pep->h = sequence->count("h", Qt::CaseInsensitive);
   pep->t = sequence->count("t", Qt::CaseInsensitive);
   pep->d = sequence->count("d", Qt::CaseInsensitive);
   pep->f = sequence->count("f", Qt::CaseInsensitive);
   pep->b = sequence->count("b", Qt::CaseInsensitive);
   pep->c = sequence->count("c", Qt::CaseInsensitive);
   pep->m = sequence->count("m", Qt::CaseInsensitive);
   pep->w = sequence->count("w", Qt::CaseInsensitive);
   pep->z = sequence->count("z", Qt::CaseInsensitive);
   pep->x = sequence->count("x", Qt::CaseInsensitive);
   pep->x = sequence->count("?");
   pep->dab = sequence->count("+");
   pep->dpr = sequence->count("@");
   pep->residues += pep->r
      + pep->l
      + pep->k
      + pep->a
      + pep->v
      + pep->g
      + pep->e
      + pep->n
      + pep->i
      + pep->j
      + pep->q
      + pep->y
      + pep->o
      + pep->p
      + pep->s
      + pep->h
      + pep->t
      + pep->d
      + pep->f
      + pep->b
      + pep->c
      + pep->m
      + pep->w
      + pep->z
      + pep->x
      + pep->dab
      + pep->dpr;

   pep->mw        += pep->a * 71.1;
   pep->weight    += pep->a * 71.1 * .74;
   pep->vbar_sum    += pep->a * .74;

   pep->mw        += pep->b * 114.6;
   pep->weight    += pep->b * 114.6*.61;
   pep->vbar_sum    += pep->b * .61;

   pep->mw        += pep->c * 103.2;
   pep->weight    += pep->c * 103.2*.63;
   pep->vbar_sum    += pep->c * .63;
   pep->e280      += pep->c * 120;

   pep->mw        += pep->d * 115.1;
   pep->weight    += pep->d * 115.1*.6;
   pep->vbar_sum    += pep->d * .6;

   pep->mw        += pep->e * 129.1;
   pep->weight    += pep->e * 129.1*.66;
   pep->vbar_sum    += pep->e * .66;

   pep->mw        += pep->f * 147.2;
   pep->weight    += pep->f * 147.2*.77;
   pep->vbar_sum    += pep->f * .77;

   pep->mw        += pep->g * 57.1;
   pep->weight    += pep->g * 57.1*.64;
   pep->vbar_sum    += pep->g * .64;

   pep->mw        += pep->h * 137.2;
   pep->weight    += pep->h * 137.2*.67;
   pep->vbar_sum    += pep->h * .67;

   pep->mw        += pep->i * 113.2;
   pep->weight    += pep->i * 113.2*.9;
   pep->vbar_sum    += pep->i * .9;

   pep->mw        += pep->j * 235.2; // James Nowick: Hao
   pep->weight    += pep->j * 235.2*.6497;
   pep->vbar_sum    += pep->j * .6497;
   pep->e280      += pep->j * 9850;

   pep->mw        += pep->dab * 101.1; // John Kulp: diaminobutyric acid
   pep->weight    += pep->dab * 101.1*0.736;
   pep->vbar_sum    += pep->dab * 0.736;

   pep->mw        += pep->dpr * 87.1; // John Kulp: diaminopropanoic acid
   pep->weight    += pep->dpr * 87.1*.669;
   pep->vbar_sum    += pep->dpr * 0.669;

   pep->mw        += pep->k * 128.2;
   pep->weight    += pep->k * 128.2 * .82;
   pep->vbar_sum    += pep->k * .82;

   pep->mw        += pep->l * 113.2;
   pep->weight    += pep->l * 113.2 * .9;
   pep->vbar_sum    += pep->l * .9;

   pep->mw        += pep->m * 131.2;
   pep->weight    += pep->m * 131.2*.75;
   pep->vbar_sum    += pep->m * .75;

   pep->mw        += pep->v * 99.1;
   pep->weight    += pep->v * 99.1 * .86;
   pep->vbar_sum    += pep->v * .86;

   pep->mw        += pep->n * 115.1;
   pep->weight    += pep->n * 115.1*.6;
   pep->vbar_sum    += pep->n * .6;

   pep->mw        += pep->o * 114.2; // James Nowick: delta-linked Ornithine
   pep->weight    += pep->o * 114.2*.7795;
   pep->vbar_sum    += pep->o * .7795;
   pep->e280      += pep->o * 0.0;

   pep->mw        += pep->p * 97.1;
   pep->weight    += pep->p * 97.1*.76;
   pep->vbar_sum    += pep->p * .76;

   pep->mw        += pep->q * 128.1;
   pep->weight    += pep->q * 128.1*.67;
   pep->vbar_sum    += pep->q * .67;

   pep->mw         += pep->r * 156.2;
   pep->weight      += pep->r * .7 * 156.2;
   pep->vbar_sum    += pep->r * .7;

   pep->mw        += pep->s * 87.1;
   pep->weight    += pep->s * 87.1*.63;
   pep->vbar_sum    += pep->s * .63;

   pep->mw        += pep->t * 101.1;
   pep->weight    += pep->t * 101.1*.7;
   pep->vbar_sum    += pep->t * .7;

   pep->mw        += pep->w * 186.2;
   pep->weight    += pep->w * 186.2*.74;
   pep->vbar_sum    += pep->w * .74;
   pep->e280      += pep->w * 5690;

   pep->mw        += pep->x * 119.75;
   pep->weight    += pep->x * 119.75*.722;
   pep->vbar_sum    += pep->x * .722;

   pep->mw        += pep->y * 163.2;
   pep->weight    += pep->y * 163.2*.71;
   pep->vbar_sum    += pep->y * .71;
   pep->e280      += pep->y * 1280;

   pep->mw        += pep->z * 128.6;
   pep->weight    += pep->z * 128.6*.665;
   pep->vbar_sum    += pep->z * .665;

   pep->vbar20 = (pep->weight / pep->mw) - 0.002125;
   pep->vbar = ((pep->weight / pep->mw) + 4.25e-4 * (*temperature - 25));

   //add one water:
   pep->mw +=  (float) 18.01;
}

float adjust_vbar20(float vbar20, float temperature)
{
   return (vbar20 + 0.002125 + 4.25e-4 * (temperature - 25.0));
}

float adjust_vbar(float vbar, float temperature)
{
   return (vbar - 0.002125 - 4.25e-4 * (temperature - 25.0));
}

float calc_buoyancy_tb(float vbar20, float temperature, float density20)
{
   float density_wt, density_wt_20, t, vbar, buoyancy_tb, density_tb;
   vbar = adjust_vbar20(vbar20, temperature);
   t = temperature;
   density_wt    = 1.000028e-3 * ((999.83952 + 16.945176 * t) / (1 + 16.879850e-3 * t))
      - 1.000028e-3 * ((7.9870401e-3 * pow((double) t, (double) 2) + 46.170461e-6 * pow((double) t, (double) 3))
                       / (1 + 16.87985e-3 * t))
      + 1.000028e-3 * ((105.56302e-9 * pow((double) t, (double) 4) - 280.54253e-12 * pow((double) t, (double) 5))
                       / (1 + 16.87985e-3 * t));
   density_wt_20    = 1.000028e-3 * ((999.83952 + 16.945176 * 20) / (1 + 16.879850e-3 * 20))
      - 1.000028e-3 * ((7.9870401e-3 * pow((double) 20, (double) 2) + 46.170461e-6 * pow((double) 20, (double) 3))
                       / (1 + 16.87985e-3 * 20))
      + 1.000028e-3 * ((105.56302e-9 * pow((double) 20, (double) 4) - 280.54253e-12 * pow((double) 20, (double) 5))
                       / (1 + 16.87985e-3 * 20));

   density_tb   = density20 * density_wt / density_wt_20;
   buoyancy_tb    = 1 - vbar * density_tb;
   return(buoyancy_tb);
}

float calc_bottom(vector <struct rotorInfo> rotor_list,
                  vector <struct centerpieceInfo> cp_list,
                  int rotor, int centerpiece, int channel, unsigned int rpm)
{
   if (centerpiece < 0)
   {
      return(-1.0);
   }
   float bottom;
   bottom = cp_list[centerpiece].bottom_position[channel];
   if (rpm != 0)
   {
      for (unsigned int i=0; i<5; i++)
      {
         bottom += rotor_list[rotor].coefficient[i] * pow((double) rpm, (double) i);
      }
   }
   return bottom;
}

#if !defined(CMDLINE)
float calc_bottom(int rotor, int centerpiece, int channel, unsigned int rpm)
{
   if (centerpiece < 0)
   {
      return(-1.0);
   }
#if !defined(USE_MPI)
   US_Config *USglobal;
   USglobal = new US_Config();
#endif
   vector <struct rotorInfo> rotor_list;
   vector <struct centerpieceInfo> cp_list;
   cp_list.clear( );
   rotor_list.clear( );
   if (!readCenterpieceInfo(&cp_list))
   {
#if !defined(USE_MPI)
      // cerr << "UltraScan Fatal Error: There was a problem opening the\n"
      //    "centerpiece database file:\n\n" + USglobal->config_list.system_dir + "/etc/centerpiece.dat\n\n"
      //    "Please install the centerpiece database file\n"
      //    "before proceeding. Exiting with -2..." << endl;
      delete USglobal;
#endif
      return(-2.0); // centerpiece could not be read
   }
   if (!readRotorInfo(&rotor_list))
   {
#if !defined(USE_MPI)
      // cerr << "UltraScan Fatal Error: There was a problem opening the\n"
      //    "rotor database file:\n\n" + USglobal->config_list.system_dir + "/etc/rotor.dat\n\n"
      //    "Please install the rotor database file\n"
      //    "before proceeding. Exiting with -3..." << endl;
      delete USglobal;
#endif
      return(-3.0);
   }

   float bottom;
   bottom = cp_list[centerpiece].bottom_position[channel];
   if (rpm != 0)
   {
      for (unsigned int i=0; i<5; i++)
      {
         bottom += rotor_list[rotor].coefficient[i] * pow((double) rpm, (double) i);
      }
   }
#if !defined(USE_MPI)
   delete USglobal;
#endif
   return bottom;
}

double stretch(int rotor, unsigned int rpm)
{
   vector <struct rotorInfo> rotor_list;
   rotor_list.clear( );
   double stretch = 0.0;
   if( readRotorInfo(&rotor_list) )
   {
      for (unsigned int i=0; i<5; i++)
      {
         stretch += rotor_list[rotor].coefficient[i] * pow((double) rpm, (double) i);
      }
   } 
   else
   {
      fprintf(stderr,"ERROR reading rotor info in us_math ... no stretch!\n");
   }
   rotor_list.clear( );
   return (stretch);
}
#else
float calc_bottom(int , int , int , unsigned int ) { return 0; }
double stretch(int , unsigned int ) { return 0; }
#endif


double stretch_with_rotor_list(int rotor, unsigned int rpm, vector <rotorInfo> *rotor_list)
{
   double stretch = 0.0;
   for (unsigned int i=0; i<5; i++)
   {
      stretch += (*rotor_list)[rotor].coefficient[i] * pow((double) rpm, (double) i);
   }
   return (stretch);
}

/*****************************************************************************

  nnls.c  (c) 2002 Turku PET Centre

  This file contains the routine NNLS (nonnegative least squares)
  and the subroutines required by it.

  This routine is based on the text and fortran code in
  C.L. Lawson and R.J. Hanson, Solving Least Squares Problems,
  Prentice-Hall, Englewood Cliffs, New Jersey, 1974.

  Version:
  2002-08-19 Vesa Oikonen


*****************************************************************************/

/* Local function definitions */
void _nnls_g1(double a, double b, double *cterm, double *sterm, double *sig);
int _nnls_h12(int mode, int lpivot, int l1, int m, double *u, int iue,
              double *up, double *cm, int ice, int icv, int ncv);
/****************************************************************************/

/*****************************************************************************
 *  Algorithm NNLS (Non-negative least-squares)
 *
 *  Given an m by n matrix A, and an m-vector B, computes an n-vector X,
 *  that solves the least squares problem
 *      A * X = B   , subject to X>=0
 *  The A matrix is formatted such that the columns are placed into a vector
 *  end-to-end, and the parameter a_dim1 contains the length of each column
 *
 *  Function returns 0 if succesful, 1, if iteration count exceeded 3*N,
 *  or 2 in case of invalid problem dimensions or memory allocation error.
 *
 *  Instead of pointers for working space, NULL can be given to let this
 *  function to allocate and free the required memory.
 */
int nnls(
         double *a, int a_dim1, int m, int n,
         /* On entry, a[] contains the m by n matrix A. On exit, a[] contains the
            product matrix Q*A, where Q is an m by n orthogonal matrix generated
            implicitly by this function. Since matrix A is processed as a set of
            vectors, a_dim1 is needed to specify the storage increment between
            vectors in a[] */
         double *b,
         /* On entry, b[] must contain the m-vector B.
            On exit, b[] contains Q*B */
         double *x,
         /* On exit, x[] will contain the solution vector */
         double *rnorm,
         /* On exit, rnorm contains the Euclidean norm of the residual vector */
         double *wp,  /* An n-array of working space, w[]. */
         /* On exit, w[] will contain the dual solution vector.
            w[i]=0.0 for all i in set p and w[i]<=0.0 for all i in set z. */
         double *zzp, /* An m-array of working space, zz[]. */
         int *indexp  /* An n-array of working space, index[]. */
         ) {
   int pfeas, ret=0, iz, jz, iz1, iz2, npp1, *index;
   double d1, d2, sm, up, ss, *w, *zz;
   int iter, k, j=0, l, itmax, izmax=0, nsetp, ii, jj=0, ip;
   double temp, wmax, t, alpha, asave, dummy, unorm, ztest, cc;


   /* Check the parameters and data */
   if(m<=0 || n<=0 || a==NULL || b==NULL || x==NULL) return(2);
   /* Allocate memory for working space, if required */
   if(wp!=NULL) w=wp; else w=(double*)calloc(n, sizeof(double));
   if(zzp!=NULL) zz=zzp; else zz=(double*)calloc(m, sizeof(double));
   if(indexp!=NULL) index=indexp; else index=(int*)calloc(n, sizeof(int));
   if(w==NULL || zz==NULL || index==NULL) return(2);

   /* Initialize the arrays INDEX[] and X[] */
   for(k=0; k<n; k++) {x[k]=0.; index[k]=k;}
   iz2=n-1; iz1=0; nsetp=0; npp1=0;

   /* Main loop; quit if all coeffs are already in the solution or */
   /* if M cols of A have been triangularized */
   iter=0; itmax=n*3;
   while(iz1<=iz2 && nsetp<m) {
      /* Compute components of the dual (negative gradient) vector W[] */
      for(iz=iz1; iz<=iz2; iz++) {
         j=index[iz]; sm=0.; for(l=npp1; l<m; l++) sm+=a[l+j*a_dim1]*b[l];
         w[j]=sm;
      }

      while(1) {
         /* Find largest positive W[j] */
         for(wmax=0., iz=iz1; iz<=iz2; iz++) {
            j=index[iz]; if(w[j]>wmax) {wmax=w[j]; izmax=iz;}}

         /* Terminate if wmax<=0.; */
         /* it indicates satisfaction of the Kuhn-Tucker conditions */
         if(wmax<=0.0) break;
         iz=izmax; j=index[iz];

         /* The sign of W[j] is ok for j to be moved to set P. */
         /* Begin the transformation and check new diagonal element to avoid */
         /* near linear dependence. */
         asave=a[npp1+j*a_dim1];
         _nnls_h12(1, npp1, npp1+1, m, &a[j*a_dim1], 1, &up, &dummy, 1, 1, 0);
         unorm=0.;
         if(nsetp!=0) for(l=0; l<nsetp; l++) {d1=a[l+j*a_dim1]; unorm+=d1*d1;}
         unorm=sqrt(unorm);
         d2=unorm+(d1=a[npp1+j*a_dim1], fabs(d1)) * 0.01;
         if((d2-unorm)>0.) {
            /* Col j is sufficiently independent. Copy B into ZZ, update ZZ */
            /* and solve for ztest ( = proposed new value for X[j] ) */
            for(l=0; l<m; l++) zz[l]=b[l];
            _nnls_h12(2, npp1, npp1+1, m, &a[j*a_dim1], 1, &up, zz, 1, 1, 1);
            ztest=zz[npp1]/a[npp1+j*a_dim1];
            /* See if ztest is positive */
            if(ztest>0.) break;
         }

         /* Reject j as a candidate to be moved from set Z to set P. Restore */
         /* A[npp1,j], set W[j]=0., and loop back to test dual coeffs again */
         a[npp1+j*a_dim1]=asave; w[j]=0.;
      } /* while(1) */
      if(wmax<=0.0) break;

      /* Index j=INDEX[iz] has been selected to be moved from set Z to set P. */
      /* Update B and indices, apply householder transformations to cols in */
      /* new set Z, zero subdiagonal elts in col j, set W[j]=0. */
      for(l=0; l<m; ++l) b[l]=zz[l];
      index[iz]=index[iz1]; index[iz1]=j; iz1++; nsetp=npp1+1; npp1++;
      if(iz1<=iz2) for(jz=iz1; jz<=iz2; jz++) {
         jj=index[jz];
         _nnls_h12(2, nsetp-1, npp1, m, &a[j*a_dim1], 1, &up,
                   &a[jj*a_dim1], 1, a_dim1, 1);
      }
      if(nsetp!=m) for(l=npp1; l<m; l++) a[l+j*a_dim1]=0.;
      w[j]=0.;
      /* Solve the triangular system; store the solution temporarily in Z[] */
      for(l=0; l<nsetp; l++) {
         ip=nsetp-(l+1);
         if(l!=0) for(ii=0; ii<=ip; ii++) zz[ii]-=a[ii+jj*a_dim1]*zz[ip+1];
         jj=index[ip]; zz[ip]/=a[ip+jj*a_dim1];
      }

      /* Secondary loop begins here */
      while(++iter<itmax) {
         /* See if all new constrained coeffs are feasible; if not, compute alpha */
         for(alpha=2.0, ip=0; ip<nsetp; ip++) {
            l=index[ip];
            if(zz[ip]<=0.) {t=-x[l]/(zz[ip]-x[l]); if(alpha>t) {alpha=t; jj=ip-1;}}
         }

         /* If all new constrained coeffs are feasible then still alpha==2. */
         /* If so, then exit from the secondary loop to main loop */
         if(alpha==2.0) break;
         /* Use alpha (0.<alpha<1.) to interpolate between old X and new ZZ */
         for(ip=0; ip<nsetp; ip++) {l=index[ip]; x[l]+=alpha*(zz[ip]-x[l]);}

         /* Modify A and B and the INDEX arrays to move coefficient i */
         /* from set P to set Z. */
         k=index[jj+1]; pfeas=1;
         do {
            x[k]=0.;
            if(jj!=(nsetp-1)) {
               jj++;
               for(j=jj+1; j<nsetp; j++) {
                  ii=index[j]; index[j-1]=ii;
                  _nnls_g1(a[j-1+ii*a_dim1], a[j+ii*a_dim1], &cc, &ss, &a[j-1+ii*a_dim1]);
                  for(a[j+ii*a_dim1]=0., l=0; l<n; l++) if(l!=ii) {
                     /* Apply procedure G2 (CC,SS,A(J-1,L),A(J,L)) */
                     temp=a[j-1+l*a_dim1];
                     a[j-1+l*a_dim1]=cc*temp+ss*a[j+l*a_dim1];
                     a[j+l*a_dim1]=-ss*temp+cc*a[j+l*a_dim1];
                  }
                  /* Apply procedure G2 (CC,SS,B(J-1),B(J)) */
                  temp=b[j-1]; b[j-1]=cc*temp+ss*b[j]; b[j]=-ss*temp+cc*b[j];
               }
            }
            npp1=nsetp-1; nsetp--; iz1--; index[iz1]=k;

            /* See if the remaining coeffs in set P are feasible; they should */
            /* be because of the way alpha was determined. If any are */
            /* infeasible it is due to round-off error. Any that are */
            /* nonpositive will be set to zero and moved from set P to set Z */
            for(jj=0; jj<nsetp; jj++) {k=index[jj]; if(x[k]<=0.) {pfeas=0; break;}}
         } while(pfeas==0);

         /* Copy B[] into zz[], then solve again and loop back */
         for(k=0; k<m; k++) zz[k]=b[k];
         for(l=0; l<nsetp; l++) {
            ip=nsetp-(l+1);
            if(l!=0) for(ii=0; ii<=ip; ii++) zz[ii]-=a[ii+jj*a_dim1]*zz[ip+1];
            jj=index[ip]; zz[ip]/=a[ip+jj*a_dim1];
         }
      } /* end of secondary loop */
      if(iter>itmax) {ret=1; break;}
      for(ip=0; ip<nsetp; ip++) {k=index[ip]; x[k]=zz[ip];}
   } /* end of main loop */
   /* Compute the norm of the final residual vector */
   sm=0.;
   if(npp1<m) for(k=npp1; k<m; k++) sm+=(b[k]*b[k]);
   else for(j=0; j<n; j++) w[j]=0.;
   *rnorm=sqrt(sm);
   /* Free working space, if it was allocated here */
   if(wp==NULL) free(w);
   if(zzp==NULL) free(zz);
   if(indexp==NULL) free(index);
   return(ret);
} /* nnls_ */
/****************************************************************************/

/*****************************************************************************
 *
 *  Compute orthogonal rotation matrix:
 *    (C, S) so that (C, S)(A) = (sqrt(A**2+B**2))
 *    (-S,C)         (-S,C)(B)   (   0          )
 *  Compute sig = sqrt(A**2+B**2):
 *    sig is computed last to allow for the possibility that sig may be in
 *    the same location as A or B.
 */
void _nnls_g1(double a, double b, double *cterm, double *sterm, double *sig)
{
   double d1, xr, yr;

   if(fabs(a)>fabs(b)) {
      xr=b/a; d1=xr; yr=sqrt(d1*d1 + 1.); d1=1./yr;
      *cterm=(a>=0.0 ? fabs(d1) : -fabs(d1));
      *sterm=(*cterm)*xr; *sig=fabs(a)*yr;
   } else if(b!=0.) {
      xr=a/b; d1=xr; yr=sqrt(d1*d1 + 1.); d1=1./yr;
      *sterm=(b>=0.0 ? fabs(d1) : -fabs(d1));
      *cterm=(*sterm)*xr; *sig=fabs(b)*yr;
   } else {
      *sig=0.; *cterm=0.; *sterm=1.;
   }
} /* _nnls_g1 */
/****************************************************************************/

/*****************************************************************************
 *
 *  Construction and/or application of a single Householder transformation:
 *           Q = I + U*(U**T)/B
 *
 *  Function returns 0 if succesful, or >0 in case of erroneous parameters.
 *
 */
int _nnls_h12(
              int mode,
              /* mode=1 to construct and apply a Householder transformation, or
                 mode=2 to apply a previously constructed transformation */
              int lpivot,     /* Index of the pivot element */
              int l1, int m,
              /* Transformation is constructed to zero elements indexed from l1 to M */
              double *u, int u_dim1, double *up,
              /* With mode=1: On entry, u[] must contain the pivot vector.
                 On exit, u[] and up contain quantities defining the vector u[] of
                 the Householder transformation. */
              /* With mode=2: On entry, u[] and up should contain quantities previously
                 computed with mode=1. These will not be modified. */
              /* u_dim1 is the storage increment between elements. */
              double *cm,
              /* On entry, cm[] must contain the matrix (set of vectors) to which the
                 Householder transformation is to be applied. On exit, cm[] will contain
                 the set of transformed vectors */
              int ice,        /* Storage increment between elements of vectors in cm[] */
              int icv,        /* Storage increment between vectors in cm[] */
              int ncv         /* Nr of vectors in cm[] to be transformed;
                                 if ncv<=0, then no operations will be done on cm[] */
              ) {
   double d1, d2, b, clinv, cl, sm;
   int incr, k, j, i2, i3, i4;

   /* Check parameters */
   if(mode!=1 && mode!=2) return(1);
   if(m<1 || u==NULL || u_dim1<1 || cm==NULL) return(2);
   if(lpivot<0 || lpivot>=l1 || l1>=m) return(0);
   /* Function Body */
   cl= (d1 = u[lpivot*u_dim1], fabs(d1));
   if(mode==2) { /* Apply transformation I+U*(U**T)/B to cm[] */
      if(cl<=0.) return(0);
   } else { /* Construct the transformation */
      for(j=l1; j<m; j++) { /* Computing MAX */
         d2=(d1=u[j*u_dim1], fabs(d1)); if(d2>cl) cl=d2;}
      if(cl<=0.) return(0);
      clinv=1.0/cl;
      /* Computing 2nd power */
      d1=u[lpivot*u_dim1]*clinv; sm=d1*d1;
      for(j=l1; j<m; j++) {d1=u[j*u_dim1]*clinv; sm+=d1*d1;}
      cl*=sqrt(sm); if(u[lpivot*u_dim1]>0.) cl=-cl;
      *up=u[lpivot*u_dim1]-cl; u[lpivot*u_dim1]=cl;
   }
   if(ncv<=0) return(0);
   b=(*up)*u[lpivot*u_dim1];
   /* b must be nonpositive here; if b>=0., then return */
   if(b>=0.) return(0);
   b=1.0/b; i2=1-icv+ice*lpivot; incr=ice*(l1-lpivot);
   for(j=0; j<ncv; j++) {
      i2+=icv; i3=i2+incr; i4=i3; sm=cm[i2-1]*(*up);
      for(k=l1; k<m; k++) {sm+=cm[i3-1]*u[k*u_dim1]; i3+=ice;}
      if(sm!=0.0) {
         sm*=b; cm[i2-1]+=sm*(*up);
         for(k=l1; k<m; k++) {cm[i4-1]+=sm*u[k*u_dim1]; i4+=ice;}
      }
   }
   return(0);
} /* _nnls_h12 */


double us_erfc(double x)
{
   // error function erfc(x) with fractional error everywhere less
   // than 1.2 × 10 7.
   double t, z, ans;
   z = fabs(x);
   t = 1.0/(1.0+0.5*z);
   ans = t*exp(-z*z-1.26551223+t*(1.00002368+t*(0.37409196+t*(0.09678418+
                                                              t*(-0.18628806+t*(0.27886807+t*(-1.13520398+t*(1.48851587+
                                                                                                             t*(-0.82215223+t*0.17087277)))))))));
   return x >= 0.0 ? ans : 2.0-ans;
}


#if defined( WIN32 ) && !defined( MINGW )

typedef unsigned int u_int32_t;
typedef union
{
   float value;
   u_int32_t word;
} ieee_float_shape_type;

#define GET_FLOAT_WORD(i,d)         \
  do {                              \
      ieee_float_shape_type gf_u;   \
      gf_u.value = (d);             \
      (i) = gf_u.word;              \
  } while (0)

int __fpclassifyf (float x)
{
   u_int32_t wx;
   int retval = FP_NORMAL;

   GET_FLOAT_WORD (wx, x);
   wx &= 0x7fffffff;
   if (wx == 0)
      retval = FP_ZERO;
   else if (wx < 0x800000)
      retval = FP_SUBNORMAL;
   else if (wx >= 0x7f800000)
      retval = wx > 0x7f800000 ? FP_NAN : FP_INFINITE;

   return retval;
}
#endif // WIN32

float int_vol_2sphere(float r1, float r2, float d) {
   // r1 & r2 are radii of spheres, d is distance between centers
   // returns volume
   float tmp;

#if defined(DEBUG)
   printf("r1 %f r2 %f d %f\n", r1, r2, d);
#endif

   if (d > r1 + r2) {
      // no intersection
#if defined(DEBUG)
      puts("no intersection");
#endif
      return 0;
   }

   if (r1 < r2) {
      tmp = r2;
      r2 = r1;
      r1 = tmp;
   }

   if (d + r2 < r1) {
      // smaller is fully subsumed
#if defined(DEBUG)
      puts("smaller is fully subsumed");
#endif
      return (4.0/3) * M_PI * pow(r2,3);
   }

   float cy = (d + (r1*r1 - r2*r2)/d) * .5;

   if (d - r2 > cy || r1 < cy) {
     // fprintf(stderr, "error: range inconsistancy! %f %f %f\n", d - r2, cy, r1);
      //    exit(-1);
   }

#if defined(DEBUG)
   printf("cy %f\n", cy);
#endif

   float vol = 
      (M_PI / 3.0) * 
      ((2.0 * r2 + d - cy) * (cy - d + r2) * (cy - d + r2) +
       (2.0 * r1 + cy) * (r1 - cy) * (r1 - cy));

   return vol;
}

QString us_double_decimal_places( double x, int dp ) {
   if ( QString( "%1" ).arg( x ).contains( "e" ) ) {
      // punt for exponential form
      return QString( "%1" ).arg( x );
   }

   double fact = (int) pow( 10, dp );

   double xuse = (double)( ((int)((x * fact) + ( x > 0 ? 0.5 : -0.5 ))) / fact);
   QString qs = QString( "%1").arg( xuse );
   if ( qs.contains( "e" ) ) {
      // punt for exponential form
      return QString( "%1" ).arg( x );
   }
   if ( !dp ) {
      // no decimal
      return qs;
   }

   if ( !qs.contains( "." ) ) {
      qs += ".";
      for ( int i = 0; i < dp; ++i ) {
         qs += "0";
      }
      return qs;
   }

   // how many zeros to add?

   QString pastdp = qs;
   pastdp.replace( QRegularExpression( "^[^.]*\\." ), "" );
   int toadd = dp - (int) pastdp.length();
   for ( int i = 0; i < toadd; ++i ) {
      qs += "0";
   }
   return qs;
};
