#include "../include/us_hydrodyn_zeno.h"
//Added by qt3to4:
#include <QTextStream>
#include "../include/us_unicode.h"

US_Hydrodyn  * zeno_us_hydrodyn;
static mQProgressBar * zeno_progress;
bool * zeno_stop_flag;
static US_Udp_Msg  * zeno_us_udp_msg;

// // note: this program uses cout and/or cerr and this should be replaced

// static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
//    return os << qPrintable(str);
// }

namespace zeno {
   /*
     Original translation from Fortran:
     Produced by Portable Fortran Translator V9.88 (11/02/10) on 5/26/12 at 9:58:08
   */
#define USZ_MAX_ID_SIZE         2048 
#define USZ_MAX_ID_SIZE_PLUS_5  2053
#define USZ_MAX_ID_SIZE_P_5_T_4 8212
#define USZ_MAX_ID_TOTSPACESIZE 20480 

#define LPROTOTYPE
#define FPROTOTYPE
#include "../include/us_pfortran.h"

#define MAXELTS 65000
#define MAXWORDS 100

   char Xfilenames[USZ_MAX_ID_TOTSPACESIZE];
   char Xfilenumbers[40];
   char Xcubit[8];
   char Xsell[8];
   char Xshush[4];
   char Xmorse[128];
   char Xrandom[4];

   extern float ran2();

   void setup(
              char *id,
              int m1[],
              char *actions,
              char *start,
              char *actout,
              unsigned int *savehits_z,
              unsigned int *savehits_i,
              unsigned int *savehits_s,
              unsigned int *savehits_p,
              int P2,
              int P1,
              int P3,
              int P4
              );

   void parse(
              int *maxelts,
              int *nelts,
              int eltype[],
              float *bv,
              float *tol,
              float *rotations,
              unsigned int *tolset,
              char *unitcode,
              unsigned int *bt,
              unsigned int *bm,
              unsigned int *bw,
              unsigned int *bc,
              unsigned int *bbf,
              double temp[],
              char *tunit,
              double mass[],
              char *munit,
              double visc[],
              char *vunit,
              double buoy[],
              float *hscale,
              float *rlaunch,
              unsigned int *launch_done,
              char *dictionary,
              int map[],
              int *maxwords,
              int *nwords,
              int P1,
              int P2,
              int P3,
              int P4,
              int P5
              );

   void begin_output(char *id,
                     int *nelts,
                     char *start,
                     int P1,
                     int P2);


   void report(
               char *id,
               char *actions,
               int m1[],
               int *nelts,
               float *tol,
               float *rlaunch,
               float *cap,
               float *delta_cap,
               float *alpha_bongo,
               float *delta_bongo,
               double *tten,
               float *volume,
               float *delta_volume,
               float *surface,
               float *delta_surface,
               float *rg2int,
               float *delta_rg2int,
               float *rg2surf,
               float *delta_rg2surf,
               float *kirk,
               float *delta_kirk,
               float *span,
               float *delta_span,
               float *shadow,
               float *delta_shadow,
               unsigned int *shadow_done,
               unsigned int *zeno_done,
               unsigned int *kirk_done,
               unsigned int *rg_done,
               unsigned int *span_done,
               unsigned int *launch_done,
               unsigned int *tol_given,
               char *unitcode,
               int *mz,
               int *mi,
               int *ms,
               int *mp,
               unsigned int *bt,
               unsigned int *bm,
               unsigned int *bw,
               unsigned int *bc,
               unsigned int *bbf,
               double temp[],
               char *tunit,
               double mass[],
               char *munit,
               double visc[],
               char *vunit,
               double buoy[],
               float *hscale,
               double *tae,
               double uae[],
               double *vae,
               double *wae,
               float *q2pade,
               double eigens[],
               double xx[],
               float q[],
               float sq[],
               float dsq[],
               float xyzlow[],
               float xyzhih[],
               int P1,
               int P2,
               int P3,
               int P6,
               int P4,
               int P5);

   void lighthalf(
                  double qa[],
                  double qb[],
                  double sqa[],
                  double sqb[],
                  double lhalf[] );

   void rudnick_gaspari(
                        double ev[],
                        double *aanum,
                        double *aaden,
                        double *aa
                        );

   void writehandy(
                   double *tae,
                   double uae[],
                   double *vae,
                   double *wae,
                   char *nounit,
                   char *vunit,
                   int P1,
                   int P2);

   void poplar(
               int *mz,
               int *ms,
               int *mi,
               int *mp,
               unsigned int *bl,
               unsigned int *bk,
               unsigned int *bs,
               unsigned int *bi,
               unsigned int *bt,
               unsigned int *bm,
               unsigned int *bw,
               unsigned int *bc,
               unsigned int *bbf,
               unsigned int *bz,
               unsigned int *bp,
               unsigned int *bpp,
               double *rl,
               double *eps,
               double cap[],
               double a11[],
               double a12[],
               double a13[],
               double a21[],
               double a22[],
               double a23[],
               double a31[],
               double a32[],
               double a33[],
               double *tten,
               double rk[],
               double surf[],
               double rg2surf[],
               double v[],
               double rg2int[],
               double rp[],
               double rap[],
               double temp[],
               double mass[],
               double eta[],
               double buoy[],
               char *lunit,
               char *tunit,
               char *munit,
               char *vunit,
               char *qunit,
               double *tae,
               double uae[],
               double *vae,
               double *wae,
               float *q2pade,
               double eigens[],
               double xx[],
               float q[],
               double *stfac,
               double lhalf[],
               double box1[],
               double box2[],
               int P1,
               int P2,
               int P3,
               int P4,
               int P5);

   void bundle(
               char *nod1,
               char *nod2,
               char *name,
               int P1,
               int P2,
               int P3);

   void summer(double a[],
               double b[],
               double c[]);

   void multiply(double a[],
                 double b[],
                 double c[]);

   void divide(double x1[],
               double x2[],
               double y[]);

   void power(
              double x[],
              double y[],
              double *exponent);

   void derive(
               char *lunit,
               char *aunit,
               char *volunit,
               char *nounit,
               int P1,
               int P2,
               int P3,
               int P4);

   void loadconstants(
                      double q1[],
                      double q2low[],
                      double q2hih[],
                      float *q2pade,
                      double q2best[],
                      double pi[],
                      double two[],
                      double three[],
                      double four[],
                      double five[],
                      double six[],
                      double boltz[],
                      double celsius[],
                      double pre1[],
                      double pre2[],
                      double pi12[],
                      double fpercm[]
                      );

   void parameters(
                   unsigned int *bt,
                   double temp[],
                   unsigned int *bc,
                   double eta[],
                   char *tunit,
                   char *vunit,
                   double celsius[],
                   int P1,
                   int P2);

   void pack80(
               char *line,
               int P1);

   void floatstring(
                    char *tline,
                    double t1[],
                    int P1);

   void dotout(
               char *name,
               char *line,
               int P1,
               int P2);

   void writeno(
                double *rl,
                char *name,
                char *lunit,
                int P1,
                int P2);

   void compvisc(
                 double temp[],
                 double eta[],
                 char *vunit,
                 int P1);

   void conventional(
                     double t1[],
                     double etam[],
                     char *lunit,
                     char *munit,
                     char *intunit,
                     int P1,
                     int P2,
                     int P3);


   void friction(
                 double t1[],
                 double fric[],
                 char *lunit,
                 char *funit,
                 int P1,
                 int P2);

   void svedberg(
                 double t1[],
                 double sed[],
                 char *munit,
                 char *lunit,
                 char *svedunit,
                 int P1,
                 int P2,
                 int P3);

   void diffuse(
                double t1[],
                double d[],
                char *lunit,
                char *dunit,
                int P1,
                int P2);

   void writeyes(
                 double tx[],
                 char *name,
                 char *u,
                 int P1,
                 int P2);


   void longjohn(
                 double tx[],
                 char *name,
                 char *u,
                 char *mane,
                 char *line,
                 char *longname,
                 int P1,
                 int P4,
                 int P3,
                 int P2,
                 int P5);

   void writeme(
                char *qunit,
                char *nounit,
                float *q,
                double *stfac1,
                double *stfac2,
                int P1,
                int P2);

   void pack20(
               char *longname,
               int P1);

   void pack49(char *longname,
               int P1);

   void shiftleft(char *String,
                  int P1);

   void shiftright(
                   char *String,
                   int P1);

   void show_errors();

   void pushon(char *byte,
               char *rs,
               int P2,
               int P1);

   void establish_codes();

   void do_launch(
                  int * /* maxelts */,
                  int *nelts,
                  int eltype[],
                  float *bv,
                  float *rlaunch,
                  float *rotations,
                  float xyzlow[],
                  float xyzhih[]);

   void alldone(char *endend,
                int P1);


   void packspaces(char *buffer,
                   int *next,
                   int P1);

   void nextstring(unsigned int *nobuffer,
                   char *buffer,
                   char *String,
                   unsigned int *atend,
                   int P1,
                   int P2);

   void cleartabs(char *buffer,
                  int P1);

   void webster(char *dictionary,
                int map[],
                int *nwords,
                int *maxwords,
                int P1);

   void commaparse(char *copy,
                   char *word,
                   int P1,
                   int P2);

   void testword(char *String,
                 int *ntype,
                 char *dictionary,
                 int map[],
                 int *nwords,
                 int *maxwords,
                 int P2,
                 int P1);

   void modifiers(unsigned int *nobuffer,
                  char *buffer,
                  char *String,
                  unsigned int *atend,
                  char *down,
                  double *download,
                  char *dictionary,
                  int map[],
                  int *maxwords,
                  int *nwords,
                  int P2,
                  int P3,
                  int P1,
                  int P4);


   void maxpillar(
                  float x[],
                  float y[],
                  float zl[],
                  float zh[],
                  float *dd);

   void propillar(
                  float x[],
                  float y[],
                  float zl[],
                  float zh[],
                  float v[],
                  float *s1,
                  float *s2);

   void insidepillar(
                     float x[],
                     float y[],
                     float zl[],
                     float zh[],
                     float p[],
                     unsigned int *inside);

   void minpillar(
                  float x[],
                  float y[],
                  float zl[],
                  float zh[],
                  float p[],
                  float *d);

   void maxcube(
                float v1[],
                float *side,
                float *dd);

   void maxsphere(
                  float c[],
                  float *r,
                  float *dd);

   void procube(
                float v1[],
                float *side,
                float v[],
                float *s1,
                float *s2);

   void mincube(
                float v1[],
                float *side,
                float p[],
                float *d);

   void prosphere(
                  float c[],
                  float *r,
                  float v[],
                  float *s1,
                  float *s2);

   void minsphere(
                  float c[],
                  float *r,
                  float p[],
                  float *d);

   void insidecube(
                   float v1[],
                   float *side,
                   float p[],
                   unsigned int *inside);

   void insidesphere(
                     float c[],
                     float *r,
                     float p[],
                     unsigned int *inside);

   void maxtriangle(
                    float v1[],
                    float v2[],
                    float v3[],
                    float *dd);

   void protriangle(
                    float v1[],
                    float v2[],
                    float v3[],
                    float v[],
                    float *s1,
                    float *s2);

   void mintriangle(
                    float v1[],
                    float v2[],
                    float v3[],
                    float p[],
                    float *d);

   void maxdisk(
                float c[],
                float n[],
                float *r,
                float *t,
                float *dd);

   void prodisk(
                float c[],
                float n[],
                float *r,
                float *tr,
                float v[],
                float *s1,
                float *s2);

   void mindisk(
                float c[],
                float n[],
                float *r,
                float *tr,
                float p[],
                float *d);

   void maxsolcyl(
                  float c[],
                  float n[],
                  float *r,
                  float *l,
                  float *t,
                  float *dd);

   void maxcylinder(
                    float c[],
                    float n[],
                    float *r,
                    float *l,
                    float *t,
                    float *dd);

   void prosolcyl(
                  float c[],
                  float n[],
                  float *r,
                  float *l,
                  float *t,
                  float v[],
                  float *s1,
                  float *s2);

   void procylinder(
                    float c[],
                    float n[],
                    float *r,
                    float *l,
                    float *t,
                    float v[],
                    float *s1,
                    float *s2);

   void minsolcyl(
                  float c[],
                  float n[],
                  float *r,
                  float *l,
                  float *t,
                  float p[],
                  float *d);

   void insidesolcyl(
                     float c[],
                     float n[],
                     float *r,
                     float *l,
                     float *t,
                     float p[],
                     unsigned int *inside);

   void mincylinder(
                    float c[],
                    float n[],
                    float *r,
                    float *l,
                    float *t,
                    float p[],
                    float *d);

   void maxtorus(
                 float c[],
                 float n[],
                 float *r1,
                 float *r2,
                 float *t,
                 float *dd);

   void protorus(
                 float c[],
                 float n[],
                 float *r1,
                 float *r2,
                 float *tr,
                 float v[],
                 float *s1,
                 float *s2);

   void mintorus(
                 float c[],
                 float n[],
                 float *r1,
                 float *r2,
                 float *t,
                 float p[],
                 float *d);

   void insidetorus(
                    float c[],
                    float n[],
                    float *r1,
                    float *r2,
                    float *t,
                    float p[],
                    unsigned int *inside);

   void maxellipsoid(
                     float c[],
                     float n1[],
                     float n2[],
                     float *aa,
                     float *bb,
                     float *cc,
                     float *t,
                     float *dd);

   void proellipsoid(
                     float c[],
                     float n1[],
                     float n2[],
                     float *aa,
                     float *bb,
                     float *cc,
                     float *t,
                     float v[],
                     float *s1,
                     float *s2);


   void minellipsoid(
                     float c[],
                     float n1[],
                     float n2[],
                     float *aa,
                     float *bb,
                     float *cc,
                     float *t,
                     float p[],
                     float *d);

   void insideellipsoid(
                        float c[],
                        float n1[],
                        float n2[],
                        float *aa,
                        float *bb,
                        float *cc,
                        float *t,
                        float po[],
                        unsigned int *inside);

   void exellipsoid(
                    float c[],
                    float n1[],
                    float n2[],
                    float *aa,
                    float *bb,
                    float *cc,
                    float *t,
                    float po[],
                    int *minmax,
                    float *d);

   void sort(
             float a[],
             float a2s[],
             int *ndif);


   void above(
              float a[],
              float p[],
              float *alam,
              float x[],
              float *sing,
              float *ccc);


   void between(
                float a[],
                float p[],
                float *alam1,
                float *alam2,
                float x1[],
                float x2[],
                float *sing1,
                float *sing2,
                int *ns);

   void eval(
             float *z,
             float a[],
             float p[],
             float x[],
             float *t);

   void deval(
              double *z,
              double a[],
              double p[],
              double x[],
              double *t);

   void eval2(
              float *z,
              float a[],
              float p[],
              float x[],
              float *t);

   void converge(
                 float *zlow,
                 float *zhih,
                 float a[],
                 float p[],
                 float x[],
                 float *alam);

   void minedge(
                float v1[],
                float v2[],
                float p[],
                float *d);


   void zeerot(
               float n3[],
               float *t);

   void xyzrot(
               float n1[],
               float n2[],
               float n3[],
               float *t);

   void rotate(
               float a[],
               float *t,
               float b[]);


   void vector_difference(
                          float a[],
                          float b[],
                          float c[]);

   void cross_product(
                      float a[],
                      float b[],
                      float c[]);

   void dotproduct(
                   float a[],
                   float b[],
                   float *c);

   void scalar_product(
                       float *a_scalar,
                       float b_vector[],
                       float c[]);

   void pythag0(
                float x[],
                float *d);

   void pythag(
               float x[],
               float y[],
               float *d);

   void sphereaway(
                   float c[],
                   float *rc,
                   float p[]);

   void sphereby(
                 float p[],
                 float c[],
                 float *rc,
                 float s[]);

   void normalize(
                  float n1[],
                  float n[]);

   void calipers(
                 int *maxelts,
                 int eltype[],
                 float *bv,
                 int *nelts,
                 int *m1,
                 float *rotations,
                 unsigned int *span_done,
                 float *span,
                 float *delta_span,
                 unsigned int *shadow_done,
                 float *shadow,
                 float *delta_shadow,
                 float *rlaunch,
                 int *mp,
                 char *id,
                 char *round,
                 unsigned int *savehits,
                 int P2,
                 int P1);

   void shine(
              int *maxelts,
              float *bv,
              int *nelts,
              float v[],
              float *rlaunch,
              int *kin);

   void blizzard(
                 int *maxelts,
                 int eltype[],
                 float *bv,
                 int *nelts,
                 int *m1,
                 float *tol,
                 float *rlaunch,
                 float *rotations,
                 float *cap,
                 float *alpha_bongo,
                 unsigned int *tol_given,
                 unsigned int *zeno_done,
                 float *delta_cap,
                 float *delta_bongo,
                 int *mz,
                 char *id,
                 double *tae,
                 double uae[],
                 double *vae,
                 double *wae,
                 char *round,
                 float bubble[],
                 float *bubble_rad,
                 int nebtab[],
                 int *nneb,
                 int *ninn,
                 float rlist[],
                 double strikes[],
                 unsigned int *savehits,
                 int P2,
                 int P1);

   void plus_or_minus(
                      int kk[],
                      char *pom,
                      int P1);

   void pade(
             float *alpha_bongo,
             float *q2pade,
             double eigens[],
             double xx[]);

   void ttdiag(double *tten,
               double eigens[]);

   void bridge(
               int *maxelts,
               int eltype[],
               float *bv,
               int *nelts,
               float *rotations,
               float v[],
               float *d);


   void distance(
                 int *maxelts,
                 int eltype[],
                 float *bv,
                 int *nelts,
                 float *rotations,
                 float p[],
                 float *ds,
                 float bubble[],
                 float *bubble_rad,
                 int nebtab[],
                 int *nneb,
                 int *ninn,
                 float rlist[],
                 int *nearto);


   void listersort(
                   float rlist[],
                   int *ninn,
                   int *nneb,
                   int nebtab[],
                   int *maxelts);

   void tally(
              int *khitp,
              int *khite,
              double *vp,
              double *ve,
              double sum[],
              float *aa,
              float *daa,
              float *cap,
              float *delta_cap,
              float *r1,
              double tae[],
              double uae[],
              double *vae,
              double *wae,
              float *rlaunch);

   void mean20(
               float xar[],
               float *x,
               float *delta);

   void accume(
               float t[],
               int kk[],
               int *khitp,
               int *khite,
               double *vp,
               double *ve,
               int *loop);

   void park(
             int *maxelts,
             int eltype[],
             float *bv,
             float *rotations,
             int *nelts,
             float p[],
             float *r,
             float *r2,
             unsigned int *hit,
             float *tol,
             float bubble[],
             float *bubble_rad,
             int nebtab[],
             int *nneb,
             int *ninn,
             float rlist[],
             int *hitelt);

   void greensphere(
                    float p[],
                    float *r,
                    float *r0,
                    unsigned int *gone);

   void reinit(
               int *khitp,
               int *khite,
               double *vp,
               double *ve,
               double sum[]);

   void charge(
               float rt[],
               float *r1,
               int kk[]);

   void sphere(
               float rt[],
               float *r1);

   void seeder();
//                char *dateline,
//                int P1);

   void gettime( char *start );

   int jrand(
             int *k1,
             int *k2);

   void wagaroo(
                int *maxelts,
                int eltype[],
                float *bv,
                int *nelts,
                float *rlaunch,
                float *rotations,
                int *m1,
                float *rg2int,
                float *delta_rg2int,
                float *volume,
                float *delta_volume,
                unsigned int *rg_done,
                int *mi,
                char *id,
                float q[],
                float sq[],
                float dsq[],
                char *round,
                unsigned int *savehits,
                float vaar[],
                double *tten,
                int P1,
                int P2);

   void toss_point(
                   float rt[],
                   int *maxelts,
                   int eltype[],
                   float *bv,
                   int *nelts,
                   float *rotations,
                   float vaar[],
                   float *volume);

   void toss_sphere(
                    float c[],
                    float *r,
                    float rt[]);

   void toss_cube(
                  float c[],
                  float *side,
                  float rt[]);

   void toss_pillar(
                    float x[],
                    float y[],
                    float zl[],
                    float zh[],
                    float rt[]);

   void toss_ellipsoid(
                       float c[],
                       float n1[],
                       float n2[],
                       float *aa,
                       float *bb,
                       float *cc,
                       float *tumble,
                       float rt[]);
   void toss_cylinder(
                      float c[],
                      float n[],
                      float *rad,
                      float *alen,
                      float *tumble,
                      float rt[]);

   void toss_torus(
                   float c[],
                   float n[],
                   float *r1,
                   float *r2,
                   float *tumble,
                   float rt[]);

   void primvol(
                int *myelt,
                float pass_vec[],
                float *tumble,
                float *result,
                unsigned int *early,
                char *mess,
                int P1);

   void pillarvol(
                  float x[],
                  float y[],
                  float zl[],
                  float zh[],
                  float *result);

   void all_around(
                   int *maxelts,
                   int eltype[],
                   float *bv,
                   int *nelts,
                   float xyzlow[],
                   float xyzhih[],
                   float *rlaunch,
                   float *rotations,
                   int *m1,
                   float *rg2int,
                   float *delta_rg2int,
                   float *volume,
                   float *delta_volume,
                   unsigned int *rg_done,
                   int *mi,
                   char *id,
                   float q[],
                   float sq[],
                   float dsq[],
                   char *round,
                   unsigned int *savehits,
                   double *tten,
                   int P1,
                   int P2);

   void inbody(
               float rt[],
               int *maxelts,
               int eltype[],
               float *bv,
               int *nelts,
               float *rotations,
               unsigned int *inside,
               unsigned int *early,
               char *mess,
               int P1);

   void captain(
                int *maxelts,
                int eltype[],
                float *bv,
                int *nelts,
                int *m1do,
                float *rotations,
                unsigned int *kirk_done,
                float saar[],
                float *kirk,
                float *delta_kirk,
                float *surface,
                float *delta_surface,
                float *rg2surf,
                float *delta_rg2surf,
                int *ms,
                char *id,
                char *round,
                unsigned int *savehits,
                int P1,
                int P2);

   void carea(
              int *maxelts,
              int eltype[],
              float *bv,
              int *nelts,
              float saar[],
              float *total);

   void getsurface(
                   int *maxelts,
                   int eltype[],
                   float *bv,
                   int *nelts,
                   float saar[],
                   float *total,
                   float p1[],
                   double trials[],
                   float *rotations,
                   int *loop);

   void pillarsurf(
                   float x[],
                   float y[],
                   float zl[],
                   float zh[],
                   float *sa,
                   float *bottom,
                   float *top,
                   float *side12,
                   float *side13,
                   float *side23);

   void trisurf(
                float v1[],
                float v2[],
                float v3[],
                float *sa);

   void ellsurf(
                float *aa,
                float *bb,
                float *cc,
                float *sa);

   void dovercube(
                  float c[],
                  float *side,
                  float p1[]);

   void doversphere(
                    float c[],
                    float *r,
                    float p1[]);

   void dovertriangle(
                      float p1[],
                      float p2[],
                      float p3[],
                      float tt[]);
   void doverdisk(
                  float c[],
                  float n[],
                  float *r,
                  float *t,
                  float p1[]);

   void backtransform(
                      float c[],
                      float *t,
                      float p1[]);

   void doversolcyl(
                    float c[],
                    float n[],
                    float *r,
                    float *al,
                    float *t,
                    float p1[]);

   void dovercylinder(
                      float c[],
                      float n[],
                      float *r,
                      float *al,
                      float *t,
                      float p1[]);

   void dovertorus(
                   float c[],
                   float n[],
                   float *r1,
                   float *r2,
                   float *t,
                   float p1[]);

   void doverellipsoid(
                       float c[],
                       float n1[],
                       float n2[],
                       float *aa,
                       float *bb,
                       float *cc,
                       float *t,
                       float p1[]);

   void doverpillar(
                    float x[],
                    float y[],
                    float zl[],
                    float zh[],
                    float p1[]);

   void dovertrap(
                  float v1[],
                  float v2[],
                  float v3[],
                  float v4[],
                  float p1[]);

   void sort3(
              float *as,
              float *bs,
              float *cs);

   void makepolar(
                  float p[],
                  float *theta,
                  float *phi);

   void tred2(
              float *a,
              int *n,
              int *np,
              float d[],
              float e[]);
   void tqli(
             float d[],
             float e[],
             int *n,
             int *np,
             float *z);

   int main( int argc, const char **argv )
   {
      /*
        Version 3.3
        Author:  Marc L. Mansfield
        Perform four different computations on bodies
        Zeno computation
        Computes the capacitance and polarizability of
        bodies by the "Zeno" algorithm, and then uses
        these to estimate transport properties.
        Interior computation
        Computes the volume, the radius of gyration, and the
        structure factor of the
        interior -- by a Monte Carlo integration -- by generating
        a set of points through the interior.
        Surface computation
        Computes the surface area, the radius of gyration
        of the surface, and the Kirkwood-hydrodynamic radius
        (i.e., the hydrodynamic radius as evaluated by the
        Kirkwood double sum <1/Rij> approximation) --  by Monte
        Carlo integration --  by generating
        surface points.
        Project-onto-a-line computation
        Computes the Giddings length, LG, which according to some models,
        is expected to be a predictor
        of the elution volume in size excusion chromatrography,
        Also computes the mean projection onto a plane.
        ========================================================
        References:
        Mansfield, Douglas, Garbozci
        "Intrinsic viscosity and the electrical
        polarizability of arbitrarily shaped objects,"
        Physical Review E, 64, 061401 (2001).
        Mansfield, Douglas
        "Numerical Path-Integration Calculation of Transport
        Properties of Star Polymers and DLA Aggregates," Condensed
        Matter Physics, 5, 249 (2002).
        Kang, Mansfield, Douglas
        "Numerical path integration technique for the calculation
        of transport properties of proteins,"
        Physical Review E, 69, 031918 (2004).
        Mansfield, Douglas, Irfan, Kang
        "Comparison of Approximate Methods for Calculating the
        Friction Coefficient and Intrinsic Viscosity of Nanoparticles
        and Macromolecules,"
        Macromolecules, 40, 2575-2589 (2007).
        Mansfield, Douglas
        "Transport properties of wormlike chains with applications
        to double helical DNA and carbon nanotubes," Macromolecules,
        41, 5412-5421 (2008).
        Mansfield, Douglas
        "Transport properties of rod-like polymers," Macromolecules,
        41, 5422-5432 (2008).
        Mansfield, Douglas
        "Improved path-integration method for estimating the intrinsic
        viscosity of arbitrarily-shaped particles," Physical Review E,
        Physical Review E, 78, 046712 (2008).
        ========================================================
        This is the command line:
        ./zeno  <identifier> <option1> <option2> <option3> <option4>
        Before issuing this command, you need to set up the body file,
        which specifies the body.  The body file must be created with
        the name <identifier>.bod
        <identifier> identifies the model.  Input (the
        specification for the model) is read in on the
        file <identifier>.bod and a report of the
        calculation is sent to <identifier>.zno
        You specify between 0 and 4 options, i.e.,
        not all four need to be given, but if you don't specify any,
        the program won't do anything.
        The options are also called action codes
        Because the options are cumulative, they can appear in any order,
        ========================================================
        Summary of the action codes:
        Each action code is a character string, with three
        parts, a one-byte prefix, a set of digits in the
        middle, and optionally, a suffix.
        The prefix is one of four bytes:
        z  Do the Zeno computation on the body, without recording hits
        Z  Do the Zeno computation on the body, and record all hits
        i  Do the interior integration on the body, without recording hits
        I  Do the interior integration on the body, and record all hits
        c  Do the covered-once-interior integration on the body,
        without recording hits
        C  Do the covered-once-interior integration on the body,
        and record all hits
        s  Do the surface computation on the body, without recording hits
        S  Do the surface computation on the body, and record hits
        p  Do the projection-onto-a-line computation on the body, with-
        out recording hits
        P  Do the projection-onto-a-line computation, and record hits
        Allowed suffixes are any of the three bytes:
        t = thousand
        m = million
        b = billion
        but these suffixes are optional.
        A few examples will be used to explain the action
        codes:
        z100t	requests that the zeno calculation be performed
        with 100 thousand random walkers
        i1b	requests that the interior computation
        be performed with one billion points
        in the interior of the body
        s5000000
        requests that the surface computation
        be performed with five
        million pairs of points distributed over the
        surface
        See below for limitations on both the surface and the
        interior calculations.
        You can specify as few as zero (which will result in
        no action being taken by the program), or as many as
        four action codes with each call.
        The program accepts at most four action codes, but that is because
        the interior and covered-once-interior actions are intended to
        be mutually exclusive.  If you try to execute one of each, you
        the later one overwrites the results of the former one.
        ========================================================
        The body is taken as the union of body elements.
        Each body element requires a set of values that specify
        size, shape, position, and spatial orientation.
        All the following body elements are now implemented.
        Also shown are variables for the values needed to
        specify the shape.
        SPHERE  cx cy cz   r
        TRIANGLE  v1x v1y v1z   v2x v2y v2z   v3x v3y v3z
        DISK  cx cy cz   nx ny nz   r
        CYLINDER  cx cy cz   nx ny nz   r   l
        SOL_CYL   cx cy cz   nx ny nz   r   l
        TORUS   cx cy cz   nx ny nz   r1   r2
        ELLIPSOID  cx cy cz   n1x n1y n1z   n2x n2y n2z   aa bb cc
        CUBE  cx cy cz   s
        PILLAR x1 y1 zl1 zh1 x2 y2 zl2 zh2 x3 y3 zl3 zh3
        ========================================================
        The complete body is specified by supplying a "body file."
        This is a file with the name:

        <identifier>.bod

        where <identifier> represents a character string that
        identifies the model.
        The data in the bod file consists of a series of commands, which
        have the general form:
        <predicate> <modifier1> <modifier2> <modifier3> ...
        The symbols <predicate> and <modifierx> each represent a character
        string, and these are delimited in the file by spaces or carriage
        returns.  The program accepts more than one string per line, up
        to a total of 80 characters per line.
        The modifiers belong to, or modify, the predicate, and
        each predicate along with its modifiers constitutes
        a single "command."
        The modifiers to each predicate come in a specific order
        following that predicate, and each predicate requires
        a specific number of modifiers.
        There are no punctuation marks flagging the end of
        one command or the beginning of another.  The command
        is defined as a valid predicate followed by the correct
        number of modifiers.  However, for ease in reading,
        the user will probably want to design the body file
        with carriage returns between commands.
        Usually (but not always) the modifiers are strings
        representing numbers.  Such strings are called "numeric
        strings," or simply "numbers."  A valid numeric
        string is any character string that can be interpreted
        by the fortran internal-read, free-format command:
        read(string,*) value
        To process the file, the program looks at the first
        string on the file.  This string must be a valid predicate.
        If it is not, then the program aborts.  Then, the program
        takes the next N strings, where N is the number of
        modifiers required for this particular predicate.  The
        program aborts if it has trouble interpreting any of
        the modifiers.  Assuming these N strings are interpreted
        successfully, the program repeats, reading a predicate
        and its modifiers, until it encounters the end of the
        file.
        The following paragraphs summarize each command-type,
        giving the valid predicate or predicates, and the valid
        modifiers.  And they also briefly explain the action
        of the command.  Full definitions of each modifer are
        given later.  Most of the commands add a body-element
        to the growing body.  All the others are used to set
        the value of a variable or other parameter.  Most of the
        commands have several synonymous predicates.  E.g.,
        the four strings "SPHERE", "sphere", "S", "s" are
        all valid predicates for the "ADD A SPHERE" command.
        The order of the modifiers as given in the following
        paragraphs must be followed in each command of the body
        file.
        ------------------------------------------------------
        ADD A SPHERE
        Valid predicates:
        SPHERE, sphere, S, s
        Internal code: sphere_code
        Modifiers:  Four numbers.
        cx, cy, cz, r
        Action:  Adds a sphere to the list of body elements.
        See below for definitions of cx, cy, cz, r.
        Example:   SPHERE  0 2 0 5
        This would add a sphere of radius 5 at the point (0,2,0)
        ------------------------------------------------------
        ADD A TRIANGLE
        Valid predicates:
        TRIANGLE, triangle, T, t
        Internal code: triangle_code
        Modifiers:  Nine numbers.
        v1x,v1y,v1z,v2x,v2y,v2z,v3x,v3y,v3z
        Action:  Adds a triangle to the list of body elements.
        See below for definitions of v1x,v1y,v1z,v2x,
        v2y,v2z,v3x,v3y,v3z.
        Example:  TRIANGLE  0 0 0   5 0 0    0 2 0
        This would add a triangle with vertices (0,0,0), (5,0,0), and
        (0,2,0) to the body.
        ------------------------------------------------------
        ADD A CIRCULAR DISK
        Valid predicates:
        DISK, disk, D, d
        Internal code: disk_code
        Modifiers:  Seven numbers.
        cx,cy,cz,nx,ny,nz,r
        Action:  Adds a circular disk to the list of body elements.
        See below for definitions of cx,cy,cz,nx,ny,nz,r.
        Example:  DISK  0 0 0    0 0 1   5
        This would add a disk of radius 5, centered at the point (0,0,0),
        and having for outward normal, the vector (0,0,1).
        ------------------------------------------------------
        ADD AN OPEN CYLINDER
        Valid predicates:
        CYLINDER, cylinder
        Internal code: open_cylinder_code
        Modifiers:  Eight numbers.
        cx,cy,cz,nx,ny,nz,r,l
        Action:  Adds an open cylinder to the list of body elements.
        See below for definitions of cx,cy,cz,nx,ny,nz,r,l.
        Example:  CYLINDER  0 0 0   0 0 2   5  3
        This would add an open cylinder of radius 5 and and height 3,
        centered at the point (0,0,0), and with its symmetry axis
        parallel to the vector (0,0,2)
        ------------------------------------------------------
        ADD A SOLID CYLINDER
        Valid predicates:
        SOL_CYL, sol_cyl, SC, sc
        Internal code:  solid_cylinder_code
        Modifiers:  Eight numbers.
        cx,cy,cz,nx,ny,nz,r,l
        Action:  Adds a solid cylinder to the list of body elements.
        See below for definitions of cx,cy,cz,nx,ny,nz,r,l.
        Example:  SOL_CYL  0 0 0   0 0 2   5  3
        This would add a solid cylinder of radius 5 and and height 3,
        centered at the point (0,0,0), and with its symmetry axis
        parallel to the vector (0,0,2)
        ------------------------------------------------------
        ADD A TORUS
        Valid predicates:
        TORUS, torus, TO, to
        Internal code:  donut_code
        Modifiers:  Eight numbers.
        cx,cy,cz,nx,ny,nz,r1,r2
        Action:  Adds a torus to the list of body elements.
        See below for definitions of cx,cy,cz,nx,ny,nz,r1,r2.
        Example:  TORUS   0 0 0    0 0 1   5 2
        This would add a torus centered at (0,0,0), with symmetry
        axis parallel to the vector (0,0,1), and with r1 and r2
        values of 5 and 2, respectively.
        ------------------------------------------------------
        ADD AN ELLIPSOID
        Valid predicates:
        ELLIPSOID, ellipsoid, E, e
        Internal code:  ellipsoid_code
        Modifiers:  Twelve numbers.
        cx,cy,cz,n1x,n1y,n1z,n2x,n2y,n2z,aa,bb,cc
        Action:  Adds an ellipsoid to the list of body elements.
        See below for definitions of cx,cy,cz,n1x,n1y,n1z,
        n2x,n2y,n2z,aa,bb,cc.
        Example:  ELLIPSOID  0 0 0   1 0 0   0 1 0   5 3 2
        This would add an ellipsoid centered at the point (0,0,0).  One
        semi-axis of length 5 is aligned parallel to the vector (1,0,0).
        A second semi-axis of length 3 is aligned parallel to the vector
        (0,1,0).  The third semi-axis has length 2, and is aligned
        parallel to the cross-product of the other two axes.
        ------------------------------------------------------
        ADD A CUBE
        Valid predicates:
        CUBE, cube
        Internal code:  cube_code
        Modifiers:  Four numbers.
        cx,cy,cz,s
        Action:  Adds a cube to the list of body elements.
        See below for definitions of cx,cy,cz,s.
        Example:  CUBE  -0.5  -0.5  -0.5    1
        This would add a unit cube centered at the origin.
        ------------------------------------------------------
        ADD A PILLAR
        Valid predicates:
        PILLAR, pillar, PI, pi
        Internal code:  pillar_code
        Modifiers:  Twelve numbers
        x1 y1 z1l z1h x2 y2 z2l z2h x3 y3 z3l z3h
        Action:  Adds a "pillar" to the list of body elements.
        See below for definitions of:
        x1 y1 z1l z1h x2 y2 z2l z2h x3 y3 z3l z3h
        Example:  PILLAR  0 0 0 2   1 0 0 3   0 1 0 5
        This would add a "pillar."
        ------------------------------------------------------
        SPECIFY VALUE OF SKIN THICKNESS
        Valid predicates:
        ST, st
        Internal code: skin_code
        Modifiers:  One number, the value of the skin thickness.
        Action:  Specifies the value of the skin thickness, epsilon.
        Note:  The skin thickness is only used in the zeno
        calculation.  You can use this command to assign a
        value for the skin thickness.  However, the command
        is optional, if not given then, as default, the
        program uses the value of 1.0e-6 times the launch
        radius.
        Example:  ST 0.0001
        This would set the skin-thickness value as 0.0001
        ------------------------------------------------------
        SPECIFY LENGTH UNITS  (1st version)
        (There are two versions of SPECIFY LENGTH UNITS)
        Valid predicates:
        UNITS, units
        Internal code:  units_code
        Modifiers:  This command takes a single alphabetic
        string as a modifier.  It does not take number
        modifiers.  Only one of the following
        five strings will be accepted as the modifier.
        Modifier      Internal
        string         code	    Meaning
        -------       --------      -------
        m          meter_code    meters
        cm         cm_code       centimeters
        nm         nm_code       nanometers
        A          angstrom_code Angstroms
        L          length_code   generic or unspecified length units
        Action:  Specifies the length units in which all
        values are expressed.
        Note:  This command is optional.  If either it  or
        the second version (below) is not found
        in the body file, the value "L" will be assigned for
        units, indicating generic or unspecified length units.
        Example:   UNITS nm
        This would indicate that all lengths are in nanometers.
        ------------------------------------------------------
        SPECIFY LENGTH UNITS  (2nd version)
        (There are two versions of SPECIFY LENGTH UNITS)
        Valid predicates:
        HUNITS, hunits
        Internal code:  hunits_code
        Modifiers:  This command takes first a number and then a
        string.  It gives the length units in which all values
        are expressed.  This gives greater versatility in
        length unit specification.
        For example, this line would instruct the program that all
        lengths given in the body file are in micron units:
        HUNITS 1000 nm
        Note that these two commands are equivalent:
        HUNITS 1 nm
        UNITS nm
        Only the following four strings are permissible modifiers:
        Modifier      Internal
        string         code	    Meaning
        -------       --------      -------
        m          meter_code    meters
        cm         cm_code       centimeters
        nm         nm_code       nanometers
        A          angstrom_code Angstroms
        Note:  This command has been added to permit compatibility
        with the program HYDRO.
        HYDRO reads an line that tells it how to convert its internal
        length units to centimeters.
        The program h2z.f converts HYDRO input files to make them
        compatible with ZENO bod files.  It converts the first line
        of the hydro input file into the HUNITS command.
        Note:  This command is optional.  If either it or the
        first version (above) is not found
        in the body file, the value "L" will be assigned for
        units, indicating generic or unspecified length units.
        Example:  HUNITS 1 nm
        This would imply that all lengths are given in units of one nm.
        ------------------------------------------------------
        SPECIFY TEMPERATURE
        This command is optional.  It needs to be present if
        you want the program to compute the diffusion coefficient
        from the Stokes-Einstein formula.
        Valid predicates:
        TEMP, temp
        Internal code: temp_code
        Modifiers:  The predicate must be followed by two modifiers.
        The first is a number giving the temperature, the second is
        a string that gives its units.
        Valid temperature unit modifiers:
        C	Celcius (Internal code:  celcius_code)
        K	Kelvin	(Internal code:  kelvin_code)
        Example:   TEMP  20 C
        This would set the temperature to be 20 Celcius.
        ------------------------------------------------------
        SPECIFY MASS
        This command is optional.  It needs to be present if you want
        the program to compute the intrinsic viscosity in conventional
        units.
        Valid predicates:
        MASS, mass
        Internal code:  mass_code
        Modifiers:  The predicate must be followed by two modifiers,
        The first is a number giving the value of the mass, and the second
        is a character string specifying the units.
        Valid mass unit codes:
        Da		Daltons  (Internal code: da_code)
        kDa		kilodaltons  (internal code:  kda_code)
        g		grams  (Internal code:  gram_code)
        kg		kilograms  (internal code:  kg_code)
        Example:   MASS   324 kDa
        This would set the mass at 324 kDa.
        ------------------------------------------------------
        SPECIFY SOLVENT VISCOSITY
        This command is optional.  Either it or the "SPECIFY SOLVENT = WATER"
        command needs to be present if you want program to calculate the
        diffusion coefficient by the Stokes-Einstein formula.
        Valid predicates:
        VISCOSITY, viscosity
        Internal code:  visc_code
        Modifiers:  The predicate must be followed by two modifiers.
        The first is a number giving the value of the solvent viscosity,
        the second gives its units.
        Valid viscosity units codes:
        p	poise   	(Internal code: poise_code)
        cp	centipoise	(Internal code: cp_code)
        Example:   VISCOSITY  1.004 cp
        This would set the solvent viscosity at 1.004 cp.
        ------------------------------------------------------
        SPECIFY SOLVENT AS WATER
        This command is optional.  If you want the program to calculate
        the diffusion coefficient by the Stokes-Einstein formula, it will
        need the solvent viscosity.  You can enter the viscosity using the
        "SPECIFY SOLVENT VISCOSITY" command, or you can issue this command,
        and it will use a standard formula  (CRC handbook of chemistry
        and physics, 55th edition p. F49)  for the viscosity of pure
        water as a function of temperature.  But for this to work, you
        will also have to specify the temperature in a separate command.
        Valid predicates:
        SOLVENT, solvent	(Internal code: solvent_code)
        The command takes only one modifier, and at present, only one of
        the following two modifiers is accepted.
        Valid modifiers:
        WATER, water		(Internal code: water_code)
        At some future date, it may be possible to include other solvents
        in this command.
        Example:  SOLVENT WATER
        This would direct the program to compute the solvent viscosity
        using a standard formula for water.
        ------------------------------------------------------
        SPECIFY BUOYANCY FACTOR
        Valid predicates:
        BF, bf
        Modifiers:  The predicate takes one modifier, the numerical value
        of the buoyancy factor.  Since it is dimensionless, no units are
        included.  Used to compute sedimentation coefficient.
        ------------------------------------------------------
        SPECIFY LAUNCH RADIUS
        Valid predicates:
        RLAUNCH, rlaunch  (User-defined launch radius, to use
        in place of radius determined
        automatically.)
        This command is optional.  The program determines the launch
        radius automatically.  However, when we are planning to do a
        conformational average, all the computations on each member of the
        ensemble have to be done with the same value for the launch
        radius, and we can use this command to override the computed
        value.
        This command only overrides the computed value if the computed
        value for the launch radius is less than or equal to the value
        supplied by the command.  Otherwise, the program aborts.
        Example:  RLAUNCH  45
        This sets the launch radius at 45.
        ------------------------------------------------------
        THE ORDER OF THE COMMANDS IS NOT IMPORTANT.
        That is because the complete body is the union of all the body
        elements, and therefore the elements can be listed in any order.
        All other commands set the values of variables, and they can
        be interspersed with the body-element commands.
        The body file can also contain comments.  These
        are any lines in the file that begin with an asterisk, *.

        The end of the body file is its physical end.
        The program continues processing the body file until
        it hits the end.
        Example of a body file (the | symbol shows the left and right
        margins):
        +--------------------------------------------------------------+
        |*  This body consists of 5 spheres                            |
        |*  Blank lines are OK:                                        |
        |                                                              |
        |*  This line inserts a sphere of radius 1 at the origin       |
        |SPHERE 0 0 0 1                                                |
        |                                                              |
        |*  The next line inserts a sphere of radius 2 tangent         |
        |*  to the first sphere with center on the x axis              |
        |S 3 0 0 2                                                     |
        |                                                              |
        |*  Carriage returns are permissible during the specification  |
        |*  of any one element:                                        |
        |sphere -3 0 0                                                 |
        |            2                                                 |
        |                                                              |
        |*  You can also run different elements together on            |
        |*  the same line                                              |
        |                                                              |
        |     S 1 1 1 1   s -1 -1 -1 1                                 |
        |* This gives the skin thickness                               |
        |  st   0.00001                                                |
        |                                                              |
        !* This command establishes nanometers as the length unit      |
        |  units nm                                                    |
        +--------------------------------------------------------------+
        Following is a detailed specification of each type of body element.
        Some body elements are space-filling, i.e., they have non-zero
        volume, otherwise they are not.
        Space-filling bodies are:  SPHERE, SOL_CYL, TORUS, ELLIPSOID,
        CUBE, and PILLAR
        Non-space-filling bodies are:  TRIANGLE, DISK, and CYLINDER
        =====================================================================
        SPHERE  cx cy cz  r
        real*4 c(3),r
        A sphere centered at c=(cx,cy,cz) with radius r.
        =====================================================================
        TRIANGLE  v1x v1y v1z   v2x v2y v2z   v3x v3y v3z
        real*4 v1(3),v2(3),v3(3)
        v1, v2, v3 are the three vertices of the triangle.
        =====================================================================
        DISK  cx cy cz   nx ny nz  r
        real*4 c(3),n(3),r
        A disk centered at c=(cx,cy,cz).
        The vector n=(nx,ny,nz) is the "unit normal," i.e., it points
        in the direction normal to the plane of the disk.  The
        vector n may be unnormalized on entry; it is automatically
        normalized upon input.
        The radius of the disk is r.
        ====================================================================
        CYLINDER  cx cy cz   nx ny nz  r l
        real*4 c(3),n(3),r,l
        The cylinder is open; i.e., a tin can with both lids cut off.
        To do a closed cylinder, use the SOL_CYL body element.
        c is the geometric center of the cylinder.  n is a vector
        pointing in the direction of the cylinder axis.  n
        may be supplied unnormalized -- it will
        be normalized automatically.
        r is the radius, l is the total length; so it extends from
        -l/2 to l/2.
        =====================================================================
        TORUS  cx cy cz   nx ny nz  r1 r2
        real*4 c(3),n(3),r1,r2
        A torus centered at c.   n is a vector pointing in the
        direction of the symmetry axis.  n may be supplied
        unnormalized -- it will be
        automatically normalized upon input.

        r1 and r2 define the dimensions of the torus:
        x                         x
        x       x                 x       x
        x         x               x         x   ___
        x         x               x         x    |
        x       x                 x       x     |  r2
        x                         x        ---
        |-----------2 * r1 -------|
        ======================================================================
        ELLIPSOID  cx cy cz   n1x n1y n1z   n2x n2y n2z  aa bb cc
        real*4 c(3),n1(3),n2(3),aa,bb,cc
        Center of ellipsoid is at c;
        n1 is a vector parallel to the aa axis;
        n2 is a vector parallel to the bb axis;
        n1 and n2 may be entered as unnormalized; they are normalized
        automatically  upon input.  It is assumed that n1 and n2 are
        orthogonal; unpredictable results will occur if not.
        The third axis, n3, is calculated by taking
        the cross product of n1 and n2.  Therefore, it is not
        entered.  aa is the length of the
        semi-axis along n1, bb along n2, and cc along n3.
        So for example, if (cx,cy,cz) were the origin,
        if n1, n2, and n3 were respectively the x, y, and z axes,
        then aa, bb, cc are defined such that this
        (x/aa)**2 + (y/bb)**2 + (z/cc)**2 = 1
        is the equation of the ellipsoid.
        ====================================================================
        CUBE  cx cy cz  s
        real*4 c(3),s
        Lower corner at (cx,cy,cz), s is the side
        Currently we only do cubes that are oriented parallel
        to the cartesian coordinates.
        ====================================================================
        PILLAR x1 y1 z1l z1h x2 y2 z2l z2h x3 y3 z3l z3h
        real*4 x1,x2,x3,y1,y2,y3,z1l,z2h,z3l,z1h,z2h,z3h
        A pillar is a body element that is thought of as lying
        parallel to the z-axis. It is a triangle in cross-section,
        and if you project it onto the z-axis,
        you get the triangle (x1,y1); (x2,y2); (x3,y3).
        It has two triangular faces at the top and the bottom, which
        are the triangles [(x1,y1,z1l); (x2,y2,z2l); (x3,y3,z3l)]
        and [(x1,y1,z1h); (x2,y2,z2h); (x3,y3,z3h)]
        It is assumed that z1l < z1h, z2l < z2h, and z3l < z3h.
        ====================================================================
        ZENO CALCULATIONS
        You call for a zeno calculation on the body whenever you use
        the z prefix on any of the action_codes.
        The electrostatic capacitance and the electrostatic
        polarizability are calculated according to the numerical
        path integration approach described in:
        Mansfield, Douglas, Garbozci
        "Intrinsic viscosity and the electrical
        polarizability of arbitrarily shaped objects,"
        Physical Review E, 64, 061401 (2001).
        Kang, Mansfield, Douglas
        "Numerical path integration technique for the calculation
        of transport properties of proteins,"
        Physical Review E, 69, 031918 (2004).
        Mansfield, Douglas
        "Improved path-integration method for estimating the intrinsic
        viscosity of arbitrarily-shaped particles," Physical Review E,
        Physical Review E, 78, 046712 (2008).
        ====================================================================
        INTERIOR CALCULATIONS
        You call for an interior calculation if you use the
        i prefix on any of the action_codes.
        The interior calculation is done by generating a set
        of points on the interior of the body, and doing a numerical
        integration over this set.  It is therefore limited to bodies
        that are unions of space filling elements:  spheres, cubes,
        solid cylinders,
        tori, and ellipsoids.  If you call for the interior
        calculation given bodies formed from other elements,
        the program omits the calculation.
        This calculation determines the volume of the body, the
        radius of gyration of the interior, and it calculates the
        structure factor at a set of momentum points scaled by the
        launch radius.  If the launch sphere has radius X, then the
        structure factor is determined at q-values between 0.01/X and
        100/X.
        The structure factor is defined as
        (1/V**2) INT_i INT_j < exp ( i q_vec dot r_ij ) >
        where q_vec is the scattering vector, q is its length, r_ij is
        the displacement vector between volume elements i and j, and
        INT_i and INT_j are volume integrals.  This calculation also
        determines a length parameter L_half, which is 1/q for the
        q-value at which S(q) = 1/2.  The value is determined by linear
        interpolation after the structure factor has been determined.
        Of course, you can create a space-filling body using a union
        of triangles distributed over the surface, but since it is
        difficult to determine a priori that the body so built is
        space-filling, we just will not try it here.  Bottom line:
        Only bodies built up from spheres, tori, cubes, solid cylinders,
        and ellipsoids will be treated with the interior calculation.
        If the other body elements (triangles, disks, open cylinders)
        are found in the body, an interior calculation will not be done,
        even if one is requested.
        The set of points is obtained by generating random points inside
        a rectangular prism, and retaining only those that are also
        inside the body.
        ======================================================================
        COVERED-ONCE INTERIOR CALCULATIONS -- or C1 INTERIOR
        This is in most ways equivalent to the INTERIOR CALCULATION,
        except that it can be much faster in situations where it can
        be applied.  Both of these interior integrations can be
        performed whenever the body formed as the union of space-filling
        elements.  The C1 INTERIOR calculation can be applied if the
        elements do not overlap, i.e., each point of the body is covered
        by one and only one primitive element, or each point is "covered
        once."  Obviously, the lack of overlap means that the volume
        can be calculated directly, and interior points can be generated
        much more rapidly.
        No effort is made by the program to determine if there is
        any overlap.  The user is on his own to verify this, and
        obviously, the results returned by the integration will be
        invalid if they are.
        ======================================================================
        SURFACE CALCULATION
        You call for a surface calculation by supplying the prefix s
        on one of the action_codes.

        The surface calculation works by generating a set of points
        distributed randomly over the surface.  It provides an
        estimate of the surface area, the radius of gyration of the
        surface, and an estimate of the hydrodynamic radius using
        the Kirkwood double sum formula.
        The capacitance-hydrodynamic radius analogy results from
        pre-averaging the Oseen tensor over orientations.  The
        well-known formula of Kirkwood for the diffusion constant
        approximates the hydrodynamic radius as the harmonic
        mean distance between arbitrary pairs of points on the
        surface.
        For the purposes
        of this calculation, a point is defined to be on the surface
        if it is on the surface of one of the body elements, and
        not inside any other space-filling body element.
        Therefore, if your body is constructed so that any of the
        above points are not accessible to the solvent, the
        results will not be trustworthy.  An example of when this
        could happen is if you use a triangular grid to set up a
        closed surface, and then you put something like a sphere
        so that it is partly outside and partly inside the triangular
        grid.  Those inside points will be counted as part of
        the surface.
        ======================================================================
        PROJECTION-ON-A-LINE CALCULATION
        You call for a projection-onto-a-line calculation by supplying
        the prefix p in any one of the action codes.
        The calculation works by generating a large number of randomly
        oriented unit vectors in random orientations.  For any given
        unit vector, it determines the two planes normal to the vector,
        and tangent to the object, which do not intersect the object.  It
        takes the average distance between these two planes,
        averaged over all orientations.  The Giddings length
        is half this average distance.
        ======================================================================
        NUMERICAL ERROR ESTIMATES
        The program performs numerical error estimates on all computations.
        The following rules apply:
        (1)  Significant figures of input variables.  The input values of
        mass, viscosity, temperature are all assumed to have a precision
        equal to 0.5 times the place value of the last digit given.
        (2)  Sampling errors of all Monte Carlo integrations are estimated
        by evaluating each integral independently 20 times, and setting
        the sampling error equal to (sigma)/sqrt(20), where sigma
        is the standard deviation of the 20 results.
        (3)  Electrostatic-hydrodynamic analogy.
        The hydrodynamic radius and volume are rounded to reflect the
        1% and 1.5% error that results from the electrostatic-hydrodynamic
        analogy.
        (4)  Errors propagate through all subsequent calculations.
        (5)  The final results are reported with rounding to the last
        significant digit.  The uncertainty in the last digit
        appears in parentheses in the computation report.
        (6)  The unrounded results are reported in an auxiliary file,
        <identifier>.znr (zeno non-rounded)
        ======================================================================
        SOME NOTES ON UNITS, DEFINITIONS, AND CONVENTIONS
        It is assumed that all parameters supplied by the .bod
        file are in the same set of length units.
        You have the option of specifying these units as either
        meters, centimeters, nanometers, or Angstroms using one of the
        two versions of
        the "specify units" command.  However, if you omit
        this command, the program just assumes generic length units.
        The capacitance is reported in length units, such that
        the capacitance of a sphere equals its radius.
        If the length-dimensions have also been set, then the
        capacitance is also reported in farads.
        The polarizability tensor is reported in volume units.
        The form of the polarizability tensor calculated internally
        by this program will be called alpha.bongo
        The form of the polarizability tensor that is output
        will be called alpha.paper.  alpha.paper is the
        definition that coincides with the Mansfield, Douglas,
        Garbozci paper.
        alpha.bongo	= polarizability tensor calculated
        internally
        by this program (units: volume)
        alpha.paper	= polarizability tensor as defined
        in Mansfield-Douglas-Garbozci
        paper and as output (units: volume)
        pi = 3.14159265...
        Then:
        alpha.paper = 4 * pi * alpha.bongo
        ===========================================================
        INTEGER CODES
        Internal codes are defined for each shape class,
        for the predicate of each command, and for the alphabetical
        modifiers that accompany each command.  These codes
        are given descriptive names by subroutine establish_codes,
        and are stored in common block morse.
        ===========================================================
        FILE NAMES AND FILE UNITS
        As many as 10 files will be created.
        File names and file units are established in subroutine setup.
        bod	"body" file -- input file for body specification and
        input variables -- opened by parse, closed by parse
        name and unit:    fbod, nbod
        zno	"zeno" file -- main output file for results --
        opened by setup, closed by main program just before
        stopping -- name and unit:  fzno, nzno
        znr	"zeno-not-rounded" file -- same output as zno but
        without rounding -- opened by poplar, closed by poplar
        name and unit:    fznr, nznr
        stk	hits on body elements during path-integration --
        opened and closed by blizzard
        name and unit:   fstk, nstk
        dfl	Unix time stamp at beginning
        efl	Unit time stamp at end
        (time stamps are copied directly to zno file, so it
        is OK to delete these)
        zh	hits during path-integration, open and closed by
        blizzard.   name and unit:   fzh, nzh
        ih	hits during interior-integration, i.e., set of
        points distributed randomly throughout the interior.
        name and unit:  fih, nih
        sh	hits during surface-integration, i.e., set of points
        distributed randomly over the surface.
        name and unit:   fsh,  nsh
        ph	hits during projection-integration, i.e., projection
        of object on random directions.
        name and unit:  fph, nph
        ===========================================================
      */

      typedef struct {
         char fbod[USZ_MAX_ID_SIZE];
         char fzno[USZ_MAX_ID_SIZE];
         char fznr[USZ_MAX_ID_SIZE];
         char fstk[USZ_MAX_ID_SIZE];
         char fdfl[USZ_MAX_ID_SIZE];
         char fefl[USZ_MAX_ID_SIZE];
         char fzh[USZ_MAX_ID_SIZE];
         char fih[USZ_MAX_ID_SIZE];
         char fsh[USZ_MAX_ID_SIZE];
         char fph[USZ_MAX_ID_SIZE];
      } Cfilenames;
      // static Cfilenames *Tfilenames = (Cfilenames*) Xfilenames;
      typedef struct {
         int nbod;
         int nzno;
         int nznr;
         int nstk;
         int ndfl;
         int nefl;
         int nzh;
         int nih;
         int nsh;
         int nph;
      } Cfilenumbers;
      static Cfilenumbers *Tfilenumbers = (Cfilenumbers*) Xfilenumbers;
      typedef struct {
         int ncube;
         float ferr;
      } Ccubit;
      static Ccubit *Tcubit = (Ccubit*) Xcubit;
      typedef struct {
         int nell;
         float rerr;
      } Csell;
      static Csell *Tsell = (Csell*) Xsell;
      typedef struct {
         unsigned int silent;
      } Cshush;
      static Cshush *Tshush = (Cshush*) Xshush;
      typedef struct {
         int sphere_code;
         int triangle_code;
         int disk_code;
         int open_cylinder_code;
         int solid_cylinder_code;
         int donut_code;
         int ellipsoid_code;
         int cube_code;
         int pillar_code;
         int skin_code;
         int units_code;
         int hunits_code;
         int meter_code;
         int cm_code;
         int nm_code;
         int angstrom_code;
         int length_code;
         int temp_code;
         int celcius_code;
         int kelvin_code;
         int mass_code;
         int da_code;
         int kda_code;
         int gram_code;
         int kg_code;
         int visc_code;
         int poise_code;
         int cp_code;
         int solvent_code;
         int water_code;
         int rlaunch_code;
         int bf_code;
      } Cmorse;
      puts("z1");
      // static Cmorse *Tmorse = (Cmorse*) Xmorse;
      static int T1,T2;
      static int eltype[MAXELTS];
      static float bv[12][MAXELTS],rotations[3][3][MAXELTS],saar[MAXELTS],vaar[MAXELTS];
      static double strikes[MAXELTS];
      static char id[USZ_MAX_ID_SIZE],round[80];
      static int m1[4],mz,mi,ms,mp,m1do,nelts;
      static float tol,xyzlow[3],xyzhih[3],cap,delta_cap,alpha_bongo[3][3],delta_bongo[3][3],volume,delta_volume,surface,
         delta_surface,rg2int,delta_rg2int;
      static double tten[3][3];
      static float q[82],sq[82],dsq[82],rg2surf,delta_rg2surf,kirk,delta_kirk,span,delta_span;
      static unsigned int zeno_done,kirk_done,rg_done,span_done,launch_done,tol_given,shadow_done;
      static double tae,uae[3],vae[3][3],wae[3][3],eigens[3],xx[2];
      static char actions[4];
      static unsigned int savehits_z,savehits_i,savehits_s,savehits_p;
      static char actout[USZ_MAX_ID_SIZE_P_5_T_4];
      // static char putout[USZ_MAX_ID_SIZE_PLUS_5];
      static char dictionary[1000];
      static int map[MAXWORDS],nwords;
      static char unitcode[2];           /*  Code for length units, five */
      /*
        allowed values:  'L ',
        'm ','cm','nm','A '.
      */
      static unsigned int bt,bm,bw,bc,bbf;
      static double temp[2];             /*  Input temp, mass, viscosity */
      static double mass[2],visc[2],buoy[2];/*  Input buoyancy factor */
      static char tunit[6];              /*  Units for temp, mass, visc. */
      static char munit[6],vunit[6];
      static float hscale;               /*  Length scale parameter set by */
      /*
        hunits command
      */
      static char start[29];             /*  Time stamps */
      static char endend[29];
      /*
        These are data structures needed for the distance computation:
      */
      static int nneb,nebtab[MAXELTS],ninn;
      static float bubble[3],bubble_rad,rlist[MAXELTS];
      // static int nerr;
      static float rlaunch,rlaunch1;
      static int j,i;
      static float q2pade,shadow,delta_shadow;
      static const char* F861[] = {
         "('  Invalid launch radius specified.')"
      };
      static const char* F902[] = {
         "('Launch radius = ',g15.7)"
      };
      static const char* F842[] = {
         "('XYZ(low)  = ',3g15.7)"
      };
      static const char* F843[] = {
         "('XYZ(high) = ',3g15.7)"
      };
      static const char* F321[] = {
         "('END:   ',a28)"
      };
      ftnini(argc,argv,NULL);
      /*
        actions !  Codes for the actions to take:
        !  z = zeno calculation,
        !  s = surface calculation,
        !  i = interior calculation,
        !  c = C1 interior calculation,
        !  p = project-onto-line calculation
        !  Upper-case versions indicate the same thing
        !  but with recording of hits
        eltype	!  A code for each element type.
        !  It maps to one of the nine variables: sphere_code,
        !     triangle_code, disk_code, open_cylinder_code,
        !     solid_cylinder_code, donut_code, ellipsoid_code,
        !     cube_code, pillar_code
        nelts	!  The total number of body elements
        bv	!  "Body values,"  bv(i,j),j=1,12 are all the
        !  floating point numbers needed to specify the
        !  body
        id	!  character string that identifies the body
        !  it is also used to make up the input and output
        !  file names
        start	!  time-stamp at beginning of run
        endend  !  time-stamp at end of run
        nin	!  file unit for the input/body file
        m1(i)	!  Total number of Monte carlo steps to be
        !  used in the i-th computation.
        rotations  !  For disks, cylinders, and tori:
        !  This stores
        !  a rotation matrix required to rotate the
        !  axis of the element into the z-axis.
        !  For ellipsoids, it stores a rotation matrix
        !     required to rotate n1 to x-axis, n2 to
        !     y-axis, n3 to z-axis
        saar(j)   !  The surface area of the j-th element
        vaar(j)   !  The volume of the j-th element -- only computed
        !    if subroutine wagaroo is entered
        tol   !  Skin thickness
        xyzlow, xyzhih    !  These define the coordinates of a box
        !  that envelops the body.  Used to generate
        !   interior points.
        cap,delta_cap   !  The capacitance and its uncertainty
        alpha_bongo,delta_bongo
        The polarizability tensor, and its
        uncertainty
        tae,uae,vae,wae are being calculated and reported for
        the cases in which an ensemble average will be taken
        eigens		!  The three eigenvalues of the polarizability
        xx		!  The shape-space coordinates of
        the polarizability
        q2pade		!  The proportionality between polarizability
        and intrinsic viscosity, as
        determined by Pade approximation
        from the eigenvalues of the polariz.
        volume,delta_volume
        The volume and its uncertainty
        surface,delta_surface
        The surface area and its uncertainty
        rg2int, delta_rg2int
        The square radius of gyration of the
        interior, and its uncertainty
        tten		!  Rg**2 tensor
        q		!  A set of momentum-values at which the
        structure factor is calculated
        sq, dsq		!  Values of the structure factor and its
        uncertainty.
        rg2surf, delta_rg2surf
        The square radius of gyration of the
        surface, and its uncertainty
        kirk, delta_kirk
        The kirkwood estimate of the hydrodynamic
        radius
        span, delta_span
        The Giddings length and its uncertainty
        zeno_done	!  The zeno calculation terminated successfully
        kirk_done	!  The surface calculation terminated successfully
        rg_done		!  The interior calculation terminated successfully
        span_done	!  The project-onto-line
        calculation terminated successfully
        shadow_done	!  The projection-onto-plane calculation was done
        launch_done	!  The launch sphere has been generated
        tol_given	!  The skin thickness was supplied in the bod file
        ncube,ferr	!  Sometimes, when using the cube body element,
        !  some points can be found "slightly" inside the
        !  cube.  If these values get set -- the user
        !  will be warned by an error that this has happened,
        !  and the user will be told how far inside the
        !  cube the offending point(s) were found.
        nell,rerr	!  I have put in a trap to check for overstretching
        of ellipsoids, because I was not completely
        confident of the stretching equations.  If these
        values get set it is an indication that over-
        stretching has happened.  The user will be
        warned about this with an error statement.
        mz, mi, ms, mp	!  Actual computation length of each integration
        bt	! temperature was set in body file
        bm	! mass was set in the body file
        bw	! solvent=water command was found in the body file
        bc	! viscosity supplied in body file
        bbf	! buoyancy factor was set in the body file
        dictionary:	!  List of all the recognized words in the body file
        map:		!  Integer code corresponding to each entry in
        dictionary
        nwords:		!  Length of dictionary = total number of words
        that are valid entries in the body file
        -------------------------------------   Execution starts here
      */
      Tshush->silent = false;        /*  Use to suppress output to */
      /*
        terminal
      */
      puts("z2");
      Tcubit->ncube = 0;
      Tsell->nell = 0;
      // nerr = 0;
      launch_done = false;
      zeno_done = false;
      kirk_done = false;
      rg_done = false;
      span_done = false;
      shadow_done = false;
      /*
        Parse the invocation string:
      */
      setup(id,
            m1,
            actions,
            start,
            actout,
            &savehits_z,
            &savehits_i,
            &savehits_s,
            &savehits_p,
            USZ_MAX_ID_SIZE,
            4,
            28,
            30);
      puts("z2z");
      /*
        Parse the body file:
      */
      T1 = MAXELTS;
      T2 = MAXWORDS;
      puts("z3");
      parse(
            &T1,
            &nelts,
            eltype,
            (float *)bv,
            &tol,
            (float *)rotations,
            &tol_given,
            unitcode,
            &bt,
            &bm,
            &bw,
            &bc,
            &bbf,
            temp,
            tunit,
            mass,
            munit,
            visc,
            vunit,
            buoy,
            &hscale,
            &rlaunch,
            &launch_done,
            dictionary,
            map,
            &T2,
            &nwords,
            2,
            6,
            6,
            6,
            10
            );
      puts("z4");
      /*
        Begin output file:
      */
      begin_output(id,&nelts,start,USZ_MAX_ID_SIZE,28);
      /*
        Work out the radius of the launch sphere and the enveloping box
      */
      puts("z5");
      T1 = MAXELTS;
      do_launch(
                &T1,
                &nelts,
                eltype,
                (float *)bv,
                &rlaunch1,
                (float *)rotations,
                xyzlow,
                xyzhih);
      /*
        rlaunch = user-supplied launch radius
        rlaunch1 = launch radius computed from the structure
        At this point launch_done = .true. if ...
        the user supplied the launch radius in the body file.
        The user-supplied launch radius is used if it is equal to or
        greater than the computed radius.  An error exit
        occurs if the user-supplied radius is less than the
        computed radius.
        If no launch radius was supplied, we use the computed radius.
      */
      if(launch_done) {
         if(rlaunch1 > rlaunch) {
            WRITE(Tfilenumbers->nzno,FMT,F861,1,0);
            Stop("Invalid launch radius");
         }
      }
      else {
         rlaunch = rlaunch1;
         launch_done = true;
      }
      /*
        At this point, rlaunch contains the launch radius which will be
        used, and launch_done = .true. always.
      */
      puts("z6");
      if(launch_done) {
         if(!Tshush->silent) {
            WRITE(OUTPUT,FMT,F902,1,REAL4,rlaunch,0);
            WRITE(OUTPUT,FMT,F842,1,MORE);
            for(j=0; j<3; j++) {
               WRITE(REAL4,xyzlow[j],MORE);
            }
            WRITE(0);
            WRITE(OUTPUT,FMT,F843,1,MORE);
            for(j=0; j<3; j++) {
               WRITE(REAL4,xyzhih[j],MORE);
            }
            WRITE(0);
         }
      }
      puts("z7");
      for(i=0; i<4; i++) {
         m1do = m1[i];
         // ftnscopy(putout,USZ_MAX_ID_SIZE_PLUS_5,(actout+i*USZ_MAX_ID_SIZE_PLUS_5),USZ_MAX_ID_SIZE_PLUS_5,NULL);
         // assemble(id,putout,round,USZ_MAX_ID_SIZE,USZ_MAX_ID_SIZE_PLUS_5,80);
         if(*(actions+i) == 'i' || *(actions+i) == 'I') {
            /*
              Do the interior integrations:
            */
            T1 = MAXELTS;
            all_around(
                       &T1,
                       eltype,
                       (float *)bv,
                       &nelts,
                       xyzlow,
                       xyzhih,
                       &rlaunch,
                       (float *)rotations,
                       &m1do,
                       &rg2int,
                       &delta_rg2int,
                       &volume,
                       &delta_volume,
                       &rg_done,
                       &mi,
                       id,
                       q,
                       sq,
                       dsq,
                       round,
                       &savehits_i,
                       (double *)tten,
                       USZ_MAX_ID_SIZE,
                       80);
         }
         else if(*(actions+i) == 'c' || *(actions+i) == 'C') {
            /*
              Do the covered-interior integrations:
            */
            T1 = MAXELTS;
            wagaroo(&T1,
                    eltype,
                    (float *)bv,
                    &nelts,
                    &rlaunch,
                    (float *)rotations,
                    &m1do,
                    &rg2int,
                    &delta_rg2int,
                    &volume,
                    &delta_volume,
                    &rg_done,
                    &mi,
                    id,
                    q,
                    sq,
                    dsq,
                    round,
                    &savehits_i,
                    vaar,
                    (double *)tten,
                    USZ_MAX_ID_SIZE,
                    80);
         }
         else if(*(actions+i) == 'z' || *(actions+i) == 'Z') {
            /*
              Do the zeno integrations:
            */
            nneb = 3;              /*  This value of nneb optimizes the run time */
            /*
              !  for about 10 different random coil models
            */
            T1 = MAXELTS;
            blizzard(
                     &T1,
                     eltype,
                     (float *)bv,
                     &nelts,
                     &m1do,
                     &tol,
                     &rlaunch,
                     (float *)rotations,
                     &cap,
                     (float *)alpha_bongo,
                     &tol_given,
                     &zeno_done,
                     &delta_cap,
                     (float *)delta_bongo,
                     &mz,
                     id,
                     &tae,
                     uae,
                     (double *)vae,
                     (double *)wae,
                     round,
                     bubble,
                     &bubble_rad,
                     nebtab,
                     &nneb,
                     &ninn,
                     rlist,
                     strikes,
                     &savehits_z,
                     USZ_MAX_ID_SIZE,
                     80);
            pade((float *)alpha_bongo,&q2pade,eigens,xx);
         }
         else if(*(actions+i) == 's' || *(actions+i) == 'S') {
            /*
              Do the surface integrations:
            */
            T1 = MAXELTS;
            captain(&T1,
                    eltype,
                    (float *)bv,
                    &nelts,
                    &m1do,
                    (float *)rotations,
                    &kirk_done,
                    saar,
                    &kirk,
                    &delta_kirk,
                    &surface,
                    &delta_surface,
                    &rg2surf,
                    &delta_rg2surf,
                    &ms,
                    id,
                    round,
                    &savehits_s,
                    USZ_MAX_ID_SIZE,
                    80);
         }
         else if(*(actions+i) == 'p' || *(actions+i) == 'P') {
            /*
              Do the project-onto-line integration:
            */
            T1 = MAXELTS;
            calipers(&T1,
                     eltype,
                     (float *)bv,
                     &nelts,
                     &m1do,
                     (float *)rotations,
                     &span_done,
                     &span,
                     &delta_span,
                     &shadow_done,
                     &shadow,
                     &delta_shadow,
                     &rlaunch,
                     &mp,
                     id,
                     round,
                     &savehits_p,
                     USZ_MAX_ID_SIZE,
                     80);
         }
      }
      puts("z8");
      alldone(endend,28);
      report(
             id,
             actions,
             m1,
             &nelts,
             &tol,
             &rlaunch,
             &cap,
             &delta_cap,
             (float *)alpha_bongo,
             (float *)delta_bongo,
             (double *)tten,
             &volume,
             &delta_volume,
             &surface,
             &delta_surface,
             &rg2int,
             &delta_rg2int,
             &rg2surf,
             &delta_rg2surf,
             &kirk,
             &delta_kirk,
             &span,
             &delta_span,
             &shadow,
             &delta_shadow,
             &shadow_done,
             &zeno_done,
             &kirk_done,
             &rg_done,
             &span_done,
             &launch_done,
             &tol_given,
             unitcode,
             &mz,
             &mi,
             &ms,
             &mp,
             &bt,
             &bm,
             &bw,
             &bc,
             &bbf,
             temp,
             tunit,
             mass,
             munit,
             visc,
             vunit,
             buoy,
             &hscale,
             &tae,
             uae,
             (double *)vae,
             (double *)wae,
             &q2pade,
             eigens,
             xx,
             q,
             sq,
             dsq,
             xyzlow,
             xyzhih,
             USZ_MAX_ID_SIZE,
             4,
             2,
             6,
             6,
             6);
      WRITE(Tfilenumbers->nzno,FMT,F321,1,STRG,endend,28,0);
      WRITE(OUTPUT,FMT,F321,1,STRG,endend,28,0);
      CLOSE(Tfilenumbers->nzno,0);
      // Stop(NULL);
      return 0;
   }
#undef MAXELTS
#undef MAXWORDS

   void begin_output( 
                     char *id,
                     int *nelts,
                     char *start,
                     int /* P1 */,
                     int /* P2 */
                     )
   {
      /*
        Send initial information to report file
      */
      typedef struct {
         int nbod;
         int nzno;
         int nznr;
         int nstk;
         int ndfl;
         int nefl;
         int nzh;
         int nih;
         int nsh;
         int nph;
      } Cfilenumbers;
      static Cfilenumbers *Tfilenumbers = (Cfilenumbers*) Xfilenumbers;
      /*
*************************************************
*/
      static const char* F902[] = {
         "('START (Version 3.3): ',a28)"
      };
      static const char* F200[] = {
         "('Body name:  ',a2048)"
      };
      static const char* F201[] = {
         "('Number of body elements: ',i7)"
      };
      static const char* F905[] = {
         "(50('='))"
      };
      WRITE(Tfilenumbers->nzno,FMT,F902,1,STRG,start,28,0);
      WRITE(OUTPUT,FMT,F902,1,STRG,start,28,0);
      WRITE(Tfilenumbers->nzno,FMT,F200,1,STRG,id,USZ_MAX_ID_SIZE,0);
      WRITE(Tfilenumbers->nzno,FMT,F201,1,INT4,*nelts,0);
      WRITE(Tfilenumbers->nzno,FMT,F905,1,0);
      return;
   }

   void report(
               char * /* id */,
               char *actions,
               int m1[],
               int * /* nelts */,
               float *tol,
               float *rlaunch,
               float *cap,
               float *delta_cap,
               float *alpha_bongo,
               float *delta_bongo,
               double *tten,
               float *volume,
               float *delta_volume,
               float *surface,
               float *delta_surface,
               float *rg2int,
               float *delta_rg2int,
               float *rg2surf,
               float *delta_rg2surf,
               float *kirk,
               float *delta_kirk,
               float *span,
               float *delta_span,
               float *shadow,
               float *delta_shadow,
               unsigned int *shadow_done,
               unsigned int *zeno_done,
               unsigned int *kirk_done,
               unsigned int *rg_done,
               unsigned int *span_done,
               unsigned int *launch_done,
               unsigned int *tol_given,
               char *unitcode,
               int *mz,
               int *mi,
               int *ms,
               int *mp,
               unsigned int *bt,
               unsigned int *bm,
               unsigned int *bw,
               unsigned int *bc,
               unsigned int *bbf,
               double temp[],
               char *tunit,
               double mass[],
               char *munit,
               double visc[],
               char *vunit,
               double buoy[],
               float *hscale,
               double *tae,
               double uae[],
               double *vae,
               double *wae,
               float *q2pade,
               double eigens[],
               double xx[],
               float q[],
               float sq[],
               float dsq[],
               float xyzlow[],
               float xyzhih[],
               int /* P1 */,
               int /* P2 */,
               int /* P3 */,
               int /* P6 */,
               int /* P4 */,
               int /* P5 */)
   {
      /*
        Generate program output to report file
        Preliminary data was already written by begin_output
        This subroutine writes everything else
      */
      typedef struct {
         int nbod;
         int nzno;
         int nznr;
         int nstk;
         int ndfl;
         int nefl;
         int nzh;
         int nih;
         int nsh;
         int nph;
      } Cfilenumbers;
      static Cfilenumbers *Tfilenumbers = (Cfilenumbers*) Xfilenumbers;
      static float delta_paper[3][3],pi,alpha_paper[3][3];
      static char b1[1],b2[1];
      static unsigned int do_something,bs,bi,bl,bk,bz,bp,bpp;
      static char lunit[6],qunit[6];
      static double capd[2],rl,eps,box1[3],box2[3],a11[2],a12[2],a13[2],a21[2],a22[2],a23[2],a31[2],a32[2],a33[2],rk[2],
         surf[2],rg2surfd[2],v[2],rg2intd[2],lhalf[2],qa[2],qb[2],sqa[2],sqb[2],rp[2],rap[2],h1,h2,h3,stfac[2][82];
      static int i,nzip,j,i1,ka,kb;
      /*
*************************************************
*/
      static const char* F445[] = {
         "('JOB SUMMARY:')"
      };
      static const char* F203[] = {
         "('       Actions      Checked if    Monte Carlo')"
      };
      static const char* F204[] = {
         "('       requested    completed     size')"
      };
      static const char* F105[] = {
         "(50('-'))"
      };
      static const char* F205[] = {
         "(50('='))"
      };
      static const char* F206[] = {
         "(11x,a1,12x,a1,7x,i10)"
      };
      static const char* F340[] = {
         "('Unable to bracket L-half')"
      };
      /*
        The following boolean variables store information about the
        computation and control reporting of results.
        bt -- Temperature was set in body file
        bm -- Mass was set in body file
        bw -- Solvent=water command was found in body file
        bc -- Solvent viscosity was set in body file
        bbf -- Buoyancy factor was set in body file
        bs -- Surface integration succeeded
        bi -- Interior integration succeeded
        bl -- Launch radius was calculated
        bk -- Skin thickness was determined
        bz -- Zeno integration succeeded
        bp -- Projection-onto-line integration succeeded
        bpp -- Projection-onto-plane integration succeeded
      */
      pi = 3.14159265F;
      h1 = (double)*hscale;
      h2 = pow(h1,2.0);
      h3 = pow(h1,3.0);
      ftnscopy((lunit),2,unitcode,2,NULL);
      ftnscopy((lunit+2),4,"    ",4,NULL);/*  Length unit */
      ftnscopy(qunit,6,lunit,6,NULL);
      ftnscopy((qunit+2),4,"^-1 ",4,NULL);
      /*
        Report all errors detected on the output file
      */
      show_errors();
      /*
        Send a job summary to the output file
      */
      WRITE(Tfilenumbers->nzno,FMT,F445,1,0);
      WRITE(Tfilenumbers->nzno,FMT,F203,1,0);
      WRITE(Tfilenumbers->nzno,FMT,F204,1,0);
      WRITE(Tfilenumbers->nzno,FMT,F105,1,0);
      /*
        Actions      Checked if    Monte Carlo
        requested    completed	   size
        ==================================================
        z            *          1000000
        000000000111111111122222222223333333333444
        123456789012345678901234567890123456789012
      */
      for(i=0; i<4; i++) {
         do_something = false;
         if(*(actions+i) == 'z') {
            *b1 = 'z';
            *b2 = ' ';
            if(*zeno_done) *b2 = '*';
            nzip = m1[i];
            do_something = true;
         }
         else if(*(actions+i) == 'Z') {
            *b1 = 'Z';
            *b2 = ' ';
            if(*zeno_done) *b2 = '*';
            nzip = m1[i];
            do_something = true;
         }
         else if(*(actions+i) == 'i') {
            *b1 = 'i';
            *b2 = ' ';
            if(*rg_done) *b2 = '*';
            nzip = m1[i];
            do_something = true;
         }
         else if(*(actions+i) == 'I') {
            *b1 = 'I';
            *b2 = ' ';
            if(*rg_done) *b2 = '*';
            nzip = m1[i];
            do_something = true;
         }
         else if(*(actions+i) == 'c') {
            *b1 = 'c';
            *b2 = ' ';
            if(*rg_done) *b2 = '*';
            nzip = m1[i];
            do_something = true;
         }
         else if(*(actions+i) == 'C') {
            *b1 = 'C';
            *b2 = ' ';
            if(*rg_done) *b2 = '*';
            nzip = m1[i];
            do_something = true;
         }
         else if(*(actions+i) == 's') {
            *b1 = 's';
            *b2 = ' ';
            if(*kirk_done) *b2 = '*';
            nzip = m1[i];
            do_something = true;
         }
         else if(*(actions+i) == 'S') {
            *b1 = 'S';
            *b2 = ' ';
            if(*kirk_done) *b2 = '*';
            nzip = m1[i];
            do_something = true;
         }
         else if(*(actions+i) == 'p') {
            *b1 = 'p';
            *b2 = ' ';
            if(*span_done) *b2 = '*';
            nzip = m1[i];
            do_something = true;
         }
         else if(*(actions+i) == 'P') {
            *b1 = 'P';
            *b2 = ' ';
            if(*span_done) *b2 = '*';
            nzip = m1[i];
            do_something = true;
         }
         if(do_something) WRITE(Tfilenumbers->nzno,FMT,F206,1,STRG,b1,1,STRG,b2,1,INT4,nzip,0);
      }
      WRITE(Tfilenumbers->nzno,FMT,F205,1,0);
      bl = *launch_done;
      rl = (double)*rlaunch*h1;
      for(i=0; i<3; i++) {
         box1[i] = (double)xyzlow[i]*h1;
         box2[i] = (double)xyzhih[i]*h1;
      }
      bk = *tol_given;
      eps = (double)*tol*h1;
      bz = *zeno_done;
      if(*zeno_done) {
         capd[0] = (double)*cap*h1;
         capd[1] = (double)*delta_cap*h1;
         for(i=0; i<3; i++) {
            for(j=0; j<3; j++) {
               alpha_paper[j][i] = *(alpha_bongo+i+j*3)*4.0F*pi;
               delta_paper[j][i] = *(delta_bongo+i+j*3)*4.0F*pi;
            }
         }
         a11[0] = (double)alpha_paper[0][0]*h3;
         a12[0] = (double)alpha_paper[1][0]*h3;
         a13[0] = (double)alpha_paper[2][0]*h3;
         a21[0] = (double)alpha_paper[0][1]*h3;
         a22[0] = (double)alpha_paper[1][1]*h3;
         a23[0] = (double)alpha_paper[2][1]*h3;
         a31[0] = (double)alpha_paper[0][2]*h3;
         a32[0] = (double)alpha_paper[1][2]*h3;
         a33[0] = (double)alpha_paper[2][2]*h3;
         a11[1] = (double)delta_paper[0][0]*h3;
         a12[1] = (double)delta_paper[1][0]*h3;
         a13[1] = (double)delta_paper[2][0]*h3;
         a21[1] = (double)delta_paper[0][1]*h3;
         a22[1] = (double)delta_paper[1][1]*h3;
         a23[1] = (double)delta_paper[2][1]*h3;
         a31[1] = (double)delta_paper[0][2]*h3;
         a32[1] = (double)delta_paper[1][2]*h3;
         a33[1] = (double)delta_paper[2][2]*h3;
         for(i=0; i<3; i++) {
            for(j=0; j<3; j++) {
               *(vae+i+j*3) = h1**(vae+i+j*3);
               *(wae+i+j*3) = h1**(wae+i+j*3);
            }
         }
         for(i=0; i<3; i++) {
            eigens[i] = eigens[i]*h3;
         }
      }
      bs = *kirk_done;
      if(*kirk_done) {
         rk[0] = (double)*kirk*h1;
         rk[1] = (double)*delta_kirk*h1;
         surf[0] = (double)*surface*h2;
         surf[1] = (double)*delta_surface*h2;
         rg2surfd[0] = (double)*rg2surf*h2;
         rg2surfd[1] = (double)*delta_rg2surf*h2;
      }
      bi = *rg_done;
      if(*rg_done) {
         v[0] = (double)*volume*h3;
         v[1] = (double)*delta_volume*h3;
         rg2intd[0] = (double)*rg2int*h2;
         rg2intd[1] = (double)*delta_rg2int*h2;
         for(i=0; i<3; i++) {
            for(i1=0; i1<3; i1++) {
               *(tten+i+i1*3) = *(tten+i+i1*3)*h2;
            }
         }
         for(i=0; i<=81; i++) {
            q[i] = q[i]/(float)h1;
            stfac[0][i] = (double)sq[i];
            stfac[1][i] = (double)dsq[i];
         }
         /*
           Find first point at which s(q) drops below 0.5
         */
         for(i=1; i<=81; i++) {
            if(stfac[0][i] < 0.5e0) {
               ka = i-1;
               kb = i;
               goto S25;
            }
         }
         WRITE(Tfilenumbers->nzno,FMT,F340,1,0);
         Stop("Unable to bracket L-half");
      S25:
         qa[0] = (double)q[ka];
         qa[1] = 0.0e0;
         qb[0] = (double)q[kb];
         qb[1] = 0.0e0;
         sqa[0] = stfac[0][ka];
         sqa[1] = stfac[1][ka];
         sqb[0] = stfac[0][kb];
         sqb[1] = stfac[1][kb];
         lighthalf(qa,qb,sqa,sqb,lhalf);
      }
      bp = *span_done;
      if(bp) {
         rp[0] = (double)(*span/2.0F)*h1;
         rp[1] = (double)(*delta_span/2.0F)*h1;
      }
      bpp = *shadow_done;
      if(bpp) {
         rap[0] = (double)*shadow*h2;
         rap[1] = (double)*delta_shadow*h2;
      }
      /*
        Record results of all calculations on output file
      */
      poplar(
             mz,
             ms,
             mi,
             mp,
             &bl,
             &bk,
             &bs,
             &bi,
             bt,
             bm,
             bw,
             bc,
             bbf,
             &bz,
             &bp,
             &bpp,
             &rl,
             &eps,
             capd,
             a11,
             a12,
             a13,
             a21,
             a22,
             a23,
             a31,
             a32,
             a33,
             tten,
             rk,
             surf,
             rg2surfd,
             v,
             rg2intd,
             rp,
             rap,
             temp,
             mass,
             visc,
             buoy,
             lunit,
             tunit,
             munit,
             vunit,
             qunit,
             tae,
             uae,
             vae,
             wae,
             q2pade,
             eigens,
             xx,
             q,
             (double *)stfac,
             lhalf,
             box1,
             box2,
             6,
             6,
             6,
             6,
             6);
      return;
   }
   void lighthalf(
                  double qa[],
                  double qb[],
                  double sqa[],
                  double sqb[],
                  double lhalf[] 
                  )
   {
      static double mhalf[2],mqa[2],msqb[2],t1[2],t2[2],t3[2],t4[2],t5[2],t6[2];
      /*
*************************************************
*/
      /*
        Linear interpolation with error propagation to estimate L-half
      */
      mhalf[0] = -0.5e0;
      mhalf[1] = 0.0e0;
      mqa[0] = -qa[0];
      mqa[1] = qa[1];
      msqb[0] = -sqb[0];
      msqb[1] = sqb[1];
      summer(qb,mqa,t1);
      summer(sqa,mhalf,t2);
      summer(sqa,msqb,t3);
      multiply(t1,t2,t4);
      divide(t4,t3,t5);
      summer(qa,t5,t6);
      t1[0] = 1.0e0;
      t1[1] = 0.0e0;
      divide(t1,t6,lhalf);
      return;
   }

   void poplar(
               int * /* mz */,
               int * /* ms */,
               int * /* mi */,
               int * /* mp */,
               unsigned int *bl,
               unsigned int *bk,
               unsigned int *bs,
               unsigned int *bi,
               unsigned int *bt,
               unsigned int *bm,
               unsigned int *bw,
               unsigned int *bc,
               unsigned int *bbf,
               unsigned int *bz,
               unsigned int *bp,
               unsigned int *bpp,
               double *rl,
               double *eps,
               double cap[],
               double a11[],
               double a12[],
               double a13[],
               double a21[],
               double a22[],
               double a23[],
               double a31[],
               double a32[],
               double a33[],
               double *tten,
               double rk[],
               double surf[],
               double rg2surf[],
               double v[],
               double rg2int[],
               double rp[],
               double rap[],
               double temp[],
               double mass[],
               double eta[],
               double buoy[],
               char *lunit,
               char *tunit,
               char *munit,
               char *vunit,
               char *qunit,
               double *tae,
               double uae[],
               double *vae,
               double *wae,
               float *q2pade,
               double eigens[],
               double xx[],
               float q[],
               double *stfac,
               double lhalf[],
               double box1[],
               double box2[],
               int /* P1 */,
               int /* P2 */,
               int /* P3 */,
               int /* P4 */,
               int /* P5 */)
   {
      /*
        Record results of all calculations on output file.
      */
      typedef struct {
         int nbod;
         int nzno;
         int nznr;
         int nstk;
         int ndfl;
         int nefl;
         int nzh;
         int nih;
         int nsh;
         int nph;
      } Cfilenumbers;
      static Cfilenumbers *Tfilenumbers = (Cfilenumbers*) Xfilenumbers;
      typedef struct {
         char fbod[USZ_MAX_ID_SIZE];
         char fzno[USZ_MAX_ID_SIZE];
         char fznr[USZ_MAX_ID_SIZE];
         char fstk[USZ_MAX_ID_SIZE];
         char fdfl[USZ_MAX_ID_SIZE];
         char fefl[USZ_MAX_ID_SIZE];
         char fzh[USZ_MAX_ID_SIZE];
         char fih[USZ_MAX_ID_SIZE];
         char fsh[USZ_MAX_ID_SIZE];
         char fph[USZ_MAX_ID_SIZE];
      } Cfilenames;
      static Cfilenames *Tfilenames = (Cfilenames*) Xfilenames;
      static unsigned int bu,bv;
      static char dunit[6],funit[6],aunit[6],volunit[6],nounit[6],intunit[6],svedunit[6],name[30];
      static double eigent[3];
      static double aa_pol_num;
      static double aa_pol_den;
      static double aa_pol;
      static double aa_rg2_num;
      static double aa_rg2_den;
      static double aa_rg2;
      static double ax2;
      static double ax1;
      static double rv[2];
      static double rat[2];
      static double fcap[2];
      static double fconv[2];
      static double fpercm[2];
      static double exponent;
      static double rgsurf[2];
      static double rgint[2];
      static double t2[2];
      static double c0[2];
      static double d[2];
      static double fric[2];
      static double q1[2];
      static double pi[2];
      static double six[2];
      static double three[2];
      static double q2low[2];
      static double q2hih[2];
      static double q2best[2];
      static double sed[2];
      static double two[2];
      static double five[2];
      static double ol[2];
      static double rh[2];
      static double t1[2];
      static double trace[2];
      static double four[2];
      static double vh[2];
      static double malf[2];
      static double sig11[2];
      static double sig22[2];
      static double sig33[2];
      static double signorm[2];
      static double sigma[2];
      static double etav[2];
      static double ssig[2];
      static double etam[2];
      static double boltz[2];
      static double pi12[2];
      static double celsius[2];
      static double sqsurf[2];/*  Square root of surface are */
      static double cruss[2];            /*  Russell and Rayleigh aprox. to C */
      static double cray[2],pre1[2],pre2[2],rod[2][12];
      static char nod[60];
      static unsigned int got[12];
      static char dig2[2];
      static int i1,i2;
      static float rg2ot;
      static int i;
      /*
*************************************************
*/
      static const char* F345[] = {
         "('RESULTS OF CALCULATION:')"
      };
      static const char* F922[] = {
         "(i1,i1)"
      };
      static const char* F205[] = {
         "(50('='))"
      };
      static const char* F301[] = {
         "('STRUCTURE FACTOR, q, S(q): ')"
      };
      static const char* F900[] = {
         "('DIMENSIONLESS RATIOS:')"
      };
      /*
        List of logical variables that control reporting of results
        bt -- temperature was set
        bm -- mass was set
        bv -- solvent viscosity was set
        bbf -- buoyancy factor was set
        bl -- launch radius was determined
        bk -- skin thickness was determined
        bz -- zeno integration succeeded
        bs -- surface integration succeeded
        bi -- interior integration succeeded
        bp -- projection-onto-line integration succeeded
        bpp -- projection-onto-plane integration succeeded
        bu -- specific, rather than generic, length units are in use
      */
      bu = !(fifmemc(lunit,"L     ",6) == 0);
      loadconstants(q1,q2low,q2hih,q2pade,q2best,pi,two,three,four,five,six,boltz,celsius,pre1,pre2,pi12,fpercm);
      derive(lunit,aunit,volunit,nounit,6,6,6,6);
      parameters(bt,temp,bc,eta,tunit,vunit,celsius,6,6);
      /*
        must convert temp to kelvin, eta to centipoise
      */
      OPEN(Tfilenumbers->nznr,FILEN,Tfilenames->fznr,USZ_MAX_ID_SIZE,STATUS,"unknown",0);
      WRITE(Tfilenumbers->nzno,FMT,F345,1,0);
      WRITE(Tfilenumbers->nznr,FMT,F345,1,0);
      if(*bl) {
         ftnscopy(name,30,"launch radius@",14,NULL);
         writeno(rl,name,lunit,30,6);
         ftnscopy(name,30,"XYZ_low(x)@",11,NULL);
         writeno(&box1[0],name,lunit,30,6);
         ftnscopy(name,30,"XYZ_low(y)@",11,NULL);
         writeno(&box1[1],name,lunit,30,6);
         ftnscopy(name,30,"XYZ_low(z)@",11,NULL);
         writeno(&box1[2],name,lunit,30,6);
         ftnscopy(name,30,"XYZ_hih(x)@",11,NULL);
         writeno(&box2[0],name,lunit,30,6);
         ftnscopy(name,30,"XYZ_hih(y)@",11,NULL);
         writeno(&box2[1],name,lunit,30,6);
         ftnscopy(name,30,"XYZ_hih(z)@",11,NULL);
         writeno(&box2[2],name,lunit,30,6);
      }
      if(*bk) {
         ftnscopy(name,30,"skin thickness@",15,NULL);
         writeno(eps,name,lunit,30,6);
      }
      if(*bt) {
         ftnscopy(name,30,"temperature@",12,NULL);
         writeyes(temp,name,tunit,30,6);
      }
      if(*bm) {
         ftnscopy(name,30,"mass@",5,NULL);
         writeyes(mass,name,munit,30,6);
      }
      bv = false;
      if(*bc) {
         ftnscopy(name,30,"solvent viscosity (supplied)@",29,NULL);
         writeyes(eta,name,vunit,30,6);
         bv = true;
      }
      if(!*bc && *bt && *bw) {
         compvisc(temp,eta,vunit,6);
         bv = true;
         ftnscopy(name,30,"solvent viscosity (computed)@",29,NULL);
         writeyes(eta,name,vunit,30,6);
      }
      if(*bbf) {
         ftnscopy(name,30,"buoyancy factor@",16,NULL);
         writeyes(buoy,name,nounit,30,6);
      }
      if(*bz) {
         ftnscopy(name,30,"capacitance, C@",15,NULL);
         writeyes(cap,name,lunit,30,6);
         if(bu) {
            ftnscopy(funit,6,"farad ",6,NULL);
            if(fifmemc(lunit,"m     ",6) == 0) {
               fconv[0] = fpercm[0]*1.0e2;
               fconv[1] = fpercm[1]*1.0e2;
            }
            else if(fifmemc(lunit,"cm    ",6) == 0) {
               fconv[0] = fpercm[0];
               fconv[1] = fpercm[1];
            }
            else if(fifmemc(lunit,"nm    ",6) == 0) {
               fconv[0] = fpercm[0]*1.0e-7;
               fconv[1] = fpercm[1]*1.0e-7;
            }
            else if(fifmemc(lunit,"A     ",6) == 0) {
               fconv[0] = fpercm[0]*1.0e-8;
               fconv[1] = fpercm[1]*1.0e-8;
            }
            multiply(cap,fconv,fcap);
            ftnscopy(name,30,"practical capacitance, C@",25,NULL);
            writeyes(fcap,name,funit,30,6);
         }
         ftnscopy(name,30,"polarizability 11@",18,NULL);
         writeyes(a11,name,volunit,30,6);
         ftnscopy(name,30,"polarizability 12@",18,NULL);
         writeyes(a12,name,volunit,30,6);
         ftnscopy(name,30,"polarizability 13@",18,NULL);
         writeyes(a13,name,volunit,30,6);
         ftnscopy(name,30,"polarizability 21@",18,NULL);
         writeyes(a21,name,volunit,30,6);
         ftnscopy(name,30,"polarizability 22@",18,NULL);
         writeyes(a22,name,volunit,30,6);
         ftnscopy(name,30,"polarizability 23@",18,NULL);
         writeyes(a23,name,volunit,30,6);
         ftnscopy(name,30,"polarizability 31@",18,NULL);
         writeyes(a31,name,volunit,30,6);
         ftnscopy(name,30,"polarizability 32@",18,NULL);
         writeyes(a32,name,volunit,30,6);
         ftnscopy(name,30,"polarizability 33@",18,NULL);
         writeyes(a33,name,volunit,30,6);
         ftnscopy(name,30,"pol.eigenvalue 1@",17,NULL);
         eigens[0] = eigens[0]*4.0e0*pi[0];
         eigens[1] = eigens[1]*4.0e0*pi[0];
         eigens[2] = eigens[2]*4.0e0*pi[0];
         writeno(&eigens[0],name,volunit,30,6);
         ftnscopy(name,30,"pol.eigenvalue 2@",17,NULL);
         writeno(&eigens[1],name,volunit,30,6);
         ftnscopy(name,30,"pol.eigenvalue 3@",17,NULL);
         writeno(&eigens[2],name,volunit,30,6);
         ftnscopy(name,30,"pol.shape ratio 1@",18,NULL);
         writeno(&xx[0],name,nounit,30,6);
         ftnscopy(name,30,"pol.shape ratio 2@",18,NULL);
         writeno(&xx[1],name,nounit,30,6);
         rudnick_gaspari(eigens,&aa_pol_num,&aa_pol_den,&aa_pol);
         ftnscopy(name,30,"A(RG).pol.num@",14,NULL);
         writeno(&aa_pol_num,name,aunit,30,6);
         ftnscopy(name,30,"A(RG).pol.den@",14,NULL);
         writeno(&aa_pol_den,name,aunit,30,6);
         ftnscopy(name,30,"A(RG).pol@",10,NULL);
         writeno(&aa_pol,name,nounit,30,6);
         multiply(q1,cap,rh);
         ftnscopy(name,30,"Rh@",3,NULL);
         writeyes(rh,name,lunit,30,6);
         summer(a11,a22,t1);
         summer(t1,a33,trace);
         ftnscopy(name,30,"Tr(alpha)@",10,NULL);
         writeyes(trace,name,volunit,30,6);
         divide(trace,three,malf);
         ftnscopy(name,30,"<alpha>@",8,NULL);
         writeyes(malf,name,volunit,30,6);
         ftnscopy(name,30,"q2(lower)@",10,NULL);
         writeno(&q2low[0],name,nounit,30,6);
         ftnscopy(name,30,"q2(pade)@",9,NULL);
         writeyes(q2best,name,nounit,30,6);
         ftnscopy(name,30,"q2(pade-no-round)@",18,NULL);
         writeno(&q2best[0],name,nounit,30,6);
         ftnscopy(name,30,"q2(upper)@",10,NULL);
         writeno(&q2hih[0],name,nounit,30,6);
         multiply(q2best,trace,t1);
         multiply(t1,two,t2);
         divide(t2,three,t1);
         divide(t1,five,vh);
         ftnscopy(name,30,"Vh@",3,NULL);
         writeyes(vh,name,volunit,30,6);
         multiply(three,vh,t1);
         divide(t1,four,t2);
         divide(t2,pi,t1);
         exponent = 1.0e0/3.0e0;
         power(t1,rv,&exponent);
         ftnscopy(name,30,"Rv (viscosity radius)@",22,NULL);
         writeyes(rv,name,lunit,30,6);
         writehandy(tae,uae,vae,wae,nounit,volunit,6,6);
      }
      if(*bp) {
         ftnscopy(name,30,"LG@",3,NULL);
         writeyes(rp,name,lunit,30,6);
      }
      if(*bpp) {
         ftnscopy(name,30,"Ap@",3,NULL);
         writeyes(rap,name,aunit,30,6);
         divide(rap,pi,t1);
         exponent = 1.0e0/2.0e0;
         power(t1,ol,&exponent);
         ftnscopy(name,30,"LO@",3,NULL);
         writeyes(ol,name,lunit,30,6);
      }
      if(*bs) {
         ftnscopy(name,30,"RK@",3,NULL);
         writeyes(rk,name,lunit,30,6);
         ftnscopy(name,30,"surface area@",13,NULL);
         writeyes(surf,name,aunit,30,6);
         exponent = 1.0e0/2.0e0;
         power(rg2surf,rgsurf,&exponent);
         ftnscopy(name,30,"Rg (surface)@",13,NULL);
         writeyes(rgsurf,name,lunit,30,6);
         exponent = 1.0e0/2.0e0;
         power(surf,sqsurf,&exponent);
         multiply(sqsurf,pre1,cruss);
         multiply(sqsurf,pre2,cray);
         ftnscopy(name,30,"R(Russell)@",11,NULL);
         writeyes(cruss,name,lunit,30,6);
         ftnscopy(name,30,"R(Rayleigh)@",12,NULL);
         writeyes(cray,name,lunit,30,6);
      }
      if(*bi) {
         ftnscopy(name,30,"volume@",7,NULL);
         writeyes(v,name,volunit,30,6);
         exponent = 1.0e0/2.0e0;
         power(rg2int,rgint,&exponent);
         ftnscopy(name,30,"Rg (interior)@",14,NULL);
         writeyes(rgint,name,lunit,30,6);
         ftnscopy(name,30,"L-half@",7,NULL);
         writeyes(lhalf,name,lunit,30,6);
         ttdiag(tten,eigent);
         for(i1=1; i1<=3; i1++) {
            for(i2=1; i2<=3; i2++) {
               WRITE(INTERNAL,dig2,2,FMT,F922,1,INT4,i1,INT4,i2,0);
               ftnscopy(name,30,"Rg2 tensor xx@",14,NULL);
               /*
                 12345678901234
               */
               ftnscopy((name+11),2,dig2,2,NULL);
               writeno((tten+i1-1+(i2-1)*3),name,aunit,30,6);
            }
         }
         ftnscopy(name,30,"Rg2.eigenvalue 1@",17,NULL);
         writeno(&eigent[0],name,aunit,30,6);
         ftnscopy(name,30,"Rg2.eigenvalue 2@",17,NULL);
         writeno(&eigent[1],name,aunit,30,6);
         ftnscopy(name,30,"Rg2.eigenvalue 3@",17,NULL);
         writeno(&eigent[2],name,aunit,30,6);
         ax1 = eigent[1]/eigent[0];
         ax2 = eigent[2]/eigent[1];
         ftnscopy(name,30,"Rg2.shape ratio 1@",18,NULL);
         writeno(&ax1,name,nounit,30,6);
         ftnscopy(name,30,"Rg2.shape ratio 2@",18,NULL);
         writeno(&ax2,name,nounit,30,6);
         rudnick_gaspari(eigent,&aa_rg2_num,&aa_rg2_den,&aa_rg2);
         ftnscopy(name,30,"A(RG).rg2.num@",14,NULL);
         writeno(&aa_rg2_num,name,aunit,30,6);
         ftnscopy(name,30,"A(RG).rg2.den@",14,NULL);
         writeno(&aa_rg2_den,name,aunit,30,6);
         ftnscopy(name,30,"A(RG).rg2@",10,NULL);
         writeno(&aa_rg2,name,nounit,30,6);
         rg2ot = (float)(eigent[0]+eigent[1]+eigent[2]);
         rg2ot = (float)(rg2int[0]/(double)rg2ot);
         /*
           print *,'Rg check should be one:  ',rg2ot
         */
      }
      if(*bi) {
         multiply(three,v,t1);
         divide(t1,four,t2);
         divide(t2,pi,t1);
         exponent = (double)(1.0F/3.0F);
         power(t1,c0,&exponent);
         ftnscopy(name,30,"C0@",3,NULL);
         writeyes(c0,name,lunit,30,6);
      }
      if(*bz && *bi) {
         divide(a11,three,t1);
         divide(t1,v,sig11);
         divide(a22,three,t1);
         divide(t1,v,sig22);
         divide(a33,three,t1);
         divide(t1,v,sig33);
         summer(sig11,sig22,signorm);
         ftnscopy(name,30,"[sigma](xy)@",12,NULL);
         writeyes(signorm,name,nounit,30,6);
         ftnscopy(name,30,"[sigma](z)@",11,NULL);
         writeyes(sig33,name,nounit,30,6);
         summer(signorm,sig33,sigma);
         ftnscopy(name,30,"[sigma]@",8,NULL);
         writeyes(sigma,name,nounit,30,6);
         multiply(q2low,sigma,etav);
         ftnscopy(name,30,"[eta](V)(lower)@",16,NULL);
         writeyes(etav,name,nounit,30,6);
         multiply(q2best,sigma,etav);
         ftnscopy(name,30,"[eta](V)(pade)@",15,NULL);
         writeyes(etav,name,nounit,30,6);
         multiply(q2hih,sigma,etav);
         ftnscopy(name,30,"[eta](V)(upper)@",16,NULL);
         writeyes(etav,name,nounit,30,6);
         multiply(q2best,sigma,etav);/*  recalculate with q2best */
         /*
           so best result gets
           carried forward
         */
      }
      if(*bi && *bs) {
         multiply(six,six,t1);
         multiply(t1,pi,t2);
         exponent = 1.0e0/3.0e0;
         power(t2,t1,&exponent);
         divide(surf,t1,t2);
         exponent = 2.0e0/3.0e0;
         power(v,t1,&exponent);
         divide(t2,t1,ssig);
         ftnscopy(name,30,"sphericity@",11,NULL);
         writeyes(ssig,name,nounit,30,6);
      }
      if(*bz && *bm) {
         divide(vh,mass,t1);
         multiply(t1,five,t2);
         divide(t2,two,t1);
         conventional(t1,etam,lunit,munit,intunit,6,6,6);
         ftnscopy(name,30,"[eta](M)@",9,NULL);
         writeyes(etam,name,intunit,30,6);
      }
      if(*bz && *bt && bv && bu) {
         multiply(boltz,temp,t1);
         divide(t1,six,t2);
         divide(t2,pi,t1);
         divide(t1,eta,t2);
         divide(t2,rh,t1);
         diffuse(t1,d,lunit,dunit,6,6);
         ftnscopy(name,30,"D@",2,NULL);
         writeyes(d,name,dunit,30,6);
      }
      if(*bm && *bbf && *bz && bu && bv) {
         multiply(mass,buoy,t1);
         divide(t1,six,t2);
         divide(t2,pi,t1);
         divide(t1,eta,t2);
         divide(t2,rh,t1);
         svedberg(t1,sed,munit,lunit,svedunit,6,6,6);
         ftnscopy(name,30,"s@",2,NULL);
         writeyes(sed,name,svedunit,30,6);
      }
      if(bu && bv && *bz) {
         multiply(six,pi,t1);
         multiply(t1,eta,t2);
         multiply(t2,rh,t1);
         friction(t1,fric,lunit,funit,6,6);
         ftnscopy(name,30,"f@",2,NULL);
         writeyes(fric,name,funit,30,6);
      }
      if(*bi) {
         WRITE(Tfilenumbers->nzno,FMT,F205,1,0);
         WRITE(Tfilenumbers->nzno,FMT,F301,1,0);
         WRITE(Tfilenumbers->nznr,FMT,F301,1,0);
         for(i=1; i<=81; i++) {
            writeme(qunit,nounit,&q[i],(stfac+i),(stfac+i+82),6,6);
         }
      }
      WRITE(Tfilenumbers->nzno,FMT,F205,1,0);
      WRITE(Tfilenumbers->nzno,FMT,F900,1,0);
      WRITE(Tfilenumbers->nznr,FMT,F900,1,0);
      for(i=1; i<=2; i++) {
         rod[i-1][0] = cap[i-1];
         rod[i-1][1] = rh[i-1];
         rod[i-1][2] = rv[i-1];
         rod[i-1][3] = rgint[i-1];
         rod[i-1][4] = rgsurf[i-1];
         rod[i-1][5] = lhalf[i-1];
         rod[i-1][6] = c0[i-1];
         rod[i-1][7] = rp[i-1];
         rod[i-1][8] = ol[i-1];
         rod[i-1][9] = rk[i-1];
         rod[i-1][10] = cruss[i-1];
         rod[i-1][11] = cray[i-1];
      }
      ftnscopy(nod,5,"C@",2,NULL);
      ftnscopy((nod+5),5,"Rh@",3,NULL);
      ftnscopy((nod+10),5,"Rv@",3,NULL);
      ftnscopy((nod+15),5,"Rg,i@",5,NULL);
      ftnscopy((nod+20),5,"Rg,s@",5,NULL);
      ftnscopy((nod+25),5,"Lhlf@",5,NULL);
      ftnscopy((nod+30),5,"C0@",3,NULL);
      ftnscopy((nod+35),5,"LG@",3,NULL);
      ftnscopy((nod+40),5,"LO@",3,NULL);
      ftnscopy((nod+45),5,"RK@",3,NULL);
      ftnscopy((nod+50),5,"RRus@",5,NULL);
      ftnscopy((nod+55),5,"RRay@",5,NULL);
      got[0] = *bz;
      got[1] = *bz;
      got[2] = *bz;
      got[3] = *bi;
      got[4] = *bs;
      got[5] = *bi;
      got[6] = *bi;
      got[7] = *bp;
      got[8] = *bpp;
      got[9] = *bs;
      got[10] = *bs;
      got[11] = *bs;
      for(i1=1; i1<=12; i1++) {
         for(i2=1; i2<=12; i2++) {
            if(i2 != i1) {
               if(got[i1-1] && got[i2-1]) {
                  bundle((nod+(i1-1)*5),(nod+(i2-1)*5),name,5,5,30);
                  t1[0] = rod[0][i1-1];
                  t1[1] = rod[1][i1-1];
                  t2[0] = rod[0][i2-1];
                  t2[1] = rod[1][i2-1];
                  divide(t1,t2,rat);
                  writeyes(rat,name,nounit,30,6);
               }
            }
         }
      }
      WRITE(Tfilenumbers->nzno,FMT,F205,1,0);
      CLOSE(Tfilenumbers->nznr,0);
      return;
   }

   void rudnick_gaspari(
                        double ev[],
                        double *aanum,
                        double *aaden,
                        double *aa
                        )
   {
      static double ss,mm;
      /*
********************************************************
*/
      ss = ev[0]+ev[1]+ev[2];
      mm = ev[0]*ev[1]+ev[0]*ev[2]+ev[1]*ev[2];
      *aanum = ss*ss-3.0e0*mm;
      *aaden = ss*ss;
      *aa = *aanum/ *aaden;
      return;
   }

   void writehandy(
                   double *tae,
                   double uae[],
                   double *vae,
                   double *wae,
                   char *nounit,
                   char *vunit,
                   int /* P1 */,
                   int /* P2 */)
   {
      typedef struct {
         int nbod;
         int nzno;
         int nznr;
         int nstk;
         int ndfl;
         int nefl;
         int nzh;
         int nih;
         int nsh;
         int nph;
      } Cfilenumbers;
      static Cfilenumbers *Tfilenumbers = (Cfilenumbers*) Xfilenumbers;
      static int i,j;
      /*
********************************************************
*/
      static const char* F900[] = {
         "('     T!   . . . . ',g20.10,1x,a6)"
      };
      static const char* F901[] = {
         "('     U!',i1,'  . . . . ',g20.10,1x,a6)"
      };
      static const char* F902[] = {
         "('     V!',2i1,' . . . . ',g20.10,1x,a6)"
      };
      static const char* F903[] = {
         "('     W!',2i1,' . . . . ',g20.10,1x,a6)"
      };
      WRITE(Tfilenumbers->nzno,FMT,F900,1,REAL8,*tae,STRG,nounit,6,0);
      WRITE(Tfilenumbers->nznr,FMT,F900,1,REAL8,*tae,STRG,nounit,6,0);
      for(i=1; i<=3; i++) {
         WRITE(Tfilenumbers->nzno,FMT,F901,1,INT4,i,REAL8,uae[i-1],STRG,nounit,6,0);
         WRITE(Tfilenumbers->nznr,FMT,F901,1,INT4,i,REAL8,uae[i-1],STRG,nounit,6,0);
      }
      for(i=1; i<=3; i++) {
         for(j=1; j<=3; j++) {
            WRITE(Tfilenumbers->nzno,FMT,F902,1,INT4,i,INT4,j,REAL8,*(vae+i-1+(j-1)*3),STRG,vunit,6,0);
            WRITE(Tfilenumbers->nznr,FMT,F902,1,INT4,i,INT4,j,REAL8,*(vae+i-1+(j-1)*3),STRG,vunit,6,0);
         }
      }
      for(i=1; i<=3; i++) {
         for(j=1; j<=3; j++) {
            WRITE(Tfilenumbers->nzno,FMT,F903,1,INT4,i,INT4,j,REAL8,*(wae+i-1+(j-1)*3),STRG,vunit,6,0);
            WRITE(Tfilenumbers->nznr,FMT,F903,1,INT4,i,INT4,j,REAL8,*(wae+i-1+(j-1)*3),STRG,vunit,6,0);
         }
      }
      return;
   }
   void bundle(
               char *nod1,
               char *nod2,
               char *name,
               int /* P1 */,
               int /* P2 */,
               int /* P3 */)
   {
      static int lk,kk,i;
      /*
********************************************************
*/
      lk = 1;
      kk = 1;
      while(*(nod1+kk-1) != '@') {
         *(name+lk-1) = *(nod1+kk-1);
         kk = kk+1;
         lk = lk+1;
      }
      *(name+lk-1) = '/';
      for(i=0; i<5; i++) {
         lk = lk+1;
         *(name+lk-1) = *(nod2+i);
      }
      return;
   }

   void summer(double a[],
               double b[],
               double c[])
   {
      /*
********************************************************
*/
      c[1] = pow(a[1],2.0)+pow(b[1],2.0);
      c[1] = sqrt(c[1]);
      c[0] = a[0]+b[0];
      return;
   }

   void multiply(double a[],
                 double b[],
                 double c[])
   {
      /*
********************************************************
*/
      c[1] = pow(b[0],2.0)*pow(a[1],2.0)+pow(a[0],2.0)*pow(b[1],2.0);
      c[1] = sqrt(c[1]);
      c[0] = a[0]*b[0];
      return;
   }

   void divide(double x1[],
               double x2[],
               double y[])
   {
      static double d1,d2;
      /*
********************************************************
*/
      d1 = 1.0e0/x2[0];
      d2 = -(x1[0]/pow(x2[0],2.0));
      y[1] = pow(d1,2.0)*pow(x1[1],2.0)+pow(d2,2.0)*pow(x2[1],2.0);
      y[1] = sqrt(y[1]);
      y[0] = x1[0]/x2[0];
      return;
   }

   void power(
              double x[],
              double y[],
              double *exponent)
   {
      static double d;
      /*
********************************************************
*/
      y[0] = pow(x[0],*exponent);
      d = *exponent*pow(x[0],*exponent-1.0e0);
      y[1] = pow(d,2.0)*pow(x[1],2.0);
      y[1] = sqrt(y[1]);
      return;
   }

   void derive(
               char *lunit,
               char *aunit,
               char *volunit,
               char *nounit,
               int /* P1 */,
               int /* P2 */,
               int /* P3 */,
               int /* P4 */)
   {
      /*
********************************************************
*/
      ftnscopy(nounit,6,"      ",6,NULL);
      ftnscopy(aunit,6,lunit,6,NULL);
      ftnscopy(volunit,6,lunit,6,NULL);
      if(*(aunit+1) == ' ') {
         ftnscopy((aunit+1),2,"^2",2,NULL);
         ftnscopy((volunit+1),2,"^3",2,NULL);
      }
      else {
         ftnscopy((aunit+2),2,"^2",2,NULL);
         ftnscopy((volunit+2),2,"^3",2,NULL);
      }
      return;
   }

   void loadconstants(
                      double q1[],
                      double q2low[],
                      double q2hih[],
                      float *q2pade,
                      double q2best[],
                      double pi[],
                      double two[],
                      double three[],
                      double four[],
                      double five[],
                      double six[],
                      double boltz[],
                      double celsius[],
                      double pre1[],
                      double pre2[],
                      double pi12[],
                      double fpercm[]
                      )
   {
      /*
********************************************************
*/
      q1[0] = 1.0e0;
      q1[1] = 0.01e0;
      q2low[0] = 3.0e0/4.0e0;
      q2low[1] = 0.0e0;
      q2hih[0] = 5.0e0/6.0e0;
      q2hih[1] = 0.0e0;
      q2best[0] = (double)*q2pade;
      q2best[1] = q2best[0]*0.015e0;
      pi[0] = 3.14159265358979;
      pi[1] = 0.0e0;
      six[0] = 6.0e0;
      six[1] = 0.0e0;
      three[0] = 3.0e0;
      three[1] = 0.0e0;
      four[0] = 4.0e0;
      four[1] = 0.0e0;
      two[0] = 2.0e0;
      two[1] = 0.0e0;
      five[0] = 5.0e0;
      five[1] = 0.0e0;
      boltz[0] = 1.38065e-23;
      boltz[1] = 1.0e-23/1.0e5;
      celsius[0] = 273.15e0;
      celsius[1] = 0.0e0;
      pre1[0] = sqrt(1.0e0/(4.0e0*pi[0]));
      pre2[0] = sqrt(1.0e0/pi[0])*(2.0e0/pi[0]);
      pre1[1] = 0.0e0;
      pre2[1] = 0.0e0;
      pi12[0] = pow(12.0e0*pi[0],1.0e0/3.0e0);
      pi12[1] = 0.0e0;
      fpercm[0] = 8.987e0;
      fpercm[1] = 0.001e0;
      fpercm[0] = fpercm[0]*1.0e11;
      fpercm[1] = fpercm[1]*1.0e11;
      return;
   }

   void parameters(
                   unsigned int *bt,
                   double temp[],
                   unsigned int *bc,
                   double eta[],
                   char *tunit,
                   char *vunit,
                   double celsius[],
                   int /* P1 */,
                   int /* P2 */)
   {
      /*
        must convert temp to kelvin, eta to centipoise
      */
      static double t1[2],t2[2];
      /*
********************************************************
*/
      if(*bt) {
         t1[0] = temp[0];
         t1[1] = temp[1];
         if(fifmemc(tunit,"C     ",6) == 0) {
            t2[0] = celsius[0];
            t2[1] = celsius[1];
            ftnscopy(tunit,6,"K     ",6,NULL);
         }
         else if(fifmemc(tunit,"K     ",6) == 0) {
            t2[0] = 0.0e0;
            t2[1] = 0.0e0;
         }
         summer(t1,t2,temp);
      }
      if(*bc) {
         t1[0] = eta[0];
         t1[1] = eta[1];
         if(fifmemc(vunit,"cp    ",6) == 0) {
            eta[0] = t1[0];
            eta[1] = t1[1];
         }
         else if(fifmemc(vunit,"p     ",6) == 0) {
            t2[0] = 100.0e0;
            t2[1] = 0.0e0;
            multiply(t1,t2,eta);
            ftnscopy(vunit,6,"cp    ",6,NULL);
         }
      }
      return;
   }

   void pack80(
               char *line,
               int /* P1 */)
   {
      static char copy[80];
      static int nz;
      /*
********************************************************
*/
      nz = 0;
      while(*line == ' ' && nz < 90) {
         ftnscopy((copy),79,(line+1),79,NULL);
         *(copy+79) = ' ';
         ftnscopy(line,80,copy,80,NULL);
         nz = nz+1;
      }
      return;
   }

   void floatstring(
                    char *tline,
                    double t1[],
                    int /* P1 */)
   {
      static string T2 = { NULL, 0 };
      static char cline[80];
      static double pre,eval,val,dig,place;
      static int J1,j2,j3,i,nex,nj,ndot;
      /*
********************************************************
*/
      J1 = fifindex(tline,80,"E",1);
      if(J1 == 0) {
         j2 = fifindex(tline,80,"e",1);
         if(j2 == 0) {
            j3 = fifindex(tline,80," ",1);
            T2.n = j3-1;T2.a = tline+T2.n;T2.n = j3+1-T2.n;
            ftnscopy(T2.a,T2.n,"e0",2,NULL);
         }
      }
      else {
         *(tline+J1-1) = 'e';
      }
      if(*tline == '-') {
         pre = -1.0e0;
         *(tline) = ' ';
      }
      else if(*tline == '+') {
         pre = 1.0e0;
         *(tline) = ' ';
      }
      else {
         pre = 1.0e0;
      }
      pack80(tline,80);
      J1 = fifindex(tline,80,"e",1);
      if(J1 == 0) Stop("e should be there by now");
      for(i=0; i<J1; i++) {
         *(cline+i) = ' ';
      }
      for(i=J1+0; i<80; i++) {
         *(cline+i) = *(tline+i);
         *(tline+i) = ' ';
      }
      *(tline+J1-1) = ' ';
      READ(INTERNAL,cline,80,LISTIO,INT4,&nex,0);
      nj = fifindex(tline,80,".",1);
      if(nj == 0) {
         nj = fifindex(tline,80," ",1);
         *(tline+nj-1) = '.';
      }
      ndot = fifindex(tline,80,".",1);
      if(ndot == 0) Stop("dot should be there by now");
      nj = fifindex(tline,80," ",1)-1;
      if(nj > ndot) {
         eval = pow(10.0e0,(double)(ndot-nj));
      }
      else if(nj == ndot) {
         eval = 1.0e0;
      }
      else {
         Stop("dotty");
      }
      val = 0.0e0;
      for(i=1; i<=nj; i++) {
         if(i != ndot) {
            READ(INTERNAL,(tline+i-1),1,LISTIO,REAL8,&dig,0);
            if(i < ndot) {
               place = pow(10.0e0,(double)(ndot-i-1));
            }
            else {
               place = pow(10.0e0,(double)(ndot-i));
            }
            val = val+dig*place;
         }
      }
      t1[0] = pre*val*pow(10.0e0,(double)nex);
      t1[1] = 0.5e0*eval*pow(10.0e0,(double)nex);
      return;
   }

   void dotout(
               char *name,
               char *line,
               int /* P1 */,
               int /* P2 */)
   {
      static int jj,i;
      /*
********************************************************
*/
      jj = fifindex(name,30,"@",1);
      ftnscopy(line,30,name,30,NULL);
      *(line+jj-1) = ' ';
      for(i=jj+1; i<=30; i++) {
         if(fifmod(i,2) == 1) *(line+i-1) = ' ';
         if(fifmod(i,2) == 0) *(line+i-1) = '.';
      }
      return;
   }
   void writeno(
                double *rl,
                char *name,
                char *lunit,
                int /* P1 */,
                int /* P2 */)
   {
      typedef struct {
         int nbod;
         int nzno;
         int nznr;
         int nstk;
         int ndfl;
         int nefl;
         int nzh;
         int nih;
         int nsh;
         int nph;
      } Cfilenumbers;
      static Cfilenumbers *Tfilenumbers = (Cfilenumbers*) Xfilenumbers;
      static char line[30];
      /*
********************************************************
*/
      static const char* F900[] = {
         "(a30,g15.6,4x,a6)"
      };
      dotout(name,line,30,30);
      WRITE(Tfilenumbers->nzno,FMT,F900,1,STRG,line,30,REAL8,*rl,STRG,lunit,6,0);
      WRITE(Tfilenumbers->nznr,FMT,F900,1,STRG,line,30,REAL8,*rl,STRG,lunit,6,0);
      return;
   }

   void compvisc(
                 double temp[],
                 double eta[],
                 char *vunit,
                 int /* P1 */)
   {
      static double tc,tm,den,z,num,tp;
      /*
********************************************************
*/
      /*
        Compute viscosity of water as a function of temperature
        Ref.  CRC handbook of chemistry and physics,
        55th edition p. F49
      */
      tc = temp[0]-273.15e0;
      if(tc <= 20.0e0) {
         tm = tc-20.0e0;
         den = 998.333e0+8.1855e0*tm+0.00585e0*pow(tm,2.0);
         z = 1301.0e0/den-3.30233e0;
         eta[0] = pow(10.0e0,z);
         eta[0] = eta[0]*100.0e0;
         eta[1] = 0.001e0;
      }
      else {
         tm = tc-20.0e0;
         tp = tc+105.0e0;
         num = -(1.3272e0*tm)-0.001053*tm*tm;
         z = num/tp;
         eta[0] = 1.002e0*pow(10.0e0,z);
         eta[1] = 0.0001e0;
      }
      ftnscopy(vunit,6,"cp    ",6,NULL);
      return;
   }

   void conventional(
                     double t1[],
                     double etam[],
                     char *lunit,
                     char *munit,
                     char *intunit,
                     int /* P1 */,
                     int /* P2 */,
                     int /* P3 */)
   {
      static double t2[2],conv[2];
      /*
********************************************************
*/
      /*
        :  possibilities for lunit are  m,cm,nm,A,L
        :  possibilities for munit are  kg,g,Da,kDa
        on entry, t1 contains the [eta]M value, but in units
        lunit**3/munit.
        This subroutine does the conversion to cm**3/gm
        and returns the result in etam
      */
      if(fifmemc(lunit,"L     ",6) == 0) {
         ftnscopy((intunit),4,"L^3/",4,NULL);
         ftnscopy((intunit+4),2,(munit),2,NULL);
         etam[0] = t1[0];
         etam[1] = t1[1];
         return;
      }
      if(fifmemc(lunit,"m     ",6) == 0) {
         conv[0] = 1.0e6;
      }
      else if(fifmemc(lunit,"cm    ",6) == 0) {
         conv[0] = 1.0e0;
      }
      else if(fifmemc(lunit,"nm    ",6) == 0) {
         conv[0] = 1.0e-21;
      }
      else if(fifmemc(lunit,"A     ",6) == 0) {
         conv[0] = 1.0e-24;
      }
      else {
         Stop("bad lunit in conventional");
      }
      conv[1] = 0.0e0;
      multiply(t1,conv,t2);
      if(fifmemc(munit,"kg    ",6) == 0) {
         conv[0] = 1.0e-3;
         conv[1] = 0.0e0;
      }
      else if(fifmemc(munit,"g     ",6) == 0) {
         conv[0] = 1.0e0;
         conv[1] = 0.0e0;
      }
      else if(fifmemc(munit,"Da    ",6) == 0) {
         conv[0] = 6.02214e23;
         conv[1] = 1.0e18;
      }
      else {
         conv[0] = 6.02214e20;
         conv[1] = 1.0e15;
      }
      multiply(t2,conv,etam);
      ftnscopy(intunit,6,"cm^3/g",6,NULL);
      return;
   }

   void friction(
                 double t1[],
                 double fric[],
                 char *lunit,
                 char *funit,
                 int /* P1 */,
                 int /* P2 */)
   {
      static double conv[2];
      /*
********************************************************
*/
      /*
        on entry, t1 contains the friction coefficient in
        units (cp lunit)
        This converts the friction constant to (d.s/cm) =
        dyne.sec/cm
        and returns the result in fric
      */
      if(fifmemc(lunit,"m     ",6) == 0) {
         conv[0] = 1.0e0;
      }
      else if(fifmemc(lunit,"cm    ",6) == 0) {
         conv[0] = 1.0e-2;
      }
      else if(fifmemc(lunit,"nm    ",6) == 0) {
         conv[0] = 1.0e-9;
      }
      else if(fifmemc(lunit,"A     ",6) == 0) {
         conv[0] = 1.0e-10;
      }
      else {
         Stop("bad lunit in friction");
      }
      conv[1] = 0.0e0;
      multiply(t1,conv,fric);
      ftnscopy(funit,6,"d.s/cm",6,NULL);
      return;
   }

   void svedberg(
                 double t1[],
                 double sed[],
                 char *munit,
                 char *lunit,
                 char *svedunit,
                 int /* P1 */,
                 int /* P2 */,
                 int /* P3 */)
   {
      static double conv[2],t2[2];
      /*
********************************************************
*/
      /*
        Upon entry, t1 contains the sedimentation coefficient in units
        of	(munit)*(centipoise^-1)*(lunit^-1)
        This subrtoutine does the conversion to svedbergs.
        Possibilities for lunit are m, cm, nm, A
        Possibilities for munit are kg, g, Da, kDa
      */
      conv[0] = 100.0e0;
      conv[1] = 0.0e0;
      multiply(t1,conv,t2);
      /*
        Units are now (munit)*(poise^-1)*(lunit^-1) =
        (munit)*(lunit^-1)*(sec)*(cm)*(gm^-1)
      */
      if(fifmemc(munit,"kg    ",6) == 0) {/*  Divide by kg/g */
         conv[0] = 1.0e-3;
         conv[1] = 0.0e0;
      }
      else if(fifmemc(munit,"g     ",6) == 0) {/*  Divide by g/g */
         conv[0] = 1.0e0;
         conv[1] = 0.0e0;
      }
      else if(fifmemc(munit,"Da    ",6) == 0) {/*  Divide by Da/g */
         conv[0] = 6.02214e23;
         conv[1] = 1.0e18;
      }
      else if(fifmemc(munit,"kDa   ",6) == 0) {/*  Divide by kDa/g */
         conv[0] = 6.02214e20;
         conv[1] = 1.0e15;
      }
      else {
         WRITE(OUTPUT,LISTIO,STRG,"munit = ",8,STRG,munit,6,0);
         Stop("unrecognized munit in svedberg");
      }
      divide(t2,conv,t1);
      /*
        Units are now (lunit^-1)(sec)(cm)
      */
      if(fifmemc(lunit,"m     ",6) == 0) {/*  multiply by 1D-2 m/cm */
         conv[0] = 1.0e-2;
         conv[1] = 0.0e0;
      }
      else if(fifmemc(lunit,"cm    ",6) == 0) {/*  multiply by 1 cm/cm */
         conv[0] = 1.0e0;
         conv[1] = 0.0e0;
      }
      else if(fifmemc(lunit,"nm    ",6) == 0) {/* multiply be 1D7 nm/cm */
         conv[0] = 1.0e7;
         conv[1] = 0.0e0;
      }
      else if(fifmemc(lunit,"A     ",6) == 0) {/* multiply by 1D8 A/cm */
         conv[0] = 1.0e8;
         conv[1] = 0.0e0;
      }
      else {
         WRITE(OUTPUT,LISTIO,STRG,"lunit = ",8,STRG,lunit,6,0);
         Stop("unrecognized lunit in Svedverg");
      }
      multiply(t1,conv,t2);
      /*
        Units are now sec, multiply by 1D13 Svedbergs/sec
      */
      conv[0] = 1.0e13;
      conv[1] = 0.0e0;
      multiply(t2,conv,sed);
      ftnscopy(svedunit,6,"Sved  ",6,NULL);
      return;
   }

   void diffuse(
                double t1[],
                double d[],
                char *lunit,
                char *dunit,
                int /* P1 */,
                int /* P2 */)
   {
      static double conv[2];
      /*
********************************************************
*/
      /*
        on entry, t1 contains the diffusion constant in
        units J/(cp lunit)
        This converts the diffusion constant to (cm^2/s)
        and returns the result in d
      */
      if(fifmemc(lunit,"m     ",6) == 0) {
         conv[0] = 1.0e7;
      }
      else if(fifmemc(lunit,"cm    ",6) == 0) {
         conv[0] = 1.0e9;
      }
      else if(fifmemc(lunit,"nm    ",6) == 0) {
         conv[0] = 1.0e16;
      }
      else if(fifmemc(lunit,"A     ",6) == 0) {
         conv[0] = 1.0e17;
      }
      else {
         Stop("bad lunit in diffuse");
      }
      conv[1] = 0.0e0;
      multiply(t1,conv,d);
      ftnscopy(dunit,6,"cm^2/s",6,NULL);
      return;
   }

   void writeyes(
                 double tx[],
                 char *name,
                 char *u,
                 int /* P1 */,
                 int /* P2 */)
   {
      typedef struct {
         int nbod;
         int nzno;
         int nznr;
         int nstk;
         int ndfl;
         int nefl;
         int nzh;
         int nih;
         int nsh;
         int nph;
      } Cfilenumbers;
      static Cfilenumbers *Tfilenumbers = (Cfilenumbers*) Xfilenumbers;
      static char line[30],mane[30],longname[49];
      static int jax,j;
      /*
********************************************************
*/
      static const char* F900[] = {
         "(a30,1x,a49)"
      };
      static const char* F700[] = {
         "(a30,2x,2g20.8,2x,a6)"
      };
      longjohn(tx,name,u,mane,line,longname,30,6,30,30,49);
      WRITE(Tfilenumbers->nzno,FMT,F900,1,STRG,line,30,STRG,longname,49,0);
      jax = fifindex(mane,30,"@",1);
      if(jax == 0) Stop("@ error");
      for(j=jax-1; j<30; j++) {
         *(mane+j) = ' ';
      }
      WRITE(Tfilenumbers->nznr,FMT,F700,1,STRG,mane,30,DO,2,REAL8,tx,STRG,u,6,0);
      return;
   }

   void longjohn(
                 double tx[],
                 char *name,
                 char *u,
                 char *mane,
                 char *line,
                 char *longname,
                 int /* P1 */,
                 int /* P4 */,
                 int /* P3 */,
                 int /* P2 */,
                 int /* P5 */)
   {
      static double t[2];
      static char String[20],byte[1],blanks[6],copy[49];
      static int k1,k2,nex,ndes,look,jq,jj;
      /*
********************************************************
*/
      static const char* F799[] = {
         "(i10,'.(',i7,')')"
      };
      static const char* F898[] = {
         "(a20,22x,'@',a6)"
      };
      static const char* F899[] = {
         "(a20,'E',a1,i20,'@',a6)"
      };
      static const char* F897[] = {
         "(a20,'E',a1,i20.2,'@',a6)"
      };
      ftnscopy(mane,30,name,30,NULL);
      t[0] = tx[0];
      t[1] = tx[1];
      ftnscopy(blanks,6,"      ",6,NULL);
      if(t[0] == 0.0e0 && t[1] == 0.0e0) {
         k1 = 0;
         k2 = 0;
      }
      else if(t[1] == 0.0e0) {
         k1 = fifnint(t[0]);
         k2 = 0;
         nex = 0;
         while(k1 > 999999 || 99999 >= k1) {
            if(k1 > 999999) {
               t[0] = t[0]/10.0e0;
               nex = nex+1;
            }
            else if(99999 >= k1) {
               t[0] = t[0]*10.0e0;
               nex = nex-1;
            }
            else {
               Stop("longjohn 1 error");
            }
            k1 = fifnint(t[0]);
         }
      }
      else {
         nex = 0;
         while(t[1] < 0.95e0 || 9.5 <= t[1]) {
            if(t[1] < 0.95e0) {
               t[0] = t[0]*10.0e0;
               t[1] = t[1]*10.0e0;
               nex = nex-1;
            }
            else if(9.5 <= t[1]) {
               t[0] = t[0]/10.0e0;
               t[1] = t[1]/10.0e0;
               nex = nex+1;
            }
            else {
               Stop("longjohn 3error");
            }
         }
         k1 = fifnint(t[0]);
         k2 = fifnint(t[1]);
      }
      WRITE(INTERNAL,String,20,FMT,F799,1,INT4,k1,INT4,k2,0);
      pack20(String,20);
      if(*String == '-') {
         ndes = 3;
      }
      else {
         ndes = 2;
      }
      look = fifindex(String,20,".",1);
      if(look == 0) Stop("longjohn error 2");
      while(look != ndes) {
         if(look > ndes) {
            shiftleft(String,20);
            nex = nex+1;
         }
         else {
            shiftright(String,20);
            nex = nex-1;
         }
         look = fifindex(String,20,".",1);
      }
      dotout(name,line,30,30);
      if(nex < 0) {
         *byte = '-';
         nex = fifiabs(nex);
      }
      else {
         *byte = '+';
      }
      if(nex == 0) {
         WRITE(INTERNAL,longname,49,FMT,F898,1,STRG,String,20,STRG,blanks,6,0);
      }
      else if(nex >= 10) {
         WRITE(INTERNAL,longname,49,FMT,F899,1,STRG,String,20,STRG,byte,1,INT4,nex,STRG,blanks,6,0);
      }
      else {
         WRITE(INTERNAL,longname,49,FMT,F897,1,STRG,String,20,STRG,byte,1,INT4,nex,STRG,blanks,6,0);
      }
      /*
        20 + 1 +1+ 20+  1 + 6 = 49
      */
      pack49(longname,49);
      jq = fifindex(longname,49,".",1);
      if(*(longname+jq) == '(') {
         *(longname+jq-1) = ' ';
         pack49(longname,49);
      }
      jj = fifindex(longname,49,"@",1);
      *(longname+jj-1) = ' ';
      if(*longname == '-') {
         ;
      }
      else {
         ftnscopy((copy+1),48,(longname),48,NULL);
         *(copy) = ' ';
         ftnscopy(longname,49,copy,49,NULL);
      }
      ftnscopy((longname+18),6,u,6,NULL);
      return;
   }

   void writeme(
                char *qunit,
                char *nounit,
                float *q,
                double *stfac1,
                double *stfac2,
                int /* P1 */,
                int /* P2 */)
   {
      typedef struct {
         int nbod;
         int nzno;
         int nznr;
         int nstk;
         int ndfl;
         int nefl;
         int nzh;
         int nih;
         int nsh;
         int nph;
      } Cfilenumbers;
      static Cfilenumbers *Tfilenumbers = (Cfilenumbers*) Xfilenumbers;
      static double t[2];
      static char name[30],line[30],mane[30],longname[49];
      static int jax,j;
      /*
************************************************************
*/
      static const char* F901[] = {
         "(g10.5,1x,a6,'@')"
      };
      static const char* F900[] = {
         "(a30,1x,a49)"
      };
      static const char* F700[] = {
         "(a30,2x,2g20.8,2x,a6)"
      };
      t[0] = *stfac1;
      t[1] = *stfac2;
      WRITE(INTERNAL,name,30,FMT,F901,1,REAL4,*q,STRG,qunit,6,0);
      longjohn(t,name,nounit,mane,line,longname,30,6,30,30,49);
      WRITE(Tfilenumbers->nzno,FMT,F900,1,STRG,line,30,STRG,longname,49,0);
      jax = fifindex(mane,30,"@",1);
      if(jax == 0) Stop("@ error");
      for(j=jax-1; j<30; j++) {
         *(mane+j) = ' ';
      }
      WRITE(Tfilenumbers->nznr,FMT,F700,1,STRG,mane,30,DO,2,REAL8,t,STRG,nounit,6,0);
      return;
   }

   void pack20(
               char *longname,
               int /* P1 */)
   {
      static char b1[1],b2[1];
      static int nwk;
      /*
********************************************************
*/
      nwk = 1;
      while(nwk < 20) {
         if(nwk == 0) nwk = 1;
         *b1 = *(longname+nwk-1);
         *b2 = *(longname+nwk+1-1);
         if(*b1 == ' ' && *b2 != ' ') {
            *(longname+nwk-1) = *b2;
            *(longname+nwk+1-1) = *b1;
            nwk = nwk-1;
         }
         else {
            nwk = nwk+1;
         }
      }
      return;
   }

   void pack49(char *longname,
               int /* P1 */)
   {
      static char b1[1],b2[1];
      static int nwk;
      /*
*********************************************************
*/
      nwk = 1;
      while(nwk < 49) {
         if(nwk == 0) nwk = 1;
         *b1 = *(longname+nwk-1);
         *b2 = *(longname+nwk+1-1);
         if(*b1 == ' ' && *b2 != ' ') {
            *(longname+nwk-1) = *b2;
            *(longname+nwk+1-1) = *b1;
            nwk = nwk-1;
         }
         else {
            nwk = nwk+1;
         }
      }
      return;
   }

   void shiftleft(char *String,
                  int /* P1 */)
   {
      static char b1[1],b2[1];
      static int idx;
      /*
*********************************************************
*/
      idx = fifindex(String,20,".",1);
      if(idx == 0) Stop("shiftleft error 1");
      if(idx == 1) Stop("shiftleft error 2");
      *b1 = *(String+idx-1-1);
      *b2 = *(String+idx-1);
      *(String+idx-1-1) = *b2;
      *(String+idx-1) = *b1;
      return;
   }

   void shiftright(
                   char *String,
                   int /* P1 */)
   {
      static char b1[1],b2[1];
      static int idx;
      /*
********************************************************
*/
      idx = fifindex(String,20,".",1);
      if(idx == 0) Stop("shiftright error 1");
      if(idx == 20) Stop("shiftright error 2");
      *b1 = *(String+idx+1-1);
      *b2 = *(String+idx-1);
      *(String+idx+1-1) = *b2;
      *(String+idx-1) = *b1;
      return;
   }

   void show_errors()
   {
      /*
        Send error list to report file
      */
      typedef struct {
         int nbod;
         int nzno;
         int nznr;
         int nstk;
         int ndfl;
         int nefl;
         int nzh;
         int nih;
         int nsh;
         int nph;
      } Cfilenumbers;
      static Cfilenumbers *Tfilenumbers = (Cfilenumbers*) Xfilenumbers;
      typedef struct {
         int ncube;
         float ferr;
      } Ccubit;
      static Ccubit *Tcubit = (Ccubit*) Xcubit;
      typedef struct {
         int nell;
         float rerr;
      } Csell;
      static Csell *Tsell = (Csell*) Xsell;
      /*
********************************************************
*/
      static const char* F611[] = {
         "('  Point found inside cube.  Only a cause')"
      };
      static const char* F612[] = {
         "('  for concern if the following number is')"
      };
      static const char* F613[] = {
         "('  not close to 1:',g20.8)"
      };
      static const char* F621[] = {
         "('  Overstretched ellipsoid.  Only a cause')"
      };
      static const char* F622[] = {
         "('  for concern if the following number is')"
      };
      static const char* F623[] = {
         "('  not close to 1:',g20.8)"
      };
      if(Tcubit->ncube == 1) {
         WRITE(Tfilenumbers->nzno,FMT,F611,1,0);
         WRITE(Tfilenumbers->nzno,FMT,F612,1,0);
         WRITE(Tfilenumbers->nzno,FMT,F613,1,REAL4,Tcubit->ferr,0);
      }
      if(Tsell->nell == 1) {
         WRITE(Tfilenumbers->nzno,FMT,F621,1,0);
         WRITE(Tfilenumbers->nzno,FMT,F622,1,0);
         WRITE(Tfilenumbers->nzno,FMT,F623,1,REAL4,Tsell->rerr,0);
      }
      return;
   }

   void pushon(char *byte,
               char *rs,
               int /* P2 */,
               int /* P1 */)
   {
      /*
        push a single byte onto the left side of a string
      */
      static char copy[15];
      /*
*************************************************
*/
      ftnscopy((copy+1),14,(rs),14,NULL);
      *(copy) = *byte;
      ftnscopy(rs,15,copy,15,NULL);
      return;
   }

   void do_launch(
                  int *maxelts,
                  int *nelts,
                  int eltype[],
                  float *bv,
                  float *rlaunch,
                  float *rotations,
                  float xyzlow[],
                  float xyzhih[])
   {
      /*
        Determine launch radius and enveloping box
      */
      typedef struct {
         int sphere_code;
         int triangle_code;
         int disk_code;
         int open_cylinder_code;
         int solid_cylinder_code;
         int donut_code;
         int ellipsoid_code;
         int cube_code;
         int pillar_code;
         int skin_code;
         int units_code;
         int hunits_code;
         int meter_code;
         int cm_code;
         int nm_code;
         int angstrom_code;
         int length_code;
         int temp_code;
         int celcius_code;
         int kelvin_code;
         int mass_code;
         int da_code;
         int kda_code;
         int gram_code;
         int kg_code;
         int visc_code;
         int poise_code;
         int cp_code;
         int solvent_code;
         int water_code;
         int rlaunch_code;
         int bf_code;
      } Cmorse;
      static Cmorse *Tmorse = (Cmorse*) Xmorse;
      static float c[3],v1[3],v2[3],v3[3],n[3],n1[3],n2[3],x[3],y[3],zl[3],zh[3],t[3][3],xaxis[3],yaxis[3],zaxis[3];
      static int i,j;
      static float side,dd,sx1,sx2,sy1,sy2,sz1,sz2,r;
      static int k;
      static float al,r1,r2,aa,bb,cc;
      /*
*************************************************
*/
      *rlaunch = 0.0F;
      for(i=0; i<3; i++) {
         xaxis[i] = 0.0F;
         yaxis[i] = 0.0F;
         zaxis[i] = 0.0F;
      }
      xaxis[0] = 1.0F;
      yaxis[1] = 1.0F;
      zaxis[2] = 1.0F;
      for(i=1; i<=*nelts; i++) {
         if(eltype[i-1] == Tmorse->cube_code) {
            for(j=0; j<3; j++) {
               v1[j] = *(bv+i-1+j**maxelts);
               side = *(bv+i-1+3**maxelts);
            }
            maxcube(v1,&side,&dd);
            procube(v1,&side,xaxis,&sx1,&sx2);
            procube(v1,&side,yaxis,&sy1,&sy2);
            procube(v1,&side,zaxis,&sz1,&sz2);
         }
         if(eltype[i-1] == Tmorse->pillar_code) {
            x[0] = *(bv+i-1+0**maxelts);
            x[1] = *(bv+i-1+4**maxelts);
            x[2] = *(bv+i-1+8**maxelts);
            y[0] = *(bv+i-1+1**maxelts);
            y[1] = *(bv+i-1+5**maxelts);
            y[2] = *(bv+i-1+9**maxelts);
            zl[0] = *(bv+i-1+2**maxelts);
            zl[1] = *(bv+i-1+6**maxelts);
            zl[2] = *(bv+i-1+10**maxelts);
            zh[0] = *(bv+i-1+3**maxelts);
            zh[1] = *(bv+i-1+7**maxelts);
            zh[2] = *(bv+i-1+11**maxelts);
            maxpillar(x,y,zl,zh,&dd);
            propillar(x,y,zl,zh,xaxis,&sx1,&sx2);
            propillar(x,y,zl,zh,yaxis,&sy1,&sy2);
            propillar(x,y,zl,zh,zaxis,&sz1,&sz2);
         }
         if(eltype[i-1] == Tmorse->sphere_code) {
            for(j=0; j<3; j++) {
               c[j] = *(bv+i-1+j**maxelts);
            }
            r = *(bv+i-1+3**maxelts);
            maxsphere(c,&r,&dd);
            prosphere(c,&r,xaxis,&sx1,&sx2);
            prosphere(c,&r,yaxis,&sy1,&sy2);
            prosphere(c,&r,zaxis,&sz1,&sz2);
         }
         if(eltype[i-1] == Tmorse->triangle_code) {
            for(j=0; j<3; j++) {
               v1[j] = *(bv+i-1+j**maxelts);
               v2[j] = *(bv+i-1+(j+3)**maxelts);
               v3[j] = *(bv+i-1+(j+6)**maxelts);
            }
            maxtriangle(v1,v2,v3,&dd);
            protriangle(v1,v2,v3,xaxis,&sx1,&sx2);
            protriangle(v1,v2,v3,yaxis,&sy1,&sy2);
            protriangle(v1,v2,v3,zaxis,&sz1,&sz2);
         }
         if(eltype[i-1] == Tmorse->disk_code) {
            for(j=0; j<3; j++) {
               c[j] = *(bv+i-1+j**maxelts);
               n[j] = *(bv+i-1+(j+3)**maxelts);
            }
            r = *(bv+i-1+6**maxelts);
            for(j=0; j<3; j++) {
               for(k=0; k<3; k++) {
                  t[k][j] = *(rotations+i-1+(j+k*3)**maxelts);
               }
            }
            maxdisk(c,n,&r,(float *)t,&dd);
            prodisk(c,n,&r,(float *)t,xaxis,&sx1,&sx2);
            prodisk(c,n,&r,(float *)t,yaxis,&sy1,&sy2);
            prodisk(c,n,&r,(float *)t,zaxis,&sz1,&sz2);
         }
         if(eltype[i-1] == Tmorse->open_cylinder_code) {
            for(j=0; j<3; j++) {
               c[j] = *(bv+i-1+j**maxelts);
               n[j] = *(bv+i-1+(j+3)**maxelts);
            }
            r = *(bv+i-1+6**maxelts);
            al = *(bv+i-1+7**maxelts);
            for(j=0; j<3; j++) {
               for(k=0; k<3; k++) {
                  t[k][j] = *(rotations+i-1+(j+k*3)**maxelts);
               }
            }
            maxcylinder(c,n,&r,&al,(float *)t,&dd);
            procylinder(c,n,&r,&al,(float *)t,xaxis,&sx1,&sx2);
            procylinder(c,n,&r,&al,(float *)t,yaxis,&sy1,&sy2);
            procylinder(c,n,&r,&al,(float *)t,zaxis,&sz1,&sz2);
         }
         if(eltype[i-1] == Tmorse->solid_cylinder_code) {
            for(j=0; j<3; j++) {
               c[j] = *(bv+i-1+j**maxelts);
               n[j] = *(bv+i-1+(j+3)**maxelts);
            }
            r = *(bv+i-1+6**maxelts);
            al = *(bv+i-1+7**maxelts);
            for(j=0; j<3; j++) {
               for(k=0; k<3; k++) {
                  t[k][j] = *(rotations+i-1+(j+k*3)**maxelts);
               }
            }
            maxsolcyl(c,n,&r,&al,(float *)t,&dd);
            prosolcyl(c,n,&r,&al,(float *)t,xaxis,&sx1,&sx2);
            prosolcyl(c,n,&r,&al,(float *)t,yaxis,&sy1,&sy2);
            prosolcyl(c,n,&r,&al,(float *)t,zaxis,&sz1,&sz2);
         }
         if(eltype[i-1] == Tmorse->donut_code) {
            for(j=0; j<3; j++) {
               c[j] = *(bv+i-1+j**maxelts);
               n[j] = *(bv+i-1+(j+3)**maxelts);
            }
            r1 = *(bv+i-1+6**maxelts);
            r2 = *(bv+i-1+7**maxelts);
            for(j=0; j<3; j++) {
               for(k=0; k<3; k++) {
                  t[k][j] = *(rotations+i-1+(j+k*3)**maxelts);
               }
            }
            maxtorus(c,n,&r1,&r2,(float *)t,&dd);
            protorus(c,n,&r1,&r2,(float *)t,xaxis,&sx1,&sx2);
            protorus(c,n,&r1,&r2,(float *)t,yaxis,&sy1,&sy2);
            protorus(c,n,&r1,&r2,(float *)t,zaxis,&sz1,&sz2);
         }
         if(eltype[i-1] == Tmorse->ellipsoid_code) {
            for(j=0; j<3; j++) {
               c[j] = *(bv+i-1+j**maxelts);
               n1[j] = *(bv+i-1+(j+3)**maxelts);
               n2[j] = *(bv+i-1+(j+6)**maxelts);
            }
            aa = *(bv+i-1+9**maxelts);
            bb = *(bv+i-1+10**maxelts);
            cc = *(bv+i-1+11**maxelts);
            for(j=0; j<3; j++) {
               for(k=0; k<3; k++) {
                  t[k][j] = *(rotations+i-1+(j+k*3)**maxelts);
               }
            }
            maxellipsoid(c,n1,n2,&aa,&bb,&cc,(float *)t,&dd);
            proellipsoid(c,n1,n2,&aa,&bb,&cc,(float *)t,xaxis,&sx1,&sx2);
            proellipsoid(c,n1,n2,&aa,&bb,&cc,(float *)t,yaxis,&sy1,&sy2);
            proellipsoid(c,n1,n2,&aa,&bb,&cc,(float *)t,zaxis,&sz1,&sz2);
         }
         *rlaunch = fifamax1(dd,*rlaunch);
         if(i == 1) {
            xyzlow[0] = sx1;
            xyzhih[0] = sx2;
            xyzlow[1] = sy1;
            xyzhih[1] = sy2;
            xyzlow[2] = sz1;
            xyzhih[2] = sz2;
         }
         else {
            xyzlow[0] = fifamin1(xyzlow[0],sx1);
            xyzhih[0] = fifamax1(xyzhih[0],sx2);
            xyzlow[1] = fifamin1(xyzlow[1],sy1);
            xyzhih[1] = fifamax1(xyzhih[1],sy2);
            xyzlow[2] = fifamin1(xyzlow[2],sz1);
            xyzhih[2] = fifamax1(xyzhih[2],sz2);
         }
      }
      return;
   }

   void alldone(char *endend,
                int /* P1 */)
   {
      /*
        get timestamp at end of job
      */
      typedef struct {
         char fbod[USZ_MAX_ID_SIZE];
         char fzno[USZ_MAX_ID_SIZE];
         char fznr[USZ_MAX_ID_SIZE];
         char fstk[USZ_MAX_ID_SIZE];
         char fdfl[USZ_MAX_ID_SIZE];
         char fefl[USZ_MAX_ID_SIZE];
         char fzh[USZ_MAX_ID_SIZE];
         char fih[USZ_MAX_ID_SIZE];
         char fsh[USZ_MAX_ID_SIZE];
         char fph[USZ_MAX_ID_SIZE];
      } Cfilenames;
      static Cfilenames *Tfilenames = (Cfilenames*) Xfilenames;
      // static char com[32];
      static char efl[USZ_MAX_ID_SIZE];
      /*
*************************************************
*/
      /*
        GEJ Do not bother getting an "end time" from the system.
        GEJ	com(1:7) = 'date > '
        GEJc                   1234567
        GEJ	com(8:32) = fefl
        GEJ	!call system(com)
      */

      gettime( endend );
      ftnscopy(efl,USZ_MAX_ID_SIZE,Tfilenames->fefl,USZ_MAX_ID_SIZE,NULL);
      return;
   }

   void setup(
              char *id,
              int m1[],
              char *actions,
              char *start,
              char *actout,
              unsigned int *savehits_z,
              unsigned int *savehits_i,
              unsigned int *savehits_s,
              unsigned int *savehits_p,
              int /* P2 */,
              int /* P1 */,
              int /* P3 */,
              int /* P4 */
              )
   {
      puts("s1");
      /*
        Parse the invocation string, and initialize random numbers
      */
      typedef struct {
         int nbod;
         int nzno;
         int nznr;
         int nstk;
         int ndfl;
         int nefl;
         int nzh;
         int nih;
         int nsh;
         int nph;
      } Cfilenumbers;
      static Cfilenumbers *Tfilenumbers = (Cfilenumbers*) Xfilenumbers;
      typedef struct {
         char fbod[USZ_MAX_ID_SIZE];
         char fzno[USZ_MAX_ID_SIZE];
         char fznr[USZ_MAX_ID_SIZE];
         char fstk[USZ_MAX_ID_SIZE];
         char fdfl[USZ_MAX_ID_SIZE];
         char fefl[USZ_MAX_ID_SIZE];
         char fzh[USZ_MAX_ID_SIZE];
         char fih[USZ_MAX_ID_SIZE];
         char fsh[USZ_MAX_ID_SIZE];
         char fph[USZ_MAX_ID_SIZE];
      } Cfilenames;
      static Cfilenames *Tfilenames = (Cfilenames*) Xfilenames;
      static string T5 = { NULL, 0 };
      static string T6 = { NULL, 0 };
      static string T7 = { NULL, 0 };
      static string T8 = { NULL, 0 };
      static string T9 = { NULL, 0 };
      static string T10 = { NULL, 0 };
      static string T11 = { NULL, 0 };
      static string T12 = { NULL, 0 };
      static string T13 = { NULL, 0 };
      static string T14 = { NULL, 0 };
      static char ac[30];
      static char pix[30];
      // nstatic char com[32];
      static char dfl[USZ_MAX_ID_SIZE];
      static int ntg,nac,i,nfo,mult,nzip,nsp;
      /*
*************************************************
*/
      *savehits_z = false;
      *savehits_i = false;
      *savehits_s = false;
      *savehits_p = false;
      /*
        Get the body ID:
      */
      puts("s2");
      fifgetarg(1,id,USZ_MAX_ID_SIZE);
      /*
        Get the action codes
      */
      ntg = fifiargc()-1;
      nac = 0;
      for(i=0; i<ntg; i++) {
         nfo = 0;
         fifgetarg(i+2,ac,30);
         if(*ac == 'z') {
            nac = nac+1;
            *(actions+nac-1) = 'z';
            nfo = 1;
         }
         else if(*ac == 'Z') {
            nac = nac+1;
            *(actions+nac-1) = 'Z';
            *savehits_z = true;
            nfo = 1;
         }
         else if(*ac == 'i') {
            nac = nac+1;
            *(actions+nac-1) = 'i';
            nfo = 1;
         }
         else if(*ac == 'I') {
            nac = nac+1;
            *(actions+nac-1) = 'I';
            *savehits_i = true;
            nfo = 1;
         }
         else if(*ac == 's') {
            nac = nac+1;
            *(actions+nac-1) = 's';
            nfo = 1;
         }
         else if(*ac == 'S') {
            nac = nac+1;
            *(actions+nac-1) = 'S';
            *savehits_s = true;
            nfo = 1;
         }
         else if(*ac == 'p') {
            nac = nac+1;
            *(actions+nac-1) = 'p';
            nfo = 1;
         }
         else if(*ac == 'P') {
            nac = nac+1;
            *(actions+nac-1) = 'P';
            nfo = 1;
            *savehits_p = true;
         }
         else if(*ac == 'c') {
            nac = nac+1;
            *(actions+nac-1) = 'c';
            nfo = 1;
         }
         else if(*ac == 'C') {
            nac = nac+1;
            *(actions+nac-1) = 'C';
            nfo = 1;
            *savehits_i = true;
         }
         else {
            Stop("bad action code -- expecting:  zisp or ZISP");
         }
         if(nac > 4) Stop("too many actions!!!!");
         if(nfo == 1) {
            ftnscopy((actout+(nac-1)*30),30,ac,30,NULL);
            *(ac) = ' ';
            ftnscopy(pix,30,ac,30,NULL);
            mult = 1;
            nzip = fifindex(pix,30,"t",1);
            if(nzip != 0) {
               mult = 1000*mult;
               *(pix+nzip-1) = ' ';
            }
            nzip = fifindex(pix,30,"m",1);
            if(nzip != 0) {
               mult = 1000000*mult;
               *(pix+nzip-1) = ' ';
            }
            nzip = fifindex(pix,30,"b",1);
            if(nzip != 0) {
               mult = 1000000000*mult;
               *(pix+nzip-1) = ' ';
            }
            READ(INTERNAL,pix,30,LISTIO,DO,1,INT4,&m1[nac-1],0);
            m1[nac-1] = m1[nac-1]*mult;
         }
      }
      puts("s3");
      if(nac < 4) {
         for(i=nac+0; i<4; i++) {
            *(actions+i) = '.';
         }
      }
      nsp = fifindex(id,USZ_MAX_ID_SIZE," ",1);
      ftnscopy(Tfilenames->fbod,USZ_MAX_ID_SIZE,id,USZ_MAX_ID_SIZE,NULL);
      T5.n = nsp-1;T5.a = Tfilenames->fbod+T5.n;T5.n = nsp+3-T5.n;
      ftnscopy(T5.a,T5.n,".bod",4,NULL);
      ftnscopy(Tfilenames->fzno,USZ_MAX_ID_SIZE,id,USZ_MAX_ID_SIZE,NULL);
      T6.n = nsp-1;T6.a = Tfilenames->fzno+T6.n;T6.n = nsp+3-T6.n;
      ftnscopy(T6.a,T6.n,".zno",4,NULL);
      ftnscopy(Tfilenames->fznr,USZ_MAX_ID_SIZE,id,USZ_MAX_ID_SIZE,NULL);
      T7.n = nsp-1;T7.a = Tfilenames->fznr+T7.n;T7.n = nsp+3-T7.n;
      ftnscopy(T7.a,T7.n,".znr",4,NULL);
      ftnscopy(Tfilenames->fstk,USZ_MAX_ID_SIZE,id,USZ_MAX_ID_SIZE,NULL);
      T8.n = nsp-1;T8.a = Tfilenames->fstk+T8.n;T8.n = nsp+3-T8.n;
      ftnscopy(T8.a,T8.n,".stk",4,NULL);
      ftnscopy(Tfilenames->fdfl,USZ_MAX_ID_SIZE,id,USZ_MAX_ID_SIZE,NULL);
      T9.n = nsp-1;T9.a = Tfilenames->fdfl+T9.n;T9.n = nsp+3-T9.n;
      ftnscopy(T9.a,T9.n,".dfl",4,NULL);
      ftnscopy(Tfilenames->fefl,USZ_MAX_ID_SIZE,id,USZ_MAX_ID_SIZE,NULL);
      T10.n = nsp-1;T10.a = Tfilenames->fefl+T10.n;T10.n = nsp+3-T10.n;
      ftnscopy(T10.a,T10.n,".efl",4,NULL);
      ftnscopy(Tfilenames->fzh,USZ_MAX_ID_SIZE,id,USZ_MAX_ID_SIZE,NULL);
      T11.n = nsp-1;T11.a = Tfilenames->fzh+T11.n;T11.n = nsp+2-T11.n;
      ftnscopy(T11.a,T11.n,".zh",3,NULL);
      ftnscopy(Tfilenames->fih,USZ_MAX_ID_SIZE,id,USZ_MAX_ID_SIZE,NULL);
      T12.n = nsp-1;T12.a = Tfilenames->fih+T12.n;T12.n = nsp+2-T12.n;
      ftnscopy(T12.a,T12.n,".ih",3,NULL);
      ftnscopy(Tfilenames->fsh,USZ_MAX_ID_SIZE,id,USZ_MAX_ID_SIZE,NULL);
      T13.n = nsp-1;T13.a = Tfilenames->fsh+T13.n;T13.n = nsp+2-T13.n;
      ftnscopy(T13.a,T13.n,".sh",3,NULL);
      ftnscopy(Tfilenames->fph,USZ_MAX_ID_SIZE,id,USZ_MAX_ID_SIZE,NULL);
      T14.n = nsp-1;T14.a = Tfilenames->fph+T14.n;T14.n = nsp+2-T14.n;
      ftnscopy(T14.a,T14.n,".ph",3,NULL);
      Tfilenumbers->nbod = 20;
      Tfilenumbers->nzno = 99;
      Tfilenumbers->nznr = 98;
      Tfilenumbers->nstk = 91;
      Tfilenumbers->ndfl = 82;
      Tfilenumbers->nefl = 82;
      Tfilenumbers->nzh = 25;
      Tfilenumbers->nih = 35;
      Tfilenumbers->nsh = 45;
      Tfilenumbers->nph = 55;
      OPEN(Tfilenumbers->nzno,FILEN,Tfilenames->fzno,USZ_MAX_ID_SIZE,STATUS,"unknown",0);
      /*
        GEJ: Do not bother getting a "start time" from the system.
        GEJ: Get it from pFortran instead -- it is used in random number genera
        GEJ	com(1:7) = 'date > '
        GEJc                   1234567
        GEJ	com(8:32) = fdfl
        GEJ	call system(com)
      */
      ftnscopy(dfl,USZ_MAX_ID_SIZE,Tfilenames->fdfl,USZ_MAX_ID_SIZE,NULL);

      // EB: check these
      // gettime(dfl,start,25,28);

      puts("s4");

      gettime( start );

      puts("s5");
      seeder();
      puts("s6");
      establish_codes();
      puts("s7");
      return;
   }

   void establish_codes()
   {
      typedef struct {
         int sphere_code;
         int triangle_code;
         int disk_code;
         int open_cylinder_code;
         int solid_cylinder_code;
         int donut_code;
         int ellipsoid_code;
         int cube_code;
         int pillar_code;
         int skin_code;
         int units_code;
         int hunits_code;
         int meter_code;
         int cm_code;
         int nm_code;
         int angstrom_code;
         int length_code;
         int temp_code;
         int celcius_code;
         int kelvin_code;
         int mass_code;
         int da_code;
         int kda_code;
         int gram_code;
         int kg_code;
         int visc_code;
         int poise_code;
         int cp_code;
         int solvent_code;
         int water_code;
         int rlaunch_code;
         int bf_code;
      } Cmorse;
      static Cmorse *Tmorse = (Cmorse*) Xmorse;
      /*
*************************************************
*/
      /*
        Map a unique integer to each one of these variables:
      */
      Tmorse->sphere_code = 333;
      Tmorse->triangle_code = 369;
      Tmorse->disk_code = 140;
      Tmorse->open_cylinder_code = -45;
      Tmorse->solid_cylinder_code = 45;
      Tmorse->donut_code = 314;
      Tmorse->ellipsoid_code = 95;
      Tmorse->cube_code = 66;
      Tmorse->pillar_code = 111;
      Tmorse->skin_code = 39;
      Tmorse->units_code = 99;
      Tmorse->hunits_code = 100;
      Tmorse->meter_code = 101;
      Tmorse->cm_code = 102;
      Tmorse->nm_code = 103;
      Tmorse->angstrom_code = -100;
      Tmorse->length_code = -101;
      Tmorse->temp_code = -102;
      Tmorse->celcius_code = -103;
      Tmorse->kelvin_code = 273;
      Tmorse->mass_code = 200;
      Tmorse->da_code = 201;
      Tmorse->kda_code = 202;
      Tmorse->gram_code = 203;
      Tmorse->kg_code = 204;
      Tmorse->visc_code = 210;
      Tmorse->poise_code = -210;
      Tmorse->cp_code = 211;
      Tmorse->solvent_code = -211;
      Tmorse->water_code = 212;
      Tmorse->rlaunch_code = 213;
      Tmorse->bf_code = 1096;
      return;
   }

   void packspaces(char *buffer,
                   int *next,
                   int /* P1 */)
   {
      static char copy[80];
      static int ngo;
      /*
*************************************************
*/
      /*
        pack leading spaces from the string "buffer"
      */
      ngo = 0;
      while(*buffer == ' ') {
         ftnscopy(copy,80,buffer,80,NULL);
         ftnscopy((buffer),79,(copy+1),79,NULL);
         *(buffer+79) = ' ';
         ngo = ngo+1;
         if(ngo == 80) {
            *next = 0;
            return;
         }
      }
      *next = fifindex(buffer,80," ",1);
      return;
   }

   void nextstring(unsigned int *nobuffer,
                   char *buffer,
                   char *String,
                   unsigned int *atend,
                   int /* P1 */,
                   int /* P2 */)
   {
      /*
        Interpret the next input string
      */
      typedef struct {
         unsigned int silent;
      } Cshush;
      // static Cshush *Tshush = (Cshush*) Xshush;
      typedef struct {
         int nbod;
         int nzno;
         int nznr;
         int nstk;
         int ndfl;
         int nefl;
         int nzh;
         int nih;
         int nsh;
         int nph;
      } Cfilenumbers;
      static Cfilenumbers *Tfilenumbers = (Cfilenumbers*) Xfilenumbers;
      static string T3 = { NULL, 0 };
      static string T4 = { NULL, 0 };
      static int next,i;
      /*
*************************************************
*/
      static const char* F980[] = {
         "(a80)"
      };
      *atend = false;
   S1:
      if(*nobuffer) {
         if(READ(Tfilenumbers->nbod,IOSTAT,NULL,1,FMT,F980,1,STRG,buffer,80,0)) goto S20;
         cleartabs(buffer,80);
         /*
           if (.not.silent) print 980,buffer
         */
         *nobuffer = false;
      }
      if(*buffer == '*') {
         *nobuffer = true;
         goto S1;
      }
      packspaces(buffer,&next,80);
      if(next == 0) {
         *nobuffer = true;
         goto S1;
      }
      for(i=0; i<80; i++) {
         *(String+i) = ' ';
      }
      T3.a = String;T3.n = next-1;
      T4.a = buffer;T4.n = next-1;
      ftnscopy(T3.a,T3.n,T4.a,T4.n,NULL);
      for(i=0; i<next-1; i++) {
         *(buffer+i) = ' ';
      }
      return;
   S20:
      *atend = true;
      return;
   }

   void cleartabs(char *buffer,
                  int /* P1 */)
   {
      static int i;
      /*
*************************************************
*/
      /*
        Convert tab-characters in input buffer to spaces
      */
      for(i=0; i<80; i++) {
         if(*(buffer+i) == 9) *(buffer+i) = ' ';
      }
      return;
   }

   void webster(char *dictionary,
                int map[],
                int *nwords,
                int * /* maxwords */,
                int /* P1 */)
   {
      typedef struct {
         int sphere_code;
         int triangle_code;
         int disk_code;
         int open_cylinder_code;
         int solid_cylinder_code;
         int donut_code;
         int ellipsoid_code;
         int cube_code;
         int pillar_code;
         int skin_code;
         int units_code;
         int hunits_code;
         int meter_code;
         int cm_code;
         int nm_code;
         int angstrom_code;
         int length_code;
         int temp_code;
         int celcius_code;
         int kelvin_code;
         int mass_code;
         int da_code;
         int kda_code;
         int gram_code;
         int kg_code;
         int visc_code;
         int poise_code;
         int cp_code;
         int solvent_code;
         int water_code;
         int rlaunch_code;
         int bf_code;
      } Cmorse;
      static Cmorse *Tmorse = (Cmorse*) Xmorse;
      static char word[10],slash1[51],slash2[51],slash3[51],slash4[51],slash5[51],slash6[51],slash7[51],copy[51];
      static int i;
      /*
*************************************************
*/
      *nwords = 65;
      ftnscopy(slash1,51,"ELLIPSOID,ellipsoid,VISCOSITY,viscosity,TRIANGLE,  ",51,NULL);
      ftnscopy(copy,51,slash1,51,NULL);
      for(i=0; i<5; i++) {
         commaparse(copy,word,51,10);
         ftnscopy((dictionary+i*10),10,word,10,NULL);
      }
      map[0] = Tmorse->ellipsoid_code;
      map[1] = Tmorse->ellipsoid_code;
      map[2] = Tmorse->visc_code;
      map[3] = Tmorse->visc_code;
      map[4] = Tmorse->triangle_code;
      ftnscopy(slash2,51,"triangle,CYLINDER,cylinder,SOLVENT,solvent,RLAUNCH,",51,NULL);
      ftnscopy(copy,51,slash2,51,NULL);
      for(i=5; i<11; i++) {
         commaparse(copy,word,51,10);
         ftnscopy((dictionary+i*10),10,word,10,NULL);
      }
      map[5] = Tmorse->triangle_code;
      map[6] = Tmorse->open_cylinder_code;
      map[7] = Tmorse->open_cylinder_code;
      map[8] = Tmorse->solvent_code;
      map[9] = Tmorse->solvent_code;
      map[10] = Tmorse->rlaunch_code;
      ftnscopy(slash3,51,"rlaunch,SOL_CYL,sol_cyl,SPHERE,sphere,HUNITS,      ",51,NULL);
      ftnscopy(copy,51,slash3,51,NULL);
      for(i=11; i<17; i++) {
         commaparse(copy,word,51,10);
         ftnscopy((dictionary+i*10),10,word,10,NULL);
      }
      map[11] = Tmorse->rlaunch_code;
      map[12] = Tmorse->solid_cylinder_code;
      map[13] = Tmorse->solid_cylinder_code;
      map[14] = Tmorse->sphere_code;
      map[15] = Tmorse->sphere_code;
      map[16] = Tmorse->hunits_code;
      ftnscopy(slash4,51,"hunits,PILLAR,pillar,UNITS,units,TORUS,torus,WATER,",51,NULL);
      ftnscopy(copy,51,slash4,51,NULL);
      for(i=17; i<25; i++) {
         commaparse(copy,word,51,10);
         ftnscopy((dictionary+i*10),10,word,10,NULL);
      }
      map[17] = Tmorse->hunits_code;
      map[18] = Tmorse->pillar_code;
      map[19] = Tmorse->pillar_code;
      map[20] = Tmorse->units_code;
      map[21] = Tmorse->units_code;
      map[22] = Tmorse->donut_code;
      map[23] = Tmorse->donut_code;
      map[24] = Tmorse->water_code;
      ftnscopy(slash5,51,"water,DISK,disk,CUBE,cube,MASS,mass,TEMP,temp,kDa, ",51,NULL);
      ftnscopy(copy,51,slash5,51,NULL);
      for(i=25; i<35; i++) {
         commaparse(copy,word,51,10);
         ftnscopy((dictionary+i*10),10,word,10,NULL);
      }
      map[25] = Tmorse->water_code;
      map[26] = Tmorse->disk_code;
      map[27] = Tmorse->disk_code;
      map[28] = Tmorse->cube_code;
      map[29] = Tmorse->cube_code;
      map[30] = Tmorse->mass_code;
      map[31] = Tmorse->mass_code;
      map[32] = Tmorse->temp_code;
      map[33] = Tmorse->temp_code;
      map[34] = Tmorse->kda_code;
      ftnscopy(slash6,51,"TO,to,ST,st,SC,sc,cm,nm,Da,kg,cp,PI,pi,S,s,T,t,D,d,",51,NULL);
      ftnscopy(copy,51,slash6,51,NULL);
      for(i=35; i<54; i++) {
         commaparse(copy,word,51,10);
         ftnscopy((dictionary+i*10),10,word,10,NULL);
      }
      map[35] = Tmorse->donut_code;
      map[36] = Tmorse->donut_code;
      map[37] = Tmorse->skin_code;
      map[38] = Tmorse->skin_code;
      map[39] = Tmorse->solid_cylinder_code;
      map[40] = Tmorse->solid_cylinder_code;
      map[41] = Tmorse->cm_code;
      map[42] = Tmorse->nm_code;
      map[43] = Tmorse->da_code;
      map[44] = Tmorse->kg_code;
      map[45] = Tmorse->cp_code;
      map[46] = Tmorse->pillar_code;
      map[47] = Tmorse->pillar_code;
      map[48] = Tmorse->sphere_code;
      map[49] = Tmorse->sphere_code;
      map[50] = Tmorse->triangle_code;
      map[51] = Tmorse->triangle_code;
      map[52] = Tmorse->disk_code;
      map[53] = Tmorse->disk_code;
      ftnscopy(slash7,51,"C,E,e,m,A,L,K,g,p,BF,bf,                           ",51,NULL);
      ftnscopy(copy,51,slash7,51,NULL);
      for(i=54; i<65; i++) {
         commaparse(copy,word,51,10);
         ftnscopy((dictionary+i*10),10,word,10,NULL);
      }
      map[54] = Tmorse->celcius_code;
      map[55] = Tmorse->ellipsoid_code;
      map[56] = Tmorse->ellipsoid_code;
      map[57] = Tmorse->meter_code;
      map[58] = Tmorse->angstrom_code;
      map[59] = Tmorse->length_code;
      map[60] = Tmorse->kelvin_code;
      map[61] = Tmorse->gram_code;
      map[62] = Tmorse->poise_code;
      map[63] = Tmorse->bf_code;
      map[64] = Tmorse->bf_code;
      return;
   }

   void commaparse(char *copy,
                   char *word,
                   int /* P1 */,
                   int /* P2 */)
   {
      static string T3 = { NULL, 0 };
      static string T4 = { NULL, 0 };
      static string T5 = { NULL, 0 };
      static string T6 = { NULL, 0 };
      static char list[51];
      static int nzip,ii;
      /*
*****************************************************************
*/
      nzip = fifindex(copy,51,",",1);
      T3.a = word;T3.n = nzip-1;
      T4.a = copy;T4.n = nzip-1;
      ftnscopy(T3.a,T3.n,T4.a,T4.n,NULL);
      *(word+nzip-1) = '@';
      if(nzip < 10) {
         for(ii=nzip+0; ii<10; ii++) {
            *(word+ii) = ' ';
         }
      }
      T5.a = list;T5.n = 51-nzip;
      T6.n = nzip;T6.a = copy+T6.n;T6.n = 51-T6.n;
      ftnscopy(T5.a,T5.n,T6.a,T6.n,NULL);
      for(ii=52-nzip-1; ii<51; ii++) {
         *(list+ii) = ' ';
      }
      ftnscopy(copy,51,list,51,NULL);
      return;
   }

   void testword(char *String,
                 int *ntype,
                 char *dictionary,
                 int map[],
                 int *nwords,
                 int * /* maxwords */,
                 int /* P2 */,
                 int /* P1 */)
   {
      typedef struct {
         int sphere_code;
         int triangle_code;
         int disk_code;
         int open_cylinder_code;
         int solid_cylinder_code;
         int donut_code;
         int ellipsoid_code;
         int cube_code;
         int pillar_code;
         int skin_code;
         int units_code;
         int hunits_code;
         int meter_code;
         int cm_code;
         int nm_code;
         int angstrom_code;
         int length_code;
         int temp_code;
         int celcius_code;
         int kelvin_code;
         int mass_code;
         int da_code;
         int kda_code;
         int gram_code;
         int kg_code;
         int visc_code;
         int poise_code;
         int cp_code;
         int solvent_code;
         int water_code;
         int rlaunch_code;
         int bf_code;
      } Cmorse;
      // static Cmorse *Tmorse = (Cmorse*) Xmorse;
      typedef struct {
         int nbod;
         int nzno;
         int nznr;
         int nstk;
         int ndfl;
         int nefl;
         int nzh;
         int nih;
         int nsh;
         int nph;
      } Cfilenumbers;
      static Cfilenumbers *Tfilenumbers = (Cfilenumbers*) Xfilenumbers;
      static int nlen,match,i;
      /*
*****************************************************************
*/
      static const char* F900[] = {
         "('Word longer than any in dictionary:  ')"
      };
      static const char* F901[] = {
         "(a80)"
      };
      static const char* F800[] = {
         "('Word not in dictionary:  ')"
      };
      /*
        Is string a recognized word?  If so encode its value in ntype
        Here is the complete dictionary:  If string does not match one
        of these, abort, otherwise return its equivalent code.
        Word              Code
        ----------------------------------
        ELLIPSOID       ellipsoid_code
        ellipsoid       ellipsoid_code
        VISCOSITY       visc_code
        viscosity       visc_code
        TRIANGLE        triangle_code
        triangle        triangle_code
        CYLINDER        open_cylinder_code
        cylinder        open_cylinder_code
        SOLVENT         solvent_code
        solvent         solvent_code
        RLAUNCH	        rlaunch_code
        rlaunch         rlaunch_code
        SOL_CYL         solid_cylinder_code
        sol_cyl         solid_cylinder_code
        SPHERE	        sphere_code
        sphere	        sphere_code
        HUNITS          hunits_code
        hunits          hunits_code
        PILLAR          pillar_code
        pillar          pillar_code
        UNITS           units_code
        units           units_code
        TORUS           donut_code
        torus           donut_code
        WATER           water_code
        water           water_code
        DISK   	        disk_code
        disk   	        disk_code
        CUBE            cube_code
        cube            cube_code
        MASS            mass_code
        mass            mass_code
        TEMP            temp_code
        temp            temp_code
        kDa             kda_code
        TO              donut_code
        to              donut_code
        ST              skin_code
        st	        skin_code
        SC              solid_cylinder_code
        sc              solid_cylinder_code
        cm              cm_code
        nm              nm_code
        Da              da_code
        kg              kg_code
        cp              cp_code
        PI              pillar_code
        pi              pillar_code
        S	        sphere_code
        s    	        sphere_code
        T	        triangle_code
        t               triangle_code
        D   	        disk_code
        d   	        disk_code
        C               celcius_code
        E	        ellipsoid_code
        e 	        ellipsoid_code
        m	        meter_code
        A	        angstrom_code
        L	        length_code
        K	        kelvin_code
        g	        gram_code
        p               poise_code
        BF		bf_code
        bf		bf_code
      */
      nlen = fifindex(String,80," ",1)-1;
      if(nlen > 9) {
         WRITE(Tfilenumbers->nzno,FMT,F900,1,0);
         WRITE(Tfilenumbers->nzno,FMT,F901,1,STRG,String,80,0);
         Stop("word longer than any in dictionary");
      }
      match = 0;
      for(i=1; i<=*nwords; i++) {
         if(*(dictionary+(i-1)*10+nlen) == '@') {
            if(fifmemc((dictionary+(i-1)*10),String,nlen-(1-1)) == 0) {
               match = i;
            }
         }
      }
      if(match == 0) {
         WRITE(Tfilenumbers->nzno,FMT,F800,1,0);
         WRITE(Tfilenumbers->nzno,FMT,F901,1,STRG,String,80,0);
         Stop("word not in dictionary");
      }
      *ntype = map[match-1];
      return;
   }

   void parse(
              int *maxelts,
              int *nelts,
              int eltype[],
              float *bv,
              float *tol,
              float *rotations,
              unsigned int *tolset,
              char *unitcode,
              unsigned int *bt,
              unsigned int *bm,
              unsigned int *bw,
              unsigned int *bc,
              unsigned int *bbf,
              double temp[],
              char *tunit,
              double mass[],
              char *munit,
              double visc[],
              char *vunit,
              double buoy[],
              float *hscale,
              float *rlaunch,
              unsigned int *launch_done,
              char *dictionary,
              int map[],
              int *maxwords,
              int *nwords,
              int /* P1 */,
              int /* P2 */,
              int /* P3 */,
              int /* P4 */,
              int /* P5 */
              )
   {
      /*
        Parse the body file
      */
      typedef struct {
         int sphere_code;
         int triangle_code;
         int disk_code;
         int open_cylinder_code;
         int solid_cylinder_code;
         int donut_code;
         int ellipsoid_code;
         int cube_code;
         int pillar_code;
         int skin_code;
         int units_code;
         int hunits_code;
         int meter_code;
         int cm_code;
         int nm_code;
         int angstrom_code;
         int length_code;
         int temp_code;
         int celcius_code;
         int kelvin_code;
         int mass_code;
         int da_code;
         int kda_code;
         int gram_code;
         int kg_code;
         int visc_code;
         int poise_code;
         int cp_code;
         int solvent_code;
         int water_code;
         int rlaunch_code;
         int bf_code;
      } Cmorse;
      static Cmorse *Tmorse = (Cmorse*) Xmorse;
      typedef struct {
         int nbod;
         int nzno;
         int nznr;
         int nstk;
         int ndfl;
         int nefl;
         int nzh;
         int nih;
         int nsh;
         int nph;
      } Cfilenumbers;
      static Cfilenumbers *Tfilenumbers = (Cfilenumbers*) Xfilenumbers;
      typedef struct {
         char fbod[USZ_MAX_ID_SIZE];
         char fzno[USZ_MAX_ID_SIZE];
         char fznr[USZ_MAX_ID_SIZE];
         char fstk[USZ_MAX_ID_SIZE];
         char fdfl[USZ_MAX_ID_SIZE];
         char fefl[USZ_MAX_ID_SIZE];
         char fzh[USZ_MAX_ID_SIZE];
         char fih[USZ_MAX_ID_SIZE];
         char fsh[USZ_MAX_ID_SIZE];
         char fph[USZ_MAX_ID_SIZE];
      } Cfilenames;
      static Cfilenames *Tfilenames = (Cfilenames*) Xfilenames;
      static float t[3][3],n1[3],n[3],nx[3],ny[3],n2[3],nz[3];
      static char down[13];
      static unsigned int nobuffer,atend;
      static char buffer[80],String[80],command[80];
      static double download[2][12];
      static int ntype,i;
      static float r1,r2;
      static int j,k,ntype2;
      /*
*************************************************
*/
      static const char* F9009[] = {
         "('Impossible torus:  r2 > r1 ')"
      };
      static const char* F902[] = {
         "('Bad modifier to UNITS command')"
      };
      static const char* F903[] = {
         "('Bad modifier to HUNITS command')"
      };
      static const char* F904[] = {
         "('Bad modifier to TEMP command')"
      };
      static const char* F905[] = {
         "('Bad modifier to MASS command')"
      };
      static const char* F906[] = {
         "('Bad modifier to VISCOSITY command')"
      };
      static const char* F711[] = {
         "('VISCOSITY and SOLVENT commands both found.')"
      };
      static const char* F712[] = {
         "('Only the first was used.')"
      };
      static const char* F907[] = {
         "('Bad modifier to SOLVENT command')"
      };
      static const char* F610[] = {
         "('Unrecognized command: ')"
      };
      static const char* F611[] = {
         "(a80)"
      };
      static const char* F612[] = {
         "('Too many elements.')"
      };
      static const char* F613[] = {
         "('No elements loaded')"
      };
      /*
        List of recognized commands:
        ------------------------------------------------------
        SPHERE, sphere, S, s    (sphere_code)
        cx, cy, cz, r
        ------------------------------------------------------
        TRIANGLE, triangle, T, t  (triangle_code)
        v1x,v1y,v1z,v2x,v2y,v2z,v3x,v3y,v3z
        ------------------------------------------------------
        DISK, disk, D, d   (disk_code)
        cx,cy,cz,nx,ny,nz,r
        ------------------------------------------------------
        CYLINDER, cylinder    (open_cylinder_code)
        cx,cy,cz,nx,ny,nz,r,l
        ------------------------------------------------------
        SOL_CYL, sol_cyl, SC, sc    (solid_cylinder_code)
        cx,cy,cz,nx,ny,nz,r,l
        ------------------------------------------------------
        TORUS, torus, TO, to     (donut_code)
        cx,cy,cz,nx,ny,nz,r1,r2
        ------------------------------------------------------
        ELLIPSOID, ellipsoid, E, e  (ellipsoid_code)
        cx,cy,cz,n1x,n1y,n1z,n2x,n2y,n2z,aa,bb,cc
        ------------------------------------------------------
        CUBE, cube		  (cube_code)
        cx,cy,cz,s
        ------------------------------------------------------
        PILLAR, pillar, PI, pi    (pillar_code)
        x1,y1,z1l,z1h,x2,y2,z2l,z2h,x3,y3,z3l,z3h
        ------------------------------------------------------
        ST, st         (skin_code)
        tol
        ------------------------------------------------------
        RLAUNCH, rlaunch  (User-defined launch radius, to use
        in place of radius determined
        automatically.)
        (rlaunch_code)
        ------------------------------------------------------
        UNITS, units	(units_code)
        Modifier      Internal
        string         code	        Meaning
        -------       --------          -------
        m		 meter_code     meters
        cm		 cm_code        centimeters
        nm		 nm_code        nanometers
        A		 angstrom_code  Angstroms
        L		 length_code    generic or unspecified length units
        ------------------------------------------------------
        TEMP, temp    (temp_code)
        First modifier:   number (value of temperature)
        Second
        modifier      Internal
        string          code            Meaning
        -------       --------          -------
        C		celcius_code    Celcius
        K            kelvin_code     Kelvin
        ------------------------------------------------------
        MASS, mass    (mass_code)
        First modifier:  number (value of mass)
        Second
        modifier      Internal
        string          code         Meaning
        -------       --------       -------
        Da		da_code	     daltons
        kDa		kda_code     kilodaltons
        g            gram_code    grams
        kg           kg_code      kilograms
        ------------------------------------------------------
        VISCOSITY, viscosity (visc_code)
        Fist modifier:  number (value of solvent viscosity)
        Second
        modifier      Internal
        string          code         Meaning
        -------       --------       -------
        p          poise_code      poise
        cp          cp_code         centipoise
        ------------------------------------------------------
        BF, bf (bf_code)
        First modifier:  number (value of buoyancy factor)
        ------------------------------------------------------
      */
      OPEN(Tfilenumbers->nbod,FILEN,Tfilenames->fbod,USZ_MAX_ID_SIZE,STATUS,"unknown",0);
      *launch_done = false;
      *nelts = 0;
      *tolset = false;
      ftnscopy(unitcode,2,"L ",2,NULL);
      *bt = false;
      *bm = false;
      *bw = false;
      *bc = false;
      *bbf = false;
      *hscale = 1.0F;                /*  Stays unity throughout, unless modified */
      /*
        !   by the HUNITS   command
      */
      webster(dictionary,map,nwords,maxwords,10);
      nobuffer = true;
      nextstring(&nobuffer,buffer,String,&atend,80,80);
      while(!atend) {                /*  GRANDDADDY loop goes through all */
         /*
           commands in the file
         */
         testword(String,&ntype,dictionary,map,nwords,maxwords,80,10);
         ftnscopy(command,80,String,80,NULL);
         if(ntype == Tmorse->sphere_code || ntype == Tmorse->cube_code) {
            ftnscopy(down,13,"rrrr@",5,NULL);
            modifiers(
                      &nobuffer,
                      buffer,
                      String,
                      &atend,
                      down,
                      (double *)download,
                      dictionary,
                      map,
                      maxwords,
                      nwords,
                      80,
                      80,
                      13,
                      10);
            *nelts = *nelts+1;
            eltype[*nelts-1] = ntype;
            for(i=0; i<4; i++) {
               *(bv+*nelts-1+i**maxelts) = (float)download[0][i];
            }
         }
         else if(ntype == Tmorse->triangle_code) {
            ftnscopy(down,13,"rrrrrrrrr@",10,NULL);
            modifiers(&nobuffer,
                      buffer,
                      String,
                      &atend,
                      down,
                      (double *)download,
                      dictionary,
                      map,
                      maxwords,
                      nwords,
                      80,
                      80,
                      13,
                      10);
            *nelts = *nelts+1;
            eltype[*nelts-1] = ntype;
            for(i=0; i<9; i++) {
               *(bv+*nelts-1+i**maxelts) = (float)download[0][i];
            }
         }
         else if(ntype == Tmorse->donut_code) {
            ftnscopy(down,13,"rrrrrrrr@",9,NULL);
            modifiers(&nobuffer,
                      buffer,
                      String,
                      &atend,
                      down,
                      (double *)download,
                      dictionary,
                      map,
                      maxwords,
                      nwords,
                      80,
                      80,
                      13,
                      10);
            *nelts = *nelts+1;
            eltype[*nelts-1] = ntype;
            for(i=0; i<8; i++) {
               *(bv+*nelts-1+i**maxelts) = (float)download[0][i];
            }
            r1 = *(bv+*nelts-1+6**maxelts);
            r2 = *(bv+*nelts-1+7**maxelts);
            if(r2 > r1) {
               WRITE(Tfilenumbers->nzno,FMT,F9009,1,0);
               Stop("Impossible torus:  r2 > r1 ");
            }
            for(j=0; j<3; j++) {
               n1[j] = *(bv+*nelts-1+(j+3)**maxelts);
            }
            normalize(n1,n);
            zeerot(n,(float *)t);
            for(j=0; j<3; j++) {
               for(k=0; k<3; k++) {
                  *(rotations+*nelts-1+(j+k*3)**maxelts) = t[k][j];
               }
            }
            for(j=0; j<3; j++) {
               *(bv+*nelts-1+(j+3)**maxelts) = n[j];
            }
         }
         else if(ntype == Tmorse->disk_code) {
            ftnscopy(down,13,"rrrrrrr@",8,NULL);
            modifiers(&nobuffer,
                      buffer,
                      String,
                      &atend,
                      down,
                      (double *)download,
                      dictionary,
                      map,
                      maxwords,
                      nwords,
                      80,
                      80,
                      13,
                      10);
            *nelts = *nelts+1;
            eltype[*nelts-1] = ntype;
            for(i=0; i<7; i++) {
               *(bv+*nelts-1+i**maxelts) = (float)download[0][i];
            }
            for(j=0; j<3; j++) {
               n1[j] = *(bv+*nelts-1+(j+3)**maxelts);
            }
            normalize(n1,n);
            zeerot(n,(float *)t);
            for(j=0; j<3; j++) {
               for(k=0; k<3; k++) {
                  *(rotations+*nelts-1+(j+k*3)**maxelts) = t[k][j];
               }
            }
            for(j=0; j<3; j++) {
               *(bv+*nelts-1+(j+3)**maxelts) = n[j];
            }
         }
         else if(ntype == Tmorse->open_cylinder_code || ntype == Tmorse->solid_cylinder_code) {
            ftnscopy(down,13,"rrrrrrrr@",9,NULL);
            modifiers(&nobuffer,
                      buffer,
                      String,
                      &atend,
                      down,
                      (double *)download,
                      dictionary,
                      map,
                      maxwords,
                      nwords,
                      80,
                      80,
                      13,
                      10);
            *nelts = *nelts+1;
            eltype[*nelts-1] = ntype;
            for(i=0; i<8; i++) {
               *(bv+*nelts-1+i**maxelts) = (float)download[0][i];
            }
            for(j=0; j<3; j++) {
               n1[j] = *(bv+*nelts-1+(j+3)**maxelts);
            }
            normalize(n1,
                      n);
            zeerot(n,(float *)t);
            for(j=0; j<3; j++) {
               for(k=0; k<3; k++) {
                  *(rotations+*nelts-1+(j+k*3)**maxelts) = t[k][j];
               }
            }
            for(j=0; j<3; j++) {
               *(bv+*nelts-1+(j+3)**maxelts) = n[j];
            }
         }
         else if(ntype == Tmorse->ellipsoid_code) {
            ftnscopy(down,13,"rrrrrrrrrrrr@",13,NULL);
            modifiers(&nobuffer,
                      buffer,
                      String,
                      &atend,
                      down,
                      (double *)download,
                      dictionary,
                      map,
                      maxwords,
                      nwords,
                      80,
                      80,
                      13,
                      10);
            *nelts = *nelts+1;
            eltype[*nelts-1] = ntype;
            for(i=0; i<12; i++) {
               *(bv+*nelts-1+i**maxelts) = (float)download[0][i];
            }
            for(j=0; j<3; j++) {
               n1[j] = *(bv+*nelts-1+(j+3)**maxelts);
               n2[j] = *(bv+*nelts-1+(j+6)**maxelts);
            }
            normalize(n1,
                      nx);
            normalize(n2,ny);
            for(j=0; j<3; j++) {
               *(bv+*nelts-1+(j+3)**maxelts) = nx[j];
               *(bv+*nelts-1+(j+6)**maxelts) = ny[j];
            }
            cross_product(nx,ny,nz);
            xyzrot(nx,ny,nz,(float *)t);
            for(j=0; j<3; j++) {
               for(k=0; k<3; k++) {
                  *(rotations+*nelts-1+(j+k*3)**maxelts) = t[k][j];
               }
            }
         }
         else if(ntype == Tmorse->pillar_code) {
            ftnscopy(down,13,"rrrrrrrrrrrr@",13,NULL);
            modifiers(&nobuffer,
                      buffer,
                      String,
                      &atend,
                      down,
                      (double *)download,
                      dictionary,
                      map,
                      maxwords,
                      nwords,
                      80,
                      80,
                      13,
                      10);
            *nelts = *nelts+1;
            eltype[*nelts-1] = ntype;
            for(i=0; i<12; i++) {
               *(bv+*nelts-1+i**maxelts) = (float)download[0][i];
            }
         }
         else if(ntype == Tmorse->skin_code) {
            ftnscopy(down,13,"r@",2,NULL);
            modifiers(&nobuffer,
                      buffer,
                      String,
                      &atend,
                      down,
                      (double *)download,
                      dictionary,
                      map,
                      maxwords,
                      nwords,
                      80,
                      80,
                      13,
                      10);
            *tol = (float)download[0][0];
            *tolset = true;
         }
         else if(ntype == Tmorse->bf_code) {
            ftnscopy(down,13,"d@",2,NULL);
            modifiers(&nobuffer,
                      buffer,
                      String,
                      &atend,
                      down,
                      (double *)download,
                      dictionary,
                      map,
                      maxwords,
                      nwords,
                      80,
                      80,
                      13,
                      10);
            buoy[0] = download[0][0];
            buoy[1] = download[1][0];
            *bbf = true;
         }
         else if(ntype == Tmorse->rlaunch_code) {
            ftnscopy(down,13,"r@",2,NULL);
            modifiers(&nobuffer,
                      buffer,
                      String,
                      &atend,
                      down,
                      (double *)download,
                      dictionary,
                      map,
                      maxwords,
                      nwords,
                      80,
                      80,
                      13,
                      10);
            *rlaunch = (float)download[0][0];
            *launch_done = true;
         }
         else if(ntype == Tmorse->units_code) {
            ftnscopy(down,13,"n@",2,NULL);
            modifiers(&nobuffer,
                      buffer,
                      String,
                      &atend,
                      down,
                      (double *)download,
                      dictionary,
                      map,
                      maxwords,
                      nwords,
                      80,
                      80,
                      13,
                      10);
            ntype2 = fifnint(download[0][0]);
            if(ntype2 == Tmorse->meter_code) {
               ftnscopy(unitcode,2,"m ",2,NULL);
            }
            else if(ntype2 == Tmorse->cm_code) {
               ftnscopy(unitcode,2,"cm",2,NULL);
            }
            else if(ntype2 == Tmorse->nm_code) {
               ftnscopy(unitcode,2,"nm",2,NULL);
            }
            else if(ntype2 == Tmorse->angstrom_code) {
               ftnscopy(unitcode,2,"A ",2,NULL);
            }
            else if(ntype2 == Tmorse->length_code) {
               ftnscopy(unitcode,2,"L ",2,NULL);
            }
            else {
               WRITE(Tfilenumbers->nzno,FMT,F902,1,0);
               Stop("bad modifier to UNITS command");
            }
         }
         else if(ntype == Tmorse->hunits_code) {
            ftnscopy(down,13,"rn@",3,NULL);
            modifiers(&nobuffer,
                      buffer,
                      String,
                      &atend,
                      down,
                      (double *)download,
                      dictionary,
                      map,
                      maxwords,
                      nwords,
                      80,
                      80,
                      13,
                      10);
            ntype2 = fifnint(download[0][1]);
            *hscale = (float)download[0][0];
            if(ntype2 == Tmorse->meter_code) {
               ftnscopy(unitcode,2,"m ",2,NULL);
            }
            else if(ntype2 == Tmorse->cm_code) {
               ftnscopy(unitcode,2,"cm",2,NULL);
            }
            else if(ntype2 == Tmorse->nm_code) {
               ftnscopy(unitcode,2,"nm",2,NULL);
            }
            else if(ntype2 == Tmorse->angstrom_code) {
               ftnscopy(unitcode,2,"A ",2,NULL);
            }
            else {
               WRITE(Tfilenumbers->nzno,FMT,F903,1,0);
               Stop("bad modifier to HUNITS command");
            }
         }
         else if(ntype == Tmorse->temp_code) {
            ftnscopy(down,13,"dn@",3,NULL);
            modifiers(&nobuffer,
                      buffer,
                      String,
                      &atend,
                      down,
                      (double *)download,
                      dictionary,
                      map,
                      maxwords,
                      nwords,
                      80,
                      80,
                      13,
                      10);
            temp[0] = download[0][0];
            temp[1] = download[1][0];
            ntype2 = fifnint(download[0][1]);
            if(ntype2 == Tmorse->kelvin_code) {
               ftnscopy(tunit,6,"K     ",6,NULL);
            }
            else if(ntype2 == Tmorse->celcius_code) {
               ftnscopy(tunit,6,"C     ",6,NULL);
            }
            else {
               WRITE(Tfilenumbers->nzno,FMT,F904,1,0);
               Stop("bad modifier to TEMP command");
            }
            *bt = true;
         }
         else if(ntype == Tmorse->mass_code) {
            ftnscopy(down,13,"dn@",3,NULL);
            modifiers(&nobuffer,
                      buffer,
                      String,
                      &atend,
                      down,
                      (double *)download,
                      dictionary,
                      map,
                      maxwords,
                      nwords,
                      80,
                      80,
                      13,
                      10);
            mass[0] = download[0][0];
            mass[1] = download[1][0];
            ntype2 = fifnint(download[0][1]);
            if(ntype2 == Tmorse->da_code) {
               ftnscopy(munit,6,"Da    ",6,NULL);
            }
            else if(ntype2 == Tmorse->kda_code) {
               ftnscopy(munit,6,"kDa   ",6,NULL);
            }
            else if(ntype2 == Tmorse->gram_code) {
               ftnscopy(munit,6,"g     ",6,NULL);
            }
            else if(ntype2 == Tmorse->kg_code) {
               ftnscopy(munit,6,"kg    ",6,NULL);
            }
            else {
               WRITE(Tfilenumbers->nzno,FMT,F905,1,0);
               Stop("bad modifier to MASS command");
            }
            *bm = true;
         }
         else if(ntype == Tmorse->visc_code) {
            ftnscopy(down,13,"dn@",3,NULL);
            modifiers(&nobuffer,
                      buffer,
                      String,
                      &atend,
                      down,
                      (double *)download,
                      dictionary,
                      map,
                      maxwords,
                      nwords,
                      80,
                      80,
                      13,
                      10);
            if(!*bw) {
               visc[0] = download[0][0];
               visc[1] = download[1][0];
               ntype2 = fifnint(download[0][1]);
               if(ntype2 == Tmorse->poise_code) {
                  ftnscopy(vunit,6,"p     ",6,NULL);
               }
               else if(ntype2 == Tmorse->cp_code) {
                  ftnscopy(vunit,6,"cp    ",6,NULL);
               }
               else {
                  WRITE(Tfilenumbers->nzno,FMT,F906,1,0);
                  Stop("bad modifier to VISCOSITY command");
               }
               *bc = true;
            }
            else {
               WRITE(Tfilenumbers->nzno,FMT,F711,1,0);
               WRITE(Tfilenumbers->nzno,FMT,F712,1,0);
            }
         }
         else if(ntype == Tmorse->solvent_code) {
            ftnscopy(down,13,"n@",2,NULL);
            modifiers(&nobuffer,
                      buffer,
                      String,
                      &atend,
                      down,
                      (double *)download,
                      dictionary,
                      map,
                      maxwords,
                      nwords,
                      80,
                      80,
                      13,
                      10);
            if(!*bc) {
               ntype2 = fifnint(download[0][0]);
               if(ntype2 == Tmorse->water_code) {
                  ;
               }
               else {
                  WRITE(Tfilenumbers->nzno,FMT,F907,1,0);
                  Stop("bad modifier to SOLVENT command");
               }
               *bw = true;
            }
            else {
               WRITE(Tfilenumbers->nzno,FMT,F711,1,0);
               WRITE(Tfilenumbers->nzno,FMT,F712,1,0);
            }
         }
         else {
            WRITE(Tfilenumbers->nzno,FMT,F610,1,0);
            WRITE(Tfilenumbers->nzno,FMT,F611,1,STRG,command,80,0);
            Stop("unrecognized command");
         }
         if(*nelts > *maxelts) {
            WRITE(Tfilenumbers->nzno,FMT,F612,1,0);
            Stop("too many elements");
         }
         nextstring(&nobuffer,buffer,String,&atend,80,80);
      }
      if(*nelts == 0) {              /*  end of GRANDDADDY loop */
         WRITE(Tfilenumbers->nzno,FMT,F613,1,0);
         Stop("no elements loaded");
      }
      CLOSE(Tfilenumbers->nbod,0);
      return;
   }

   void modifiers(unsigned int *nobuffer,
                  char *buffer,
                  char *String,
                  unsigned int *atend,
                  char *down,
                  double *download,
                  char *dictionary,
                  int map[],
                  int *maxwords,
                  int *nwords,
                  int /* P2 */,
                  int /* P3 */,
                  int /* P1 */,
                  int /* P4 */)
   {
      typedef struct {
         int nbod;
         int nzno;
         int nznr;
         int nstk;
         int ndfl;
         int nefl;
         int nzh;
         int nih;
         int nsh;
         int nph;
      } Cfilenumbers;
      static Cfilenumbers *Tfilenumbers = (Cfilenumbers*) Xfilenumbers;
      static double w[2];
      static int i,ntype;
      /*
*************************************************
*/
      static const char* F400[] = {
         "('Unexpected end of body file')"
      };
      static const char* F620[] = {
         "('Trying to interpret this string as a number: ')"
      };
      static const char* F621[] = {
         "(a80)"
      };
      for(i=0; i<13; i++) {
         if(*(down+i) == '@') {
            return;
         }
         else if(*(down+i) == 'n') {
            nextstring(nobuffer,buffer,String,atend,80,80);
            if(*atend) {
               WRITE(Tfilenumbers->nzno,FMT,F400,1,0);
               Stop("unexpected end of body file");
            }
            testword(String,&ntype,dictionary,map,nwords,maxwords,80,10);
            *(download+i) = (double)ntype;
            *(download+i+12) = 0.0e0;
         }
         else if(*(down+i) == 'r') {
            nextstring(nobuffer,buffer,String,atend,80,80);
            if(*atend) {
               WRITE(Tfilenumbers->nzno,FMT,F400,1,0);
               Stop("unexpected end of body file");
            }
            if(READ(INTERNAL,String,80,IOSTAT,NULL,2,LISTIO,DO,1,REAL8,(download+i),0)) goto S999;
            *(download+i+12) = 0.0e0;
         }
         else if(*(down+i) == 'd') {
            nextstring(nobuffer,buffer,String,atend,80,80);
            if(*atend) {
               WRITE(Tfilenumbers->nzno,FMT,F400,1,0);
               Stop("unexpected end of body file");
            }
            floatstring(String,w,80);
            *(download+i) = w[0];
            *(download+i+12) = w[1];
         }
      }
      return;
   S999:
      WRITE(Tfilenumbers->nzno,FMT,F620,1,0);
      WRITE(Tfilenumbers->nzno,FMT,F621,1,STRG,String,80,0);
      Stop("number conversion error");
   }

   void maxpillar(
                  float x[],
                  float y[],
                  float zl[],
                  float zh[],
                  float *dd)
   {
      static float v[3];
      static int is,i;
      static float d;
      /*
*************************************************
*/
      /*
        Because pillars are convex, you just need to check six vertices
      */
      is = 0;
      for(i=0; i<3; i++) {
         v[0] = x[i];
         v[1] = y[i];
         v[2] = zl[i];
         pythag0(v,&d);
         if(is == 0) {
            is = 1;
            *dd = d;
         }
         else {
            *dd = fifamax1(*dd,d);
         }
         v[2] = zh[i];
         pythag0(v,&d);
         *dd = fifamax1(*dd,d);
      }
      return;
   }

   void propillar(
                  float x[],
                  float y[],
                  float zl[],
                  float zh[],
                  float v[],
                  float *s1,
                  float *s2)
   {
      static float p[3];
      static int is,i;
      static float dot;
      /*
*************************************************
*/
      is = 0;
      for(i=0; i<3; i++) {
         p[0] = x[i];
         p[1] = y[i];
         p[2] = zl[i];
         dotproduct(v,p,&dot);
         if(is == 0) {
            *s1 = dot;
            *s2 = dot;
            is = 1;
         }
         else {
            *s1 = fifamin1(*s1,dot);
            *s2 = fifamax1(*s2,dot);
         }
         p[2] = zh[i];
         dotproduct(v,p,&dot);
         *s1 = fifamin1(*s1,dot);
         *s2 = fifamax1(*s2,dot);
      }
      return;
   }

   void insidepillar(
                     float x[],
                     float y[],
                     float zl[],
                     float zh[],
                     float p[],
                     unsigned int *inside)
   {
      static float b1[2],b2[2],s[2],alpha[2],zoom,zupper,zlower;
      /*
*************************************************
*/
      /*
        Is the point [p(1),p(2)] inside the triangle [x(1),y(1)],
        [x(2),y(2)], [x(3),y(3)] ?
      */
      b1[0] = x[1]-x[0];
      b1[1] = y[1]-y[0];
      b2[0] = x[2]-x[0];
      b2[1] = y[2]-y[0];
      s[0] = p[0]-x[0];
      s[1] = p[1]-y[0];
      zoom = b1[0]*b2[1]-b2[0]*b1[1];
      alpha[0] = (s[0]*b2[1]-b2[0]*s[1])/zoom;
      alpha[1] = (b1[0]*s[1]-s[0]*b1[1])/zoom;
      if(alpha[0] < 0.0F) {
         *inside = false;
         return;
      }
      if(alpha[1] < 0.0F) {
         *inside = false;
         return;
      }
      if(alpha[0]+alpha[1] > 1.0F) {
         *inside = false;
         return;
      }
      /*
        If we have fallen out here, then we know that the image of the
        point falls inside the triangle.
      */
      zupper = zh[0]+alpha[0]*(zh[1]-zh[0])+alpha[1]*(zh[2]-zh[0]);
      zlower = zl[0]+alpha[0]*(zl[1]-zl[0])+alpha[1]*(zl[2]-zl[0]);
      *inside = zlower <= p[2] && p[2] <= zupper;
      return;
   }

   void minpillar(
                  float x[],
                  float y[],
                  float zl[],
                  float zh[],
                  float p[],
                  float *d)
   {
      static float v1[3],v2[3],v3[3],d1;
      /*
*************************************************
*/
      /*
        First triangle will be the bottom face
      */
      v1[0] = x[0];
      v1[1] = y[0];
      v1[2] = zl[0];
      v2[0] = x[1];
      v2[1] = y[1];
      v2[2] = zl[1];
      v3[0] = x[2];
      v3[1] = y[2];
      v3[2] = zl[2];
      mintriangle(v1,v2,v3,p,&d1);
      *d = d1;
      /*
        Second triangle will be the top face
      */
      v1[0] = x[0];
      v1[1] = y[0];
      v1[2] = zh[0];
      v2[0] = x[1];
      v2[1] = y[1];
      v2[2] = zh[1];
      v3[0] = x[2];
      v3[1] = y[2];
      v3[2] = zh[2];
      mintriangle(v1,v2,v3,p,&d1);
      *d = fifamin1(d1,*d);
      v1[0] = x[0];
      v1[1] = y[0];
      v1[2] = zl[0];
      v2[0] = x[1];
      v2[1] = y[1];
      v2[2] = zl[1];
      v3[0] = x[0];
      v3[1] = y[0];
      v3[2] = zh[0];
      mintriangle(v1,v2,v3,p,&d1);
      *d = fifamin1(d1,*d);
      v1[0] = x[0];
      v1[1] = y[0];
      v1[2] = zh[0];
      v2[0] = x[1];
      v2[1] = y[1];
      v2[2] = zh[1];
      v3[0] = x[1];
      v3[1] = y[1];
      v3[2] = zl[1];
      mintriangle(v1,v2,v3,p,&d1);
      *d = fifamin1(d1,*d);
      v1[0] = x[0];
      v1[1] = y[0];
      v1[2] = zl[0];
      v2[0] = x[2];
      v2[1] = y[2];
      v2[2] = zl[2];
      v3[0] = x[0];
      v3[1] = y[0];
      v3[2] = zh[0];
      mintriangle(v1,v2,v3,p,&d1);
      *d = fifamin1(d1,*d);
      v1[0] = x[0];
      v1[1] = y[0];
      v1[2] = zh[0];
      v2[0] = x[2];
      v2[1] = y[2];
      v2[2] = zh[2];
      v3[0] = x[2];
      v3[1] = y[2];
      v3[2] = zl[2];
      mintriangle(v1,v2,v3,p,&d1);
      *d = fifamin1(d1,*d);
      v1[0] = x[1];
      v1[1] = y[1];
      v1[2] = zl[1];
      v2[0] = x[2];
      v2[1] = y[2];
      v2[2] = zl[2];
      v3[0] = x[1];
      v3[1] = y[1];
      v3[2] = zh[1];
      mintriangle(v1,v2,v3,p,&d1);
      *d = fifamin1(d1,*d);
      v1[0] = x[1];
      v1[1] = y[1];
      v1[2] = zh[1];
      v2[0] = x[2];
      v2[1] = y[2];
      v2[2] = zh[2];
      v3[0] = x[2];
      v3[1] = y[2];
      v3[2] = zl[2];
      mintriangle(v1,v2,v3,p,&d1);
      *d = fifamin1(d1,*d);
      return;
   }

   void maxcube(
                float v1[],
                float *side,
                float *dd)
   {
      static float v2[3];
      static int ix,iy,iz;
      static float d;
      /*
*************************************************
*/
      /*
        Given a cube with low corner at v1 and high corner at v2,
        Return in dd the maximum distance between all points on the surface of
        the cube and the origin.
      */
      for(ix=0; ix<=1; ix++) {
         for(iy=0; iy<=1; iy++) {
            for(iz=0; iz<=1; iz++) {
               v2[0] = v1[0]+(float)ix**side;
               v2[1] = v1[1]+(float)iy**side;
               v2[2] = v1[2]+(float)iz**side;
               pythag0(v2,&d);
               if(ix+iy+iz == 0) {
                  *dd = d;
               }
               else {
                  *dd = fifamax1(*dd,d);
               }
            }
         }
      }
      return;
   }

   void maxsphere(
                  float c[],
                  float *r,
                  float *dd)
   {
      /*
        Given a sphere with center at an arbitrary point c and with radius r.
        Return in dd the maximum distance between all points on the surface of
        the sphere and the origin.
      */
      static float d1;
      /*
*************************************************
*/
      pythag0(c,&d1);
      *dd = d1+*r;
      return;
   }

   void procube(
                float v1[],
                float *side,
                float v[],
                float *s1,
                float *s2)
   {
      static float vt[3];
      static int it,ix,iy,iz;
      static float dot;
      /*
*************************************************
*/
      /*
        Given a cube with lower corner at v1 and side = side,
        also given an arbitrary direction v, determine the distance
        between the two tangents
      */
      it = 0;
      for(ix=0; ix<=1; ix++) {
         for(iy=0; iy<=1; iy++) {
            for(iz=0; iz<=1; iz++) {
               vt[0] = v1[0]+(float)ix**side;
               vt[1] = v1[1]+(float)iy**side;
               vt[2] = v1[2]+(float)iz**side;
               dotproduct(v,vt,&dot);
               if(it == 0) {
                  *s1 = dot;
                  *s2 = dot;
                  it = 1;
               }
               else {
                  *s1 = fifamin1(*s1,dot);
                  *s2 = fifamax1(*s2,dot);
               }
            }
         }
      }
      return;
   }

   void mincube(
                float v1[],
                float *side,
                float p[],
                float *d)
   {
      typedef struct {
         int ncube;
         float ferr;
      } Ccubit;
      static Ccubit *Tcubit = (Ccubit*) Xcubit;
      /*
        Given a cube with low corner at v1 and with length of side = side,
        also given an arbitrary point p outside the cube
        Return in d the minimum distance from p to the cube
      */
      static float c[3],pr[3],s2;
      static int i,nwk;
      static float ss;
      /*
*************************************************
*/
      s2 = *side/2.0F;
      for(i=0; i<3; i++) {
         c[i] = v1[i]+s2;
      }
      for(i=0; i<3; i++) {
         pr[i] = p[i]-c[i];
      }
      for(i=0; i<3; i++) {
         pr[i] = fifabs(pr[i]);
      }
      nwk = 1;
      while(nwk < 3) {
         if(pr[nwk-1] <= pr[nwk]) {
            nwk = nwk+1;
         }
         else {
            ss = pr[nwk-1];
            pr[nwk-1] = pr[nwk];
            pr[nwk] = ss;
            nwk = nwk-1;
         }
         if(nwk == 0) nwk = 1;
      }
      if(pr[2] < s2) {
         if(Tcubit->ncube == 0) {
            Tcubit->ncube = 1;
            Tcubit->ferr = pr[2]/s2;
         }
         else {
            Tcubit->ferr = fifamin1(Tcubit->ferr,pr[2]/s2);
         }
         *d = 0.0F;
         return;
      }
      if(pr[1] < s2) {
         *d = pr[2]-s2;
         return;
      }
      if(pr[0] < s2) {
         *d = fifpow(pr[2]-s2,2.0F)+fifpow(pr[1]-s2,2.0F);
         *d = fifsqrt(*d);
         return;
      }
      *d = fifpow(pr[0]-s2,2.0F)+fifpow(pr[1]-s2,2.0F)+fifpow(pr[2]-s2,2.0F);
      *d = fifsqrt(*d);
      return;
   }

   void prosphere(
                  float c[],
                  float *r,
                  float v[],
                  float *s1,
                  float *s2)
   {
      static float dot;
      /*
*************************************************
*/
      /*
        calculate the projection span of a sphere onto an
        arbitary line
      */
      dotproduct(c,v,&dot);
      *s1 = dot-*r;
      *s2 = dot+*r;
      return;
   }

   void minsphere(
                  float c[],
                  float *r,
                  float p[],
                  float *d)
   {
      /*
        Given a sphere with center at an arbitrary point c and with radius r.
        Also given an arbitrary point p.
        Return in d the minimum distance between all points on the sphere
        and the point p.
      */
      static float r1;
      /*
*************************************************
*/
      pythag(c,p,&r1);
      *d = fifabs(r1-*r);
      return;
   }

   void insidecube(
                   float v1[],
                   float *side,
                   float p[],
                   unsigned int *inside)
   {
      /*
        Given a cube with low corner at v1 and with length of side = side,
        also given an arbitrary point p
        report in "inside" whether or not p is inside the cube
      */
      static float c[3],pr[3],s2;
      static int i,nwk;
      static float ss;
      /*
*************************************************
*/
      s2 = *side/2.0F;
      for(i=0; i<3; i++) {
         c[i] = v1[i]+s2;
      }
      for(i=0; i<3; i++) {
         pr[i] = p[i]-c[i];
      }
      for(i=0; i<3; i++) {
         pr[i] = fifabs(pr[i]);
      }
      nwk = 1;
      while(nwk < 3) {
         if(pr[nwk-1] <= pr[nwk]) {
            nwk = nwk+1;
         }
         else {
            ss = pr[nwk-1];
            pr[nwk-1] = pr[nwk];
            pr[nwk] = ss;
            nwk = nwk-1;
         }
         if(nwk == 0) nwk = 1;
      }
      *inside = pr[2] < s2;
      return;
   }

   void insidesphere(
                     float c[],
                     float *r,
                     float p[],
                     unsigned int *inside)
   {
      /*
        Given a sphere with center at an arbitrary point c and with radius r.
        Also given an arbitrary point p.
        return inside=.true. if p is inside the sphere.
      */
      static float r1;
      /*
*************************************************
*/
      pythag(c,p,&r1);
      *inside = r1 < *r;
      return;
   }

   void maxtriangle(
                    float v1[],
                    float v2[],
                    float v3[],
                    float *dd)
   {
      /*
        Given a triangle with vertices at v1, v2, v3.
        Return in dd the maximum distance between all points on the surface
        of the triangle and the origin.
        Assumption:  One of the three vertices is at the maximum.
      */
      static float d1,d2,d3,dd1;
      /*
*************************************************
*/
      pythag0(v1,&d1);
      pythag0(v2,&d2);
      pythag0(v3,&d3);
      dd1 = fifamax1(d1,d2);
      *dd = fifamax1(dd1,d3);
      return;
   }

   void protriangle(
                    float v1[],
                    float v2[],
                    float v3[],
                    float v[],
                    float *s1,
                    float *s2)
   {
      static float dot;
      /*
*************************************************
*/
      /*
        calculate the projection span of a triangle onto an
        arbitary line
      */
      dotproduct(v1,v,&dot);
      *s1 = dot;
      *s2 = dot;
      dotproduct(v2,v,&dot);
      *s1 = fifamin1(*s1,dot);
      *s2 = fifamax1(*s2,dot);
      dotproduct(v3,v,&dot);
      *s1 = fifamin1(*s1,dot);
      *s2 = fifamax1(*s2,dot);
      return;
   }

   void mintriangle(
                    float v1[],
                    float v2[],
                    float v3[],
                    float p[],
                    float *d)
   {
      /*
        Given a triangle with vertices at v1, v2, v3.
        Also given an arbitrary point p.
        Return in d the minimum distance between all points on the
        surface of the triangle and the point p.
      */
      static float b2[3],b1[3],b3[3],p1[3],t3b3[3],q[3],dp1b3,db3b3,t3,d11,d12,d22,q1,q2,t1,t2,d1,d2,d3,dx;
      /*
*************************************************
*/
      vector_difference(v2,v1,b1);
      vector_difference(v3,v1,b2);
      cross_product(b1,b2,b3);
      vector_difference(p,v1,p1);
      dotproduct(p1,b3,&dp1b3);
      dotproduct(b3,b3,&db3b3);
      t3 = dp1b3/db3b3;
      scalar_product(&t3,b3,t3b3);
      vector_difference(p1,t3b3,q);
      dotproduct(b1,b1,&d11);
      dotproduct(b1,b2,&d12);
      dotproduct(b2,b2,&d22);
      dotproduct(q,b1,&q1);
      dotproduct(q,b2,&q2);
      t1 = (q1*d22-q2*d12)/(d11*d22-d12*d12);
      t2 = (d11*q2-q1*d12)/(d11*d22-d12*d12);
      if(t1 >= 0.0F && t2 >= 0.0F && t1+t2 <= 1.0F) {
         pythag0(t3b3,d);
      }
      else {
         minedge(v1,v2,p,&d1);
         minedge(v1,v3,p,&d2);
         minedge(v2,v3,p,&d3);
         dx = fifamin1(d1,d2);
         *d = fifamin1(dx,d3);
      }
      return;
   }

   void maxdisk(
                float c[],
                float [] /* n */,
                float *r,
                float *t,
                float *dd)
      /*
        Given a disk centered at c, with unit normal n and radius r.
        This returns the maximum distance of points on the disk away
        from the origin.
      */
   {
      static float T1;
      static float oi[3],q[3],qq;
      /*
*************************************************
*/
      T1 = -1.0F;
      scalar_product(&T1,c,oi);
      rotate(q,t,oi);
      qq = fifsqrt(fifpow(q[0],2.0F)+fifpow(q[1],2.0F));
      *dd = fifpow(q[2],2.0F)+fifpow(qq+*r,2.0F);
      *dd = fifsqrt(*dd);
      return;
   }

   void prodisk(
                float c[],
                float [] /* n[] */,
                float *r,
                float *tr,
                float v[],
                float *s1,
                float *s2)
   {
      static float b[3],x1[3],x2[3],r1,dot1,dot2;
      /*
*************************************************
*/
      /*
        calculate the projection span of a disk onto an
        arbitary line
      */
      rotate(b,tr,v);
      r1 = fifsqrt(fifpow(b[0],2.0F)+fifpow(b[1],2.0F));
      if(r1 == 0.0F) {
         x1[0] = *r;
         x1[1] = 0.0F;
         x1[2] = 0.0F;
      }
      else {
         x1[0] = *r*b[0]/r1;
         x1[1] = *r*b[1]/r1;
         x1[2] = 0.0F;
      }
      x2[0] = -x1[0];
      x2[1] = -x1[1];
      x2[2] = 0.0F;
      backtransform(c,tr,x1);
      backtransform(c,tr,x2);
      dotproduct(x1,v,&dot1);
      dotproduct(x2,v,&dot2);
      *s2 = fifamax1(dot1,dot2);
      *s1 = fifamin1(dot1,dot2);
      return;
   }

   void mindisk(
                float c[],
                float n[],
                float *r,
                float * /* tr */,
                float p[],
                float *d)
   {
      /*
        Given a disk centered at c, with unit normal n and radius r.
        Also given an arbitrary point p.
        Return in d the minimum distance between the point p and the disk.
      */
      static float pmc[3],ta[3],nn[3],t,quid;
      /*
*************************************************
*/
      vector_difference(p,c,pmc);
      dotproduct(pmc,n,&t);
      scalar_product(&t,n,ta);
      vector_difference(pmc,ta,nn);
      pythag0(nn,&quid);
      if(quid <= *r) {
         *d = fifabs(t);
      }
      else {
         *d = fifpow(t,2.0F)+fifpow(quid-*r,2.0F);
         *d = fifsqrt(*d);
      }
      return;
   }

   void maxsolcyl(
                  float c[],
                  float /* n */ [],
                  float *r,
                  float *l,
                  float *t,
                  float *dd)
   {
      static float T1;
      /*
        Given a solid cylinder centered at c, with unit normal n, radius r,
        length l.
        This returns the maximum distance of points on the cylinder
        away from the origin.
      */
      static float oi[3],q[3],rr,zz,dd1,dd2;
      /*
*************************************************
*/
      T1 = -1.0F;
      scalar_product(&T1,c,oi);
      rotate(q,t,oi);
      rr = fifsqrt(fifpow(q[0],2.0F)+fifpow(q[1],2.0F));
      zz = q[2];
      dd1 = fifsqrt(fifpow(rr+*r,2.0F)+fifpow(zz-*l/2.0F,2.0F));
      dd2 = fifsqrt(fifpow(rr+*r,2.0F)+fifpow(zz+*l/2.0F,2.0F));
      *dd = fifamax1(dd1,dd2);
      return;
   }

   void maxcylinder(
                    float c[],
                    float /* n */ [],
                    float *r,
                    float *l,
                    float *t,
                    float *dd)
   {
      static float T1;
      /*
        Given a cylinder centered at c, with unit normal n, radius r,
        length l.
        This returns the maximum distance of points on the cylinder
        away from the origin.
      */
      static float oi[3],q[3],rr,zz,dd1,dd2;
      /*
*************************************************
*/
      T1 = -1.0F;
      scalar_product(&T1,c,oi);
      rotate(q,t,oi);
      rr = fifsqrt(fifpow(q[0],2.0F)+fifpow(q[1],2.0F));
      zz = q[2];
      dd1 = fifsqrt(fifpow(rr+*r,2.0F)+fifpow(zz-*l/2.0F,2.0F));
      dd2 = fifsqrt(fifpow(rr+*r,2.0F)+fifpow(zz+*l/2.0F,2.0F));
      *dd = fifamax1(dd1,dd2);
      return;
   }

   void prosolcyl(
                  float c[],
                  float n[],
                  float *r,
                  float *l,
                  float *t,
                  float v[],
                  float *s1,
                  float *s2)
   {
      static float c1[3],c2[3];
      static int i;
      static float u1,u2,z1,z2;
      /*
*************************************************
*/
      /*
        calculate the projection span of a solid cylinder onto an
        arbitary line
      */
      for(i=0; i<3; i++) {
         c1[i] = c[i]+n[i]**l/2.0F;
         c2[i] = c[i]-n[i]**l/2.0F;
      }
      prodisk(c1,n,r,t,v,&u1,&u2);
      prodisk(c2,n,r,t,v,&z1,&z2);
      *s1 = fifamin1(u1,z1);
      *s2 = fifamax1(u2,z2);
      return;
   }

   void procylinder(
                    float c[],
                    float n[],
                    float *r,
                    float *l,
                    float *t,
                    float v[],
                    float *s1,
                    float *s2)
   {
      static float c1[3],c2[3];
      static int i;
      static float u1,u2,z1,z2;
      /*
*************************************************
*/
      /*
        calculate the projection span of a cylinder onto an
        arbitary line
      */
      for(i=0; i<3; i++) {
         c1[i] = c[i]+n[i]**l/2.0F;
         c2[i] = c[i]-n[i]**l/2.0F;
      }
      prodisk(c1,n,r,t,v,&u1,&u2);
      prodisk(c2,n,r,t,v,&z1,&z2);
      *s1 = fifamin1(u1,z1);
      *s2 = fifamax1(u2,z2);
      return;
   }

   void minsolcyl(
                  float c[],
                  float /* n */ [],
                  float *r,
                  float *l,
                  float *t,
                  float p[],
                  float *d)
      /*
        Given a solid cylinder centered at c, with unit normal n, radius r,
        length l.
        This returns the minimum distance between points on the cylinder
        and the arbitrary point p.
      */
   {
      static float pmc[3],q[3],ro,dp,zz;
      /*
*************************************************
*/
      vector_difference(p,c,pmc);
      rotate(q,t,pmc);
      ro = fifsqrt(fifpow(q[0],2.0F)+fifpow(q[1],2.0F));
      dp = fifabs(ro-*r);
      if(fifabs(q[2]) > *l/2.0F) {
         if(ro <= *r) {
            *d = fifabs(q[2])-*l/2.0F;
         }
         else {
            zz = fifabs(q[2])-*l/2.0F;
            *d = fifsqrt(fifpow(zz,2.0F)+fifpow(dp,2.0F));
         }
      }
      else {
         *d = dp;
      }
      return;
   }

   void insidesolcyl(
                     float c[],
                     float /* n */ [],
                     float *r,
                     float *l,
                     float *t,
                     float p[],
                     unsigned int *inside)
      /*
        Given a solid cylinder centered at c, with unit normal n, radius r,
        length l.
        This returns the minimum distance between points on the cylinder
        and the arbitrary point p.
      */
   {
      static float pmc[3],q[3],ro;
      // static float dp;
      /*
*************************************************
*/
      vector_difference(p,c,pmc);
      rotate(q,t,pmc);
      ro = fifsqrt(fifpow(q[0],2.0F)+fifpow(q[1],2.0F));
      // dp = fifabs(ro-*r);
      *inside = false;
      if(fifabs(q[2]) < *l/2.0F) {
         if(ro <= *r) {
            *inside = true;
         }
      }
      return;
   }

   void mincylinder(
                    float c[],
                    float /* n */ [],
                    float *r,
                    float *l,
                    float *t,
                    float p[],
                    float *d)
      /*
        Given a cylinder centered at c, with unit normal n, radius r,
        length l.
        This returns the minimum distance between points on the cylinder
        and the arbitrary point p.
      */
   {
      static float pmc[3],q[3],ro,dp,zz;
      /*
*************************************************
*/
      vector_difference(p,c,pmc);
      rotate(q,t,pmc);
      ro = fifsqrt(fifpow(q[0],2.0F)+fifpow(q[1],2.0F));
      dp = fifabs(ro-*r);
      if(fifabs(q[2]) > *l/2.0F) {
         zz = fifabs(q[2])-*l/2.0F;
         *d = fifsqrt(fifpow(zz,2.0F)+fifpow(dp,2.0F));
      }
      else {
         *d = dp;
      }
      return;
   }
   void maxtorus(
                 float c[],
                 float /* n */ [],
                 float *r1,
                 float *r2,
                 float *t,
                 float *dd)
   {
      static float T1;
      /*
        TORUS[(cx,cy,cz),(nx,ny,nz),r1,r2]
        Given a torus centered at c, with unit normal n, radii r1 and r2.
        This returns the maximum distance of points on the torus away
        from the origin.
        x                         x
        x       x                 x       x
        x         x               x         x   ___
        x         x               x         x    |
        x       x                 x       x     |  r2
        x                         x        ---
        |---------2 * r1 ---------|
      */
      static float oi[3],q[3],qq,sig,dod,x1,x2,y1,y2,d1,d2,d3,d4;
      /*
*************************************************
*/
      T1 = -1.0F;
      scalar_product(&T1,c,oi);
      rotate(q,t,oi);
      qq = fifsqrt(fifpow(q[0],2.0F)+fifpow(q[1],2.0F));
      sig = q[2]/(qq-*r1);
      dod = fifsqrt(fifpow(*r2,2.0F)/(1.0F+fifpow(sig,2.0F)));
      x1 = *r1+dod;
      x2 = *r1-dod;
      y1 = sig*dod;
      y2 = -(sig*dod);
      d1 = fifpow(qq-x1,2.0F)+fifpow(q[2]-y1,2.0F);
      d2 = fifpow(qq-x2,2.0F)+fifpow(q[2]-y2,2.0F);
      sig = q[2]/(qq+*r1);
      dod = fifsqrt(fifpow(*r2,2.0F)/(1.0F+fifpow(sig,2.0F)));
      x1 = -*r1+dod;
      x2 = -*r1-dod;
      y1 = sig*dod;
      y2 = -(sig*dod);
      d3 = fifpow(qq-x1,2.0F)+fifpow(q[2]-y1,2.0F);
      d4 = fifpow(qq-x2,2.0F)+fifpow(q[2]-y2,2.0F);
      *dd = fifamax1(d1,d2);
      *dd = fifamax1(*dd,d3);
      *dd = fifamax1(*dd,d4);
      *dd = fifsqrt(*dd);
      return;
   }

   void protorus(
                 float c[],
                 float /* n */ [],
                 float *r1,
                 float *r2,
                 float *tr,
                 float v[],
                 float *s1,
                 float *s2)
   {
      static float b[3],x1[3],x2[3],ct,st,cp,sp,dot1,dot2;
      /*
*************************************************
*/
      /*
        calculate the projection span of a torus onto an
        arbitary line
      */
      rotate(b,tr,v);
      ct = b[2];
      st = fifsqrt(fifpow(b[0],2.0F)+fifpow(b[1],2.0F));
      if(st == 0.0F) {
         x1[0] = 0.0F;
         x1[1] = 0.0F;
         x1[2] = *r2;
      }
      else {
         cp = b[0]/st;
         sp = b[1]/st;
         x1[0] = (*r1+*r2*st)*cp;
         x1[1] = (*r1+*r2*st)*sp;
         x1[2] = *r2*ct;
      }
      x2[0] = -x1[0];
      x2[1] = -x1[1];
      x2[2] = -x1[2];
      backtransform(c,tr,x1);
      backtransform(c,tr,x2);
      dotproduct(x1,v,&dot1);
      dotproduct(x2,v,&dot2);
      *s2 = fifamax1(dot1,dot2);
      *s1 = fifamin1(dot1,dot2);
      return;
   }

   void mintorus(
                 float c[],
                 float /* n */ [],
                 float *r1,
                 float *r2,
                 float *t,
                 float p[],
                 float *d)
   {
      /*
        TORUS[(cx,cy,cz),(nx,ny,nz),r1,r2]
        Given a torus centered at c, with unit normal n, radii r1 and r2.
        This returns the minimum distance of points on the torus away
        from an arbitrary point p.
        x                         x
        x       x                 x       x
        x         x               x         x   ___
        x         x               x         x    |
        x       x                 x       x     |  r2
        x                         x        ---
        |--------2 * r1 ----------|
      */
      static float pmc[3],q[3],qq,sig,dod,x1,x2,y1,y2,d1,d2,d3,d4;
      /*
*************************************************
*/
      vector_difference(p,c,pmc);
      rotate(q,t,pmc);
      qq = fifsqrt(fifpow(q[0],2.0F)+fifpow(q[1],2.0F));
      sig = q[2]/(qq-*r1);
      dod = fifsqrt(fifpow(*r2,2.0F)/(1.0F+fifpow(sig,2.0F)));
      x1 = *r1+dod;
      x2 = *r1-dod;
      y1 = sig*dod;
      y2 = -(sig*dod);
      d1 = fifpow(qq-x1,2.0F)+fifpow(q[2]-y1,2.0F);
      d2 = fifpow(qq-x2,2.0F)+fifpow(q[2]-y2,2.0F);
      sig = q[2]/(qq+*r1);
      dod = fifsqrt(fifpow(*r2,2.0F)/(1.0F+fifpow(sig,2.0F)));
      x1 = -*r1+dod;
      x2 = -*r1-dod;
      y1 = sig*dod;
      y2 = -(sig*dod);
      d3 = fifpow(qq-x1,2.0F)+fifpow(q[2]-y1,2.0F);
      d4 = fifpow(qq-x2,2.0F)+fifpow(q[2]-y2,2.0F);
      *d = fifamin1(d1,d2);
      *d = fifamin1(*d,d3);
      *d = fifamin1(*d,d4);
      *d = fifsqrt(*d);
      return;
   }

   void insidetorus(
                    float c[],
                    float /* n */ [],
                    float *r1,
                    float *r2,
                    float *t,
                    float p[],
                    unsigned int *inside)
   {
      /*
        TORUS[(cx,cy,cz),(nx,ny,nz),r1,r2]
        Given a torus centered at c, with unit normal n, radii r1 and r2.
        This returns inside as true if p is insde the torus.
        x                         x
        x       x                 x       x
        x         x               x         x   ___
        x         x               x         x    |
        x       x                 x       x     |  r2
        x                         x        ---
        |--------2 * r1 ----------|
      */
      static float pmc[3],q[3],qq,d1;
      /*
*************************************************
*/
      vector_difference(p,c,pmc);
      rotate(q,t,pmc);
      qq = fifsqrt(fifpow(q[0],2.0F)+fifpow(q[1],2.0F));
      d1 = fifpow(qq-*r1,2.0F)+fifpow(q[2],2.0F);
      d1 = fifsqrt(d1);
      if(d1 < *r2) {
         *inside = true;
         return;
      }
      d1 = fifpow(qq+*r1,2.0F)+fifpow(q[2],2.0F);
      d1 = fifsqrt(d1);
      if(d1 < *r2) {
         *inside = true;
         return;
      }
      *inside = false;
      return;
   }

   void maxellipsoid(
                     float c[],
                     float n1[],
                     float n2[],
                     float *aa,
                     float *bb,
                     float *cc,
                     float *t,
                     float *dd)
   {
      static float p[3];
      static int minmax;
      /*
*************************************************
*/
      /*
        return in dd the maximum distance of points in this
        ellipsoid away from the origin
      */
      p[0] = 0.0F;
      p[1] = 0.0F;
      p[2] = 0.0F;
      minmax = 1;
      exellipsoid(c,n1,n2,aa,bb,cc,t,p,&minmax,dd);
      /*
        dd always comes out slightly low, so lets do this
        to compensate.  It only means we are using a
        slightly larger launch sphere.
      */
      *dd = *dd*1.001F;
      return;
   }

   void proellipsoid(
                     float c[],
                     float /* n1 */ [],
                     float /* n2 */ [],
                     float *aa,
                     float *bb,
                     float *cc,
                     float *t,
                     float v[],
                     float *s1,
                     float *s2)
   {
      static float vi[3],rp[3];
      static int i,j;
      static float alam,zc;
      /*
*************************************************
*/
      /*
        calculate the projection span of a ellipsoid onto an
        arbitary line
        Upon entry, v is an arbitrary unit vector, upon which we are
        projecting the object
        Upon entry, c is the center of the ellipsoid
        t is the a rotation matrix required to rotate n1 to x-axis,
        n2 to y-axis, n3 to z-axis
      */
      for(i=0; i<3; i++) {
         vi[i] = 0.0F;
         for(j=0; j<3; j++) {
            vi[i] = vi[i]+*(t+i+j*3)*v[j];
         }
      }
      alam = fifsqrt(fifpow(vi[0]**aa,2.0F)+fifpow(vi[1]**bb,2.0F)+fifpow(vi[2]**cc,2.0F));
      alam = 1.0F/alam;
      rp[0] = alam*vi[0]**aa**aa;
      rp[1] = alam*vi[1]**bb**bb;
      rp[2] = alam*vi[2]**cc**cc;
      *s2 = rp[0]*vi[0]+rp[1]*vi[1]+rp[2]*vi[2];
      *s1 = -*s2;
      zc = c[0]*v[0]+c[1]*v[1]+c[2]*v[2];
      *s1 = *s1+zc;
      *s2 = *s2+zc;
      return;
   }

   void minellipsoid(
                     float c[],
                     float n1[],
                     float n2[],
                     float *aa,
                     float *bb,
                     float *cc,
                     float *t,
                     float p[],
                     float *d)
   {
      static int minmax;
      /*
*************************************************
*/
      /*
        return in d the minimum distance between the ellipsoid and
        the point p
      */
      minmax = -1;
      exellipsoid(c,n1,n2,aa,bb,cc,t,p,&minmax,d);
      return;
   }

   void insideellipsoid(
                        float c[],
                        float /* n1 */ [],
                        float /* n2 */ [],
                        float *aa,
                        float *bb,
                        float *cc,
                        float *t,
                        float po[],
                        unsigned int *inside)
   {
      /*
        return inside=.true. if point p is on the interior of the
        ellipsoid
      */
      static float pomc[3],p[3],zz;
      /*
*************************************************
*/
      vector_difference(po,c,pomc);
      rotate(p,t,pomc);
      zz = fifpow(p[0]/ *aa,2.0F)+fifpow(p[1]/ *bb,2.0F)+fifpow(p[2]/ *cc,2.0F);
      *inside = zz < 1.0F;
      return;
   }

   void exellipsoid(
                    float c[],
                    float /* n1 */ [],
                    float /* n2 */ [],
                    float *aa,
                    float *bb,
                    float *cc,
                    float *t,
                    float po[],
                    int *minmax,
                    float *d)
   {
      static float T1,T2;
      static float pomc[3],a[3],a2s[3],p[3],x[3],x1[3],x2[3],xsol[3][6];
      static int i,nsol,ndif;
      static float alam,alam1,alam2;
      static int ns;
      static float dd,ss;
      static int j;
      /*
*************************************************
*/
      vector_difference(po,c,pomc);
      rotate(p,t,pomc);
      a[0] = *aa;
      a[1] = *bb;
      a[2] = *cc;
      /*
        The determination of extremal points on the surface of
        an ellipsoid is difficult when any component of p is
        close to zero, because the problem becomes degenerate.
        We deal with the problem here by making any very small
        coordinates not so small.  The justification for this
        adjustment is different in each of two different
        cases:  1.  All three coordinates near zero, and 2. Only
        one or two coordinates near zero.
        When all three coordinates are near zero, we change
        them to a small positive value epsilon.  This increases
        the maximum distance by an amount of order
        epsilon.  We are obviously at a
        point inside the ellipsoid, and so we are attempting
        to set the radius of the launch sphere.  Therefore,
        the only effect of this manipulation is to make
        the launch sphere slightly larger.
        If we are not on the interior of the ellipsoid trying
        to set the launch sphere radius, then we must be on
        the exterior and trying to compute the minimum distance
        to the surface from an exterior point. Therefore,
        the second case above only occurs when we are
        on the outside looking in.  Then, if
        we change only one or two zero coordinates by a small
        amount epsilon,
        the effect on the distance measurement is of order
        epsilon**2 (it's like we are rotating a lever arm)
        and therefore negligible.
      */
      for(i=0; i<3; i++) {
         if(fifabs(p[i])/a[i] < 1.0e-6F) p[i] = 1.0e-4F*a[i];
      }
      nsol = 0;
      sort(a,a2s,&ndif);
      /*
        a2s contains squared and sorted a's:
        ndif = 3:  a2s(1) < a2s(2) < a2s(3)
        ndif = 2:  a2s(1) < a2s(2) and a2s(3) is irrelevant
        ndif = 1:  sphere, a2s(1) is only relevant entry
      */
      if(ndif == 3) {
         T1 = -a2s[0];
         T2 = 1.0F;
         above(a,p,&alam,x,&T1,&T2);
         nsol = nsol+1;
         xsol[0][nsol-1] = x[0];
         xsol[1][nsol-1] = x[1];
         xsol[2][nsol-1] = x[2];
         T1 = -a2s[1];
         T2 = -a2s[0];
         between(a,p,&alam1,&alam2,x1,x2,&T1,&T2,&ns);
         if(ns == 2) {
            xsol[0][nsol] = x1[0];
            xsol[1][nsol] = x1[1];
            xsol[2][nsol] = x1[2];
            xsol[0][nsol+1] = x2[0];
            xsol[1][nsol+1] = x2[1];
            xsol[2][nsol+1] = x2[2];
            nsol = nsol+2;
         }
         T1 = -a2s[2];
         T2 = -a2s[1];
         between(a,p,&alam1,&alam2,x1,x2,&T1,&T2,&ns);
         if(ns == 2) {
            xsol[0][nsol] = x1[0];
            xsol[1][nsol] = x1[1];
            xsol[2][nsol] = x1[2];
            xsol[0][nsol+1] = x2[0];
            xsol[1][nsol+1] = x2[1];
            xsol[2][nsol+1] = x2[2];
            nsol = nsol+2;
         }
         T1 = -a2s[2];
         T2 = -1.0F;
         above(a,p,&alam,x,&T1,&T2);
         nsol = nsol+1;
         xsol[0][nsol-1] = x[0];
         xsol[1][nsol-1] = x[1];
         xsol[2][nsol-1] = x[2];
      }
      else if(ndif == 2) {
         T1 = -a2s[0];
         T2 = 1.0F;
         above(a,p,&alam,x,&T1,&T2);
         nsol = nsol+1;
         xsol[0][nsol-1] = x[0];
         xsol[1][nsol-1] = x[1];
         xsol[2][nsol-1] = x[2];
         T1 = -a2s[1];
         T2 = -a2s[0];
         between(a,p,&alam1,&alam2,x1,x2,&T1,&T2,&ns);
         if(ns == 2) {
            xsol[0][nsol] = x1[0];
            xsol[1][nsol] = x1[1];
            xsol[2][nsol] = x1[2];
            xsol[0][nsol+1] = x2[0];
            xsol[1][nsol+1] = x2[1];
            xsol[2][nsol+1] = x2[2];
            nsol = nsol+2;
         }
         T1 = -a2s[1];
         T2 = -1.0F;
         above(a,p,&alam,x,&T1,&T2);
         nsol = nsol+1;
         xsol[0][nsol-1] = x[0];
         xsol[1][nsol-1] = x[1];
         xsol[2][nsol-1] = x[2];
      }
      else {
         T1 = -a2s[0];
         T2 = 1.0F;
         above(a,p,&alam,x,&T1,&T2);
         nsol = nsol+1;
         xsol[0][nsol-1] = x[0];
         xsol[1][nsol-1] = x[1];
         xsol[2][nsol-1] = x[2];
         T1 = -a2s[0];
         T2 = -1.0F;
         above(a,p,&alam,x,&T1,&T2);
         nsol = nsol+1;
         xsol[0][nsol-1] = x[0];
         xsol[1][nsol-1] = x[1];
         xsol[2][nsol-1] = x[2];
      }
      for(i=1; i<=nsol; i++) {
         dd = 0.0F;
         ss = 0.0F;
         for(j=0; j<3; j++) {
            dd = dd+fifpow(p[j]-xsol[j][i-1],2.0F);
            ss = ss+fifpow(xsol[j][i-1]/a[j],2.0F);
         }
         dd = fifsqrt(dd);
         if(i == 1) {
            *d = dd;
         }
         else {
            if(*minmax == -1) {
               *d = fifamin1(dd,*d);
            }
            else {
               *d = fifamax1(dd,*d);
            }
         }
      }
      return;
   }

   void sort(
             float a[],
             float a2s[],
             int *ndif)
   {
      static int i,nwk;
      static float z;
      /*
*************************************************
*/
      for(i=0; i<3; i++) {
         a2s[i] = fifpow(a[i],2.0F);
      }
      *ndif = 3;
      nwk = 1;
      while(nwk < *ndif) {
         if(nwk == 0) nwk = 1;
         if(a2s[nwk-1] < a2s[nwk]) {
            nwk = nwk+1;
         }
         else if(a2s[nwk-1] > a2s[nwk]) {
            z = a2s[nwk-1];
            a2s[nwk-1] = a2s[nwk];
            a2s[nwk] = z;
            nwk = nwk-1;
         }
         else {
            a2s[nwk] = a2s[*ndif-1];
            *ndif = *ndif-1;
         }
      }
      return;
   }

   void above(
              float a[],
              float p[],
              float *alam,
              float x[],
              float *sing,
              float *ccc)
   {
      static float add,z,t,zhih,ztry,zlow,zz;
      /*
*************************************************
*/
      add = 1.0F;
   S1:
      z = *sing+add**ccc;
      eval(&z,a,p,x,&t);
      if(t < 1.0F) {
         zhih = z;
         goto S2;
      }
      add = add*1.5F;
      goto S1;
   S2:
      ztry = zhih;
   S3:
      ztry = (ztry+*sing)/2.0F;
      eval(&ztry,a,p,x,&t);
      if(t > 1.0F) {
         zlow = ztry;
         goto S4;
      }
      goto S3;
   S4:
      if(*ccc < 0.0F) {
         zz = zlow;
         zlow = zhih;
         zhih = zz;
      }
      converge(&zlow,&zhih,a,p,x,alam);
      return;
   }

   void between(
                float a[],
                float p[],
                float *alam1,
                float *alam2,
                float x1[],
                float x2[],
                float *sing1,
                float *sing2,
                int *ns)
   {
      static float x[3],z,t,zlow;
      static int k1;
      static float zhih,ztest,zmid,zc,z1;
      /*
*************************************************
*/
      z = (*sing1+*sing2)/2.0F;
      eval2(&z,a,p,x,&t);
      if(t > 0.0F) {
         zlow = z;
         k1 = 1;
      }
      else {
         zhih = z;
         k1 = 2;
      }
      if(k1 == 1) {
      S1:
         ztest = (zlow+*sing2)/2.0F;
         eval2(&ztest,a,p,x,&t);
         if(t < 0.0F) {
            zhih = ztest;
            goto S10;
         }
         zlow = ztest;
         goto S1;
      }
      else {
      S2:
         ztest = (zhih+*sing1)/2.0F;
         eval2(&ztest,a,p,x,&t);
         if(t > 0.0F) {
            zlow = ztest;
            goto S10;
         }
         zhih = ztest;
         goto S2;
      }
   S10:
      while((zhih-zlow)*2.0F/(fifabs(zhih)+fifabs(zlow)) > 1.0e-5F) {
         zmid = (zhih+zlow)/2.0F;
         eval2(&zmid,a,p,x,&t);
         if(t < 0.0F) {
            zhih = zmid;
         }
         else {
            zlow = zmid;
         }
      }
      zc = 0.5F*(zhih+zlow);
      eval(&zc,a,p,x,&t);
      if(t > 1.0F) {
         *ns = 0;
         return;
      }
      *ns = 2;
      z1 = zc;
   S12:
      eval(&z1,a,p,x,&t);
      if(t > 1.0F) {
         goto S13;
      }
      z1 = (z1+*sing1)/2.0F;
      goto S12;
   S13:
      converge(&z1,&zc,a,p,x1,alam1);
      z1 = zc;
   S20:
      eval(&z1,a,p,x,&t);
      if(t > 1.0F) {
         goto S21;
      }
      z1 = (z1+*sing2)/2.0F;
      goto S20;
   S21:
      converge(&zc,&z1,a,p,x2,alam2);
      return;
   }

   void eval(
             float *z,
             float a[],
             float p[],
             float x[],
             float *t)
   {
      static int i;
      static float q;
      /*
*************************************************
*/
      for(i=0; i<3; i++) {
         q = 1.0F+*z/fifpow(a[i],2.0F);
         q = 1.0F/q;
         x[i] = q*p[i];
      }
      *t = 0.0F;
      for(i=0; i<3; i++) {
         *t = *t+fifpow(x[i]/a[i],2.0F);
      }
      return;
   }

   void deval(
              double *z,
              double a[],
              double p[],
              double x[],
              double *t)
   {
      static double q;
      static int i;
      /*
*************************************************
*/
      for(i=0; i<3; i++) {
         q = 1.0e0+*z/pow(a[i],2.0);
         q = 1.0e0/q;
         x[i] = q*p[i];
      }
      *t = 0.0e0;
      for(i=0; i<3; i++) {
         *t = *t+pow(x[i]/a[i],2.0);
      }
      return;
   }

   void eval2(
              float *z,
              float a[],
              float p[],
              float x[],
              float *t)
   {
      static int i;
      static float q,den;
      /*
*************************************************
*/
      for(i=0; i<3; i++) {
         q = 1.0F+*z/fifpow(a[i],2.0F);
         q = 1.0F/q;
         x[i] = q*p[i];
      }
      *t = 0.0F;
      for(i=0; i<3; i++) {
         den = fifpow(fifpow(a[i],2.0F)+*z,2.0F);
         *t = *t+x[i]*p[i]/den;
      }
      return;
   }


   void converge(
                 float *zlow,
                 float *zhih,
                 float a[],
                 float p[],
                 float x[],
                 float *alam)
   {
      static double da[3],dp[3],dx[3],dhih,dlow,dm,dt,dthih,dtlow;
      static int i;
      static float t;
      /*
*************************************************
*/
      dhih = (double)*zhih;
      dlow = (double)*zlow;
      for(i=0; i<3; i++) {
         da[i] = (double)a[i];
         dp[i] = (double)p[i];
      }
      deval(&dhih,da,dp,dx,&dthih);
      deval(&dlow,da,dp,dx,&dtlow);
      while((dhih-dlow)*2.0e0/(fabs(dhih)+fabs(dlow)) > 1.0e-12) {
         dm = 0.5e0*(dhih+dlow);
         deval(&dm,da,dp,dx,&dt);
         if((dthih-1.0e0)*(dt-1.0e0) > 0.0e0) {
            dhih = dm;
            dthih = dt;
         }
         else {
            dlow = dm;
            dtlow = dt;
         }
      }
      *zhih = (float)dhih;
      *zlow = (float)dlow;
      *alam = (*zhih+*zlow)/2.0F;
      eval(alam,a,p,x,&t);
      return;
   }

   void minedge(
                float v1[],
                float v2[],
                float p[],
                float *d)
   {
      /*
        Given two points v1 and v2 defining a line segment in space
        Also given an arbitrary point p
        Then return in d the minimum distance between the point p and the
        line segment
      */
      static float p1[3],b[3],tb[3],n[3],tb2,b2,t;
      /*
*************************************************
*/
      vector_difference(p,v1,p1);
      vector_difference(v2,v1,b);
      dotproduct(p1,b,&tb2);
      dotproduct(b,b,&b2);
      t = tb2/b2;
      if(t < 0.0F) {
         pythag(v1,p,d);
      }
      else if(t > 1.0F) {
         pythag(v2,p,d);
      }
      else {
         scalar_product(&t,b,tb);
         vector_difference(p1,tb,n);
         pythag0(n,d);
      }
      return;
   }

   void zeerot(
               float n3[],
               float *t)
   {
      static float R1 = 1.0F;
      static float n1[3],n2[3],nx[3],dx3,beta,alpha;
      static int i;
      /*
*************************************************
*/
      /*
        Upon entry, n3 is a normalized vector.  This returns a
        rotation matrix in t that rotates n into the positive z-axis.
        Proceed by taking an arbitrary normalized vector, but throw it
        away and start over if it is too close to n3.  The second axis
        is taken as a linear combination of these two vectors
      */
   S1:
      sphere(nx,&R1);
      dotproduct(nx,n3,&dx3);
      if(1.0F-fifabs(dx3) <= 0.1F) goto S1;
      beta = fifsqrt(1.0F/(1.0F-fifpow(dx3,2.0F)));
      alpha = -(beta*dx3);
      for(i=0; i<3; i++) {
         n1[i] = alpha*n3[i]+beta*nx[i];
      }
      cross_product(n3,n1,n2);
      for(i=0; i<3; i++) {
         *(t+0+i*3) = n1[i];
         *(t+1+i*3) = n2[i];
         *(t+2+i*3) = n3[i];
      }
      return;
   }

   void xyzrot(
               float n1[],
               float n2[],
               float n3[],
               float *t)
   {
      static int i;
      /*
*************************************************
*/
      for(i=0; i<3; i++) {
         *(t+0+i*3) = n1[i];
         *(t+1+i*3) = n2[i];
         *(t+2+i*3) = n3[i];
      }
      return;
   }
   void rotate(
               float a[],
               float *t,
               float b[])
   {
      static int i,j;
      /*
*************************************************
*/
      /*
        t is a rotation matrix.  Return in a the product:
        a = t.b
      */
      for(i=0; i<3; i++) {
         a[i] = 0.0F;
         for(j=0; j<3; j++) {
            a[i] = a[i]+*(t+i+j*3)*b[j];
         }
      }
      return;
   }

   void vector_difference(
                          float a[],
                          float b[],
                          float c[])
   {
      static int i;
      /*
*************************************************
*/
      /*
        c = a - b
      */
      for(i=0; i<3; i++) {
         c[i] = a[i]-b[i];
      }
      return;
   }

   void cross_product(
                      float a[],
                      float b[],
                      float c[])
   {
      /*
*************************************************
*/
      /*
        c = a x b
      */
      c[2] = a[0]*b[1]-a[1]*b[0];
      c[0] = a[1]*b[2]-a[2]*b[1];
      c[1] = a[2]*b[0]-a[0]*b[2];
      return;
   }

   void dotproduct(
                   float a[],
                   float b[],
                   float *c)
   {
      static int i;
      /*
*************************************************
*/
      /*
        c = a . b
      */
      *c = 0.0F;
      for(i=0; i<3; i++) {
         *c = *c+a[i]*b[i];
      }
      return;
   }

   void scalar_product(
                       float *a_scalar,
                       float b_vector[],
                       float c[])
   {
      /*
*************************************************
*/
      /*
        c = a_scalar * b_vector
      */
      c[0] = *a_scalar*b_vector[0];
      c[1] = *a_scalar*b_vector[1];
      c[2] = *a_scalar*b_vector[2];
      return;
   }

   void pythag0(
                float x[],
                float *d)
   {
      /*
*************************************************
*/
      /*
        Compute distance between x and origin by the pythagorean theorem
        GEJ	d = sqrt(x(1)**2 + x(2)**2 + x(3)**2) ! Rewrite as a product.
      */
      *d = fifsqrt(x[0]*x[0]+x[1]*x[1]+x[2]*x[2]);
      return;
   }

   void pythag(
               float x[],
               float y[],
               float *d)
   {
      static float r;
      static int i;
      /*
*************************************************
*/
      /*
        Compute distance between two points x and y by the
        pythagorian theorem
      */
      *d = 0.0F;
      for(i=0; i<3; i++) {
         /*
           GEJ	Optimize this computation -- rewrite as a product
           d = d + (x(i)-y(i))**2
         */
         r = x[i]-y[i];
         *d = *d+r*r;
      }
      *d = fifsqrt(*d);
      return;
   }

   void sphereaway(
                   float c[],
                   float *rc,
                   float p[])
   {
      static float T1;
      static float n[3],cc;
      /*
*************************************************
*/
      /*
        There is a sphere centered at (cx,cy,cz) with radius rc.  Return in
        p the coordinates of the point on the surface of the sphere that is
        farthest from the origin.
      */
      pythag0(c,&cc);
      if(cc == 0.0F) {
         p[0] = 0.0F;
         p[1] = 0.0F;
         p[2] = *rc;
      }
      else {
         normalize(c,n);
         T1 = cc+*rc;
         scalar_product(&T1,n,p);
      }
      return;
   }

   void sphereby(
                 float p[],
                 float c[],
                 float *rc,
                 float s[])
   {
      /*
        There is a sphere centered at (cx,cy,cz) with radius rc.  Return in
        s the coordinates of the point on the surface of the sphere that is
        nearest the point p.
      */
      static float T1;
      static float n[3],v[3],cmp,cc;
      /*
*************************************************
*/
      vector_difference(c,p,&cmp);
      pythag0(&cmp,&cc);
      normalize(&cmp,n);
      T1 = cc-*rc;
      scalar_product(&T1,n,v);
      s[0] = p[0]+v[0];
      s[1] = p[1]+v[1];
      s[2] = p[2]+v[2];
      return;
   }

   void normalize(
                  float n1[],
                  float n[])
   {
      static float z;
      static int i;
      /*
*************************************************
*/
      dotproduct(n1,n1,&z);
      z = fifsqrt(z);
      for(i=0; i<3; i++) {
         n[i] = n1[i]/z;
      }
      return;
   }

   void calipers(
                 int *maxelts,
                 int eltype[],
                 float *bv,
                 int *nelts,
                 int *m1,
                 float *rotations,
                 unsigned int *span_done,
                 float *span,
                 float *delta_span,
                 unsigned int *shadow_done,
                 float *shadow,
                 float *delta_shadow,
                 float *rlaunch,
                 int *mp,
                 char *id,
                 char * /* round */ ,
                 unsigned int *savehits,
                 int /* P2 */ ,
                 int /* P1 */ )
   {
      typedef struct {
         unsigned int silent;
      } Cshush;
      static Cshush *Tshush = (Cshush*) Xshush;
      typedef struct {
         int sphere_code;
         int triangle_code;
         int disk_code;
         int open_cylinder_code;
         int solid_cylinder_code;
         int donut_code;
         int ellipsoid_code;
         int cube_code;
         int pillar_code;
         int skin_code;
         int units_code;
         int hunits_code;
         int meter_code;
         int cm_code;
         int nm_code;
         int angstrom_code;
         int length_code;
         int temp_code;
         int celcius_code;
         int kelvin_code;
         int mass_code;
         int da_code;
         int kda_code;
         int gram_code;
         int kg_code;
         int visc_code;
         int poise_code;
         int cp_code;
         int solvent_code;
         int water_code;
         int rlaunch_code;
         int bf_code;
      } Cmorse;
      static Cmorse *Tmorse = (Cmorse*) Xmorse;
      typedef struct {
         char fbod[USZ_MAX_ID_SIZE];
         char fzno[USZ_MAX_ID_SIZE];
         char fznr[USZ_MAX_ID_SIZE];
         char fstk[USZ_MAX_ID_SIZE];
         char fdfl[USZ_MAX_ID_SIZE];
         char fefl[USZ_MAX_ID_SIZE];
         char fzh[USZ_MAX_ID_SIZE];
         char fih[USZ_MAX_ID_SIZE];
         char fsh[USZ_MAX_ID_SIZE];
         char fph[USZ_MAX_ID_SIZE];
      } Cfilenames;
      static Cfilenames *Tfilenames = (Cfilenames*) Xfilenames;
      typedef struct {
         int nbod;
         int nzno;
         int nznr;
         int nstk;
         int ndfl;
         int nefl;
         int nzh;
         int nih;
         int nsh;
         int nph;
      } Cfilenumbers;
      static Cfilenumbers *Tfilenumbers = (Cfilenumbers*) Xfilenumbers;
      static float R3 = 1.0F;
      static int kin;
      static unsigned int do_shadow;
      static char flush[2000];
      static double sum[20],acd[20],shoo[20];
      static float x[20],v[3],rlsh;
      static int i;
      // static int mout;
      static int mtdo,jax,loop;
      static float d;
      // static int need;
      /*
*************************************************
*/
      static const char* F200[] = {
         "('PROJECTION CALCULATION',2x,a2048,i12)"
      };
      static const char* F201[] = {
         "(79('='))"
      };
      static const char* F4444[] = {
         "(g20.8)"
      };
      // static const char* F202[] = {
      // "(a2000,$)"
      // };
      static const char* F203[] = {
         "(' ')"
      };
      if(*savehits) OPEN(Tfilenumbers->nph,FILEN,Tfilenames->fph,USZ_MAX_ID_SIZE,STATUS,"unknown",0);
      *span_done = false;
      *shadow_done = false;
      rlsh = 3.14159265F**rlaunch**rlaunch;
      /*
        Attempt to calculate planar projection only if the
        body is composed completely of spheres
      */
      do_shadow = true;
      for(i=0; i<*nelts; i++) {
         if(eltype[i] != Tmorse->sphere_code) {
            do_shadow = false;
            goto S1;
         }
      }
   S1:
      for(i=1; i<2000; i++) {
         *(flush+i) = 0;
      }
      for(i=0; i<20; i++) {
         sum[i] = 0.0e0;
         acd[i] = 0.0e0;
         shoo[i] = 0.0e0;
      }
      if(!Tshush->silent) WRITE(OUTPUT,FMT,F200,1,STRG,id,USZ_MAX_ID_SIZE,INT4,*m1,0);
      if(!Tshush->silent) WRITE(OUTPUT,FMT,F201,1,0);
      // mout = 0;
      mtdo = *m1;
      int steps = mtdo / 100;
      for(jax=1; jax<=mtdo; jax++) {
         loop = fifmod(jax,20)+1;
         sum[loop-1] = sum[loop-1]+1.0e0;
         sphere(v,&R3);
         bridge(maxelts,eltype,bv,nelts,rotations,v,&d);
         if(*savehits) WRITE(Tfilenumbers->nph,FMT,F4444,1,REAL4,d,0);
         acd[loop-1] = acd[loop-1]+(double)d;
         if(do_shadow) {
            shine(maxelts,bv,nelts,v,rlaunch,&kin);
            shoo[loop-1] = shoo[loop-1]+(double)kin;
         }
         //          need = fifnint(79.0F*(float)jax/(float)mtdo); 
         //          while(mout < need) {
         //             *(flush) = *(round+mout+1-1);
         //             /*
         //               GEJ if (.not.silent) print 202, flush  ! I don't see any purpose f
         //             */
         //             mout = mout+1;
         //          }
         if ( zeno_progress && !( jax % steps ) )
         {
            zeno_progress->setValue( zeno_progress->value() + 1 );
            qApp->processEvents();
            if ( zeno_us_hydrodyn->stopFlag )
            {
               // somehow abort
               return;
            }
         }
      }
      if(!Tshush->silent) WRITE(OUTPUT,FMT,F203,1,0);
      for(i=0; i<20; i++) {
         x[i] = (float)(acd[i]/sum[i]);
      }
      mean20(x,span,delta_span);
      *span_done = true;
      if(do_shadow) {
         for(i=0; i<20; i++) {
            x[i] = (float)(shoo[i]/sum[i]);
         }
         mean20(x,shadow,delta_shadow);
         *shadow_done = true;
         *shadow = *shadow*rlsh;
         *delta_shadow = *delta_shadow*rlsh;
      }
      *mp = mtdo;
      if(*savehits) CLOSE(Tfilenumbers->nph,0);
      return;
   }

   void shine(
              int *maxelts,
              float *bv,
              int *nelts,
              float v[],
              float *rlaunch,
              int *kin)
   {
      static float R1 = 1.0F;
      static float vn[3],vp[3],vm[3],vs[3],c[3],pc[3],beta;
      static int j;
      static float z,rpick,tpick,x,y;
      static int i;
      static float rad,alpha,rt2;
      /*
*************************************************
*/
      *kin = 0;
      /*
        construct an arbitrary vn, normal to v
      */
      sphere(vp,&R1);
      beta = 0.0F;
      for(j=0; j<3; j++) {
         beta = beta+v[j]*vp[j];
      }
      for(j=0; j<3; j++) {
         vn[j] = vp[j]-beta*v[j];
      }
      beta = vn[0]*vn[0]+vn[1]*vn[1]+vn[2]*vn[2];
      beta = fifsqrt(beta);
      vn[0] = vn[0]/beta;
      vn[1] = vn[1]/beta;
      vn[2] = vn[2]/beta;
      /*
        construct vm, normal to both v and vn
      */
      vm[0] = v[1]*vn[2]-v[2]*vn[1];
      vm[1] = v[2]*vn[0]-v[0]*vn[2];
      vm[2] = v[0]*vn[1]-v[1]*vn[0];
      /*
        Pick a point at random inside the projection of the launch radius
      */
      z = ran2();
      rpick = *rlaunch*fifsqrt(z);
      tpick = 2.0F*3.14159265F*ran2();
      x = rpick*fifcos(tpick);
      y = rpick*fifsin(tpick);
      for(i=0; i<3; i++) {
         vs[i] = x*vn[i]+y*vm[i];
      }
      for(i=0; i<*nelts; i++) {
         for(j=0; j<3; j++) {
            c[j] = *(bv+i+j**maxelts);
         }
         rad = *(bv+i+3**maxelts);
         alpha = 0.0F;
         for(j=0; j<3; j++) {
            alpha = alpha+v[j]*c[j];
         }
         for(j=0; j<3; j++) {
            pc[j] = c[j]-alpha*v[j];
         }
         rt2 = 0.0F;
         for(j=0; j<3; j++) {
            rt2 = rt2+fifpow(pc[j]-vs[j],2.0F);
         }
         rt2 = fifsqrt(rt2);
         if(rt2 < rad) {
            *kin = 1;
            return;
         }
      }
      return;
   }

   void blizzard(
                 int *maxelts,
                 int eltype[],
                 float *bv,
                 int *nelts,
                 int *m1,
                 float *tol,
                 float *rlaunch,
                 float *rotations,
                 float *cap,
                 float *alpha_bongo,
                 unsigned int *tol_given,
                 unsigned int *zeno_done,
                 float *delta_cap,
                 float *delta_bongo,
                 int *mz,
                 char * /* id */ ,
                 double *tae,
                 double uae[],
                 double *vae,
                 double *wae,
                 char * /* round */ ,
                 float bubble[],
                 float *bubble_rad,
                 int nebtab[],
                 int *nneb,
                 int *ninn,
                 float rlist[],
                 double strikes[],
                 unsigned int *savehits,
                 int /* P2 */ ,
                 int /* P1 */ )
   {
      /*
        Generate random walk trajectories to do path-integration
      */
      typedef struct {
         char fbod[USZ_MAX_ID_SIZE];
         char fzno[USZ_MAX_ID_SIZE];
         char fznr[USZ_MAX_ID_SIZE];
         char fstk[USZ_MAX_ID_SIZE];
         char fdfl[USZ_MAX_ID_SIZE];
         char fefl[USZ_MAX_ID_SIZE];
         char fzh[USZ_MAX_ID_SIZE];
         char fih[USZ_MAX_ID_SIZE];
         char fsh[USZ_MAX_ID_SIZE];
         char fph[USZ_MAX_ID_SIZE];
      } Cfilenames;
      static Cfilenames *Tfilenames = (Cfilenames*) Xfilenames;
      typedef struct {
         int nbod;
         int nzno;
         int nznr;
         int nstk;
         int ndfl;
         int nefl;
         int nzh;
         int nih;
         int nsh;
         int nph;
      } Cfilenumbers;
      static Cfilenumbers *Tfilenumbers = (Cfilenumbers*) Xfilenumbers;
      typedef struct {
         unsigned int silent;
      } Cshush;
      static Cshush *Tshush = (Cshush*) Xshush;
      static double grand;
      // static float copy[3][3];
      // static float dopy[3][3];
      static int kk[3],khitp[20][3],khite[20][3];
      static double vp[20][3][3],ve[20][3][3],sum[20];
      static float aa[3][3],daa[3][3];
      static double taer[3];
      static float rt[3];
      static unsigned int hit;
      static char flush[2000];
      static int hitelt;
      static char pom[3];
      static float r,r2;
      static int mtdo;
      static int i;
      // static int mout;
      static int jax;
      static int loop;
      // static int need;
      static int j;
      static int ii;
      static int jj;
      /*
*************************************************
*/
//       static const char* F654[] = {
//          "('ZENO CALCULATION',8x,a2048,i12)"
//       };
//       static const char* F777[] = {
//          "(79('='))"
//       };
      static const char* F4444[] = {
         "(a3,3g20.8)"
      };
//       static const char* F776[] = {
//          "(a2000,$)"
//       };
//       static const char* F656[] = {
//          "(' ')"
//       };
      static const char* F975[] = {
         "(i10,f20.0)"
      };
      static const char* F974[] = {
         "('Grand total:  ',f20.0)"
      };
      if(*savehits) OPEN(Tfilenumbers->nzh,FILEN,Tfilenames->fzh,USZ_MAX_ID_SIZE,STATUS,"unknown",0);
      *zeno_done = false;
      if(!*tol_given) {
         *tol = *rlaunch/1.0e6F;
         *tol_given = true;
      }
      r = *rlaunch;
      r2 = fifpow(r,2.0F);
      mtdo = *m1;
      for(i=1; i<2000; i++) {
         *(flush+i) = 0;
         /*
           unprintable characters
           in order to flush the
           output buffer
         */
      }
      for(i=0; i<*nelts; i++) {
         strikes[i] = 0.0e0;
      }
      reinit((int *)khitp,
             (int *)khite,
             (double *)vp,
             (double *)ve,
             sum);
      // if(!Tshush->silent) WRITE(OUTPUT,FMT,F654,1,STRG,id,USZ_MAX_ID_SIZE,INT4,*m1,0);
      // if(!Tshush->silent) WRITE(OUTPUT,FMT,F777,1,0);
      if (!Tshush->silent) {
         zeno_us_hydrodyn->editor->append( "ZENO calculation start\n" );         
         qApp->processEvents();
      }

      // mout = 0;
      int steps = mtdo / 100;
      for(jax=1; jax<=mtdo; jax++) {
         loop = fifmod(jax,20)+1;
         sum[loop-1] = sum[loop-1]+1.0e0;
         sphere(rt,&r);
         charge(rt,&r,kk);
         park(maxelts,eltype,bv,rotations,nelts,rt,&r,&r2,&hit,tol,bubble,bubble_rad,nebtab,nneb,ninn,rlist,&hitelt);
         if(hit) {
            accume(rt,
                   kk,
                   (int *)khitp,
                   (int *)khite,
                   (double *)vp,
                   (double *)ve,
                   &loop);
            strikes[hitelt-1] = strikes[hitelt-1]+1.0e0;
            if(*savehits) {
               plus_or_minus(kk,pom,3);
               WRITE(Tfilenumbers->nzh,FMT,F4444,1,STRG,pom,3,DO,3,REAL4,rt,0);
            }
         }
//          need = fifnint(79.0F*(float)jax/(float)mtdo);
//          while(mout < need) {
//             *(flush) = *(round+mout+1-1);
//             /*
//               GEJ if (.not.silent) print 776,flush  ! I don't see any purpose for t
//             */
//             mout = mout+1;
//          }
         if ( zeno_progress && !( jax % steps ) )
         {
            zeno_progress->setValue( zeno_progress->value() + 1 );
            qApp->processEvents();
            if ( zeno_us_hydrodyn->stopFlag )
            {
               // somehow abort
               return;
            }
         }
      }
//       if(!Tshush->silent) WRITE(OUTPUT,FMT,F656,1,0);
      tally(
            (int *)khitp,
            (int *)khite,
            (double *)vp,
            (double *)ve,
            sum,
            (float *)aa,
            (float *)daa,
            cap,
            delta_cap,
            &r,
            taer,
            uae,
            vae,
            wae,
            rlaunch);
      *tae = taer[0];
      for(i=0; i<3; i++) {
         for(j=0; j<3; j++) {
            *(wae+i+j*3) = *(wae+i+j*3)*(12.0e0*3.14159265e0)*pow((double)r,2.0);
            *(vae+i+j*3) = *(vae+i+j*3)*(12.0e0*3.14159265e0)*pow((double)r,2.0);
         }
      }
      /*
        Because of sampling error, the polarizability tensor is not
        exactly symmetric.  Now we symmetrize it.
      */
      for(ii=0; ii<3; ii++) {
         for(jj=0; jj<3; jj++) {
            // copy[jj][ii] = aa[jj][ii];
            // dopy[jj][ii] = daa[jj][ii];
         }
      }
      aa[1][0] = 0.5F*(aa[1][0]+aa[0][1]);
      aa[2][0] = 0.5F*(aa[2][0]+aa[0][2]);
      aa[2][1] = 0.5F*(aa[2][1]+aa[1][2]);
      aa[0][1] = aa[1][0];
      aa[0][2] = aa[2][0];
      aa[1][2] = aa[2][1];
      daa[1][0] = 0.5F*(daa[1][0]+daa[0][1]);
      daa[2][0] = 0.5F*(daa[2][0]+daa[0][2]);
      daa[2][1] = 0.5F*(daa[2][1]+daa[1][2]);
      daa[0][1] = daa[1][0];
      daa[0][2] = daa[2][0];
      daa[1][2] = daa[2][1];
      for(ii=0; ii<3; ii++) {
         for(jj=0; jj<3; jj++) {
            *(alpha_bongo+ii+jj*3) = aa[jj][ii];
            *(delta_bongo+ii+jj*3) = daa[jj][ii];
         }
      }
      *zeno_done = true;
      *mz = mtdo;
      OPEN(Tfilenumbers->nstk,FILEN,Tfilenames->fstk,USZ_MAX_ID_SIZE,STATUS,"unknown",0);
      grand = 0.0e0;
      for(i=1; i<=*nelts; i++) {
         grand = grand+strikes[i-1];
         WRITE(Tfilenumbers->nstk,FMT,F975,1,INT4,i,REAL8,strikes[i-1],0);
      }
      WRITE(Tfilenumbers->nstk,FMT,F974,1,REAL8,grand,0);
      CLOSE(Tfilenumbers->nstk,0);
      if(*savehits) CLOSE(Tfilenumbers->nzh,0);
      return;
   }

   void plus_or_minus(
                      int kk[],
                      char *pom,
                      int /* P1 */ )
   {
      static int i;
      /*
*************************************************
*/
      for(i=0; i<3; i++) {
         if(kk[i] == -1) {
            *(pom+i) = '-';
         }
         else if(kk[i] == 1) {
            *(pom+i) = '+';
         }
         else {
            Stop("pom pom error");
         }
      }
      return;
   }

   void pade(
             float *alpha_bongo,
             float *q2pade,
             double eigens[],
             double xx[])
   {
      /*
        This does the Pade approximant to estimate the [eta]/[sigma]
        ratio, i.e., q2, from the polarizability tensor
      */
      static int K1 = 3;
      static float a[3][3],d[3],e[3];
      static char jet[516];
      static int var[4][2][6],varmag[4][2][6];
      static float fv[6],m,alpha,c,bb,b,delta,num,den;
      static int i,j,nwk;
      static float save;
      static int J1,j2,j3;
      static float x1,x2,coef,poww;
      /*
*************************************************
*/
      for(i=0; i<3; i++) {
         for(j=0; j<3; j++) {
            a[j][i] = *(alpha_bongo+i+j*3);
         }
      }
      /*
        Diagonalize the polarizability tensor, eigenvalues wind up in d
      */
      tred2((float *)a,&K1,&K1,d,e);
      tqli(d,e,&K1,&K1,(float *)a);
      /*
        bubble-sort the eigenvalues:
      */
      nwk = 1;
      while(nwk < 3) {
         if(nwk == 0) nwk = 1;
         if(d[nwk-1] <= d[nwk]) {
            nwk = nwk+1;
         }
         else {
            save = d[nwk-1];
            d[nwk-1] = d[nwk];
            d[nwk] = save;
            nwk = nwk-1;
         }
      }
      /*
        Since the tensor is positive definite, eigenvalues
        can only be negative by round-off
      */
      if(d[0] <= 0.0F) d[0] = 0.0F;
      for(i=0; i<3; i++) {
         eigens[i] = (double)d[i];
      }
      ftnscopy(jet,43,"delta 4800 -3    660 -3  -1247 -3    787 -3",43,NULL);
      ftnscopy((jet+258),43,"k        0 -3   1040 -3   2012 -3   2315 -3",43,NULL);
      ftnscopy((jet+43),43,"b      680 -3  -7399 -3   1048 -3    136 -3",43,NULL);
      ftnscopy((jet+301),43,"t        0 -3   1063 -3    895 -3   4993 -3",43,NULL);
      ftnscopy((jet+86),43,"B     1925 -3  -8611 -3   1652 -3   -120 -3",43,NULL);
      ftnscopy((jet+344),43,"q        0 -3   1344 -3   2029 -3   1075 -3",43,NULL);
      ftnscopy((jet+129),43,"c     1343 -2   1617 -2     51 -2   -586 -2",43,NULL);
      ftnscopy((jet+387),43,"r        0 -3    489 -3    879 -3   2447 -3",43,NULL);
      ftnscopy((jet+172),43,"alpha 1623 -2  -1592 -2   1483 -2   -374 -2",43,NULL);
      ftnscopy((jet+430),43,"v        0 -3    462 -3   1989 -3   4608 -3",43,NULL);
      ftnscopy((jet+215),43,"m     2786 -3    293 -3   -110 -3     12 -3",43,NULL);
      ftnscopy((jet+473),43,"u        0 -3    556 -3   2034 -3   3024 -3",43,NULL);
      /*
        1234567890123456789012345678901234567890123
      */
      for(i=0; i<6; i++) {
         for(j=0; j<2; j++) {
            ftnscopy((jet+(i+j*6)*43),5,"     ",5,NULL);
         }
      }
      for(J1=0; J1<6; J1++) {
         for(j2=0; j2<2; j2++) {
            READ(INTERNAL,(jet+(J1+j2*6)*43),43,LISTIO,MORE);
            for(j3=0; j3<4; j3++) {
               READ(DO,1,INT4,&var[j3][j2][J1],DO,1,INT4,&varmag[j3][j2][J1],MORE);
            }
            READ(0);
         }
      }
      if(d[0] != 0.0F) {
         xx[0] = (double)(d[1]/d[0]);
         xx[1] = (double)(d[2]/d[1]);
         x1 = fiflog(d[1]/d[0]);
         x2 = fiflog(d[2]/d[1]);
         for(J1=0; J1<6; J1++) {
            fv[J1] = 0.0F;
            for(j3=0; j3<4; j3++) {
               coef = (float)var[j3][0][J1]*fifpow(10.0F,(float)varmag[j3][0][J1]);
               poww = (float)var[j3][1][J1]*fifpow(10.0F,(float)varmag[j3][1][J1]);
               fv[J1] = fv[J1]+coef*fifexp(-(poww*x1));
            }
         }
      }
      else {
         /*
           The object must be two-dimensional
           x1 = infinity
         */
         xx[0] = (double)(d[1]/d[0]);
         xx[1] = (double)(d[2]/d[1]);
         x2 = fiflog(d[2]/d[1]);
         for(J1=0; J1<6; J1++) {
            coef = (float)var[0][0][J1]*fifpow(10.0F,(float)varmag[0][0][J1]);
            fv[J1] = coef;
         }
      }
      delta = fv[0];
      b = fv[1];
      bb = fv[2];
      c = fv[3];
      alpha = fv[4];
      m = fv[5];
      num = delta*alpha+c*x2+b*fifpow(x2,2.0F)+4.0F*fifpow(x2,m);
      den = 6.0F*alpha+6.0F*c*x2/delta+bb*fifpow(x2,2.0F)+5.0F*fifpow(x2,m);
      *q2pade = num/den;
      return;
   }

   void ttdiag(double *tten,
               double eigens[])
   {
      /*
        This diagonalizes the (Rg2 = T) tensor
      */
      static int K1 = 3;
      static float a[3][3],d[3],e[3];
      static int i,j,nwk;
      static float save;
      /*
*************************************************
*/
      for(i=0; i<3; i++) {
         for(j=0; j<3; j++) {
            a[j][i] = (float)*(tten+i+j*3);
         }
      }
      /*
        Diagonalize the polarizability tensor, eigenvalues wind up in d
      */
      tred2((float *)a,&K1,&K1,d,e);
      tqli(d,e,&K1,&K1,(float *)a);
      /*
        bubble-sort the eigenvalues:
      */
      nwk = 1;
      while(nwk < 3) {
         if(nwk == 0) nwk = 1;
         if(d[nwk-1] <= d[nwk]) {
            nwk = nwk+1;
         }
         else {
            save = d[nwk-1];
            d[nwk-1] = d[nwk];
            d[nwk] = save;
            nwk = nwk-1;
         }
      }
      /*
        Since the tensor is positive definite, eigenvalues
        can only be negative by round-off
      */
      if(d[0] <= 0.0F) d[0] = 0.0F;
      for(i=0; i<3; i++) {
         eigens[i] = (double)d[i];
      }
      return;
   }

   void bridge(
               int *maxelts,
               int eltype[],
               float *bv,
               int *nelts,
               float *rotations,
               float v[],
               float *d)
   {
      /*
        Upon entry, v is an arbitrary unit vector.
        d is returned as the span of the body projected onto the
        vector v.
      */
      typedef struct {
         int sphere_code;
         int triangle_code;
         int disk_code;
         int open_cylinder_code;
         int solid_cylinder_code;
         int donut_code;
         int ellipsoid_code;
         int cube_code;
         int pillar_code;
         int skin_code;
         int units_code;
         int hunits_code;
         int meter_code;
         int cm_code;
         int nm_code;
         int angstrom_code;
         int length_code;
         int temp_code;
         int celcius_code;
         int kelvin_code;
         int mass_code;
         int da_code;
         int kda_code;
         int gram_code;
         int kg_code;
         int visc_code;
         int poise_code;
         int cp_code;
         int solvent_code;
         int water_code;
         int rlaunch_code;
         int bf_code;
      } Cmorse;
      static Cmorse *Tmorse = (Cmorse*) Xmorse;
      static float c[3],v1[3],v2[3],v3[3],n[3],n1[3],n2[3],x[3],y[3],zl[3],zh[3],t[3][3];
      static int i;
      static float s1,s2;
      static int j;
      static float side,r;
      static int k;
      static float al,r1,r2,aa,bb,cc,d1,d2;
      /*
*************************************************
*/
      for(i=1; i<=*nelts; i++) {
         if(eltype[i-1] == Tmorse->pillar_code) {
            x[0] = *(bv+i-1+0**maxelts);
            x[1] = *(bv+i-1+4**maxelts);
            x[2] = *(bv+i-1+8**maxelts);
            y[0] = *(bv+i-1+1**maxelts);
            y[1] = *(bv+i-1+5**maxelts);
            y[2] = *(bv+i-1+9**maxelts);
            zl[0] = *(bv+i-1+2**maxelts);
            zl[1] = *(bv+i-1+6**maxelts);
            zl[2] = *(bv+i-1+10**maxelts);
            zh[0] = *(bv+i-1+3**maxelts);
            zh[1] = *(bv+i-1+7**maxelts);
            zh[2] = *(bv+i-1+11**maxelts);
            propillar(x,y,zl,zh,v,&s1,&s2);
         }
         else if(eltype[i-1] == Tmorse->cube_code) {
            for(j=0; j<3; j++) {
               v1[j] = *(bv+i-1+j**maxelts);
            }
            side = *(bv+i-1+3**maxelts);
            procube(v1,&side,v,&s1,&s2);
         }
         else if(eltype[i-1] == Tmorse->sphere_code) {
            for(j=0; j<3; j++) {
               c[j] = *(bv+i-1+j**maxelts);
            }
            r = *(bv+i-1+3**maxelts);
            prosphere(c,&r,v,&s1,&s2);
         }
         else if(eltype[i-1] == Tmorse->triangle_code) {
            for(j=0; j<3; j++) {
               v1[j] = *(bv+i-1+j**maxelts);
               v2[j] = *(bv+i-1+(j+3)**maxelts);
               v3[j] = *(bv+i-1+(j+6)**maxelts);
            }
            protriangle(v1,v2,v3,v,&s1,&s2);
         }
         else if(eltype[i-1] == Tmorse->disk_code) {
            for(j=0; j<3; j++) {
               c[j] = *(bv+i-1+j**maxelts);
               n[j] = *(bv+i-1+(j+3)**maxelts);
            }
            r = *(bv+i-1+6**maxelts);
            for(j=0; j<3; j++) {
               for(k=0; k<3; k++) {
                  t[k][j] = *(rotations+i-1+(j+k*3)**maxelts);
               }
            }
            prodisk(c,n,&r,(float *)t,v,&s1,&s2);
         }
         else if(eltype[i-1] == Tmorse->open_cylinder_code) {
            for(j=0; j<3; j++) {
               c[j] = *(bv+i-1+j**maxelts);
               n[j] = *(bv+i-1+(j+3)**maxelts);
            }
            r = *(bv+i-1+6**maxelts);
            al = *(bv+i-1+7**maxelts);
            for(j=0; j<3; j++) {
               for(k=0; k<3; k++) {
                  t[k][j] = *(rotations+i-1+(j+k*3)**maxelts);
               }
            }
            procylinder(c,n,&r,&al,(float *)t,v,&s1,&s2);
         }
         else if(eltype[i-1] == Tmorse->solid_cylinder_code) {
            for(j=0; j<3; j++) {
               c[j] = *(bv+i-1+j**maxelts);
               n[j] = *(bv+i-1+(j+3)**maxelts);
            }
            r = *(bv+i-1+6**maxelts);
            al = *(bv+i-1+7**maxelts);
            for(j=0; j<3; j++) {
               for(k=0; k<3; k++) {
                  t[k][j] = *(rotations+i-1+(j+k*3)**maxelts);
               }
            }
            prosolcyl(c,n,&r,&al,(float *)t,v,&s1,&s2);
         }
         else if(eltype[i-1] == Tmorse->donut_code) {
            for(j=0; j<3; j++) {
               c[j] = *(bv+i-1+j**maxelts);
               n[j] = *(bv+i-1+(j+3)**maxelts);
            }
            r1 = *(bv+i-1+6**maxelts);
            r2 = *(bv+i-1+7**maxelts);
            for(j=0; j<3; j++) {
               for(k=0; k<3; k++) {
                  t[k][j] = *(rotations+i-1+(j+k*3)**maxelts);
               }
            }
            protorus(c,n,&r1,&r2,(float *)t,v,&s1,&s2);
         }
         else if(eltype[i-1] == Tmorse->ellipsoid_code) {
            for(j=0; j<3; j++) {
               c[j] = *(bv+i-1+j**maxelts);
               n1[j] = *(bv+i-1+(j+3)**maxelts);
               n2[j] = *(bv+i-1+(j+6)**maxelts);
            }
            aa = *(bv+i-1+9**maxelts);
            bb = *(bv+i-1+10**maxelts);
            cc = *(bv+i-1+11**maxelts);
            for(j=0; j<3; j++) {
               for(k=0; k<3; k++) {
                  t[k][j] = *(rotations+i-1+(j+k*3)**maxelts);
               }
            }
            proellipsoid(c,n1,n2,&aa,&bb,&cc,(float *)t,v,&s1,&s2);
         }
         if(i == 1) {
            d1 = s1;
            d2 = s2;
         }
         else {
            d1 = fifamin1(d1,s1);
            d2 = fifamax1(d2,s2);
         }
      }
      *d = d2-d1;
      return;
   }

   void distance(
                 int *maxelts,
                 int eltype[],
                 float *bv,
                 int *nelts,
                 float *rotations,
                 float p[],
                 float *ds,
                 float bubble[],
                 float *bubble_rad,
                 int nebtab[],
                 int *nneb,
                 int *ninn,
                 float rlist[],
                 int *nearto)
   {
      /*
        Compute distance from point p to the surface of the body
        The distance is returned as ds.  The sphere centered at point
        p having radius ds is called the jump sphere.
      */
      typedef struct {
         int sphere_code;
         int triangle_code;
         int disk_code;
         int open_cylinder_code;
         int solid_cylinder_code;
         int donut_code;
         int ellipsoid_code;
         int cube_code;
         int pillar_code;
         int skin_code;
         int units_code;
         int hunits_code;
         int meter_code;
         int cm_code;
         int nm_code;
         int angstrom_code;
         int length_code;
         int temp_code;
         int celcius_code;
         int kelvin_code;
         int mass_code;
         int da_code;
         int kda_code;
         int gram_code;
         int kg_code;
         int visc_code;
         int poise_code;
         int cp_code;
         int solvent_code;
         int water_code;
         int rlaunch_code;
         int bf_code;
      } Cmorse;
      static Cmorse *Tmorse = (Cmorse*) Xmorse;
      static float c[3],v1[3],v2[3],v3[3],n[3],n1[3],n2[3],x[3],y[3],zl[3],zh[3],t[3][3];
      static int i;
      static float d;
      static int j;
      static float side,r;
      static int k;
      static float al,r1,r2,aa,bb,cc,close;
      static int ii;
      static float pb;
      /*
*************************************************
*/
      /*
        ---------------------------------------------------------------------
      */
      if(*bubble_rad < 0.0F) {
         goto S1234;                /*  Need to generate a new bubble and neighborlist */
         /*
           before computing distance
         */
      }
      else {
         goto S9876;                /*  The current bubble and neighborlist are */
         /*
           assumed to be OK -- we will jump back to
           1234 if we later find out they are not
         */
      }
      /*
        ---------------------------------------------------------------------
        Generate a new neighbor table
      */
   S1234:
      /*
        table by probing all body elements
      */
      *ninn = 0;                     /*  Generating jump sphere and the new neighbor */
      *nearto = 0;
      for(i=1; i<=*nelts; i++) {
         if(eltype[i-1] == Tmorse->pillar_code) {
            x[0] = *(bv+i-1+0**maxelts);
            x[1] = *(bv+i-1+4**maxelts);
            x[2] = *(bv+i-1+8**maxelts);
            y[0] = *(bv+i-1+1**maxelts);
            y[1] = *(bv+i-1+5**maxelts);
            y[2] = *(bv+i-1+9**maxelts);
            zl[0] = *(bv+i-1+2**maxelts);
            zl[1] = *(bv+i-1+6**maxelts);
            zl[2] = *(bv+i-1+10**maxelts);
            zh[0] = *(bv+i-1+3**maxelts);
            zh[1] = *(bv+i-1+7**maxelts);
            zh[2] = *(bv+i-1+11**maxelts);
            minpillar(x,y,zl,zh,p,&d);
         }
         else if(eltype[i-1] == Tmorse->cube_code) {
            for(j=0; j<3; j++) {
               v1[j] = *(bv+i-1+j**maxelts);
            }
            side = *(bv+i-1+3**maxelts);
            mincube(v1,&side,p,&d);
         }
         else if(eltype[i-1] == Tmorse->sphere_code) {
            for(j=0; j<3; j++) {
               c[j] = *(bv+i-1+j**maxelts);
            }
            r = *(bv+i-1+3**maxelts);
            minsphere(c,&r,p,&d);
         }
         else if(eltype[i-1] == Tmorse->triangle_code) {
            for(j=0; j<3; j++) {
               v1[j] = *(bv+i-1+j**maxelts);
               v2[j] = *(bv+i-1+(j+3)**maxelts);
               v3[j] = *(bv+i-1+(j+6)**maxelts);
            }
            mintriangle(v1,v2,v3,p,&d);
         }
         else if(eltype[i-1] == Tmorse->disk_code) {
            for(j=0; j<3; j++) {
               c[j] = *(bv+i-1+j**maxelts);
               n[j] = *(bv+i-1+(j+3)**maxelts);
            }
            r = *(bv+i-1+6**maxelts);
            for(j=0; j<3; j++) {
               for(k=0; k<3; k++) {
                  t[k][j] = *(rotations+i-1+(j+k*3)**maxelts);
               }
            }
            mindisk(c,n,&r,(float *)t,p,&d);
         }
         else if(eltype[i-1] == Tmorse->open_cylinder_code) {
            for(j=0; j<3; j++) {
               c[j] = *(bv+i-1+j**maxelts);
               n[j] = *(bv+i-1+(j+3)**maxelts);
            }
            r = *(bv+i-1+6**maxelts);
            al = *(bv+i-1+7**maxelts);
            for(j=0; j<3; j++) {
               for(k=0; k<3; k++) {
                  t[k][j] = *(rotations+i-1+(j+k*3)**maxelts);
               }
            }
            mincylinder(c,n,&r,&al,(float *)t,p,&d);
         }
         else if(eltype[i-1] == Tmorse->solid_cylinder_code) {
            for(j=0; j<3; j++) {
               c[j] = *(bv+i-1+j**maxelts);
               n[j] = *(bv+i-1+(j+3)**maxelts);
            }
            r = *(bv+i-1+6**maxelts);
            al = *(bv+i-1+7**maxelts);
            for(j=0; j<3; j++) {
               for(k=0; k<3; k++) {
                  t[k][j] = *(rotations+i-1+(j+k*3)**maxelts);
               }
            }
            minsolcyl(c,n,&r,&al,(float *)t,p,&d);
         }
         else if(eltype[i-1] == Tmorse->donut_code) {
            for(j=0; j<3; j++) {
               c[j] = *(bv+i-1+j**maxelts);
               n[j] = *(bv+i-1+(j+3)**maxelts);
            }
            r1 = *(bv+i-1+6**maxelts);
            r2 = *(bv+i-1+7**maxelts);
            for(j=0; j<3; j++) {
               for(k=0; k<3; k++) {
                  t[k][j] = *(rotations+i-1+(j+k*3)**maxelts);
               }
            }
            mintorus(c,n,&r1,&r2,(float *)t,p,&d);
         }
         else if(eltype[i-1] == Tmorse->ellipsoid_code) {
            for(j=0; j<3; j++) {
               c[j] = *(bv+i-1+j**maxelts);
               n1[j] = *(bv+i-1+(j+3)**maxelts);
               n2[j] = *(bv+i-1+(j+6)**maxelts);
            }
            aa = *(bv+i-1+9**maxelts);
            bb = *(bv+i-1+10**maxelts);
            cc = *(bv+i-1+11**maxelts);
            for(j=0; j<3; j++) {
               for(k=0; k<3; k++) {
                  t[k][j] = *(rotations+i-1+(j+k*3)**maxelts);
               }
            }
            minellipsoid(c,n1,n2,&aa,&bb,&cc,(float *)t,p,&d);
         }
         *ninn = *ninn+1;
         rlist[*ninn-1] = d;
         nebtab[*ninn-1] = i;
         listersort(rlist,ninn,nneb,nebtab,maxelts);
         if(*nearto == 0) {
            *nearto = i;
            close = d;
         }
         else {
            if(d < close) {
               close = d;
               *nearto = i;
            }
         }
      }
      *ds = rlist[0];
      *bubble_rad = rlist[*ninn-1];
      for(i=1; i<=3; i++) {
         bubble[i-1] = p[i-1];
      }
      if(*ds != close) {
         WRITE(OUTPUT,LISTIO,STRG,"ds,close = ",11,REAL4,*ds,REAL4,close,0);
         WRITE(OUTPUT,LISTIO,STRG," ds .ne. close    error",23,0);
         Stop("1");
      }
      return;
      /*
        --------------------------------------------------------------
        Scan neighbor table for smallest distance
      */
   S9876:
      /*
        nearby body elements.  If any part
        of the jump sphere is later found to
        lie outside the bubble, we will redo
        this step by jumping to 1234.
      */
      *nearto = 0;                   /*  Generating the jump sphere by probing */
      for(ii=1; ii<=*ninn; ii++) {
         i = nebtab[ii-1];
         if(eltype[i-1] == Tmorse->pillar_code) {
            x[0] = *(bv+i-1+0**maxelts);
            x[1] = *(bv+i-1+4**maxelts);
            x[2] = *(bv+i-1+8**maxelts);
            y[0] = *(bv+i-1+1**maxelts);
            y[1] = *(bv+i-1+5**maxelts);
            y[2] = *(bv+i-1+9**maxelts);
            zl[0] = *(bv+i-1+2**maxelts);
            zl[1] = *(bv+i-1+6**maxelts);
            zl[2] = *(bv+i-1+10**maxelts);
            zh[0] = *(bv+i-1+3**maxelts);
            zh[1] = *(bv+i-1+7**maxelts);
            zh[2] = *(bv+i-1+11**maxelts);
            minpillar(x,y,zl,zh,p,&d);
         }
         else if(eltype[i-1] == Tmorse->cube_code) {
            for(j=0; j<3; j++) {
               v1[j] = *(bv+i-1+j**maxelts);
            }
            side = *(bv+i-1+3**maxelts);
            mincube(v1,&side,p,&d);
         }
         else if(eltype[i-1] == Tmorse->sphere_code) {
            for(j=0; j<3; j++) {
               c[j] = *(bv+i-1+j**maxelts);
            }
            r = *(bv+i-1+3**maxelts);
            minsphere(c,&r,p,&d);
         }
         else if(eltype[i-1] == Tmorse->triangle_code) {
            for(j=0; j<3; j++) {
               v1[j] = *(bv+i-1+j**maxelts);
               v2[j] = *(bv+i-1+(j+3)**maxelts);
               v3[j] = *(bv+i-1+(j+6)**maxelts);
            }
            mintriangle(v1,v2,v3,p,&d);
         }
         else if(eltype[i-1] == Tmorse->disk_code) {
            for(j=0; j<3; j++) {
               c[j] = *(bv+i-1+j**maxelts);
               n[j] = *(bv+i-1+(j+3)**maxelts);
            }
            r = *(bv+i-1+6**maxelts);
            for(j=0; j<3; j++) {
               for(k=0; k<3; k++) {
                  t[k][j] = *(rotations+i-1+(j+k*3)**maxelts);
               }
            }
            mindisk(c,n,&r,(float *)t,p,&d);
         }
         else if(eltype[i-1] == Tmorse->open_cylinder_code) {
            for(j=0; j<3; j++) {
               c[j] = *(bv+i-1+j**maxelts);
               n[j] = *(bv+i-1+(j+3)**maxelts);
            }
            r = *(bv+i-1+6**maxelts);
            al = *(bv+i-1+7**maxelts);
            for(j=0; j<3; j++) {
               for(k=0; k<3; k++) {
                  t[k][j] = *(rotations+i-1+(j+k*3)**maxelts);
               }
            }
            mincylinder(c,n,&r,&al,(float *)t,p,&d);
         }
         else if(eltype[i-1] == Tmorse->solid_cylinder_code) {
            for(j=0; j<3; j++) {
               c[j] = *(bv+i-1+j**maxelts);
               n[j] = *(bv+i-1+(j+3)**maxelts);
            }
            r = *(bv+i-1+6**maxelts);
            al = *(bv+i-1+7**maxelts);
            for(j=0; j<3; j++) {
               for(k=0; k<3; k++) {
                  t[k][j] = *(rotations+i-1+(j+k*3)**maxelts);
               }
            }
            minsolcyl(c,n,&r,&al,(float *)t,p,&d);
         }
         else if(eltype[i-1] == Tmorse->donut_code) {
            for(j=0; j<3; j++) {
               c[j] = *(bv+i-1+j**maxelts);
               n[j] = *(bv+i-1+(j+3)**maxelts);
            }
            r1 = *(bv+i-1+6**maxelts);
            r2 = *(bv+i-1+7**maxelts);
            for(j=0; j<3; j++) {
               for(k=0; k<3; k++) {
                  t[k][j] = *(rotations+i-1+(j+k*3)**maxelts);
               }
            }
            mintorus(c,n,&r1,&r2,(float *)t,p,&d);
         }
         else if(eltype[i-1] == Tmorse->ellipsoid_code) {
            for(j=0; j<3; j++) {
               c[j] = *(bv+i-1+j**maxelts);
               n1[j] = *(bv+i-1+(j+3)**maxelts);
               n2[j] = *(bv+i-1+(j+6)**maxelts);
            }
            aa = *(bv+i-1+9**maxelts);
            bb = *(bv+i-1+10**maxelts);
            cc = *(bv+i-1+11**maxelts);
            for(j=0; j<3; j++) {
               for(k=0; k<3; k++) {
                  t[k][j] = *(rotations+i-1+(j+k*3)**maxelts);
               }
            }
            minellipsoid(c,n1,n2,&aa,&bb,&cc,(float *)t,p,&d);
         }
         if(ii == 1) {
            *ds = d;
         }
         else {
            *ds = fifamin1(*ds,d);
         }
         if(*nearto == 0) {
            close = d;
            *nearto = i;
         }
         else {
            if(d < close) {
               close = d;
               *nearto = i;
            }
         }
      }
      if(*ds != close) {
         WRITE(OUTPUT,LISTIO,STRG,"ds,close = ",11,REAL4,*ds,REAL4,close,0);
         WRITE(OUTPUT,LISTIO,STRG," ds .ne. close    error",23,0);
         Stop("2");
      }
      /*
        --------------------------------------------------------------
        At this point we have identified a jump sphere of radius ds
        centered on the point p, which is the current position of the
        random walker.  However, it was determined only by probing
        the nearby body elements.  Therefore, it must be rejected
        if any part of this sphere lies outside of the bubble generated
        at the time that this particular list of nearby elements
        was generated.  So if it is, is we jump back up and generate the
        jump sphere by probing all body elements.
        pb is the distance from point p to the center of the bubble
      */
      pb = 0.0F;
      for(i=1; i<=3; i++) {
         pb = pb+fifpow(bubble[i-1]-p[i-1],2.0F);
      }
      pb = fifsqrt(pb);
      if(pb+*ds > *bubble_rad) goto S1234;
      /*
        --------------------------------------------------------------
      */
      return;
   }

   void listersort(
                   float rlist[],
                   int *ninn,
                   int *nneb,
                   int nebtab[],
                   int * /* maxelts */ )
   {
      static int nwk;
      static float copy;
      static int kopy;
      /*
*************************************************
*/
      nwk = 1;
      while(nwk < *ninn) {
         if(nwk == 0) nwk = 1;
         if(rlist[nwk-1] <= rlist[nwk]) {
            nwk = nwk+1;
         }
         else {
            copy = rlist[nwk-1];
            rlist[nwk-1] = rlist[nwk];
            rlist[nwk] = copy;
            kopy = nebtab[nwk-1];
            nebtab[nwk-1] = nebtab[nwk];
            nebtab[nwk] = kopy;
            nwk = nwk-1;
         }
      }
      if(*ninn > *nneb) *ninn = *nneb;
      return;
   }

   void tally(
              int *khitp,
              int *khite,
              double *vp,
              double *ve,
              double sum[],
              float *aa,
              float *daa,
              float *cap,
              float *delta_cap,
              float *r1,
              double tae[],
              double uae[],
              double *vae,
              double *wae,
              float * /* rlaunch */ )
   {
      /*
        convert raw statistics on random walk trajectories into
        polarizability tensor and capacitance
        tae,uae,vae,wae are being calculated and reported for
        the cases in which an ensemble average will be taken
      */
      static double tt[20][3],tu[20][3],tv[20][3][3],tw[20][3][3];
      static float capar[20],aar[20][3][3];
      static double nan;
      static int i,j,k;
      /*
*************************************************
*/
      nan = 0.0e0;
      for(i=0; i<20; i++) {
         nan = nan+sum[i];
      }
      for(i=0; i<3; i++) {
         for(j=0; j<20; j++) {
            tt[j][i] = (double)(*(khitp+i+j*3)+*(khite+i+j*3))/sum[j];
            tu[j][i] = (double)(*(khitp+i+j*3)-*(khite+i+j*3))/sum[j];
         }
      }
      for(i=0; i<3; i++) {
         tae[i] = 0.0e0;
         uae[i] = 0.0e0;
         for(j=0; j<20; j++) {
            tae[i] = tae[i]+(double)(*(khitp+i+j*3)+*(khite+i+j*3));
            uae[i] = uae[i]+(double)(*(khitp+i+j*3)-*(khite+i+j*3));
         }
         tae[i] = tae[i]/nan;
         uae[i] = uae[i]/nan;
      }
      for(i=0; i<3; i++) {
         for(j=0; j<3; j++) {
            for(k=0; k<20; k++) {
               tv[k][j][i] = (*(vp+i+(j+k*3)*3)+*(ve+i+(j+k*3)*3))/sum[k];
               tw[k][j][i] = (*(vp+i+(j+k*3)*3)-*(ve+i+(j+k*3)*3))/sum[k];
            }
         }
      }
      for(i=0; i<3; i++) {
         for(j=0; j<3; j++) {
            *(vae+i+j*3) = 0.0e0;
            *(wae+i+j*3) = 0.0e0;
            for(k=0; k<20; k++) {
               *(vae+i+j*3) = *(vp+i+(j+k*3)*3)+*(ve+i+(j+k*3)*3)+*(vae+i+j*3);
               *(wae+i+j*3) = *(vp+i+(j+k*3)*3)-*(ve+i+(j+k*3)*3)+*(wae+i+j*3);
            }
            *(vae+i+j*3) = *(vae+i+j*3)/nan;
            *(wae+i+j*3) = *(wae+i+j*3)/nan;
         }
      }
      for(k=0; k<20; k++) {
         capar[k] = (float)(tt[k][0]*(double)*r1);
      }
      for(i=0; i<3; i++) {
         for(j=0; j<3; j++) {
            for(k=0; k<20; k++) {
               aar[k][j][i] = (float)(tw[k][j][i]-tu[k][j]*tv[k][j][i]/tt[k][j]);
            }
         }
      }
      for(i=0; i<3; i++) {
         for(j=0; j<3; j++) {
            for(k=0; k<20; k++) {
               aar[k][j][i] = 3.0F**r1**r1*aar[k][j][i];
            }
         }
      }
      mean20(capar,cap,delta_cap);
      for(i=0; i<3; i++) {
         for(j=0; j<3; j++) {
            for(k=0; k<20; k++) {
               capar[k] = aar[k][j][i];
            }
            mean20(capar,(aa+i+j*3),(daa+i+j*3));
         }
      }
      return;
   }

   void mean20(
               float xar[],
               float *x,
               float *delta)
   {
      static float s1,s2;
      static int i;
      /*
*************************************************
*/
      s1 = 0.0F;
      s2 = 0.0F;
      for(i=0; i<20; i++) {
         s1 = s1+xar[i];
         s2 = s2+fifpow(xar[i],2.0F);
      }
      *x = s1/20.0F;
      s1 = s1/20.0F;
      s2 = s2/20.0F;
      s2 = s2-fifpow(s1,2.0F);
      if(s2 < 0.0F) s2 = 0.0F;
      *delta = fifsqrt(s2/20.0F);
      return;
   }

   void accume(
               float t[],
               int kk[],
               int *khitp,
               int *khite,
               double *vp,
               double *ve,
               int *loop)
   {
      /*
        Accumulate statistics on random walks
        It's probably not necessary at present, but
        just in case this ever gets run with very
        large numbers of trajectories, accumulate
        statistics in double precision
      */
      static int j,jj;
      /*
*************************************************
*/
      for(j=0; j<3; j++) {
         if(kk[j] == 1) {
            *(khitp+j+(*loop-1)*3) = *(khitp+j+(*loop-1)*3)+1;
            for(jj=0; jj<3; jj++) {
               *(vp+jj+(j+(*loop-1)*3)*3) = *(vp+jj+(j+(*loop-1)*3)*3)+(double)t[jj];
            }
         }
         else {
            *(khite+j+(*loop-1)*3) = *(khite+j+(*loop-1)*3)+1;
            for(jj=0; jj<3; jj++) {
               *(ve+jj+(j+(*loop-1)*3)*3) = *(ve+jj+(j+(*loop-1)*3)*3)+(double)t[jj];
            }
         }
      }
      return;
   }

   void park(
             int *maxelts,
             int eltype[],
             float *bv,
             float *rotations,
             int *nelts,
             float p[],
             float *r,
             float *r2,
             unsigned int *hit,
             float *tol,
             float bubble[],
             float *bubble_rad,
             int nebtab[],
             int *nneb,
             int *ninn,
             float rlist[],
             int *hitelt)
   {
      static float d[3],ds;
      static unsigned int gone,firstpass;
      static float r0;
      static int nearto,i;
      /*
*************************************************
*/
      /*
        --------------------------------------------------------
        Upon entry, the random walker sits at a point p on
        the launch sphere.  r is the radius of the launch
        sphere, and r2 is its square.
        This subroutine lets the walker drift down onto the
        object or else out to infinity.  If it is lost to
        infinity, hit is returned as .false., and if it
        drifts onto the body, hit is returned as .true. and
        p is returned with the point at which the walker hits.
        This is the algorithm obeyed by the subroutine:
        1.  If the point p lies inside the launch sphere, proceed
        to step 2.  If it lies outside the launch sphere,
        move it onto the launch sphere using the charge-
        outside-a-sphere Green's function, which may also
        move the point off to infinity.  If the point gets
        moved off to infinity, set hit = .false. and
        return.
        2.  The walker is now on or inside the launch sphere,
        but outside the body.  A call to distance returns
        the distance to the body, ds.  If ds is less than
        tol, the program assumes that the walker has
        adsorbed and so hit is set equal to .true. and
        we return.  Otherwise, we jump to the surface
        of the sphere that is centered on the current
        point and has radius ds.
        3.  Loop back to step 1.
        --------------------------------------------------------
      */
      firstpass = true;
      *bubble_rad = -1.0F;           /*  On the first time, a new bubble */
      /*
        is always needed.
      */
   S1:
      /*
        STEP 1:
      */
      r0 = fifpow(p[0],2.0F)+fifpow(p[1],2.0F)+fifpow(p[2],2.0F);
      if(r0 > *r2) {
         if(firstpass) {            /*  No need to call */
            /*
              greensphere, on first
              pass it could only be
              due to round-off that
              we are here.
            */
            gone = false;
         }
         else {
            r0 = fifsqrt(r0);
            greensphere(p,r,&r0,&gone);
            *bubble_rad = -1.0F;   /*  A new bubble is needed */
            /*
              everytime we reposition at
              the launch sphere
            */
         }
         if(gone) {
            *hit = false;
            return;
         }
      }
      /*
        STEP 2:
      */
      distance(maxelts,eltype,bv,nelts,rotations,p,&ds,bubble,bubble_rad,nebtab,nneb,ninn,rlist,&nearto);
      if(ds < *tol) {
         *hit = true;
         *hitelt = nearto;
         return;
      }
      sphere(d,&ds);
      for(i=0; i<3; i++) {
         p[i] = p[i]+d[i];          /*  Tentative new position */
      }
      /*
        STEP 3:
      */
      firstpass = false;
      goto S1;
   }

   void greensphere(
                    float p[],
                    float *r,
                    float *r0,
                    unsigned int *gone)
   {
      static float alpha;
      static int i;
      static float cost,thetr,phir,s,cx,t1,t2,t3,sx,pip,sp,cp,sint,zn,xn,cosp,sinp,yn;
      /*
*************************************************
*/
      /*
        Upon entry, p contains the coordinate of the
        point, r contains the radius of the launch
        sphere, and r0 contains the distance of p
        from the origin.  It is known that p lies
        outside the launch sphere.
        This determines the probability that the
        walker escapes to infinity without ever
        returning to the launch sphere, and lets the
        walker escape with that probablity.  Otherwise,
        it returns the walker to a new point on the
        launch sphere.
        Upon exit, gone = .true. if the walker has
        escaped to infinity.  Otherwise, gone = .false.,
        and p contains the coordinates of the new
        point on the launch sphere.
      */
      alpha = *r/ *r0;
      *gone = ran2() > alpha;
      if(*gone) return;
      for(i=0; i<3; i++) {
         p[i] = p[i]/ *r0;
      }
      /*
        Pull off the numbers needed to transform the p-vector
        to the plus z-axis:
      */
      cost = p[2];
      /*
        Minor correction for round-off errors:
      */
      if(cost > 1.0F) cost = 1.0F;
      if(cost < -1.0F) cost = -1.0F;
      thetr = fifacos(cost);
      phir = fifatan2(p[1],p[0]);
      /*
        With modern fortran implementations, atan2
        has a range of 2*pi, so further correction of
        phir is not necessary
        For the time being, assume a new coordinate system for
        which the radius vector is on the z-axis (thetr and phir
        will permit us to transform back later)
      */
      s = ran2();
      if(s == 0.0F) {
         cx = -1.0F;
      }
      else {
         t1 = (1.0F+fifpow(alpha,2.0F))/(2.0F*alpha);
         t2 = fifpow(1.0F-fifpow(alpha,2.0F),2.0F);
         t3 = fifpow(1.0F-alpha+2.0F*alpha*s,2.0F);
         cx = t1-t2/(2.0F*alpha*t3);
      }
      /*
        Minor correction for roundoff errors:
      */
      if(cx > 1.0F) cx = 1.0F;
      if(cx < -1.0F) cx = -1.0F;
      sx = fifsqrt(1.0F-fifpow(cx,2.0F));
      pip = 2.0F*3.14159265F*ran2();
      sp = fifsin(pip);
      cp = fifcos(pip);
      /*
        This is the new position of the diffusor in the 2nd
        coordinate system
      */
      p[0] = *r*sx*cp;
      p[1] = *r*sx*sp;
      p[2] = *r*cx;
      /*
        Now transform to the first coordinate system, first
        by a rotation about y-axis through thetr
      */
      sint = fifsin(thetr);
      zn = p[2]*cost-p[0]*sint;
      xn = p[2]*sint+p[0]*cost;
      p[0] = xn;
      p[2] = zn;
      /*
        and then by a rotation about z-axis through phir
      */
      cosp = fifcos(phir);
      sinp = fifsin(phir);
      xn = p[0]*cosp-p[1]*sinp;
      yn = p[1]*cosp+p[0]*sinp;
      p[0] = xn;
      p[1] = yn;
      return;
   }

   void reinit(
               int *khitp,
               int *khite,
               double *vp,
               double *ve,
               double sum[])
   {
      /*
        initialize statistical registers
      */
      static int k,i,j;
      /*
*************************************************
*/
      for(k=0; k<20; k++) {
         for(i=0; i<3; i++) {
            for(j=0; j<3; j++) {
               *(vp+i+(j+k*3)*3) = 0.0e0;
               *(ve+i+(j+k*3)*3) = 0.0e0;
            }
            *(khite+i+k*3) = 0;
            *(khitp+i+k*3) = 0;
         }
         sum[k] = 0.0e0;
      }
      return;
   }

   void charge(
               float rt[],
               float *r1,
               int kk[])
   {
      /*
        Assign three charges to random walker, kk(i) = i-th charge
      */
      static int i;
      static float x,p;
      /*
*************************************************
*/
      for(i=0; i<3; i++) {
         x = rt[i]/ *r1;
         p = 0.5F*(1.0F+x);
         if(ran2() < p) {
            kk[i] = 1;
         }
         else {
            kk[i] = -1;
         }
      }
      return;
   }

   void sphere(
               float rt[],
               float *r1)
   {
      static int j;
      static float dd;
      /*
*************************************************
*/
      /*
        Get a point distributed randomly on the
        surface of sphere of radius r1
      */
   S1:
      for(j=0; j<3; j++) {
         rt[j] = ran2()*2.0F-1.0F;
      }
      /*
        GEJ	    dd = rt(1)**2 + rt(2)**2 + rt(3)**2
      */
      dd = rt[0]*rt[0]+rt[1]*rt[1]+rt[2]*rt[2];
      if(dd > 1.0F) goto S1;
      dd = fifsqrt(dd);
      for(j=0; j<3; j++) {
         rt[j] = *r1*rt[j]/dd;
      }
      return;
   }

   void seeder()
   {
      /*
        This program uses the UNIX date command to generate a
        seed for
        the sequence of random numbers
      */
      typedef struct {
         int idum;
      } Crandom;
      static Crandom *Trandom = (Crandom*) Xrandom;
      // static char shore[8];
      /*
*************************************************
*/
      /*
        Sat Oct 29 14:39:43 MDT 2011
        0000000001111111111222222222
        1234567890123456789012345678
      */
//       ftnscopy((shore),2,(dateline+8),2,NULL);
//       ftnscopy((shore+2),2,(dateline+11),2,NULL);
//       ftnscopy((shore+4),2,(dateline+14),2,NULL);
//       ftnscopy((shore+6),2,(dateline+17),2,NULL);
//       READ(INTERNAL,shore,8,LISTIO,INT4,&Trandom->idum,0);
      QString time = QString( "" ).sprintf( "%d%d%d%d",
                                            QTime::currentTime().hour(),
                                            QTime::currentTime().minute(),
                                            QTime::currentTime().second(),
                                            QTime::currentTime().msec() );

      Trandom->idum = time.toInt();
      Trandom->idum = -Trandom->idum;
      return;
   }

   void gettime( char *start )
   {
      typedef struct {
         int nbod;
         int nzno;
         int nznr;
         int nstk;
         int ndfl;
         int nefl;
         int nzh;
         int nih;
         int nsh;
         int nph;
      } Cfilenumbers;
//       static Cfilenumbers *Tfilenumbers = (Cfilenumbers*) Xfilenumbers;
      // static char datest[10],timest[8];
      /*
*************************************************
*/
      /*
        GEJ	open(unit=ndfl,file=dfl,status='old')
        GEJ	read(ndfl,900) start
        GEJ900	format(a28)
        GEJ	close(ndfl)
        GEJ:   Use this algorithm for getting date/time for the random number g
        GEJ:   rather than reading it from file=dfl
        GEJ    Use the pFortran date and time subroutines
      */
//       fifdate(datest,10);
//       fiftime(timest,8);
//       ftnscopy(start,28,"2012/05/25 15:56:10         ",28,NULL);/* Sample date/time string */
//       ftnscopy((start+8),2,(datest+3),2,NULL);
//       ftnscopy((start+11),8,(timest),8,NULL);
//       ftnscopy((start+5),2,(datest),2,NULL);
//       ftnscopy((start+2),2,(datest+6),2,NULL);
      QString time = QDateTime::currentDateTime().toString( "ddd MMM dd hh:mm:ss.zzz yyyy" );
      strncpy( start, time.toLatin1().data(), 28 );
      start[28] = 0;
      return;
   }

   int jrand(
             int *k1,
             int *k2)
   {
      typedef struct {
         int seed;
      } Crandom;
//       static Crandom *Trandom = (Crandom*) Xrandom;
      static int jrand;
      static float rr;
      /*
*************************************************
*/
      if(*k2 < *k1) Stop("args out of order in jrand");
      if(*k1 < 0) Stop("neg. arg in jrand");
      if(*k2 == *k1) {
         jrand = *k1;
      }
      else {
      S1:
         rr = ran2();
         rr = (float)(*k2-*k1+1)*rr+(float)*k1;
         jrand = fifiaint(rr);
         if(jrand < *k1 || jrand > *k2) goto S1;
      }
      return jrand;
   }

   float ran2()
   {
#define im1 2147483563
#define im2 2147483399
#define am (1.0F/(float)im1)
#define imm1 (im1-1)
#define ia1 40014
#define ia2 40692
#define iq1 53668
#define iq2 52774
#define ir1 12211
#define ir2 3791
#define ntab 32
#define ndiv (1+imm1/ntab)
#define eps (1.2e-7F)
#define rnmx (1.0F-eps)
      typedef struct {
         int idum;
      } Crandom;
      static Crandom *Trandom = (Crandom*) Xrandom;
      static int idum2 = 123456789;
      static int iv[ntab] = {
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
      };
      static int iy = 0;
      static float ran2;
      static int j,k;
      /*
****************************************************************
This is the numerical recipes subroutine ran2
(From Numerical Recipes in Fortran 77, 2nd ed., p. 272)
Long period ( > 2e18) random number generator of L'Ecuyer with
Bays-Durham shuffle and added safeguards.  Returns a uniform
random deviate between 0.0 and 1.0 (exclusive of the endpoint
values).  Call with idum a negative integer to initialize;
thereafter, do not alter idum between successive deviates in
a sequence.  rnmx should approximate the largest floating value
that is less than 1.
      */
         if(Trandom->idum <= 0) {
            Trandom->idum = fifmax0(-Trandom->idum,1);
            idum2 = Trandom->idum;
            for(j=ntab+8; j>=1; j--) {
               /*
                 (after 8 warm-ups)
               */
               k = Trandom->idum/iq1;
               Trandom->idum = ia1*(Trandom->idum-k*iq1)-k*ir1;
               if(Trandom->idum < 0) Trandom->idum = Trandom->idum+im1;
               if(j <= ntab) iv[j-1] = Trandom->idum;
            }
            iy = iv[0];
         }
         k = Trandom->idum/iq1;         /*  Start here when not */
         /*
           initializing
         */
         Trandom->idum = ia1*(Trandom->idum-k*iq1)-k*ir1;/*  Compute idum=mod(ia1*dum, */
         /*
           im1) without overflows
           by Schrage's method
         */
   if(Trandom->idum < 0) Trandom->idum = Trandom->idum+im1;
   k = idum2/iq2;
   idum2 = ia2*(idum2-k*iq2)-k*ir2;/*  Likewise for idum2 */
   if(idum2 < 0) idum2 = idum2+im2;
   j = 1+iy/ndiv;                 /*  Will be in range 1:ntab */
   /*
     Now idum is shuffled, idum and idum2 are combined to generate output
   */
   iy = iv[j-1]-idum2;
   iv[j-1] = Trandom->idum;
   if(iy < 1) iy = iy+imm1;
   ran2 = fifamin1(am*(float)iy,rnmx);/*  Do not output endpoint values */
   return ran2;
   }
#undef im1
#undef im2
#undef am
#undef imm1
#undef ia1
#undef ia2
#undef iq1
#undef iq2
#undef ir1
#undef ir2
#undef ntab
#undef ndiv
#undef eps
#undef rnmx

   void wagaroo(
                int *maxelts,
                int eltype[],
                float *bv,
                int *nelts,
                float *rlaunch,
                float *rotations,
                int *m1,
                float *rg2int,
                float *delta_rg2int,
                float *volume,
                float *delta_volume,
                unsigned int *rg_done,
                int *mi,
                char * /* id */ ,
                float q[],
                float sq[],
                float dsq[],
                char * /* round */ ,
                unsigned int *savehits,
                float vaar[],
                double *tten,
                int /* P1 */ ,
                int /* P2 */ )
   {
      /*
        numerical integration of the radius of gyration
        and the volume
      */
      typedef struct {
         unsigned int silent;
      } Cshush;
      static Cshush *Tshush = (Cshush*) Xshush;
      typedef struct {
         int nbod;
         int nzno;
         int nznr;
         int nstk;
         int ndfl;
         int nefl;
         int nzh;
         int nih;
         int nsh;
         int nph;
      } Cfilenumbers;
      static Cfilenumbers *Tfilenumbers = (Cfilenumbers*) Xfilenumbers;
      typedef struct {
         char fbod[USZ_MAX_ID_SIZE];
         char fzno[USZ_MAX_ID_SIZE];
         char fznr[USZ_MAX_ID_SIZE];
         char fstk[USZ_MAX_ID_SIZE];
         char fdfl[USZ_MAX_ID_SIZE];
         char fefl[USZ_MAX_ID_SIZE];
         char fzh[USZ_MAX_ID_SIZE];
         char fih[USZ_MAX_ID_SIZE];
         char fsh[USZ_MAX_ID_SIZE];
         char fph[USZ_MAX_ID_SIZE];
      } Cfilenames;
      static Cfilenames *Tfilenames = (Cfilenames*) Xfilenames;
      static double trials[20],rg2sum[20],rg2norm[20],ttennorm,sqsum[20][82],rij,qd,arg;
      static float pass_vec[12],tumble[3][3],rad[20],rt1[3],rt2[3];
      static unsigned int early;
      static char mess[10],flush[2000];
      static int i,myelt,j,k;
      static float result,ox;
      // static int mout;
      static int ii;
      static int kk1;
      static int kk2;
      static int loop;
      static int jj;
      static int jj1;
      static int jj2;
      static int kk;
      // static int need;
      static int ii1;
      static int ii2;
      static float an0,an1,tn0,tn1;
      /*
*************************************************
*/
      static const char* F871[] = {
         "('  Body element of type: ',a10)"
      };
      static const char* F872[] = {
         "('  is inconsistent with C1-INTERIOR calculation.')"
      };
//       static const char* F900[] = {
//          "('C1 INTERIOR CALCULATION',1x,a2048,i12)"
//       };
//       static const char* F777[] = {
//          "(79('='))"
//       };
      static const char* F4444[] = {
         "(3g20.8)"
      };
//       static const char* F776[] = {
//          "(a2000,$)"
//       };
//       static const char* F774[] = {
//          "(' ')"
//       };
      *volume = 0.0F;
      *delta_volume = 0.0F;
      for(i=0; i<*nelts; i++) {
         myelt = eltype[i];
         for(j=0; j<12; j++) {
            pass_vec[j] = *(bv+i+j**maxelts);
         }
         for(j=0; j<3; j++) {
            for(k=0; k<3; k++) {
               tumble[k][j] = *(rotations+i+(j+k*3)**maxelts);
            }
         }
         primvol(&myelt,pass_vec,(float *)tumble,&result,&early,mess,10);
         vaar[i] = result;
         *volume = *volume+result;
         if(early) {
            WRITE(Tfilenumbers->nzno,FMT,F871,1,STRG,mess,10,0);
            WRITE(Tfilenumbers->nzno,FMT,F872,1,0);
            return;
         }
      }
      if(*savehits) OPEN(Tfilenumbers->nih,FILEN,Tfilenames->fih,USZ_MAX_ID_SIZE,STATUS,"unknown",0);
      *rg_done = false;
      q[0] = 0.0F;
      for(i=1; i<=81; i++) {
         ox = (float)(i-41)/20.0F;
         q[i] = fifpow(10.0F,ox)/ *rlaunch;
      }
      for(i=2; i<=2000; i++) {
         *(flush+i-1) = 0;
         /*
           unprintable characters
           in order to flush the
           output buffer
         */
      }
//       if(!Tshush->silent) WRITE(OUTPUT,FMT,F900,1,STRG,id,USZ_MAX_ID_SIZE,INT4,*m1,0);
//       if(!Tshush->silent) WRITE(OUTPUT,FMT,F777,1,0);
//       mout = 0;
      if (!Tshush->silent) {
         zeno_us_hydrodyn->editor->append( "ZENO interior calculation start\n" );
         qApp->processEvents();
      }

      // I think we can simply parallize on these trials
      // i.e. upto a max of 20 threads

      for(k=0; k<20; k++) {
         trials[k] = 0.0e0;
         rg2sum[k] = 0.0e0;
         rg2norm[k] = 0.0e0;
         for(ii=0; ii<=81; ii++) {
            sqsum[k][ii] = 0.0e0;
         }
      }
      for(kk1=0; kk1<3; kk1++) {
         for(kk2=0; kk2<3; kk2++) {
            *(tten+kk1+kk2*3) = 0.0e0;
         }
      }
      ttennorm = 0.0e0;
      int steps = *m1 / 100;

      for(i=1; i<=*m1; i++) {
         loop = fifmod(i,20)+1;
         /*
           rt1 is the first point
         */
         toss_point(rt1,maxelts,eltype,bv,nelts,rotations,vaar,volume);
         trials[loop-1] = trials[loop-1]+1.0e0;
         /*
           rt2 is the second point
         */
         toss_point(rt2,maxelts,eltype,bv,nelts,rotations,vaar,volume);
         trials[loop-1] = trials[loop-1]+1.0e0;
         /*
           At this point, we have two points, rt1 and rt2, and we know that
           both of them lie inside the body.
         */
         if(*savehits) {
            WRITE(Tfilenumbers->nih,FMT,F4444,1,DO,3,REAL4,rt1,0);
            WRITE(Tfilenumbers->nih,FMT,F4444,1,DO,3,REAL4,rt2,0);
         }
         rij = 0.0e0;
         for(jj=0; jj<3; jj++) {
            rij = rij+pow((double)(rt1[jj]-rt2[jj]),2.0);
            rg2sum[loop-1] = rg2sum[loop-1]+pow((double)(rt1[jj]-rt2[jj]),2.0);
         }
         rg2norm[loop-1] = rg2norm[loop-1]+2.0e0;
         for(jj1=0; jj1<3; jj1++) {
            for(jj2=0; jj2<3; jj2++) {
               *(tten+jj1+jj2*3) = *(tten+jj1+jj2*3)+(double)((rt1[jj1]-rt2[jj1])*(rt1[jj2]-rt2[jj2]));
            }
         }
         ttennorm = ttennorm+2.0e0;
         rij = sqrt(rij);
         for(kk=0; kk<=81; kk++) {
            qd = (double)q[kk];
            arg = qd*rij;
            if(kk == 0) {
               sqsum[loop-1][kk] = sqsum[loop-1][kk]+1.0e0;
            }
            else {
               sqsum[loop-1][kk] = sqsum[loop-1][kk]+sin(arg)/arg;
            }
         }
//          need = fifnint(79.0F*(float)i/(float)*m1);
//          while(mout < need) {
//             *(flush) = *(round+mout+1-1);
//             /*
//               GEJ if (.not.silent) print 776,flush
//             */
//             mout = mout+1;
//          }
         if ( zeno_progress && !( i % steps ) )
         {
            {
               zeno_progress->setValue( zeno_progress->value() + 1 );
               qApp->processEvents();
               if ( zeno_us_hydrodyn->stopFlag )
               {
                  // somehow abort
                  return;
               }
            }
         }
      }
//       if(!Tshush->silent) WRITE(OUTPUT,FMT,F774,1,0);
      for(i=1; i<=20; i++) {
         rad[i-1] = (float)(rg2sum[i-1]/rg2norm[i-1]);
      }
      mean20(rad,rg2int,delta_rg2int);
      for(ii1=0; ii1<3; ii1++) {
         for(ii2=0; ii2<3; ii2++) {
            *(tten+ii1+ii2*3) = *(tten+ii1+ii2*3)/ttennorm;
         }
      }
      for(i=1; i<=20; i++) {
         rad[i-1] = (float)sqsum[i-1][0];
      }
      mean20(rad,&an0,&an1);
      sq[0] = 1.0F;
      dsq[0] = an1/an0;
      for(j=1; j<=81; j++) {
         for(i=1; i<=20; i++) {
            rad[i-1] = (float)sqsum[i-1][j];
         }
         mean20(rad,&tn0,&tn1);
         sq[j] = tn0/an0;
         dsq[j] = tn1/an0;
      }
      *rg_done = true;
      *mi = *m1;
      if(*savehits) CLOSE(Tfilenumbers->nih,0);
      return;
   }

   void toss_point(
                   float rt[],
                   int *maxelts,
                   int eltype[],
                   float *bv,
                   int *nelts,
                   float *rotations,
                   float vaar[],
                   float *volume)
   {
      typedef struct {
         int sphere_code;
         int triangle_code;
         int disk_code;
         int open_cylinder_code;
         int solid_cylinder_code;
         int donut_code;
         int ellipsoid_code;
         int cube_code;
         int pillar_code;
         int skin_code;
         int units_code;
         int hunits_code;
         int meter_code;
         int cm_code;
         int nm_code;
         int angstrom_code;
         int length_code;
         int temp_code;
         int celcius_code;
         int kelvin_code;
         int mass_code;
         int da_code;
         int kda_code;
         int gram_code;
         int kg_code;
         int visc_code;
         int poise_code;
         int cp_code;
         int solvent_code;
         int water_code;
         int rlaunch_code;
         int bf_code;
      } Cmorse;
      static Cmorse *Tmorse = (Cmorse*) Xmorse;
      static float c[3],x[3],y[3],zl[3],zh[3],n[3],n1[3],n2[3],tumble[3][3],zip,sum;
      static int i,kdo;
      static float side,r;
      static int j;
      static float r1,r2;
      static int k;
      static float rad,alen,aa,bb,cc;
      /*
*************************************************
*/
      zip = ran2()**volume;
      sum = 0.0F;
      for(i=1; i<=*nelts; i++) {
         sum = sum+vaar[i-1];
         if(zip < sum) {
            kdo = i;
            goto S1;
         }
      }
      kdo = *nelts;
   S1:
      if(eltype[kdo-1] == Tmorse->cube_code) {
         c[0] = *(bv+kdo-1+0**maxelts);
         c[1] = *(bv+kdo-1+1**maxelts);
         c[2] = *(bv+kdo-1+2**maxelts);
         side = *(bv+kdo-1+3**maxelts);
         toss_cube(c,&side,rt);
      }
      else if(eltype[kdo-1] == Tmorse->pillar_code) {
         x[0] = *(bv+kdo-1+0**maxelts);
         x[1] = *(bv+kdo-1+4**maxelts);
         x[2] = *(bv+kdo-1+8**maxelts);
         y[0] = *(bv+kdo-1+1**maxelts);
         y[1] = *(bv+kdo-1+5**maxelts);
         y[2] = *(bv+kdo-1+9**maxelts);
         zl[0] = *(bv+kdo-1+2**maxelts);
         zl[1] = *(bv+kdo-1+6**maxelts);
         zl[2] = *(bv+kdo-1+10**maxelts);
         zh[0] = *(bv+kdo-1+3**maxelts);
         zh[1] = *(bv+kdo-1+7**maxelts);
         zh[2] = *(bv+kdo-1+11**maxelts);
         toss_pillar(x,y,zl,zh,rt);
      }
      else if(eltype[kdo-1] == Tmorse->sphere_code) {
         c[0] = *(bv+kdo-1+0**maxelts);
         c[1] = *(bv+kdo-1+1**maxelts);
         c[2] = *(bv+kdo-1+2**maxelts);
         r = *(bv+kdo-1+3**maxelts);
         toss_sphere(c,&r,rt);
      }
      else if(eltype[kdo-1] == Tmorse->donut_code) {
         for(j=0; j<3; j++) {
            c[j] = *(bv+i-1+j**maxelts);
            n[j] = *(bv+i-1+(j+3)**maxelts);
         }
         r1 = *(bv+i-1+6**maxelts);
         r2 = *(bv+i-1+7**maxelts);
         for(j=0; j<3; j++) {
            for(k=0; k<3; k++) {
               tumble[k][j] = *(rotations+i-1+(j+k*3)**maxelts);
            }
         }
         toss_torus(c,n,&r1,&r2,(float *)tumble,rt);
      }
      else if(eltype[kdo-1] == Tmorse->solid_cylinder_code) {
         for(j=0; j<3; j++) {
            c[j] = *(bv+i-1+j**maxelts);
            n[j] = *(bv+i-1+(j+3)**maxelts);
         }
         rad = *(bv+i-1+6**maxelts);
         alen = *(bv+i-1+7**maxelts);
         for(j=0; j<3; j++) {
            for(k=0; k<3; k++) {
               tumble[k][j] = *(rotations+i-1+(j+k*3)**maxelts);
            }
         }
         toss_cylinder(c,n,&rad,&alen,(float *)tumble,rt);
      }
      else if(eltype[kdo-1] == Tmorse->ellipsoid_code) {
         for(j=0; j<3; j++) {
            c[j] = *(bv+i-1+j**maxelts);
            n1[j] = *(bv+i-1+(j+3)**maxelts);
            n2[j] = *(bv+i-1+(j+6)**maxelts);
         }
         aa = *(bv+i-1+9**maxelts);
         bb = *(bv+i-1+10**maxelts);
         cc = *(bv+i-1+11**maxelts);
         for(j=0; j<3; j++) {
            for(k=0; k<3; k++) {
               tumble[k][j] = *(rotations+i-1+(j+k*3)**maxelts);
            }
         }
         toss_ellipsoid(c,n1,n2,&aa,&bb,&cc,(float *)tumble,rt);
      }
      else {
         Stop("bad code in toss_point");
      }
      return;
   }

   void toss_sphere(
                    float c[],
                    float *r,
                    float rt[])
   {
      static int i;
      static float sum;
      /*
*************************************************
*/
   S1:
      for(i=0; i<3; i++) {
         rt[i] = (2.0F*ran2()-1.0F)**r;
      }
      sum = fifpow(rt[0],2.0F)+fifpow(rt[1],2.0F)+fifpow(rt[2],2.0F);
      if(sum > fifpow(*r,2.0F)) goto S1;
      for(i=0; i<3; i++) {
         rt[i] = rt[i]+c[i];
      }
      return;
   }

   void toss_cube(
                  float c[],
                  float *side,
                  float rt[])
   {
      static int i;
      /*
*************************************************
*/
      for(i=0; i<3; i++) {
         rt[i] = ran2()**side+c[i];
      }
      return;
   }

   void toss_pillar(
                    float /* x */ [],
                    float /* y */ [],
                    float /* zl */ [],
                    float /* zh */ [],
                    float /* rt */ [])
   {
      /*
*************************************************
*/
      static const char* F800[] = {
         "('SORRY!!!!   toss_pillar not ready')"
      };
      WRITE(OUTPUT,FMT,F800,1,0);
      Stop(NULL);
   }

   void toss_ellipsoid(
                       float /* c */ [],
                       float /* n1 */ [],
                       float /* n2 */ [],
                       float * /* aa */ ,
                       float * /* bb */ ,
                       float * /* cc */ ,
                       float * /* tumble */ ,
                       float /* rt */ [])
   {
      /*
*************************************************
*/
      static const char* F800[] = {
         "('SORRY  ---  toss_ellipsoid not ready')"
      };
      WRITE(OUTPUT,FMT,F800,1,0);
      Stop(NULL);
   }

   void toss_cylinder(
                      float /* c */ [],
                      float /* n */ [],
                      float * /* rad */ ,
                      float * /* alen */ ,
                      float * /* tumble */ ,
                      float /* rt */ [])
   {
      /*
*************************************************
*/
      static const char* F800[] = {
         "('SORRY  ---  toss_cylinder not ready')"
      };
      WRITE(OUTPUT,FMT,F800,1,0);
      Stop(NULL);
   }

   void toss_torus(
                   float /* c */ [],
                   float /* n */ [],
                   float * /* r1 */ ,
                   float * /* r2 */ ,
                   float * /* tumble */ ,
                   float /* rt */ [])
   {
      /*
*************************************************
*/
      static const char* F800[] = {
         "('SORRY!!!!   toss_torus not ready')"
      };
      WRITE(OUTPUT,FMT,F800,1,0);
      Stop(NULL);
   }

   void primvol(
                int *myelt,
                float pass_vec[],
                float * /* tumble */ ,
                float *result,
                unsigned int *early,
                char *mess,
                int /* P1 */ )
   {
      typedef struct {
         int sphere_code;
         int triangle_code;
         int disk_code;
         int open_cylinder_code;
         int solid_cylinder_code;
         int donut_code;
         int ellipsoid_code;
         int cube_code;
         int pillar_code;
         int skin_code;
         int units_code;
         int hunits_code;
         int meter_code;
         int cm_code;
         int nm_code;
         int angstrom_code;
         int length_code;
         int temp_code;
         int celcius_code;
         int kelvin_code;
         int mass_code;
         int da_code;
         int kda_code;
         int gram_code;
         int kg_code;
         int visc_code;
         int poise_code;
         int cp_code;
         int solvent_code;
         int water_code;
         int rlaunch_code;
         int bf_code;
      } Cmorse;
      static Cmorse *Tmorse = (Cmorse*) Xmorse;
      static float x[3],y[3],zl[3],zh[3],pi,side,r,al,r1,r2,aa,bb,cc;
      /*
*************************************************
*/
      pi = 3.14159265F;
      if(*myelt == Tmorse->cube_code) {
         side = pass_vec[3];
         *result = fifpow(side,3.0F);
         ftnscopy(mess,10,"CUBE      ",10,NULL);
         *early = false;
      }
      else if(*myelt == Tmorse->pillar_code) {
         x[0] = pass_vec[0];
         x[1] = pass_vec[4];
         x[2] = pass_vec[8];
         y[0] = pass_vec[1];
         y[1] = pass_vec[5];
         y[2] = pass_vec[9];
         zl[0] = pass_vec[2];
         zl[1] = pass_vec[6];
         zl[2] = pass_vec[10];
         zh[0] = pass_vec[3];
         zh[1] = pass_vec[7];
         zh[2] = pass_vec[11];
         pillarvol(x,y,zl,zh,result);
         ftnscopy(mess,10,"PILLAR    ",10,NULL);
         *early = false;
      }
      else if(*myelt == Tmorse->sphere_code) {
         r = pass_vec[3];
         *result = 4.0F*pi/3.0F*(r*r*r);
         ftnscopy(mess,10,"SPHERE    ",10,NULL);
         *early = false;
      }
      else if(*myelt == Tmorse->triangle_code) {
         *early = true;
         ftnscopy(mess,10,"TRIANGLE  ",10,NULL);
      }
      else if(*myelt == Tmorse->disk_code) {
         *early = true;
         ftnscopy(mess,10,"DISK      ",10,NULL);
      }
      else if(*myelt == Tmorse->open_cylinder_code) {
         *early = true;
         ftnscopy(mess,10,"O_CYLINDER",10,NULL);
      }
      else if(*myelt == Tmorse->solid_cylinder_code) {
         r = pass_vec[6];
         al = pass_vec[7];
         *result = 4.0F*pi*r*r*al;
         *early = false;
         ftnscopy(mess,10,"S_CYLINDER",10,NULL);
      }
      else if(*myelt == Tmorse->donut_code) {
         r1 = pass_vec[6];
         r2 = pass_vec[7];
         *result = 2.0F*pi*pi*r1*r2*r2;
         *early = false;
         ftnscopy(mess,10,"TORUS     ",10,NULL);
      }
      else if(*myelt == Tmorse->ellipsoid_code) {
         aa = pass_vec[9];
         bb = pass_vec[10];
         cc = pass_vec[11];
         *result = 4.0F*pi/3.0F*aa*bb*cc;
         *early = false;
         ftnscopy(mess,10,"ELLIPSOID ",10,NULL);
      }
      else {
         Stop("look-up error in primvol");
      }
      return;
   }
   void pillarvol(
                  float /* x */ [],
                  float /* y */ [],
                  float /* zl */ [],
                  float /* zh */ [],
                  float * /* result */ )
   {
      /*
**********************************************************************
*/
      Stop("pillarvol not ready -- sorry");
   }

   void all_around(
                   int *maxelts,
                   int eltype[],
                   float *bv,
                   int *nelts,
                   float xyzlow[],
                   float xyzhih[],
                   float *rlaunch,
                   float *rotations,
                   int *m1,
                   float *rg2int,
                   float *delta_rg2int,
                   float *volume,
                   float *delta_volume,
                   unsigned int *rg_done,
                   int *mi,
                   char * /* id */,
                   float q[],
                   float sq[],
                   float dsq[],
                   char * /* round */,
                   unsigned int *savehits,
                   double *tten,
                   int /* P1 */ ,
                   int /* P2 */ )
   {
      /*
        numerical integration of the radius of gyration
        and the volume
      */
      typedef struct {
         unsigned int silent;
      } Cshush;
      static Cshush *Tshush = (Cshush*) Xshush;
      typedef struct {
         int nbod;
         int nzno;
         int nznr;
         int nstk;
         int ndfl;
         int nefl;
         int nzh;
         int nih;
         int nsh;
         int nph;
      } Cfilenumbers;
      static Cfilenumbers *Tfilenumbers = (Cfilenumbers*) Xfilenumbers;
      typedef struct {
         char fbod[USZ_MAX_ID_SIZE];
         char fzno[USZ_MAX_ID_SIZE];
         char fznr[USZ_MAX_ID_SIZE];
         char fstk[USZ_MAX_ID_SIZE];
         char fdfl[USZ_MAX_ID_SIZE];
         char fefl[USZ_MAX_ID_SIZE];
         char fzh[USZ_MAX_ID_SIZE];
         char fih[USZ_MAX_ID_SIZE];
         char fsh[USZ_MAX_ID_SIZE];
         char fph[USZ_MAX_ID_SIZE];
      } Cfilenames;
      static Cfilenames *Tfilenames = (Cfilenames*) Xfilenames;
      static double trials[20],rg2sum[20],rg2norm[20],ttennorm,sqsum[20][82],rij,qd,arg;
      static float rad[20],sm1[20],rt1[3],rt2[3];
      static unsigned int inside,early;
      static char mess[10],flush[2000];
      static int i;
      static float ox;
//       static int mout;
      static int k;
      static int ii;
      static int jj1;
      static int jj2;
      static int loop;
      static int jj;
      static int kk;
//       static int need;
      static float volbox;
      static int i1,i2;
      static float an0,an1;
      static int j;
      static float tn0,tn1;
      /*
**********************************************************************
*/
//       static const char* F900[] = {
//          "('INTERIOR CALCULATION',4x,a2048,i12)"
//       };
//       static const char* F777[] = {
//          "(79('='))"
//       };
      static const char* F871[] = {
         "('  Body element of type: ',a10)"
      };
      static const char* F872[] = {
         "('  is inconsistent with INTERIOR calculation.')"
      };
      static const char* F4444[] = {
         "(3g20.8)"
      };
//       static const char* F776[] = {
//          "(a2000,$)"
//       };
//       static const char* F774[] = {
//          "(' ')"
//       };
      if(*savehits) OPEN(Tfilenumbers->nih,FILEN,Tfilenames->fih,USZ_MAX_ID_SIZE,STATUS,"unknown",0);
      *rg_done = false;
      q[0] = 0.0F;
      for(i=1; i<=81; i++) {
         ox = (float)(i-41)/20.0F;
         q[i] = fifpow(10.0F,ox)/ *rlaunch;
      }
      for(i=2; i<=2000; i++) {
         *(flush+i-1) = 0;
         /*
           unprintable characters
           in order to flush the
           output buffer
         */
      }
      // if(!Tshush->silent) WRITE(OUTPUT,FMT,F900,1,STRG,id,USZ_MAX_ID_SIZE,INT4,*m1,0);
      // if(!Tshush->silent) WRITE(OUTPUT,FMT,F777,1,0);
      if (!Tshush->silent) {
         zeno_us_hydrodyn->editor->append( "ZENO interior calculation start\n" );         
         qApp->processEvents();
      }
      // mout = 0;
      for(k=0; k<20; k++) {
         trials[k] = 0.0e0;
         sm1[k] = 0.0e0F;
         rg2sum[k] = 0.0e0;
         rg2norm[k] = 0.0e0;
         for(ii=0; ii<=81; ii++) {
            sqsum[k][ii] = 0.0e0;
         }
      }
      for(jj1=0; jj1<3; jj1++) {
         for(jj2=0; jj2<3; jj2++) {
            *(tten+jj1+jj2*3) = 0.0e0;
         }
      }
      ttennorm = 0.0e0;
      
      int steps = *m1 / 100;

      for(i=1; i<=*m1; i++) {
         loop = fifmod(i,20)+1;
         sm1[loop-1] = (float)((double)sm1[loop-1]+1.0e0);
      S1:
         for(ii=1; ii<=3; ii++) {   /*  This is a stack of */
            rt1[ii-1] = xyzlow[ii-1]+ran2()*(xyzhih[ii-1]-xyzlow[ii-1]);
         }
         /*
           rt1 is the trial point
         */
         inbody(rt1,maxelts,eltype,bv,nelts,rotations,&inside,&early,mess,10);
         if(early) {
            WRITE(Tfilenumbers->nzno,FMT,F871,1,STRG,mess,10,0);
            WRITE(Tfilenumbers->nzno,FMT,F872,1,0);
            if(*savehits) CLOSE(Tfilenumbers->nih,0);
            return;
         }
         trials[loop-1] = trials[loop-1]+1.0e0;
         if(!inside) goto S1;
         sm1[loop-1] = (float)((double)sm1[loop-1]+1.0e0);
      S2:
         for(ii=1; ii<=3; ii++) {
            rt2[ii-1] = xyzlow[ii-1]+ran2()*(xyzhih[ii-1]-xyzlow[ii-1]);
         }
         /*
           rt2 is the second trial point
         */
         inbody(rt2,maxelts,eltype,bv,nelts,rotations,&inside,&early,mess,10);
         trials[loop-1] = trials[loop-1]+1.0e0;
         if(!inside) goto S2;
         /*
           At this point, we have two points, rt1 and rt2, and we know that
           both of them lie inside the body.
         */
         if(*savehits) {
            WRITE(Tfilenumbers->nih,FMT,F4444,1,DO,3,REAL4,rt1,0);
            WRITE(Tfilenumbers->nih,FMT,F4444,1,DO,3,REAL4,rt2,0);
         }
         rij = 0.0e0;
         for(jj=0; jj<3; jj++) {
            rij = rij+pow((double)(rt1[jj]-rt2[jj]),2.0);
            rg2sum[loop-1] = rg2sum[loop-1]+pow((double)(rt1[jj]-rt2[jj]),2.0);
         }
         rg2norm[loop-1] = rg2norm[loop-1]+2.0e0;
         for(jj1=0; jj1<3; jj1++) {
            for(jj2=0; jj2<3; jj2++) {
               *(tten+jj1+jj2*3) = *(tten+jj1+jj2*3)+(double)((rt1[jj1]-rt2[jj1])*(rt1[jj2]-rt2[jj2]));
            }
         }
         ttennorm = ttennorm+2.0e0;
         rij = sqrt(rij);
         for(kk=0; kk<=81; kk++) {
            qd = (double)q[kk];
            arg = qd*rij;
            if(kk == 0) {
               sqsum[loop-1][kk] = sqsum[loop-1][kk]+1.0e0;
            }
            else {
               sqsum[loop-1][kk] = sqsum[loop-1][kk]+sin(arg)/arg;
            }
         }
//          need = fifnint(79.0F*(float)i/(float)*m1);
//          while(mout < need) {
//             *(flush) = *(round+mout+1-1);
//             /*
//               GEJ if (.not.silent) print 776,flush
//             */
//             mout = mout+1;
//          }
         if ( zeno_progress && !( i % steps ) )
         {
            zeno_progress->setValue( zeno_progress->value() + 1 );
            qApp->processEvents();
            if ( zeno_us_hydrodyn->stopFlag )
            {
               // somehow abort
               return;
            }
         }
         
      }
//       if(!Tshush->silent) WRITE(OUTPUT,FMT,F774,1,0);
      volbox = 1.0F;
      for(i=1; i<=3; i++) {
         volbox = volbox*(xyzhih[i-1]-xyzlow[i-1]);
      }
      for(i=1; i<=20; i++) {
         rad[i-1] = volbox*(float)((double)sm1[i-1]/trials[i-1]);
      }
      mean20(rad,volume,delta_volume);
      for(i=1; i<=20; i++) {
         rad[i-1] = (float)(rg2sum[i-1]/rg2norm[i-1]);
      }
      mean20(rad,rg2int,delta_rg2int);
      for(i1=0; i1<3; i1++) {
         for(i2=0; i2<3; i2++) {
            *(tten+i1+i2*3) = *(tten+i1+i2*3)/ttennorm;
         }
      }
      for(i=1; i<=20; i++) {
         rad[i-1] = (float)sqsum[i-1][0];
      }
      mean20(rad,&an0,&an1);
      sq[0] = 1.0F;
      dsq[0] = an1/an0;
      for(j=1; j<=81; j++) {
         for(i=1; i<=20; i++) {
            rad[i-1] = (float)sqsum[i-1][j];
         }
         mean20(rad,&tn0,&tn1);
         sq[j] = tn0/an0;
         dsq[j] = tn1/an0;
      }
      *rg_done = true;
      *mi = *m1;
      if(*savehits) CLOSE(Tfilenumbers->nih,0);
      return;
   }

   void inbody(
               float rt[],
               int *maxelts,
               int eltype[],
               float *bv,
               int *nelts,
               float *rotations,
               unsigned int *inside,
               unsigned int *early,
               char *mess,
               int /* P1 */ )
   {
      /*
        Returns inside as true if the point rt lies inside the body
      */
      typedef struct {
         int sphere_code;
         int triangle_code;
         int disk_code;
         int open_cylinder_code;
         int solid_cylinder_code;
         int donut_code;
         int ellipsoid_code;
         int cube_code;
         int pillar_code;
         int skin_code;
         int units_code;
         int hunits_code;
         int meter_code;
         int cm_code;
         int nm_code;
         int angstrom_code;
         int length_code;
         int temp_code;
         int celcius_code;
         int kelvin_code;
         int mass_code;
         int da_code;
         int kda_code;
         int gram_code;
         int kg_code;
         int visc_code;
         int poise_code;
         int cp_code;
         int solvent_code;
         int water_code;
         int rlaunch_code;
         int bf_code;
      } Cmorse;
      static Cmorse *Tmorse = (Cmorse*) Xmorse;
      static float x[3],y[3],zl[3],zh[3],c[3],n[3],n1[3],n2[3],t[3][3];
      static int i,j;
      static float side,r,r1,r2;
      static int k;
      static float rad,alen,aa,bb,cc;
      /*
*************************************************
*/
      *inside = false;
      *early = false;
      for(i=0; i<*nelts; i++) {
         if(eltype[i] == Tmorse->pillar_code) {
            x[0] = *(bv+i+0**maxelts);
            x[1] = *(bv+i+4**maxelts);
            x[2] = *(bv+i+8**maxelts);
            y[0] = *(bv+i+1**maxelts);
            y[1] = *(bv+i+5**maxelts);
            y[2] = *(bv+i+9**maxelts);
            zl[0] = *(bv+i+2**maxelts);
            zl[1] = *(bv+i+6**maxelts);
            zl[2] = *(bv+i+10**maxelts);
            zh[0] = *(bv+i+3**maxelts);
            zh[1] = *(bv+i+7**maxelts);
            zh[2] = *(bv+i+11**maxelts);
            insidepillar(x,y,zl,zh,rt,inside);
            if(*inside) return;
         }
         if(eltype[i] == Tmorse->cube_code) {
            for(j=0; j<3; j++) {
               c[j] = *(bv+i+j**maxelts);
            }
            side = *(bv+i+3**maxelts);
            insidecube(c,&side,rt,inside);
            if(*inside) return;
         }
         if(eltype[i] == Tmorse->sphere_code) {
            for(j=0; j<3; j++) {
               c[j] = *(bv+i+j**maxelts);
            }
            r = *(bv+i+3**maxelts);
            insidesphere(c,&r,rt,inside);
            if(*inside) return;
         }
         if(eltype[i] == Tmorse->triangle_code) {
            ftnscopy(mess,10,"TRIANGLE  ",10,NULL);
            *early = true;
            return;
         }
         if(eltype[i] == Tmorse->disk_code) {
            ftnscopy(mess,10,"DISK      ",10,NULL);
            *early = true;
            return;
         }
         if(eltype[i] == Tmorse->open_cylinder_code) {
            ftnscopy(mess,10,"O-CYLINDER",10,NULL);
            *early = true;
            return;
         }
         if(eltype[i] == Tmorse->donut_code) {
            for(j=0; j<3; j++) {
               c[j] = *(bv+i+j**maxelts);
               n[j] = *(bv+i+(j+3)**maxelts);
            }
            r1 = *(bv+i+6**maxelts);
            r2 = *(bv+i+7**maxelts);
            for(j=0; j<3; j++) {
               for(k=0; k<3; k++) {
                  t[k][j] = *(rotations+i+(j+k*3)**maxelts);
               }
            }
            insidetorus(c,n,&r1,&r2,(float *)t,rt,inside);
            if(*inside) return;
         }
         if(eltype[i] == Tmorse->solid_cylinder_code) {
            for(j=0; j<3; j++) {
               c[j] = *(bv+i+j**maxelts);
               n[j] = *(bv+i+(j+3)**maxelts);
            }
            rad = *(bv+i+6**maxelts);
            alen = *(bv+i+7**maxelts);
            for(j=0; j<3; j++) {
               for(k=0; k<3; k++) {
                  t[k][j] = *(rotations+i+(j+k*3)**maxelts);
               }
            }
            insidesolcyl(c,n,&rad,&alen,(float *)t,rt,inside);
            if(*inside) return;
         }
         if(eltype[i] == Tmorse->ellipsoid_code) {
            for(j=0; j<3; j++) {
               c[j] = *(bv+i+j**maxelts);
               n1[j] = *(bv+i+(j+3)**maxelts);
               n2[j] = *(bv+i+(j+6)**maxelts);
            }
            aa = *(bv+i+9**maxelts);
            bb = *(bv+i+10**maxelts);
            cc = *(bv+i+11**maxelts);
            for(j=0; j<3; j++) {
               for(k=0; k<3; k++) {
                  t[k][j] = *(rotations+i+(j+k*3)**maxelts);
               }
            }
            insideellipsoid(c,n1,n2,&aa,&bb,&cc,(float *)t,rt,inside);
         }
      }
      return;
   }

   void captain(
                int *maxelts,
                int eltype[],
                float *bv,
                int *nelts,
                int *m1do,
                float *rotations,
                unsigned int *kirk_done,
                float saar[],
                float *kirk,
                float *delta_kirk,
                float *surface,
                float *delta_surface,
                float *rg2surf,
                float *delta_rg2surf,
                int *ms,
                char * /* id */ ,
                char * /* round */ ,
                unsigned int *savehits,
                int /* P1 */ ,
                int /* P2 */ )
   {
      typedef struct {
         int sphere_code;
         int triangle_code;
         int disk_code;
         int open_cylinder_code;
         int solid_cylinder_code;
         int donut_code;
         int ellipsoid_code;
         int cube_code;
         int pillar_code;
         int skin_code;
         int units_code;
         int hunits_code;
         int meter_code;
         int cm_code;
         int nm_code;
         int angstrom_code;
         int length_code;
         int temp_code;
         int celcius_code;
         int kelvin_code;
         int mass_code;
         int da_code;
         int kda_code;
         int gram_code;
         int kg_code;
         int visc_code;
         int poise_code;
         int cp_code;
         int solvent_code;
         int water_code;
         int rlaunch_code;
         int bf_code;
      } Cmorse;
      static Cmorse *Tmorse = (Cmorse*) Xmorse;
      typedef struct {
         unsigned int silent;
      } Cshush;
      static Cshush *Tshush = (Cshush*) Xshush;
      typedef struct {
         int nbod;
         int nzno;
         int nznr;
         int nstk;
         int ndfl;
         int nefl;
         int nzh;
         int nih;
         int nsh;
         int nph;
      } Cfilenumbers;
      static Cfilenumbers *Tfilenumbers = (Cfilenumbers*) Xfilenumbers;
      typedef struct {
         char fbod[USZ_MAX_ID_SIZE];
         char fzno[USZ_MAX_ID_SIZE];
         char fznr[USZ_MAX_ID_SIZE];
         char fstk[USZ_MAX_ID_SIZE];
         char fdfl[USZ_MAX_ID_SIZE];
         char fefl[USZ_MAX_ID_SIZE];
         char fzh[USZ_MAX_ID_SIZE];
         char fih[USZ_MAX_ID_SIZE];
         char fsh[USZ_MAX_ID_SIZE];
         char fph[USZ_MAX_ID_SIZE];
      } Cfilenames;
      static Cfilenames *Tfilenames = (Cfilenames*) Xfilenames;
      static float v1[3],v2[3];
      static double sum1[20],sum2[20],trials[20],successes[20],rg2sum[20],rg2norm[20];
      static float total,rad[20];
      static char flush[2000];
      static int npil;
      static int ncub;
      static int i;
//       static int mout;
      static int loop;
      static float rr2;
      static int j;
      static float rr;
      // static int need;
      static int k;
      static float rho,delta_rho;
      /*
**********************************************************************
*/
      static const char* F600[] = {
         "('SURFACE integration may be unreliable whenever there')"
      };
      static const char* F601[] = {
         "('are abutting pillars.')"
      };
      static const char* F602[] = {
         "('are abutting cubes.')"
      };
//       static const char* F900[] = {
//          "('SURFACE CALCULATION',5x,a20485,i12)"
//       };
//       static const char* F777[] = {
//          "(79('='))"
//       };
      static const char* F4444[] = {
         "(3g20.8)"
      };
//       static const char* F776[] = {
//          "(a2000,$)"
//       };
//       static const char* F700[] = {
//          "(' ')"
//       };
      if(*savehits) OPEN(Tfilenumbers->nsh,FILEN,Tfilenames->fsh,USZ_MAX_ID_SIZE,STATUS,"unknown",0);
      *kirk_done = false;
      npil = 0;
      ncub = 0;
      for(i=0; i<*nelts; i++) {
         if(eltype[i] == Tmorse->pillar_code) npil = npil+1;
         if(eltype[i] == Tmorse->cube_code) ncub = ncub+1;
      }
      if(npil > 1) {
         WRITE(Tfilenumbers->nzno,FMT,F600,1,0);
         WRITE(Tfilenumbers->nzno,FMT,F601,1,0);
      }
      if(ncub > 1) {
         WRITE(Tfilenumbers->nzno,FMT,F600,1,0);
         WRITE(Tfilenumbers->nzno,FMT,F602,1,0);
      }
      carea(maxelts,eltype,bv,nelts,saar,&total);
      for(i=0; i<20; i++) {
         sum1[i] = 0.0e0;
         sum2[i] = 0.0e0;
         trials[i] = 0.0e0;
         successes[i] = 0.0e0;
         rg2sum[i] = 0.0e0;
         rg2norm[i] = 0.0e0;
      }
      for(i=1; i<2000; i++) {
         *(flush+i) = 0;
         /*
           unprintable characters
           in order to flush the
           output buffer
         */
      }
//       if(!Tshush->silent) WRITE(OUTPUT,FMT,F900,1,STRG,id,USZ_MAX_ID_SIZE,INT4,*m1do,0);
//       if(!Tshush->silent) WRITE(OUTPUT,FMT,F777,1,0);
//       mout = 0;
      if (!Tshush->silent) {
         zeno_us_hydrodyn->editor->append( "ZENO surface calculation start\n" );
         qApp->processEvents();
      }

      int steps = *m1do / 100;

      for(i=1; i<=*m1do; i++) {
         loop = fifmod(i,20)+1;
         getsurface(maxelts,eltype,bv,nelts,saar,&total,v1,trials,rotations,&loop);
         getsurface(maxelts,eltype,bv,nelts,saar,&total,v2,trials,rotations,&loop);
         if(*savehits) {
            WRITE(Tfilenumbers->nsh,FMT,F4444,1,DO,3,REAL4,v1,0);
            WRITE(Tfilenumbers->nsh,FMT,F4444,1,DO,3,REAL4,v2,0);
         }
         successes[loop-1] = successes[loop-1]+2.0e0;
         rr2 = 0.0F;
         for(j=0; j<3; j++) {
            rr2 = rr2+fifpow(v1[j]-v2[j],2.0F);
         }
         rr = fifsqrt(rr2);
         sum2[loop-1] = sum2[loop-1]+1.0e0/(double)rr;
         sum1[loop-1] = sum1[loop-1]+1.0e0;
         rg2sum[loop-1] = rg2sum[loop-1]+(double)rr2;
         rg2norm[loop-1] = rg2norm[loop-1]+2.0e0;
         //         need = fifnint(79.0F*(float)i/(float)*m1do);
//          while(mout < need) {
//             *(flush) = *(round+mout+1-1);
//             /*
//               GEJ if (.not.silent) print 776,flush
//             */
//             mout = mout+1;
//          }
         if ( zeno_progress && !( i % steps ) )
         {
            {
               zeno_progress->setValue( zeno_progress->value() + 1 );
               qApp->processEvents();
               if ( zeno_us_hydrodyn->stopFlag )
               {
                  // somehow abort
                  return;
               }
            }
         }
      }
//       if(!Tshush->silent) WRITE(OUTPUT,FMT,F700,1,0);
      for(k=0; k<20; k++) {
         rad[k] = total*(float)(successes[k]/trials[k]);
      }
      mean20(rad,surface,delta_surface);
      for(i=1; i<=20; i++) {
         rad[i-1] = (float)(rg2sum[i-1]/rg2norm[i-1]);
      }
      mean20(rad,rg2surf,delta_rg2surf);
      for(k=0; k<20; k++) {
         rad[k] = (float)(sum2[k]/sum1[k]);
      }
      mean20(rad,&rho,&delta_rho);
      *kirk = 1.0F/rho;
      *delta_kirk = delta_rho/fifpow(rho,2.0F);
      *kirk_done = true;
      *ms = *m1do;
      if(*savehits) CLOSE(Tfilenumbers->nsh,0);
      return;
   }

   void carea(
              int *maxelts,
              int eltype[],
              float *bv,
              int *nelts,
              float saar[],
              float *total)
   {
      typedef struct {
         int sphere_code;
         int triangle_code;
         int disk_code;
         int open_cylinder_code;
         int solid_cylinder_code;
         int donut_code;
         int ellipsoid_code;
         int cube_code;
         int pillar_code;
         int skin_code;
         int units_code;
         int hunits_code;
         int meter_code;
         int cm_code;
         int nm_code;
         int angstrom_code;
         int length_code;
         int temp_code;
         int celcius_code;
         int kelvin_code;
         int mass_code;
         int da_code;
         int kda_code;
         int gram_code;
         int kg_code;
         int visc_code;
         int poise_code;
         int cp_code;
         int solvent_code;
         int water_code;
         int rlaunch_code;
         int bf_code;
      } Cmorse;
      static Cmorse *Tmorse = (Cmorse*) Xmorse;
      static float v1[3],v2[3],v3[3],x[3],y[3],zl[3],zh[3],pi;
      static int i;
      static float side,sa,bottom,top,side12,side13,side23,r;
      static int j;
      static float al,r1,r2,aa,bb,cc;
      /*
**********************************************************************
*/
      pi = 3.14159265F;
      *total = 0.0F;
      for(i=0; i<*nelts; i++) {
         if(eltype[i] == Tmorse->cube_code) {
            side = *(bv+i+3**maxelts);
            sa = 6.0F*side*side;
         }
         if(eltype[i] == Tmorse->pillar_code) {
            x[0] = *(bv+i+0**maxelts);
            x[1] = *(bv+i+4**maxelts);
            x[2] = *(bv+i+8**maxelts);
            y[0] = *(bv+i+1**maxelts);
            y[1] = *(bv+i+5**maxelts);
            y[2] = *(bv+i+9**maxelts);
            zl[0] = *(bv+i+2**maxelts);
            zl[1] = *(bv+i+6**maxelts);
            zl[2] = *(bv+i+10**maxelts);
            zh[0] = *(bv+i+3**maxelts);
            zh[1] = *(bv+i+7**maxelts);
            zh[2] = *(bv+i+11**maxelts);
            pillarsurf(x,y,zl,zh,&sa,&bottom,&top,&side12,&side13,&side23);
         }
         if(eltype[i] == Tmorse->sphere_code) {
            r = *(bv+i+3**maxelts);
            sa = 4.0F*pi*r*r;
         }
         if(eltype[i] == Tmorse->triangle_code) {
            for(j=0; j<3; j++) {
               v1[j] = *(bv+i+j**maxelts);
               v2[j] = *(bv+i+(j+3)**maxelts);
               v3[j] = *(bv+i+(j+6)**maxelts);
            }
            trisurf(v1,v2,v3,&sa);
         }
         if(eltype[i] == Tmorse->disk_code) {
            r = *(bv+i+6**maxelts);
            sa = pi*r*r;
         }
         if(eltype[i] == Tmorse->open_cylinder_code) {
            r = *(bv+i+6**maxelts);
            al = *(bv+i+7**maxelts);
            sa = 2.0F*pi*al*r;
         }
         if(eltype[i] == Tmorse->solid_cylinder_code) {
            r = *(bv+i+6**maxelts);
            al = *(bv+i+7**maxelts);
            sa = 2.0F*pi*al*r;
            sa = sa+2.0F*pi*r*r;
         }
         if(eltype[i] == Tmorse->donut_code) {
            r1 = *(bv+i+6**maxelts);
            r2 = *(bv+i+7**maxelts);
            sa = 4.0F*pi*pi*r1*r2;
         }
         if(eltype[i] == Tmorse->ellipsoid_code) {
            aa = *(bv+i+9**maxelts);
            bb = *(bv+i+10**maxelts);
            cc = *(bv+i+11**maxelts);
            ellsurf(&aa,&bb,&cc,&sa);
         }
         saar[i] = sa;
         *total = *total+sa;
      }
      return;
   }

   void getsurface(
                   int *maxelts,
                   int eltype[],
                   float *bv,
                   int *nelts,
                   float saar[],
                   float *total,
                   float p1[],
                   double trials[],
                   float *rotations,
                   int *loop)
   {
      /*
        Generate a point, p1, distributed uniformly over the surface
      */
      typedef struct {
         int sphere_code;
         int triangle_code;
         int disk_code;
         int open_cylinder_code;
         int solid_cylinder_code;
         int donut_code;
         int ellipsoid_code;
         int cube_code;
         int pillar_code;
         int skin_code;
         int units_code;
         int hunits_code;
         int meter_code;
         int cm_code;
         int nm_code;
         int angstrom_code;
         int length_code;
         int temp_code;
         int celcius_code;
         int kelvin_code;
         int mass_code;
         int da_code;
         int kda_code;
         int gram_code;
         int kg_code;
         int visc_code;
         int poise_code;
         int cp_code;
         int solvent_code;
         int water_code;
         int rlaunch_code;
         int bf_code;
      } Cmorse;
      static Cmorse *Tmorse = (Cmorse*) Xmorse;
      static float c[3],n[3],v1[3],v2[3],v3[3],x[3],y[3],zl[3],zh[3],t[3][3],n1[3],n2[3];
      static unsigned int inside;
      static float zip,sum;
      static int i,kdo,j;
      static float side,r;
      static int k;
      static float al,r1,r2,aa,bb,cc,rt,rad,alen;
      /*
**********************************************************************
*/
   S100:
      trials[*loop-1] = trials[*loop-1]+1.0e0;
      zip = ran2()**total;
      sum = 0.0F;
      for(i=1; i<=*nelts; i++) {
         sum = sum+saar[i-1];
         if(zip < sum) {
            kdo = i;
            goto S1;
         }
      }
      kdo = *nelts;
   S1:
      i = kdo;
      if(eltype[kdo-1] == Tmorse->cube_code) {
         for(j=0; j<3; j++) {
            c[j] = *(bv+i-1+j**maxelts);
         }
         side = *(bv+i-1+3**maxelts);
         dovercube(c,&side,p1);
      }
      if(eltype[kdo-1] == Tmorse->pillar_code) {
         x[0] = *(bv+i-1+0**maxelts);
         x[1] = *(bv+i-1+4**maxelts);
         x[2] = *(bv+i-1+8**maxelts);
         y[0] = *(bv+i-1+1**maxelts);
         y[1] = *(bv+i-1+5**maxelts);
         y[2] = *(bv+i-1+9**maxelts);
         zl[0] = *(bv+i-1+2**maxelts);
         zl[1] = *(bv+i-1+6**maxelts);
         zl[2] = *(bv+i-1+10**maxelts);
         zh[0] = *(bv+i-1+3**maxelts);
         zh[1] = *(bv+i-1+7**maxelts);
         zh[2] = *(bv+i-1+11**maxelts);
         doverpillar(x,y,zl,zh,p1);
      }
      if(eltype[kdo-1] == Tmorse->sphere_code) {
         for(j=0; j<3; j++) {
            c[j] = *(bv+i-1+j**maxelts);
         }
         r = *(bv+i-1+3**maxelts);
         doversphere(c,&r,p1);
      }
      if(eltype[kdo-1] == Tmorse->triangle_code) {
         for(j=0; j<3; j++) {
            v1[j] = *(bv+i-1+j**maxelts);
            v2[j] = *(bv+i-1+(j+3)**maxelts);
            v3[j] = *(bv+i-1+(j+6)**maxelts);
         }
         dovertriangle(v1,v2,v3,p1);
      }
      if(eltype[kdo-1] == Tmorse->disk_code) {
         for(j=0; j<3; j++) {
            c[j] = *(bv+i-1+j**maxelts);
            n[j] = *(bv+i-1+(j+3)**maxelts);
         }
         r = *(bv+i-1+6**maxelts);
         for(j=0; j<3; j++) {
            for(k=0; k<3; k++) {
               t[k][j] = *(rotations+i-1+(j+k*3)**maxelts);
            }
         }
         doverdisk(c,n,&r,(float *)t,p1);
      }
      if(eltype[kdo-1] == Tmorse->open_cylinder_code) {
         for(j=0; j<3; j++) {
            c[j] = *(bv+i-1+j**maxelts);
            n[j] = *(bv+i-1+(j+3)**maxelts);
         }
         r = *(bv+i-1+6**maxelts);
         al = *(bv+i-1+7**maxelts);
         for(j=0; j<3; j++) {
            for(k=0; k<3; k++) {
               t[k][j] = *(rotations+i-1+(j+k*3)**maxelts);
            }
         }
         dovercylinder(c,n,&r,&al,(float *)t,p1);
      }
      if(eltype[kdo-1] == Tmorse->solid_cylinder_code) {
         for(j=0; j<3; j++) {
            c[j] = *(bv+i-1+j**maxelts);
            n[j] = *(bv+i-1+(j+3)**maxelts);
         }
         r = *(bv+i-1+6**maxelts);
         al = *(bv+i-1+7**maxelts);
         for(j=0; j<3; j++) {
            for(k=0; k<3; k++) {
               t[k][j] = *(rotations+i-1+(j+k*3)**maxelts);
            }
         }
         doversolcyl(c,n,&r,&al,(float *)t,p1);
      }
      if(eltype[kdo-1] == Tmorse->donut_code) {
         for(j=0; j<3; j++) {
            c[j] = *(bv+i-1+j**maxelts);
            n[j] = *(bv+i-1+(j+3)**maxelts);
         }
         r1 = *(bv+i-1+6**maxelts);
         r2 = *(bv+i-1+7**maxelts);
         for(j=0; j<3; j++) {
            for(k=0; k<3; k++) {
               t[k][j] = *(rotations+i-1+(j+k*3)**maxelts);
            }
         }
         dovertorus(c,n,&r1,&r2,(float *)t,p1);
      }
      if(eltype[kdo-1] == Tmorse->ellipsoid_code) {
         for(j=0; j<3; j++) {
            c[j] = *(bv+i-1+j**maxelts);
            n1[j] = *(bv+i-1+(j+3)**maxelts);
            n2[j] = *(bv+i-1+(j+6)**maxelts);
         }
         aa = *(bv+i-1+9**maxelts);
         bb = *(bv+i-1+10**maxelts);
         cc = *(bv+i-1+11**maxelts);
         for(j=0; j<3; j++) {
            for(k=0; k<3; k++) {
               t[k][j] = *(rotations+i-1+(j+k*3)**maxelts);
            }
         }
         doverellipsoid(c,n1,n2,&aa,&bb,&cc,(float *)t,p1);
      }
      /*
        p1 is now a point distributed over the surface
        of the elements.  But we must remove it if it
        is inside any other element.
      */
      for(i=1; i<=*nelts; i++) {
         if(i != kdo) {
            if(eltype[i-1] == Tmorse->cube_code) {
               for(j=0; j<3; j++) {
                  c[j] = *(bv+i-1+j**maxelts);
               }
               side = *(bv+i-1+3**maxelts);
               insidecube(c,&side,p1,&inside);
               if(inside) goto S100;
            }
            if(eltype[i-1] == Tmorse->pillar_code) {
               x[0] = *(bv+i-1+0**maxelts);
               x[1] = *(bv+i-1+4**maxelts);
               x[2] = *(bv+i-1+8**maxelts);
               y[0] = *(bv+i-1+1**maxelts);
               y[1] = *(bv+i-1+5**maxelts);
               y[2] = *(bv+i-1+9**maxelts);
               zl[0] = *(bv+i-1+2**maxelts);
               zl[1] = *(bv+i-1+6**maxelts);
               zl[2] = *(bv+i-1+10**maxelts);
               zh[0] = *(bv+i-1+3**maxelts);
               zh[1] = *(bv+i-1+7**maxelts);
               zh[2] = *(bv+i-1+11**maxelts);
               insidepillar(x,y,zl,zh,&rt,&inside);
               if(inside) goto S100;
            }
            if(eltype[i-1] == Tmorse->sphere_code) {
               for(j=0; j<3; j++) {
                  c[j] = *(bv+i-1+j**maxelts);
               }
               r = *(bv+i-1+3**maxelts);
               insidesphere(c,&r,p1,&inside);
               if(inside) goto S100;
            }
            if(eltype[i-1] == Tmorse->donut_code) {
               for(j=0; j<3; j++) {
                  c[j] = *(bv+i-1+j**maxelts);
                  n[j] = *(bv+i-1+(j+3)**maxelts);
               }
               r1 = *(bv+i-1+6**maxelts);
               r2 = *(bv+i-1+7**maxelts);
               for(j=0; j<3; j++) {
                  for(k=0; k<3; k++) {
                     t[k][j] = *(rotations+i-1+(j+k*3)**maxelts);
                  }
               }
               insidetorus(c,n,&r1,&r2,(float *)t,p1,&inside);
               if(inside) goto S100;
            }
            if(eltype[i-1] == Tmorse->solid_cylinder_code) {
               for(j=0; j<3; j++) {
                  c[j] = *(bv+i-1+j**maxelts);
                  n[j] = *(bv+i-1+(j+3)**maxelts);
               }
               rad = *(bv+i-1+6**maxelts);
               alen = *(bv+i-1+7**maxelts);
               for(j=0; j<3; j++) {
                  for(k=0; k<3; k++) {
                     t[k][j] = *(rotations+i-1+(j+k*3)**maxelts);
                  }
               }
               insidesolcyl(c,n,&rad,&alen,(float *)t,&rt,&inside);
               if(inside) goto S100;
            }
            if(eltype[i-1] == Tmorse->ellipsoid_code) {
               for(j=0; j<3; j++) {
                  c[j] = *(bv+i-1+j**maxelts);
                  n1[j] = *(bv+i-1+(j+3)**maxelts);
                  n2[j] = *(bv+i-1+(j+6)**maxelts);
               }
               aa = *(bv+i-1+9**maxelts);
               bb = *(bv+i-1+10**maxelts);
               cc = *(bv+i-1+11**maxelts);
               for(j=0; j<3; j++) {
                  for(k=0; k<3; k++) {
                     t[k][j] = *(rotations+i-1+(j+k*3)**maxelts);
                  }
               }
               insideellipsoid(c,n1,n2,&aa,&bb,&cc,(float *)t,p1,&inside);
               if(inside) goto S100;
            }
         }
      }
      return;
   }

   void pillarsurf(
                   float x[],
                   float y[],
                   float zl[],
                   float zh[],
                   float *sa,
                   float *bottom,
                   float *top,
                   float *side12,
                   float *side13,
                   float *side23)
   {
      /*
        Compute surface area of pillar, and also return areas of the
        individual faces
      */
      static float v1[3],v2[3],v3[3],alt2,alt,s1,s2,s3;
      /*
**********************************************************************
*/
      /*
        Use trisurf to get the areas of the top and bottom faces
      */
      v1[0] = x[0];
      v1[1] = y[0];
      v1[2] = zl[0];
      v2[0] = x[1];
      v2[1] = y[1];
      v2[2] = zl[1];
      v3[0] = x[2];
      v3[1] = y[2];
      v3[2] = zl[2];
      trisurf(v1,v2,v3,bottom);
      v1[0] = x[0];
      v1[1] = y[0];
      v1[2] = zh[0];
      v2[0] = x[1];
      v2[1] = y[1];
      v2[2] = zh[1];
      v3[0] = x[2];
      v3[1] = y[2];
      v3[2] = zh[2];
      trisurf(v1,v2,v3,top);
      alt2 = fifpow(x[0]-x[1],2.0F)+fifpow(y[0]-y[1],2.0F);
      alt = fifsqrt(alt2);
      s1 = zh[0]-zl[0];
      s2 = zh[1]-zl[1];
      *side12 = alt*0.5F*(s1+s2);
      alt2 = fifpow(x[0]-x[2],2.0F)+fifpow(y[0]-y[2],2.0F);
      alt = fifsqrt(alt2);
      s1 = zh[0]-zl[0];
      s3 = zh[2]-zl[2];
      *side13 = alt*0.5F*(s1+s3);
      alt2 = fifpow(x[1]-x[2],2.0F)+fifpow(y[1]-y[2],2.0F);
      alt = fifsqrt(alt2);
      s2 = zh[1]-zl[1];
      s3 = zh[2]-zl[2];
      *side23 = alt*0.5F*(s2+s3);
      *sa = *top+*bottom+*side12+*side13+*side23;
      return;
   }

   void trisurf(
                float v1[],
                float v2[],
                float v3[],
                float *sa)
   {
      /*
        Compute surface area of triangle
      */
      static float p23[3],p21[3],q[3],av[3],top,bot,tt,h;
      static int i;
      static float a;
      /*
**********************************************************************
*/
      vector_difference(v2,v3,p23);
      vector_difference(v2,v1,p21);
      dotproduct(p23,p21,&top);
      dotproduct(p21,p21,&bot);
      tt = top/bot;
      h = fifsqrt(bot);
      for(i=0; i<3; i++) {
         q[i] = tt*v1[i]+(1.0F-tt)*v2[i];
      }
      vector_difference(v3,q,av);
      pythag0(av,&a);
      *sa = a*h/2.0F;
      return;
   }

   void ellsurf(
                float *aa,
                float *bb,
                float *cc,
                float *sa)
   {
      /*
        Compute surface area of ellipsoid
        Sort the eigenvalues:
      */
      static float as,bs,cs,pi,phi,t1,t2,hphi;
      static int iphi;
      static float sum,htheta;
      static int itheta;
      static float theta,p,rr;
      /*
**********************************************************************
*/
      as = *aa;
      bs = *bb;
      cs = *cc;
      sort3(&as,&bs,&cs);
      /*
        Evaluate surface integral by double simpsons
      */
      pi = 3.14159265F;
      if(as == bs && bs == cs) {
         *sa = 4.0F*pi*cs*cs;
         return;
      }
      else if(as == bs) {
         phi = fifasin(fifsqrt(1.0F-fifpow(cs/as,2.0F)));
         t1 = fiflog(fiftan(pi/4.0F+phi/2.0F));
         t1 = t1*as*cs*cs/fifsqrt(fifpow(as,2.0F)-fifpow(cs,2.0F));
         t2 = fifsqrt(fifpow(as,2.0F)-2.0F*fifpow(cs,2.0F)+fifpow(cs,4.0F)/fifpow(as,2.0F));
         t2 = t2*as;
         *sa = 2.0F*pi*(fifpow(cs,2.0F)+t1+t2);
         return;
      }
      else if(bs == cs) {
         phi = fifasin(fifsqrt(1.0F-fifpow(cs/as,2.0F)));
         t1 = cs*as*as/fifsqrt(fifpow(as,2.0F)-fifpow(cs,2.0F));
         *sa = 2.0F*pi*(fifpow(cs,2.0F)+t1*phi);
         return;
      }
      *sa = 0.0F;
      hphi = 2.0F*pi/1000.0F;
      for(iphi=0; iphi<=1000; iphi++) {
         phi = (float)iphi*hphi;
         sum = 0.0F;
         htheta = pi/1000.0F;
         for(itheta=0; itheta<=1000; itheta++) {
            theta = (float)itheta*htheta;
            p = fifpow(bs*cs*fifsin(theta)*fifcos(phi),2.0F);
            p = p+fifpow(as*cs*fifsin(theta)*fifsin(phi),2.0F);
            p = p+fifpow(as*bs*fifcos(theta),2.0F);
            p = fifsin(theta)*fifsqrt(p);
            if(itheta == 0) {
               rr = 1.0F;
            }
            else if(itheta == 1000) {
               rr = 1.0F;
            }
            else if(fifmod(itheta,2) == 1) {
               rr = 4.0F;
            }
            else {
               rr = 2.0F;
            }
            sum = sum+rr*p;
         }
         sum = htheta*sum/3.0F;
         if(iphi == 0) {
            rr = 1.0F;
         }
         else if(iphi == 1000) {
            rr = 1.0F;
         }
         else if(fifmod(iphi,2) == 1) {
            rr = 4.0F;
         }
         else {
            rr = 2.0F;
         }
         *sa = *sa+rr*sum;
      }
      *sa = hphi**sa/3.0F;
      return;
   }

   void dovercube(
                  float c[],
                  float *side,
                  float p1[])
   {
      /*
        Generate point distributed over a cube
      */
      static int K1 = 1;
      static int K2 = 3;
      static int K3 = 2;
      static int idir,kk,is;
      /*
**********************************************************************
*/
      idir = jrand(&K1,&K2);
      for(kk=1; kk<=3; kk++) {
         if(kk != idir) {
            p1[kk-1] = *side*(ran2()-0.5F);
         }
         else {
            is = jrand(&K1,&K3);
            if(is == 1) {
               p1[kk-1] = -(*side/2.0F);
            }
            else {
               p1[kk-1] = *side/2.0F;
            }
         }
         p1[kk-1] = p1[kk-1]+c[kk-1]+*side/2.0F;
      }
      return;
   }

   void doversphere(
                    float c[],
                    float *r,
                    float p1[])
   {
      /*
        Generate a point distributed randomly over sphere
      */
      static int i;
      /*
**********************************************************************
*/
      sphere(p1,r);
      for(i=0; i<3; i++) {
         p1[i] = p1[i]+c[i];
      }
      return;
   }

   void dovertriangle(
                      float p1[],
                      float p2[],
                      float p3[],
                      float tt[])
   {
      /*
        Generate a point distributed randomly over a triangle
      */
      static float p23[3],p21[3],q[3],av[3],p31[3],t1[3],top,bot,tat;
      static int i;
      static float a,th,tl,ts,randy,aa1,bb1,cc1,aa2,bb2,cc2,gamma2,gamma3;
      /*
**********************************************************************
*/
      vector_difference(p2,p3,p23);
      vector_difference(p2,p1,p21);
      dotproduct(p23,p21,&top);
      dotproduct(p21,p21,&bot);
      tat = top/bot;
      for(i=0; i<3; i++) {
         q[i] = tat*p1[i]+(1.0F-tat)*p2[i];
      }
      /*
        av = altitude vector normal to (p1...p2) side
      */
      vector_difference(p3,q,av);    /* nm */
      /*
        a = altitude normal to (p1...p2) side
      */
      pythag0(av,&a);                /* nm */
      th = fifamax1(1.0F,tat);       /* nm**0 */
      tl = fifamin1(0.0F,tat);       /* nm**0 */
   S1:
      ts = tl+ran2()*(th-tl);        /* nm**0 */
      randy = ran2();
      for(i=0; i<3; i++) {
         tt[i] = ts*p1[i]+(1.0F-ts)*p2[i]+randy*av[i];/* nm */
      }
      vector_difference(tt,p1,t1);   /* nm */
      dotproduct(t1,p21,&aa1);       /* nm**2 */
      bb1 = bot;                     /* nm**2 */
      vector_difference(p3,p1,p31);  /* nm */
      dotproduct(p31,p21,&cc1);      /* nm**2 */
      dotproduct(t1,p31,&aa2);       /* nm**2 */
      bb2 = cc1;                     /* nm**2 */
      dotproduct(p31,p31,&cc2);      /* nm**2 */
      gamma2 = (aa1*cc2-cc1*aa2)/(bb1*cc2-cc1*bb2);/* nm**0 */
      if(gamma2 < 0.0F) goto S1;
      gamma3 = (bb1*aa2-aa1*bb2)/(bb1*cc2-cc1*bb2);/* nm**0 */
      if(gamma3 < 0.0F) goto S1;
      if(gamma2+gamma3 > 1.0F) goto S1;
      return;
   }

   void doverdisk(
                  float c[],
                  float /* n */ [],
                  float *r,
                  float *t,
                  float p1[])
   {
      /*
        generate point distributed randomly over circular disk
      */
      static float xx;
      /*
**********************************************************************
*/
   S1:
      p1[0] = 2.0F**r*(ran2()-0.5F);
      p1[1] = 2.0F**r*(ran2()-0.5F);
      xx = fifpow(p1[0],2.0F)+fifpow(p1[1],2.0F);
      xx = fifsqrt(xx);
      if(xx > *r) goto S1;
      p1[2] = 0.0F;
      backtransform(c,t,p1);
      return;
   }

   void backtransform(
                      float c[],
                      float *t,
                      float p1[])
   {
      static float v[3];
      static int i,j;
      /*
**********************************************************************
*/
      /*
        Since it is the reverse rotation, use the
        transpose
      */
      for(i=0; i<3; i++) {
         v[i] = 0.0F;
         for(j=0; j<3; j++) {
            v[i] = v[i]+*(t+j+i*3)*p1[j];
         }
      }
      for(i=0; i<3; i++) {
         p1[i] = v[i]+c[i];
      }
      return;
   }

   void doversolcyl(
                    float c[],
                    float n[],
                    float *r,
                    float *al,
                    float *t,
                    float p1[])
   {
      /*
        distribute a point randomly over cylinder
      */
      static float v1[3],pi,scyl,sfac,tot,diddle;
      static int i;
      /*
**********************************************************************
*/
      pi = 3.14159265F;
      scyl = 2.0F*pi**r**al;
      sfac = pi**r**r;
      tot = scyl+sfac+sfac;
      diddle = ran2()*tot;
      if(diddle < scyl) {
         dovercylinder(c,n,r,al,t,p1);
      }
      else if(diddle < scyl+sfac) {
         for(i=0; i<3; i++) {
            v1[i] = c[i]+*al*n[i]/2.0F;
         }
         doverdisk(v1,n,r,t,p1);
      }
      else {
         for(i=0; i<3; i++) {
            v1[i] = c[i]-*al*n[i]/2.0F;
         }
         doverdisk(v1,n,r,t,p1);
      }
      return;
   }

   void dovercylinder(
                      float c[],
                      float /* n */ [],
                      float *r,
                      float *al,
                      float *t,
                      float p1[])
   {
      /*
        distribute a point randomly over cylinder
      */
      static float theta;
      /*
**********************************************************************
*/
      p1[2] = *al*(ran2()-0.5F);
      theta = 2.0F*3.14159265F*ran2();
      p1[0] = *r*fifcos(theta);
      p1[1] = *r*fifsin(theta);
      backtransform(c,t,p1);
      return;
   }

   void dovertorus(
                   float c[],
                   float /* n */ [],
                   float *r1,
                   float *r2,
                   float *t,
                   float p1[])
   {
      /*
        distribute a point randomly over torus
      */
      static float p2[3],pi,theta,rstretch,rmax,probkeep,phi,rtest;
      /*
**********************************************************************
*/
      pi = 3.14159265F;
   S1:
      theta = ran2()*2.0F*pi;
      /*
        Generate a point on a circle in the x-z plane
      */
      p2[0] = *r2*fifcos(theta)+*r1;
      p2[1] = 0.0F;
      p2[2] = *r2*fifsin(theta);
      /*
        Record stretching data
      */
      rstretch = p2[0];
      rmax = *r1+*r2;
      probkeep = rstretch/rmax;
      /*
        Rotate about z-axis through random angle phi
      */
      phi = ran2()*2.0F*pi;
      p1[0] = fifcos(phi)*p2[0]-fifsin(phi)*p2[1];
      p1[1] = fifsin(phi)*p2[0]+fifcos(phi)*p2[1];
      p1[2] = p2[2];
      rtest = ran2();
      if(rtest > probkeep) goto S1;
      backtransform(c,t,p1);
      return;
   }

   void doverellipsoid(
                       float c[],
                       float /* n1 */ [],
                       float /* n2 */ [],
                       float *aa,
                       float *bb,
                       float *cc,
                       float *t,
                       float p1[])
   {
      /*
        distribute a point randomly over ellipsoid
      */
      typedef struct {
         int nell;
         float rerr;
      } Csell;
      static Csell *Tsell = (Csell*) Xsell;
      static float R1 = 1.0F;
      static float p2[3],am[3],ak[3],as,bs,cs,stretchmax,theta,phi,stretch1,stretch2,stretch,probkeep,rtest;
      /*
**********************************************************************
*/
      /*
        Sort the eigenvalues:
      */
      as = *aa;
      bs = *bb;
      cs = *cc;
      sort3(&as,&bs,&cs);            /*  as > bs > cs */
      stretchmax = fifpow(as*bs,2.0F)+fifpow(fifpow(as,2.0F)-fifpow(bs,2.0F),2.0F)/4.0F;
      stretchmax = fifsqrt(stretchmax);
   S1:
      sphere(p2,&R1);
      p1[0] = *aa*p2[0];
      p1[1] = *bb*p2[1];
      p1[2] = *cc*p2[2];
      makepolar(p2,&theta,&phi);
      am[0] = fifcos(theta)*fifcos(phi);
      am[1] = fifcos(theta)*fifsin(phi);
      am[2] = -fifsin(theta);
      ak[0] = -fifsin(phi);
      ak[1] = fifcos(phi);
      ak[2] = 0.0F;
      am[0] = *aa*am[0];
      am[1] = *bb*am[1];
      am[2] = *cc*am[2];
      ak[0] = *aa*ak[0];
      ak[1] = *bb*ak[1];
      ak[2] = *cc*ak[2];
      stretch1 = fifpow(am[0],2.0F)+fifpow(am[1],2.0F)+fifpow(am[2],2.0F);
      stretch2 = fifpow(ak[0],2.0F)+fifpow(ak[1],2.0F)+fifpow(ak[2],2.0F);
      stretch = fifsqrt(stretch1*stretch2);
      if(stretch > stretchmax) {
         if(Tsell->nell == 0) {
            Tsell->rerr = stretch/stretchmax;
            Tsell->nell = 1;
         }
         else if(Tsell->nell == 1) {
            Tsell->rerr = fifamax1(Tsell->rerr,stretch/stretchmax);
         }
      }
      probkeep = stretch/stretchmax;
      rtest = ran2();
      if(rtest > probkeep) goto S1;
      backtransform(c,t,p1);
      return;
   }

   void doverpillar(
                    float x[],
                    float y[],
                    float zl[],
                    float zh[],
                    float p1[])
   {
      static float sa,bottom,top,side12,side13,side23,v1[3],v2[3],v3[3],v4[3],piddle;
      /*
**********************************************************************
*/
      pillarsurf(x,y,zl,zh,&sa,&bottom,&top,&side12,&side13,&side23);
      piddle = ran2()*sa;
      if(piddle < bottom) {
         /*
           Find a point on the bottom triangle
         */
         v1[0] = x[0];
         v1[1] = y[0];
         v1[2] = zl[0];
         v2[0] = x[1];
         v2[1] = y[1];
         v2[2] = zl[1];
         v3[0] = x[2];
         v3[1] = y[2];
         v3[2] = zl[2];
         dovertriangle(v1,v2,v3,p1);
      }
      else if(piddle < bottom+top) {
         /*
           Find a point on the top triangle
         */
         v1[0] = x[0];
         v1[1] = y[0];
         v1[2] = zh[0];
         v2[0] = x[1];
         v2[1] = y[1];
         v2[2] = zh[1];
         v3[0] = x[2];
         v3[1] = y[2];
         v3[2] = zh[2];
         dovertriangle(v1,v2,v3,p1);
      }
      else if(piddle < bottom+top+side12) {
         /*
           Find a point on side12
         */
         v1[0] = x[0];
         v1[1] = y[0];
         v1[2] = zl[0];
         v2[0] = x[1];
         v2[1] = y[1];
         v2[2] = zl[1];
         v3[0] = x[0];
         v3[1] = y[0];
         v3[2] = zh[0];
         v4[0] = x[1];
         v4[1] = y[1];
         v4[2] = zh[1];
         dovertrap(v1,v2,v3,v4,p1);
      }
      else if(piddle < bottom+top+side12+side13) {
         /*
           Find a point on side13
         */
         v1[0] = x[0];
         v1[1] = y[0];
         v1[2] = zl[0];
         v2[0] = x[2];
         v2[1] = y[2];
         v2[2] = zl[2];
         v3[0] = x[0];
         v3[1] = y[0];
         v3[2] = zh[0];
         v4[0] = x[2];
         v4[1] = y[2];
         v4[2] = zh[2];
         dovertrap(v1,v2,v3,v4,p1);
      }
      else {
         /*
           Find a point on side23
         */
         v1[0] = x[1];
         v1[1] = y[1];
         v1[2] = zl[1];
         v2[0] = x[2];
         v2[1] = y[2];
         v2[2] = zl[2];
         v3[0] = x[1];
         v3[1] = y[1];
         v3[2] = zh[1];
         v4[0] = x[2];
         v4[1] = y[2];
         v4[2] = zh[2];
         dovertrap(v1,v2,v3,v4,p1);
      }
      return;
   }

   void dovertrap(
                  float v1[],
                  float v2[],
                  float v3[],
                  float v4[],
                  float p1[])
   {
      static float s24,s13,pearl;
      /*
**********************************************************************
*/
      s24 = v4[2]-v2[2];
      s13 = v3[2]-v1[2];
      pearl = ran2()*(s24+s13);
      if(pearl < s13) {
         dovertriangle(v1,v3,v4,p1);
      }
      else {
         dovertriangle(v1,v2,v4,p1);
      }
      return;
   }

   void sort3(
              float *as,
              float *bs,
              float *cs)
   {
      /*
        sort the three axes
      */
      static float a[3];
      static int nwk;
      static float save;
      /*
**********************************************************************
*/
      a[0] = *as;
      a[1] = *bs;
      a[2] = *cs;
      nwk = 1;
      while(nwk < 3) {
         if(nwk == 0) nwk = 1;
         if(a[nwk-1] >= a[nwk]) {
            nwk = nwk+1;
         }
         else {
            save = a[nwk-1];
            a[nwk-1] = a[nwk];
            a[nwk] = save;
            nwk = nwk-1;
         }
      }
      *as = a[0];
      *bs = a[1];
      *cs = a[2];
      return;
   }

   void makepolar(
                  float p[],
                  float *theta,
                  float *phi)
   {
      /*
        convert unit vector in p to spherical-polar coordinates
      */
      /*
**********************************************************************
*/
      if(p[2] < -1.0F) {
         *theta = 3.14159265F;
         *phi = 0.0F;
      }
      else if(p[2] > 1.0F) {
         *theta = 0.0F;
         *phi = 0.0F;
      }
      else {
         *theta = fifacos(p[2]);
         *phi = fifatan2(p[1],p[0]);
      }
      return;
   }

   void tred2(
              float *a,
              int *n,
              int *np,
              float d[],
              float e[])
   {
      /*
        Borrowed from "Numerical Recipes in Fortran 77, 2nd edition,
        Press, Teukolsky, Vettering, Flannery,
        Cambridge University Press
      */
      static int i,l;
      static float h,scale;
      static int k;
      static float f,g;
      static int j;
      static float hh;
      /*
**********************************************************************
*/
      if(*n > 1) {
         for(i= *n-1; i>=1; i--) {
            l = i;
            h = 0.F;
            scale = 0.F;
            if(l > 1) {
               for(k=0; k<l; k++) {
                  scale = scale+fifabs(*(a+i+k**np));
               }
               if(scale == 0.F) {
                  e[i] = *(a+i+(l-1)**np);
               }
               else {
                  for(k=0; k<l; k++) {
                     *(a+i+k**np) = *(a+i+k**np)/scale;
                     h = h+fifpow(*(a+i+k**np),2.0F);
                  }
                  f = *(a+i+(l-1)**np);
                  g = -fifsign(fifsqrt(h),f);
                  e[i] = scale*g;
                  h = h-f*g;
                  *(a+i+(l-1)**np) = f-g;
                  f = 0.F;
                  for(j=1; j<=l; j++) {
                     *(a+j-1+i**np) = *(a+i+(j-1)**np)/h;
                     g = 0.F;
                     for(k=0; k<j; k++) {
                        g = g+*(a+j-1+k**np)**(a+i+k**np);
                     }
                     if(l > j) {
                        for(k=j+0; k<l; k++) {
                           g = g+*(a+k+(j-1)**np)**(a+i+k**np);
                        }
                     }
                     e[j-1] = g/h;
                     f = f+e[j-1]**(a+i+(j-1)**np);
                  }
                  hh = f/(h+h);
                  for(j=1; j<=l; j++) {
                     f = *(a+i+(j-1)**np);
                     g = e[j-1]-hh*f;
                     e[j-1] = g;
                     for(k=0; k<j; k++) {
                        *(a+j-1+k**np) = *(a+j-1+k**np)-f*e[k]-g**(a+i+k**np);
                     }
                  }
               }
            }
            else {
               e[i] = *(a+i+(l-1)**np);
            }
            d[i] = h;
         }
      }
      d[0] = 0.F;
      e[0] = 0.F;
      for(i=0; i<*n; i++) {
         l = i;
         if(d[i] != 0.F) {
            for(j=1; j<=l; j++) {
               g = 0.F;
               for(k=0; k<l; k++) {
                  g = g+*(a+i+k**np)**(a+k+(j-1)**np);
               }
               for(k=0; k<l; k++) {
                  *(a+k+(j-1)**np) = *(a+k+(j-1)**np)-g**(a+k+i**np);
               }
            }
         }
         d[i] = *(a+i+i**np);
         *(a+i+i**np) = 1.F;
         if(l >= 1) {
            for(j=1; j<=l; j++) {
               *(a+i+(j-1)**np) = 0.F;
               *(a+j-1+i**np) = 0.F;
            }
         }
      }
      return;
   }

   void tqli(
             float d[],
             float e[],
             int *n,
             int *np,
             float *z)
   {
      /*
        Borrowed from "Numerical Recipes in Fortran 77, 2nd edition,
        Press, Teukolsky, Vettering, Flannery,
        Cambridge University Press
      */
      static int i,l,iter,m;
      static float dd,g,r,s,c,p,f,b;
      static int k;
      /*
*********************************************************************
*/
      if(*n > 1) {
         for(i=1; i<*n; i++) {
            e[i-1] = e[i];
         }
         e[*n-1] = 0.F;
         for(l=1; l<=*n; l++) {
            iter = 0;
         S1:
            for(m=l; m<=*n-1; m++) {
               dd = fifabs(d[m-1])+fifabs(d[m]);
               if(fifabs(e[m-1])+dd == dd) goto S2;
            }
            m = *n;
         S2:
            if(m != l) {
               if(iter == 30) PAUSE("too many iterations",19);
               iter = iter+1;
               g = (d[l]-d[l-1])/(2.F*e[l-1]);
               r = fifsqrt(fifpow(g,2.0F)+1.F);
               g = d[m-1]-d[l-1]+e[l-1]/(g+fifsign(r,g));
               s = 1.F;
               c = 1.F;
               p = 0.F;
               for(i=m-1; i>=l; i--) {
                  f = s*e[i-1];
                  b = c*e[i-1];
                  if(fifabs(f) >= fifabs(g)) {
                     c = g/f;
                     r = fifsqrt(fifpow(c,2.0F)+1.F);
                     e[i] = f*r;
                     s = 1.F/r;
                     c = c*s;
                  }
                  else {
                     s = f/g;
                     r = fifsqrt(fifpow(s,2.0F)+1.F);
                     e[i] = g*r;
                     c = 1.F/r;
                     s = s*c;
                  }
                  g = d[i]-p;
                  r = (d[i-1]-g)*s+2.F*c*b;
                  p = s*r;
                  d[i] = g+p;
                  g = c*r-b;
                  for(k=0; k<*n; k++) {
                     f = *(z+k+i**np);
                     *(z+k+i**np) = s**(z+k+(i-1)**np)+c*f;
                     *(z+k+(i-1)**np) = c**(z+k+(i-1)**np)-s*f;
                  }
               }
               d[l-1] = d[l-1]-p;
               e[l-1] = g;
               e[m-1] = 0.F;
               goto S1;
            }
         }
      }
      return;
   }
};

int
zeno_main(
          int argc,
          char const* argv[])
{
   for ( int i = 0; i < argc; i++ )
   {
      printf( "zeno_main: arg %d <%s>\n", i, argv[ i ] );
   }
   return zeno::main( argc, argv );
}

US_Hydrodyn_Zeno::US_Hydrodyn_Zeno( 
                                   hydro_options *         options,
                                   hydro_results *         results,
                                   mQProgressBar *         use_progress,
                                   US_Hydrodyn *           us_hydrodyn
                                   )
{
   this->options     = options;
   this->results     = results;
   this->us_hydrodyn = us_hydrodyn;
   zeno_us_hydrodyn  = us_hydrodyn;
   zeno_progress     = use_progress;
   zeno_stop_flag    = & us_hydrodyn->stopFlag;
}

bool US_Hydrodyn_Zeno::test()
{
   int argc = 5;
   const char *argv0 = "us_zeno";
   const char *argv1 = "zenotest";
   const char *argv2 = "i1t";
   const char *argv3 = "s1t";
   const char *argv4 = "z1t";
   const char *argv[ 5 ] = { argv0, argv1, argv2, argv3, argv4 };

   if ( !QFile::exists(  QString( "%1.bod" ).arg( argv1 ) ) )
   {
      cout << 
         QString( "error: %1 %2 does not exist" )
         .arg( QDir::current().path() )
         .arg( QString( "%1.bod" ).arg( argv1 ) ).toLatin1().data();
      return false;
   }
   zeno_progress->setMaximum( 108 * 3 );
   zeno_main( argc, argv );
   return true;
}

bool US_Hydrodyn_Zeno::run(
                           QString                 filename,
                           vector < PDB_atom > *   bead_model,
                           double              &   sum_mass,
                           double              &   sum_volume,
                           const double        &   Rg,
                           mQProgressBar       *   use_progress,
                           bool                    keep_files,
                           bool                    zeno_cxx,
                           int                     threads
                           )
{
   us_qdebug( QString( "zeno run start\n" ) );
   this->filename    = filename;
   this->bead_model  = bead_model;
   this->keep_files  = keep_files;
   zeno_progress     = use_progress;

   error_msg = "";

   if ( !QDir::setCurrent( QFileInfo( filename ).filePath() ) )
   {
      // cout << QString( "current dir is %1\n" ).arg( QDir::current().path() );
      // error_msg = QString( "Error: can not change to directory: %1" ).arg( QFileInfo( filename ).path() );
      // return false;
   }

   cout << QString( "current dir is %1\n" ).arg( QDir::current().path() ).toLatin1().data();

   if ( options->unit != -9 &&
        options->unit != -10 )
   {
      error_msg = QString( "Error: units must be A(10) or nm(9) and value is %1" ).arg( -options->unit );
      return false;
   }

   // create .bod file
   // assume overwrite, maybe check in us_hydrodyn call

   QString outname = filename;
   if ( !outname.contains( QRegExp( ".bod$" ) ) )
   {
      outname += ".bod";
   } else {
      filename.replace( QRegExp( ".bod$" ), "" );
   }

   QFile fout ( outname );
   if ( !fout.open( QIODevice::WriteOnly ) )
   {
      error_msg = QString( "Error: can not create file: %1" ).arg( outname );
      return false;
   }
    
   // output bead model

   QTextStream tso( &fout );

   sum_mass = 0e0;

   sum_volume = 0e0;

   for ( unsigned int i = 0; i < bead_model->size(); i++ )
   {
      tso << 
         QString( "S %1 %2 %3 %4\n" )
         .arg( (*bead_model)[ i ].bead_coordinate.axis[ 0 ] )
         .arg( (*bead_model)[ i ].bead_coordinate.axis[ 1 ] )
         .arg( (*bead_model)[ i ].bead_coordinate.axis[ 2 ] )
         .arg( (*bead_model)[ i ].bead_computed_radius )
         ;
      sum_mass   += (*bead_model)[ i ].bead_ref_mw + (*bead_model)[ i ].bead_ref_ionized_mw_delta;
      sum_volume += (4e0 / 3e0 ) * M_PI * pow( (*bead_model)[ i ].bead_computed_radius, 3 );
   }

   // additional info

   tso << QString( "temp      %1 C\n"  ).arg( options->temperature       );
   // tso << QString( "solvent   %1\n"    ).arg( options->solvent_name      );
   tso << QString( "viscosity %1 cp\n"    ).arg( us_hydrodyn->use_solvent_visc() );
   tso << QString( "mass      %1 Da\n" ).arg( options->mass_correction ?
                                              options->mass : sum_mass );
   tso << QString( "units     %1\n"    ).arg( options->unit == -9 ?
                                              "nm" : "A" );

   // add skin thickness
   if ( options->zeno_surface_thickness_from_rg ) {
      // linear
      // double st = options->zeno_surface_thickness_from_rg_a + options->zeno_surface_thickness_from_rg_b * Rg;

      // sigmoid
      double st = options->zeno_surface_thickness_from_rg_a / ( 1 + exp( -( Rg - options->zeno_surface_thickness_from_rg_b ) / options->zeno_surface_thickness_from_rg_c ) );

      qDebug() << QString( "Rg %1 st %2\n" ).arg( Rg ).arg( st );
      if ( st > 0 ) {
         tso << QString( "st        %1\n"    ).arg( st );
      }
   } else {
      if ( options->zeno_surface_thickness > 0.0 )
      {
         tso << QString( "st        %1\n"    ).arg( options->zeno_surface_thickness );
      }
   }

   fout.close();
   us_hydrodyn->editor_msg( "black", QString( "Created %1\n" ).arg( outname ) );
   us_hydrodyn->editor_msg( "black", QString( "Beads used %1\n" ).arg( bead_model->size() ) );

   if ( zeno_cxx ) {
      int argc = 0;
      char *argv[ 8 ];

      argv[ argc++ ] = strdup( "us_zeno_cxx" );

      argv[ argc++ ] = strdup( "-i" );
      argv[ argc++ ] = strdup( outname.toLatin1().data() );
      
      // previous zeno version
      // argv[ argc++ ] = strdup( "-n" );
      // argv[ argc++ ] = strdup( QString( "%1" ).arg(options->zeno_zeno_steps * 1000 ).toLatin1().data() );
      if ( us_hydrodyn->gparams[ "zeno_max_cap" ] == "true" ) {
         argv[ argc++ ] = strdup( QString( "--max-rsd-capacitance=%1" ).arg( us_hydrodyn->gparams[ "zeno_max_cap_pct" ] ).toLatin1().data() );
      } else {
         argv[ argc++ ] = strdup( QString( "--num-walks=%1" ).arg(options->zeno_zeno_steps * 1000 ).toLatin1().data() );
      }
      
      us_qdebug( QString( "zeno steps %1" ).arg( options->zeno_zeno_steps * 1000 ) );

      // previous zeno version
      // argv[ argc++ ] = strdup( "-t" );
      // argv[ argc++ ] = strdup( QString( "%1" ).arg( threads ).toLatin1().data() );
      argv[ argc++ ] = strdup( QString( "--num-threads=%1" ).arg( threads ).toLatin1().data() );

      argv[ argc++ ] = strdup( QString( "--seed=%1" ).arg( QDateTime::currentDateTime().toTime_t() ).toLatin1().data() );
      us_qdebug( QString( "current datetime %1" ).arg( argv[ argc - 1 ] ) );

      int progress_steps = 100;

      zeno_progress->setValue( 0 ); zeno_progress->setMaximum( progress_steps );
      us_qdebug( QString( "zeno run start zeno_cxx_main\n" ) );
#if !defined(USE_OLD_ZENO) && __cplusplus >= 201103L
      zeno_cxx_main(
                    argc
                    ,argv
                    ,QString( "%1.zno%2" ).arg( filename ).arg( us_hydrodyn->batch_avg_hydro_active() || us_hydrodyn->zeno_mm ? "_Tmp_Remove" : "" ).toLatin1().data()
                    ,false
                    ,zeno_us_udp_msg
                    );
#endif
      us_qdebug( QString( "zeno run return zeno_cxx_main\n" ) );
      zeno_progress->reset();

      if ( !us_hydrodyn->stopFlag )
      {
         if ( !us_hydrodyn->batch_avg_hydro_active() && !us_hydrodyn->zeno_mm ) {
            us_hydrodyn->last_hydro_res = QFileInfo( filename + ".zno" ).fileName();
            us_hydrodyn->editor_msg( "black", QString( "Created %1\n" ).arg( filename + ".zno" ) );
         } else {
            us_hydrodyn->last_hydro_res = QFileInfo( filename + ".zno_Tmp_Remove" ).fileName();
         }            
      }
   } else {

      int argc = 0;
      const char *argv[ 5 ];

      argv[ argc++ ] = "us_zeno";

      QString cmdfile = QFileInfo( filename ).fileName();
#if QT_VERSION >= 0x050000
      char *data_cmdfile = 0;
      data_cmdfile = new char[cmdfile.size() + 1];
      strcpy(data_cmdfile, cmdfile.toLatin1().data());
      argv[ argc++ ] = data_cmdfile;
#else
      argv[ argc++ ] = cmdfile.toLatin1().data();
#endif

      cout << QString ( " zeno <%1> <%2> <%3>\n" )
         .arg( options->zeno_zeno_steps )
         .arg( options->zeno_interior_steps )
         .arg( options->zeno_surface_steps ).toLatin1().data();


      QString qs_zeno     = QString( "" ).sprintf( "z%ut", options->zeno_zeno_steps     );
      QString qs_interior = QString( "" ).sprintf( "i%ut", options->zeno_interior_steps );
      QString qs_surface  = QString( "" ).sprintf( "s%ut", options->zeno_surface_steps  );

      int progress_steps = 0;

#if QT_VERSION >= 0x050000
      char *data_zeno = 0;
      char *data_interior = 0;
      char *data_surface = 0;

      if ( options->zeno_zeno )
      {
         progress_steps += 108;
         data_zeno = new char[qs_zeno.size() + 1];
         strcpy(data_zeno, qs_zeno.toLatin1().data());
         argv[ argc++ ] = data_zeno;
      }

      if ( options->zeno_interior )
      {
         progress_steps += 108;
         data_interior = new char[qs_interior.size() + 1];
         strcpy(data_interior, qs_interior.toLatin1().data());
         argv[ argc++ ] = data_interior;
      }
      if ( options->zeno_surface )
      {
         progress_steps += 108;
         data_surface = new char[qs_surface.size() + 1];
         strcpy(data_surface, qs_surface.toLatin1().data());
         argv[ argc++ ] = data_surface;
      }
#else
      if ( options->zeno_zeno )
      {
         progress_steps += 108;
         argv[ argc++ ] = qs_zeno.toLatin1().data();
      }
      if ( options->zeno_interior )
      {
         progress_steps += 108;
         argv[ argc++ ] = qs_interior.toLatin1().data();
      }
      if ( options->zeno_surface )
      {
         progress_steps += 108;
         argv[ argc++ ] = qs_surface.toLatin1().data();
      }
#endif

      zeno_progress->setValue( 0 ); zeno_progress->setMaximum( progress_steps );
      zeno_main( argc, argv );
      zeno_progress->reset();

#if QT_VERSION >= 0x050000
      delete[] data_cmdfile;
      if ( data_zeno ) {
         delete[] data_zeno;
      }
      if ( data_interior ) {
         delete[] data_interior;
      }
      if ( data_surface ) {
         delete[] data_surface;
      }
#endif

      if ( !us_hydrodyn->stopFlag && !us_hydrodyn->batch_avg_hydro_active() && !us_hydrodyn->zeno_mm )
      {
         if ( !us_hydrodyn->batch_avg_hydro_active() && !us_hydrodyn->zeno_mm ) {
            us_hydrodyn->last_hydro_res = QFileInfo( filename + ".zno" ).fileName();
            us_hydrodyn->editor_msg( "black", QString( "Created %1\n" ).arg( filename + ".zno" ) );
         } else {
            us_hydrodyn->last_hydro_res = QFileInfo( filename + ".zno_Tmp_Remove" ).fileName();
         }            
      }
   }


   return true;
}

bool US_Hydrodyn::calc_zeno()
{
   // cout << "calc zeno\n";
   progress->set_cli_prefix( "ch" );

   if ( !hydro.zeno_zeno &&
        !hydro.zeno_interior &&
        !hydro.zeno_surface )
   {
      editor_msg( "dark red", us_tr( "No Zeno methods selected.  Select in SOMO->Hydrodynamic Calculations Zeno" ) );
      return false;
   }

   editor->append( "\n" );

   if ( hydro.zeno_zeno )
   {
      editor_msg( "dark blue", 
                  QString( us_tr( "Zeno integration selected with %1 thousand MC iterations" ) )
                  .arg( hydro.zeno_zeno_steps ) );
   }


   if ( hydro.zeno_interior )
   {
      editor_msg( "dark blue", 
                  QString( us_tr( "Zeno interior integration selected with %1 thousand MC iterations" ) )
                  .arg( hydro.zeno_interior_steps ) );
   }

   if ( hydro.zeno_surface )
   {
      editor_msg( "dark blue", 
                  QString( us_tr( "Zeno surface integration selected with %1 thousand MC iterations" ) )
                  .arg( hydro.zeno_surface_steps ) );
   }


   stopFlag = false;
   bool was_hydro_enabled = pb_calc_hydro->isEnabled();
   pb_stop_calc->setEnabled(true);
   pb_calc_hydro->setEnabled(false);
   pb_calc_zeno->setEnabled(false);
   pb_calc_grpy->setEnabled(false);
   pb_calc_hullrad->setEnabled(false);
   display_default_differences();
   editor->append("\nBegin hydrodynamic calculations (Zeno) \n\n");
   qApp->processEvents();

   int models_to_proc = 0;
   int first_model_no = 0;
   for (current_model = 0; current_model < (unsigned int)lb_model->count(); current_model++) {
      if (lb_model->item(current_model)->isSelected()) {
         if (somo_processed[current_model]) {
            if (!first_model_no) {
               first_model_no = current_model + 1;
            }
            models_to_proc++;
            editor->append(QString("Model %1 will be included\n").arg(current_model + 1));
            bead_model = bead_models[current_model];
         }
         else
         {
            editor->append(QString("Model %1 - selected but bead model not built\n").arg(current_model + 1));
         }
      }
   }
   zeno_mm = models_to_proc > 1;
   mm_mode = zeno_mm;
   US_Hydrodyn_Zeno uhz( &hydro, &results, ( zeno_mm ? mprogress : progress ), this );
      
   QDir::setCurrent( get_somo_dir() );

   qApp->processEvents();
   if (stopFlag)
   {
      editor->append("Stopped by user\n\n");
      pb_calc_hydro->setEnabled( was_hydro_enabled );
      pb_calc_zeno->setEnabled(true);
      pb_calc_grpy->setEnabled(true);
      pb_calc_hullrad->setEnabled(true);
      pb_bead_saxs->setEnabled(true);
      pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
      pb_show_hydro_results->setEnabled(false);
      progress->reset();
      mprogress->hide();
      return false;
   }
   le_bead_model_suffix->setText(bead_model_suffix);

   hydro_results zeno_results;
   hydro_results zeno_results2;

   zeno_results.mass                  = 0e0;
   zeno_results.s20w                  = 0e0;
   zeno_results.s20w_sd               = 0e0;
   zeno_results.D20w                  = 0e0;
   zeno_results.D20w_sd               = 0e0;
   zeno_results.viscosity             = 0e0;
   zeno_results.viscosity_sd          = 0e0;
   zeno_results.rs                    = 0e0;
   zeno_results.rs_sd                 = 0e0;
   zeno_results.rg                    = 0e0;
   zeno_results.rg_sd                 = 0e0;
   zeno_results.tau                   = 0e0;
   zeno_results.tau_sd                = 0e0;
   zeno_results.vbar                  = 0e0;
   zeno_results.asa_rg_pos            = 0e0;
   zeno_results.asa_rg_neg            = 0e0;
   zeno_results.ff0                   = 0e0;
   zeno_results.ff0_sd                = 0e0;

   zeno_results.solvent_name          = hydro.solvent_name;
   zeno_results.solvent_acronym       = hydro.solvent_acronym;
   zeno_results.solvent_viscosity     = use_solvent_visc();
   zeno_results.solvent_density       = use_solvent_dens();
   zeno_results.temperature           = hydro.temperature;
   zeno_results.pH                    = hydro.pH;
   zeno_results.name                  = project;
   zeno_results.used_beads            = 0;
   zeno_results.used_beads_sd         = 0e0;
   zeno_results.total_beads           = 0;
   zeno_results.total_beads_sd        = 0e0;
   zeno_results.vbar                  = 0;

   zeno_results.num_models            = 0;

   zeno_results2 = zeno_results;
   QString zeno_model_list            = "";

   bool zeno_cxx                      =
#if QT_VERSION < 0x040000
                 gparams.count( "zeno_cxx" ) && gparams[ "zeno_cxx" ] == "true" && advanced_config.expert_mode
#else
                 true
#endif
                 ;
   
#if defined(USE_OLD_ZENO)
   zeno_cxx = false;
#endif

#if !defined(USE_OLD_ZENO) &&__cplusplus < 201103L
   if ( zeno_cxx ) {
      editor_msg( "darkRed", "Notice: ZENO method is not currently not available for this platform" );
      qApp->processEvents();
      zeno_cxx = false;
   }
# if QT_VERSION >= 0x040000
   return false;
# endif
#endif
   

# if !defined(USE_OLD_ZENO) && QT_VERSION < 0x040000
   if ( zeno_cxx ) {
      editor_msg( "darkRed", "Notice: the new ZENO method is active" );
      qApp->processEvents();
   }
#endif
   
   if ( zeno_cxx ) {
      editor_msg( "black", QString( "ZENO will use %1 threads\n" ).arg( USglobal->config_list.numThreads ) );
   }
   
   double sum_mass   = 0e0;
   double sum_volume = 0e0;

   // QTimer *t_pe = new QTimer(this);
   // connect( t_pe, SIGNAL( timeout() ), SLOT( process_events() ) );
           
   int repeats = gparams.count( "zeno_repeats" ) ? gparams[ "zeno_repeats" ].toUInt() : 1;
   if ( repeats < 1 ) {
      repeats = 1;
   }

   if ( repeats > 1 ) {
      editor_msg( "blue", QString( "ZENO will repeat %1 times\n" ).arg( repeats ) );
   }

   bool has_overlap = overlap_check( true, true, true,
                                     hydro.overlap_cutoff ? hydro.overlap : overlap_tolerance, 1, true ) > 0;

   if ( has_overlap ) {
      editor_msg( "dark red", QString( us_tr( "Bead models have overlap, dimensionless intrinsic viscosity not computed.\n" ) ) );
   }      


   QString mc =
      gparams[ "zeno_max_cap" ] == "true" ?
      QString( "_MSDP%1" ).arg( gparams[ "zeno_max_cap_pct" ]  )
      :
      QString( "_MC%1" ).arg( hydro.zeno_zeno_steps );
   ;

   zeno_mm_save_params.data_vector.clear();
   zeno_mm_results = "";
   zeno_mm_name   = 
      get_somo_dir() 
      + QDir::separator() 
      + project 
      + mc
      + QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "" )
      ;
   
   if ( zeno_mm ) {
      progress->setValue( 0 );
      progress->setMaximum( models_to_proc );
      mprogress->setValue( 0 );
      mprogress->setFormat( "Model %p%" );
      mprogress->show();
   }

   int models_procd = 0;

   for (current_model = 0; current_model < (unsigned int)lb_model->count(); current_model++) {
      if (lb_model->item(current_model)->isSelected()) {
         if (somo_processed[current_model]) {
            double used_skin_thickness = 0e0;
            if ( zeno_mm ) {
               progress->setValue( models_procd++ );
            }
            for ( int this_repeat = 0; this_repeat < repeats; ++this_repeat ) {
               if (!first_model_no) {
                  first_model_no = current_model + 1;
               }
               bead_model = bead_models[current_model];
               // t_pe->start( 1000 );

               // QString mc =
               //    gparams[ "zeno_max_cap" ] == "true" ?
               //    QString( "_MSDP%1" ).arg( gparams[ "zeno_max_cap_pct" ]  )
               //    :
               //    QString( "_MC%1" ).arg( hydro.zeno_zeno_steps );
               // ;

               QString fname =
                  QString( get_somo_dir() 
                           + QDir::separator() 
                           + project 
                           + QString( "_%1" ).arg( current_model + 1 )
                           + mc
                           + ( repeats > 1 ? QString( "_r%1" ).arg( this_repeat ) : QString( "" ) )
                           + QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "" )
                           + ".zno" )
                  ;

               if ( !overwrite_hydro ) {
                  fname = fileNameCheck( fname, 0, this );
               }

               fname = fname.replace( QRegExp( "\\.(zno)$" ), "" );

               if ( hydro.zeno_surface_thickness_from_rg ) {
                  // linear
                  // double st = hydro.zeno_surface_thickness_from_rg_a + hydro.zeno_surface_thickness_from_rg_b * model_vector[ current_model ].asa_rg_pos;
                  // sigmoid
                  double st = hydro.zeno_surface_thickness_from_rg_a / ( 1 + exp( -( model_vector[ current_model ].asa_rg_pos - hydro.zeno_surface_thickness_from_rg_b ) / hydro.zeno_surface_thickness_from_rg_c ) );
                  editor_msg(
                             "darkblue"
                             ,QString( us_tr( "Computed skin thickness for model %1 from Rg %2 [%3] is %4 [%5]\n" ) )
                             .arg( current_model + 1 )
                             .arg( model_vector[ current_model ].asa_rg_pos, 0, 'f', 3 )
                             .arg( UNICODE_ANGSTROM )
                             .arg( st, 0, 'f', 3 )
                             .arg( UNICODE_ANGSTROM )
                             );
                  if ( st <= 0 ) {
                     editor_msg( "red", us_tr( "NOTICE: zero or negative computed skin thickness will be ignored, reverting to ZENO default\n" ) );
                  } else {
                     used_skin_thickness = st;
                  }
               } else {
                  if ( hydro.zeno_surface_thickness >= 0 ) {
                     used_skin_thickness = hydro.zeno_surface_thickness;
                  }
               }

               US_Timer           us_timers;
               us_timers          .clear_timers();
               us_timers.init_timer( "compute zeno" );

               us_timers.start_timer( "compute zeno" );

               bool result = 
                  uhz.run( 
                          fname,
                          &bead_models[ current_model ], 
                          sum_mass,
                          sum_volume,
                          model_vector[ current_model ].asa_rg_pos,
                          zeno_mm ? mprogress : progress,
                          true,
                          zeno_cxx,
                          USglobal->config_list.numThreads
                           );
               us_timers.end_timer( "compute zeno" );
               // t_pe->stop();
               if (stopFlag)
               {
                  editor->append("Stopped by user\n\n");
                  pb_calc_hydro->setEnabled( was_hydro_enabled );
                  pb_calc_zeno->setEnabled(true);
                  pb_bead_saxs->setEnabled(true);
                  pb_calc_grpy->setEnabled(true);
                  pb_calc_hullrad->setEnabled(true);
                  pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
                  pb_show_hydro_results->setEnabled(false);
                  progress->reset();
                  mprogress->hide();
                  return false;
               }
               if ( !result )
               {
                  editor_msg( "red", "ZENO computation failed" );
                  editor_msg( "red", us_tr( uhz.error_msg ) );
                  pb_calc_hydro->setEnabled( was_hydro_enabled );
                  pb_calc_zeno->setEnabled(true);
                  pb_bead_saxs->setEnabled(true);
                  pb_calc_grpy->setEnabled(true);
                  pb_calc_hullrad->setEnabled(true);
                  pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
                  pb_show_hydro_results->setEnabled(false);
                  progress->reset();
                  mprogress->hide();
                  return false;
               } else {
                  // setup save data
                  // add text output also
                  save_data this_data = US_Hydrodyn_Save::save_data_initialized_from_bead_model( bead_models[ current_model ], false );

                  us_timers.start_timer( "compute zeno" );
                  this_data.results.solvent_viscosity     = use_solvent_visc();
                  this_data.results.solvent_density       = use_solvent_dens();
                  this_data.results.pH                    = hydro.pH;

                  this_data.hydro                         = hydro;
                  this_data.hydro.solvent_density         = use_solvent_dens();
                  this_data.hydro.solvent_viscosity       = use_solvent_visc();
                  this_data.model_idx                     = model_vector[ current_model ].model_id;
                  this_data.results.num_models            = 1;
                  this_data.results.name                  = QFileInfo( last_hydro_res ).completeBaseName();
                  this_data.results.used_beads            = bead_models [ current_model ].size();
                  this_data.results.total_beads           = bead_models [ current_model ].size();
                  this_data.results.vbar                  = use_vbar( model_vector[ current_model ].vbar );
                  this_data.con_factor                    = pow(10.0, this_data.hydro.unit + 9);
                  this_data.zeno_skin_thickness           = used_skin_thickness;
                  
                  if ( bead_models[ current_model ].size() &&
                       bead_models[ current_model ][0].is_vdw == "vdw" ) {
                     this_data.hydrate_probe_radius          = bead_models[ current_model ][0].asa_hydrate_probe_radius;
                     this_data.hydrate_threshold             = bead_models[ current_model ][0].asa_hydrate_threshold;
                     this_data.vdw_theo_waters               = bead_models[ current_model ][0].vdw_theo_waters;
                     this_data.vdw_exposed_residues          = bead_models[ current_model ][0].vdw_count_exposed;
                     this_data.vdw_exposed_waters            = bead_models[ current_model ][0].vdw_theo_waters_exposed;
                  }

                  this_data.fractal_dimension_parameters         = model_vector[ current_model ].fractal_dimension_parameters;
                  this_data.fractal_dimension                    = model_vector[ current_model ].fractal_dimension;
                  this_data.fractal_dimension_sd                 = model_vector[ current_model ].fractal_dimension_sd;
                  this_data.fractal_dimension_wtd                = model_vector[ current_model ].fractal_dimension_wtd;
                  this_data.fractal_dimension_wtd_sd             = model_vector[ current_model ].fractal_dimension_wtd_sd;
                  this_data.fractal_dimension_wtd_wtd            = model_vector[ current_model ].fractal_dimension_wtd_wtd;
                  this_data.fractal_dimension_wtd_wtd_sd         = model_vector[ current_model ].fractal_dimension_wtd_wtd_sd;
                  this_data.rg_over_fractal_dimension            = model_vector[ current_model ].rg_over_fractal_dimension;
                  this_data.rg_over_fractal_dimension_sd         = model_vector[ current_model ].rg_over_fractal_dimension_sd;
                  this_data.rg_over_fractal_dimension_wtd        = model_vector[ current_model ].rg_over_fractal_dimension_wtd;
                  this_data.rg_over_fractal_dimension_wtd_sd     = model_vector[ current_model ].rg_over_fractal_dimension_wtd_sd;
                  this_data.rg_over_fractal_dimension_wtd_wtd    = model_vector[ current_model ].rg_over_fractal_dimension_wtd_wtd;
                  this_data.rg_over_fractal_dimension_wtd_wtd_sd = model_vector[ current_model ].rg_over_fractal_dimension_wtd_wtd_sd;

                  bead_model = bead_models[ current_model ];
                  // bead_check( false, true, true );
                  this_data.results.asa_rg_pos            = model_vector[ current_model ].asa_rg_pos;
                  this_data.results.asa_rg_neg            = model_vector[ current_model ].asa_rg_neg;
                  // qDebug() << "calc_zeno() asa rg +/- " << this_data.results.asa_rg_pos << " " << this_data.results.asa_rg_neg;

                  // need to get rg
                  // this_data.results.rg            = model_vector[ current_model ].Rg;

                  this_data.proc_time                     = (double)(us_timers.times[ "compute zeno" ]) / 1e3;

                  QFile f( last_hydro_res );
                  if ( !f.exists() || !f.open( QIODevice::ReadOnly ) )
                  {
                  
                     editor_msg( "red", "ZENO computation failed to produce output file" );
                     pb_calc_hydro->setEnabled( was_hydro_enabled );
                     pb_calc_zeno->setEnabled(true);
                     pb_bead_saxs->setEnabled(true);
                     pb_calc_grpy->setEnabled(true);
                     pb_calc_hullrad->setEnabled(true);
                     pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
                     pb_show_hydro_results->setEnabled(false);
                     progress->reset();
                     mprogress->hide();
                     return false;
                  }
                  QTextStream ts( &f );

                  QStringList qsl;
                  while( !ts.atEnd() )
                  {
                     QString qs = ts.readLine();
                     this_data.hydro_res += qs + "\n";
                     qsl << qs;
                  
                  }
                  f.close();
                  if ( batch_avg_hydro_active() || zeno_mm ) {
                     QFile::remove( last_hydro_res );
                     last_hydro_res = "";
                  }

                  // editor_msg( "dark blue", 
                  //             QString( "ZENO computation output file %1 opened %2 lines" )
                  //             .arg( last_hydro_res )
                  //             .arg( qsl.size() ) );

                  vector < QRegExp     > param_rx;
                  vector < QString     > param_name;
                  vector < int         > param_cap_pos;
                  vector < QStringList > param_qsl;

                  if ( zeno_cxx ) {

#if defined( NEW_ZENO_PRE_5 )
                     // previous zeno version
                     // param_rx     .push_back( QRegExp( "^Capacitance:\\s+(\\S+)\\s*$" ) );
                     // param_name   .push_back( "results.rs" );
                     // param_cap_pos.push_back( 1 );

                     // param_rx     .push_back( QRegExp( "^Intrinsic viscosity:\\s+(\\S+)\\s*$" ) );
                     // param_name   .push_back( "results.viscosity" );
                     // param_cap_pos.push_back( 1 );

                     // param_rx     .push_back( QRegExp( "^Volume:\\s+(\\S+)\\s*$" ) );
                     // param_name   .push_back( "used_beads_vol" );
                     // param_cap_pos.push_back( 1 );
#else

                     param_rx     .push_back( QRegExp( "^Capacitance .(?:\\S+).:\\s+(\\S+)\\s+./-\\s+(\\S+)$" ) );
                     param_name   .push_back( "results.rs" );
                     param_cap_pos.push_back( 1 );

                     param_rx     .push_back( QRegExp( "^Capacitance .(?:\\S+).:\\s+(\\S+)\\s+./-\\s+(\\S+)$" ) );
                     param_name   .push_back( "results.rs_sd" );
                     param_cap_pos.push_back( 2 );

                     param_rx     .push_back( QRegExp( "^Intrinsic viscosity with mass units .cm.3/g.:\\s+(\\S+)\\s+./-\\s+(\\S+)$" ) );
                     param_name   .push_back( "results.viscosity" );
                     param_cap_pos.push_back( 1 );

                     param_rx     .push_back( QRegExp( "^Intrinsic viscosity with mass units .cm.3/g.:\\s+(\\S+)\\s+./-\\s+(\\S+)$" ) );
                     param_name   .push_back( "results.viscosity_sd" );
                     param_cap_pos.push_back( 2 );

                     param_rx     .push_back( QRegExp( "^Friction coefficient .d.s/cm.:\\s+(\\S+)\\s+./-\\s+(\\S+)$" ) );
                     param_name   .push_back( "tra_fric_coef" );
                     param_cap_pos.push_back( 1 );

                     param_rx     .push_back( QRegExp( "^Friction coefficient .d.s/cm.:\\s+(\\S+)\\s+./-\\s+(\\S+)$" ) );
                     param_name   .push_back( "tra_fric_coef_sd" );
                     param_cap_pos.push_back( 2 );

                     param_rx     .push_back( QRegExp( "^Diffusion coefficient .cm.2/s.:\\s+(\\S+)\\s+./-\\s+(\\S+)$" ) );
                     param_name   .push_back( "results.D20w" );
                     param_cap_pos.push_back( 1 );

                     param_rx     .push_back( QRegExp( "^Diffusion coefficient .cm.2/s.:\\s+(\\S+)\\s+./-\\s+(\\S+)$" ) );
                     param_name   .push_back( "results.D20w_sd" );
                     param_cap_pos.push_back( 2 );

                     param_rx     .push_back( QRegExp( "^Prefactor for computing intrinsic viscosity:\\s+(\\S+)\\s+./-\\s+(\\S+)$" ) );
                     param_name   .push_back( "zeno_eta_prefactor" );
                     param_cap_pos.push_back( 1 );

                     param_rx     .push_back( QRegExp( "^Prefactor for computing intrinsic viscosity:\\s+(\\S+)\\s+./-\\s+(\\S+)$" ) );
                     param_name   .push_back( "zeno_eta_prefactor_sd" );
                     param_cap_pos.push_back( 2 );

                     param_rx     .push_back( QRegExp( "^Mean electric polarizability .(?:\\S+).:\\s+(\\S+)\\s+./-\\s+(\\S+)$" ) );
                     param_name   .push_back( "zeno_mep" );
                     param_cap_pos.push_back( 1 );

                     param_rx     .push_back( QRegExp( "^Mean electric polarizability .(?:\\S+).:\\s+(\\S+)\\s+./-\\s+(\\S+)$" ) );
                     param_name   .push_back( "zeno_mep_sd" );
                     param_cap_pos.push_back( 2 );
#endif
                  } else {

                     param_rx     .push_back( QRegExp( "^.eta..M. . . . . . . . . . . .\\s+(\\S+)\\s+" ) );
                     param_name   .push_back( "results.viscosity" );
                     param_cap_pos.push_back( 1 );

                     param_rx     .push_back( QRegExp( "^D  . . . . . . . . . . . . . .\\s+(\\S+)\\s+" ) );
                     param_name   .push_back( "results.D20w" );
                     param_cap_pos.push_back( 1 );

                     param_rx     .push_back( QRegExp( "^mass . . . . . . . . . . . . .\\s+(\\S+)\\s+" ) );
                     param_name   .push_back( "results.mass" );
                     param_cap_pos.push_back( 1 );

                     param_rx     .push_back( QRegExp( "^Rg .interior.  . . . . . . . .\\s+(\\S+)\\s+" ) );
                     param_name   .push_back( "results.rg" );
                     param_cap_pos.push_back( 1 );

                     param_rx     .push_back( QRegExp( "^surface area . . . . . . . . .\\s+(\\S+)\\s+" ) );
                     param_name   .push_back( "tot_surf_area" );
                     param_cap_pos.push_back( 1 );

                     param_rx     .push_back( QRegExp( "^volume . . . . . . . . . . . .\\s+(\\S+)\\s+" ) );
                     param_name   .push_back( "used_beads_vol" );
                     param_cap_pos.push_back( 1 );

                     param_rx     .push_back( QRegExp( "^Rh . . . . . . . . . . . . . .\\s+(\\S+)\\s+" ) );
                     param_name   .push_back( "results.rs" );
                     param_cap_pos.push_back( 1 );

                     param_rx     .push_back( QRegExp( "^f  . . . . . . . . . . . . . .\\s+(\\S+)\\s+" ) );
                     param_name   .push_back( "tra_fric_coef" );
                     param_cap_pos.push_back( 1 );
                  }

                  for ( unsigned int i = 0; i < ( unsigned int ) param_rx.size(); i++ )
                  {
                     param_qsl.push_back( qsl.filter( param_rx[ i ] ) );
                  }               

                  for ( unsigned int i = 0; i < ( unsigned int ) param_rx.size(); i++ )
                  {
                     us_qdebug( QString( "%1 param_qsl[%2].size() == %3" ).arg( param_rx[ i ].pattern() ).arg( i ).arg( param_qsl[ i ].size() ) );
                     switch( param_qsl[ i ].size() )
                     {
                     case 1 :
                        {
                           param_rx[ i ].indexIn( param_qsl[ i ][ 0 ] );
                           QString qs = param_rx[ i ].cap( param_cap_pos[ i ] );
                           // us_qdebug( QString( "returned searching %1 returned %2" ).arg( param_qsl[ i ][ 0 ] ).arg( qs ) );
                        
                           // QRegExp rx( param_rx[ i ] );
                           // rx.indexIn( param_qsl[ i ][ 0 ] );                        
                           // QString qs = rx.cap( param_cap_pos[ i ] );

                           qs.replace( QRegExp( "\\(\\d+\\)" ), "" );
                           double qd = qs.toDouble();
                           // cout << QString( "zeno cap %1 as %2\n" ).arg( param_name[ i ] ).arg( qd );

                           // editor_msg( "dark blue", QString( "ZENO produced parameter: %1 value %2\n" ).arg( param_name[ i ] ).arg( qs ) );

                           if ( param_name[ i ] == "results.viscosity" )
                           {
                              this_data.results.viscosity = qd;
                              break;
                           }
                           if ( param_name[ i ] == "results.viscosity_sd" )
                           {
                              this_data.results.viscosity_sd = qd;
                              break;
                           }
                           if ( param_name[ i ] == "results.D20w" )
                           {
                              this_data.results.D20w      = qd;
                              break;
                           }
                           if ( param_name[ i ] == "results.D20w_sd" )
                           {
                              this_data.results.D20w_sd   = qd;
                              break;
                           }
                           if ( param_name[ i ] == "results.mass" )
                           {
                              this_data.results.mass      = qd;
                              this_data.use_bead_mass     = qd;
                              break;
                           }
                           if ( param_name[ i ] == "results.rg" )
                           {
                              this_data.results.rg        = qd;
                              break;
                           }
                           if ( param_name[ i ] == "results.rs" )
                           {
                              this_data.results.rs        = qd * 0.1;
                              if ( hydro.unit == -9 ) {
                                 this_data.results.rs      *= 10;
                              }
                                 
                              break;
                           }
                           if ( param_name[ i ] == "results.rs_sd" )
                           {
                              this_data.results.rs_sd     = qd * 0.1;
                              if ( hydro.unit == -9 ) {
                                 this_data.results.rs_sd      *= 10;
                              }
                                 
                              break;
                           }
                           if ( param_name[ i ] == "tot_surf_area" )
                           {
                              this_data.tot_surf_area     = qd;
                              this_data.use_beads_surf    = qd;
                              break;
                           }
                           if ( param_name[ i ] == "used_beads_vol" )
                           {
                              this_data.use_beads_vol     = qd;
                              break;
                           }
                           if ( param_name[ i ] == "tra_fric_coef" )
                           {
                              this_data.tra_fric_coef     = qd;
                              break;
                           }
                           if ( param_name[ i ] == "tra_fric_coef_sd" )
                           {
                              this_data.tra_fric_coef_sd     = qd;
                              break;
                           }
                           if ( param_name[ i ] == "zeno_eta_prefactor" )
                           {
                              this_data.zeno_eta_prefactor     = qd;
                              break;
                           }
                           if ( param_name[ i ] == "zeno_eta_prefactor_sd" )
                           {
                              this_data.zeno_eta_prefactor_sd  = qd;
                              break;
                           }
                           if ( param_name[ i ] == "zeno_mep" )
                           {
                              this_data.zeno_mep     = qd;
                              break;
                           }
                           if ( param_name[ i ] == "zeno_mep_sd" )
                           {
                              this_data.zeno_mep_sd  = qd;
                              break;
                           }
                        } 
                        break;
                     case 0:
                        {
                           // editor_msg( "red", QString( "ZENO did not produce expected parameter: %1\n" ).arg( param_name[ i ] ) );
                        }
                        break;
                     default :
                        {
                           editor_msg( "red", QString( "ZENO produced multiple values for parameter: %1\n" ).arg( param_name[ i ] ) );
                        }
                        break;
                     }
                  }

                  us_qdebug( QString( "volume %1" ).arg( this_data.use_beads_vol ) );

                  // computed

                  if ( zeno_cxx ) {
                     // must compute proper eta
                     double use_mass = hydro.mass_correction ? hydro.mass : sum_mass;
                     this_data.results.mass = use_mass;
                     this_data.use_bead_mass = use_mass;
#if defined( NEW_ZENO_PRE_5 )
                     if ( use_mass ) {
                        // double tmp = (4e0/3e0) * this_data.results.viscosity * AVOGADRO * 1e-24 * sum_volume;
                        // this_data.results.viscosity = tmp / sum_mass;
                        this_data.results.viscosity *= this_data.use_beads_vol / ( use_mass * 1.660538921 );
                     } else {
                        this_data.results.viscosity = 0e0;
                     }

                     this_data.tra_fric_coef = 6e0 * M_PI * use_solvent_visc() * this_data.results.rs * 1e-1;
                     
                     if ( this_data.tra_fric_coef ) {
                        // 1.3864852e-8 is  boltzman's constant with a conversion, probably needs fconv
                        this_data.results.D20w = ( K0 + hydro.temperature ) * 1.38064852e-8 / ( this_data.tra_fric_coef * fconv );
                     } else {
                        this_data.results.D20w = 0e0;
                     }
#endif
                  }

               
                  // us_qdebug( QString( "fric coeff %1" ).arg( this_data.tra_fric_coef ) );


                  {
                     double fconv = pow(10.0, this_data.hydro.unit + 9);
                     // us_qdebug( QString( "fconv %1" ).arg( fconv ) );

                     // frictional ratio

                     // us_qdebug(
                     //        QString( "tra_fric_coeff %1\nrs             %2\nvisc           %3\n" )
                     //        .arg( this_data.tra_fric_coef )
                     //        .arg( this_data.results.rs )
                     //        .arg( this_data.results.viscosity )
                     //        );


                     if ( 
                         this_data.results.viscosity &&
                         this_data.results.mass &&
                         this_data.results.vbar
                          ) {
                        //                  this_data.results.ff0 = this_data.tra_fric_coef / ( 6e0 * M_PI *  this_data.results.viscosity * this_data.results.rs );

                        // us_qdebug( 
                        //        QString( "f is %1 ETAo %2 partvol %3 fconv %4 mass %5" )
                        //        .arg( this_data.tra_fric_coef )
                        //        .arg( use_solvent_visc() * 1e-2 )
                        //        .arg( this_data.results.vbar )
                        //        .arg( 1 )
                        //        .arg( this_data.results.mass )
                        //         );

                        this_data.results.ff0 = 
                           this_data.tra_fric_coef * 10 / 
                           ( fconv * 6e0 * M_PI *  use_solvent_visc() * 
                             pow( 3.0 * this_data.results.mass * this_data.results.vbar / (4.0 * M_PI * AVOGADRO), 1.0/3.0 ) );

                        this_data.results.ff0_sd = this_data.results.ff0 * this_data.tra_fric_coef_sd / this_data.tra_fric_coef;

                        us_qdebug( QString( "computed ff0 %1" ).arg( this_data.results.ff0 ) );
                     }

                     // s20w

                     if (this_data.tra_fric_coef ) {
                        // us_qdebug( 
                        //        QString( "s20w mass is %1 partvol %2 DENS %3 f %4 d20w %5" )
                        //        .arg( this_data.results.mass )
                        //        .arg( this_data.results.vbar )
                        //        .arg( use_solvent_dens() )
                        //        .arg( this_data.tra_fric_coef ) 
                        //        .arg( this_data.results.D20w ) 
                        //         );
                        this_data.results.s20w = 
                           // previous way
                           //    ( this_data.results.mass * 1e20 * 
                           //      ( 1e0 - this_data.results.vbar * use_solvent_dens() ) / 
                           //      ( this_data.tra_fric_coef * fconv * AVOGADRO ) );
                        
                           this_data.results.mass * 1e22 *
                           ( 1e0 - ( this_data.results.vbar * use_solvent_dens() ) ) /
                           ( 6e0 * M_PI * use_solvent_visc() * this_data.results.rs * AVOGADRO );

                        this_data.results.s20w_sd =
                           this_data.results.s20w * this_data.results.rs_sd / this_data.results.rs;

                        // alternate way via Dt
                        // double alt_s20w = 
                        //    1e13 * ( this_data.results.mass * this_data.results.D20w * 
                        //             ( 1e0 - ( this_data.results.vbar * use_solvent_dens() ) ) ) /
                        //    ( Rbar * ( this_data.hydro.temperature + K0 ) );
                        // us_qdebug( QString( "s20w old way %1, dt way %2\n" ).arg( this_data.results.s20w ).arg( alt_s20w ) );
                     }
                  
                     // bead model rg

                     if ( this_data.results.mass ) {
                        // compute center of mass
                        point cm;
                        cm.axis[ 0 ] = 0.0;
                        cm.axis[ 1 ] = 0.0;
                        cm.axis[ 2 ] = 0.0;
                        double total_cm_mw = 0e0;

                        for ( unsigned int i = 0; i < bead_model.size(); ++i )
                        {
                           PDB_atom *this_atom = &(bead_model[i]);
                           double bead_tot_mw = this_atom->bead_mw + this_atom->bead_ionized_mw_delta;
                           cm.axis[ 0 ] += bead_tot_mw * this_atom->bead_coordinate.axis[ 0 ];
                           cm.axis[ 1 ] += bead_tot_mw * this_atom->bead_coordinate.axis[ 1 ];
                           cm.axis[ 2 ] += bead_tot_mw * this_atom->bead_coordinate.axis[ 2 ];
                           total_cm_mw += bead_tot_mw;
                        }

                        cm.axis[ 0 ] /= total_cm_mw;
                        cm.axis[ 1 ] /= total_cm_mw;
                        cm.axis[ 2 ] /= total_cm_mw;

                        // now compute Rg
                        double Rg2 = 0e0;

                        for ( unsigned int i = 0; i < bead_model.size(); ++i )
                        {
                           PDB_atom *this_atom = &(bead_model[i]);
                           Rg2 += (double) ( this_atom->bead_mw + this_atom->bead_ionized_mw_delta ) * 
                              ( 
                               (double) ( this_atom->bead_coordinate.axis[ 0 ] - cm.axis[ 0 ] ) *
                               ( this_atom->bead_coordinate.axis[ 0 ] - cm.axis[ 0 ] ) +
                               (double) ( this_atom->bead_coordinate.axis[ 1 ] - cm.axis[ 1 ] ) *
                               ( this_atom->bead_coordinate.axis[ 1 ] - cm.axis[ 1 ] ) +
                               (double) ( this_atom->bead_coordinate.axis[ 2 ] - cm.axis[ 2 ] ) *
                               ( this_atom->bead_coordinate.axis[ 2 ] - cm.axis[ 2 ] ) 
                                );
                        }

                        double Rg = sqrt( Rg2 / total_cm_mw );

                        this_data.results.rg = Rg * fconv;
                        us_qdebug( QString( "rg %1 fconv %2 rg2 %3 total_cm_mw %4" ).arg( Rg ).arg( fconv ).arg( Rg2 ).arg( total_cm_mw ) );
                     }
                  }

                  {
                     // dimensionless params
                     double use_rs    = this_data.results.rs;
                     double use_rs_sd = this_data.results.rs_sd;
                     if ( hydro.unit == -10 ) {
                        use_rs    *= 10;
                        use_rs_sd *= 10;
                     }

                     if ( this_data.results.rs ) {
                        this_data.dt_d0 = 1e0 / use_rs;
                        if ( this_data.results.rs_sd ) {
                           this_data.dt_d0_sd = use_rs_sd / ( use_rs * use_rs );
                        }
                     }

                     if ( !has_overlap && sum_volume ) {
                        this_data.dimless_eta = this_data.zeno_eta_prefactor * this_data.zeno_mep / sum_volume;
                        if ( this_data.zeno_eta_prefactor_sd && this_data.zeno_mep_sd ) {
                           this_data.dimless_eta_sd = ( this_data.zeno_eta_prefactor * this_data.zeno_mep / sum_volume ) *
                              sqrt( this_data.zeno_eta_prefactor_sd * this_data.zeno_eta_prefactor_sd / ( this_data.zeno_eta_prefactor * this_data.zeno_eta_prefactor ) +
                                    this_data.zeno_mep_sd           * this_data.zeno_mep_sd           / ( this_data.zeno_mep * this_data.zeno_mep ) );
                        }
                     }
                  }

                  // nsa physical stats

                  {
                     vector < vector < PDB_atom > >  save_bead_models = bead_models;
                     saxs_util->bead_models.resize( 1 );
                     saxs_util->bead_models[ 0 ] = bead_model;
                     if ( "empty model" != saxs_util->nsa_physical_stats() )
                     {
                        this_data.tot_volume_of = saxs_util->nsa_physical_stats_map[ "result excluded volume" ].toDouble();

                        double fconv = pow(10.0, this_data.hydro.unit + 9);
                  
                        this_data.max_ext_x = saxs_util->nsa_physical_stats_map[ "result radial extent bounding box size x" ].toDouble() * fconv;
                        this_data.max_ext_y = saxs_util->nsa_physical_stats_map[ "result radial extent bounding box size y" ].toDouble() * fconv;
                        this_data.max_ext_z = saxs_util->nsa_physical_stats_map[ "result radial extent bounding box size z" ].toDouble() * fconv;

                        this_data.axi_ratios_xz = saxs_util->nsa_physical_stats_map[ "result radial extent axial ratios x:z" ].toDouble();
                        this_data.axi_ratios_xy = saxs_util->nsa_physical_stats_map[ "result radial extent axial ratios x:y" ].toDouble();
                        this_data.axi_ratios_yz = saxs_util->nsa_physical_stats_map[ "result radial extent axial ratios y:z" ].toDouble();
                     } else {
                        editor_msg( "red", QString( "Internal error: Bead model is empty?" ) );
                     }
                  }


                  // append to zno file
                  {
                     QString add_to_zeno;
                     if ( zeno_cxx ) {

                        add_to_zeno =
#if defined( NEW_ZENO_PRE_5 )
                           "---------------------------------------------------------\n" 
                           + us_tr( "N.B. The above parameters, directly calculated by ZENO,\n"
                                 "are not linked to the model's physical dimensions,\n"
                                 "in contrast to the US-SOMO Derived parameters below.\n" )
                           + "---------------------------------------------------------\n"
#else
                           "\n---------------------------------------------------------\n"
                           + us_tr(
                                "Summary information\n"
                                "The Sedimentation coefficient and Frictional Ratio are\n"
                                "derived from the ZENO results.\n"
                                )
                           + "---------------------------------------------------------\n"
#endif
                           ;
                     }

                     add_to_zeno += QString( "\nZENO computed on %1 Model %2%3\n" ).arg( project ).arg( current_model + 1 ).arg( bead_model_suffix.length() ? (" Bead model suffix: " + bead_model_suffix) : "" );
                     add_to_zeno += QString( "Number of beads used: %1\n" ).arg( bead_model.size() );
                     add_to_zeno += QString( "MW: %1 [Da]\n" ).arg( sum_mass );
                     add_to_zeno += pH_msg();
                     add_to_zeno += vbar_msg( model_vector[ current_model ].vbar, true );
                     add_to_zeno += visc_dens_msg( true );
                     if ( hydro.zeno_surface_thickness_from_rg ) {
                        add_to_zeno += QString( "Computed skin thickness: %1\n" )
                           .arg( hydro.zeno_surface_thickness_from_rg_a + hydro.zeno_surface_thickness_from_rg_b * model_vector[ current_model ].Rg );
                     }
                     
                     if ( hydro.mass_correction ) {
                        add_to_zeno += QString( "Manually corrected MW: %1 [Da]\n" ).arg( hydro.mass );
                     }

                     add_to_zeno += 
                        QString( 
                                us_tr( 
                                   "\n"
                                   "US-SOMO Derived Parameters:\n"
                                   "\n"
                                   " Sedimentation Coefficient             s : %1%2\n"
                                   " Frictional Ratio                   f/f0 : %3%4\n"
                                   " Radius of Gyration                   Rg : %5\n"
                                    ) )
                        .arg( QString( "" ).sprintf( "%4.2e S" , this_data.results.s20w ) )
                        .arg( this_data.results.s20w_sd ? QString( "" ).sprintf( " [%4.2e]"      , this_data.results.s20w_sd ) : "" )
                        .arg( QString( "" ).sprintf( "%3.2f"   , this_data.results.ff0  ) )
                        .arg( this_data.results.ff0_sd ? QString( "" ).sprintf( " [%4.2e]"      , this_data.results.ff0_sd ) : "" )
                        .arg( QString( "" ).sprintf( "%4.2e nm", this_data.results.rg   ) )
                        ;

                     if ( zeno_cxx ) {
                        add_to_zeno +=
                           QString( 
                                   " Stokes Radius                        Rs : %1%2\n"
                                   " Intrinsic Viscosity               [eta] : %3%4\n"
                                   " Tr. Frictional coefficient            f : %5%6\n"
                                   " Tr. Diffusion Coefficient            Dt : %7%8\n"
                                    )
                           .arg( QString( "" ).sprintf( "%4.2e nm"      , this_data.results.rs ) )
                           .arg( this_data.results.rs_sd ? QString( "" ).sprintf( " [%4.2e]"      , this_data.results.rs_sd ) : "" )
                           .arg( QString( "" ).sprintf( "%4.2e cm^3/g"  , this_data.results.viscosity ) )
                           .arg( this_data.results.viscosity_sd ? QString( "" ).sprintf( " [%4.2e]"      , this_data.results.viscosity_sd ) : "" )
                           .arg( QString( "" ).sprintf( "%4.2e g/s"     , this_data.tra_fric_coef ) )
                           .arg( this_data.tra_fric_coef_sd ? QString( "" ).sprintf( " [%4.2e]"      , this_data.tra_fric_coef_sd ) : "" )
                           .arg( QString( "" ).sprintf( "%4.2e cm^2/sec", this_data.results.D20w ) )
                           .arg( this_data.results.D20w_sd ? QString( "" ).sprintf( " [%4.2e]"      , this_data.results.D20w_sd ) : "" )
                           ;

                        {
                           double use_rs    = this_data.results.rs;
                           double use_rs_sd = this_data.results.rs_sd;
                           if ( hydro.unit == -10 ) {
                              use_rs    *= 10;
                              use_rs_sd *= 10;
                           }

                           add_to_zeno +=
                              QString(
                                      "                                   Dt/d0 : %1%2\n"
                                      )
                              .arg( QString( "" ).sprintf( "%4.2e"      , this_data.dt_d0 ) )
                              .arg( this_data.dt_d0_sd ? QString( "" ).sprintf( " [%4.2e]"  , this_data.dt_d0_sd ) : "" )
                              ;
                           
                           if ( !has_overlap ) {
                              add_to_zeno +=
                                 QString(
                                         " Dimensionless Intrinsic Viscosity [eta] : %1%2\n"
                                         )
                                 .arg( QString( "" ).sprintf( "%4.2e"      , this_data.dimless_eta ) )
                                 .arg( this_data.dimless_eta_sd ? QString( "" ).sprintf( " [%4.2e]"      , this_data.dimless_eta_sd ) : "" )
                                 ;
                           }
                        }
                     }

                     add_to_zeno +=
                        QString(
                                us_tr( 
                                      " Maximum extension                     X : %1\n"
                                      " Maximum extension                     Y : %2\n"
                                      " Maximum extension                     Z : %3\n"
                                      " Axial ratio                         X:Z : %4\n"
                                      " Axial ratio                         X:Y : %5\n"
                                      " Axial ratio                         Y:Z : %6\n"
                                       ) )
                        .arg( QString( "%1 nm" ).arg( this_data.max_ext_x, 0, 'g', 4 ) )
                        .arg( QString( "%1 nm" ).arg( this_data.max_ext_y, 0, 'g', 4 ) )
                        .arg( QString( "%1 nm" ).arg( this_data.max_ext_z, 0, 'g', 4 ) )
                        .arg( this_data.axi_ratios_xz, 0, 'g', 3 )
                        .arg( this_data.axi_ratios_xy, 0, 'g', 3 )
                        .arg( this_data.axi_ratios_yz, 0, 'g', 3 )
                        ;


                     if ( !batch_avg_hydro_active() && !zeno_mm ) {
                        QFile f( last_hydro_res );
                        if ( f.exists() && f.open( QIODevice::WriteOnly | QIODevice::Append ) )
                        {
                           QTextStream ts( &f );
                           ts << add_to_zeno;
                           f.close();
                        }
                     }
                     this_data.hydro_res += add_to_zeno;
                     if ( zeno_mm ) {
                        zeno_mm_results += this_data.hydro_res;
                     }
                  }

                  if ( zeno_mm ) {
                     zeno_mm_save_params.data_vector.push_back( this_data );
                  }

                  if ( batch_widget &&
                       batch_window->save_batch_active )
                  {
                     save_params.data_vector.push_back( this_data );
                     printf("batch save on, push back info into save_params!\n");
                  }

                  // this needs to be averaged for multiple runs

                  if ( !zeno_results.num_models ) {
                     zeno_results.method  = "Zeno";
                     zeno_model_list = QString( "%1" ).arg( current_model + 1 );
                  } else {
                     zeno_model_list += QString( ",%1" ).arg( current_model + 1 );
                  }                   

                  zeno_results.num_models++;

                  zeno_results.mass                  += this_data.results.mass;
                  zeno_results.s20w                  += this_data.results.s20w;
                  zeno_results.D20w                  += this_data.results.D20w;
                  zeno_results.viscosity             += this_data.results.viscosity;
                  zeno_results.rs                    += this_data.results.rs;
                  zeno_results.rg                    += this_data.results.rg;
                  zeno_results.vbar                  += this_data.results.vbar;
                  zeno_results.ff0                   += this_data.results.ff0;
                  zeno_results.used_beads            += this_data.results.used_beads;
                  zeno_results.total_beads           += this_data.results.total_beads;
                  if ( this_data.results.rs_sd ) {
                     zeno_results.rs_sd              = this_data.results.rs_sd;
                  }
                  if ( this_data.results.viscosity_sd ) {
                     zeno_results.viscosity_sd       = this_data.results.viscosity_sd;
                  }
                  if ( this_data.results.D20w_sd ) {
                     zeno_results.D20w_sd            = this_data.results.D20w_sd;
                  }

                  // no sd: zeno_results2.mass                 += this_data.results.mass        * this_data.results.mass;
                  zeno_results2.s20w                 += this_data.results.s20w        * this_data.results.s20w;
                  zeno_results2.D20w                 += this_data.results.D20w        * this_data.results.D20w;
                  zeno_results2.viscosity            += this_data.results.viscosity   * this_data.results.viscosity;
                  zeno_results2.rs                   += this_data.results.rs          * this_data.results.rs;
                  zeno_results2.rg                   += this_data.results.rg          * this_data.results.rg;
                  // no sd: zeno_results2.vbar                 += this_data.results.vbar        * this_data.results.vbar;
                  zeno_results2.ff0                  += this_data.results.ff0         * this_data.results.ff0;
                  zeno_results2.used_beads           += this_data.results.used_beads  * this_data.results.used_beads;
                  zeno_results2.total_beads          += this_data.results.total_beads * this_data.results.total_beads;

                  bool create_hydro_res = !(batch_widget &&
                                            batch_window->save_batch_active);
                  if ( saveParams && create_hydro_res & !zeno_mm )
                  {
                     QString fname = this_data.results.name + ".zeno.csv";
                     if ( !overwrite_hydro ) {
                        fname = fileNameCheck( fname, 0, this );
                     }
                     FILE *of = us_fopen(fname, "wb");
                     if ( of )
                     {
                        fprintf(of, "%s", save_util->header().toLatin1().data());

                        fprintf(of, "%s", save_util->dataString(&this_data).toLatin1().data());
                        fclose(of);
                     }
                  }
                  // // print out results ?
                  // save_util->header();
                  // save_util->dataString(&this_data);
               }
            } // repeats
         }
      }
   }

   {
      double num = (double) zeno_results.num_models;
      if ( num <= 1 ) {
         results = zeno_results;
      } else {
         zeno_results.name += " models:" + zeno_model_list;
         double numinv = 1e0 / num;
         zeno_results.mass          *= numinv;
         zeno_results.s20w          *= numinv;
         zeno_results.D20w          *= numinv;
         zeno_results.viscosity     *= numinv;
         zeno_results.rs            *= numinv;
         zeno_results.rg            *= numinv;
         zeno_results.vbar          *= numinv;
         zeno_results.ff0           *= numinv;
         zeno_results.used_beads    *= numinv;
         zeno_results.total_beads   *= numinv;
         if ( num <= 1 ) {
            results = zeno_results;
         } else {
            double numdecinv = 1e0 / ( num - 1e0 );
         
            zeno_results.s20w_sd           = sqrt( fabs( ( zeno_results2.s20w        - zeno_results.s20w        * zeno_results.s20w        * num ) * numdecinv ) );
            zeno_results.D20w_sd           = sqrt( fabs( ( zeno_results2.D20w        - zeno_results.D20w        * zeno_results.D20w        * num ) * numdecinv ) );
            zeno_results.viscosity_sd      = sqrt( fabs( ( zeno_results2.viscosity   - zeno_results.viscosity   * zeno_results.viscosity   * num ) * numdecinv ) );
            zeno_results.rs_sd             = sqrt( fabs( ( zeno_results2.rs          - zeno_results.rs          * zeno_results.rs          * num ) * numdecinv ) );
            zeno_results.rg_sd             = sqrt( fabs( ( zeno_results2.rg          - zeno_results.rg          * zeno_results.rg          * num ) * numdecinv ) );
            zeno_results.ff0_sd            = sqrt( fabs( ( zeno_results2.ff0         - zeno_results.ff0         * zeno_results.ff0         * num ) * numdecinv ) );
            zeno_results.used_beads_sd     = sqrt( fabs( ( zeno_results2.used_beads  - (double) zeno_results.used_beads  * zeno_results.used_beads  * num ) * numdecinv ) );
            zeno_results.total_beads_sd    = sqrt( fabs( ( zeno_results2.total_beads - (double) zeno_results.total_beads * zeno_results.total_beads * num ) * numdecinv ) );
            
            results = zeno_results;
         }
      }
   }      

   if ( zeno_mm ) {
      vector < save_data > stats = save_util->stats( & zeno_mm_save_params.data_vector );

      {
         QString zeno_out_name = zeno_mm_name + ".zno";
         if ( !overwrite_hydro ) {
            zeno_out_name = fileNameCheck( zeno_out_name, 0, this );
         }
         QFile f( zeno_out_name );
         if ( !f.open( QIODevice::WriteOnly ) ) {
            editor_msg( "red", QString( us_tr( "Error: could not open output file %1 for writing" ) ).arg( zeno_out_name ) );
         } else {
            QTextStream t( &f );
            t << zeno_mm_results;
            t << save_util->hydroFormatStats( stats, US_Hydrodyn_Save::HYDRO_ZENO );
            editor_msg( "dark blue", QString( us_tr( "Wrote %1" ) ).arg( zeno_out_name ) );
            f.close();
            last_hydro_res = zeno_out_name;
         }
      }

      bool create_hydro_res = !(batch_widget &&
                                batch_window->save_batch_active);

      if ( saveParams && create_hydro_res ) {
         QString zeno_out_name = zeno_mm_name + ".zeno.csv";
         if ( !overwrite_hydro ) {
            zeno_out_name = fileNameCheck( zeno_out_name, 0, this );
         }
         QFile f( zeno_out_name );
         if ( !f.open( QIODevice::WriteOnly ) ) {
            editor_msg( "red", QString( us_tr( "Error: could not open output file %1 for writing" ) ).arg( zeno_out_name ) );
         } else {
            QTextStream t( &f );
            t << save_util->header().toLatin1().data();

            for ( int i = 0; i < (int) zeno_mm_save_params.data_vector.size(); ++i ) {
               t << save_util->dataString( & zeno_mm_save_params.data_vector[ i ] ).toLatin1().data();
            }
            for ( int i = 0; i < (int) stats.size(); ++i ) {
               t << save_util->dataString( & stats[ i ] ).toLatin1().data();
            }
               
            editor_msg( "dark blue", QString( us_tr( "Wrote %1" ) ).arg( zeno_out_name ) );
            f.close();
         }

      }
   }

   QDir::setCurrent(somo_tmp_dir);

   pb_show_hydro_results->setEnabled( true );
   pb_calc_hydro->setEnabled( was_hydro_enabled );
   pb_calc_zeno->setEnabled(true);
   pb_calc_grpy->setEnabled(true);
   pb_calc_hullrad->setEnabled(true);
   pb_bead_saxs->setEnabled(true);
   pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );

   pb_stop_calc->setEnabled(false);
   editor_msg( "black", "Calculate RB hydrodynamics ZENO completed\n");
   editor_msg( "dark blue", info_cite( "zeno" ) );

   if ( advanced_config.auto_show_hydro ) 
   {
      show_zeno();
   }
   play_sounds(1);
   progress->reset();
   mprogress->hide();
   qApp->processEvents();
   return true;
}

void US_Hydrodyn::show_zeno()
{
   cout << "show zeno\n";
}

void US_Hydrodyn::show_zeno_options()
{
   if ( hydro_zeno_widget)
   {
      if ( hydro_zeno_window->isVisible() )
      {
         hydro_zeno_window->raise();
      }
      else
      {
         hydro_zeno_window->show();
      }
      return;
   }
   else
   {
      hydro_zeno_window = new US_Hydrodyn_Hydro_Zeno( &hydro, &hydro_zeno_widget, this );
      hydro_zeno_window->show();
   }
}

