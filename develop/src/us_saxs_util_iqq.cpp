#include "../include/us_saxs_util.h"
#include "../include/us_revision.h"

bool US_Saxs_Util::read_control( QString controlfile )
{
   output_files          .clear();
   saxs_inputfile_for_csv.clear();
   saxs_model_for_csv    .clear();
   saxs_method_for_csv   .clear();
   saxs_q_for_csv        .clear();
   saxs_I_for_csv        .clear();
   write_output_count     = 0;
   
   env_ultrascan = getenv("ULTRASCAN");
   cout << "$ULTRASCAN = " << env_ultrascan << endl;

   QFile f( controlfile );
   errormsg = "";
   if ( !f.exists() )
   {
      errormsg = QString( "Error: %1 file does not exist" ).arg( controlfile );
      return false;
   }

   if ( !f.open( IO_ReadOnly ) )
   {
      errormsg = QString( "Error: %1 can not be opened.  Check permissions" ).arg( controlfile );
      return false;
   }

   if ( controlfile.contains( QRegExp( "\\.(tgz|TGZ)$" ) ) )
   {
      f.close();
      US_Gzip usg;
      // rename
      QString dest = controlfile;
      dest.replace( QRegExp( "\\.(tgz|TGZ)$" ), ".tar.gz" );
      QDir qd;
      qd.remove( dest );
      
      if ( !qd.rename( controlfile, dest ) )
      {
         errormsg = QString("Error: renaming %1 to %2 ").arg( controlfile ).arg( dest );
         return false;
      }
         
      controlfile = dest;
      
      qd.remove( dest.replace( QRegExp("\\.(gz|GZ)$"), "" ) );
      cout << "controlfile sent to gunzip: " << controlfile << endl;
      int result = usg.gunzip( controlfile );
      cout << "last_written_name from gunzip: " << usg.last_written_name << endl;
      if ( GZIP_OK != result )
      {
         errormsg = QString("Error: %1 problem ungzipping (%2)").arg( controlfile ).arg( usg.explain( result ) );
         return false;
      }

      controlfile = usg.last_written_name;

      cout << "controlfile is now " << controlfile << endl;
      controlfile = QFileInfo( controlfile ).fileName();
      f.setName( controlfile );
      if ( !f.open( IO_ReadOnly ) )
      {
         errormsg = QString( "Error: %1 can not be opened.  Check permissions" ).arg( controlfile );
         return false;
      }
   }

   if ( controlfile.contains( QRegExp( "\\.(tar|TAR)$" ) ) )
   {
      f.close();
      cout << "extracting tar archive\n";
      US_Tar ust;
      QStringList qslt;
      int result = ust.list( controlfile, qslt, true );
      if ( TAR_OK != result )
      {
         errormsg = QString("Error: %1 problem extracting tar archive (%2)").arg( controlfile ).arg( result );
         return false;
      }
      result = ust.extract( controlfile );
      if ( TAR_OK != result )
      {
         errormsg = QString("Error: %1 problem extracting tar archive (%2)").arg( controlfile ).arg( result );
         return false;
      }
      if ( !qslt.size() )
      {
         errormsg = QString("Error: tar archive %1 appears empty").arg( controlfile );
         return false;
      }
         
      cout << QString( "from tar file %1, new control file %2\n" ).arg( controlfile ).arg( qslt[ 0 ] );
      controlfile = qslt[ 0 ];

      f.setName( controlfile );
      if ( !f.open( IO_ReadOnly ) )
      {
         errormsg = QString( "Error: %1 can not be opened.  Check permissions" ).arg( controlfile );
         return false;
      }
   }

   // read and setup control

   QTextStream ts( &f );
   QRegExp rx_blank  ( "^\\s*$" );
   QRegExp rx_comment( "#.*$" );
   QRegExp rx_valid  ( 
                       "^("
                       "residuefile|"
                       // "hydrationfile|"
                       "atomfile|"
                       "hybridfile|"
                       "saxsfile|"
                       "saxs|"
                       "iqmethod|"
                       "fdbinsize|"
                       "fdmodulation|"
                       "hypoints|"
                       "crysolharm|"
                       "crysolgrid|"
                       "crysolchs|"
                       "wateredensity|"
                       "startq|"
                       "endq|"
                       "deltaq|"
                       "pdballmodels|"
                       "experimentgrid|"
                       "inputfile|"
                       "output|"
                       "outputfile|"
                       "process|"
                       "taroutput|"
                       "tgzoutput|"
                       "remark)$"
                       );

   QRegExp rx_file   ( 
                      "^("
                      "residuefile|"
                      // "hydrationfile|"
                      "atomfile|"
                      "hybridfile|"
                      "saxsfile|"
                      "experimentgrid|"
                      "inputfile)$"
                      );

   QRegExp rx_arg_1  ( 
                      "^("
                      "residuefile|"
                      // "hydrationfile|"
                      "atomfile|"
                      "hybridfile|"
                      "saxsfile|"
                      "iqmethod|"
                      "fdbinsize|"
                      "fdmodulation|"
                      "hypoints|"
                      "crysolharm|"
                      "crysolgrid|"
                      "crysolchs|"
                      "wateredensity|"
                      "startq|"
                      "endq|"
                      "deltaq|"
                      "experimentgrid|"
                      "inputfile|"
                      "output|"
                      "taroutput|"
                      "tgzoutput|"
                      "outputfile)$"
                      );

   QRegExp rx_valid_saxs_iqmethod (
                                   "^("
                                   "db|"
                                   "hy|"
                                   "hya|"
                                   "h2|"
                                   "h2a|"
                                   "h3|"
                                   "h3a|"
                                   "foxs|"
                                   "crysol|"
                                   "fd)$"
                                   );

   QRegExp rx_flush  ( 
                      "^("
                      "experimentgrid|"
                      "startq|"
                      "endq|"
                      "deltaq)$"
                      );

   unsigned int line = 0;

   while ( !ts.atEnd() )
   {
      QString qs = ts.readLine().replace( rx_comment, "" ).replace( "^\\s+", "" ).replace( "\\s+$", "" );

      line++;

      if ( qs.contains( rx_blank ) )
      {
         continue;
      }

      QStringList qsl = QStringList::split( QRegExp("\\s+"), qs );

      if ( !qsl.size() )
      {
         continue;
      }

      if ( rx_valid.search( qsl[ 0 ].lower() ) == -1 )
      {
         errormsg = QString( "Error reading %1 line %2 : Unrecognized token %3" )
            .arg( controlfile )
            .arg( line )
            .arg( qsl[ 0 ] );
         return false;
      }

      QString option = qsl[ 0 ].lower();
      qsl.pop_front();
      control_parameters[ option ] = qsl.join(" ");

      if ( rx_arg_1.search( option ) != -1 && 
           qsl.size() < 1 )
      {
         errormsg = QString( "Error reading %1 line %2 : Missing argument " )
            .arg( controlfile )
            .arg( line );
         return false;
      }

      if ( rx_file.search( option ) != -1 )
      {
         QFile qfc( qsl[ 0 ] );
         if ( !qfc.exists() )
         {
            errormsg = QString( "Error reading %1 line %2 : File %3 does not exist " )
               .arg( controlfile )
               .arg( line )
               .arg( qsl[ 0 ] );
            return false;
         }
      }         

      if ( rx_flush.search( option ) != -1 )
      {
         if ( !flush_output() )
         {
            return false;
         }
      }         

      if ( option == "residuefile" )
      {
         cout << QString("read residue %1\n").arg( qsl[ 0 ] );
         if ( !select_residue_file( qsl[ 0 ] ) )
         {
            errormsg = QString( "Error %1 line %2 : %3" )
               .arg( controlfile )
               .arg( line )
               .arg( errormsg );
            return false;
         }
         if ( !noticemsg.isEmpty() )
         {
            cout << noticemsg;
         }
      }         

      if ( option == "atomfile" )
      {
         cout << QString("read atom %1\n").arg( qsl[ 0 ] );
         if ( !select_atom_file( qsl[ 0 ] ) )
         {
            errormsg = QString( "Error %1 line %2 : %3" )
               .arg( controlfile )
               .arg( line )
               .arg( errormsg );
            return false;
         }
      }         

      if ( option == "hybridfile" )
      {
         cout << QString("read hybrid %1\n").arg( qsl[ 0 ] );
         if ( !select_hybrid_file( qsl[ 0 ] ) )
         {
            errormsg = QString( "Error %1 line %2 : %3" )
               .arg( controlfile )
               .arg( line )
               .arg( errormsg );
            return false;
         }
      }         

      if ( option == "saxsfile" )
      {
         cout << QString("read saxs atom %1\n").arg( qsl[ 0 ] );
         if ( !select_saxs_file( qsl[ 0 ] ) )
         {
            errormsg = QString( "Error %1 line %2 : %3" )
               .arg( controlfile )
               .arg( line )
               .arg( errormsg );
            return false;
         }
      }

      if ( option == "iqmethod" )
      {
         if ( rx_valid_saxs_iqmethod.search( qsl[ 0 ] ) == -1 )
         {
            errormsg = QString( "Error %1 line %2 : invalid %3 %4" )
               .arg( controlfile )
               .arg( line )
               .arg( option )
               .arg( qsl[ 0 ] );
            return false;
         }
      }            

      if ( option == "experimentgrid" )
      {
         if ( !set_control_parameters_from_experiment_file( qsl[ 0 ] ) )
         {
            return false;
         }
      }            



      if ( option == "inputfile" )
      {
         // read pdb, needs residue file
         QString ext = QFileInfo( qsl[ 0 ] ).extension( false ).lower();

         if ( ext != "pdb" )
         {
            errormsg = QString( "Error %1 line %2 : only PDB files currently supported <%3>" )
               .arg( controlfile )
               .arg( line )
               .arg( qsl[ 0 ] );
            return false;
         }

         if ( !read_pdb( qsl[ 0 ] ) )
         {
            return false;
         }
         if ( !noticemsg.isEmpty() )
         {
            cout << noticemsg;
         }
         if ( model_vector.size() > 1 &&
              !control_parameters.count( "pdballmodels" ) )
         {
            cout << "Notice: an NMR style model was loaded, but \"PDBAllModels\" was not selected, so only the first model will be loaded\n";
            model_vector.resize( 1 );
            model_vector_as_loaded = model_vector;
         }
      }

      if ( option == "taroutput" )
      {
         flush_output();
         if ( !output_files.size() )
         {
            errormsg = QString( "Error %1 line %2 : no output files available to collect in tar file %3" )
               .arg( controlfile )
               .arg( line )
               .arg( qsl[ 0 ] );
            return false;
         }
         if ( !create_tar_output( qsl[ 0 ] ) )
         {
            return false;
         }
      }

      if ( option == "tgzoutput" )
      {
         flush_output();
         if ( !output_files.size() )
         {
            errormsg = QString( "Error %1 line %2 : no output files available to collect in gzipped tar file %3" )
               .arg( controlfile )
               .arg( line )
               .arg( qsl[ 0 ] );
            return false;
         }
         if ( !create_tgz_output( qsl[ 0 ] ) )
         {
            return false;
         }
      }

      if ( option == "process" )
      {
         if ( !validate_control_parameters() )
         {
            errormsg = QString( "Error %1 line %2 : %3" )
               .arg( controlfile )
               .arg( line )
               .arg( errormsg );
            return false;
         }
         setup_saxs_options();
         if ( !run_iqq() )
         {
            return false;
         } 
         if ( !noticemsg.isEmpty() )
         {
            cout << noticemsg;
         }
      }
   }
    
   f.close();
   flush_output();
   return true;
}

bool US_Saxs_Util::set_control_parameters_from_experiment_file( QString filename )
{
   errormsg = "";
   QFile f( filename );
   if ( !f.exists() )
   {
      errormsg = QString("Error: %1 does not exist").arg( filename );
      return false;
   }
   
   QString ext = QFileInfo( filename ).extension( false ).lower();

   QRegExp rx_valid_ext (
                         "^("
                         "csv|"
                         "dat|"
                         "int|"
                         // "out|"
                         "ssaxs)$" );

   if ( rx_valid_ext.search( ext ) == -1 )
   {
      errormsg = QString("Error: %1 unsupported file extension %2").arg( filename ).arg( ext );
      return false;
   }
      
   if ( !f.open( IO_ReadOnly ) )
   {
      errormsg = QString("Error: can not open %1, check permissions ").arg( filename );
      return false;
   }

   QTextStream ts(&f);
   vector < QString > qv;
   QStringList qsl;
   while ( !ts.atEnd() )
   {
      QString qs = ts.readLine();
      qv.push_back( qs );
      qsl << qs;
   }
   f.close();

   if ( !qv.size() )
   {
      errormsg = QString("Error: the file %1 is empty ").arg( filename );
      return false;
   }

   vector < double > q;

   if ( ext == "csv" )
   {
      QStringList qsl_headers = qsl.grep("\"Name\",\"Type; q:\"");
      if ( qsl_headers.size() == 0 )
      {
         errormsg = QString("Error: the file %1 has no valid q grid lines").arg( filename );
         return false;
      }
      QStringList qsl_q = QStringList::split( ",", qsl_headers[ 0 ], true );
      if ( qsl_q.size() < 3 )
      {
         errormsg = QString("Error: the file %1 q grid line has insufficient columns").arg( filename );
         return false;
      }

      q.push_back(qsl_q[2].toDouble());

      QStringList::iterator it = qsl_q.begin();
      it += 3;
      for ( ; it != qsl_q.end(); it++ )
      {
         if ( (*it).toDouble() > q[q.size() - 1] )
         {
            q.push_back((*it).toDouble());
         } else {
            break;
         }
      }

      if ( q.size() < 2 )
      {
         errormsg = QString("Error: the file %1 q grid line has insufficient columns").arg( filename );
         return false;
      }

   } else {
      QRegExp rx_ok_line("^(\\s+|\\d+|\\.|\\d(E|e)(\\+|-|\\d))+$");
      rx_ok_line.setMinimal( true );
      for ( unsigned int i = 1; i < (unsigned int) qv.size(); i++ )
      {
         if ( qv[i].contains(QRegExp("^#")) ||
              rx_ok_line.search( qv[i] ) == -1 )
         {
            continue;
         }
         
         QStringList tokens = QStringList::split(QRegExp("\\s+"), qv[i].replace(QRegExp("^\\s+"),""));

         if ( tokens.size() > 1 )
         {
            double this_q = tokens[ 0 ].toDouble();
            if ( q.size() && this_q <= q[ q.size() - 1 ] )
            {
               cout << QString(" breaking %1 %2\n").arg( this_q ).arg( q[ q.size() - 1 ] );
               break;
            }
            q.push_back( this_q );
         }
      }

      if ( q.size() < 2 )
      {
         errormsg = QString("Error: file %1 error trying to get q grid").arg( filename );
         return false;
      }
   }

   control_parameters[ "startq" ] = QString("%1").arg( q[ 0 ] );
   control_parameters[ "endq" ]   = QString("%1").arg( q[ q.size() - 1 ] );
   control_parameters[ "deltaq" ] = QString("%1").arg( q[ 1 ] - q[ 0 ] );

   if ( control_parameters[ "deltaq" ].toDouble() <= 0e0 )
   {
      errormsg = QString("Error: the file %1 q grid apparent deltaq is zero or negative %2").arg( filename ).arg( control_parameters[ "deltaq" ] );
      return false;
   }

   cout << QString("Grid from file %1 q(%2:%3) deltaq %4\n")
      .arg( filename )
      .arg( control_parameters[ "startq" ] )
      .arg( control_parameters[ "endq" ] )
      .arg( control_parameters[ "deltaq" ] );
   return true;
}

void US_Saxs_Util::validate_control_parameters_set_one( QStringList &checks, 
                                                        QStringList &vals )
{
   for ( unsigned int i = 0; i < checks.size(); i++ )
   {
      if ( !control_parameters.count( checks[ i ] ) )
      {
         control_parameters[ checks[ i ] ] = vals [ i ];
         cout << QString("Notice: %1 set to default of %2\n").arg( checks[ i ] ).arg( vals[ i ] );
      }
   }

   checks.clear();
   vals  .clear();
}


bool US_Saxs_Util::validate_control_parameters()
{
   errormsg = "";
   noticemsg = "";

   // first check required parameters

   QString missing_required;

   QStringList qsl_required;

   // we don't need all of these for external crysol & foxs (fix later)
   {
      qsl_required << "residuefile";
      qsl_required << "atomfile";
      qsl_required << "hybridfile";
      qsl_required << "saxsfile";
      qsl_required << "startq";
      qsl_required << "endq";
      qsl_required << "deltaq";
      qsl_required << "iqmethod";
      qsl_required << "inputfile";
      qsl_required << "output";
      qsl_required << "outputfile";
      
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
      errormsg = QString( "Error: Process requires prior definition of:%1" )
         .arg( missing_required );
      return false;
   }

   // if certain parameters are not set, set them to defaults
   QStringList checks;
   QStringList vals;

   {
      checks << "wateredensity";
      vals   << "0.334";
      validate_control_parameters_set_one( checks, vals );
   }

   QRegExp rx_fd_params(
                        "^("
                        "hy|"
                        "hya|"
                        "h2|"
                        "h2a|"
                        "h3|"
                        "h3a|"
                        "fd)$" );

   if ( rx_fd_params.search( control_parameters[ "iqmethod" ] ) != -1 )
   {
      checks << "fdbinsize";
      vals   << "0.5";
      checks << "fdmodulation";
      vals   << "0.23";

      validate_control_parameters_set_one( checks, vals );
   }

   QRegExp rx_hy_params(
                        "^("
                        "hy|"
                        "hya|"
                        "h2|"
                        "h2a|"
                        "h3|"
                        "h3a)$"
                        );
                        
   if ( rx_hy_params.search( control_parameters[ "iqmethod" ] ) != -1 )
   {
      checks << "hypoints";
      vals   << "15";

      validate_control_parameters_set_one( checks, vals );
   }

   QRegExp rx_crysol_params(
                            "^("
                            "crysol)$"
                            );
                        
   if ( rx_crysol_params.search( control_parameters[ "iqmethod" ] ) != -1 )
   {
      checks << "crysolharm";
      vals   << "15";
      checks << "crysolgrid";
      vals   << "17";
      checks << "crysolcsh";
      vals   << "15";

      validate_control_parameters_set_one( checks, vals );
   }

   return true;
}

bool US_Saxs_Util::create_tar_output( QString filename )
{
   errormsg = "";
   US_Tar ust;
   int result;
   QStringList list;
   result = ust.create( filename, output_files, &list );

   if ( result != TAR_OK )
   {
      errormsg = QString("Error: Problem creating tar archive %1").arg( filename );
      return false;
   }

   return true;
}

bool US_Saxs_Util::create_tgz_output( QString filename )
{
   errormsg = "";
   US_Tar ust;
   int result;
   QStringList list;
   QString tgz_filename = filename;
   filename.replace( QRegExp( "\\.(tgz|TGZ))$" ), ".tar" );
   QString tar_dot_gz_filename = filename + ".gz";

   result = ust.create( filename, output_files, &list );

   if ( result != TAR_OK )
   {
      errormsg = QString("Error: Problem creating tar archive %1. %2").arg( filename ).arg( ust.explain( result ) );
      return false;
   }

   US_Gzip usg;
   result = usg.gzip( filename );
   if ( result != GZIP_OK )
   {
      errormsg = QString("Error: Problem gzipping tar archive %1. %2").arg( filename ).arg( usg.explain( result ) );
      return false;
   }

   QDir qd;
   qd.remove( tgz_filename );
   if ( !qd.rename( tar_dot_gz_filename, tgz_filename ) )
   {
      errormsg = QString("Error renaming %1 to %2").arg( tar_dot_gz_filename ).arg( tgz_filename );
      return false;
   }

   return true;
}

QString US_Saxs_Util::vector_double_to_csv( vector < double > &vd )
{
   QString result;
   for ( unsigned int i = 0; i < vd.size(); i++ )
   {
      result += QString("%1,").arg(vd[i]);
   }
   return result;
}

bool US_Saxs_Util::write_output( unsigned int model, vector < double > &q, vector < double > &I )
{
   cout << "write output\n";
   if ( control_parameters.count( "output" ) &&
        control_parameters[ "output" ] == "csv" )
   {
      cout << "write output for csv\n";
      saxs_inputfile_for_csv.push_back( control_parameters[ "inputfile" ] );
      saxs_model_for_csv.push_back( model );
      saxs_method_for_csv.push_back( control_parameters[ "iqmethod" ]  );
      saxs_q_for_csv = q;
      saxs_I_for_csv.push_back( I );
   }

   if ( control_parameters.count( "output" ) &&
        ( control_parameters[ "output" ] == "ssaxs" ||
          control_parameters[ "output" ] == "dat" ) )
   {
      cout << "write output for ssaxs or dat\n";
      QString fsaxs_name = control_parameters[ "outputfile" ] + QString("_%1").arg( model + 1 ) + iqq_suffix() + "." +  control_parameters[ "output" ];

      FILE *fsaxs = fopen(fsaxs_name, "w");
      if ( fsaxs ) 
      {
         output_files << fsaxs_name;
         noticemsg += "SAXS curve file: " + fsaxs_name + " created.\n";
         QString last_saxs_header =
            QString("")
            .sprintf(
                     "Simulated SAXS data generated from %s by US_SOMO %s %s q(%f:%f) step %f\n"
                     , control_parameters[ "inputfile" ].ascii()
                     , US_Version.ascii()
                     , REVISION
                     , our_saxs_options.start_q
                     , our_saxs_options.end_q
                     , our_saxs_options.delta_q
                     );
         fprintf(fsaxs, "%s",
                 last_saxs_header.ascii() );
         for ( unsigned int i = 0; i < q.size(); i++ )
         {
            fprintf(fsaxs, "%.6e\t%.6e\n", q[i], I[i]);
         }
         fclose(fsaxs);
      } 
      else
      {
         errormsg = "Error: Could not create SAXS curve file: " + fsaxs_name + "\n";
         return false;
      }
   }
   return true;
}

bool US_Saxs_Util::flush_output()
{
   cout << "flush output\n";
   if ( saxs_inputfile_for_csv.size() &&
        control_parameters.count( "output" ) &&
        control_parameters[ "output" ] == "csv" )
   {
      QString fname = 
         control_parameters[ "outputfile" ] + 
         ( write_output_count ? QString("_%1").arg( write_output_count ) : "" ) +
         "_iqq" + ".csv";

      write_output_count++;

      FILE *of = fopen(fname, "wb");
      if ( of )
      {
         output_files << fname;
         QString header = QString("")
            .sprintf(
                     "Simulated SAXS data generated by US_SOMO %s %s\n"
                     , US_Version.ascii()
                     , REVISION
                     );

         fprintf(of, "\"Name\",\"Type; q:\",%s,\"%s\"\n", 
                 vector_double_to_csv( saxs_q_for_csv ).ascii(),
                 header.ascii());

         for ( unsigned int i = 0; i < saxs_inputfile_for_csv.size(); i++ )
         {
            QString name = QString("%1 Model %2 %3")
               .arg( saxs_inputfile_for_csv[ i ] )
               .arg( saxs_model_for_csv[ i ] )
               .arg( saxs_method_for_csv[ i ] );

            fprintf(of, "\"%s\",\"%s\",%s\n", 
                    name.ascii(),
                    "I(q)",
                    vector_double_to_csv(saxs_I_for_csv[i]).ascii());
         }            
         fclose( of );
         noticemsg += QString("file %1 written\n").arg( fname );
      } else {
         errormsg = QString("Error: could not open %1 for writing").arg( fname );
         return false;
      }
      
      saxs_inputfile_for_csv.clear();
      saxs_model_for_csv    .clear();
      saxs_method_for_csv   .clear();
      saxs_q_for_csv        .clear();
      saxs_I_for_csv        .clear();
   }
   return true;
}
