//! \file us_gui_util.cpp
#include <QtSvg>

#include "us_gui_util.h"
#include "us_gzip.h"
#include "us_settings.h"
#if QT_VERSION > 0x050000
#include "qwt_plot_renderer.h"
#endif

// Save SVG+PNG or PNG file
int US_GuiUtil::save_plot( const QString& filename, const QwtPlot* plot )
{
   int status = 0;

   if ( filename.contains( ".svg" ) )
   {  // Save the file as SVG, then save a PNG version
      save_svg( filename, plot );

      QString fnamepng  = QString( filename ).section( ".", 0, -2 ) + ".png";
      save_png( fnamepng, plot );
   }

   else if ( filename.endsWith( ".png" ) )
   {  // Save the file as PNG
      save_png( filename, plot );
   }

   else
   {  // Flag an error:  file name does not end in ".svg" or ".png"
      status = 1;
   }

   return status;
}

// Save SVG file
int US_GuiUtil::save_svg( const QString& filename, const QwtPlot* plot )
{
   int status = 0;

   if ( filename.contains( ".svg" ) )
   {  // Save the file as SVG
      QString fnsvg   = QString( filename ).section( ".", 0, -2 ) + ".svg";

      // Set resolution to screen resolution
      double px  = (double)qApp->desktop()->width();
      double in  = (double)qApp->desktop()->widthMM() / 25.4;
      int    res = qRound( px / in );

      // Generate the SVG file
#if QT_VERSION < 0x050000
      int    pw  = plot->width()  + res;
      int    ph  = plot->height() + res;
      QSvgGenerator generator;
      generator.setResolution( res );
      generator.setFileName  ( fnsvg );
      generator.setSize      ( plot->size() );
      generator.setViewBox   ( QRect( QPoint( 0, 0 ), QPoint( pw, ph ) ) );

      plot->print( generator );
#else
      QwtPlotRenderer pltrend;
      QSizeF psize   = plot->size();
      pltrend.renderDocument( (QwtPlot*)plot, fnsvg, psize, res );
#endif

      // Compress it and save SVGZ file
      US_Gzip gz;
      gz.gzip( fnsvg );
   }

   else
   {
      status = 1;
   }

   return status;
}

// Save PNG file
int US_GuiUtil::save_png( const QString& filename, const QwtPlot* plot )
{
   int status = 0;


   if ( filename.endsWith( ".png" ) )
   {  // Save the file as a PNG version
#if QT_VERSION > 0x050000
      QPixmap pixmap = ((QWidget*)plot)->grab();
#else
      int pw  = plot->width();
      int ph  = plot->height();
      QPixmap pixmap = QPixmap::grabWidget( (QWidget*)plot, 0, 0, pw, ph );
#endif
      if ( ! pixmap.save( filename ) )
         status = 2;
   }

   else
   {  // Mark error:  filename does not end with ".png"
      status = 1;
   }

   return status;
}

int US_GuiUtil::save_csv( const QString& filename, const QwtPlot* plot )
{
   int status = 0;


   if ( filename.endsWith( ".csv" ) )
   {  // Save the file as a CSV
      // iterate over all plot items and construct the output data
      QVector<QVector<QString>> export_data;
      export_data.clear();
      // determine axis title
      QString x_axis_title = plot->axisTitle( QwtPlot::xBottom ).text();
      if ( x_axis_title.isEmpty() )
      {
         x_axis_title = plot->axisTitle( QwtPlot::xTop ).text();
      }
      if ( x_axis_title.isEmpty() )
      {
         x_axis_title = QString("x");
      }
      QString y_axis_title = plot->axisTitle( QwtPlot::yLeft ).text();
      if ( y_axis_title.isEmpty() )
      {
         y_axis_title = plot->axisTitle( QwtPlot::yRight ).text();
      }
      if ( y_axis_title.isEmpty() )
      {
         y_axis_title = QString("y");
      }
      int max_length = 0;
      for ( QwtPlotItem* it: plot->itemList( QwtPlotItem::Rtti_PlotCurve ) )
      {
         if ( it->rtti() != QwtPlotItem::Rtti_PlotCurve )
         {
            continue;
         }
         QwtPlotCurve* item = static_cast<QwtPlotCurve*>(it);
         // create two Vectors for x and y respective
         QVector<QString> x_data;
         x_data.clear();
         QVector<QString> y_data;
         y_data.clear();
         x_data << item->title().text() + " " + x_axis_title;
         y_data << item->title().text() + " " + y_axis_title;
         const QwtSeriesData<QPointF>* data = item->data();
         for (size_t i = 0; i < data->size(); i++)
         {
            QPointF point = data->sample(i);
            x_data << QString::number(point.x());
            y_data << QString::number(point.y());
         }
         if ( max_length < x_data.size() )
         {
            max_length = x_data.size();
         }
         if ( max_length < y_data.size() )
         {
            max_length = y_data.size();
         }
         export_data << x_data << y_data;
      }
      // iterate over all entries to ensure a proper csv format
      for ( int i = 0; i < export_data.size(); i++ )
      {
         if ( export_data[i].size() < max_length )
         {
            QVector<QString> vec = export_data[i];
            for ( int i = vec.size(); i < max_length; i++ )
            {
               vec << QString("");
            }
            export_data[i] = vec;
         }
      }
      // dump everything into a file
      QFile myFile( filename );
      if ( !myFile.open( QIODevice::WriteOnly ) )
      {
         qDebug() << "Could not write to file:" << filename << "Error string:" << myFile.errorString();
      }
      else
      {
         QTextStream out(&myFile);
         for ( int ii = 0; ii < max_length; ii++ )
         {
            for ( int jj = 0; jj < export_data.size() - 1; jj++ )
            {
               out << export_data[ jj ][ ii ] << ", ";
            }
            out << export_data.last()[ ii ] << Qt::endl;
         }
         myFile.flush();
      }

      myFile.close();
   }

   else
   {  // Mark error:  filename does not end with ".csv"
       status = 1;
   }

   return status;
}