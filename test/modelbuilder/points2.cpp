#include "points2.h"
#include "RegularGrid.h"

point::point() {
    point(0e0);
}

point::point(double v, int id) {
    x.resize(DIM);
    for (int i = 0; i < DIM; ++i) {
        x[ i ] = v;
    }
    this->id = id;
}

point::point(vector < double > x, int id) {
    this->x = x;
    this->x.resize(DIM);
    this->id = id;
}

point::point(double *x, int id) {
    this->x.resize(DIM);
    for (int i = 0; i < DIM; ++i) {
        this->x[ i ] = x[ i ];
    }
    this->id = id;
}

point::point(QVector2D *targetPoint, int id) //NOTE: only for use when DIM = 2
{
    this->x.resize(DIM);
    this->x[0] = targetPoint->x();
    this->x[1] = targetPoint->y();
    this->id = id;
    
    delete targetPoint;
}

/*static*/ point point::point_min(const point& x, const point& y) {
    point z = x;
    for (int i = 0; i < DIM; ++i) {
        if (z.x[ i ] > y.x[ i ]) {
            z.x[ i ] = y.x[ i ];
        }
    }
    return z;
}

/*static*/ point point::point_max(const point& x, const point& y) {
    point z = x;
    for (int i = 0; i < DIM; ++i) {
        if (z.x[ i ] < y.x[ i ]) {
            z.x[ i ] = y.x[ i ];
        }
    }
    return z;
}

/*static*/ double point::mag2(const point& x) {
    double mag2 = 0e0;
    for (int i = 0; i < DIM; ++i) {
        mag2 += x.x[ i ] * x.x[ i ];
    }
    return mag2;
}

/*static*/ double point::r2(const point& x, const point& y) {
    double r2 = 0;
    for (int i = 0; i < DIM; ++i) {
        double tmp = x.x[ i ] - y.x[ i ];
        r2 += tmp * tmp;
    }
    return r2;
}

/*static*/ double point::r3(const point& x, const point& y) {
    double r3 = 0;
    for (int i = 0; i < DIM; ++i) {
        double tmp = fabs(x.x[ i ] - y.x[ i ]);
        r3 += tmp * tmp * tmp;
    }
    return r3;
}

/*static*/ double point::r6(const point& x, const point& y) {
    double r6 = 0;
    for (int i = 0; i < DIM; ++i) {
        double tmp = x.x[ i ] - y.x[ i ];
        r6 += tmp * tmp * tmp * tmp * tmp * tmp;
    }
    return r6;
}

point point::operator + (const point& x) {
    point z = *this;
    for (int i = 0; i < DIM; ++i) {
        z.x[ i ] += x.x[ i ];
    }
    return z;
}

point point::operator - (const point& x) {
    point z = *this;
    for (int i = 0; i < DIM; ++i) {
        z.x[ i ] -= x.x[ i ];
    }
    return z;
}

point point::operator*(const double &y) {
    point z = *this;
    for (int i = 0; i < DIM; ++i) {
        z.x[ i ] *= y;
    }
    return z;
}

point point::operator / (const double &y) {
    point z = *this;
    for (int i = 0; i < DIM; ++i) {
        z.x[ i ] /= y;
    }
    return z;
}

bool point::operator<(const point &x) const {
    return this->id < x.id;
}

bool point::operator == (const point &x) const {
    return this->id == x.id;
}

bool point::operator != (const point &x) const {
    return this->id != x.id;
}

ostream& operator << (ostream& os, const point& x) {
    os << x.id << ":[";
    int i = 0;
    os << x.x[ i ];
    for (i = 1; i < DIM; ++i) {
        os << "," << x.x[ i ];
    }
    os << "]";
    return os;
}

ostream& operator << (ostream& os, const set < point > & s) {
    for (set < point >::iterator it = s.begin();
            it != s.end();
            ++it) {
        os << *it << endl;
    }
    os << endl;
    return os;
}

/*
grid::grid( 
           point     start                 // starting corner
           ,point     end                  // ending corner
           ,point     pts                  // points in each direction for initial grid
           ,double    (*charge)( point )   // provided function which returns the charge of a point
           ,double    deltat               // delta t
           ,int       neighbour_points     // maximum nearest neighbour points, 0 for no nearest neighbour 
           ,int       neighbour_steps      // number of delta t steps before recomputing neighbour 
            ) {

   this->deltat             = deltat;
   this->charge             = charge;
   this->neighbour_points   = neighbour_points;
   this->neighbour_steps    = neighbour_steps;
          
   // add the corners
   {
      pmin = point::point_min( start, end );
      pmax = point::point_max( start, end );
      pwidth = pmax - pmin;

      cout << "pmin: " << pmin << endl;
      cout << "pmax: " << pmax << endl;
      cout << "pwidth: " << pwidth << endl;

      int pts_to_add = (int) pow( 2, DIM );

      for ( int i = 0; i < pts_to_add; ++i ) {
         point p   = pmin;
         int  pos  = 0;
         for ( int s = i;
               s;
               s = s >> 1, 
               ++pos ) {
            if ( s & 1 ) {
               p.x[ pos ] = pmax.x[ pos ];
            }
         }
         p.id = (int) corners.size();
         corners.insert( p );
      }
   }

   pgrid = corners;

   // build an n-dimensional grid
   {
      map < int, vector < double > > pdpoints;

      int totalpts = 1;

      for ( int i = 0; i < DIM; ++i ) {
         if ( pts.x[ i ] < 2 ) {
            cerr << "too few grid points ( " << pts.x[ i ] << " ) in dimension " << i << endl;
         } else {
            totalpts *= pts.x[ i ];

            point p = pmin;

            double s = pmin.x[ i ];
            pdpoints[ i ].push_back( s );
            double e = pmax.x[ i ];
            double spacing = ( e - s ) / ( pts.x[ i ] - 1 );
            for ( int j = 1; j < pts.x[ i ] - 1; ++j ) {
               p.x[ i ] = s + j * spacing;
               pdpoints[ i ].push_back( p.x[ i ] );
            }
            pdpoints[ i ].push_back( e );
         }
      }

      cout << "total points " << totalpts << endl;

      for ( int i = 0; i < totalpts; ++i ) {
         point p = pmin;
         int j = 0;
         int div = i;
         while ( div ) {
            int mod = div % (int) pts.x[ j ];
            p.x[ j ] = pdpoints[ j ][ mod ];
            div /= (int) pts.x[ j ];
            ++j;
         }
         p.id = pgrid.size();
         pgrid.insert( p );
      }

      // compute edges

      for ( set < point >::iterator it = pgrid.begin();
            it != pgrid.end();
            ++it ) {
         for ( int i = 0; i < DIM; ++i ) {
            if ( (*it).x[ i ] == pmin.x[ i ] ||
                 (*it).x[ i ] == pmax.x[ i ] ) {
               edges.insert( *it );
            }
         }
      }

      // duplicate boxs

      cerr << "duplicate box" << endl << flush;

      {
         set < point > additional_grid;
         set < point > grid_to_copy;

         for ( set < point >::iterator it = pgrid.begin();
               it != pgrid.end();
               ++it ) {
            grid_to_copy.insert( *it );
         }

         int total_adds = (int) pow( 3, DIM );
         // +/0/- each dimension in combo & subtract 1 for 0/0/0
         // so total number of boxes to add is 3^n - 1

         cout << "total boundry boxes to add " << ( total_adds - 1 ) << endl;

         for ( int i = 0; i < total_adds; ++i ) {
            vector < int > choice( DIM );
            int div = i;
            for ( int j = 0; j < DIM; ++j ) {
               int mod = div % 3;
               choice[ j ] = div % 3 - 1;
               div /= 3;
            }

            point offset( 0e0 );

            bool noop = true;

            for ( int j = 0; j < DIM; ++j ) {
               switch( choice[ j ] ) {
               case -1 : 
                  offset.x[ j ] = -pwidth.x[ j ];
                  noop = false;
                  break;
               case 1 : 
                  offset.x[ j ] = pwidth.x[ j ];
                  noop = false;
                  break;
               default :
                  break;
               }
            }

            if ( !noop ) {
               cout << "adding box offset " << offset << endl;
               for ( set < point >::iterator it = grid_to_copy.begin();
                     it != grid_to_copy.end();
                     ++it ) {
                  point p = *it;
                  p.id = pgrid.size();
                  pgrid.insert( p + offset );
                  edges.insert( p + offset );
               }
            }
         }
      }

      if ( neighbour_points ) {
         recompute_neighbours( 6 );
         trim_edges_not_neighbours();
      }
   }

   cout << "minimum: " << pmin << endl;
   cout << "maximum: " << pmax << endl;

   cout << "points:\n" << pts << endl;

   // cout << "corners:\n" << corners;

   // cout << "edges:\n" << edges;

   // cout << "pgrid:\n" << pgrid;
}*/

grid::grid(RegularGrid* rg, point pts, double deltat, int neighbour_points, int neighbour_steps) {
    this->calculatedGrid = rg;
    this->deltat = deltat;
    this->neighbour_points = neighbour_points;
    this->neighbour_steps = neighbour_steps;

    int grid_X_dim = calculatedGrid->getGrid()->size();
    int grid_Y_dim = calculatedGrid->getGrid()->first()->size();
    double startCoords[] = {calculatedGrid->getGrid()->at(1)->at(1)->x(), calculatedGrid->getGrid()->at(1)->at(1)->y()};
    double endCoords[] = {calculatedGrid->getGrid()->at(grid_X_dim - 2)->at(grid_Y_dim - 2)->x()
        , calculatedGrid->getGrid()->at(grid_X_dim - 2)->at(grid_Y_dim - 2)->y()};

    point start(startCoords);
    point end(endCoords);

    // add the corners
    {
        pmin = point::point_min(start, end);
        pmax = point::point_max(start, end);
        pwidth = pmax - pmin;

        cout << "pmin: " << pmin << endl;
        cout << "pmax: " << pmax << endl;
        cout << "pwidth: " << pwidth << endl;

        int pts_to_add = (int) pow(2, DIM);

        for (int i = 0; i < pts_to_add; ++i) {
            point p = pmin;
            int pos = 0;
            for (int s = i;
                    s;
                    s = s >> 1,
                    ++pos) {
                if (s & 1) {
                    p.x[ pos ] = pmax.x[ pos ];
                }
            }
            p.id = (int) corners.size();
            corners.insert(p);
        }
    }

    pgrid = corners;

    // build an n-dimensional grid
    {
        map < int, vector < double > > pdpoints;

        int totalpts = 1;

        for (int i = 0; i < DIM; ++i) {
            if (pts.x[ i ] < 2) {
                cerr << "too few grid points ( " << pts.x[ i ] << " ) in dimension " << i << endl;
            } else {
                totalpts *= pts.x[ i ];

                point p = pmin;

                double s = pmin.x[ i ];
                pdpoints[ i ].push_back(s);
                double e = pmax.x[ i ];
                double spacing = (e - s) / (pts.x[ i ] - 1);
                for (int j = 1; j < pts.x[ i ] - 1; ++j) {
                    p.x[ i ] = s + j * spacing;
                    pdpoints[ i ].push_back(p.x[ i ]);
                }
                pdpoints[ i ].push_back(e);
            }
        }

        cout << "total points " << totalpts << endl;

        for (int i = 0; i < totalpts; ++i) {
            point p = pmin;
            int j = 0;
            int div = i;
            while (div) {
                int mod = div % (int) pts.x[ j ];
                p.x[ j ] = pdpoints[ j ][ mod ];
                div /= (int) pts.x[ j ];
                ++j;
            }
            p.id = pgrid.size();
            pgrid.insert(p);
        }

        // compute edges

        for (set < point >::iterator it = pgrid.begin();
                it != pgrid.end();
                ++it) {
            for (int i = 0; i < DIM; ++i) {
                if ((*it).x[ i ] == pmin.x[ i ] ||
                        (*it).x[ i ] == pmax.x[ i ]) {
                    edges.insert(*it);
                }
            }
        }

        // duplicate boxes

        cerr << "duplicate box" << endl << flush;

        {
            set < point > additional_grid;
            set < point > grid_to_copy;

            for (set < point >::iterator it = pgrid.begin();
                    it != pgrid.end();
                    ++it) {
                grid_to_copy.insert(*it);
            }

            int total_adds = (int) pow(3, DIM);
            // +/0/- each dimension in combo & subtract 1 for 0/0/0
            // so total number of boxes to add is 3^n - 1

            cout << "total boundry boxes to add " << (total_adds - 1) << endl;

            for (int i = 0; i < total_adds; ++i) {
                vector < int > choice(DIM);
                int div = i;
                for (int j = 0; j < DIM; ++j) {
                    int mod = div % 3;
                    choice[ j ] = div % 3 - 1;
                    div /= 3;
                }

                point offset(0e0);

                bool noop = true;

                for (int j = 0; j < DIM; ++j) {
                    switch (choice[ j ]) {
                        case -1:
                            offset.x[ j ] = -pwidth.x[ j ];
                            noop = false;
                            break;
                        case 1:
                            offset.x[ j ] = pwidth.x[ j ];
                            noop = false;
                            break;
                        default:
                            break;
                    }
                }

                if (!noop) {
                    cout << "adding box offset " << offset << endl;
                    for (set < point >::iterator it = grid_to_copy.begin();
                            it != grid_to_copy.end();
                            ++it) {
                        point p = *it;
                        p.id = pgrid.size();
                        pgrid.insert(p + offset);
                        edges.insert(p + offset);
                    }
                }
            }
        }

        if (neighbour_points) {
            recompute_neighbours(6);
            trim_edges_not_neighbours();
        }
    }

    cout << "minimum: " << pmin << endl;
    cout << "maximum: " << pmax << endl;

    cout << "points:\n" << pts << endl;

    // cout << "corners:\n" << corners;

    // cout << "edges:\n" << edges;

    // cout << "pgrid:\n" << pgrid;
}

/*grid::grid( RegularGrid* rg, double deltat, int neighbour_points, int neighbour_steps)
{
   calculatedGrid = rg;
   
   this->deltat             = deltat;
   this->neighbour_points   = neighbour_points;
   this->neighbour_steps    = neighbour_steps;
   
   double startCoords[] = {calculatedGrid->getGrid()->first()->first()->x(), calculatedGrid->getGrid()->first()->first()->y()};
   double endCoords[] = {calculatedGrid->getGrid()->last()->last()->x(), calculatedGrid->getGrid()->last()->last()->y()};
   
   point start(startCoords);
   point end(endCoords);
          
   // add the corners
   {
      pmin = point::point_min( start, end );
      pmax = point::point_max( start, end );
      pwidth = pmax - pmin;

      cout << "pmin: " << pmin << endl;
      cout << "pmax: " << pmax << endl;
      cout << "pwidth: " << pwidth << endl;

      int pts_to_add = (int) pow( 2, DIM );

      for ( int i = 0; i < pts_to_add; ++i ) {
         point p   = pmin;
         int  pos  = 0;
         for ( int s = i;
               s;
               s = s >> 1, 
               ++pos ) {
            if ( s & 1 ) {
               p.x[ pos ] = pmax.x[ pos ];
            }
         }
         p.id = (int) corners.size();
         corners.insert( p );
      }
   }

   pgrid = corners;

   // build an n-dimensional grid
   {
      map < int, vector < double > > pdpoints;

      int totalpts = 1;

      for(int r = 0; r < calculatedGrid->getGrid()->size(); r++)
      {
        for(int c = 0; c < calculatedGrid->getGrid()->at(r)->size(); c++)
        {
                QVector3D* original = calculatedGrid->getGrid()->at(r)->at(c);
                double coords[] = {original->x(), original->y()};
      		
                point p(coords);
                cerr << "added: " << original->x() << ", " << original->y() << ", " << original->z() << " to pgrid";
                pgrid.insert(p);
        }
      }

      // compute edges

      for ( set < point >::iterator it = pgrid.begin();
            it != pgrid.end();
            ++it ) {
         for ( int i = 0; i < DIM; ++i ) {
            if ( (*it).x[ i ] == pmin.x[ i ] ||
                 (*it).x[ i ] == pmax.x[ i ] ) {
               edges.insert( *it );
            }
         }
      }

      // duplicate boxs

      cerr << "duplicate box" << endl << flush;

      {
         set < point > additional_grid;
         set < point > grid_to_copy;

         for ( set < point >::iterator it = pgrid.begin();
               it != pgrid.end();
               ++it ) {
            grid_to_copy.insert( *it );
         }

         int total_adds = (int) pow( 3, DIM );
         // +/0/- each dimension in combo & subtract 1 for 0/0/0
         // so total number of boxes to add is 3^n - 1

         cout << "total boundry boxes to add " << ( total_adds - 1 ) << endl;

         for ( int i = 0; i < total_adds; ++i ) {
            vector < int > choice( DIM );
            int div = i;
            for ( int j = 0; j < DIM; ++j ) {
               int mod = div % 3;
               choice[ j ] = div % 3 - 1;
               div /= 3;
            }

            point offset( 0e0 );

            bool noop = true;

            for ( int j = 0; j < DIM; ++j ) {
               switch( choice[ j ] ) {
               case -1 : 
                  offset.x[ j ] = -pwidth.x[ j ];
                  noop = false;
                  break;
               case 1 : 
                  offset.x[ j ] = pwidth.x[ j ];
                  noop = false;
                  break;
               default :
                  break;
               }
            }

            if ( !noop ) {
               cout << "adding box offset " << offset << endl;
               for ( set < point >::iterator it = grid_to_copy.begin();
                     it != grid_to_copy.end();
                     ++it ) {
                  point p = *it;
                  p.id = pgrid.size();
                  pgrid.insert( p + offset );
                  edges.insert( p + offset );
               }
            }
         }
      }

      if ( neighbour_points ) {
         recompute_neighbours( 6 );
         trim_edges_not_neighbours();
      }
   }

   cout << "minimum: " << pmin << endl;
   cout << "maximum: " << pmax << endl;

   //cout << "points:\n" << pts << endl;

   // cout << "corners:\n" << corners;

   // cout << "edges:\n" << edges;

   // cout << "pgrid:\n" << pgrid;
}*/

point grid::F(point p) {
    point F(0e0);

    double this_charge = charge(p);
    
    //cout << "point:" << p << " charge: " << this_charge << endl;

    if (neighbour_points) {
        for (set < point >::iterator it = neighbours[ p ].begin();
                it != neighbours[ p ].end();
                ++it) {
            F = F + ((p - *it) * this_charge * charge(*it)) / point::r3(p, *it);
        }
    } else {
        for (set < point >::iterator it = pgrid.begin();
                it != pgrid.end();
                ++it) {
            if (*it != p) {
                F = F + ((p - *it) * this_charge * charge(*it)) / point::r3(p, *it);
            }
        }
    }

    return F;
}


/*bool grid::write_pgrid( string filename ) {
   ofstream file;
   file.open ( filename.c_str() );
   if ( !file.is_open() ) {
      return false;
   }

   for ( set < point >::iterator it = pgrid.begin();
         it != pgrid.end();
         ++it ) {
      for ( int i = 0; i < DIM; ++i ) {
         file << (*it).x[ i ] << " ";
      }
      file << endl;
   }

   file.close();
   return true;
}*/


//write only points that are not edges
bool grid::write_pgrid(string filename) {
    ofstream file;
    file.open(filename.c_str());
    if (!file.is_open()) {
        return false;
    }

    for (set < point >::iterator it = pgrid.begin();
            it != pgrid.end();
            ++it) {

        if (edges.find(*it) == edges.end()) {
            for (int i = 0; i < DIM; ++i) {
                file << (*it).x[ i ] << " ";
            }
        }
        file << endl;
    }

    file.close();
    return true;
}

void grid::trim_edges_not_neighbours() {
    set < point > used_neighbours;

    for (map < point, set < point > >::iterator it = neighbours.begin();
            it != neighbours.end();
            ++it) {
        for (set < point >::iterator it2 = it->second.begin();
                it2 != it->second.end();
                ++it2) {
            used_neighbours.insert(*it2);
        }
    }

    set < point > new_pgrid;

    for (set < point >::iterator it = pgrid.begin();
            it != pgrid.end();
            ++it) {
        if (used_neighbours.count(*it)) {
            new_pgrid.insert(*it);
        } else {
            if (edges.count(*it)) {
                edges.erase(*it);
            }
        }
    }
    pgrid = new_pgrid;
}

#ifdef THREADS

struct thread_arg {
    int offset;
    int increment;
    int use_neighbour_points;
    set < point > * pgrid;
    set < point > * edges;
    map < point, set < point > > * neighbours;
};

void * neighbour_thread(void * arg) {
    int offset = ((thread_arg *) arg)->offset;
    int increment = ((thread_arg *) arg)->increment;
    int use_neighbour_points = ((thread_arg *) arg)->use_neighbour_points;
    set < point > * pgrid = ((thread_arg *) arg)->pgrid;
    set < point > * edges = ((thread_arg *) arg)->edges;
    map < point, set < point > > * neighbours = ((thread_arg *) arg)->neighbours;

    // cout << "hello from thread " << offset << " increment " << increment << " pgrid.size() " << (*pgrid).size() << endl << flush;

    set < point >::iterator it = (*pgrid).begin();

    for (int i = 0; i < offset; ++i) {
        it++;
    }

    while (it != (*pgrid).end()) {
        if (!(*edges).count(*it)) {
            map < double, vector < point > > dist_map;

            for (set < point >::iterator it2 = (*pgrid).begin();
                    it2 != (*pgrid).end();
                    ++it2) {
                if (*it != *it2) {
                    dist_map[ point::r2(*it, *it2) ].push_back(*it2);
                }
            }
            int count = 0;
            for (map < double, vector < point > >::iterator it2 = dist_map.begin();
                    it2 != dist_map.end();
                    ++it2) {
                for (int i = 0; i < (int) it2->second.size(); ++i) {
                    if (count++ > use_neighbour_points) {
                        break;
                    }
                    (*neighbours)[ *it ].insert(it2->second[ i ]);
                }
                if (count > use_neighbour_points) {
                    break;
                }
            }
        }
        for (int i = 0; i < increment; ++i) {
            if (it != (*pgrid).end()) {
                it++;
            }
        }
    }
    // cout << "thread " << offset << " neighbours recomputed" << endl << flush;

    return NULL;
}

void grid::recompute_neighbours(double mult) {
    neighbours.clear();

    int use_neighbour_points = mult ? (int) (mult * (double) neighbour_points) : neighbour_points;

    cout << "recomputing threaded neighbours points: " << use_neighbour_points << endl;

    vector < map < point, set < point > > > thread_neighbours(THREADS);

    pthread_t threads[ THREADS ];
    thread_arg args [ THREADS ];

    for (int i = 0; i < THREADS; ++i) {
        args[ i ].offset = i;
        args[ i ].increment = THREADS;
        args[ i ].use_neighbour_points = use_neighbour_points;
        args[ i ].pgrid = &pgrid;
        args[ i ].edges = &edges;
        args[ i ].neighbours = &(thread_neighbours[ i ]);

        if (pthread_create(&(threads[ i ]), NULL, neighbour_thread, &(args[ i ]))) {
            cerr << "thread " << i << " create error\n";
        }
    }

    // maybe do work here

    // join threads

    for (int i = 0; i < THREADS; ++i) {
        if (pthread_join(threads[ i ], NULL)) {
            cerr << "thread " << i << " join error\n";
        }
    }

    // join neighbours together

    neighbours = thread_neighbours[ 0 ];

    for (int i = 1; i < THREADS; ++i) {
        for (map < point, set < point > >::iterator it = thread_neighbours[ i ].begin();
                it != thread_neighbours[ i ].end();
                ++it) {
            neighbours[ it->first ] = it->second;
        }
    }
    cout << "neighbours recomputed" << endl;

    // for ( map < point, set < point > >::iterator it = neighbours.begin();
    //       it != neighbours.end();
    //       ++it ) {
    //    cout << "neighbours for " << it->first << " are " << it->second << endl;
    // }
}
#else   

void grid::recompute_neighbours(double mult) {
    neighbours.clear();

    int use_neighbour_points = mult ? (int) (mult * (double) neighbour_points) : neighbour_points;

    cout << "recomputing neighbours points: " << use_neighbour_points << endl;

    for (set < point >::iterator it = pgrid.begin();
            it != pgrid.end();
            ++it) {
        if (!edges.count(*it)) {
            map < double, vector < point > > dist_map;

            for (set < point >::iterator it2 = pgrid.begin();
                    it2 != pgrid.end();
                    ++it2) {
                if (*it != *it2) {
                    dist_map[ point::r2(*it, *it2) ].push_back(*it2);
                }
            }
            int count = 0;
            for (map < double, vector < point > >::iterator it2 = dist_map.begin();
                    it2 != dist_map.end();
                    ++it2) {
                for (int i = 0; i < (int) it2->second.size(); ++i) {
                    if (count++ > use_neighbour_points) {
                        break;
                    }
                    neighbours[ *it ].insert(it2->second[ i ]);
                }
                if (count > use_neighbour_points) {
                    break;
                }
            }
            // cout << "neighbours for " << *it << " are computed" << endl;
            // cout << "neighbours for " << *it << " are: " << neighbours[ *it ] << endl;
        }
    }

    cout << "neighbours recomputed" << endl;

    // for ( map < point, set < point > >::iterator it = neighbours.begin();
    //       it != neighbours.end();
    //       ++it ) {
    //    cout << "neighbours for " << it->first << " are " << it->second << endl;
    // }
}
#endif

void grid::run(int steps, bool do_write, QwtPlot* grid_display) {
    //declare display point curve and data structures
    QwtPlotCurve* curve = NULL;
    double* dim_0_values;
    double* dim_1_values;
    
    if(grid_display != NULL) {
      //clear display
      curve->setData(QwtArray<QwtDoublePoint>());
      
      curve = new QwtPlotCurve("E-Min Points");
      curve->setStyle(QwtPlotCurve::NoCurve);
      
      QwtSymbol sym;
      sym.setStyle(QwtSymbol::Ellipse);
      sym.setSize(QSize(5, 5));
      curve->setSymbol(sym);
      
      curve->attach(grid_display);
    }
  
    // compute forces
    map < point, point > Fi;

    for (int i = 0; i < steps; ++i) {
        // compute forces

        if (neighbour_points &&
                (!neighbour_steps ||
                !(i % neighbour_steps))) {
            recompute_neighbours();
        }

        if (do_write) {
            ostringstream oss;
            oss << "out/pgrid_" << i << ".txt";
            write_pgrid(oss.str());
            cout << "wrote " << oss.str() << endl;
        }

        cout << "step " << i << endl;
        // cout << "pgrid:\n" << pgrid << endl;

        cout << "forces:\n";
        double totmag2 = 0e0;
        for (set < point >::iterator it = pgrid.begin();
                it != pgrid.end();
                ++it) {
            if (!edges.count(*it)) {
		// Assign force, I think
                Fi[ *it ] = F(*it);
		
                //cout << "F:" << *it << " = " << Fi[ *it ] << " mag2:" << point::mag2( Fi[ *it ] ) <<  endl;
		 
                totmag2 += point::mag2(Fi[ *it ]);
            }
        }
        cout << "total mag2 force: " << totmag2 << endl;

        // move points
	
        {
            set < point > new_pgrid;

            for (set < point >::iterator it = pgrid.begin();
                    it != pgrid.end();
                    ++it) {
		//case for points that are not the fixed repulsion edge
                //if (!edges.count(*it)) {
		if (edges.find(*it) == edges.end()) {
                    point porg = *it;
		    
		    //new_pgrid.insert(porg + Fi[ *it ] * deltat); //old insertion code
		    
		    if(!in_bounds(porg))
		      cout << porg << " is out of bounds!" << endl;
		    
		    point vector = Fi[ *it ] * deltat; //precompute vector
		    point direction = check_bounds(porg + vector);
		    
		    //cout << "Point: " << porg << " - Force vector: " << vector << endl;
		    
		    //check if new point is in bounds
		    if(in_bounds(porg + vector)) {
			new_pgrid.insert(porg + vector);
		    }
		    //otherwise, don't move TODO: Ensure that points cannot go out of bounds
		    else {
			point npt = do_reflect(porg, vector, direction);
			
			if(!in_bounds(npt))
			  cout << "New point is not in bounds!" << endl;
			
			new_pgrid.insert(npt);
		    }
                }
                
                //case for points that comprise the fixed edge 
                else {
                    new_pgrid.insert(*it);
                }
            }
            pgrid = new_pgrid;
        }
        
        //if needed, update plot
	if(grid_display != NULL) {
	  //curve->setData(QwtArray<QwtDoublePoint>());
	  
	  dim_0_values = get_dim_values(0);
	  dim_1_values = get_dim_values(1);
	  curve->setData(dim_0_values, dim_1_values, pgrid.size());
	  delete dim_0_values;
	  delete dim_1_values;
	  
	  grid_display->replot();
	}
    }
}

//directional out of bounds detection; in-bounds = origin (0, 0)
point grid::check_bounds(point p) {
  int x = 0;
  int y = 0;
  point quadrant(0e0);
  
  //get direction for x-axis
  if(p.x[0] > pmax.x[0])
    x = 1;
  else if(p.x[0] < pmin.x[0])
    x = -1;
  
  //get direction for y-axis
  if(p.x[1] > pmax.x[1])
    y = 1;
  else if(p.x[1] < pmin.x[1])
    y = -1;
  
  //setup point for return
  quadrant.x[0] = x;
  quadrant.x[1] = y;
  
  //return
  return quadrant;
}

point grid::do_reflect(point p, point vector, point dir){
  //extract deltax and deltay
  double delta_x = vector.x[0];
  double delta_y = vector.x[1];
  
  point new_vector(0e0);
  
  //do vector reflection math, reducing magnitude
  if(dir.x[0] != 0.0)
    delta_x = -delta_x * .5;
  if(dir.x[1] != 0.0)
    delta_y = -delta_y * .5;
  
  //fill vector
  new_vector.x[0] = delta_x;
  new_vector.x[1] = delta_y;
  
  //generate result point
  point result = (p + new_vector);
  
  //check if in bounds
  if(in_bounds(result))
      return result;
  else // not in bounds
  {
      return do_reflect(p, new_vector, dir);
  }
}

//n-dimensional boolean out of bounds detection function; disabled to allow for directional 2d check
bool grid::in_bounds(point p) {
    //check all dimensions
    for (int i = 0; i < DIM; ++i) {
        //if any values at any dimension exceed bounds, return false
        if (p.x[i] < pmin.x[i] || p.x[i] > pmax.x[i])
            return false;
    }

    //otherwise, presume that value is in bounds
    return true;
}

double grid::charge(point p) {
    //return 1;
    
    //get RMSD value
    double interpolated = calculatedGrid->interpolate(p);
    return interpolated;
}

double* grid::get_dim_values(int dim) {
  //create double vector with length of pgrid
  double* values = new double[pgrid.size()];
  int index = 0;
  
  //iterate over pgrid
  for(set < point >::iterator it = pgrid.begin();
                it != pgrid.end();
                ++it) {
    if (edges.find(*it) == edges.end()) {
      values[index++] = it->x[dim];
    }
  }
  
  return values;
}
