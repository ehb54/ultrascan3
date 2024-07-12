#include "../include/us_hydrodyn.h"
#include "../include/us_fractal_dimension.h"
#include "../include/us_hydrodyn_asab1.h"
#include <qmessagebox.h>
#include "../include/us_plot_util.h"
#include "../include/us_unicode.h"
#include "../include/us_average.h"
#include "../include/us_hydrodyn_fractal_dimension_options.h"

#define UHFD_PLOT_EPSILON 2e-3

static bool linear_fit(
                       const double                                   & xmin
                       ,const double                                  & xmax
                       ,const vector < vector < vector < double > > >   x
                       ,const vector < vector < vector < double > > >   y
                       ,mQwtPlot                                      * plot
                       ,vector < vector < QwtPlotCurve * > >          & fitcurves
                       ,QTextEdit                                     * msgbox
                       ,double                                        & fd
                       ,double                                        & fd_sd
                       ,double                                        & fd_wtd
                       ,double                                        & fd_wtd_sd
                       ,double                                        & fd_wtd_wtd
                       ,double                                        & fd_wtd_wtd_sd
                       ) {

   fd            = -1;
   fd_sd         = -1;
   fd_wtd        = -1;
   fd_wtd_sd     = -1;
   fd_wtd_wtd    = -1;
   fd_wtd_wtd_sd = -1;
   
   qDebug() << QString( "linear_fit(); xmin is %1, xmax is %2" ).arg( xmin ).arg( xmax );

   if ( !x.size() ) {
      msgbox->setText( us_tr( "Internal error: linear_fit, no curves!" ) );
      return false;
   }      

   if ( x.size() != fitcurves.size() ) {
      msgbox->setText( us_tr( "Internal error: fitcurves x size mismatch" ) );
      return false;
   }

   double global_a_sum    = 0;
   double global_b_sum    = 0;
   double global_siga_sum = 0;
   double global_sigb_sum = 0;
   int    global_count    = 0;

   vector < double > global_no_weights_a_v;
   vector < double > global_no_weights_siga_v;
   vector < double > global_no_weights_b_v;
   vector < double > global_no_weights_sigb_v;
   vector < double > global_weighted_b_v;
   vector < double > global_weighted_sigb_v;

   msgbox->setText( "" );

   for ( size_t g = 0; g < x.size(); ++g ) {
      if ( x[ g ].size() != fitcurves[ g ].size() ) {
         msgbox->setText( us_tr( "Internal error: fitcurves x size mismatch" ) );
         return false;
      }
      double a_sum    = 0;
      double b_sum    = 0;
      double siga_sum = 0;
      double sigb_sum = 0;

      vector < double > a_v;
      vector < double > siga_v;
      vector < double > b_v;
      vector < double > sigb_v;

      for ( size_t i = 0; i < x[ g ].size(); ++i ) {

         vector < double > use_x;
         vector < double > use_y;

         for ( size_t j = 0; j < x[ g ][ i ].size(); ++j ) {
            if ( x[ g ][ i ][ j ] >= xmin - UHFD_PLOT_EPSILON && x[ g ][ i ][ j ] <= xmax + UHFD_PLOT_EPSILON ) {
               use_x.push_back( x[ g ][ i ][ j ] );
               use_y.push_back( y[ g ][ i ][ j ] );
            }
         }

         if ( !use_x.size() ) {
            msgbox->setText( us_tr( "Empty fit range" ) );
            vector < double > fitx;
            vector < double > fity;
            if ( fitcurves[ g ][ i ] ) {
               fitcurves[ g ][ i ]->setSamples(
                                               (double *)&( fitx[ 0 ] )
                                               ,(double *)&( fity[ 0 ] )
                                               ,0
                                               );
               plot->replot();
            }
            return false;
         }

         double a;
         double b;
         double siga;
         double sigb;
         double chi2;
   
         US_Saxs_Util::linear_fit( use_x, use_y, a, b, siga, sigb, chi2 );

         a_sum    += a;
         b_sum    += b;
         siga_sum += siga;
         sigb_sum += sigb;
      
         global_a_sum    += a;
         global_b_sum    += b;
         global_siga_sum += siga;
         global_sigb_sum += sigb;
         ++global_count;

         a_v.push_back( a );
         siga_v.push_back( siga );
         b_v.push_back( b );
         sigb_v.push_back( sigb );

         // qDebug() << QString( "a is %1, b is %2" ).arg( a ).arg( b );

         // plot line

         if ( fitcurves[ g ][ i ] ) {
            vector < double > fitx = { xmin, xmax };
            vector < double > fity = { a + b * xmin, a + b * xmax };
            fitcurves[ g ][ i ]->setSamples(
                                            (double *)&( fitx[ 0 ] )
                                            ,(double *)&( fity[ 0 ] )
                                            ,2
                                            );
         }
      }
      {
         double a_avg_no_weights;
         double a_sd_no_weights;
         double b_avg_no_weights;
         double b_sd_no_weights;
         double b_avg_weighted;
         double b_sd_weighted;
         QString errorsfull;
         QString errors;

         if ( !US_Average::compute( US_Average::NO_WEIGHTS, a_v, siga_v, a_avg_no_weights, a_sd_no_weights, errors ) ) {
            errorsfull += errors;
         }
         if ( !US_Average::compute( US_Average::NO_WEIGHTS, b_v, sigb_v, b_avg_no_weights, b_sd_no_weights, errors ) ) {
            errorsfull += errors;
         }
         if ( !US_Average::compute( US_Average::ONE_OVER_SD_SQ, b_v, sigb_v, b_avg_weighted, b_sd_weighted, errors ) ) {
            errorsfull += errors;
         }

         global_no_weights_a_v.push_back( a_avg_no_weights );
         global_no_weights_siga_v.push_back( a_sd_no_weights );
         global_no_weights_b_v.push_back( b_avg_no_weights );
         global_no_weights_sigb_v.push_back( b_sd_no_weights );
         global_weighted_b_v.push_back( b_avg_weighted );
         global_weighted_sigb_v.push_back( b_sd_weighted );

         fd        = b_avg_no_weights;
         fd_sd     = b_sd_no_weights;
         fd_wtd    = b_avg_weighted;
         fd_wtd_sd = b_sd_weighted;

         msgbox->setText(
                         msgbox->toPlainText()
                         + QString( "%1" ).arg(
                                               x.size() > 1
                                               ? QString( "Group %1 (size %2): " ).arg( g + 1 ).arg( x[ g ].size() )
                                               : QString( "" )
                                               )
                         + QString( "a = %1 (%2%3)  b = %4 (%5%6), wtd. b = %7 (%8%9) %10\n" )
                         .arg( a_avg_no_weights, 0, 'f', 3 )
                         .arg( UNICODE_PLUSMINUS )
                         .arg( a_sd_no_weights, 0, 'f', 3 )
                         .arg( b_avg_no_weights, 0, 'f', 3 )
                         .arg( UNICODE_PLUSMINUS )
                         .arg( b_sd_no_weights, 0, 'f', 3 )
                         .arg( b_avg_weighted, 0, 'f', 3 )
                         .arg( UNICODE_PLUSMINUS )
                         .arg( b_sd_weighted, 0, 'f', 3 )
                         .arg( errors )
                         );

      }
   }

   // compute global averages if needed

   if ( global_no_weights_a_v.size() > 1
        && global_count ) {
      double a_avg_no_weights;
      double a_sd_no_weights;
      double b_avg_no_weights;
      double b_sd_no_weights;
      double b_avg_weighted;
      double b_sd_weighted;
      double b_avg_no_weights_from_weighted;
      double b_sd_no_weights_from_weighted;
      QString errorsfull;
      QString errors;

      if ( !US_Average::compute( US_Average::NO_WEIGHTS, global_no_weights_a_v, global_no_weights_siga_v, a_avg_no_weights, a_sd_no_weights, errors ) ) {
         errorsfull += errors;
      }
      if ( !US_Average::compute( US_Average::NO_WEIGHTS, global_no_weights_b_v, global_no_weights_sigb_v, b_avg_no_weights, b_sd_no_weights, errors ) ) {
         errorsfull += errors;
      }
      if ( !US_Average::compute( US_Average::NO_WEIGHTS, global_weighted_b_v, global_weighted_sigb_v, b_avg_no_weights_from_weighted, b_sd_no_weights_from_weighted, errors ) ) {
         errorsfull += errors;
      }
      if ( !US_Average::compute( US_Average::ONE_OVER_SD_SQ, global_weighted_b_v, global_weighted_sigb_v, b_avg_weighted, b_sd_weighted, errors ) ) {
         errorsfull += errors;
      }

      fd            = b_avg_no_weights;
      fd_sd         = b_sd_no_weights;
      fd_wtd        = b_avg_no_weights_from_weighted;
      fd_wtd_sd     = b_sd_no_weights_from_weighted;
      fd_wtd_wtd    = b_avg_weighted;
      fd_wtd_wtd_sd = b_sd_weighted;

      msgbox->setText(
                      msgbox->toPlainText()
                      + QString( "%1" ).arg(
                                            x.size() > 1
                                            ? "Global average : "
                                            : ""
                                            )
                      + QString( "a = %1 (%2%3)  b = %4 (%5%6), avg of wtd. b = %7 (%8%9), wtd. of wtd. b = %10 (%11%12) %13\n" )
                      .arg( a_avg_no_weights, 0, 'f', 3 )
                      .arg( UNICODE_PLUSMINUS )
                      .arg( a_sd_no_weights, 0, 'f', 3 )
                      .arg( b_avg_no_weights, 0, 'f', 3 )
                      .arg( UNICODE_PLUSMINUS )
                      .arg( b_sd_no_weights, 0, 'f', 3 )
                      .arg( b_avg_no_weights_from_weighted, 0, 'f', 3 )
                      .arg( UNICODE_PLUSMINUS )
                      .arg( b_sd_no_weights_from_weighted, 0, 'f', 3 )
                      .arg( b_avg_weighted, 0, 'f', 3 )
                      .arg( UNICODE_PLUSMINUS )
                      .arg( b_sd_weighted, 0, 'f', 3 )
                      .arg( errors )
                      );
      
   }

   plot->replot();

   return true;
}

void US_Hydrodyn::fractal_dimension( bool from_parameters, save_info * fd_save_info ) {
   qDebug() << "US_Hydrodyn::fractal_dimension()";
   if ( !batch_active() ) {
      stopFlag = false;
   }
   bool quiet = false;

   if ( from_parameters
        && gparam_value( US_Hydrodyn_Fractal_Dimension_Options::paramname( US_Hydrodyn_Fractal_Dimension_Options::ENABLED ) ) != "true" ) {
      qDebug() << "US_Hydrodyn::fractal_dimension() - not enabled";
      return;
   }

   // for each selected model

   set < int > selected_models;

   // pdb loaded check

   for( int i = 0; i < lb_model->count(); i++ ) {
      if ( lb_model->item( i )->isSelected() ) {
         selected_models.insert( i );
      }
   }
   
   if ( !selected_models.size() ) {
      if ( !quiet ) {
         QMessageBox::critical( this,
                                windowTitle() + us_tr( ": Fractal Dimension" ),
                                us_tr( "No models selected" )
                                );
      }
      return;
   }
      
   double sas_asa_threshold             = asa.threshold;
   double sas_asa_probe_radius          = 1.5;
   double angstrom_start                = 0;
   double angstrom_end                  = 0;
   double angstrom_steps                = 0;
   double enright_ca_pct_start          = 0;
   double enright_ca_pct_end            = 0;
   double roll_sphere_start             = 0;
   double roll_sphere_end               = 0;
   double roll_sphere_steps             = 0;
   US_Fractal_Dimension::methods method = US_Fractal_Dimension::USFD_BOX_MODEL;
   bool show_plots                      = true;
   bool save_plot_data                  = false; // chosen in the show plots window
   bool show_bead_models                = false;

   double fd                            = -1;
   double fd_sd                         = -1;
   double fd_wtd                        = -1;
   double fd_wtd_sd                     = -1;
   double fd_wtd_wtd                    = -1;
   double fd_wtd_wtd_sd                 = -1;

   // pat 1st model for extents
   point extents;
   if ( !from_parameters ) {
      vector < PDB_atom > model;
      current_model = *(selected_models.begin());
      for ( size_t j = 0; j < model_vector[current_model].molecule.size(); ++j ) {
         for ( size_t k = 0; k < model_vector[current_model].molecule[j].atom.size(); ++k ) {
            PDB_atom this_atom = model_vector[current_model].molecule[j].atom[k];

            if ( !this_atom.p_residue ) {
               editor_msg( "red", QString( us_tr( "Internal error: p_residue not set!, contact the developers" ) ) );
               return;
            }
            if ( !this_atom.p_atom ) {
               editor_msg( "red", QString( us_tr( "Internal error: p_atom not set!, contact the developers" ) ) );
               return;
            }

            QString use_res_name  = this_atom.name != "OXT" ? this_atom.p_residue->name : "OXT";
            QString use_atom_name = this_atom.p_residue->name == "N1" ? "N1" : this_atom.name;

            if ( !k &&
                 this_atom.name == "N" ) {
               if ( use_res_name == "PRO" ) {
                  use_res_name = "N1-";
               } else {
                  use_res_name = "N1";
               }
               use_atom_name = use_res_name;
            }
              
            QString res_idx =
               QString("%1|%2")
               .arg( use_res_name )
               .arg( use_atom_name )
               ;

            if ( vdwf.count( res_idx ) ) {
               // QTextStream( stdout ) << "found ionized_mw_delta " << vdwf[ res_idx ].ionized_mw_delta << endl;
               PDB_atom tmp_atom;
               _vdwf this_vdwf            = vdwf[ res_idx ];
               // overwrite from p_atom
               this_vdwf.mw               = this_atom.p_atom->hybrid.mw;
               this_vdwf.ionized_mw_delta = this_atom.p_atom->hybrid.ionized_mw_delta;
               this_vdwf.r                = this_atom.p_atom->hybrid.radius;
               this_vdwf.w                = this_atom.p_atom->hydration;
               this_vdwf.e                = this_atom.p_atom->hybrid.num_elect;

               if ( this_atom.resName == "WAT" ) {
                  this_vdwf.mw = 0.0;
               }
               tmp_atom.bead_coordinate = this_atom.coordinate;
               tmp_atom.bead_computed_radius = this_vdwf.r;

               tmp_atom.radius                    = tmp_atom.bead_computed_radius;
               tmp_atom.bead_ref_mw               = this_vdwf.mw;
               tmp_atom.bead_ref_ionized_mw_delta = this_vdwf.ionized_mw_delta;
               tmp_atom.mw                        = this_vdwf.mw;
               tmp_atom.ionized_mw_delta          = this_vdwf.ionized_mw_delta;
               tmp_atom.bead_mw                   = this_vdwf.mw;
               tmp_atom.bead_ionized_mw_delta     = this_vdwf.ionized_mw_delta;
               tmp_atom.bead_color                = this_vdwf.color;
               // #warning recolored for hydration
               // tmp_atom.bead_color = hydrate && this_vdwf.w ? 1 : 6;
               // #warning recolored for ASA testing
               // tmp_atom.bead_color                = this_atom.asa >= asa.hydrate_threshold ? 6 : 8;
               tmp_atom.bead_recheck_asa          = this_atom.asa;
               tmp_atom.num_elect                 = this_vdwf.e;
               tmp_atom.exposed_code              = 1;
               tmp_atom.all_beads                 .clear( );
               tmp_atom.active                    = true;
               tmp_atom.name                      = this_atom.name;
               tmp_atom.resName                   = this_atom.resName;
               tmp_atom.iCode                     = this_atom.iCode;
               tmp_atom.chainID                   = this_atom.chainID;
               if ( this_atom.asa >= asa.hydrate_threshold && this_vdwf.w ) {
                  tmp_atom.bead_hydration            = this_vdwf.w;
               } else {
                  tmp_atom.bead_hydration            = 0;
               }
               tmp_atom.saxs_data.saxs_name       = "";
               tmp_atom.bead_model_code = QString( "%1.%2.%3.%4.%5" )
                  .arg( this_atom.serial )
                  .arg( this_atom.name )
                  .arg( this_atom.resName )
                  .arg( this_atom.chainID )
                  .arg( this_atom.resSeq )
                  ;
               model.push_back( tmp_atom );
            } else {
               editor_msg( "red", QString( "Residue atom pair %1 unknown in vdwf.json" ).arg( res_idx ) );
               return;
            }
         }
      }
      if ( !model.size() ) {
         QMessageBox::critical( this,
                                windowTitle() + us_tr( ": Fractal Dimension - PAT" ),
                                us_tr( "Internal error - PAT computation failed - empty model" )
                                );
                                                     
         return;
      }         

      US_Saxs_Util usu;

      usu.bead_models.resize( 1 );
      usu.bead_models[ 0 ] = model;
      if ( "empty model" != usu.nsa_physical_stats() ) {
         // double fconv = pow(10.0, this_data.hydro.unit + 9);
         double fconv = 1; 
                  
         extents.axis[ 0 ] = usu.nsa_physical_stats_map[ "result radial extent bounding box size x" ].toDouble() * fconv;
         extents.axis[ 1 ] = usu.nsa_physical_stats_map[ "result radial extent bounding box size y" ].toDouble() * fconv;
         extents.axis[ 2 ] = usu.nsa_physical_stats_map[ "result radial extent bounding box size z" ].toDouble() * fconv;

      } else {
         QMessageBox::critical( this,
                                windowTitle() + us_tr( ": Fractal Dimension - PAT" ),
                                us_tr( "Internal error - PAT computation failed - empty model(?)" )
                                );
                                                     
         return;
      }         
      
      // if ( !pat_model( model ) ) {
      //    QMessageBox::critical( this,
      //                           windowTitle() + us_tr( ": Fractal Dimension - PAT" ),
      //                           us_tr( "Internal error - PAT computation failed" )
      //                           );
                                                     
      //    return;
      // }
      
      // point pmin = model[0].bead_coordinate;
      // point pmax = model[0].bead_coordinate;
      
      // for ( auto const & p : model ) {
      //    for ( size_t i = 0; i < 3; ++i ) {
      //       if ( pmin.axis[ i ] > p.bead_coordinate.axis[ i ] ) {
      //          pmin.axis[ i ] = p.bead_coordinate.axis[ i ];
      //       }
      //       if ( pmax.axis[ i ] < p.bead_coordinate.axis[ i ] ) {
      //          pmax.axis[ i ] = p.bead_coordinate.axis[ i ];
      //       }
      //    }
      // }
      
      // extents = pmax;
      // for ( size_t i = 0; i < 3; ++i ) {
      //    extents.axis[ i ] -= pmin.axis[ i ];
      // }
   }

   // box model parameters dialog
   if ( !from_parameters ) {
      bool try_again = false;

      do {

         QDialog dialog(this);
         dialog.setWindowTitle( windowTitle() + us_tr( ": Fractal Dimension : Set parameters" ) );
         // Use a layout allowing a label next to each field
         dialog.setMinimumWidth( 200 );

         QFormLayout form(&dialog);

         // Add some text above the fields
         form.addRow( new QLabel(
                                 QString(
                                         us_tr(
                                               "Set the box counting parameters<br>"
                                               "Fill out the values below and click OK"
                                               "<hr>"
                                               "Maximum extents (model %1, [%2]) x,y,z : [%3,%4,%5]"
                                               "<hr>"
                                               )
                                         )
                                 .arg( *(selected_models.begin()) + 1 )
                                 .arg( UNICODE_ANGSTROM )
                                 .arg( extents.axis[ 0 ], 0, 'f', 2 )
                                 .arg( extents.axis[ 1 ], 0, 'f', 2 )
                                 .arg( extents.axis[ 2 ], 0, 'f', 2 )
                                 )
                      );

         // Add the lineEdits with their respective labels
         QList<QWidget *> fields;
   
         vector < QString > labels =
            {
               QString( us_tr( "ASA threshold [%1^2]" ) ).arg( UNICODE_ANGSTROM )
               ,QString( us_tr( "ASA probe radius [%1]" ) ).arg( UNICODE_ANGSTROM )
               ,QString( us_tr( "Start [%1]" ) ).arg( UNICODE_ANGSTROM )
               ,QString( us_tr( "End [%1]" ) ).arg( UNICODE_ANGSTROM )
               ,us_tr( "Number of steps" )
               ,QString( us_tr( "Start C%1 dist. % (Enright only)" ) ).arg( UNICODE_ALPHA )
               ,QString( us_tr( "End C%1 dist. % (Enright only)" ) ).arg( UNICODE_ALPHA )
               ,QString( us_tr( "Probe radius start [%1] (Rolling sphere only)" ) ).arg( UNICODE_ANGSTROM )
               ,QString( us_tr( "Probe radius end [%1] (Rolling sphere only)" ) ).arg( UNICODE_ANGSTROM )
               ,us_tr( "Probe radius steps (Rolling sphere only)" )
            };
      
         vector < QWidget * > widgets =
            {
               new QLineEdit( &dialog )
               ,new QLineEdit( &dialog )
               ,new QLineEdit( &dialog )
               ,new QLineEdit( &dialog )
               ,new QLineEdit( &dialog )
               ,new QLineEdit( &dialog )
               ,new QLineEdit( &dialog )
               ,new QLineEdit( &dialog )
               ,new QLineEdit( &dialog )
               ,new QLineEdit( &dialog )
            };

         vector < double >  defaults =
            {
               0
               ,sas_asa_probe_radius
               ,5
               ,20
               ,20
               ,0
               ,10
               ,1
               ,2
               ,5
            };

         vector < bool >  intvalidator =
            {
               false
               ,false
               ,false
               ,false
               ,true
               ,false
               ,false
               ,false
               ,false
               ,true
            };

         for( int i = 0; i < (int) widgets.size(); ++i ) {
            form.addRow( labels[i], widgets[i] );
            if ( widgets[i] ) {
               // could switch based on widgets[i]->className()
               if ( intvalidator[ i ] ) {
                  ((QLineEdit *)widgets[i])->setValidator( new QIntValidator(this) );
               } else {
                  ((QLineEdit *)widgets[i])->setValidator( new QDoubleValidator(this) );
               }                  
               ((QLineEdit *)widgets[i])->setText( QString( "%1" ).arg( defaults[i] ) );
               fields << widgets[i];
            }
         }

         // add out-of-band a qcombobox
         // could integrate to the above generalization
         
         QComboBox * cmb_method = new QComboBox( this );
         cmb_method->setPalette( PALET_NORMAL );
         AUTFBACK( cmb_method );
         cmb_method->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
         cmb_method->setEnabled(true);
         cmb_method->setMaxVisibleItems( 1 );

         cmb_method->addItem( US_Fractal_Dimension::method_name( US_Fractal_Dimension::USFD_BOX_MODEL    ), US_Fractal_Dimension::USFD_BOX_MODEL );
         cmb_method->addItem( US_Fractal_Dimension::method_name( US_Fractal_Dimension::USFD_BOX_ALT      ), US_Fractal_Dimension::USFD_BOX_ALT );
         cmb_method->addItem( US_Fractal_Dimension::method_name( US_Fractal_Dimension::USFD_BOX_MASS     ), US_Fractal_Dimension::USFD_BOX_MASS );
         cmb_method->addItem( US_Fractal_Dimension::method_name( US_Fractal_Dimension::USFD_ENRIGHT      ), US_Fractal_Dimension::USFD_ENRIGHT );
         cmb_method->addItem( US_Fractal_Dimension::method_name( US_Fractal_Dimension::USFD_ENRIGHT_FULL ), US_Fractal_Dimension::USFD_ENRIGHT_FULL );
         cmb_method->addItem( US_Fractal_Dimension::method_name( US_Fractal_Dimension::USFD_ROLL_SPHERE  ), US_Fractal_Dimension::USFD_ROLL_SPHERE );
         
         form.addRow( us_tr( "Method" ), cmb_method );
         
         // Add some standard buttons (Cancel/Ok) at the bottom of the dialog
         QDialogButtonBox buttonBox(
                                    QDialogButtonBox::Ok | QDialogButtonBox::Cancel
                                    ,Qt::Horizontal
                                    ,&dialog
                                    );
         form.addRow(&buttonBox);
         QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
         QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

         // Show the dialog as modal
         if (dialog.exec() == QDialog::Accepted) {
            if ( ((QLineEdit *)fields[0])->text().toDouble() < 0
                 || ((QLineEdit *)fields[1])->text().toDouble() <= 0
                 || ((QLineEdit *)fields[2])->text().toDouble() <= 0
                 || ((QLineEdit *)fields[3])->text().toDouble() <= 0
                 || ((QLineEdit *)fields[2])->text().toDouble() >= ((QLineEdit *)fields[3])->text().toDouble()
                 || ((QLineEdit *)fields[4])->text().toDouble() <= 1

                 // enright
                 || ((QLineEdit *)fields[5])->text().toDouble() < 0
                 || ((QLineEdit *)fields[6])->text().toDouble() < 0
                 || ((QLineEdit *)fields[6])->text().toDouble() > 100
                 || ((QLineEdit *)fields[5])->text().toDouble() >= ((QLineEdit *)fields[6])->text().toDouble()

                 // roll sphere
                 || ((QLineEdit *)fields[7])->text().toDouble() <= 0
                 || ((QLineEdit *)fields[8])->text().toDouble() <= 0
                 || ((QLineEdit *)fields[7])->text().toDouble() >= ((QLineEdit *)fields[8])->text().toDouble()
                 || ((QLineEdit *)fields[9])->text().toDouble() < 1
                 ) {
               try_again = true;
            } else {
               // If the user didn't dismiss the dialog, do something with the fields
               sas_asa_threshold    = ((QLineEdit *)fields[0])->text().toDouble();
               sas_asa_probe_radius = ((QLineEdit *)fields[1])->text().toDouble();
               angstrom_start       = ((QLineEdit *)fields[2])->text().toDouble();
               angstrom_end         = ((QLineEdit *)fields[3])->text().toDouble();
               angstrom_steps       = ((QLineEdit *)fields[4])->text().toDouble();
               enright_ca_pct_start = ((QLineEdit *)fields[5])->text().toDouble();
               enright_ca_pct_end   = ((QLineEdit *)fields[6])->text().toDouble();
               roll_sphere_start    = ((QLineEdit *)fields[7])->text().toDouble();
               roll_sphere_end      = ((QLineEdit *)fields[8])->text().toDouble();
               roll_sphere_steps    = ((QLineEdit *)fields[9])->text().toDouble();
               method               = (US_Fractal_Dimension::methods)cmb_method->currentData().toInt();
               qDebug() << QString( "selected method %1" ).arg( (int) method );
            }
         } else {
            return;
         }
      } while ( try_again );
   } else {
      // from_parameters

      sas_asa_threshold =
         gparam_value( US_Hydrodyn_Fractal_Dimension_Options::paramname( US_Hydrodyn_Fractal_Dimension_Options::ASA_THRESHOLD ) ).toDouble();
      sas_asa_probe_radius =
         gparam_value( US_Hydrodyn_Fractal_Dimension_Options::paramname( US_Hydrodyn_Fractal_Dimension_Options::ASA_PROBE_RADIUS ) ).toDouble();
      angstrom_start =
         gparam_value( US_Hydrodyn_Fractal_Dimension_Options::paramname( US_Hydrodyn_Fractal_Dimension_Options::ANGSTROM_START ) ).toDouble();
      angstrom_end =
         gparam_value( US_Hydrodyn_Fractal_Dimension_Options::paramname( US_Hydrodyn_Fractal_Dimension_Options::ANGSTROM_END ) ).toDouble();
      angstrom_steps =
         gparam_value( US_Hydrodyn_Fractal_Dimension_Options::paramname( US_Hydrodyn_Fractal_Dimension_Options::ANGSTROM_END ) ).toInt();
      enright_ca_pct_start =
         gparam_value( US_Hydrodyn_Fractal_Dimension_Options::paramname( US_Hydrodyn_Fractal_Dimension_Options::ENRIGHT_CA_PCT_START ) ).toDouble();
      enright_ca_pct_end =
         gparam_value( US_Hydrodyn_Fractal_Dimension_Options::paramname( US_Hydrodyn_Fractal_Dimension_Options::ENRIGHT_CA_PCT_END ) ).toDouble();
      roll_sphere_start =
         gparam_value( US_Hydrodyn_Fractal_Dimension_Options::paramname( US_Hydrodyn_Fractal_Dimension_Options::ROLL_SPHERE_START ) ).toDouble();
      roll_sphere_end =
         gparam_value( US_Hydrodyn_Fractal_Dimension_Options::paramname( US_Hydrodyn_Fractal_Dimension_Options::ROLL_SPHERE_END ) ).toDouble();
      roll_sphere_steps =
         gparam_value( US_Hydrodyn_Fractal_Dimension_Options::paramname( US_Hydrodyn_Fractal_Dimension_Options::ROLL_SPHERE_STEPS ) ).toInt();
      method = (US_Fractal_Dimension::methods)
         gparam_value( US_Hydrodyn_Fractal_Dimension_Options::paramname( US_Hydrodyn_Fractal_Dimension_Options::METHOD ) ).toInt();
      show_plots =
         gparam_value( US_Hydrodyn_Fractal_Dimension_Options::paramname( US_Hydrodyn_Fractal_Dimension_Options::PLOTS ) ) == "true";
      save_plot_data =
         gparam_value( US_Hydrodyn_Fractal_Dimension_Options::paramname( US_Hydrodyn_Fractal_Dimension_Options::SAVE_PLOT_DATA ) ) == "true";

      // could add it if we need to
      // show_bead_models =
      //    gparam_value( US_Hydrodyn_Fractal_Dimension_Options::paramname( US_Hydrodyn_Fractal_Dimension_Options::SHOW_BEAD_MODELS ) ) == "true";
   }
      
   QTextStream( stdout ) << QString(
                                    "US_Hydrodyn::fractal_dimension()\n"
                                    "sas_asa_threshold    %1\n"
                                    "sas_asa_probe_radius %2\n"
                                    "angstrom_start       %3\n"
                                    "angstrom_end         %4\n"
                                    "angstrom_steps       %5\n"
                                    "enright_ca_pct_start %6\n"
                                    "enright_ca_pct_end   %7\n"
                                    "roll_sphere_start    %8\n"
                                    "roll_sphere_end      %9\n"
                                    "roll_sphere_steps    %10\n"
                                    )
      .arg( sas_asa_threshold )
      .arg( sas_asa_probe_radius )
      .arg( angstrom_start )
      .arg( angstrom_end )
      .arg( angstrom_steps )
      .arg( enright_ca_pct_start )
      .arg( enright_ca_pct_end )
      .arg( roll_sphere_start )
      .arg( roll_sphere_end )
      .arg( roll_sphere_steps )
      ;

   if ( !from_parameters ) {
      show_bead_models =
         method == US_Fractal_Dimension::USFD_ROLL_SPHERE
         ? false
         : QMessageBox::Yes == QMessageBox::question(
                                                     this,
                                                     windowTitle() + us_tr( ": Fractal Dimension" ),
                                                     us_tr("Display bead models of SAS ?" ),
                                                     QMessageBox::Yes, 
                                                     QMessageBox::No | QMessageBox::Default
                                                     );
   }
   
   for ( auto const current_model : selected_models ) {
      vector < vector < vector < double > > > x;
      vector < vector < vector < double > > > y;
      QString                                 type;
      QString                                 x_title;
      QString                                 y_title;
      vector < pointmass >                    sas;

      if ( from_parameters ) {
         editor_msg( "darkblue",
                     QString( us_tr( "Processing model %1 : %2\n" ) )
                     .arg( current_model + 1 )
                     .arg( US_Fractal_Dimension::method_name( method ) )
                     );
      } else {
         editor_msg( "darkblue", QString( us_tr( "Fractal dimension processing model %1\n" ) ).arg( current_model + 1 ) );
      }

      if ( show_bead_models ) {
         bead_model.clear( );
      }

      if ( method == US_Fractal_Dimension::USFD_ROLL_SPHERE ) {

         x.resize( 1 );
         y.resize( 1 );
         x[ 0 ].resize( 1 );
         y[ 0 ].resize( 1 );
         
         double stepsize = ( roll_sphere_end - roll_sphere_start ) / roll_sphere_steps;
         if ( stepsize <= 0 ) {
            QMessageBox::critical( this,
                                   windowTitle() + us_tr( ": Fractal Dimension - rolling sphere" ),
                                   QString( us_tr( "Invalid computed stepsize  (%1)" ) ).arg( stepsize )
                                   );
                                                     
            return;
         }
         
         for ( double radius = roll_sphere_start; radius <= roll_sphere_end; radius += stepsize ) {
            vector < PDB_atom * > active_atoms;
            for (unsigned int j = 0; j < model_vector[current_model].molecule.size(); ++j) {
               for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size(); ++k) {
                  PDB_atom *this_atom = &(model_vector[current_model].molecule[j].atom[k]);
                  active_atoms.push_back( this_atom );
               }
            }
            editor_msg( "darkblue", QString( us_tr( "Computing ASA for probe radius %1 [%2]" ) ).arg( radius, 0, 'f', 3 ).arg( UNICODE_ANGSTROM ) );
            progress->reset();
            qApp->processEvents();
            int retval;
            {
               float save_radius = asa.probe_radius;
               asa.probe_radius  = radius;

               retval = us_hydrodyn_asab1_main(
                                               active_atoms,
                                               &asa,
                                               &results,
                                               false,
                                               progress,
                                               editor,
                                               this
                                               );
               asa.probe_radius  = save_radius;
            }         

            if ( retval ) {
               editor->append("Errors found during ASA calculation\n");
               progress->reset();
               qApp->processEvents();
               if (stopFlag) {
                  return;
               }
               switch ( retval )
               {
               case US_HYDRODYN_ASAB1_ERR_MEMORY_ALLOC:
                  {
                     QMessageBox::critical( this,
                                            windowTitle() + us_tr( ": Fractal Dimension" ),
                                            us_tr( "US_HYDRODYN_ASAB1 encountered a memory allocation error") );
                     return;
                     break;
                  }
               default:
                  {
                     QMessageBox::critical( this,
                                            windowTitle() + us_tr( ": Fractal Dimension" ),
                                            us_tr( "US_HYDRODYN_ASAB1 encountered an unknown error") );
                     return;
                     break;
                  }
               }
            }

            double asa_total = 0;

            for ( auto const & atom : active_atoms ) {
               asa_total += atom->asa;
            }

            if ( asa_total <= 0 ) {
               QMessageBox::critical( this,
                                      windowTitle() + us_tr( ": Fractal Dimension - rolling sphere" ),
                                      QString( us_tr( "ASA total %1 for radius %2 [%3] invalid" ) )
                                      .arg( asa_total )
                                      .arg( radius )
                                      .arg( UNICODE_ANGSTROM )
                                      );
               return;
            }

            x[ 0 ][ 0 ].push_back( log10( radius ) );
            y[ 0 ][ 0 ].push_back( log10( asa_total ) );
         }
         x_title = "log10( Probe radius [" + UNICODE_ANGSTROM_QS + "] )";
         y_title = "log10( ASA [" + UNICODE_ANGSTROM_QS + "^2] )";
         type    = US_Fractal_Dimension::method_name( method );
         // US_Vector::printvector2( "rolling sphere, log ASA, log probe radius", x[0], y[0] );
         // return;
      } else {      
         // compute ASA

         {
            vector < PDB_atom * > active_atoms;
            for (unsigned int j = 0; j < model_vector[current_model].molecule.size(); ++j) {
               for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size(); ++k) {
                  PDB_atom *this_atom = &(model_vector[current_model].molecule[j].atom[k]);
                  active_atoms.push_back( this_atom );
               }
            }
            progress->reset();
            qApp->processEvents();
            int retval;
            {
               float save_radius = asa.probe_radius;
               asa.probe_radius  = sas_asa_probe_radius; 

               retval = us_hydrodyn_asab1_main(
                                               active_atoms,
                                               &asa,
                                               &results,
                                               false,
                                               progress,
                                               editor,
                                               this
                                               );
               asa.probe_radius  = save_radius;
            }         

            if ( retval ) {
               editor->append("Errors found during ASA calculation\n");
               progress->reset();
               qApp->processEvents();
               if (stopFlag) {
                  return;
               }
               switch ( retval )
               {
               case US_HYDRODYN_ASAB1_ERR_MEMORY_ALLOC:
                  {
                     QMessageBox::critical( this,
                                            windowTitle() + us_tr( ": Fractal Dimension" ),
                                            us_tr( "US_HYDRODYN_ASAB1 encountered a memory allocation error") );
                     return;
                     break;
                  }
               default:
                  {
                     QMessageBox::critical( this,
                                            windowTitle() + us_tr( ": Fractal Dimension" ),
                                            us_tr( "US_HYDRODYN_ASAB1 encountered an unknown error") );
                     return;
                     break;
                  }
               }
            }
         }
      
         // find SAS points, build set

         size_t total_atoms = 0;
         double asa_total   = 0;

         for ( unsigned int j = 0; j < model_vector[current_model].molecule.size(); ++j) {
            for ( unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size(); ++k ) {
               PDB_atom *this_atom = &(model_vector[current_model].molecule[j].atom[k]);

               ++total_atoms;
            
               if ( !this_atom->p_residue ) {
                  editor_msg( "red", QString( us_tr( "Internal error: p_residue not set!, contact the developers" ) ) );
                  return;
               }
               if ( !this_atom->p_atom ) {
                  editor_msg( "red", QString( us_tr( "Internal error: p_atom not set!, contact the developers" ) ) );
                  return;
               }

               asa_total += this_atom->asa;

               if ( this_atom->asa >= sas_asa_threshold ) {
                  {
                     QString use_res_name  = this_atom->name != "OXT" ? this_atom->p_residue->name : "OXT";
                     QString use_atom_name = this_atom->p_residue->name == "N1" ? "N1" : this_atom->name;

                     if ( !k &&
                          this_atom->name == "N" ) {
                        if ( use_res_name == "PRO" ) {
                           use_res_name = "N1-";
                        } else {
                           use_res_name = "N1";
                        }
                        use_atom_name = use_res_name;
                     }
              
                     QString res_idx =
                        QString("%1|%2")
                        .arg( use_res_name )
                        .arg( use_atom_name )
                        ;

                     double saxs_excl_vol;
                     {
                        QString mapkey = QString("%1|%2").arg(this_atom->resName).arg(this_atom->name);
                        if ( this_atom->name == "OXT" ) {
                           mapkey = "OXT|OXT";
                        }
                        if ( !residue_atom_hybrid_map.count( mapkey ) ) {
                           editor_msg( "red", QString( us_tr( "Error: Missing hybrid name for key %1" ) ).arg( mapkey ) );
                           return;
                        }
                        QString hybrid_name = residue_atom_hybrid_map[mapkey];
                        QString this_atom_name = hybrid_name == "ABB" ? "ABB" : this_atom->name;
                        if ( !atom_map.count( this_atom_name + "~" + hybrid_name ) ) {
                           editor_msg( "red", QString( us_tr( "Error: Missing hybrid name for key %1" ) ).arg( mapkey ) );
                           return;
                        }
                        saxs_excl_vol = atom_map[this_atom_name + "~" + hybrid_name].saxs_excl_vol;
                     }

                     // QTextStream( stdout ) << "vdwf: res_idx is " << res_idx << " " << endl;
         
                     if ( vdwf.count( res_idx ) ) {
                        // QTextStream( stdout ) << "found ionized_mw_delta " << vdwf[ res_idx ].ionized_mw_delta << endl;
                        PDB_atom tmp_atom;
                        _vdwf this_vdwf            = vdwf[ res_idx ];
                        // overwrite from p_atom
                        this_vdwf.mw               = this_atom->p_atom->hybrid.mw;
                        this_vdwf.ionized_mw_delta = this_atom->p_atom->hybrid.ionized_mw_delta;
                        this_vdwf.r                = this_atom->p_atom->hybrid.radius;
                        this_vdwf.w                = this_atom->p_atom->hydration;
                        this_vdwf.e                = this_atom->p_atom->hybrid.num_elect;

                        if ( this_atom->resName == "WAT" ) {
                           this_vdwf.mw = 0.0;
                        }
                        tmp_atom.bead_coordinate = this_atom->coordinate;
                        tmp_atom.bead_computed_radius = this_vdwf.r;

                        tmp_atom.radius                    = tmp_atom.bead_computed_radius;
                        tmp_atom.bead_ref_mw               = this_vdwf.mw;
                        tmp_atom.bead_ref_ionized_mw_delta = this_vdwf.ionized_mw_delta;
                        tmp_atom.mw                        = this_vdwf.mw;
                        tmp_atom.ionized_mw_delta          = this_vdwf.ionized_mw_delta;
                        tmp_atom.bead_mw                   = this_vdwf.mw;
                        tmp_atom.bead_ionized_mw_delta     = this_vdwf.ionized_mw_delta;
                        tmp_atom.bead_color                = this_vdwf.color;
                        // #warning recolored for hydration
                        // tmp_atom.bead_color = hydrate && this_vdwf.w ? 1 : 6;
                        // #warning recolored for ASA testing
                        // tmp_atom.bead_color                = this_atom->asa >= asa.hydrate_threshold ? 6 : 8;
                        tmp_atom.bead_recheck_asa          = this_atom->asa;
                        tmp_atom.num_elect                 = this_vdwf.e;
                        tmp_atom.exposed_code              = 1;
                        tmp_atom.all_beads                 .clear( );
                        tmp_atom.active                    = true;
                        tmp_atom.name                      = this_atom->name;
                        tmp_atom.resName                   = this_atom->resName;
                        tmp_atom.iCode                     = this_atom->iCode;
                        tmp_atom.chainID                   = this_atom->chainID;
                        tmp_atom.saxs_excl_vol             = saxs_excl_vol;
                        if ( this_atom->asa >= asa.hydrate_threshold && this_vdwf.w ) {
                           tmp_atom.bead_hydration            = this_vdwf.w;
                        } else {
                           tmp_atom.bead_hydration            = 0;
                        }
                        tmp_atom.saxs_data.saxs_name       = "";
                        tmp_atom.bead_model_code = QString( "%1.%2.%3.%4.%5" )
                           .arg( this_atom->serial )
                           .arg( this_atom->name )
                           .arg( this_atom->resName )
                           .arg( this_atom->chainID )
                           .arg( this_atom->resSeq )
                           ;

                        {
                           pointmass tmpsas;
                           tmpsas.axis[0]    = this_atom->coordinate.axis[0];
                           tmpsas.axis[1]    = this_atom->coordinate.axis[1];
                           tmpsas.axis[2]    = this_atom->coordinate.axis[2];
                           tmpsas.mass       = tmp_atom.mw + tmp_atom.ionized_mw_delta;
                           tmpsas.name       = tmp_atom.name;
                           // tmpsas.radius     = tmp_atom.radius;
                           sas.push_back( tmpsas );
                        }
                        if ( show_bead_models ) {
                           bead_model.push_back(tmp_atom);
                        }
                     } else {
                        editor_msg( "red", QString( "Residue atom pair %1 unknown in vdwf.json" ).arg( res_idx ) );
                        return;
                     }
                  }
               }
            }
         }
         if ( show_bead_models ) {
            QString use_dir = somo_dir;
            QString spt_name = QString("%1_%2-SAS").arg( project ).arg( current_model + 1 );
            spt_name = spt_name.left( 30 );
            write_bead_spt( use_dir + QDir::separator() + spt_name, & bead_model );
            model_viewer( use_dir + QDir::separator() + spt_name + ".spt", "-script" );
         }

         if ( total_atoms && sas.size() != total_atoms ) {
            editor_msg( "black",
                        QString( us_tr( "SAS atoms: %1 (%2%) of total %3 atoms" ) )
                        .arg( sas.size() )
                        .arg( 100.0 * (double) sas.size() / total_atoms, 0, 'f', 1 )
                        .arg( total_atoms )
                        );
         }
      }

      // call US_Hydrodyn_Fractal_Dimension::compute_counting();

      {
         US_Fractal_Dimension          ufd;
         QString                       errormsg;

         if ( method != US_Fractal_Dimension::USFD_ROLL_SPHERE ) {

            if ( !ufd.compute(
                              method
                              ,sas
                              ,angstrom_start
                              ,angstrom_end
                              ,angstrom_steps
                              ,enright_ca_pct_start
                              ,enright_ca_pct_end
                              ,fd
                              ,x
                              ,y
                              ,x_title
                              ,y_title
                              ,type
                              ,errormsg
                              ) ) {
               if ( !quiet ) {
                  QMessageBox::critical( this,
                                         windowTitle() + us_tr( ": Fractal Dimension" ),
                                         QString( us_tr( "Error computing for model %1 : %2" ) )
                                         .arg( current_model + 1 )
                                         .arg( errormsg )
                                         );
               }
               return;
            }
         }

         // setup a dialog to show the plot

         if ( !x.size() ) {
            if ( !quiet ) {
               QMessageBox::critical( this,
                                      windowTitle() + us_tr( ": Fractal Dimension" ),
                                      QString( us_tr( "Error computing for model %1 : %2 - no points" ) )
                                      .arg( current_model + 1 )
                                      .arg( errormsg )
                                      );
            }
            return;
         }
            
         if ( !x[ 0 ].size() ) {
            if ( !quiet ) {
               QMessageBox::critical( this,
                                      windowTitle() + us_tr( ": Fractal Dimension" ),
                                      QString( us_tr( "Error computing for model %1 : %2 - no points" ) )
                                      .arg( current_model + 1 )
                                      .arg( errormsg )
                                      );
            }
            return;
         }

         if ( !x[ 0 ][ 0 ].size() ) {
            if ( !quiet ) {
               QMessageBox::critical( this,
                                      windowTitle() + us_tr( ": Fractal Dimension" ),
                                      QString( us_tr( "Error computing for model %1 : %2 - no points" ) )
                                      .arg( current_model + 1 )
                                      .arg( errormsg )
                                      );
            }
            return;
         }
            
         double use_xmin;
         double use_xmax;
         
         {
            double xmin = x[ 0 ][ 0 ].front();
            double xmax = x[ 0 ][ 0 ].back();

            // quicker if we just check the ends, but this was ensures against out-of-data in US_Fractal_Dimension::compute*()
            for ( auto const & v : x ) {
               for ( auto const & vv : v ) {
                  for ( auto const & vvv : vv ) {
                     if ( xmin > vvv ) {
                        xmin = vvv;
                     }
                     if ( xmax < vvv ) {
                        xmax = vvv;
                     }
                  }
               }
            }
               
            use_xmin = xmin;
            use_xmax = xmax;
            
            {
               QDialog dialog(this);
               dialog.setWindowTitle( windowTitle() + us_tr( ": Fractal Dimension" ) );
               // Use a layout allowing a label next to each field
               dialog.setMinimumWidth( 900 );

               QFormLayout form(&dialog);

               mQwtPlot    * plot;
               US_Plot       usp_plot(
                                      plot
                                      , ""
                                      , x_title
                                      , y_title
                                      , &dialog
                                      );

               US_Plot_Colors upc( plot->canvasBackground().color() );

               form.addRow( new QLabel( QString( us_tr( "Fractal Dimension %1 plot"  ) )
                                        .arg( type ) ) );
               form.addRow( &usp_plot );

               vector < vector < QwtPlotCurve * > > fitcurves;
               
               for ( size_t g = 0; g < x.size(); ++g ) {
                  vector < QwtPlotCurve * > this_fitcurves;
                  for ( size_t i = 0; i < x[ g ].size(); ++i ) {
                     QwtPlotCurve * curve = new QwtPlotCurve( QString( "fd_calc-%1" ).arg( i + 1 ) );
                     curve->setStyle( QwtPlotCurve::Dots );
                     curve->setSamples(
                                       (double *)&( x[ g ][ i ][ 0 ] ),
                                       (double *)&( y[ g ][ i ][ 0 ] ),
                                       x[ g ][ i ].size() );
                     // curve->setPen( QPen( Qt::cyan, 3, Qt::SolidLine ) );
                     curve->setPen( QPen( upc.color( i ), 3, Qt::SolidLine ) );
                     curve->attach( plot );

                     {
                        QwtPlotCurve * fitcurve = new QwtPlotCurve( QString( "fd_fit_%1" ).arg( i + 1 ) );
                        fitcurve->setStyle( QwtPlotCurve::Lines );
                        fitcurve->setPen( Qt::green, 2, Qt::DashDotLine );
                        fitcurve->attach( plot );
                        this_fitcurves.push_back( fitcurve );
                     }
                  }
                  fitcurves.push_back( this_fitcurves );
               }
      
               QList<QWidget *> fields;

               vector < QString > labels =
                  {
                     us_tr( "Fit start x value :" )
                     ,us_tr( "Fit end x value   :" )
                     ,us_tr( "Messages          :" )
                  };
      
               vector < QWidget * > widgets =
                  {
                     new QLineEdit( &dialog )
                     ,new QLineEdit( &dialog )
                     ,new QTextEdit( &dialog )
                  };

               vector < double >  defaults =
                  {
                     use_xmin
                     ,use_xmax
                     ,0
                  };

               vector < bool >  isinput  =
                  {
                     true
                     ,true
                     ,false
                  };

               for( int i = 0; i < (int) widgets.size(); ++i ) {
                  form.addRow( labels[ i ], widgets[ i ] );
                  if ( widgets[ i ] ) {
                     // could switch based on widgets[ i ]->className()
                     // assuming all input fields are doubles for now
                     if ( isinput[ i ] ) {
                        ((QLineEdit *)widgets[ i ])->setValidator( new QDoubleValidator(this) );
                        ((QLineEdit *)widgets[ i ])->setText( QString( "%1" ).arg( defaults[ i ] ) );
                     } else {
                        ((QTextEdit *)widgets[ i ])->setReadOnly( true );
                        QFontMetrics m( ((QTextEdit *)widgets[ i ])->font() );
                        ((QTextEdit *)widgets[ i ])->setFixedHeight( 11 * m.lineSpacing() + 5 );
                     }
                     fields << widgets[ i ];
                  }
               }

               QDialogButtonBox *buttonBox = new QDialogButtonBox( Qt::Horizontal, &dialog );

               buttonBox->addButton( QDialogButtonBox::Cancel );
               QPushButton * pb_replot = new QPushButton( us_tr( "Replot" ), &dialog );
               buttonBox->addButton( pb_replot, QDialogButtonBox::ActionRole );
               
               buttonBox->addButton( us_tr( "Export plot data to CSV" ), QDialogButtonBox::AcceptRole );

               form.addRow( buttonBox );
               QObject::connect( pb_replot
                                 , &QPushButton::clicked
                                 , [&](){
                                    use_xmin = ((QLineEdit *)fields[0])->text().toDouble();
                                    use_xmax = ((QLineEdit *)fields[1])->text().toDouble();
                                    linear_fit(
                                               use_xmin
                                               ,use_xmax
                                               ,x
                                               ,y
                                               ,plot
                                               ,fitcurves
                                               ,(QTextEdit *)widgets[ 2 ]
                                               ,fd
                                               ,fd_sd
                                               ,fd_wtd
                                               ,fd_wtd_sd
                                               ,fd_wtd_wtd
                                               ,fd_wtd_wtd_sd
                                               );
                                 } );

               QObject::connect( buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
               QObject::connect( buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

               linear_fit(
                          use_xmin
                          ,use_xmax
                          ,x
                          ,y
                          ,plot
                          ,fitcurves
                          ,(QTextEdit *)widgets[ 2 ]
                          ,fd
                          ,fd_sd
                          ,fd_wtd
                          ,fd_wtd_sd
                          ,fd_wtd_wtd
                          ,fd_wtd_wtd_sd
                          );

               plot->replot();

               if ( !show_plots ) {
                  if ( save_plot_data ) {
                     qDebug() << "export csv!";
                     US_Plot_Util upu;
                     map < QString, QwtPlot *> plots =
                        {
                           {
                              QString("_%1_%2_FD_%3_pr%4_thr%5_start%6_end%7_steps%8" )
                              .arg( project )
                              .arg( current_model + 1 )
                              .arg( type )
                              .arg( sas_asa_probe_radius )
                              .arg( sas_asa_threshold )
                              .arg( angstrom_start )
                              .arg( angstrom_end )
                              .arg( angstrom_steps )
                              ,plot
                           }
                        };
                     QString errors;
                     QString messages;
                     if ( !upu.printtofile( "Fractal Dimension " + type
                                            ,plots
                                            ,errors
                                            ,messages ) ) {
                        QMessageBox::warning( this,
                                              windowTitle() + us_tr( ": Fractal Dimension" ),
                                              QString( us_tr( "Errors saving plots\n:%1" ) )
                                              .arg( errors )
                                              );
                     } else {
                        QMessageBox::information( this,
                                                  windowTitle() + us_tr( ": Fractal Dimension" ),
                                                  QString( "Plots saved in directory <i>%1</i><br><br>%2" )
                                                  .arg( somo_tmp_dir.replace( "//", "/" ) )
                                                  .arg( messages )
                                                  );
                     }
                  }
               } else {
                  switch ( dialog.exec() ) {
                  case QDialog::Accepted :
                     {
                        qDebug() << "export csv!";
                        US_Plot_Util upu;
                        map < QString, QwtPlot *> plots =
                           {
                              {
                                 QString("_%1_%2_FD_%3_pr%4_thr%5_start%6_end%7_steps%8" )
                                 .arg( project )
                                 .arg( current_model + 1 )
                                 .arg( type )
                                 .arg( sas_asa_probe_radius )
                                 .arg( sas_asa_threshold )
                                 .arg( angstrom_start )
                                 .arg( angstrom_end )
                                 .arg( angstrom_steps )
                                 ,plot
                              }
                           };
                        QString errors;
                        QString messages;
                        if ( !upu.printtofile( "Fractal Dimension " + type
                                               ,plots
                                               ,errors
                                               ,messages ) ) {
                           QMessageBox::warning( this,
                                                 windowTitle() + us_tr( ": Fractal Dimension" ),
                                                 QString( us_tr( "Errors saving plots\n:%1" ) )
                                                 .arg( errors )
                                                 );
                        } else {
                           QMessageBox::information( this,
                                                     windowTitle() + us_tr( ": Fractal Dimension" ),
                                                     QString( "Plots saved in directory <i>%1</i><br><br>%2" )
                                                     .arg( somo_tmp_dir.replace( "//", "/" ) )
                                                     .arg( messages )
                                                     );
                        }
                     }
                     break;

                  case QDialog::Rejected :
                     qDebug() << "rejected";
                     break;

                  default :
                     qDebug() << "unknown exec response";
                     break;
                  }
               }
            }
         }

         if ( method == US_Fractal_Dimension::USFD_ROLL_SPHERE ) {
            fd     = 2.0 - fd;
            fd_wtd = 2.0 - fd_wtd;
         }

         if ( from_parameters ) {
            editor_msg( "black", us_tr( US_Hydrodyn_Fractal_Dimension_Options::options( gparams, use_xmin, use_xmax ) ) );
         }

         double rg_over_fd            = fd == 0 || fd == -1 ? -1 : model_vector[ current_model ].Rg / fd;
         double rg_over_fd_sd         = fd == 0 || fd == -1 ? -1 : fd_sd / fd;

         double rg_over_fd_wtd        = fd_wtd == 0 || fd_wtd == -1 ? -1 : model_vector[ current_model ].Rg / fd_wtd;
         double rg_over_fd_wtd_sd     = fd_wtd == 0 || fd_wtd == -1 ? -1 : fd_wtd_sd / fd_wtd;

         double rg_over_fd_wtd_wtd    = fd_wtd_wtd == 0 || fd_wtd_wtd == -1 ? -1 : model_vector[ current_model ].Rg / fd_wtd_wtd;
         double rg_over_fd_wtd_wtd_sd = fd_wtd_wtd == 0 || fd_wtd_wtd == -1 ? -1 : fd_wtd_wtd_sd / fd_wtd_wtd;

         editor_msg( "black",
                     QString(
                             us_tr( "Fractal dimension for model %1 : %2 %3 %4\n" )
                             .arg( current_model + 1 )
                             .arg( fd, 0, 'f', 3 )
                             .arg( UNICODE_PLUSMINUS )
                             .arg( fd_sd, 0, 'f', 3 )
                             )
                     );
         if ( rg_over_fd != -1 && advanced_config.expert_mode ) {
            editor_msg( "black",
                        QString(
                                us_tr( "Rg / Fractal dimension for model %1.: %2 %3 %4 [%5]\n" )
                                .arg( current_model + 1 )
                                .arg( rg_over_fd, 0, 'f', 3 )
                                .arg( UNICODE_PLUSMINUS )
                                .arg( rg_over_fd_sd, 0, 'f', 3 )
                                .arg( UNICODE_ANGSTROM )
                                )
                        );
         }

         if ( fd_wtd_wtd == -1 ) {
            editor_msg( "black",
                        QString(
                                us_tr( "Fractal dimension for model %1 wtd.: %2 %3 %4\n" )
                                .arg( current_model + 1 )
                                .arg( fd_wtd, 0, 'f', 3 )
                                .arg( UNICODE_PLUSMINUS )
                                .arg( fd_wtd_sd, 0, 'f', 3 )
                                )
                        );
            if ( rg_over_fd_wtd != -1 && advanced_config.expert_mode ) {
               editor_msg( "black",
                           QString(
                                   us_tr( "Rg / Fractal dimension for model %1 wtd.: %2 %3 %4 [%5]\n" )
                                   .arg( current_model + 1 )
                                   .arg( rg_over_fd_wtd, 0, 'f', 3 )
                                   .arg( UNICODE_PLUSMINUS )
                                   .arg( rg_over_fd_wtd_sd, 0, 'f', 3 )
                                   .arg( UNICODE_ANGSTROM )                                   
                                   )
                           );
            }
               
         } else {
            editor_msg( "black",
                        QString(
                                us_tr( "Fractal dimension for model %1 avg. of wtd.: %2 %3 %4\n" )
                                .arg( current_model + 1 )
                                .arg( fd_wtd, 0, 'f', 3 )
                                .arg( UNICODE_PLUSMINUS )
                                .arg( fd_wtd_sd, 0, 'f', 3 )
                                )
                        );
            if ( rg_over_fd_wtd != -1 && advanced_config.expert_mode ) {
               editor_msg( "black",
                           QString(
                                   us_tr( "Rg / Fractal dimension for model %1 avg. of wtd.: %2 %3 %4 [%5]\n" )
                                   .arg( current_model + 1 )
                                   .arg( rg_over_fd_wtd, 0, 'f', 3 )
                                   .arg( UNICODE_PLUSMINUS )
                                   .arg( rg_over_fd_wtd_sd, 0, 'f', 3 )
                                   .arg( UNICODE_ANGSTROM )                                   
                                   )
                           );
            }

            editor_msg( "black",
                        QString(
                                us_tr( "Fractal dimension for model %1 wtd. of wtd.: %2 %3 %4\n" )
                                .arg( current_model + 1 )
                                .arg( fd_wtd_wtd, 0, 'f', 3 )
                                .arg( UNICODE_PLUSMINUS )
                                .arg( fd_wtd_wtd_sd, 0, 'f', 3 )
                                )
                        );

            if ( rg_over_fd_wtd_wtd != -1 && advanced_config.expert_mode ) {
               editor_msg( "black",
                           QString(
                                   us_tr( "Rg / Fractal dimension for model %1 wtd. of wtd.: %2 %3 %4 [%5]\n" )
                                   .arg( current_model + 1 )
                                   .arg( rg_over_fd_wtd_wtd, 0, 'f', 3 )
                                   .arg( UNICODE_PLUSMINUS )
                                   .arg( rg_over_fd_wtd_wtd_sd, 0, 'f', 3 )
                                   .arg( UNICODE_ANGSTROM )                                   
                                   )
                           );
            }
         }
         if ( from_parameters ) {
            model_vector[ current_model ].fractal_dimension_parameters         = US_Hydrodyn_Fractal_Dimension_Options::options( gparams, use_xmin, use_xmax );
            model_vector[ current_model ].fractal_dimension                    = fd;
            model_vector[ current_model ].fractal_dimension_sd                 = fd_sd;
            model_vector[ current_model ].fractal_dimension_wtd                = fd_wtd;
            model_vector[ current_model ].fractal_dimension_wtd_sd             = fd_wtd_sd;
            model_vector[ current_model ].fractal_dimension_wtd_wtd            = fd_wtd_wtd;
            model_vector[ current_model ].fractal_dimension_wtd_wtd_sd         = fd_wtd_wtd_sd;
            model_vector[ current_model ].rg_over_fractal_dimension            = rg_over_fd;
            model_vector[ current_model ].rg_over_fractal_dimension_sd         = rg_over_fd_sd;
            model_vector[ current_model ].rg_over_fractal_dimension_wtd        = rg_over_fd_wtd;
            model_vector[ current_model ].rg_over_fractal_dimension_wtd_sd     = rg_over_fd_wtd_sd;
            model_vector[ current_model ].rg_over_fractal_dimension_wtd_wtd    = rg_over_fd_wtd_wtd;
            model_vector[ current_model ].rg_over_fractal_dimension_wtd_wtd_sd = rg_over_fd_wtd_wtd_sd;

            model_vector_as_loaded[ current_model ].fractal_dimension_parameters         = US_Hydrodyn_Fractal_Dimension_Options::options( gparams, use_xmin, use_xmax );
            model_vector_as_loaded[ current_model ].fractal_dimension                    = fd;
            model_vector_as_loaded[ current_model ].fractal_dimension_sd                 = fd_sd;
            model_vector_as_loaded[ current_model ].fractal_dimension_wtd                = fd_wtd;
            model_vector_as_loaded[ current_model ].fractal_dimension_wtd_sd             = fd_wtd_sd;
            model_vector_as_loaded[ current_model ].fractal_dimension_wtd_wtd            = fd_wtd_wtd;
            model_vector_as_loaded[ current_model ].fractal_dimension_wtd_wtd_sd         = fd_wtd_wtd_sd;
            model_vector_as_loaded[ current_model ].rg_over_fractal_dimension            = rg_over_fd;
            model_vector_as_loaded[ current_model ].rg_over_fractal_dimension_sd         = rg_over_fd_sd;
            model_vector_as_loaded[ current_model ].rg_over_fractal_dimension_wtd        = rg_over_fd_wtd;
            model_vector_as_loaded[ current_model ].rg_over_fractal_dimension_wtd_sd     = rg_over_fd_wtd_sd;
            model_vector_as_loaded[ current_model ].rg_over_fractal_dimension_wtd_wtd    = rg_over_fd_wtd_wtd;
            model_vector_as_loaded[ current_model ].rg_over_fractal_dimension_wtd_wtd_sd = rg_over_fd_wtd_wtd_sd;

            qDebug() <<
               QString( "US_Hydrodyn::fractal_dimension() set model_vector[ %1 ], FD %2 +/- %3 ; %4\n" )
               .arg( current_model )
               .arg( model_vector[ current_model ].fractal_dimension )
               .arg( model_vector[ current_model ].fractal_dimension_sd )
               .arg( model_vector[ current_model ].fractal_dimension_parameters )
               ;

            if ( fd_save_info ) {
               fd_save_info->data                                      = US_Hydrodyn_Save::save_data_initialized();
               fd_save_info->data.results.name                         = QString("%1_%2").arg( project ).arg( current_model + 1 );
               fd_save_info->data.fractal_dimension_parameters         = US_Hydrodyn_Fractal_Dimension_Options::options( gparams, use_xmin, use_xmax );
               fd_save_info->data.fractal_dimension                    = fd;
               fd_save_info->data.fractal_dimension_sd                 = fd_sd;
               fd_save_info->data.fractal_dimension_wtd                = fd_wtd;
               fd_save_info->data.fractal_dimension_wtd_sd             = fd_wtd_sd;
               fd_save_info->data.fractal_dimension_wtd_wtd            = fd_wtd_wtd;
               fd_save_info->data.fractal_dimension_wtd_wtd_sd         = fd_wtd_wtd_sd;
               fd_save_info->data.rg_over_fractal_dimension            = rg_over_fd;
               fd_save_info->data.rg_over_fractal_dimension_sd         = rg_over_fd_sd;
               fd_save_info->data.rg_over_fractal_dimension_wtd        = rg_over_fd_wtd;
               fd_save_info->data.rg_over_fractal_dimension_wtd_sd     = rg_over_fd_wtd_sd;
               fd_save_info->data.rg_over_fractal_dimension_wtd_wtd    = rg_over_fd_wtd_wtd;
               fd_save_info->data.rg_over_fractal_dimension_wtd_wtd_sd = rg_over_fd_wtd_wtd_sd;
               fd_save_info->data_vector.push_back( fd_save_info->data );
            }      
         }
      }
   }
}

void US_Hydrodyn::show_fractal_dimension_options() {
   if ( fractal_dimension_options_widget ) {
      if ( fractal_dimension_options_window->isVisible() ) {
         fractal_dimension_options_window->raise();
      } else {
         fractal_dimension_options_window->show();
      }
      return;
   } else {
      fractal_dimension_options_window =
         new US_Hydrodyn_Fractal_Dimension_Options(
                                                   &gparams
                                                   ,&fractal_dimension_options_widget
                                                   ,this
                                                   );
      fixWinButtons( fractal_dimension_options_window );
      fractal_dimension_options_window->show();
   }
}
