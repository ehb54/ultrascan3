#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_lm.h"
//Added by qt3to4:
#include <QTextStream>
#include <qwt_plot_layout.h>

// note: this program uses cout and/or cerr and this should be replaced

#define TSO QTextStream(stdout)

void US_Hydrodyn_Saxs::plot_one_pr(vector < double > r, vector < double > pr, QString name, bool skip_mw ) {
   qDebug() << "plot_one_pr() old style pr call, DEPRECATE!";
   vector < double > pr_error;
   return plot_one_pr( r, pr, pr_error, name, skip_mw );
}

void US_Hydrodyn_Saxs::plot_one_pr(
                                   vector < double > r
                                   ,vector < double > pr
                                   ,vector < double > pr_error
                                   ,QString name
                                   ,bool /* skip_mw */
                                   ,bool do_replot
                                   ) {
   if ( r.size() < pr.size() )
   {
      pr.resize(r.size());
   }
   if ( pr.size() < r.size() )
   {
      r.resize(pr.size());
   }

   if ( pr_error.size() ) {
      
      if ( pr_error.size() < pr.size()
           || US_Saxs_Util::is_zero_vector( pr_error ) ) {
         pr_error.clear();
      } else {
         pr_error.resize( pr.size() );
      }
   }

   set_pr_sd( r, pr, pr_error );
   crop_pr_tail( r, pr, pr_error );

   plotted_r                      .push_back(r);
   plotted_pr_not_normalized      .push_back(pr);
   plotted_pr_not_normalized_error.push_back(pr_error);

   // skip_mw = !cb_normalize->isChecked();

   plotted_pr_mw.push_back(
                           cb_normalize->isChecked()
                           ? get_mw( name, false, true )
                           : -1e0
                           );
   
   if ( plotted_pr_mw.back() == -1 ) {
      plotted_pr_mw.back() = 1;
      cb_normalize->setChecked( false );
   }

   if ( cb_normalize->isChecked() )
   {
      normalize_pr(r, &pr, &pr_error, get_mw(name, do_replot));
   }

   plotted_pr                     .push_back(pr);
   plotted_pr_error               .push_back(pr_error);


   QString plot_name = name;
   int extension = 0;
   while ( dup_plotted_pr_name_check.count(plot_name) )
   {
      plot_name = QString("%1-%2").arg(name).arg(++extension);
   }
   qsl_plotted_pr_names << plot_name;
   dup_plotted_pr_name_check[plot_name] = true;
   unsigned int p = plotted_r.size() - 1;
                  
   {
      if ( cb_pr_eb->isChecked() && pr_error.size() && pr_error.size() == r.size() ) {
         {
            QwtPlotCurve *curve = new QwtPlotCurve( plot_name );
            curve->setStyle( QwtPlotCurve::NoCurve );
            curve->setSamples(
                              (double *)&( r[ 0 ] ), 
                              (double *)&( pr[ 0 ] ),
                              (int)r.size()
                              );
            curve->setPen( QPen( plot_colors[ p % plot_colors.size() ], 6 + pen_width * 1, Qt::SolidLine ) );
            QwtSymbol sym;
            sym.setStyle(QwtSymbol::Diamond);
            sym.setSize( pen_width );
            sym.setPen(QPen(plot_colors[p % plot_colors.size()]));
            sym.setBrush(Qt::white);
            curve->setSymbol( new QwtSymbol( sym.style(), sym.brush(), sym.pen(), sym.size() ) );
            curve->attach( plot_pr );
         }

         for ( int i = 0; i < (int) pr_error.size(); ++i ) {
            QwtPlotCurve *curve = new QwtPlotCurve( UPU_EB_PREFIX + name );
            vector < double > eb_x(2, r[i]);
            vector < double > eb_y(2);
            eb_y[0] = pr[i] - pr_error[i];
            eb_y[1] = pr[i] + pr_error[i];
         
            curve->setStyle( QwtPlotCurve::Lines );

            curve->setSamples(
                              (double *)&( eb_x[ 0 ] ), 
                              (double *)&( eb_y[ 0 ] ),
                              2
                              );
            curve->setPen( QPen( plot_colors[ p % plot_colors.size() ], pen_width, Qt::SolidLine ) );
            curve->setItemAttribute( QwtPlotItem::Legend, false );
            curve->attach( plot_pr );
         }
      } else {
         QwtPlotCurve *curve = new QwtPlotCurve( plot_name );
         curve->setStyle( QwtPlotCurve::Lines );

         curve->setSamples(
                           (double *)&( r[ 0 ] ), 
                           (double *)&( pr[ 0 ] ),
                           (int)r.size()
                           );
         curve->setPen( QPen( plot_colors[ p % plot_colors.size() ], pen_width, Qt::SolidLine ) );
         curve->attach( plot_pr );
      }
   }

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
   plot_pr_zoomer->setRubberBandPen( QPen( Qt::red, 1, Qt::DotLine ) );
   plot_pr_zoomer->setTrackerPen( QPen( Qt::red ) );

   if ( do_replot ) {
      plot_pr->replot();
   }
                  
   if ( !plotted )
   {
      plotted = true;
      if ( do_replot ) {
         editor->append("P(r) plot legend:\n");
      }
   }

   if ( do_replot ) {
      editor_msg( plot_colors[p % plot_colors.size()], plot_saxs->canvasBackground().color(), name );
   }

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
         bool has_error = plotted_pr_error[ i ].size() == plotted_pr[ i ].size();
         double this_miny = plotted_pr[ i ][ 0 ];
         double this_maxy = plotted_pr[ i ][ 0 ];
         if ( has_error ) {
            this_miny -= plotted_pr_error[ i ][ 0 ];
            this_maxy += plotted_pr_error[ i ][ 0 ];
         }
         if ( has_error ) {
            for ( unsigned int j = 1; j < plotted_pr[ i ].size(); j++ ) {
               if ( this_miny > plotted_pr[ i ][ j ] - plotted_pr_error[ i ][ j ] ) {
                  this_miny = plotted_pr[ i ][ j ] - plotted_pr_error[ i ][ j ] ;
               }
               if ( this_maxy < plotted_pr[ i ][ j ] + plotted_pr_error[ i ][ j ] ) {
                  this_maxy = plotted_pr[ i ][ j ] + plotted_pr_error[ i ][ j ];
               }
            }
         } else {
            for ( unsigned int j = 1; j < plotted_pr[ i ].size(); j++ ) {
               if ( this_miny > plotted_pr[ i ][ j ] ) {
                  this_miny = plotted_pr[ i ][ j ];
               }
               if ( this_maxy < plotted_pr[ i ][ j ] ){
                  this_maxy = plotted_pr[ i ][ j ];
               }
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
         TSO << QString( "ff computes %1 ffe computes %2\n" ).arg( ffv ).arg( ffe );
      }
#endif                     
      if ( is_zero_vector( I_error ) )
      {
         bool ok;
         QString text = "";
         QString dupmsg;
         do {
            dupmsg = "";
            text = US_Static::getText(
                                         us_tr("US-SOMO: Compute structure factors"),
                                         us_tr( "If you want to compute structure factors for this curve\n"
                                             "Please define a dummy saxs name or press CANCEL to skip this computation.\n"
                                             + dupmsg +
                                             "Name for these structure factors:" )
                                         , 
                                         QLineEdit::Normal,
                                         text, 
                                         &ok, 
                                         this );
            text.replace( QRegExp( "\\s" ), "_" );
            text = text.toUpper();
            if ( ok && !text.isEmpty() )
            {
               // user entered something and pressed OK
               dupmsg = "";
               if ( saxs_map.count( text ) )
               {
                  switch ( QMessageBox::question(this, 
                                                 us_tr( "US-SOMO: Compute structure factors : duplicate name" ),
                                                 QString( us_tr( "The name %1 was previously defined\n"
                                                              "Do you want to overwrite the values?" ) ).arg( text ),
                                                 us_tr( "&Overwrite" ), 
                                                 us_tr( "&Try again" ),
                                                 QString(),
                                                 1,
                                                 0
                                                 ) )
                  {
                  case 0 : 
                     break;
                  case 1 : 
                     dupmsg = us_tr( "Name already exists, choose a unique name\n" );
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
            TSO << "compute exponentials\n" << flush;
            editor_msg( "blue", us_tr( "Computing 4 & 5 term exponentials" ) );
            QMessageBox::information( this, 
                                      us_tr( "US-SOMO: Compute structure factors : start computation" ),
                                      QString( us_tr( "Start computation\n"
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
               TSO << usu.errormsg << endl;
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
               TSO << "Terms4: " << qs4 << endl;
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
               TSO << "Terms5: " << qs5 << endl;
               editor_msg( "dark blue", QString( "Exponentials 5 terms norm %1 nnorm %2 (%3): %4" )
                           .arg( norm5 )
                           .arg( nnorm5 )
                           .arg( norm5tag )
                           .arg( qs5 ) );
               switch ( QMessageBox::question(this, 
                                              us_tr( "US-SOMO: Compute structure factors : computed" ),
                                              QString( us_tr( "Saxs name %1\n"
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
                                              us_tr("&Save to file"), 
                                              us_tr("&Forget them") 
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
                        double res = US_Static::getDouble(
                                                             us_tr( "US-SOMO: Compute structure factors : enter excluded volume" ),
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
                                                     us_tr( "US-SOMO: Compute structure factors : excluded volume" ),
                                                     QString( us_tr( "Excluded volume is set to %1\n" ) ).arg( saxs_map[ text ].volume ) );
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
                           filename = QFileDialog::getSaveFileName( this , us_tr( "US-SOMO: Compute structure factors : save data" ) , org_filename , "*.saxs_atoms *.SAXS_ATOMS" );

                           TSO << QString( "filename is %1\n" ).arg( filename );
                           if ( !filename.isEmpty() )
                           {
                              f.setFileName( filename );
                              switch ( QMessageBox::question(
                                                            this,
                                                            us_tr( "US-SOMO: Compute structure factors : save data" ),
                                                            us_tr( "A file called %1 already exists."
                                                                "Do you want to overwrite it?" )
                                                            .arg( filename ),
                                                            us_tr( "&Yes" ), 
                                                            us_tr( "&No" ),
                                                            QString(), 
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
                                                       us_tr( "US-SOMO: Compute structure factors : save data" ),
                                                       QString( us_tr( "Error trying to write the file %1 (possibly permissions)\n" ) )
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
                              ts << saxs_list[i].saxs_name.toUpper() << "\t"
                                 << saxs_list[i].a[0] << "\t"
                                 << saxs_list[i].b[0] << "\t"
                                 << saxs_list[i].a[1] << "\t"
                                 << saxs_list[i].b[1] << "\t"
                                 << saxs_list[i].a[2] << "\t"
                                 << saxs_list[i].b[2] << "\t"
                                 << saxs_list[i].a[3] << "\t"
                                 << saxs_list[i].b[3] << "\t"
                                 << saxs_list[i].c << "\t"
                                 << saxs_list[i].volume << Qt::endl;
                              ts << saxs_list[i].saxs_name.toUpper() << "\t"
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
                                 << saxs_list[i].volume << Qt::endl;
                           }
                           f.close();
                           if ( our_saxs_options->default_saxs_filename != filename )
                           {
                              our_saxs_options->default_saxs_filename = filename;
                              QMessageBox::information( this, 
                                                        us_tr( "US-SOMO: Compute structure factors : new default saxs filename" ),
                                                        QString( us_tr( 
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
                                                        us_tr( "US-SOMO: Compute structure factors : new default saxs dummy atom name" ),
                                                        QString( us_tr( 
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
                  dupmsg = us_tr( "Name already exists, choose a unique name\n" );
                  break;
               default :
                  break;
               }
            }
         }
      } else {
         editor_msg( "dark red", QString( us_tr( "Skipped: Computing %1 term exponentials, since I(q) has errors" ) )
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
      
   plotted_iq_names_to_pos[plot_name] = plotted_Iq.size();


   QwtPlotCurve *curve = new QwtPlotCurve( name );
   curve->setStyle( QwtPlotCurve::Lines );

   plotted_q.push_back(q);
   plotted_Iq.push_back( curve );
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

   {
      vector < double > q;
      vector < double > q2;
      vector < double > I;

      if ( cb_kratky->isChecked() ) {
         q  = plotted_q[ p ];
         q2 = plotted_q2[ p ];
         for ( unsigned int i = 0; i < q_points; ++i ) {
            I.push_back( plotted_q2[ p ][ i ] * plotted_I[ p ][ i ] );
         }
      } else {
         for ( unsigned int i = 0; i < q_points; ++i ) {
            if ( plotted_I[ p ][ i ] > 0e0 ) {
               q .push_back( plotted_q [ p ][ i ] );
               q2.push_back( plotted_q2[ p ][ i ] );
               I .push_back( plotted_I [ p ][ i ] );
            }
         }
      }
         
      curve->setSamples(
                     cb_guinier->isChecked() ? (double *)&(q2[0]) : (double *)&(q[0]), 
                     (double *)&(I[0]),
                     q.size() 
                     );
      curve->setPen( QPen( plot_colors[ p % plot_colors.size() ], pen_width, Qt::SolidLine ) );
      curve->attach( plot_saxs );
   }

   // if ( plot_saxs_zoomer )
   // {
   //    delete plot_saxs_zoomer;
   // }
   // plot_saxs_zoomer = new ScrollZoomer(plot_saxs->canvas());
   // plot_saxs_zoomer->setRubberBandPen( QPen( Qt::red, 1, Qt::DotLine ) );
   // plot_saxs_zoomer->setTrackerPen( QPen( Qt::red ) );
   plot_saxs_zoomer->setZoomBase();
   plot_saxs->replot();

   if ( !plotted )
   {
      plotted = true;
      editor->append("I(q) vs q plot legend:\n");
   }

   editor_msg( plot_colors[p % plot_colors.size()], plot_saxs->canvasBackground().color(), name );

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

   plot_resid->detachItems( QwtPlotItem::Rtti_PlotCurve ); plot_resid->detachItems( QwtPlotItem::Rtti_PlotMarker );;
   // if ( plot_resid_zoomer )
   // {
   //    delete plot_resid_zoomer;
   //    plot_resid_zoomer = (ScrollZoomer *) 0;
   // }

   int niqsize = (int)plotted_Iq.size();

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
      editor_msg( "dark red", us_tr( "Warning: points with zero s.d.'s or I(q)'s are not displayed" ) );
   }

   if ( any_plotted )
   {
#if defined( OLD_WAY )
      plot_resid->setAxisTitle(QwtPlot::yLeft,
                               us_tr(
                                  cb_cs_guinier->isChecked() ?
                                  (  cb_resid_pct->isChecked() ?
                                     // "% diff [100*(ln(q*I(q)) - Guin.)/ln(q*I(q))]" :
                                     "Residual %" :
                                     ( cb_resid_sd->isChecked() ?
                                       // "(ln(q*I(q)) - Guinier line) / S.D." : 
                                       "Residual / SD" : 
                                       // "ln(q*I(q)) - Guinier line" 
                                       "Residual" 
                                       ) 
                                     ) 
                                  :
                                  ( cb_Rt_guinier->isChecked() ?
                                    (  cb_resid_pct->isChecked() ?
                                       // "% diff [100*(ln(q^2*I(q)) - Guin.)/ln(q^2*I(q))]" :
                                       "Residual %" :
                                       ( cb_resid_sd->isChecked() ?
                                         // "(ln(q^2*I(q)) - Guinier line) / S.D." : 
                                         "Residual / SD" : 
                                         // "ln(q^2*I(q)) - Guinier line" 
                                         "Residual" 
                                         ) 
                                       ) 
                                    :
                                    (  cb_resid_pct->isChecked() ?
                                       // "% diff [100*(ln(I(q)) - Guin.)/ln(I(q))]" :
                                       "Residual %" :
                                       ( cb_resid_sd->isChecked() ?
                                         // "(ln(I(q)) - Guinier line) / S.D." :
                                         "Residual / SD" :
                                         // "ln(I(q)) - Guinier line"
                                         "Residual"
                                         )
                                       ) 
                                    )
                                  )
                                 );

#endif
      plot_resid->setAxisTitle(QwtPlot::yLeft, us_tr( "Residual" ) );

      US_Plot_Util::align_plot_extents( { plot_saxs, plot_resid } );

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

         QwtPlotCurve *curve = new QwtPlotCurve( "base" );
         curve->setStyle( QwtPlotCurve::Lines );

         curve->setPen( QPen( Qt::red, 1, Qt::DotLine ) );
         curve->setSamples(
                        (double *)&x_d2[ 0 ],
                        (double *)&y_d[ 0 ],
                        2
                        );
         curve->attach( plot_resid );
      }

      if ( cb_guinier->isChecked() ) {
         minq2 =
            le_guinier_lowq2->text().toDouble() ?
            le_guinier_lowq2->text().toDouble() :
            minq2;

         maxq2 = 
            le_guinier_highq2->text().toDouble() ?
            le_guinier_highq2->text().toDouble() :
            maxq2;
      }

      plot_saxs->setAxisScale( QwtPlot::xBottom, minq2, maxq2 );
      plot_saxs_zoomer->setZoomBase();
      plot_resid->setAxisScale( QwtPlot::xBottom, minq2, maxq2 );
      plot_resid->setAxisScale( QwtPlot::yLeft  , -maxabse * 1.2e0 , maxabse * 1.2e0 );
      plot_resid_zoomer->setZoomBase();
      
      // plot_resid_zoomer = new ScrollZoomer(plot_resid->canvas());
      // plot_resid_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
      // connect( plot_resid_zoomer, SIGNAL( zoomed( const QRectF & ) ), SLOT( resid_zoomed( const QRectF & ) ) );

      plot_saxs->replot();
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

void US_Hydrodyn_Saxs::resid_zoomed( const QRect & ) {
   // qDebug() << "_Saxs::resid_zoomed()";
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

   // plot_resid->plotLayout()->setAlignCanvasToScale( QwtPlot::xTop,    !plot_resid->plotLayout()->alignCanvasToScale( QwtPlot::xTop    ) );
   // plot_saxs ->plotLayout()->setAlignCanvasToScale( QwtPlot::xBottom, !plot_saxs ->plotLayout() ->alignCanvasToScale( QwtPlot::xBottom ) );
   do_plot_resid();
   emit pb_rescale_y->clicked();
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
   emit pb_rescale_y->clicked();
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
      US_Plot_Util::align_plot_extents( { plot_saxs, plot_resid } );
      usp_plot_saxs->rescale_mode = US_Plot::MODE_NO_SCALE;
      usp_plot_saxs->scroll_zoomer = plot_saxs_zoomer;
      usp_plot_resid->rescale_mode = US_Plot::MODE_NO_SCALE;

      usp_plot_resid->scroll_zoomer = plot_resid_zoomer;

      plot_saxs->setObjectName( "plot_saxs" );
      usp_plot_saxs->setObjectName( "usp_plot_saxs" );
      plot_resid->setObjectName( "plot_resid" );
      usp_plot_resid->setObjectName( "usp_plot_resid" );

      connect(((QObject*)plot_saxs ->axisWidget(QwtPlot::xBottom)) , SIGNAL(scaleDivChanged () ), usp_plot_resid, SLOT(scaleDivChangedXSlot () ), Qt::UniqueConnection );
      connect(((QObject*)plot_resid->axisWidget(QwtPlot::xBottom)) , SIGNAL(scaleDivChanged () ), usp_plot_saxs , SLOT(scaleDivChangedXSlot () ), Qt::UniqueConnection );
      plot_saxs->enableAxis( QwtPlot::xBottom, false );
      // gets in the was of manual settings
      // qbl_plots->setStretchFactor( plot_saxs, 80 );
      // qbl_plots->setStretchFactor( qbl_resid, 20 );
      hide_widgets( resid_widgets, false );
      if ( !started_in_expert_mode ||
           plotted_q.size() != 1 )
      {
         cb_manual_guinier->hide();
      }
   } else {
      plot_saxs->enableAxis( QwtPlot::xBottom, true );
      // qbl_plots->setStretchFactor( plot_saxs, 0 );
      // qbl_plots->setStretchFactor( qbl_resid, 0 );
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
   emit pb_rescale_y->clicked();
}


void US_Hydrodyn_Saxs::do_plot_resid( vector < double > & x, 
                                      vector < double > & y, 
                                      vector < double > & e,
                                      QColor qc )
{
   {
      QwtPlotCurve *curve = new QwtPlotCurve( "base" );
      curve->setStyle( QwtPlotCurve::Lines );

      curve->setPen( QPen( Qt::red, 1, Qt::SolidLine ) );
      curve->setSamples(
                     (double *)&x[ 0 ],
                     (double *)&y[ 0 ],
                     x.size()
                     );
      curve->attach( plot_resid );
   }

   {
      QwtPlotCurve *curve = new QwtPlotCurve( "errors" );
      curve->setStyle( QwtPlotCurve::Lines );

      curve->setPen( QPen( qc, 1, Qt::SolidLine ) );
      curve->setSamples(
                     (double *)&x[ 0 ],
                     (double *)&e[ 0 ],
                     x.size()
                     );
      curve->setStyle( QwtPlotCurve::Sticks );
      curve->attach( plot_resid );
   }
}   

void US_Hydrodyn_Saxs::do_plot_resid( vector < double > & x_d, 
                                      vector < double > & e_d,
                                      QColor qc )
{
   {
      QwtPlotCurve *curve = new QwtPlotCurve( "errors" );
      curve->setStyle( QwtPlotCurve::Lines );

      curve->setPen( QPen( qc, 1, Qt::DotLine ) );
      curve->setSamples(
                     (double *)&x_d[ 0 ],
                     (double *)&e_d[ 0 ],
                     x_d.size()
                     );
      curve->setStyle( QwtPlotCurve::Sticks );
      curve->attach( plot_resid );
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
         // TSO << QString( "do plot resid errs: %1 %2\n"  ).arg( x[ i ] ).arg( e[ i ] );
         QwtPlotMarker* marker = new QwtPlotMarker;
         marker->setSymbol( new QwtSymbol( sym.style(), sym.brush(), sym.pen(), sym.size() ) );
         marker->setValue( x[ i ], e[ i ] );
         marker->attach( plot_resid );
      }
   }
}   

void US_Hydrodyn_Saxs::pp() 
{
   QString use_dir = QDir::currentPath();
   ((US_Hydrodyn  *)us_hydrodyn)->select_from_directory_history( use_dir, this );
   QString fn = 
      QFileDialog::getSaveFileName( this , us_tr( "Select a prefix name to save the plot data" ) , use_dir , "*.csv" );

   if ( fn.isEmpty() )
   {
      return;
   }

   fn = QFileInfo( fn ).path() + QDir::separator() + QFileInfo( fn ).completeBaseName();

   QString errors;
   QString messages;

   map < QString, QwtPlot *>  use_plot_info = plot_info;

   for ( map < QString, US_Hydrodyn_Saxs_Iqq_Residuals *>::iterator it = saxs_iqq_residuals_windows.begin();
         it != saxs_iqq_residuals_windows.end();
         ++it ) {
      if ( it->second &&
           saxs_iqq_residuals_widgets.count( it->first ) &&
           saxs_iqq_residuals_widgets[ it->first ] &&
           it->second->plot ) {
         use_plot_info[ "US-SOMO SAXS Residuals " + it->first ] = it->second->plot;
      }
   }

   if ( saxs_residuals_widget ) {
      use_plot_info[ "US-SOMO SAXS PRR Residuals" ] = saxs_residuals_window->plot;
   }

   // save and restore flags since SD errorbars duplicate Err. Maybe should remove sd errorbars flag or mirror
   bool eb_set = cb_eb                  ->isChecked();
   bool sd_set = cb_resid_show_errorbars->isChecked();
   
   if ( eb_set || sd_set ) {
      cb_eb                  ->setChecked( true );
      cb_resid_show_errorbars->setChecked( false );
      set_eb();
      set_resid_show_errorbars();
   }

   if ( !US_Plot_Util::printtofile( fn, use_plot_info, errors, messages ) )
   {
      editor_msg( "red", errors );
   } else {
      editor_msg( "blue", messages );
   }

   cb_eb                  ->setChecked( eb_set );
   cb_resid_show_errorbars->setChecked( sd_set );
   set_eb();
   set_resid_show_errorbars();

}

void US_Hydrodyn_Saxs::set_eb() 
{
   // us_qdebug( "set_eb(), calling set_guinier()" );
   set_guinier();
}

void US_Hydrodyn_Saxs::set_pr_eb() 
{
   bool had_legend = pr_legend_vis;
   pr_replot();
   pr_legend_vis = had_legend;
   set_pr_legend();
}

void US_Hydrodyn_Saxs::set_width() 
{
   pen_width++;
   if ( pen_width > 5 )
   {
      pen_width = 1;
   }
   set_guinier();

   replot_pr();
}

void US_Hydrodyn_Saxs::replot_pr()
{
   QwtPlotItemList ilist = plot_pr->itemList();
   for ( int ii = 0; ii < ilist.size(); ii++ )
   {
      QwtPlotItem* plitem = ilist[ ii ];
      if ( plitem->rtti() != QwtPlotItem::Rtti_PlotCurve ) {
         continue;
      }
         
      QwtPlotCurve* curve = (QwtPlotCurve*) plitem;
      QPen qp = curve->pen();
      qp.setWidth( pen_width );
      curve->setPen( qp );
   }
   
   plot_pr->replot();
}

void US_Hydrodyn_Saxs::fix_xBottom() {
   plot_saxs->enableAxis( QwtPlot::xBottom, !plot_resid->isVisible() );
}

void US_Hydrodyn_Saxs::do_rescale() {
   // qDebug() << "do_rescale";
}

void US_Hydrodyn_Saxs::do_rescale_y() {
   qDebug() << "do_rescale_y";
   map < QString, QwtPlot *>            plot_info;
   map < QwtPlot *, ScrollZoomer * >    plot_to_zoomer;

   plot_info[ "plot_saxs" ]  = plot_saxs;
   plot_info[ "plot_resid" ] = plot_resid;
   
   plot_to_zoomer[ plot_saxs ]           = plot_saxs_zoomer;
   plot_to_zoomer[ plot_resid ]          = plot_resid_zoomer;

   US_Plot_Util::rescale( plot_info, plot_to_zoomer, true, true );

   // try maxHeight ? - this works but rescaling is an issue

   plot_resid->setMaximumHeight( (int) ( 0.2 * ( plot_resid->height() + plot_saxs->height() ) ) );
   // plot_resid->replot();
   emit plot_resid->resized();

   // try setting autoscale?

   // static int autoscale_cnt = 0;
   // static bool autoscale = false;
   
   // autoscale_cnt++;
   // if ( !(autoscale_cnt % 3 ) ) {
   //    autoscale = !autoscale;
   // }
   // qDebug() << "autoscale: " << ( autoscale ? "true" : "false" );
   // plot_resid->setAxisAutoScale( QwtPlot::yLeft, autoscale );
   // plot_resid->replot();


   // try set align to scale?
   // static bool align = false;

   // align = !align;
   // qDebug() << "set resid pct align toggle: " << ( align ? "true" : "false" );
   // plot_resid->plotLayout()->setAlignCanvasToScales( align );
   // plot_saxs ->plotLayout()->setAlignCanvasToScales( align );
   // plot_resid->replot();
   // plot_saxs->replot();
}

void US_Hydrodyn_Saxs::resid_resized() {
   // qDebug() << "resid_resized";
   plot_resid->setMaximumHeight( (int) ( 0.2 * ( plot_resid->height() + plot_saxs->height() ) ) );
   plot_resid->replot();
}

void US_Hydrodyn_Saxs::pr_replot() {

   // save plot info
   
   vector < vector < double > > save_plotted_pr                      = plotted_pr;
   vector < vector < double > > save_plotted_pr_error                = plotted_pr_error;
   vector < vector < double > > save_plotted_pr_not_normalized       = plotted_pr_not_normalized;
   vector < vector < double > > save_plotted_pr_not_normalized_error = plotted_pr_not_normalized_error;
   vector < vector < double > > save_plotted_r                       = plotted_r;
   vector < float >             save_plotted_pr_mw                   = plotted_pr_mw;
   QStringList                  save_qsl_plotted_pr_names            = qsl_plotted_pr_names;

   // clear plots
   clear_plot_pr( true );
   
   // replot with plot_one_pr ...

   for ( int i = 0; i < (int) save_plotted_r.size(); ++i ) {
      plot_one_pr(
                  save_plotted_r[i]
                  ,save_plotted_pr[i]
                  ,save_plotted_pr_error[i]
                  ,save_qsl_plotted_pr_names[i]
                  ,true
                  ,false
                  );
   }

   plot_pr->replot();
}

void US_Hydrodyn_Saxs::set_pr_sd(
                                 vector < double > & r
                                 ,vector < double > & pr
                                 ,vector < double > & pr_error
                                 ) {
   if ( !pr_error.size() ) {
      // no errors
      return;
   }

   if ( !r.size() ) {
      qDebug() << "error set_pr_sd() r is empty";
      return;
   }

   if ( r.size() != pr.size() || r.size() != pr_error.size() ) {
      qDebug() << "error set_pr_sd() r,pr,pr_error size mismatch";
      return;
   }

   for ( int i = 0; i < (int) pr_error.size(); ++i ) {
      if ( pr_error[i] < 0 ) {
         qDebug() << "error set_pr_sd() negative pr_error!";
      }
   }

   // 1st deal with r == 0
   if (
       r[0] == 0
       && pr[0] == 0
       && pr_error[0] == 0
       ) {
      list < double > sd_gz;
      for ( int i = 0; i < (int) pr_error.size(); ++i ) {
         if ( pr_error[i] > 0 ) {
            sd_gz.push_back( pr_error[i] );
         }
      }
      sd_gz.sort();
      pr_error[0] = sd_gz.front() * 1e-3;
   }

   // 2nd deal with tail
   prop_pr_sd_tail( pr_error, (int) pr_error.size() );
   
   // 3rd check for internal zeros
   {
      bool has_non_pos_sds = false;
      for ( int i = 0; i < (int) pr_error.size(); ++i ) {
         if ( pr_error[i] <= 0 ) {
            has_non_pos_sds = true;
         }
      }
      if ( has_non_pos_sds ) {
         // use natural spline
         US_Saxs_Util usu;
         vector < double > y2;
         vector < double > r_nz;
         vector < double > pr_error_nz;
         for ( int i = 0; i < (int) r.size(); ++i ) {
            if ( pr_error[i] > 0 ) {
               r_nz       .push_back( r[i] );
               pr_error_nz.push_back( pr_error[ i ] );
            }
         }
         usu.natural_spline( r_nz, pr_error_nz, y2 );
         for ( int i = 0; i < (int) pr_error.size(); ++i ) {
            if ( pr_error[i] <= 0 ) {
               double new_pr_sd;
               if ( !usu.apply_natural_spline( r_nz, pr_error_nz, y2, r[i], new_pr_sd ) ) {
                  qDebug() <<  "error set_pr_sd() error applying natural spline";
               } else {
                  pr_error[i] = new_pr_sd;
               }
            }
         }
         
         // verify natural spline worked
         has_non_pos_sds = false;
         for ( int i = 0; i < (int) pr_error.size(); ++i ) {
            if ( pr_error[i] <= 0 ) {
               has_non_pos_sds = true;
            }
         }
         if ( has_non_pos_sds ) {
            qDebug() <<  "error set_pr_sd() still has non positive SDs after natural spline!";
            return;
         }
      }
   }
}

void US_Hydrodyn_Saxs::crop_pr_tail(
                                    vector < double > & r
                                    ,vector < double > & pr
                                    ,vector < double > & pr_error
                                    ) {
   bool pop_pr_error = pr_error.size() != 0;
   
   while( pr.size() > 1 && pr.back() == 0 && *std::prev( pr.end(), 2 ) == 0 ) {
      r       .pop_back();
      pr      .pop_back();
      if ( pop_pr_error ) {
         pr_error.pop_back();
      }
   }
}

void US_Hydrodyn_Saxs::prop_pr_sd_tail(
                                       vector < double > & pr_error
                                       ,unsigned int len
                                       ) {
   if ( !pr_error.size() ) {
      // no errors
      return;
   }

   // take care of any existing trailing zero sds

   if ( pr_error.back() == 0 ) {
      // find last non-zero
      double last_nz_sd = 0;
      for ( int i = (int) pr_error.size() - 1; i > 0; --i ) {
         if ( pr_error[i] > 0 ) {
            last_nz_sd = pr_error[i];
            break;
         }
      }
      if ( last_nz_sd == 0 ) {
         qDebug() << "error set_pr_sd() could not find nonzero sd";
         return;
      }
            
      for ( int i = (int) pr_error.size() - 1; i > 0; --i ) {
         if ( pr_error[i] == 0 ) {
            pr_error[i] = last_nz_sd;
         } else {
            break;
         }
      }
   }

   // resize to requested len using sd value
   pr_error.resize( len, pr_error.back() );
}

void US_Hydrodyn_Saxs::pad_pr_plotted() {
   // sets all to max size
   qDebug() << "pad_pr_plotted() - not yet";
   // possible reinterpolate to common grid (current bin size?)
   // then pad tails with 0s and apply sds

   if ( !plotted_r.size() ) {
      return;
   }
   if ( plotted_r[0].size() < 2 ) {
      qDebug() << "error - unexpected r grid on curve 0";
      return;
   }

   double max_r      = plotted_r[0].back();
   double r_grid     = plotted_r[0][1] - plotted_r[0][0];
   bool needs_interp = false;
   // bool matches_bin  = r_grid == (double)our_saxs_options->bin_size;
   int max_grid_pos  = 0;

   for ( int i = 1; i < (int) plotted_r.size(); ++i ) {
      if ( plotted_r[i].size() < 2 ) {
         qDebug() << "error - unexpected r grid on curve " << i;
         return;
      }

      if ( max_r < plotted_r[i].back() ) {
         max_r = plotted_r[i].back();
         max_grid_pos = i;
      }
      
      if ( !needs_interp ) {
         double this_r_grid = plotted_r[i][1] - plotted_r[i][0];
         if ( this_r_grid != r_grid ) {
            needs_interp = true;
         }
      }
   }

   if ( needs_interp ) {
      qDebug() << "need to interpolate, not padded";
      editor_msg( "darkred", "Loaded curves have varying r spacing, interpolating all to current bin size\n" );
      
      vector < vector < double > > new_r                      (plotted_r.size());
      vector < vector < double > > new_pr                     (plotted_r.size());
      vector < vector < double > > new_pr_error               (plotted_r.size());
      vector < vector < double > > new_pr_not_normalized      (plotted_r.size());
      vector < vector < double > > new_pr_not_normalized_error(plotted_r.size());

      bool ok = true;

      for ( int i = 0; i < (int) plotted_r.size(); ++i ) {
         if ( plotted_r[i].size() > 2 ) {
            for ( double r = 0; r <= plotted_r[i].back(); r += (double)our_saxs_options->bin_size ) {
               new_r[i].push_back( r );
            }

            if ( !interpolate( new_r[i], plotted_r[i], plotted_pr[i], new_pr[i]) ) {
               editor_msg( "red", QString( "Error interpolating p(r) grid to current binsize %1\n" ).arg( our_saxs_options->bin_size ) );
               ok = false;
               break;
            }

            if ( plotted_pr_error[i].size() ) {
               if ( !interpolate( new_r[i], plotted_r[i], plotted_pr_error[i], new_pr_error[i] ) ) {
                  ok = false;
                  break;
               }
            }
            
            if ( !interpolate( new_r[i], plotted_r[i], plotted_pr_not_normalized[i], new_pr_not_normalized[i] ) ) {
               ok = false;
               break;
            }

            if ( plotted_pr_not_normalized_error[i].size() ) {
               if ( !interpolate( new_r[i], plotted_r[i], plotted_pr_not_normalized_error[i], new_pr_not_normalized_error[i] ) ) {
                  ok = false;
                  break;
               }
            }
         }
      }
      
      if ( !ok ) {
         editor_msg( "red", QString( "Error interpolating p(r) grid to current binsize %1\n" ).arg( our_saxs_options->bin_size ) );
         return;
      } else {
         plotted_r                       = new_r;
         plotted_pr                      = new_pr;
         plotted_pr_error                = new_pr_error;
         plotted_pr_not_normalized       = new_pr_not_normalized;
         plotted_pr_not_normalized_error = new_pr_not_normalized_error;

         max_r      = plotted_r[0].back();
         r_grid     = plotted_r[0][1] - plotted_r[0][0];
         needs_interp = false;
         // matches_bin  = r_grid == (double)our_saxs_options->bin_size;
         max_grid_pos  = 0;

         for ( int i = 1; i < (int) plotted_r.size(); ++i ) {
            if ( plotted_r[i].size() < 2 ) {
               qDebug() << "error - unexpected r grid on curve " << i;
               return;
            }

            if ( max_r < plotted_r[i].back() ) {
               max_r = plotted_r[i].back();
               max_grid_pos = i;
            }
      
            if ( !needs_interp ) {
               double this_r_grid = plotted_r[i][1] - plotted_r[i][0];
               if ( this_r_grid != r_grid ) {
                  needs_interp = true;
               }
            }
         }
      }

      if ( needs_interp ) {
         editor_msg( "red", QString( "error: incompatible r spacing after interpolation\n" ) );
         return;
      }
   }

   int max_size = (int) plotted_r[ max_grid_pos ].size();

   for ( int i = 0; i < (int) plotted_r.size(); ++i ) {
      plotted_r[i] = plotted_r[ max_grid_pos ];
      plotted_pr[i].resize( max_size, 0 );
      prop_pr_sd_tail( plotted_pr_error[i], max_size );
      plotted_pr_not_normalized[i].resize( max_size, 0 );
      prop_pr_sd_tail( plotted_pr_not_normalized_error[i], max_size );
   }
}

void US_Hydrodyn_Saxs::clear_plot_pr( bool full_clear ) {
   if ( full_clear ) {
      plotted_pr                     .clear();
      plotted_pr_error               .clear();
      plotted_pr_not_normalized      .clear();
      plotted_pr_not_normalized_error.clear();
      plotted_r                      .clear();
      plotted_pr_mw                  .clear();
      qsl_plotted_pr_names           .clear();
      dup_plotted_pr_name_check      .clear();
      plot_pr->detachItems( QwtPlotItem::Rtti_PlotCurve ); plot_pr->detachItems( QwtPlotItem::Rtti_PlotMarker );;
      plot_pr->replot();
      pr_legend_vis = false;
      set_pr_legend();
      return;
   }
   // if plotted data with errors & those without errors exist, clear those without errors
   // if only plotted data with errors exist, drop the last one

   bool save_pr_legend_vis = pr_legend_vis;
   
   int total_pr = (int) plotted_pr.size();
   if ( total_pr <= 1 ) {
      return clear_plot_pr( true );
   }
   
   if (
       (int) plotted_r.size() != total_pr
       || (int) plotted_pr_error.size() != total_pr
       || (int) plotted_pr_not_normalized.size() != total_pr
       || (int) plotted_pr_not_normalized_error.size() != total_pr
       || (int) plotted_pr_mw.size() != total_pr
       || (int) qsl_plotted_pr_names.size() != total_pr ) {
      qDebug() << "internal inconsistency, clearing all!";
      return clear_plot_pr( true );
   }

   vector < int > pr_with_error_index;

   for ( int i = 0; i < total_pr; ++i ) {
      if ( plotted_pr_error[i].size() == plotted_r[i].size() ) {
         pr_with_error_index.push_back( i );
      }
   }
   
   if ( !pr_with_error_index.size() ) {
      // nothing with error, clear all
      return clear_plot_pr( true );
   }      

   bool save_pr_eb                           = cb_pr_eb->isChecked();
   auto save_plotted_pr                      = plotted_pr;
   auto save_plotted_pr_error                = plotted_pr_error;
   auto save_plotted_pr_not_normalized       = plotted_pr_not_normalized;
   auto save_plotted_pr_not_normalized_error = plotted_pr_not_normalized_error;
   auto save_plotted_r                       = plotted_r;
   auto save_plotted_pr_mw                   = plotted_pr_mw;
   auto save_qsl_plotted_pr_names            = qsl_plotted_pr_names;
   auto save_dup_plotted_pr_name_check       = dup_plotted_pr_name_check;

   if ( (int) pr_with_error_index.size() == total_pr ) {
      pr_with_error_index.pop_back();
   }

   clear_plot_pr( true );
   
   cb_pr_eb->setChecked( save_pr_eb );

   for ( int i = 0; i < (int) pr_with_error_index.size(); ++i ) {
      plot_one_pr(
                  save_plotted_r[pr_with_error_index[i]]
                  ,save_plotted_pr[pr_with_error_index[i]]
                  ,save_plotted_pr_error[pr_with_error_index[i]]
                  ,save_qsl_plotted_pr_names[pr_with_error_index[i]]
                  ,true
                  ,false
                  );
   }

   plot_pr->replot();
   pr_legend_vis = save_pr_legend_vis;
   set_pr_legend();
}
