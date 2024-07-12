// ----------------------------------- asab1.c -------------------------------------
// #warning asab1.c 

/*  ENGLISH VERSION 17 MAY 2002   */
/*  UPDATED SEPTEMBER 2002        */
/*  UPDATED DECEMBER 2002 IN GLASGOW FOR COLOR CODING DIFFERENTLY THE CARBOHYDRATES, BUT THEN NOT UTILIZED  */
/*  ALSO FIXED OLD MISTAKES: 1-MISPELLED "PPO" INSTEAD OF "PRO" IN THE ASA OF PEPTIDE BOND ROUTINE */
/*                           2-COUNTING ONE ATOM LESS WHEN "PRO" IS FOUND IN PEPTIDE BOND ROUTINE */
/*              3-MISASSIGNING A "6" VALUE FOR "N" OF A LEU PEPTIDE BOND (FIXED BUT NOT UNDERSTOOD) */
/*  UPDATED FEBRUARY 2003 IN GENOVA, FIXED ANOTHER MISTAKE IN THE BEAD COLOR ASSIGNMENT */
/*                                   FIXED RE-CHECK ROUTINE */
/*  UPDATED APRIL 2003 IN GENOVA, ADDED OCTYL GLUCOSIDE AND PSV  */
/*  UPDATED JUNE 2005 IN GENOVA, FIXED INIT3.C PROBLEMS  */
/*  FIXED JUNE 2006 IN GENOVA, INCORRECT ASSIGNMENT OF PEPTIDE BOND CODES FOR BURIED/EXPOSED DUE TO ORDER IN PDB FILE */
/*  UPDATED AUGUST 2006 WITH AUTOMATIC SEARCH FOR TABELLA1.COR FILE BY M. NOLLMANN  */

#undef DEBUG

#define   false   0
#define   true   1

#define US_HYDRODYN_ASAB1_ERR_MEMORY_ALLOC -1

// #include "num.h"

#include "../include/us_hydrodyn_asab1.h"
#include <float.h>
// #if defined(WIN32)
// #  define isnan _isnan
// #endif

// #include "dati.h"
// #include "var.h"
// #define  NMAX   30000      /* max number of atoms in the molecule */
// #define  NMAX1   4000
#define  PI   ((float) 3.141592654)
#define  SMAX   20
#define  IMAX   20

struct dati1
{
   float x, y, z;      /* coordinates of the sphere center  */
   float r;         /* radius of the sphere              */
   int m;         /* mass                             */
   int col;         /* color                            */
   char elm[4];
   char amin[4];
   char sugar[4];
   float tab;
   float vol;
   char descr[10];
};

struct dati2
{
   float xi, yi, ai;      /* coordinates of the intersection angle */
};

// extern struct dati1  dt[NMAX], dts[NMAX];
// extern struct dati2  *trans;
// extern struct dati2  inter[NMAX][IMAX];

// extern struct dati2  dis[IMAX];

// extern char ridotto[20],ridotto_rmc[20];

// extern char outfile[30],outfile1[30],outfile2[30],outris[30];

// extern char nome[10][SMAX]; /* array for the name of the molecule to be drawn */
// extern char ragcol[SMAX];       /* array for the file name of the radii, masses and colors file */
// 
// extern int nat,numol,cont1,numdis; /* index of the sphere under process */
// extern int nc;
// extern int form,form1,vel;  
// extern int pos[NMAX],passi,quota,q1,q2;
// extern int indec[NMAX1];
// extern int corr,sim,arr[30],rbulk; 
// extern int flag1;

// extern float raggio;          /* radius of the bead */
// extern float psv;             /* partial specific volume */
// extern float fl,k;
// extern float maxx,maxy,minx,miny,maxz,minz;
// extern float xp1,xp2,yp1,yp2;
// extern float asa[NMAX];
// extern float m1,m2,medio,base,altez;
// extern float arr1[60],ro,ro1,xm,ym,zm;
// extern float rprobe;

// extern double ix1,ix2,iy1,iy2; /* coordinates of the intersection points */

// extern void em(char *s);

static void mami1();
static void ragir();
static void init();
static void init2();
static void initarray();
static void formato();
static void scala();
static void plotinit();
static void plotcircle();
static void plotarc(int);
static void plotend();
static void ordcol();
static void ord_a();
static void calcdis();
static void cercaint(int);
static void valida(float, float, int);
static void pulisci();
static void raggio_probe();
static float ang(float, float);
static float dist(float, float, float, float);
static float unit_conv;

static FILE *mol;
static FILE *mol1;
static FILE *mol2;

// static FILE *rmc;
// static FILE *pl;
// static FILE *ord;
static FILE *pippa;
// static FILE *pippa1;
// static FILE *brook;

static struct dati2 *trans;

static struct dati2 dis[IMAX];

// static char nome[10][SMAX];   /* array for the models to be drawn */
static char ridotto[20], ridotto_rmc[20];   /* array for the output file name after ASA re-check */

static char ragcol[SMAX];   /* array for the file name where radii, masses and colors of the beads are stored */

static int nat;
// static int numol;
static int numdis;   /* index of the bead under examination */
static int nc;
static int form, form1, vel;
static int quota, passi, cont1;
static int q1;
static int q2;
static int corr;
static int sim;
// static int arr[30];
static int rbulk;
static int flag1;
static int countb;

static char outfile[30];
static char outfile1[30];
static char outfile2[30];
static char outris[30];

static float raggio;      /* radius of the sphere */
static float psv;      /* partial specific volume */
static float fl, k;
static float maxx, minx, maxy, miny, maxz, minz;
static float xp1, xp2, yp1, yp2;
static float m1, m2, medio, base, altez;
static float arr1[60], ro, ro1, xm, ym, zm;
static float min_asa = (float) -1.0;
static float rprobe = (float) 0.0;

static double ix1;
static double ix2;
static double iy1;
static double iy2;
// static double ia;   /* coordinates of the intersection points */

static struct dati1 *dt = 0;       // [NMAX]
static struct dati1 *dts = 0;      // [NMAX]
static struct dati2 *inter = 0;    // [NMAX][IMAX];
static int *pos = 0;               // [NMAX];
static int *indec = 0;             // [NMAX1];
static int *f = 0;                 // [NMAX1];
static int *interna = 0;           // [NMAX1];
static float *asa = 0;             // [NMAX];

static int nmax;
static int nmax1;
static QProgressBar *progress;
static QTextEdit *editor;
static asa_options *asa_opts;
static hydro_results *results;
static bool recheck;
static vector <PDB_atom *> active_atoms;
static US_Hydrodyn *us_hydrodyn;

// static void
// em(const char *s)
// {
//    puts(s);
//    fflush(stdout);
// }

static void
asab1_free_alloced()
{
   if (dt)
   {
      free(dt);
      dt = 0;
   }
   if (dts)
   {
      free(dts);
      dts = 0;
   }
   if (inter)
   {
      free(inter);
      inter = 0;
   }
   if (pos)
   {
      free(pos);
      pos = 0;
   }
   if (indec)
   {
      free(indec);
      indec = 0;
   }
   if (f)
   {
      free(f);
      f = 0;
   }
   if (interna)
   {
      free(interna);
      interna = 0;
   }
   if (asa)
   {
      free(asa);
      asa = 0;
   }
   us_hydrodyn->lbl_core_progress->setText("");
}

static int
asab1_alloc()
{
   dt = (struct dati1 *) malloc(nmax * sizeof(struct dati1));
   if (!dt)
   {
      asab1_free_alloced();
      fprintf(stderr, "memory allocation error\n");
      return US_HYDRODYN_ASAB1_ERR_MEMORY_ALLOC;
   }
   memset(dt, 0, nmax * sizeof(struct dati1));

   dts = (struct dati1 *) malloc(nmax * sizeof(struct dati1));
   if (!dts)
   {
      asab1_free_alloced();
      fprintf(stderr, "memory allocation error\n");
      return US_HYDRODYN_ASAB1_ERR_MEMORY_ALLOC;
   }
   memset(dts, 0, nmax * sizeof(struct dati1));

   inter = (struct dati2 *) malloc(IMAX * nmax * sizeof(struct dati2));
   if (!inter)
   {
      asab1_free_alloced();
      fprintf(stderr, "memory allocation error\n");
      return US_HYDRODYN_ASAB1_ERR_MEMORY_ALLOC;
   }
   memset(inter, 0, IMAX * nmax * sizeof(struct dati2));

   pos = (int *) malloc(nmax * sizeof(int));
   if (!pos)
   {
      asab1_free_alloced();
      fprintf(stderr, "memory allocation error\n");
      return US_HYDRODYN_ASAB1_ERR_MEMORY_ALLOC;
   }
   memset(pos, 0, nmax * sizeof(int));

   indec = (int *) malloc(nmax1 * sizeof(int));
   if (!indec)
   {
      asab1_free_alloced();
      fprintf(stderr, "memory allocation error\n");
      return US_HYDRODYN_ASAB1_ERR_MEMORY_ALLOC;
   }
   memset(indec, 0, nmax1 * sizeof(int));

   f = (int *) malloc(nmax1 * sizeof(int));
   if (!f)
   {
      asab1_free_alloced();
      fprintf(stderr, "memory allocation error\n");
      return US_HYDRODYN_ASAB1_ERR_MEMORY_ALLOC;
   }
   memset(f, 0, nmax1 * sizeof(int));

   interna = (int *) malloc(nmax1 * sizeof(int));
   if (!interna)
   {
      asab1_free_alloced();
      fprintf(stderr, "memory allocation error\n");
      return US_HYDRODYN_ASAB1_ERR_MEMORY_ALLOC;
   }
   memset(interna, 0, nmax1 * sizeof(int));

   asa = (float *) malloc(nmax * sizeof(float));
   if (!asa)
   {
      asab1_free_alloced();
      fprintf(stderr, "memory allocation error\n");
      return US_HYDRODYN_ASAB1_ERR_MEMORY_ALLOC;
   }
   memset(asa, 0, nmax * sizeof(float));

   return 0;
}

int
us_hydrodyn_asab1_main(vector <PDB_atom *> use_active_atoms, 
                       asa_options *use_asa_opts,
                       hydro_results *use_results,
                       bool use_recheck,
                       QProgressBar *use_progress,
                       QTextEdit *use_editor,
                       US_Hydrodyn *use_us_hydrodyn
                       )
{

   // em("asab1_main");
   // em("asab1_main 1");
   //  nmax1 = 4000;
   // em("asab1_main 2");
   progress = use_progress;
   editor = use_editor;
   asa_opts = use_asa_opts;
   results = use_results;
   recheck = use_recheck;
   active_atoms = use_active_atoms;
   nmax = nmax1 = active_atoms.size();
   us_hydrodyn = use_us_hydrodyn;
   if (int retval = asab1_alloc())
   {
      return retval;
   }
   unit_conv = pow(10.0,9 + use_us_hydrodyn->hydro.unit);
   // qDebug() << "unit_conv " << unit_conv;


   int i, ii, l, j, s, kk, kkk, ini, contatom, contatom1, indCA, indC, indO;
   int posiz, massa = 0, check_asa = 0;
   float sommarc, Dz, d1z, zz, temp, asamin, asalevel, asapep, asatot, perc, voltot;
   char azoto[2];
   char carbonio0[3];
   char carbonio1[3];
   char carbonio2[4];
   //    char pippo[10];
   // char pluto[1];
   // int topolino;

   struct dati1 *dd1, *dd2;

   /* memory addressing   */
   //    trans = &(inter[0][0]);
   trans = inter;

   flag1 = 0;

#if defined( ASA_DEBUG )
   printf("########################################################\n");
   printf("#    National Institute for Cancer Research (IST)      #\n");
   printf("#          Advanced Biotechnologies Center (CBA)       #\n");
   printf("#                    Genova, ITALY                     #\n");
   printf("########################################################\n");
   printf("#    ASAB1 - Preparing PDB files for bead modelling    #\n");
   printf("#          - Re-checking bead models for ASA           #\n");
   printf("#                                                      #\n");
   printf("#                Version 3.5, August 2006              #\n");
   printf("#                Compiled for 30000 beads              #\n");
   printf("########################################################\n");

   printf("\n\n\n\n To process a PBD file, enter 2\n ");
   printf("To re-check a bead model, enter 3\n\n ");
   printf("--> ");
#endif
   //    scanf("%d", &ini);
   ini = 3;

   /* printf("\n\n%s%d\t%d\n","Valore flag all'inizio, valore ini: ",flag1,ini);
      scanf("%s",pippo);
      getchar(); */

   if (ini != 3)
   {
      /* emptying the files     SOME MOVED INSIDE INIT3.C */

      /*   mol=us_fopen("asaris","w");
           fprintf(mol,"");
           fclose(mol);  */

      mol = us_fopen("plotter", "w");
      //fprintf(mol, "");
      fclose(mol);

      mol = us_fopen("plotter1", "w");
      //fprintf(mol, "");
      fclose(mol);

      mol = us_fopen("controll", "w");
      //fprintf(mol, "");
      fclose(mol);

      /*   mol=us_fopen("provaly","w");
           fprintf(mol,"");
           fclose(mol);

           mol=us_fopen("provaly1","w");
           fprintf(mol,"");
           fclose(mol);

           mol=us_fopen("provaly2","w");
           fprintf(mol,"");
           fclose(mol);  */
   }

   /*choice of the probe's radius*/

   raggio_probe();

   if (ini == 2)

   {
      printf("\n\n- Insert the ASA threshold level in Angstroms^2 [usually 10] : ");
      if ( 1 == scanf("%f", &asalevel) ) {};
      // em("s1");
      initarray();
      // em("s2");
   }

   else if (ini == 3)
   {
      // em("s3");
      flag1 = 1;
      init2();
      qApp->processEvents();
      if (us_hydrodyn->stopFlag)
      {
         asab1_free_alloced();
         return -1;
      }
      // em("s4");
      ini = 2;
      check_asa = 1;
      min_asa = asa_opts->threshold_percent;
      while (min_asa < 0.0)
      {
         printf("\n\nASA RE-CHECK  - Insert the minimum ASA %% threshold level [0-99] : ");
         if ( 1 == scanf("%f", &min_asa) ) {};
         if ((min_asa < 0.0) | (min_asa > 99.0))
            min_asa = (float) -1.0;
      }
   }
   else
   {

      // em("s5");
      init();
      // em("s6");
   }
   // em("s7");

   // cout << QString(" rprobe %1\n" ).arg( rprobe );
   // cout << QString(" min_asa %1\n" ).arg( min_asa );

   if (check_asa != 1)
      dt[nat - 1].m = 17;   /* 17 is assigned as mass for OXT - WARNING, good for single-chain structures only!!  */

   qApp->processEvents();
   if (us_hydrodyn->stopFlag)
   {
      asab1_free_alloced();
      return -1;
   }
   // em("s8");
   ragir();
   // em("s9");
   if ( !recheck )
   {
      results->asa_rg_pos = ro;
      results->asa_rg_neg = ro1;
   }

   qApp->processEvents();
   if (us_hydrodyn->stopFlag)
   {
      asab1_free_alloced();
      return -1;
   }

   mol1 = us_fopen("controll", "w");
   for (l = 0; l < nat; l++)
   {
      fprintf(mol1, "%s\t", dt[l].amin);
      fprintf(mol1, "%s\t", dt[l].elm);
      fprintf(mol1, "%d\t", dt[l].col);
      fprintf(mol1, "%d\n", dt[l].m);
   }
   fclose(mol1);

   xm = ym = zm = (float) 0.0;

   for (i = 0; i < nat; i++)
   {
      xm += dt[i].x * dt[i].m;
      ym += dt[i].y * dt[i].m;
      zm += dt[i].z * dt[i].m;

      asa[i] = (float) 0.0;
      pos[i] = 0;
      massa += dt[i].m;
   }

   xm = xm / massa;
   ym = ym / massa;
   zm = zm / massa;

   mami1();
   formato();

   qApp->processEvents();
   if (us_hydrodyn->stopFlag)
   {
      asab1_free_alloced();
      return -1;
   }

   /*
     ord_d();
   */

   Dz = ((float) fabs(maxz - minz)) / passi;
#if defined( ASA_DEBUG )
   printf("\n\n\n");
#endif

   for (i = 1; i < passi; i++)   /* iteration for the number of steps */
   {
      qApp->processEvents();
      if (us_hydrodyn->stopFlag)
      {
         asab1_free_alloced();
         return -1;
      }
      
      zz = maxz - (Dz * i);   /* slab to be examined */
      kk = 0;

      for (l = 0; l < nat; l++)   /* iteration for the number of atoms */
      {
         if (fabs(dt[l].z - zz) < dt[l].r)
         {
            dts[kk] = dt[l];
            dts[kk].z = zz;
            temp = (float) (fabs(dt[l].z - zz));
            dts[kk].r = (float) (sqrt((dt[l].r * dt[l].r) - (temp * temp)));
            pos[kk] = l;
            kk++;
         }
      }

      cont1 = kk;
      for (l = 0; l < cont1; l++)
      {
         indec[l] = 0;
         interna[l] = 0;
         f[l] = 0;
      }

      if (i == quota)
      {
         scala();
         plotinit();
      }

      us_hydrodyn->lbl_core_progress->setText(QString("Iteration %1 of %2 with %3 %4")
                                              .arg(i+1)
                                              .arg(passi)
                                              .arg(cont1)
                                              .arg(recheck ? "beads" : "atoms"));
      qApp->processEvents();

      nc = 0;
#if defined( DEBUG_ASA )
      printf("%s%d\t", "Iteration number  = ", i + 1);
      if (flag1 == 0)
      {
         printf("%s%d  ", "Number of atoms in this iteration = ", cont1);
      }
      else
      {
         printf("%s%d  ", "Number of beads in this iteration = ", cont1);
      }
      fflush(stdout);
#endif

      while (nc < cont1)
      {
         sommarc = (float) 0.0;

         for (s = nc + 1; s < cont1; s++)
         {
            /* nc internal to s */
            if ((dist(dts[nc].x, dts[nc].y, dts[s].x, dts[s].y) + dts[nc].r <= dts[s].r) || (interna[nc] < 0))
            {
               s = cont1;
               goto RET0;
            }
            else
            {
               if (dist(dts[nc].x, dts[nc].y, dts[s].x, dts[s].y) < dts[nc].r + dts[s].r)
               {
                  /* s interna ad nc */
                  if (dist(dts[nc].x, dts[nc].y, dts[s].x, dts[s].y) + dts[s].r <= dts[nc].r)
                     interna[s] = -1;
                  else
                  {
                     f[nc] = f[s] = 1;
                     cercaint(s);
                     valida((float) ix1, (float) iy1, s);
                     valida((float) ix2, (float) iy2, s);
                  }
               }
            }
            /* closing initial for */
         }

         if (indec[nc] == 0)
         {
            if (f[nc] == 1)
               ;
            else
            {
               if (i == quota)
                  plotcircle();
               posiz = pos[nc];
               temp = (float) (sqrt((dt[posiz].r * dt[posiz].r) - (dts[nc].r * dts[nc].r)));
               if (Dz / 2.0 > temp)
                  d1z = temp;
               else
                  d1z = Dz / ((float) 2.0);
               asa[posiz] += (((float) 2.0) * PI * dts[nc].r * dt[posiz].r / dts[nc].r * (Dz / ((float) 2.0) + d1z));
            }
         }
         else
         {
            j = indec[nc];
            ord_a();
            calcdis();

            for (ii = 0; ii < (j / 2); ii++)
            {
               if (dis[ii].ai != 0.0)
               {
                  sommarc += dis[ii].ai;
                  if (i == quota)
                     plotarc(ii);
               }
            }

            posiz = pos[nc];
            temp = (float) (sqrt((dt[posiz].r * dt[posiz].r) - (dts[nc].r * dts[nc].r)));

            if (Dz / 2.0 > temp)
               d1z = temp;
            else
               d1z = Dz / ((float) 2.0);

            temp = PI * dts[nc].r / ((float) 180.0);

            asa[posiz] += sommarc * temp * dt[posiz].r / dts[nc].r * (Dz / ((float) 2.0) + d1z);
         }

      RET0:
         nc = nc + 1;

         /* closing initial while */
      }

      printf("\r");

      /* closing for of the number of steps */
   }

   plotend();
   ordcol();

   if (check_asa != 1)
      pippa = us_fopen(outris, "w");
   else
      goto a300;

   perc = (float) 0.0;
   kkk = 1;
   azoto[0] = 'N';
   azoto[1] = 0;

   strcpy(carbonio0, "C1");
   strcpy(carbonio2, "C11");
   strcpy(carbonio1, "C7");

   /* carboniof[0]='C';
      carboniof[1]='1'; 
      carboniof[2]='1'; 
      carboniof[3]=0; 
      carbonio0[0]='C';
      carbonio0[1]='1'; 
      carbonio0[2]=0; 
      plutone[0]='C';
      plutone[1]='7';
      plutone[2]=0; */

   asamin = asa[0];

   voltot = dt[0].vol;
   asatot = 0;

   fprintf(pippa, " N.\t   Res.       ASA MAXASA   %% \n\n\n");
   for (l = 1; l < nat; l++)   /* GROUPING ASA OF ATOMS INTO ASA OF RESIDUES */
   {
      dd1 = dd2 = dt;
      dd1 += l;
      dd2 += (l - 1);

      /*   printf("\n%d\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n",l,"dd1->descr, dd1->elm, azoto, carbonio0, carbonio1, carbonio2: ",dd1->descr,dd1->elm,azoto,carbonio0,carbonio1,carbonio2);
           scanf("%s",pluto);
           getchar(); */

      /*   if(strcmp(dd1->elm,azoto)!=0)
           asamin+=asa[l];
           else
           {

           printf("\n%s\n","Ora passo");
           scanf("%d",&topolino);    

           if(dt[l-1].tab<.001)
           perc=100*asamin/((float) .001);
           else
           perc=100*asamin/dt[l-1].tab;
           asatot+=asamin;
           voltot+=dt[l].vol;
           fprintf(pippa,"[ %d\t%s ] %5.0f %5.0f  %.1f\n",kkk,dt[l-1].amin,asamin,dt[l-1].tab,perc);
           asamin=asa[l];
           kkk++;
           } */

      /*   if((strcmp(dd1->elm,azoto)==0) || (strcmp(dd1->elm,carbonio0)==0)) / * NEW VERSION INCLUDING CARBOHYDRATES */

      /* NEW VERSION INCLUDING CARBOHYDRATES AND OG */

      if ((strcmp(dd1->elm, azoto) == 0) || (strcmp(dd1->elm, carbonio0) == 0)
          || ((strcmp(dd1->elm, carbonio1) == 0) && (strcmp(dd1->amin, "OG2") == 0)) || ((strcmp(dd1->elm, carbonio2) == 0)
                                                                                         && (strcmp(dd1->amin, "OG3") == 0)))
      {

         /*      printf("\n%s\n","Ora passo");
                 printf("\n%d\t%s%d%s\t%f\t%s\t%f\n",l-1,"dt[",l-1,"].tab: ",dt[l-1].tab,"asamin: ",asamin);
                 scanf("%d",&topolino);     
                 getchar(); */

         if (dt[l - 1].tab < .001)
            perc = 100 * asamin / ((float) .001);
         else
            perc = 100 * asamin / dt[l - 1].tab;
         asatot += asamin;
         voltot += dt[l].vol;
         fprintf(pippa, "[ %d\t%s ]  %5.0f %5.0f  %.1f\n", kkk, dt[l - 1].descr, asamin, dt[l - 1].tab, perc);
         asamin = asa[l];
         kkk++;
      }
      else
         asamin += asa[l];

   }

   qApp->processEvents();
   if (us_hydrodyn->stopFlag)
   {
      asab1_free_alloced();
      return -1;
   }

   asatot += asamin;

   if (dt[l - 1].tab < .001)
      perc = 100 * asamin / ((float) .001);
   else
      perc = ((float) 100.0) * asamin / (dt[l - 1].tab);

   fprintf(pippa, "[ %d\t%s ]  %5.0f %5.0f  %.1f\n\n", kkk, dt[l - 1].descr, asamin, dt[l - 1].tab, perc);

   fprintf(pippa, "\n\n\t%s%.0f\t%s%.1f%s\n", "TOTAL ASA OF THE MOLECULE    =  ", asatot, "[A^2]  (Threshold used: ", asalevel,
           " A^2]");
   fprintf(pippa, "\t%s%.2f\t%s\n", "TOTAL VOLUME OF THE MOLECULE =  ", voltot, "[A^3]");
   fprintf(pippa, "\t%s%.2f\t%s\n", "RADIUS OF GYRATION (+r) =  ", ro, "[A]");
   fprintf(pippa, "\t%s%.2f\t%s\n", "RADIUS OF GYRATION (-r) =  ", ro1, "[A]");
   fprintf(pippa, "\t%s%d\t%s\n", "MASS OF THE MOLECULE    =  ", massa, "[Da]");
   fprintf(pippa, "\t%s%.4f %.4f %.4f [A]\n", "CENTRE OF MASS          =  ", xm, ym, zm);


   fclose(pippa);

   /* computation of the ASA of the peptide bond */

   asapep = (float) 0.0;

   for (l = 1; l < nat; l++)
   {
      dd1 = dt;
      dd1 += l;
      /*    printf("\n%d\t%s\t%s\t%s\t%d\t%s\t%s\n",l,"dd1->elm: ",dd1->elm,"dd1->col: ",dd1->col,"dd1->descr: ",dd1->descr); 
            scanf("%s",pluto);
            getchar(); */
      if (strcmp(dd1->elm, "CA") == 0)
      {
         asapep += asa[l];
         indCA = l;
         /*            printf("\n%s\n","Ora passo in asapep CA");
                       printf("\n%s\t%d\t%s\t%f\t%s\t%f\n","indCA= ",indCA,"asapep: ",asapep,"asa[l]",asa[l]);
                       scanf("%d",&topolino);    
                       getchar(); */
      }

      if (strcmp(dd1->elm, "C") == 0)
      {
         asapep += asa[l];
         indC = l;
         /*             printf("\n%s\n","Ora passo in asapep C");
                        printf("\n%s\t%d\t%s\t%f\t%s\t%f\n","indC= ",indC,"asapep: ",asapep,"asa[l]",asa[l]);
                        scanf("%d",&topolino);    
                        getchar(); */
      }
      if (strcmp(dd1->elm, "O") == 0)
      {
         asapep += asa[l];
         indO = l;
         /*            printf("\n%s\n","Ora passo in asapep O");
                       printf("\n%s\t%d\t%s\t%f\t%s\t%f\n","indO= ",indO,"asapep: ",asapep,"asa[l]",asa[l]);
                       scanf("%d",&topolino);    
                       getchar(); */
      }

      if (strcmp(dd1->elm, "N") == 0)
      {
         /*        printf("\n%s\n","Ora passo in N     ");
                   scanf("%d",&topolino);    
                   getchar(); */
         dd2 = dt;
         if (strcmp(dd1->amin, "PRO") != 0)   /* IF NOT A PROLINE, COUNTING "N" FOR THE ASA */
         {
            asapep += asa[l];
            /*             printf("\n%s\n","Ora passo in N no PRO");
                           printf("\n%d\t%s\t%s\t%s\t%s\n",l,"dd1->amin, azoto, carbonio0: ",dd1->amin,azoto,carbonio0);
                           printf("\n%s\t%d\t%s\t%f\t%s\t%f\n","indO= ",indO,"asapep: ",asapep,"asa[l]",asa[l]);
                           scanf("%d",&topolino);    
                           getchar(); */
            if (asapep <= asalevel)
            {
               /*             printf("\n%s\n","Ora passo in asa<asamin"); */
               dd2 += indCA;
               dd2->col = 10;
               /*     printf("\n%d\t%s\t%s\t%s\t%d\t%s\t%s\n",indCA,"dd2->elm: ",dd2->elm,"dd2->col: ",dd2->col,"dd2->descr: ",dd2->descr); */
               /*                      dd2++; */
               dd2 = dt;
               dd2 += indC;
               dd2->col = 10;
               /*     printf("\n%d\t%s\t%s\t%s\t%d\t%s\t%s\n",indC,"dd2->elm: ",dd2->elm,"dd2->col: ",dd2->col,"dd2->descr: ",dd2->descr);  */
               /*              dd2++; */
               dd2 = dt;
               dd2 += indO;
               dd2->col = 10;
               /*     printf("\n%d\t%s\t%s\t%s\t%d\t%s\t%s\n",indO,"dd2->elm: ",dd2->elm,"dd2->col: ",dd2->col,"dd2->descr: ",dd2->descr); */
               dd1->col = 10;
               /*     printf("\n%d\t%s\t%s\t%s\t%d\t%s\t%s\n",indO,"dd1->elm: ",dd1->elm,"dd1->col: ",dd1->col,"dd1->descr: ",dd1->descr);  
                      scanf("%s",pluto);
                      getchar(); */
            }
         }
         else
         {
            /*             printf("\n%s\n","Ora passo N PRO");
                           scanf("%d",&topolino);    
                           getchar(); */
            if (asapep <= asalevel)
            {
               dd2 += indCA;
               dd2->col = 10;
               /*              dd2++; */
               dd2 = dt;
               dd2 += indC;
               dd2->col = 10;
               /*              dd2++; */
               dd2 = dt;
               dd2 += indO;
               dd2->col = 10;
            }
         }
         asapep = (float) 0.0;
      }
   }


   qApp->processEvents();
   if (us_hydrodyn->stopFlag)
   {
      asab1_free_alloced();
      return -1;
   }

   /*****************************************************/

   if (check_asa != 1)
   {
      mol = us_fopen(outfile, "w");
      mol1 = us_fopen(outfile1, "w");
      mol2 = us_fopen(outfile2, "w");

      fprintf(mol, "%d\t%f\t%s\n", nat, 0.0, outfile1);
   }

   perc = (float) 0.0;
   kkk = 1;
   azoto[0] = 'N';
   asamin = asa[0];
   contatom = 1;
   contatom1 = 0;

   for (l = 1; l < nat; l++)
   {
      dd1 = dt;
      dd1 += l;

      /*   if(strcmp(dd1->elm,azoto)!=0)
           {
           contatom++;
           asamin+=asa[l];
           }
           else
           {
           for(i=0;i<contatom;i++)
           {
           dd2=dt;
           dd2+=(i+contatom1);
           if(check_asa!=1)
           {
           fprintf(mol,"%f\t",dd2->x);
           fprintf(mol,"%f\t",dd2->y);
           fprintf(mol,"%f\n",dd2->z);
           fprintf(mol1,"%f\t",dd2->r-rprobe);
           fprintf(mol1,"%d\t",dd2->m);
         
           if(dd2->col==10)
           fprintf(mol1,"%d\n",10);
           else
           {
           if(asamin<10)
           fprintf(mol1,"%d\n",6);
           else
           fprintf(mol1,"%d\n",1);
           }

           fprintf(mol2,"%s\t",dd2->elm);
           fprintf(mol2,"%s\t",dd2->amin);
           fprintf(mol2,"%s\n",dd2->descr);
           }
           }
           contatom1+=contatom;
           contatom=1;
           asamin=asa[l];
           }  */

      /*   NEW VERSION INCLUDING THE CARBOHYDRATES AND OG  */
      /*   if((strcmp(dd1->elm,azoto)==0) || (strcmp(dd1->elm,carbonio0)==0)) */
      if ((strcmp(dd1->elm, azoto) == 0) || (strcmp(dd1->elm, carbonio0) == 0)
          || ((strcmp(dd1->elm, carbonio1) == 0) && (strcmp(dd1->amin, "OG2") == 0)) || ((strcmp(dd1->elm, carbonio2) == 0)
                                                                                         && (strcmp(dd1->amin, "OG3") == 0)))
      {
         for (i = 0; i < contatom; i++)
         {
            dd2 = dt;
            dd2 += (i + contatom1);
            if (check_asa != 1)
            {
               fprintf(mol, "%f\t", dd2->x);
               fprintf(mol, "%f\t", dd2->y);
               fprintf(mol, "%f\n", dd2->z);
               fprintf(mol1, "%f\t", dd2->r - rprobe);
               fprintf(mol1, "%d\t", dd2->m);

               if (dd2->col == 10)
               {
                  fprintf(mol1, "%d\n", 10);
                  /*   printf("\n%d\t%s\t%s\t%s\t%d\t%s\t%s\n",i,"Minni dd2->elm: ",dd2->elm,"dd2->col: ",dd2->col,"dd2->descr: ",dd2->descr);
                       scanf("%s",pluto);
                       getchar();   */
               }
               else
               {
                  if (asamin < asalevel)
                  {
                     if ((strcmp(dd2->amin, "MAN") == 0) || (strcmp(dd2->amin, "GAL") == 0) || (strcmp(dd2->amin, "SIA") == 0) || (strcmp(dd2->amin, "FUC") == 0) || (strcmp(dd2->amin, "NAG") == 0) || (strcmp(dd2->amin, "OG1") == 0) || (strcmp(dd2->amin, "OG2") == 0) || (strcmp(dd2->amin, "OG3") == 0))   /* COLOR CODING DIFFERENTLY THE CARBOHYDRATES AND OG */
                     {
                        fprintf(mol1, "%d\n", 6);   /* REVERTED TO ORIGINAL */
                     }
                     else
                     {
                        if (strcmp(dd2->elm, azoto) == 0)
                        {
                           if (dd2->col == 10)
                           {
                              fprintf(mol1, "%d\n", 10);
                              /*   printf("\n%d\t%s\t%s\t%s\t%d\t%s\t%s\n",i,"Pluto dd2->elm: ",dd2->elm,"dd2->col: ",dd2->col,"dd2->descr: ",dd2->descr);
                                   scanf("%s",pluto);
                                   getchar(); */
                           }
                           else
                              fprintf(mol1, "%d\n", 1);
                        }
                        /* FIXED 27/2/2003 TO AVOID PRINTING '6' FOR THE CA, C, O ATOMS OF THE PEPTIDE BOND */
                        else if (strcmp(dd2->elm, "CA") == 0 || strcmp(dd2->elm, "C") == 0
                                 || strcmp(dd2->elm, "O") == 0)
                           fprintf(mol1, "%d\n", 1);
                        else
                        {
                           fprintf(mol1, "%d\n", 6);
                           /*   printf("\n%d\t%s\t%s\t%s\t%d\t%s\t%s\n",i,"Pippo dd2->elm: ",dd2->elm,"dd2->col: ",dd2->col,"dd2->descr: ",dd2->descr);
                                scanf("%s",pluto);
                                getchar(); */
                        }
                     }
                  }
                  else
                  {
                     if ((strcmp(dd2->amin, "MAN") == 0) || (strcmp(dd2->amin, "GAL") == 0) || (strcmp(dd2->amin, "SIA") == 0) || (strcmp(dd2->amin, "FUC") == 0) || (strcmp(dd2->amin, "NAG") == 0) || (strcmp(dd2->amin, "OG1") == 0) || (strcmp(dd2->amin, "OG2") == 0) || (strcmp(dd2->amin, "OG3") == 0))   /* COLOR CODING DIFFERENTLY THE CARBOHYDRATES AND OG */
                     {
                        fprintf(mol1, "%d\n", 1);   /* REVERTED TO ORIGINAL  */
                     }
                     else
                        fprintf(mol1, "%d\n", 1);
                  }
               }

               fprintf(mol2, "%s\t", dd2->elm);
               fprintf(mol2, "%s\t", dd2->amin);
               fprintf(mol2, "%s\n", dd2->descr);
            }
         }
         contatom1 += contatom;
         contatom = 1;
         asamin = asa[l];
      }
      else
      {
         contatom++;
         asamin += asa[l];
      }
   }


   qApp->processEvents();
   if (us_hydrodyn->stopFlag)
   {
      asab1_free_alloced();
      return -1;
   }

   for (i = 0; i < contatom; i++)
   {
      dd2 = dt;
      dd2 += (i + contatom1);

      if (check_asa != 1)
      {
         fprintf(mol, "%f\t", dd2->x);
         fprintf(mol, "%f\t", dd2->y);
         fprintf(mol, "%f\n", dd2->z);
         fprintf(mol1, "%f\t", dd2->r - rprobe);
         fprintf(mol1, "%d\t", dd2->m);

         if (dd2->col == 10)
            fprintf(mol1, "%d\n", 10);
         else
         {
            if (asamin < asalevel)
               fprintf(mol1, "%d\n", 6);
            else
               fprintf(mol1, "%d\n", 1);
         }

         fprintf(mol2, "%s\t", dd2->elm);
         fprintf(mol2, "%s\t", dd2->amin);
         fprintf(mol2, "%s\n", dd2->descr);
      }
   }

   if (check_asa != 1)
   {
      fclose(mol);
      fclose(mol1);
      fclose(mol2);
   }
 a300:

   if (check_asa == 1)
   {
#if defined( DEBUG_ASA )
      printf("\n\nRE-CHECK\n");
#endif
      /*   pippa=us_fopen(outfile1,"w");   */
      dd2 = dt;
      countb = 0;

      for (l = 0; l < nat; l++)
      {
#if defined( ASA_DEBUG )
         printf("%d %.2f\n", l, asa[l]);
#endif
         float sa;
         float sapp;
         if ( recheck ) {
            sa = 4.0f * M_PI * active_atoms[l]->bead_computed_radius * active_atoms[l]->bead_computed_radius;
            sapp = 4.0f * M_PI * (rprobe + active_atoms[l]->bead_computed_radius) * (rprobe + active_atoms[l]->bead_computed_radius);
         } else {
            sa = 4.0f * M_PI * active_atoms[l]->radius * active_atoms[l]->radius;
            sapp = 4.0f * M_PI * (rprobe + active_atoms[l]->radius) * (rprobe + active_atoms[l]->radius);
         }

         if ( us_isnan(asa[l]) )
         {
            printf("ASA WARNING NAN begin replaced by zero: atom %u asa %f > sa+p %f (sa %f)\n",
                   l,
                   asa[l],
                   sapp,
                   sa);
            asa[l] = 0;
         }

         if ( asa[l] > sapp )
         {
            printf("ASA WARNING: atom %u asa %f > sa+p %f (sa %f)\n",
                   l,
                   asa[l],
                   sapp,
                   sa);
         } else {
#if defined( ASA_DEBUG )
            printf("ASA: atom %u asa %f sapp %f sa %f\n",
                   l,
                   asa[l],
                   sapp,
                   sa);
#endif
         }            

         if (!recheck) 
         {
            active_atoms[l]->asa = asa[l];
         } else {
            active_atoms[l]->bead_recheck_asa = asa[l];
         }
         if (dt[l].col == 6)
         {
            /*bead's surface */
            zz = ((float) 4.0) * PI * dt[l].r * dt[l].r;
            temp = asa[l] / zz;
            temp *= (float) 100.0;
            if (temp >= min_asa)
            {
               countb = countb + 1;
               dt[l].col = 8;
#if defined( ASA_DEBUG )
               printf("\n #%d [bead %4d] - Surf_%6.2f - ASA_%6.2f o/oASA_%6.2f - Threshold_%6.2f", countb, l + 1, zz,
                      asa[l], asa[l] * 100 / zz, min_asa);
#endif
            }
         }

         /*      fprintf(pippa,"%f\t%d\t%d\n",dt[l].r-rprobe,dt[l].m,dt[l].col); */
      }
      /*   fclose(pippa);   */

      /*   i=0;
           for(l=0;l<nat;l++)
           {
           if(dt[l].col!=8)
           i++;
           }
      */
      mol = us_fopen(ridotto, "w");
      if ((raggio >= ((float) -2.1)) && (raggio <= ((float) -1.9)))
         fprintf(mol, "%d\t%f\t%s\t%f\n", nat, raggio, ridotto_rmc, psv);
      else
         fprintf(mol, "%d\t%f\t%s\n", nat, raggio, ridotto_rmc);

      mol1 = us_fopen(ridotto_rmc, "w");
      for (l = 0; l < nat; l++)
      {
         /*      if(dt[l].col!=8)
                 {   */
         fprintf(mol, "%f\t%f\t%f\n", dt[l].x, dt[l].y, dt[l].z);
         if ((raggio >= ((float) -4.1)) && (raggio <= ((float) -3.9)))
            fprintf(mol1, "%f\t%d\t%d\t%s\n", dt[l].r - rprobe, dt[l].m, dt[l].col, dt[l].descr);
         else
            fprintf(mol1, "%f\t%d\t%d\n", dt[l].r - rprobe, dt[l].m, dt[l].col);
         /*         }   */
      }
      fclose(mol1);
      fclose(mol);
   }
#if defined( ASA_DEBUG )
   printf("\n\n\n");
#endif
   asab1_free_alloced();
   QFile::remove("controll");
   QFile::remove("plotter");
   QFile::remove("plotter1");

   qApp->processEvents();
   if (us_hydrodyn->stopFlag)
   {
      return -1;
   }
   return 0;
}

// ----------------------------------- bsort.c -------------------------------------
// #warning bsort.c 

#if defined(NOT_USED)
static void
ord_d()
{
   int i, j;
   struct dati1 x;
   struct dati1 *y1, *y2;

   for (i = 1; i < nat; i++)
   {
      for (j = nat - 1; j >= i; j--)
      {
         y1 = dt;
         y1 += j;
         y2 = y1 - 1;

         if (y2->z < y1->z)
         {
            x = *y2;
            *y2 = *y1;
            *y1 = x;
         }
      }
   }
}
#endif

static void
ord_a()
{

   int i, j;
   struct dati2 x;
   struct dati2 *y1, *y2;

   for (i = 1; i < indec[nc]; i++)
   {
      for (j = indec[nc] - 1; j >= i; j--)
      {
         y1 = trans;
         y1 += (nc * IMAX) + j;
         y2 = y1 - 1;

         if (y2->ai > y1->ai)
         {
            x = *y2;
            *y2 = *y1;
            *y1 = x;
         }
      }
   }
}

// ----------------------------------- calc.c -------------------------------------
// #warning calc.c 

// float ang(float, float);
// float ang1(float, float, int);
// float dist(float, float, float, float);

static float
ang(float xi, float yi)
{
   float ia, x, y;
   struct dati1 *xy;

   xy = dts;
   xy += nc;

   x = xy->x;
   y = xy->y;

   if (xi == x)
   {
      if (yi > y)
         ia = PI / ((float) 2.0);
      else
         ia = ((float) 3.0) * PI / ((float) 2.0);
   }

   else
   {
      ia = (float) atan((yi - y) / (xi - x));

      if (xi >= x)
      {
         if (ia < 0.0)
            ia = (float) fmod((2.0 * PI + ia), (2.0 * PI));
      }
      else
         ia = ia + PI;
   }

   return (ia);
}

/**************************************************/

static float
ang1(float xi, float yi, int k)
{
   float ia, x, y;
   struct dati1 *xy;

   xy = dts;
   xy += k;

   x = xy->x;
   y = xy->y;

   if (xi == x)
   {
      if (yi > y)
         ia = PI / ((float) 2.0);
      else
         ia = ((float) 3.0) * PI / ((float) 2.0);
   }
   else
   {
      ia = (float) atan((yi - y) / (xi - x));
      if (xi >= x)
      {
         if (ia < 0.0)
            ia = (float) fmod((2.0 * PI + ia), (2.0 * PI));
      }
      else
         ia = ia + PI;
   }

   return (ia);
}

/**********************************************/

static float
dist(float x1, float y1, float x2, float y2)
{
   float d;

   d = (float) sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
   return (d);
}

// ----------------------------------- cdis.c -------------------------------------
// #warning cdis.c 

// extern float dist(float, float, float, float);

static void
calcdis()
{
   int i, j, flag;
   float ap, xp, yp;
   float x, y, r;

   struct dati1 *xy;
   struct dati2 *z1, *z2;

   xy = dts;
   xy += nc;

   x = xy->x;
   y = xy->y;
   r = xy->r;

   z1 = trans;
   z1 += (nc * IMAX);
   z2 = z1;
   z2++;

   ap = (z1->ai + z2->ai) / ((float) 2.0);
   xp = ((float) cos(ap)) * r + x;
   yp = ((float) sin(ap)) * r + y;

   i = 0;
   flag = 1;

   while ((i < cont1) && (flag == 1))
   {
      if (i == nc)
         i++;
      else
      {
         xy = dts;
         xy += i;

         x = xy->x;
         y = xy->y;
         r = xy->r;

         if (dist(xp, yp, x, y) <= r)
            flag = 0;
      }
      i++;
   }

   if (flag == 1)
   {
      i = 0;
      j = 0;

      while (i < indec[nc])
      {
         z1 = trans;
         z1 += (nc * IMAX) + i;
         z2 = z1;
         z2++;

         dis[j].xi = z1->xi;
         dis[j].yi = z1->yi;
         dis[j].ai = (z2->ai - z1->ai) * 180 / PI;

         i = i + 2;
         j++;
      }
   }
   else
   {
      i = 1;
      j = 0;

      while (i < indec[nc] - 1)
      {
         z1 = trans;
         z1 += (nc * IMAX) + i;
         z2 = z1;
         z2++;

         dis[j].xi = z1->xi;
         dis[j].yi = z1->yi;
         dis[j].ai = (z2->ai - z1->ai) * ((float) 180.0) / PI;

         i = i + 2;
         j++;
      }

      z1 = trans;
      z1 += (nc * IMAX);
      z2 = z1;
      z1 += (indec[nc] - 1);

      dis[j].xi = z1->xi;
      dis[j].yi = z1->yi;
      dis[j].ai = (((float) 2.0) * PI - z1->ai + z2->ai) * ((float) 180.0) / PI;
   }
}

// ----------------------------------- cor.c -------------------------------------
// #warning cor.c 

// void cordis(int);

#if defined(NOT_USED)
static void
cordis(int s)
{
   double a, b, c, d;
   double k1, k2, k3, k4, k5;
   double x1, x2, y1, y2;
   double fraxon;
   int k;

   struct dati1 *xy;
   double xx, yy, rr;

   xy = dts;
   xy += nc;
   xx = xy->x;
   yy = xy->y;
   rr = xy->r;

   d = fl * (1 + (ceil(nat / 15)) / 2.0);

   for (k = 0; k < s / 2; k++)
   {
      a = (yy - dis[k].yi);
      b = (rr * rr - d * d - xx * xx - yy * yy + dis[k].xi * dis[k].xi + dis[k].yi * dis[k].yi) / 2.0;
      c = (dis[k].xi - xx);

      if (dis[k].xi != xx)
      {
         k1 = (a * a + c * c);
         k2 = (a * b - a * c * dis[k].xi - c * c * dis[k].yi);
         k3 = (b - c * dis[k].xi) * (b - c * dis[k].xi) + c * c * (dis[k].yi * dis[k].yi - d * d);

         y1 = (-k2 + sqrt(k2 * k2 - k1 * k3)) / k1;
         y2 = (-k2 - sqrt(k2 * k2 - k1 * k3)) / k1;
         x1 = (a * y1 + b) / c;
         x2 = (a * y2 + b) / c;
      }
      else
      {
         k4 = (-a * a * dis[k].xi);
         k5 = (b + a * dis[k].yi) * (b + a * dis[k].yi) + a * a * (dis[k].xi * dis[k].xi - d * d);

         y1 = (-b / a);
         y2 = (-b / a);
         x1 = (-k4 + sqrt(k4 * k4 - a * a * k5)) / (a * a);
         x2 = (-k4 - sqrt(k4 * k4 - a * a * k5)) / (a * a);
      }

      if (dis[k].xi > xx)
      {
         if (y1 > dis[k].yi)
         {
            dis[k].yi = (float) y1;
            dis[k].xi = (float) x1;
         }
         else
         {
            dis[k].yi = (float) y2;
            dis[k].xi = (float) x2;
         }
      }

      else
      {
         if (y1 < dis[k].yi)
         {
            dis[k].yi = (float) y1;
            dis[k].xi = (float) x1;
         }
         else
         {
            dis[k].yi = (float) y2;
            dis[k].xi = (float) x2;
         }
      }

      fraxon = asin((d / (2 * rr))) * 180.0 / PI;
      dis[k].ai = dis[k].ai - ((float) (4.0 * fraxon));
   }
}
#endif

// ----------------------------------- gir.c -------------------------------------
// #warning gir.c 

// void ragir();

// compute radius of gyration
static void
ragir()
{
   // qDebug() << "US_Hydrodyn_ASAB1::ragir() nat " << nat;
   int i;
   float ro2, ro3, mt, rg, xm, ym, zm;

   struct dati1 *dd;
   float xx, yy, zz, rr;
   int mm;

   mt = (float) 0.0;
   xm = (float) 0.0;
   ym = (float) 0.0;
   zm = (float) 0.0;
   ro2 = (float) 0.0;
   ro3 = (float) 0.0;

   dd = dt;
   for (i = 0; i < nat; i++)
   {
      xx = dd->x;
      yy = dd->y;
      zz = dd->z;
      mm = dd->m;

      xm += xx * mm;
      ym += yy * mm;
      zm += zz * mm;
      mt += mm;

      dd++;
   }

   xm = xm / mt;
   ym = ym / mt;
   zm = zm / mt;

   dd = dt;
   for (i = 0; i < nat; i++)
   {
      xx = dd->x;
      yy = dd->y;
      zz = dd->z;
      rr = dd->r;
      mm = dd->m;

      rg = (float) 0.6 * (rr - rprobe) * (rr - rprobe);
      ro2 += mm * ((float) (pow((xx - xm), 2) + pow((yy - ym), 2) + pow((zz - zm), 2) + rg));
      ro3 += mm * ((float) (pow((xx - xm), 2) + pow((yy - ym), 2) + pow((zz - zm), 2   )));

      dd++;
   }

   ro2 = ro2 / mt;
   ro3 = ro3 / mt;

   ro = (float) sqrt(ro2);
   ro1 = (float) sqrt(ro3);
}

// ----------------------------------- init1.c -------------------------------------
// #warning init1.c 

static FILE *init1_mol;
static FILE *init1_mol1;
static FILE *init1_rmc;

// void init();
// void pulisci();

static void
init()
{
   int i;

   pulisci();

   init1_mol = us_fopen("test", "r");
   init1_mol1 = us_fopen("provaly2", "r");

   if ( 1 == fscanf(init1_mol, "%d", &nat) ) {};
   if ( 1 == fscanf(init1_mol, "%f", &raggio) ) {};
   if ( 1 == fscanf(init1_mol, "%s", ragcol) ) {};

   init1_rmc = us_fopen(ragcol, "r");   /* opening the file containing the radii, masses and colors */

   for (i = 0; i < nat; i++)
   {
      if ( 1 == fscanf(init1_mol, "%f", &(dt[i].x)) ) {};
      if ( 1 == fscanf(init1_mol, "%f", &(dt[i].y)) ) {};
      if ( 1 == fscanf(init1_mol, "%f", &(dt[i].z)) ) {};
      if ( 1 == fscanf(init1_rmc, "%f", &(dt[i].r)) ) {};
      if ( 1 == fscanf(init1_rmc, "%d", &(dt[i].m)) ) {};
      if ( 1 == fscanf(init1_rmc, "%d", &(dt[i].col)) ) {};
      //fscanf(init1_mol1, "%s", &(dt[i].elm));
      //fscanf(init1_mol1, "%s", &(dt[i].amin));
      if ( 1 == fscanf(init1_mol1, "%4c", &(dt[i].elm[0])) ) {};
      if ( 1 == fscanf(init1_mol1, "%4c", &(dt[i].amin[0])) ) {};
   }

   fclose(init1_mol);
   fclose(init1_rmc);
   fclose(init1_mol1);
}

static void
pulisci()
{
   /* printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"); */
}

// ----------------------------------- init2.c -------------------------------------
// #warning init2.c 

/* MODIFIED DEC 2002 IN GLASGOW FOR CHECK ON THE OUTPUT FILES WHEN RE-CHECKING ASA */
/* MODIFIED APRIL 2003 IN GENOVA FOR INCLUSION OF PSV */

// static FILE *init2_mol;
static FILE *init2_mol1;
// static FILE *init2_rmc;

// void init2();

// extern void pulisci();
static void
init2()
{
   struct dati1 *dd;

   // int i;
   // int fe, fe1;
   // char rido[10];
   // char command[200];
   // char nome[30];
   // extern int flag1;

   /*   printf("\n\n\t%s%d","Valore flag: ",flag1);
        scanf("%s",rido);
        getchar(); */

   pulisci();

#if defined(NOT_USED)
   while (init2_mol == NULL)
   {
      /*   pulisci();  */
      printf("\n\n\t%s", "Insert the bead model filename: ");
      scanf("%s", nome);
      init2_mol = us_fopen(nome, "r");

   }
   /*init2_mol1=NULL;
     while(init2_mol1==NULL) 
     { */
   /*   pulisci(); */
 a50:
   printf("\n\n\t%s", "Insert the re-checked bead model filename: ");
   scanf("%s", ridotto);
   init2_mol1 = us_fopen(ridotto, "r");
   if (init2_mol1 != NULL)
   {
      printf("\n");
      printf("\t*** CAUTION : File already exists ! ***\n\n");
      printf("\t  Select one option:\n\n");
      printf("\t 1) Overwrite existing file\n");
      printf("\t 2) Create new file\n\n");
      printf("\t** Select (1/2) :___ ");
      scanf("%d", &fe);
      getchar();
      fclose(init2_mol1);
      if (fe == 1)
      {
         QFile::remove(ridotto);
      }
      if (fe == 2)
         goto a50;
   }
#endif
   strcpy(ridotto, "asab1_output");
   init2_mol1 = us_fopen(ridotto, "w");
   /*   }   */
   fclose(init2_mol1);

   /* init2_mol1=NULL;
      while(init2_mol1==NULL)
      { */
   /*   pulisci(); */

   strcpy(ridotto_rmc, ridotto);
   strcat(ridotto_rmc, ".rmc");
#if defined( DEBUG_ASA )
   printf("\n");
   printf("\t Creating file %s for radii, masses and colors\n", ridotto_rmc);
#endif

   qApp->processEvents();
   if (us_hydrodyn->stopFlag)
   {
      return;
   }

#if defined(NOT_USED)
 a55:
   init2_mol1 = us_fopen(ridotto_rmc, "r");
   if (init2_mol1 != NULL)
   {
      printf("\n");
      printf("\t*** CAUTION : File %s already exists ! ***\n\n", ridotto_rmc);
      printf("\t  Select one option:\n\n");
      printf("\t 1) Overwrite existing .rmc file\n");
      printf("\t 2) Create new .rmc file\n\n");
      printf("\t** Select (1/2) :___ ");
      scanf("%d", &fe1);
      getchar();
      fclose(init2_mol1);
      if (fe1 == 1)
      {
         QFile::remove(ridotto_rmc);
      }
      if (fe1 == 2)
      {
         printf("\n\n\t%s", "Insert the filename for the rmc file of the re-checked bead model: ");
         scanf("%s", ridotto_rmc);
         goto a55;
      }
   }
#endif

   /*   printf("\n\n\t%s","Insert the filename for the rmc file of the re-checked bead model: ");
        scanf("%s",ridotto_rmc); */
   //    init2_mol1 = us_fopen(ridotto_rmc, "w");
   /*   } */
   //    fclose(init2_mol1);

   /*   printf("\n\n\t%s%d","Valore flag1: ",flag1);
        scanf("%s",rido);
        getchar(); */

   //    if (flag1 != 1)
   //   init2_mol1 = us_fopen("provaly2", "r");

   nat = active_atoms.size();
   // fscanf(init2_mol, "%d", &nat);
   // fscanf(init2_mol, "%f", &raggio);
   raggio = -2;
   psv = results->vbar;
   // fscanf(init2_mol, "%s", ragcol);
   // if ((raggio >= ((float) -2.1)) && (raggio <= ((float) -1.9)))
   //   fscanf(init2_mol, "%f", &psv);

   // init2_rmc = us_fopen(ragcol, "r");   /* opening the file containing the radii, masses and colors */

   dd = dt;

   maxz = dd->z + dd->r;
   minz = dd->z - dd->r;

#if defined(NOT_USED)
   for (i = 0; i < nat; i++)
   {
      fscanf(init2_mol, "%f", &(dt[i].x));
      fscanf(init2_mol, "%f", &(dt[i].y));
      fscanf(init2_mol, "%f", &(dt[i].z));
      fscanf(init2_rmc, "%f", &(dt[i].r));
      (dt[i].r) += rprobe;
      fscanf(init2_rmc, "%d", &(dt[i].m));
      fscanf(init2_rmc, "%d", &(dt[i].col));
      if (flag1 != 1)
      {
         fscanf(init2_mol1, "%s", &(dt[i].elm));
         fscanf(init2_mol1, "%s", &(dt[i].amin));
         fscanf(init2_mol1, "%s", &(dt[i].descr));
      }
      if ((raggio >= ((float) -4.1)) && (raggio <= ((float) -3.9)))
         fscanf(init2_rmc, "%s", &(dt[i].descr));

      if (maxz < (dd->z + dd->r))
         maxz = dd->z + dd->r;
      if (minz > (dd->z - dd->r))
         minz = dd->z - dd->r;

      dd++;
   }
#endif

   if (!recheck) 
   {
      for (unsigned int i = 0; i < active_atoms.size(); i++) 
      {
         dt[i].r = active_atoms[i]->radius;
         dt[i].r += rprobe;
         dt[i].x = active_atoms[i]->coordinate.axis[0];
         dt[i].y = active_atoms[i]->coordinate.axis[1];
         dt[i].z = active_atoms[i]->coordinate.axis[2];
         dt[i].m = (int)active_atoms[i]->mw;
         dt[i].col = active_atoms[i]->bead_color;
      }
   }  else {
      for (unsigned int i = 0; i < active_atoms.size(); i++) 
      {
         dt[i].r = active_atoms[i]->bead_computed_radius;
         dt[i].r += rprobe;
         dt[i].x = active_atoms[i]->bead_coordinate.axis[0];
         dt[i].y = active_atoms[i]->bead_coordinate.axis[1];
         dt[i].z = active_atoms[i]->bead_coordinate.axis[2];
         dt[i].m = (int)active_atoms[i]->bead_mw;
         dt[i].col = active_atoms[i]->bead_color;
      }
   }

#if defined(DEBUG)
   for(int i = 0; i < nat; i++) {
      printf("%d %s %s %.4f %.4f %.4f %.4f %d %d\n", 
             i,
             active_atoms[i]->name.toLatin1().data(),
             active_atoms[i]->resName.toLatin1().data(),
             dt[i].r, 
             dt[i].x, 
             dt[i].y, 
             dt[i].z, 
             dt[i].m, 
             dt[i].col);
   }
#endif
   //  fclose(init2_mol);
   // fclose(init2_rmc);
   // if (flag1 != 1)
   //   fclose(init2_mol1);
}

// ----------------------------------- init3.c -------------------------------------
// #warning init3.c 

/* MODIFIED IN GLASGOW DEC 2002 TO INSERT THE VARIABLE FILENAMES  */
/* MODIFIED APRIL 2003 IN GENOVA FOR OCTYL GLUCOSIDE */
/* FIXED JUNE 2005 IN GENOVA FOR NO SPACE BETWEEN 'ATOM' OR 'HETATM' FIELDS AND PROGRESSIVE NUMBER */

static FILE *init3_brook;
static FILE *init3_mol;
static FILE *init3_mol1;
static FILE *init3_pippa;
static FILE *new_mol;
static FILE *new_mol1;
static FILE *new_mol2;
static FILE *new_mol3;

// void initarray();
static void readline(int);
// void readlinec(int);
static void assignrad(int);
static void assigntab(int);
// static char pluto;
static int flagr1, fe2, fe3;
// static float flrad;

static char arr_temp[200];
// static char command[200];
// extern char outfile[30];
// extern char outfile1[30];
// extern char outfile2[30];
// extern char outris[30];

// extern void pulisci();

static void
initarray()
{
   int i;
   char nome[30];
   char filename[100];
   char *tmp;
   // em("i3_1");
   tmp = (char *) malloc(100 * sizeof(char *));
   tmp = (char *) getenv("SOMO_PATH");
   // em("i3_2");
   strcpy(filename, tmp);
   // em("i3_3");
   strcat(filename, "/bin/tabella1.cor");
   fprintf(stderr, "\nInput tabella: %s", filename);
   // em("i3_4");

   init3_pippa = us_fopen(filename, "r");
   // em("i3_5");

   /*init3_pippa=us_fopen("tabella1.cor","r");*/

   /* IMPORTING FROM A TABLE (tabella.cor) THE VALUES FOR THE ACCESSIBLE SURFACE AREA OF RESIDUES */
   /* THE VALUES IN ROWS 1-25 HAVE BEEN CALCULATED FOR A PROBE RADIUS OF 0 FROM THOSE IN ROWS 29-48 (WRONG VALUES!) */
   /* THE VALUES IN ROWS 29-48 ARE ASA FROM TRIPEPTIDES GLY-XXX-GLY TAKEN FROM TABLE 4.4 OF CREIGHTON */
   /* IN ROWS 21-26 AND 49-54 ARE THE VALUES FOR GAL, NAG, FUC, MAN, SIA AND GLUC (OG1) DEDUCED FROM KALIANNAN 2001 */
   /* IN ROWS 27-28 AND 55-56 ARE THE SUPPOSED VALUES FOR OG2 AND OG3 (TAIL OF OCTYL GLUCOSIDE) */
   // em("i3_6");
   for (i = 0; i < 56; i++)
   {
      if ( 1 == fscanf(init3_pippa, "%d", &nat) ) {};
      arr1[i] = (float) nat;
   }
   fclose(init3_pippa);
   // em("i3_7");

   nat = 0;
   flagr1 = 0;

   init3_brook = us_fopen(nome, "r");

   init3_mol1 = us_fopen("contrall", "w");
   //fprintf(init3_mol1, "");
   fclose(init3_mol1);
   // em("i3_8");

   init3_mol1 = us_fopen("contrall", "ab");

   // em("i3_9");
   while (init3_brook == NULL)
   {
      pulisci();
      printf("\n\n\tInsert the PDB filename : ");
      if ( 1 == scanf("%s", nome) ) {};
      init3_brook = us_fopen(nome, "r");
   }

   printf("\n\n\t%s", "Output files: Enter '0' for default (provaly, provaly1, provaly2, asaris)");
   printf("\n\t%s", "                    '1' for new filenames\n\n\t\t\t-->");
   if ( 1 == scanf("%d", &fe2) ) {};
   getchar();
   if (fe2 == 0)
   {

      strcat(outfile, "provaly");
      strcat(outfile1, "provaly1");
      strcat(outfile2, "provaly2");
      strcat(outris, "asaris");
      /*         printf("\n outfile= %s",outfile); 
                 printf("\n outfile1= %s",outfile1); 
                 printf("\n outfile2= %s",outfile2); 
                 printf("\n outris= %s",outris); */
      init3_mol = us_fopen(outfile, "w");
      //fprintf(init3_mol, "");
      fclose(init3_mol);

      init3_mol = us_fopen(outfile1, "w");
      //fprintf(init3_mol, "");
      fclose(init3_mol);

      init3_mol = us_fopen(outfile2, "w");
      //fprintf(init3_mol, "");
      fclose(init3_mol);

      init3_mol = us_fopen(outris, "w");
      //fprintf(init3_mol, "");
      fclose(init3_mol);

   }

   if (fe2 == 1)
   {
   a10:
      printf("\n\t** Insert the root output filename :___ ");
      if ( 1 == scanf("%s", outfile) ) {};
      getchar();

      strcpy(outris, outfile);
      strcat(outris, ".asaris");
      /*         printf("\n outris= %s",outris); */
      strcat(outfile, ".asa_res");
      /*         printf("\n outfile= %s",outfile); */
      strcpy(outfile1, outfile);
      strcat(outfile1, "1");
      /*         printf("\n outfile1= %s",outfile1); */
      strcpy(outfile2, outfile);
      strcat(outfile2, "2");
      /*         printf("\n outfile2= %s",outfile2); */

      new_mol = us_fopen(outfile, "r");

      if (new_mol != NULL)
      {
         printf("\n");
         printf("\t*** CAUTION : Root file %s already exists ! ***\n\n", outfile);
         new_mol1 = us_fopen(outfile1, "r");

         if (new_mol1 != NULL)
         {
            printf("\t*** File %s also already exists ! ***\n", outfile1);
         }
         new_mol2 = us_fopen(outfile2, "r");

         if (new_mol2 != NULL)
         {
            printf("\t*** File %s also already exists ! ***\n", outfile2);
         }
         new_mol3 = us_fopen(outris, "r");

         if (new_mol3 != NULL)
         {
            printf("\t*** File %s also already exists ! ***\n", outris);
         }
         printf("\n\t   Do you want:\n\n");
         printf("\t 1) Overwrite existing file(s)\n");
         printf("\t 2) Create new files\n\n");
         printf("\t** Select (1/2) :___ ");
         if ( 1 == scanf("%d", &fe3) ) {};
         getchar();
         fclose(new_mol);
         fclose(new_mol1);
         fclose(new_mol2);
         fclose(new_mol3);
         if (fe3 == 1)
         {
            QFile::remove(outfile);
            QFile::remove(outfile1);
            QFile::remove(outfile2);
            QFile::remove(outris);
         }
         if (fe3 == 2)
         {
            goto a10;
         }
      }
   }

 a50:
   if ( 1 == fscanf(init3_brook, "%6s", arr_temp) ) {};
   if (strcmp(arr_temp, "END") == 0)
      goto a200;

   /* printf("\n%s\t%s\n","pluto arr_temp",arr_temp);
      if ( 1 == scanf("%s",pluto) ) {};
      getchar(); */

   if ((strncmp(arr_temp, "ATOM", 6) == 0) || (strncmp(arr_temp, "HETATM", 6) == 0))
      goto a150;

   else
      goto a50;

 a150:
   /* printf("\n%s\t%s\n","pluto arr_temp",arr_temp);
      if ( 1 == scanf("%s",pluto) ) {};
      getchar(); */
   if (strncmp(arr_temp, "ATOM", 6) == 0)
      flagr1 = 0;

   else if (strncmp(arr_temp, "HETATM", 6) == 0)
      flagr1 = 1;

   //  a100:

   readline(0);
   while (sim == 1)
   {
      readline(corr);
   }

   /*   printf("\n%s\n","qui arrivo");
        if ( 1 == scanf("%s",pluto) ) {};
        getchar(); */

   if (strcmp(arr_temp, "END") == 0)
      goto a200;

 a200:

   fclose(init3_brook);
   fclose(init3_mol1);

   init3_pippa = us_fopen("asaris", "ab");
   fprintf(init3_pippa, "\n\n\t%s\n\n\n", nome);
   fclose(init3_pippa);
}

/*************************************************************/

static void
readline(int ind)
{
   int j;
   char stringa[30];
   struct dati1 *dd;

   dd = dt;
   dd += ind;

   for (j = 0; j < 24; j++) {
      if ( 1 == fscanf(init3_brook, "%c", &(stringa[j])) ) {};
   }

   nat++;

   if ( 1 == sscanf(stringa + 7, "%s", dd->elm) ) {};
   /*      printf("\n%s\t%s\n","dd->elm  ",dd->elm);
           printf("\n%s\t%d\n","flagr1 atom",flagr1);  
           if ( 1 == scanf("%s",pluto) ) {};
           getchar(); */

   if (flagr1 != 1)
   {

      if ( 1 == sscanf(stringa + 12, "%s", dd->amin) ) {};
      /*   printf("\n%s\t%s\n","dd->amin  ",dd->amin); 
           if ( 1 == scanf("%s",pluto) ) {};
           getchar(); */

      dd->descr[9] = 0;
      for (j = 0; j < 9; j++)
      {
         if (stringa[13 + j] == ' ')
            stringa[13 + j] = '_';
         dd->descr[j] = stringa[13 + j];
      }
   }

   else
   {
      if ( 1 == sscanf(stringa + 11, "%s", dd->amin) ) {};
      /*   printf("\n%s\t%s\n","carb dd->amin  ",dd->amin);
           if ( 1 == scanf("%s",pluto) ) {};
           getchar(); */

      dd->descr[9] = 0;
      for (j = 0; j < 9; j++)
      {
         /*   printf("\n%s\t%s\n","stringa[11+j]  ",stringa); */
         if (stringa[11 + j] == ' ')
            stringa[11 + j] = '_';
         dd->descr[j] = stringa[11 + j];
         /*      printf("\n%s\t%s\n","carb dd->desc  ",dd->descr);
                 if ( 1 == scanf("%s",pluto) ) {};
                 getchar(); */
      }
   }

   if ( 1 == fscanf(init3_brook, "%f", &(dd->x)) ) {};
   if ( 1 == fscanf(init3_brook, "%f", &(dd->y)) ) {};
   if ( 1 == fscanf(init3_brook, "%f", &(dd->z)) ) {};

   /* printf("\n%s\t%f\n","dd->x  ",dd->x);
      printf("\n%s\t%f\n","dd->y  ",dd->y);
      printf("\n%s\t%f\n","dd->z  ",dd->z); */

   dd->col = 1;
   dd->m = 1;

   assigntab(ind);
   assignrad(ind);

   /*  printf("\n%s\t%s\n","dd->elm  ",dd->elm);
       printf("\n%s\t%f\n","dd->r  ",dd->r);
       printf("\n%s\t%d\n","dd->m  ",dd->m);
       if ( 1 == scanf("%s",pluto) ) {};
       getchar(); */

   if ( 1 == fscanf(init3_brook, "%[^\n]%*c", arr_temp) ) {};

   ind++;
   corr = ind;

   /* printf("\n%s\t%d\n","corr= ",corr);
      if ( 1 == scanf("%s",pluto) ) {};
      getchar(); */

   if ( 1 == fscanf(init3_brook, "%6s", arr_temp) ) {};

   /* printf("\n%s\t%s\n","zeppo arr_temp",arr_temp);
      if ( 1 == scanf("%s",pluto) ) {};
      getchar(); */
   /* if(strcmp(arr_temp,"ATOM")==0) */
   if ((strncmp(arr_temp, "ATOM", 6) == 0) || (strncmp(arr_temp, "HETATM", 6) == 0))
      sim = 1;
   else
      sim = 0;

   if (strncmp(arr_temp, "ATOM", 6) == 0)
      flagr1 = 0;

   else if (strncmp(arr_temp, "HETATM", 6) == 0)
      flagr1 = 1;
}

/***********************************************************/

/* void readlinec(int ind)
   {
   int   j;
   char   stringa[30];
   struct   dati1 *dd;

   dd=dt;
   dd+=ind;

   for(j=0;j<26;j++) {
      if ( 1 == fscanf(init3_brook,"%c",&(stringa[j])) ) {};
   }

   nat++;

   if ( 1 == sscanf(stringa+7,"%s",dd->elm) ) {};
   printf("\n%s\t%s\n","carb dd->elm  ",dd->elm);
   if ( 1 == scanf("%s",pluto) ) {};
   getchar(); 
   if ( 1 == sscanf(stringa+11,"%s",dd->amin);     ) {};
   printf("\n%s\t%s\n","carb dd->amin  ",dd->amin);
   if ( 1 == scanf("%s",pluto) ) {};
   getchar(); 

   dd->descr[9]=0;
   for(j=0;j<9;j++)   
   {
   printf("\n%s\t%s\n","stringa[11+j]  ",stringa);
   if(stringa[11+j]==' ')
   stringa[11+j]='_';
   dd->descr[j]=stringa[11+j];
   printf("\n%s\t%s\n","carb dd->desc  ",dd->descr);
   if ( 1 == scanf("%s",pluto) ) {};
   getchar(); 
   }

   if ( 1 == fscanf(init3_brook,"%f",&(dd->x)); ) {};
   if ( 1 == fscanf(init3_brook,"%f",&(dd->y)); ) {};
   if ( 1 == fscanf(init3_brook,"%f",&(dd->z)); ) {};
   printf("\n%s\t%f\n","carb dd->x  ",dd->x);
   printf("\n%s\t%f\n","carb dd->y  ",dd->y);
   printf("\n%s\t%f\n","carb dd->z  ",dd->z);

   dd->col=1;
   dd->m=1;

   assigntab(ind);
   assignrad(ind);

   printf("%s\t%f\n",dd->elm,flrad);

   if ( 1 == fscanf(init3_brook,"%[^\n]%*c",arr_temp) ) {};

   ind++;
   corr=ind;

   if ( 1 == fscanf(init3_brook,"%6s",arr_temp); ) {};

   if(strncmp(arr_temp,"HETATM", 6)==0)
   sim=1;  
   else
   sim=0;
   } */

/***********************************************************/

static void
assignrad(int xx)
{
   float flrad = 0.0;
   struct dati1 *dd;

   dd = dt;
   dd += xx;

   switch (dd->elm[0])
   {
   case 'C':
      if (isalpha(dd->elm[1]) != 0 || isdigit(dd->elm[1]) != 0)   /* selects c alpha etc. */
      {
         if (dd->elm[1] == 'A')   /* C alpha C4H1 */
         {
            flrad = (float) 1.87;
            if (strcmp(dd->amin, "GLY") != 0)
               dd->m = 13;
            else
               dd->m = 14;
         }

         else if (dd->elm[1] == 'B')   /* C beta C4H2  */
         {
            flrad = (float) 1.87;
            if (strcmp(dd->amin, "ALA") != 0)
               dd->m = 14;
            else
               dd->m = 15;
         }

         else if (dd->elm[1] == 'G')   /* C gamma  */
         {
            if ((strcmp(dd->amin, "VAL") == 0) || (strcmp(dd->amin, "THR") == 0))   /* C4H3 */
            {
               flrad = (float) 1.87;
               dd->m = 15;
            }
            else if (strcmp(dd->amin, "ILE") == 0)   /* C4H2 & C4H3 */
            {
               flrad = (float) 1.87;
               if (dd->elm[2] == '1')
                  dd->m = 14;
               else
                  dd->m = 15;
            }
            else if (strcmp(dd->amin, "LEU") == 0)   /* C4H1 */
            {
               flrad = (float) 1.87;
               dd->m = 13;
            }
            else if ((strcmp(dd->amin, "PRO") == 0) || (strcmp(dd->amin, "MET") == 0))   /* C4H2 */
            {
               flrad = (float) 1.87;
               dd->m = 14;
            }
            else if ((strcmp(dd->amin, "GLU") == 0) || (strcmp(dd->amin, "GLN") == 0))   /* C4H2 */
            {
               flrad = (float) 1.87;
               dd->m = 14;
            }
            else if ((strcmp(dd->amin, "ARG") == 0) || (strcmp(dd->amin, "LYS") == 0))   /* C4H2 */
            {
               flrad = (float) 1.87;
               dd->m = 14;
            }
            else if ((strcmp(dd->amin, "PHE") == 0) || (strcmp(dd->amin, "TRP") == 0))   /* C3H0 */
            {
               flrad = (float) 1.76;
               dd->m = 12;
            }
            else if ((strcmp(dd->amin, "ASP") == 0) || (strcmp(dd->amin, "ASN") == 0))   /* C3H0 */
            {
               flrad = (float) 1.76;
               dd->m = 12;
            }
            else if ((strcmp(dd->amin, "HIS") == 0) || (strcmp(dd->amin, "TYR") == 0))   /* C3H0 */
            {
               flrad = (float) 1.76;
               dd->m = 12;
            }
         }

         else if (dd->elm[1] == 'D')   /* C delta  */
         {
            if (strcmp(dd->amin, "PRO") == 0)   /* C4H2 */
            {
               flrad = (float) 1.87;
               dd->m = 14;
            }
            else if (strcmp(dd->amin, "PHE") == 0)   /* C3H1 */
            {
               flrad = (float) 1.76;
               dd->m = 13;
            }
            else if (strcmp(dd->amin, "TRP") == 0)   /* C3H1 & C3H0 */
            {
               flrad = (float) 1.76;
               if (dd->elm[2] == '1')
                  dd->m = 13;
               else
                  dd->m = 12;
            }
            else if ((strcmp(dd->amin, "ILE") == 0) || (strcmp(dd->amin, "LEU") == 0))   /* C4H3 */
            {
               flrad = (float) 1.87;
               dd->m = 15;
            }
            else if ((strcmp(dd->amin, "GLU") == 0) || (strcmp(dd->amin, "GLN") == 0))   /* C3H0 */
            {
               flrad = (float) 1.76;
               dd->m = 12;
            }
            else if ((strcmp(dd->amin, "ARG") == 0) || (strcmp(dd->amin, "LYS") == 0))   /* C4H2 */
            {
               flrad = (float) 1.87;
               dd->m = 14;
            }
            else if ((strcmp(dd->amin, "HIS") == 0) || (strcmp(dd->amin, "TYR") == 0))   /* C3H1 */
            {
               flrad = (float) 1.76;
               dd->m = 13;
            }
         }

         else if (dd->elm[1] == 'E')   /* C epsilon  */
         {
            if (strcmp(dd->amin, "MET") == 0)   /* C4H3 */
            {
               flrad = (float) 1.87;
               dd->m = 15;
            }
            else if (strcmp(dd->amin, "HIS") == 0)   /* C3H1 */
            {
               flrad = (float) 1.76;
               dd->m = 13;
            }
            else if (strcmp(dd->amin, "LYS") == 0)   /* C4H2 */
            {
               flrad = (float) 1.87;
               dd->m = 14;
            }
            else if (strcmp(dd->amin, "TRP") == 0)   /* C3H0 & C3H1 */
            {
               flrad = (float) 1.76;
               if (dd->elm[2] == '2')
                  dd->m = 12;
               else
                  dd->m = 13;
            }
            else if ((strcmp(dd->amin, "PHE") == 0) || (strcmp(dd->amin, "TYR") == 0))   /* C3H1 */
            {
               flrad = (float) 1.76;
               dd->m = 13;
            }
         }

         else if (dd->elm[1] == 'Z')   /* C zeta  */
         {
            if ((strcmp(dd->amin, "PHE") == 0) || (strcmp(dd->amin, "TRP") == 0))   /* C3H1 */
            {
               flrad = (float) 1.76;
               dd->m = 13;
            }
            else if ((strcmp(dd->amin, "TYR") == 0) || (strcmp(dd->amin, "ARG") == 0))   /* C3H0 */
            {
               flrad = (float) 1.76;
               dd->m = 12;
            }
         }

         else if (dd->elm[1] == 'H')   /* C eta  C3H1  */
         {
            flrad = (float) 1.76;
            dd->m = 13;
         }

         else if (dd->elm[1] == '1')   /* C1X of sugar or detergent */
         {
            if (isalpha(dd->elm[2]) != 0 || isdigit(dd->elm[2]) != 0)   /* selects c >= 10  */
            {
               if (dd->elm[2] == '0')   /* C10 of sugar or detergent */
               {
                  if (strcmp(dd->amin, "SIA") == 0)   /* C3H0 */
                  {
                     flrad = (float) 1.76;
                     dd->m = 12;
                  }
                  else if (strcmp(dd->amin, "OG2") == 0)   /* OCTYL GLUCOSIDE C10 C4H2 */
                  {
                     /*                  strcpy(dd->amin,"OG2"); */
                     flrad = (float) 1.87;
                     dd->m = 14;
                  }
                  else   /* unknown  */
                  {
                     flrad = (float) 1.87;
                     dd->m = 13;
                  }
               }
               else if (dd->elm[2] == '1')   /* C11 of sugar or detergent */
               {
                  if (strcmp(dd->amin, "SIA") == 0)   /* C4H3 */
                  {
                     flrad = (float) 1.87;
                     dd->m = 15;
                  }
                  else if (strcmp(dd->amin, "OG3") == 0)   /* OCTYL GLUCOSIDE C11 C4H2 */
                  {
                     /*                  strcpy(dd->amin,"OG3"); */
                     flrad = (float) 1.87;
                     dd->m = 14;
                  }
                  else   /* unknown */
                  {
                     flrad = (float) 1.87;
                     dd->m = 13;
                  }
               }
               else if (dd->elm[2] == '2')   /* C12 of detergent */
               {
                  if (strcmp(dd->amin, "OG3") == 0)   /* OCTYL GLUCOSIDE C12 C4H2 */
                  {
                     /*                  strcpy(dd->amin,"OG3"); */
                     flrad = (float) 1.87;
                     dd->m = 14;
                  }
                  else   /* unknown */
                  {
                     flrad = (float) 1.87;
                     dd->m = 13;
                  }
               }
               else if (dd->elm[2] == '3')   /* C13 of detergent */
               {
                  if (strcmp(dd->amin, "OG3") == 0)   /* OCTYL GLUCOSIDE C13 C4H2 */
                  {
                     /*                  strcpy(dd->amin,"OG3"); */
                     flrad = (float) 1.87;
                     dd->m = 14;
                  }
                  else   /* unknown */
                  {
                     flrad = (float) 1.87;
                     dd->m = 13;
                  }
               }
               else if (dd->elm[2] == '4')   /* C14 of detergent */
               {
                  if (strcmp(dd->amin, "OG3") == 0)   /* OCTYL GLUCOSIDE C14 C4H3 */
                  {
                     /*                  strcpy(dd->amin,"OG3"); */
                     flrad = (float) 1.87;
                     dd->m = 15;
                  }
                  else   /* unknown */
                  {
                     flrad = (float) 1.87;
                     dd->m = 13;
                  }
               }
            }
            else      /* selects C1 */
            {
               if (strcmp(dd->amin, "SIA") == 0)   /* C3H0 */
               {
                  flrad = (float) 1.76;
                  dd->m = 12;
               }
               else if (strcmp(dd->amin, "OG1") == 0)   /* OCTYL GLUCOSIDE C1 C4H1 */
               {
                  /*               strcpy(dd->amin,"OG1");  */
                  flrad = (float) 1.87;
                  dd->m = 13;
               }
               else   /* MAN, GAL, NAG, FUC  C4H1 */
               {
                  flrad = (float) 1.87;
                  dd->m = 13;
               }
            }
         }

         else if (dd->elm[1] == '2')   /* C2 of sugar */
         {
            if (strcmp(dd->amin, "SIA") == 0)   /* C4H0? */
            {
               flrad = (float) 1.87;
               dd->m = 12;
            }
            else if (strcmp(dd->amin, "OG1") == 0)   /* OCTYL GLUCOSIDE C2 C4H1 */
            {
               /*            strcpy(dd->amin,"OG1"); */
               flrad = (float) 1.87;
               dd->m = 13;
            }
            else      /* MAN, GAL, NAG, FUC  C4H1 */
            {
               flrad = (float) 1.87;
               dd->m = 13;
            }
         }

         else if (dd->elm[1] == '3')   /* C3 of sugar */
         {
            if (strcmp(dd->amin, "SIA") == 0)   /* C4H2 */
            {
               flrad = (float) 1.87;
               dd->m = 14;
            }
            else if (strcmp(dd->amin, "OG1") == 0)   /* OCTYL GLUCOSIDE C3 C4H1 */
            {
               /*            strcpy(dd->amin,"OG1"); */
               flrad = (float) 1.87;
               dd->m = 13;
            }
            else      /* MAN, GAL, NAG, FUC  C4H1 */
            {
               flrad = (float) 1.87;
               dd->m = 13;
            }
         }

         else if (dd->elm[1] == '4')   /* C4 of sugar  C4H1 */
         {
            if (strcmp(dd->amin, "OG1") == 0)   /* OCTYL GLUCOSIDE C4 C4H1 */
            {
               /*            strcpy(dd->amin,"OG1"); */
               flrad = (float) 1.87;
               dd->m = 13;
            }
            else      /* MAN, GAL, NAG, FUC  C4H1 */
            {
               flrad = (float) 1.87;
               dd->m = 13;
            }
         }

         else if (dd->elm[1] == '5')   /* C5 of sugar  C4H1 */
         {
            if (strcmp(dd->amin, "OG1") == 0)   /* OCTYL GLUCOSIDE C5 C4H1 */
            {
               /*            strcpy(dd->amin,"OG1"); */
               flrad = (float) 1.87;
               dd->m = 13;
            }
            else      /* MAN, GAL, NAG, FUC  C4H1 */
            {
               flrad = (float) 1.87;
               dd->m = 13;
            }
         }

         else if (dd->elm[1] == '6')   /* C6 of sugar */
         {
            if (strcmp(dd->amin, "SIA") == 0)   /* C4H1 */
            {
               flrad = (float) 1.87;
               dd->m = 13;
            }
            else if (strcmp(dd->amin, "FUC") == 0)   /* C4H3 */
            {
               flrad = (float) 1.87;
               dd->m = 15;
            }
            else if (strcmp(dd->amin, "OG1") == 0)   /* OCTYL GLUCOSIDE C6 C4H2 */
            {
               /*            strcpy(dd->amin,"OG1"); */
               flrad = (float) 1.87;
               dd->m = 14;
            }
            else      /* MAN, GAL, NAG  C4H2 */
            {
               flrad = (float) 1.87;
               dd->m = 14;
            }
         }

         else if (dd->elm[1] == '7')   /* C7 of sugar */
         {
            if (strcmp(dd->amin, "NAG") == 0)   /* C3H0 */
            {
               flrad = (float) 1.76;
               dd->m = 12;
            }
            else if (strcmp(dd->amin, "SIA") == 0)   /* C4H1 */
            {
               flrad = (float) 1.87;
               dd->m = 13;
            }
            else if (strcmp(dd->amin, "OG2") == 0)   /* OCTYL GLUCOSIDE C7 C4H2 */
            {
               /*            strcpy(dd->amin,"OG2"); */
               flrad = (float) 1.87;
               dd->m = 14;
            }
            else      /* unknown */
            {
               flrad = (float) 1.87;
               dd->m = 14;
            }
         }

         else if (dd->elm[1] == '8')   /* C8 of sugar */
         {
            if (strcmp(dd->amin, "NAG") == 0)   /* C4H3 */
            {
               flrad = (float) 1.87;
               dd->m = 15;
            }
            else if (strcmp(dd->amin, "SIA") == 0)   /* C4H1 */
            {
               flrad = (float) 1.87;
               dd->m = 13;
            }
            else if (strcmp(dd->amin, "OG2") == 0)   /* OCTYL GLUCOSIDE C8 C4H2 */
            {
               /*            strcpy(dd->amin,"OG2"); */
               flrad = (float) 1.87;
               dd->m = 14;
            }
            else      /* unknown */
            {
               flrad = (float) 1.87;
               dd->m = 14;
            }
         }

         else if (dd->elm[1] == '9')   /* C9 of sugar */
         {
            if (strcmp(dd->amin, "SIA") == 0)   /* C4H2 */
            {
               flrad = (float) 1.87;
               dd->m = 14;
            }
            else if (strcmp(dd->amin, "OG2") == 0)   /* OCTYL GLUCOSIDE C9 C4H2 */
            {
               /*            strcpy(dd->amin,"OG2"); */
               flrad = (float) 1.87;
               dd->m = 14;
            }
            else      /* unknown */
            {
               flrad = (float) 1.87;
               dd->m = 14;
            }
         }

         else      /*  unknown  */
         {
            flrad = (float) 1.8;
            dd->m = 14;
         }
      }
      else         /* C of peptide bond  C3H0 */
      {
         flrad = (float) 1.76;
         dd->m = 12;
      }
      break;

   case 'O':
      if (isalpha(dd->elm[1]) != 0 || isdigit(dd->elm[1]) != 0)   /* selects Ox  */
      {
         if ((dd->elm[1] == 'D') || (dd->elm[1] == 'E'))   /* selects OD & OE  O1H0 */
         {
            flrad = (float) 1.42;
            dd->m = 16;
         }

         else if ((dd->elm[1] == 'G') || (dd->elm[1] == 'H'))   /* selects OG & OH  O2H1 */
         {
            flrad = (float) 1.46;
            dd->m = 17;
         }
         else if (dd->elm[1] == '1')   /* O1X of sugar */
         {
            if (isalpha(dd->elm[2]) != 0 || isdigit(dd->elm[2]) != 0)   /* selects O >= 10 or O1A/O1B  */
            {
               if ((dd->elm[2] == 'A') || (dd->elm[2] == 'B'))   /* selects O1A and O1B of SIA */
               {
                  if (strcmp(dd->amin, "SIA") == 0)   /* O1H0 */
                  {
                     flrad = (float) 1.42;
                     dd->m = 16;
                  }
                  else   /* unknown  */
                  {
                     flrad = (float) 1.40;
                     dd->m = 16;
                  }
               }
               else if (dd->elm[2] == '0')   /* O10 of sugar */
               {
                  if (strcmp(dd->amin, "SIA") == 0)   /* O1H0 */
                  {
                     flrad = (float) 1.42;
                     dd->m = 16;
                  }
                  else   /* unknown  */
                  {
                     flrad = (float) 1.40;
                     dd->m = 16;
                  }
               }
            }
            else      /* selects O1 */
            {
               if (strcmp(dd->amin, "SIA") == 0)   /* O1H0 */
               {
                  flrad = (float) 1.42;
                  dd->m = 16;
               }
               else if (strcmp(dd->amin, "OG1") == 0)   /* OCTYL GLUCOSIDE O1 O1H0 */
               {
                  /*               strcpy(dd->amin,"OG1"); */
                  flrad = (float) 1.42;
                  dd->m = 16;
               }
               else   /* MAN, GAL, NAG, FUC  O2H1 */
               {
                  flrad = (float) 1.46;
                  dd->m = 17;
               }
            }
         }

         else if (dd->elm[1] == '2')   /* O2 of sugar */
         {
            if (strcmp(dd->amin, "OG1") == 0)   /* OCTYL GLUCOSIDE O2 O2H1 */
            {
               /*               strcpy(dd->amin,"OG1"); */
               flrad = (float) 1.46;
               dd->m = 17;
            }
            else      /* MAN, GAL, FUC, SIA  O2H1 */
            {
               flrad = (float) 1.46;
               dd->m = 17;
            }
         }

         else if (dd->elm[1] == '3')   /* O3 of sugar */
         {
            if (strcmp(dd->amin, "OG1") == 0)   /* OCTYL GLUCOSIDE O3 O2H1 */
            {
               /*               strcpy(dd->amin,"OG1"); */
               flrad = (float) 1.46;
               dd->m = 17;
            }
            else      /* MAN, GAL, NAG, FUC  O2H1 */
            {
               flrad = (float) 1.46;
               dd->m = 17;
            }
         }

         else if (dd->elm[1] == '4')   /* O4 of sugar  O2H1 */
         {
            flrad = (float) 1.46;
            dd->m = 17;
            /*             if(strcmp(dd->amin,"OG1")==0) / * OCTYL GLUCOSIDE O4 O2H1 */
            /*               strcpy(dd->amin,"OG1"); */
         }

         else if (dd->elm[1] == '5')   /* O5 of sugar */
         {
            /* MAN, GAL, NAG, FUC O1H0 */
            {
               flrad = (float) 1.42;
               dd->m = 16;
               /*             if(strcmp(dd->amin,"OG1")==0) / * OCTYL GLUCOSIDE O5 O2H1 */
               /*               strcpy(dd->amin,"OG1"); */
            }
         }

         else if (dd->elm[1] == '6')   /* O6 of sugar */
         {
            if (strcmp(dd->amin, "SIA") == 0)   /* O1H0 */
            {
               flrad = (float) 1.42;
               dd->m = 16;
            }
            else if (strcmp(dd->amin, "OG1") == 0)   /* OCTYL GLUCOSIDE O6 O2H1 */
            {
               /*            strcpy(dd->amin,"OG1"); */
               flrad = (float) 1.46;
               dd->m = 17;
            }
            else      /* MAN, GAL, NAG  O2H1 */
            {
               flrad = (float) 1.46;
               dd->m = 17;
            }
         }

         else if (dd->elm[1] == '7')   /* O7 of sugar */
         {
            if (strcmp(dd->amin, "NAG") == 0)   /* O1H0 */
            {
               flrad = (float) 1.42;
               dd->m = 16;
            }
            else if (strcmp(dd->amin, "SIA") == 0)   /* O2H1 */
            {
               flrad = (float) 1.46;
               dd->m = 17;
            }
            else      /* unknown */
            {
               flrad = (float) 1.40;
               dd->m = 16;
            }
         }

         else if (dd->elm[1] == '8')   /* O8 of sugar */
         {
            if (strcmp(dd->amin, "SIA") == 0)   /* O2H1 */
            {
               flrad = (float) 1.46;
               dd->m = 17;
            }
            else      /* unknown */
            {
               flrad = (float) 1.40;
               dd->m = 16;
            }
         }

         else if (dd->elm[1] == '9')   /* O9 of sugar */
         {
            if (strcmp(dd->amin, "SIA") == 0)   /* O2H1 */
            {
               flrad = (float) 1.46;
               dd->m = 17;
            }
            else      /* unknown */
            {
               flrad = (float) 1.40;
               dd->m = 16;
            }
         }
         else
         {
            flrad = (float) 1.4;
            dd->m = 16;
         }
      }
      else
      {
         flrad = (float) 1.4;
         dd->m = 16;
      }
      break;

   case 'N':
      if (isalpha(dd->elm[1]) != 0 || isdigit(dd->elm[1]) != 0)
         /*      flrad=(float) 1.8;      */
      {
         if ((dd->elm[1] == 'D') || (dd->elm[1] == 'E'))   /* selects ND & NE  */
         {
            if ((strcmp(dd->amin, "GLN") == 0) || (strcmp(dd->amin, "ASN") == 0))   /* N3H2 */
            {
               flrad = (float) 1.64;
               dd->m = 16;
            }
            else if ((strcmp(dd->amin, "TRP") == 0) || (strcmp(dd->amin, "HIS") == 0))   /* N3H2 */
            {
               if (dd->elm[2] == '1')   /* ND1 of HIS or NE1  N3H1 */
               {
                  flrad = (float) 1.64;
                  dd->m = 15;
               }
               else   /* NE2 of HIS  N3H0 */
               {
                  flrad = (float) 1.64;
                  dd->m = 14;
               }
            }
            else if (strcmp(dd->amin, "ARG") == 0)   /* N3H1 */
            {
               /* printf("qui arrivo arg");
                  if ( 1 == scanf("%s",pluto) ) {};
                  getchar(); */
               flrad = (float) 1.64;
               dd->m = 15;
               /* printf("qui arrivo arg flrad= %f\n",flrad);
                  if ( 1 == scanf("%s",pluto) ) {};
                  getchar(); */
            }
         }

         else if (dd->elm[1] == 'Z')   /* N zeta  */
         {
            if (strcmp(dd->amin, "LYS") == 0)   /* N4H3 */
            {
               flrad = (float) 1.64;
               dd->m = 17;
            }
            else if (strcmp(dd->amin, "ARG") == 0)   /* N3H2 */
            {
               flrad = (float) 1.64;
               dd->m = 16;
            }
         }

         else if (dd->elm[1] == '2')   /* N2 of sugar NAG N3H1 */
         {
            flrad = (float) 1.64;
            dd->m = 15;
         }

         else if (dd->elm[1] == '5')   /* N5 of sugar SIA N3H1 */
         {
            flrad = (float) 1.64;
            dd->m = 15;
         }
         else
         {
            flrad = (float) 1.64;
            dd->m = 14;
         }
      }
      else
      {
         flrad = (float) 1.64;
         if (strcmp(dd->amin, "PRO") != 0)
            dd->m = 15;
         else
            dd->m = 14;
      }
      break;

   case 'S':
      if (strcmp(dd->amin, "MET") == 0)   /* S2H0 */
      {
         flrad = (float) 1.77;
         dd->m = 32;
      }

      else if (strcmp(dd->amin, "CYS") == 0)   /* S2H1 */
      {
         flrad = (float) 1.77;
         dd->m = 33;
      }

      break;

   default:
      flrad = (float) 1.8;
   }

   fprintf(init3_mol1, "%s\t%f\n", dd->elm, flrad);
   dd->r = flrad + rprobe;
}

/************************************************************************/

static void
assigntab(int xx)
{
   int k;
   float x;
   struct dati1 *dd;

   dd = dt;
   dd += xx;

   /* printf("rbulk= %d\trprobe= %f\n",rbulk,rprobe);
      if ( 1 == scanf("%s",pluto) ) {};
      getchar(); */

   if (rbulk == 0)
      k = 0;
   else
      k = 28;

   if (rprobe > 0.001)      /* 25/6/02 CONTROLLO, E' STRANO! LA FORMULA È PER L'AREA DEL CERCHIO, NON PER LA SUPERFICE DELLA SFERA!!!!! IMPLEMENTATA QUELLA GIUSTA SOTTO, E RICALCOLATI I PARAMETRI PER LA TABELLA */
   {
      /*   for(rbulk=0;rbulk<20;rbulk++)
           {
           x=(float) (arr1[rbulk]);
           x/=PI;
           x=(float) sqrt(x);
           x+=rprobe;
           x*=x;
           x*=PI;
           arr1[20+rbulk]=(int) x;
           printf("%s%d%s%f\n","arr1[",20+rbulk,"]= ",arr1[20+rbulk]);
           }    */

      for (rbulk = 0; rbulk < 28; rbulk++)
      {
         x = (float) (arr1[rbulk]);
         x /= (4.0 * PI);
         x = (float) sqrt(x);
         x += rprobe;
         x *= x;
         x *= (4.0 * PI);
         arr1[28 + rbulk] = (int) x;
         /*      printf("%s%d%s%f\n","arr1[",28+rbulk,"]= ",arr1[28+rbulk]); */
      }
   }

   switch (dd->amin[0])
   {
   case ('A'):
      switch (dd->amin[1])
      {
      case ('L'):
         dd->tab = arr1[k];
         dd->vol = (float) 87.8;
         if (strcmp(dd->elm, "CB") == 0)
            dd->m = 15;
         break;

      case ('R'):
         dd->tab = arr1[k + 1];
         dd->vol = (float) 188.2;

         switch (dd->elm[0])
         {
         case ('C'):
            dd->m = 14;   /*C? */
            if (dd->elm[1] == 'Z')
               dd->m = 12;
            /*CZ*/ if (dd->elm[1] == 'A')
               dd->m = 13;
            /*CA*/ break;

         case ('N'):
            if (dd->elm[1] == 'E')
               dd->m = 15;
            /*NE*/ if (dd->elm[2] == '1')
               dd->m = 16;   /*NH1 */
            if (dd->elm[2] == '2')
               dd->m = 17;   /*NH2 */
            break;
         }
         break;

      case ('S'):
         if (dd->amin[2] == 'N')
            /*ASN*/
         {
            dd->tab = arr1[k + 2];
            dd->vol = (float) 120.1;

            switch (dd->elm[0])
            {
            case ('C'):
               dd->m = 14;
               /*CB*/ if (dd->elm[1] == 'G')
                  dd->m = 12;
               /*CG*/ break;

            case ('O'):
               dd->m = 16;   /*OD1 */
               break;

            case ('N'):
               dd->m = 18;   /*ND2 */
               break;
            }
         }
         else
            /*ASP*/
         {
            dd->tab = arr1[k + 3];
            dd->vol = (float) 115.4;

            switch (dd->elm[0])
            {
            case ('C'):
               dd->m = 14;
               /*CB*/ if (dd->elm[1] == 'G')
                  dd->m = 12;
               /*CG*/ break;

            case ('O'):
               dd->m = 16;   /*OD1 */
               break;
            }
         }
         break;
      }
      break;

   case ('C'):
      dd->tab = arr1[k + 4];
      dd->vol = (float) 105.4;
      if (dd->elm[0] == 'C')
         dd->m = 14;
      else
         dd->m = 33;
      break;

   case ('G'):
      switch (dd->amin[2])
      {
      case ('N'):
         dd->tab = arr1[k + 5];
         dd->vol = (float) 145.1;

         switch (dd->elm[0])
         {
         case ('C'):
            dd->m = 14;   /*C? */
            if (dd->elm[1] == 'D')
               dd->m = 12;
            /*CD*/ break;

         default:
            dd->m = 16;   /* OE1, NE2 */
         }
         break;

      case ('U'):
         dd->tab = arr1[k + 6];
         dd->vol = (float) 140.9;

         switch (dd->elm[0])
         {
         case ('C'):
            dd->m = 14;   /*C? */
            if (dd->elm[1] == 'D')
               dd->m = 12;
            /*CD*/ break;

         default:
            dd->m = 16;   /* OE1, OE2 */
         }
         break;

      case ('Y'):
         dd->tab = arr1[k + 7];
         dd->vol = (float) 59.9;
         break;

      case ('L'):      /* GAL */
         dd->tab = arr1[k + 20];
         dd->vol = (float) 165.0;
         break;
      }
      break;

   case ('H'):
      dd->tab = arr1[k + 8];
      dd->vol = (float) 156.3;

      switch (dd->elm[0])
      {
      case ('C'):
         dd->m = 13;      /*C? */
         if (dd->elm[1] == 'B')
            dd->m = 14;
         /*CB*/ if (dd->elm[1] == 'G')
            dd->m = 12;
         /*CG*/ break;

      case ('N'):
         dd->m = 15;      /*ND1 */
         if (dd->elm[1] == 'E')
            dd->m = 14;   /*NE2 */
         break;
      }
      break;

   case ('I'):
      dd->tab = arr1[k + 9];
      dd->vol = (float) 166.1;
      dd->m = 15;      /*C? */
      if (dd->elm[1] == 'B')
         dd->m = 13;
      /*CB*/ if (strcmp(dd->elm, "CG1") == 0)
         dd->m = 14;      /*CG1 */
      break;

   case ('N'):      /* NAG */
      dd->tab = arr1[k + 21];
      dd->vol = (float) 224.7;
      break;

   case ('F'):      /* FUC */
      dd->tab = arr1[k + 22];
      dd->vol = (float) 162.4;
      break;

   case ('L'):
      switch (dd->amin[1])
      {
      case ('E'):
         dd->tab = arr1[k + 10];
         dd->vol = 168;

         switch (dd->elm[1])
         {
         case ('B'):
            dd->m = 14;
            /*CB*/ break;
         case ('G'):
            dd->m = 13;
            /*CG*/ break;
         case ('D'):
            dd->m = 15;   /*CD? */
            break;
         }
         break;

      case ('Y'):
         dd->tab = arr1[k + 11];
         dd->vol = (float) 172.7;

         switch (dd->elm[0])
         {
         case ('C'):
            dd->m = 14;   /*C? */
            break;

         default:
            dd->m = 17;
            /*NZ*/}
         break;
      }
      break;

   case ('M'):
      switch (dd->amin[1])
      {
      case ('E'):
         dd->tab = arr1[k + 12];
         dd->vol = (float) 165.2;

         switch (dd->elm[1])
         {
         case ('D'):
            dd->m = 32;
            /*SD*/ break;
         case ('E'):
            dd->m = 15;
            /*CE*/ break;
         default:
            dd->m = 14;   /*C? */
         }
         break;
      case ('A'):      /* MAN */
         dd->tab = arr1[k + 23];
         dd->vol = (float) 165.0;
         break;
      }
      break;

   case ('P'):
      switch (dd->amin[1])
      {
      case ('H'):
         dd->tab = arr1[k + 13];
         dd->vol = (float) 189.7;

         switch (dd->elm[1])
         {
         case ('B'):
            dd->m = 14;
            /*CB*/ break;
         case ('G'):
            dd->m = 12;
            /*CG*/ break;
         default:
            dd->m = 13;   /*C? */
         }
         break;
         break;

      case ('R'):
         dd->tab = arr1[k + 14];
         dd->vol = (float) 123.3;
         dd->m = 14;      /*C? */
         break;
      }
      break;

   case ('S'):
      switch (dd->amin[1])
      {
      case ('E'):
         dd->tab = arr1[k + 15];
         dd->vol = (float) 91.7;
         if (dd->elm[0] == 'C')
            dd->m = 14;
         else
            dd->m = 17;

         break;

      case ('I'):      /* SIA */
         dd->tab = arr1[k + 24];
         dd->vol = (float) 280.8;
         break;
      }
      break;

   case ('T'):
      switch (dd->amin[1])
      {
      case ('H'):
         dd->tab = arr1[k + 16];
         dd->vol = (float) 118.3;
         if (strcmp(dd->elm, "CB") == 0)
            dd->m = 13;
         if (strcmp(dd->elm, "OG1") == 0)
            dd->m = 17;
         if (strcmp(dd->elm, "CG2") == 0)
            dd->m = 15;
         break;

      case ('R'):
         dd->tab = arr1[k + 17];
         dd->vol = (float) 227.9;

         switch (dd->elm[0])
         {
         case ('N'):
            dd->m = 15;   /*NE1 */
            break;
         case ('C'):
            if (strcmp(dd->elm, "CB") == 0)
               dd->m = 14;
            else if ((strcmp(dd->elm, "CG") == 0) || (strcmp(dd->elm, "CD2") == 0) || (strcmp(dd->elm, "CE2") == 0))
               dd->m = 12;
            else
               dd->m = 13;   /*C? */
         }
         break;

      case ('Y'):
         dd->tab = arr1[k + 18];
         dd->vol = (float) 191.2;

         switch (dd->elm[0])
         {
         case ('O'):
            dd->m = 17;
            /*OH*/ break;
         case ('C'):
            if (strcmp(dd->elm, "CB") == 0)
               dd->m = 14;
            else if ((strcmp(dd->elm, "CG") == 0) || (strcmp(dd->elm, "CZ") == 0))
               dd->m = 12;
            else
               dd->m = 13;   /*C? */
         }
         break;
      }
      break;

   case ('V'):
      dd->tab = arr1[k + 19];
      dd->vol = (float) 138.8;

      if (strcmp(dd->elm, "CB") == 0)
         dd->m = 13;
      else
         dd->m = 15;
      break;

   case ('O'):
      switch (dd->amin[2])
      {
      case ('1'):      /* OG1 */
         dd->tab = arr1[k + 25];
         dd->vol = (float) 165.0;

         break;

      case ('2'):      /* OG2 */
         dd->tab = arr1[k + 26];
         dd->vol = (float) 100.0;

         break;

      case ('3'):      /* OG3 */
         dd->tab = arr1[k + 27];
         dd->vol = (float) 100.0;

         break;
      }

      /* printf("%s%s\t%s%f\n","dd->amin= ",dd->amin,"dd->tab= ",dd->tab);
         if ( 1 == scanf("%s",pluto) ) {};
         getchar(); 
         break;
         printf("%s%s\t%s%f\n","dd->amin= ",dd->amin,"dd->tab= ",dd->tab);
         if ( 1 == scanf("%s",pluto) ) {};
         getchar(); */
   }
   /* printf("%s%s\t%s%f\n","dd->amin= ",dd->amin,"dd->tab= ",dd->tab);
      if ( 1 == scanf("%s",pluto) ) {};
      getchar(); */
}

// ----------------------------------- maxmin.c -------------------------------------
// #warning maxmin.c 

// void mami();
// void mami1();

static void
mami()
{
   int i;

   struct dati1 *dd;

   dd = dts;

   minx = dd->x - dd->r;
   miny = dd->y - dd->r;
   maxx = dd->x + dd->r;
   maxy = dd->y + dd->r;

   for (i = 1; i < cont1; i++)
   {
      dd++;
      if (maxx < (dd->x + dd->r))
         maxx = dd->x + dd->r;
      if (minx > (dd->x - dd->r))
         minx = dd->x - dd->r;
      if (maxy < (dd->y + dd->r))
         maxy = dd->y + dd->r;
      if (miny > (dd->y - dd->r))
         miny = dd->y - dd->r;
   }
}

void
mami1()
{
   int i;

   struct dati1 *dd;

   dd = dt;

   minz = dd->z - dd->r;
   maxz = dd->z + dd->r;

   for (i = 1; i < nat; i++)
   {
      dd++;
      if (minz > (dd->z - dd->r))
         minz = dd->z - dd->r;
      if (maxz < (dd->z + dd->r))
         maxz = dd->z + dd->r;
   }
}

// ----------------------------------- ordcol.c -------------------------------------
// #warning ordcol.c 

static FILE *ordcol_ord;
static FILE *ord1;

// void ordcol();
static int conf(char *, char *, int);

static void
ordcol()
{
   char key[200];
   char sp[4], fine[4];
   int i;

   fine[0] = 'S';
   fine[1] = 'P';
   fine[2] = ';';

   ordcol_ord = us_fopen("plotter", "rb");
   ord1 = us_fopen("plotter1", "ab");

   if ( 1 == fscanf(ordcol_ord, "%s", key) ) {};
   fprintf(ord1, "%s%c", key, ' ');

   if (form1 == 0)
   {
      if ( 1 == fscanf(ordcol_ord, "%s", key) ) {};
      fprintf(ord1, "%s\n", key);
   }

   for (i = 1; i < 9; i++)
   {
      rewind(ordcol_ord);
      if ( 1 == fscanf(ordcol_ord, "%s", key) ) {};
      sp[0] = 'S';
      sp[1] = 'P';

      switch (i)
      {
      case (1):
         sp[2] = '1';
         break;
      case (2):
         sp[2] = '2';
         break;
      case (3):
         sp[2] = '3';
         break;
      case (4):
         sp[2] = '4';
         break;
      case (5):
         sp[2] = '5';
         break;
      case (6):
         sp[2] = '6';
         break;
      case (7):
         sp[2] = '7';
         break;
      case (8):
         sp[2] = '8';
         break;
      }

      while (conf(key, fine, 3) == 0)
      {
         while ((conf(key, sp, 3) == 0) && (conf(key, fine, 3) == 0)) {
            if ( 1 == fscanf(ordcol_ord, "%s", key) ) {};
         }

         if (conf(key, fine, 3) == 0)
         {
            fprintf(ord1, "%s\n", key);
            if ( 1 == fscanf(ordcol_ord, "%s", key) ) {};
         }
      }
   }

   fprintf(ord1, "%s", "SP;");
   if (numdis > 1)
      fprintf(ord1, "%s%d%c\n", "RP", numdis - 1, ';');

   fclose(ordcol_ord);
   fclose(ord1);
}

/**************************************************************************/

static int
conf(char a[200], char b[4], int c)
{

   int i, flag;

   i = 0;
   flag = 0;

   while ((a[i] == b[i]) && (i < c))
      i++;

   if (i == c)
      flag = 1;

   return (flag);
}

/**************************************************************************/

// ----------------------------------- plotter.c -------------------------------------
// #warning plotter.c 

static FILE *plotter_pl;

static void plotinit();
static void plotcircle();
static void plotarc(int);
static void plotend();

/* plotter initializations  */
static void
plotinit()
{
   plotter_pl = us_fopen("plotter", "ab");
   if (form == 1)
   {
      fprintf(plotter_pl, "%s%d%c", "DF;IN;IP;VS", vel, ';');

      if (numdis > 1)
         fprintf(plotter_pl, "%s", "GM;BF;");

      if (form1 == 0)
      {
         fprintf(plotter_pl, "%s", "RO 90;IP;");
         fprintf(plotter_pl, "%s%d%c%d%c%d%c%d%c", "IP", 200, ',', 1830, ',', 7400, ',', 9030, ';');
         fprintf(plotter_pl, "%s%f%c%f%c%f%c%f%c\n", "SC", xp1, ',', xp2, ',', yp1, ',', yp2, ';');
      }
      else
      {
         fprintf(plotter_pl, "%s%d%c%d%c%d%c%d%c", "IP", 1830, ',', 200, ',', 9030, ',', 7400, ';');
         fprintf(plotter_pl, "%s%f%c%f%c%f%c%f%c\n", "SC", xp1, ',', xp2, ',', yp1, ',', yp2, ';');
      }
   }
   else
   {
      fprintf(plotter_pl, "%s%d%c", "DF;IN;IP;VS", vel, ';');
      if (numdis > 1)
         fprintf(plotter_pl, "%s", "GM;BF;");
      if (form1 == 0)
      {
         fprintf(plotter_pl, "%s", "RO 90;IP;");
         fprintf(plotter_pl, "%s%d%c%d%c%d%c%d%c", "IP", 430, ',', 2965, ',', 10430, ',', 12965, ';');
         fprintf(plotter_pl, "%s%f%c%f%c%f%c%f%c\n", "SC", xp1, ',', xp2, ',', yp1, ',', yp2, ';');
      }
      else
      {
         fprintf(plotter_pl, "%s%d%c%d%c%d%c%d%c\n", "IP", 2965, ',', 430, ',', 12965, ',', 10430, ';');
         fprintf(plotter_pl, "%s%f%c%f%c%f%c%f%c\n", "SC", xp1, ',', xp2, ',', yp1, ',', yp2, ';');
      }
   }

   fclose(plotter_pl);
}

/* drawing a complete circle */
static void
plotcircle()
{
   float d;
   struct dati1 *dd;

   dd = dts;
   dd += nc;

   plotter_pl = us_fopen("plotter", "ab");

   d = dd->r - (fl * (((float) 1.0) + ((float) ceil(nat / 15.)) / ((float) 2.0)));

   fprintf(plotter_pl, "%s%d%c", "SP", dd->col, ';');
   fprintf(plotter_pl, "%s%f%c%f%c", "PA", dd->x, ',', dd->y, ';');
   fprintf(plotter_pl, "%s%f%c%d%c", "CI", d, ',', 5, ';');
   fprintf(plotter_pl, "%s%c\n", "PU", ';');

   fclose(plotter_pl);
}

/* disegna gli archi */
static void
plotarc(int i)
{
   struct dati1 *dd;

   dd = dts;
   dd += nc;

   plotter_pl = us_fopen("plotter", "ab");
   fprintf(plotter_pl, "%s%d%c", "SP", dd->col, ';');
   fprintf(plotter_pl, "%s%f%c%f%c", "PA", dis[i].xi, ',', dis[i].yi, ';');
   fprintf(plotter_pl, "%s%f%c%f%c%f%c", "PD;AA", dd->x, ',', dd->y, ',', dis[i].ai, ';');
   fprintf(plotter_pl, "%s%c\n", "PU", ';');

   fclose(plotter_pl);
}

static void
plotend()
{
   plotter_pl = us_fopen("plotter", "ab");
   fprintf(plotter_pl, "%s", "SP;");
   fclose(plotter_pl);
}

// ----------------------------------- rint.c -------------------------------------
// #warning rint.c 

static void
cercaint(int k)
{
   double a, b, c;
   double k1, k2, k3, k4, k5, delta;

   struct dati1 *yy1, *yy2;

   yy1 = yy2 = dts;
   yy1 += nc;
   yy2 += k;

   a = (yy1->y - yy2->y);
   b = (yy1->r * yy1->r - yy2->r * yy2->r - yy1->x * yy1->x - yy1->y * yy1->y + yy2->x * yy2->x + yy2->y * yy2->y) / 2.0;
   c = (yy2->x - yy1->x);

   if (yy2->x != yy1->x)
   {
      k1 = (a * a + c * c);
      k2 = (a * b - a * c * yy2->x - c * c * yy2->y);
      k3 = (b - c * yy2->x) * (b - c * yy2->x) + c * c * (yy2->y * yy2->y - yy2->r * yy2->r);

      delta = (k2 * k2 - k1 * k3);
      if ((delta < 0.0)) // && (delta > -0.00001))
         delta = 0.0;

      iy1 = (-k2 + sqrt(delta)) / k1;
      iy2 = (-k2 - sqrt(delta)) / k1;
      ix1 = (a * iy1 + b) / c;
      ix2 = (a * iy2 + b) / c;
   }
   else
   {
      k4 = (-a * a * yy2->x);
      k5 = (b + a * yy2->y) * (b + a * yy2->y) + a * a * (yy2->x * yy2->x - yy2->r * yy2->r);

      delta = (k4 * k4 - a * a * k5);
      if ((delta < 0.0)) // && (delta > -0.00001))
         delta = 0.0;

      iy1 = (-b / a);
      iy2 = (-b / a);
      ix1 = (-k4 + sqrt(delta)) / (a * a);
      ix2 = (-k4 - sqrt(delta)) / (a * a);
   }
}

// ----------------------------------- scala.c -------------------------------------
// #warning scala.c 

// static void mami();

// static void formato();
// static void raggio_probe();
// static void scala();

// static void pulisci();

static void
formato()
{
   // int  p;
   int conferma;
   float passo;

   pulisci();
#if defined( DEBUG_ASA )
   printf("\n\n%s%f%s", "Molecule's extension along the z-axis = ", fabs(maxz - minz), " [angstrom]");
#endif

   conferma = 0;
   while (conferma == 0)
   {
#if defined( DEBUG_ASA )
      printf("\n\nInsert the integration step in angstroms : ");
#endif
      // scanf("%f", &passo);
      passo = asa_opts->asab1_step;
      printf("%.2f\n", passo);

      while (passo <= 0)
      {
         printf("\n");
         printf("Wrong number!\n");
         printf("Re-insert the step: ");
         if ( 1 == scanf("%f", &passo) ) {};
      }

#if defined( DEBUG_ASA )
      printf("\n\n%s%.0f\n\n", "Number of resulting iterations: ", ceil(fabs(maxz - minz) / passo));
#endif
      // printf("%s", "Confirm ? [yes=1;no=0] ");
      // scanf("%d", &conferma);
      conferma = 1;
   }

   passi = (int) (ceil(fabs(maxz - minz) / passo));

   quota = 2;
   form = 1;
   form1 = 1;
   vel = 1;
   k = 1;
   // p = 0;
   fl = (float) 0.022;
   q1 = 3;
   numdis = 1;

   /* closing the format procedure */
}

/******************************************************************/

static void
scala()
{
   mami();

   base = maxx - minx;
   altez = maxy - miny;

   if (base > altez)
      medio = base / ((float) 2.0);
   else
      medio = altez / ((float) 2.0);

   m1 = (maxx + minx) / ((float) 2.0);
   m2 = (maxy + miny) / ((float) 2.0);

   if (q1 == 1)
      /* opening the quadrants' case */
   {
      switch (q2)
      {
      case 1:
         {
            xp1 = (m1 - k * 3 * medio);
            yp1 = (m2 - k * 3 * medio);
            xp2 = (m1 + k * medio);
            yp2 = (m2 + k * medio);
            break;
         }

      case 2:
         {
            xp1 = (m1 - k * medio);
            yp1 = (m2 - k * 3 * medio);
            xp2 = (m1 + k * 3 * medio);
            yp2 = (m2 + k * medio);

            break;
         }

      case 3:
         {
            xp1 = (m1 - k * medio);
            yp1 = (m2 - k * medio);
            xp2 = (m1 + k * 3 * medio);
            yp2 = (m2 + k * 3 * medio);

            break;
         }

      case 4:
         {
            xp1 = (m1 - k * 3 * medio);
            yp1 = (m2 - k * medio);
            xp2 = (m1 + k * medio);
            yp2 = (m2 + k * 3 * medio);

            break;
         }

      }         /* closing the quadrants' switch */

   }            /* closing the quadrants' if */

   else if (q1 == 2)      /* opening the halves case */
   {
      pulisci();
      switch (q2)
      {
      case 1:
         {
            xp1 = (m1 - k * 3 * medio);
            yp1 = (m2 - k * 2 * medio);
            xp2 = (m1 + k * medio);
            yp2 = (m2 + k * 2 * medio);

            break;
         }

      case 2:
         {
            xp1 = (m1 - k * medio);
            yp1 = (m2 - k * 2 * medio);
            xp2 = (m1 + k * 3 * medio);
            yp2 = (m2 + k * 2 * medio);

            break;
         }

      case 3:
         {
            xp1 = (m1 - k * m2 * medio);
            yp1 = (m2 - k * 3 * medio);
            xp2 = (m1 + k * 2 * medio);
            yp2 = (m2 + k * medio);

            break;
         }

      case 4:
         {
            xp1 = (m1 - k * m2 * medio);
            yp1 = (m2 - k * medio);
            xp2 = (m1 + k * 2 * medio);
            yp2 = (m2 + k * 3 * medio);

            break;
         }

      }         /* closing the halves' switch */

   }            /* closing the halves' if */

   else         /* case of the drawing in the center of the sheet */
   {
      xp1 = (m1 - k * medio);
      yp1 = (m2 - k * medio);
      xp2 = (m1 + k * medio);
      yp2 = (m2 + k * medio);
   }
}

static void
raggio_probe()
{
   // printf("\n\n\nPROBE RADIUS [>=0.0]___");
   // rbulk = scanf("%f", &rprobe);
    
   // while ((rbulk != 1) || (rprobe < 0.0))
   //{
   //printf("\n\n\n");
   //printf("Wrong number !!\n");
   //printf("Re-insert the PROBE RADIUS");
   //printf("\n\n\nPROBE RADIUS [>=0.0]___");
   //rbulk = scanf("%f", &rprobe);
   //}
   if(recheck) {
      rprobe = asa_opts->probe_recheck_radius;
   } else {
      rprobe = asa_opts->probe_radius;
   }
    
   rbulk = 1;

   rprobe /= 10 * unit_conv;

   if (rprobe <= 0.001)
      rbulk = 0;

   // qDebug() << "probe radius " << rprobe;
}

// ----------------------------------- val.c -------------------------------------
// #warning val.c 

static float ang(float, float);
static float ang1(float, float, int);
static float dist(float, float, float, float);

static void valida(float, float, int);

static void
valida(float ix, float iy, int k)
{
   int flag, j, temp1;

   struct dati1 *dd;
   struct dati2 *y1;

   flag = 1;
   j = 0;

   while ((j < cont1) && (flag == 1))
   {
      if ((j == k) || (j == nc))
         j++;
      else
      {
         dd = dts;
         dd += j;
         if (dist(dd->x, dd->y, ix, iy) < dd->r)
            flag = 0;
         else
            j++;
      }
   }

   if (flag == 1)
   {
      temp1 = indec[k];
      y1 = trans;
      y1 += (k * IMAX) + temp1;

      y1->xi = ix;
      y1->yi = iy;
      y1->ai = ang1(ix, iy, k);
      indec[k]++;

      temp1 = indec[nc];
      y1 = trans;
      y1 += (nc * IMAX) + temp1;

      y1->xi = ix;
      y1->yi = iy;
      y1->ai = ang(ix, iy);
      indec[nc]++;
   }
}
