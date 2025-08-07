#ifndef __EXAMPLE_H__
#define __EXAMPLE_H__

#include <math.h>
#include <qwt3d_function.h>
#include <qwt3d_parametricsurface.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "../include/us_util.h"

// For some reson WIN32 is not picking up M_PI from math.h...

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


using namespace std;
using namespace Qwt3D;

class C_s_k : public Function {
   public:
      SA2d_control_variables *sa2d_ctrl_vars;
      vector<struct element_3D> solutes;

      C_s_k(SA2d_control_variables *tmp_sa2d_ctrl_vars, vector<struct element_3D> tmp_solutes, SurfacePlot &pw) :
          Function(pw) {
         sa2d_ctrl_vars = tmp_sa2d_ctrl_vars;
         solutes = tmp_solutes;
         setMesh((*sa2d_ctrl_vars).meshx, (*sa2d_ctrl_vars).meshy);
         setDomain((*sa2d_ctrl_vars).minx, (*sa2d_ctrl_vars).maxx, (*sa2d_ctrl_vars).miny, (*sa2d_ctrl_vars).maxy);
         setMinZ(0.0);
      }
      ~C_s_k() {}

      double operator()(double x, double y) {
         // add distance, concentration from list
         unsigned int i;
         double retval = 0e0, dist;
         for (i = 0; i < solutes.size(); i++) {
            dist = sqrt(((x - solutes[ i ].x) * (x - solutes[ i ].x)) + ((y - solutes[ i ].y) * (y - solutes[ i ].y)));
            if (dist <= (*sa2d_ctrl_vars).beta) {
               retval += solutes[ i ].z * pow(cos(dist * M_PI / ((*sa2d_ctrl_vars).beta * 2)), (*sa2d_ctrl_vars).alpha);
            }
         }
         return (retval * (*sa2d_ctrl_vars).zscaling);
      }
};

//void createCone(Qwt3D::TripleField& conepos, Qwt3D::CellField& conecell);

#endif
