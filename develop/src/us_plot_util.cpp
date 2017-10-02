#include "../include/us_plot_util.h"
//Added by qt3to4:
#include <QTextStream>

bool US_Plot_Util::printtofile( QString basename,
                                map < QString, QwtPlot * > plots, 
                                QString & errors,
                                QString & messages )
{
   errors   = "";
   messages = "";

   bool any_plotted = false;

#ifdef QT4
   for ( map < QString, QwtPlot * >::iterator it = plots.begin();
         it != plots.end();
         ++it )
   {
      vector < QString > titles;
      vector < vector < double > > x;
      vector < vector < double > > y;

      int max_rows = 0;
      int curves   = 0;

      // qDebug() << QString( "plot for %1\n" ).arg( it->first );

      QwtPlotItemList ilist = it->second->itemList();
      for ( int ii = 0; ii < ilist.size(); ii++ )
      {
         QwtPlotItem* plitem = ilist[ ii ];
         if ( plitem->rtti() != QwtPlotItem::Rtti_PlotCurve ) {
            continue;
         }

         QwtPlotCurve* curve = (QwtPlotCurve*) plitem;

         // qDebug() << curve->title().text();
         
         if ( curve->dataSize() )
         {
            vector < double > tmp_x;
            vector < double > tmp_y;

            for ( int i = 0; i < curve->dataSize(); ++i )
            {
            // ts << QString( "%1 %2\n" )
            //    .arg( it->second->curve( ck )->x( i ) )
            //    .arg( it->second->curve( ck )->y( i ) )
            //    ;
               tmp_x.push_back( curve->x( i ) );
               tmp_y.push_back( curve->y( i ) );
            }

            titles.push_back( curve->title().text() );
            x     .push_back( tmp_x );
            y     .push_back( tmp_y );

            if ( max_rows < (int) curve->dataSize() )
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
            errors = QString( "File %1 can not open for writing" ).arg( f.fileName() );
            return false;
         }

         QTextStream ts( &f );

         bool skipx = plotname == "HPLC_SAXS_Guinier_Summary";
         
         set < int > skipx_vals;
         if ( skipx ) {
            for ( int i = 1; i < curves; ++i ) {
               if ( x[ i ] == x[ 0 ] ) {
                  skipx_vals.insert( i );
               }
            }
         }

         if ( skipx && (int) skipx_vals.size() != curves - 1 ) {
            us_qdebug( "fix up" );
            vector < vector < double > > new_x;
            vector < vector < double > > new_y;

            // make master list

            vector < double > union_x = US_Vector::vunion( x );
            vector < map < double, double > > y_vals;
            
            for ( int i = 0; i < curves; ++i ) {
               map < double, double > this_y;
               for ( int j = 0; j < (int) y[ i ].size(); ++j ) {
                  this_y[ x[ i ][ j ] ] = y[ i ][ j ];
               }
               y_vals.push_back( this_y );
               new_x.push_back( union_x );
            }
             
            for ( int i = 0; i < curves; ++i ) {
               vector < double > this_y;
               for ( int j = 0; j < (int) new_x[ i ].size(); ++j ) {
                  this_y.push_back( y_vals[ i ].count( new_x[ i ][ j ] ) ?
                                    y_vals[ i ][ new_x[ i ][ j ] ] : 
#if defined( NAN )
                                    NAN
#else
                                    sqrt(-1e0)
#endif
                                    );
               }
               new_y.push_back( this_y );
            }
            
            x = new_x;
            y = new_y;
         }

         // title row

         for ( int i = 0; i < curves; ++i )
         {
            if ( skipx ) {
               if ( i ) {
                  ts << "\"" + titles[ i ] + "\",";
               } else {
                  ts << "\"Frame\",\"" + titles[ i ] + "\",";
               }
            } else {
               ts << "\"" + titles[ i ] + ":x\",\"" + titles[ i ] + ":y\",";
            }
         }
         ts << "\n";

         // data rows

         for ( int j = 0; j < max_rows; ++j )
         {
            for ( int i = 0; i < curves; i++ )
            {
               if ( skipx && i ) {
                  if ( j < (int) x[ i ].size() ) {
                     ts << y[ i ][ j ] << ",";
                  } else {
                     ts << ",";
                  }
               } else {
                  if ( j < (int) x[ i ].size() ) {
                     ts << x[ i ][ j ] << "," << y[ i ][ j ] << ",";
                  } else {
                     ts << ",,";
                  }
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
            errors = QString( "File %1 can not open for writing" ).arg( f.fileName() );
            return false;
         }

         QTextStream ts( &f );

         bool skipx = plotname == "HPLC_SAXS_Guinier_Summary";
         
         set < int > skipx_vals;
         if ( skipx ) {
            for ( int i = 1; i < curves; ++i ) {
               if ( x[ i ] == x[ 0 ] ) {
                  skipx_vals.insert( i );
               }
            }
         }

         if ( skipx && (int) skipx_vals.size() != curves - 1 ) {
            us_qdebug( "fix up" );
            vector < vector < double > > new_x;
            vector < vector < double > > new_y;

            // make master list

            vector < double > union_x = US_Vector::vunion( x );
            vector < map < double, double > > y_vals;
            
            for ( int i = 0; i < curves; ++i ) {
               map < double, double > this_y;
               for ( int j = 0; j < (int) y[ i ].size(); ++j ) {
                  this_y[ x[ i ][ j ] ] = y[ i ][ j ];
               }
               y_vals.push_back( this_y );
               new_x.push_back( union_x );
            }
             
            for ( int i = 0; i < curves; ++i ) {
               vector < double > this_y;
               for ( int j = 0; j < (int) new_x[ i ].size(); ++j ) {
                  this_y.push_back( y_vals[ i ].count( new_x[ i ][ j ] ) ?
                                    y_vals[ i ][ new_x[ i ][ j ] ] : 
#if defined( NAN )
                                    NAN
#else
                                    sqrt(-1e0)
#endif
                                    );
               }
               new_y.push_back( this_y );
            }
            
            x = new_x;
            y = new_y;
         }

         // title row

         for ( int i = 0; i < curves; ++i )
         {
            if ( skipx ) {
               if ( i ) {
                  ts << "\"" + titles[ i ] + "\",";
               } else {
                  ts << "\"Frame\",\"" + titles[ i ] + "\",";
               }
            } else {
               ts << "\"" + titles[ i ] + ":x\",\"" + titles[ i ] + ":y\",";
            }
         }
         ts << "\n";

         // data rows

         for ( int j = 0; j < max_rows; ++j )
         {
            for ( int i = 0; i < curves; i++ )
            {
               if ( skipx && i ) {
                  if ( j < (int) x[ i ].size() ) {
                     ts << y[ i ][ j ] << ",";
                  } else {
                     ts << ",";
                  }
               } else {
                  if ( j < (int) x[ i ].size() ) {
                     ts << x[ i ][ j ] << "," << y[ i ][ j ] << ",";
                  } else {
                     ts << ",,";
                  }
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

