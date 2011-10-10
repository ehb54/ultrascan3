#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_hydrodyn.h"

void US_Hydrodyn_Saxs::plot_one_pr(vector < double > r, vector < double > pr, QString name)
{
   if ( r.size() < pr.size() )
   {
      pr.resize(r.size());
   }
   if ( pr.size() < r.size() )
   {
      r.resize(pr.size());
   }

#ifndef QT4
   long ppr = plot_pr->insertCurve("p(r) vs r");
   plot_saxs->setCurveStyle(ppr, QwtCurve::Lines);
#else
   QwtPlotCurve *curve = new QwtPlotCurve( "P(r) vs r" );
   curve->setStyle( QwtPlotCurve::Lines );
#endif
   plotted_r.push_back(r);

   plotted_pr_not_normalized.push_back(pr);
   plotted_pr_mw.push_back(get_mw(name,false));

   if ( cb_normalize->isChecked() )
   {
      normalize_pr(r, &pr, get_mw(name));
   }

   plotted_pr.push_back(pr);
   QString plot_name = name;
   int extension = 0;
   while ( dup_plotted_pr_name_check.count(plot_name) )
   {
      plot_name = QString("%1-%2").arg(name).arg(++extension);
   }
   qsl_plotted_pr_names << plot_name;
   dup_plotted_pr_name_check[plot_name] = true;
   unsigned int p = plotted_r.size() - 1;
                  
#ifndef QT4
   plot_pr->setCurveData(ppr, (double *)&(r[0]), (double *)&(pr[0]), (int)pr.size());
   plot_pr->setCurvePen(ppr, QPen(plot_colors[p % plot_colors.size()], 2, SolidLine));
#else
   curve->setData(
                  (double *)&( r[ 0 ] ), 
                  (double *)&( pr[ 0 ] ),
                  (int)r.size()
                  );
   curve->setPen( QPen( plot_colors[ p % plot_colors.size() ], 2, Qt::SolidLine ) );
   curve->attach( plot_pr );
#endif
   plot_pr->replot();
                  
   if ( !plotted )
   {
      plotted = true;
      editor->append("P(r) plot legend:\n");
   }
   QColor save_color = editor->color();
   editor->setParagraphBackgroundColor ( editor->paragraphs() - 1, QColor("dark gray") );
   editor->setColor(plot_colors[p % plot_colors.size()]);
   editor->append(name + "\n");
   editor->setColor(save_color);

   // to save to csv, write just contributing models?, target, model & residual
   // don't forget to make target part of it even if it isn't selected.
}


void US_Hydrodyn_Saxs::plot_one_iqq( vector < double > q, 
                                     vector < double > I, 
                                     QString name )
{
   vector < double > I_error = I;
   for ( unsigned int i = 0; i < I_error.size(); i++ )
   {
      I_error[ i ] = 0e0;
   }
   plot_one_iqq( q, I, I_error, name );
}

void US_Hydrodyn_Saxs::plot_one_iqq( vector < double > q, 
                                     vector < double > I, 
                                     vector < double > I_error, 
                                     QString name )
{
   if ( q.size() < I.size() )
   {
      q.resize(I.size());
   }
   if ( I.size() < q.size() )
   {
      I.resize(q.size());
   }
   
   QString plot_name = name;
   int extension = 0;
   while ( dup_plotted_iq_name_check.count(plot_name) )
   {
      plot_name = QString("%1-%2").arg(name).arg(++extension);
   }
   qsl_plotted_iq_names << plot_name;
   dup_plotted_iq_name_check[plot_name] = true;
      
#ifndef QT4
   plotted_iq_names_to_pos[plot_name] = plotted_Iq.size();

   long Iq = plot_saxs->insertCurve("I(q) vs q");
   plot_saxs->setCurveStyle(Iq, QwtCurve::Lines);
#else
   plotted_iq_names_to_pos[plot_name] = plotted_Iq_curves.size();

   QwtPlotCurve *curve = new QwtPlotCurve( "I(q) vs q" );
   curve->setStyle( QwtPlotCurve::Lines );
#endif

   plotted_q.push_back(q);
#ifndef QT4
   plotted_Iq.push_back(Iq);
#else
   plotted_Iq_curves.push_back( curve );
#endif
   {
      vector < double > q2(q.size());
      for ( unsigned int i = 0; i < q.size(); i++ )
      {
         q2[i] = q[i] * q[i];
      }
      plotted_q2.push_back(q2);
   }
   plotted_I.push_back(I);
   plotted_I_error.push_back(I_error);

   unsigned int q_points = q.size();
   unsigned int p = plotted_q.size() - 1;

#ifndef QT4
   plot_saxs->setCurveData(Iq, 
                           cb_guinier->isChecked() ?
                           (double *)&(plotted_q2[p][0]) : (double *)&(plotted_q[p][0]), 
                           (double *)&(plotted_I[p][0]), q_points);
   plot_saxs->setCurvePen(Iq, QPen(plot_colors[p % plot_colors.size()], 2, SolidLine));
#else
   curve->setData(
                  cb_guinier->isChecked() ?
                  (double *)&(plotted_q2[p][0]) : (double *)&(plotted_q[p][0]), 
                  (double *)&(plotted_I[p][0]),
                  q_points
                  );
   curve->setPen( QPen( plot_colors[ p % plot_colors.size() ], 2, Qt::SolidLine ) );
   curve->attach( plot_saxs );
#endif

   // figure out how to plot points for guinier plots
   plot_saxs->replot();

   if ( !plotted )
   {
      plotted = true;
      editor->append("I(q) vs q plot legend:\n");
   }

   QColor save_color = editor->color();
   editor->setParagraphBackgroundColor ( editor->paragraphs() - 1, QColor("dark gray") );
   editor->setColor(plot_colors[p % plot_colors.size()]);
   editor->append(name + "\n");
   editor->setColor(save_color);
   saxs_search_update_enables();
}

void US_Hydrodyn_Saxs::push_back_zero_I_error()
{
   if ( !plotted_I.size() )
   {
      return;
   }

   vector < double > I_error( plotted_I[ plotted_I.size() - 1 ] );
   for ( unsigned int i = 0; i < I_error.size(); i++ )
   {
      I_error[ i ] = 0e0;
   }
   plotted_I_error.push_back( I_error );
}

bool US_Hydrodyn_Saxs::is_zero_vector( vector < double > &v )
{
   bool all_zero = true;
   for ( unsigned int i = 0; i < v.size(); i++ )
   {
      if ( v[ i ] != 0e0 )
      {
         all_zero = false;
         break;
      }
   }
   return all_zero;
}

bool US_Hydrodyn_Saxs::is_nonzero_vector( vector < double > &v )
{
   bool non_zero = v.size() > 0;
   for ( unsigned int i = 0; i < v.size(); i++ )
   {
      if ( v[ i ] == 0e0 )
      {
         non_zero = false;
         break;
      }
   }
   return non_zero;
}

QString US_Hydrodyn_Saxs::Iq_plotted_summary()
{
   QString qs = "Iq_plotted_summary:\n";

   qs += 
      QString(
              "plotted_q.size() %1\n"
              "plotted_q2.size() %2\n"
              "plotted_I.size() %3\n"
              "plotted_I_error.size() %4\n" 
              "\n"
              )
      .arg( plotted_q.size() )
      .arg( plotted_q2.size() )
      .arg( plotted_I.size() )
      .arg( plotted_I_error.size() )
      ;

   for ( unsigned int i = 0; i < plotted_q.size(); i++ )
   {
      bool all_zero = is_zero_vector( plotted_I_error[ i ] );
      bool is_nonzero = is_nonzero_vector( plotted_I_error[ i ] );
         
      qs += 
         QString( "pos %1 q.size() %2 q2.size() %3 I.size() %4 I_error.size() %5 %6 %7 q[%8:%9]\n" )
         .arg( i )
         .arg( plotted_q[ i ].size() )
         .arg( plotted_q2[ i ].size() )
         .arg( plotted_I[ i ].size() )
         .arg( plotted_I_error[ i ].size() )
         .arg( all_zero ? "zero error" : "contains error" )
         .arg( is_nonzero ? "all nonzero" : "" )
         .arg( plotted_q[ i ].size()
               ? plotted_q[ i ][ 0 ]
               : 0 )
         .arg( plotted_q[ i ].size()
               ? plotted_q[ i ][ plotted_q[ i ].size() - 1 ] 
               : 0 );
      if ( !is_nonzero && !all_zero )
      {
         for ( unsigned int j = 0; j < plotted_I_error[i].size(); j++ )
         {
            if ( plotted_I_error[i][j] == 0 )
            {
               qs += QString("    %1 %2 error is zero\n").arg(i).arg(j);
            }
         }
      }
   }

   return qs;
}
