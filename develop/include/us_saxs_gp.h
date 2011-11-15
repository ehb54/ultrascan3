#ifndef US_SAXS_GP_H
#define US_SAXS_GP_H

// structs/classes for saxs bead GP

// notable control_parameters:
// "sbgpquantum" : distance quantum in angstrom
//  all distances discretize to this
//  all sphere sizes discretize to this

// the basic tree node:

class sgp_node
{
 public:
   sgp_node            ();
   sgp_node            ( point normal, unsigned int distance, unsigned int radius );
   ~sgp_node           ();

   sgp_node *          parent;
   list < sgp_node * > children;

   point               normal;

   // these are scaled via control_parameters[ "sbgpquantum" ]:

   unsigned int        distance;
   unsigned int        radius;

   QString             contents    ( bool include_children = true );
   unsigned int        size        ();
   unsigned int        depth       ();
   sgp_node *          ref         ( unsigned int pos );
   sgp_node *          copy        ( sgp_node *node );
   bool                insert_copy ( unsigned int pos, sgp_node *node );
   bool                prune       ( unsigned int pos );

   sgp_node *          random      ( 
                                    unsigned int size,
                                    unsigned int min_distance,
                                    unsigned int max_distance,
                                    unsigned int min_radius,
                                    unsigned int max_radius
                                    );

   void                test        ();
};

#endif

