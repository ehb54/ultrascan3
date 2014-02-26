#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_lm.h"
//Added by qt3to4:
#include <Q3TextStream>

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

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
   plot_pr->setCurvePen(ppr, QPen(plot_colors[p % plot_colors.size()], pen_width, SolidLine));
#else
   curve->setData(
                  (double *)&( r[ 0 ] ), 
                  (double *)&( pr[ 0 ] ),
                  (int)r.size()
                  );
   curve->setPen( QPen( plot_colors[ p % plot_colors.size() ], pen_width, Qt::SolidLine ) );
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

   editor_msg( plot_colors[p % plot_colors.size()], plot_saxs->canvasBackground(), name );

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
            text = text.upper();
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
            vector < double > coeffv;
            double            norm4;
            double            norm5;
            double            normv;
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
                                           coeffv,
                                           norm4,
                                           norm5,
                                           normv,
                                           our_saxs_options->bead_models_use_var_len_sf ?
                                           our_saxs_options->bead_models_var_len_sf_max : 5
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
                           filename = Q3FileDialog::getSaveFileName( org_filename,
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
                                    
                              if ( ok && !filename.isEmpty() && !f.open( QIODevice::WriteOnly | QIODevice::Text ) )
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
                           
                           Q3TextStream ts( &f );
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
   plot_saxs->setCurvePen(Iq, QPen(plot_colors[p % plot_colors.size()], pen_width, SolidLine));
#else
   curve->setData(
                  cb_guinier->isChecked() ?
                  (double *)&(plotted_q2[p][0])  : (double *)&(plotted_q[p][0]), 
                  cb_kratky ->isChecked() ?
                  (double *)&(q2I[0])            : (double *)&(plotted_I[p][0]),
                  q_points
                  );
   curve->setPen( QPen( plot_colors[ p % plot_colors.size() ], pen_width, Qt::SolidLine ) );
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

   editor_msg( plot_colors[p % plot_colors.size()], plot_saxs->canvasBackground(), name );

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

void US_Hydrodyn_Saxs::do_plot_resid()
{
   if ( !cb_guinier->isChecked() )
   {
      hide_widgets( resid_widgets );
      hide_widgets( manual_guinier_widgets );
      return;
   }

   plot_resid->clear();
   if ( plot_resid_zoomer )
   {
      delete plot_resid_zoomer;
      plot_resid_zoomer = (ScrollZoomer *) 0;
   }

#ifndef QT4
   int niqsize = (int)plotted_Iq.size();
#else
   int niqsize = (int)plotted_Iq_curves.size();
#endif
   bool any_plotted = false;

   bool first     = true;
   double minq2   = 0e0;
   double maxq2   = 0e0;
   double maxabse = 0e0;

   double plot_minx;
   double plot_maxx;

   plot_domain( plot_minx, plot_maxx );

   bool some_pts_skipped = false;

   if ( cb_cs_guinier->isChecked() )
   {
      for ( int i = 0; i < niqsize; i++ )
      {
         if ( plotted_cs_guinier_valid.count(i) &&
              plotted_cs_guinier_valid[i] == true )
         {
            vector < double > x;
            vector < double > y;
            vector < double > e;
            vector < double > x_d;
            vector < double > e_d;
            
            for ( int q = 0; q < (int)plotted_q2[ i ].size(); ++q )
            {
               if ( plotted_q2[ i ][ q ] >= plotted_cs_guinier_lowq2[ i ] &&
                    plotted_q2[ i ][ q ] <= plotted_cs_guinier_highq2[ i ] )
               {
                  x.push_back( plotted_q2[ i ][ q ] );
                  y.push_back( 0e0 );
                  
                  if ( cb_resid_pct->isChecked() )
                  {
                     e.push_back( 100e0 * ( log ( plotted_q[ i ][ q ] * plotted_I[ i ][ q ] > 0e0 ?
                                                  plotted_q[ i ][ q ] * plotted_I[ i ][ q ] : 1e-99 ) - 
                                            ( plotted_cs_guinier_a[i] + plotted_cs_guinier_b[i] * plotted_q2[ i ][ q ] ) )
                                  /
                                  fabs( log ( plotted_q[ i ][ q ] * plotted_I[ i ][ q ] > 0e0 ?
                                              plotted_q[ i ][ q ] * plotted_I[ i ][ q ] : 1e-99 ) )
                                  );
                  } else {
                     if ( cb_resid_sd->isChecked() )
                     {
                        if ( (int)plotted_I_error.size() > i &&
                             (int)plotted_I_error[ i ].size() > q &&
                             plotted_I_error[ i ][ q ] != 0e0 &&
                             plotted_I[ i ][ q ] != 0e0 )
                        {
                           e.push_back( ( log ( plotted_q[ i ][ q ] * plotted_I[ i ][ q ] > 0e0 ?
                                                plotted_q[ i ][ q ] * plotted_I[ i ][ q ] : 1e-99 ) - 
                                          ( plotted_cs_guinier_a[i] + plotted_cs_guinier_b[i] * plotted_q2[ i ][ q ] ) )
                                        / 
                                        ( plotted_I_error[ i ][ q ] / plotted_I[ i ][ q ] )
                                     );
                        } else {
                           e.push_back( 0e0 );
                           some_pts_skipped = true;
                        }
                     } else {
                        e.push_back( log ( plotted_q[ i ][ q ] * plotted_I[ i ][ q ] > 0e0 ?
                                           plotted_q[ i ][ q ] * plotted_I[ i ][ q ] : 1e-99 ) - 
                                     ( plotted_cs_guinier_a[i] + plotted_cs_guinier_b[i] * plotted_q2[ i ][ q ] )
                                     );
                     }
                  }

                  if ( first )
                  {
                     first = false;
                     minq2 = x.back();
                     maxq2 = x.back();
                     maxabse = fabs( e.back() );
                  } else {
                     if ( minq2 > x.back() )
                     {
                        minq2 = x.back();
                     }
                     if ( maxq2 < x.back() )
                     {
                        maxq2 = x.back();
                     }
                     if ( maxabse < fabs( e.back() ) )
                     {
                        maxabse = fabs( e.back() );
                     }
                  }
               } else {
                  if ( plotted_q2[ i ][ q ] >= plot_minx &&
                       plotted_q2[ i ][ q ] <= plot_maxx )
                  {
                     x_d.push_back( plotted_q2[ i ][ q ] );
                     if ( cb_resid_pct->isChecked() )
                     {
                        e_d.push_back( 100e0 * ( log ( plotted_q[ i ][ q ] * plotted_I[ i ][ q ] > 0e0 ?
                                                       plotted_q[ i ][ q ] * plotted_I[ i ][ q ] : 1e-99 ) - 
                                                 ( plotted_cs_guinier_a[i] + plotted_cs_guinier_b[i] * plotted_q2[ i ][ q ] ) )
                                       /
                                       fabs( log ( plotted_q[ i ][ q ] * plotted_I[ i ][ q ] > 0e0 ?
                                                   plotted_q[ i ][ q ] * plotted_I[ i ][ q ] : 1e-99 ) )
                                     );
                     } else {
                        if ( cb_resid_sd->isChecked() )
                        {
                           if ( (int)plotted_I_error[ i ].size() > q &&
                                plotted_I_error[ i ][ q ] != 0e0 &&
                                plotted_I[ i ][ q ] != 0e0 )
                           {
                              e_d.push_back( ( log ( plotted_q[ i ][ q ] * plotted_I[ i ][ q ] > 0e0 ?
                                                     plotted_q[ i ][ q ] * plotted_I[ i ][ q ] : 1e-99 ) - 
                                               ( plotted_cs_guinier_a[i] + plotted_cs_guinier_b[i] * plotted_q2[ i ][ q ] ) )
                                             / 
                                             ( plotted_I_error[ i ][ q ] / plotted_I[ i ][ q ] )
                                             );
                           } else {
                              e_d.push_back( 0e0 );
                              some_pts_skipped = true;
                           }
                        } else {
                           e_d.push_back( log ( plotted_q[ i ][ q ] * plotted_I[ i ][ q ] > 0e0 ?
                                                plotted_q[ i ][ q ] * plotted_I[ i ][ q ] : 1e-99 ) - 
                                          ( plotted_cs_guinier_a[i] + plotted_cs_guinier_b[i] * plotted_q2[ i ][ q ] )
                                          );
                        }
                     }

                     if ( first )
                     {
                        first = false;
                        minq2 = x_d.back();
                        maxq2 = x_d.back();
                        maxabse = fabs( e_d.back() );
                     } else {
                        if ( minq2 > x_d.back() )
                        {
                           minq2 = x_d.back();
                        }
                        if ( maxq2 < x_d.back() )
                        {
                           maxq2 = x_d.back();
                        }
                        if ( maxabse < fabs( e_d.back() ) )
                        {
                           maxabse = fabs( e_d.back() );
                        }
                     }
                  }
               }
            }
            if ( x.size() )
            {
               if ( plotted_cs_guinier_pts_removed.count( i ) &&
                    plotted_cs_guinier_pts_removed[ i ].size() )
               {
                  do_plot_resid( x, y, e, plotted_cs_guinier_pts_removed[ i ], plot_colors[i % plot_colors.size()] );
               } else {
                  do_plot_resid( x, y, e, plot_colors[i % plot_colors.size()] );
               }
            }
            if ( x_d.size() )
            {
               do_plot_resid( x_d, e_d, plot_colors[i % plot_colors.size()] );
            }

            any_plotted = true;
         }
      }
   } else {
      if ( cb_Rt_guinier->isChecked() )
      {
         for ( int i = 0; i < niqsize; i++ )
         {
            if ( plotted_Rt_guinier_valid.count(i) &&
                 plotted_Rt_guinier_valid[i] == true )
            {
               vector < double > x;
               vector < double > y;
               vector < double > e;
               vector < double > x_d;
               vector < double > e_d;
            
               for ( int q = 0; q < (int)plotted_q2[ i ].size(); ++q )
               {
                  if ( plotted_q2[ i ][ q ] >= plotted_Rt_guinier_lowq2[ i ] &&
                       plotted_q2[ i ][ q ] <= plotted_Rt_guinier_highq2[ i ] )
                  {
                     x.push_back( plotted_q2[ i ][ q ] );
                     y.push_back( 0e0 );
                  
                     if ( cb_resid_pct->isChecked() )
                     {
                        e.push_back( 100e0 * ( log ( plotted_q[ i ][ q ] * plotted_q[ i ][ q ] * plotted_I[ i ][ q ] > 0e0 ?
                                                     plotted_q[ i ][ q ] * plotted_q[ i ][ q ] * plotted_I[ i ][ q ] : 1e-99 ) - 
                                               ( plotted_Rt_guinier_a[i] + plotted_Rt_guinier_b[i] * plotted_q2[ i ][ q ] ) )
                                     /
                                     fabs( log ( plotted_q[ i ][ q ] * plotted_q[ i ][ q ] * plotted_I[ i ][ q ] > 0e0 ?
                                                 plotted_q[ i ][ q ] * plotted_q[ i ][ q ] * plotted_I[ i ][ q ] : 1e-99 ) )
                                     );
                     } else {
                        if ( cb_resid_sd->isChecked() )
                        {
                           if ( (int)plotted_I_error.size() > i &&
                                (int)plotted_I_error[ i ].size() > q &&
                                plotted_I_error[ i ][ q ] != 0e0 &&
                                plotted_I[ i ][ q ] != 0e0 )
                           {
                              e.push_back( ( log ( plotted_q[ i ][ q ] * plotted_q[ i ][ q ] * plotted_I[ i ][ q ] > 0e0 ?
                                                   plotted_q[ i ][ q ] * plotted_q[ i ][ q ] * plotted_I[ i ][ q ] : 1e-99 ) - 
                                             ( plotted_Rt_guinier_a[i] + plotted_Rt_guinier_b[i] * plotted_q2[ i ][ q ] ) )
                                           / 
                                           ( plotted_I_error[ i ][ q ] / plotted_I[ i ][ q ] )
                                           );
                           } else {
                              e.push_back( 0e0 );
                              some_pts_skipped = true;
                           }
                        } else {
                           e.push_back( log ( plotted_q[ i ][ q ] * plotted_q[ i ][ q ] * plotted_I[ i ][ q ] > 0e0 ?
                                              plotted_q[ i ][ q ] * plotted_q[ i ][ q ] * plotted_I[ i ][ q ] : 1e-99 ) - 
                                        ( plotted_Rt_guinier_a[i] + plotted_Rt_guinier_b[i] * plotted_q2[ i ][ q ] )
                                        );
                        }
                     }

                     if ( first )
                     {
                        first = false;
                        minq2 = x.back();
                        maxq2 = x.back();
                        maxabse = fabs( e.back() );
                     } else {
                        if ( minq2 > x.back() )
                        {
                           minq2 = x.back();
                        }
                        if ( maxq2 < x.back() )
                        {
                           maxq2 = x.back();
                        }
                        if ( maxabse < fabs( e.back() ) )
                        {
                           maxabse = fabs( e.back() );
                        }
                     }
                  } else {
                     if ( plotted_q2[ i ][ q ] >= plot_minx &&
                          plotted_q2[ i ][ q ] <= plot_maxx )
                     {
                        x_d.push_back( plotted_q2[ i ][ q ] );
                        if ( cb_resid_pct->isChecked() )
                        {
                           e_d.push_back( 100e0 * ( log ( plotted_q[ i ][ q ] * plotted_q[ i ][ q ] * plotted_I[ i ][ q ] > 0e0 ?
                                                          plotted_q[ i ][ q ] * plotted_q[ i ][ q ] * plotted_I[ i ][ q ] : 1e-99 ) - 
                                                    ( plotted_Rt_guinier_a[i] + plotted_Rt_guinier_b[i] * plotted_q2[ i ][ q ] ) )
                                          /
                                          fabs( log ( plotted_q[ i ][ q ] * plotted_q[ i ][ q ] * plotted_I[ i ][ q ] > 0e0 ?
                                                      plotted_q[ i ][ q ] * plotted_q[ i ][ q ] * plotted_I[ i ][ q ] : 1e-99 ) )
                                          );
                        } else {
                           if ( cb_resid_sd->isChecked() )
                           {
                              if ( (int)plotted_I_error[ i ].size() > q &&
                                   plotted_I_error[ i ][ q ] != 0e0 &&
                                   plotted_I[ i ][ q ] != 0e0 )
                              {
                                 e_d.push_back( ( log ( plotted_q[ i ][ q ] * plotted_q[ i ][ q ] * plotted_I[ i ][ q ] > 0e0 ?
                                                        plotted_q[ i ][ q ] * plotted_q[ i ][ q ] * plotted_I[ i ][ q ] : 1e-99 ) - 
                                                  ( plotted_Rt_guinier_a[i] + plotted_Rt_guinier_b[i] * plotted_q2[ i ][ q ] ) )
                                                / 
                                                ( plotted_I_error[ i ][ q ] / plotted_I[ i ][ q ] )
                                                );
                              } else {
                                 e_d.push_back( 0e0 );
                                 some_pts_skipped = true;
                              }
                           } else {
                              e_d.push_back( log ( plotted_q[ i ][ q ] * plotted_q[ i ][ q ] * plotted_I[ i ][ q ] > 0e0 ?
                                                   plotted_q[ i ][ q ] * plotted_q[ i ][ q ] * plotted_I[ i ][ q ] : 1e-99 ) - 
                                             ( plotted_Rt_guinier_a[i] + plotted_Rt_guinier_b[i] * plotted_q2[ i ][ q ] )
                                             );
                           }
                        }

                        if ( first )
                        {
                           first = false;
                           minq2 = x_d.back();
                           maxq2 = x_d.back();
                           maxabse = fabs( e_d.back() );
                        } else {
                           if ( minq2 > x_d.back() )
                           {
                              minq2 = x_d.back();
                           }
                           if ( maxq2 < x_d.back() )
                           {
                              maxq2 = x_d.back();
                           }
                           if ( maxabse < fabs( e_d.back() ) )
                           {
                              maxabse = fabs( e_d.back() );
                           }
                        }
                     }
                  }
               }
               if ( x.size() )
               {
                  if ( plotted_Rt_guinier_pts_removed.count( i ) &&
                       plotted_Rt_guinier_pts_removed[ i ].size() )
                  {
                     do_plot_resid( x, y, e, plotted_Rt_guinier_pts_removed[ i ], plot_colors[i % plot_colors.size()] );
                  } else {
                     do_plot_resid( x, y, e, plot_colors[i % plot_colors.size()] );
                  }
               }
               if ( x_d.size() )
               {
                  do_plot_resid( x_d, e_d, plot_colors[i % plot_colors.size()] );
               }

               any_plotted = true;
            }
         }
      } else {
         for ( int i = 0; i < niqsize; i++ )
         {
            if ( plotted_guinier_valid.count(i) &&
                 plotted_guinier_valid[i] == true )
            {
               vector < double > x;
               vector < double > y;
               vector < double > e;
               vector < double > x_d;
               vector < double > e_d;

               for ( int q = 0; q < (int)plotted_q2[ i ].size(); ++q )
               {
                  if ( plotted_q2[ i ][ q ] >= plotted_guinier_lowq2[ i ] &&
                       plotted_q2[ i ][ q ] <= plotted_guinier_highq2[ i ] )
                  {
                     x.push_back( plotted_q2[ i ][ q ] );
                     y.push_back( 0e0 );
                     if ( cb_resid_pct->isChecked() )
                     {
                        e.push_back( 100e0 * ( log ( plotted_I[ i ][ q ] > 0e0 ?
                                                     plotted_I[ i ][ q ] : 1e-99 ) - 
                                               ( plotted_guinier_a[i] + plotted_guinier_b[i] * plotted_q2[ i ][ q ] ) )
                                     /
                                     fabs( log ( plotted_I[ i ][ q ] > 0e0 ?
                                                 plotted_I[ i ][ q ] : 1e-99 ) )
                                     );
                     } else {
                        if ( cb_resid_sd->isChecked() )
                        {
                           if (
                               (int)plotted_I_error.size() > i &&
                               (int)plotted_I_error[ i ].size() > q &&
                               plotted_I_error[ i ][ q ] != 0e0 &&
                               plotted_I[ i ][ q ] != 0e0 )
                           {
                              e.push_back( ( log ( plotted_I[ i ][ q ] > 0e0 ?
                                                   plotted_I[ i ][ q ] : 1e-99 ) - 
                                             ( plotted_guinier_a[i] + plotted_guinier_b[i] * plotted_q2[ i ][ q ] ) )
                                           / 
                                           ( plotted_I_error[ i ][ q ] / plotted_I[ i ][ q ] )
                                           );
                           } else {
                              e.push_back( 0e0 );
                              some_pts_skipped = true;
                           }
                        } else {
                           e.push_back( log ( plotted_I[ i ][ q ] > 0e0 ?
                                              plotted_I[ i ][ q ] : 1e-99 ) - 
                                        ( plotted_guinier_a[i] + plotted_guinier_b[i] * plotted_q2[ i ][ q ] )
                                        );
                        }
                     }
                     if ( first )
                     {
                        first = false;
                        minq2 = x.back();
                        maxq2 = x.back();
                        maxabse = fabs( e.back() );
                     } else {
                        if ( minq2 > x.back() )
                        {
                           minq2 = x.back();
                        }
                        if ( maxq2 < x.back() )
                        {
                           maxq2 = x.back();
                        }
                        if ( maxabse < fabs( e.back() ) )
                        {
                           maxabse = fabs( e.back() );
                        }
                     }
                  } else {
                     if ( plotted_q2[ i ][ q ] >= plot_minx &&
                          plotted_q2[ i ][ q ] <= plot_maxx )
                     {
                        x_d.push_back( plotted_q2[ i ][ q ] );
                        if ( cb_resid_pct->isChecked() )
                        {
                           e_d.push_back( 100e0 * ( log ( plotted_I[ i ][ q ] > 0e0 ?
                                                          plotted_I[ i ][ q ] : 1e-99 ) - 
                                                    ( plotted_guinier_a[i] + plotted_guinier_b[i] * plotted_q2[ i ][ q ] ) )
                                          /
                                          fabs( log ( plotted_I[ i ][ q ] > 0e0 ?
                                                      plotted_I[ i ][ q ] : 1e-99 ) )
                                          );
                        } else {
                           if ( cb_resid_sd->isChecked() )
                           {
                              if ( (int)plotted_I_error[ i ].size() > q &&
                                   plotted_I_error[ i ][ q ] != 0e0 &&
                                   plotted_I[ i ][ q ] != 0e0 )
                              {
                                 e_d.push_back( ( log ( plotted_I[ i ][ q ] > 0e0 ?
                                                        plotted_I[ i ][ q ] : 1e-99 ) - 
                                                  ( plotted_guinier_a[i] + plotted_guinier_b[i] * plotted_q2[ i ][ q ] ) )
                                                / 
                                                ( plotted_I_error[ i ][ q ] / plotted_I[ i ][ q ] )
                                                );
                              } else {
                                 e_d.push_back( 0e0 );
                                 some_pts_skipped = true;
                              }
                           } else {
                              e_d.push_back( log ( plotted_I[ i ][ q ] > 0e0 ?
                                                   plotted_I[ i ][ q ] : 1e-99 ) - 
                                             ( plotted_guinier_a[i] + plotted_guinier_b[i] * plotted_q2[ i ][ q ] )
                                             );
                           }
                        }
                        if ( first )
                        {
                           first = false;
                           minq2 = x_d.back();
                           maxq2 = x_d.back();
                           maxabse = fabs( e_d.back() );
                        } else {
                           if ( minq2 > x_d.back() )
                           {
                              minq2 = x_d.back();
                           }
                           if ( maxq2 < x_d.back() )
                           {
                              maxq2 = x_d.back();
                           }
                           if ( maxabse < fabs( e_d.back() ) )
                           {
                              maxabse = fabs( e_d.back() );
                           }
                        }
                     }
                  }
               }
               if ( x.size() )
               {
                  if ( plotted_guinier_pts_removed.count( i ) &&
                       plotted_guinier_pts_removed[ i ].size() )
                  {
                     do_plot_resid( x, y, e, plotted_guinier_pts_removed[ i ], plot_colors[i % plot_colors.size()] );
                  } else {
                     do_plot_resid( x, y, e, plot_colors[i % plot_colors.size()] );
                  }
               }
               if ( x_d.size() )
               {
                  do_plot_resid( x_d, e_d, plot_colors[i % plot_colors.size()] );
               }
               any_plotted = true;
            }
         }
      }         
   }

   if ( some_pts_skipped )
   {
      editor_msg( "dark red", tr( "Warning: points with zero s.d.'s or I(q)'s are not displayed" ) );
   }

   if ( any_plotted )
   {
      plot_resid->setAxisTitle(QwtPlot::yLeft,
                               tr(
                                  cb_cs_guinier->isChecked() ?
                                  (  cb_resid_pct->isChecked() ?
                                     "% difference [100*(ln(q*I(q)) - Guinier)/ln(q*I(q))]" :
                                     ( cb_resid_sd->isChecked() ?
                                       "(ln(q*I(q)) - Guinier line) / S.D." : 
                                       "ln(q*I(q)) - Guinier line" 
                                       ) 
                                     ) 
                                  :
                                  ( cb_Rt_guinier->isChecked() ?
                                    (  cb_resid_pct->isChecked() ?
                                       "% difference [100*(ln(q^2*I(q)) - Guinier)/ln(q^2*I(q))]" :
                                       ( cb_resid_sd->isChecked() ?
                                         "(ln(q^2*I(q)) - Guinier line) / S.D." : 
                                         "ln(q^2*I(q)) - Guinier line" 
                                         ) 
                                       ) 
                                    :
                                    (  cb_resid_pct->isChecked() ?
                                       "% difference [100*(ln(I(q)) - Guinier)/ln(I(q))]" :
                                       ( cb_resid_sd->isChecked() ?
                                         "(ln(I(q)) - Guinier line) / S.D." :
                                         "ln(I(q)) - Guinier line" 
                                         ) 
                                       ) 
                                    )
                                  )
                                 );

      if ( cb_resid_pct->isChecked() )
      {
         if ( maxabse < 5e-1 )
         {
            maxabse = 5e-1;
         }
         if ( maxabse > 10e0 )
         {
            maxabse = 10e0;
         }
      }

      {
         vector < double > x_d2( 2 );
         vector < double > y_d( 2 );

         x_d2[ 0 ] = minq2;
         x_d2[ 1 ] = maxq2;

         y_d[ 0 ] = 0e0;
         y_d[ 1 ] = 0e0;

#ifndef QT4
         long curve;
         curve = plot_resid->insertCurve( "base" );
         plot_resid->setCurveStyle( curve, QwtCurve::Lines );
#else
         QwtPlotCurve *curve = new QwtPlotCurve( "base" );
         curve->setStyle( QwtPlotCurve::Lines );
#endif

#ifndef QT4
         plot_resid->setCurvePen( curve, QPen( Qt::darkRed, 1, Qt::DotLine ) );
         plot_resid->setCurveData( curve,
                                   (double *)&x_d2[ 0 ],
                                   (double *)&y_d[ 0 ],
                                   2
                                   );
#else
         curve->setPen( QPen( Qt::red, 1, Qt::DotLine ) );
         curve->setData(
                        (double *)&x_d2[ 0 ],
                        (double *)&y_d[ 0 ],
                        2
                        );
         curve->attach( plot_resid );
#endif
      }

      plot_resid->setAxisScale( QwtPlot::xBottom, minq2, maxq2 );
      plot_resid->setAxisScale( QwtPlot::yLeft  , -maxabse * 1.2e0 , maxabse * 1.2e0 );
      
      plot_resid_zoomer = new ScrollZoomer(plot_resid->canvas());
      plot_resid_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
#ifndef QT4
         plot_resid_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#endif
         // connect( plot_resid_zoomer, SIGNAL( zoomed( const QwtDoubleRect & ) ), SLOT( plot_zoomed( const QwtDoubleRect & ) ) );

      plot_resid->replot();
      set_resid_show();
      if ( started_in_expert_mode &&
           plotted_q.size() == 1 )
      {
         cb_manual_guinier->show();
         set_manual_guinier();
      } else {
         hide_widgets( manual_guinier_widgets );
         cb_manual_guinier->hide();
      }
   } else {
      hide_widgets( resid_widgets );
      hide_widgets( manual_guinier_widgets );
   }
}

void US_Hydrodyn_Saxs::set_resid_pct()
{
   if ( cb_resid_sd->isChecked() )
   {
      disconnect(cb_resid_sd, SIGNAL(clicked()), 0, 0 );
      cb_resid_sd->setChecked( false );
      connect(cb_resid_sd, SIGNAL(clicked()), SLOT(set_resid_sd()));
   }
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_cb_resid_pct" ] = cb_resid_pct->isChecked() ? "1" : "0"; 
   do_plot_resid();
}

void US_Hydrodyn_Saxs::set_resid_sd()
{
   if ( cb_resid_pct->isChecked() )
   {
      disconnect(cb_resid_pct, SIGNAL(clicked()), 0, 0 );
      cb_resid_pct->setChecked( false );
      connect(cb_resid_pct, SIGNAL(clicked()), SLOT(set_resid_pct()));
   }
   do_plot_resid();
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_cb_resid_sd" ] = cb_resid_sd->isChecked() ? "1" : "0"; 
}

void US_Hydrodyn_Saxs::set_manual_guinier()
{
   if ( cb_manual_guinier->isChecked() )
   {
      hide_widgets( manual_guinier_widgets, false );
   } else {
      hide_widgets( manual_guinier_widgets );
   }
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_cb_manual_guinier" ] = cb_resid_sd->isChecked() ? "1" : "0"; 
}

void US_Hydrodyn_Saxs::set_resid_show()
{
   if ( cb_resid_show->isChecked() )
   {
      hide_widgets( resid_widgets, false );
      if ( !started_in_expert_mode ||
           plotted_q.size() != 1 )
      {
         cb_manual_guinier->hide();
      }
   } else {
      hide_widgets( resid_widgets );
      cb_resid_show->show();
      cb_resid_show_errorbars->show();
      if ( started_in_expert_mode &&
           plotted_q.size() == 1 )
      {
         cb_manual_guinier->show();
      }
   }
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_cb_resid_show" ] = cb_resid_show->isChecked() ? "1" : "0";
}

void US_Hydrodyn_Saxs::set_resid_show_errorbars()
{
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_cb_resid_show_errorbars" ] = cb_resid_show_errorbars->isChecked() ? "1" : "0";
   set_guinier();
}


void US_Hydrodyn_Saxs::do_plot_resid( vector < double > & x, 
                                      vector < double > & y, 
                                      vector < double > & e,
                                      QColor qc )
{
   {
#ifndef QT4
      long curve;
      curve = plot_resid->insertCurve( "base" );
      plot_resid->setCurveStyle( curve, QwtCurve::Lines );
#else
      QwtPlotCurve *curve = new QwtPlotCurve( "base" );
      curve->setStyle( QwtPlotCurve::Lines );
#endif

#ifndef QT4
      plot_resid->setCurvePen( curve, QPen( Qt::darkRed, 1, Qt::SolidLine ) );
      plot_resid->setCurveData( curve,
                                 (double *)&x[ 0 ],
                                 (double *)&y[ 0 ],
                                 x.size()
                                 );
#else
      curve->setPen( QPen( Qt::red, 1, Qt::SolidLine ) );
      curve->setData(
                     (double *)&x[ 0 ],
                     (double *)&y[ 0 ],
                     x.size()
                     );
      curve->attach( plot_resid );
#endif
   }

   {
#ifndef QT4
      long curve;
      curve = plot_resid->insertCurve( "errors" );
      plot_resid->setCurveStyle( curve, QwtCurve::Lines );
#else
      QwtPlotCurve *curve = new QwtPlotCurve( "errors" );
      curve->setStyle( QwtPlotCurve::Lines );
#endif

#ifndef QT4
      plot_resid->setCurvePen( curve, QPen( qc, 1, Qt::SolidLine ) );
      plot_resid->setCurveData( curve,
                                 (double *)&x[ 0 ],
                                 (double *)&e[ 0 ],
                                 x.size()
                                 );
      plot_resid->curve( curve )->setStyle( QwtCurve::Sticks );
#else
      curve->setPen( QPen( qc, 1, Qt::SolidLine ) );
      curve->setData(
                     (double *)&x[ 0 ],
                     (double *)&e[ 0 ],
                     x.size()
                     );
      curve->setStyle( QwtPlotCurve::Sticks );
      curve->attach( plot_resid );
#endif
   }
}   

void US_Hydrodyn_Saxs::do_plot_resid( vector < double > & x_d, 
                                      vector < double > & e_d,
                                      QColor qc )
{
   {
#ifndef QT4
      long curve;
      curve = plot_resid->insertCurve( "errors" );
      plot_resid->setCurveStyle( curve, QwtCurve::Lines );
#else
      QwtPlotCurve *curve = new QwtPlotCurve( "errors" );
      curve->setStyle( QwtPlotCurve::Lines );
#endif

#ifndef QT4
      plot_resid->setCurvePen( curve, QPen( qc, 1, Qt::DotLine ) );
      plot_resid->setCurveData( curve,
                                 (double *)&x_d[ 0 ],
                                 (double *)&e_d[ 0 ],
                                 x_d.size()
                                 );
      plot_resid->curve( curve )->setStyle( QwtCurve::Sticks );
#else
      curve->setPen( QPen( qc, 1, Qt::DotLine ) );
      curve->setData(
                     (double *)&x_d[ 0 ],
                     (double *)&e_d[ 0 ],
                     x_d.size()
                     );
      curve->setStyle( QwtPlotCurve::Sticks );
      curve->attach( plot_resid );
#endif
   }
}   

void US_Hydrodyn_Saxs::do_plot_resid( vector < double > & x, 
                                      vector < double > & y, 
                                      vector < double > & e,
                                      map < double, double > & pts_removed,
                                      QColor qc )
{
   do_plot_resid( x, y, e, qc );
   // plot red X's 

   // puts( "do plot resid errs" );

   QwtSymbol sym;
   sym.setStyle(QwtSymbol::XCross);
   sym.setSize(10);
   sym.setBrush(Qt::red);
   sym.setPen  ( QPen( Qt::red ) );

   for ( int i = 0; i < (int)x.size(); i++ )
   {
      if ( pts_removed.count( x[ i ] ) )
      {
         // cout << QString( "do plot resid errs: %1 %2\n"  ).arg( x[ i ] ).arg( e[ i ] );
#ifndef QT4
         long marker = plot_resid->insertMarker();
         plot_resid->setMarkerSymbol( marker, sym );
         plot_resid->setMarkerPos   ( marker, x[ i ], e[ i ] );
#else
         QwtPlotMarker* marker = new QwtPlotMarker;
         marker->setSymbol( sym );
         marker->setValue( x[ i ], e[ i ] );
         marker->attach( plot_resid );
#endif
      }
   }
}   
