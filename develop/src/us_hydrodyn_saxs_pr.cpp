#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_hydrodyn_saxs_mw.h"
#include "../include/us_hydrodyn.h"
#include <qregexp.h>

#define SLASH QDir::separator()
double US_Hydrodyn_Saxs::get_mw( QString filename, bool display_mw_msg )
{
   // enter MW and PSV
   filename = QFileInfo(filename).fileName();
   float mw = 0.0;
   bool remember = true;
   bool use_partial = false;
   QString partial = filename;
   QString msg = QString(tr(" Enter values for total molecular weight: "));
   QString source = "";
   bool found = false;
   if ( (*remember_mw).count(filename) )
   {
      mw = (*remember_mw)[filename];
      if ( (*remember_mw_source).count(filename) )
      {
         source = (*remember_mw_source)[filename];
      }
      found = true;
   } else {
      if ( !(*match_remember_mw).empty() )
      {
         // puts("dammix_match_remember not empty");
         for (map < QString, float >::iterator it = (*match_remember_mw).begin();
              it != (*match_remember_mw).end();
              it++)
         {
            // printf("iterator first %s\n", it->first.ascii());
            // printf("iterator second %f\n", it->second);
            
            if ( filename.contains(it->first) )
            {
               mw = (*match_remember_mw)[it->first];
               source = QString(tr("file name matched \"%1\"")).arg(it->first);
               found = true;
               break;
            }
         }
      }
   }
   
   if ( found ) 
   {
      // cout <<  QString("mw: %1 %2 %3\n").arg(filename).arg(mw).arg(source);
      if ( display_mw_msg )
      {
         editor->append(QString("%1 Molecular weight %2 (%3)\n")
                        .arg(filename)
                        .arg(mw)
                        .arg(source)
                        );
      }
   } else {
      US_Hydrodyn_Saxs_Mw *smw = new US_Hydrodyn_Saxs_Mw(
                                                         msg,
                                                         &mw,
                                                         &last_used_mw,
                                                         &remember,
                                                         &use_partial,
                                                         &partial,
                                                         this
                                                         );
      do {
         smw->exec();
      } while ( mw <= 0.0 );
      last_used_mw = mw;
      
      delete smw;
      this->isVisible() ? this->raise() : this->show();
      
      if ( remember ) 
      {
         (*remember_mw)[filename] = mw;
         (*remember_mw_source)[filename] = "manually entered value";
      }
      if ( use_partial ) 
      {
         (*match_remember_mw)[partial] = mw;
      }
   }
   return mw;
}

void US_Hydrodyn_Saxs::normalize_pr( vector < double > r, vector < double > *pr , double mw )
{
   if ( !our_saxs_options->normalize_by_mw )
   {
      mw = 1e0;
   }
#if defined(NORMALIZE_OLD_WAY)
   // set distribution to a 1 peak
   double max = 0e0;
   if ( pr->size() )
   {
      max = (*pr)[0];
   }
   for ( unsigned int i = 1; i < pr->size(); i++ )
   {
      if ( (*pr)[i] > max )
      {
         max = (*pr)[i];
      }
   }
   if ( max > 0e0 )
   {
      for ( unsigned int i = 0; i < pr->size(); i++ )
      {
         (*pr)[i] /= max;
      }
   }
#else
   // integrate
   // assuming regular grid:
   double area = 0e0;
   if ( pr->size() > 1 )
   {
      double gridspacing = r[1] - r[0];
      for ( unsigned int i = 0; i < pr->size(); i++ )
      {
         area += (*pr)[i] * gridspacing;
      }
      if ( area > 0e0 )
      {
         area /= mw;
         for ( unsigned int i = 0; i < pr->size(); i++ )
         {
            (*pr)[i] /= area;
         }
      }
      // cout << "normalize_pr area " << area << "\n" << flush;
      {
         double area = 0e0;
         for ( unsigned int i = 0; i < pr->size(); i++ )
         {
            area += (*pr)[i] * gridspacing;
         }
         // cout << "after normalize_pr area " << area << "\n" << flush;
      }
   }
   /* riemann sum?
   double area = 0e0;
   for ( unsigned int i = 1; i < pr->size(); i++ )
   {
      area += ( (*pr)[i-1] + (*pr)[i] ) / ( 2e0 * ( r[i] - r[i-1] ) );
   }
   if ( area > 0e0 )
   {
      for ( unsigned int i = 0; i < pr->size(); i++ )
      {
         (*pr)[i] *= pr->size() / area ;
      }
   }
   // cout << "normalize_pr area " << area << "\n" << flush;
   {
      double area = 0e0;
      for ( unsigned int i = 1; i < pr->size(); i++ )
      {
         area += ( (*pr)[i-1] + (*pr)[i] ) / ( 2e0 * ( r[i] - r[i-1] ) );
      }
      // cout << "after normalize_pr area " << area << "\n" << flush;
   }
   */
#endif
}

void US_Hydrodyn_Saxs::check_pr_grid( vector < double > &r, vector < double > &pr )
{
   if ( r.size() < 3 )
   {
      return;
   }

   if ( r.size() != pr.size() )
   {
      editor_msg( "red" , 
                  QString( "Internal error: the r grid has %1 points, put the P(r) values have %2 points!" )
                  .arg( r.size() )
                  .arg( pr.size() ) );
      return;
   }

   double delta = r[ 1 ] - r[ 0 ];

   bool ok = true;
   for ( unsigned int i = 1; i < r.size() - 1; i++ )
   {
      if ( r[ i + 1 ] - r[ i ] != delta )
      {
         ok = false;
         break;
      }
   }
   if ( ok )
   {
      return;
   }

   editor_msg( "dark red" , "The P(r) grid does not have uniform spacing: interpolating to current bin size" );

   vector < double > new_r;
   double start_r = r[ 0 ];
   double end_r   = r[ r.size() - 1 ];

   for ( double this_r = start_r; 
         this_r <= end_r + our_saxs_options->bin_size; 
         this_r += our_saxs_options->bin_size )
   {
      new_r.push_back( this_r );
   }

   // fix r for duplicate values:

   vector < double > org_r;
   vector < double > org_pr;

   {
      map < double, double       > fixed_pr;
      map < double, unsigned int > fixed_pr_count;
      
      for ( unsigned int i = 0; i < r.size(); i++ )
      {
         fixed_pr      [ r[ i ] ] += pr[ i ];
         fixed_pr_count[ r[ i ] ] ++;
      }
      for ( map < double, double >::iterator it = fixed_pr.begin();
            it != fixed_pr.end();
            it++ )
      {
         
         org_r .push_back( it->first );
         org_pr.push_back( it->second / fixed_pr_count[ it->first ] );
      }
      r  = org_r;
      pr = org_pr;
   }

   US_Saxs_Util usu;
   vector < double > y2;
   usu.natural_spline( r, pr, y2 );

   vector < double > new_pr( new_r.size() );

   for ( unsigned int i = 0; i < new_r.size(); i++ )
   {
      if ( !usu.apply_natural_spline( r, pr, y2, new_r[ i ], new_pr[ i ] ) )
      {
         editor_msg( "red", usu.errormsg );
         editor_msg( "red", tr( "Error attempting to interpolate" ) );
         return;
      }
   }

   r = new_r;
   pr = new_pr;
   return;
}
