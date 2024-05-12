#include "../include/us_hydrodyn_pdbdefs.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_fractal_dimension.h"
#include "../include/us_hydrodyn_asab1.h"
#include <qmessagebox.h>
#include "../include/us_plot_util.h"
#include "../include/us_unicode.h"

#define UHFD_PLOT_EPSILON 2e-3

static bool linear_fit(
                       const double                                   & xmin
                       ,const double                                  & xmax
                       ,const vector < vector < vector < double > > >   x
                       ,const vector < vector < vector < double > > >   y
                       ,mQwtPlot                                      * plot
                       ,vector < vector < QwtPlotCurve * > >          & fitcurves
                       ,QTextEdit                                     * msgbox
                       ) {
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
         double xsizeinv = 1e0 / (double) x[ g ].size();
         msgbox->setText(
                         msgbox->toPlainText()
                         + QString( "%1" ).arg(
                                               x.size() > 1
                                               ? QString( "Group %1 (size %2): " ).arg( g + 1 ).arg( x[ g ].size() )
                                               : QString( "" )
                                               )
                         + QString( "a = %1 (%2%3)  b = %4 (%5%6)\n" )
                         .arg( a_sum * xsizeinv, 0, 'f', 3 )
                         .arg( UNICODE_PLUSMINUS )
                         .arg( siga_sum * xsizeinv, 0, 'f', 3 )
                         .arg( b_sum * xsizeinv, 0, 'f', 3 )
                         .arg( UNICODE_PLUSMINUS )
                         .arg( sigb_sum * xsizeinv, 0, 'f', 3 )
                         );
      }
   }

   // compute global averages if needed

   if ( x.size() > 1
        && global_count ) {
      double globalcountinv = 1e0 / (double) global_count;
      msgbox->setText(
                      msgbox->toPlainText()
                      + QString( "%1" ).arg(
                                            x.size() > 1
                                            ? "Global average : "
                                            : ""
                                            )
                      + QString( "a = %1 (%2%3)  b = %4 (%5%6)" )
                      .arg( global_a_sum * globalcountinv, 0, 'f', 3 )
                      .arg( UNICODE_PLUSMINUS )
                      .arg( global_siga_sum * globalcountinv, 0, 'f', 3 )
                      .arg( global_b_sum * globalcountinv, 0, 'f', 3 )
                      .arg( UNICODE_PLUSMINUS )
                      .arg( global_sigb_sum * globalcountinv, 0, 'f', 3 )
                      );
   }

   plot->replot();

   return true;
}

void US_Hydrodyn::fractal_dimension() {
   qDebug() << "US_Hydrodyn::fractal_dimension()";
   stopFlag = false;

   // for each selected model

   set < int > selected_models;

   // pdb loaded check

   for( int i = 0; i < lb_model->count(); i++ ) {
      if ( lb_model->item( i )->isSelected() ) {
         selected_models.insert( i );
      }
   }
   
   if ( !selected_models.size() ) {
      QMessageBox::critical( this,
                             windowTitle() + us_tr( ": Fractal Dimension" ),
                             us_tr( "No models selected" )
                             );
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
   
   // pat 1st model for extents
   point extents;
   {
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
   {
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
               QString( us_tr( "SAS ASA threshold [%1^2]" ) ).arg( UNICODE_ANGSTROM )
               ,QString( us_tr( "SAS ASA probe radius [%1]" ) ).arg( UNICODE_ANGSTROM )
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

   bool do_display_bead_models =
      method == US_Fractal_Dimension::USFD_ROLL_SPHERE
      ? false
      : QMessageBox::Yes == QMessageBox::question(
                                                  this,
                                                  windowTitle() + us_tr( ": Fractal Dimension" ),
                                                  us_tr("Display bead models of SAS ?" ),
                                                  QMessageBox::Yes, 
                                                  QMessageBox::No | QMessageBox::Default
                                                  );
   
   for ( auto const current_model : selected_models ) {
      vector < vector < vector < double > > > x;
      vector < vector < vector < double > > > y;
      QString                                 type;
      QString                                 x_title;
      QString                                 y_title;
      vector < pointmass >                    sas;

      editor_msg( "darkblue", QString( us_tr( "Fractal dimension processing model %1\n" ) ).arg( current_model + 1 ) );

      if ( do_display_bead_models ) {
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
                        if ( do_display_bead_models ) {
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
         if ( do_display_bead_models ) {
            QString use_dir = somo_dir;
            QString spt_name = QString("%1_%2-SAS").arg( project ).arg( current_model + 1 );
            spt_name = spt_name.left( 30 );
            write_bead_spt( use_dir + QDir::separator() + spt_name, & bead_model );
            model_viewer( use_dir + QDir::separator() + spt_name + ".spt", "-script" );
         }

         if ( total_atoms ) {
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
         double                        fd = 0;
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
               QMessageBox::critical( this,
                                      windowTitle() + us_tr( ": Fractal Dimension" ),
                                      QString( us_tr( "Error computing for model %1 : %2" ) )
                                      .arg( current_model + 1 )
                                      .arg( errormsg )
                                      );
               return;
            }
         }

         // setup a dialog to show the plot

         if ( !x.size() ) {
            QMessageBox::critical( this,
                                   windowTitle() + us_tr( ": Fractal Dimension" ),
                                   QString( us_tr( "Error computing for model %1 : %2 - no points" ) )
                                   .arg( current_model + 1 )
                                   .arg( errormsg )
                                   );
            return;
         }
            
         if ( !x[ 0 ].size() ) {
            QMessageBox::critical( this,
                                   windowTitle() + us_tr( ": Fractal Dimension" ),
                                   QString( us_tr( "Error computing for model %1 : %2 - no points" ) )
                                   .arg( current_model + 1 )
                                   .arg( errormsg )
                                   );
            return;
         }

         if ( !x[ 0 ][ 0 ].size() ) {
            QMessageBox::critical( this,
                                   windowTitle() + us_tr( ": Fractal Dimension" ),
                                   QString( us_tr( "Error computing for model %1 : %2 - no points" ) )
                                   .arg( current_model + 1 )
                                   .arg( errormsg )
                                   );
            return;
         }
            
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
               
            double use_xmin = xmin;
            double use_xmax = xmax;
            
            {
               QDialog dialog(this);
               dialog.setWindowTitle( windowTitle() + us_tr( ": Fractal Dimension" ) );
               // Use a layout allowing a label next to each field
               dialog.setMinimumWidth( 600 );

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
                                    linear_fit( use_xmin, use_xmax, x, y, plot, fitcurves, (QTextEdit *)widgets[ 2 ] );
                                 } );

               QObject::connect( buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
               QObject::connect( buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

               linear_fit( use_xmin, use_xmax, x, y, plot, fitcurves, (QTextEdit *)widgets[ 2 ] );

               plot->replot();
               
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

         editor_msg( "black", QString( us_tr( "Fractal dimension for model %1 : %2\n" )
                                       .arg( current_model + 1 )
                                       .arg( fd ) ) );
      }
   }
}
