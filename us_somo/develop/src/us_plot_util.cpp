#include "../include/us_plot_util.h"
//Added by qt3to4:
#include <QTextStream>
#include <limits>

bool US_Plot_Util::printtofile( QString basename,
                                map < QString, QwtPlot * > plots, 
                                QString & errors,
                                QString & messages )
{
   errors   = "";
   messages = "";

   bool any_plotted = false;

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

            for ( int i = 0; i < (int) curve->dataSize(); ++i )
            {
            // ts << QString( "%1 %2\n" )
            //    .arg( it->second->curve( ck )->x( i ) )
            //    .arg( it->second->curve( ck )->y( i ) )
            //    ;
               tmp_x.push_back( curve->sample( i ).x() );
               tmp_y.push_back( curve->sample( i ).y() );
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
   
   return any_plotted;
}

void US_Plot_Util::plotinfo(
                            QString name,
                            QwtPlot * plot
                            ) {
   QTextStream out( stdout );
   QString result = "-->plotinfo info for " + name + "\n";

   QwtPlotItemList ilist = plot->itemList();
   for ( int ii = 0; ii < ilist.size(); ii++ )
   {
      QwtPlotItem* plitem = ilist[ ii ];
      if ( plitem->rtti() != QwtPlotItem::Rtti_PlotCurve ) {
         result += QString( "    item %1 not a Rtti_PlotCurve\n" ).arg( ii );
         continue;
      }

      QwtPlotCurve* curve = (QwtPlotCurve*) plitem;

      result += QString( "    item %1 title %2 size %3\n" ).arg( ii ).arg( curve->title().text() ).arg( curve->dataSize() );

   }
   out << result;
}

void US_Plot_Util::align_plot_extents( const vector < QwtPlot * > & plots, bool scale_x_to_first ) {
   // QTextStream tso( stdout );
   // tso << "align_plot_extents\n";

   int size = (int) plots.size();
   if ( size <= 1 ) {
      return;
   }

   vector < double > extents( plots.size() );

   double max_extent = extents[ 0 ] = plots[ 0 ]->axisWidget( QwtPlot::yLeft )->scaleDraw()->extent( plots[ 0 ]->axisWidget( QwtPlot::yLeft )->font() );

   if ( scale_x_to_first ) {
      // tso << "align_plot_extents also scale x\n";
      const QwtScaleDiv scaleDiv = plots[ 0 ]->axisScaleDiv( QwtPlot::xBottom );
      
      // tso << QString( "scaleDiv->lowerBound %1 ->upperBound() %2\n" )
      //    .arg( scaleDiv.lowerBound() )
      //    .arg( scaleDiv.upperBound() )
      //    ;

      for ( int i = 1; i < size; ++i ) {
         extents[ i ] = plots[ i ]->axisWidget( QwtPlot::yLeft )->scaleDraw()->extent( plots[ i ]->axisWidget( QwtPlot::yLeft )->font() );
         if ( max_extent < extents[ i ] ) {
            max_extent = extents[ i ];
         }
      }

      for ( int i = 0; i < size; ++i ) {
         plots[ i ]->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( max_extent );
         plots[ i ]->setAxisScaleDiv( QwtPlot::xBottom, scaleDiv );
         plots[ i ]->replot();
      }
   } else {
      for ( int i = 1; i < size; ++i ) {
         extents[ i ] = plots[ i ]->axisWidget( QwtPlot::yLeft )->scaleDraw()->extent( plots[ i ]->axisWidget( QwtPlot::yLeft )->font() );
         if ( max_extent < extents[ i ] ) {
            max_extent = extents[ i ];
         }
      }

      for ( int i = 0; i < size; ++i ) {
         if ( extents[ i ] < max_extent ) {
            plots[ i ]->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( max_extent );
            plots[ i ]->updateLayout();
         }
      }
   }      
}

// #define DEBUG_RESCALE
// #define DEBUG_RESCALE_CURVES
// #define DEBUG_RESCALE_INTERNAL

void US_Plot_Util::rescale( map < QString, QwtPlot * > plots, map < QwtPlot *, ScrollZoomer * > plot_to_zoomer, bool only_scale_y ) {
#if defined( DEBUG_RESCALE )
   qDebug() << "US_Plot_Util::rescale type 1\n";
#endif
   for ( map < QString, QwtPlot * >::iterator it = plots.begin();
         it != plots.end();
         ++it )
   {
      if ( !plot_to_zoomer.count( it->second ) ||
           !plot_to_zoomer[ it->second ] ) {
#if defined( DEBUG_RESCALE_INTERNAL )
         qDebug() << "internal error: missing plot_to_zoomer for plot " << it->first << "\n";
#endif
      } else {
#if defined( DEBUG_RESCALE )
         qDebug() << "rescaling for plot " << it->first << "\n";
#endif
         rescale( it->second, plot_to_zoomer[ it->second ], only_scale_y );
      }
   }
}

void US_Plot_Util::rescale( const vector < QwtPlot * > & plots, map < QwtPlot *, ScrollZoomer * > plot_to_zoomer, bool only_scale_y ) {
#if defined( DEBUG_RESCALE )
   qDebug() << "US_Plot_Util::rescale type 2\n";
#endif
   for ( int i = 0; i < (int) plots.size(); ++i ) {
      if ( !plot_to_zoomer.count( plots[ i ] ) ) {
         qDebug() << "internal error: missing plot_to_zoomer for plot\n";
      } else {
         rescale( plots[ i ], plot_to_zoomer[ plots[ i ] ], only_scale_y );
      }
   }
}

void US_Plot_Util::rescale( QwtPlot * plot, ScrollZoomer * zoomer, bool only_scale_y ) {
#if defined( DEBUG_RESCALE )
   qDebug() << "US_Plot_Util::rescale type 3\n";
#endif
   if ( !only_scale_y ) {
#if defined( DEBUG_RESCALE_INTERNAL )
      qDebug() << "Internal error: US_Plot_Util::rescale( *, false ) not supported";
#endif
      return;
   }
   if ( !zoomer ) {
#if defined( DEBUG_RESCALE_INTERNAL )
      qDebug() << "internal error: missing plot_to_zoomer for plot in rescale\n";
#endif
      return;
   }

   QTextStream tso( stdout );

#if defined( DEBUG_RESCALE )
   tso << "zoomer ptr " << zoomer << "\n";
   tso << "object name " << plot->objectName() << "\n";
   tso << "US_Plot_Util::rescale( QwtPlot * plot, " << ( only_scale_y ? "true" : "false" ) << ")\n";
   tso << QString().sprintf(
                            "plot->axisScaleDiv( QwtPlot::xBottom ).lower,upperBound()     %g\t%g\n"
                            "plot->axisScaleDiv( QwtPlot::yLeft ).lower,upperBound()       %g\t%g\n"

                            ,plot->axisScaleDiv( QwtPlot::xBottom ).lowerBound()
                            ,plot->axisScaleDiv( QwtPlot::xBottom ).upperBound()
                            ,plot->axisScaleDiv( QwtPlot::yLeft ).lowerBound()
                            ,plot->axisScaleDiv( QwtPlot::yLeft ).upperBound()
                            );
#endif
   vector < double > x;
   vector < double > y;

   int curves   = 0;

   double min_x = plot->axisScaleDiv( QwtPlot::xBottom ).lowerBound();
   double max_x = plot->axisScaleDiv( QwtPlot::xBottom ).upperBound();

#if defined( DEBUG_RESCALE )
   double min_x_alt = zoomer->zoomRect().bottomLeft().x();
   double max_x_alt = zoomer->zoomRect().bottomRight().x();
   
   tso << QString().sprintf(
                            "min_x, max_x                                                  %g\t%g\n"
                            "min_x_alt, max_x_alt                                          %g\t%g\n"

                            ,min_x
                            ,max_x
                            ,min_x_alt
                            ,max_x_alt
                            );
#endif
                            
   double min_y = numeric_limits<double>::max();
   double max_y = numeric_limits<double>::min();

   QwtPlotItemList ilist = plot->itemList();
   for ( int ii = 0; ii < ilist.size(); ii++ )
   {
#if defined( DEBUG_RESCALE_CURVES )
      tso << "curve " << ii << "\n";
#endif

      QwtPlotItem* plitem = ilist[ ii ];
      if ( plitem->rtti() != QwtPlotItem::Rtti_PlotCurve ) {
         continue;
      }

      QwtPlotCurve* curve = (QwtPlotCurve*) plitem;

      // qDebug() << curve->title().text();
         
      if ( curve->dataSize() )
      {
         double x;
         double y;

         for ( int i = 0; i < (int) curve->dataSize(); ++i )
         {
            x = curve->sample( i ).x();
            if ( x >= min_x &&
                 x <= max_x ) {
#if defined( DEBUG_RESCALE_CURVES )
               tso << "point " << i << " x,y: " <<  curve->sample( i ).x() << " , " << curve->sample( i ).y() << "\n";
#endif
               y = curve->sample( i ).y();
               if ( min_y > y ) {
                  min_y = y;
               }
               if ( max_y < y ) {
                  max_y = y;
               }
            }
         }
         curves++;
      }
   }
   
   if ( !curves ) {
#if defined( DEBUG_RESCALE )
      tso << "no curves found\n";
#endif
      return;
   }
#if defined( DEBUG_RESCALE )
   tso << QString().sprintf(
                            "computed min_y, max_y                                         %g\t%g\n"

                            ,min_y
                            ,max_y
                            );

   // check zoom stack
   tso << QString().sprintf(
                            "zoomer->zoomRect().bottomLeft().x(),y()                       %g\t%g\n"
                            "zoomer->zoomRect().bottomRight().x(),y()                      %g\t%g\n"
                            "zoomer->zoomRect().topLeft().x(),y()                          %g\t%g\n"
                            "zoomer->zoomRect().topRight().x(),y()                         %g\t%g\n"

                            ,zoomer->zoomRect().bottomLeft().x()
                            ,zoomer->zoomRect().bottomLeft().y()
                            ,zoomer->zoomRect().bottomRight().x()
                            ,zoomer->zoomRect().bottomRight().y()
                            ,zoomer->zoomRect().topLeft().x()
                            ,zoomer->zoomRect().topLeft().y()
                            ,zoomer->zoomRect().topRight().x()
                            ,zoomer->zoomRect().topRight().y()
                          
                            );
#endif

   // ok, construct QRectF an zoom
   if ( zoomer->symmetric_rescale ) {
#if defined( DEBUG_RESCALE )
      tso << "symmetric rescale enabled\n";
#endif
      double smax_y = fabs( max_y ) > fabs( min_y ) ? fabs( max_y ) : fabs( min_y );
      zoomer->zoom(
                   QRectF(
                          QPointF( zoomer->zoomRect().topLeft().x(), smax_y * 1.2e0 ),
                          QPointF( zoomer->zoomRect().bottomRight().x(), -smax_y * 1.2e0 )
                          )
                   );
      
   } else {
      zoomer->zoom(
                   QRectF(
                          QPointF( zoomer->zoomRect().topLeft().x(), max_y * 1.1e0 ),
                          QPointF( zoomer->zoomRect().bottomRight().x(), min_y * 0.9e0 )
                          )
                   );
   }
   
}
