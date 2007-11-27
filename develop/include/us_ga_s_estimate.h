#ifndef US_GA_S_ESTIMATE_H
#define US_GA_S_ESTIMATE_H

extern int s_estimate(char *);

#define MAX_INPUT_LIN_LENGTH 4096
#define PCT_EXTENSION 10
#define MIN_S (   .01 * 1e-13 )
#define MAX_S ( 20.0  * 1e-13 )

#define MIN_DIFFUSION 1e-7
#define MAX_DIFFUSION 10e-7
#define MIN_DIFFUSION_SCALED 1
#define MAX_DIFFUSION_SCALED 10

#define MAX_SOLUTES 100
#define WEIGHT_TRIM 3                      // segments whose total weight is less than this value will be eliminated

extern double s_estimates[MAX_SOLUTES][2];        // s values for start & end of range
extern double ff0_estimates[MAX_SOLUTES][2];      // ff0 values for start & end of range
extern double s_estimates_scaled[MAX_SOLUTES][2]; // scaled for gradient search methods
extern double s_estimate_weights[MAX_SOLUTES];    // freq sum for this range
extern double s_estimate_max[MAX_SOLUTES];        // max freq
extern double s_estimate_max_s[MAX_SOLUTES];      // s value corresponding to max freq
extern int s_estimate_solutes;                    // # of s values found
extern int s_estimate_indices[MAX_SOLUTES];      // indices into s_estimates for inactive solute skipping

#endif
