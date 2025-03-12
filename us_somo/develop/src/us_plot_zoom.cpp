#include "../include/us_plot_zoom.h"

#include <qdebug.h>

US_Plot_Zoom::US_Plot_Zoom( QwtPlot * plot, ScrollZoomer * zoomer ) {
   qDebug() << "US_Plot_Zoom::US_Plot_Zoom()";
   this->plot   = plot;
   this->zoomer = zoomer;
   save();
}

void US_Plot_Zoom::save() {
   qDebug() << "US_Plot_Zoom::save()";
   if ( zoomer ) {
      stack = zoomer->zoomStack();
      currentIndex = zoomer->zoomRectIndex();
   } else {
      stack.clear();
      currentIndex = 0;
   }
   // QTextStream( stdout ) << info( "in save after work" );
}

void US_Plot_Zoom::restore( bool replot ) {
   qDebug() << "US_Plot_Zoom::restore()";

   if ( zoomer && stack.size() ) {
      // QTextStream( stdout ) << info( QString( "in restore, will restore stack to index %1" ).arg( currentIndex ) );
      zoomer->setZoomStack( stack, currentIndex );
      if ( replot ) {
         // qDebug() << "US_Plot_Zoom::restore() replot";
         plot->replot();
      }
   }
}

QString US_Plot_Zoom::info( const QString & msg ) {
   qDebug() << "US_Plot_Zoom::info()";

   QStringList results;
   
   int pos = 0;

   results
      << "========================================"
      << "US_Plot_Zoom::info( " + msg + " )"
      << "----------------------------------------"
      <<  QString( "Current Index: %1" ).arg( currentIndex ) 
      ;
   
   if ( zoomer ) {
      for ( auto qr : zoomer->zoomStack() ) {
         qreal x1, y1, x2, y2;
         qr.getCoords( & x1, & y1, & x2, & y2 );

         results
            << QString( "[%1] zoom stack top left %2 %3, bottom right %4 %5" )
            .arg( pos++ )
            .arg( x1 )
            .arg( y1 )
            .arg( x2 )
            .arg( y2 )
            ;
      }
   } else {
      results << "null zoomer";
   }

   results
      << "----------------------------------------"
      ;
   
   return results.join( "\n" ) + "\n";
}
