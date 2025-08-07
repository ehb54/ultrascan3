/* Copyright Oleg Tsodikov 2002 (C) */
/* The following source code code is protected by the US and International
 * Copyright laws*/
/* Distribution or reproduction of this code or parts thereof in any form is
 * prohibited without explicit consent of the author*/
/* Used with permission */
/* integrated into UltraScan by E. Brookes, 2008 */

#undef DEBUG

#include "../include/us_surfracer.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../include/us_hydrodyn.h"
#define VERSION 3.0

static float *a;
static float *ar;
static float *ver;
static float *cir;
static float *molarea;
static float *curvat; /*atom coordinates, radii and molecular (smooth) areas and
                         average curvature */
static float *ve;     // atomic coordinates
static float *ci;     // radii
static float *aarea;  // areas
static short *visits;

static int *verat;
static int *cirat;
static int *edge;
static int *edgeatom;
static int *veat;
static int *atomcon;
static int *ciat;

static char *veflag;

static float prober;
static int nv;
static int ncircle;
static int ned;
static int atomnumber;
static int calcmode;

static QProgressBar *progress;
static QTextEdit *editor;

#define MAXCYCLES 40
// #define DEBUGMSG

static void dbg(const char *
#if defined(DEBUGMSG)
                    s
#endif
) {
#if defined(DEBUGMSG)
  printf("sr: %s\n", s);
  fflush(stdout);
#endif
}

static float dist2(int i, int j) /*square of the distance between two atoms */
{
  float dx, dy, dz;

  dx = a[3 * j] - a[3 * i];
  dy = a[3 * j + 1] - a[3 * i + 1];
  dz = a[3 * j + 2] - a[3 * i + 2];
  return (dx * dx + dy * dy + dz * dz);
}

static float dis2(float *p1,
                  float *p2) /*square of the distance between two points */
{
  float dx, dz, dy;

  dx = p1[0] - p2[0];
  dy = p1[1] - p2[1];
  dz = p1[2] - p2[2];
  return (dx * dx + dy * dy + dz * dz);
}

static float dist(int i, int j) /*distance between two atoms i and j */
{
  return (sqrt(dist2(i, j)));
}

static void vsum(float *v1, float *v2, float *sum) /*vector sum */
{
  sum[0] = v1[0] + v2[0];
  sum[1] = v1[1] + v2[1];
  sum[2] = v1[2] + v2[2];
}

static void vdiff(float *v1, float *v2, float *diff) /*vector difference */
{
  diff[0] = v1[0] - v2[0];
  diff[1] = v1[1] - v2[1];
  diff[2] = v1[2] - v2[2];
}

static void vtimk(float *v, float k) /*vector times a constant */
{
  v[0] *= k;
  v[1] *= k;
  v[2] *= k;
}

static float scalarprod(float *v1, float *v2) /*scalar product */
{
  return (v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]);
}

static void torusaxis(int i, int j,
                      float *uij) /*torus axis unit vector (aj-ai) */
{
  vdiff(&a[3 * j], &a[3 * i], uij);
  vtimk(uij, 1. / dist(i, j));
}

static void toruscenter(int i, int j, float *tij) /*torus center */
{
  float c;

  c = (ar[i] * ar[i] - ar[j] * ar[j]) / (dist2(i, j));

  tij[0] = 0.5 * (a[3 * i] + a[3 * j] + (a[3 * j] - a[3 * i]) * c);
  tij[1] =
      0.5 * (a[3 * i + 1] + a[3 * j + 1] + (a[3 * j + 1] - a[3 * i + 1]) * c);
  tij[2] =
      0.5 * (a[3 * i + 2] + a[3 * j + 2] + (a[3 * j + 2] - a[3 * i + 2]) * c);
}

static float trad2(int i, int j) /*square of torus radius */
{
  float d2;
  d2 = dist2(i, j);
  if (ar[i] == ar[j]) return (0.25 * ((ar[i] + ar[j]) * (ar[i] + ar[j]) - d2));
  return (0.25 * (((ar[i] + ar[j]) * (ar[i] + ar[j]) - d2) *
                  (d2 - (ar[i] - ar[j]) * (ar[i] - ar[j])) / d2));
}

static float torusrad(int i, int j) /*torus radius */
{
  return (sqrt(trad2(i, j)));
}

#if defined(US_SURFRACER_COMPUTE_EXTRAS)
static float cosba(int i, int j, int k) /*cosine of base triangle angle */
{
  float uij[3], uik[3];

  torusaxis(i, j, uij);
  torusaxis(i, k, uik);

  return (scalarprod(uij, uik));
}

static float baseangle(int i, int j, int k) /*base triangle angle */
{
  float uij[3], uik[3], sp;

  torusaxis(i, j, uij);
  torusaxis(i, k, uik);

  sp = scalarprod(uij, uik);

  if (sp >= 1.)
    return (0.);
  else if (sp <= -1.)
    return (M_PI);

  else
    return (acos(sp));
}
#endif

static void vectorprod(float *v1, float *v2, float *vp) /*vector product */
{
  vp[0] = v1[1] * v2[2] - v1[2] * v2[1];
  vp[1] = v1[2] * v2[0] - v1[0] * v2[2];
  vp[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

#if defined(US_SURFRACER_COMPUTE_EXTRAS)
static void basenorm(int i, int j, int k,
                     float *uijk) /*base plane normal vector */
{
  float uij[3], uik[3];
  float cb;

  torusaxis(i, j, uij);
  torusaxis(i, k, uik);

  vectorprod(uij, uik, uijk);
  cb = scalarprod(uij, uik);

  vtimk(uijk, 1. / sqrt(1. - cb * cb));
}

static void torbpoint(int i, int j, int k,
                      float *utb) /*torus basepoint unit vector */
{
  float uij[3], uijk[3];

  torusaxis(i, j, uij);
  basenorm(i, j, k, uijk);

  vectorprod(uijk, uij, utb);
}

static void bpoint(int i, int j, int k, float *bijk) /*base point */
{
  float tij[3], utb[3], uik[3], tik[3], uij[3];
  float c, cb;

  torusaxis(i, j, uij);
  torusaxis(i, k, uik);
  toruscenter(i, j, tij);
  toruscenter(i, k, tik);

  torbpoint(i, j, k, utb);
  vdiff(tik, tij, bijk);

  cb = scalarprod(uij, uik);

  c = scalarprod(uik, bijk) / sqrt(1. - cb * cb);

  vtimk(utb, c);
  vsum(tij, utb, bijk);
}
#endif

static float circdispl(
    int i,
    int j) /*contact circle displacement divided by (ar[i]-prober)/ar[i] */
{
  float tij[3],
      diff[3]; /*no need to do this multiplication because this factor */
  /*cancels out when dc is divided by rc */
  /*in the saddle width angle calculation */
  toruscenter(i, j, tij);
  vdiff(tij, &a[3 * i], diff);

  return (sqrt(scalarprod(diff, diff)));
}

#if defined(US_SURFRACER_COMPUTE_EXTRAS)
static float probeh2(int i, int j, int k) /*square of probe height */
{
  float bijk[3];

  bpoint(i, j, k, bijk);
  vdiff(bijk, &a[3 * i], bijk);

  return (ar[i] * ar[i] - scalarprod(bijk, bijk));
}

static void probepos(
    int i, int j, int k, float *pijk1,
    float *pijk2) /*two probe positions when it has 0 degrees of freedom */
{
  float bijk[3], uijk[3], hijk;

  bpoint(i, j, k, bijk);
  basenorm(i, j, k, uijk);
  hijk = sqrt(probeh2(i, j, k));

  vtimk(uijk, hijk);

  vsum(bijk, uijk, pijk1);
  vdiff(bijk, uijk, pijk2);
}
#endif

static void cunnvec(
    int i, int j, float *pijk,
    float *nijk) /*UNNORMALIZED concave arc plane normal vector */
{
  float tij[3], uij[3], v[3];

  toruscenter(i, j, tij);
  torusaxis(
      i, j,
      uij); /*nijk points in the right direction (surface is on the left) */

  vdiff(pijk, tij, v);
  vectorprod(v, uij, nijk);
}

static float phis(int i, int j, int nvert1, int nvert2) /*saddle wrap angle */
{
  float uij[3], prod[3], nijk[3], nijl[3];
  float check, d, scp, answer;

  torusaxis(i, j,
            uij); /* uij always points to the inaccessible area of atom i */
  cunnvec(i, j, &ver[nvert1 * 3], nijk); /*normalized vectors now */
  d = sqrt(scalarprod(nijk, nijk));
  vtimk(nijk, 1. / d);
  cunnvec(i, j, &ver[nvert2 * 3], nijl);
  d = sqrt(scalarprod(nijl, nijl));
  vtimk(nijl, 1. / d);

  scp = scalarprod(nijk, nijl);
  if (scp >= 1.)
    answer = 0.;
  else if (scp <= -1.)
    answer = (float)M_PI;
  else
    answer = acos(scp);

  if (answer != M_PI && answer != 0.) {
    vectorprod(nijk, nijl, prod);
    check = scalarprod(prod, uij);
    if (check > 0) /*corrected Connolly's error */
      return (2. * M_PI - answer);
  }
  return (answer);
}

static float thetasi(int i, int j) /*saddle width angle- here torusrad taken
                                      because of the cancellation */
{
  return (atan(circdispl(i, j) / torusrad(i, j)));
}

static float betav(int i, int j, int k, int nvert) /*concave triangle angle */
{
  float nijk[3], nikj[3];
  float mn1, mn2, sp;

  cunnvec(i, j, &ver[nvert * 3], nijk);
  cunnvec(i, k, &ver[nvert * 3], nikj);
  mn1 = sqrt(scalarprod(nijk, nijk));
  mn2 = sqrt(scalarprod(nikj, nikj));

  vtimk(nijk, 1. / mn1);
  vtimk(nikj, 1. / mn2);

  sp = scalarprod(nijk, nikj);
  if (sp >= 1.)
    return (0.);
  else if (sp <= -1.)
    return ((float)M_PI);
  else

    return (acos(sp));
}

static void concave(
    int nvert,
    int natom) /*calculating the concave part of the molecular surface */
{
  float b;

  b = betav(verat[nvert * 3], verat[nvert * 3 + 1], verat[nvert * 3 + 2],
            nvert); /*summing up the contribution from the three angles */
  b += betav(verat[nvert * 3 + 1], verat[nvert * 3], verat[nvert * 3 + 2],
             nvert);
  b += betav(verat[nvert * 3 + 2], verat[nvert * 3], verat[nvert * 3 + 1],
             nvert);

  molarea[natom] +=
      prober * prober * (b - M_PI) /
      3.; /*distributing it equally among the 3 participating atoms */

  if (calcmode == 3) curvat[natom] += (-2. / 3.) * prober * (b - M_PI);
}

static void saddle(int nedge, int natom,
                   int atom2) /*saddle part of the molecular surface; nedge is
                                 the edge number */
{
  float ratio, phi, rij, theti, thetj, theta1, a1, a2, si, sj;

  if (edge[nedge * 2] == -2)
    phi = 2. * (float)M_PI; /*it's a circle */
  else
    phi = phis(natom, atom2, edge[nedge * 2 + 1],
               edge[nedge * 2]); /*it's an arc */

  rij = torusrad(natom, atom2);
  theti = thetasi(natom, atom2);
  thetj = thetasi(atom2, natom);
  si = sin(theti);
  sj = sin(thetj);

  if (prober <= rij) {
    a1 = phi * (rij * prober * theti - prober * prober * si);
    molarea[natom] += a1;

    if (calcmode == 3) curvat[natom] += phi * (2. * prober * si - rij * theti);

    a2 = phi * (rij * prober * thetj - prober * prober * sj);
    molarea[atom2] += a2;

    if (calcmode == 3) curvat[atom2] += phi * (2. * prober * sj - rij * thetj);
  }

  else {
    ratio = rij / prober; /*self-overlapping surface */
    if (ratio >= 1.)
      theta1 = 0.;
    else if (ratio <= -1.)
      theta1 = (float)M_PI;
    else
      theta1 = acos(ratio);

    a1 = phi * (prober * prober * (2. * sin(theta1) - si) -
                prober * rij * (2. * theta1 - theti));
    molarea[natom] += a1;

    if (calcmode == 3)
      curvat[natom] += phi * (prober * (-4. * sin(theta1) + 2. * si) +
                              rij * (2. * theta1 - theti));

    a2 = phi * (prober * prober * (2. * sin(theta1) - sj) -
                prober * rij * (2. * theta1 - thetj));
    molarea[atom2] += a2;

    if (calcmode == 3)
      curvat[atom2] += phi * (prober * (-4. * sin(theta1) + 2. * sj) +
                              rij * (2. * theta1 - thetj));
  }
}

/*this function checks whether point *point is internal or external to cycle*/
/*nver is the maximal index of the cycle array (all ordered vertices of the
 * cycle)*/
/*function returns 1 if the point is internal and 0 if the point is external*/
/*cycle should not contain degenerate vertices!*/
/*j is the number of the atom that contains the cycle*/
static int checkcycle(int j, int nver, float cycle[][3], float *point) {
  int i;
  float e3[3], proj[50][3], v[3],
      diff[50][3]; /*LIMIT: maximum of 50 vertices per cycle */
  float sp, ssp, c, sumangle;

  if (nver == 1)
    return (1); /*here cycle has nver+1=2 edges therefore the point is
                   automatically internal */

  /*if nver>1 then the following is executed */
  vdiff(point, &a[j * 3], e3);
  vtimk(e3, 1. / ar[j]); /*point becomes the "north pole" and e3 is a unit
                            vector from the center of the atom to it */
  /*pointing "north" */
  sumangle = 0; /*the sum of the external angles of the projection */

  for (i = 0; i <= nver;
       i++) /*building the spherical projection of the cycle */
  {         /*recording it in proj[50][3] */
    vdiff(cycle[i], point, v);
    vtimk(v,
          1. / sqrt(dis2(
                   point,
                   cycle[i]))); /*unit vector from point to vertex cycle[i] */
    c = -2. * ar[j] / scalarprod(e3, v);
    vtimk(v, c); /*now v points from the point to the vertex projection */
    vsum(point, v, proj[i]); /* vector to the projection from (0,0,0) */
  }
  /*all projections are recorded */

  /*connecting projections by the vectors and recording the polygon into
   * diff[][] */
  for (i = 0; i <= nver - 1; i++) {
    vdiff(proj[i + 1], proj[i], diff[i]);
    c = sqrt(scalarprod(diff[i], diff[i]));
    vtimk(diff[i], 1. / c); /*these are unit vectors */
  }
  vdiff(proj[0], proj[nver],
        diff[nver]); /*making the last connection-now the polygon is built */
  c = sqrt(scalarprod(diff[nver], diff[nver]));
  vtimk(diff[nver], 1. / c); /*all are unit vectors */

  /*summing the external angles */
  for (i = 0; i <= nver - 1; i++) {
    vectorprod(diff[i], diff[i + 1], v);
    ssp = scalarprod(diff[i], diff[i + 1]);
    if (ssp >= 1.)
      c = 0;
    else if (ssp <= -1.)
      c = (float)M_PI;
    else
      c = acos(ssp);

    if (scalarprod(v, e3) > 0)
      sumangle += c;
    else
      sumangle -= c;
  }
  vectorprod(diff[nver], diff[0], v); /*the last one */
  sp = scalarprod(diff[nver], diff[0]);

  if (sp >= 1.)
    c = 0.;
  else if (sp <= -1.)
    c = (float)M_PI;
  else
    c = acos(sp);

  if (scalarprod(v, e3) > 0)
    sumangle += c;
  else
    sumangle -= c;

  if (sumangle > 0) return (1); /*point is internal */

  return (0); /*point is external */
}

/*searchcycle() finds all cycles that surround the same surface as cycle1,loads
 * their numbers in *hits*/
/*internal-external relationships between cycles are loaded in the
 * respect[][MAXCYCLES] array*/
/*returns the number of these cycles*/
static int searchcycle(int ncycles, int cycle1, int respect[][MAXCYCLES],
                       int *hits) /*LIMIT: max of MAXCYCLES cycles per atom */
{
  int i, j, nhit = 0;

  for (i = 0; i < MAXCYCLES; i++) /*initializing hits */
    hits[i] = -1;

  for (i = 0; i <= ncycles - 1; i++) {
    if (i != cycle1) /*do not analyze the same cycle */
    {
      if (respect[cycle1][i] &&
          respect[i][cycle1]) /*they have each other in their interior */
      {
        for (j = 0; j <= ncycles - 1; j++) {
          if (j != i && j != cycle1) /*checking the other cycles */
          {
            if (respect[cycle1][j] &&
                respect[i][j]) /*cycle j is in the interior of cycles cycle1 and
                                  i */
            {
              if (respect[j][cycle1] &&
                  respect[j]
                         [i]) /*cycles cycle1 and i are in the interior of j */
              {
                continue;
              } else {
                break;
              }
            }
          }
        }
        if (j == ncycles) /*all cycles that are in the interior of cycle1 and i
                             have cycle 1 in their interior */
        {
          hits[nhit] = i;
          nhit++;
        }
      }
    }
  }
  return (nhit);
}

/*potoedge() forms an edge starting from a vertex*/
/*at this point degenerate vertices should be resolved*/
/*natom is the atom containing the edge*/
/*verte[v0] is the starting vertex number- coordinates of it are important, but
 * the atoms are not because of possible*/
/*degeneracy discussed above*/
/*verte[v1] is the next vertex forming the edge with v0*/
/*RETURNS the number of atom shares the edge with natom*/
/*verte are all vertices on natom (degeneracy is still present here)*/
/*nverte is the total number of this vertices*/
/*IMPORTANT:v0 and v1 are indices of verte and not of ver!!!!!*/
static int potoedge(int natom, int v0, int *v1, int *verte, int nverte) {
  int i, j, degen[MAXCYCLES], ndegen, degatom[MAXCYCLES],
      ndegat; /*degen are the vertices that are degenerate with v0 including v0
               */
  int corratom, k; /*degatom are the atoms forming the degenerate vertices */
  float e1[3], e2[3], e3[3], vect[MAXCYCLES][3], tni[3],
      check[3]; /*vect are unit vectors corresponding to degatoms */
  float sssp, sp, ssp, ang, minangle, d;
  float finvect[3], vp[3]; /*the correct unit vector from vect array */
  int vercom[MAXCYCLES + 10], vc;
  dbg("potoedge 0");
  if (verte[v0] == -1) {
    printf("protoedge entered with verte[v0]==-1!!\n");
    fflush(stdout);
  }
  degen[0] = v0; /*finding all degenerate vertexes */
  for (i = 0, ndegen = 1; i <= nverte - 1; i++)
    if (i != v0 && verte[i] != -1) /*looking for vertices that coincide with v0
                                      and not eliminated yet */
      if (fabs(ver[verte[v0] * 3] - ver[verte[i] * 3]) < 1.e-6 &&
          fabs(ver[verte[v0] * 3 + 1] - ver[verte[i] * 3 + 1]) < 1.e-6 &&
          fabs(ver[verte[v0] * 3 + 2] - ver[verte[i] * 3 + 2]) <
              1.e-6) /*version 3.0 change. Precise equality replaced with the
                        interval */
      {
        degen[ndegen] = i; /*i are also indices of verte */
        ndegen++;
        veflag[verte[i]] = '1';
        /*marking off all degenerate vertices here */
      } /*x y and z coincide */
  /*all degenerate vertices are recorded now */
  /*ndegen=1 means that v0 is NOT degenerate */
  dbg("potoedge 1");

  vdiff(&ver[verte[v0] * 3], &a[natom * 3], e1);
  vtimk(e1, 1. / ar[natom]); /*e1 is a unit vector from the center of atom atomn
                                to the vertex verte[v0] */

  dbg("potoedge 2");
  ndegat = 0;
  // printf("v0 %d ", v0); fflush(stdout);
  // printf("verte[v0] %d\n", verte[v0]); fflush(stdout);
  if (verat[verte[v0] * 3] != natom) {
    degatom[ndegat] = verat[verte[v0] * 3];
    ndegat++;
  }

  if (verat[verte[v0] * 3 + 1] != natom) {
    degatom[ndegat] = verat[verte[v0] * 3 + 1];
    ndegat++;
  }

  if (verat[verte[v0] * 3 + 2] != natom) {
    degatom[ndegat] = verat[verte[v0] * 3 + 2];
    ndegat++; /*recording the first 2 atoms */
  }
  /*ndegat>=2 (2 if nondegenerate) */
  dbg("potoedge 3");

  for (i = 1; i <= ndegen - 1;
       i++) /*recording the atoms forming the degenerate vertexes */
  {
    dbg("potoedge 3");
    if (verat[verte[degen[i]] * 3] != natom) {
      for (j = 0; j <= ndegat - 1; j++) /*making sure the atoms do not repeat */
        if (verat[verte[degen[i]] * 3] == degatom[j]) break;

      if (j == ndegat) /*found a new one */
      {
        degatom[ndegat] = verat[verte[degen[i]] * 3];
        ndegat++;
      }
    }

    dbg("potoedge 4");
    if (verat[verte[degen[i]] * 3 + 1] != natom) {
      for (j = 0; j <= ndegat - 1; j++) /*making sure the atoms do not repeat */
        if (verat[verte[degen[i]] * 3 + 1] == degatom[j]) break;

      if (j == ndegat) /*found a new one */
      {
        degatom[ndegat] = verat[verte[degen[i]] * 3 + 1];
        ndegat++;
      }
    }
    dbg("potoedge 5");

    if (verat[verte[degen[i]] * 3 + 2] != natom) {
      for (j = 0; j <= ndegat - 1; j++) /*making sure the atoms do not repeat */
        if (verat[verte[degen[i]] * 3 + 2] == degatom[j]) break;

      if (j == ndegat) /*found a new one */
      {
        degatom[ndegat] = verat[verte[degen[i]] * 3 + 2];
        ndegat++;
      }
    }
  }
  /*loaded all possibilities for the atom number to return by this function */

  dbg("potoedge 6");
  //  printf("ndegat %d\n", ndegat);
  for (i = 0; i <= ndegat - 1; i++) /*  loading vect */
  {
    dbg("potoedge 6.1");

    cunnvec(natom, degatom[i], &ver[verte[v0] * 3], vect[i]);
    dbg("potoedge 6.2");
    d = sqrt(scalarprod(vect[i], vect[i]));
    dbg("potoedge 6.3");
    vtimk(vect[i], 1. / d);
    dbg("potoedge 6.4");
  }
  /*vect are a correctly oriented unit vectors we were looking for */
  /*all oriented vectors are loaded here, now we can find the correct next atom
   * (corratom) */
  dbg("potoedge 7");

  corratom = degatom[0]; /*arbitrary start */
  finvect[0] = vect[0][0];
  finvect[1] = vect[0][1];
  finvect[2] = vect[0][2];

  for (i = 1; i <= ndegat - 1; i++) {
    ssp = scalarprod(finvect, vect[i]);

    if (ssp < 1 && ssp > -1) {
      vectorprod(finvect, vect[i], check);
      sp = scalarprod(check, e1);

      if (sp > 0) /*looking for a maximal angle less than pi */
      {
        finvect[0] = vect[i][0];
        finvect[1] = vect[i][1];
        finvect[2] = vect[i][2];

        corratom = degatom[i];
      }
    }
  }
  dbg("potoedge 8");
  /*The correct atom is found and the unit vector in the right direction is
   * finvect */
  /*vertex degeneracy problem is solved at this point */

  /*for the future: here is a good place to erase irrelevant degenerate vertices
   */

  /*loading vertices having common atoms atomn and corratom - some may be
   * degenerate. Do not inclute the vertices degenerate with the current one */
  for (i = 0, vc = 0; i <= nverte - 1; i++)
    if (i != v0 && verte[i] != -1) {
      for (k = 1; k <= ndegen - 1; k++)
        if (i == degen[k]) break;

      if (k == ndegen) /*the vertex is not degenerate to the current vertex */
        if (verat[verte[i] * 3] == corratom ||
            verat[verte[i] * 3 + 1] == corratom ||
            verat[verte[i] * 3 + 2] == corratom) {
          vercom[vc] = i;
          vc++;
        } /*here again vercom contains indices of verte */
    }

  dbg("potoedge 9");
  if (vc >= 2) { /*finding the next vertex */
    minangle = (float)(3. * M_PI);
    *v1 = vercom[0];

    toruscenter(natom, corratom, tni);
    torusaxis(natom, corratom,
              e2); /*e2 is a unit vector pointing to the buried part */

    vdiff(&ver[verte[v0] * 3], tni, e3);
    d = sqrt(scalarprod(e3, e3));
    vtimk(e3, 1. / d); /*unit vector e3  points from torus center to vertex
                          given by v0 */

    for (i = 0; i <= vc - 1; i++) {
      vdiff(&ver[verte[vercom[i]] * 3], tni, check);

      vtimk(check,
            1. / d); /*check is a unit vector from torus center to a vertex */

      sssp = scalarprod(check, e3);
      if (sssp >= 1.)
        ang = 0.;
      else if (sssp <= -1.)
        ang = (float)M_PI;
      else
        ang = acos(sssp);

      if (ang != 0. && ang != M_PI) /*0 and pi are not considered here */
                                    /*not zero or pi */
      {
        vectorprod(check, e3, vp);
        sp = scalarprod(vp, e2);

        if (sp <
            0) /*the angle is more than 180 counting counterclockwise from e3 */
          ang = 2. * M_PI - ang;
      }
      /*finished calculating the angle */

      if (ang < minangle /* && ang!=0 */) {
        minangle = ang;
        *v1 = vercom[i];
      }
    }
  } else
    *v1 = vercom[0]; /*this is the correct next vertex, however it can be
                        degenerate */
  /*it has correct coordinates but not necessarily atom numbers */
  dbg("potoedge 10");

  return (corratom); /*returning the atom forming the correct edge with natom */
}

/*convexarea() returns the convex area of surface surrounded by cycles pointed
 * by hits*/
static float convexarea(int natom, int cycles[][MAXCYCLES],
                        int common[][MAXCYCLES], int *hits, int nhits,
                        int *nvincyc) {
  int i, j;
  float area = 0.;
  /*nhits is number of cycles */
  for (i = 0; i <= nhits - 1; i++) {
    if (cycles[hits[i]][0] == -2) /*circle */
      area += 2. * M_PI * sin(thetasi(natom, common[hits[i]][0]));
    else /*cycles consisting of edges */
    {
      for (j = 0; j <= nvincyc[hits[i]] - 2; j++)
        area += phis(natom, common[hits[i]][j], cycles[hits[i]][j],
                     cycles[hits[i]][j + 1]) *
                sin(thetasi(natom, common[hits[i]][j]));

      /*the last element */ /* corrected the sign error in Connolly's paper */
      area += phis(natom, common[hits[i]][nvincyc[hits[i]] - 1],
                   cycles[hits[i]][nvincyc[hits[i]] - 1], cycles[hits[i]][0]) *
              sin(thetasi(natom, common[hits[i]][nvincyc[hits[i]] - 1]));

      area -= betav(natom, common[hits[i]][nvincyc[hits[i]] - 1],
                    common[hits[i]][0],
                    cycles[hits[i]][0]); /*the first vertex element */
      for (j = 1; j <= nvincyc[hits[i]] - 1; j++)
        area -= betav(natom, common[hits[i]][j - 1], common[hits[i]][j],
                      cycles[hits[i]][j]);
    }
  }

  return (ar[natom] * ar[natom] * (2. * M_PI * (2. - nhits) + area));
}

/*managedge records and erases edge records such that each atom's area is
 * counted only once*/
/*the arguments are exactly the same as in convexarea() above*/
static void managedge(int natom, int cycles[][MAXCYCLES],
                      int common[][MAXCYCLES], int *hits, int nhits,
                      int *nvincyc) {
  int i, j, l, p;
  dbg("me 1");

  for (i = 0; i <= nhits - 1; i++) {
    dbg("me 2");
    if (cycles[hits[i]][0] == -2) /*circle */
    {
      dbg("me 3");
      for (j = 0; j <= ned - 1; j++)
        if (edge[j * 2] == -2 && edge[j * 2 + 1] == cycles[hits[i]][1]) {
          edge[j * 2] = -1;
          break; /*found exactly this one- erase it!!! */
        }

      if (j == ned) /*this is a brand new one */
      {
        for (l = 0; l <= ned - 1 && edge[l * 2] != -1; l++) {
        }; /*breaks when encounters an empty spot */

        edge[l * 2] = -2; /*records cycle papameters into the empty spot */
        edge[l * 2 + 1] = cycles[hits[i]][1];
        edgeatom[l] = common[hits[i]][0]; /*not analyzed atom goes first */

        if (calcmode > 1)
          saddle(l, natom, edgeatom[l]); /*calculating the saddle area */

        veflag[nv + cycles[hits[i]][1]] =
            '1'; /*marking off all circles of the outer surface */
      }
    } else /* a cycle is formed of vertex edges */
    {
      dbg("me 4");

      for (j = 0; j <= nvincyc[hits[i]] - 2;
           j++) /*checking all edges but the last one */
      {
        dbg("me 4.1");
        for (l = 0; l <= ned - 1; l++)
          if ((cycles[hits[i]][j] == edge[l * 2] ||
               cycles[hits[i]][j + 1] == edge[l * 2 + 1]) &&
              edgeatom[l] == natom) /*found it- the  vertexes (in the right
                                       order!) are the same */
          {
            dbg("me 4.2");
            if (calcmode > 1) concave(edge[l * 2 + 1], natom);
            edge[l * 2] = -1;
            edge[l * 2 + 1] = -1; /*erasing it */
            break;
          }

        dbg("me 4.3");
        if (l == ned) /*did not find it */
        {
          dbg("me 4.4");
          for (p = 0; p < ned && edge[p * 2] != -1; p++) {
          }; /*found an empty spot */  // emre this is where it fails
          dbg("me 4.5");

          edge[p * 2] =
              cycles[hits[i]][j + 1]; /*recording in the reverse order */
          edge[p * 2 + 1] = cycles[hits[i]][j];
          edgeatom[p] = common[hits[i]][j];

          veflag[edge[p * 2]] =
              '1'; /*marking off a vertex of the outer surface */

          if (calcmode > 1) {
            saddle(p, natom, edgeatom[p]); /*area calculation */
            concave(edge[p * 2],
                    natom); /*calculating the concave molecular surface and
                               writing it into molarea */
          }
        }
        dbg("me 4.6");
      }

      dbg("me 5");
      /*the last element */
      for (l = 0; l <= ned - 1; l++)
        if ((cycles[hits[i]][nvincyc[hits[i]] - 1] == edge[l * 2] ||
             cycles[hits[i]][0] == edge[l * 2 + 1]) &&
            edgeatom[l] == natom) {
          if (calcmode > 1) concave(edge[l * 2 + 1], natom);
          edge[l * 2] = -1;
          edge[l * 2 + 1] = -1; /*found it- erase */

          break;
        }

      if (l == ned) /*did not find it */
      {
        dbg("me 6");
        for (p = 0; p < ned && edge[p * 2] != -1; p++) {
        }; /*found an empty spot */

        edge[p * 2] = cycles[hits[i]][0]; /*recording in the reverse order */
        edge[p * 2 + 1] = cycles[hits[i]][nvincyc[hits[i]] - 1];
        edgeatom[p] = common[hits[i]][nvincyc[hits[i]] - 1];

        veflag[edge[p * 2]] =
            '1'; /*marking off a vertex of the outer surface */

        if (calcmode > 1) {
          saddle(p, natom, edgeatom[p]);
          concave(edge[p * 2], natom); /*calculating the concave molecular
                                          surface and writing it into molarea */
        }
      }
      dbg("me 7");
    }
    dbg("me 8");
  }
  dbg("me 9");
}

/*buildcycles builds all cycles for natom-loads them into cycles[][]*/
/*returns the number of cycles*/
static int buildcycles(int natom, int cycles[][MAXCYCLES],
                       int common[][MAXCYCLES], int *nvincyc) {
  int verte[50], i, ii, j, nve, icycle, v0, v1, comatom, p, cycmin, pflag;
  float v00[3];
  dbg("bc0");
  for (i = 0; i <= 49; i++) verte[i] = -1; /*initializing verte[] with -1 */

  /*loading vertexes of atom number natom */
  dbg("bc1");

  for (i = 0, nve = 0; i <= nv - 1; i++) {
    //      printf("bc 1 i %d nve %d nv %d\n", i, nve, nv);
    if (verat[i * 3] > natom) break;
    if (verat[i * 3] == natom || verat[i * 3 + 1] == natom ||
        verat[i * 3 + 2] == natom) {
      verte[nve] = i;
      nve++;
      if (nve >= 50) {
        printf("sr: buildcycles error limit 1 nve=%d\n", nve);
        fflush(stdout);
        exit(-1);
      }
    }
  }
  // printf("sr: NOTICE nve=%d\n", nve);
  // fflush(stdout);

  dbg("bc2");
  /*loading cycles */
  /*circles are considered in this part */

  icycle = 0;
  for (i = 0; i <= ncircle - 1; i++) {
    if (cirat[i * 2] > natom) break;
    if (cirat[i * 2] == natom ||
        cirat[i * 2 + 1] == natom) /*i-th circle is on natom */
    {
      cycles[icycle][0] =
          -2; /*  -2 in the 0th element means it's a circle !!!!!  */
      cycles[icycle][1] = i; /*load circle number in the first element */

      nvincyc[icycle] = 1; /*set to 1 if the cycle is a circle */

      if (cirat[i * 2] != natom)
        common[icycle][0] = cirat[i * 2];
      else
        common[icycle][0] =
            cirat[i * 2 + 1]; /*common has the other atom in the 0-th element */

      icycle++; /*degenerate cycles are not chemically possible and extremely
                   unlikely */
    }
  }
  // printf("bc 2 icycle %d ncircle %d\n", icycle, ncircle);

  dbg("bc3");
  /*circles are loaded- now it's time to load more complex cycles */

  while (1) /*break inside */
  {
    dbg("bc4");
    for (i = 0; i <= nve - 1; i++) /*going over all vertices of natom */
      if (verte[i] != -1) break;   /*found an untreated vertex */

    if (i == nve)
      break; /*all vertices are taken into account- break the while loop */

    /*so we are still here- that means verte[i] is the vertex that will start a
     * brand new cycle for us!!! */
    if (verte[i] == -1) {
      printf("ERROR dc4!!\n");
    }

    v0 = i; /*we will have its number in v0 at first */
    v00[0] = ver[verte[v0] * 3];
    v00[1] = ver[verte[v0] * 3 + 1];
    v00[2] = ver[verte[v0] * 3 + 2];
    v1 = v0;
    j = 0;

    dbg("bc5");
    nvincyc[icycle] = 0; /*initialize */
    if (verte[v0] == -1) {
      printf("ERROR bc5!!\n");
    }

    do {
      dbg("bc5.1");
      nvincyc[icycle]++; /*count the number of vertices in the cycle */
      dbg("bc5.2");

      v0 = v1; /*the initial vertex of an edge is v0 */
      dbg("bc5.3");
      cycles[icycle][j] = verte[v1];
      dbg("bc5.4");
      if (verte[v0] == -1) {
        printf("ERROR bc5.4!!\n");
      }
      comatom = potoedge(natom, v0, &v1, verte, nve);
      dbg("bc5.5");
      common[icycle][j] = comatom;
      dbg("bc5.6");
      j++;
      dbg("bc6");
      //    printf("v1 = %d\n", v1); fflush(stdout);
    } while (j < MAXCYCLES && v1 >= 0 && verte[v1] != -1 &&
             (fabs(ver[verte[v1] * 3] - v00[0]) >= 1.e-6 ||
              fabs(ver[verte[v1] * 3 + 1] - v00[1]) >= 1.e-6 ||
              fabs(ver[verte[v1] * 3 + 2] - v00[2]) >= 1.e-6));
    // printf("after bc6 j = %d\n", j);
    dbg("bc6.1");
    /*stop when the coordinates coincide- the cycle is then closed */
    /*comparing coordinates in case of degeneracy */
    for (p = 0; p <= j - 1; p++)
      for (ii = 0; ii <= nve - 1; ii++)
        if (verte[ii] != -1 && cycles[icycle][p] == verte[ii]) verte[ii] = -1;
    dbg("bc7");

    for (p = 0; p <= j - 1; p++) {
      dbg("bc8");
      cycmin = 0;
      pflag = 0;
      for (ii = 0; ii <= nve - 1; ii++)
        if (cycles[icycle][p] != verte[ii] && verte[ii] != -1)  // emre?
          if (fabs(ver[cycles[icycle][p] * 3] - ver[verte[ii] * 3]) < 1.e-6 &&
              fabs(ver[cycles[icycle][p] * 3 + 1] - ver[verte[ii] * 3 + 1]) <
                  1.e-6 &&
              fabs(ver[cycles[icycle][p] * 3 + 2] - ver[verte[ii] * 3 + 2]) <
                  1.e-6) {
            if (p != 0) {
              if (((verte[ii] < cycmin && pflag == 1) || (pflag == 0)) &&
                  (verat[3 * verte[ii]] == common[icycle][p - 1] ||
                   verat[3 * verte[ii] + 1] == common[icycle][p - 1] ||
                   verat[3 * verte[ii] + 2] == common[icycle][p - 1])) {
                veflag[cycles[icycle][p]] = '1';
                cycles[icycle][p] = verte[ii];
                cycmin = verte[ii];
                pflag = 1;
              }
            } else if (((verte[ii] < cycmin && pflag == 1) || pflag == 0) &&
                       (verat[3 * verte[ii]] == common[icycle][j - 1] ||
                        verat[3 * verte[ii] + 1] == common[icycle][j - 1] ||
                        verat[3 * verte[ii] + 2] == common[icycle][j - 1])) {
              veflag[cycles[icycle][p]] = '1';
              cycles[icycle][p] = verte[ii];
              cycmin = verte[ii];
              pflag = 1;
            }
            verte[ii] =
                -1; /*this will eliminate possible degenerate ones as well */
          }
    } /*again possible degeneracy causes coordinate comparison here */
    icycle++;
    dbg("bc9");
  }
  dbg("bc10");

  return (icycle);
}

/*findcycle() finds the first cycle that 's a part of the surface to be
 * calculated*/
/*from the edge number*/
/*it returns the number of that cycle in the cycles[][] array*/
static int find1cyc(int nedge, int cycles[][MAXCYCLES], int icycle,
                    int *nvincyc) /*icycle is the total number of cycles */
{
  int i, j;

  if (edge[nedge * 2] == -2) /*it's a circle */
    for (i = 0; i <= icycle - 1; i++) {
      if (cycles[i][0] == -2)                    /*the cycle is also a circle */
        if (edge[nedge * 2 + 1] == cycles[i][1]) /*it's the one */
          return (i);
    }

  /*got to here- that means it's a vertex-type edge */

  for (i = 0; i <= icycle - 1; i++) {
    if (cycles[i][0] == -2)
      continue; /*make sure it's not a circle */
    else {
      for (j = 0; j <= nvincyc[i] - 1; j++)
        if (cycles[i][j] == edge[nedge * 2]) /*cycle[i] and the edge share the
                                                same vertex!!- found it */
          break;
    }
    if (j < nvincyc[i]) /*break occured */
      return (i);
  }
  return (0);
}

#if defined(US_SURFRACER_COMPUTE_EXTRAS)
/*this is same as above, only using a vertex or a circle number as a search
 * clue*/
/*fi1cyc() is used in the cavity calculation part*/
static int fi1cyc(int i, int cycles[][MAXCYCLES], int icycle, int *nvincyc) {
  int j, k;

  if (i >= nv) /*it's circle number i-nv */
    for (j = 0; j <= icycle - 1; j++) {
      if (cycles[j][0] == -2)         /*the cycle is also a circle */
        if ((i - nv) == cycles[j][1]) /*it's the one */
          return (j);
    }

  /*got to here- that means it's vertex number i */

  for (j = 0; j <= icycle - 1; j++) {
    if (cycles[j][0] == -2)
      continue; /*make sure it's not a circle */
    else {
      for (k = 0; k <= nvincyc[j] - 1; k++)
        if (cycles[j][k] == i) /*cycle[j] contains vertex i!!- found it */
          break;
    }
    if (k < nvincyc[j]) /*break occured */
      return (j);
  }
  return (0);
}
#endif

/*loadresp() loads internal-external relationships into respect[][]*/
/*the notation is the same as above*/
static void loadresp(int natom, int icycle, int cycles[][MAXCYCLES],
                     int respect[][MAXCYCLES], int *nvincyc) {
  int i, j, k;
  float verte[MAXCYCLES][3];

  if (icycle == 1) /*only one cycle- no relationships to figure out */
  {
    respect[0][0] = 0;
    return;
  }

  for (i = 0; i <= icycle - 1; i++) {
    respect[i][i] = 0; /*a cycle is always external with respect to itself */

    if (cycles[i][0] == -2) /*a cycle is a circle */
    {
      for (j = 0; j <= icycle - 1; j++)
        if (j != i)
          respect[i][j] = 1; /*any cycle is internal with respect to a circle */
    }

    else /*it's not a circle */
    {
      for (j = 0; j <= nvincyc[i] - 1; j++) {
        verte[j][0] = ver[cycles[i][j] * 3]; /*loading cycle[i] into verte */
        verte[j][1] = ver[cycles[i][j] * 3 + 1];
        verte[j][2] = ver[cycles[i][j] * 3 + 2];
      }

      for (k = 0; k <= icycle - 1; k++) {
        if (k != i) /*checking the cycles k other than i itself */
        {
          if (j == 2)
            respect[i][k] = 1; /*if cycle i consists of 2 vertices any other one
                                  is internal to it */
          else {
            if (cycles[k][0] == -2) /*cycle k is a circle */
              respect[i][k] = checkcycle(
                  natom, j - 1, verte,
                  &cir[cycles[k][1] * 3]); /*check a point on the circle */
            else
              respect[i][k] = checkcycle(
                  natom, j - 1, verte,
                  &ver[cycles[k][0] * 3]); /*check the first vertex */
          }
        }
      }
    }
  }
}

static void free_alloced(void) {
  if (a) {
    free(a);
    a = (float *)0;
  }
  if (ar) {
    free(ar);
    ar = (float *)0;
  }
  if (ver) {
    free(ver);
    ver = (float *)0;
  }
  if (cir) {
    free(cir);
    cir = (float *)0;
  }
  if (molarea) {
    free(molarea);
    molarea = (float *)0;
  }
  if (curvat) {
    free(curvat);
    curvat = (float *)0;
  }

  if (visits) {
    free(visits);
    visits = (short *)0;
  }
  if (ve) {
    free(ve);
    ve = (float *)0;
  }
  if (ci) {
    free(ci);
    ci = (float *)0;
  }
  if (aarea) {
    free(aarea);
    aarea = (float *)0;
  }
  if (verat) {
    free(verat);
    verat = (int *)0;
  }
  if (cirat) {
    free(cirat);
    cirat = (int *)0;
  }
  if (edge) {
    free(edge);
    edge = (int *)0;
  }
  if (edgeatom) {
    free(edgeatom);
    edgeatom = (int *)0;
  }
  if (veat) {
    free(veat);
    veat = (int *)0;
  }
  if (atomcon) {
    free(atomcon);
    atomcon = (int *)0;
  }
  if (ciat) {
    free(ciat);
    ciat = (int *)0;
  }
  if (veflag) {
    free(veflag);
    veflag = (char *)0;
  }
}

int surfracer_main(float prober, vector<PDB_atom *> active_atoms, bool recheck,
                   QProgressBar *use_progress, QTextEdit *use_editor) {
  progress = use_progress;
  editor = use_editor;
  /* HANDLE hStdin; */
  // char str1[255];
  // char str[255];
  // char rad[5]="    ";
  // char pdbf[32] =
  // "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"; char
  // pr[15]="\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
  /* DWORD result; */
  /* COORD coord;    */

  // FILE *vdwradfile;
  // FILE *pdbfile;
  // FILE *asafile;
  // FILE *cavfile;
  // FILE *resfile;

  float uijk[3];
  float utb[3];
  float bijk[3];
  float c;
  float uconij[10000][3];
  float tconij[10000][3];
  float dist2ij[10000]; /*3.0 version change- 500 is changed to 10000 here */
  float cba;
  float sba;
  float rsum;
  float delta;
  float xmax;
  float xmin;
  float ymax;
  float ymin;
  float zmin;
  float ph2;
  float ve1[3];
  float ve2[3];
  float diff[3];
  float re[3];
  float z;
  float zmax;
  float highest[3];
  float verte[MAXCYCLES][3];
  // float polarea;
  // float nparea;
  float asar;
  int j;
  int i;
  int k;
  int ai;
  int aj;
  int ak;
  int nvk;
  int l;
  int bur1;
  int bur2;
  int atom0;
  int ncycle;
  int cycles[MAXCYCLES][MAXCYCLES];
  int common[MAXCYCLES][MAXCYCLES];
  int respect[MAXCYCLES][MAXCYCLES];
  int nhits;
  int hits[MAXCYCLES];
  int cyc1;
  int nvincyc[MAXCYCLES];

  // clock_t start1;
  // clock_t end1;
  // clock_t start2;
  // clock_t end2;

#if defined(US_SURFRACER_COMPUTE_EXTRAS)
  int cavn;
  float tused;
  float molnparea;
  float molpolarea;
  char acidname[4] = "\0\0\0";
  char vdwsetnumber;
  char atomname[5] = "\0\0\0\0";
  static char radi[5] = "    ", asa[6] = "     ";
  float bbnpasa = 0.;   // accessible surface areas of backbone and side-chains
  float bbpolasa = 0.;  // accessible surface areas of backbone and side-chains
  float scnpasa = 0.;   // accessible surface areas of backbone and side-chains
  float scpolasa = 0.;  // accessible surface areas of backbone and side-chains
  float scplus = 0.;    // accessible surface areas of backbone and side-chains
  float scminus = 0.;   // accessible surface areas of backbone and side-chains
  float bbnpmsa = 0.;   // molecular surface areas
  float bbpolmsa = 0.;  // molecular surface areas
  float scnpmsa = 0.;   // molecular surface areas
  float scpolmsa = 0.;  // molecular surface areas
  float scmplus = 0.;   // molecular surface areas
  float scmminus = 0.;  // molecular surface areas
#endif

  // nullify memory pointers
  a = (float *)0;
  ar = (float *)0;
  ver = (float *)0;
  cir = (float *)0;
  molarea = (float *)0;
  curvat = (float *)0;
  visits = (short *)0;
  ve = (float *)0;     // atomic coordinates
  ci = (float *)0;     // radii
  aarea = (float *)0;  // areas

  verat = (int *)0;
  cirat = (int *)0;
  edge = (int *)0;
  edgeatom = (int *)0;
  veat = (int *)0;
  atomcon = (int *)0;
  ciat = (int *)0; /*veat is external */

  veflag = (char *)0;

  // puts("sr3.0");

  // puts("Surface Racer 3.0 by Oleg Tsodikov");
  // puts("Integrated into UltraScan by E. Brookes");
  //  #define DEBUG

#if defined(DEBUG)
  for (unsigned int i = 0; i < active_atoms.size(); i++) {
    printf("active atom %d %s %s rad %f resseq %d\n", i,
           active_atoms[i]->name.toLatin1().data(),
           active_atoms[i]->resName.toLatin1().data(), active_atoms[i]->radius,
           active_atoms[i]->resSeq);
    fflush(stdout);
  }
#endif

  editor->append("Finding spatial groups\n");
  qApp->processEvents();
  int next_group = 1;
  for (unsigned int i = 0; i < active_atoms.size(); i++) {
    active_atoms[i]->group = 0;
  }
  float separation;
  vector<int> removed_groups;
  for (unsigned int i = 0; i < active_atoms.size() - 1; i++) {
    for (unsigned int j = i + 1; j < active_atoms.size(); j++) {
      if (!recheck) {
        separation =

            sqrt(pow(active_atoms[i]->coordinate.axis[0] -
                         active_atoms[j]->coordinate.axis[0],
                     2) +
                 pow(active_atoms[i]->coordinate.axis[1] -
                         active_atoms[j]->coordinate.axis[1],
                     2) +
                 pow(active_atoms[i]->coordinate.axis[2] -
                         active_atoms[j]->coordinate.axis[2],
                     2)) -
            active_atoms[i]->radius - active_atoms[j]->radius;
      } else {
        separation =

            sqrt(pow(active_atoms[i]->bead_coordinate.axis[0] -
                         active_atoms[j]->bead_coordinate.axis[0],
                     2) +
                 pow(active_atoms[i]->bead_coordinate.axis[1] -
                         active_atoms[j]->bead_coordinate.axis[1],
                     2) +
                 pow(active_atoms[i]->bead_coordinate.axis[2] -
                         active_atoms[j]->bead_coordinate.axis[2],
                     2)) -
            active_atoms[i]->bead_computed_radius -
            active_atoms[j]->bead_computed_radius;
      }
      if (separation < prober) {
        if (!active_atoms[i]->group && !active_atoms[j]->group) {
          editor->append(QString("New group %1\n").arg(next_group));
          qApp->processEvents();
          active_atoms[i]->group = active_atoms[j]->group = next_group;
          next_group++;
        } else {
          if (!active_atoms[i]->group && active_atoms[j]->group) {
            active_atoms[i]->group = active_atoms[j]->group;
          } else {
            if (active_atoms[i]->group && !active_atoms[j]->group) {
              active_atoms[j]->group = active_atoms[i]->group;
            } else {
              if (active_atoms[i]->group != active_atoms[j]->group) {
                //
                int joined_group =
                    active_atoms[i]->group > active_atoms[j]->group
                        ? active_atoms[i]->group
                        : active_atoms[j]->group;
                int removed_group =
                    active_atoms[i]->group > active_atoms[j]->group
                        ? active_atoms[j]->group
                        : active_atoms[i]->group;
                removed_groups.push_back(removed_group);
                editor->append(QString("Joining group %1 into group %2\n")
                                   .arg(removed_group)
                                   .arg(joined_group));
                qApp->processEvents();
                for (unsigned int k = 0; k < active_atoms.size(); k++) {
                  if (active_atoms[k]->group == removed_group) {
                    active_atoms[k]->group = joined_group;
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  int total_groups = next_group - 1 - (int)removed_groups.size();
  editor->append(QString("There are %1 active groups\n").arg(total_groups));
  qApp->processEvents();

  // printf("\nEnter a number to choose the calculation mode:");
  // printf("\n1- Accessible surface area only");
  // printf("\n2- Accessible and molecular surface areas");
  // printf("\n3- Accessible, molecular surface areas and average curvature of
  // MS"); printf("\nMode number:"); scanf("%d",&calcmode);

  if (!recheck) {
    QFile::remove("bead_model.asa");
  } else {
    QFile::remove("bead_model_recheck.asa");
  }

  vector<vector<PDB_atom *> > atom_groups;
  vector<int> any_buried;
  atom_groups.resize(next_group);
  any_buried.resize(next_group);
  for (unsigned int i = 0; i < active_atoms.size(); i++) {
    atom_groups[active_atoms[i]->group].push_back(active_atoms[i]);
    if (recheck) {
      if (active_atoms[i]->exposed_code == 6 ||
          active_atoms[i]->exposed_code == 10) {
        any_buried[active_atoms[i]->group] = 1;
      }
    }
  }

  for (unsigned int i = 1; i < atom_groups.size(); i++) {
    if (atom_groups[i].size()) {
      editor->append(QString("Group %1 contains %2 %3%4\n")
                         .arg(i)
                         .arg(atom_groups[i].size())
                         .arg(recheck ? "beads" : "atoms")
                         .arg((recheck && any_buried[i]) ? " with buried beads"
                                                         : " no buried beads"));
      printf("%s\n", QString("Group %1 contains %2 %3%4")
                         .arg(i)
                         .arg(atom_groups[i].size())
                         .arg(recheck ? "beads" : "atoms")
                         .arg((recheck && any_buried[i]) ? " with buried beads"
                                                         : " no buried beads")
                         .toLatin1()
                         .data());
      fflush(stdout);
    }
  }
  qApp->processEvents();

  for (unsigned int atom_group = 1; atom_group < atom_groups.size();
       atom_group++) {
    if (atom_groups[atom_group].size() &&
        (!recheck || any_buried[atom_group])) {
      editor->append(QString("Computing ASA for group %1\n").arg(atom_group));
      qApp->processEvents();
      printf("%s", QString("Computing ASA for group %1\n")
                       .arg(atom_group)
                       .toLatin1()
                       .data());
      fflush(stdout);

      memset(cycles, 0, MAXCYCLES * MAXCYCLES * sizeof(int));
      memset(common, 0, MAXCYCLES * MAXCYCLES * sizeof(int));
      memset(respect, 0, MAXCYCLES * MAXCYCLES * sizeof(int));
      memset(hits, 0, MAXCYCLES * sizeof(int));
      memset(nvincyc, 0, MAXCYCLES * sizeof(int));

      memset(verte, 0, MAXCYCLES * 3 * sizeof(float));
      memset(uconij, 0, 10000 * 3 * sizeof(float));
      memset(tconij, 0, 10000 * 3 * sizeof(float));
      memset(dist2ij, 0, 10000 * sizeof(float));

      calcmode = 1;

      //    atomnumber = (int) active_atoms.size();
      atomnumber = (int)atom_groups[atom_group].size();

      // printf("\nReading atomic coordinates and assigning radii ...");
      // fflush(stdout);

      /*allocating memory for coordinate, radius, and area arrays */
      a = (float *)calloc(atomnumber * 3, sizeof(float)); /*   coordinates   */
      if (!a) {
        free_alloced();
        // fprintf(stderr, "memory allocation error\n");
        return (US_SURFRACER_ERR_MEMORY_ALLOC);
      }
      memset(a, 0, atomnumber * 3 * sizeof(float));

      ar = (float *)calloc(atomnumber, sizeof(float)); /* radius */
      if (!ar) {
        free_alloced();
        //   fprintf(stderr, "memory allocation error\n");
        return (US_SURFRACER_ERR_MEMORY_ALLOC);
      }
      memset(ar, 0, atomnumber * sizeof(float));

      aarea = (float *)calloc(atomnumber, sizeof(float)); /* accessible area  */
      if (!aarea) {
        free_alloced();
        //   fprintf(stderr, "memory allocation error\n");
        return (US_SURFRACER_ERR_MEMORY_ALLOC);
      }
      memset(aarea, 0, atomnumber * sizeof(float));

      molarea = (float *)calloc(atomnumber,
                                sizeof(float)); /*  molecular (smooth) area  */
      if (!molarea) {
        free_alloced();
        //   fprintf(stderr, "memory allocation error\n");
        return (US_SURFRACER_ERR_MEMORY_ALLOC);
      }
      memset(molarea, 0, atomnumber * sizeof(float));

      curvat = (float *)calloc(atomnumber, sizeof(float)); /*  curvature  */
      if (!curvat) {
        free_alloced();
        //   fprintf(stderr, "memory allocation error\n");
        return (US_SURFRACER_ERR_MEMORY_ALLOC);
      }
      memset(curvat, 0, atomnumber * sizeof(float));

      visits = (short *)calloc(atomnumber,
                               sizeof(short)); /* count # of visits E54 */
      if (!visits) {
        free_alloced();
        //   fprintf(stderr, "memory allocation error\n");
        return (US_SURFRACER_ERR_MEMORY_ALLOC);
      }
      memset(visits, 0, atomnumber * sizeof(short));

      /*This part of the program loads up atomic coordinates from the file and
       * assigns radii */
      i = 0; /*atom number counter */
      {
        if (system("pwd")) {
        };
        QString outfile = QString("group_%1_%2.mod")
                              .arg(atom_group)
                              .arg(recheck ? "bead" : "atom");

        FILE *fout = us_fopen(outfile.toLatin1().data(), "w");
        fprintf(fout, "\tatomnumber = %d;\n", atomnumber);
        fprintf(fout, "\tint apos = 0;\n");

        QString bmsfile = QString("group_%1_%2.bms")
                              .arg(atom_group)
                              .arg(recheck ? "bead" : "atom");
        FILE *fbms = us_fopen(bmsfile.toLatin1().data(), "w");
        fprintf(fbms, "%d\nmodel\n", atomnumber);

        QString sptfile = QString("group_%1_%2.spt")
                              .arg(atom_group)
                              .arg(recheck ? "bead" : "atom");
        FILE *fspt = us_fopen(sptfile.toLatin1().data(), "w");
        fprintf(
            fspt,
            "load xyz %s\nselect all\nwireframe off\nset background white\n",
            bmsfile.toLatin1().data());

        if (!recheck) {
          for (i = 0; i < atomnumber; i++) {
            ar[i] = atom_groups[atom_group][i]->radius;
            // printf("radius %d %f\n", i, ar[i]);
            a[3 * i] = atom_groups[atom_group][i]->coordinate.axis[0];
            a[3 * i + 1] = atom_groups[atom_group][i]->coordinate.axis[1];
            a[3 * i + 2] = atom_groups[atom_group][i]->coordinate.axis[2];
            fprintf(fout, "\tar[apos] = %f;\n", ar[i]);
            fprintf(fout, "\ta[3 * apos] = %f;\n", a[3 * i]);
            fprintf(fout, "\ta[3 * apos + 1] = %f;\n", a[3 * i + 1]);
            fprintf(fout, "\ta[3 * apos + 2] = %f;\n", a[3 * i + 2]);
            fprintf(fout, "\tapos++;\n");
            fprintf(fbms, "Pb %.2f %.2f %.2f\n", a[3 * i], a[3 * i + 1],
                    a[3 * i + 2]);
            fprintf(fspt, "select atomno=%d\nspacefill %.2f\ncolour %s\n", i,
                    ar[i], "blue");
          }
        } else {
          for (i = 0; i < atomnumber; i++) {
            ar[i] = atom_groups[atom_group][i]->bead_computed_radius;
            // printf("radius %d %f\n", i, ar[i]);
            a[3 * i] = atom_groups[atom_group][i]->bead_coordinate.axis[0];
            a[3 * i + 1] = atom_groups[atom_group][i]->bead_coordinate.axis[1];
            a[3 * i + 2] = atom_groups[atom_group][i]->bead_coordinate.axis[2];
            fprintf(fout, "\tar[apos] = %f;\n", ar[i]);
            fprintf(fout, "\ta[3 * apos] = %f;\n", a[3 * i]);
            fprintf(fout, "\ta[3 * apos + 1] = %f;\n", a[3 * i + 1]);
            fprintf(fout, "\ta[3 * apos + 2] = %f;\n", a[3 * i + 2]);
            fprintf(fout, "\tapos++;\n");
            fprintf(fbms, "Pb %.2f %.2f %.2f\n", a[3 * i], a[3 * i + 1],
                    a[3 * i + 2]);
            fprintf(fspt, "select atomno=%d\nspacefill %.2f\ncolour %s\n", i,
                    ar[i], "blue");
          }
        }
        fclose(fspt);
        fclose(fbms);
        fclose(fout);
      }

      /*The end of the loading and assigning of radii and coordinates */

      for (i = 0; i <= atomnumber - 1;
           i++) /*incrementing all atomic radii by the probe radius */
      {
        ar[i] += prober;
      }

      // need to check why he is doing this?

      //    for (i = 0; i <= atomnumber / 2.; i++)
      //    {
      //   a[i * 3] += (float) 0.01;
      //    }

      atomcon = (int *)calloc(atomnumber,
                              sizeof(int)); /*allocating memory for array
                                               storing contacts for an atom */
      if (!atomcon) {
        free_alloced();
        //   fprintf(stderr, "memory allocation error\n");
        return (US_SURFRACER_ERR_MEMORY_ALLOC);
      }
      memset(atomcon, 0, atomnumber * sizeof(int));

      veat = (int *)calloc(atomnumber * 6 * 3,
                           sizeof(int)); /*vertex forming atom triplets */
      if (!veat) {
        free_alloced();
        //   fprintf(stderr, "memory allocation error\n");
        return (US_SURFRACER_ERR_MEMORY_ALLOC);
      }
      memset(veat, 0, atomnumber * 6 * 3 * sizeof(int));

      ve = (float *)calloc(atomnumber * 6 * 3, sizeof(float));
      if (!ve) {
        free_alloced();
        //   fprintf(stderr, "memory allocation error\n");
        return (US_SURFRACER_ERR_MEMORY_ALLOC);
      }
      memset(ve, 0, atomnumber * 6 * 3 * sizeof(int));

      ciat = (int *)calloc(atomnumber * 2,
                           sizeof(int)); /*contact circle forming atom pairs */
      if (!ciat) {
        free_alloced();
        //   fprintf(stderr, "memory allocation error\n");
        return (US_SURFRACER_ERR_MEMORY_ALLOC);
      }
      memset(ciat, 0, atomnumber * 2 * sizeof(int));

      ci = (float *)calloc(atomnumber * 3, sizeof(float));
      if (!ci) {
        free_alloced();
        //   fprintf(stderr, "memory allocation error\n");
        return (US_SURFRACER_ERR_MEMORY_ALLOC);
      }
      memset(ci, 0, atomnumber * 3 * sizeof(float));

      /*loadind unburied vertexes (probe positions) and contact circles*/
      //  printf("\nBuilding the surface..."); fflush(stdout);
      nv = 0; /*setting the number of vertexes and circles at 0 */
      ncircle = 0;

      delta = (2. + prober) * 2.; /*box size around each atom */

      // start1 = clock();

      for (ai = 0; ai <= atomnumber - 1;
           ai++) /*taking an atom (going through all) */
      {
        // if (ai != 0)
        //     if (ai % 1000 == 0)
        //    printf("\n%d atoms traced", ai);

        xmax = a[ai * 3] + delta;
        xmin = a[ai * 3] - delta;
        ymax = a[ai * 3 + 1] + delta;
        ymin = a[ai * 3 + 1] - delta;
        zmax = a[ai * 3 + 2] + delta;
        zmin = a[ai * 3 + 2] - delta;

        for (i = 0, k = 0; i <= atomnumber - 1;
             i++)      /*finding overlaps and filling contact atom array */
          if (i != ai) /*all but atom ai */
            if (a[i * 3] < xmax && a[i * 3] > xmin && a[i * 3 + 1] < ymax &&
                a[i * 3 + 1] > ymin && a[i * 3 + 2] < zmax &&
                a[i * 3 + 2] > zmin) /*eliminating the atoms whose centers lie
                                        outside of the box */
            {
              rsum = ar[ai] + ar[i];

              if ((dist2ij[k] = dist2(ai, i)) <
                  rsum * rsum) /* here filling up dist2ij array here too (limit
                                  = 500 contacting atoms!!) */
              {
                dist2ij[k] = sqrt(dist2ij[k]); /*storing actual distances */

                vdiff(&a[3 * i], &a[3 * ai], uconij[k]);
                vtimk(uconij[k], 1. / dist2ij[k]); /*calculation of torus axis
                                                      unit vector uij */

                c = (ar[ai] * ar[ai] - ar[i] * ar[i]) /
                    dist2ij[k]; /*torus center tij */

                tconij[k][0] = 0.5 * (a[3 * ai] + a[3 * i] + uconij[k][0] * c);
                tconij[k][1] =
                    0.5 * (a[3 * ai + 1] + a[3 * i + 1] + uconij[k][1] * c);
                tconij[k][2] =
                    0.5 * (a[3 * ai + 2] + a[3 * i + 2] + uconij[k][2] * c);

                atomcon[k] = i; /*storing atom i */
                k++;            /*incrementing the atomcon array index k */
              }
            }

        /*at this point the atoms overlapping with atom ai are stored in array
         * atomcon */

        for (aj = 0; aj <= k - 1;
             aj++) /*aj is an index in atomcon (atoms contacting ai) */
        {
          if (atomcon[aj] > ai) /*choosing so that the atom number is larger
                                   than ai- ordered contact */
          {
            for (ak = aj + 1, nvk = 0; ak <= k - 1;
                 ak++) /*because the atoms in atomcon are ordered already- start
                          with the next index here to get an ordered triplet */
            { /*if no intersections with the circle formed by atomcon[aj] and
                 ai,atomcon[aj] pair, nvk stays 0 */
              rsum = ar[atomcon[ak]] + ar[atomcon[aj]];

              if (dist2(atomcon[ak], atomcon[aj]) <
                  rsum *
                      rsum) /*atom atomcon[ak] overlaps with atom atomcon[aj] */
              {             /*torus axis unit vectors */

                cba = scalarprod(uconij[aj],
                                 uconij[ak]); /*cosine of base triangle angle
                                                 for these three atoms */
                if (cba < 1 && cba > -1)      /*the three are not collinear */
                {
                  vectorprod(uconij[aj], uconij[ak], uijk);
                  sba = sqrt(1. - cba * cba); /*sin of base angle */
                  vtimk(uijk, 1. / sba); /*this is base plane normal vector */
                  vectorprod(uijk, uconij[aj],
                             utb); /*torus basepoint unit vector */

                  /*calculation of base point bijk */

                  vdiff(tconij[ak], tconij[aj], bijk);
                  vtimk(utb, scalarprod(uconij[ak], bijk) / sba);
                  vsum(tconij[aj], utb, bijk); /*base point */

                  vdiff(bijk, &a[ai * 3],
                        utb); /*utb is screwed up- don't use it except below */
                  ph2 = ar[ai] * ar[ai] -
                        scalarprod(utb, utb); /*square of probe height */

                  if (ph2 >
                      0) /*there are two probe positions (two vertexes)! */
                  {      /*recording the two vertexes into ve1 and ve2 */
                    nvk = 1;

                    vtimk(uijk, sqrt(ph2));
                    vsum(bijk, uijk, ve1);  /*first vertex */
                    vdiff(bijk, uijk, ve2); /*second vertex */

                    /*check whether vertexes 1 and 2 are buried */
                    bur1 = 0;
                    bur2 = 0;
                    if (k > 2) {
                      for (l = 0; l <= k - 1; l++) {
                        if (l != aj && l != ak) /*a different atom */
                          if (dis2(ve1, &a[atomcon[l] * 3]) <
                              ar[atomcon[l]] * ar[atomcon[l]]) {
                            bur1 = 1; /*first vertex is buried */
                            break;
                          }
                      }

                      for (l = 0; l <= k - 1; l++) {
                        if (l != aj && l != ak) /*a different atom */
                          if (dis2(ve2, &a[atomcon[l] * 3]) <
                              ar[atomcon[l]] * ar[atomcon[l]]) {
                            bur2 = 1; /*second vertex is buried */
                            break;
                          }
                      }
                    }
                    /*finished checking vertex burial here */

                    if (!bur1) /*first vertex is not buried */
                    {
                      veat[3 * nv] = ai; /*storing the vertex atoms */
                      veat[3 * nv + 1] = atomcon[aj];
                      veat[3 * nv + 2] = atomcon[ak];

                      ve[3 * nv] = ve1[0]; /*storing the vertex coordinates */
                      ve[3 * nv + 1] = ve1[1];
                      ve[3 * nv + 2] = ve1[2];

                      nv++; /*increment vertex count */

                      if (nv >= atomnumber * 6) /*if goes into here allocate
                                                   more memory and recompile */
                      {
                        // fprintf(stderr, "\nNot enough computer memory for
                        // vertexes");
                        free_alloced();
                        return (US_SURFRACER_ERR_MEMORY_ALLOC);
                      }
                    }

                    if (!bur2) /*second vertex is not buried */
                    {
                      veat[3 * nv] = ai; /*storing the vertex atoms */
                      veat[3 * nv + 1] = atomcon[aj];
                      veat[3 * nv + 2] = atomcon[ak];

                      ve[3 * nv] = ve2[0]; /*storing the vertex coordinates */
                      ve[3 * nv + 1] = ve2[1];
                      ve[3 * nv + 2] = ve2[2];

                      nv++;

                      if (nv >= atomnumber * 6) /*if goes into here allocate
                                                   more memory and recompile */
                      {
                        fprintf(stderr,
                                "\nNot enough computer memory for vertexes");
                        free_alloced();
                        return (US_SURFRACER_ERR_MEMORY_ALLOC);
                      }
                    }
                  }
                  /* loaded vertexes else  there are no such probe positions to
                   * form them */
                }
                /*else here means the atoms are collinear */
              }
              /*the third atom does not overlap with the second */
            }
            /*finished running through the third atoms here */

            if (nvk == 0) /*checking whether atom ak forms a vertex (possibly
                             buried) with ai and atomcon[aj] */
              if (k > 1)
                for (ak = 0; ak <= aj - 1; ak++) {
                  rsum = ar[atomcon[ak]] + ar[atomcon[aj]];
                  if (dist2(atomcon[ak], atomcon[aj]) <
                      rsum * rsum) /*atoms overlap */
                  {                /*torus axis unit vectors */

                    cba = scalarprod(
                        uconij[aj], uconij[ak]); /*cosine of base triangle angle
                                                    for these three atoms */
                    if (cba < 1 && cba > -1) /*the three are not collinear */
                    {
                      vectorprod(uconij[aj], uconij[ak], uijk);
                      sba = sqrt(1. - cba * cba); /*sin of base angle */
                      vtimk(uijk,
                            1. / sba); /*this is base plane normal vector */
                      vectorprod(uijk, uconij[aj],
                                 utb); /*torus basepoint unit vector */

                      /*calculation of base point bijk */

                      vdiff(tconij[ak], tconij[aj], bijk);
                      vtimk(utb, scalarprod(uconij[ak], bijk) / sba);
                      vsum(tconij[aj], utb, bijk); /*base point */

                      vdiff(bijk, &a[ai * 3], utb); /*utb is screwed up- don't
                                                       use it except below */
                      ph2 = ar[ai] * ar[ai] -
                            scalarprod(utb, utb); /*square of probe height */

                      if (ph2 > 0) {
                        nvk = 1;
                        break;
                      } /*vertex can be placed but it is buried so it was not
                           recorded */
                    }
                  }
                }

            if (nvk == 0) /*no atoms messing in */
            { /*one bad possiblility left - the whole contact circle can be
                 buried */

              /*re is a point on the circle */
              /*calculation of point on the surface */
              if (uconij[aj][0] != 0. || uconij[aj][1] != 0.) {
                c = sqrt(uconij[aj][0] * uconij[aj][0] +
                         uconij[aj][1] * uconij[aj][1]);
                diff[0] = -uconij[aj][1] / c;
                diff[1] = uconij[aj][0] / c;
                diff[2] = 0.;
              } else {
                c = sqrt(uconij[aj][0] * uconij[aj][0] +
                         uconij[aj][2] * uconij[aj][2]);
                diff[0] = -uconij[aj][2] / c;
                diff[1] = 0;
                diff[2] = uconij[aj][0] / c;
              }

              vtimk(diff,
                    torusrad(
                        ai,
                        atomcon[aj]));    /*diff1 is a radial vector of torus
                                             radius length from the torus center */
              vsum(tconij[aj], diff, re); /*finished re calculation */

              /*it is sufficient to check burial of only this point re */

              bur1 = 0;
              if (k > 1) /*if more than one atom contacts ai */
                for (l = 0; l <= k - 1; l++) {
                  if (atomcon[l] != atomcon[aj])
                    if (dis2(re, &a[atomcon[l] * 3]) <
                        ar[atomcon[l]] * ar[atomcon[l]]) {
                      bur1 = 1; /*circle is buried */
                      break;
                    }
                }
              /*finished checking circle burial here */

              if (!bur1) /*the circle is not buried */
              {
                ciat[ncircle * 2] = ai; /*storing circle atoms */
                ciat[ncircle * 2 + 1] = atomcon[aj];

                ci[3 * ncircle] = re[0]; /*storing circle point coordinates */
                ci[3 * ncircle + 1] = re[1];
                ci[3 * ncircle + 2] = re[2];

                ncircle++; /*incrementing the circle counter */
                if (ncircle >= atomnumber) {
                  //             fprintf(stderr, "\nNot enough RAM for the
                  //             contact circle array");
                  free_alloced();
                  return (US_SURFRACER_ERR_MEMORY_ALLOC);
                }
              }
            }
            /*finished loading circles */
          }
          /*chose the ordered atom atomcon[aj] else, its ai */
        }
        /*finished considering all contacting atoms */
      }
      /*finished going through all atoms- all surface and cavity vertexes and
       * contact circles are loaded */

      free(atomcon); /*frees space that will not be used further */
      atomcon = (int *)0;

      // end1 = clock();

      /*optimizing memory allocation here */
      if (nv > 0) /*if there are vertices */
      {
        verat = (int *)calloc(nv * 3,
                              sizeof(int)); /*vertex forming atom triplets */
        if (!verat) {
          // fprintf(stderr, "\nNot enough computer memory for vertexes");
          free_alloced();
          return (US_SURFRACER_ERR_MEMORY_ALLOC);
        }
        memset(verat, 0, nv * 3 * sizeof(int));

        ver = (float *)calloc(nv * 3, sizeof(float));
        if (!ver) {
          // fprintf(stderr, "\nNot enough computer memory for vertexes");
          free_alloced();
          return (US_SURFRACER_ERR_MEMORY_ALLOC);
        }
        memset(ver, 0, nv * 3 * sizeof(float));
      }

      if (ncircle > 0) /*if there are circles */
      {
        cirat = (int *)calloc(
            ncircle * 2, sizeof(int)); /*contact circle forming atom pairs */
        if (!cirat) {
          // fprintf(stderr, "\nNot enough computer memory for vertexes");
          free_alloced();
          return (US_SURFRACER_ERR_MEMORY_ALLOC);
        }
        memset(cirat, 0, ncircle * 2 * sizeof(int));

        cir = (float *)calloc(ncircle * 3, sizeof(float));
        if (!cir) {
          // fprintf(stderr, "\nNot enough computer memory for vertexes");
          free_alloced();
          return (US_SURFRACER_ERR_MEMORY_ALLOC);
        }
        memset(cir, 0, ncircle * 3 * sizeof(float));
      }

      veflag = (char *)calloc(ncircle + nv, sizeof(char));
      if (!veflag) {
        // fprintf(stderr, "\nNot enough computer memory for vertexes");
        free_alloced();
        return (US_SURFRACER_ERR_MEMORY_ALLOC);
      }
      memset(veflag, 0, (ncircle + nv) * sizeof(char));

      for (i = 0; i <= nv - 1; i++) {
        verat[i * 3] = veat[i * 3];
        verat[i * 3 + 1] = veat[i * 3 + 1];
        verat[i * 3 + 2] = veat[i * 3 + 2];
        ver[i * 3] = ve[i * 3];
        ver[i * 3 + 1] = ve[i * 3 + 1];
        ver[i * 3 + 2] = ve[i * 3 + 2];
      }

      if (ve) {
        free(ve);
        ve = (float *)0;
      }
      if (veat) {
        free(veat);
        veat = (int *)0;
      }

      for (i = 0; i <= ncircle - 1; i++) {
        cirat[i * 2] = ciat[i * 2];
        cirat[i * 2 + 1] = ciat[i * 2 + 1];
        cir[i * 3] = ci[i * 3];
        cir[i * 3 + 1] = ci[i * 3 + 1];
        cir[i * 3 + 2] = ci[i * 3 + 2];
      }

      if (ci) {
        free(ci);
        ci = (float *)0;
      }
      if (ciat) {
        free(ciat);
        ciat = (int *)0;
      }

      /*finished memory optimization */

      /*one caveat to be taken care of later is a very unlikely but possible
       * exact coincidence of two or more vertices */
      /*these are called degenerate vertices further */
      /*the degenerate vertices are resolved in potoedge */
#if defined(DEBUG)
      printf("nv %d ncircle %d\n", nv, ncircle);
      fflush(stdout);
#endif

      ned = (3 * nv / 2 + ncircle);

      edge = (int *)calloc(ned * 2, sizeof(int)); /* radius */
      if (!edge) {
        // fprintf(stderr, "\nNot enough computer memory for vertexes");
        free_alloced();
        return (US_SURFRACER_ERR_MEMORY_ALLOC);
      }
      memset(edge, 0, ned * 2 * sizeof(int));

      edgeatom = (int *)calloc(ned, sizeof(int)); /* radius */
      if (!edgeatom) {
        // fprintf(stderr, "\nNot enough computer memory for vertexes");
        free_alloced();
        return (US_SURFRACER_ERR_MEMORY_ALLOC);
      }
      memset(edgeatom, 0, ned * sizeof(int));

      for (i = 0; i <= ned - 1;
           i++) /*initializing the first elements of each pair with -1 */
      {
        edge[i * 2] = -1;
        edgeatom[i] = -1;
      }

      // start2 = clock();

      // printf("\nSurface built successfully...");
      // printf("\nStarting area calculation..."); fflush(stdout);

      /*beginning of surface calculation */
      /*finding the atom with the highest north pole-it's accessible and will be
       * the first one */
      zmax = -100000.;
      atom0 = -1;
      dbg("0");
      for (i = 0; i <= atomnumber - 1; i++) {
        if ((z = a[i * 3 + 2] + ar[i]) > zmax) {
          zmax = z;
          atom0 = i;
        }
      }

      /*the first atom is atom0 */
      dbg("1");

      /* building all ncycle cycles of atom0 */
      ncycle = buildcycles(atom0, cycles, common, nvincyc);

      dbg("2");
      /*loading all internal-external relationships between the cycles of atom0
       */
      loadresp(atom0, ncycle, cycles, respect, nvincyc);

      /*the highest point belongs to the accessible surface area */
      /*we need to create another cycle - circle formed by this highest point */
      dbg("3");

      for (i = 0; i <= ncycle - 1; i++) {
        respect[ncycle][i] = 1; /*all cycles are internal with respect to it */
      }

      respect[ncycle][ncycle] = 0; /*except for itself */

      highest[0] = a[atom0 * 3];
      highest[1] = a[atom0 * 3 + 1];
      highest[2] = a[atom0 * 3 + 2] + ar[atom0]; /*the highest north pole */

      if (ncycle == 1) /*only one cycle- the point is always internal with
                          respect to it */
      {
        respect[0][1] = 1;
      } else { /*more than one cycle */
        for (i = 0; i <= ncycle - 1; i++) {
          if (cycles[i][0] == -2) {
            respect[i][ncycle] =
                1; /*it's internal with respect to all circles */
          } else {
            for (j = 0; j <= MAXCYCLES && cycles[i][j] != -1; j++) {
              verte[j][0] = ver[cycles[i][j] * 3]; /*j vertices of cycle i */
              verte[j][1] = ver[cycles[i][j] * 3 + 1];
              verte[j][2] = ver[cycles[i][j] * 3 + 2];
            }
            respect[i][ncycle] =
                checkcycle(atom0, j - 1, verte,
                           highest); /*highest with respect to cycle i */
          }
        }
      }

      dbg("4");
      /*finds all nhits number of cycles to be included in the surface
       * calculation*/
      nhits = searchcycle(ncycle + 1, ncycle, respect,
                          hits); /*nhits does not include the cycle ncycle!!! */

      asar = convexarea(
          atom0, cycles, common, hits, nhits,
          nvincyc); /*here we don't want to include the ncycle-th cycle */
      if (asar >= 0.001)
        aarea[atom0] += asar;
            /*this is the water accessible surface area of atom0 */ /*accessible
                                                                       convex
                                                                       area */
      else
        aarea[atom0] += (float)0.0001;
      if (calcmode > 1)
        molarea[atom0] += asar * (ar[atom0] - prober) * (ar[atom0] - prober) /
                          (ar[atom0] * ar[atom0]); /*molecular convex area */

      visits[atom0]++;
      managedge(atom0, cycles, common, hits, nhits,
                nvincyc); /*edge array management */

      dbg("5");
#if defined PDEBUG
      printf("\nProbe rolled over atom %d\n",
             atom0 + 1); /*line used in debugging */
#endif

      /*surface calculation of the rest of the molecule*/
      while (1) {
        dbg("5.1");
        for (i = 0; i <= ned - 1; i++)
          if (edge[i * 2] != -1 && visits[edgeatom[i]] < 2)
            break; /*found a new edge */
        dbg("5.2");

        if (i == ned) /*outer surface calculation completed */
        { /* printf("\n\nCalculation complete for the outer surface");  */

          dbg("5.3");
          // end2=clock();
          // tused=(end1-start1+end2-start2)/CLK_TCK;
          // printf("\nTime used=%f sec ",tused);

          break;
        } else /*perform calculation for the next atom */
        {
#if defined PDEBUG
          printf("\nProbe rolled over atom %d\n",
                 edgeatom[i]); /* line used in debugging */
#endif

          dbg("5.4");
          /*  if(edgeatom[i]==23066)
              printf("\nI'm here");

              if(edgeatom[i]==29881)
              printf("\nI'm here");               */

          ncycle = buildcycles(edgeatom[i], cycles, common,
                               nvincyc); /*build all cycles */
          dbg("5.5");
          loadresp(edgeatom[i], ncycle, cycles, respect,
                   nvincyc); /*load relationships */
          dbg("5.6");
          cyc1 = find1cyc(i, cycles, ncycle, nvincyc); /*find first cycle */
          dbg("5.7");
          nhits = searchcycle(ncycle, cyc1, respect,
                              hits); /*hits does not include cyc1 */
          dbg("5.8");
          hits[nhits] = cyc1;
          dbg("5.9");
          nhits++; /*now ALL cycles are included!! */
          dbg("5.10");
          asar = convexarea(edgeatom[i], cycles, common, hits, nhits,
                            nvincyc); /*accessible area */
          dbg("5.11");
          if (asar >= 0.001)
            aarea[edgeatom[i]] += asar;
          else
            aarea[edgeatom[i]] += (float)0.0001;
          if (calcmode > 1)
            molarea[edgeatom[i]] += asar * (ar[edgeatom[i]] - prober) *
                                    (ar[edgeatom[i]] - prober) /
                                    (ar[edgeatom[i]] * ar[edgeatom[i]]);
          dbg("5.12");
          visits[edgeatom[i]]++;
          managedge(edgeatom[i], cycles, common, hits, nhits,
                    nvincyc); /*edge array management */
#if defined PDEBUG
          printf("asa atom: %d aarea[%d]=%f\n", edgeatom[i], edgeatom[i],
                 aarea[edgeatom[i]]);
          fflush(stdout);
#endif
          dbg("5.13");
        }
      }
      free(visits);
      visits = (short *)0;

      dbg("6");
      // puts("us_surfracer 1");
      // fflush(stdout);
      FILE *aafile;
      if (!recheck) {
        aafile = us_fopen("bead_model.asa", "a");
      } else {
        aafile = us_fopen("bead_model_recheck.asa", "a");
      }

      fprintf(aafile, "atom group %d\n", atom_group);

      // puts("us_surfracer 2");
      // fflush(stdout);
      for (i = 0; i < atomnumber; i++) {
        //   active_atoms[i]->asa = aarea[i];
        atom_groups[atom_group][i]->asa = aarea[i];
        fprintf(aafile, "atom %d %f %f %f %f %f\n", i + 1, a[i * 3],
                a[i * 3 + 1], a[i * 3 + 2], ar[i] - prober, aarea[i]);
      }
      fclose(aafile);
      // puts("us_surfracer 3");
      // fflush(stdout);
      free_alloced();
      // puts("us_surfracer 4");
      // fflush(stdout);
    }
  }
  return (0);

#if defined(US_SURFRACER_COMPUTE_EXTRAS)
  resfile = us_fopen("result.txt",
                     "a"); /*resfile will have the breakdown of all areas */
  /*calculating the curvatures*/
  if (calcmode == 3)
    for (i = 0; i <= atomnumber - 1; i++) {
      if (molarea[i] > 0) {
        curvat[i] += aarea[i] / (ar[i] * ar[i]) * 2. *
                     (ar[i] - prober); /*adding the convex part */
        curvat[i] /= -2. * molarea[i]; /*the final division */
      }
    }
  /*saving the surface areas in the file *.asa */

  pdbfile = us_fopen(pdbf, "r"); /*opening pdb file for reading */

  for (i = 0; pdbf[i] != '.'; i++); /*found the period */
  i++;

  pdbf[i] = 't';
  pdbf[i + 1] = 'x';
  pdbf[i + 2] = 't'; /* creating the same name with the txt suffix */

  asafile = us_fopen(pdbf, "w"); /*opening asa file for writing */

  polarea = 0.;
  nparea = 0.;
  molnparea = 0.;
  molpolarea = 0.;

  k = 0; /*atom number */
  while (fgets(str, 162, pdbfile) != NULL) {
    str[4] = '\0';
    if (strcmp("ATOM", str) == 0) {
      str[4] = ' ';
      for (i = 17; i <= 19; i++) acidname[i - 17] = str[i];

      atomname[0] = ' ';
      atomname[1] = ' ';
      atomname[2] = ' ';
      for (j = 13; j <= 15; j++)
        atomname[j - 13] = str[j]; /*loading the atom name */

      if (strcmp(acidname, "DOD") != 0 && strcmp(acidname, "HOH") != 0 &&
          atomname[0] != 'H' && atomname[0] != 'D' &&
          (str[16] == 'A' ||
           str[16] == ' ')) /*disregarding bound waters and hydrogens */
      {
        sprintf(radi, "%.2f", ar[k] - prober);
        // gcvt(ar[k]-prober,4,radi);

        for (j = 0; radi[j] != '\0'; j++);
        radi[j] = ' ';
        for (j = j + 1; j <= 4; j++) radi[j] = ' ';

        sprintf(asa, "%.2f", aarea[k]);
        // gcvt(aarea[k],5,asa);
        for (j = 0; asa[j] != '\0'; j++);
        asa[j] = ' ';
        for (j = j + 1; j <= 5; j++) asa[j] = ' ';

        for (j = 0; j <= 3; j++) str[56 + j] = radi[j];

        str[60] = ' ';
        str[61] = ' ';
        str[62] = ' ';

        for (j = 0; j <= 4; j++) str[63 + j] = asa[j];

        str[68] = ' ';
        str[69] = ' ';

        sprintf(asa, "%.2f", molarea[k]);
        // gcvt(molarea[k],5,asa);           /*converting molecular areas into a
        // string*/
        for (j = 0; asa[j] != '\0'; j++);
        asa[j] = ' ';
        for (j = j + 1; j <= 5; j++) asa[j] = ' ';

        for (j = 0; j <= 4; j++) str[70 + j] = asa[j];

        str[75] = ' ';

        sprintf(asa, "%.2f", curvat[k]);
        // gcvt(curvat[k],5,asa);           /*converting molecular areas into a
        // string*/
        for (j = 0; asa[j] != '\0'; j++);
        asa[j] = ' ';
        for (j = j + 1; j <= 5; j++) asa[j] = ' ';

        for (j = 0; j <= 4; j++) str[76 + j] = asa[j];

        str[81] = ' ';
        str[82] = ' ';
        str[83] = ' ';
        str[84] = ' ';
        str[85] = ' ';
        str[86] = '\n';
        str[87] = '\0';

        fputs(str, asafile); /*writing the radii and areas to the file */

        if (atomname[0] == 'C') /*carbon */
        {
          nparea += aarea[k];      /*total nonpolar ASA */
          molnparea += molarea[k]; /*total nonpolar MSA */
          if (atomname[1] == ' ' ||
              (atomname[1] == 'A' && atomname[2] == ' ')) /*backbone carbons */
          {
            bbnpasa += aarea[k];   /*backbone np ASA */
            bbnpmsa += molarea[k]; /*backbone np MSA */
          } else {
            scnpasa += aarea[k];   /*side chain np ASA */
            scnpmsa += molarea[k]; /*side chain np MSA */
          }
        } else {
          polarea += aarea[k];      /*total polar ASA */
          molpolarea += molarea[k]; /*total polar MSA */
          if ((atomname[0] == 'O' || atomname[0] == 'N') &&
              atomname[1] == ' ') /*backbone oxygen and nitrogen */
          {
            bbpolasa += aarea[k];   /*backbone polar ASA */
            bbpolmsa += molarea[k]; /*backbone polar MSA */
          } else {
            scpolasa += aarea[k];   /*side chain polar ASA */
            scpolmsa += molarea[k]; /*side chain polar MSA */
            if (strcmp(atomname, "O1P") == 0 || strcmp(atomname, "O2P") == 0 ||
                (strcmp(acidname, "ASP") == 0 &&
                 (strcmp(atomname, "OD1") == 0 ||
                  strcmp(atomname, "OD2") == 0)) ||
                (strcmp(acidname, "GLU") == 0 &&
                 (strcmp(atomname, "OE1") == 0 ||
                  strcmp(atomname, "OE2") == 0))) {
              scminus += aarea[k];
              scmminus += molarea[k];
            } else if ((strcmp(acidname, "ARG") == 0 &&
                        (strcmp(atomname, "NH1") == 0 ||
                         strcmp(atomname, "NH2") == 0)) ||
                       (strcmp(acidname, "LYS") == 0 &&
                        strcmp(atomname, "NZ ") == 0) ||
                       (strcmp(acidname, "HIS") == 0 &&
                        (strcmp(atomname, "NE2") == 0 ||
                         strcmp(atomname, "ND1") == 0))) {
              scplus += aarea[k];
              scmplus += molarea[k];
            }
          }
        }

        k++;
      }
    }
  }

  // printf("\n\nSolvent accessible surface areas (in Angstrom^2):");
  // printf("\nTotal area = %.2f, Polar area= %.2f , Non-polar area= %.2f",
  // polarea + nparea, polarea, nparea); printf("\n\nMolecular surface areas (in
  // Angstrom^2):"); printf("\nTotal area = %.2f, Polar area= %.2f , Non-polar
  // area= %.2f", molpolarea + molnparea, molpolarea, molnparea);
  // printf("\nVertices=%d  circles=%d", nv, ncircle);
  // printf("\n\nPress <Enter> to check for cavities");
  // getchar();
  // getchar();

  fclose(asafile); /*closing the file */

  asafile = us_fopen(pdbf, "r"); /*opening asa file for reading */

  for (i = 0; pdbf[i] != '.'; i++); /*found the period */
  i++;

  pdbf[i] = 'c';
  pdbf[i + 1] = 'a';
  pdbf[i + 2] = 'v'; /* creating the same name with the txt suffix */

  cavfile = us_fopen(pdbf, "w"); /*opening cav file for writing */

  /*cavity calculation */

  cavn = 1;

  while (1) {
    for (i = 0; i <= nv + ncircle - 1; i++)
      if (veflag[i] ==
          0) /*found untreated vertex or circle- it must be in a cavity */
      {
        // printf("\nFound cavity %d", cavn);

        for (j = 0; j <= atomnumber - 1; j++) {
          aarea[j] = 0.; /*reinitializing atomic areas and curvature */
          molarea[j] = 0.;
          curvat[j] = 0.;
        }
        break;
      }

    /* if(i==23720)
       printf("\nvertex=%d",i);        */

    if (i ==
        nv +
            ncircle) /*did not find untreated elements- there are NO cavities */
    {
      // if (cavn == 1)
      // printf("\nThe structure contains no cavities");
      break;
    } else                            /*cavity found */
    { /*cavity surface calculation */ /*finding the first atom */
      if (i >= nv)                    /*it's a circle */
        atom0 = cirat[(i - nv) * 2];  /*i-nv is the circle number */
      else                            /*it's a vertex */
        atom0 = verat[i * 3];         /*i is the vertex number */

      /*calculating atom0 surface area */
      ncycle = buildcycles(atom0, cycles, common,
                           nvincyc); /*build all cycles for atom0 */
      loadresp(atom0, ncycle, cycles, respect, nvincyc); /*load relationships */
      cyc1 = fi1cyc(i, cycles, ncycle,
                    nvincyc); /*find first cycle by essentially the same
                                 function as find1cyc */
      nhits = searchcycle(ncycle, cyc1, respect,
                          hits); /*hits includes all cycles that belong the
                                    surface of interest but cyc1 */
      hits[nhits] = cyc1;
      nhits++; /*now ALL cycles are included!! */
      asar = convexarea(atom0, cycles, common, hits, nhits, nvincyc);
      if (asar >= 0.001)
        aarea[atom0] += asar; /*surface area */
      else
        aarea[atom0] += 0.0001;
      if (calcmode > 1)
        molarea[atom0] += asar * (ar[atom0] - prober) * (ar[atom0] - prober) /
                          (ar[atom0] * ar[atom0]);
      managedge(atom0, cycles, common, hits, nhits,
                nvincyc); /*edge array management */

      /*doing it for the rest of that cavity */
      while (1) {
        for (i = 0; i <= ned - 1; i++)
          if (edge[i * 2] != -1) break; /*found a new edge */

        if (i == ned) /*outer surface calculation completed */
        {
          // printf("\n\nCalculation complete for cavity %d", cavn);
          break;
        } else /*perform calculation for the next atom */
        { /*printf("\nProbe rolled over atom %d",edgeatom[i]); */ /* line used
                                                                     in
                                                                     debugging
                                                                   */

          /*  if(edgeatom[i]==24563)
              printf("\nI'm here");

              if(edgeatom[i]==30275)
              printf("\nI'm here");         */

          ncycle = buildcycles(edgeatom[i], cycles, common,
                               nvincyc); /*build all cycles */
          loadresp(edgeatom[i], ncycle, cycles, respect,
                   nvincyc);                           /*load relationships */
          cyc1 = find1cyc(i, cycles, ncycle, nvincyc); /*find first cycle */
          nhits = searchcycle(ncycle, cyc1, respect,
                              hits); /*hits does not include cyc1 */
          hits[nhits] = cyc1;
          nhits++; /*now ALL cycles are included!! */
          asar = convexarea(edgeatom[i], cycles, common, hits, nhits, nvincyc);
          if (asar >= 0.001)
            aarea[edgeatom[i]] += asar; /*surface area */
          else
            aarea[edgeatom[i]] += 0.0001;
          if (calcmode > 1)
            molarea[edgeatom[i]] += asar * (ar[edgeatom[i]] - prober) *
                                    (ar[edgeatom[i]] - prober) /
                                    (ar[edgeatom[i]] * ar[edgeatom[i]]);
          managedge(edgeatom[i], cycles, common, hits, nhits,
                    nvincyc); /*edge array management */
        }
      }

      fprintf(cavfile, "CAVITY %d\n", cavn);

      if (calcmode == 3)
        for (i = 0; i <= atomnumber - 1; i++) {
          if (molarea[i] > 0) {
            curvat[i] += aarea[i] / (ar[i] * ar[i]) * 2. *
                         (ar[i] - prober); /*adding the convex part */
            curvat[i] /= -2. * molarea[i]; /*the final division */
          }
        }

      /*appending the cavity areas to the file */
      for (i = 0; i <= atomnumber - 1; i++) {
        fgets(str1, 162, asafile); /*read a string */
        if (aarea[i] != 0.)        /*found the cavity atom */
        {
          sprintf(asa, "%.2f", aarea[i]);
          // gcvt(aarea[i],5,asa);        /*convert to a string*/
          for (j = 0; asa[j] != '\0'; j++);
          asa[j] = ' ';
          for (j = j + 1; j <= 5; j++) asa[j] = ' ';

          for (j = 0; j <= 4; j++)
            str1[63 + j] = asa[j]; /*write into the string */

          sprintf(asa, "%.2f", molarea[i]);
          // gcvt(molarea[i],5,asa);           /*converting molecular areas into
          // a string*/
          for (j = 0; asa[j] != '\0'; j++);
          asa[j] = ' ';
          for (j = j + 1; j <= 5; j++) asa[j] = ' ';

          for (j = 0; j <= 4; j++) str1[70 + j] = asa[j];

          str[75] = ' ';

          sprintf(asa, "%.2f", curvat[i]);
          // gcvt(curvat[i],5,asa);           /*converting molecular areas into
          // a string*/
          for (j = 0; asa[j] != '\0'; j++);
          asa[j] = ' ';
          for (j = j + 1; j <= 5; j++) asa[j] = ' ';

          for (j = 0; j <= 4; j++) str1[76 + j] = asa[j];

          fputs(str1, cavfile); /*write down the cavity atom string */
        }
      }

      rewind(asafile);
      cavn++;
    }
  }

  fclose(asafile);
  fclose(cavfile);
  fclose(pdbfile);

  cavfile = us_fopen(pdbf, "r"); /*opening cav file for reading */

  for (i = 0; pdbf[i] != '.'; i++); /*found the period */
  i++;

  pdbf[i] = 't';
  pdbf[i + 1] = 'x';
  pdbf[i + 2] = 't'; /* creating the same name with the txt suffix */

  asafile = us_fopen(pdbf, "a"); /*opening area file for appending */

  while (fgets(str1, 162, cavfile) != NULL) /*read a string */
    fputs(str1, asafile);                   /*append cavity info at the end */

  fclose(asafile);
  fclose(cavfile);

  for (i = 0; pdbf[i] != '.'; i++); /*found the period */
  i++;

  pdbf[i] = 'c';
  pdbf[i + 1] = 'a';
  pdbf[i + 2] = 'v'; /* creating the same name with the cav suffix */

  remove(pdbf); /*delete the *.cav file */

  for (i = 0; pdbf[i] != '.'; i++); /*found the period */

  pdbf[i] = ' ';
  pdbf[i + 1] = ' ';
  pdbf[i + 2] = ' ';
  pdbf[i + 3] = ' '; /* creating the same name with the cav suffix */

  fprintf(resfile, "The surface area of %s\n", pdbf);
  fprintf(resfile, "Number of non-HOH, non-H atoms=%d\n", atomnumber);
  fprintf(resfile, "Time used=%f sec\n", tused);
  fprintf(resfile, "Probe radius=%.2f\n", prober);
  fprintf(resfile,
          "TOTAL ASA=%.2f                             TOTAL MSA=%.2f\n",
          nparea + polarea, molnparea + molpolarea);
  fprintf(resfile,
          "Polar ASA=%.2f     Non-polar ASA=%.2f    \nPolar MSA=%.2f     "
          "Non-polar MSA=%.2f\n",
          polarea, nparea, molpolarea, molnparea);
  fprintf(resfile, "Total backbone ASA=%.2f   Total backbone MSA=%.2f\n",
          bbnpasa + bbpolasa, bbnpmsa + bbpolmsa);
  fprintf(resfile,
          "Polar backbone ASA=%.2f Non-polar backbone ASA=%.2f \nPolar "
          "backbone MSA=%.2f Non-polar backbone MSA=%.2f\n",
          bbpolasa, bbnpasa, bbpolmsa, bbnpmsa);
  fprintf(resfile,
          "Polar side chain ASA=%.2f Non-polar side chain ASA=%.2f \nPolar "
          "side chain MSA=%.2f Non-polar side chain MSA=%.2f\n",
          scpolasa, scnpasa, scpolmsa, scnpmsa);
  fprintf(resfile,
          "+charge ASA=%.2f  -charge ASA=%.2f   \n+charge MSA=%.2f  -charge "
          "MSA=%.2f\n",
          scplus, scminus, scmplus, scmminus);
  fprintf(resfile, "Structure contains %d cavities\n\n", cavn - 1);
  fclose(resfile);

  // printf("\n\nThe solvent accessible atomic areas are saved in the file
  // *.txt"); printf("\n\nThe breakdown of surface areas is in the file
  // result.txt");

  // printf("\n\nPress <Enter> to quit");

  // getchar();

  /*the end of all calculations */
  /*freeing the dynamic memory */
  free(ver);
  free(verat);
  free(cir);
  free(cirat);
  free(edge);
  free(edgeatom);
  free(aarea);
  free(ar);
  free(veflag);
  free(molarea);
  free(curvat);

#endif
  return 0;
}
