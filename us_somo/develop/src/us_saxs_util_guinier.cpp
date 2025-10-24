#include "../include/us_saxs_util.h"
#include "../include/us_vector.h"

#if defined(WIN32)
#   include <dos.h>
#   include <stdlib.h>
#   include <float.h>
// #   define isnan _isnan
#   undef SHOW_TIMING
#endif

void US_Saxs_Util::linear_fit( 
                              vector < double > x, 
                              vector < double > y, 
                              double &a,
                              double &b,
                              double &siga,
                              double &sigb,
                              double &chi2
                              )
{
   unsigned int i;
   double t;
   double sxoss;
   double sx = 0e0;
   double sy = 0e0;
   double st2 = 0e0;
   double ss;
   double sigdat;
   unsigned int ndata = x.size();
   b = 0e0;

   for ( i = 0; i < ndata; i++ )
   {
      sx += x[i];
      sy += y[i];
   }
   ss = ndata;

   sxoss = sx / ss;

   for ( i = 0; i < ndata; i++ ) 
   {
      t = x[i] - sxoss;
      st2 += t * t;
      b += t * y[i];
   }
   b /= st2;
   a = ( sy - sx * b) / ss;
   siga = sqrt( ( 1e0 + sx * sx / ( ss * st2 ) ) / ss );
   sigb = sqrt( 1e0 /st2 );
   chi2 = 0e0;

   for ( i = 0; i < ndata; i++ ) 
   {
      chi2 += ( y[i] - a - b * x[i] ) * ( y[i] - a - b * x[i] );
   }
   sigdat = sqrt( chi2/ ( ndata - 2 ) );
   siga *= sigdat;
   sigb *= sigdat;
}

void US_Saxs_Util::linear_fit( 
                              vector < double > x, 
                              vector < double > y, 
                              vector < double > e, 
                              double &a,
                              double &b,
                              double &siga,
                              double &sigb,
                              double &chi2
                              )
{
   unsigned int i;
   double t;
   double sxoss;
   double sx = 0e0;
   double sy = 0e0;
   double st2 = 0e0;
   double ss = 0e0;
   double ssd = 0e0;
   double sigdat;
   unsigned int ndata = x.size();
   b = 0e0;

   if ( !is_nonzero_vector( e ) )
   {
      cout << "error: linear_fit with errors given zeros in error vector\n";
      return linear_fit( x, y, a, b, siga, sigb, chi2 );
   }

   for ( i = 0; i < ndata; i++ )
   {
      double wt = 1e0 / ( e[ i ] * e[ i ] );
      ssd += e[ i ];
      ss += wt;
      sx += x[i] * wt;
      sy += y[i] * wt;
   }

   sxoss = sx / ss;

   for ( i = 0; i < ndata; i++ ) 
   {
      t = ( x[i] - sxoss ) / e[ i ];
      st2 += t * t;
      b += t * y[i] / e[ i ];
   }
   b /= st2;
   a = ( sy - sx * b) / ss;
   siga = sqrt( ( 1e0 + sx * sx / ( ss * st2 ) ) / ss );
   sigb = sqrt( 1e0 /st2 );
   chi2 = 0e0;

   for ( i = 0; i < ndata; i++ ) 
   {
      chi2 += ( ( y[i] - a - b * x[i] ) / e[ i ] ) * ( ( y[i] - a - b * x[i] ) / e[ i ] );
   }
   sigdat = sqrt( chi2 / ( ndata - 2 ) );
   siga *= sigdat;
   sigb *= sigdat;
   chi2 *= ( ssd / ndata ) * ( ssd / ndata );
}

bool US_Saxs_Util::guinier_fit( 
                               QString &log,
                               QString tag,  // tag needs to be preprocessed with guinierplot
                               unsigned int startpos,
                               unsigned int endpos,
                               double &a,
                               double &b,
                               double &siga,
                               double &sigb,
                               double &chi2,
                               double &Rg,
                               double &I0,
                               double &smax,
                               double &smin,
                               double &sRgmin,
                               double &sRgmax,
                               bool   compute_Rc,
                               bool   compute_Rt
                               )
{
   vector < double > x;
   vector < double > y;
   errormsg = "";
   if ( startpos > endpos - 4 )
   {
      errormsg = "Guinier fit needs at least 4 points";
      return false;
   }

   if ( wave[tag].q.size() <= endpos )
   {
      errormsg = QString("selected end point %1 is more than the number of points available (%2)") 
                         .arg( endpos + 1 )
                         .arg( wave[tag].q.size() );
      return false;
   }
      
   for( unsigned int i = startpos; i <= endpos; i++ )
   {
      x.push_back(wave[tag].q[i]);
      y.push_back(wave[tag].r[i]);
   }

   // US_Vector::printvector( "s in guininer_fit", wave[ tag ].s );
   if ( wave[ tag ].s.size() && wave[ tag ].s.size() == wave[ tag ].q.size() && is_nonzero_vector( wave[ tag ].s ) )
   {
      // cout << "guinier fit with errors\n";
  
      vector < double > e;
      for( unsigned int i = startpos; i <= endpos; i++ )
      {
         e.push_back(wave[tag].s[i]);
      }
      linear_fit(x, y, e, a, b, siga, sigb, chi2);
   } else {
      //       cout << QString( "guinier fit no errors s.size: %1; %2; %3\n" )
      //          .arg( wave[ tag ].s.size() )
      //          .arg( wave[ tag ].s.size() == wave[ tag ].q.size() ? "equals q size" : "not equal q size" )
      //          .arg( is_nonzero_vector( wave[ tag ].s ) ? "is non zero" : "is not nonzero" );
      linear_fit(x, y, a, b, siga, sigb, chi2);
   }

   /* 
   cout << QString( "last linear fit: a %1 b %2 siga %3 sigb %4 ndf %5 chi2 %6\n" )
      .arg( a )
      .arg( b )
      .arg( siga )
      .arg( sigb )
      .arg( x.size() - 2 )
      .arg( chi2 )
      .toLatin1().data()
      ;
   */

   // cout << QString( "Org slope %1 slope sd %2\n" ).arg( b ).arg( sigb );

   double mult = ( compute_Rc ? -2e0 : (compute_Rt ? -1e0 : -3e0 ) );
   Rg = sqrt( mult * b );
   // sigb = fabs( Rg * sigb / b );
   sigb = b != 0e0 ? fabs( Rg * 5e-1 * sigb / b ) : 0e0;
   I0 = exp(a);
   // siga = fabs( I0 * siga / a );
   siga = fabs( I0 * siga );
   smin = sqrt(wave[tag].q[startpos]);
   smax = sqrt(wave[tag].q[endpos]);
   sRgmin = Rg * smin;
   sRgmax = Rg * smax;

   log += QString::asprintf( "|| %u || %u || %g || %g || %g || %g || %g || %g || %g || %g || %g || %g || %g ||\n",
                             startpos + 1,
                             endpos + 1,
                             smin,
                             smax,
                             sRgmin,
                             sRgmax,
                             Rg,
                             I0,
                             a,
                             b,
                             siga,
                             sigb,
                             chi2 );
   log += QString(
#if QT_VERSION < 0x040000
                  "pnggnuplot.pl -p 1.5 -g -l points -c %1 %1 -m %1 %1 %1 %1 %1g%1_%1.png %1g.dat\n"
#else
                  "pnggnuplot.pl -p 1.5 -g -l points -c %1 %2 -m %3 %4 %5 %6 %7g%8_%9.png %10g.dat\n"
#endif
                  )
      .arg(wave[tag].q[startpos]* .2 )
      .arg(wave[tag].q[endpos]* 1.2 )
      .arg(a)
      .arg(b)
      .arg(smin)
      .arg(smax)
      .arg(tag)
      .arg(startpos+1)
      .arg(endpos+1)
      .arg(tag);
;
   log += QString("[[Image(htdocs:pngs/%1g%2_%3.png)]]\n")
      .arg(tag)
      .arg(startpos+1)
      .arg(endpos+1);

   return true;
}

bool US_Saxs_Util::guinier_fit2( 
                                QString &log,
                                QString tag,             // tag needs to be preprocessed with guinierplot
                                unsigned int pointsmin,  // the minimum # of points allowed typically 10
                                unsigned int pointsmax,  // the maximum # of points allowed typically 100
                                double sRgmaxlimit,      // maximum sRg allowed! typically 1.3
                                double pointweightpower, // the exponent ofnumber of points when computing the best one (3 seems to work well)
                                //                          i.e. fitness = chi2 / ( number_of_points ** pointweightpower )
                                double guiniermaxq,
                                double &a,
                                double &b,
                                double &siga,
                                double &sigb,
                                double &chi2,
                                double &Rg,
                                double &I0,
                                double &smin,
                                double &smax,
                                double &sRgmin,
                                double &sRgmax,
                                unsigned int &beststart,
                                unsigned int &bestend,
                                bool   compute_Rc,
                                bool   compute_Rt
                       )
{
   errormsg = "";
   double bestfitness = 9e99;
   beststart = 0;
   bestend = 0;
   double fitness;

   for ( unsigned int i = 0; i < wave[tag].q.size() - pointsmin; i++ )
   {
      for ( unsigned int j = i + pointsmin - 1; j < wave[tag].q.size() && j - i <= pointsmax; j++ )
      {
         QString mylog; // only keep the good ones
         if ( sqrt(wave[tag].q[j]) > guiniermaxq )
         {
            break;
         }

         if ( !guinier_fit(
                           mylog,
                           tag,
                           i,
                           j,
                           a,
                           b,
                           siga,
                           sigb,
                           chi2,
                           Rg,
                           I0,
                           smin,
                           smax,
                           sRgmin,
                           sRgmax,
                           compute_Rc,
                           compute_Rt
                           ) )
         {
            // log += mylog;
            continue; // return false;
         }
         
         // cout << QString( "guinier2: i %1 j %2 Rg %3\n").arg( i ).arg( j ).arg( Rg );

         if ( sRgmax > sRgmaxlimit ) {
            // cout << "sRg violation\n";
            break;
         }
         if ( us_isnan(Rg) )  // positive slope
         {
            continue;
         }
         // log += mylog;
         fitness = chi2 / pow(j - i, pointweightpower);
         if ( fitness < bestfitness )
         {
            bestfitness = fitness;
            beststart = i;
            bestend = j;
         }

      }
         
   }


   if ( !guinier_fit(
                     log,
                     tag,
                     beststart,
                     bestend,
                     a,
                     b,
                     siga,
                     sigb,
                     chi2,
                     Rg,
                     I0,
                     smin,
                     smax,
                     sRgmin,
                     sRgmax,
                     compute_Rc,
                     compute_Rt
                     ) )
   {
      return false;
   }
   
   return true;
}

bool US_Saxs_Util::guinier_plot(QString outtag, QString tag)
{
   errormsg = "";
   wave[outtag].clear( );
   wave[outtag].filename = QString("guinier_%1").arg(tag);
   wave[outtag].header = wave[tag].header;
   
   bool use_errors = wave[ tag ].s.size() == wave[ tag ].q.size();
   for( unsigned int i = 0; i < wave[tag].q.size(); i++ )
   {
      wave[ outtag ].q.push_back(wave[tag].q[i] * wave[tag].q[i]);
      wave[ outtag ].r.push_back(wave[tag].r[i] > 0e0 ? log(wave[tag].r[i]) : 0e0);
      wave[ outtag ].s.push_back( use_errors && wave[tag].r[i] > 0e0 ? 
                                  // wave[ outtag ].r.back() * ( wave[ tag ].s[ i ] / wave[ tag ].r[ i ] ) : 0e0 );
                                  fabs( wave[ tag ].s[ i ] / wave[ tag ].r[ i ] ) : 0e0 );
   }

   // US_Vector::printvector3( "after guinier_plot() q2 I e", wave[outtag].q, wave[outtag].r, wave[outtag].s );

   return true;
}

bool US_Saxs_Util::guinier_remove_points(
                                         QString outtag,
                                         QString tag,
                                         map < double, double > & removed,
                                         unsigned int   & pts_removed,
                                         unsigned int   & startpos,
                                         unsigned int   & endpos,
                                         double  a,
                                         double  b,
                                         double  sd_limit )
{
   // cout << QString( "remove pts: a %1 b %2 sd_limit %3\n" ).arg( a ).arg( b ).arg( sd_limit );
   // check wave & see if there are any points outsize of sd from line
   // if so, remove them
   // also adjust start/end pos accordingly
   pts_removed = 0;

   errormsg = "";

   if ( startpos > endpos - 4 )
   {
      errormsg = "Guinier fit needs at least 4 points";
      return false;
   }

   if ( wave[tag].s.size() < wave[ tag ].q.size() )
   {
      errormsg = "Guinier_remove_points: insufficient S.D.'s";
      return false;
   }

   if ( wave[tag].q.size() <= endpos )
   {
      errormsg = QString("selected end point %1 is more than the number of points available (%2)") 
                         .arg( endpos + 1 )
                         .arg( wave[tag].q.size() );
      return false;
   }
      
   wave[outtag].clear( );
   wave[outtag].filename = QString("guinier_removed_%1").arg(tag);
   wave[outtag].header = wave[tag].header;

   for( unsigned int i = 0; i < startpos; i++ )
   {
      wave[ outtag ].q.push_back( wave[tag].q[i] );
      wave[ outtag ].r.push_back( wave[tag].r[i] );
      wave[ outtag ].s.push_back( wave[tag].s[i] );
   }

   unsigned new_endpos = endpos;

   for( unsigned int i = startpos; i <= endpos; i++ )
   {
      
      /*
      cout << QString( "pos %1: " ).arg( i );
      cout << QString( "q %1 r %2 s %3 fabs() %4 sdl*s %5\n" )
         .arg( wave[tag].q[i] )
         .arg( wave[tag].r[i] )
         .arg( wave[tag].s[i] )
         .arg( fabs( a + b * wave[tag].q[i] - wave[tag].r[i] ) )
         .arg( sd_limit * wave[ tag ].s[ i ] );
      */

      //       if ( fabs( exp( a + b * wave[tag].q[i] ) - exp( wave[tag].r[i] ) ) <
      //            sd_limit * fabs( exp( wave[tag].r[i] ) * wave[ tag ].s[ i ] / ( wave[ tag ].r[ i ] != 0e0 ? wave[ tag ].r[ i ] : 1e0 ) ) )
      if ( fabs( a + b * wave[tag].q[i] - wave[tag].r[i] ) < sd_limit * fabs( wave[ tag ].s[ i ] ) )
      {
         wave[ outtag ].q.push_back( wave[tag].q[i] );
         wave[ outtag ].r.push_back( wave[tag].r[i] );
         wave[ outtag ].s.push_back( wave[tag].s[i] );
      } else {
         removed[ wave[ tag ].q[ i ] ] = wave[ tag ].r[ i ];
         pts_removed++ ;
         if ( new_endpos >= startpos &&
              new_endpos > 0 )
         {
            new_endpos--;
         }
         // cout << QString( "remove point %1 %2 pts removed %3 endpos %4\n" ).arg( wave[ tag ].q[ i ] ).arg( wave[ tag ].r[ i ] ).arg( pts_removed ).arg( new_endpos );
      }
   }

   for( unsigned int i = endpos + 1; i < wave[tag].q.size(); i++ )
   {
      wave[ outtag ].q.push_back( wave[tag].q[i] );
      wave[ outtag ].r.push_back( wave[tag].r[i] );
      wave[ outtag ].s.push_back( wave[tag].s[i] );
   }

   endpos = new_endpos;

   return true;
}
                                         

bool US_Saxs_Util::guinier_fit_with_removal( 
                                            QString &log,
                                            QString tag,  // tag needs to be preprocessed with guinierplot
                                            unsigned int startpos,
                                            unsigned int endpos,
                                            double &a,
                                            double &b,
                                            double &siga,
                                            double &sigb,
                                            double &chi2,
                                            double &Rg,
                                            double &I0,
                                            double &smax,
                                            double &smin,
                                            double &sRgmin,
                                            double &sRgmax,
                                            double sd_limit,
                                            map < double, double > & removed,
                                            unsigned int   & pts_removed,
                                            bool   compute_Rc,
                                            bool   compute_Rt
                                            )
{
   // puts( "su: guinier_fit_with_removal" );
   removed.clear( );
   pts_removed = 0;

   QString outtag = "removed_" + tag;
   
   unsigned int this_pts_removed = 0;

   do 
   {
      if ( !wave[ tag ].q.size() )
      {
         errormsg = pts_removed ? "No points left, they were all removed as outliers" : "No points in initial curve";
         return false;
      }

      if ( endpos - startpos < 3 )
      {
         errormsg = pts_removed ? "Too few points for Guinier fit after removing outliers" : 
            "Too few points for Guinier fit";
         return false;
      }

      if ( !guinier_fit( log,
                         tag,
                         startpos,
                         endpos,
                         a,
                         b,
                         siga,
                         sigb,
                         chi2,
                         Rg,
                         I0,
                         smax,
                         smin,
                         sRgmin,
                         sRgmax,
                         compute_Rc,
                         compute_Rt
                         ) )
      {
         return false;
      }


      if ( !guinier_remove_points( outtag,
                                   tag,
                                   removed,
                                   this_pts_removed,
                                   startpos,
                                   endpos,
                                   a,
                                   b,
                                   sd_limit ) )
      {
         return false;
      }

      wave[ tag ] = wave[ outtag ];
      pts_removed += this_pts_removed;

   } while( this_pts_removed );
   
   return true;
}
