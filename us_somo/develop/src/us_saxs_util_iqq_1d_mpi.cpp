#include <mpi.h>
#include "../include/us_saxs_util.h"
#include "../include/us_file_util.h"
#include "../include/us_timer.h"
#include <sys/time.h>
//Added by qt3to4:
#include <QTextStream>
extern int npes;
extern int myrank;
extern QString outputData;

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

#if defined( CUDA )
#  include "us_cuda.h"
   extern int env_mpi_node;
#endif

static US_Timer usui1m_timer;

bool US_Saxs_Util::run_1d_mpi( QString controlfile )
{
   // for now, everyone reads the control file & sets things up to the point of nsa run
   if ( !myrank )
   {
      usui1m_timer.init_timer ( "1d" );
      usui1m_timer.init_timer ( "1d init" );
      usui1m_timer.init_timer ( "1d closeout" );
      usui1m_timer.init_timer ( "1d random generation" );
      usui1m_timer.start_timer ( "1d" );
      usui1m_timer.start_timer ( "1d init" );
   }

   QString qs_base_dir = QDir::currentPath();

   outputData = QString( "%1" ).arg( getenv( "outputData" ) );
   if ( outputData.isEmpty() )
   {
      outputData = "outputData";
   }
   if ( !myrank )
   {
      cout << QString( "Notice: outputData is \"%1\"\n" ).arg( outputData );
   }

   int errorno = -1;
   nsa_mpi = true;

   errormsg = "";
   if ( !controlfile.contains( QRegExp( "\\.(tgz|TGZ|tar|TAR)$" ) ) )
   {
      errormsg = QString( "controlfile must be .tgz or .tar, was %1" ).arg( controlfile );
      cerr << errormsg << endl << flush;
      MPI_Abort( MPI_COMM_WORLD, errorno );
      exit( errorno );
   }         
   errorno--;

   if ( !QFile::exists( controlfile ) )
   {
      errormsg = QString( "controlfile %1 does not exist" ).arg( controlfile );
      cerr << errormsg << endl << flush;
      MPI_Abort( MPI_COMM_WORLD, errorno );
      exit( errorno );
   }         
   errorno--;

   QStringList qslt;
   unsigned int sizeoflist;

   // this first barrier seems useless, but there seems to be an issue
   // where if one process takes awhile to get started, then the later barrier
   // gets stuck
   // this was noted on mpich2-1.4.1p1 &
   // openmpi2-1.4.3

   // cout << QString("%1: initial universal barrier\n" ).arg( myrank ) << flush;
   if ( MPI_SUCCESS != MPI_Barrier( MPI_COMM_WORLD ) )
   {
      MPI_Abort( MPI_COMM_WORLD, errorno );
      exit( errorno );
   }         
   errorno--;

   QString original_controlfile;

   if ( myrank )
   {
      // wait until rank 0 process has extracted the file:
      // cout << QString("%1: waiting for process 0 to finish\n" ).arg( myrank ) << flush;
      if ( MPI_SUCCESS != MPI_Barrier( MPI_COMM_WORLD ) )
      {
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }         
      errorno--;

      // cout << QString("%1: waiting for broadcast of size of list\n" ).arg( myrank ) << flush;
      if ( MPI_SUCCESS != MPI_Bcast( &sizeoflist, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD ) )
      {
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }         
      errorno--;

      char char_files[ sizeoflist + 1 ];

      // cout << QString("%1: waiting for broadcast of list data\n" ).arg( myrank ) << flush;
      if ( MPI_SUCCESS != MPI_Bcast( char_files, sizeoflist + 1, MPI_CHAR, 0, MPI_COMM_WORLD ) )
      {
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }         
      errorno--;

      // qslt = (QString( "%1" ).split( "\n" , Qt::SkipEmptyParts ).arg( char_files ) );
      {
         QString qs = QString( "%1" ).arg( char_files );
         qslt = (qs ).split( "\n" , Qt::SkipEmptyParts );
      }
   } else {
      // cout << QString("%1: extracting job files\n").arg( myrank ) << flush;
      
      QString dest = controlfile;
      int result;

      // copy here
      US_File_Util usu;
      usu.copy( controlfile, QDir::currentPath() + QDir::separator() + QFileInfo( controlfile ).fileName() );
      cout << QString( "copying %1 %2 <%3>\n" )
         .arg( controlfile )
         .arg( QDir::currentPath() + QDir::separator() + QFileInfo( controlfile ).fileName() )
         .arg( usu.errormsg );
      dest = QFileInfo( controlfile ).fileName();

      cout << QString( "dest is now %1\n" )
         .arg( dest );
      original_controlfile = dest;

      if ( controlfile.contains( QRegExp( "\\.(tgz|TGZ)$" ) ) )
      {
         // gunzip controlfile, must be renamed for us_gzip
         
         // rename
         dest.replace( QRegExp( "\\.(tgz|TGZ)$" ), ".tar.gz" );
         QDir qd;
         qd.remove( dest );
         if ( !qd.rename( controlfile, dest ) )
         {
            cout << QString("Error renaming %1 to %2\n").arg( controlfile ).arg( dest );
            MPI_Abort( MPI_COMM_WORLD, errorno );
            exit( errorno );
         }
         errorno--;
         
         controlfile = dest;

         US_Gzip usg;
         result = usg.gunzip( controlfile );
         if ( GZIP_OK != result )
         {
            cout << QString("Error: %1 problem gunzipping (%2)\n").arg( controlfile ).arg( usg.explain( result ) );
            MPI_Abort( MPI_COMM_WORLD, errorno );
            exit( errorno );
         }
         errorno--;

         controlfile.replace( QRegExp( "\\.gz$" ), "" );
      }

      // tar open controlfile
      US_Tar ust;
      result = ust.extract( controlfile, &qslt );
      if ( TAR_OK != result )
      {
         cout << QString("Error: %1 problem extracting tar archive (%2)\n").arg( controlfile ).arg( ust.explain( result ) );
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }
      errorno--;

      // cout << QString("%1: finished extracting job files\n" ).arg( myrank ) << flush;

      QString qs_files = qslt[ 0 ];
      sizeoflist = qs_files.length();
      char char_files[ sizeoflist + 1 ];
      strncpy( char_files, qs_files.toLatin1().data(), sizeoflist + 1 );

      // cout << QString("%1: signaling end of barrier\n" ).arg( myrank ) << flush;
      if ( MPI_SUCCESS != MPI_Barrier( MPI_COMM_WORLD ) )
      {
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }         
      errorno--;

      // cout << QString("%1: broadcasting size\n" ).arg( myrank ) << flush;
      if ( MPI_SUCCESS != MPI_Bcast( &sizeoflist, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD ) )
      {
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }         
      errorno--;
         
      // cout << QString("%1: broadcast list data\n" ).arg( myrank ) << flush;
      if ( MPI_SUCCESS != MPI_Bcast( char_files, sizeoflist + 1, MPI_CHAR, 0, MPI_COMM_WORLD ) )
      {
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }         
      errorno--;
   }

   // we should all have the same qslt now

   // cout << QString( "%1: %2\n" ).arg( myrank ).arg( qslt.join( ":" ) ) << flush;

   // everyone reads for now
   controlfile = qslt[ 0 ];

   if ( !myrank )
   {
      usui1m_timer.end_timer ( "1d init" );
   }
   if ( !read_control( controlfile ) )
   {
      cout << QString( "%1: Error: %2\n" ).arg( myrank ).arg( errormsg ) << flush;
      MPI_Abort( MPI_COMM_WORLD, errorno );
      exit( errorno );
   }         
   errorno--;

#if defined( CUDA )
   cuda_ipcrm();
#endif

   // cout << QString("%1: signaling end of barrier\n" ).arg( myrank ) << flush;
   if ( MPI_SUCCESS != MPI_Barrier( MPI_COMM_WORLD ) )
   {
      MPI_Abort( MPI_COMM_WORLD, errorno );
      exit( errorno );
   }         
   
   //    cout << 
   //       QString( "%1: my output files:<%2>\n" )
   //       .arg( myrank )
   //       .arg( output_files.join( ":" ) ) 
   //         << flush;

   // join up outputfiles:

   if ( !myrank )
   {
      usui1m_timer.start_timer ( "1d closeout" );

      QStringList org_output_files = output_files;
      output_files.clear( );
      QString errors;

      for ( int i = 0; i < ( int ) org_output_files.size(); i++ )
      {
         QString qs   = org_output_files[ i ];
         qs.replace( "_rank0_" , "_rank%1_" );
         QString name = org_output_files[ i ];
         name.replace( QRegExp( "^.*_rank0_" ) , "" );
         cout << QString( "new name <%1>\n" ).arg( name );

         vector < double > q;
         vector < double > I;

         // open up & average:

         int number_of_files_read = 0;

         for ( int r = 0; r < npes; r++ )
         {
            QFile f( QString( qs ).arg( r ) );

            if ( !f.exists() )
            {
               cout << QString( "%1: expected output file %2 does not exist\n" )
                  .arg( myrank )
                  .arg( f.fileName() )
                       << flush;
               MPI_Abort( MPI_COMM_WORLD, -5001 );
               exit( -5001 );
            }
            if ( !set_control_parameters_from_experiment_file( f.fileName(), true ) )
            {
               errors +=  QString( "%1: Error trying to get grid, ignorming this processor%2\n" ).arg( myrank ).arg( errormsg );
               continue;
            }
            number_of_files_read++;
            if ( !r )
            {
               q = sgp_exp_q;
               I = sgp_exp_I;
            } else {
               if ( I.size() != sgp_exp_I.size() )
               {
                  cout << QString( "%1: collect results size mismatch %1 vs %2 \n" )
                     .arg( myrank )
                     .arg( I.size() )
                     .arg( sgp_exp_I.size() )
                       << flush;
                  MPI_Abort( MPI_COMM_WORLD, -5003 );
                  exit( -5003 );
               }
               for ( unsigned int i = 0; i < ( unsigned int )sgp_exp_I.size(); i++ )
               {
                  I[ i ] += sgp_exp_I[ i ];
               }
            }
         }
         for ( unsigned int i = 0; i < ( unsigned int )sgp_exp_I.size(); i++ )
         {
            I[ i ] /= (double) number_of_files_read;
         }
         if ( !write_output( name, q, I ) )
         {
            cout << QString( "%1: %2\n" )
               .arg( myrank )
               .arg( errormsg )
                 << flush;

            MPI_Abort( MPI_COMM_WORLD, -5004 );
            exit( -5004 );
         }
      }
      cout << 
         QString( "%1: my output files:<%2>\ncontrolfile %3\n" )
         .arg( myrank )
         .arg( output_files.join( ":" ) ) 
         .arg( controlfile )
           << flush;

      if ( !errors.isEmpty() )
      {
         QFile f( "errors" );
         if ( !f.open( QIODevice::WriteOnly ) )
         {
            cout << "Error: errors exist but can not open errors file\n" << flush;
            MPI_Abort( MPI_COMM_WORLD, -5002 );
            exit( -5002 );
         }

         QTextStream ts( &f );
         ts << errors;
         f.close();
         output_files << "errors";
      }

      usui1m_timer.end_timer ( "1d closeout" );
      usui1m_timer.end_timer ( "1d" );

      QFile f( "runinfo" );
      if ( f.open( QIODevice::WriteOnly ) )
      {
         QTextStream ts( &f );
         ts << QString( "number of processors %1\n" ).arg( npes );
         ts << "timings:\n";
         ts << usui1m_timer.list_times();;
         ts << "end-timings\n";
         QFile fc( controlfile );
         if ( fc.open( QIODevice::ReadOnly ) )
         {
            QTextStream tsc( &fc );
            ts << "controlfile:\n";
            while( !tsc.atEnd() )
            {
               ts << tsc.readLine() << Qt::endl;
            }
            ts << "end-controlfile\n";
            fc.close();
         }
         f.close();
         output_files << "runinfo";
      } else {
         cout << "Warning: could not create timings\n" << flush;
      }

      // package output
      if ( !create_tgz_output( controlfile + "_out.tgz" ) )
      {
         cout << QString( "%1: %2\n" )
            .arg( myrank )
            .arg( errormsg )
              << flush;
         MPI_Abort( MPI_COMM_WORLD, -5005 );
         exit( -5005 );
      }
   }

   if ( !myrank )
   {
      QString results_file = original_controlfile;
      results_file.replace( QRegExp( "\\.(tgz|TGZ|tar|TGZ)$" ), "" );
      results_file += "_out.tgz";

      QDir dod( outputData );
      if ( !dod.exists() )
      {
         QDir current = QDir::current();
            
         QString newdir = outputData;
         while ( newdir.left( 3 ) == "../" )
         {
            current.cdUp();
            newdir.replace( "../", "" );
         }
         QDir::setCurrent( current.path() );
         QDir ndod;
         if ( !ndod.mkdir( newdir ) )
         {
            cout << QString("Warning: could not create outputData \"%1\" directory\n" ).arg( ndod.path() );
         }
         QDir::setCurrent( qs_base_dir );
      }
      if ( dod.exists() )
      {
         QString dest = outputData + QDir::separator() + QFileInfo( results_file ).fileName();
         QDir qd;
         cout << QString("renaming: %1 to %2\n" )
               .arg( results_file )
               .arg( dest );
         if ( !qd.rename( results_file, dest ) )
         {
            cout << QString("Warning: could not rename outputData %1 to %2\n" )
               .arg( results_file )
               .arg( dest );
         }
      } else {
         cout << QString( "Error: %1 does not exist\n" ).arg( outputData );
      }
   }

   MPI_Finalize();

   exit( 0 );
   return true;
}

bool US_Saxs_Util::compute_1d_mpi()
{
   errormsg = "";
   noticemsg = "";

   QString missing_required;

   puts( "compute 1d" );
   QStringList qsl_required;
   if ( control_parameters.count( "output" ) &&
        ( control_parameters[ "output" ] == "csv" ) )
   {
      if ( !myrank )
      {
         errormsg = "compute_1d_mpi: output type 'csv' not currently supported";
      }
      return false;
   }

   {
      qsl_required << "residuefile";
      qsl_required << "1dlambda";
      qsl_required << "1ddetectordistance";
      qsl_required << "1ddetectorwidth";
      qsl_required << "1ddetectorpixelswidth";
      qsl_required << "1drho0";
      qsl_required << "1ddeltar";
      qsl_required << "1dproberadius";
      qsl_required << "1dthreshold";
      qsl_required << "1dsamplerotations";
      qsl_required << "1daxisrotations";
      qsl_required << "1drotationfile";
      for ( unsigned int i = 0; i < qsl_required.size(); i++ )
      {
         if ( !control_parameters.count( qsl_required[ i ] ) )
         {
            missing_required += " " + qsl_required[ i ];
         }
      }
   }

   if ( !missing_required.isEmpty() )
   {
      errormsg = QString( "Error: Process1d requires prior definition of:%1" )
         .arg( missing_required );
      return false;
   }

   detector_pixels_width         = control_parameters[ "1ddetectorpixelswidth" ].toInt();
   detector_distance             = control_parameters[ "1ddetectordistance"    ].toDouble();
   detector_width                = control_parameters[ "1ddetectorwidth"       ].toDouble() * 1e-3;
   lambda                        = control_parameters[ "1dlambda"              ].toDouble();
   rho0                          = control_parameters[ "1drho0"                ].toDouble();
   deltaR                        = control_parameters[ "1ddeltar"              ].toDouble();
   probe_radius                  = control_parameters[ "1dproberadius"         ].toDouble();
   threshold                     = control_parameters[ "1dthreshold"           ].toDouble();
   sample_rotations              = control_parameters[ "1dsamplerotations"     ].toUInt();
   int axis_rotations            = control_parameters[ "1daxisrotations"       ].toUInt();

   detector_width_per_pixel      = detector_width  / detector_pixels_width;

   map < unsigned int, bool > intermediate_saves;
   if ( control_parameters.count( "1dintermediatesaves" ) )
   {
      QString     qs  = control_parameters[ "1dintermediatesaves" ];
      qs.replace( ",", " " );
      qs.trimmed();
      QStringList qsl = (qs ).split( QRegExp( "\\s+" ) , Qt::SkipEmptyParts );
      for ( unsigned int i = 0; i < ( unsigned int ) qsl.size(); i++ )
      {
         unsigned int pos = npes * ( int ) ( qsl[ i ].toUInt() / npes );
         if ( !pos || pos >= sample_rotations )
         {
            errormsg =
               QString( "1drun: 1dintermediatesaves bad value %1 found at pos %2 in line <%3>\n"
                        "either not a positive natural number or value larger than total sample rotations %4" )
               .arg( pos )
               .arg( i + 1 )
               .arg( qsl[ i ] )
               .arg( sample_rotations )
               ;
            return false;
         }
         intermediate_saves[ pos ] = true;
      }
   }

   double spec_multiplier = 1e0;
   if ( control_parameters.count( "1dspecmult" ) )
   {
      spec_multiplier = control_parameters[ "1dspecmult" ].toDouble();
   }

   QString qs_total_plots = QString( "%1" ).arg( sample_rotations * axis_rotations );

   // compute complex curves, display modulus on 1d array
   // compute for each point on detector
   plot_count = 0;

   bool is_ok = true;

   if ( detector_pixels_width <= 0 )
   {
      errormsg += "Detector pixel count must be positive\n";
      is_ok = false;
   }

   if ( lambda <= 0e0 )
   {
      errormsg +=  "The wavelength must be positive\n";
      is_ok = false;
   }
      
   if ( detector_width <= 0 )
   {
      errormsg +=  "Detector width must be positive\n";
      is_ok = false;
   }

   if ( detector_distance <= 0 )
   {
      errormsg +=  "Detector distance must be positive\n";
      is_ok = false;
   }

   if ( !is_ok )
   {
      return false;
   }

   //    if ( !setup_excluded_volume_map() )
   //    {
   //       errormsg +=  errormsg;
   //       return;
   //    }

   vector < vector < double > > rotations;


   if ( !myrank )
   {
      usui1m_timer.start_timer ( "1d random generation" );
   }

   if ( control_parameters.count( "1drotationsuserand" ) )
   {
      hypercube_rejection_drand_rotations( sample_rotations, rotations );
   } else {
      if ( !load_rotations_mpi( sample_rotations, rotations ) )
      {
         return false;
      }
   }
   if ( !myrank )
   {
      usui1m_timer.end_timer ( "1d random generation" );
   }

   // setup atoms
   QRegExp count_hydrogens("H(\\d)");

   if ( our_saxs_options.iqq_use_atomic_ff )
   {
      cout << "using explicit hydrogens\n";
   }

   vector < saxs_atom > atoms;

   double atomic_scaler = 1e0; // pdbs and Q in Angstrom units
   double atomic_scaler_inv = 1e0; // / atomic_scaler;

   unsigned int current_model = 0;

   QString name = 
      QString( "rank%1_%2_tp%3_rot%4_ar%5_d3R%6_rho0%7_pr%8_t%9" )
      .arg( myrank )
      .arg( model_vector[ current_model ].model_id )
      .arg( sample_rotations * axis_rotations ) 
      .arg( sample_rotations ) 
      .arg( axis_rotations )
      .arg( deltaR ) 
      .arg( rho0 ) 
      .arg( probe_radius ) 
      .arg( threshold ) 
      ;
   cout << QString( "starting 1d for %1\n" ).arg( name );

   {

      double tot_excl_vol      = 0e0;
      double tot_excl_vol_noh  = 0e0;
      unsigned int total_e     = 0;
      unsigned int total_e_noh = 0;

      saxs_atom new_atom;

      for ( unsigned int j = 0; j < model_vector[current_model].molecule.size(); j++ )
      {
         for ( unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size(); k++ )
         {
            PDB_atom *this_atom = &(model_vector[current_model].molecule[j].atom[k]);

            // keep everything in angstroms!
            new_atom.pos[ 0 ] = this_atom->coordinate.axis[ 0 ] * atomic_scaler;
            new_atom.pos[ 1 ] = this_atom->coordinate.axis[ 1 ] * atomic_scaler;
            new_atom.pos[ 2 ] = this_atom->coordinate.axis[ 2 ] * atomic_scaler;

            if ( this_atom->name == "XH" && !our_saxs_options.iqq_use_atomic_ff )
            {
               continue;
            }

            QString use_resname = this_atom->resName;
            use_resname.replace( QRegExp( "_.*$" ), "" );

            QString mapkey = QString("%1|%2")
               .arg( use_resname )
               .arg( this_atom->name );

            if ( this_atom->name == "OXT" )
            {
               mapkey = "OXT|OXT";
            }

            QString hybrid_name = residue_atom_hybrid_map[mapkey];

            if ( hybrid_name.isEmpty() || !hybrid_name.length() )
            {
               noticemsg += 
                  QString("%1Molecule %2 Residue %3 %4 Hybrid name missing. Atom skipped.\n")
                  .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                  .arg(j+1)
                  .arg(use_resname)
                  .arg(this_atom->resSeq )
                  ;
               continue;
            }

            if ( !hybrid_map.count(hybrid_name) )
            {
               noticemsg += 
                  QString("%1Molecule %2 Residue %3 %4 Hybrid %5 name missing from Hybrid file. Atom skipped.\n")
                  .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                  .arg(j+1)
                  .arg(use_resname)
                  .arg(this_atom->resSeq)
                  .arg(hybrid_name)
                  ;
               continue;
            }

            if ( !atom_map.count(this_atom->name + "~" + hybrid_name) )
            {
               noticemsg += 
                  QString("%1Molecule %2 Atom %3 Residue %4 %5 Hybrid %6 name missing from Atom file. Atom skipped.\n")
                  .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                  .arg(j+1)
                  .arg(this_atom->name)
                  .arg(use_resname)
                  .arg(this_atom->resSeq)
                  .arg(hybrid_name)
                  ;
               continue;
            }

            new_atom.excl_vol = atom_map[this_atom->name + "~" + hybrid_name].saxs_excl_vol;

            new_atom.atom_name = this_atom->name;
            new_atom.residue_name = use_resname;

            if ( our_saxs_options.use_somo_ff )
            {
               double this_ev = get_ff_ev( new_atom.residue_name, new_atom.atom_name );
               if ( this_ev )
               {
                  new_atom.excl_vol = this_ev;
               }
            }

            total_e += hybrid_map[ hybrid_name ].num_elect;
            if ( this_atom->name == "OW" && our_saxs_options.swh_excl_vol > 0e0 )
            {
               new_atom.excl_vol = our_saxs_options.swh_excl_vol;
            }
            if ( this_atom->name == "XH" )
            {
               // skip excl vol for now
               new_atom.excl_vol = 0e0;
            }

            if ( our_saxs_options.hybrid_radius_excl_vol )
            {
               new_atom.excl_vol = M_PI * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius;
            }

            if ( our_saxs_options.iqq_use_saxs_excl_vol )
            {
               new_atom.excl_vol = saxs_map[hybrid_map[hybrid_name].saxs_name].volume;
            }

            if ( this_atom->name != "OW" )
            {
               new_atom.excl_vol *= our_saxs_options.scale_excl_vol;
               tot_excl_vol_noh  += new_atom.excl_vol;
               total_e_noh       += hybrid_map[ hybrid_name ].num_elect;
            }

            new_atom.radius = hybrid_map[hybrid_name].radius;
            tot_excl_vol += new_atom.excl_vol;

            new_atom.saxs_name = hybrid_map[hybrid_name].saxs_name; 
            new_atom.hybrid_name = hybrid_name;
            new_atom.hydrogens = 0;
            if ( !our_saxs_options.iqq_use_atomic_ff &&
                 count_hydrogens.indexIn(hybrid_name) != -1 )
            {
               new_atom.hydrogens = count_hydrogens.cap(1).toInt();
            }

            if ( !saxs_map.count(hybrid_map[hybrid_name].saxs_name) )
            {
               noticemsg += 
                  QString("%1Molecule %2 Residue %3 %4 Hybrid %5 Saxs name %6 name missing from SAXS atom file. Atom skipped.\n")
                  .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                  .arg(j+1)
                  .arg(use_resname)
                  .arg(this_atom->resSeq)
                  .arg(hybrid_name)
                  .arg(hybrid_map[hybrid_name].saxs_name)
                  ;
               continue;
            }

            atoms.push_back(new_atom);
         }
      }
      // ok now we have all the atoms
   }

   if ( !atoms.size() )
   {
      errormsg = "Error: no atoms found!\n";
      return false;
   }

   // place 1st atom at 0,0,0

   // save value for excluded volume
   point zerooffset;
   zerooffset.axis[ 0 ] = atoms[ 0 ].pos[ 0 ];
   zerooffset.axis[ 1 ] = atoms[ 0 ].pos[ 1 ];
   zerooffset.axis[ 2 ] = atoms[ 0 ].pos[ 2 ];

   if ( atoms.size() > 1 )
   {
      for ( unsigned int a = 1; a < atoms.size(); a++ )
      {
         atoms[ a ].pos[ 0 ] -= atoms[ 0 ].pos[ 0 ];
         atoms[ a ].pos[ 1 ] -= atoms[ 0 ].pos[ 1 ];
         atoms[ a ].pos[ 2 ] -= atoms[ 0 ].pos[ 2 ];
      }
   }

   atoms[ 0 ].pos[ 0 ] = 0.0f;
   atoms[ 0 ].pos[ 1 ] = 0.0f;
   atoms[ 0 ].pos[ 2 ] = 0.0f;

   vector < point > atom_positions;

   for ( unsigned int a = 0; a < atoms.size(); a++ )
   {
      point this_point;
      this_point.axis[ 0 ] = atoms[ a ].pos[ 0 ] * atomic_scaler_inv;
      this_point.axis[ 1 ] = atoms[ a ].pos[ 1 ] * atomic_scaler_inv;
      this_point.axis[ 2 ] = atoms[ a ].pos[ 2 ] * atomic_scaler_inv;
      atom_positions.push_back( this_point );
   }

   vector < point > transform_from;
   {
      point this_point;
      this_point.axis[ 0 ] = 0.0f;
      this_point.axis[ 1 ] = 0.0f;
      this_point.axis[ 2 ] = 0.0f;
      transform_from.push_back( this_point );
      this_point.axis[ 0 ] = 1.0f;
      this_point.axis[ 1 ] = 0.0f;
      this_point.axis[ 2 ] = 0.0f;
      transform_from.push_back( this_point );
      this_point.axis[ 0 ] = 0.0f;
      this_point.axis[ 1 ] = 1.0f;
      this_point.axis[ 2 ] = 0.0f;
      transform_from.push_back( this_point );
      this_point.axis[ 0 ] = 0.0f;
      this_point.axis[ 1 ] = 0.0f;
      this_point.axis[ 2 ] = 1.0f;
      transform_from.push_back( this_point );
      this_point.axis[ 0 ] = -1.0f;
      this_point.axis[ 1 ] = 0.0f;
      this_point.axis[ 2 ] = 0.0f;
      transform_from.push_back( this_point );
      this_point.axis[ 0 ] = 0.0f;
      this_point.axis[ 1 ] = -1.0f;
      this_point.axis[ 2 ] = 0.0f;
      transform_from.push_back( this_point );
      this_point.axis[ 0 ] = 0.0f;
      this_point.axis[ 1 ] = 0.0f;
      this_point.axis[ 2 ] = -1.0f;
      transform_from.push_back( this_point );
   }
   vector < point > transform_to = transform_from;

   saxs saxsH = saxs_map["H"];
   double one_over_4pi   = 1.0 / (4.0 * M_PI);
   double one_over_4pi_2 = one_over_4pi * one_over_4pi;

   total_modulii.resize( detector_pixels_width );
   for ( int i = 0; i < ( int ) total_modulii.size(); i++ )
   {
      total_modulii[ i ] = 0e0;
   }

   double deltapsi = 2.0 * M_PI / ( double ) axis_rotations;

   for ( unsigned int r = 0; r < rotations.size(); r++ )
   {
      if ( !( r % 200 ) )
      {
         cout << QString( "processing rotation %1 of %2\n" ).arg( r + 1 ).arg( rotations.size() );
      }
      if ( !get_excluded_volume_map() )
      {
         return false;
      }

      for ( unsigned int i = 0; i < ( unsigned int )excluded_volume.size(); i++ )
      {
         excluded_volume[ i ].axis[ 0 ] -= zerooffset.axis[ 0 ];
         excluded_volume[ i ].axis[ 1 ] -= zerooffset.axis[ 1 ];
         excluded_volume[ i ].axis[ 2 ] -= zerooffset.axis[ 2 ];
      }


      //       data.resize( detector_pixels_width );
      //       for ( int i = 0; i < ( int ) data.size(); i++ )
      //       {
      //          data[ i ] = complex < double > ( 0.0, 0.0 );
      //       }

      if ( rotations.size() > 1 )
      {
         transform_to[ 1 ].axis[ 0 ] = rotations[ r ][ 0 ];
         transform_to[ 1 ].axis[ 1 ] = rotations[ r ][ 1 ];
         transform_to[ 1 ].axis[ 2 ] = rotations[ r ][ 2 ];
         
         transform_to[ 2 ].axis[ 0 ] = -rotations[ r ][ 1 ];
         transform_to[ 2 ].axis[ 1 ] = rotations[ r ][ 0 ];
         // check this!
         transform_to[ 2 ].axis[ 2 ] = 0.0f; // rotations[ r ][ 2 ];

         transform_to[ 3 ] = 
            normal( cross( transform_to[ 1 ], transform_to[ 2 ] ) );

         transform_to[ 4 ].axis[ 0 ] = -transform_to[ 1 ].axis[ 0 ];
         transform_to[ 4 ].axis[ 1 ] = -transform_to[ 1 ].axis[ 1 ];
         transform_to[ 4 ].axis[ 2 ] = -transform_to[ 1 ].axis[ 2 ];

         transform_to[ 5 ].axis[ 0 ] = -transform_to[ 2 ].axis[ 0 ];
         transform_to[ 5 ].axis[ 1 ] = -transform_to[ 2 ].axis[ 1 ];
         transform_to[ 5 ].axis[ 2 ] = -transform_to[ 2 ].axis[ 2 ];

         transform_to[ 6 ].axis[ 0 ] = -transform_to[ 3 ].axis[ 0 ];
         transform_to[ 6 ].axis[ 1 ] = -transform_to[ 3 ].axis[ 1 ];
         transform_to[ 6 ].axis[ 2 ] = -transform_to[ 3 ].axis[ 2 ];

         vector < point > result;

         if ( !atom_align (
                           transform_from, 
                           transform_to, 
                           atom_positions,
                           result ) )
         {
            errormsg = "alignment failure";
            return false;
         }

         for ( unsigned int a = 0; a < atoms.size(); a++ )
         {
            atoms[ a ].pos[ 0 ] = result[ a ].axis[ 0 ] * atomic_scaler;
            atoms[ a ].pos[ 1 ] = result[ a ].axis[ 1 ] * atomic_scaler;
            atoms[ a ].pos[ 2 ] = result[ a ].axis[ 2 ] * atomic_scaler;
         }
         
         // and rotate excluded volume (this may have to be done piecemeal to save memory

         if ( !atom_align (
                           transform_from, 
                           transform_to, 
                           excluded_volume,
                           result 
                           ) )
         {
            errormsg = "alignment 2 failure";
            return false;
         }

         excluded_volume = result;
         result.clear( );
      }

      for ( int t = 0; t < axis_rotations; t++ )
      {

         double psi = ( double ) t * deltapsi;
         double cospsi = cos( psi );
         double sinpsi = sin( psi );

         s1d_data.resize( detector_pixels_width );
         for ( int i = 0; i < ( int ) s1d_data.size(); i++ )
         {
            s1d_data[ i ] = complex < double > ( 0.0, 0.0 );
         }
         // for each atom, compute scattering factor for each element on the detector


         for ( unsigned int a = 0; a < atoms.size(); a++ )
         {
            for ( unsigned int i = 0; i < s1d_data.size(); i++ )
            {
               double pixpos = ( double ) i * detector_width_per_pixel;
            
               double S_length = sqrt( detector_distance * detector_distance + pixpos * pixpos );
            
               vector < double > Q( 3 );
               Q[ 0 ] = 2.0 * M_PI * ( ( pixpos / S_length ) / lambda );
               Q[ 1 ] = 2.0 * M_PI * ( ( ( detector_distance / S_length ) - 1e0 ) / lambda );
               Q[ 2 ] = 0e0;
               
               vector < double > Rv( 3 );
               Rv[ 0 ] = (cospsi + (1.0 - cospsi) * rotations[ r ][ 0 ] * rotations[ r ][ 0 ]) * ( double ) atoms[ a ].pos[ 0 ];
               Rv[ 0 ] += ((1.0 - cospsi) * rotations[ r ][ 0 ] * rotations[ r ][ 1 ] - rotations[ r ][ 2 ] * sinpsi) * ( double ) atoms[ a ].pos[ 1 ];
               Rv[ 0 ] += ((1.0 - cospsi) * rotations[ r ][ 0 ] * rotations[ r ][ 2 ] + rotations[ r ][ 1 ] * sinpsi) * ( double ) atoms[ a ].pos[ 2 ];

               Rv[ 1 ] = ((1.0 - cospsi) * rotations[ r ][ 0 ] * rotations[ r ][ 1 ] + rotations[ r ][ 2 ] * sinpsi) * ( double ) atoms[ a ].pos[ 0 ];
               Rv[ 1 ] += (cospsi + (1.0 - cospsi) * rotations[ r ][ 1 ] * rotations[ r ][ 1 ]) * ( double ) atoms[ a ].pos[ 1 ];
               Rv[ 1 ] += ((1.0 - cospsi) * rotations[ r ][ 1 ] * rotations[ r ][ 2 ] - rotations[ r ][ 0 ] * sinpsi) * ( double ) atoms[ a ].pos[ 2 ];

               Rv[ 2 ] = ((1.0 - cospsi) * rotations[ r ][ 0 ] * rotations[ r ][ 2 ] - rotations[ r ][ 1 ] * sinpsi) * ( double ) atoms[ a ].pos[ 0 ];
               Rv[ 2 ] += ((1.0 - cospsi) * rotations[ r ][ 1 ] * rotations[ r ][ 2 ] + rotations[ r ][ 0 ] * sinpsi) * ( double ) atoms[ a ].pos[ 1 ];
               Rv[ 2 ] += (cospsi + (1.0 - cospsi) * rotations[ r ][ 2 ] * rotations[ r ][ 2 ]) * ( double ) atoms[ a ].pos[ 2 ];

               //                vector < double > Rv( 3 );
               //                Rv[ 0 ] = ( double ) atoms[ a ].pos[ 0 ];
               //                Rv[ 1 ] = ( double ) atoms[ a ].pos[ 1 ];
               //                Rv[ 2 ] = ( double ) atoms[ a ].pos[ 2 ];
               
               double QdotR = 
                  Q[ 0 ] * Rv[ 0 ] +
                  Q[ 1 ] * Rv[ 1 ] +
                  Q[ 2 ] * Rv[ 2 ];
               
               complex < double > iQdotR = complex < double > ( 0e0, -QdotR );
            
               complex < double > expiQdotR = exp( iQdotR );
               
               // F_atomic
               
               saxs saxs = saxs_map[ atoms[ a ].saxs_name ];
               
               double q = sqrt( Q[ 0 ] * Q[ 0 ] + Q[ 1 ] * Q[ 1 ] + Q[ 2 ] * Q[ 2 ] );

               double q_2_over_4pi = q * q * one_over_4pi_2;

               double F_at =
                  compute_ff( saxs,
                              saxsH,
                              atoms[ a ].residue_name,
                              atoms[ a ].saxs_name,
                              atoms[ a ].atom_name,
                              atoms[ a ].hydrogens,
                              q,
                              q_2_over_4pi );
            
               s1d_data[ i ] += complex < double > ( F_at, 0e0 ) * expiQdotR;
            }
         }

         // now subtract excluded volume

         if ( rho0 )
         {
            for ( unsigned int i = 0; i < s1d_data.size(); i++ )
            {
               double pixpos = ( double ) i * detector_width_per_pixel;

               double S_length = sqrt( detector_distance * detector_distance + pixpos * pixpos );

               vector < double > Q( 3 );
               Q[ 0 ] = 2.0 * M_PI * ( ( pixpos / S_length ) / lambda );
               Q[ 1 ] = 2.0 * M_PI * ( ( ( detector_distance / S_length ) - 1e0 ) / lambda );
               Q[ 2 ] = 0e0;
               
               for ( unsigned int j = 0; j < ( unsigned int )excluded_volume.size(); j++ )
               {
                  vector < double > Rv( 3 );
                  Rv[ 0 ] = (cospsi + (1.0 - cospsi) * rotations[ r ][ 0 ] * rotations[ r ][ 0 ]) * ( double ) excluded_volume[ j ].axis[ 0 ];
                  Rv[ 0 ] += ((1.0 - cospsi) * rotations[ r ][ 0 ] * rotations[ r ][ 1 ] - rotations[ r ][ 2 ] * sinpsi) * ( double ) excluded_volume[ j ].axis[ 1 ];
                  Rv[ 0 ] += ((1.0 - cospsi) * rotations[ r ][ 0 ] * rotations[ r ][ 2 ] + rotations[ r ][ 1 ] * sinpsi) * ( double ) excluded_volume[ j ].axis[ 2 ];

                  Rv[ 1 ] = ((1.0 - cospsi) * rotations[ r ][ 0 ] * rotations[ r ][ 1 ] + rotations[ r ][ 2 ] * sinpsi) * ( double ) excluded_volume[ j ].axis[ 0 ];
                  Rv[ 1 ] += (cospsi + (1.0 - cospsi) * rotations[ r ][ 1 ] * rotations[ r ][ 1 ]) * ( double ) excluded_volume[ j ].axis[ 1 ];
                  Rv[ 1 ] += ((1.0 - cospsi) * rotations[ r ][ 1 ] * rotations[ r ][ 2 ] - rotations[ r ][ 0 ] * sinpsi) * ( double ) excluded_volume[ j ].axis[ 2 ];

                  Rv[ 2 ] = ((1.0 - cospsi) * rotations[ r ][ 0 ] * rotations[ r ][ 2 ] - rotations[ r ][ 1 ] * sinpsi) * ( double ) excluded_volume[ j ].axis[ 0 ];
                  Rv[ 2 ] += ((1.0 - cospsi) * rotations[ r ][ 1 ] * rotations[ r ][ 2 ] + rotations[ r ][ 0 ] * sinpsi) * ( double ) excluded_volume[ j ].axis[ 1 ];
                  Rv[ 2 ] += (cospsi + (1.0 - cospsi) * rotations[ r ][ 2 ] * rotations[ r ][ 2 ]) * ( double ) excluded_volume[ j ].axis[ 2 ];

                  double QdotR = 
                     Q[ 0 ] * Rv[ 0 ] +
                     Q[ 1 ] * Rv[ 1 ] +
                     Q[ 2 ] * Rv[ 2 ];

                  QdotR *= spec_multiplier;

                  complex < double > iQdotR = complex < double > ( 0e0, -QdotR );

                  complex < double > expiQdotR = exp( iQdotR );

                  complex < double > rho0expiQdotR = complex < double > ( rho0, 0e0 ) * expiQdotR;

                  s1d_data[ i ] -= rho0expiQdotR * complex < double > ( deltaR * deltaR * deltaR, 0 );
               }
            }
         }

         if ( !update_image() )
         {
            return false;
         }
         if ( intermediate_saves.count( npes * ( r + 1 ) ) && t == axis_rotations - 1 )
         {
            vector < double > q( detector_pixels_width );
            vector < double > I = total_modulii;
            for ( int i = 0; i < ( int ) q.size(); i++ )
            {
               q[ i ] = q_of_pixel( i );
               I[ i ] /= ( double ) plot_count;
            }

            QString qs_plot_count = QString( "%1" ).arg( plot_count );
            while ( qs_plot_count.length() < qs_total_plots.length() )
            {
               qs_plot_count = "0" + qs_plot_count;
            }

            QString name = 
               QString( "rank%1_%2_tp%3_rot%4_ar%5_d3R%6_rho0%7_pr%8_t%9" )
               .arg( myrank )
               .arg( model_vector[ current_model ].model_id )
               .arg( qs_plot_count ) 
               .arg( npes * ( r + 1 ) )
               .arg( axis_rotations ) 
               .arg( deltaR ) 
               .arg( rho0 ) 
               .arg( probe_radius ) 
               .arg( threshold ) 
               ;

            if ( !write_output( name, q, I ) )
            {
               return false;
            }
         }         
      } // axis rotations
   } // rotations


   // save the data

   vector < double > q( detector_pixels_width );
   vector < double > I = total_modulii;
   for ( int i = 0; i < ( int ) q.size(); i++ )
   {
      q[ i ] = q_of_pixel( i );
      I[ i ] /= ( double ) plot_count;
   }

   if ( !write_output( name, q, I ) )
   {
      return false;
   }
   return true;
}

void US_Saxs_Util::hypercube_rejection_drand_rotations( 
                                                       unsigned int number, 
                                                       vector < vector < double > > &rotations )
{
   cout << QString("%1: initial hypercube rejection random rotations\n" ).arg( myrank ) << flush;
   if ( MPI_SUCCESS != MPI_Barrier( MPI_COMM_WORLD ) )
   {
      MPI_Abort( MPI_COMM_WORLD, -9999 );
      exit( -9999 );
   }         

   int proc_rots  = ( int ) number / npes + 1;
   int proc_rots3 = 3 * proc_rots;

   vector < double > send_recv_rotations;

   if ( !myrank )
   {
      struct timeval tv;
      gettimeofday(&tv, NULL);
      srand48( tv.tv_usec + myrank );
   
      // generate rotations for the other processes
      for ( int i = 1; i < npes; i++ )
      {
         send_recv_rotations.clear( );
         double x;
         double y;
         double z;
         
         while ( ( int ) send_recv_rotations.size() < proc_rots3 )
         {
            x = drand48() * 2.0 - 1.0;
            y = drand48() * 2.0 - 1.0;
            z = drand48() * 2.0 - 1.0;
         
            double mag = sqrt( x * x + y * y + z * z );
            if ( mag && mag < 1.0 )
            {
               x /= mag;
               y /= mag;
               z /= mag;
               send_recv_rotations.push_back( x );
               send_recv_rotations.push_back( y );
               send_recv_rotations.push_back( z );
            }
         }
         // send them
         if ( MPI_SUCCESS != MPI_Send( ( void * ) &send_recv_rotations[ 0 ],
                                       proc_rots3,
                                       MPI_DOUBLE,
                                       i,
                                       i,
                                       MPI_COMM_WORLD ) )
         {
            MPI_Abort( MPI_COMM_WORLD, -10000 - myrank );
            exit( -10000 );
         }         
      }
      // and make up ours:
      rotations.clear( );
      vector < double > p( 3 );
      while ( ( int ) rotations.size() < proc_rots )
      {
         p[ 0 ] = drand48() * 2.0 - 1.0;
         p[ 1 ] = drand48() * 2.0 - 1.0;
         p[ 2 ] = drand48() * 2.0 - 1.0;
         
         double mag = sqrt( p[ 0 ] * p[ 0 ] + p[ 1 ] * p[ 1 ] + p[ 2 ] * p[ 2 ] );
         if ( mag && mag < 1.0 )
         {
            p[ 0 ] /= mag;
            p[ 1 ] /= mag;
            p[ 2 ] /= mag;
            rotations.push_back( p );
         }
      }
   } else {
      // recv them
      MPI_Status mpi_status;
      send_recv_rotations.resize( proc_rots3 );
      if ( MPI_SUCCESS != MPI_Recv( ( void * ) &send_recv_rotations[ 0 ],
                                    proc_rots3,
                                    MPI_DOUBLE,
                                    0, 
                                    myrank,
                                    MPI_COMM_WORLD,
                                    &mpi_status ) )
      {
         MPI_Abort( MPI_COMM_WORLD, -10000 - myrank );
         exit( -10000 - myrank );
      } 
      rotations.clear( );
      vector < double > p( 3 );
      for ( int i = 0; i < proc_rots3; i+=3 )
      {
         p[ 0 ] = send_recv_rotations[ i     ];
         p[ 1 ] = send_recv_rotations[ i + 1 ];
         p[ 2 ] = send_recv_rotations[ i + 2 ];
         rotations.push_back( p );
      }
   }
   cout << QString( "%1: my rotations size %2\n" ).arg( myrank ).arg( rotations.size() ) << flush;
   return;
}

bool US_Saxs_Util::load_rotations_mpi( unsigned int number, vector < vector < double > > &rotations )
{
   QFile f( control_parameters[ "1drotationfile" ] );
   if ( !f.exists() )
   {
      errormsg = QString( "Notice: cached rotations file %1 does not exist" )
         .arg( f.fileName() );
      return false;
   }

   if ( !f.open( QIODevice::ReadOnly ) )
   {
      errormsg = QString( "Notice: found cached rotations file %1 but could not open it" )
         .arg( f.fileName() );
      return false;
   }

   QTextStream ts( &f );

   unsigned int line = 0;

   vector < double > p(3);
   rotations.clear( );

   while ( !ts.atEnd() && line < number )
   {
      QString     qs  = ts.readLine();
      line++;

      QStringList qsl = (qs ).split( QRegExp( "\\s+" ) , Qt::SkipEmptyParts );

      if ( qsl.size() != 3 )
      {
         errormsg =  QString( "Notice: error in found cached rotations file %1 line %2, does not contain 3 tokens" )
            .arg( f.fileName() )
            .arg( line )
            ;
         f.close();
         return false;
      }
      if ( ( int ) ( ( line - 1 ) % npes ) == myrank )
      {
         //3cout << QString( "%1: load_rotations took line %2\n" ).arg( myrank ).arg( line );
         p[ 0 ] = qsl[ 0 ].toDouble();
         p[ 1 ] = qsl[ 1 ].toDouble();
         p[ 2 ] = qsl[ 2 ].toDouble();
         rotations.push_back( p );
      }
   }
   f.close();
   if ( line < number )
   {
      errormsg = 
         QString( "Notice: error:  cached rotations file %1 line %2, does not contain sufficient rotations (%3 requested vs %4 found)" )
         .arg( f.fileName() )
         .arg( line )
         .arg( number )
         .arg( rotations.size() )
         ;
      rotations.clear( );
      return false;
   }
   // cout << QString( "%1: load_rotations: rotation.size() %2\n" ).arg( myrank).arg( rotations.size() );
   return true;
}
