#include "../include/us_saxs_util.h"

static unsigned int sgp_creates;
static unsigned int sgp_deletes;

sgp_node::sgp_node()
{
   sgp_creates++;
   children         .clear();
   parent           = ( sgp_node * ) 0;
   normal.axis[ 0 ] = 0.0;
   normal.axis[ 1 ] = 0.0;
   normal.axis[ 2 ] = 0.0;
   distance         = 0;
   radius           = 0;
}

sgp_node::sgp_node( point normal, unsigned int distance, unsigned int radius )
{
   sgp_creates++;
   children         .clear();
   parent           = ( sgp_node * ) 0;
   this->normal     = normal;
   this->distance   = distance;
   this->radius     = radius;
}

sgp_node::~sgp_node()
{
   sgp_deletes++;

   // cout << QString( "in deconstructor: has parent %1, children size %2, distance %3, radius %4\n" )
   //      .arg( parent ? "yes" : "no" )
   // .arg( children.size() )
   // .arg( distance )
   // .arg( radius );

   list < sgp_node * >::iterator it = children.begin();

   while ( ( it = children.begin() ) != children.end() )
   {
      // cout << "deleting children:\n";
      delete *it;
      // cout << "deleting children iterator increment:\n";
   }
         
   if ( parent )
   {
      // cout << "removing from parent:\n";
      parent->children.remove( this );
   }

}

QString sgp_node::contents( bool include_children )
{
   QString qs;

   qs += QString( "sgp: depth %1 distance %2 radius %3 normal [%4,%5,%6]\n" )
      .arg( depth() )
      .arg( distance )
      .arg( radius )
      .arg( normal.axis[ 0 ] )
      .arg( normal.axis[ 1 ] )
      .arg( normal.axis[ 2 ] );

   if ( include_children )
   {
      for ( list < sgp_node * >::iterator it = children.begin();
            it != children.end();
            it++ )
      {
         qs += (*it)->contents();
      }
   }

   return qs;
}

unsigned int sgp_node::size()
{
   unsigned int i = 1;
   
   for ( list < sgp_node * >::iterator it = children.begin();
         it != children.end();
         it++ )
   {
      i += (*it)->size();
   }
   
   return i;
}

unsigned int sgp_node::depth()
{
   unsigned int i = 0;

   sgp_node* tmp_sgp = parent;
   
   while ( tmp_sgp )
   {
      i++;
      tmp_sgp = tmp_sgp->parent;
   }

   return i;
}

sgp_node * sgp_node::ref( unsigned int pos )
{
   // cout << QString( "ref called pos %1 size %2\n" ).arg( pos ).arg( size() );
   if ( pos >= size() )
   {
      return ( sgp_node * ) 0;
   }

   if ( !pos )
   {
      // cout << QString( "return this\n" );
      return this;
   }

   pos--;

   for ( list < sgp_node * >::iterator it = children.begin();
         it != children.end();
         it++ )
   {
      // cout << QString( "going through children, pos now %1\n" ).arg( pos );
      if ( (*it)->size() > pos )
      {
         return (*it)->ref( pos );
      }
      pos -= (*it)->size();
   }

   // shouldn't get here 
   cout << QString( "Internal error: sgp_node::returning 0 at end\n" );
   return ( sgp_node * ) 0;
}

bool sgp_node::prune( unsigned int pos )
{
   if ( pos >= size() )
   {
      return false;
   }

   if ( !pos )
   {
      for ( list < sgp_node * >::iterator it = children.begin();
            it != children.end();
            it++ )
      {
         delete *it;
      }

      if ( parent )
      {
         parent->children.remove( this );
      }
      return true;
   } else {
      return ref( pos )->prune( 0 );
   }
   return true;
}

sgp_node * sgp_node::copy( sgp_node * node )
{
   sgp_node *tmp_node = new sgp_node( node->normal, node->distance, node->radius );
   for ( list < sgp_node * >::iterator it = node->children.begin();
         it != node->children.end();
         it++ )
   {
      sgp_node * tmp2_node = copy( *it );
      tmp2_node->parent = tmp_node;
      tmp_node->children.push_back( tmp2_node );
   }
   return tmp_node;
}

bool sgp_node::insert_copy( unsigned int pos, sgp_node* node )
{
   if ( pos >= size() )
   {
      return false;
   }

   if ( !pos )
   {
      sgp_node* tmp_node = copy( node );
      tmp_node->parent = this;
      children.push_back( tmp_node );
      return true;
   } else {
      return ref( pos )->insert_copy( 0, node );
   }
}


// FIX THIS: change do distribution based?

sgp_node* sgp_node::random( 
                           unsigned int size,
                           unsigned int min_distance,
                           unsigned int max_distance,
                           unsigned int min_radius,
                           unsigned int max_radius
                           )
{
   if ( !size )
   {
      return ( sgp_node* ) 0;
   }

   // create base node

   point n;

   // we are going do be making lots of these, so lets make sure we don't have a zero vector:

   do {
      n.axis[ 0 ] = drand48() - 0.5;      
      n.axis[ 1 ] = drand48() - 0.5;
      n.axis[ 2 ] = drand48() - 0.5;
   } while ( fabs ( n.axis[ 0 ] ) < 1e-5 &&
             fabs ( n.axis[ 1 ] ) < 1e-5 &&
             fabs ( n.axis[ 2 ] ) < 1e-5 );

   unsigned int dist = min_distance + ( unsigned int )( drand48() * ( max_distance - min_distance ) );
   unsigned int rad  = min_radius   + ( unsigned int )( drand48() * ( max_radius   - min_radius   ) );
   // we could remove overlap by forcing distance to be adjacent, but we want
   // arbitrary electron densities for now

   sgp_node *new_sgp = new sgp_node( n, dist, rad );

   for ( unsigned int i = 1; i < size; i++ )
   {
      unsigned int pos = ( unsigned int )( drand48() * i );
      cout << QString( "random inserting at position %1\n" ).arg( pos );
      sgp_node *tmp_sgp = random( 1, 
                                  min_distance,
                                  max_distance,
                                  min_radius,
                                  max_radius );
      tmp_sgp->parent = new_sgp->ref( pos );
      new_sgp->ref( pos )->children.push_back( tmp_sgp );
   }
   return new_sgp;
}


point sgp_node::get_coordinate()
{
   point p1;
   if ( !parent )
   {
      p1.axis[ 0 ] = 1.0;
      p1.axis[ 1 ] = 0.0;
      p1.axis[ 2 ] = 0.0;
   } else {
      p1 = parent->normal;
   }
   if ( dot( p1, normal ) )
   {
      p1 = cross( p1, normal );
   } else {
      p1 = normal;
   }

   p1 = scale( norm( p1 ), (float) distance );
   if ( parent )
   {
      p1 = plus( p1, parent->get_coordinate() );
   } // else: rooted at 0,0,0
   return p1;
}

vector < sgp_sphere > sgp_node::sgp_spheres()
{
   vector < sgp_sphere > result;

   for ( unsigned int i = 0; i < size(); i++ )
   {
      sgp_sphere tmp_sphere;
      tmp_sphere.radius     = ref( i )->radius;
      tmp_sphere.coordinate = ref( i )->get_coordinate();
      
      result.push_back( tmp_sphere );
   }

   return result;
}

point sgp_node::cross( point p1, point p2) // p1 cross p2
{
   point result;
   result.axis[0] = p1.axis[1] * p2.axis[2] -  p1.axis[2] * p2.axis[1];
   result.axis[1] = p1.axis[2] * p2.axis[0] -  p1.axis[0] * p2.axis[2];
   result.axis[2] = p1.axis[0] * p2.axis[1] -  p1.axis[1] * p2.axis[0];
   return result;
}

float sgp_node::dot( point p1, point p2) // p1 dot p2
{
   return 
      p1.axis[0] * p2.axis[0] +
      p1.axis[1] * p2.axis[1] +
      p1.axis[2] * p2.axis[2];
}

point sgp_node::norm( point p1 )
{
   point result = p1;
   float divisor = sqrt( result.axis[0] * result.axis[0] +
                         result.axis[1] * result.axis[1] +
                         result.axis[2] * result.axis[2] );
   result.axis[0] /= divisor;
   result.axis[1] /= divisor;
   result.axis[2] /= divisor;
   return result;
}   

point sgp_node::plus( point p1, point p2 ) 
{
   point result;
   result.axis[0] = p1.axis[0] + p2.axis[0];
   result.axis[1] = p1.axis[1] + p2.axis[1];
   result.axis[2] = p1.axis[2] + p2.axis[2];
   return result;
}

point sgp_node::scale( point p, float m )
{
   for ( unsigned int i = 0; i < 3; i++ )
   {
      p.axis[i] *= m;
   }
   return p;
}

void sgp_node::test()
{
   sgp_creates = 0;
   sgp_deletes = 0;

   srand48( ( long int )QTime::currentTime().msec() );
      
   point n;

   n.axis[ 0 ] = drand48();
   n.axis[ 1 ] = drand48();
   n.axis[ 2 ] = drand48();

   sgp_node *our_sgp = new sgp_node( n, 1, 2 );

   cout << QString( "current creates %1 deletes %2\n" ).arg( sgp_creates ).arg( sgp_deletes );

   cout << "Current size(): " << our_sgp->size() << endl;
   cout << "Initial node:\n" << our_sgp->contents();
   vector < sgp_sphere > spheres = our_sgp->sgp_spheres();
   
   n.axis[ 0 ] = drand48();
   n.axis[ 1 ] = drand48();
   n.axis[ 2 ] = drand48();

   sgp_node *tmp_sgp = new sgp_node( n, 2, 3 );

   our_sgp->insert_copy( 0, tmp_sgp );

   delete tmp_sgp;

   cout << QString( "current creates %1 deletes %2\n" ).arg( sgp_creates ).arg( sgp_deletes );
   cout << "Current size(): " << our_sgp->size() << endl;
   cout << "Added one node:\n" << our_sgp->contents();

   n.axis[ 0 ] = drand48();
   n.axis[ 1 ] = drand48();
   n.axis[ 2 ] = drand48();

   tmp_sgp = new sgp_node( n, 3, 4 );
   our_sgp->insert_copy( 0, tmp_sgp );
   delete tmp_sgp;

   cout << QString( "current creates %1 deletes %2\n" ).arg( sgp_creates ).arg( sgp_deletes );
   cout << "Current size(): " << our_sgp->size() << endl;
   cout << "Added two at top level:\n" << our_sgp->contents();

   cout << "List by pos:\n";
   for ( unsigned int i = 0; i < our_sgp->size(); i++ )
   {
      cout << i << " : " << ( our_sgp->ref( i ) ? our_sgp->ref( i )->contents( false ) : "null pos\n" );
   }

   // ok, lets make a bunch of random nodes
   
   cout << "now delete\n";
   delete our_sgp;
   

   for ( unsigned int i = 0; i < 10; i++ )
   {
      cout << QString( "current creates %1 deletes %2\n" ).arg( sgp_creates ).arg( sgp_deletes );
      cout << QString( "random try %1 size %2\n" ).arg( i ).arg( i * 10 + 10 );
      our_sgp = random( i * 10 + 10, 2, 10, 10, 20 );
      cout << "Current size(): " << our_sgp->size() << endl;
      cout << "contents:\n" << our_sgp->contents();
      
      cout << "List by pos:\n";
      for ( unsigned int k = 0; k < our_sgp->size(); k++ )
      {
         cout << k << " : " << ( our_sgp->ref( k ) ? our_sgp->ref( k )->contents( false ) : "null pos\n" );
      }
      vector < sgp_sphere > spheres = our_sgp->sgp_spheres();
      for ( unsigned int j = 0; j < spheres.size(); j++ )
      {
         cout << 
            QString( "%1: [%2,%3,%4] r %5\n" )
            .arg( j )
            .arg( spheres[ j ].coordinate.axis[ 0 ] )
            .arg( spheres[ j ].coordinate.axis[ 1 ] )
            .arg( spheres[ j ].coordinate.axis[ 2 ] )
            .arg( spheres[ j ].radius );
      }

      cout <<  "trying to write\n";
      QFile f( QString( "sgp_%1.bead_model" ).arg( i ) );

      if ( f.open( IO_WriteOnly ) )
      {
         QTextStream ts( &f );
         ts << our_sgp->qs_bead_model();
         f.close();
         cout << QString( "written: %1\n" ).arg( f.name() );
      }
               
      cout << "now delete\n";
      delete our_sgp;
      cout << QString( "current creates %1 deletes %2\n" ).arg( sgp_creates ).arg( sgp_deletes );
   }
}


QString sgp_node::qs_bead_model()
{
   vector < sgp_sphere > spheres = sgp_spheres();

   QString qs;

   qs += QString( "%1 %2\n" ).arg( spheres.size() ).arg( .7 );

   for ( unsigned int i = 0; i < spheres.size(); i++ )
   {
      qs +=
         QString( "%1 %2 %3 %4  20 8 Unk 10\n")
         .arg( spheres[ i ].coordinate.axis[ 0 ] )
         .arg( spheres[ i ].coordinate.axis[ 1 ] )
         .arg( spheres[ i ].coordinate.axis[ 2 ] )
         .arg( spheres[ i ].radius );
   }
   return qs;
}
