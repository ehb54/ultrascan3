#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/us_ga_s_estimate.h"

double s_estimates[MAX_SOLUTES][2];        // s values for start & end of range
double ff0_estimates[MAX_SOLUTES][2];      // ff0 values for start & end of range
double s_estimates_scaled[MAX_SOLUTES][2]; // s values scaled for gradient search methods
double s_estimate_weights[MAX_SOLUTES];    // freq sum for this range
double s_estimate_max[MAX_SOLUTES];        // max freq
double s_estimate_max_s[MAX_SOLUTES];      // s value corresponding to max freq
int s_estimate_solutes;                    // # of s values found
int s_estimate_indices[MAX_SOLUTES];      // indices into s_estimates for inactive solute skipping

int s_estimate(char *filename) {
   //  read vhw_his file & store data in s_estimates
   FILE *load_file;
   char line[MAX_INPUT_LIN_LENGTH];
   char *p;
   double this_s;
   double this_freq;
   int nonzero = 0;
   int pos = -1;

   s_estimate_solutes = 0;
   if((load_file = fopen(filename, "r")) == NULL) {
      fprintf(stderr,"vwh raw file open error %s\n", filename);
      return -1;
   }

   // file format "S-value (Envelope)"   "Frequency"   "S-value (Histogram)"   "Frequency:"
   if(fgets(line, sizeof(line), load_file) == NULL) {
      fprintf(stderr,"vhw raw file format error 1 %s\n", filename);
      exit(-1);
   }

   if(!strncmp("manual", line, 6)) {
      // use manual partitioning
      puts("s-estimates using manual partitioning");
      while(fgets(line, sizeof(line), load_file)) {
         // format start s-value \t end s-value (unscaled, i.e. not *1e-13)
         p = line;
         s_estimates[s_estimate_solutes][0] = atof(p) * 1e-13;
         if((p = strchr(p, '\t')) == NULL) {
            fprintf(stderr,"vhw (manual) raw file format error 2 %s\n", filename);
            exit(-1);
         }
         p++;
         s_estimates[s_estimate_solutes][1] = atof(p) * 1e-13;
         s_estimate_solutes++;
      }
    
      for(pos = 0; pos < s_estimate_solutes; pos++) {
         if(s_estimates[pos][0] >= s_estimates[pos][1] ||
            s_estimates[pos][0] < MIN_S ||
            s_estimates[pos][1] > MAX_S ||
            (pos > 0 &&
             s_estimates[pos - 1][1] >= s_estimates[pos][0])) {
            fprintf(stderr,
                    "manual s-estimates error on line %d %g %g\n",
                    pos, s_estimates[pos][0],  s_estimates[pos][1]);
            exit(-1);
         }
         s_estimate_weights[pos] = 1e0;
         s_estimate_max_s[pos] = (s_estimates[pos][0] + s_estimates[pos][1]) / 2e0;
      }
   } // end 'use manual partitioning'
   else {
      // not manual, vhw file
      puts("s-estimates using vhw partitioning");
     
      while(fgets(line, sizeof(line), load_file)) {
         p = line;
         // s-value (envelope)
         if((p = strchr(p, '\t')) == NULL) {
            fprintf(stderr,"vhw raw file format error 2 %s\n", filename);
            exit(-1);
         }
         p++;
         // frequency
         if((p = strchr(p, '\t')) == NULL) {
            // end of file
            break;
         }
         if((p = strchr(p, '\t'))) {
            p++;
            // s-value
            this_s = atof(p);
            this_s *= 1e-13;
            if((p = strchr(p, '\t')) == NULL) {
               fprintf(stderr,"vhw raw file format error 3 %s\n", filename);
               exit(-1);
            }
            p++;
            // frequency
            this_freq = atof(p);
   
            if(nonzero) {            // we are previously accumulating data
               if(this_freq > 0e0) {    // we add to this data
                  s_estimates[pos][1] = this_s;
                  s_estimate_weights[pos] += this_freq;
                  if(s_estimate_max[pos] < this_freq) {
                     s_estimate_max[pos] = this_freq;
                     s_estimate_max_s[pos] = this_s;
                  }
               } else {                  // we close off our accumulation
                  nonzero = 0;
                  if(s_estimate_weights[pos] < WEIGHT_TRIM) {
                     pos--;
                     s_estimate_solutes--;
                  }
               }
            } else {                 // we are not previously accumulating data
               if(this_freq > 0e0) {    // we have to start accumulating
                  nonzero = 1;
                  pos++;
                  s_estimate_solutes++;
       
                  s_estimates[pos][0] = this_s;
                  s_estimates[pos][1] = this_s;
                  s_estimate_weights[pos] = this_freq;
                  s_estimate_max[pos] = this_freq;
                  s_estimate_max_s[pos] = this_s;
               } else {                  // nothing to do
               }
            }
         }
      }
      if(nonzero) {
         if(s_estimate_weights[pos] < WEIGHT_TRIM) {
            pos--;
            s_estimate_solutes--;
         }
      }
    
      printf("# of peaks %d\n", s_estimate_solutes);
      for(pos = 0; pos < s_estimate_solutes; pos++) {
         printf("%d start %g end %g weight %g peak %g\n", 
                pos,
                s_estimates[pos][0],
                s_estimates[pos][1],
                s_estimate_weights[pos],
                s_estimate_max_s[pos]);
      }
      puts("after extension");
      pos = 0;
    
      s_estimates[pos][0] -= s_estimates[pos][0] * PCT_EXTENSION / 1e2;
      if(s_estimates[pos][0] < MIN_S) {
         s_estimates[pos][0] = MIN_S;
      }
      s_estimates[pos][1] += s_estimates[pos][1] * PCT_EXTENSION / 1e2;
      if(s_estimates[pos][1] > MAX_S) {
         s_estimates[pos][0] = MAX_S;
      }

      for(pos = 1; pos < s_estimate_solutes; pos++) {
         s_estimates[pos][0] -= s_estimates[pos][0] * PCT_EXTENSION / 1e2;
         if(s_estimates[pos][0] < MIN_S) {
            s_estimates[pos][0] = MIN_S;
         }
         if(s_estimates[pos][0] < s_estimates[pos-1][1]) {
            s_estimates[pos][0] = s_estimates[pos-1][1];
         }
         s_estimates[pos][1] += s_estimates[pos][1] * PCT_EXTENSION / 1e2;
         if(s_estimates[pos][1] > MAX_S) {
            s_estimates[pos][0] = MAX_S;
         }
      }
   } // end of 'not manual, vhw file'
   for(pos = 0; pos < s_estimate_solutes; pos++) {
      s_estimates_scaled[pos][0] = s_estimates[pos][0] * 1e13;
      s_estimates_scaled[pos][1] = s_estimates[pos][1] * 1e13;
   }
   for(pos = 0; pos < s_estimate_solutes; pos++) {
      printf("%d start %g end %g gsm [%g,%g] weight %g peak %g\n", 
             pos,
             s_estimates[pos][0],
             s_estimates[pos][1],
             s_estimates_scaled[pos][0],
             s_estimates_scaled[pos][1],
             s_estimate_weights[pos],
             s_estimate_max_s[pos]);
   }
   return(s_estimate_solutes);
}


