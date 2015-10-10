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
      errormsg = QString( tr( "Internal error: unified_ggaussians_to_f_gaussians(): common_size %1 + per_file_size %2 != gaussian_type_size %3\n" ) )
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
                                          vector < double > & fit
                                          )
{
   if ( !unified_ggaussian_ok )
   {
      editor_msg( "red", tr( "Internal error: gg_chi_fit_vector() called but unified gaussians not ok" ) );
      return false;
   }

   fit.clear();

   bool use_errors = unified_ggaussian_use_errors && cb_sd_weight->isChecked();

   // qDebug( use_errors ? "use errors ON" : "use errors OFF" );

   vector < double > tmp_g( gaussian_type_size );

   // need to redo this... see compute_ggaussian_gaussian_sum

   vector < double > this_t;
   vector < double > I;
   vector < double > e;

   unsigned int      t_size;
   double fit_max = -1e0;

   int pos;


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

      double rmsd = 0e0;
      double tmp;

      // qDebug( US_Vector::qs_vector3( "this_t, G, I", this_t, G, f_Is[ unified_ggaussian_files[ i ] ] ) );

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
         // qDebug( QString( "curve %1 nu %2 rmsd %3" ).arg( unified_ggaussian_files[ i ] ).arg( nu ).arg( rmsd / nu ) );
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

   // qDebug( US_Vector::qs_vector2( "q rmsd", unified_ggaussian_qvals, fit ) );
   
   ggqfit_plot->clear();
   
   ggqfit_plot->setAxisTitle( QwtPlot::yLeft, tr( use_errors ? "normalized Chi^2" : "RMSD" ) );


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
      }
   }

   // setup range, zoomer

   ggqfit_plot->setAxisScale( QwtPlot::xBottom, 0.95 * unified_ggaussian_qvals[0], 1.05 * unified_ggaussian_qvals.back() );
   ggqfit_plot->setAxisScale( QwtPlot::yLeft  , 0, fit_max * 1.15 );
      
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
      editor_msg( "red", tr( "Internal error: gaussian rmsd called but unified gaussians not ok" ) );
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
      editor_msg( "red", tr( "Internal error: gaussian rmsd called but unified gaussians not ok" ) );
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

   vector < double > fit;
   gg_fit_vector( fit );

   return sqrt( rmsd );
}
