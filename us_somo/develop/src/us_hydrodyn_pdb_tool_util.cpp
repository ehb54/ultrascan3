#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_pdb_tool_selres.h"
//Added by qt3to4:
#include <QTextStream>

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

#define SLASH QDir::separator()

void US_Hydrodyn_Pdb_Tool::sel_nearest_residues( QTreeWidget *lv )
{
   map < QString, QString > parameters;
   US_Hydrodyn_Pdb_Tool_Selres *selres = 
      new US_Hydrodyn_Pdb_Tool_Selres(
                                      this,
                                      & parameters,
                                      this );
   US_Hydrodyn::fixWinButtons( selres );
   selres->exec();
   delete selres;

   if ( !parameters.count( "go" ) )
   {
      return;
   }

   editor_msg( "blue", us_tr( "Select nearest atoms" ) );
   // pdb_sel_count counts = count_selected( lv );

   double max_dist = parameters[ "max_dist" ].toDouble();

   bool only_new = parameters.count( "only_new" ) &&
      parameters[ "only_new" ] == "true";

   QString save_sel = 
      parameters.count( "save_sel" ) ? parameters[ "save_sel" ] : "";

   double max_asa = 
      parameters.count( "asa" ) ? parameters[ "asa" ].toDouble() : 0e0;

   set < QTreeWidgetItem * > exposed_set;
   if ( max_asa )
   {
      exposed_set = 
         parameters.count( "naccess" ) ? 
         get_exposed_set_naccess( lv, max_asa, 1 == parameters.count( "naccess_sc_or_mc" ), false ) :
         get_exposed_set( lv, max_asa, false );
      if ( !errormsg.isEmpty() )
      {
         editor_msg( "red", errormsg );
      }
   }

   map < QTreeWidgetItem *, double > distmap;

   QTreeWidgetItemIterator it1( lv );
   while ( (*it1) ) 
   {
      QTreeWidgetItem *item1 = (*it1);
      // take selected items
      if ( !item1->childCount() && is_selected( item1 ) )
      {
         QTreeWidgetItemIterator it2( lv );
         if ( !only_new )
         {
            distmap[ item1 ] = 0e0;
         }
         // find distances to not-selected
         while ( (*it2) ) 
         {
            QTreeWidgetItem *item2 = (*it2);
            if ( item1 != item2 && !item2->childCount() && !is_selected( item2 ) )
            {
               double d = pair_dist( item1, item2 );
               // keep those within distance
               if ( d <= max_dist )
               {
                  if ( !distmap.count( item2 ) )
                  {
                     distmap[ item2 ] = d;
                  } else {
                     if ( distmap[ item2 ] > d )
                     {
                        distmap[ item2 ] = d;
                     }
                  }
               }
            }
            ++it2;
         }
      }
      ++it1;
   }

   // now select residues within distmap

   lv->clearSelection();
   
   QTreeWidgetItemIterator it( lv );
   while ( (*it) ) 
   {
      QTreeWidgetItem *item = (*it);
      if ( distmap.count( item ) &&
           ( !max_asa || exposed_set.count( item ) ) )
      {
         item->setSelected( true );
         // editor_msg("blue", QString( "size setting for dist %1 key %2" ).arg( distmap[ item ] ).arg( key( item ) ) );
      }
      ++it;
   }

   select_residues_with_atoms_selected( lv );

   clean_selection( lv );

   if ( lv == lv_csv )
   {
      emit csv_selection_changed();
   } else {
      emit csv2_selection_changed();
   }
   editor_msg( "blue", us_tr( "Select nearest residues done") );
   if ( !save_sel.isEmpty() )
   {
      if ( !save_sel.contains( "/" ) )
      {
         save_sel = ((US_Hydrodyn *)us_hydrodyn)->somo_pdb_dir + SLASH + save_sel;
      }

      if ( QFile::exists( save_sel ) )
      {
         save_sel = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( save_sel, 0, this );
      }
      QFile f( save_sel );
      if ( !f.open( QIODevice::WriteOnly | QIODevice::Text ) )
      {
         editor_msg( "red", QString( us_tr( "Error: Can not create file %1 for writing" ) ).arg( f.fileName() ) );
         return;
      }
      QTextStream t( &f );
      csv out_csv = to_csv( lv, ( lv == lv_csv ) ? csv1 : csv2[ csv2_pos ], true );
      t << csv_to_pdb( out_csv );
      f.close();
      editor_msg( "dark blue", QString( us_tr( "Created selected residue file %1" ) ).arg( f.fileName() ) );
   }
   {
      QString out;
      QTreeWidgetItemIterator it1( lv );
      while ( (*it1) ) 
      {
         QTreeWidgetItem *item1 = (*it1);
         if ( US_Static::lvi_depth( item1 ) == 2 && is_selected( item1 ) )
         {
            out += QString( "%1%2" ).arg( out.isEmpty() ? "" : "," ).arg( get_residue_number( item1 ) );
         }
         it1++;
      }
      lv == lv_csv ?
         csv_msg( "blue", out ) : csv2_msg( "blue", out );
   }
}

set < QTreeWidgetItem * > US_Hydrodyn_Pdb_Tool::get_exposed_set_naccess( QTreeWidget * /* lv */, 
                                                                         double /* max_asa */, 
                                                                         bool /* sc_or_mc */,
                                                                         bool /* only_selected */ )

{
   set < QTreeWidgetItem * > result;
#if QT_VERSION < 0x040000
   vector < vector < QTreeWidgetItem * > > lv_models  = separate_models( lv );
   vector < QStringList >                qsl_models = separate_models( lv == lv_csv  ? csv1 : csv2[ csv2_pos ] );
   vector < QString >                    models     = get_models( lv );

   if ( lv_models.size() != qsl_models.size() ||
        models.size() != lv_models.size() )
   {
      editor_msg( "red", 
                  QString( us_tr( "Error: NACCESS splitting models size mismatch %1 vs %2 vs %3" ) )
                  .arg( lv_models.size() )
                  .arg( qsl_models.size() )
                  .arg( models.size() )
                  );
      return result;
   }

   // get a temp pdb file

   QString use_dir = ((US_Hydrodyn *)us_hydrodyn)->somo_tmp_dir + QDir::separator();

   unsigned int pos = 0;
   QString filename;

   do {
      filename = QString("%1temp%2.pdb").arg( use_dir ).arg( pos++ );
   } while( QFile::exists( filename ) );

   QFile f( filename );

   for ( int i = 0; i < (int) qsl_models.size(); ++i )
   {
      if ( !f.open( QIODevice::WriteOnly ) )
      {
         QMessageBox::warning( this, windowTitle(),
                               QString(us_tr("Could not open %1 for writing!")).arg( filename ) );
         return result;
      }

      QTextStream t( &f );
      for ( int j = 0; j < (int) qsl_models[ i ].size(); ++j )
      {
         t << qsl_models[ i ][ j ];
      }
      f.close();

      naccess_run( filename );
      while ( naccess_running )
      {
         mQThread::msleep( 333 );
         qApp->processEvents();
      }
      if ( !naccess_result_data.size() )
      {
         editor_msg( "red", QString( us_tr( "Error: NACCESS did not return any results for model %1" ) ).arg( i + 1 ) );
         return result;
      }
      set < QString > exposed;
      for ( int j = 0; j < (int) naccess_result_data.size(); ++j )
      {
         cout << naccess_result_data[ j ] << endl;
         if ( naccess_result_data[ j ].left( 3 ) == "RES" )
         {
            QString residue = 
               naccess_result_data[ j ].mid( 4, 3 ) + "~" +
               naccess_result_data[ j ].mid( 8, 1 ).trimmed() + "~" +
               naccess_result_data[ j ].mid( 9, 4 ).trimmed()
               ;
            double this_asa    = naccess_result_data[ j ].mid( 23, 5 ).trimmed().toDouble();
            double this_asa_sc = naccess_result_data[ j ].mid( 36, 5 ).trimmed().toDouble();
            double this_asa_mc = naccess_result_data[ j ].mid( 49, 5 ).trimmed().toDouble();
            if ( sc_or_mc )
            {
               this_asa = this_asa_sc > this_asa_mc ? this_asa_sc : this_asa_mc;
            }
            if ( this_asa >= max_asa )
            {
               // cout << QString( "'%1'\n" ).arg( residue );
               exposed.insert( residue );
            }
         }               
      }
      for ( set < QString >::iterator it = exposed.begin();
            it != exposed.end();
            it++ )
      {
         cout << *it << endl;
      }

      QTreeWidgetItemIterator it( lv );
      while ( (*it) ) 
      {
         QTreeWidgetItem *item = (*it);
         if ( get_model_id( item ) == models[ i ] &&
              exposed.count( QString( "%1~%2~%3" )
                             .arg( get_residue_name( item ) )
                             .arg( get_chain_id( item ).trimmed() )
                             .arg( get_residue_number( item ) ) ) &&
              ( !only_selected || is_selected( item ) ) )
         {
            result.insert( item );
            cout << QString( "Adding Model %1 %2~%3~%4 to exposed\n" )
               .arg( models[ i ] )
               .arg( get_residue_name( item ) )
               .arg( get_chain_id( item ) )
               .arg( get_residue_number( item ) )
               ;
         }
         ++it;
      }
   }
# else
   QMessageBox::warning( this, windowTitle(),
                         us_tr( "NACCESS is not currently supported" ) );
#endif

   return result;
}

set < QTreeWidgetItem * > US_Hydrodyn_Pdb_Tool::get_exposed_set( QTreeWidget * lv, 
                                                               double max_asa, 
                                                               bool only_selected )
{
   set < QTreeWidgetItem * > result;
   errormsg = "";
   QString qs;
   if ( !usu->select_residue_file( ((US_Hydrodyn *)us_hydrodyn)->residue_filename ) )
   {
      errormsg = usu->errormsg;
      return result;
   }
   if ( !usu->select_atom_file( ((US_Hydrodyn *)us_hydrodyn)->saxs_options.default_atom_filename ) )
   {
      errormsg = usu->errormsg;
      return result;
   }
   if ( !usu->select_hybrid_file( ((US_Hydrodyn *)us_hydrodyn)->saxs_options.default_hybrid_filename ) )
   {
      errormsg = usu->errormsg;
      return result;
   }
   if ( !usu->select_saxs_file( ((US_Hydrodyn *)us_hydrodyn)->saxs_options.default_saxs_filename ) )
   {
      errormsg = usu->errormsg;
      return result;
   }

   usu->control_parameters[ "asamethod"          ] = QString( "%1" ).arg( ((US_Hydrodyn *)us_hydrodyn)->asa.method );
   usu->control_parameters[ "pdbmissingatoms"    ] = QString( "%1" ).arg( ((US_Hydrodyn *)us_hydrodyn)->pdb_parse.missing_atoms );
   usu->control_parameters[ "pdbmissingresidues" ] = QString( "%1" ).arg( ((US_Hydrodyn *)us_hydrodyn)->pdb_parse.missing_residues );
   usu->control_parameters[ "saxsfile"           ] = ((US_Hydrodyn *)us_hydrodyn)->saxs_options.default_saxs_filename;
   usu->control_parameters[ "hybridfile"         ] = ((US_Hydrodyn *)us_hydrodyn)->saxs_options.default_hybrid_filename;
   usu->control_parameters[ "atomfile"           ] = ((US_Hydrodyn *)us_hydrodyn)->saxs_options.default_atom_filename;

   // if this is a very large pdb, it should probably go to a disk file
   QStringList qsl = csv_to_pdb_qsl( lv == lv_csv ? csv1 : csv2[ csv2_pos ] );

   if ( !usu->read_pdb( qsl ) )
   {
      errormsg = usu->errormsg;
      return result;
   }

   usu->asa              = ((US_Hydrodyn *)us_hydrodyn)->asa;
   usu->asa.hydrate_probe_radius = usu->asa.probe_radius;
   usu->our_saxs_options = ((US_Hydrodyn *)us_hydrodyn)->saxs_options;
   usu->control_parameters[ "inputfile" ] = "pdbtool_asa.pdb";
   usu->current_model    = 0;
   for (unsigned int i = 0; i < usu->model_vector.size (); i++) 
   {
      usu->current_model = i;
      if ( !usu->pdb_asa_for_saxs_hydrate() )
      {
         errormsg = usu->errormsg;
         return result;
      }
   }

   float total_asa = 0.0;
   float total_ref_asa = 0.0;
   float total_vol = 0.0;
   float total_mass = 0.0;

   QString last_residue = "";
   int seqno = 0;
   float residue_asa = 0;
   float residue_ref_asa = 0;

   int aa = 0;

   QTreeWidgetItemIterator it( lv );
   while ( (*it) ) 
   {
      QTreeWidgetItem *item = (*it);
      if ( item->isSelected() )
      {
         cout << QString( "model id <%1> chain id <%2> residue name <%3> residue number <%4>\n" )
            .arg( get_model_id( item ) )
            .arg( get_chain_id( item ) )
            .arg( get_residue_name( item ) )
            .arg( get_residue_number( item ) )
            ;
      }
      ++it;
   }

   vector < QString > models = get_models( lv );
   if ( usu->model_vector.size() != models.size() )
   {
      editor_msg( "red", 
                  QString( us_tr( "Error: model size mismatch after read pdb (%1 != %2)" ) )
                  .arg( models.size() )
                  .arg( usu->model_vector.size() )
                  );
      return result;
   }

   for (unsigned int i = 0; i < (unsigned int) usu->model_vector.size(); i++) 
   {
      set < QString > exposed;

      for (unsigned int j = 0; j < usu->model_vector[ i ].molecule.size(); j++ ) 
      {
         for ( unsigned int k = 0; k < usu->model_vector[ i ].molecule[ j ].atom.size (); k++ ) 
         {
            if ( usu->model_vector[ i ].molecule[ j ].atom[ k ].active ) 
            {
               aa++;
               total_asa += usu->model_vector[ i ].molecule[ j ].atom[ k ].bead_asa;
               total_ref_asa += usu->model_vector[ i ].molecule[ j ].atom[ k ].ref_asa;
               total_mass += usu->model_vector[ i ].molecule[ j ].atom[ k ].bead_ref_mw;
               total_vol += usu->model_vector[ i ].molecule[ j ].atom[ k ].bead_ref_volume_unhydrated;

               QString residue =
                  usu->model_vector[ i ].molecule[ j ].atom[ k ].resName + "~" +
                  (usu->model_vector[ i ].molecule[ j ].atom[ k ].chainID == " " ? "~" : usu->model_vector[ i ].molecule[ j ].atom[ k ].chainID) +
                  QString("~%1").arg(usu->model_vector[ i ].molecule[ j ].atom[ k ].resSeq);
               if (residue != last_residue) {
                  if (last_residue != "") {
                     printf(
                            " [ %-6d %s ]\t%.0f\t%.0f\t%.2f\n",
                            seqno, last_residue.toLatin1().data(), residue_asa, residue_ref_asa, 100.0 * residue_asa / residue_ref_asa);
                     if ( 100.0 * residue_asa / residue_ref_asa >= max_asa )
                     {
                        exposed.insert( last_residue );
                     }
                  }
                  residue_asa = 0;
                  residue_ref_asa = 0;
                  last_residue = residue;
                  seqno++;
               }
               residue_asa += usu->model_vector[ i ].molecule[ j ].atom[ k ].bead_asa;
               residue_ref_asa = usu->model_vector[ i ].molecule[ j ].atom[ k ].ref_asa;
            }
         }

         if (last_residue != "") {
            printf( 
                   " [ %-6d %s ]\t%.0f\t%.0f\t%.2f\n",
                   seqno, last_residue.toLatin1().data(), residue_asa, residue_ref_asa, 100.0 * residue_asa / residue_ref_asa);
            if ( 100.0 * residue_asa / residue_ref_asa >= max_asa )
            {
               exposed.insert( last_residue );
            }
         }
      }

      for ( set < QString >::iterator it = exposed.begin();
            it != exposed.end();
            it++ )
      {
         cout << *it << endl;
      }

      QTreeWidgetItemIterator it( lv );
      while ( (*it) ) 
      {
         QTreeWidgetItem *item = (*it);
         if ( get_model_id( item ) == models[ i ] &&
              exposed.count( QString( "%1~%2~%3" )
                             .arg( get_residue_name( item ) )
                             .arg( get_chain_id( item ).trimmed() )
                             .arg( get_residue_number( item ) ) ) &&
              ( !only_selected || is_selected( item ) ) )
         {
            result.insert( item );
            cout << QString( "Adding Model %1 %2~%3~%4 to exposed\n" )
               .arg( models[ i ] )
               .arg( get_residue_name( item ) )
               .arg( get_chain_id( item ) )
               .arg( get_residue_number( item ) )
               ;
         }
         ++it;
      }
   }
   cout << QString( "model vector size %1 aa %2\n" ).arg( usu->model_vector.size() ).arg( aa );
   return result;
}

vector < QString > US_Hydrodyn_Pdb_Tool::get_models( QTreeWidget *lv )
{
   vector < QString > result;
   QString last_model = "unknown";

   QTreeWidgetItemIterator it( lv );
   while ( (*it) ) 
   {
      QTreeWidgetItem *item = (*it);
      QString this_model = get_model_id( item );
      if ( this_model != last_model )
      {
         result.push_back( this_model );
         last_model = this_model;
      }
      ++it;
   }
   return result;
}


// -------------------- Naccess ------------------------------

bool US_Hydrodyn_Pdb_Tool::naccess_run( QString /* pdb */ )
{
#if QT_VERSION < 0x040000
   naccess_running = false;
   naccess_last_pdb = pdb;
   naccess_result_data.clear( );
   QString prog = 
      USglobal->config_list.system_dir + SLASH + "bin"
#if defined(BIN64)
      "64"
#endif
      + SLASH
      + "naccess" 
      ;

   QString radii = 
      USglobal->config_list.system_dir + SLASH + "bin"
#if defined(BIN64)
      "64"
#endif
      + SLASH
      + "vdw.radii" 
      ;

   {
      QFileInfo qfi(prog);
      if ( !qfi.exists() )
      {
         editor_msg("red", QString("Naccess program '%1' does not exist\n").arg(prog));
         return false;
      }
      if ( !qfi.isExecutable() )
      {
         editor_msg("red", QString("Naccess program '%1' is not executable\n").arg(prog));
         return false;
      }
   }

   {
      QFileInfo qfi(radii);
      if ( !qfi.exists() )
      {
         editor_msg("red", QString("Naccess support file '%1' does not exist\n").arg(radii));
         return false;
      }
   }

   QFileInfo fi( pdb );
   if ( !fi.exists() )
   {
      editor_msg("red", QString("Naccess called but PDB file '%1' does not exist\n").arg(pdb));
      return false;
   }

   naccess = new QProcess( this );
   //   naccess->setWorkingDirectory( dir );
   naccess->addArgument( prog );
   naccess->addArgument( pdb );
   naccess->addArgument( "-p" );
   naccess->addArgument( QString( "%1" ).arg( ((US_Hydrodyn *)us_hydrodyn)->asa.probe_radius ) );
   // naccess->addArgument( "-z" );
   // naccess->addArgument( QString( "%1" ).arg( ((US_Hydrodyn *)us_hydrodyn)->asa.asab1_step ) );
   
   connect( naccess, SIGNAL(readyReadStandardOutput()), this, SLOT(naccess_readFromStdout()) );
   connect( naccess, SIGNAL(readyReadStandardError()), this, SLOT(naccess_readFromStderr()) );
   connect( naccess, SIGNAL(finished( int, QProcess::ExitStatus )), this, SLOT(naccess_finished( int, QProcess::ExitStatus )) );
   connect( naccess, SIGNAL(started()), this, SLOT(naccess_started()) );

   editor->append("\n\nStarting Naccess\n");
   naccess->start();
   naccess_running = true;

   return true;
#else
   return false;
#endif
}

void US_Hydrodyn_Pdb_Tool::naccess_readFromStdout()
{
#if QT_VERSION < 0x040000
   while ( naccess->canReadLineStdout() )
   {
      editor_msg("brown", naccess->readLineStdout() + "\n");
   }
#else
   editor_msg( "brown", QString( naccess->readAllStandardOutput() ) );
#endif   
   //  qApp->processEvents();
}
   
void US_Hydrodyn_Pdb_Tool::naccess_readFromStderr()
{
#if QT_VERSION < 0x040000
   while ( naccess->canReadLineStderr() )
   {
      editor_msg("red", naccess->readLineStderr() + "\n");
   }
#else
   editor_msg( "red", QString( naccess->readAllStandardError() ) );
#endif   
   //  qApp->processEvents();
}
   
void US_Hydrodyn_Pdb_Tool::naccess_finished( int, QProcess::ExitStatus )
{
   //   for ( int i = 0; i < 10000; i++ )
   //   {
   naccess_readFromStderr();
   naccess_readFromStdout();
      //   }
   disconnect( naccess, SIGNAL(readyReadStandardOutput()), 0, 0);
   disconnect( naccess, SIGNAL(readyReadStandardError()), 0, 0);
   disconnect( naccess, SIGNAL(finished( int, QProcess::ExitStatus )), 0, 0);
   editor->append("Naccess finished.\n");

   // naccess creates 2 files:
   // pdb_rsa

   QString naccess_result_file = QString( naccess_last_pdb ).replace( QRegExp( ".pdb$" ), "" ) + ".rsa";

   QFile f( naccess_result_file );

   if ( !f.exists() )
   {
      editor_msg("red", QString(us_tr("Error: Naccess did not create file %1")).arg( naccess_result_file ));
      naccess_running = false;
      return;
   }

   if ( !f.open( QIODevice::ReadOnly ) )
   {
      editor_msg("red", QString(us_tr("Error: can not open Naccess result file %1")).arg( naccess_result_file ));
      naccess_running = false;
      return;
   }

   QTextStream ts( &f );
   while ( !ts.atEnd() )
   {
      naccess_result_data << ts.readLine();
   }
   f.close();
   //    if ( !f.remove() )
   //    {
   //       editor_msg("dark red", QString(us_tr("Warning: could not remove Naccess result file %1")).arg( naccess_result_file ));
   //    }
      
   naccess_running = false;
}
   
void US_Hydrodyn_Pdb_Tool::naccess_started()
{
   editor_msg("brown", "Naccess launch exited\n");
   disconnect( naccess, SIGNAL(started()), 0, 0);
}

vector < vector < QTreeWidgetItem * > > US_Hydrodyn_Pdb_Tool::separate_models( QTreeWidget *lv )
{
   vector < QTreeWidgetItem * >            vlvi;
   vector < vector < QTreeWidgetItem * > > vvlvi;

   QString last_model = "unknown";
   QTreeWidgetItemIterator it( lv );
   while ( (*it) ) 
   {
      QTreeWidgetItem *item = (*it);
      QString this_model = get_model_id( item );
      if ( this_model != last_model )
      {
         if ( last_model != "unknown" )
         {
            vvlvi.push_back( vlvi );
         }
         last_model = this_model;
         vlvi.clear( );
      }
      vlvi.push_back( item );
      ++it;
   }
   if ( vlvi.size() )
   {
      vvlvi.push_back( vlvi );
   }
   return vvlvi;
}

vector < QStringList > US_Hydrodyn_Pdb_Tool::separate_models( csv &ref_csv )
{
   vector < QStringList > result;

   QStringList qsl_pdb = csv_to_pdb_qsl( ref_csv );

   QRegExp rx_model ( "^MODEL"  );
   QRegExp rx_endmdl( "^ENDMDL" );
   QRegExp rx_end   ( "^END" );
   QRegExp rx_skip  ( "^(MODEL|ENDMDL|HEADER|REMARK)"  );

   QStringList qsl_model;

   for ( int i = 0; i < (int) qsl_pdb.size(); i++ )
   {
      if ( rx_model.indexIn( qsl_pdb[ i ] ) != -1 )
      {
         // new model line
         if ( qsl_model.size() )
         {
            if ( rx_end.indexIn( qsl_model.back() ) == -1 )
            {
               qsl_model << "END\n";
            }
            result.push_back( qsl_model );
            qsl_model.clear( );
         }
      }
      if ( rx_skip.indexIn( qsl_pdb[ i ] ) == -1 )
      {
         qsl_model.push_back( qsl_pdb[ i ] );
      }
   }
   if ( qsl_model.size() )
   {
      if ( rx_end.indexIn( qsl_model.back() ) == -1 )
      {
         qsl_model << "END\n";
      }
      result.push_back( qsl_model );
   }
   return result;
}

void US_Hydrodyn_Pdb_Tool::select_residues_with_atoms_selected( QTreeWidget *lv )
{
   QTreeWidgetItemIterator it( lv );
   while ( (*it) ) 
   {
      QTreeWidgetItem *item = (*it);
      if ( !item->isSelected() &&
           US_Static::lvi_depth( item ) == 2 )
      {
#if QT_VERSION < 0x040000
         QTreeWidgetItem *myChild = item->firstChild();
         while ( myChild )
         {
            if ( myChild->isSelected() )
            {
               item->setSelected( true );
               break;
            }
            myChild = myChild->nextSibling();
         }
#else
         {
            int children = item->childCount();
            if ( children ) { 
               for ( int i = 0; i < children; ++i ) {
                  QTreeWidgetItem *myChild = item->child( i );
                  if ( myChild->isSelected() )
                  {
                     item->setSelected( true );
                     break;
                  }
               }
            }
         }
#endif
      }
      ++it;
   }
}

void US_Hydrodyn_Pdb_Tool::sel( QTreeWidget *lv )
{
   // make sure atoms are selected
   clean_selection( lv );

   bool ok;


   QString cur_res_sel;
   {
      QTreeWidgetItemIterator it( lv );

      while ( (*it) ) 
      {
         QTreeWidgetItem *item = (*it);
         if ( US_Static::lvi_depth( item ) == 2 &&
              is_selected( item ) )
         {
            QString rn = get_residue_number( item );
            if ( rn != "unknown" )
            {
               cur_res_sel += cur_res_sel.isEmpty() ? "" : ",";
               cur_res_sel += rn;
            }
         }
         ++it;
      }
   }
   
   QString text = US_Static::getText(
                                        windowTitle() + us_tr( ": Select" ),
                                        us_tr( "Enter comma seperated residue number selection list\n" ),
                                        QLineEdit::Normal,
                                        cur_res_sel,
                                        &ok, 
                                        this );
   if ( !ok )
   {
      return;
   }

   // make list of selections

   QStringList sels = (text ).split( "," , Qt::SkipEmptyParts );
   set < int > to_sel;

   QRegExp rx_range( "(\\d+)-(\\d+)" );

   for ( int i = 0; i < (int) sels.size(); ++i )
   {
      if ( rx_range.indexIn( sels[ i ] ) != -1 )
      {
         int startr = rx_range.cap( 1 ).toInt();
         int endr   = rx_range.cap( 2 ).toInt();
         for ( int j = startr; j <= endr; ++j )
         {
            to_sel.insert( j );
         }
      } else {
         to_sel.insert( sels[ i ].toInt() );
      }
   }

   for ( set < int >::iterator it = to_sel.begin();
         it != to_sel.end();
         it++ )
   {
      cout << QString( "to sel: %1\n" ).arg( *it );
   }

   {
      QTreeWidgetItemIterator it( lv );

      while ( (*it) ) 
      {
         QTreeWidgetItem *item = (*it);
         if ( to_sel.count( get_residue_number( item ).toInt() ) )
         {
            item->setSelected( true );
         }
         ++it;
      }
   }

   // lv->triggerUpdate();

   clean_selection( lv );
   if ( lv == lv_csv )
   {
      emit csv_selection_changed();
   } else {
      emit csv2_selection_changed();
   }
}
