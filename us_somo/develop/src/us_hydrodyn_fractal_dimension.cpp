#include "../include/us_hydrodyn_pdbdefs.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_fractal_dimension.h"
#include "../include/us_hydrodyn_asab1.h"
#include <qmessagebox.h>
#include "../include/us_plot_util.h"

void US_Hydrodyn::fractal_dimension() {
   qDebug() << "US_Hydrodyn::fractal_dimension()";

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
      
   double sas_asa_threshold    = asa.threshold;
   double sas_asa_probe_radius = 1.5;
   double box_angstrom_start   = 0;
   double box_angstrom_end     = 0;
   double box_angstrom_steps   = 0;
   
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
                                 us_tr(
                                       "Set the box counting parameters\n"
                                       "Fill out the values below and click OK\n"
                                       )
                                 ) );

         // Add the lineEdits with their respective labels
         QList<QWidget *> fields;
   
          vector < QString > labels =
            {
               QString( us_tr( "SAS ASA threshold [%1^2]   :" ) ).arg( UNICODE_ANGSTROM )
               ,QString( us_tr( "SAS ASA probe radius [%1] :" ) ).arg( UNICODE_ANGSTROM )
               ,QString( us_tr( "Start box edge size [%1]  :" ) ).arg( UNICODE_ANGSTROM )
               ,QString( us_tr( "Ending box edge size [%1] :" ) ).arg( UNICODE_ANGSTROM )
               ,us_tr( "Number of steps           :" )
               
            };
      
         vector < QWidget * > widgets =
            {
               new QLineEdit( &dialog )
               ,new QLineEdit( &dialog )
               ,new QLineEdit( &dialog )
               ,new QLineEdit( &dialog )
               ,new QLineEdit( &dialog )
            };

         vector < double >  defaults =
            {
               1
               ,sas_asa_probe_radius
               ,.5
               ,5
               ,100
            };

         vector < bool >  intvalidator =
            {
               false
               ,false
               ,false
               ,false
               ,true
            };

         for( int i = 0; i < (int) widgets.size(); ++i ) {
            form.addRow( labels[i], widgets[i] );
            if ( widgets[i] ) {
               // could switch based on widgets[i]->className()
               // assuming all input fields are doubles for now
               if ( intvalidator[ i ] ) {
                  ((QLineEdit *)widgets[i])->setValidator( new QIntValidator(this) );
               } else {
                  ((QLineEdit *)widgets[i])->setValidator( new QDoubleValidator(this) );
               }                  
               ((QLineEdit *)widgets[i])->setText( QString( "%1" ).arg( defaults[i] ) );
               fields << widgets[i];
            }
         }

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
                 || ((QLineEdit *)fields[2])->text().toDouble() <= 0
                 || ((QLineEdit *)fields[3])->text().toDouble() <= 0
                 || ((QLineEdit *)fields[4])->text().toDouble() <= 0
                 || ((QLineEdit *)fields[2])->text().toDouble() >= ((QLineEdit *)fields[3])->text().toDouble()
                 ) {
               try_again = true;
            } else {
               // If the user didn't dismiss the dialog, do something with the fields
               sas_asa_threshold    = ((QLineEdit *)fields[0])->text().toDouble();
               sas_asa_probe_radius = ((QLineEdit *)fields[1])->text().toDouble();
               box_angstrom_start   = ((QLineEdit *)fields[2])->text().toDouble();
               box_angstrom_end     = ((QLineEdit *)fields[3])->text().toDouble();
               box_angstrom_steps   = ((QLineEdit *)fields[4])->text().toDouble();
            }
         } else {
            return;
         }
      } while ( try_again );
   }

   QTextStream( stdout ) << QString( "US_Hydrodyn::fractal_dimension()\n"
                                     "sas_asa_threshold    %1\n"
                                     "sas_asa_probe_radius %2\n"
                                     "box_angstrom_start   %3\n"
                                     "box_angstrom_end     %4\n"
                                     "box_angstrom_steps   %5\n" )
      .arg( sas_asa_threshold )
      .arg( sas_asa_probe_radius )
      .arg( box_angstrom_start )
      .arg( box_angstrom_end )
      .arg( box_angstrom_steps )
      ;

   bool do_display_bead_models =
      QMessageBox::Yes == QMessageBox::question(
                                                this,
                                                windowTitle() + us_tr( ": Fractal Dimension" ),
                                                us_tr("Display bead models of SAS ?" ),
                                                QMessageBox::Yes, 
                                                QMessageBox::No | QMessageBox::Default
                                                );
   
   for ( auto const current_model : selected_models ) {
      editor_msg( "darkblue", QString( us_tr( "Fractal dimension processing model %1\n" ) ).arg( current_model + 1 ) );

      if ( do_display_bead_models ) {
         bead_model.clear( );
      }

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

      vector < point > sas;

      size_t total_atoms = 0;

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


            if ( this_atom->asa >= sas_asa_threshold ) {
               sas.push_back( this_atom->coordinate );
               if ( do_display_bead_models ) {
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

                     bead_model.push_back(tmp_atom);
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

      // call US_Hydrodyn_Fractal_Dimension::compute_box_counting();
      {
         US_Fractal_Dimension ufd;
         QString              errormsg;
         double               fd = 0;
         vector < double >    x;
         vector < double >    y;

         if ( !ufd.compute_box_counting(
                                        sas
                                        ,box_angstrom_start
                                        ,box_angstrom_end
                                        ,box_angstrom_steps
                                        ,fd
                                        ,errormsg
                                        ,x
                                        ,y
                                         ) ) {
            QMessageBox::critical( this,
                                   windowTitle() + us_tr( ": Fractal Dimension" ),
                                   QString( us_tr( "Error computing for model %1 : %2" ) )
                                   .arg( current_model + 1 )
                                   .arg( errormsg )
                                   );
            return;
         }

         // setup a dialog to show the plot

         {
            QDialog dialog(this);
            dialog.setWindowTitle( windowTitle() + us_tr( ": Fractal Dimension" ) );
            // Use a layout allowing a label next to each field
            dialog.setMinimumWidth( 600 );

            QFormLayout form(&dialog);

            mQwtPlot    * plot;
            US_Plot       usp_plot( plot, "", "log(volume of each box [" + UNICODE_ANGSTROM_QS + "^3])", "log(number of boxes)", &dialog );

            form.addRow( new QLabel( us_tr( "Fractal Dimension plot" ) ) );
            form.addRow( &usp_plot );

            QwtPlotCurve curve( "fd_calc" );
            curve.setStyle( QwtPlotCurve::Dots );
            curve.setSamples(
                              (double *)&( x[ 0 ] ),
                              (double *)&( y[ 0 ] ),
                              x.size() );
            curve.setPen( QPen( QColor( Qt::cyan ), 3, Qt::SolidLine ) );
            curve.attach( plot );
      
            plot->replot();

            QDialogButtonBox *buttonBox = new QDialogButtonBox( Qt::Horizontal, &dialog );

            buttonBox->addButton( QDialogButtonBox::Cancel );
            buttonBox->addButton( us_tr( "Export plot data to CSV" ), QDialogButtonBox::AcceptRole );

            form.addRow( buttonBox );

            QObject::connect( buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
            QObject::connect( buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));
            if ( dialog.exec() == QDialog::Accepted ) {
               qDebug() << "export csv!";
               US_Plot_Util upu;
               map < QString, QwtPlot *> plots =
                  {
                     {
                        QString("_%1_%2_FD_box_pr%3_thr%4_start%5_end%6_steps%7")
                        .arg( project )
                        .arg( current_model + 1 )
                        .arg( sas_asa_probe_radius )
                        .arg( sas_asa_threshold )
                        .arg( box_angstrom_start )
                        .arg( box_angstrom_end )
                        .arg( box_angstrom_steps )
                        ,plot
                     }
                  };
               QString errors;
               QString messages;
               if ( !upu.printtofile( "Fractal Dimension Box"
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
         }

         editor_msg( "black", QString( us_tr( "Fractal dimension for model %1 : %2\n" )
                                       .arg( current_model + 1 )
                                       .arg( fd ) ) );
      }
   }
}
