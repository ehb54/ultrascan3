#include "../include/us_plot_util.h"
//Added by qt3to4:
#include <Q3TextStream>

bool US_Plot_Util::printtofile( QString basename,
                                map < QString, QwtPlot * > plots, 
                                QString & errors,
                                QString & messages )
{
   errors   = "";
   messages = "";

   bool any_plotted = false;

#ifdef QT4
   messages = "Not currently available in US3 versions";
#else

   for ( map < QString, QwtPlot * >::iterator it = plots.begin();
         it != plots.end();
         ++it )
   {
      vector < QString > titles;
      vector < vector < double > > x;
      vector < vector < double > > y;

      int max_rows = 0;
      int curves   = 0;

      // ts << QString( "plot for %1\n" ).arg( it->first );

      QwtPlotCurveIterator itc = it->second->curveIterator();
      for ( const QwtPlotCurve *c = itc.toFirst(); c != 0; c = ++itc )
      {
         long ck = itc.currentKey();

         // ts << QString( "Curve title %1 size %2\n" )
         //    .arg( it->second->curveTitle( ck ) )
         //    .arg( it->second->curve( ck )->dataSize() )
         //    ;

         if ( it->second->curve( ck )->dataSize() )
         {
            vector < double > tmp_x;
            vector < double > tmp_y;

            for ( int i = 0; i < it->second->curve( ck )->dataSize(); ++i )
            {
            // ts << QString( "%1 %2\n" )
            //    .arg( it->second->curve( ck )->x( i ) )
            //    .arg( it->second->curve( ck )->y( i ) )
            //    ;
               tmp_x.push_back( it->second->curve( ck )->x( i ) );
               tmp_y.push_back( it->second->curve( ck )->y( i ) );
            }

            titles.push_back( it->second->curveTitle( ck ) );
            x     .push_back( tmp_x );
            y     .push_back( tmp_y );

            if ( max_rows < (int) it->second->curve( ck )->dataSize() )
            {
               max_rows = (int) tmp_x.size();
            }
            curves++;
         }
      }

      if ( curves && max_rows )
      {
         QString plotname = QString( "%1" ).arg( it->first ).replace( " ", "_" );
         QString filename = basename + plotname + ".csv";

         if ( QFileInfo( filename ).exists() )
         {
            int ext = 1;
            do {
               filename = basename + plotname + QString( "-%1" ).arg( ext ) + ".csv";
            } while ( QFileInfo( filename ).exists() );
         }
            
         QFile f( filename );

         if ( !f.open( QIODevice::WriteOnly ) )
         {
            errors = QString( "File %1 can not open for writing" ).arg( f.name() );
            return false;
         }

         Q3TextStream ts( &f );

         // title row

         for ( int i = 0; i < curves; ++i )
         {
            ts << "\"" + titles[ i ] + ":x\",\"" + titles[ i ] + ":y\",";
         }
         ts << "\n";

         // data rows

         for ( int j = 0; j < max_rows; ++j )
         {
            for ( int i = 0; i < curves; i++ )
            {
               if ( j < (int) x[ i ].size() ) {
                  ts << x[ i ][ j ] << "," << y[ i ][ j ] << ",";
               } else {
                  ts << ",,";
               }
            }
            ts << "\n";
         }

         f.close();
         messages += QString( "Created plot file %1\n" ).arg( filename );
         any_plotted = true;
      }
   }

   if ( !any_plotted )
   {
      messages = "No plots to save";
   }

#endif
   return any_plotted;
}

