#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_lm.h"

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
   long ppr = plot_pr->insertCurve( plot_name );
   plot_saxs->setCurveStyle(ppr, QwtCurve::Lines);
#else
   QwtPlotCurve *curve = new QwtPlotCurve( plot_name );
   curve->setStyle( QwtPlotCurve::Lines );
#endif

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

   if ( plot_pr_zoomer )
   {
      delete plot_pr_zoomer;
   }
   double minx;
   double maxx;
   double miny;
   double maxy;
   set_plot_pr_range( minx, maxx, miny, maxy );
   plot_pr->setAxisScale( QwtPlot::xBottom, minx, maxx );
   plot_pr->setAxisScale( QwtPlot::yLeft  , miny, maxy );

   plot_pr_zoomer = new ScrollZoomer(plot_pr->canvas());
#ifndef QT4
   plot_pr_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
   plot_pr_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#else
   plot_pr_zoomer->setRubberBandPen( QPen( Qt::red, 1, Qt::DotLine ) );
   plot_pr_zoomer->setTrackerPen( QPen( Qt::red ) );
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

void US_Hydrodyn_Saxs::set_plot_pr_range( double &minx, 
                                          double &maxx,
                                          double &miny,
                                          double &maxy )
{
   minx = 0e0;
   maxx = 1e0;
   miny = 0e0;
   maxy = 1e0;

   bool any_x_set = false;
   bool any_y_set = false;

   if ( plotted_r.size() &&
        plotted_r[ 0 ].size() )
   {
      minx = plotted_r[ 0 ][ 0 ];
      maxx = plotted_r[ 0 ][ plotted_r[ 0 ].size() - 1 ];
      any_x_set = true;
   }

   for ( unsigned int i = 1; i < plotted_r.size(); i++ )
   {
      if ( plotted_r[ i ].size() )
      {
         if ( any_x_set )
         {
            if ( minx > plotted_r[ i ][ 0 ] )
            {
               minx = plotted_r[ i ][ 0 ];
            }
            if ( maxx < plotted_r[ i ][ plotted_r[ i ].size() - 1 ] )
            {
               maxx = plotted_r[ i ][ plotted_r[ i ].size() - 1 ];
            }
         } else {
            minx = plotted_r[ i ][ 0 ];
            maxx = plotted_r[ i ][ plotted_r[ i ].size() - 1 ];
            any_x_set = true;
         }            
      }
   }

   for ( unsigned int i = 0; i < plotted_pr.size(); i++ )
   {
      if ( plotted_pr[ i ].size() )
      {
         double this_miny = plotted_pr[ i ][ 0 ];
         double this_maxy = plotted_pr[ i ][ 0 ];
         for ( unsigned int j = 1; j < plotted_pr[ i ].size(); j++ )
         {
            if ( this_miny > plotted_pr[ i ][ j ] )
            {
               this_miny = plotted_pr[ i ][ j ];
            }
            if ( this_maxy < plotted_pr[ i ][ j ] )
            {
               this_maxy = plotted_pr[ i ][ j ];
            }
         }
         if ( any_y_set )
         {
            if ( miny > this_miny )
            {
               miny = this_miny;
            }
            if ( maxy < this_maxy )
            {
               maxy = this_maxy;
            }
         } else {
            miny = this_miny;
            maxy = this_maxy;
            any_y_set = true;
         }
      }
   }
   maxy *= 1.05e0;
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

   if ( our_saxs_options->compute_exponentials )
   {
#if defined( SIMPLE_TEST )
      // simple test:
      {
         saxs saxsC = saxs_map[ "C" ];
         saxs saxsH = saxs_map[ "H" ];
         QString gly = "GLY";
         QString C = "C";
         
         double ffv = compute_ff( saxsC, saxsH,
                                  gly,
                                  C,
                                  C,
                                  0,
                                  2e0,
                                  pow( 2e0 / ( 4e0 * M_PI ) , 2 ) );

         double ffe;
         {
            using namespace LM;
            minusoneoverfourpisq  = -1e0 / ( 16e0 * M_PI * M_PI );
            exponential_terms = 4;
            vector < double > par( exponential_terms * 2 + 1 );
            par[ 0 ] = saxsC.a[ 0 ];
            par[ 1 ] = saxsC.b[ 0 ];
            par[ 2 ] = saxsC.a[ 1 ];
            par[ 3 ] = saxsC.b[ 1 ];
            par[ 4 ] = saxsC.a[ 2 ];
            par[ 5 ] = saxsC.b[ 2 ];
            par[ 6 ] = saxsC.a[ 3 ];
            par[ 7 ] = saxsC.b[ 3 ];
            par[ 8 ] = saxsC.c;
            ffe = compute_exponential_f( 2e0, (const double *)&(par[ 0 ] ) );
         }
         cout << QString( "ff computes %1 ffe computes %2\n" ).arg( ffv ).arg( ffe );
      }
#endif                     
      if ( is_zero_vector( I_error ) )
      {
         bool ok;
         QString text = "";
         QString dupmsg;
         do {
            dupmsg = "";
            text = QInputDialog::getText(
                                         tr("US-SOMO: Compute structure factors"),
                                         tr( "If you want to compute structure factors for this curve\n"
                                             "Please define a dummy saxs name or press CANCEL to skip this computation.\n"
                                             + dupmsg +
                                             "Name for these structure factors:" )
                                         , 
                                         QLineEdit::Normal,
                                         text, 
                                         &ok, 
                                         this );
            text.replace( QRegExp( "\\s" ), "_" );
            if ( ok && !text.isEmpty() )
            {
               // user entered something and pressed OK
               dupmsg = "";
               if ( saxs_map.count( text ) )
               {
                  switch ( QMessageBox::question(this, 
                                                 tr( "US-SOMO: Compute structure factors : duplicate name" ),
                                                 QString( tr( "The name %1 was previously defined\n"
                                                              "Do you want to overwrite the values?" ) ).arg( text ),
                                                 tr( "&Overwrite" ), 
                                                 tr( "&Try again" ),
                                                 QString::null,
                                                 1,
                                                 0
                                                 ) )
                  {
                  case 0 : 
                     break;
                  case 1 : 
                     dupmsg = tr( "Name already exists, choose a unique name\n" );
                     break;
                  default :
                     break;
                  }
               }
            } 
         } while ( ok && ( text.isEmpty() || !dupmsg.isEmpty() ) );

         if ( ok )
         {
            vector < double > coeff4;
            vector < double > coeff5;
            double            norm4;
            double            norm5;
            double            nnorm4;
            double            nnorm5;
            US_Saxs_Util usu;
            cout << "compute exponentials\n" << flush;
            editor_msg( "blue", tr( "Computing 4 & 5 term exponentials" ) );
            QMessageBox::information( this, 
                                      tr( "US-SOMO: Compute structure factors : start computation" ),
                                      QString( tr( "Start computation\n"
                                                   "NOTE: This could take some time and the program will appear unresponsive" ) ) );
            
            if ( !usu.compute_exponential( q,
                                           I, 
                                           coeff4,
                                           coeff5,
                                           norm4,
                                           norm5
                                           ) )
            {
               editor_msg( "red", usu.errormsg );
               cout << usu.errormsg << endl;
            } else {
               nnorm4 = norm4 / q.size();
               nnorm5 = norm5 / q.size();

               QString norm4tag = "unacceptable";
               if ( nnorm4 < 1000e0 )
               {
                  norm4tag = "bad";
               }
               if ( nnorm4 < 500e0 )
               {
                  norm4tag = "not so good";
               }
               if ( nnorm4 < 250e0 )
               {
                  norm4tag = "ok";
               }
               if ( nnorm4 < 10e0 )
               {
                  norm4tag = "good";
               }
               if ( nnorm4 < 1e0 )
               {
                  norm4tag = "very good";
               }
               if ( nnorm4 < 1e-2 )
               {
                  norm4tag = "excellent";
               }
               if ( nnorm4 < 1e-4 )
               {
                  norm4tag = "amazingly excellent";
               }

               QString norm5tag = "unacceptable";
               if ( nnorm5 < 1000e0 )
               {
                  norm5tag = "bad";
               }
               if ( nnorm5 < 500e0 )
               {
                  norm5tag = "not so good";
               }
               if ( nnorm5 < 250e0 )
               {
                  norm5tag = "ok";
               }
               if ( nnorm5 < 10e0 )
               {
                  norm5tag = "good";
               }
               if ( nnorm5 < 1e0 )
               {
                  norm5tag = "very good";
               }
               if ( nnorm5 < 1e-2 )
               {
                  norm5tag = "excellent";
               }
               if ( nnorm5 < 1e-4 )
               {
                  norm5tag = "amazingly excellent";
               }

               QString qs4 = text;
               coeff4.resize( 9 );
               for ( unsigned int i = 1; i < ( unsigned int ) coeff4.size(); i++ )
               {
                  qs4 += QString( " %1" ).arg( coeff4[ i ] );
               }
               qs4 += QString( " %1" ).arg( coeff4[ 0 ] );
               cout << "Terms4: " << qs4 << endl;
               editor_msg( "dark blue", QString( "Exponentials 4 terms norm %1 nnorm %2 (%3): %4" )
                           .arg( norm4 )
                           .arg( nnorm4 )
                           .arg( norm4tag )
                           .arg( qs4 ) );
                  

               QString qs5 = text;
               for ( unsigned int i = 1; i < ( unsigned int ) coeff5.size(); i++ )
               {
                  qs5 += QString( " %1" ).arg( coeff5[ i ] );
               }
               qs5 += QString( " %1" ).arg( coeff5[ 0 ] );
               cout << "Terms5: " << qs5 << endl;
               editor_msg( "dark blue", QString( "Exponentials 5 terms norm %1 nnorm %2 (%3): %4" )
                           .arg( norm5 )
                           .arg( nnorm5 )
                           .arg( norm5tag )
                           .arg( qs5 ) );
               switch ( QMessageBox::question(this, 
                                              tr( "US-SOMO: Compute structure factors : computed" ),
                                              QString( tr( "Saxs name %1\n"
                                                           "4 term exponentials: norm %2 nnorm %3 %4\n"
                                                           "5 term exponentials: norm %5 nnorm %6 %7\n"
                                                           "Do you want to overwrite the values?" ) )
                                              .arg( text )
                                              .arg( norm4 )
                                              .arg( nnorm4 )
                                              .arg( norm4tag )
                                              .arg( norm5 )
                                              .arg( nnorm5 )
                                              .arg( norm5tag )
                                              ,
                                              tr("&Save to file"), 
                                              tr("&Forget them") 
                                              ) )
               {
               case 0 : 
                  {
                     // add to saxs_map
                     bool add_to_saxs_list = !saxs_map.count( text );

                     saxs_map[ text ].saxs_name = text;
                     for ( unsigned int i = 0; i < 4; i++ )
                     {
                        saxs_map[ text ].a[ i ] = coeff4[ 1 + i * 2 ];
                        saxs_map[ text ].b[ i ] = coeff4[ 2 + i * 2 ];
                     }
                     saxs_map[ text ].c = coeff4[ 0 ];

                     for ( unsigned int i = 0; i < 5; i++ )
                     {
                        saxs_map[ text ].a5[ i ] = coeff5[ 1 + i * 2 ];
                        saxs_map[ text ].b5[ i ] = coeff5[ 2 + i * 2 ];
                     }
                     saxs_map[ text ].c5 = coeff5[ 0 ];

                     // get excluded volume
                     {
                        bool ok;
                        double res = QInputDialog::getDouble(
                                                             tr( "US-SOMO: Compute structure factors : enter excluded volume" ),
                                                             "Enter a default excluded volume in Angstroms^3 (can be zero):", 
                                                             0, 
                                                             0,
                                                             1e20,
                                                             4, 
                                                             &ok
                                                             , this );
                        if ( ok ) {
                           saxs_map[ text ].volume = res;
                        } else {
                           if ( add_to_saxs_list )
                           {
                              saxs_map[ text ].volume = 0e0;
                           } else {
                              // leave alone
                           }
                           QMessageBox::information( this, 
                                                     tr( "US-SOMO: Compute structure factors : excluded volume" ),
                                                     QString( tr( "Excluded volume is set to %1\n" ) ).arg( saxs_map[ text ].volume ) );
                        }
                     }

                     if ( add_to_saxs_list )
                     {
                        saxs_list.push_back( saxs_map[ text ] );
                     } else {
                        for ( unsigned int i = 0; i < saxs_list.size(); i++ )
                        {
                           if ( saxs_list[ i ].saxs_name == text )
                           {
                              saxs_list[ i ] = saxs_map[ text ];
                           }
                        }
                     }

                     // now write out file
                     {
                        QString org_filename = our_saxs_options->default_saxs_filename;
                        bool ok;
                        QFile f;
                        QString filename;

                        do {
                           ok = false;
                           filename = QFileDialog::getSaveFileName( org_filename,
                                                                    "*.saxs_atoms *.SAXS_ATOMS", 
                                                                    this,
                                                                    "save file dialog",
                                                                    tr( "US-SOMO: Compute structure factors : save data" ) );
                           cout << QString( "filename is %1\n" ).arg( filename );
                           if ( !filename.isEmpty() )
                           {
                              f.setName( filename );
                              switch ( QMessageBox::question(
                                                            this,
                                                            tr( "US-SOMO: Compute structure factors : save data" ),
                                                            tr( "A file called %1 already exists."
                                                                "Do you want to overwrite it?" )
                                                            .arg( filename ),
                                                            tr( "&Yes" ), 
                                                            tr( "&No" ),
                                                            QString::null, 
                                                            1, 
                                                            0 ) )
                              {
                              case 0: 
                                 ok = true;
                                 break;
                              case 1:
                                 ok = false;
                                 break;
                              default:
                                 ok = false;
                                 break;
                              }
                                    
                              if ( ok && !filename.isEmpty() && !f.open( IO_WriteOnly | IO_Translate ) )
                              {
                                 QMessageBox::warning( this, 
                                                       tr( "US-SOMO: Compute structure factors : save data" ),
                                                       QString( tr( "Error trying to write the file %1 (possibly permissions)\n" ) )
                                                       .arg( filename ) );

                                 ok = false;
                              }
                           } else {
                              ok = true;
                           }
                        } while ( !ok );

                        if ( ok && !filename.isEmpty() )
                        {
                           
                           QTextStream ts( &f );
                           for ( unsigned int i=0; i < saxs_list.size(); i++ )
                           {
                              ts << saxs_list[i].saxs_name.upper() << "\t"
                                 << saxs_list[i].a[0] << "\t"
                                 << saxs_list[i].b[0] << "\t"
                                 << saxs_list[i].a[1] << "\t"
                                 << saxs_list[i].b[1] << "\t"
                                 << saxs_list[i].a[2] << "\t"
                                 << saxs_list[i].b[2] << "\t"
                                 << saxs_list[i].a[3] << "\t"
                                 << saxs_list[i].b[3] << "\t"
                                 << saxs_list[i].c << "\t"
                                 << saxs_list[i].volume << endl;
                              ts << saxs_list[i].saxs_name.upper() << "\t"
                                 << saxs_list[i].a5[0] << "\t"
                                 << saxs_list[i].b5[0] << "\t"
                                 << saxs_list[i].a5[1] << "\t"
                                 << saxs_list[i].b5[1] << "\t"
                                 << saxs_list[i].a5[2] << "\t"
                                 << saxs_list[i].b5[2] << "\t"
                                 << saxs_list[i].a5[3] << "\t"
                                 << saxs_list[i].b5[3] << "\t"
                                 << saxs_list[i].a5[4] << "\t"
                                 << saxs_list[i].b5[4] << "\t"
                                 << saxs_list[i].c5 << "\t"
                                 << saxs_list[i].volume << endl;
                           }
                           f.close();
                           if ( our_saxs_options->default_saxs_filename != filename )
                           {
                              our_saxs_options->default_saxs_filename = filename;
                              QMessageBox::information( this, 
                                                        tr( "US-SOMO: Compute structure factors : new default saxs filename" ),
                                                        QString( tr( 
                                                                    "The default saxs atoms file is now %1\n" 
                                                                    ) )
                                                        .arg( filename ) 
                                                        );
                              lbl_saxs_table->setText( QFileInfo( f ).fileName() );
                           }
                           if ( our_saxs_options->dummy_saxs_name != text )
                           {
                              our_saxs_options->dummy_saxs_name = text;
                              QMessageBox::information( this, 
                                                        tr( "US-SOMO: Compute structure factors : new default saxs dummy atom name" ),
                                                        QString( tr( 
                                                                    "The default dummy atom name is now %1\n" 
                                                                    ) )
                                                        .arg( text ) 
                                                        );
                           }                              
                           select_saxs_file( filename );
                           ((US_Hydrodyn *)us_hydrodyn)->saxs_util->select_saxs_file( filename );
                        }
                     }
                  }

                  break;
               case 2 : 
                  dupmsg = tr( "Name already exists, choose a unique name\n" );
                  break;
               default :
                  break;
               }
            }
         }
      } else {
         editor_msg( "dark red", QString( tr( "Skipped: Computing %1 term exponentials, since I(q) has errors" ) )
                     .arg( our_saxs_options->compute_exponential_terms ) );
      }
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

   long Iq = plot_saxs->insertCurve( name );
   plot_saxs->setCurveStyle(Iq, QwtCurve::Lines);
#else
   plotted_iq_names_to_pos[plot_name] = plotted_Iq_curves.size();

   QwtPlotCurve *curve = new QwtPlotCurve( name );
   curve->setStyle( QwtPlotCurve::Lines );
#endif

   plotted_q.push_back(q);
#ifndef QT4
   plotted_Iq.push_back(Iq);
#else
   plotted_Iq_curves.push_back( curve );
#endif
   {
      vector < double > q2 ( q.size() );
      for ( unsigned int i = 0; i < q.size(); i++ )
      {
         q2 [ i ] = q[i] * q[i];
      }
      plotted_q2 .push_back( q2  );
   }
   plotted_I.push_back(I);
   plotted_I_error.push_back(I_error);

   unsigned int q_points = q.size();
   unsigned int p = plotted_q.size() - 1;

   vector < double > q2I;
   if ( cb_kratky->isChecked() )
   {
      for ( unsigned int i = 0; i < plotted_q[ p ].size(); i++ )
      {
         q2I.push_back( plotted_q2[ p ][ i ] * plotted_I[ p ][ i ] );
      }
   }

#ifndef QT4
   plot_saxs->setCurveData(Iq, 
                           cb_guinier->isChecked() ?
                           (double *)&(plotted_q2[p][0])  : (double *)&(plotted_q[p][0]), 
                           cb_kratky ->isChecked() ?
                           (double *)&(q2I[0])            : (double *)&(plotted_I[p][0]),
                           q_points);
   plot_saxs->setCurvePen(Iq, QPen(plot_colors[p % plot_colors.size()], 2, SolidLine));
#else
   curve->setData(
                  cb_guinier->isChecked() ?
                  (double *)&(plotted_q2[p][0])  : (double *)&(plotted_q[p][0]), 
                  cb_kratky ->isChecked() ?
                  (double *)&(q2I[0])            : (double *)&(plotted_I[p][0]),
                  q_points
                  );
   curve->setPen( QPen( plot_colors[ p % plot_colors.size() ], 2, Qt::SolidLine ) );
   curve->attach( plot_saxs );
#endif

   if ( plot_saxs_zoomer )
   {
      delete plot_saxs_zoomer;
   }
   plot_saxs_zoomer = new ScrollZoomer(plot_saxs->canvas());
#ifndef QT4
   plot_saxs_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
   plot_saxs_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#else
   plot_saxs_zoomer->setRubberBandPen( QPen( Qt::red, 1, Qt::DotLine ) );
   plot_saxs_zoomer->setTrackerPen( QPen( Qt::red ) );
#endif

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
