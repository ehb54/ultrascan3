#ifndef US_CONSTRAINTS_H
#define US_CONSTRAINTS_H

#include <QtCore>
#include <vector>
#include "us_femglobal.h"

using namespace std;

class US_EXTERN US_Constraints
{
   public:

   class Constraint
   {
      public:
      bool   fit;
      double low;
      double high;
   };

   class SimulationComponentConstraints
   {
      public:
      Constraint vbar20;
      Constraint mw;
      Constraint s;
      Constraint D;
      Constraint sigma;
      Constraint delta;
      Constraint concentration;
      Constraint f_f0;
   };

   class AssociationConstraints
   {
      public:
      Constraint keq;
      Constraint koff;
   };

   class ModelSystemConstraints

   {
      public:
      vector< SimulationComponentConstraints > component_constraints;
      vector< AssociationConstraints >         association_constraints;

      // Number of radial grid points used in simulation
      uint   simpoints;

      US_FemGlobal::MeshType  mesh;

      // Use moving or fixed time grid   (bool?)
      int    moving_grid;

      // loading volume (of lamella) in a band-forming centerpiece
      double band_volume;
   };

   static int read_constraints(    ModelSystem&,
                                   ModelSystemConstraints&,
                                   const QString& );

   static int read_constraints(    ModelSystem&,
                                   ModelSystemConstraints&,
                                   const QStringList& );

   static int write_constraints(   ModelSystem&,
                                   ModelSystemConstraints&,
                                   const QString& );
 




};
#endif
