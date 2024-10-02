/*       PROGRAM SUPC VERSION 4.0 (Updated 31 Jan 2001)       */
/*                            4.1 (Updated  3 Jun 2004)       */
/*               4.2 (Updated 14 Jun 2007) introduced overlap choice in over.c */
/*                                    introduced version # in supc.c */
/*                                    fixed MAX EXT in supc.c */
/*               5.0 (SOMO UltraScan integration, 2009-2010   */
/*       Readapted for Linux from Win version 15/5/2002       */
/* Computation of the hydrodynamic properties of rigid models */
/* composed of non-overlapping beads of different radii.      */

// #define DEBUG_FILES
// #define DEBUG_EV
#include "../include/us_math.h"
#include <qregexp.h>

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

// #define OLD_WAY_CHECK
// #define  PI M_PI
// below are moved to variable values
// #define  ETAo VISC_20W
// #define  DENS DENS_20W
// #define  TE 293.15
#define  AVO AVOGADRO
#define  KB  (8.314472e+07/AVOGADRO)
#define  SMAX 512
#define  RM_COMMAND "rm "
#include <stdlib.h>      /* Added for 'print_time' function use */

#include <stdio.h>
#include <math.h>
#include <time.h>
#ifdef OSX
#  include <sys/malloc.h>
#endif
#include <string.h>

#include "../include/us_hydrodyn_supc.h"
#include "../include/us_hydrodyn_pat.h"

// #define USE_THREADS
// only define exactly one of MODE_1 & MODE_2 below
#define MODE_1 
// #define MODE_2
// #define DEBUG_THREAD
#if defined(USE_THREADS)
  static int threads;
  static vector < supc_thr_t* > supc_thr_threads;
#endif

// #define SHOW_TIMING
#if defined(SHOW_TIMING)
# include <sys/time.h>
  static struct timeval s3tv0;
//  static struct timeval tv1;
//  static struct timeval tv2;
  static struct timeval s3tv3;
  static struct timeval s3tv4;
  static struct timeval s3tv5;
//  static struct timeval tv6;
  static struct timeval s3tv7;
  static struct timeval s3tv8;
  static struct timeval s3tv9;
  static struct timeval s1tv0;
  static struct timeval s1tv9;
  static struct timeval s2tv0;
  static struct timeval s2tv9;
  unsigned long cholsl_s1;
  unsigned long cholsl_s2;
  unsigned long supc3_s1;
  unsigned long supc3_s2;
  unsigned long supc1_tot;
  unsigned long supc2_tot;
  unsigned long supc3_tot;
  unsigned long supc_tot;
#endif

#undef R // it's defined as a us #define above & we use R as a local variable

static QProgressBar * smi_progress;
static void ev3( vector < vector < long double > > & );
static double ETAo;
static double DENS;
static double TE;
static QString tag1;
static QString tag2;
static hydro_results *supc_results;
static FILE *mol;
static FILE *rmc;
#if defined(USE_MAIN)
static FILE *new_mol;
#endif
static FILE *new_mol1;
static FILE *new_rmc;
static FILE *ris;
#if defined(CREATE_EXE_TIME)
 static FILE *exe_time;
#endif


#if defined(CREATE_TOT_MOL)
 static FILE *tot_mol;
#else
 static vector < QString > molecola_v;
 static vector < int > nat_v;
 static vector < int > prima_v;
 static vector < int > ultima_v;
 static vector < float > raggio_v;
#endif

#if defined(OLD_WAY_CHECK)
 static FILE *interinp;
 static FILE *interinp1;
#endif
#if defined(OLD_WAY)
 static FILE *interout;
#endif

static char molecola[SMAX+1];
static char ragcol[SMAX+1];
static char risultati[SMAX+1];
static QString qs_risultati;
static char molecola_nb[SMAX+1];
static char fil001[SMAX+1];
// static char corresp[SMAX+1];
// static char data_stp;

static int numero_sfere;
static int prima;
static int ultima;
static int num;
static int kkk;
#if defined(USE_MAIN)
static int fe;
#endif
static int colorzero;
static int colorsix;
static int colorsixf;
static int metodo;
static int scelta;
// static int metod1;
static int volcor;
static int sfecalc;
static int mascor;
static float mascor1;
// static int d;
static int cd;
static int cc;
static int nat;
// static int natt;
// static int ind1;
// static int ind2;
static int vt;
static int flag_norm;
static int cdmolix;
static int num001;
#if defined(USE_MAIN)
static int numor;
#endif
// static int i;
// static int k;
/* int annov; */
// static int flag_mem;

static float isigmat[9];
static float sigmat[9];
static float sigmaRr[9];
static float sigmaoc[9];
static float soct[9];
static float sigmaoR[9];
static float roR[3];
static float inver[6][6];
static float Doc[9];
static float Doct[9];
static float Dot[9];
static float cv[9];
static long double Dr[9];
static float roD[3];
static float DDt[9];
static float taoh;
static float taom;
static float taod;
static float stdinv;
static float taodin;
static long double tao[5];
// static float aa;
// static float qq;
// static float ri;
// static float rj;
// static float del;
// static float d1;
// static float d2;
// static float rr;
// static float tt;
static float f;
// static float tol;
static float raggio;
static float partvol;
static float org_vbar;
static float tc_vbar;
static float tot_partvol;
static float partvolc;
static float partvolc1;
static float partvolc2;
static float xm;
static float ym;
static float zm;
static float mtx;
static float raflag;
static float taoflag;
static float vT[9];
static float Tv[9];
static float bb[3];
static float vis;
#if defined(TSUDA_DOUBLESUM)
static float vis1;
static float vis2;
#endif
static float vis3;
#if defined(TSUDA_DOUBLESUM)
static float vis4;
#endif
static float vc[3];
static float ro;
static float rou;
static float pesmol;
static float totvol;
static float totsup;
static float interm1;
static float totvolb;
static long double dl1;
static long double dl2;
static long double dl3;
static float fconv;
static float fconv1;
static float volcor1;
static float vol_mas;
static float correz;
static float Rg;
static float Rgu;
static float RSt;
static float ff0_t;
static float RSr[3];
static float CdT;
static float CfT;
static float CST;
static float CSTF;
static float CdR1;
static float CfR1;
static float CfR[3];
static float CdR[3];
static float RT[5];
static float CT[5];
static float CTH;
static float CTM;
static float VIM;
static float VIMC;
#if defined(TSUDA_DOUBLESUM)
static float VIMDS;
static float VIMDS2;
static float REDS;
static float REDS2;
static float RETM;
static float RETM2;
static float RETV;
static float RETV2;
static float VIMTM;
static float VIMTM2;
static float VIMTV;
static float VIMTV2;
#endif
static float RE;
static float REC;
static float Rg2;
static float Rgu2;
static float RSt2;
static float ff0_t2;
static float RSr2[3];
static float CdT2;
static float CfT2;
static float CST2;
static float CSTF2;
static float CdR12;
static float CfR12;
static float CfR2[3];
static float CdR2[3];
static float RT2[5];
static float CT2[5];
static float CTH2;
static float CTM2;
static float VIM2;
static float VIMC2;
static float RE2;
static float REC2;
static float maxx;
static float maxy;
static float maxz;

static void print_time(int seconds);
#if defined(CREATE_EXE_TIME)
static void print_time_2IO(int seconds);
#endif

static void intestazione();
static void presentazione();
#if defined(USE_MAIN)
static void vedimatrici();
static void stampamatrice(float *n);
static void stampamatrice1(float *n);
static void stampamatrice1l(long double *n);
#endif
static void stampa_ris();
static void mem_ris(int);
static void val_med();
#if defined(OLD_WAY_CHECK)
 static void inp_inter();
#endif
#if defined(OLD_WAY)
 static void out_inter();
#endif
static void mem_mol();
static void autovalori();
static void riempimatrice();
static void choldc(int N);
static void cholsl(int N);
static void inizializza_b1();
static void inverti(int N);
static void DDtcalc();
static void sigmaoRcalc();
static void Gets_date(char *day, char *month, int *year, int *numday, char *hour);
#if defined(NOT_USED)
static void sigmaocalc();
static void sigmatcalc1();
#endif
static void sigmaocalc1();
static void sigmatcalc2();
static void sigmarRcalc1();
static int overlap();
static void vxT(float v[3], float T[9]);
static void vxTl(float v[3], long double T[9]);
static void Txv(float T[9], float v[3]);
static void Mxv(float M[9], float v[3]);
static void calcqij();
static void calcR();
static void calcD();
static void secondo(long double b, long double c);
#if defined( DEBUG_EV )
static void terzo(long double b, long double c, long double d);
#endif
static void visco();
#if defined(TSUDA_DOUBLESUM)
static void tsuda();
static void tsuda1();
static void doublesum();
#endif
static void init_da_a();
static void inv(float r[9]);
static void inv6x6(float a[6][6]);
static void initarray(int);
static void diffcalc();
static void relax_rigid_calc();
static void ragir();
static void maxest();

// #define DEBUG_WW
#if defined(DEBUG_WW)
static int log_cnt = 0;
static double cks;
static FILE *logfx;
static void dww(char *s) {
   fprintf(logfx, "dww %s: %.12e\n", s, cks);
}
#endif

// #define NMAX 1690

static struct dati1_supc *dt = 0;   // [2 * NMAX];
static struct dati1_supc *dtn = 0;   // [NMAX];
static float *rRi = 0;      // [3 * NMAX];
static float *rRis = 0;      // [3 * NMAX];
static float *b1 = 0;      // [3 * NMAX];
static float *p = 0;      // [3 * NMAX];
static float *gp = 0;      // [NMAX][9];
static float *q = 0;      // [NMAX][NMAX][9];
static float *a = 0;      // [3 * NMAX][3 * NMAX];

static int nmax;
// static QApplication *qApp;
static QProgressBar *progress;
static QTextEdit *editor;
static int ppos;
static int mppos;
static double overlap_tolerance;
static float tot_tot_beads;
static float tot_tot_beads2;
static float tot_used_beads;
static float tot_used_beads2;
static vector <PDB_model> *model_vector;
static vector <int> model_idx;  // maps seq model # to bead_models offset
static vector < vector <PDB_atom> > *bead_models;
static vector < vector <int> > active_idx;  // maps into bead_model
static vector <int> bead_count;  // counts # of active beads
static int active_model;
static US_Hydrodyn *us_hydrodyn;
static vector < float > total_asa;
static vector < float > used_asa;
static vector < float > total_s_a;
static vector < float > used_s_a;
static vector < float > total_vol;
static vector < float > used_vol;
static vector < float > asa_vol;

static bool      smi_mm;
static QString   smi_mm_name;
static save_info smi_mm_save_params;

static US_Timer           supc_timers;

static bool      hydro_use_avg_for_volume;

static void
supc_free_alloced()
{
   supc_timers.clear_timers();
   if (dt)
   {
      free(dt);
      dt = 0;
   }
   if (dtn)
   {
      free(dtn);
      dtn = 0;
   }
   if (rRi)
   {
      free(rRi);
      rRi = 0;
   }
   if (rRis)
   {
      free(rRis);
      rRi = 0;
   }
   if (b1)
   {
      free(b1);
      b1 = 0;
   }
   if (p)
   {
      free(p);
      p = 0;
   }
   if (gp)
   {
      free(gp);
      gp = 0;
   }
   if (a)
   {
      free(a);
      a = 0;
   }
   if (q)
   {
      free(q);
      q = 0;
   }
   us_hydrodyn->lbl_core_progress->setText("");
}

static void
supc_free_alloced_2()
{
   if (a)
   {
      free(a);
      a = 0;
   }
   if (q)
   {
      free(q);
      q = 0;
   }
}

static int
supc_alloc()
{
   dt = (struct dati1_supc *) malloc(2 * nmax * sizeof(struct dati1_supc));
   if (!dt)
   {
      supc_free_alloced();
      fprintf(stderr, "memory allocation error\n");
      return US_HYDRODYN_SUPC_ERR_MEMORY_ALLOC;
   }
   memset(dt, 0, 2 * nmax * sizeof(struct dati1_supc));

   dtn = (struct dati1_supc *) malloc(nmax * sizeof(struct dati1_supc));
   if (!dtn)
   {
      supc_free_alloced();
      fprintf(stderr, "memory allocation error\n");
      return US_HYDRODYN_SUPC_ERR_MEMORY_ALLOC;
   }
   memset(dtn, 0, nmax * sizeof(struct dati1_supc));

   rRi = (float *) malloc(nmax * 3 * sizeof(float));
   if (!rRi)
   {
      supc_free_alloced();
      fprintf(stderr, "memory allocation error\n");
      return US_HYDRODYN_SUPC_ERR_MEMORY_ALLOC;
   }
   memset(rRi, 0, nmax * 3 * sizeof(float));

#if defined(USE_THREADS)
   rRis = (float *) malloc(nmax * 3 * sizeof(float));
   if (!rRis)
   {
      supc_free_alloced();
      fprintf(stderr, "memory allocation error\n");
      return US_HYDRODYN_SUPC_ERR_MEMORY_ALLOC;
   }
   memset(rRis, 0, nmax * 3 * sizeof(float));
#endif

   b1 = (float *) malloc(nmax * 3 * sizeof(float));
   if (!b1)
   {
      supc_free_alloced();
      fprintf(stderr, "memory allocation error\n");
      return US_HYDRODYN_SUPC_ERR_MEMORY_ALLOC;
   }
   memset(b1, 0, nmax * 3 * sizeof(float));

   p = (float *) malloc(nmax * 3 * sizeof(float));
   if (!p)
   {
      supc_free_alloced();
      fprintf(stderr, "memory allocation error\n");
      return US_HYDRODYN_SUPC_ERR_MEMORY_ALLOC;
   }
   memset(p, 0, nmax * 3 * sizeof(float));

   gp = (float *) malloc(nmax * 9 * sizeof(float));
   if (!gp)
   {
      supc_free_alloced();
      fprintf(stderr, "memory allocation error\n");
      return US_HYDRODYN_SUPC_ERR_MEMORY_ALLOC;
   }
   memset(gp, 0, nmax * 9 * sizeof(float));

   return 0;
}

static int
supc_alloc_2()
{
   a = (float *) malloc((double) nat * 3 * nat * 3 * sizeof(float));
   if (!a)
   {
      supc_free_alloced();
      fprintf(stderr, "memory allocation error\n");
      return US_HYDRODYN_SUPC_ERR_MEMORY_ALLOC;
   }
   memset(a, 0, (double) nat * 3 * nat * 3 * sizeof(float));

   q = (float *) malloc((double)nat * nat * 9 * sizeof(float));
   if (!q)
   {
      supc_free_alloced();
      fprintf(stderr, "memory allocation error\n");
      return US_HYDRODYN_SUPC_ERR_MEMORY_ALLOC;
   }
   memset(q, 0, (double) nat * nat * 9 * sizeof(float));
   return 0;
}

static void
print_time(int seconds)
{
   div_t min_sec;

   min_sec = div(seconds, 60);
   printf("Time used for computing: %d minutes and %d seconds\n", min_sec.quot, min_sec.rem);
}

#if defined(CREATE_EXE_TIME)
static void
print_time_2IO(int seconds)
{
   div_t min_sec;

   min_sec = div(seconds, 60);
   fprintf(exe_time, "Time used for computing: %d minutes and %d seconds\n", min_sec.quot, min_sec.rem);
}
#endif

/*********************************************************
 *                      *
 *       Procedure:                                   *
 *                                                        *
 *           void Gets_date(char *day, char *month,       *
 *                             char *year, int *numday,   *
 *               char *hour)                *
 *                                                        *
 **********************************************************/
static void
Gets_date(char *day, char *month, int *year, int *numday, char *hour)
{

   char tempo[100];
   char anno[5];
   time_t timer;
   struct tm *tblock;

   timer = time(NULL);
   tblock = localtime(&timer);
   strcpy(tempo, asctime(tblock));
   if ( 5 == sscanf(tempo, "%s %s %d %s %s\n", day, month, numday, hour, anno) ) {};
   /* annov=strtol(anno,NULL,0); */
   anno[0] = anno[2];
   anno[1] = anno[3];
   anno[2] = '\0';
   *year = atoi(anno);
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int
us_hydrodyn_supc_main(hydro_results *hydro_results, 
                      hydro_options *hydro, 
                      double use_overlap_tolerance,
                      vector < vector <PDB_atom> > *use_bead_models, 
                      vector <int> *somo_processed,
                      vector <PDB_model> *use_model_vector,
                      QListWidget *lb_model,
                      const char *filename,
                      const char *res_filename,
                      vector < QString > model_names,
                      QProgressBar *use_progress,
                      QTextEdit *use_editor,
                      US_Hydrodyn *use_us_hydrodyn)
{
   supc_timers.clear_timers();
   dt = 0;
   dtn = 0;
   rRi = 0;
   rRis = 0;
   b1 = 0;   
   p = 0;   
   gp = 0;   
   q = 0;   
   a = 0;   
   tot_partvol = 0.0;

   hydro_use_avg_for_volume = !hydro->volume_correction && hydro->use_avg_for_volume;
   // qDebug() << "hydro_use_avg_for_volume is " << ( hydro_use_avg_for_volume ? "true" : "false" );

   cout << "supc:filename    : " << filename << endl;
   cout << "supc:res_filename: " << res_filename << endl;
#if !defined(CREATE_TOT_MOL)
   molecola_v.clear( );
   nat_v.clear( );
   prima_v.clear( );
   ultima_v.clear( );
   raggio_v.clear( );
#endif
   us_hydrodyn = use_us_hydrodyn;

   smi_mm_save_params.data_vector.clear();
   
   ETAo = us_hydrodyn->use_solvent_visc() / 100e0; // hydro->solvent_viscosity / 100;
   DENS = us_hydrodyn->use_solvent_dens();         // hydro->solvent_density;
   TE = K0 + hydro->temperature;
   hydro_results->method = "SMI";
   hydro_results->solvent_name = hydro->solvent_name;
   hydro_results->solvent_acronym = hydro->solvent_acronym;
   hydro_results->solvent_viscosity = us_hydrodyn->use_solvent_visc(); // hydro->solvent_viscosity;
   hydro_results->solvent_density = us_hydrodyn->use_solvent_dens();   // hydro->solvent_density;
   hydro_results->temperature = hydro->temperature;
   tag1 = QString("%1@%2C").arg(hydro->solvent_acronym).arg(hydro->temperature);
   tag2 = QString("%1C,%2").arg(hydro->temperature).arg(hydro->solvent_acronym);

   //  vector <PDB_atom> *bead_model;
   tot_tot_beads = 0;
   tot_tot_beads2 = 0;
   tot_used_beads = 0;
   tot_used_beads2 = 0;

   progress = use_progress;
   editor = use_editor;
   overlap_tolerance = use_overlap_tolerance;
   model_vector = use_model_vector;
   bead_models = use_bead_models;
#if defined(DEBUG_WW)
   cks = 0e0;
   {
      char s[30];
      sprintf(s, "supc_log-%d", log_cnt++);
      logfx = us_fopen(s, "w");
   }
#endif
   model_idx.clear( );
   bead_count.clear( );
   active_idx.clear( );

   nmax = 0;
   int models_to_proc = 0;
   QString use_filename = filename;
   total_asa  .clear();
   used_asa   .clear();
   total_s_a  .clear();
   used_s_a   .clear();
   total_vol  .clear();
   used_vol   .clear();
   asa_vol    .clear();
   
   float lconv = pow(10.0,9 + hydro->unit);
   float lconv2 = lconv * lconv;
   float lconv3 = lconv2 * lconv;
   printf("lconv %e lconv2 %e lconv3 %e\n",
          lconv,
          lconv2,
          lconv3);

   double this_pr = us_hydrodyn->asa.probe_recheck_radius / (10 * lconv);

   for (int current_model = 0; current_model < (int)lb_model->count(); current_model++) {
      if (lb_model->item(current_model)->isSelected()) {
         if ((*somo_processed)[current_model]) {
            model_idx.push_back(current_model);
            vector < int > tmp_active_idx;
            int tmp_count                  = 0;
            float this_total_asa           = 0.0f;
            float this_used_asa            = 0.0f;
            float this_total_s_a           = 0.0f;
            float this_used_s_a            = 0.0f;
            float this_total_vol           = 0.0f;
            float this_used_vol            = 0.0f;
            float this_asa_vol             = 0.0f;
            double this_total_vol_from_asa = 0;
            double this_used_vol_from_asa  = 0;
            // float this_exposed_vol   = 0.0f;
            // int this_exposed_beads = 0;

            for(int i = 0; i < (int)(*bead_models)[current_model].size(); i++) {
               if((*bead_models)[current_model][i].active) {
                  tmp_active_idx.push_back(i);
                  tmp_count++;
                  double this_asa = (*bead_models)[current_model][i].bead_recheck_asa;; // should already be in units // * lconv2; // * 1.25;
                  this_total_asa += this_asa;
                  float bead_total_s_a = 4.0f * M_PI * lconv2 *
                     (*bead_models)[current_model][i].bead_computed_radius *
                     (*bead_models)[current_model][i].bead_computed_radius;
                  this_total_s_a += bead_total_s_a;
                  float bead_total_vol = ( 4.0f / 3.0f ) * M_PI * lconv3 *
                     (*bead_models)[current_model][i].bead_computed_radius *
                     (*bead_models)[current_model][i].bead_computed_radius *
                     (*bead_models)[current_model][i].bead_computed_radius;
                  this_total_vol += bead_total_vol;
                  double this_r = sqrt( this_asa / ( 4 * M_PI ) );
                  double this_sa = 4.0f * M_PI *
                     ( (*bead_models)[current_model][i].bead_computed_radius + this_pr ) *
                     ( (*bead_models)[current_model][i].bead_computed_radius + this_pr );
                  double this_frac = this_asa / this_sa;
                  if ( this_frac > 1 ) {
                     this_frac = 1;
                  }
                  QTextStream(stdout) << QString().sprintf(
                                                           "~ bead radius %f asa %f sa %f pct asa %f asa_r %f pr %f \n"
                                                           ,(*bead_models)[current_model][i].bead_computed_radius
                                                           ,this_asa
                                                           ,this_sa
                                                           ,100.0 * ( this_asa / this_sa )
                                                           ,this_r
                                                           ,this_pr
                                                           );
                  this_total_vol_from_asa += this_frac * bead_total_vol;

                  if ( us_hydrodyn->get_color(&(*bead_models)[current_model][i]) != 6 ) {
                     this_used_asa += (*bead_models)[current_model][i].bead_recheck_asa * lconv2;
                     this_used_s_a += bead_total_s_a;
                     this_used_vol += bead_total_vol;
                     this_used_vol_from_asa += this_frac * bead_total_vol;
                  }
               }
            }
            if (nmax < (int) (*bead_models)[current_model].size()) {
               nmax = (int) (*bead_models)[current_model].size();
            }

            this_asa_vol = this_used_vol_from_asa;
            // // cap logically (asa_vol not greater than used vol)
            // if ( this_asa_vol > this_used_vol ) {
            //    this_asa_vol = this_used_vol;
            // }
            this_asa_vol /= lconv3;

            bead_count     .push_back( tmp_count );
            active_idx     .push_back( tmp_active_idx );
            total_asa      .push_back( this_total_asa );
            used_asa       .push_back( this_used_asa );
            total_s_a      .push_back( this_total_s_a );
            used_s_a       .push_back( this_used_s_a );
            total_vol      .push_back( this_total_vol );
            used_vol       .push_back( this_used_vol );
            asa_vol        .push_back( this_asa_vol );
            models_to_proc++;

            QTextStream( stdout ) <<
               QString("").sprintf(
                                   "model                  %d\n"

                                   " total beads           %d\n"
                                   " total volume          %g\n"
                                   " avg volume from total %g\n"

                                   " used beads            %d\n"
                                   " used volume           %g\n"
                                   " avg volume            %g\n"
                                   "\n"

                                   ,current_model

                                   ,(int) (*bead_models)[current_model].size()
                                   ,this_total_vol
                                   ,this_total_vol / (double) (*bead_models)[current_model].size()
                                   
                                   ,tmp_count
                                   ,this_used_vol
                                   ,this_used_vol / (double) tmp_count
                                   )
               ;


            QTextStream( stdout ) << QString().sprintf(
                                          "model %d\n"
                                          "total_asa          %f\n"
                                          "total_s_a          %f\n"
                                          "total_vol_from_asa %f\n"
                                          "total_vol          %f\n"
                                          "used_asa           %f\n"
                                          "used_s_a           %f\n"
                                          "used_vol_from_asa  %f\n"
                                          "used_vol           %f\n"
                                          "asa_vol            %f\n"
                                          ,current_model
                                          ,this_total_asa
                                          ,this_total_s_a
                                          ,this_total_vol_from_asa
                                          ,this_total_vol
                                          ,this_used_asa
                                          ,this_used_s_a
                                          ,this_used_vol_from_asa
                                          ,this_used_vol
                                          ,this_asa_vol
                                          );
                                          
         }
      }
   }


   if(!models_to_proc) {
      editor->append("\nNo selected models ready to compute hydrodynamics.\n");
      return US_HYDRODYN_SUPC_NO_SEL_MODELS;
   }
    
   smi_mm      = models_to_proc > 1;
   smi_mm_name = filename;
   smi_mm_name = smi_mm_name.replace( QRegExp( ".beams$" ), "" ).replace( "_%1", "" );
   if ( smi_mm ) {
      smi_progress = us_hydrodyn->mprogress;
      smi_progress->setValue( 0 );
      smi_progress->setFormat( "Model %p%" );
      smi_progress->show();
   } else {
      smi_progress = us_hydrodyn->progress;
   }
      
   supc_results = hydro_results;
   supc_results->total_beads = nmax;
   supc_results->solvent_density   = us_hydrodyn->use_solvent_dens();
   supc_results->solvent_viscosity = us_hydrodyn->use_solvent_visc();
   supc_results->pH                = us_hydrodyn->hydro.pH;
    
   supc_results->name = use_filename;
   supc_results->name.replace(QRegExp("\\.beams$"),"");
   supc_results->name.replace(QRegExp("_%1"),"");
   if ( lb_model->count() > 1 && model_idx.size() ) 
   {
      supc_results->name += 
         QString(" model%1: ")
         .arg((model_idx.size() > 1) ? "s" : "");
      int inc;
      for ( unsigned int i = 0; i < model_idx.size(); i++ ) {
         if ( i ) 
         {
            supc_results->name += ",";
         }
         for ( inc = 1;
               (i + inc < model_idx.size()) && 
                  (model_idx[i + inc] == model_idx[i] + inc);
               inc++ ) {};
         inc--;
         supc_results->name += QString("%1").arg(model_idx[i]+1);
         if ( inc > 1 ) {
            supc_results->name += QString("-%1")
               .arg(model_idx[i+inc]+1);
            i += inc;
        } 
      }
   }
   printf("model result name <%s>\n", supc_results->name.toLatin1().data());

   if (int retval = supc_alloc())
   {
      return retval;
   }
#if defined(DEBUG_WW)
   cks += (double)nmax;
   dww("start");
#endif
   int i;
   int k; 
   int flag_mem;
   int bc;
#if defined(USE_MAIN)
   char r1;
   char command[200];
#endif
   float temp, x, y, z;
   time_t primo, secondo;
   // void Gets_date(char *day, char *month, int *year, int *numday, char *hour);
   char day[4];
   char month[3];
   char hour[9];
   int numday, year;

   printf("\n\n ** REMOVING RESERVED FILES, IF PRESENT ** \n\n");

#if defined(CREATE_TOT_MOL)
   printf("Removing tot_mol\n");
   QFile::remove("tot_mol");
#endif
#if defined(OLD_WAY)
   printf("Removing ifraxon\n");
   QFile::remove("ifraxon");
   printf("Removing ofraxon\n");
   QFile::remove("ofraxon");
   printf("Removing ifraxon1\n");
   QFile::remove("ifraxon1");
   printf("Removing ofraxon1\n");
   QFile::remove("ofraxon1");
#endif

   Gets_date(day, month, &year, &numday, hour);

   num = 0;
   colorzero = 0;
   colorsix = 0;
   colorsixf = 0;
   metodo = 2;
   flag_mem = 0;
   flag_norm = 0;
   cd = 3;
   cc = 0;
   scelta = 1;
   volcor = 3;
   mascor = 3;

   num = models_to_proc;
    

#if defined(USE_MAIN)
   while ((num < 1) || (num > 100))
   {
      intestazione();
      printf("\n\n** Compiled for %d beads **\n\n", nmax);
      printf("\n\n\n** Insert number of models to analyze (max=100) :___ ");
      if ( 1 == scanf("%d", &num) ){};
      getchar();
   }

   if (num != 1)
   {
      printf("\n - Sequential files?\n");
      printf(" 1) Yes\n 2) No\n\n");
      printf(" Your choice ? (1/2) :__ ");
      if ( 1 == scanf("%d", &cdmolix) ){};
      if (cdmolix == 1)
      {
         printf("\n-Enter filename prefix  ");
         if ( 1 == scanf("%s", fil001) ){};
         getchar();
         printf("\n-Enter first file number   ");
         if ( 1 == scanf("%d", &num001) ){};
         numor = 1;
      }
      r1 = 'y';
      flag_mem = 1;

      /* Initialization for average values */

      Rg = Rgu = RSt = CfT = CST = CSTF = CdT = CfR1 = CdR1 = RE = REC = VIM = VIMC = 
         CTH = CTM = 0.0;

      ff0_t = 
         ff0_t2 = 0.0;

#if defined(TSUDA_DOUBLESUM)
      REDS = RETM = RETV = VIMDS = VIMTM = VIMTV = 0.0;
#endif

      Rg2 = Rgu2 = RSt2 = CfT2 = CST2 = CSTF2 = CdT2 = CfR12 = CdR12 = RE2 = REC2 = VIM2 = VIMC2 =
         CTH2 = CTM2 = 0.0;

#if defined(TSUDA_DOUBLESUM)
      REDS2 = RETM2 = RETV2 = VIMDS2 = VIMTM2 = VIMTV2 = 0.0;
#endif

      for (k = 0; k < 3; k++)
      {
         RSr[k] = RSr2[k] = 0.0;
         CfR[k] = CfR2[k] = 0.0;
         CdR[k] = CdR2[k] = 0.0;
      }
      for (k = 0; k < 5; k++)
      {
         RT[k] = RT2[k] = 0.0;
         CT[k] = CT2[k] = 0.0;
      }
   }

   else

   {
      r1 = 'a';
      while ((r1 != 'n') && (r1 != 'N') && (r1 != 'y') && (r1 != 'S'))
      {
         intestazione();
         printf("\n\n\n** Number of models to analyze:___ %d", num);
         printf("\n** Do you want the results printed to a file ? (y/n) :___ ");
         if ( 1 == scanf("%s", &r1) ){};
         getchar();
      }
   }

   if ((r1 == 'y') || (r1 == 'Y'))
   {
   a50:
      flag_mem = 1;
      printf("\n** Insert the name of results' file :___ ");
      if ( 1 == scanf("%s", risultati) ){};
      getchar();
   }

   new_mol = us_fopen(risultati, "r");
    

   if (new_mol != NULL)
   {
      printf("\n");
      printf("*** CAUTION : File already exists ! ***\n\n");
      printf("   Do you want:\n\n");
      printf(" 1) Append to existing file\n");
      printf(" 2) Overwrite existing file\n");
      printf(" 3) Create new file\n\n");
      printf("** Select (1/2/3) :___ ");
      if ( 1 == scanf("%d", &fe) ){};
      getchar();
      fclose(new_mol);
      if (fe == 2)
      {
         strcpy(command, RM_COMMAND);
         strcat(command, risultati);
         if (system(command) != 0)
            printf("Error writing file");
      }
      if (fe == 3)
      {
         goto a50;
         /*      printf("** Insert the name of results file :___ ");
                 if ( 1 == scanf("%s",risultati) ){};
                 getchar();  */
      }
   }

   printf("\n\t\tWARNING !\n");
   printf("If the coordinates and the radius of the beads are expressed \n");
   printf("in units other than nanometers, insert the correct conversion\n");
   printf("factor to turn them into nanometers [else insert 1].\n");
   printf("The following table shows some examples of conversions:\n\n");

   printf("- - )   micrometers => nanometers [ Conversion factor = 1000 ]\n");
   printf("- - )   nanometers  => nanometers [ Conversion factor = 1    ]\n");
   printf("- - )   angstroms   => nanometers [ Conversion factor = 0.1  ]\n\n");
   printf("\n** Insert the conversion factor :___ ");

   if ( 1 == scanf("%f", &fconv) ){};
   fconv1 = 1.0 / fconv;

   while ((cd != 1) && (cd != 2))
   {
      intestazione();
      printf("\n- Reference System :\n\n");
      printf(" 1) Cartesian Origin \n 2) Diffusion Center\n\n");
      printf(" Select (1/2) :___ ");
      if ( 1 == scanf("%d", &cd) ){};
      getchar();
   }

   while ((cc != 1) && (cc != 2))
   {
      intestazione();
      printf("\n- Boundary Conditions :\n\n");
      printf(" 1) STICK boundary condition (6*M_PI*ETAo)\n");
      printf(" 2) SLIP boundary condition (4*M_PI*ETAo)\n\n");
      printf(" Select (1/2) :___ ");
      if ( 1 == scanf("%d", &cc) ){};
      getchar();
   }

   while ((volcor != 1) && (volcor != 2))
   {
      intestazione();
      printf("\n- Volume correction:\n\n");
      printf(" 1) Automatic (Total volume of the beads)\n");
      printf(" 2) Manual    (From keyboard)\n\n");
      printf(" Select (1/2) :___ ");
      if ( 1 == scanf("%d", &volcor) ){};
      getchar();
   }

   if (volcor == 2)
   {
      printf("\n\n\n- Insert value of the volume : ");
      if ( 1 == scanf("%f", &volcor1) ){};
   }

   while ((mascor != 1) && (mascor != 2))
   {
      intestazione();
      printf("\n- Mass correction:\n\n");
      printf(" 1) Automatic (Masses from file) \n");
      printf(" 2) Manual    (Total Mass from Keyboard)\n\n");
      printf(" Select (1/2) :___ ");
      if ( 1 == scanf("%d", &mascor) ){};
      getchar();
   }

   if (mascor == 2)
   {
      printf("\n\n\n- Insert value of mass : ");
      if ( 1 == scanf("%f", &mascor1) ){};
   }

   while ((sfecalc != 1) && (sfecalc != 2))
   {
      intestazione();
      printf("\n- Beads to be included in the computation:\n\n");
      printf(" 1) ALL beads\n");
      printf(" 2) Color Code Selection (Beads Color Coded \"6\" will be excluded)\n\n");
      printf(" Select (1/2) :___ ");
      if ( 1 == scanf("%d", &sfecalc) ){};
      getchar();
      if ((sfecalc == 2) && (volcor == 1))
      {
         printf("\n YOU HAVE SELECTED TO EXCLUDE BEADS COLOR CODED \"6\"\n\n");
         printf("\n HOWEVER, THEIR VOLUME MAY BE IMPORTANT FOR THE\n");
         printf("\n ROTATIONAL DIFFUSION AND INTRINSIC VISCOSITY CORRECTIONS\n");
         printf("\n You may choose to include their volume in:\n\n");

         printf(" 0) Neither correction\n");
         printf(" 1) The rotational diffusion correction only\n");
         printf(" 2) The intrinsic viscosity correction only\n");
         printf(" 3) Both corrections\n\n");
         printf(" Select (0/1/2/3) :___ ");
         if ( 1 == scanf("%d", &colorsixf) ){};
         getchar();

      }

   }

   intestazione();
   r1 = 'y';
#else
   cdmolix = 2; // non sequential files
   flag_mem = 1;

   /* Initialization for average values */

   Rg = Rgu = RSt = CfT = CST = CSTF = CdT = CfR1 = CdR1 = RE = REC = VIM = VIMC = 
      CTH = CTM = 0.0;

   ff0_t =
      ff0_t2 = 0.0;

#if defined(TSUDA_DOUBLESUM)
   REDS = RETM = RETV = VIMDS = VIMTM = VIMTV = 0.0;
#endif

   Rg2 = Rgu2 = RSt2 = CfT2 = CST2 = CSTF2 = CdT2 = CfR12 = CdR12 = RE2 = REC2 = VIM2 = VIMC2 =
      CTH2 = CTM2 = 0.0;

#if defined(TSUDA_DOUBLESUM)
   REDS2 = RETM2 = RETV2 = VIMDS2 = VIMTM2 = VIMTV2 = 0.0;
#endif
    
   for (k = 0; k < 3; k++)
   {
      RSr[k] = RSr2[k] = 0.0;
      CfR[k] = CfR2[k] = 0.0;
      CdR[k] = CdR2[k] = 0.0;
   }
   for (k = 0; k < 5; k++)
   {
      RT[k] = RT2[k] = 0.0;
      CT[k] = CT2[k] = 0.0;
   }

   intestazione();
   num = models_to_proc;
   strncpy(risultati, res_filename, SMAX);
   printf("risultati (as res_filename): %s\n", risultati);
   risultati[SMAX-1] = 0;
   if(strlen(risultati) > 6) {
      risultati[strlen(risultati) - 5] = 0; // remove .beams
   }
   strncat(risultati, "hydro_res", SMAX - strlen(risultati));
   risultati[SMAX-1] = 0;
   printf("risultati: %s\n", risultati);
   // QFile::remove(risultati);
   flag_mem = 1;
    
   strncpy(molecola, use_filename.contains( "%1" ) ? 
           QString( use_filename ).arg( model_names[ 0 ] ).toLatin1().data() :
           use_filename.toLatin1().data() , SMAX); // first model
   molecola[SMAX-1] = 0;
   strncpy(molecola_nb, molecola, SMAX);
   molecola_nb[SMAX-1] = 0;
   if(strlen(molecola_nb) > 7) {
      molecola_nb[strlen(molecola_nb) - 6] = 0; // remove .beams
   }
   
#if defined(DEBUG_FILES)
   if(!(mol = us_fopen(molecola, "r"))) {
      supc_free_alloced();
      return US_HYDRODYN_SUPC_FILE_NOT_FOUND;
   }
   fclose(mol);
#endif
   fconv = pow(10.0,hydro->unit + 9);
   printf("fconv = %f\n", fconv);
   fconv1 = 1.0 / fconv;
   cd = hydro->reference_system ? 1 : 2;
   cc = hydro->boundary_cond ? 2 : 1;
   volcor = hydro->volume_correction ? 2 : 1;
   if (volcor == 2) 
   {
      volcor1 = (float)hydro->volume;
      if ( volcor1 < 1e-10 ) {
         volcor1 = 1e-10;
      }
   }
   mascor = hydro->mass_correction ? 2 : 1;
   if (mascor == 2) 
   {
      mascor1 = (float)hydro->mass;
   }
   sfecalc = hydro->bead_inclusion ? 1 : 2;
   if ((sfecalc == 2) && (volcor == 1))
   {
      colorsixf = 
         (hydro->rotational ? 1 : 0) + 
         (hydro->viscosity ? 2 : 0);
   }
   //    init_da_a();
#endif

   printf("- Computational Method : SUPERMATRIX INVERSION\n\n");

   if (flag_mem == 1 &&
       !(us_hydrodyn->batch_widget &&
         us_hydrodyn->batch_window->save_batch_active))
   {
      qs_risultati = risultati;
      if ( !us_hydrodyn->overwrite_hydro ) {
         qs_risultati = us_hydrodyn->fileNameCheck( qs_risultati, 0, us_hydrodyn );
      }

      ris = us_fopen(qs_risultati, "wb");
      fprintf(ris, "\n%s", "BEAMS -           IST. CBA                - COEFF/SUPCW v. 5.0\n");
      fprintf(ris, "\n%s", "**************************************************************");
      fprintf(ris, "\n%s\n", "***** Computational Method : Supermatrix Inversion ***********");
      fprintf(ris, "%s\n", "**************************************************************");
      fprintf(ris, "* Date: %s %d %s %d %s                                *\n", day, numday, month, year, hour);

      fprintf(ris, "%s\n", "**************************************************************");

      fclose(ris);
   }

   if ( smi_mm ) {
      us_hydrodyn->progress->setMaximum( num  );
   }

   for (k = 0; k < num; k++)
   {
      //      current_model = model_idx[k];
      if ( smi_mm ) {
         us_hydrodyn->progress->setValue( k );
      }

      /* Check for file existence and selects whole or part of the models for sequential files only   */
      if (cdmolix == 1) // never true in our case, cdmolix == 2
      {
         {
            char newf[SMAX - 12];
            strncpy( newf, fil001, SMAX-13 );
            newf[SMAX-13] = 0;
            snprintf(molecola, SMAX, "%s%d", newf, num001);
         }
         num001 = num001 + 1;
         mol = us_fopen(molecola, "r");
         while (mol == NULL)
         {
            printf("\n");
            printf("The Model(s) do not exist!!!\n");
            printf("Insert the models' correct prefix :___");
            if ( 1 == scanf("%s", molecola) ){};
            mol = us_fopen(molecola, "r");
         }
         if ( 1 == fscanf(mol, "%d", &nat) ) {};
         fclose(mol);
         if (k == 0)
         {
            printf("\n%s%d%s", "** TOTAL Number of BEADS in the MODELS :___", nat, " **\n\n");
            prima = (-1);
            while ((prima < 0) || (prima > (nat - 1)))
            {
               printf("%s", "** Insert FIRST BEAD # to be included:___");
               if ( 1 == scanf("%d", &prima) ) {};
               getchar();
               printf("\n");
            }
            ultima = nat + 1;
            while ((ultima < prima) || (ultima > nat))
            {
               printf("%s", "** Insert LAST BEAD # to be included:___");
               if ( 1 == scanf("%d", &ultima) ) {};
               getchar();
               printf("\n");
            }
         }

#if defined(CREATE_TOT_MOL)
         tot_mol = us_fopen("tot_mol", "wb");
         fprintf(tot_mol, "%s\n", molecola);
         fprintf(tot_mol, "%d\t%d\t%d\n", nat, prima, ultima);
         fclose(tot_mol);
#else
         molecola_v.clear( );
         nat_v.clear( );
         prima_v.clear( );
         ultima_v.clear( );
         raggio_v.clear( );
         molecola_v.push_back(QString("%1").arg(molecola));
         nat_v.push_back(nat);
         prima_v.push_back(prima);
         ultima_v.push_back(ultima);
#endif
      }

      else
      {  // this code is always executed

#if defined(USE_MAIN)
         if (num != 1)
         {
            printf("\n%s%d%s", "** Insert file name of model #", k + 1, " to be analyzed :___ ");
         }
         else
         {
            printf("\n** Insert file name of the model to be analyzed :___ ");
         }
         if ( 1 == scanf("%s", molecola) ) {};
         getchar();

         init_da_a();
#endif
         printf( "opening file: %s\n", 
                 use_filename.contains( "%1" ) ? 
                 QString( use_filename ).arg( model_names[ k ] ).toLatin1().data() :
                 use_filename.toLatin1().data() );
         strncpy( molecola, 
                  use_filename.contains( "%1" ) ? 
                  QString( use_filename ).arg( model_names[ k ] ).toLatin1().data() :
                  use_filename.toLatin1().data(), SMAX); // first model
         molecola[SMAX-1] = 0;
         strncpy(molecola_nb, molecola, SMAX);
         molecola_nb[SMAX-1] = 0;
         if(strlen(molecola_nb) > 7) {
            molecola_nb[strlen(molecola_nb) - 6] = 0; // remove .beams
         }
         active_model = k;

         init_da_a();
      }
   }

#if defined(CREATE_TOT_MOL)
   tot_mol = us_fopen("tot_mol", "r");
#endif

   for (k = 0; k < num; k++)
   {
      supc_timers.init_timer( "compute smi" );
      supc_timers.start_timer( "compute smi" );

      active_model = k;

      QColor save_color = editor->textColor();
      editor->setTextColor("dark blue");
      editor->append(QString("\nProcessing model %1 bead count %2 vbar %3%4%5%6\n")
                     .arg(k+1)
                     .arg(bead_count[k])
                     .arg(us_hydrodyn->misc.compute_vbar ?
                          (int)(((*model_vector)[model_idx[active_model]].vbar * 1000) + 0.5) / 1000.0 :
                          us_hydrodyn->misc.vbar)
                     .arg(us_hydrodyn->misc.compute_vbar ? "" : " (User Entered)")
                     .arg(hydro->mass_correction ? 
                          QString(" MW %1 (User Entered)").arg(hydro->mass) : "")
                     .arg(hydro->volume_correction ? 
                          QString(" Volume %1 (User Entered)").arg(hydro->volume) : "")
                     );
      editor->setTextColor(save_color);

      supc_free_alloced_2();

      initarray(k);
      editor->append(QString("Using %1 beads for the matrix\n").arg(nat));
      qApp->processEvents();
      if (us_hydrodyn->stopFlag)
      {
         supc_free_alloced();
         return -1;
      }

      printf("nat = %d\n", nat);
      if (int retval = supc_alloc_2())
      {
         supc_free_alloced();
         return retval;
      }
   
      ppos = 1;
      mppos = (1 + 3 + 3) * nat + 17;
      smi_progress->setMaximum(mppos);
      smi_progress->setValue(ppos++); // 1
      qApp->processEvents();
      if (us_hydrodyn->stopFlag)
      {
         supc_free_alloced();
         return -1;
      }

#if defined(CREATE_TOT_MOL)
      if ( 1 == fscanf(tot_mol, "%f", &raflag) ) {};
#else
      raflag = raggio_v[k];
#endif

      presentazione();

      kkk = k + 1;
#if defined(OLD_WAY_CHECK)
      inp_inter();
#endif
      {
         // qDebug() << "supc::main() b4pat nmax nat " << nmax << " " << nat;
         
         struct dati1_pat *out_dt = 0;

         out_dt = (struct dati1_pat *) malloc(nmax * sizeof(struct dati1_pat));
         if (!out_dt)
         {
            supc_free_alloced();
            fprintf(stderr, "memory allocation error\n");
            return US_HYDRODYN_SUPC_ERR_MEMORY_ALLOC;
         }
         memset(out_dt, 0, nmax * sizeof(struct dati1_pat));

         int out_nat;
         printf("\n\n- Starting PAT ...\n");
         {
            // us_hydrodyn->write_dati1_supc_bead_model( "supc_pre_pat", nat, dt );
            int retval = us_hydrodyn_pat_main(nmax,
                                              nat,
                                              dt,
                                              &out_nat,
                                              out_dt);
            printf("pat returns %d\n", retval);
            if ( retval )
            {
               if ( out_dt )
               {
                  free(out_dt);
                  out_dt = 0;
               }
               supc_free_alloced();
               return -1;
            }
            // us_hydrodyn->write_dati1_pat_bead_model( "supc_post_pat", out_nat, out_dt, dt );
         }
         printf("\n\n- End of PAT ...\n\n");
         smi_progress->setValue(ppos++); // 2
         qApp->processEvents();
         if (us_hydrodyn->stopFlag)
         {
            supc_free_alloced();
            return -1;
         }
         
#if defined(OLD_WAY)
         out_inter();
         
         printf("Removing ifraxon\n");
         QFile::remove("ifraxon");
         printf("Removing ofraxon\n");
         QFile::remove("ofraxon");
         printf("Removing ifraxon1\n");
         QFile::remove("ifraxon1");
         printf("Removing ofraxon1\n");
         QFile::remove("ofraxon1");
#endif
         
         printf("out_nat %d\n", out_nat); fflush(stdout);
         printf("nat %d\n", nat); fflush(stdout);

         for (i = 0; i < out_nat; i++)
         {
#if defined(MIMIC_FILE)
            dtn[i].x = QString("").sprintf("%f",out_dt[i].x).toFloat();
            dtn[i].y = QString("").sprintf("%f",out_dt[i].y).toFloat();
            dtn[i].z = QString("").sprintf("%f",out_dt[i].z).toFloat();
#else
            dtn[i].x = out_dt[i].x;
            dtn[i].y = out_dt[i].y;
            dtn[i].z = out_dt[i].z;
#endif
            if ( i < 3 ) 
            {
               printf("after out_dt bead %d @ %f %f %f\n", i, out_dt[i].x,out_dt[i].y,out_dt[i].z);
               printf("after bead %d @ %f %f %f\n", i, dtn[i].x,dtn[i].y,dtn[i].z);
            }
         }
         if ( out_dt ) {
            free( out_dt );
         }
      }
      
      for (i = 0; i < nat; i++)
      {
         x = fabs(dt[i].x - dtn[i].x);
         y = fabs(dt[i].y - dtn[i].y);
         z = fabs(dt[i].z - dtn[i].z);

         if ((x > 1) || (y > 1) || (z > 1))
            flag_norm = 1;
      }

      for (i = 0; i < nat; i++)
      {
         dt[i].x = dtn[i].x;
         dt[i].y = dtn[i].y;
         dt[i].z = dtn[i].z;
      }
#if defined(DEBUG_WW)
      for (i = 0; i < nat; i++) {
         cks += (double)dt[i].x;
         cks += (double)dt[i].y;
         cks += (double)dt[i].z;
      }
      dww("main before overlap");
#endif

      if (overlap() == 1)
      {
#if defined(USE_MAIN)
         printf("\n\n Hit any key to exit");
         getchar();
#else
# if defined(DEBUG_WW)
         dww("final/overlaps");
         fclose(logfx);
# endif
         printf("overlaps detected\n");
         supc_free_alloced();
         smi_progress->setValue(mppos); 
         qApp->processEvents();
         if (us_hydrodyn->stopFlag)
         {
            supc_free_alloced();
            return -1;
         }
         return US_HYDRODYN_SUPC_OVERLAPS_EXIST;
#endif
         break;
      }
      /*      MOVED INSIDE INIT.C
              printf("\n\n Starting function: ragir()\n");
              ragir(); 
              printf("\n End of function: ragir()\n"); */

      smi_progress->setValue(ppos++); //3
      qApp->processEvents();
      if (us_hydrodyn->stopFlag)
      {
         supc_free_alloced();
         return -1;
      }
      /* RECOMPUTING THE CENTER OF MASS */

      mtx = 0.0;
      xm = 0.0;
      ym = 0.0;
      zm = 0.0;

      for (i = 0; i < nat; i++)
         mtx += dt[i].m;

      for (i = 0; i < nat; i++)
      {
         xm += dt[i].x * dt[i].m;
         ym += dt[i].y * dt[i].m;
         zm += dt[i].z * dt[i].m;
      }

      xm = xm / mtx;
      ym = ym / mtx;
      zm = zm / mtx;

#if defined(TSUDA_DOUBLESUM)
      printf("\n\n Starting function: tsuda()\n");
      tsuda();
      printf("\n End of function: tsuda()\n");
#endif
      smi_progress->setValue(ppos++); // 4
      qApp->processEvents();
      if (us_hydrodyn->stopFlag)
      {
         supc_free_alloced();
         return -1;
      }
#if defined(TSUDA_DOUBLESUM)
      printf("\n Starting function: doublesum()\n");
      doublesum();
      printf("\n End of function: doublesum()\n");
#endif
      smi_progress->setValue(ppos++); // 5
      qApp->processEvents();
      if (us_hydrodyn->stopFlag)
      {
         supc_free_alloced();
         return -1;
      }

      printf("\n Starting function: calcqij()\n");
      calcqij();
      printf("\n\n End of function: calcqij()\n");
      smi_progress->setValue(ppos++); // 6
      qApp->processEvents();
      if (us_hydrodyn->stopFlag)
      {
         supc_free_alloced();
         return -1;
      }

      presentazione();
      printf("- Computational Method : SUPERMATRIX INVERSION\n\n");
#if defined(CREATE_EXE_TIME)
      if (flag_mem == 1)
      {
         exe_time = us_fopen("exe_time", "wb");
         fprintf(exe_time, "\n%s\n", "BEAMS -           IST. CBA                    COEFF/SUPC");
         fprintf(exe_time, "\n%s\n\n", "*** Computational Method : SUPERMATRIX INVERSION chol ***");
         fprintf(exe_time, "Date: %s %d %s %d %s\n", day, numday, month, year, hour);
         fprintf(exe_time, "%s", "MODEL FILE NAME :___ ");
         fprintf(exe_time, "%s\n", molecola_nb);
         fprintf(exe_time, "%s%d\n", "BEADS in the MODEL :___ ", numero_sfere);
         // fprintf(exe_time, "%s%d\n", "FIRST BEAD # INCLUDED  :___ ", prima);
         // fprintf(exe_time, "%s%d\n", "LAST BEAD # INCLUDED :___ ", ultima);

         if (flag_norm == 1)
            fprintf(exe_time, "\n%s\n\n", "- NORMALIZED MOLECULE -");
         else
            fprintf(exe_time, "\n");
         fclose(exe_time);
         // system("date >> exe_time");
      }
#endif

      primo = time(NULL);   /* Gets system time */
      smi_progress->setValue(ppos++); // 7
      qApp->processEvents();
      if (us_hydrodyn->stopFlag)
      {
         supc_free_alloced();
         return -1;
      }

#if defined(SHOW_TIMING)
      cholsl_s1 = 0l;
      cholsl_s2 = 0l;
      supc3_s1 = 0l;
      supc3_s2 = 0l;
      supc3_tot = 0l;

      supc1_tot = 0l;
      supc2_tot = 0l;

      supc_tot = 0l;
#endif
#if defined(USE_THREADS)
      US_Config *USglobal = new US_Config();
      threads = USglobal->config_list.numThreads;
      if ( threads > 1 )
      {
         // create threads

         cout << QString("Using %1 threads for matrix inversion.\n").arg(threads);
         // editor->append(QString("Using %1 threads for matrix inversion.\n").arg(threads));
         
         supc_thr_threads.resize(threads);
         for ( int j = 0; j < threads; j++ )
         {
            supc_thr_threads[j] = new supc_thr_t(j);
            supc_thr_threads[j]->start();
         }
      }
      delete USglobal;
#endif
         
      riempimatrice();

      smi_progress->setValue(ppos++); // 8
      qApp->processEvents();
      if (us_hydrodyn->stopFlag)
      {
         supc_free_alloced();
         return -1;
      }

      inverti(nat);

      printf("- Matrix S_ij     : ");
      printf("calculated\n");

#if defined(SHOW_TIMING)
      supc_tot = supc1_tot + supc2_tot + supc3_tot;
      printf("supc timing:\n"
             "supc1 tot      %lu %.2g\n"
             "supc2 tot      %lu %.2g\n"
             "supc3 s1       %lu %.2g\n"
             "cholsl loop 1  %lu %.2g\n"
             "cholsl loop 2  %lu %.2g\n"
             "supc3 s2       %lu %.2g\n"
             "supc3 tot      %lu %.2g\n"
             "supc tot       %lu\n"
             , supc1_tot, (double) supc1_tot / (double) supc_tot
             , supc2_tot, (double) supc2_tot / (double) supc_tot
             , supc3_s1, (double) supc3_s1 / (double) supc_tot
             , cholsl_s1, (double) cholsl_s1 / (double) supc_tot
             , cholsl_s2, (double) cholsl_s2 / (double) supc_tot
             , supc3_s2, (double) supc3_s2 / (double) supc_tot
             , supc3_tot, (double) supc3_s1 / (double) supc_tot
             , supc_tot
             );
#endif

#if defined(USE_THREADS)
      if ( threads > 1 ) 
      {
         // destroy threads

         int j;
    
         for ( j = 0; j < threads; j++ )
         {
            supc_thr_threads[j]->supc_thr_shutdown();
         }
         
         for ( j = 0; j < threads; j++ )
         {
            supc_thr_threads[j]->wait();
         }
         
         for ( j = 0; j < threads; j++ )
         {
            delete supc_thr_threads[j];
         }
      }
#endif


      qApp->processEvents();
      if (us_hydrodyn->stopFlag)
      {
         supc_free_alloced();
         return -1;
      }

      visco();
#if defined(TSUDA_DOUBLESUM)
      tsuda1();
#endif
      smi_progress->setValue(ppos++); // 9
      qApp->processEvents();
      if (us_hydrodyn->stopFlag)
      {
         supc_free_alloced();
         return -1;
      }

      printf("- Matrix KSI_T  : ");
      sigmatcalc2();
      printf("calculated\n");

      smi_progress->setValue(ppos++); // 10
      qApp->processEvents();
      if (us_hydrodyn->stopFlag)
      {
         supc_free_alloced();
         return -1;
      }

      printf("- Matrix KSI_OC : ");
      sigmaocalc1();
      printf("calculated\n");
      smi_progress->setValue(ppos++); // 11
      qApp->processEvents();
      if (us_hydrodyn->stopFlag)
      {
         supc_free_alloced();
         return -1;
      }

      calcR();

      smi_progress->setValue(ppos++); // 12
      qApp->processEvents();
      if (us_hydrodyn->stopFlag)
      {
         supc_free_alloced();
         return -1;
      }
      printf("- Matrix KSI_Rr  : ");
      sigmarRcalc1();
      printf("calculated\n");

      smi_progress->setValue(ppos++); // 13
      qApp->processEvents();
      if (us_hydrodyn->stopFlag)
      {
         supc_free_alloced();
         return -1;
      }
      printf("- Matrix KSI_OR: ");
      sigmaoRcalc();
      printf("calculated\n");
      smi_progress->setValue(ppos++); // 14
      qApp->processEvents();
      if (us_hydrodyn->stopFlag)
      {
         supc_free_alloced();
         return -1;
      }

      diffcalc();

      smi_progress->setValue(ppos++); // 15
      qApp->processEvents();
      if (us_hydrodyn->stopFlag)
      {
         supc_free_alloced();
         return -1;
      }

      relax_rigid_calc();

      smi_progress->setValue(ppos++); // 16
      qApp->processEvents();
      if (us_hydrodyn->stopFlag)
      {
         supc_free_alloced();
         return -1;
      }

      if (cd == 2)
      {
         calcD();
         DDtcalc();
      }

      printf("- Matrix Doc      : calculated\n");
      printf("- Matrix Doct     : calculated\n");
      printf("- Matrix Dot      : calculated\n");
      printf("- Matrix Dr       : calculated\n");
      secondo = time(NULL);   /* Gets system time again */

#if defined(CREATE_EXE_TIME)
      if (flag_mem == 1)
      {
         // system("date >> exe_time");
         exe_time = us_fopen("exe_time", "ab");
         print_time_2IO((int) difftime(secondo, primo));
         /*
           Substitute the previous line with the following for compiling under SUN O.S.
           print_time_2IO(secondo-primo);
         */
         fclose(exe_time);
      }
#endif

      presentazione();
      printf("- Computational Method : SUPERMATRIX INVERSION\n\n\n");
      totvol = 0.0;
      totsup = 0.0;
      totvolb = 0.0;

      if ((volcor == 1) && (sfecalc == 2)) {
         totvolb = interm1 / (6.0 * ETAo);
      }
      
      if ((volcor == 1) && ((colorsixf == 2) || (colorsixf == 3))) {
         totvol = interm1 / (6.0 * ETAo);
      } else {
         for (i = 0; i < nat; i++) {
            totvol += (4.0 / 3.0 * M_PI * dt[i].r * dt[i].r * dt[i].r);
         }
         totvolb = totvol;
      }

      for (i = 0; i < nat; i++) {
         totsup += M_PI * dt[i].r * dt[i].r * 4.0;
      }

      if (num != 1)      /* average values of parameters */
      {

         temp = 0.0;

         if (cc == 1)
            bc = 6;
         else
            bc = 4;

         if (mascor == 1)
            mascor1 = (float) pesmol;

         // ff0 calc
         temp = f * 1.0E-08 / 
            ( 6.0 * M_PI * ETAo *
              pow( 3.0 * mascor1 * partvol / (4.0 * M_PI * AVOGADRO), 1.0/3.0));
         ff0_t += temp;
         ff0_t2 += temp * temp;
         temp = 0.0;

         CfT += f * 1.0E-7 * fconv;
         CfT2 += pow((f * 1.0E-7 * fconv), 2);

         CdT += (KB * TE * 1.0E7) / f * fconv1;
         CdT2 += pow(((KB * TE * 1.0E7) / f * fconv1), 2);

         if ((raflag == -1.0) || (raflag == -3.0))
         {
            CST += (mascor1 * 1.0E20 * (1.0 - partvolc * DENS)) / (f * fconv * AVO);
            CST2 += pow((mascor1 * 1.0E20 * (1.0 - partvolc * DENS)) / (f * fconv * AVO), 2);
         }

         if ((raflag == -2.0) || (raflag == -3.0) || (raflag == -5.0))
         {
            CSTF += (mascor1 * 1.0E20 * (1.0 - partvol * DENS)) / (f * fconv * AVO);
            CSTF2 += pow((mascor1 * 1.0E20 * (1.0 - partvol * DENS)) / (f * fconv * AVO), 2);
         }

         Rg += ro * fconv;
         Rg2 += pow((ro * fconv), 2);

         if ((raflag == -1.0) || (raflag == -3.0))
         {
            Rgu += rou * fconv;
            Rgu2 += pow((rou * fconv), 2);
         }

         RSt += f * fconv / (bc * M_PI * ETAo);
         RSt2 += pow((f * fconv / (bc * M_PI * ETAo)), 2);

         temp = 0.0;
         for (i = 0; i < 3; i++)
            temp += 1.0E-21 / Dr[i * 4] * pow(fconv, 3);
         CfR1 += temp / 3.0;
         CfR12 += pow((temp / 3.0), 2);

         temp = 0.0;
         for (i = 0; i < 3; i++)
            temp += (KB * TE * Dr[i * 4] / 1.0E-21) * pow(fconv1, 3);
         CdR1 += temp / 3.0;
         CdR12 += pow((temp / 3.0), 2);

         for (i = 0; i < 3; i++)
         {
            temp = 0.0;
            RSr[i] += pow((3.0 / Dr[i * 4] / bc / 4.0 / M_PI / ETAo), 0.333333L) * fconv;
            temp += pow((3.0 / Dr[i * 4] / bc / 4.0 / M_PI / ETAo), 0.333333L) * fconv;
            RSr2[i] += pow(temp, 2);

            CdR[i] += (KB * TE * Dr[i * 4] / 1.0E-21) * pow(fconv1, 3);
            temp = (KB * TE * Dr[i * 4] / 1.0E-21) * pow(fconv1, 3);
            CdR2[i] += pow(temp, 2);

            CfR[i] += 1.0E-21 / Dr[i * 4] * pow(fconv, 3.0f);
            temp = 1.0E-21 / Dr[i * 4] * pow(fconv, 3.0f);
            CfR2[i] += pow(temp, 2.0f);
         }

         if (mascor == 2)
         {
            correz = pesmol / mascor1;
            vol_mas = mascor1;
         }
         else
         {
            correz = 1.0;
            vol_mas = pesmol;
         }

         VIM += (double) vis * correz * pow(fconv, 3);
         temp = (double) vis * correz * pow(fconv, 3);
         VIM2 += pow(temp, 2.0f);

         RE += 1.0E7 * pow((0.3 * pesmol * vis / (M_PI * AVO)), 0.333333) * fconv;
         temp = 1.0E7 * pow((0.3 * pesmol * vis / (M_PI * AVO)), 0.333333) * fconv;
         RE2 += pow(temp, 2);

         VIMC += (vis * correz + vis3 * totvol / vol_mas) * pow(fconv, 3);
         temp = (vis * correz + vis3 * totvol / vol_mas) * pow(fconv, 3);
         VIMC2 += pow(temp, 2);

         REC += 1.0E7 * pow((0.3 * vol_mas * (vis * correz + vis3 * totvol / vol_mas) / (M_PI * AVO)), 0.333333) * fconv;
         temp = 1.0E7 * pow((0.3 * vol_mas * (vis * correz + vis3 * totvol / vol_mas) / (M_PI * AVO)), 0.333333) * fconv;
         REC2 += pow(temp, 2);

#if defined(TSUDA_DOUBLESUM)
         VIMDS += (vis4 * correz) * pow(fconv, 3);
         temp = (vis4 * correz) * pow(fconv, 3);
         VIMDS2 += pow(temp, 2.0f);

         REDS += 1.0E7 * pow((0.3 * pesmol * vis4 / (M_PI * AVO)), 0.333333) * fconv;
         temp = 1.0E7 * pow((0.3 * pesmol * vis4 / (M_PI * AVO)), 0.333333) * fconv;
         REDS2 += pow(temp, 2);

         VIMTM += (vis1 * correz) * pow(fconv, 3);
         temp = (vis1 * correz) * pow(fconv, 3);
         VIMTM2 += pow(temp, 2.0f);

         RETM += 1.0E7 * pow((0.3 * pesmol * vis1 / (M_PI * AVO)), 0.333333) * fconv;
         temp = 1.0E7 * pow((0.3 * pesmol * vis1 / (M_PI * AVO)), 0.333333) * fconv;
         RETM2 += pow(temp, 2.0f);

         VIMTV += (vis2 * correz) * pow(fconv, 3);
         temp = (vis2 * correz) * pow(fconv, 3);
         VIMTV2 += pow(temp, 2.0f);

         RETV += 1.0E7 * pow((0.3 * pesmol * vis2 / (M_PI * AVO)), 0.333333) * fconv;
         temp = 1.0E7 * pow((0.3 * pesmol * vis2 / (M_PI * AVO)), 0.333333) * fconv;
         RETV2 += pow(temp, 2);
#endif

         for (i = 0; i < 5; i++)
         {
            CT[i] += tao[i] * pow(fconv, 3.0f);
            temp = tao[i] * pow(fconv, 3.0f);
            CT2[i] += pow(temp, 2.0f);
         }
         CTM += taom * pow(fconv, 3.0f);
         temp = taom * pow(fconv, 3.0f);
         CTM2 += pow(temp, 2.0f);
         CTH += taoh * 1.0E9 * pow(fconv, 3.0f);
         temp = taoh * 1.0E9 * pow(fconv, 3.0f);
         CTH2 += pow(temp, 2.0f);

      }

      maxest();

      supc_timers.end_timer( "compute smi" );

      stampa_ris();

      if (flag_mem == 1)
         mem_ris(k);

      if (num != 1)
      {
         if ((raflag == -4.0) || (raflag == -5.0))   /* CLEARING THE MEMORY SPACE ALLOCATED FOR AA-BEADS CORRESPONDENCE */
         {
            for (i = 0; i < nat; i++)
            {
               free(dt[i].cor);
            }
         }
      }
      if (num == 1)
      {
         while (scelta == 1)
         {
            print_time((int) difftime(secondo, primo));

            /*
              Substitute the previous line with the following for compiling under SUN O.S.

              print_time(secondo-primo);
            */
#if defined(USE_MAIN)
            printf("\n\n- Options :       \n\n");
            printf("  0) EXIT\n");
            printf("  1) VIEW MATRICES\n");

            if (flag_norm == 1)
            {
               printf("  2) STORE Bead Model\n\n");
               printf("  Select (0/1/2) : ");
            }
            else
            {
               printf("\n");
               printf("  Select (0/1) : ");
            }

            if ( 1 == scanf("%d", &scelta) ) {};
            getchar();

            if (scelta == 1)
               vedimatrici();
#else
            scelta = 0;
#endif
         }

         if (scelta == 2)
         {
            presentazione();
            mem_mol();
         }
         if ((raflag == -4.0) || (raflag == -5.0))   /* CLEARING THE MEMORY SPACE ALLOCATED FOR AA-BEADS CORRESPONDENCE */
         {
            for (i = 0; i < numero_sfere; i++)
            {
               if ( dt[i].cor ) {
                  free(dt[i].cor);
                  dt[i].cor = 0;
               }
            }
         }
      }

   }

   supc_results->num_models = num;
   
   if (num != 1)
   {
      val_med();
   }
   else
   {
      supc_results->total_beads_sd = 0;
      supc_results->used_beads_sd = 0;
   }

#if defined(CREATE_TOT_MOL)
   fclose(tot_mol);
   //   QFile::remove("tot_mol");
#endif

#if defined(CREATE_EXE_TIME)
   QFile::remove("exe_time");
#endif

#if defined(DEBUG_WW)
   dww("final");
   fclose(logfx);
#endif

   supc_free_alloced();
   smi_progress->setValue(mppos); 
   qApp->processEvents();
   if (us_hydrodyn->stopFlag)
   {
      return -1;
   }

   return 0;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

static void
intestazione()
{
   printf("####################################################\n");
   printf("#   National Institute for Cancer Research (IST)   #\n");
   printf("#         Advanced Biotechnologies Center (CBA)    #\n");
   printf("#                   Genova, ITALY                  #\n");
   printf("####################################################\n");
   printf("#  SUPC - HYDRODYNAMIC PROPERTIES COMPUTATION      #\n");
   printf("#            Version 5.0  Februaury 2010           #\n");
   printf("####################################################\n\n");
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

static void
presentazione()
{
   printf("####################################################\n");
   printf("#   National Institute for Cancer Research (IST)   #\n");
   printf("#         Advanced Biotechnologies Center (CBA)    #\n");
   printf("#                   Genova, ITALY                  #\n");
   printf("####################################################\n");
   printf("#  SUPC - HYDRODYNAMIC PROPERTIES COMPUTATION      #\n");
   printf("#            Version 5.0  Februaury 2010           #\n");
   printf("####################################################\n\n");
   printf("- Model        : %s\n", molecola);
   printf("- TOTAL Beads in the MODEL : %d\n", numero_sfere);

   // printf("%s%d\n", "- FIRST Bead Included  : ", prima);
   // printf("%s%d\n", "- LAST Bead Included : ", ultima);
   if (colorzero == 1)
   {
      printf("\n%s%d%s\n", "- WARNING: THERE IS ", colorzero, " BEAD COLOR CODED '0' [RADIUS < 0.005 NM]");
      printf("%s\n", "- THIS BEAD IS NOT USED IN THE HYDRODYNAMIC COMPUTATIONS\n");
   }
   if (colorzero > 1)
   {
      printf("\n%s%d%s\n", "- WARNING: THERE ARE ", colorzero, " BEADS COLOR CODED '0' [RADIUS < 0.005 NM]");
      printf("%s\n", "- THOSE BEADS ARE NOT USED IN THE HYDRODYNAMIC COMPUTATIONS\n");
   }

   if (flag_norm == 1)
      printf("%s\n", "- Model     : NORMALIZED");

   if (cc == 1)
      printf("\n- STICK boundary condition (6*M_PI*ETAo)\n");
   else
      printf("- SLIP boundary condition (4*M_PI*ETAo)\n\n");
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

#if defined(USE_MAIN)
static void
vedimatrici()
{

   int mtt;

   mtt = 1;
   while (mtt != 0)
   {
      presentazione();
      printf("\n- Matrices :\n");
      printf("\n  1)  KSI_T\n");
      printf("  2)  KSI_OC\n");
      printf("  3)  KSI_Rr\n");
      printf("  4)  KSI_OR\n");
      printf("  5)  Doc\n");
      printf("  6)  Doct\n");
      printf("  7)  Dot\n");
      printf("  8)  Dr\n");
      printf("\n  0)  Back\n");

      printf("\n  Select(0/8) : ");
      if ( 1 == scanf("%d", &mtt) ) {};
      getchar();

      switch (mtt)
      {
      case 0:
         return;
      case 1:
         {
            presentazione();
            printf("\n- Matrix : KSI_T\n\n\n");
            stampamatrice(sigmat);
         }
         break;
      case 2:
         {
            presentazione();
            printf("\n- Matrix : KSI_OC\n\n\n");
            stampamatrice(sigmaoc);
         }
         break;
      case 3:
         {
            presentazione();
            printf("\n- Matrix : KSIRr\n\n\n");
            stampamatrice(sigmaRr);
         }
         break;
      case 4:
         {
            presentazione();
            printf("\n- Matrix : KSIR_OR\n\n\n");
            stampamatrice(sigmaoR);
         }
         break;
      case 5:
         {
            presentazione();
            printf("\n- Matrix : Doc\n\n\n");
            stampamatrice1(Doc);
         }
         break;
      case 6:
         {
            presentazione();
            printf("\n- Matrix : Doct\n\n\n");
            stampamatrice1(Doct);
         }
         break;
      case 7:
         {
            presentazione();
            printf("\n- Matrix : Dot\n\n\n");
            stampamatrice1(Dot);
         }
         break;
      case 8:
         {
            presentazione();
            printf("\n- Matrix : Dr\n\n\n");
            stampamatrice1l(Dr);
         }
         break;
      }
   }
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

static void
stampamatrice(float *n)
{

   int i;
   float ss;

   ss = 1.0;
   if (metodo != 3)
   {
      if (cc == 1)
         ss = 6.0 * M_PI * ETAo;
      else
         ss = 4.0 * M_PI * ETAo;
   }

   for (i = 0; i < 3; i++)
   {
      printf("\t%f\t%f\t%f", ss * n[3 * i], ss * n[3 * i + 1], ss * n[3 * i + 2]);
      printf("\n");
   }

   printf("\n\n  Hit RETURN to go back");
   getchar();

}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

static void
stampamatrice1(float *n)
{

   int i;
   float ss;

   ss = 1.0;
   if (metodo != 3)
   {
      if (cc == 1)
         ss = 6.0 * M_PI * ETAo;
      else
         ss = 4.0 * M_PI * ETAo;
   }

   for (i = 0; i < 3; i++)
   {
      printf("\t%f\t%f\t%f", n[3 * i] / ss, n[3 * i + 1] / ss, n[3 * i + 2] / ss);
      printf("\n");
   }

   printf("\n\n  Hit RETURN to go back");
   getchar();

}

static void
stampamatrice1l(long double *n)
{

   int i;
   float ss;

   ss = 1.0;
   if (metodo != 3)
   {
      if (cc == 1)
         ss = 6.0 * M_PI * ETAo;
      else
         ss = 4.0 * M_PI * ETAo;
   }

   for (i = 0; i < 3; i++)
   {
      printf("\t%f\t%f\t%f", (double) (n[3 * i] / ss), (double) (n[3 * i + 1] / ss), (double) (n[3 * i + 2] / ss));
      printf("\n");
   }

   printf("\n\n  Hit RETURN to go back");
   getchar();

}
#endif
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

static void
stampa_ris()
{

   float einst, bc, temp;
   int i;
   if (cc == 1)
      bc = 6;
   else
      bc = 4;

   // solvent area
   printf("Solvent:                %s\n", supc_results->solvent_name.toLatin1().data());
   printf("Temperature:            %5.2f\n", supc_results->temperature);
   printf("Solvent viscosity (cP): %f\n", supc_results->solvent_viscosity);
   printf("Solvent density (g/ml): %f\n", supc_results->solvent_density);
   printf("\n");

   // temporary vbar area
   printf("Original vbar:          %5.3f (%s)\n", 
          org_vbar, 
          us_hydrodyn->misc.compute_vbar ?
          ( us_hydrodyn->bead_model_from_file ?
            "from file" : "computed" ) : "user entered");
   if ( !us_hydrodyn->misc.compute_vbar )
   {
      printf("Original vbar temp (C): %5.2f\n", us_hydrodyn->misc.vbar_temperature);
   }
   printf("Temp corrected vbar:    %5.3f\n", tc_vbar);

   printf("\n\n%s%d\n", "- Used BEADS Number  = ", nat);
   supc_results->used_beads = nat;
   tot_used_beads += (float)nat;
   tot_used_beads2 += (float)nat * (float)nat;

   if (volcor == 1)
   {
      if ((colorsixf == 0) && (sfecalc == 2))
         printf("%s%.2f%s\n", "- Used BEADS Volume = ", volcor1 * pow(fconv, 3.0f),
                "  [nm^3] (NO contribution from buried beads)");
      if ((colorsixf == 1) && (sfecalc == 2))
      {
         printf("%s%.2f%s\n", "- Used BEADS Volume = ", volcor1 * pow(fconv, 3.0f),
                "  [nm^3] (contribution from buried beads only for Dr)");
         printf("%s%.2f%s\n", "- Used BEADS Volume = ", totvol * pow(fconv, 3.0f), "  [nm^3] (for [n])");
      }
      if ((colorsixf == 2) && (sfecalc == 2))
      {
         printf("%s%.2f%s\n", "- Used BEADS Volume = ", volcor1 * pow(fconv, 3.0f),
                "  [nm^3] (contribution from buried beads only for [n])");
         printf("%s%.2f%s\n", "- Used BEADS Volume = ", totvol * pow(fconv, 3.0f), "  [nm^3] (for Dr)");
      }
      if ((colorsixf == 3) && (sfecalc == 2))
         printf("%s%.2f%s\n", "- Used BEADS Volume  = ", volcor1 * pow(fconv, 3.0f), "  [nm^3]");
      if (sfecalc == 1)
         printf("%s%.2f%s\n", "- Used BEADS Volume  = ", volcor1 * pow(fconv, 3.0f), "  [nm^3]");
   }
   if (volcor == 2)
      printf("%s%.2f%s\n", "- Used BEADS Volume  = ", volcor1 * pow(fconv, 3.0f), "  [nm^3]");

   if (mascor == 1)
      mascor1 = (float) pesmol;

   printf("%s%.2f%s\n", "- Used BEADS Mass    = ", mascor1, "  [Da]");
   supc_results->mass = mascor1;
   printf("\n\n%s%.3e\t%s (%s)\n", "- TRANS. FRICT. COEFF.  = ", f * 1.0E-07 * fconv, "[g/s] ", tag1.toLatin1().data());
   // supc_results->s20w = f * 1.0E-07 * fconv;

   printf("%s%.2e\t%s (%s)\n", "- TRANS. DIFF. COEFF.   = ", (KB * TE * 1.0E7) / f * fconv1, "[cm^2/s] ", tag2.toLatin1().data());
   supc_results->D20w = (KB * TE * 1.0E7) / f * fconv1;


   //   printf("%s%.4e\n", "- FRICTIONAL RATIO (from Mass, s) = ", 
   //     ( supc_results->mass * (1.0 - partvol * DENS_20W) / 
   //       ( AVOGADRO * supc_results->s20w * 1e-13) ) 
   //     /
   //     ( 6.0 * M_PI * ETAo * 
   //       pow(3.0 * supc_results->mass * partvol / (4.0 * M_PI * AVOGADRO), 1.0/3.0)) 
   //     );

   if (raflag == -1.0) 
   {
      printf("%s%.3g\t%s (%s)\n", "- SED. COEFF. (psv from unhydrated radii) = ",
             (mascor1 * 1.0E20 * (1.0 - partvolc * DENS)) / (f * fconv * AVO), "[S] ", tag2.toLatin1().data());
#if defined( DEBUG_S )
      us_qdebug( QString( "ra -1 mascor1 is %1 partvolc %2 DENS %3 f %4" )
              .arg( mascor1 )
              .arg( partvolc )
              .arg( DENS )
              .arg( f ) );
#endif
      supc_results->s20w = (mascor1 * 1.0E20 * (1.0 - partvolc * DENS)) / (f * fconv * AVO);
   }

   if ((raflag == -2.0) || (raflag == -5.0))
   {
      printf("- SED. COEFF. (psv %s) = %.3g\t%s (%s)\n", 
             us_hydrodyn->misc.compute_vbar ?
             ( us_hydrodyn->bead_model_from_file ?
               "from file" : "computed" ) : "user entered",
             (mascor1 * 1.0E20 * (1.0 - partvol * DENS)) / (f * fconv * AVO), "[S] ", 
             tag2.toLatin1().data()
             );
#if defined( DEBUG_S )
       us_qdebug( QString( "ra -2 or -5 mascor1 is %1 partvol %2 DENS %3 f %4" )
               .arg( mascor1 )
               .arg( partvol )
               .arg( DENS )
               .arg( f ) );
#endif
      supc_results->s20w = (mascor1 * 1.0E20 * (1.0 - partvol * DENS)) / (f * fconv * AVO);
      if ((nat + colorzero + colorsix) < numero_sfere)
         printf
            ("- !!WARNING: ONLY PART OF THE MODEL HAS BEEN ANALYZED, BUT THE PSV UTILIZED         IS THAT OF THE ENTIRE MODEL!! - \n");
   }

   if (raflag == -3.0)
   {
      printf("- SED. COEFF. (psv %s) = %.3g\t%s (%s)\n",
             us_hydrodyn->misc.compute_vbar ?
             ( us_hydrodyn->bead_model_from_file ?
               "from file" : "computed" ) : "user entered",
             (mascor1 * 1.0E20 * (1.0 - partvol * DENS)) / (f * fconv * AVO), "[S] ", 
             tag2.toLatin1().data()
             );
#if defined( DEBUG_S )
       us_qdebug( QString( "ra -3 mascor1 is %1 partvol %2 DENS %3 f %4" )
               .arg( mascor1 )
               .arg( partvol )
               .arg( DENS )
               .arg( f ) );
#endif
      supc_results->s20w = (mascor1 * 1.0E20 * (1.0 - partvol * DENS)) / (f * fconv * AVO);
      if ((nat + colorzero + colorsix) < numero_sfere)
         printf
            ("- !!WARNING: ONLY PART OF THE MODEL HAS BEEN ANALYZED, BUT THE PSV UTILIZED         IS THAT OF THE ENTIRE MODEL!! - \n");

      printf("%s%.2g\t%s (%s)\n", "- SED. COEFF. (psv from unhydrated radii) = ",
             (mascor1 * 1.0E20 * (1.0 - partvolc * DENS)) / (f * fconv * AVO), "[S] ", tag2.toLatin1().data());
   }

#if defined( DEBUG_S )
   us_qdebug( QString( "f is %1 ETAo %2 partvol %3 fconv %4 mass %5" ).arg( f ).arg( ETAo ).arg( partvol ).arg( fconv ).arg( supc_results->mass ) );
#endif

   // ff0 calc
   supc_results->ff0 = 
      f * 1.0E-08 / ( 6.0 * M_PI * ETAo *
      pow( 3.0 * supc_results->mass * partvol / (4.0 * M_PI * AVOGADRO), 1.0/3.0));

   printf("%s%.2g\n", "- FRICTIONAL RATIO                = ", supc_results->ff0);

   if ( us_hydrodyn->advanced_config.expert_mode ) {
      temp = 0.0;
      for (i = 0; i < 3; i++)
         temp += 1.0E-21 / Dr[i * 4] * pow(fconv, 3);
      printf("\n%s%.3e\t%s (%s)\n", "- ROT. FRICT. COEFF.    = ", temp / 3.0, "[g*cm^2/s] ", tag1.toLatin1().data());
      temp = 0.0;
      for (i = 0; i < 3; i++)
         temp += (KB * TE * Dr[i * 4] / 1.0E-21) * pow(fconv1, 3);
      printf("%s%.0f\t%s (%s)\n\n", "- ROT. DIFF. COEFF.     = ", temp / 3.0, "[1/s] ", tag2.toLatin1().data());

#if !defined( MINGW )
      printf("%s%.3Le\t%s (%s)\n", "- ROT. FRICT. COEFF. [ X ] = ", 1.0E-21 / Dr[0] * pow(fconv, 3), "[g*cm^2/s] ", tag1.toLatin1().data());
      printf("%s%.3Le\t%s (%s)\n", "- ROT. FRICT. COEFF. [ Y ] = ", 1.0E-21 / Dr[4] * pow(fconv, 3), "[g*cm^2/s] ", tag1.toLatin1().data());
      printf("%s%.3Le\t%s (%s)\n", "- ROT. FRICT. COEFF. [ Z ] = ", 1.0E-21 / Dr[8] * pow(fconv, 3), "[g*cm^2/s] ", tag1.toLatin1().data());
      printf("%s%.0Lf\t%s (%s)\n", "- ROT. DIFF. COEFF.  [ X ] = ", (KB * TE / 1.0E-21 * Dr[0]) * pow(fconv1, 3), "[1/s] ", tag2.toLatin1().data());
      printf("%s%.0Lf\t%s (%s)\n", "- ROT. DIFF. COEFF.  [ Y ] = ", (KB * TE / 1.0E-21 * Dr[4]) * pow(fconv1, 3), "[1/s] ", tag2.toLatin1().data());
      printf("%s%.0Lf\t%s (%s)\n\n", "- ROT. DIFF. COEFF.  [ Z ] = ", (KB * TE / 1.0E-21 * Dr[8]) * pow(fconv1, 3), "[1/s] ", tag2.toLatin1().data());
#else
      printf("%s%.3e\t%s (%s)\n", "- ROT. FRICT. COEFF. [ X ] = ", (double)(1.0E-21 / Dr[0] * pow(fconv, 3)), "[g*cm^2/s] ", tag1.toLatin1().data());
      printf("%s%.3e\t%s (%s)\n", "- ROT. FRICT. COEFF. [ Y ] = ", (double)(1.0E-21 / Dr[4] * pow(fconv, 3)), "[g*cm^2/s] ", tag1.toLatin1().data());
      printf("%s%.3e\t%s (%s)\n", "- ROT. FRICT. COEFF. [ Z ] = ", (double)(1.0E-21 / Dr[8] * pow(fconv, 3)), "[g*cm^2/s] ", tag1.toLatin1().data());
      printf("%s%.0f\t%s (%s)\n", "- ROT. DIFF. COEFF.  [ X ] = ", (double)((KB * TE / 1.0E-21 * Dr[0]) * pow(fconv1, 3)), "[1/s] ", tag2.toLatin1().data());
      printf("%s%.0f\t%s (%s)\n", "- ROT. DIFF. COEFF.  [ Y ] = ", (double)((KB * TE / 1.0E-21 * Dr[4]) * pow(fconv1, 3)), "[1/s] ", tag2.toLatin1().data());
      printf("%s%.0f\t%s (%s)\n\n", "- ROT. DIFF. COEFF.  [ Z ] = ", (double)((KB * TE / 1.0E-21 * Dr[8]) * pow(fconv1, 3)), "[1/s] ", tag2.toLatin1().data());
#endif
   }
   printf("%s%.2f\t%s\n", "- MOLECULAR WEIGHT   (from file)         = ", pesmol, "[Da]");
   if (sfecalc == 2)
      printf("%s%.2f\t%s\n", "- BEADS TOTAL VOLUME (from file) = ", interm1 / (6.0 * ETAo) * pow(fconv, 3.0f), "[nm^3]");
   else
      printf("%s%.2f\t%s\n", "- BEADS TOTAL VOLUME (from file) = ", totvol * pow(fconv, 3), "[nm^3]");
   printf("%s%.2f\t%s\n", "- BEADS TOTAL SURFACE AREA       = ", totsup * pow(fconv, 3), "[nm^2]");

   if (raflag == -1.0)
      printf("%s%.3f\t%s\n", "- PARTIAL SPECIFIC VOLUME (from unhydrated radii) = ", partvolc, "[cm^3/g]");

   if ((raflag == -2.0) || (raflag == -5.0))
      printf("- PARTIAL SPECIFIC VOLUME %s = %.3f\t%s\n", 
             us_hydrodyn->misc.compute_vbar ?
             ( us_hydrodyn->bead_model_from_file ?
               "(from file)   " : "(computed)    " ) : "(user entered)",
             partvol, 
             "[cm^3/g]"
             );

   if (raflag == -3.0)
   {
      printf("- PARTIAL SPECIFIC VOLUME %s          = %.3f\t%s\n", 
             us_hydrodyn->misc.compute_vbar ?
             ( us_hydrodyn->bead_model_from_file ?
               "(from file)   " : "(computed)    " ) : "(user entered)", 
             partvol, "[cm^3/g]");
      printf("%s%.3f\t%s\n", 
             "- PARTIAL SPECIFIC VOLUME (from unhydrated radii) = ", partvolc, "[cm^3/g]");
   }

   if ((raflag == -1.0) || (raflag == -3.0))
   {
      us_qdebug( QString( "ra -1 or -3 ro %1" ).arg( ro ) );
      printf("\n%s%.2f\t%s\n", "- RADIUS OF GYRATION (Hydrated Beads)   = ", ro * fconv, "[nm]");
      supc_results->rg = (double) ro * fconv;
      printf("%s%.2f\t%s\n", "- RADIUS OF GYRATION (Unhydrated Beads) = ", rou * fconv, "[nm]");
   }
   else
   {
      us_qdebug( QString( "ra !(-1 or -3) ro %1" ).arg( ro ) );
      printf("\n%s%.2f\t%s\n", "- RADIUS OF GYRATION              = ", ro * fconv, "[nm]");
      supc_results->rg = (double) ro * fconv;
   }

   printf("%s%.2f\t%s\n", "- TRANSLATIONAL STOKES' RADIUS    = ", f * fconv / (bc * M_PI * ETAo), "[nm]");
   supc_results->rs = f * fconv / (bc * M_PI * ETAo);

   if ( us_hydrodyn->advanced_config.expert_mode ) {
      printf("%s%.2f\t%s\n", "- ROTATIONAL STOKES' RADIUS [ X ] = ", (double)( pow((3.0 / Dr[0] / bc / 4.0 / M_PI / ETAo), (0.33333L)) * fconv ),
             "[nm]");
      printf("%s%.2f\t%s\n", "- ROTATIONAL STOKES' RADIUS [ Y ] = ", (double)( pow((3.0 / Dr[4] / bc / 4.0 / M_PI / ETAo), (0.33333L)) * fconv ),
             "[nm]");
      printf("%s%.2f\t%s\n\n", "- ROTATIONAL STOKES' RADIUS [ Z ] = ",
             ( double )( pow((3.0 / Dr[8] / bc / 4.0 / M_PI / ETAo), (0.33333L)) * fconv ), "[nm]");
   }

   printf("%s%5.2f\t%5.2f\t%5.2f\t%s\n", "- CENTRE OF RESISTANCE  :  ", roR[0] * fconv, roR[1] * fconv, roR[2] * fconv,
          "[nm]");
   printf("%s%5.2f\t%5.2f\t%5.2f\t%s\n", "- CENTRE OF MASS        :  ", xm * fconv, ym * fconv, zm * fconv, "[nm]");
   if (cd == 2)
      printf("%s%5.2f\t%5.2f\t%5.2f\t%s\n", "- CENTRE OF DIFFUSION   :  ", roD[0] * fconv, roD[1] * fconv, roD[2] * fconv,
             "[nm]");

   printf("%s%5.2f\t%5.2f\t%5.2f\t%s\n\n", "- CENTRE OF VISCOSITY   :  ", vc[0] * fconv, vc[1] * fconv, vc[2] * fconv, "[nm]");

   if (mascor == 2)
   {
      correz = pesmol / mascor1;
      vol_mas = mascor1;
   }
   else
   {
      correz = 1.0;
      vol_mas = pesmol;
   }

   if ( us_hydrodyn->advanced_config.expert_mode ) {
      printf("%s%.2f\t%s\n", "- UNCORRECTED INTRINSIC VISCOSITY     = ", (double) vis * correz * pow(fconv, 3), "[cm^3/g]");

      einst = pow(0.3 * pesmol * vis / ( M_PI * AVO), 0.33333);
      einst = 1E7 * einst;
      printf("%s%.2f\t%s\n", "- UNCORRECTED EINSTEIN'S RADIUS       = ", einst * fconv, "[nm]");
      printf( "vis %f correz %f vis3 %f totvol %f vol_mas %f\n",
              vis,
              correz,
              vis3,
              totvol,
              vol_mas );
      if ((volcor == 1) && ((colorsixf == 0) || (colorsixf == 1) || (colorsixf == 2)))
      {
         if ( hydro_use_avg_for_volume ) {
            qDebug() << "method 1 avg - use asa vol " << asa_vol[ active_model ];
            double use_vol = asa_vol[ active_model ];
            printf("%s%.2f\t%s\n", "- CORRECTED INTRINSIC VISCOSITY       = ",
                   ((double) vis * correz + vis3 * use_vol / vol_mas) * pow(fconv, 3), "[cm^3/g]");
            supc_results->viscosity = ((double) vis * correz + vis3 * use_vol / vol_mas) * pow(fconv, 3);
            einst = pow(0.3 * vol_mas * ((double) vis * correz + vis3 * use_vol / vol_mas) / ( M_PI * AVO), 0.33333);
            einst = 1E7 * einst;
            // printf("%s%.2f\t%s\n", "- EINSTEIN'S RADIUS (GDLT corrected)  = ", einst * fconv, "[nm]");
            printf("%s%.2f\t%s\n", "- CORRECTED EINSTEIN'S RADIUS         = ", einst * fconv, "[nm]");
         } else {
            puts( "method 1 auto totvol" );
            // printf("%s%.2f\t%s\n", "- INTRINSIC VISCOSITY(GDLT corrected) = ",
            printf("%s%.2f\t%s\n", "- CORRECTED INTRINSIC VISCOSITY       = ",
                   (vis * correz + vis3 * totvol / vol_mas) * pow(fconv, 3), "[cm^3/g]");
            supc_results->viscosity = (vis * correz + vis3 * totvol / vol_mas) * pow(fconv, 3);
            einst = pow(0.3 * vol_mas * (vis * correz + vis3 * totvol / vol_mas) / ( M_PI * AVO), 0.33333);
            einst = 1E7 * einst;
            // printf("%s%.2f\t%s\n", "- EINSTEIN'S RADIUS (GDLT corrected)  = ", einst * fconv, "[nm]");
            printf("%s%.2f\t%s\n", "- CORRECTED EINSTEIN'S RADIUS         = ", einst * fconv, "[nm]");
         }
      }
      else
      {
         puts( "method 2 - manual volume" );
         // printf("%s%.2f\t%s\n", "- INTRINSIC VISCOSITY (GDLT corrected) = ",
         printf("%s%.2f\t%s\n", "- CORRECTED INTRINSIC VISCOSITY       = ",
                (vis * correz + vis3 * volcor1 / vol_mas) * pow(fconv, 3), "[cm^3/g]");
         supc_results->viscosity = (vis * correz + vis3 * volcor1 / vol_mas) * pow(fconv, 3);
         einst = pow(0.3 * vol_mas * (vis * correz + vis3 * volcor1 / vol_mas) / ( M_PI * AVO), 0.33333);
         einst = 1E7 * einst;
         // printf("%s%.2f\t%s\n", "- EINSTEIN'S RADIUS (GDLT corrected)  = ", einst * fconv, "[nm]");
         printf("%s%.2f\t%s\n", "- CORRECTED EINSTEIN'S RADIUS         = ", einst * fconv, "[nm]");
      }

#if defined(TSUDA_DOUBLESUM)
      printf("%s%.2f\t%s\n", "- INTRINSIC VISCOSITY(DoubleSum CM)   = ", vis4 * correz * pow(fconv, 3.0f), "[cm^3/g]");
      einst = pow(0.3 * pesmol * vis4 / ( M_PI * AVO), 0.33333);
      einst = 1E7 * einst;
      printf("%s%.2f\t%s\n", "- EINSTEIN'S RADIUS (DoubleSum CM)    = ", einst * fconv, "[nm]");
      printf("%s%.2f\t%s\n", "- INTRINSIC VISCOSITY(Tsuda CM)       = ", vis1 * correz * pow(fconv, 3.0f), "[cm^3/g]");
      einst = pow(0.3 * pesmol * vis1 / ( M_PI * AVO), 0.33333);
      einst = 1E7 * einst;
      printf("%s%.2f\t%s\n", "- EINSTEIN'S RADIUS (Tsuda CM)        = ", einst * fconv, "[nm]");
      printf("%s%.2f\t%s\n", "- INTRINSIC VISCOSITY(Tsuda CV)       = ", vis2 * correz * pow(fconv, 3.0f), "[cm^3/g]");
      einst = pow(0.3 * pesmol * vis2 / ( M_PI * AVO), 0.33333);
      einst = 1E7 * einst;
      printf("%s%.2f\t%s\n", "- EINSTEIN'S RADIUS (Tsuda CV)        = ", einst * fconv, "[nm]");
#endif

      printf("\nRELAXATION TIMES\n\n");

#if !defined( MINGW )
      if (taoflag == 1.0)
      {
         printf("%s\t%.2Lf\t%s (%s)\n", " Tau(1) ", tao[0] * pow(fconv, 3.0f), "[ns] ", tag2.toLatin1().data());
         printf("%s\t%.2Lf\t%s (%s)\n", " Tau(2) ", tao[1] * pow(fconv, 3.0f), "[ns] ", tag2.toLatin1().data());
         printf("%s\t%.2Lf\t%s (%s)\n", " Tau(3) ", tao[1] * pow(fconv, 3.0f), "[ns] ", tag2.toLatin1().data());
         printf("%s\t%.2Lf\t%s (%s)\n", " Tau(4) ", tao[3] * pow(fconv, 3.0f), "[ns] ", tag2.toLatin1().data());
         printf("%s\t%.2Lf\t%s (%s)\n", " Tau(5) ", tao[3] * pow(fconv, 3.0f), "[ns] ", tag2.toLatin1().data());
      }
      if (taoflag == 2.0)
      {
         printf("%s\t%.2Lf\t%s (%s)\n", " Tau(1) ", tao[4] * pow(fconv, 3.0f), "[ns] ", tag2.toLatin1().data());
         printf("%s\t%.2Lf\t%s (%s)\n", " Tau(2) ", tao[1] * pow(fconv, 3.0f), "[ns] ", tag2.toLatin1().data());
         printf("%s\t%.2Lf\t%s (%s)\n", " Tau(3) ", tao[1] * pow(fconv, 3.0f), "[ns] ", tag2.toLatin1().data());
         printf("%s\t%.2Lf\t%s (%s)\n", " Tau(4) ", tao[3] * pow(fconv, 3.0f), "[ns] ", tag2.toLatin1().data());
         printf("%s\t%.2Lf\t%s (%s)\n", " Tau(5) ", tao[3] * pow(fconv, 3.0f), "[ns] ", tag2.toLatin1().data());
      }
      if (taoflag == 0.0)
      {
         printf("%s\t%.2Lf\t%s (%s)\n", " Tau(1) ", tao[0] * pow(fconv, 3.0f), "[ns] ", tag2.toLatin1().data());
         printf("%s\t%.2Lf\t%s (%s)\n", " Tau(2) ", tao[1] * pow(fconv, 3.0f), "[ns] ", tag2.toLatin1().data());
         printf("%s\t%.2Lf\t%s (%s)\n", " Tau(3) ", tao[2] * pow(fconv, 3.0f), "[ns] ", tag2.toLatin1().data());
         printf("%s\t%.2Lf\t%s (%s)\n", " Tau(4) ", tao[3] * pow(fconv, 3.0f), "[ns] ", tag2.toLatin1().data());
         printf("%s\t%.2Lf\t%s (%s)\n", " Tau(5) ", tao[4] * pow(fconv, 3.0f), "[ns] ", tag2.toLatin1().data());
      }
#else
      if (taoflag == 1.0)
      {
         printf("%s\t%.2f\t%s (%s)\n", " Tau(1) ", (double)(tao[0] * pow(fconv, 3.0f)), "[ns] ", tag2.toLatin1().data());
         printf("%s\t%.2f\t%s (%s)\n", " Tau(2) ", (double)(tao[1] * pow(fconv, 3.0f)), "[ns] ", tag2.toLatin1().data());
         printf("%s\t%.2f\t%s (%s)\n", " Tau(3) ", (double)(tao[1] * pow(fconv, 3.0f)), "[ns] ", tag2.toLatin1().data());
         printf("%s\t%.2f\t%s (%s)\n", " Tau(4) ", (double)(tao[3] * pow(fconv, 3.0f)), "[ns] ", tag2.toLatin1().data());
         printf("%s\t%.2f\t%s (%s)\n", " Tau(5) ", (double)(tao[3] * pow(fconv, 3.0f)), "[ns] ", tag2.toLatin1().data());
      }
      if (taoflag == 2.0)
      {
         printf("%s\t%.2f\t%s (%s)\n", " Tau(1) ", (double)(tao[4] * pow(fconv, 3.0f)), "[ns] ", tag2.toLatin1().data());
         printf("%s\t%.2f\t%s (%s)\n", " Tau(2) ", (double)(tao[1] * pow(fconv, 3.0f)), "[ns] ", tag2.toLatin1().data());
         printf("%s\t%.2f\t%s (%s)\n", " Tau(3) ", (double)(tao[1] * pow(fconv, 3.0f)), "[ns] ", tag2.toLatin1().data());
         printf("%s\t%.2f\t%s (%s)\n", " Tau(4) ", (double)(tao[3] * pow(fconv, 3.0f)), "[ns] ", tag2.toLatin1().data());
         printf("%s\t%.2f\t%s (%s)\n", " Tau(5) ", (double)(tao[3] * pow(fconv, 3.0f)), "[ns] ", tag2.toLatin1().data());
      }
      if (taoflag == 0.0)
      {
         printf("%s\t%.2f\t%s (%s)\n", " Tau(1) ", (double)(tao[0] * pow(fconv, 3.0f)), "[ns] ", tag2.toLatin1().data());
         printf("%s\t%.2f\t%s (%s)\n", " Tau(2) ", (double)(tao[1] * pow(fconv, 3.0f)), "[ns] ", tag2.toLatin1().data());
         printf("%s\t%.2f\t%s (%s)\n", " Tau(3) ", (double)(tao[2] * pow(fconv, 3.0f)), "[ns] ", tag2.toLatin1().data());
         printf("%s\t%.2f\t%s (%s)\n", " Tau(4) ", (double)(tao[3] * pow(fconv, 3.0f)), "[ns] ", tag2.toLatin1().data());
         printf("%s\t%.2f\t%s (%s)\n", " Tau(5) ", (double)(tao[4] * pow(fconv, 3.0f)), "[ns] ", tag2.toLatin1().data());
      }
#endif

      printf("\n%s\t%.2f\t%s (%s)\n", " Tau(m) ", taom * pow(fconv, 3.0f), "[ns] ", tag2.toLatin1().data());
      printf("%s\t%.2f\t%s (%s)\n\n", " Tau(h) ", taoh * 1.0E+09 * pow(fconv, 3.0f), "[ns] ", tag2.toLatin1().data());
      supc_results->tau = taoh * 1.0E+09 * pow(fconv, 3.0f);
   }

   printf("\n%s", "- MAX EXTENSIONS:");
   printf("\n%s%.2f%s%s%.2f%s%s%.2f%s\n", "[X axis] = ", (maxx * fconv), " [nm];  ", "[Y axis] = ", (maxy * fconv), " [nm];  ",
          "[Z axis] = ", (maxz * fconv), " [nm]");
   printf("%s%.1f%s%.1f%s%.1f%s\n\n", "- AXIAL RATIOS : [X:Z] = ", (maxx / maxz), "; [X:Y] = ", (maxx / maxy), "; [Y:Z] = ",
          (maxy / maxz), "");

   //    system("sleep 3");

}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
   
static void
mem_ris(int model)
{

   float einst, bc, temp;
   int i;

   if (cc == 1)
      bc = 6.0;
   else
      bc = 4.0;

   save_data this_data = US_Hydrodyn_Save::save_data_initialized();
   // may have to zero this
   this_data.hydro = us_hydrodyn->hydro;
   this_data.model_idx = QString("%1").arg(model);
   this_data.results.num_models = 1;

   this_data.results.asa_rg_pos = (*model_vector)[ model_idx[ model ] ].asa_rg_pos;
   this_data.results.asa_rg_neg = (*model_vector)[ model_idx[ model ] ].asa_rg_neg;
   // qDebug() << "us_hydrodyn_supc::mem_ris(" << model << ") asa_rg +/- " << this_data.results.asa_rg_pos << " " << this_data.results.asa_rg_neg;
   this_data.hydro_res = "";
   this_data.proc_time = (double)(supc_timers.times[ "compute smi" ]) / 1e3;

   this_data.fractal_dimension_parameters          = (*model_vector)[ model_idx[ model ] ].fractal_dimension_parameters;
   this_data.fractal_dimension                     = (*model_vector)[ model_idx[ model ] ].fractal_dimension;
   this_data.fractal_dimension_sd                  = (*model_vector)[ model_idx[ model ] ].fractal_dimension_sd;
   this_data.fractal_dimension_wtd                 = (*model_vector)[ model_idx[ model ] ].fractal_dimension_wtd;
   this_data.fractal_dimension_wtd_sd              = (*model_vector)[ model_idx[ model ] ].fractal_dimension_wtd_sd;
   this_data.fractal_dimension_wtd_wtd             = (*model_vector)[ model_idx[ model ] ].fractal_dimension_wtd_wtd;
   this_data.fractal_dimension_wtd_wtd_sd          = (*model_vector)[ model_idx[ model ] ].fractal_dimension_wtd_wtd_sd;
   this_data.rg_over_fractal_dimension             = (*model_vector)[ model_idx[ model ] ].rg_over_fractal_dimension;
   this_data.rg_over_fractal_dimension_sd          = (*model_vector)[ model_idx[ model ] ].rg_over_fractal_dimension_sd;
   this_data.rg_over_fractal_dimension_wtd         = (*model_vector)[ model_idx[ model ] ].rg_over_fractal_dimension_wtd;
   this_data.rg_over_fractal_dimension_wtd_sd      = (*model_vector)[ model_idx[ model ] ].rg_over_fractal_dimension_wtd_sd;
   this_data.rg_over_fractal_dimension_wtd_wtd     = (*model_vector)[ model_idx[ model ] ].rg_over_fractal_dimension_wtd_wtd;
   this_data.rg_over_fractal_dimension_wtd_wtd_sd  = (*model_vector)[ model_idx[ model ] ].rg_over_fractal_dimension_wtd_wtd_sd;

   QString hydro_res;
   QString hydro_format_string;

   bool create_hydro_res = !(us_hydrodyn->batch_widget &&
                             us_hydrodyn->batch_window->save_batch_active);

   // printf("create hydrores %s\n", create_hydro_res ? "true" : "false");

   if ( create_hydro_res ) {
      ris = us_fopen(qs_risultati, "ab");
      us_hydrodyn->last_hydro_res = QString("%1").arg( qs_risultati );
      cout << "last_hydro_res " << us_hydrodyn->last_hydro_res << endl;
   }

   hydro_res.sprintf("%s", "MODEL File Name  :___ ");
   create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
   this_data.hydro_res += hydro_res;

   strncpy(molecola_nb, molecola, SMAX);
   molecola_nb[SMAX-1] = 0;
   if(strlen(molecola_nb) > 7) {
      molecola_nb[strlen(molecola_nb) - 6] = 0; // remove .beams
   }
   this_data.results.name = QString("%1").arg(molecola_nb);

   hydro_res.sprintf("%s\n", molecola_nb);
   create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
   this_data.hydro_res += hydro_res;

   hydro_res.sprintf("%s%d\n", "TOTAL Beads in the MODEL :___ ", numero_sfere);
   create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
   this_data.hydro_res += hydro_res;
   this_data.results.total_beads = numero_sfere;

   // hydro_res.sprintf("%s%.2f [nm^2]\n", "TOTAL ASA of Beads in the MODEL :___ ", total_asa[model]);
   // create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
   // this_data.hydro_res += hydro_res;

   hydro_res.sprintf("%s%.2f [nm^2]\n", "TOTAL Surface Area of Beads in the MODEL :___ ", total_s_a[model]);
   create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
   this_data.hydro_res += hydro_res;
   this_data.tot_surf_area = total_s_a[model];

   hydro_res.sprintf("%s%.2f [nm^3]\n", "TOTAL Volume of Beads in the MODEL :___ ", total_vol[model]);
   create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
   this_data.hydro_res += hydro_res;
   this_data.tot_volume_of = total_vol[model];

   tot_tot_beads += (float) numero_sfere;
   tot_tot_beads2 += (float) numero_sfere * (float) numero_sfere;
   supc_results->total_beads = numero_sfere;
   supc_results->vbar = partvol;
   this_data.results.vbar = partvol;
    
   // hydro_res.sprintf("%s%d\n", "FIRST Bead Included  :___ ", prima);
   // create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
   // this_data.hydro_res += hydro_res;

   // hydro_res.sprintf("%s%d\n\n", "LAST Bead Included :___ ", ultima);
   // create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
   // this_data.hydro_res += hydro_res;

   hydro_res.sprintf("\n");
   create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
   this_data.hydro_res += hydro_res;

   // solvent area
   hydro_res.sprintf("Solvent:                %s\n", supc_results->solvent_name.toLatin1().data());
   create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
   this_data.hydro_res += hydro_res;

   hydro_res.sprintf("Temperature:            %5.2f\n", supc_results->temperature);
   create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
   this_data.hydro_res += hydro_res;

   // pH
   hydro_res.sprintf("pH                      %.2f\n", this_data.hydro.pH );
   create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
   this_data.hydro_res += hydro_res;

   hydro_res.sprintf("Solvent viscosity (cP): %f\n", supc_results->solvent_viscosity);
   create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
   this_data.hydro_res += hydro_res;

   hydro_res.sprintf("Solvent density (g/ml): %f\n", supc_results->solvent_density);
   create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
   this_data.hydro_res += hydro_res;

   hydro_res.sprintf("\n");
   create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
   this_data.hydro_res += hydro_res;


   // temporary vbar area
   hydro_res.sprintf("Original vbar:          %5.3f %s\t[cm^3/g]\n", 
                     org_vbar, 
                     us_hydrodyn->misc.compute_vbar ?
                     ( us_hydrodyn->bead_model_from_file ?
                       "(from file)" : "(computed) " ) : "user entered");
   create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
   this_data.hydro_res += hydro_res;

   if ( !us_hydrodyn->misc.compute_vbar )
   {
      hydro_res.sprintf("Original vbar temp (C): %5.2f\n", us_hydrodyn->misc.vbar_temperature);
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
   }
   hydro_res.sprintf("Temp corrected vbar:    %5.3f            \t[cm^3/g]\n", tc_vbar);
   create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
   this_data.hydro_res += hydro_res;
   
   hydro_res.sprintf("\n");
   create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
   this_data.hydro_res += hydro_res;
   
   if (colorzero == 1)
   {
      hydro_res.sprintf("%s%d%s\n", "- WARNING: THERE IS ", colorzero, " BEAD COLOR CODED '0' [RADIUS < 0.005 NM]");
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
      
      hydro_res.sprintf("%s\n", "- THIS BEAD IS NOT USED IN THE HYDRODYNAMIC COMPUTATIONS\n");
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
   }
   if (colorzero > 1)
   {
      hydro_res.sprintf("%s%d%s\n", "- WARNING: THERE ARE ", colorzero, " BEADS COLOR CODED '0' [RADIUS < 0.005 NM]");
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
      
      hydro_res.sprintf("%s\n", "- THOSE BEADS ARE NOT USED IN THE HYDRODYNAMIC COMPUTATIONS\n");
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
   }
   this_data.num_of_unused = this_data.results.total_beads - this_data.results.used_beads;

   if (flag_norm == 1)
   {
      hydro_res.sprintf("%s\n", "- 'NORMALIZED' model -");
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
   }

   if (cc == 1)
   {
      hydro_res.sprintf("- STICK BOUNDARY CONDITIONS (6*PI*ETAo)\n\n");
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
   }
   else
   {
      hydro_res.sprintf("- SLIP BOUNDARY CONDITIONS (4*PI*ETAo)\n\n");
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
   }

   hydro_res.sprintf("%s%d\n", "- Used BEADS Number       = ", nat);
   create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
   this_data.hydro_res += hydro_res;
   
   this_data.results.used_beads = nat;
   if (volcor == 1)
   {
      if ((colorsixf == 0) && (sfecalc == 2))
      {
         hydro_res.sprintf("%s%.2f%s\n", "- Used BEADS Volume       = ", volcor1 * pow(fconv, 3.0f),
                 "  [nm^3] (NO contribution from buried beads)");
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;         
      }
      if ((colorsixf == 1) && (sfecalc == 2))
      {
         hydro_res.sprintf("%s%.2f%s\n", "- Used BEADS Volume       = ", volcor1 * pow(fconv, 3.0f),
                 "  [nm^3] (contribution from buried beads only for Dr)");
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
         
         hydro_res.sprintf("%s%.2f%s\n", "- Used BEADS Volume       = ", totvol * pow(fconv, 3.0f), "  [nm^3] (for [n])");
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
      }
      if ((colorsixf == 2) && (sfecalc == 2))
      {
         hydro_res.sprintf("%s%.2f%s\n", "- Used BEADS Volume       = ", volcor1 * pow(fconv, 3.0f),
                 "  [nm^3] (contribution from buried beads only for [n])");
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
         
         hydro_res.sprintf("%s%.2f%s\n", "- Used BEADS Volume       = ", totvol * pow(fconv, 3.0f), "  [nm^3] (for Dr)");
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
      }
      if ((colorsixf == 3) && (sfecalc == 2)) 
      {
         hydro_res.sprintf("%s%.2f%s\n", "- Used BEADS Volume       = ", volcor1 * pow(fconv, 3.0f), "  [nm^3]");
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
      }
      if (sfecalc == 1)
      {
         hydro_res.sprintf("%s%.2f%s\n", "- Used BEADS Volume       = ", volcor1 * pow(fconv, 3.0f), "  [nm^3]");
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;         
      }
      this_data.use_beads_vol =  (double) volcor1 * pow(fconv, 3.0f);
   }
   if (volcor == 2)
   {
      hydro_res.sprintf("%s%.2f%s\n", "- Used BEADS Volume       = ", volcor1 * pow(fconv, 3.0f), "  [nm^3]");
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
      this_data.use_beads_vol =  (double) volcor1 * pow(fconv, 3.0f);
   }

   // hydro_res.sprintf("%s%.2f [nm^2]\n", "- Used BEADS ASA          = ", used_asa[model]);
   // create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
   // this_data.hydro_res += hydro_res;

   hydro_res.sprintf("%s%.2f [nm^2]\n", "- Used BEADS Surface Area = ", used_s_a[model]);
   create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
   this_data.hydro_res += hydro_res;
   this_data.use_beads_surf = used_s_a[model];

   // hydro_res.sprintf("%s%.2f [nm^3]\n", "- Used BEADS Volume       = ", used_vol[model]);
   // create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
   // this_data.hydro_res += hydro_res;   

   if (mascor == 1)
      mascor1 = (float) pesmol;

   hydro_res.sprintf("%s%.2f%s\n", "- Used BEAD Mass     = ", mascor1, "  [Da]");
   create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
   this_data.hydro_res += hydro_res;
   this_data.use_bead_mass = mascor1;

   hydro_res.sprintf("%s%.2f\n", "- Conversion Factor  = ", fconv);
   create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
   this_data.hydro_res += hydro_res;
   this_data.con_factor = fconv;

   hydro_res.sprintf("\n%s%.3e\t%s (%s)\n", "- TRANS. FRICT. COEFF.  = ", f * 1.0E-07 * fconv, "[g/s] ", tag1.toLatin1().data());
   create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
   this_data.hydro_res += hydro_res;
   this_data.tra_fric_coef = f * 1.0E-07 * fconv;

   hydro_res.sprintf("%s%.2e\t%s (%s)\n", "- TRANS. DIFF. COEFF.   = ", (KB * TE * 1.0E7) / f * fconv1, "[cm^2/s] ", tag2.toLatin1().data());
   create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
   this_data.hydro_res += hydro_res;
   this_data.results.D20w = (KB * TE * 1.0E7) / f * fconv1;

   if (raflag == -1.0)
   {
      hydro_res.sprintf("%s%.3g\t%s (%s)\n", "- SED. COEFF. (psv from unhydrated radii) = ",
              (mascor1 * 1.0E20 * (1.0 - partvolc * DENS)) / (f * fconv * AVO), "[S] ", tag2.toLatin1().data());
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
      this_data.results.s20w = (mascor1 * 1.0E20 * (1.0 - partvolc * DENS)) / (f * fconv * AVO);
   }
      

   if ((raflag == -2.0) || (raflag == -5.0))
   {
      hydro_res.sprintf("%s%.3g\t\t%s (%s)\n", "- SED. COEFF.           = ",
              (mascor1 * 1.0E20 * (1.0 - partvol * DENS)) / (f * fconv * AVO), "[S] ", tag2.toLatin1().data());
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
      this_data.results.s20w = (mascor1 * 1.0E20 * (1.0 - partvol * DENS)) / (f * fconv * AVO);
      if ((nat + colorzero + colorsix) < numero_sfere)
      {
         hydro_res.sprintf(                 "- !!WARNING: ONLY PART OF THE MODEL HAS BEEN ANALYZED, BUT THE PSV UTILIZED         IS THAT OF THE ENTIRE MODEL!! - \n");
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
      }
   }

   if (raflag == -3.0)
   {
      hydro_res.sprintf("- SED. COEFF. (psv %s) = %.3g\t%s (%s)\n", 
              us_hydrodyn->misc.compute_vbar ?
              ( us_hydrodyn->bead_model_from_file ?
                "from file" : "computed" ) : "user entered",
              (mascor1 * 1.0E20 * (1.0 - partvol * DENS)) / (f * fconv * AVO), "[S] ", tag2.toLatin1().data());
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
      this_data.results.s20w = (mascor1 * 1.0E20 * (1.0 - partvol * DENS)) / (f * fconv * AVO);

      if ((nat + colorzero + colorsix) < numero_sfere)
      {
         hydro_res.sprintf(                 "- !!WARNING: ONLY PART OF THE MODEL HAS BEEN ANALYZED, BUT THE PSV UTILIZED         IS THAT OF THE ENTIRE MODEL!! - \n");
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
      }

      hydro_res.sprintf("%s%.3g\t%s (%s)\n", "- SED. COEFF. (psv from unhydrated radii) = ",
              (mascor1 * 1.0E20 * (1.0 - partvolc * DENS)) / (f * fconv * AVO), "[S] ", tag2.toLatin1().data());
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;      
   }

   // ff0 calc
   hydro_res.sprintf("%s%.2f\n", "- FRICTIONAL RATIO      = ", 
           f * 1.0E-08 / 
           ( 6.0 * M_PI * ETAo *
             pow( 3.0 * supc_results->mass * partvol / (4.0 * M_PI * AVOGADRO), 1.0/3.0)));
   create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
   this_data.hydro_res += hydro_res;

   // ff0 calc
   this_data.results.ff0 = 
      f * 1.0E-08 / 
      ( 6.0 * M_PI * ETAo *
        pow( 3.0 * supc_results->mass * partvol / (4.0 * M_PI * AVOGADRO), 1.0/3.0));

   if ( us_hydrodyn->advanced_config.expert_mode ) {
      temp = 0.0;
      for (i = 0; i < 3; i++)
         temp += 1.0E-21 / Dr[i * 4] * pow(fconv, 3);
      hydro_res.sprintf("%s%.3e\t%s (%s)\n", "- ROT. FRICT. COEFF.    = ", temp / 3.0, "[g*cm^2/s] ", tag1.toLatin1().data());
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
      this_data.rot_fric_coef = temp / 3.0;
      
      temp = 0.0;
      for (i = 0; i < 3; i++)
         temp += (KB * TE * Dr[i * 4] / 1.0E-21) * pow(fconv1, 3);
      hydro_res.sprintf("%s%.0f\t%s (%s)\n\n", "- ROT. DIFF. COEFF.     = ", temp / 3.0, "[1/s] ", tag2.toLatin1().data());
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
      this_data.rot_diff_coef = temp / 3.0;

#if !defined( MINGW )
      hydro_res.sprintf("%s%.3Le\t%s (%s)\n", "- ROT. FRICT. COEFF. [ X ] = ", 1.0E-21 / Dr[0] * pow(fconv, 3), "[g*cm^2/s] ", tag1.toLatin1().data());
#else
      hydro_res.sprintf("%s%.3e\t%s (%s)\n", "- ROT. FRICT. COEFF. [ X ] = ", (double)(1.0E-21 / Dr[0] * pow(fconv, 3)), "[g*cm^2/s] ", tag1.toLatin1().data());
#endif
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
      this_data.rot_fric_coef_x = 1.0E-21 / Dr[0] * pow(fconv, 3);

#if !defined( MINGW )
      hydro_res.sprintf("%s%.3Le\t%s (%s)\n", "- ROT. FRICT. COEFF. [ Y ] = ", 1.0E-21 / Dr[4] * pow(fconv, 3), "[g*cm^2/s] ", tag1.toLatin1().data());
#else
      hydro_res.sprintf("%s%.3e\t%s (%s)\n", "- ROT. FRICT. COEFF. [ Y ] = ", (double)(1.0E-21 / Dr[4] * pow(fconv, 3)), "[g*cm^2/s] ", tag1.toLatin1().data());
#endif
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
      this_data.rot_fric_coef_y = 1.0E-21 / Dr[4] * pow(fconv, 3);

#if !defined( MINGW )
      hydro_res.sprintf("%s%.3Le\t%s (%s)\n", "- ROT. FRICT. COEFF. [ Z ] = ", 1.0E-21 / Dr[8] * pow(fconv, 3), "[g*cm^2/s] ", tag1.toLatin1().data());
#else
      hydro_res.sprintf("%s%.3e\t%s (%s)\n", "- ROT. FRICT. COEFF. [ Z ] = ", (double)(1.0E-21 / Dr[8] * pow(fconv, 3)), "[g*cm^2/s] ", tag1.toLatin1().data());
#endif
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
      this_data.rot_fric_coef_z = 1.0E-21 / Dr[8] * pow(fconv, 3);

#if !defined( MINGW )
      hydro_res.sprintf("%s%.2Lf\t%s (%s)\n", "- ROT. DIFF. COEFF.  [ X ] = ", KB * TE * Dr[0] / 1.0e-21 * pow(fconv1, 3),
                        "[1/s] ", tag2.toLatin1().data());
#else
      hydro_res.sprintf("%s%.2f\t%s (%s)\n", "- ROT. DIFF. COEFF.  [ X ] = ", (double)(KB * TE * Dr[0] / 1.0e-21 * pow(fconv1, 3)),
                        "[1/s] ", tag2.toLatin1().data());
#endif
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
      this_data.rot_diff_coef_x = KB * TE * Dr[0] / 1.0e-21 * pow(fconv1, 3);

#if !defined( MINGW )
      hydro_res.sprintf("%s%.2Lf\t%s (%s)\n", "- ROT. DIFF. COEFF.  [ Y ] = ", KB * TE * Dr[4] / 1.0e-21 * pow(fconv1, 3),
                        "[1/s] ", tag2.toLatin1().data());
#else
      hydro_res.sprintf("%s%.2f\t%s (%s)\n", "- ROT. DIFF. COEFF.  [ Y ] = ", (double)(KB * TE * Dr[4] / 1.0e-21 * pow(fconv1, 3)),
                        "[1/s] ", tag2.toLatin1().data());
#endif
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
      this_data.rot_diff_coef_y = KB * TE * Dr[4] / 1.0e-21 * pow(fconv1, 3);

#if !defined( MINGW )
      hydro_res.sprintf("%s%.2Lf\t%s (%s)\n\n", "- ROT. DIFF. COEFF.  [ Z ] = ", KB * TE * Dr[8] / 1.0e-21 * pow(fconv1, 3),
                        "[1/s] ", tag2.toLatin1().data());
#else
      hydro_res.sprintf("%s%.2f\t%s (%s)\n\n", "- ROT. DIFF. COEFF.  [ Z ] = ", (double)(KB * TE * Dr[8] / 1.0e-21 * pow(fconv1, 3)),
                        "[1/s] ", tag2.toLatin1().data());
#endif
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
      this_data.rot_diff_coef_z = KB * TE * Dr[8] / 1.0e-21 * pow(fconv1, 3);
   }

   hydro_res.sprintf("%s%.2f\t%s\n", "- MOLECULAR WEIGHT (from file)           = ", pesmol, "[Da]");
   create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
   this_data.hydro_res += hydro_res;
   this_data.results.mass = pesmol;

   //   if (sfecalc == 2)
   //   {
   //      hydro_res.sprintf("%s%.2f\t%s\n", "- BEADS TOTAL VOLUME (from file) = ", interm1 / (6.0 * ETAo) * pow(fconv, 3), "[nm^3]");
   //      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
   //      this_data.hydro_res += hydro_res;
   //   }
   

   //   else
   //      hydro_res.sprintf("%s%.2f\t%s\n", "- BEADS TOTAL VOLUME (from file) = ", totvol * pow(fconv, 3), "[nm^3]");
   //   hydro_res.sprintf("%s%.2f\t%s\n", "- BEADS TOTAL SURFACE AREA       = ", totsup * pow(fconv, 2), "[nm^2]");

   if (raflag == -1.0)
   {
      hydro_res.sprintf("%s%.3f\t%s\n", "- PARTIAL SPECIFIC VOLUME (from unhydrated radii) = ", partvolc, "[cm^3/g]");
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;      
   }

   if ((raflag == -2.0) || (raflag == -5.0))
   {
      hydro_res.sprintf("- PARTIAL SPECIFIC VOLUME %s = %.3f\t%s\n", 
              us_hydrodyn->misc.compute_vbar ?
              ( us_hydrodyn->bead_model_from_file ?
                "(from file)   " : "(computed)    " ) : "(user entered)",
              partvol, "[cm^3/g]");
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;      
   }

   if (raflag == -3.0)
   {
      hydro_res.sprintf("- PARTIAL SPECIFIC VOLUME %s          = %.3f\t%s\n",
              us_hydrodyn->misc.compute_vbar ?
              ( us_hydrodyn->bead_model_from_file ?
               "(from file)   " : "(computed)    " ) : "(user entered)", 
              partvol, "[cm^3/g]");
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
      
      hydro_res.sprintf("%s%.3f\t%s\n", "- PARTIAL SPECIFIC VOLUME (from unhydrated radii) = ", partvolc, "[cm^3/g]");
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
   }

   if ((raflag == -1.0) || (raflag == -3.0))
   {
      hydro_res.sprintf("\n%s%.2f\t%s\n", "- RADIUS OF GYRATION (Hydrated Beads)  = ", ro * fconv, "[nm]");
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
      
      this_data.results.rg = (double) ro * fconv;
      hydro_res.sprintf("%s%.2f\t%s\n", "- RADIUS OF GYRATION (Unydrated Beads) = ", (double) rou * fconv, "[nm]");
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
   }
   else
   {
      hydro_res.sprintf("\n%s%.2f\t%s\n", "- RADIUS OF GYRATION              = ", ro * fconv, "[nm]");
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
      this_data.results.rg = (double) ro * fconv;
   }
   hydro_res.sprintf("%s%.2f\t%s\n", "- TRANSLATIONAL STOKES' RADIUS    = ", f * fconv / (bc * M_PI * ETAo), "[nm]");
   create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
   this_data.hydro_res += hydro_res;
   this_data.results.rs =  f * fconv / (bc * M_PI * ETAo);


   if ( us_hydrodyn->advanced_config.expert_mode ) {
      hydro_res.sprintf("%s%.2f\t%s\n", "- ROTATIONAL STOKES' RADIUS [ X ] = ",
                        (double)( pow((3.0 / Dr[0] / bc / 4.0 / M_PI / ETAo), (long double)(0.33333)) * fconv ), "[nm]");
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
      this_data.rot_stokes_rad_x = pow((3.0 / Dr[0] / bc / 4.0 / M_PI / ETAo), (long double)(0.33333)) * fconv;

      hydro_res.sprintf("%s%.2f\t%s\n", "- ROTATIONAL STOKES' RADIUS [ Y ] = ",
                        ( double )( pow((3.0 / Dr[4] / bc / 4.0 / M_PI / ETAo), (long double)(0.33333)) * fconv ), "[nm]");
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
      this_data.rot_stokes_rad_y = pow((3.0 / Dr[4] / bc / 4.0 / M_PI / ETAo), (long double)(0.33333)) * fconv;

      hydro_res.sprintf("%s%.2f\t%s\n\n", "- ROTATIONAL STOKES' RADIUS [ Z ] = ",
                        ( double )( pow((3.0 / Dr[8] / bc / 4.0 / M_PI / ETAo),(long double) (0.33333)) * fconv ), "[nm]");
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
      this_data.rot_stokes_rad_z = pow((3.0 / Dr[8] / bc / 4.0 / M_PI / ETAo), (long double)(0.33333)) * fconv;
   }

   hydro_res.sprintf("%s%5.2f\t%5.2f\t%5.2f\t%s\n", "- CENTRE OF RESISTANCE   :  ", roR[0] * fconv, roR[1] * fconv, roR[2] * fconv,
           "[nm]");
   create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
   this_data.hydro_res += hydro_res;
   this_data.cen_of_res_x = (double) roR[0] * fconv;
   this_data.cen_of_res_y = (double) roR[1] * fconv;
   this_data.cen_of_res_z = (double) roR[2] * fconv;

   hydro_res.sprintf("%s%5.2f\t%5.2f\t%5.2f\t%s\n", "- CENTRE OF MASS         :  ", xm * fconv, ym * fconv, zm * fconv, "[nm]");
   create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
   this_data.hydro_res += hydro_res;
   this_data.cen_of_mass_x = (double) xm * fconv;
   this_data.cen_of_mass_y = (double) ym * fconv;
   this_data.cen_of_mass_z = (double) zm * fconv;

   if (cd == 2)
   {
      hydro_res.sprintf("%s%5.2f\t%5.2f\t%5.2f\t%s\n", "- CENTRE OF DIFFUSION    :  ", roD[0] * fconv, roD[1] * fconv,
              roD[2] * fconv, "[nm]");
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
      this_data.cen_of_diff_x = (double) roD[0] * fconv;
      this_data.cen_of_diff_y = (double) roD[1] * fconv;
      this_data.cen_of_diff_z = (double) roD[2] * fconv;
   }

   hydro_res.sprintf("%s%5.2f\t%5.2f\t%5.2f\t%s\n\n", "- CENTRE OF VISCOSITY    :  ", vc[0] * fconv, vc[1] * fconv, vc[2] * fconv,
           "[nm]");
   create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
   this_data.hydro_res += hydro_res;
   this_data.cen_of_visc_x = (double) vc[0] * fconv;
   this_data.cen_of_visc_y = (double) vc[1] * fconv;
   this_data.cen_of_visc_z = (double) vc[2] * fconv;

   if (mascor == 2)
   {
      correz = pesmol / mascor1;
      vol_mas = mascor1;
   }
   else
   {
      correz = 1.0;
      vol_mas = pesmol;
   }

   if ( us_hydrodyn->advanced_config.expert_mode ) {
      // hydro_res.sprintf("%s%.2f\t%s\n", "- INTRINSIC VISCOSITY                  = ", vis * correz * pow(fconv, 3.0f), "[cm^3/g]");
      //   create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      // this_data.hydro_res += hydro_res;

      hydro_res.sprintf("%s%.2f\t%s\n", "- UNCORRECTED INTRINSIC VISCOSITY      = ", vis * correz * pow(fconv, 3.0f), "[cm^3/g]");
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
      this_data.unc_int_visc = (double) vis * correz * pow(fconv, 3.0f);

      einst = pow(0.3 * pesmol * vis / ( M_PI * AVO), 0.33333);
      einst = 1E7 * einst;
      // hydro_res.sprintf("%s%.2f\t%s\n", "- EINSTEIN'S RADIUS                    = ", einst * fconv, "[nm]");
      // create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      // this_data.hydro_res += hydro_res;   

      hydro_res.sprintf("%s%.2f\t%s\n", "- UNCORRECTED EINSTEIN'S RADIUS        = ", einst * fconv, "[nm]");
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
      this_data.unc_einst_rad = (double) einst * fconv;

      if ((volcor == 1) && ((colorsixf == 0) || (colorsixf == 1) || (colorsixf == 2)))
      {
         if ( hydro_use_avg_for_volume ) {
            double use_vol = asa_vol[ active_model ];
            hydro_res.sprintf("%s%.2f\t%s\n", "- CORRECTED INTRINSIC VISCOSITY        = ",
                              ((double)vis * correz + vis3 * use_vol / vol_mas) * pow(fconv, 3), "[cm^3/g]");
            create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
            this_data.hydro_res += hydro_res;
            this_data.results.viscosity =  ((double) vis * correz + vis3 * use_vol / vol_mas) * pow(fconv, 3);
            this_data.cor_int_visc = ((double) vis * correz + vis3 * use_vol / vol_mas) * pow(fconv, 3);

            einst = pow(0.3 * vol_mas * ((double) vis * correz + vis3 * use_vol / vol_mas) / ( M_PI * AVO), 0.33333);
            einst = 1E7 * einst;
            // hydro_res.sprintf("%s%.2f\t%s\n", "- EINSTEIN'S RADIUS (GDLT corrected)   = ", einst * fconv, "[nm]");
            // create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
            // this_data.hydro_res += hydro_res;
      
            hydro_res.sprintf("%s%.2f\t%s\n", "- CORRECTED EINSTEIN'S RADIUS          = ", einst * fconv, "[nm]");
            create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
            this_data.hydro_res += hydro_res;
            this_data.cor_einst_rad = (double) einst * fconv;
         } else {
            // hydro_res.sprintf("%s%.2f\t%s\n", "- INTRINSIC VISCOSITY (GDLT corrected) = ",
            hydro_res.sprintf("%s%.2f\t%s\n", "- CORRECTED INTRINSIC VISCOSITY        = ",
                              (vis * correz + vis3 * totvol / vol_mas) * pow(fconv, 3), "[cm^3/g]");
            create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
            this_data.hydro_res += hydro_res;
            this_data.results.viscosity =  (vis * correz + vis3 * totvol / vol_mas) * pow(fconv, 3);
            this_data.cor_int_visc = (vis * correz + vis3 * totvol / vol_mas) * pow(fconv, 3);

            einst = pow(0.3 * vol_mas * (vis * correz + vis3 * totvol / vol_mas) / ( M_PI * AVO), 0.33333);
            einst = 1E7 * einst;
            // hydro_res.sprintf("%s%.2f\t%s\n", "- EINSTEIN'S RADIUS (GDLT corrected)   = ", einst * fconv, "[nm]");
            // create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
            // this_data.hydro_res += hydro_res;
      
            hydro_res.sprintf("%s%.2f\t%s\n", "- CORRECTED EINSTEIN'S RADIUS          = ", einst * fconv, "[nm]");
            create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
            this_data.hydro_res += hydro_res;
            this_data.cor_einst_rad = (double) einst * fconv;
         }
      }
      else
      {
         // hydro_res.sprintf("%s%.2f\t%s\n", "- INTRINSIC VISCOSITY (GDLT corrected)  = ",
         hydro_res.sprintf("%s%.2f\t%s\n", "- CORRECTED INTRINSIC VISCOSITY         = ",
                           (vis * correz + vis3 * volcor1 / vol_mas) * pow(fconv, 3), "[cm^3/g]");
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
         this_data.results.viscosity = (vis * correz + vis3 * volcor1 / vol_mas) * pow(fconv, 3);
         this_data.cor_int_visc = (vis * correz + vis3 * volcor1 / vol_mas) * pow(fconv, 3);

         einst = pow(0.3 * vol_mas * (vis * correz + vis3 * volcor1 / vol_mas) / ( M_PI * AVO), 0.33333);
         einst = 1E7 * einst;
         // hydro_res.sprintf("%s%.2f\t%s\n", "- EINSTEIN'S RADIUS (GDLT corrected)   = ", einst * fconv, "[nm]");
         // create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         // this_data.hydro_res += hydro_res;
      
         hydro_res.sprintf("%s%.2f\t%s\n", "- CORRECTED EINSTEIN'S RADIUS          = ", einst * fconv, "[nm]");
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
         this_data.cor_einst_rad = (double) einst * fconv;
      }

#if defined(TSUDA_DOUBLESUM)
      hydro_res.sprintf("%s%.2f\t%s\n", "- INTRINSIC VISCOSITY(DoubleSum CM)    = ", vis4 * correz * pow(fconv, 3), "[cm^3/g]");
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
   
      einst = pow(0.3 * pesmol * vis4 / ( M_PI * AVO), 0.33333);
      einst = 1E7 * einst;
      hydro_res.sprintf("%s%.2f\t%s\n", "- EINSTEIN'S RADIUS (DoubleSum CM)     = ", einst * fconv, "[nm]");
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
   
      hydro_res.sprintf("%s%.2f\t%s\n", "- INTRINSIC VISCOSITY(Tsuda CM)        = ", vis1 * correz * pow(fconv, 3), "[cm^3/g]");
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
   
      einst = pow(0.3 * pesmol * vis1 / ( M_PI * AVO), 0.33333);
      einst = 1E7 * einst;
      hydro_res.sprintf("%s%.2f\t%s\n", "- EINSTEIN'S RADIUS (Tsuda CM)         = ", einst * fconv, "[nm]");
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
   
      hydro_res.sprintf("%s%.2f\t%s\n", "- INTRINSIC VISCOSITY(Tsuda CV)        = ", vis2 * correz * pow(fconv, 3), "[cm^3/g]");
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
   
      einst = pow(0.3 * pesmol * vis2 / ( M_PI * AVO), 0.33333);
      einst = 1E7 * einst;
      hydro_res.sprintf("%s%.2f\t%s\n", "- EINSTEIN'S RADIUS (Tsuda CV)         = ", einst * fconv, "[nm]");
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
#endif

      hydro_res.sprintf("\nRELAXATION TIMES\n\n");
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;   

#if !defined( MINGW )
      if (taoflag == 1.0)
      {
         hydro_res.sprintf("%s\t%.2Lf\t%s (%s)\n", " Tau(1) ", tao[0] * pow(fconv, 3.0f), "[ns] ", tag2.toLatin1().data());
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
         this_data.rel_times_tau_1 = tao[0] * pow(fconv, 3.0f);

         hydro_res.sprintf("%s\t%.2Lf\t%s (%s)\n", " Tau(2) ", tao[1] * pow(fconv, 3.0f), "[ns] ", tag2.toLatin1().data());
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
         this_data.rel_times_tau_2 = tao[1] * pow(fconv, 3.0f);

         hydro_res.sprintf("%s\t%.2Lf\t%s (%s)\n", " Tau(3) ", tao[1] * pow(fconv, 3.0f), "[ns] ", tag2.toLatin1().data());
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
         this_data.rel_times_tau_3 = tao[1] * pow(fconv, 3.0f);

         hydro_res.sprintf("%s\t%.2Lf\t%s (%s)\n", " Tau(4) ", tao[3] * pow(fconv, 3.0f), "[ns] ", tag2.toLatin1().data());
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
         this_data.rel_times_tau_4 = tao[3] * pow(fconv, 3.0f);

         hydro_res.sprintf("%s\t%.2Lf\t%s (%s)\n", " Tau(5) ", tao[3] * pow(fconv, 3.0f), "[ns] ", tag2.toLatin1().data());
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
         this_data.rel_times_tau_5 = tao[3] * pow(fconv, 3.0f);
      }
      if (taoflag == 2.0)
      {
         hydro_res.sprintf("%s\t%.2Lf\t%s (%s)\n", " Tau(1) ", tao[4] * pow(fconv, 3.0f), "[ns] ", tag2.toLatin1().data());
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
         this_data.rel_times_tau_1 = tao[4] * pow(fconv, 3.0f);

         hydro_res.sprintf("%s\t%.2Lf\t%s (%s)\n", " Tau(2) ", tao[1] * pow(fconv, 3.0f), "[ns] ", tag2.toLatin1().data());
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
         this_data.rel_times_tau_2 = tao[1] * pow(fconv, 3.0f);

         hydro_res.sprintf("%s\t%.2Lf\t%s (%s)\n", " Tau(3) ", tao[1] * pow(fconv, 3.0f), "[ns] ", tag2.toLatin1().data());
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
         this_data.rel_times_tau_3 = tao[1] * pow(fconv, 3.0f);

         hydro_res.sprintf("%s\t%.2Lf\t%s (%s)\n", " Tau(4) ", tao[3] * pow(fconv, 3.0f), "[ns] ", tag2.toLatin1().data());
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
         this_data.rel_times_tau_4 = tao[3] * pow(fconv, 3.0f);

         hydro_res.sprintf("%s\t%.2Lf\t%s (%s)\n", " Tau(5) ", tao[3] * pow(fconv, 3.0f), "[ns] ", tag2.toLatin1().data());
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
         this_data.rel_times_tau_5 = tao[3] * pow(fconv, 3.0f);
      }
      if (taoflag == 0.0)
      {
         hydro_res.sprintf("%s\t%.2Lf\t%s (%s)\n", " Tau(1) ", tao[0] * pow(fconv, 3.0f), "[ns] ", tag2.toLatin1().data());
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
         this_data.rel_times_tau_1 = tao[0] * pow(fconv, 3.0f);

         hydro_res.sprintf("%s\t%.2Lf\t%s (%s)\n", " Tau(2) ", tao[1] * pow(fconv, 3.0f), "[ns] ", tag2.toLatin1().data());
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
         this_data.rel_times_tau_2 = tao[1] * pow(fconv, 3.0f);

         hydro_res.sprintf("%s\t%.2Lf\t%s (%s)\n", " Tau(3) ", tao[2] * pow(fconv, 3.0f), "[ns] ", tag2.toLatin1().data());
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
         this_data.rel_times_tau_3 = tao[2] * pow(fconv, 3.0f);

         hydro_res.sprintf("%s\t%.2Lf\t%s (%s)\n", " Tau(4) ", tao[3] * pow(fconv, 3.0f), "[ns] ", tag2.toLatin1().data());
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
         this_data.rel_times_tau_4 = tao[3] * pow(fconv, 3.0f);

         hydro_res.sprintf("%s\t%.2Lf\t%s (%s)\n", " Tau(5) ", tao[4] * pow(fconv, 3.0f), "[ns] ", tag2.toLatin1().data());
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
         this_data.rel_times_tau_5 = tao[4] * pow(fconv, 3.0f);
      }
#else
      if (taoflag == 1.0)
      {
         hydro_res.sprintf("%s\t%.2f\t%s (%s)\n", " Tau(1) ", (double)(tao[0] * pow(fconv, 3.0f)), "[ns] ", tag2.toLatin1().data());
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
         this_data.rel_times_tau_1 = tao[0] * pow(fconv, 3.0f);

         hydro_res.sprintf("%s\t%.2f\t%s (%s)\n", " Tau(2) ", (double)(tao[1] * pow(fconv, 3.0f)), "[ns] ", tag2.toLatin1().data());
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
         this_data.rel_times_tau_2 = tao[1] * pow(fconv, 3.0f);

         hydro_res.sprintf("%s\t%.2f\t%s (%s)\n", " Tau(3) ", (double)(tao[1] * pow(fconv, 3.0f)), "[ns] ", tag2.toLatin1().data());
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
         this_data.rel_times_tau_3 = tao[1] * pow(fconv, 3.0f);

         hydro_res.sprintf("%s\t%.2f\t%s (%s)\n", " Tau(4) ", (double)(tao[3] * pow(fconv, 3.0f)), "[ns] ", tag2.toLatin1().data());
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
         this_data.rel_times_tau_4 = tao[3] * pow(fconv, 3.0f);

         hydro_res.sprintf("%s\t%.2f\t%s (%s)\n", " Tau(5) ", (double)(tao[3] * pow(fconv, 3.0f)), "[ns] ", tag2.toLatin1().data());
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
         this_data.rel_times_tau_5 = tao[3] * pow(fconv, 3.0f);
      }
      if (taoflag == 2.0)
      {
         hydro_res.sprintf("%s\t%.2f\t%s (%s)\n", " Tau(1) ", (double)(tao[4] * pow(fconv, 3.0f)), "[ns] ", tag2.toLatin1().data());
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
         this_data.rel_times_tau_1 = tao[4] * pow(fconv, 3.0f);

         hydro_res.sprintf("%s\t%.2f\t%s (%s)\n", " Tau(2) ", (double)(tao[1] * pow(fconv, 3.0f)), "[ns] ", tag2.toLatin1().data());
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
         this_data.rel_times_tau_2 = tao[1] * pow(fconv, 3.0f);

         hydro_res.sprintf("%s\t%.2f\t%s (%s)\n", " Tau(3) ", (double)(tao[1] * pow(fconv, 3.0f)), "[ns] ", tag2.toLatin1().data());
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
         this_data.rel_times_tau_3 = tao[1] * pow(fconv, 3.0f);

         hydro_res.sprintf("%s\t%.2f\t%s (%s)\n", " Tau(4) ", (double)(tao[3] * pow(fconv, 3.0f)), "[ns] ", tag2.toLatin1().data());
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
         this_data.rel_times_tau_4 = tao[3] * pow(fconv, 3.0f);

         hydro_res.sprintf("%s\t%.2f\t%s (%s)\n", " Tau(5) ", (double)(tao[3] * pow(fconv, 3.0f)), "[ns] ", tag2.toLatin1().data());
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
         this_data.rel_times_tau_5 = tao[3] * pow(fconv, 3.0f);
      }
      if (taoflag == 0.0)
      {
         hydro_res.sprintf("%s\t%.2f\t%s (%s)\n", " Tau(1) ", (double)(tao[0] * pow(fconv, 3.0f)), "[ns] ", tag2.toLatin1().data());
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
         this_data.rel_times_tau_1 = tao[0] * pow(fconv, 3.0f);

         hydro_res.sprintf("%s\t%.2f\t%s (%s)\n", " Tau(2) ", (double)(tao[1] * pow(fconv, 3.0f)), "[ns] ", tag2.toLatin1().data());
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
         this_data.rel_times_tau_2 = tao[1] * pow(fconv, 3.0f);

         hydro_res.sprintf("%s\t%.2f\t%s (%s)\n", " Tau(3) ", (double)(tao[2] * pow(fconv, 3.0f)), "[ns] ", tag2.toLatin1().data());
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
         this_data.rel_times_tau_3 = tao[2] * pow(fconv, 3.0f);

         hydro_res.sprintf("%s\t%.2f\t%s (%s)\n", " Tau(4) ", (double)(tao[3] * pow(fconv, 3.0f)), "[ns] ", tag2.toLatin1().data());
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
         this_data.rel_times_tau_4 = tao[3] * pow(fconv, 3.0f);

         hydro_res.sprintf("%s\t%.2f\t%s (%s)\n", " Tau(5) ", (double)(tao[4] * pow(fconv, 3.0f)), "[ns] ", tag2.toLatin1().data());
         create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
         this_data.hydro_res += hydro_res;
         this_data.rel_times_tau_5 = tao[4] * pow(fconv, 3.0f);
      }
#endif

      hydro_res.sprintf("\n%s\t%.2f\t%s (%s)\n", " Tau(m) ", taom * pow(fconv, 3.0f), "[ns] ", tag2.toLatin1().data());
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
      this_data.rel_times_tau_m = (double) taom * pow(fconv, 3.0f);

      hydro_res.sprintf("%s\t%.2f\t%s (%s)\n", " Tau(h) ", taoh * 1.0E+09 * pow(fconv, 3.0f), "[ns] ", tag2.toLatin1().data());
      create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
      this_data.hydro_res += hydro_res;
      this_data.results.tau = taoh * 1.0E+09 * pow(fconv, 3.0f);
      this_data.rel_times_tau_h = taoh * 1.0E+09 * pow(fconv, 3.0f);
   }

   hydro_res.sprintf("\n%s", "- MAX EXTENSIONS:");
   create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
   this_data.hydro_res += hydro_res;
   
   hydro_res.sprintf("\n%s%.2f%s%s%.2f%s%s%.2f%s\n", "[X axis] = ", (maxx * fconv), " [nm];  ", "[Y axis] = ", (maxy * fconv),
                     " [nm];  ", "[Z axis] = ", (maxz * fconv), " [nm]");
   create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
   this_data.hydro_res += hydro_res;
   this_data.max_ext_x = (maxx * fconv);
   this_data.max_ext_y = (maxy * fconv);
   this_data.max_ext_z = (maxz * fconv);

   hydro_res.sprintf("%s%.1f%s%.1f%s%.1f%s\n\n", "- AXIAL RATIOS : [X:Z] = ", (maxx / maxz), "; [X:Y] = ", (maxx / maxy),
           "; [Y:Z] = ", (maxy / maxz), "");
   create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
   this_data.hydro_res += hydro_res;
   this_data.axi_ratios_xz = (maxx /maxz);
   this_data.axi_ratios_xy = (maxx /maxy);
   this_data.axi_ratios_yz = (maxy /maxz);

   hydro_res.sprintf("\n********************************************************************************\n");
   create_hydro_res && fprintf(ris, "%s", hydro_res.toLatin1().data());
   this_data.hydro_res += hydro_res;

   create_hydro_res && fclose(ris);
   // add text output also
   if ( us_hydrodyn->batch_widget &&
        us_hydrodyn->batch_window->save_batch_active )
   {
      //      if ( us_hydrodyn->save_params.data_vector.size() &&
      //           ( us_hydrodyn->save_params.raflag != raflag ||
      //             us_hydrodyn->save_params.taoflag != taoflag ) )
      //      {
      //         printf("WARNING: ******* differing raflag (%f != %f) or taofalg (%f != %f)\n",
      //                us_hydrodyn->save_params.raflag, raflag,
      //                us_hydrodyn->save_params.taoflag, taoflag);
      //      } else {
      //         us_hydrodyn->save_params.raflag = raflag;
      //         us_hydrodyn->save_params.taoflag = taoflag;
      //      }
      us_hydrodyn->save_params.data_vector.push_back(this_data);
      printf("batch save on, push back info into save_params!\n");
   }
   if ( smi_mm ) {
     smi_mm_save_params.data_vector.push_back( this_data );
   }


   if ( us_hydrodyn->saveParams &&
        create_hydro_res && !smi_mm )
   {
      QString fname = this_data.results.name + ".smi.csv";
      if ( !us_hydrodyn->overwrite_hydro ) {
         fname = us_hydrodyn->fileNameCheck( fname, 0, us_hydrodyn );
      }
      FILE *of = us_fopen(fname, "wb");
      if ( of )
      {
         fprintf(of, "%s", us_hydrodyn->save_util->header().toLatin1().data());
         fprintf(of, "%s", us_hydrodyn->save_util->dataString(&this_data).toLatin1().data());
         fclose(of);
      }
   }
   // print out results:
   us_hydrodyn->save_util->header();
   us_hydrodyn->save_util->dataString(&this_data);
   // printf("end of mem_ris\n");
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

static void
val_med()
{

   float temp;

   QString out_name = smi_mm_name + ".hydro_res";
   if ( !us_hydrodyn->overwrite_hydro ) {
      out_name = us_hydrodyn->fileNameCheck( out_name, 0, us_hydrodyn );
   }

   ris = us_fopen( out_name, "ab" );
   us_hydrodyn->last_hydro_res = QString("%1").arg( out_name );
   cout << "last_hydro_res " << us_hydrodyn->last_hydro_res << endl;

   fprintf(ris, "\n\t AVERAGE PARAMETERS \n");
   fprintf(ris, "\n\t\t\t\t Mean value\tSt. Dev.\n");
   supc_results->total_beads = tot_tot_beads / num;
   supc_results->total_beads_sd = sqrt(fabs((tot_tot_beads2 - tot_tot_beads * tot_tot_beads / num ) / (num - 1.0)));
   supc_results->used_beads = tot_used_beads / num;
   supc_results->used_beads_sd = sqrt(fabs((tot_used_beads2 - tot_used_beads * tot_used_beads / num ) / (num - 1.0)));
   supc_results->vbar = tot_partvol / num;

   temp = fabs((CfT2 - pow(CfT, 2) / num) / (num - 1));
   fprintf(ris, "\n%s\t%.3e\t%.3e\t%s\n", "- TRANS. FRICT. COEFF.        ", CfT / num, sqrt(temp), "[g/s]");

   temp = fabs((CdT2 - pow(CdT, 2) / num) / (num - 1));
   fprintf(ris, "%s\t%.2e\t%.3e\t%s\n", "- TRANS. DIFF. COEFF.         ", CdT / num, sqrt(temp), "[cm^2/s]");
   supc_results->D20w = CdT / num;
   supc_results->D20w_sd = sqrt(temp);

   if (raflag == -1.0)
   {
      temp = fabs((CST2 - pow(CST, 2) / num) / (num - 1));
      fprintf(ris, "%s\t%.3g\t\t%.2f\t\t%s\n", "- SED. COEFF. (psv unhyd.rad.)", CST / num, sqrt(temp), "[S]");
      supc_results->s20w = CST / num;
      supc_results->s20w_sd = sqrt(temp);
   }

   if ((raflag == -2.0) || (raflag == -5.0))
   {
      temp = fabs((CSTF2 - pow(CSTF, 2) / num) / (num - 1));
      fprintf(ris, "- SED. COEFF. (psv %s) \t%.3g\t\t%.3g\t\t%s\n", 
              us_hydrodyn->misc.compute_vbar ?
              ( us_hydrodyn->bead_model_from_file ?
                "from file" : "computed" ) : "user entered",
              CSTF / num, sqrt(temp), "[S]");
      if ((nat + colorzero + colorsix) < numero_sfere)
         fprintf(ris,
                 "- !!WARNING: ONLY PART(S) OF THE MODELS HAVE BEEN ANALYZED, BUT THE PSV UTILIZED    IS THAT OF THE ENTIRE MODEL!! - \n");
      supc_results->s20w = CSTF / num;
      supc_results->s20w_sd = sqrt(temp);
   }

   if (raflag == -3.0)
   {
      temp = fabs((CSTF2 - pow(CSTF, 2) / num) / (num - 1));
      fprintf(ris, "- SED. COEFF. (psv %s) \t%.3g\t\t%.3g\t\t%s\n", 
              us_hydrodyn->misc.compute_vbar ?
              ( us_hydrodyn->bead_model_from_file ?
                "from file" : "computed" ) : "user entered",
              CSTF / num, sqrt(temp), "[S]");
      supc_results->s20w = CSTF / num;
      supc_results->s20w_sd = sqrt(temp);
      if ((nat + colorzero + colorsix) < numero_sfere)
         fprintf(ris,
                 "- !!WARNING: ONLY PART(S) OF THE MODELS HAVE BEEN ANALYZED, BUT THE PSV UTILIZED    IS THAT OF THE ENTIRE MODEL!! - \n");
      temp = fabs((CST2 - pow(CST, 2) / num) / (num - 1));
      fprintf(ris, "%s\t%.3g\t\t%.3g\t\t%s\n", "- SED. COEFF. (psv unhyd.rad.)", CST / num, sqrt(temp), "[S]");
   }

   temp = sqrt(fabs((ff0_t2 - pow(ff0_t, 2) / num) / (num - 1)));
   fprintf(ris, "%s\t%.2f\t%.3e\n", "- FRICTIONAL RATIO            ", ff0_t / num, temp);
   supc_results->ff0 = ff0_t / num;
   supc_results->ff0_sd = temp;

   if ( us_hydrodyn->advanced_config.expert_mode ) {
      temp = fabs((CfR12 - pow(CfR1, 2) / num) / (num - 1));
      fprintf(ris, "\n%s\t%.3e\t%.3e\t%s\n", "- ROT. FRICT. COEFF.          ", CfR1 / num, sqrt(temp), "[g*cm^2/s]");

      temp = fabs((CdR12 - pow(CdR1, 2) / num) / (num - 1));
      fprintf(ris, "%s\t%.0f\t\t%.0f\t\t%s\n", "- ROT. DIFF. COEFF.           ", CdR1 / num, sqrt(temp), "[1/s]");

      temp = fabs((CfR2[0] - pow(CfR[0], 2) / num) / (num - 1));
      fprintf(ris, "%s\t%.3e\t%.3e\t%s\n", "- ROT. FRICT. COEFF. [ X ]    ", CfR[0] / num, sqrt(temp), "[g*cm^2/s]");

      temp = fabs((CfR2[1] - pow(CfR[1], 2) / num) / (num - 1));
      fprintf(ris, "%s\t%.3e\t%.3e\t%s\n", "- ROT. FRICT. COEFF. [ Y ]    ", CfR[1] / num, sqrt(temp), "[g*cm^2/s]");
      temp = fabs((CfR2[2] - pow(CfR[2], 2) / num) / (num - 1));
      fprintf(ris, "%s\t%.3e\t%.3e\t%s\n", "- ROT. FRICT. COEFF. [ Z ]    ", CfR[2] / num, sqrt(temp), "[g*cm^2/s]");
      temp = fabs((CdR2[0] - pow(CdR[0], 2) / num) / (num - 1));
      fprintf(ris, "%s\t%.0f\t\t%.0f\t\t%s\n", "- ROT. DIFF. COEFF. [ X ]     ", CdR[0] / num, sqrt(temp), "[1/s]");
      temp = fabs((CdR2[1] - pow(CdR[1], 2) / num) / (num - 1));
      fprintf(ris, "%s\t%.0f\t\t%.0f\t\t%s\n", "- ROT. DIFF. COEFF. [ Y ]     ", CdR[1] / num, sqrt(temp), "[1/s]");
      temp = fabs((CdR2[2] - pow(CdR[2], 2) / num) / (num - 1));
      fprintf(ris, "%s\t%.0f\t\t%.0f\t\t%s\n\n", "- ROT. DIFF. COEFF. [ Z ]     ", CdR[2] / num, sqrt(temp), "[1/s]");
   }
   if ((raflag == -1.0) || (raflag == -3.0))
   {
      temp = fabs((Rg2 - pow(Rg, 2) / num) / (num - 1));
      fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", "- RADIUS OF GYRATION (Hydr.)  ", Rg / num, sqrt(temp), "[nm]");
      supc_results->rg = Rg / num;
      supc_results->rg_sd = sqrt(temp);
      temp = fabs((Rgu2 - pow(Rgu, 2) / num) / (num - 1));
      fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", "- RADIUS OF GYRATION (Unhydr.)", Rgu / num, sqrt(temp), "[nm]");
   }
   else
   {
      temp = fabs((Rg2 - pow(Rg, 2) / num) / (num - 1));
      fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", "- RADIUS OF GYRATION          ", Rg / num, sqrt(temp), "[nm]");
      supc_results->rg = Rg / num;
      supc_results->rg_sd = sqrt(temp);
   }

   temp = fabs((RSt2 - pow(RSt, 2) / num) / (num - 1));
   fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", "- TRANS. STOKES' RADIUS       ", RSt / num, sqrt(temp), "[nm]");
   supc_results->rs = RSt / num;
   supc_results->rs_sd = sqrt(temp);

   if ( us_hydrodyn->advanced_config.expert_mode ) {
      temp = fabs((RSr2[0] - pow(RSr[0], 2) / num) / (num - 1));
      fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", "- ROTAT. STOKES' RADIUS [ X ] ", RSr[0] / num, sqrt(temp), "[nm]");

      temp = fabs((RSr2[1] - pow(RSr[1], 2) / num) / (num - 1));
      fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", "- ROTAT. STOKES' RADIUS [ Y ] ", RSr[1] / num, sqrt(temp), "[nm]");

      temp = fabs((RSr2[2] - pow(RSr[2], 2) / num) / (num - 1));
      fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", "- ROTAT. STOKES' RADIUS [ Z ] ", RSr[2] / num, sqrt(temp), "[nm]");

      temp = fabs((VIM2 - pow(VIM, 2) / num) / (num - 1));
      // fprintf(ris, "\n%s\t%.2f\t\t%.2f\t\t%s\n", "- INTRINSIC VISCOSITY         ", VIM / num, sqrt(temp), "[cm^3/g]");
      fprintf(ris, "\n%s\t%.2f\t\t%.2f\t\t%s\n", "- UNCORRECTED INTRINSIC VISC. ", VIM / num, sqrt(temp), "[cm^3/g]");

      temp = fabs((RE2 - pow(RE, 2) / num) / (num - 1));
      // fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", "- EINSTEIN'S RADIUS           ", RE / num, sqrt(temp), "[nm]");
      fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", "- UNCORRECTED EINSTEIN'S RADIUS", RE / num, sqrt(temp), "[nm]");

      temp = fabs((VIMC2 - pow(VIMC, 2) / num) / (num - 1));
      // fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", "- INTRINSIC VISC. (GDLT corr.)", VIMC / num, sqrt(temp), "[cm^3/g]");
      fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", "- CORRECTED INTRINSIC VISCOSITY", VIMC / num, sqrt(temp), "[cm^3/g]");
      supc_results->viscosity = VIMC / num;
      supc_results->viscosity_sd = sqrt(temp);

      temp = fabs((REC2 - pow(REC, 2) / num) / (num - 1));
      // fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", "- EINSTEIN'S RADIUS (GDLT co.)", REC / num, sqrt(temp), "[nm]");
      fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", "- CORRECTED EINSTEIN'S RADIUS", REC / num, sqrt(temp), "[nm]");

#if defined(TSUDA_DOUBLESUM)
      temp = fabs((VIMDS2 - pow(VIMDS, 2) / num) / (num - 1));
      fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", "- INTRINSIC VISC. (Double Sum)", VIMDS / num, sqrt(temp), "[cm^3/g]");

      temp = fabs((REDS2 - pow(REDS, 2) / num) / (num - 1));
      fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", "- EINSTEIN'S RADIUS (D. Sum)  ", REDS / num, sqrt(temp), "[nm]");

      temp = fabs((VIMTM2 - pow(VIMTM, 2) / num) / (num - 1));
      fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", "- INTRINSIC VISC. (Tsuda CM)  ", VIMTM / num, sqrt(temp), "[cm^3/g]");

      temp = fabs((RETM2 - pow(RETM, 2) / num) / (num - 1));
      fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", "- EINSTEIN'S RADIUS (Tsuda CM)", RETM / num, sqrt(temp), "[nm]");

      temp = fabs((VIMTV2 - pow(VIMTV, 2) / num) / (num - 1));
      fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", "- INTRINSIC VISC. (Tsuda CV)  ", VIMTV / num, sqrt(temp), "[cm^3/g]");

      temp = fabs((RETV2 - pow(RETV, 2) / num) / (num - 1));
      fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", "- EINSTEIN'S RADIUS (Tsuda CV)", RETV / num, sqrt(temp), "[nm]");
#endif

      fprintf(ris, "\nRELAXATION TIMES\n\n");

      taoh = 0.0;
      taom = 0.0;
      taod = 0.0;
      taodin = 0.0;

      if (taoflag == 1.0)
      {
         temp = fabs((CT2[0] - pow(CT[0], 2) / num) / (num - 1));
         fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", " Tau(1)                       ", CT[0] / num, sqrt(temp), "[ns]");
         taom = taom + (CT[0] / num / temp);
         stdinv = (1 / (CT[0] / num)) - (1 / ((CT[0] / num) - (sqrt(temp))));
         taoh = taoh + (1 / (CT[0] / num) / (pow(stdinv, 2)));
         taod = taod + (1 / temp);
         taodin = taodin + (1 / (pow(stdinv, 2)));

         temp = fabs((CT2[1] - pow(CT[1], 2) / num) / (num - 1));
         fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", " Tau(2)                       ", CT[1] / num, sqrt(temp), "[ns]");
         fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", " Tau(3)                       ", CT[1] / num, sqrt(temp), "[ns]");
         taom = taom + (CT[1] / num / temp);
         stdinv = (1 / (CT[1] / num)) - (1 / ((CT[1] / num) - (sqrt(temp))));
         taoh = taoh + (1 / (CT[1] / num) / (pow(stdinv, 2)));
         taod = taod + (1 / temp);
         taodin = taodin + (1 / (pow(stdinv, 2)));

         temp = fabs((CT2[3] - pow(CT[3], 2) / num) / (num - 1));
         fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", " Tau(4)                       ", CT[3] / num, sqrt(temp), "[ns]");
         fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n\n", " Tau(5)                       ", CT[3] / num, sqrt(temp), "[ns]");
         taom = taom + (CT[3] / num / temp);
         stdinv = (1 / (CT[3] / num)) - (1 / ((CT[3] / num) - (sqrt(temp))));
         taoh = taoh + (1 / (CT[3] / num) / (pow(stdinv, 2)));
         taod = taod + (1 / temp);
         taodin = taodin + (1 / (pow(stdinv, 2)));
         //      fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", " Tau(m) (Weighted average)    ", (taom / taod), (sqrt(1 / taod)), "[ns]");
         stdinv = (taoh / taodin) + ((sqrt(1 / taodin)));
         //      fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", " Tau(h) (Weighted average)    ", (1 / (taoh / taodin)),
         //              ((1 / (taoh / taodin)) - (1 / stdinv)), "[ns]");
      }

      if (taoflag == 2.0)
      {
         temp = fabs((CT2[4] - pow(CT[4], 2) / num) / (num - 1));
         fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", " Tau(1)                       ", CT[4] / num, sqrt(temp), "[ns]");
         taom = taom + (CT[4] / num / temp);
         stdinv = (1 / (CT[4] / num)) - (1 / ((CT[4] / num) - (sqrt(temp))));
         taoh = taoh + (1 / (CT[4] / num) / (pow(stdinv, 2)));
         taod = taod + (1 / temp);
         taodin = taodin + (1 / (pow(stdinv, 2)));

         temp = fabs((CT2[1] - pow(CT[1], 2) / num) / (num - 1));
         fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", " Tau(2)                       ", CT[1] / num, sqrt(temp), "[ns]");
         fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", " Tau(3)                       ", CT[1] / num, sqrt(temp), "[ns]");
         taom = taom + (CT[1] / num / temp);
         stdinv = (1 / (CT[1] / num)) - (1 / ((CT[1] / num) - (sqrt(temp))));
         taoh = taoh + (1 / (CT[1] / num) / (pow(stdinv, 2)));
         taod = taod + (1 / temp);
         taodin = taodin + (1 / (pow(stdinv, 2)));

         temp = fabs((CT2[3] - pow(CT[3], 2) / num) / (num - 1));
         fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", " Tau(4)                       ", CT[2] / num, sqrt(temp), "[ns]");
         fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n\n", " Tau(5)                       ", CT[2] / num, sqrt(temp), "[ns]");
         taom = taom + (CT[3] / num / temp);
         stdinv = (1 / (CT[3] / num)) - (1 / ((CT[3] / num) - (sqrt(temp))));
         taoh = taoh + (1 / (CT[3] / num) / (pow(stdinv, 2)));
         taod = taod + (1 / temp);
         taodin = taodin + (1 / (pow(stdinv, 2)));
         //      fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", " Tau(m) (Weighted average)    ", (taom / taod), (sqrt(1 / taod)), "[ns]");
         stdinv = (taoh / taodin) + ((sqrt(1 / taodin)));
         // fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", " Tau(h) (Weighted average)    ", (1 / (taoh / taodin)),
         //        ((1 / (taoh / taodin)) - (1 / stdinv)), "[ns]");
      }

      if (taoflag == 0.0)
      {
         temp = fabs((CT2[0] - pow(CT[0], 2) / num) / (num - 1));
         fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", " Tau(1)                       ", CT[0] / num, sqrt(temp), "[ns]");
         taom = taom + (CT[0] / num / temp);
         stdinv = (1 / (CT[0] / num)) - (1 / ((CT[0] / num) - (sqrt(temp))));
         taoh = taoh + (1 / (CT[0] / num) / (pow(stdinv, 2)));
         taod = taod + (1 / temp);
         taodin = taodin + (1 / (pow(stdinv, 2)));

         temp = fabs((CT2[1] - pow(CT[1], 2) / num) / (num - 1));
         fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", " Tau(2)                       ", CT[1] / num, sqrt(temp), "[ns]");
         taom = taom + (CT[1] / num / temp);
         stdinv = (1 / (CT[1] / num)) - (1 / ((CT[1] / num) - (sqrt(temp))));
         taoh = taoh + (1 / (CT[1] / num) / (pow(stdinv, 2)));
         taod = taod + (1 / temp);
         taodin = taodin + (1 / (pow(stdinv, 2)));

         temp = fabs((CT2[2] - pow(CT[2], 2) / num) / (num - 1));
         fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", " Tau(3)                       ", CT[2] / num, sqrt(temp), "[ns]");
         taom = taom + (CT[2] / num / temp);
         stdinv = (1 / (CT[2] / num)) - (1 / ((CT[2] / num) - (sqrt(temp))));
         taoh = taoh + (1 / (CT[2] / num) / (pow(stdinv, 2)));
         taod = taod + (1 / temp);
         taodin = taodin + (1 / (pow(stdinv, 2)));

         temp = fabs((CT2[3] - pow(CT[3], 2) / num) / (num - 1));
         fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", " Tau(4)                       ", CT[3] / num, sqrt(temp), "[ns]");
         taom = taom + (CT[3] / num / temp);
         stdinv = (1 / (CT[3] / num)) - (1 / ((CT[3] / num) - (sqrt(temp))));
         taoh = taoh + (1 / (CT[3] / num) / (pow(stdinv, 2)));
         taod = taod + (1 / temp);
         taodin = taodin + (1 / (pow(stdinv, 2)));

         temp = fabs((CT2[4] - pow(CT[4], 2) / num) / (num - 1));
         fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", " Tau(5)                       ", CT[4] / num, sqrt(temp), "[ns]");
         taom = taom + (CT[4] / num / temp);
         stdinv = (1 / (CT[4] / num)) - (1 / ((CT[4] / num) - (sqrt(temp))));
         taoh = taoh + (1 / (CT[4] / num) / (pow(stdinv, 2)));
         taod = taod + (1 / temp);
         taodin = taodin + (1 / (pow(stdinv, 2)));
         //      fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", " Tau(m) (Weighted average)    ", (taom / taod), (sqrt(1 / taod)), "[ns]");
         stdinv = (taoh / taodin) + ((sqrt(1 / taodin)));
         //      fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", " Tau(h) (Weighted average)    ", (1 / (taoh / taodin)),
         //              ((1 / (taoh / taodin)) - (1 / stdinv)), "[ns]");
      }

      temp = fabs((CTM2 - pow(CTM, 2) / num) / (num - 1));
      fprintf(ris, "\n%s\t%.2f\t\t%.2f\t\t%s\n", " Tau(m) (Unweighted average)  ", CTM / num, sqrt(temp), "[ns]");
      temp = fabs((CTH2 - pow(CTH, 2) / num) / (num - 1));
      fprintf(ris, "%s\t%.2f\t\t%.2f\t\t%s\n", " Tau(h) (Unweighted average)  ", CTH / num, sqrt(temp), "[ns]");
      supc_results->tau = CTH / num;
      supc_results->tau_sd = sqrt(temp);
   }
   
   fprintf(ris, "\n****************************************************************\n");
   fclose(ris);

   if ( smi_mm ) {
      bool create_hydro_res = !(us_hydrodyn->batch_widget &&
                                us_hydrodyn->batch_window->save_batch_active);

      if ( us_hydrodyn->saveParams && create_hydro_res ) {
         vector < save_data > stats = us_hydrodyn->save_util->stats( & smi_mm_save_params.data_vector );

         QString smi_out_name = smi_mm_name + ".smi.csv";
         if ( !us_hydrodyn->overwrite_hydro ) {
            smi_out_name = us_hydrodyn->fileNameCheck( smi_out_name, 0, us_hydrodyn );
         }
         QFile f( smi_out_name );
         if ( !f.open( QIODevice::WriteOnly ) ) {
            us_hydrodyn->editor_msg( "red", QString( us_tr( "Error: could not open output file %1 for writing" ) ).arg( smi_out_name ) );
         } else {
            QTextStream t( &f );
            t << us_hydrodyn->save_util->header().toLatin1().data();

            for ( int i = 0; i < (int) smi_mm_save_params.data_vector.size(); ++i ) {
               t << us_hydrodyn->save_util->dataString( & smi_mm_save_params.data_vector[ i ] ).toLatin1().data();
            }
            for ( int i = 0; i < (int) stats.size(); ++i ) {
               t << us_hydrodyn->save_util->dataString( & stats[ i ] ).toLatin1().data();
            }
               
            us_hydrodyn->editor_msg( "dark blue", QString( us_tr( "Wrote %1" ) ).arg( smi_out_name ) );
            f.close();
         }

      }
   }
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#if defined(OLD_WAY_CHECK)

static void
inp_inter()
{

   int i;

   interinp = us_fopen("ifraxon", "wb");
   interinp1 = us_fopen("ifraxon1", "wb");

   fprintf(interinp, "%d\n", nat);
   fprintf(interinp, "%f\n", 0.0);
   fprintf(interinp, "%s\n", "ifraxon1");

#if defined(DEBUG_WW)
   for (i = 0; i < nat; i++) {
      cks += (double)dt[i].x;
      cks += (double)dt[i].y;
      cks += (double)dt[i].z;
      cks += (double)dt[i].r;
      cks += (double)dt[i].m;
      cks += (double)dt[i].col;
   }
   dww("inp_inter before writing");
#endif
   for (i = 0; i < nat; i++)
   {
      fprintf(interinp, "%f\t", dt[i].x);
      fprintf(interinp, "%f\t", dt[i].y);
      fprintf(interinp, "%f\n", dt[i].z);
      fprintf(interinp1, "%f\t", dt[i].r);
      fprintf(interinp1, "%f\t", dt[i].m);
      fprintf(interinp1, "%d\n", dt[i].col);
   }

   fclose(interinp);
   fclose(interinp1);
}
#endif

#if defined(OLD_WAY)
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

static void
out_inter()
{

   int i;
   int n;
   char ramaco[20];

   interout = us_fopen("ofraxon", "r");

   if ( 1 == fscanf(interout, "%d", &n) ) {};
   if ( 1 == fscanf(interout, "%f", &raggio) ) {};
   if ( 1 == fscanf(interout, "%s", ramaco) ) {};

   for (i = 0; i < n; i++)
   {
      if ( 1 == fscanf(interout, "%f", &dtn[i].x) ) {};
      if ( 1 == fscanf(interout, "%f", &dtn[i].y) ) {};
      if ( 1 == fscanf(interout, "%f", &dtn[i].z) ) {};
   }

#if defined(DEBUG_WW)
   for (i = 0; i < nat; i++) {
      cks += (double)dtn[i].x;
      cks += (double)dtn[i].y;
      cks += (double)dtn[i].z;
   }
   dww("out_inter after reading");
#endif
   fclose(interout);

}
#endif
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

static void
mem_mol()
{

   int i;
   int n;
   char nmolecola[SMAX], nragcol[SMAX];
   char risp1, risp2, risp3;
 a100:
   printf("\n\n** Insert file name for coordinates :___ ");
   if ( 1 == scanf("%s", nmolecola) ) {};
   getchar();

   new_mol1 = us_fopen(nmolecola, "r");

   if (new_mol1 != NULL)
   {
      printf("\n");
      printf("*** CAUTION : File already exists ! ***\n");
      printf("** Do you want change the file name ? (y/n) :___ ");
      if ( 1 == scanf("%s", &risp2) ) {};
      getchar();
      fclose(new_mol1);
      if ((risp2 == 'y') || (risp2 == 'Y'))
         goto a100;
   }

   if (nat == numero_sfere)
   {
      printf("** Same File Name r_m_c ? (y/n) :___ ");
      if ( 1 == scanf("%s", &risp1) ) {};
      getchar();
   }

   else
      risp1 = 'n';

   if ((risp1 == 'n') || (risp1 == 'N'))
   {
   a150:
      printf("** Insert file name r_m_c :___ ");
      if ( 1 == scanf("%s", nragcol) ) {};
      getchar();
      new_mol1 = us_fopen(nragcol, "r");

      if (new_mol1 != NULL)
      {
         printf("\n");
         printf("*** CAUTION : File already exists ! ***\n");
         printf("** Do you want change the file name ? (y/n) :___ ");
         if ( 1 == scanf("%s", &risp3) ) {};
         getchar();
         fclose(new_mol1);
         if ((risp3 == 'y') || (risp3 == 'Y'))
            goto a150;

      }
   }
   else
   {
      mol = us_fopen(molecola, "r");
      if ( 1 == fscanf(mol, "%d", &n) ) {};
      if ( 1 == fscanf(mol, "%f", &raggio) ) {};
      if ( 1 == fscanf(mol, "%s", nragcol) ) {};
      fclose(mol);
      raflag = raggio; // ?? ==
   }

   new_mol1 = us_fopen(nmolecola, "w");

   fprintf(new_mol1, "%d\t", nat);
   if (raflag == 0.0)
      fprintf(new_mol1, "%f\t", 0.0);
   if (raflag == -1.0)
      fprintf(new_mol1, "%f\t", -1.0);
   if (raflag == -2.0)
      fprintf(new_mol1, "%f\t", -2.0);
   if (raflag == -3.0)
      fprintf(new_mol1, "%f\t", -3.0);
   if (raflag == -4.0)
      fprintf(new_mol1, "%f\t", -4.0);
   if (raflag == -5.0)
      fprintf(new_mol1, "%f\t", -5.0);
   if ((raflag == -2.0) || (raflag == -3.0) || (raflag == -5.0))
   {
      fprintf(new_mol1, "%s\t", nragcol);
      fprintf(new_mol1, "%f\n", partvol);
   }
   else
      fprintf(new_mol1, "%s\n", nragcol);

   for (i = 0; i < nat; i++)
   {
      fprintf(new_mol1, "%f\t", dt[i].x);
      fprintf(new_mol1, "%f\t", dt[i].y);
      fprintf(new_mol1, "%f\n", dt[i].z);
   }

   fclose(new_mol1);

   if (risp1 == 'n')
   {
      new_rmc = us_fopen(nragcol, "w");
      if ((raflag == 0.0) || (raflag == -2.0))
      {
         for (i = 0; i < nat; i++)
         {
            fprintf(new_rmc, "%f\t", dt[i].r);
            fprintf(new_rmc, "%f\t", dt[i].m);
            fprintf(new_rmc, "%d\n", dt[i].col);
         }
      }
      if ((raflag == -1.0) || (raflag == -3.0))
      {
         for (i = 0; i < nat; i++)
         {
            fprintf(new_rmc, "%f\t", dt[i].r);
            fprintf(new_rmc, "%f\t", dt[i].ru);
            fprintf(new_rmc, "%f\t", dt[i].m);
            fprintf(new_rmc, "%d\n", dt[i].col);
         }
      }
      if ((raflag == -4.0) || (raflag == -5.0))
      {
         for (i = 0; i < nat; i++)
         {
            fprintf(new_rmc, "%f\t", dt[i].r);
            fprintf(new_rmc, "%f\t", dt[i].m);
            fprintf(new_rmc, "%d\t", dt[i].col);
            fprintf(new_rmc, "%s\n", dt[i].cor);
         }
      }
      fclose(new_rmc);
   }

}

static void
maxest()
{

   int i, j;
   float temp;

   temp = 0.0;
   maxx = 0.0;
   maxy = 0.0;
   maxz = 0.0;

   for (i = 0; i < nat; i++)
   {
      for (j = i + 1; j < nat; j++)
      {
         if ((temp = (fabs(dt[i].x - dt[j].x) + dt[i].r + dt[j].r)) > maxx)
            maxx = temp;
         if ((temp = (fabs(dt[i].y - dt[j].z) + dt[i].r + dt[j].r)) > maxy)
            maxy = temp;
         if ((temp = (fabs(dt[i].z - dt[j].z) + dt[i].r + dt[j].r)) > maxz)
            maxz = temp;
      }
   }

}

/**************************************************************************/
/*                                  End                           */
/**************************************************************************/

// autov.c

static void
autovalori()
{

   int a1, a2;
   long double b, c; // unused : d;
   long double a[3][3];
   // long slip;

   /* COMPUTATION OF THE EIGENVALUES dl1, dl2, dl3  OF THE MATRIX A */

   // slip = 1.0;
   // if (cc == 2)
   //    slip = 1.5;

#if defined( DEBUG_EV )
   {
      QTextStream( stdout ) <<
         "autovalori() Dr ";
      for ( int i = 0; i < 9; ++i ) {
         QTextStream( stdout ) << QString().sprintf( "%Le ", Dr[i] );
      }
      QTextStream( stdout ) << "\n";
   }
#endif
   
   for (a1 = 0; a1 < 3; a1++)
      for (a2 = 0; a2 < 3; a2++)
      {
         if (fabs(Dr[a1 * 3 + a2]) < 0.0000000001)
            a[a1][a2] = 0.0;
         else
            a[a1][a2] = Dr[a1 * 3 + a2] * KB * TE / 1.0E-21;
      }

#if defined( DEBUG_EV )
   {
      QTextStream( stdout ) <<
         "autovalori()\n\ta = [ ";
      for ( int i = 0; i < 3; ++i ) {
         for ( int j = 0; j < 3; ++j ) {
            QTextStream( stdout ) << QString().sprintf( "%Le ", a[i][j] );
         }
         QTextStream( stdout ) << ";\n";
      }
      QTextStream( stdout ) << "\n";
   }
#endif
   
   if ((a[0][1] == 0.0) && (a[1][2] == 0.0) && (a[0][2] == 0.0))
   {
#if defined( DEBUG_EV )
      qDebug() << "zeros";
#endif
      dl1 = a[0][0];
      dl2 = a[1][1];
      dl3 = a[2][2];
   }

   else if ((a[0][2] == 0.0) && (a[1][2] == 0.0))
   {
#if defined( DEBUG_EV )
      qDebug() << "secondo() 1";
#endif
      dl1 = a[2][2];
      b = (-a[0][0] - a[1][1]);
      c = a[0][0] * a[1][1] - a[0][1] * a[1][0];
      secondo(b, c);
   }

   else if ((a[0][2] == 0.0) && (a[0][1] == 0.0))
   {
#if defined( DEBUG_EV )
      qDebug() << "secondo() 2";
#endif
      dl1 = a[0][0];
      b = (-a[2][2] - a[1][1]);
      c = a[2][2] * a[1][1] - a[2][1] * a[1][2];
      secondo(b, c);
   }

   else
   {
#if defined( DEBUG_EV )
      qDebug() << "terzo()";
      b = (-(a[0][0] + a[1][1] + a[2][2]));
      c = a[0][0] * a[1][1] + a[0][0] * a[2][2] + a[1][1] * a[2][2] - a[0][2] * a[2][0] - a[1][2] * a[2][1] -
         a[0][1] * a[1][0];
      d = (-a[0][0] * a[1][1] * a[2][2] - 2.0 * a[0][1] * a[0][2] * a[1][2] + a[0][2] * a[0][2] * a[1][1] +
           a[1][2] * a[1][2] * a[0][0] + a[0][1] * a[0][1] * a[2][2]);
      terzo(b, c, d);
      printf("EIGENVALUES II in autva from terzo:\t ");
      printf("%Le%s%Le%s%Le\n\n\n",dl1,",",dl2,",",dl3);
      
      qDebug() << "terzo->ev";
#endif
      vector < vector < long double > > A(3);
      for ( int i = 0; i < 3; ++i ) {
         for ( int j = 0; j < 3; ++j ) {
            A[i].push_back( a[i][j] );
         }
      }
      ev3( A );
#if defined( DEBUG_EV )
      printf("EIGENVALUES II in autva from ev3:\t ");
      printf("%Le%s%Le%s%Le\n\n\n",dl1,",",dl2,",",dl3);
#endif

   }

#if defined( DEBUG_EV )
   printf("EIGENVALUES II in autva :\t ");
   printf("%Le%s%Le%s%Le\n\n\n",dl1,",",dl2,",",dl3);
#endif

}

// chol.c

static void
riempimatrice()
{

   int i, j, k, l;

   printf("SUPERMATRIX INVERSION\n\n");

   printf("Cycle 1 of 3; model %d of %d\n\n", kkk, num);

   editor->append("Supermatrix inversion Cycle 1 of 3\n");
   qApp->processEvents();
#if defined(SHOW_TIMING)
   gettimeofday(&s1tv0, NULL);
#endif
   for (i = 0; i < nat; i++)
   {

      smi_progress->setValue(ppos++);
      qApp->processEvents();
      us_hydrodyn->lbl_core_progress->setText(QString("Iteration %1 of %2")
                                              .arg(i+1)
                                              .arg(nat));
      if (us_hydrodyn->stopFlag)
      {
         return;
      }

      printf("%s%d%s%d", "Iteration  ", i + 1, " of ", nat);
      
      fflush(stdout);

      for (j = 0; j < nat; j++)
      {
         for (k = 0; k < 3; k++)
         {
            for (l = 0; l < 3; l++)
            {
               a[(3 * i + k) * (3 * nat) + 3 * j + l] = q[i * (nat * 9) + j * 9 + k * 3 + l];
            }
         }
      }

      printf("%c", '\r');

   }

#if defined(SHOW_TIMING)
   gettimeofday(&s1tv9, NULL);
   supc1_tot = 1000000l * (s1tv9.tv_sec - s1tv0.tv_sec) + s1tv9.tv_usec - s1tv0.tv_usec;
#endif

   printf("\n\n");

}

/*************************************************************************/

static void
choldc(int N)
{
   int i, j, k;
   float sum;

   printf("Cycle 2 of 3; model %d of %d\n\n", kkk, num);
   editor->append("Supermatrix inversion Cycle 2 of 3\n");
   qApp->processEvents();

   for (i = 0; i < 3 * N; i++)
   {

      smi_progress->setValue(ppos++);
      qApp->processEvents();
      if (us_hydrodyn->stopFlag)
      {
         return;
      }

      printf("%s%d%s%d", "Iteration  ", i + 1, " of ", 3 * N);
      fflush(stdout);
      us_hydrodyn->lbl_core_progress->setText(QString("Iteration %1 of %2")
                                              .arg(i+1)
                                              .arg(3 * N));

      for (j = i; j < 3 * N; j++)
      {

         for (sum = a[i * (3 * nat) + j], k = i - 1; k >= 0; k--)
         {
            sum -= a[i * (3 * nat) + k] * a[j * (3 * nat) + k];
         }
         if (i == j)
         {
            if (sum <= 0.0)
            {
               printf("The matrix to be inverted is not defined positive");
               exit(0);
            }
            p[i] = sqrt(sum);
         }
         else
            a[j * (3 * nat) + i] = sum / p[i];

      }
      printf("%c", '\r');
   }
}

/************************************************************************/

static void
cholsl(int N)
{

   int i = 0;
   int k = 0;
   float sum = 0.0;

#if defined(SHOW_TIMING)
   gettimeofday(&s3tv3, NULL);
#endif

   for (i = 0; i < 3 * N; i++)
   {
      // this sum could be parallized
      for (sum = b1[i], k = i - 1; k >= 0; k--)
         sum -= a[i * (3 * nat) + k] * rRi[k];
      rRi[i] = sum / p[i];
   }

#if defined(SHOW_TIMING)
   gettimeofday(&s3tv4, NULL);
#endif

#if defined(USE_THREADS)
   //   int threads_used = 0;
   //   int threads_not_used = 0;
   if ( threads > 1 )
   {
      vector < float > psum(threads);
#if defined(MODE_1) 
      for (i = 3 * N - 1; i >= 0; i--)
      {
         if ( 3 * N - i < 4600 )
         {
            //            threads_not_used++;
            for (sum = rRi[i], k = i + 1; k < 3 * N; k++)
               sum -= a[k * (3 * nat) + i] * rRi[k];
         } else {
            //            threads_used++;
            int j;
            sum = rRi[i];             
            for ( j = 0; j < threads; j++ )
            {
# if defined(DEBUG_THREAD)
               cout << "thread " << j << endl;
# endif            
               psum[j] = 0.0;
               supc_thr_threads[j]->supc_thr_setup(
                                                   threads,
                                                   1, // mode 1, loop over k
                                                   p,
                                                   rRi,
                                                   rRi,
                                                   a,
                                                   N,
                                                   i,
                                                   nat,
                                                   &psum[j]
                                                   );
            }
            for ( j = 0; j < threads; j++ )
            {
               supc_thr_threads[j]->supc_thr_wait();
               sum += psum[j];
            }
         }
         rRi[i] = sum / p[i];
      }
#endif
#if defined(MODE_2)
      int j;
      memcpy(rRis, rRi, sizeof(float) * nmax * 3);
      for ( j = 0; j < threads; j++ )
      {
# if defined(DEBUG_THREAD)
         cout << "thread " << j << endl;
# endif            
         supc_thr_threads[j]->supc_thr_setup(
                                             threads,
                                             2, // mode 2 loop over i
                                             p,
                                             rRis,
                                             rRi,
                                             a,
                                             N,
                                             i,
                                             nat,
                                             &psum[j]
                                             );
      }
      for ( j = 0; j < threads; j++ )
      {
         supc_thr_threads[j]->supc_thr_wait();
      }
#endif
   } else {
      for (i = 3 * N - 1; i >= 0; i--)
      {
         for (sum = rRi[i], k = i + 1; k < 3 * N; k++)
            sum -= a[k * (3 * nat) + i] * rRi[k];
         rRi[i] = sum / p[i];
      }
   }
#else
   for (i = 3 * N - 1; i >= 0; i--)
   {
      for (sum = rRi[i], k = i + 1; k < 3 * N; k++)
         sum -= a[k * (3 * nat) + i] * rRi[k];
      rRi[i] = sum / p[i];
   }
#endif


#if defined(SHOW_TIMING)
   gettimeofday(&s3tv5, NULL);
   cholsl_s1 += 1000000l * (s3tv4.tv_sec - s3tv3.tv_sec) + s3tv4.tv_usec - s3tv3.tv_usec;
   cholsl_s2 += 1000000l * (s3tv5.tv_sec - s3tv4.tv_sec) + s3tv5.tv_usec - s3tv4.tv_usec;
#endif
#if defined(USE_THREADSX)
   if ( threads > 1 ) 
   {
      printf("\n cholsl threads used %d, not used %d",
             threads_used,
             threads_not_used);
   }
#endif
          
}

/*************************************************************************/

static void
inizializza_b1()
{

   int i;

   for (i = 0; i < 3 * nat; i++)
   {
      b1[i] = 0.0;
      rRi[i] = 0.0;
   }

}

/*************************************************************************/

// diff.c

/**********************************************************************/
/*                                                                    */
/* Function that computes the relaxation times for a rigid model      */
/* Changed the taoh and taom means 3/6/2004 (always five times)       */
/*                                                                    */
/**********************************************************************/

static void
relax_rigid_calc()
{


   // char pluto;
   float ddd[3];
   // float rrr[3];
   long double ddr[3];
   long double pd[3] = { 0.0, 0.0, 0.0 };
   int a;
   // char pluto1;

   autovalori();

   {
      us_qdebug( QString( "supc compute_tau: input ev's: %1 %2 %3 fconv %4" )
              .arg( (double)dl1 )
              .arg( (double)dl2 )
              .arg( (double)dl3 )
              .arg( fconv ) );
      vector < double > results;
      double x1 = dl1;
      double x2 = dl2;
      double x3 = dl3;
      US_Saxs_Util::compute_tau( x1, x2, x3, fconv, results );
      us_qdebug( US_Vector::qs_vector( "supc compute_tau results", results ) );
   }

   ddr[0] = dl1;
   ddr[1] = dl2;
   ddr[2] = dl3;

#if defined( DEBUG_EV )
# if !defined( MINGW )
   printf("\nsupc compute_tau: ddr[0] ddr[1] ddr[2] : %Lf\t%Lf\t%Lf\n",dl1,dl2,dl3);
   printf("\nsupc compute_tau: ddr[0] ddr[1] ddr[2] : %Lf\t%Lf\t%Lf\n",ddr[0],ddr[1],ddr[2]);
# else
   printf("\nsupc compute_tau: ddr[0] ddr[1] ddr[2] : %f\t%f\t%f\n",(double)dl1,(double)dl2,(double)dl3);
   printf("\nsupc compute_tau: ddr[0] ddr[1] ddr[2] : %f\t%f\t%f\n",(double)ddr[0],(double)ddr[1],(double)ddr[2]);
# endif
#endif

   /*      printf("\nValori ddr[0] ddr[1] ddr[2] : %Lf\t%Lf\t%Lf\n",ddr[0],ddr[1],ddr[2]);
           if ( 1 == scanf("%s",&pluto1) ) {};
           getchar();    */

   for (a = 0; a < 3; a++)
   {
      ddd[a] = 0.0;
      // rrr[a] = 0.0;
   }

   a = (int) 0.0;
   if ((fabs(ddr[0] - ddr[1]) / ddr[0]) < 0.01)
   {
      a = (int) 1.0;
      ddr[1] = (ddr[0] + ddr[1]) / 2;

      pd[0] = ddr[1];
      pd[1] = ddr[1];
      pd[2] = ddr[2];
      if (ddr[2] <= ddr[1])
         a = (int) 2.0;

      /*      printf("\nCaso 1, valore a= %d\n",a);
              if ( 1 == scanf("%s",&pluto1) ) {};
              getchar();    */
   }

   if ((fabs(ddr[0] - ddr[2]) / ddr[0]) < 0.01)
   {
      a = (int) 1.0;
      ddr[2] = (ddr[0] + ddr[2]) / 2;

      pd[0] = ddr[2];
      pd[1] = ddr[2];
      pd[2] = ddr[1];
      if (ddr[1] <= ddr[2])
         a = (int) 2.0;

      /*            printf("\nCaso 2, valore a= %d\n",a);
                    if ( 1 == scanf("%s",&pluto1) ) {};
                    getchar();    */
   }

   if ((fabs(ddr[1] - ddr[2]) / ddr[1]) < 0.01)
   {
      a = (int) 1.0;
      ddr[1] = (ddr[1] + ddr[2]) / 2;

      pd[0] = ddr[1];
      pd[1] = ddr[1];
      pd[2] = ddr[0];
      if (ddr[0] <= ddr[1])
         a = (int) 2.0;

      /*      printf("\nCaso 3, valore a= %d\n",a);
              if ( 1 == scanf("%s",&pluto1) ) {};
              getchar();     */
   }

   if (a == 1.0)
   {
      tao[0] = 6.0 * pd[0];
      tao[1] = 5.0 * pd[0] + pd[2];
      tao[2] = tao[1];
      tao[3] = 2.0 * pd[0] + 4.0 * pd[2];
      tao[4] = tao[3];
      taoflag = 1.0;
      taoh = (tao[0] + (2 * tao[1]) + (2 * tao[3])) / 5;
      taoh = 1.0 / taoh;
      for (a = 0; a < 5; a++)
      {
         tao[a] = tao[a] * 1E-09;
      }
      taom = ((1.0 / tao[0]) + (2 * (1.0 / tao[1])) + (2 * (1.0 / tao[3]))) / 5;
   }

   if (a == 2.0)
   {
      tao[4] = 6.0 * pd[0];
      tao[1] = 5.0 * pd[0] + pd[2];
      tao[2] = tao[1];
      tao[3] = 2.0 * pd[0] + 4.0 * pd[2];
      tao[0] = tao[3];
      taoflag = 2.0;
      taoh = ((2 * tao[1]) + (2 * tao[3]) + tao[4]) / 5;
      taoh = 1.0 / taoh;
      for (a = 0; a < 5; a++)
      {
         tao[a] = tao[a] * 1E-09;
      }
      taom = ((2 * (1.0 / tao[1])) + (2 * (1.0 / tao[3])) + (1.0 / tao[4])) / 5;
   }

   if (a == 0.0)

   {
      /*      printf("\nCaso 4, valore a= %d\n",a);
              if ( 1 == scanf("%s",&pluto1) ) {};
              getchar();      */

      ddd[0] = ddr[0];
      ddd[1] = ddr[1];
      ddd[2] = ddr[2];

      if ((ddr[2] > ddr[1]) && (ddr[2] > ddr[0]))
      {
         if (ddr[1] > ddr[0])
         {
            ddr[2] = ddd[2];
            ddr[1] = ddd[1];
            ddr[0] = ddd[0];
         }
         else
         {
            ddr[2] = ddd[2];
            ddr[1] = ddd[0];
            ddr[0] = ddd[1];
         }
      }

      else if ((ddr[1] > ddr[2]) && (ddr[1] > ddr[0]))
      {
         if (ddr[2] > ddr[0])
         {
            ddr[2] = ddd[1];
            ddr[1] = ddd[2];
            ddr[0] = ddd[0];
         }
         else
         {
            ddr[2] = ddd[1];
            ddr[1] = ddd[0];
            ddr[0] = ddd[2];
         }
      }

      else
      {
         if (ddr[2] > ddr[1])
         {
            ddr[2] = ddd[0];
            ddr[1] = ddd[2];
            ddr[0] = ddd[1];
         }
         else
         {
            ddr[2] = ddd[0];
            ddr[1] = ddd[1];
            ddr[0] = ddd[2];
         }
      }
      pd[0] = ddr[0] * ddr[0] + ddr[1] * ddr[1] + ddr[2] * ddr[2] - ddr[0] * ddr[1] - ddr[0] * ddr[2] - ddr[1] * ddr[2];
      pd[0] = pow(pd[0], (long double).5);
      pd[1] = (ddr[0] + ddr[1] + ddr[2]) / 3.0;
      tao[0] = 6.0 * pd[1] - 2.0 * pd[0];
      tao[1] = 3.0 * (pd[1] + ddr[1]);
      tao[2] = 3.0 * (pd[1] + ddr[0]);
      tao[3] = 3.0 * (pd[1] + ddr[2]);
      tao[4] = 6.0 * pd[1] + 2.0 * pd[0];
      taoflag = 0.0;

      taom = 0.0;
      taoh = 0.0;

      for (a = 0; a < 5; a++)
      {
         taoh += tao[a];
         tao[a] = tao[a] * 1E-09;
         taom += 1.0 / tao[a];
      }
      taoh = taoh / 5.0;
      taoh = 1.0 / taoh;
      taom = taom / 5.0;
   }

   for (a = 0; a < 5; a++)
   {
      /*      printf("\n%s%d%s%f\n","Tao[",a,"] = ",tao[a]);     */
      tao[a] = 1.0 / tao[a];
      /*      printf("\n%s%d%s%Lg\n","Tao[",a,"] = ",tao[a]);
              if ( 1 == scanf("%s",&pluto1) ) {};
              getchar();       */
   }

   // for ( int i = 0; i < 5; ++i )
   // {
   //    us_qdebug( QString( "supc tau results %1 %2" ).arg( i+1 ).arg( (double)tao[ i ] ) );
   // }
   // us_qdebug( QString( "supc tau results %1 %2" ).arg( 5 ).arg( taoh ) );
   // us_qdebug( QString( "supc tau results %1 %2" ).arg( 6 ).arg( taom ) );

}

/**********************************************************************/
/*                              */
/* Function that calculates the diffusion tensors for a rigid model   */
/*                              */
/**********************************************************************/

static void
diffcalc()
{
   float ro[6][6];
   int i, j;

   for (i = 0; i < 3; i++)
   {
      for (j = 0; j < 6; j++)
      {
         if (j < 3)
            ro[i][j] = sigmat[j + 3 * i];
         else
            ro[i][j] = soct[j - 3 + 3 * i];
      }
   }

   for (i = 3; i < 6; i++)
   {
      for (j = 0; j < 6; j++)
      {
         if (j < 3)
            ro[i][j] = sigmaoc[j + 3 * i - 9];
         else
            ro[i][j] = sigmaoR[j + 3 * i - 12];
      }
   }

#if defined( DEBUG_EV )
   {
      QTextStream( stdout ) <<
         "diffcalc() ro:\n";
      for ( int i = 0; i < 6; ++i ) {
         QTextStream( stdout ) << "\t";
         for ( int j = 0; j < 6; ++j ) {
            QTextStream( stdout ) << QString().sprintf( "%e ", ro[i][j] );
         }
         QTextStream( stdout ) << "\n";
      }
      QTextStream( stdout ) << "\n";
   }
#endif   

   inv6x6(ro);

#if defined( DEBUG_EV )
   {
      QTextStream( stdout ) <<
         "diffcalc() inver:\n";
      for ( int i = 0; i < 6; ++i ) {
         QTextStream( stdout ) << "\t";
         for ( int j = 0; j < 6; ++j ) {
            QTextStream( stdout ) << QString().sprintf( "%e ", inver[i][j] );
         }
         QTextStream( stdout ) << "\n";
      }
      QTextStream( stdout ) << "\n";
   }
#endif
   
   for (i = 0; i < 6; i++)
      for (j = 0; j < 6; j++)
         ro[i][j] = inver[i][j];   /* WITHOUT CONSTANTS         */
   /* (BOLTZMANN & TEMPERATURE) */

   for (i = 0; i < 3; i++)
      for (j = 0; j < 3; j++)
         Dot[j + 3 * i] = ro[i][j];

   for (i = 0; i < 3; i++)
      for (j = 3; j < 6; j++)
         Doct[j - 3 + 3 * i] = ro[i][j];

   for (i = 3; i < 6; i++)
      for (j = 0; j < 3; j++)
         Doc[j + 3 * i - 9] = ro[i][j];

   for (i = 3; i < 6; i++)
      for (j = 3; j < 6; j++)
         Dr[j + 3 * i - 12] = ro[i][j];

}

// gir.c

static void
ragir()
{
   int i;
   float ro2;
   float mt;
   float rg;
   float rgu;
   float rou2 = 0.0;

   extern float raflag;

   mt = 0.0;
   xm = 0.0;
   ym = 0.0;
   zm = 0.0;
   ro2 = 0.0;

   // do as doubles for better sum
   double dmt = 0e0;
   for (i = 0; i < nat; i++)
   {
      dmt += ((int)((double)dt[i].m * 100e0 + 5e-1)) / 1e2;
      //   mt += dt[i].m;
   }
   mt = dmt;
   //   printf("ragir pre mw doub %f float not rounded %f float rounded %f, 14306.82 %f\n", dmt, mtu, mt, 14306.82f);

   for (i = 0; i < nat; i++)
   {
      xm += dt[i].x * dt[i].m;
      ym += dt[i].y * dt[i].m;
      zm += dt[i].z * dt[i].m;
   }

   xm = xm / mt;
   ym = ym / mt;
   zm = zm / mt;

   for (i = 0; i < nat; i++)
   {
      rg = 0.6 * dt[i].r * dt[i].r;
      ro2 += dt[i].m * (pow((dt[i].x - xm), 2) + pow((dt[i].y - ym), 2) + pow((dt[i].z - zm), 2) + rg);

      if ((raflag == -1.0) || (raflag == -3.0))
      {
         rgu = 0.6 * dt[i].ru * dt[i].ru;
         rou2 += dt[i].m * (pow((dt[i].x - xm), 2) + pow((dt[i].y - ym), 2) + pow((dt[i].z - zm), 2) + rgu);
      }
   }

   ro2 = ro2 / mt;
   ro = sqrt(ro2);

   if ((raflag == -1.0) || (raflag == -3.0))
   {
      rou2 = rou2 / mt;
      rou = sqrt(rou2);

   }

   pesmol = mt;

   /*
     if(sfecalc==2)  // computation with all beads or with only the 'exposed'

     {
     j=0;
     for(i=0;i<nat;i++)
     {
     if(dt[i].col==6)  ;
   
     else

     {
     dt[j]=dt[i];
     j++;
     }

     }
     nat=j;
     } 

   */
}

/********************************INIT.C****************************************/
/*                                                                            */
/* Module that loads the content of the files of the model to be analyzed into*/
/* the correspondent fields of the work record dt. Includes code for selecting*/
/* only part of the models to be analyzed IF NOT coming from sequential files.*/
/******************************************************************************/

// init.c

// FILE *mol;
// FILE *rmc;
// FILE *tot_mol;
static int j;
// extern float interm1;
// extern int colorzero;
// extern int colorsix;
// extern int colorsixf;
// static char pippo1;
static char ricorda[SMAX];

static void
init_da_a()
{

#if defined(DEBUG_FILES)
   mol = us_fopen(molecola, "r");   /* Check for the file existence  */
   // this is the .beams_file

   while (mol == NULL)
   {
      printf("\n");
      printf("** The Model does NOT exist !!\n");
      printf("** Insert the correct name :___");
      if ( 1 == scanf("%s", molecola) ) {};
      mol = us_fopen(molecola, "r");
   }

   if ( 1 == fscanf(mol, "%d", &nat) ) {};
   if ( 1 == fscanf(mol, "%f", &raggio) ) {};

   fclose(mol);
#endif

   printf("!!!nat %d %d active_model %d\n", nat, bead_count[active_model], active_model);

   nat = bead_count[active_model];
   raggio = -2.000000;

   /* Selects for whole or part of the model(s) to be analyzed */

   printf("\n%s%d%s", "** TOTAL Number of BEADS in the MODEL :___ ", nat, " **\n\n");

#if defined(USE_MAIN)
   prima = (-1);
   while ((prima < 0) || (prima > (nat - 1)))
   {
      printf("%s", "** Insert FIRST BEAD # to be included  :___ ");
      if ( 1 == scanf("%d", &prima) ) {};
      getchar();
      printf("\n");
   }

   ultima = nat + 1;
   while ((ultima < prima) || (ultima > nat))
   {
      printf("%s", "** Insert LAST BEAD # to be included :___ ");
      if ( 1 == scanf("%d", &ultima) ) {};
      getchar();
      printf("\n");
   }
#else
   prima = 1;
   ultima = nat;
#endif

   printf("adding file: %s to tot_mol\n",  molecola);
#if defined(CREATE_TOT_MOL)
   tot_mol = us_fopen("tot_mol", "ab");
   fprintf(tot_mol, "%s\n", molecola);
   fprintf(tot_mol, "%d\t%d\t%d\t%f\n", nat, prima, ultima, raggio);
   fclose(tot_mol);
#else
   molecola_v.push_back(QString("%1").arg(molecola));
   nat_v.push_back(nat);
   prima_v.push_back(prima);
   ultima_v.push_back(ultima);
   raggio_v.push_back(raggio);
#endif
   
}

/*********************************************************************/

static void
initarray(int k)
{
   int i;
   char temp[34];
   memset(temp, 0, 34);

#if defined(CREATE_TOT_MOL)
   if ( 1 == fscanf(tot_mol, "%s", molecola) ) {};
   strcpy(ricorda, molecola);
   if ( 1 == fscanf(tot_mol, "%d", &nat) ) {};
   if ( 1 == fscanf(tot_mol, "%d", &prima) ) {};
   if ( 1 == fscanf(tot_mol, "%d", &ultima) ) {};
#else
   strcpy(molecola, molecola_v[k].toLatin1().data());
   strcpy(ricorda, molecola);
   nat = nat_v[k];
   prima = prima_v[k];
   ultima = ultima_v[k];
#endif

   //    editor->append(QString("initarray - 0 From file: %1 beads\n").arg(nat));

   for (i = 0; i < nat; i++)
      dt[i].cor = temp;

   //    int count6 = 0;
#if defined(DEBUG_WW)
   for (i = 0; i < nat; i++) {
      if(i < 5 || i > nat - 5) {
         fprintf(logfx,"dt[%d]=",i);
      }
      for (int j = 0; j < 34; j++) {
         if(i < 5 || i > nat - 5) {
            fprintf(logfx,"%d ",dt[i].cor[j]);
         }
         cks += (double)dt[i].cor[j];
      }
      if(i < 5 || i > nat - 5) {
         fprintf(logfx,"\n");
      }
   }
   dww("initarray 1");
#endif

   vt = 0;

#if defined(DEBUG_FILES)
   mol = us_fopen(molecola, "r");
   if ( 1 == fscanf(mol, "%d", &nat) ) {};
   if ( 1 == fscanf(mol, "%f", &raggio) ) {};
   printf("!! opening molecola == '%s' nat == %d raggio == %f\n",
          molecola, nat, raggio);
#endif
   raggio = -2.0;

   if (raggio == 0.0)      /* Variable hydrated radii only */
   {
      if ( 1 == fscanf(mol, "%s", ragcol) ) {};

      rmc = us_fopen(ragcol, "r");

      for (i = 0; i < nat; i++)
      {
         if ( 1 == fscanf(mol, "%f", &dt[i].x) ) {};
         if ( 1 == fscanf(mol, "%f", &dt[i].y) ) {};
         if ( 1 == fscanf(mol, "%f", &dt[i].z) ) {};
         if ( 1 == fscanf(rmc, "%f", &dt[i].r) ) {};
         if ( 1 == fscanf(rmc, "%f", &dt[i].m) ) {};
         if ( 1 == fscanf(rmc, "%d", &dt[i].col) ) {};
      }
#if defined(DEBUG_WW)
      for (i = 0; i < nat; i++) {
         cks += (double)dt[i].x;
         cks += (double)dt[i].y;
         cks += (double)dt[i].z;
         cks += (double)dt[i].r;
         cks += (double)dt[i].m;
         cks += (double)dt[i].col;
      }
      dww("initarray 2");
#endif

      fclose(rmc);
      goto a1200;
   }

   if (raggio == -1.0)      /* Variable hydrated and unhydrated radii */
   {

      if ( 1 == fscanf(mol, "%s", ragcol) ) {};

      rmc = us_fopen(ragcol, "r");

      for (i = 0; i < nat; i++)
      {
         if ( 1 == fscanf(mol, "%f", &dt[i].x) ) {};
         if ( 1 == fscanf(mol, "%f", &dt[i].y) ) {};
         if ( 1 == fscanf(mol, "%f", &dt[i].z) ) {};
         if ( 1 == fscanf(rmc, "%f", &dt[i].r) ) {};
         if ( 1 == fscanf(rmc, "%f", &dt[i].ru) ) {};
         if ( 1 == fscanf(rmc, "%f", &dt[i].m) ) {};
         if ( 1 == fscanf(rmc, "%d", &dt[i].col) ) {};
      }
      fclose(rmc);
      goto a1200;
   }

   if (raggio == -2.0)      /* Variable hydrated radii and part. spec. vol. */
   {

#if defined(DEBUG_FILES)
      if ( 1 == fscanf(mol, "%s", ragcol) ) {};

      if ( 1 == fscanf(mol, "%f", &partvol) ) {};
      printf("!! partvol %f vbar %f %s %f\n", 
             partvol, (*model_vector)[model_idx[active_model]].vbar,
             us_hydrodyn->misc.compute_vbar ? "computed-vbar" : "user vbar",
             us_hydrodyn->misc.vbar
             );
      rmc = us_fopen(ragcol, "r");
#endif

      org_vbar = (*model_vector)[model_idx[active_model]].vbar;
      partvol = (int)((
                       (
                        (*model_vector)[model_idx[active_model]].vbar +
                        (4.25e-4 * (TE - K20))
                        )
                       * 1000) + 0.5) / 1000.0;

      if (!us_hydrodyn->misc.compute_vbar) {
         org_vbar = us_hydrodyn->misc.vbar;
         partvol = (int)((
                          (
                           us_hydrodyn->misc.vbar -
                           (4.25e-4 * (K0 + us_hydrodyn->misc.vbar_temperature - K20)) +
                           (4.25e-4 * (TE - K20))
                           )
                          * 1000) + 0.5) / 1000.0;
      }
      tc_vbar = partvol;
      tot_partvol += partvol;

      printf("psv = %f\n", partvol);


      int decpts = -(int)log10(us_hydrodyn->overlap_tolerance/9.9999) + 1;
      if (decpts < 4) {
         decpts = 4;
      }
      int decpow = (int)pow(10.0, (decpts));
      printf("!!rounding to %d digits (%d)\n", decpts, decpow);

      int mw_c = 0;
      float pre_mw = 0.0;
      float post_mw = 0.0;
      double dpre_mw = 0e0;
      double dpost_mw = 0e0;

      for (i = 0; i < nat; i++)
      {
         dt[i].x = (*bead_models)[model_idx[active_model]][active_idx[active_model][i]].bead_coordinate.axis[0];
         dt[i].y = (*bead_models)[model_idx[active_model]][active_idx[active_model][i]].bead_coordinate.axis[1];
         dt[i].z = (*bead_models)[model_idx[active_model]][active_idx[active_model][i]].bead_coordinate.axis[2];
         dt[i].r = (*bead_models)[model_idx[active_model]][active_idx[active_model][i]].bead_computed_radius;
         dt[i].m = (*bead_models)[model_idx[active_model]][active_idx[active_model][i]].bead_ref_mw;
         dt[i].m += (*bead_models)[model_idx[active_model]][active_idx[active_model][i]].bead_ref_ionized_mw_delta;
         dt[i].col = us_hydrodyn->get_color(&((*bead_models)[model_idx[active_model]][active_idx[active_model][i]]));
         dt[i].x = ((int)((dt[i].x * decpow) + (dt[i].x > 0 ? 0.5 : -0.5))) / (float)decpow;
         dt[i].y = ((int)((dt[i].y * decpow) + (dt[i].y > 0 ? 0.5 : -0.5))) / (float)decpow;
         dt[i].z = ((int)((dt[i].z * decpow) + (dt[i].z > 0 ? 0.5 : -0.5))) / (float)decpow;
         dt[i].r = ((int)((dt[i].r * decpow) + (dt[i].r > 0 ? 0.5 : -0.5))) / (float)decpow;
         if ( us_hydrodyn->advanced_config.debug_1 )
         {
            pre_mw += dt[i].m;
            dpre_mw += dt[i].m;
         }
         dt[i].m = (float)((int)((double)dt[i].m * 100e0 + 5e-1)) / 1e2;
         post_mw += dt[i].m;
         if ( us_hydrodyn->advanced_config.debug_1 )
         {
            dpost_mw += ((int)((double)dt[i].m * 100e0 + 5e-1)) / 1e2;
            mw_c++;
         }
#if defined(DEBUG_FILES)
         {
            float fx, fy, fz, fr, fm;
            int fc;
            if ( 1 == fscanf(mol, "%f", &fx) ) {};
            if ( 1 == fscanf(mol, "%f", &fy) ) {};
            if ( 1 == fscanf(mol, "%f", &fz) ) {};
            if ( 1 == fscanf(rmc, "%f", &fr) ) {};
            if ( 1 == fscanf(rmc, "%f", &fm) ) {};
            if ( 1 == fscanf(rmc, "%d", &fc) ) {};
            printf("!!(%f %f %f) %f %f %f %f %f %d\n",
                   (*bead_models)[model_idx[active_model]][active_idx[active_model][i]].bead_coordinate.axis[0],
                   dt[i].x, fx,
                   dt[i].x - fx,
                   dt[i].y - fy,
                   dt[i].z - fz,
                   dt[i].r - fr,
                   dt[i].m - fm,
                   dt[i].col - fc
                   ); fflush(stdout);
         }
#endif
      }
      if ( us_hydrodyn->advanced_config.debug_1 )
      {
         printf("hydro pre mw %.6f (%d), post mw %.6f (%d), diff %.6f\n", pre_mw, mw_c, post_mw, mw_c, pre_mw - post_mw);
         printf("as d hydro pre mw %.6f (%d), post mw %.6f (%d), diff %.6f\n", dpre_mw, mw_c, dpost_mw, mw_c, dpre_mw - dpost_mw);
      }
#if defined(DEBUG_FILES)
      fclose(rmc);
#endif
      goto a1200;
   }

   if (raggio == -3.0)      /* Variable hydrated and unhydrated radii and psv */
   {

      if ( 1 == fscanf(mol, "%s", ragcol) ) {};

      if ( 1 == fscanf(mol, "%f", &partvol) ) {};

      rmc = us_fopen(ragcol, "r");

      for (i = 0; i < nat; i++)
      {
         if ( 1 == fscanf(mol, "%f", &dt[i].x) ) {};
         if ( 1 == fscanf(mol, "%f", &dt[i].y) ) {};
         if ( 1 == fscanf(mol, "%f", &dt[i].z) ) {};
         if ( 1 == fscanf(rmc, "%f", &dt[i].r) ) {};
         if ( 1 == fscanf(rmc, "%f", &dt[i].ru) ) {};
         if ( 1 == fscanf(rmc, "%f", &dt[i].m) ) {};
         if ( 1 == fscanf(rmc, "%d", &dt[i].col) ) {};
      }

      fclose(rmc);
      goto a1200;
   }

   if (raggio == -4.0)      /* Variable hydrated radii and bead-aminoacids correspondence */
   {

      if ( 1 == fscanf(mol, "%s", ragcol) ) {};

      rmc = us_fopen(ragcol, "r");

      for (i = 0; i < nat; i++)
      {
         if ( 1 == fscanf(mol, "%f", &dt[i].x) ) {};
         if ( 1 == fscanf(mol, "%f", &dt[i].y) ) {};
         if ( 1 == fscanf(mol, "%f", &dt[i].z) ) {};
         if ( 1 == fscanf(rmc, "%f", &dt[i].r) ) {};
         if ( 1 == fscanf(rmc, "%f", &dt[i].m) ) {};
         if ( 1 == fscanf(rmc, "%d", &dt[i].col) ) {};
         dt[i].cor = (char *) malloc(10 * sizeof(char));
         if ( 1 == fscanf(rmc, "%s", dt[i].cor) ) {};
      }

      fclose(rmc);
      goto a1200;
   }

   if (raggio == -5.0)      /* Variable hydrated radii, bead-aminoacids correspondence and psv */
   {

      if ( 1 == fscanf(mol, "%s", ragcol) ) {};

      if ( 1 == fscanf(mol, "%f", &partvol) ) {};

      rmc = us_fopen(ragcol, "r");

      for (i = 0; i < nat; i++)
      {
         if ( 1 == fscanf(mol, "%f", &dt[i].x) ) {};
         if ( 1 == fscanf(mol, "%f", &dt[i].y) ) {};
         if ( 1 == fscanf(mol, "%f", &dt[i].z) ) {};
         if ( 1 == fscanf(rmc, "%f", &dt[i].r) ) {};
         if ( 1 == fscanf(rmc, "%f", &dt[i].m) ) {};
         if ( 1 == fscanf(rmc, "%d", &dt[i].col) ) {};
         dt[i].cor = (char *) malloc(10 * sizeof(char));
         if ( 1 == fscanf(rmc, "%s", dt[i].cor) ) {};
      }

      fclose(rmc);
      goto a1200;
   }

   else         /* Constant radius, mass and color */
   {
      vt = 1;
      for (i = 0; i < nat; i++)
      {
         if ( 1 == fscanf(mol, "%f", &dt[i].x) ) {};
         if ( 1 == fscanf(mol, "%f", &dt[i].y) ) {};
         if ( 1 == fscanf(mol, "%f", &dt[i].z) ) {};
         dt[i].r = raggio;
         dt[i].m = (float) 1.0;
         dt[i].col = 1;
      }
   }

 a1200:

#if defined(DEBUG_FILES)
   fclose(mol);
#endif

   if ((raggio == -1.0) || (raggio == -3.0))
   {
      partvolc1 = 0.0;
      partvolc2 = 0.0;
      for (i = 0; i < nat; i++)
      {
         partvolc1 = partvolc1 + (4.0 / 3.0 * M_PI * dt[i].ru * dt[i].ru * dt[i].ru * fconv1 * fconv1 * fconv1 * 1E-21);
         partvolc2 = partvolc2 + dt[i].m;
      }
      partvolc = partvolc1 * AVO / partvolc2;
   }

   if ((prima == 1) && (ultima == nat));
   else
   {
      if ((raggio == 0.0) || (raggio == -2.0))
         for (i = 0; i < nat; i++)
         {
            dt[i].x = dt[i + prima - 1].x;
            dt[i].y = dt[i + prima - 1].y;
            dt[i].z = dt[i + prima - 1].z;
            dt[i].r = dt[i + prima - 1].r;
            dt[i].m = dt[i + prima - 1].m;
            dt[i].col = dt[i + prima - 1].col;
         }
      if ((raggio == -1.0) || (raggio == -3.0))
         for (i = 0; i < nat; i++)
         {
            dt[i].x = dt[i + prima - 1].x;
            dt[i].y = dt[i + prima - 1].y;
            dt[i].z = dt[i + prima - 1].z;
            dt[i].r = dt[i + prima - 1].r;
            dt[i].m = dt[i + prima - 1].m;
            dt[i].ru = dt[i + prima - 1].ru;
            dt[i].col = dt[i + prima - 1].col;
         }
      if ((raggio == -4.0) || (raggio == -5.0))
         for (i = 0; i < nat; i++)
         {
            dt[i].x = dt[i + prima - 1].x;
            dt[i].y = dt[i + prima - 1].y;
            dt[i].z = dt[i + prima - 1].z;
            dt[i].r = dt[i + prima - 1].r;
            dt[i].m = dt[i + prima - 1].m;
            dt[i].col = dt[i + prima - 1].col;
            dt[i].cor = dt[i + prima - 1].cor;
         }

   }

   strcpy(molecola, ricorda);
   numero_sfere = nat;
   nat = ultima - prima + 1;
   //    editor->append(QString("initarray - 1 (ultima - prima): %1 beads\n").arg(nat));

   printf("\n\n Starting function: ragir()\n");
   ragir();
   printf("\n End of function: ragir()\n");

   /* removing from the hydrodynamic computations the beads color-coded '0' */

   // for (i = 0; i < nat; i++) {
   //      printf("bead %d col %d\n", i, dt[i].col);
   // if(dt[i].col == 6) {
   //   count6++;
   //      }
   //    }

   {
      j = 0;
      for (i = 0; i < nat; i++)
      {
         if (dt[i].col == 0);

         else

         {
            dt[j] = dt[i];
            j++;
         }

      }
      colorzero = nat - j;
      nat = j;
   }

   //    editor->append(QString("initarray - 2 (remove cc 0): %1 beads\n").arg(nat));

   if (sfecalc == 2)      /* computation with all beads or with only the 'exposed'                           beads */

   {
      j = 0;
      interm1 = 0;
      for (i = 0; i < nat; i++)
      {
         /*     if(colorsixf==1)     */
         interm1 += pow(dt[i].r, 3) * M_PI * 8.0 * ETAo;   /* calculating the total volume correction of the beads, including the buried ones */

         if (dt[i].col == 6);

         else

         {
            dt[j] = dt[i];
            j++;
         }

      }
      colorsix = nat - j;
      nat = j;
   }

   //    editor->append(QString("initarray - 3 (only exposed): %1 beads (count6 == %2)\n").arg(nat).arg(count6));

   if (nat > nmax)
   {
      printf("\n%s%d%s", " TOO MANY BEADS to COMPUTE ! (max: ", nmax, ")\n");
      exit(0);
   }
#if defined(DEBUG_WW)
   for (i = 0; i < nat; i++) {
      cks += (double)dt[i].x;
      cks += (double)dt[i].y;
      cks += (double)dt[i].z;
      cks += (double)dt[i].r;
      cks += (double)dt[i].m;
      cks += (double)dt[i].col;
   }
   dww("initarray 3");
#endif
   // qDebug() << QString("supc::initarray() end: %1 beads of %2\n").arg(nat).arg( numero_sfere );

}

/*******************************************************************/

// inv.c

static void
inv(float r[9])
{
   // int i;

   float deter, rdet;
   float cofa1, cofa2, cofa3, cofa4, cofa5, cofa6, cofa7, cofa8, cofa9;

   cofa1 = r[4] * r[8] - r[5] * r[7];
   cofa2 = r[5] * r[6] - r[3] * r[8];
   cofa3 = r[3] * r[7] - r[4] * r[6];
   cofa4 = r[0] * r[8] - r[2] * r[6];
   cofa5 = r[1] * r[6] - r[0] * r[7];
   cofa6 = r[0] * r[4] - r[1] * r[3];
   cofa7 = r[7] * r[2] - r[1] * r[8];
   cofa8 = r[1] * r[5] - r[4] * r[2];
   cofa9 = r[3] * r[2] - r[0] * r[5];
   deter = r[0] * cofa1 + r[1] * cofa2 + r[2] * cofa3;

   if (fabs(deter) == 0.0)
      printf("SINGULAR MATRIX\n");
   else
   {

      rdet = 1.0 / deter;
      r[0] = cofa1 * rdet;
      r[3] = cofa2 * rdet;
      r[6] = cofa3 * rdet;
      r[4] = cofa4 * rdet;
      r[7] = cofa5 * rdet;
      r[8] = cofa6 * rdet;
      r[1] = cofa7 * rdet;
      r[2] = cofa8 * rdet;
      r[5] = cofa9 * rdet;
   }

}

static void
inv6x6(float a[6][6])
{

   float a1[5][5], a2[4][4];
   float det, c1, elem, elem1, k1;
   int i, j, i1, i2, j1, j2, j3;

   det = 0.0;
   k1 = 0.0;
   for (i = 0; i < 6; i++)
   {
      for (j = 0; j < 6; j++)
      {
         for (i1 = 0; i1 < 5; i1++)
         {
            i2 = i1;
            if (i1 >= i)
               i2 = i1 + 1;
            for (j1 = 0; j1 < 5; j1++)
            {
               j2 = j1;
               if (j1 >= j)
                  j2 = j1 + 1;
               a1[i1][j1] = a[i2][j2];
            }
         }

         det = 0.0;
         for (j1 = 0; j1 < 5; j1++)
         {

            for (i2 = 0; i2 < 4; i2++)
            {
               for (j2 = 0; j2 < 4; j2++)
               {
                  j3 = j2;
                  if (j2 >= j1)
                     j3 = j2 + 1;
                  a2[i2][j2] = a1[i2 + 1][j3];
               }
            }

            c1 = a2[0][0] * (a2[1][1] * (a2[2][2] * a2[3][3] - a2[2][3] * a2[3][2])
                             - a2[1][2] * (a2[2][1] * a2[3][3] - a2[2][3] * a2[3][1])
                             + a2[1][3] * (a2[2][1] * a2[3][2] - a2[2][2] * a2[3][1]));
            c1 = c1 - a2[0][1] * (a2[1][0] * (a2[2][2] * a2[3][3] - a2[2][3] * a2[3][2])
                                  - a2[1][2] * (a2[2][0] * a2[3][3] - a2[3][0] * a2[2][3])
                                  + a2[1][3] * (a2[2][0] * a2[3][2] - a2[3][0] * a2[2][2]));
            c1 = c1 + a2[0][2] * (a2[1][0] * (a2[2][1] * a2[3][3] - a2[3][1] * a2[2][3])
                                  - a2[1][1] * (a2[2][0] * a2[3][3] - a2[2][3] * a2[3][0])
                                  + a2[1][3] * (a2[2][0] * a2[3][1] - a2[2][1] * a2[3][0]));
            c1 = c1 - a2[0][3] * (a2[1][0] * (a2[2][1] * a2[3][2] - a2[2][2] * a2[3][1])
                                  - a2[1][1] * (a2[2][0] * a2[3][2] - a2[3][0] * a2[2][2])
                                  + a2[1][2] * (a2[2][0] * a2[3][1] - a2[2][1] * a2[3][0]));

            elem = 1.0;
            if (j1 == 1)
               elem = (-1.0);
            if (j1 == 3)
               elem = (-1.0);

            det = det + c1 * elem * a1[0][j1];

         }

         elem1 = (-1.0);
         for (j2 = 0; j2 <= i + j; j2++)
            elem1 = elem1 * (-1.0);
         inver[j][i] = det * elem1;
         if (i == 0)
            k1 = k1 + det * elem1 * a[i][j];
      }
   }

   for (i = 0; i < 6; i++)
   {
      for (j = 0; j < 6; j++)
         inver[j][i] = inver[j][i] / k1;
   }

}

/**************************************************************************/

static void
inverti(int N)
{

   int i, j, k1, k2, k3, k4, k5;

   printf("MATRIX DECOMPOSITION (Chol.Dec.)\n\n");

#if defined(SHOW_TIMING)
   gettimeofday(&s2tv0, NULL);
#endif

   choldc(N); // cycle 2

#if defined(SHOW_TIMING)
   gettimeofday(&s2tv9, NULL);
   supc2_tot = 1000000l * (s2tv9.tv_sec - s2tv0.tv_sec) + s2tv9.tv_usec - s2tv0.tv_usec;
#endif

   if (us_hydrodyn->stopFlag)
   {
      return;
   }
   printf("\n\nINVERSION BY COLUMNS\n\n");

   inizializza_b1();
   b1[0] = 1.0;

   printf("Cycle 3 of 3; model %d of %d\n\n", kkk, num);
   editor->append("Supermatrix inversion Cycle 3 of 3\n");
   qApp->processEvents();

#if defined(SHOW_TIMING)
      gettimeofday(&s3tv0, NULL);
#endif
      
   for (j = 1; j <= 3 * N; j++)
   {
      smi_progress->setValue(ppos++);
      qApp->processEvents();
      if (us_hydrodyn->stopFlag)
      {
         return;
      }

      printf("%s%d%s%d", "Iteration  ", j, " of ", 3 * N);
      fflush(stdout);
      us_hydrodyn->lbl_core_progress->setText(QString("Iteration %1 of %2")
                                              .arg(j)
                                              .arg(3 * N));

      k1 = (int) (floor((j - 1) / 3.0));
      k2 = (j - 1) % 3;

      if (j != 1)
      {
         b1[j - 2] = 0.0;
         b1[j - 1] = 1.0;
      }

      cholsl(N);
#if defined(SHOW_TIMING)
      gettimeofday(&s3tv7, NULL);
#endif

      for (i = 1; i <= 3 * N; i++)
      {
         k3 = (int) (floor((i - 1) / 3.0));
         k4 = (i - 1) % 3;
         k5 = k2 + 3 * k4;
         q[k3 * (nat * 9) + k1 * 9 + k5] = rRi[i - 1];
      }

      printf("%c", '\r');
#if defined(SHOW_TIMING)
      gettimeofday(&s3tv8, NULL);
      supc3_s2 = 1000000l * (s3tv8.tv_sec - s3tv7.tv_sec) + s3tv8.tv_usec - s3tv7.tv_usec;
#endif
   }
#if defined(SHOW_TIMING)
   gettimeofday(&s3tv9, NULL);
   supc3_s1 = cholsl_s1 + cholsl_s2;
   supc3_tot = 1000000l * (s3tv9.tv_sec - s3tv0.tv_sec) + s3tv9.tv_usec - s3tv0.tv_usec;
   printf("supc timing:\n"
          "cholsl loop 1 %lu %.2g\n"
          "cholsl loop 2 %lu %.2g\n"
          "supc3 s1      %lu %.2g\n"
          "supc3 s2      %lu %.2g\n"
          "supc3 tot     %lu\n"
          , cholsl_s1, (double) cholsl_s1 / (double) supc3_tot
          , cholsl_s2, (double) cholsl_s2 / (double) supc3_tot
          , supc3_s1, (double) supc3_s1 / (double) supc3_tot
          , supc3_s2, (double) supc3_s2 / (double) supc3_tot
          , supc3_tot
          );
#endif

   printf("\n");
}

/**************************************************************************/

/* int test; */
// extern float interm1;
// extern int colorsixf;
static void
DDtcalc()
{
   int i;
   float p1[9], p2[9], p3[9];

   vxTl(roD, Dr);
   Txv(vT, roD);

   for (i = 0; i < 9; i++)
      p1[i] = Tv[i];

   vxT(roD, Doc);

   for (i = 0; i < 9; i++)
      p2[i] = vT[i];

   Doct[0] = Doc[0];
   Doct[1] = Doc[3];
   Doct[2] = Doc[6];
   Doct[3] = Doc[1];
   Doct[4] = Doc[4];
   Doct[5] = Doc[7];
   Doct[6] = Doc[2];
   Doct[7] = Doc[5];
   Doct[8] = Doc[8];

   Txv(Doct, roD);

   for (i = 0; i < 9; i++)
      p3[i] = Tv[i];

   for (i = 0; i < 9; i++)
      DDt[i] = Dot[i] - p1[i] - p2[i] + p3[i];

}

/**************** computation of the matrix SIGMAOR ***************************/

static void
sigmaoRcalc()
{
   int i;
   float p1[9], p2[9], p3[9];

   vxT(roR, sigmat);
   Txv(vT, roR);

   for (i = 0; i < 9; i++)
      p1[i] = Tv[i];

   Txv(sigmaoc, roR);

   for (i = 0; i < 9; i++)
      p2[i] = Tv[i];

   soct[0] = sigmaoc[0];
   soct[1] = sigmaoc[3];
   soct[2] = sigmaoc[6];
   soct[3] = sigmaoc[1];
   soct[4] = sigmaoc[4];
   soct[5] = sigmaoc[7];
   soct[6] = sigmaoc[2];
   soct[7] = sigmaoc[5];
   soct[8] = sigmaoc[8];

   vxT(roR, soct);

   for (i = 0; i < 9; i++)
      p3[i] = vT[i];

   for (i = 0; i < 9; i++)
      sigmaoR[i] = sigmaRr[i] + p1[i] - p2[i] + p3[i];

}

/***************** computation ofthe matrix SIGMAOC ***************************/

#if defined(NOT_USED)
static void
sigmaocalc()
{
   int i, j;

   for (i = 0; i < 9; i++)
   {
      sigmaoc[i] = 0.0;
   }

   for (i = 0; i < 3; i++)
   {
      for (j = 0; j < nat; j++)
      {
         sigmaoc[i] += dt[j].r * (gp[j * 9 + 6 + i] * dt[j].y - gp[j * 9 + 3 + i] * dt[j].z);   /* manca 6*PI*ETAo */
         sigmaoc[i + 3] += dt[j].r * (gp[j * 9 + i] * dt[j].z - gp[j * 9 + 6 + i] * dt[j].x);
         sigmaoc[i + 6] += dt[j].r * (gp[j * 9 + 3 + i] * dt[j].x - gp[j * 9 + i] * dt[j].y);
      }

   }
}
#endif

/************************************************************************/

static void
sigmaocalc1()
{
   int i, j, k;

   for (i = 0; i < 9; i++)
   {
      sigmaoc[i] = 0.0;
   }

   for (k = 0; k < nat; k++)
   {
      for (j = 0; j < nat; j++)
      {
         for (i = 0; i < 3; i++)
         {
            sigmaoc[i] += (q[k * (nat * 9) + j * 9 + 6 + i] * dt[k].y - q[k * (nat * 9) + j * 9 + 3 + i] * dt[k].z);
            sigmaoc[i + 3] += (q[k * (nat * 9) + j * 9 + i] * dt[k].z - q[k * (nat * 9) + j * 9 + 6 + i] * dt[k].x);
            sigmaoc[i + 6] += (q[k * (nat * 9) + j * 9 + 3 + i] * dt[k].x - q[k * (nat * 9) + j * 9 + i] * dt[k].y);
         }
      }
   }
}

/************************************************************************/
#if defined(NOT_USED)
static void
sigmatcalc1()
{

   int i, j, k;

   for (i = 0; i < nat; i++)
   {
      for (k = 0; k < 9; k++)
         gp[i * 9 + k] = 0.0;

      for (j = 0; j < nat; j++)
      {
         for (k = 0; k < 9; k++)
            gp[i * 9 + k] += q[i * (nat * 9) + j * 9 + k] * dt[j].r;
      }

      for (k = 0; k < 9; k++)
         gp[i * 9 + k] = (1.0 / dt[i].r) * gp[i * 9 + k];
   }

   for (k = 0; k < 9; k++)
   {
      sigmat[k] = 0.0;

      for (i = 0; i < nat; i++)
         sigmat[k] += dt[i].r * gp[i * 9 + k];

      isigmat[k] = sigmat[k];
   }

   inv(isigmat);
   f = 3.0 / (isigmat[0] + isigmat[4] + isigmat[8]);

}
#endif

/************************************************************************/

static void
sigmatcalc2()
{

   int i, j, k;

   for (k = 0; k < 9; k++)
      sigmat[k] = 0.0;

   for (i = 0; i < nat; i++)
   {
      for (j = 0; j < nat; j++)
      {
         for (k = 0; k < 9; k++)
            sigmat[k] += q[i * (nat * 9) + j * 9 + k];
      }
   }

   for (k = 0; k < 9; k++)
      isigmat[k] = sigmat[k];

   inv(isigmat);
   f = 3.0 / (isigmat[0] + isigmat[4] + isigmat[8]);

}

/************************************************************************/

static void
sigmarRcalc1()
{

   int l, k, i, j, kmeno, kpiu, lmeno, lpiu;
   float xi[2], xj[2], interm;

   for (k = 0; k < 3; k++)
   {

      kmeno = k - 1;
      if (k == 0)
         kmeno = 2;

      kpiu = k + 1;
      if (k == 2)
         kpiu = 0;

      for (l = 0; l < 3; l++)
      {

         sigmaRr[3 * k + l] = 0.0;

         lmeno = l - 1;
         if (l == 0)
            lmeno = 2;

         lpiu = l + 1;
         if (l == 2)
            lpiu = 0;

         for (i = 0; i < nat; i++)
         {

            xi[0] = rRi[3 * i + kpiu];
            xi[1] = rRi[3 * i + kmeno];

            for (j = 0; j < nat; j++)
            {

               xj[0] = rRi[3 * j + lpiu];
               xj[1] = rRi[3 * j + lmeno];

               interm = xi[0] * xj[0] * q[i * (nat * 9) + j * 9 + 3 * kmeno + lmeno];
               interm = interm + xi[1] * xj[1] * q[i * (nat * 9) + j * 9 + 3 * kpiu + lpiu];
               interm = interm - xi[0] * xj[1] * q[i * (nat * 9) + j * 9 + 3 * kmeno + lpiu];
               interm = interm - xi[1] * xj[0] * q[i * (nat * 9) + j * 9 + 3 * kpiu + lmeno];

               /*          interm=dt[j].r*interm;   */
               sigmaRr[3 * k + l] += interm;

            }

         }
      }
   }

   QString hydro_msg = "";
   interm = 0.0;
   /*      printf("%d%d\n\n volcor,colorsixf",volcor,"   ",colorsixf);
           if ( 1 == scanf("%d",&test) ) {};
           getchar();  */
   if ((volcor == 1) && ((colorsixf == 1) || (colorsixf == 3))) {
      interm = interm1;   /* total vol. of beads, buried included */
      hydro_msg = "Using total volume of all beads including buried for eq 11. ";
   } else {
      if (volcor == 1)
      {
         hydro_msg = "Using total volume of all exposed beads for eq 11. ";
         for (i = 0; i < nat; i++)
            interm += pow(dt[i].r, 3) * M_PI * 8.0 * ETAo;
      } else {
         hydro_msg = "Using manual volume for eq 11. ";
         interm = volcor1 * 6.0 * ETAo;   /* manual volume correction */
      }
   }


   if ( hydro_use_avg_for_volume ) {
   // when we tested average
   // hydro_msg = "Using average volume of all beads including buried for eq 11. ";
   // interm /= numero_sfere;
      hydro_msg = "Using the minimum of ASA volume and total volume of used beads for eq 11. ";
      QTextStream( stdout ) << QString().sprintf(
                                                 "active model : %d\n"
                                                 "model_idx    : %d\n"
                                                 ,active_model
                                                 ,model_idx[active_model]
                                                 );
      interm = asa_vol[ active_model ] * 6.0 * ETAo;
   }
   
   for (i = 0; i < 3; i++)
   {
      if (cc == 1)
         sigmaRr[4 * i] += interm;
      else
         sigmaRr[4 * i] += 0.0;
   }

   if (cc == 1)
      volcor1 = interm / 6.0 / ETAo;
   else
      volcor1 = 0.0;

   hydro_msg += QString( "\nCorrection volume value %1 [A^3]." ).arg( volcor1 );
   us_hydrodyn->editor_msg( "dark blue", hydro_msg );
   // qDebug() << "supc::sigmarRcalc1() volcor1 interm  " << volcor1 << " " << interm;
   // qDebug() << "supc::sigmarRcalc1() nat numero_sfere" << nat << " " << numero_sfere;
}

/*************************************************************************/
// over.c

static int
overlap()
{
   int i, j, flag;
   float dist;
   // float overlval;
#if defined(USE_MAIN)
   char r5;
#endif

   flag = 0;

   printf("\n** Performing overlap test **\n");
   printf("\n** WARNING: Overlaps can lead to wrong results! Detection cutoff > %f **\n", overlap_tolerance);

   int decpts = -(int)log10(us_hydrodyn->overlap_tolerance/9.9999) + 1;
   if (decpts < 4) {
      decpts = 4;
   }
   int decpow = (int)pow(10.0, (decpts));

   for (i = 0; i < nat; i++)
   {
      for (j = i + 1; j < nat; j++)
      {
         dist = pow((dt[i].x - dt[j].x), 2) + pow((dt[i].y - dt[j].y), 2) + pow((dt[i].z - dt[j].z), 2);
         // overlval = (dist - pow((dt[i].r + dt[j].r), 2));
         // if (dist - pow((dt[i].r + dt[j].r), 2)) < -0.01)
         float diff = sqrt(dist) - (dt[i].r + dt[j].r);
         diff = ((int)((diff * decpow) + (diff > 0 ? 0.5 : -0.5))) / (float)decpow;
         if ( diff < - overlap_tolerance * 1.04 ) 
         {
            printf("\n%s%d%s%d%s%.6f\n", "Notice: Overlap among bead ", i + 1, " and bead ", j + 1, ". Value = ",
                   -(sqrt(dist) - (dt[i].r + dt[j].r)));
         }
         if ( diff < -( ( 0.001 + overlap_tolerance ) * 1.04) )
         {
            editor->append(QString("").sprintf("\n%s%d%s%d%s%.6f\n", "ERROR: Overlap among bead ", i + 1, " and bead ", j + 1, ". Value = ",
                                               -(sqrt(dist) - (dt[i].r + dt[j].r))));
            printf("\n%s%d%s%d%s%.6f\n", "OVERLAP AMONG BEAD ", i + 1, " and BEAD ", j + 1, " | Value = ",
                   (sqrt(dist) - (dt[i].r + dt[j].r)));
#if defined(USE_MAIN)
            printf("\n** Do you want to proceed anyway? (y/n) ");
            if ( 1 == scanf("%s", &r5) ) {};
            getchar();
            if ((r5 == 'y') || (r5 == 'Y'))
            {
               flag = 0;
            }
            else
            {
               flag = 1;
               goto a99;
            }
#else
            return (1);
#endif
         }

      }
   }
   printf("\n** Overlap test completed **\n");
#if defined(USE_MAIN)
 a99:
#endif
   return (flag);
}

// pvt.c

static void
vxT(float v[3], float T[9])
{
   int i;

   for (i = 0; i < 3; i++)
      vT[i] = T[i + 6] * v[1] - T[i + 3] * v[2];

   for (i = 3; i < 6; i++)
      vT[i] = T[i - 3] * v[2] - T[i + 3] * v[0];

   for (i = 6; i < 9; i++)
      vT[i] = T[i - 3] * v[0] - T[i - 6] * v[1];
}

static void
vxTl(float v[3], long double T[9])
{
   int i;

   for (i = 0; i < 3; i++)
      vT[i] = T[i + 6] * v[1] - T[i + 3] * v[2];

   for (i = 3; i < 6; i++)
      vT[i] = T[i - 3] * v[2] - T[i + 3] * v[0];

   for (i = 6; i < 9; i++)
      vT[i] = T[i - 3] * v[0] - T[i - 6] * v[1];
}

static void
Txv(float T[9], float v[3])
{
   int i;

   for (i = 0; i < 3; i++)
   {
      Tv[3 * i] = T[3 * i + 1] * v[2] - T[3 * i + 2] * v[1];
      Tv[3 * i + 1] = T[3 * i + 2] * v[0] - T[3 * i] * v[2];
      Tv[3 * i + 2] = T[3 * i] * v[1] - T[3 * i + 1] * v[0];
   }
}

static void
Mxv(float M[9], float v[3])
{
   int i;

   for (i = 0; i < 3; i++)
   {
      vc[i] = M[3 * i] * v[0] + M[3 * i + 1] * v[1] + M[3 * i + 2] * v[2];
   }
}

// qij.c

static void
calcqij()
{
   int i, j, k, l, inx;
   float qqx, qqy, qqz, del, aa, ri;
   // float rj;
   float d2, qq, rr, tt;
   // float tempij[3];
   float tempji[3], dij, etai;

   del = 0.0;

   for (i = 0; i < nat; i++)
   {
      for (j = i; j < nat; j++)
      {
         ri = dt[i].r;
         // rj = dt[j].r;

         if (cc == 1)   /* Corrected 9 Oct 1997 MR */
            etai = 6.0 * M_PI * ETAo * ri;
         else
            etai = 4 * M_PI * ETAo * ri;

         aa = 0.0;

         qqx = dt[i].x - dt[j].x;
         aa += qqx * qqx;
         tempji[0] = (-qqx);
         // tempij[0] = qqx;

         qqy = dt[i].y - dt[j].y;
         aa += qqy * qqy;
         tempji[1] = (-qqy);
         // tempij[1] = qqy;

         qqz = dt[i].z - dt[j].z;
         aa += qqz * qqz;
         tempji[2] = (-qqz);
         // tempij[2] = qqz;

         aa = sqrt(aa);
         dij = aa;

         for (k = 0; k < 3; k++)
         {
            for (l = 0; l < 3; l++)
            {
               inx = l + 3 * k;
               if (k == l)
                  del = 1.0;
               else
                  del = 0.0;

               if (i == j)
                  q[i * (nat * 9) + j * 9 + inx] = del / etai;
               else
               {
                  d2 = dij * dij;
                  qq = (tempji[l] * tempji[k]) / d2;
                  rr = (dt[i].r * dt[i].r + dt[j].r * dt[j].r);
                  tt = del + qq + (rr / d2) * (del / 3.0 - qq);
                  tt = tt / (dij * 8.0 * M_PI * ETAo);
                  q[i * (nat * 9) + j * 9 + inx] = tt;
                  q[j * (nat * 9) + i * 9 + inx] = tt;
               }
            }
         }
      }
   }

}

// rot.c

static void
calcR()
{

   float temp[9];
   int i;

   temp[0] = sigmat[4] + sigmat[8];
   temp[1] = (-sigmat[1]);
   temp[2] = (-sigmat[2]);
   temp[3] = (-sigmat[1]);
   temp[4] = sigmat[0] + sigmat[8];
   temp[5] = (-sigmat[5]);
   temp[6] = (-sigmat[2]);
   temp[7] = (-sigmat[5]);
   temp[8] = sigmat[0] + sigmat[4];

   inv(temp);

   for (i = 0; i < 3; i++)
      roR[i] =
         temp[3 * i] * (sigmaoc[7] - sigmaoc[5]) + temp[3 * i + 1] * (sigmaoc[2] - sigmaoc[6]) + temp[3 * i +
                                                                                                      2] * (sigmaoc[3] -
                                                                                                            sigmaoc[1]);

   for (i = 0; i < nat; i++)
   {
      rRi[3 * i] = dt[i].x - roR[0];
      rRi[3 * i + 1] = dt[i].y - roR[1];
      rRi[3 * i + 2] = dt[i].z - roR[2];
   }
}

/*************************************************************************/

static void
calcD()
{

   float temp[9];
   int i;

   temp[0] = (Dr[4] + Dr[8]);
   temp[1] = (-Dr[1]);
   temp[2] = (-Dr[2]);
   temp[3] = (-Dr[1]);
   temp[4] = Dr[0] + Dr[8];
   temp[5] = (-Dr[5]);
   temp[6] = (-Dr[2]);
   temp[7] = (-Dr[5]);
   temp[8] = (Dr[0] + Dr[4]);

   inv(temp);

   for (i = 0; i < 3; i++)
      roD[i] = temp[3 * i] * (-Doc[7] + Doc[5]) + temp[3 * i + 1] * (-Doc[2] + Doc[6]) + temp[3 * i + 2] * (-Doc[3] + Doc[1]);

}

// secondo.c

static void
secondo(long double b, long double c)
{
   float delta, pfraz, pin;
   // int coco;

   delta = b * b - 4.0 * c;

   pfraz = fabs(delta);
   pin = floor(pfraz);
   pfraz = pfraz - pin;

   if (((pin == 0) && (pfraz < 0.00001) && (delta <= 0.0)) || ((pin == 0) && (pfraz < 0.2) && (delta > 0.0)))
   {
      dl2 = (-b / 2.0);
      dl3 = dl2;
   }
   else if (delta > 0.0)
   {
      dl2 = (-b + sqrt(b * b - 4.0 * c)) / 2.0;
      dl3 = (-b - sqrt(b * b - 4.0 * c)) / 2.0;
   }
   // else
   //    coco = 1;      /* TWO COMPLEX CONJUGATED EIGENVALUES */

   /*
     if(coco==1)
     {
     printf("EIGENVALUES of the MATRIX A ARE :\t ");
     printf("%f%s\n\n\n",dl1,", c.c. , c.c.");
     }
     else
     {
     printf("EIGENVALUES of the MATRIX A ARE :\t ");
     printf("%f%s%f%s%f\n\n\n",dl1,",",dl2,",",dl3);
     }
   */

}

#if defined( DEBUG_EV )
// terzo.c

static void
terzo(long double b, long double c, long double d)
{

   // float nb, nc, nd;
   float p, q, s, r, pq, epsi, y1, y2, y3, alfa, alf1;
   float unterzo, beta, bet1, rad, pfraz, pin;
   // int coco;

#if defined( DEBUG_EV )
   qDebug() << QString().sprintf( "terzo b %Lf c %Lf d %Lf", b, c, d );
#endif

   if ((c == 0.0) && (d == 0.0))
   {
      dl1 = 0.0;
      dl3 = 0.0;
      dl2 = (-b);
      goto RET0;
   }

   if (d == 0.0)
   {
      dl1 = 0.0;
      secondo(b, c);
      goto RET0;
   }

   rad = atan(1.00) / 45;
   p = (-(b * b) / 3.0 + c);
   q = (2.0 * b * b * b - 9.0 * c * b + 27.0 * d) / 27.0;
   s = (q * q) / 4.0 + (p * p * p) / 27.0;

   pfraz = fabs(s);
   pin = floor(pfraz);
   pfraz = pfraz - pin;

   if ((s > 0.0) && ((pin != 0) || ((pin == 0) && (pfraz > 0.01))))
   {
#if defined( DEBUG_EV )
      qDebug() << "two complex conjugated eigenvalues?";
#endif
   }
   //      coco = 1;      /* TWO COMPLEX CONJUGATED EIGENVALUES */

   else if ((s < 0.0) && ((pin != 0) || ((pin == 0) && (pfraz > 0.01))))
   {
      r = sqrt(-4.0 * p / 3.0);
      pq = 4.0 * q / (r * r * r);
      epsi = (asin(pq)) / 3.0;
      y1 = r * sin(epsi);
      y2 = r * sin(60.0 * rad - epsi);
      y3 = (-r * sin(60.0 * rad + epsi));

      dl1 = y1 - b / 3.0;
      dl2 = y2 - b / 3.0;
      dl3 = y3 - b / 3.0;

      goto RET0;
   }
   else
   {
      alfa = (-q / 2.0 + sqrt(q * q / 4.0 + p * p * p / 27.0));
      unterzo = (float)(1.0 / 3.0);

      if (alfa < 0.0)
      {
         alfa = (-alfa);
         alf1 = (-pow(alfa, unterzo));
      }

      else
         alf1 = pow(alfa, unterzo);

      beta = (-q / 2.0 - sqrt(q * q / 4.0 + p * p * p / 27.0));

      if (beta < 0.0)
      {
         beta = (-beta);
         bet1 = (-pow(beta, unterzo));
      }

      else
         bet1 = pow(beta, unterzo);

      dl1 = alf1 + bet1;
      dl2 = (-(alf1 + bet1) / 2.0);
      dl3 = dl2;

      dl1 = dl1 - b / 3.0;
      dl2 = dl2 - b / 3.0;
      dl3 = dl3 - b / 3.0;

   }

 RET0:

   ;
}
#endif

// visc.c

static void
visco()
{
   // float numx, numy, numz, den, bc;
   float temp, temp1, temp2, temp3, etai;
   // float mas;
   float sum1, sum11, sum111;
   int i, j;
   // float a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, E, H, I, RRcosalfa, ri, rj, Ri, Rj, R;

   // int Na = 1;

   // mas = 0.0;
   // numx = 0.0;
   // numy = 0.0;
   // numz = 0.0;
   // den = 0.0;
   vis = 0.0;

   /* COMPUTATION OF THE VISCOSITY CENTER COORDINATES */

   for (i = 0; i < 9; i++)
      cv[i] = 0.0;
   for (i = 0; i < 3; i++)
      bb[i] = 0.0;

   for (i = 0; i < nat; i++)
   {
      for (j = 0; j < nat; j++)
      {
         etai = 1.0;

         etai = 1.0 / (6.0 * M_PI * dt[j].r * ETAo);

         cv[0] += etai * (8.0 * q[i * (nat * 9) + j * 9 + 0] +
                          6.0 * q[i * (nat * 9) + j * 9 + 4] + 6.0 * q[i * (nat * 9) + j * 9 + 8]);
         cv[1] += etai * (q[i * (nat * 9) + j * 9 + 1] + q[i * (nat * 9) + j * 9 + 3]);
         cv[2] += etai * (q[i * (nat * 9) + j * 9 + 2] + q[i * (nat * 9) + j * 9 + 6]);
         bb[0] += etai * ((dt[i].x + dt[j].x) *
                          (4.0 * q[i * (nat * 9) + j * 9 + 0] +
                           3.0 * q[i * (nat * 9) + j * 9 + 4] +
                           3.0 * q[i * (nat * 9) + j * 9 + 8]) +
                          dt[j].y * (3.0 * q[i * (nat * 9) + j * 9 + 3] -
                                     2.0 * q[i * (nat * 9) + j * 9 + 1]) +
                          dt[j].z * (3.0 * q[i * (nat * 9) + j * 9 + 6] -
                                     2.0 * q[i * (nat * 9) + j * 9 + 2]) +
                          dt[j].y * (3.0 * q[i * (nat * 9) + j * 9 + 1] -
                                     2.0 * q[i * (nat * 9) + j * 9 + 3]) +
                          dt[j].z * (3.0 * q[i * (nat * 9) + j * 9 + 2] - 2.0 * q[i * (nat * 9) + j * 9 + 6]));

         cv[4] += etai * (8.0 * q[i * (nat * 9) + j * 9 + 4] +
                          6.0 * q[i * (nat * 9) + j * 9 + 8] + 6.0 * q[i * (nat * 9) + j * 9 + 0]);
         cv[5] += etai * (q[i * (nat * 9) + j * 9 + 5] + q[i * (nat * 9) + j * 9 + 7]);
         cv[3] += etai * (q[i * (nat * 9) + j * 9 + 3] + q[i * (nat * 9) + j * 9 + 1]);
         bb[1] += etai * ((dt[i].y + dt[j].y) * (4.0 * q[i * (nat * 9) + j * 9 + 4] +
                                                 3.0 * q[i * (nat * 9) + j * 9 + 8] +
                                                 3.0 * q[i * (nat * 9) + j * 9 + 0]) +
                          dt[j].z * (3.0 * q[i * (nat * 9) + j * 9 + 7] -
                                     2.0 * q[i * (nat * 9) + j * 9 + 5]) +
                          dt[j].x * (3.0 * q[i * (nat * 9) + j * 9 + 1] -
                                     2.0 * q[i * (nat * 9) + j * 9 + 3]) +
                          dt[j].z * (3.0 * q[i * (nat * 9) + j * 9 + 5] -
                                     2.0 * q[i * (nat * 9) + j * 9 + 7]) +
                          dt[j].x * (3.0 * q[i * (nat * 9) + j * 9 + 3] - 2.0 * q[i * (nat * 9) + j * 9 + 1]));

         cv[8] += etai * (8.0 * q[i * (nat * 9) + j * 9 + 8] +
                          6.0 * q[i * (nat * 9) + j * 9 + 4] + 6.0 * q[i * (nat * 9) + j * 9 + 0]);
         cv[6] += etai * (q[i * (nat * 9) + j * 9 + 6] + q[i * (nat * 9) + j * 9 + 2]);
         cv[7] += etai * (q[i * (nat * 9) + j * 9 + 7] + q[i * (nat * 9) + j * 9 + 5]);
         bb[2] += etai * ((dt[i].z + dt[j].z) * (4.0 * q[i * (nat * 9) + j * 9 + 8] +
                                                 3.0 * q[i * (nat * 9) + j * 9 + 4] +
                                                 3.0 * q[i * (nat * 9) + j * 9 + 0]) +
                          dt[j].x * (3.0 * q[i * (nat * 9) + j * 9 + 2] -
                                     2.0 * q[i * (nat * 9) + j * 9 + 6]) +
                          dt[j].y * (3.0 * q[i * (nat * 9) + j * 9 + 5] -
                                     2.0 * q[i * (nat * 9) + j * 9 + 7]) +
                          dt[j].x * (3.0 * q[i * (nat * 9) + j * 9 + 6] -
                                     2.0 * q[i * (nat * 9) + j * 9 + 2]) +
                          dt[j].y * (3.0 * q[i * (nat * 9) + j * 9 + 7] - 2.0 * q[i * (nat * 9) + j * 9 + 5]));
      }
   }

   inv(cv);

   Mxv(cv, bb);

   // if (cc == 1)
   //    bc = 6.0;
   // else
   //    bc = 4.0;

   for (i = 0; i < nat; i++)
   {
      for (j = 0; j < nat; j++)
      {

         /*    etai=bc*PI*dt[j].r;  */

         etai = 1.0;

         temp =
            (dt[i].x - vc[0]) * q[i * (nat * 9) + j * 9 + 0] *
            (dt[j].x - vc[0]) + (dt[i].y - vc[1]) * q[i * (nat * 9) + j * 9 + 4] * (dt[j].y - vc[1]) +
            (dt[i].z - vc[2]) * q[i * (nat * 9) + j * 9 + 8] * (dt[j].z - vc[2]);
         temp1 = (dt[i].x - vc[0]) * ((dt[j].y - vc[1]) * (q[i * (nat * 9) + j * 9 + 3]) +
                                      (dt[j].z - vc[2]) * q[i * (nat * 9) + j * 9 + 6]);
         temp2 = (dt[i].y - vc[1]) * ((dt[j].x - vc[0]) * (q[i * (nat * 9) + j * 9 + 1]) +
                                      (dt[j].z - vc[2]) * q[i * (nat * 9) + j * 9 + 7]);
         temp3 = (dt[i].z - vc[2]) * ((dt[j].x - vc[0]) * (q[i * (nat * 9) + j * 9 + 2]) +
                                      (dt[j].y - vc[1]) * q[i * (nat * 9) + j * 9 + 5]);
         sum1 = temp1 + temp2 + temp3;

         temp1 = (dt[i].x - vc[0]) * ((dt[j].y - vc[1]) * (q[i * (nat * 9) + j * 9 + 1]) +
                                      (dt[j].z - vc[2]) * q[i * (nat * 9) + j * 9 + 2]);
         temp2 = (dt[i].y - vc[1]) * ((dt[j].x - vc[0]) * (q[i * (nat * 9) + j * 9 + 3]) +
                                      (dt[j].z - vc[2]) * q[i * (nat * 9) + j * 9 + 5]);
         temp3 = (dt[i].z - vc[2]) * ((dt[j].x - vc[0]) * (q[i * (nat * 9) + j * 9 + 6]) +
                                      (dt[j].y - vc[1]) * q[i * (nat * 9) + j * 9 + 7]);
         sum11 = temp1 + temp2 + temp3;

         temp1 = (dt[i].x - vc[0]) * (dt[j].x - vc[0]) * (q[i * (nat * 9) + j * 9 + 4] + q[i * (nat * 9) + j * 9 + 8]);
         temp2 = (dt[i].y - vc[1]) * (dt[j].y - vc[1]) * (q[i * (nat * 9) + j * 9 + 0] + q[i * (nat * 9) + j * 9 + 8]);
         temp3 = (dt[i].z - vc[2]) * (dt[j].z - vc[2]) * (q[i * (nat * 9) + j * 9 + 0] + q[i * (nat * 9) + j * 9 + 4]);
         sum111 = temp1 + temp2 + temp3;

         vis += etai * ((1.0 / 15.0) * temp + (1.0 / 20.0) * sum1 - (1.0 / 30.0) * sum11 + (1.0 / 20.0) * sum111);

      }
   }

   /*for(i=0;i<nat;i++) mas+=dt[i].m;  total mass of the model */

   vis = vis / pesmol * AVO * 1.0E-21 / ETAo;

   vis3 = (float)(2.5 * AVO * 1.0E-21);
}


#if defined(TSUDA_DOUBLESUM)

/*******************************************************************************/

/*  INTRISIC VISCOSITY COMPUTATION ACCORDING TO TSUDA (CM) */

static void
tsuda()
{

   int i, j;
   float a1;
   float a2;
   float a3;
   float a4;
   float a5;
   float a6;
   float a7;
   float a8;
   float a9;
   float a10;
   float a11;
   float a12;
   float a13;
   float a14;
   float a15;
   float a16;
   float E;
   float H;
   float I;
   float RRcosalfa;
   float ri = 0.0;
   float rj;
   float Ri = 0.0;
   float Rj;
   float R;

   I = 0.0;
   E = 0.0;
   H = 0.0;

   /* COMPUTATION OF OTHER PARAMETERS NECESSARY FOR THE INTRINSIC VISCOSITY */

   for (i = 0; i < nat; i++)
   {
      for (j = 0; j < nat; j++)
      {
         if (i != j)
         {
            ri = dt[i].r;

            rj = dt[j].r;

            Ri = sqrt(pow((dt[i].x - xm), 2) + pow((dt[i].y - ym), 2) + pow((dt[i].z - zm), 2));

            Rj = sqrt(pow((dt[j].x - xm), 2) + pow((dt[j].y - ym), 2) + pow((dt[j].z - zm), 2));

            R = sqrt(pow((dt[i].x - dt[j].x), 2) + pow((dt[i].y - dt[j].y), 2) + pow((dt[i].z - dt[j].z), 2));

            RRcosalfa = (dt[i].x - xm) * (dt[j].x - xm) + (dt[i].y - ym) * (dt[j].y - ym) + (dt[i].z - zm) * (dt[j].z - zm);

            a1 = log10((R * R - (ri - rj) * (ri - rj)) / (R * R + (ri - rj) * (ri - rj)));

            a2 = (52 * (Ri * Ri + Rj * Rj) + 34 * (ri * ri + rj * rj) - 54 * R * R) / (5 * R);

            a3 = 104 * ri * ri * (RRcosalfa - Ri * Ri) / (15 * R * R * R) -
               104 * rj * rj * (Rj * Rj - RRcosalfa) / (15 * R * R * R);

            a4 = a1 * ((Ri * Ri - Rj * Rj + ri * ri - rj * rj) * (Ri * Ri - Rj * Rj + ri * ri - rj * rj) +
                       4.0 / 3.0 * (Ri * Ri * ri * ri + Rj * Rj * rj * rj)) / (10 * R * ri * rj);

            a5 = 3.0 * rj * rj - 3.0 * R * R - 5.0 * ri * ri +
               a1 * (-ri * ri * (6.0 * rj * rj - 3.0 * ri * ri - 2.0 * R * R) +
                     3.0 * (R * R - rj * rj) * (R * R - rj * rj)) / (4.0 * ri * rj);

            a6 = (3.0 * (RRcosalfa - Ri * Ri) * (RRcosalfa - Ri * Ri) - R * R * Ri * Ri) * a5 / (15 * R * R * R * R * R);

            a7 = 3.0 * ri * ri - 3.0 * R * R - 5.0 * rj * rj +
               a1 * (-rj * rj * (6.0 * ri * ri - 3.0 * rj * rj - 2.0 * R * R) +
                     3.0 * (R * R - ri * ri) * (R * R - ri * ri)) / (4.0 * ri * rj);

            a8 = (3.0 * (Rj * Rj - RRcosalfa) * (Rj * Rj - RRcosalfa) - R * R * Rj * Rj) * a7 / (15 * R * R * R * R * R);

            a9 = 1.0 + a1 * (rj * rj - ri * ri - R * R) / (4.0 * ri * rj);

            a10 = 4.0 * (Ri * Ri - Rj * Rj + ri * ri - rj * rj) * (RRcosalfa - Ri * Ri) * a9 / (5.0 * R * R * R);

            a11 = 1.0 + a1 * (ri * ri - rj * rj - R * R) / (4.0 * ri * rj);

            a12 = 4.0 * (Ri * Ri - Rj * Rj + ri * ri - rj * rj) * (Rj * Rj - RRcosalfa) * a11 / (5.0 * R * R * R);

            a13 = 4.0 * (RRcosalfa - Ri * Ri) * (Rj * Rj - RRcosalfa) / (5.0 * R * R * R * R * R);

            a14 = a13 * (R * R + ri * ri + rj * rj + a1 * ((ri * ri - rj * rj) * (ri * ri - rj * rj) - R * R * R * R));

            a15 = 2.0 * (Ri * Ri * Rj * Rj - RRcosalfa * RRcosalfa) / (5.0 * R * R * R * R * R);

            a16 =
               a15 * (R * R - ri * ri - rj * rj +
                      a1 / (4.0 * ri * rj) * ((ri + rj) * (ri + rj) - R * R) * (R * R - (ri - rj) * (ri - rj)));

            I += (ri * ri * rj * rj) * (a2 + a3 + a4 + a6 + a8 - a10 - a12 - a14 - a16);

         }
      }
      E += ri * ri * (ri * ri + Ri * Ri);

      H += ri * ri * ri * (3.0 * ri * ri + 10 * Ri * Ri);
   }

   E = 4.0 * E;

   H = 32.0 / 15.0 * H;

   vis1 = 4.0 * AVO * 1.0E-21 * M_PI * E * E / (3.0 * pesmol * (H + I));

}

/*********************************************************************************/

/*  INTRINSIC VISCOSITY COMPUTATION ACCORDING TO TSUDA (CV) */

static void
tsuda1()
{

   int i, j;
   float a1;
   float a2;
   float a3;
   float a4;
   float a5;
   float a6;
   float a7;
   float a8;
   float a9;
   float a10;
   float a11;
   float a12;
   float a13;
   float a14;
   float a15;
   float a16;
   float E;
   float H;
   float I;
   float RRcosalfa;
   float ri = 0.0;
   float rj;
   float Ri = 0.0;
   float Rj;
   float R;

   I = E = H = 0.0;

   /* COMPUTATION OF OTHER PARAMETERS NECESSARY FOR THE INTRINSIC VISCOSITY */

   for (i = 0; i < nat; i++)
   {
      for (j = 0; j < nat; j++)
      {
         if (i != j)
         {
            ri = dt[i].r;

            rj = dt[j].r;

            Ri = sqrt(pow((dt[i].x - vc[0]), 2) + pow((dt[i].y - vc[1]), 2) + pow((dt[i].z - vc[2]), 2));

            Rj = sqrt(pow((dt[j].x - vc[0]), 2) + pow((dt[j].y - vc[1]), 2) + pow((dt[j].z - vc[2]), 2));

            R = sqrt(pow((dt[i].x - dt[j].x), 2) + pow((dt[i].y - dt[j].y), 2) + pow((dt[i].z - dt[j].z), 2));

            RRcosalfa = (dt[i].x - vc[0]) * (dt[j].x - vc[0]) + (dt[i].y - vc[1]) * (dt[j].y - vc[1]) +
               (dt[i].z - vc[2]) * (dt[j].z - vc[2]);

            a1 = log10((R * R - (ri - rj) * (ri - rj)) / (R * R + (ri - rj) * (ri - rj)));

            a2 = (52 * (Ri * Ri + Rj * Rj) + 34 * (ri * ri + rj * rj) - 54 * R * R) / (5 * R);

            a3 = 104 * ri * ri * (RRcosalfa - Ri * Ri) / (15 * R * R * R) -
               104 * rj * rj * (Rj * Rj - RRcosalfa) / (15 * R * R * R);

            a4 = a1 * ((Ri * Ri - Rj * Rj + ri * ri - rj * rj) * (Ri * Ri - Rj * Rj + ri * ri - rj * rj) +
                       4.0 / 3.0 * (Ri * Ri * ri * ri + Rj * Rj * rj * rj)) / (10 * R * ri * rj);

            a5 = 3.0 * rj * rj - 3.0 * R * R - 5.0 * ri * ri +
               a1 * (-ri * ri * (6.0 * rj * rj - 3.0 * ri * ri - 2.0 * R * R) +
                     3.0 * (R * R - rj * rj) * (R * R - rj * rj)) / (4.0 * ri * rj);

            a6 = (3.0 * (RRcosalfa - Ri * Ri) * (RRcosalfa - Ri * Ri) - R * R * Ri * Ri) * a5 / (15 * R * R * R * R * R);

            a7 = 3.0 * ri * ri - 3.0 * R * R - 5.0 * rj * rj +
               a1 * (-rj * rj * (6.0 * ri * ri - 3.0 * rj * rj - 2.0 * R * R) +
                     3.0 * (R * R - ri * ri) * (R * R - ri * ri)) / (4.0 * ri * rj);

            a8 = (3.0 * (Rj * Rj - RRcosalfa) * (Rj * Rj - RRcosalfa) - R * R * Rj * Rj) * a7 / (15 * R * R * R * R * R);

            a9 = 1.0 + a1 * (rj * rj - ri * ri - R * R) / (4.0 * ri * rj);

            a10 = 4.0 * (Ri * Ri - Rj * Rj + ri * ri - rj * rj) * (RRcosalfa - Ri * Ri) * a9 / (5.0 * R * R * R);

            a11 = 1.0 + a1 * (ri * ri - rj * rj - R * R) / (4.0 * ri * rj);

            a12 = 4.0 * (Ri * Ri - Rj * Rj + ri * ri - rj * rj) * (Rj * Rj - RRcosalfa) * a11 / (5.0 * R * R * R);

            a13 = 4.0 * (RRcosalfa - Ri * Ri) * (Rj * Rj - RRcosalfa) / (5.0 * R * R * R * R * R);

            a14 = a13 * (R * R + ri * ri + rj * rj + a1 * ((ri * ri - rj * rj) * (ri * ri - rj * rj) - R * R * R * R));

            a15 = 2.0 * (Ri * Ri * Rj * Rj - RRcosalfa * RRcosalfa) / (5.0 * R * R * R * R * R);

            a16 =
               a15 * (R * R - ri * ri - rj * rj +
                      a1 / (4.0 * ri * rj) * ((ri + rj) * (ri + rj) - R * R) * (R * R - (ri - rj) * (ri - rj)));

            I += (ri * ri * rj * rj) * (a2 + a3 + a4 + a6 + a8 - a10 - a12 - a14 - a16);

         }
      }
      E += ri * ri * (ri * ri + Ri * Ri);

      H += ri * ri * ri * (3.0 * ri * ri + 10 * Ri * Ri);
   }

   E = 4.0 * E;

   H = 32.0 / 15.0 * H;

   vis2 = 4.0 * AVO * 1.0E-21 * M_PI * E * E / (3.0 * pesmol * (H + I));

}

/*********************************************************************/

/*  INTRINSIC VISCOSITY COMPUTATION BY THE DOUBLE SUM APPROXIMATION   */

static void
doublesum()
{

   int i;
   int j;
   float a1, a2, RRcosalfa, ri, rj, Ri, Rj, R;

   a1 = a2 = 0.0;

   /* COMPUTATION OF OTHER PARAMETERS NECESSARY FOR THE INTRINSIC VISCOSITY */

   for (i = 0; i < nat; i++)
   {

      ri = dt[i].r;

      Ri = sqrt(pow((dt[i].x - xm), 2) + pow((dt[i].y - ym), 2) + pow((dt[i].z - zm), 2));

      a1 += 6.0 * M_PI * ETAo * ri * Ri * Ri;

      for (j = 0; j < nat; j++)
      {

         rj = dt[j].r;

         Rj = sqrt(pow((dt[j].x - xm), 2) + pow((dt[j].y - ym), 2) + pow((dt[j].z - zm), 2));

         if (i != j)
         {

            R = sqrt(pow((dt[i].x - dt[j].x), 2) + pow((dt[i].y - dt[j].y), 2) + pow((dt[i].z - dt[j].z), 2));

            RRcosalfa = (dt[i].x - xm) * (dt[j].x - xm) + (dt[i].y - ym) * (dt[j].y - ym) + (dt[i].z - zm) * (dt[j].z - zm);

            a2 += 6.0 * M_PI * ETAo * ri * rj * RRcosalfa / R;

         }
      }
   }

   vis4 = AVO * 1.0E-21 / 6.0 / ETAo * (a1 * a1) / (a1 + a2) / pesmol;

}
#endif

#if defined(USE_MAIN)
int
main()
{
   int use_nmax;
   do
   {
      printf("\n\n\n** Insert number of beads to use? :  ");
      if ( 1 == scanf("%d", &use_nmax) ) {};
   }
   while (use_nmax < 1);
   us_hydro_supc_main(use_nmax);
}
#endif


//--------- thread for supc -----------

// #define DEBUG_THREAD

supc_thr_t::supc_thr_t(int a_thread) : QThread()
{
   thread = a_thread;
   work_to_do = 0;
   work_done = 1;
   work_to_do_waiters = 0;
   work_done_waiters = 0;
}

void supc_thr_t::supc_thr_setup(
                                unsigned int threads,
                                int mode,
                                float *p,
                                float *rRis,
                                float *rRi,
                                float *a,
                                int N,
                                int i,
                                int nat,
                                float *sum
                                )
{
   /* this starts up a new work load for the thread */
   this->threads = threads;

   this->mode = mode;

   this->p = p;
   this->rRis = rRis;
   this->rRi = rRi;
   this->a = a;
   this->N = N;
   this->i = i;
   this->nat = nat;
   this->sum = sum;

   work_mutex.lock();
   work_to_do = 1;
   work_done = 0;
   work_mutex.unlock();
   cond_work_to_do.wakeOne();
#if defined(DEBUG_THREAD)
   cerr << "thread " << thread << " has new work to do\n";
#endif
}

void supc_thr_t::supc_thr_shutdown()
{
   /* this signals the thread to exit the run method */
   work_mutex.lock();
   work_to_do = -1;
   work_mutex.unlock();
   cond_work_to_do.wakeOne();

#if defined(DEBUG_THREAD)
   cerr << "thread " << thread << " shutdown requested\n";
#endif
}

void supc_thr_t::supc_thr_wait()
{
   /* this is for the master thread to wait until the work is done */
   work_mutex.lock();

#if defined(DEBUG_THREAD)
   cerr << "thread " << thread << " has a waiter\n";
#endif

   while(!work_done) {
      cond_work_done.wait(&work_mutex);
   }
   work_done = 0;
   work_mutex.unlock();

#if defined(DEBUG_THREAD)
   cerr << "thread " << thread << " waiter released\n";
#endif
}

int supc_thr_t::supc_thr_work_status()
{
   work_mutex.lock();
   int retval = work_done;
   work_mutex.unlock();
   return retval;
}

void supc_thr_t::run()
{
   while(1)
   {
      work_mutex.lock();
#if defined(DEBUG_THREAD)
      cerr << "thread " << thread << " waiting for work\n";
#endif
      work_to_do_waiters++;

      while(!work_to_do)
      {
         cond_work_to_do.wait(&work_mutex);
      }

      if(work_to_do == -1)
      {
#if defined(DEBUG_THREAD)
         cerr << "thread " << thread << " shutting down\n";
#endif
         work_mutex.unlock();
         return;
      }

      work_to_do_waiters = 0;
      work_mutex.unlock();
#if defined(DEBUG_THREAD)
      cerr << "thread " << thread << " starting work\n";
#endif
      
      switch ( mode ) {
      case 1: // update partial sum
         {
            for ( int k = i + 1 + thread; k <  3 * N; k += threads )
            {
#if defined(DEBUG_THREAD)
               cerr << "thread " << thread << " i = " << i << endl;
#endif
               *sum -= a[k * (3 * nat) + i] * rRi[k];
            }
         }
         break;

      case 2: // update rRi
         {
            int k;
            float sum;
            for (i = 3 * N - 1 - thread; i >= 0; i -= threads )
            {
               for (sum = rRis[i], k = i + 1; k < 3 * N; k++)
                  sum -= a[k * (3 * nat) + i] * rRis[k];
               rRi[i] = sum / p[i];
            }
         }
         break;
      }

#if defined(DEBUG_THREAD)
      cerr << "thread " << thread << " finished work\n";
#endif
      work_mutex.lock();
      work_done = 1;
      work_to_do = 0;
      work_mutex.unlock();
      cond_work_done.wakeOne();
   }
}

//--------- end thread for supc --------------

void ev3( vector < vector < long double > > & A ) {
   // Given a real symmetric 3x3 matrix A, compute the eigenvalues
   // Note that acos and cos operate on angles in radians

   long double p1 =
      A[0][1] *  A[0][1] +
      A[0][2] *  A[0][2] +
      A[1][2] *  A[1][2]
      ;

   dl1 = 0;
   dl2 = 0;
   dl3 = 0;
   // long double eig1 = 0;
   // long double eig2 = 0;
   // long double eig3 = 0;
   
   if ( p1 == 0 ) {
      dl1 = A[0][0];
      dl2 = A[1][1];
      dl3 = A[2][2];
   } else {
      long double q = ( A[0][0] + A[1][1] + A[2][2] ) / 3e0;

      long double p2 =
         (A[0][0] - q) * (A[0][0] - q) +
         (A[1][1] - q) * (A[1][1] - q) +
         (A[2][2] - q) * (A[2][2] - q) +
         2e0 * p1
         ;
      long double p = sqrt(p2 / 6e0);
      long double one_over_p = 1e0 / p;
      vector < vector < long double > > B = A;
      // B = (1 / p) * (A - q * I)    % I is the identity matrix

      for ( int i = 0; i < 3; ++i ) {
         for ( int j = 0; j < 3; ++j ) {
            if ( i == j ) {
               B[ i ][ j ] = one_over_p * ( A[ i ][ j ] - q );
            } else {
               B[ i ][ j ] = one_over_p * A[ i ][ j ];
            }
         }
      }
            
#define A_ B[0][0]
#define B_ B[0][1]
#define C_ B[0][2]
#define D_ B[1][0]
#define E_ B[1][1]
#define F_ B[1][2]
#define G_ B[2][0]
#define H_ B[2][1]
#define I_ B[2][2]

      // r = det(B) / 2

      long double r =
         ( A_ * ( E_ * I_ - F_ * H_ ) -
           B_ * ( D_ * I_ - F_ * G_ ) +
           C_ * ( D_ * H_ - E_ * G_ ) ) / 2e0
         ;
         
         // B[0][0] * (B[1][1] * B[2][2] - B[1][2] * B[2][1] ) -
         // B[0][1] * (B[1][1] * B[2][2] - B[1][2] * B[2][0] ) +
         // B[0][2] * (B[1][1] * B[2][1] - B[1][1] * B[2][0] )
      ;

      // In exact arithmetic for a symmetric matrix  -1 <= r <= 1
      // but computation error can leave it slightly outside this range.
      long double phi;
      if (r <= -1e0) {
         phi = M_PI / 3e0;
      } else if ( r >= 1e0 ) {
         phi = 0;
      } else {
         phi = acos( r ) / 3e0;
      }

      // % the eigenvalues satisfy eig3 <= eig2 <= eig1
      dl1 = q + 2e0 * p * cos(phi);
      dl3 = q + 2e0 * p * cos(phi + (2e0 * M_PI / 3e0));
      dl2 = 3e0 * q - dl1 - dl3;
   }

   // swap 1 & 2 for consistency with terzo
   long double dlx = dl1;
   dl1 = dl2;
   dl2 = dlx;
   
   printf( "eigenvalues %Lf, %Lf, %Lf\n", dl1, dl2, dl3 );
}
