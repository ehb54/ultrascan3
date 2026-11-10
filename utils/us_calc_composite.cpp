//! \file us_calc_composite.cpp
#include "us_calc_composite.h"
#include "us_calc_fp.h" // floating-point policy

#include "us_calc_version.h"

#include <QStringList>
#include <QtGlobal>

namespace US_Calc
{
namespace
{
// US_Model::constant_vbar and constant_ff0:  the range of a value over the
// components is below a limit.
bool constant( const QVector<Component>& comps, double Component::*value, double limit )
{
   double valmin = comps[0].*value;
   double valmax = comps[0].*value;
   for ( int ii = 1; ii < comps.size(); ii++ )
   {
      valmin = qMin( valmin, comps[ii].*value );
      valmax = qMax( valmax, comps[ii].*value );
   }
   return ( valmax - valmin ) < limit;
}
} // namespace

QString mc_solute_key( double s, double second )
{
   return QString::asprintf( "%10.4f %8.5f", s * 1.0e+13, second );
}

CompositeResult mc_composite( const QVector<Model>& iterations )
{
   CompositeResult result;
   result.algorithm         = algorithm( "mc_composite" );
   result.model.monte_carlo = true;
   if ( iterations.isEmpty() )
   {
      result.diagnostics << Diagnostic{ Issue::EmptyModel, Severity::Error, "there are no Monte Carlo iterations" };
      return result;
   }

   int                ncnstv = 0;
   int                ncnstk = 0;
   QVector<Component> all;
   for ( int ii = 0; ii < iterations.size(); ii++ )
   {
      const QVector<Component>& comps = iterations[ii].components;
      if ( comps.isEmpty() )
      {
         result.diagnostics << Diagnostic{ Issue::EmptyModel,
                                           Severity::Error,
                                           QString( "iteration %1 has no components" ).arg( ii + 1 ) };
         return result;
      }
      if ( constant( comps, &Component::vbar20, 1.0e-4 ) )
      {
         ncnstv++;
      }
      if ( constant( comps, &Component::f_f0, 1.0e-3 ) )
      {
         ncnstk++;
      }
      all << comps;
   }

   // As US_Model::load_multi_model:  a key per component, the unique keys
   // sorted, and for each key the concentrations summed in order.
   const double sclnrm  = 1.0 / (double) iterations.size();
   const bool   cnst_vb = ( ncnstv >= ncnstk );
   QStringList  sklist; // key of every component
   QStringList  skvals; // unique keys
   for ( const Component& c : all )
   {
      const QString key = mc_solute_key( c.s, cnst_vb ? c.f_f0 : c.vbar20 );
      sklist << key;
      if ( !skvals.contains( key ) )
      {
         skvals << key;
      }
   }
   skvals.sort();

   for ( const QString& key : skvals )
   {
      double conc = 0.0;
      int    last = 0;
      for ( int jj = 0; jj < sklist.size(); jj++ )
      {
         if ( sklist[jj] == key )
         {
            last = jj;
            conc += all[jj].signal_concentration;
         }
      }
      Component merged            = all[last];
      merged.signal_concentration = conc * sclnrm;
      result.model.components << merged;
      result.source_index << last;
   }

   const QString& first     = iterations.front().description;
   result.model.description = first.section( '.', 0, -3 ) + "." + first.section( '.', -2, -2 ).section( '_', 0, -2 ) +
                              QString::asprintf( "_mcN%03i", static_cast<int>( iterations.size() ) ) + "." +
                              first.section( '.', -1, -1 );
   return result;
}
} // namespace US_Calc
