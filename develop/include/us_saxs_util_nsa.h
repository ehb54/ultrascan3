#ifndef US_SAXS_UTIL_NSA_H
#define US_SAXS_UTIL_NSA_H

class nsa_ga_individual
{
 public:
   vector < double > v;
   double            fitness;
   bool operator < (const nsa_ga_individual & objIn) const
   {
      return fitness < objIn.fitness;
   }

   bool operator == (const nsa_ga_individual & objIn) const
   {
      return v == objIn.v;
   }
   bool operator != (const nsa_ga_individual & objIn) const
   {
      return v != objIn.v;
   }
};

#endif
