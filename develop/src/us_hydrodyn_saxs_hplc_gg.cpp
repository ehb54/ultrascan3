#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_saxs_hplc.h"
// #include <assert.h>

bool US_Hydrodyn_Saxs_Hplc::unified_ggaussian_to_f_gaussians()
{
   if ( !unified_ggaussian_ok )
   {
      return false;
   }

   // cb_fix_width
   // cb_fix_dist1
   // cb_fix_dist2

   // layouts:
   // n is number of gaussians
   // layout of regular (file specific gaussians)
   // g=height,center,width,{dist1,{dist2}}
   // g1,g2,...,gn

   // layout of unified global gaussians
   // common portion (shared by all): f=center{,width}{,dist1}{,dist2}
   // f1,f2,...,fn
   // variable portion (per file): v=height{,width}{,dist1}{,dist2}
   // v1,v2,...,vn

   // these are both per gaussian:
   /* now handled in create_unified_ggaussian_target 
      unsigned int common_size   = 0;
      unsigned int per_file_size = 0;

      vector < bool >         is_common; // is common maps the offsets to layout of the regular file specific gaussians
      vector < unsigned int > offset;

      // height:
      is_common.push_back( false           );  // height always variable
      offset   .push_back( per_file_size++ );  // first variable entry

      // center
      is_common.push_back( true            );  // center always common
      offset   .push_back( common_size++   );  // first common entry

      // width
      if ( cb_fix_width->isChecked() )
      {
      is_common.push_back( true );
      offset   .push_back( common_size++   );  // first common entry
      } else {
      is_common.push_back( false );
      offset   .push_back( per_file_size++ );  // first variable entry
      }

      if ( dist1_active )
      {
      if ( cb_fix_dist1->isChecked() )
      {
      is_common.push_back( true );
      offset   .push_back( common_size++   );  // first common entry
      } else {
      is_common.push_back( false );
      offset   .push_back( per_file_size++ );  // first variable entry
      }
      if ( dist2_active )
      {
      if ( cb_fix_dist2->isChecked() )
      {
      is_common.push_back( true );
      offset   .push_back( common_size++   );  // first common entry
      } else {
      is_common.push_back( false );
      offset   .push_back( per_file_size++ );  // first variable entry
      }
      }
      }
   */

   // US_Vector::printvector( "unified_ggaussian_to_f_gaussians is_common", is_common );
   // US_Vector::printvector( "unified_ggaussian_to_f_gaussians offset"   , offset    );

   if ( common_size + per_file_size != (unsigned int)gaussian_type_size )
   {
      errormsg = QString( us_tr( "Internal error: unified_ggaussians_to_f_gaussians(): common_size %1 + per_file_size %2 != gaussian_type_size %3\n" ) )
         .arg( common_size )
         .arg( per_file_size )
         .arg( gaussian_type_size )
         ;
      editor_msg( "red", errormsg );
      return false;
   }

   for ( unsigned int i = 0; i < ( unsigned int ) unified_ggaussian_files.size(); i++ )
   {
      vector < double > g;
      unsigned int  index = common_size * unified_ggaussian_gaussians_size + i * per_file_size * unified_ggaussian_gaussians_size;

      for ( unsigned int j = 0; j < unified_ggaussian_gaussians_size; j++ )
      {
         for ( int k = 0; k < gaussian_type_size; ++k )
         {
            if ( is_common[ k ] )
            {
               g.push_back( unified_ggaussian_params[ offset[ k ] + common_size * j           ] );
            } else {
               g.push_back( unified_ggaussian_params[ offset[ k ] + per_file_size * j + index ] );
            }
         }               
      }
      f_gaussians[ unified_ggaussian_files[ i ] ] = g;
   }

   return true;
}

bool US_Hydrodyn_Saxs_Hplc::gg_fit_vector(
                                          vector < double > & fit,
                                          vector < double > & pfit
                                          )
{
   if ( !unified_ggaussian_ok )
   {
      editor_msg( "red", us_tr( "Internal error: gg_chi_fit_vector() called but unified gaussians not ok" ) );
      return false;
   }

   fit.clear();
   pfit.clear();
   ggaussian_last_pfit_N    .clear();
   ggaussian_last_pfit_C    .clear();
   ggaussian_last_pfit_S    .clear();
   ggaussian_last_gg        .clear();
   ggaussian_last_ggig      .clear();
   ggaussian_last_gg_t      .clear();
   ggaussian_last_I         .clear();
   ggaussian_last_e         .clear();
   ggaussian_pts_chi2       .clear();
   ggaussian_pts_pfit       .clear();
   ggaussian_pts_chi2_marked.clear();
   ggaussian_pts_pfit_marked.clear();
   ggaussian_last_pfit_map  .clear();

   bool use_errors = unified_ggaussian_use_errors && cb_sd_weight->isChecked();

   // us_qdebug( use_errors ? "use errors ON" : "use errors OFF" );

   vector < double > tmp_g( gaussian_type_size );

   // need to redo this... see compute_ggaussian_gaussian_sum

   vector < double > this_t;
   vector < double > I;
   vector < double > e;

   unsigned int      t_size;
   double fit_max = -1e0;

   double pfit_max = 0e0;
   double pfit_min = 1e0;

   int pos;

   int cormap_green  = 0;
   int cormap_yellow = 0;
   int cormap_red    = 0;

   double alpha        = 
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "hplc_cormap_alpha" ) 
      ? ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_cormap_alpha" ].toDouble() * 5e0 
      : 0.05;
   double alpha_over_5 = 0.2 * alpha;

   for ( int i = 0; i < (int) unified_ggaussian_curves; ++i )
   {
      t_size = unified_ggaussian_q_end[ i ] - unified_ggaussian_q_start[ i ];
      this_t.resize( t_size );
      I     .resize( t_size );
      pos = unified_ggaussian_q_start[ i ];
      if ( unified_ggaussian_use_errors )
      {
         e     .resize( t_size );
         for ( int t = 0; t < (int) t_size; ++t, ++pos )
         {
            this_t[ t ] = unified_ggaussian_q[ pos ];
            I     [ t ] = unified_ggaussian_I[ pos ];
            e     [ t ] = unified_ggaussian_e[ pos ];
         }
      } else {
         for ( int t = 0; t < (int) t_size; ++t, ++pos )
         {
            this_t[ t ] = unified_ggaussian_q[ pos ];
            I     [ t ] = unified_ggaussian_I[ pos ];
         }
      }         
      // build up g

      vector < double > g;
      unsigned int  index = common_size * unified_ggaussian_gaussians_size + i * per_file_size * unified_ggaussian_gaussians_size;

      for ( unsigned int j = 0; j < unified_ggaussian_gaussians_size; ++j )
      {
         for ( int k = 0; k < gaussian_type_size; ++k )
         {
            if ( is_common[ k ] )
            {
               g.push_back( unified_ggaussian_params[ offset[ k ] + common_size * j           ] );
            } else {
               g.push_back( unified_ggaussian_params[ offset[ k ] + per_file_size * j + index ] );
            }
         }               
      }
      
      vector < double > G = compute_gaussian_sum( this_t, g );
      ggaussian_last_ggig.push_back( compute_gaussian_sum_partials );

      // compute cormap p value
      {
         vector < double >            q = this_t;
         vector < vector < double > > Icm( 2 );
         vector < vector < double > > rkl;
         int                          N;
         int                          S;
         int                          C;
         double                       P;

         Icm[ 0 ] = I;
         Icm[ 1 ] = G;

         if ( !usu->cormap( q, Icm, rkl, N, S, C, P ) ) {
            editor_msg( "red", usu->errormsg );
         }
         // if ( P > 0.05 ) {
         //    P = 0.05;
         // } 
         if ( pfit_max < P )
         {
            pfit_max = P;
         }
         if ( pfit_min > P )
         {
            pfit_min = P;
            if ( pfit_min < 1e-10 ) {
               pfit_min = 1e-10;
            }
         }
         
         if ( P >= alpha ) {
            cormap_green++;
         } else {
            if ( P >= alpha_over_5 ) {
               cormap_yellow++;
            } else {
               cormap_red++;
            }
         }

         ggaussian_last_pfit_N.push_back( N );
         ggaussian_last_pfit_C.push_back( S );
         ggaussian_last_pfit_S.push_back( C );
         pfit.push_back( P );
         ggaussian_last_pfit_map[ unified_ggaussian_files[ i ] ] = P;
         ggaussian_last_gg    .push_back( G );
         ggaussian_last_gg_t  .push_back( this_t );
         ggaussian_last_I     .push_back( I );
         ggaussian_last_e     .push_back( e );
         // us_qdebug( QString( "p fit for %1 = %2" ).arg( unified_ggaussian_files[ i ] ).arg( pfit.back() ) );
      }

      double rmsd = 0e0;
      double tmp;

      // us_qdebug( US_Vector::qs_vector3( "this_t, G, I", this_t, G, f_Is[ unified_ggaussian_files[ i ] ] ) );

      int size = (int) G.size();

      if ( use_errors ) 
      {
         for ( int j = 0; j < size; ++j )
         {
            tmp = ( I[ j ] - G[ j ] ) / e[ j ];
            rmsd += tmp * tmp;
         }
         double nu = (double)( size - (int) g.size() - 1 );
         if ( nu <= 0e0 )
         {
            nu = (double) size;
            if ( nu <= 0e0 )
            {
               nu = 1e0;
            }
         }
         // us_qdebug( QString( "curve %1 nu %2 rmsd %3" ).arg( unified_ggaussian_files[ i ] ).arg( nu ).arg( rmsd / nu ) );
         rmsd /= nu;
      } else {
         for ( int j = 0; j < size; ++j )
         {
            tmp = ( I[ j ] - G[ j ] );
            rmsd += tmp * tmp;
         }
         rmsd = sqrt( rmsd );
      } 
      if ( fit_max < rmsd )
      {
         fit_max = rmsd;
      }
      fit.push_back( rmsd );
   }

   {
      int cormap_tot = cormap_green + cormap_yellow + cormap_red;
      if ( cormap_tot ) {
         double tot_c_pct     =  100e0 / (double) ( cormap_green + cormap_yellow + cormap_red );
         QString msg = 
            QString("").sprintf(
                                "P value analysis summary:\n"
                                " %5.1f%% P >= %.2f (%.1f%% P >= %.2f) + (%.1f%% %.2f > P >= %.2f) pairs\n"
                                " %5.1f%% %.2f > P pairs\n"
                                ,tot_c_pct * (double) (cormap_green + cormap_yellow )
                                ,alpha_over_5
                                ,tot_c_pct * (double) cormap_green
                                ,alpha
                                ,tot_c_pct * (double) cormap_yellow
                                ,alpha
                                ,alpha_over_5
                                ,tot_c_pct * (double) cormap_red
                                ,alpha_over_5
                                )
            ;
         editor_msg( "darkblue", msg );
      }
   }

   // us_qdebug( US_Vector::qs_vector2( "q rmsd", unified_ggaussian_qvals, fit ) );
   
   ggqfit_plot->clear();

   ggqfit_plot->enableAxis  ( QwtPlot::yLeft , false );
   ggqfit_plot->enableAxis  ( QwtPlot::yRight , false );
   
   if ( cb_ggq_plot_chi2->isChecked() ) {
      ggqfit_plot->enableAxis  ( QwtPlot::yLeft , true );
      
      ggqfit_plot->setAxisTitle( QwtPlot::yLeft, us_tr( use_errors ? "normalized Chi^2" : "RMSD" ) );

      {
         QPen use_pen = QPen( Qt::green, use_line_width, Qt::DotLine );

#ifdef QT4
         QwtPlotCurve * curve = new QwtPlotCurve( "ggqfit" );
         curve->setStyle( QwtPlotCurve::Lines );
         curve->setData(
                        (double *)&(unified_ggaussian_qvals[0]),
                        (double *)&(fit[0]),
                        unified_ggaussian_qvals.size() );
         curve->setPen( use_pen );
         curve->attach( ggqfit_plot  );
#else
         long curve = ggqfit_plot->insertCurve( "ggqfit" );
         ggqfit_plot->setCurveStyle( curve, QwtCurve::Lines );
         ggqfit_plot->setCurveData( curve,
                                    (double *)&(unified_ggaussian_qvals[0]),
                                    (double *)&(fit[0]),
                                    unified_ggaussian_qvals.size() );
         ggqfit_plot->setCurvePen( curve, use_pen );
#endif
      }

      // as markers
      {
         QwtSymbol sym;
         sym.setSize( use_line_width * 3 + 1 );
         sym.setStyle( QwtSymbol::Diamond );

         for ( int i = 0; i < (int) unified_ggaussian_files.size(); ++i )
         {
            QColor qc = plot_colors[ f_pos[ unified_ggaussian_files[ i ] ] % plot_colors.size() ];
            sym.setPen  ( qc );
            sym.setBrush( qc );

#ifdef QT4
            QwtPlotMarker* marker = new QwtPlotMarker;
            marker->setSymbol( sym );
            marker->setValue( unified_ggaussian_qvals[ i ], fit[ i ] );
            marker->attach( ggqfit_plot );
#else
            long marker = ggqfit_plot->insertMarker();
            ggqfit_plot->setMarkerSymbol( marker, sym );
            ggqfit_plot->setMarkerPos   ( marker, unified_ggaussian_qvals[ i ], fit[ i ] );
#endif
            ggaussian_pts_chi2.push_back( marker );
         }
      }
      ggqfit_plot->setAxisScale( QwtPlot::yLeft  , 0, fit_max * 1.15 );
   }

   if ( cb_ggq_plot_P->isChecked() ) {
      ggqfit_plot->enableAxis  ( QwtPlot::yRight , true );
      ggqfit_plot->setAxisTitle( QwtPlot::yRight, us_tr( "P value (log scale)" ) );

//    // pvalue
//    {
//       QPen use_pen = QPen( Qt::yellow, use_line_width, Qt::DotLine );

// #ifdef QT4
//       QwtPlotCurve * curve = new QwtPlotCurve( "ggqfit_p_value" );
//       curve->setStyle( QwtPlotCurve::Lines );
//       curve->setData(
//                      (double *)&(unified_ggaussian_qvals[0]),
//                      (double *)&(pfit[0]),
//                      unified_ggaussian_qvals.size() );
//       curve->setPen( use_pen );
//       curve->setAxis  ( QwtPlot::xBottom , QwtPlot::yRight );
//       curve->attach( ggqfit_plot  );
// #else
//       long curve = ggqfit_plot->insertCurve( "ggqfit_p_values", QwtPlot::xBottom, QwtPlot::yRight );
//       ggqfit_plot->setCurveStyle( curve, QwtCurve::Lines );
//       ggqfit_plot->setCurveData( curve,
//                                  (double *)&(unified_ggaussian_qvals[0]),
//                                  (double *)&(pfit[0]),
//                                  unified_ggaussian_qvals.size() );
//       ggqfit_plot->setCurvePen( curve, use_pen );
// #endif
//    }


      // make horizontal yellow & green lines

      {
         vector < double > x( 2 );
         vector < double > y( 2 );

         x[ 0 ] = 0e0;
         x[ 1 ] = 7;

         {
            y[ 0 ] = 0.05;
            y[ 1 ] = 0.05;
         
         
            QPen use_pen = QPen( Qt::green, use_line_width, Qt::DashDotLine );

#ifdef QT4
            QwtPlotCurve * curve = new QwtPlotCurve( "ggqfit_p_alpha_green" );
            curve->setStyle( QwtPlotCurve::Lines );
            curve->setData(
                           (double *)&(x[0]),
                           (double *)&(y[0]),
                           2 );
            curve->setPen( use_pen );
            curve->setAxis  ( QwtPlot::xBottom , QwtPlot::yRight );
            curve->attach( ggqfit_plot  );
#else
            long curve = ggqfit_plot->insertCurve( "ggqfit_p_alpha_green", QwtPlot::xBottom, QwtPlot::yRight );
            ggqfit_plot->setCurveStyle( curve, QwtCurve::Lines );
            ggqfit_plot->setCurveData( curve,
                                       (double *)&(x[0]),
                                       (double *)&(y[0]),
                                       2 );
            ggqfit_plot->setCurvePen( curve, use_pen );
#endif
         }

         {
            y[ 0 ] = 0.01;
            y[ 1 ] = 0.01;
         
            QPen use_pen = QPen( Qt::yellow, use_line_width, Qt::DashDotLine );

#ifdef QT4
            QwtPlotCurve * curve = new QwtPlotCurve( "ggqfit_p_alpha_green" );
            curve->setStyle( QwtPlotCurve::Lines );
            curve->setData(
            (double *)&(x[0]),
               (double *)&(y[0]),
               2 );
            curve->setPen( use_pen );
            curve->setAxis  ( QwtPlot::xBottom , QwtPlot::yRight );
            curve->attach( ggqfit_plot  );
#else
            long curve = ggqfit_plot->insertCurve( "ggqfit_p_alpha_green", QwtPlot::xBottom, QwtPlot::yRight );
            ggqfit_plot->setCurveStyle( curve, QwtCurve::Lines );
            ggqfit_plot->setCurveData( curve,
               (double *)&(x[0]),
               (double *)&(y[0]),
               2 );
            ggqfit_plot->setCurvePen( curve, use_pen );
#endif
         }
      }

   // P as markers
      {
         QwtSymbol sym;
         sym.setSize( use_line_width * 2 + 1 );
         sym.setStyle( QwtSymbol::Rect );

         for ( int i = 0; i < (int) unified_ggaussian_files.size(); ++i )
         {
            QColor qc = plot_colors[ f_pos[ unified_ggaussian_files[ i ] ] % plot_colors.size() ];
            sym.setPen  ( qc );
            sym.setBrush( qc );

            double P = pfit[ i ];
            if ( P <= 1e-10 ) {
               P = 1e-10;
            }

#ifdef QT4
            QwtPlotMarker* marker = new QwtPlotMarker;
            marker->setSymbol( sym );
            marker->setValue( unified_ggaussian_qvals[ i ], P );
            marker->setAxis  ( QwtPlot::xBottom , QwtPlot::yRight );
            marker->attach( ggqfit_plot );
#else
            long marker = ggqfit_plot->insertMarker( "", QwtPlot::xBottom , QwtPlot::yRight );
            ggqfit_plot->setMarkerSymbol( marker, sym );
            ggqfit_plot->setMarkerPos   ( marker, unified_ggaussian_qvals[ i ], P );
#endif
            ggaussian_pts_pfit.push_back( marker );
         }
      }
      ggqfit_plot->setAxisScale( QwtPlot::yRight  , pfit_min * 0.95, pfit_max * 1.15 );
      // ggqfit_plot->setAxisScale( QwtPlot::yRight  , 0, 0.06 );
   }

   // setup range, zoomer

   ggqfit_plot->setAxisScale( QwtPlot::xBottom, 0.95 * unified_ggaussian_qvals[0], 1.05 * unified_ggaussian_qvals.back() );
      
   if ( ggqfit_plot_zoomer )
   {
      delete ggqfit_plot_zoomer;
   }
   ggqfit_plot_zoomer = new ScrollZoomer(ggqfit_plot->canvas());
   ggqfit_plot_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
#ifndef QT4
   ggqfit_plot_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#endif

   ggqfit_plot->replot();

   return false;
}

vector < double > US_Hydrodyn_Saxs_Hplc::compute_ggaussian_gaussian_sum()
{
   vector < double > result;
   if ( !unified_ggaussian_ok )
   {
      editor_msg( "red", us_tr( "Internal error: gaussian rmsd called but unified gaussians not ok" ) );
      return result;
   }

   // assert( unified_ggaussian_curves > 1 && "unified_ggaussian_curves > 1" );

   result.resize( unified_ggaussian_I.size() );

   // cout << QString( "cggs: unified_ggaussian_I.size() %1\n" ).arg( result.size() );

   vector < double > this_t;
   unsigned int      t_size;

   for ( unsigned int i = 0; i < unified_ggaussian_curves; ++i )
   {
      t_size = unified_ggaussian_q_end[ i ] - unified_ggaussian_q_start[ i ];
      this_t.resize( t_size );
      /*
      cout << QString( "cggs: curve %1 q start %2 q end %3 t_size %4\n" )
         .arg( i )
         .arg( unified_ggaussian_q_start[ i ] )
         .arg( unified_ggaussian_q_end[ i ] )
         .arg( t_size )
         ;         
      */
      for ( unsigned int t = 0; t < t_size; ++t )
      {
         this_t[ t ] = unified_ggaussian_q[ unified_ggaussian_q_start[ i ] + t ];
      }

      // build up g

      vector < double > g;
      unsigned int  index = common_size * unified_ggaussian_gaussians_size + i * per_file_size * unified_ggaussian_gaussians_size;

      for ( unsigned int j = 0; j < unified_ggaussian_gaussians_size; ++j )
      {
         for ( int k = 0; k < gaussian_type_size; ++k )
         {
            if ( is_common[ k ] )
            {
               g.push_back( unified_ggaussian_params[ offset[ k ] + common_size * j           ] );
            } else {
               g.push_back( unified_ggaussian_params[ offset[ k ] + per_file_size * j + index ] );
            }
         }               
      }
      
      // US_Vector::printvector( QString( "cggs: file %1, this_t" ).arg( i ), this_t );
      // US_Vector::printvector( QString( "cggs: file %1, g" ).arg( i ), g );
      vector < double > this_result = compute_gaussian_sum( this_t, g );

      for ( unsigned int t = 0; t < t_size; ++t )
      {
         result[ unified_ggaussian_q_start[ i ] + t ] = this_result[ t ];
      }
   }
   return result;
}

double US_Hydrodyn_Saxs_Hplc::ggaussian_rmsd( bool norm_chi )
{
   if ( !unified_ggaussian_ok )
   {
      editor_msg( "red", us_tr( "Internal error: gaussian rmsd called but unified gaussians not ok" ) );
      return 1e99;
   }

   vector < double > result = compute_ggaussian_gaussian_sum();

   double rmsd = 0e0;

   if ( unified_ggaussian_use_errors && cb_sd_weight->isChecked() )
   {
      for ( unsigned int i = 0; i < ( unsigned int ) result.size(); i++ )
      {
         double tmp = ( result[ i ] - unified_ggaussian_I[ i ] ) / unified_ggaussian_e[ i ];
         rmsd += tmp * tmp;
      }
      if ( norm_chi )
      {
         double nu = (double)( (int) result.size() - (int) unified_ggaussian_params.size() - 1 );
         if ( nu <= 0e0 )
         {
            nu = 1e0;
         }
         rmsd /= nu;
      }
   } else {
      for ( unsigned int i = 0; i < ( unsigned int ) result.size(); i++ )
      {
         double tmp = result[ i ] - unified_ggaussian_I[ i ];
         rmsd += tmp * tmp;
      }
   }

   //    printvector( "rmsd, ugq", unified_ggaussian_q );

   update_plot_errors( unified_ggaussian_t, unified_ggaussian_I, result, unified_ggaussian_e );
   plot_errors_jump_markers();

   // vector < double > fit;
   //   vector < double > pfit;
   gg_fit_vector( ggaussian_last_chi2, ggaussian_last_pfit_P );

   return sqrt( rmsd );
}

bool US_Hydrodyn_Saxs_Hplc::gg_fit_replot() {
   if ( !unified_ggaussian_ok ) {
      editor_msg( "red", us_tr( "Internal error (gg_fit_replot): Global Gaussian mode, but unified Global Gaussians are not ok." ) );
      ggaussian_enables();
      return false;
   }
            
   gg_fit_vector( ggaussian_last_chi2, ggaussian_last_pfit_P );
   return true;
}
