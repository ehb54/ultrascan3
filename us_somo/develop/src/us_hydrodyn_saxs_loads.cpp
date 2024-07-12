#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_saxs_util.h"
#include "../include/us_hydrodyn_saxs_iqq_load_csv.h"
#include "../include/us_hydrodyn_saxs_load_csv.h"

#define SLASH QDir::separator()

#ifdef WIN32
# include <float.h>
//Added by qt3to4:
#include <QTextStream>
#endif

#define TSO QTextStream(stdout)

static QStringList pr_csv_pad_holes( const QStringList &qsl ) {
   TSO << "pr_csv_pad_holes()\n";

   // find r vector

   QStringList qsl_headers = qsl.filter("\"Name\",\"MW (Daltons)\",\"Area\",\"Type; r:\"");
   if ( !qsl_headers.size() ) {
      TSO << "pr_csv_pad_holes() -- no proper header\n";
      return qsl;
   }

   QStringList qsl_r = (qsl_headers[0]).split( "," );

   if ( qsl_r.size() < 6 )
   {
      TSO << "pr_csv_pad_holes() does not appear to contain any r values in the header rows.\n";
      return qsl;
   }

   vector < double > r;

   r.push_back(qsl_r[4].toDouble());
   {
      QStringList::iterator it = qsl_r.begin();
      it += 5;
      for ( ; it != qsl_r.end(); it++ )
      {
         if ( (*it).toDouble() > r[r.size() - 1] )
         {
            r.push_back((*it).toDouble());
         } else {
            break;
         }
      }
   }
   
   if ( r.size() < 3 ) {
      TSO << "pr_csv_pad_holes() r.size() too small to consider.\n";
      return qsl;
   }      

   // assume first two entries are spacing
   double delta = r[1] - r[0];

   TSO << QString( "pr_csv_pad_holes() r.size() %1\n" ).arg( delta );

   // check if ok as-is

   bool ok      = true;
   bool fixable = true;
   map < int, int > pad_here;

   for ( int i = 1; i < (int) r.size(); ++i ) {
      double this_delta =  r[i] - r[i-1];
      if ( this_delta != delta ) {
         ok = false;
         double inserts = this_delta / delta - 1;
         if ( inserts != (int)inserts ) {
            fixable = false;
            break;
         }
         // i+4 for offset into original
         pad_here[ i + 4 ] = (int)inserts;
      }
   }
         
   TSO << QString( "pr_csv_pad_holes() r is %1\n" ).arg( ok ? "OK" : "not OK" );
   if ( ok ) {
      return qsl;
   }

   // US_Vector::printvector( "pr_csv_pad_holes() r", r );

   // TSO << QString( "pr_csv_pad_holes() r is %1\n" ).arg( fixable ? "fixable" : "not Fixable" );

   if ( !fixable ) {
      return qsl;
   }

   // ok, fix !
   
   QStringList qsl_header_new = qsl_headers[0].split( "," );

   QStringList qsl_data = qsl.filter(",\"P(r)");
   vector < QStringList > qsl_data_new;
   for ( int i = 0; i < (int) qsl_data.size(); ++i ) {
      qsl_data_new.push_back( qsl_data[i].split( "," ) );
      qsl_data_new.back().removeLast();
      while ( qsl_data_new.back().size() < qsl_header_new.size() - 2 ) {
         qsl_data_new.back() << "0";
      }
   }

   for ( auto it = pad_here.begin();
         it != pad_here.end();
         ++it ) {
      TSO << QString( "pr_csv_pad_holes() pad_here[%1] = %2\n" ).arg( it->first ).arg( it->second );
      for ( int i = 0; i < it->second; ++i ) {
         // fix header
         qsl_header_new.insert( it->first + i, QString( "%1" ).arg( qsl_header_new[it->first - 1].toDouble() + ( delta * ( i + 1 ) ) ) );
         // fix datalines
         for ( int j = 0; j < (int) qsl_data_new.size(); ++j ) {
            qsl_data_new[j].insert( it->first + i, "0" );
         }
      }
   }

   // check for SD lines an if they exist, store 10^-3 * minimum non-zero value
   // map names for pr lookup on pr sd line
   {
      map < QString, int > name2pr;
      map < QString, int > name2pr_normed;
      
      for ( int i = 0; i < (int) qsl_data_new.size(); ++i ) {
         if ( qsl_data_new[i].size() > 4 ) {
            if ( qsl_data_new[i][3] == "\"P(r)\"" ) {
               name2pr       [ qsl_data_new[i][0] ] = i;
            } else if ( qsl_data_new[i][3] == "\"P(r) normed\"" ) {
               name2pr_normed[ qsl_data_new[i][0] ] = i;
            }
         }
      }

      for ( int i = 0; i < (int) qsl_data_new.size(); ++i ) {
         if ( qsl_data_new[i].size() > 4 &&
              qsl_data_new[i][3].contains( QRegularExpression( "^\"P\\(r\\)(| normed) sd\"$" ) ) ) {
            // qDebug() << "found SD line";
            list < double > sd_gz;
            // find min sd
            for ( int j = 4; j < (int) qsl_data_new[i].size(); ++j )  {
               if ( qsl_data_new[i][j].toDouble() > 0
                    && (
                        (
                         qsl_data_new[i][3] == "\"P(r) sd\""
                         && name2pr.count( qsl_data_new[i][0] )
                         && qsl_data_new[ name2pr[ qsl_data_new[i][0] ] ].size() > j
                         && qsl_data_new[ name2pr[ qsl_data_new[i][0] ] ][j].toDouble() > 0
                         )
                        || 
                        (
                         qsl_data_new[i][3] == "\"P(r) normed sd\""
                         && name2pr_normed.count( qsl_data_new[i][0] )
                         && qsl_data_new[ name2pr_normed[ qsl_data_new[i][0] ] ].size() > j
                         && qsl_data_new[ name2pr_normed[ qsl_data_new[i][0] ] ][j].toDouble() > 0
                         )
                        )
                    ) {
                  sd_gz.push_back( qsl_data_new[i][j].toDouble() );
               }
            }
            if ( !sd_gz.size() ) {
               // no nonneg sds! punt
               qDebug() << QString( "pr_csv_pad_holes() : curve %1 %2 no nonneg sds! punt" ).arg( qsl_data_new[i][0] ).arg( qsl_data_new[i][3] ) ;
               break;
            }
            sd_gz.sort();
            double use_min_sd = sd_gz.front() * 1e-3;
            // let set_pr_sd handle setting up the values
            use_min_sd = 0;
            // qDebug() << QString( "use min sd for curve %1 is %2" ).arg( qsl_data_new[i][0] ).arg( use_min_sd );
            // replace zeros with
            for ( int j = 4; j < (int) qsl_data_new[i].size(); ++j )  {
               if ( qsl_data_new[i][j].toDouble() <= 0 ) {
                  qsl_data_new[i][j] = QString( "%1" ).arg( use_min_sd );
               }
            }
         }
      }
   }
            
   // TSO << "pr_csv_pad_holes() fixed header\n" << qsl_header_new.join( "\n" ) << "\n";
   // TSO << "pr_csv_pad_holes() fixed lines\n";
   // for ( int i = 0; i < (int) qsl_data_new.size(); ++i ) {
   //    TSO << qsl_data_new[i].join( "\n" ) << "\n";
   // }
   
   // reassemble
   QStringList qsl_new;
   qsl_new << qsl_header_new.join(",");
   for ( int i = 0; i < (int) qsl_data_new.size(); ++i ) {
      qsl_new << qsl_data_new[i].join(",");
   }

   TSO << "pr_csv_pad_holes() fixed qsl\n" << qsl_new.join("\n") << "\n";

   return qsl_new;
}

static QStringList csv_transpose( const QStringList &qsl ) {
   // qDebug() << "csv_transpose()";
   // QTextStream( stdout ) << "source:" << qsl.join( "\n" ) << "\n";

   map < int, map < int, QString > >  data;

   int rows = (int) qsl.size();

   int max_cols = 0;

   for ( int i = 0; i < rows; ++i ) {
      QStringList row = qsl[i].split( "," );
      int cols = (int) row.size();
      if ( max_cols < cols ) {
         max_cols = cols;
      }
      
      for ( int j = 0; j < cols; j++ ) {
         data[ i ][ j ] = row[ j ];
      }
   }

   QStringList res;

   {
      for ( int j = 0; j < max_cols; ++j ) {
         QString line;
         for ( int i = 0; i < rows; ++i ) {
            if ( data.count( i ) && data[ i ].count( j ) ) {
               line += data[ i ][ j ];
            }
            line += ",";
         }
         res << line;
      }
   }
   
   // QTextStream( stdout )  << "result\n" << res.join( "\n" ) << "\n";
   
   return res;
}
      
static QStringList csv_pr2iq( const QStringList &qsl ) {
   qDebug() << "csv_pr2iq()";
   // QTextStream( stdout ) << "source:" << qsl.join( "\n" ) << "\n";

   QStringList res;

   int rows = (int) qsl.size();

   bool insert0 = true;

   // do we need to insert a zero?
   if ( rows ) {
      QStringList row = qsl[0].split( "," );
      if ( row.size() > 4 &&
           row[4] == "0" ) {
         insert0 = false;
      }
   }

   // QTextStream(stdout) << "insert0s: " << ( insert0 ? "true" : "false" ) << "\n";

   for ( int i = 0; i < rows; ++i ) {
      QStringList row = qsl[i].split( "," );
      if ( row.size() < 4 ) {
         continue;
      }

      row = row.mid(0,1) + row.mid( 3 );

      if ( row[1] == "\"P(r)\"" ) {
         // drop either "\"P(r)\"" or "\"P(r) normed\""
         continue;
      }

      if ( row[1] == "\"P(r) normed\"" ) {
         // sync with above drop decision & also in ::unify_csv_files() !
         row[1] = "\"I(q)\"";
      }

      if ( row[1] == "\"Type; r:\"" ) {
         row[1] = "\"Type; q:\"";
      }

      // QTextStream( stdout ) << "row[1] [" << row[1] << "]\n";

      if ( insert0 ) {
         row.insert(2, "0" );
      }

      // QTextStream( stdout ) << row.join( "," ) << "\n";
      res << row.join( "," );
   }

   // QTextStream( stdout )  << "result\n" << res.join( "\n" ) << "\n";
   
   return res;
}
      
void US_Hydrodyn_Saxs::load_iqq_csv( QString filename, bool just_plotted_curves )
{

   vector < QString > filenames; // for later use to support multiple csv file loads

   if ( filename.isEmpty() && !just_plotted_curves )
   {
      return;
   }
   QFile f( filename );

   // attempt to read a csv file
   QStringList qsl;
   QStringList qsl_data_lines_plotted;  // for potential later average save
   if ( !just_plotted_curves )
   {
      if ( !f.open(QIODevice::ReadOnly) )
      {
         QMessageBox::warning( this, "UltraScan",
                               QString(us_tr("Can not open file ")
                                    + filename ) );
         return;
      }
      QTextStream ts(&f);
      while ( !ts.atEnd() )
      {
         qsl << ts.readLine();
      }
      f.close();
   }
   
   QStringList qsl_headers = qsl.filter("\"Name\",\"Type; q:\"");
   QStringList qsl_t_headers = qsl.filter( QRegExp( "^\"(Name|Type; q:)\"" ) );
   if ( qsl_t_headers.size() == 2 ) {
      qsl = csv_transpose( qsl );
      qsl_headers = qsl.filter("\"Name\",\"Type; q:\"");         
   }

   // check if p(r) and load as Iq
   {
      QStringList test_qsl_headers = qsl.filter("\"Name\",\"MW (Daltons)\",\"Area\",\"Type; r:\"");
      qDebug() << "checking for p(r)";
      if ( test_qsl_headers.size() != 0 ) {
         switch( QMessageBox::warning(
                                      this
                                      ,windowTitle()
                                      ,us_tr("The file appears to be in P(r) format.\n"
                                             "Load as if it were an I(q)?")
                                      ,QMessageBox::Ok | QMessageBox::Cancel
                                      ,QMessageBox::Cancel
                                      ) ) {
         case QMessageBox::Ok :
            break;
         case QMessageBox::Cancel :
         default:
            return;
            break;
         }
         // reprocess
         qsl = csv_pr2iq( qsl );
         qsl_headers = qsl.filter("\"Name\",\"Type; q:\"");
      }
   }
            
   if ( qsl_headers.size() == 0 && !just_plotted_curves ) 
   {
      // QMessageBox mb(us_tr("UltraScan Warning"),
      //                us_tr("The csv file ") + filename + us_tr(" does not appear to contain a correct header.\nPlease manually correct the csv file."),
      //                QMessageBox::Critical,
      //                QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
      // mb.exec();
      QMessageBox::critical( this,
                             us_tr("UltraScan Warning"),
                             us_tr("The csv file ") + filename + us_tr(" does not appear to contain a correct header.\nPlease manually correct the csv file."),
                             QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton );
      return;
   }
   
   if ( qsl_headers.size() > 1 ) 
   {
      QString ref = qsl_headers[0];
      for ( unsigned int i = 1; i < (unsigned int)qsl_headers.size(); i++ )
      {
         if ( ref != qsl_headers[i] )
         {
            // QMessageBox mb(us_tr("UltraScan Warning"),
            //                us_tr("The csv file ") + filename + us_tr(" contains multiple different headers\nPlease manually correct the csv file."),
            //                QMessageBox::Critical,
            //                QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
            // mb.exec();
            QMessageBox::critical( this,
                                   us_tr("UltraScan Warning"),
                                   us_tr("The csv file ") + filename + us_tr(" contains multiple different headers\nPlease manually correct the csv file."),
                                   QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton );
            return;
         }
      }
   }
   
   vector < double > q;
   vector < double > I;
   vector < double > I_errors;

   // get the q values
   QStringList qsl_q;
   QString header_tag;
   
   if ( just_plotted_curves &&
        !qsl_plotted_iq_names.size() )
   {
         QMessageBox::warning( this, "UltraScan",
                               QString(us_tr("There is nothing plotted!")) );
         return;
   }      

   QString grid_target = "";
   vector < double > original_q;

   if ( !just_plotted_curves )
   {
      if ( qsl_plotted_iq_names.size() )
      {
         bool ok;
         grid_target = US_Static::getItem(
                                                us_tr("Set I(q) Grid"),
                                                us_tr("Select the target plotted data set for the data being loaded grid:\n"
                                                   "or Cancel if you wish to interpolate the plotted to the data being loaded")
                                                , 
                                                qsl_plotted_iq_names, 
                                                0, 
                                                false, 
                                                &ok,
                                                this );
         if ( ok ) {
            // user selected an item and pressed OK
         } else {
            grid_target = "";
         }
      }

      qsl_q = (qsl_headers[0]).split( "," );
      if ( qsl_q.size() < 3 )
      {
         // QMessageBox mb(us_tr("UltraScan Warning"),
         //                us_tr("The csv file ") + filename + us_tr(" does not appear to contain any q values in the header rows.\n"),
         //                QMessageBox::Critical,
         //                QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
         // mb.exec();
         QMessageBox::critical( this,
                                us_tr("UltraScan Warning"),
                                us_tr("The csv file ") + filename + us_tr(" does not appear to contain any q values in the header rows.\n"),
                                QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton );
         return;
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

      // now, reinterpolate the q 
      if ( !grid_target.isEmpty() ) {
         // QTextStream(stdout) << "reinterpolate non empty grid target\n";

         if ( !plotted_iq_names_to_pos.count( grid_target ) )
         {
            editor_msg( "red", QString( us_tr("Internal error: could not find %1 in plotted data" ) ).arg( grid_target ) );
            grid_target = "";
         } else {
            original_q = q;
            q = plotted_q[ plotted_iq_names_to_pos[ grid_target ] ];
            // check q against original_q for cropping
            if ( q[ 0 ] < original_q[ 0 ] || q.back() > original_q.back() )
            {
               editor_msg( 
                          "dark red", 
                          QString( "Notice: the target curve is cropped to q(%1:%2) to prevent extrapolation" )
                          .arg( original_q[ 0 ] )
                          .arg( original_q.back() ) 
                          );
                          
               nnls_csv_footer <<
                  QString( "\nNotice: the target curve is cropped to prevent extrapolation q range\",%1,%2\n" )
                  .arg( original_q[ 0 ] )
                  .arg( original_q.back() ) 
                  ;

               unsigned int pos = plotted_iq_names_to_pos[ grid_target ];

               vector < double > new_q;
               vector < double > new_q2;
               vector < double > new_I;
               vector < double > new_I_error;

               for ( unsigned int i = 0; i < q.size(); i++ )
               {
                  if ( q[ i ] >= original_q[ 0 ] && q[ i ] <= original_q.back() )
                  {
                     new_q       .push_back( q[ i ] );
                     if ( plotted_q2[ pos ].size() > i )
                     {
                        new_q2      .push_back( plotted_q2[ pos ][ i ] );
                     }
                     new_I       .push_back( plotted_I[ pos ][ i ] );
                     if ( plotted_I_error[ pos ].size() > i )
                     {
                        new_I_error .push_back( plotted_I_error[ pos ][ i ] );
                     }
                  }
               }
               plotted_q       [ pos ] = new_q;
               plotted_q2      [ pos ] = new_q2;
               plotted_I       [ pos ] = new_I;
               plotted_I_error [ pos ] = new_I_error;
               q = new_q;
            }

            // reset header
            QStringList qsl_q = (qsl_headers[0]).split( "," );
            QString msg = qsl_q[ qsl_q.size() - 1 ];
            msg.replace("\"", "");
            qsl[0] = QString( "\"Name\",\"Type; q:\",%1,\"%2 reinterpolated to q(%3:%4) step %5\"" )
               .arg( vector_double_to_csv( q ) )
               .arg( msg )
               .arg( q.size() ? q[ 0 ] : 0 )
               .arg( q.size() ? q[ q.size() - 1 ] : 0 )
               .arg( q.size() > 1 ? q[ 1 ] - q[ 0 ] : 0 );
            qsl_headers[ 0 ] = qsl[ 0 ];
            TSO << qsl[ 0 ] << endl;
            QStringList new_qsl;
            QStringList new_qsl_errors;
            new_qsl << qsl[ 0 ];

            // also also reinterpolate all the data lines
            QStringList qsl_data = qsl.filter(",\"I(q)\",");
            QStringList qsl_sd   = qsl.filter(",\"I(q) sd\",");
            map < QString, QString > sd_map;
            for ( unsigned int i = 0; i < (unsigned int) qsl_sd.size(); i++ )
            {
               QStringList qsl_s = (qsl_sd[i]).split( "," );
               sd_map[ qsl_s[ 0 ] ] = qsl_sd[ i ];
            }
               
            US_Saxs_Util usu;

            for ( unsigned int i = 0; i < (unsigned int) qsl_data.size(); i++ )
            {
               QStringList qsl_d = (qsl_data[i]).split( "," );
               vector < double > original_i;
               vector < double > original_i_error;
               for ( int j = 2; j < (int) qsl_d.size() - 1; j++ )
               {
                  original_i.push_back( qsl_d[j].toDouble() );
               }
               if ( original_i.size() < original_q.size() ) {
                  original_i.resize( original_q.size(), 0 );
               }
               if ( sd_map.count( qsl_d[ 0 ] ) )
               {
                  QStringList qsl_s = (sd_map[ qsl_d[ 0 ] ]).split( "," );
                  for ( int j = 2; j < (int) qsl_s.size() - 1; j++ )
                  {
                     original_i_error.push_back( qsl_s[j].toDouble() );
                  }
                  if ( original_i_error.size() < original_q.size() ) {
                     original_i_error.resize( original_q.size(), 0 );
                  }
               }
               // now interpolate
               vector < double > ni;
               vector < double > ni_error;
               // QTextStream(stdout) << US_Vector::qs_vector3( "before usu interpolate original_q, original_i, original_i_error", original_q, original_i, original_i_error ) << "\n";
               if ( !usu.interpolate_iqq_by_case( original_q, original_i, original_i_error, q, ni, ni_error ) )
               {
                  editor_msg("red", usu.errormsg );
                  QMessageBox::warning( this, "US-SOMO",
                                        QString( us_tr( "There was an error attempting to interpolate\n"
                                                     "%1 q(%2:%3) to the common grid of q(%4:%5)\n"
                                                     "Error : \"%6\"" ) )
                                        .arg( qsl_d[ 0 ] )
                                        .arg( original_q.size() ? original_q[ 0 ] : 0 )
                                        .arg( original_q.size() ? original_q[ original_q.size() - 1 ] : 0 )
                                        .arg( q.size() ? q[ 0 ] : 0 )
                                        .arg( q.size() ? q[ q.size() - 1 ] : 0 )
                                        .arg( usu.errormsg )
                                        );
                  return;
               }
               // QTextStream(stdout) << US_Vector::qs_vector3( "after usu interpolate q, ni, ni_error", q, ni, ni_error ) << "\n";

               new_qsl << QString( "%1,\"I(q)\",%2" ).arg( qsl_d[ 0 ] ).arg( vector_double_to_csv( ni ) );
               if ( sd_map.count( qsl_d[ 0 ] ) )
               {
                  new_qsl_errors << QString( "%1,\"I(q) sd\",%2" ).arg( qsl_d[ 0 ] ).arg( vector_double_to_csv( ni_error ) );
               }
            }
            for ( unsigned int i = 0; i < (unsigned int) new_qsl_errors.size(); i++ )
            {
               new_qsl << new_qsl_errors[ i ];
            }
            qsl = new_qsl;
         }
      }
   } else {
      // QTextStream(stdout) << "reinterpolate empty grid target\n";
      q = plotted_q[0];
      header_tag = "Plotted I(q) curves";
      QString header = 
         QString("\"Name\",\"Type; q:\",%1,%2\n")
         .arg(vector_double_to_csv(q))
         .arg(header_tag);
      qsl << header;
      qsl_headers << header;
   }
   
   // ok, we have a header line
   
   // do we have multiple additonal files to plot ?
   // this needs to be gone over again in detail, since it is a copy of the prr version

   if ( filenames.size() > 1 )
   {
      map < QString, bool > interp_msg_done;
      for ( unsigned int i = 1; i < (unsigned int)filenames.size(); i++ )
      {
         TSO << QString("trying file %1 %2\n").arg(i).arg(filenames[i]);
         QFile f2(filenames[i]);
         if ( !f2.open(QIODevice::ReadOnly) )
         {
            QMessageBox::information( this, "UltraScan",
                                      us_tr("Can not open the file for reading:\n") +
                                      f2.fileName()
                                      );
            return;
         }
         
         // append (and possibly interpolate) all the other files
         QStringList qsl2;
         QTextStream ts(&f2);
         while ( !ts.atEnd() )
         {
            qsl2 << ts.readLine();
         }
         f2.close();
         QStringList qsl2_headers = qsl2.filter("\"Name\",\"Type; q:\"");
         
         qsl2_headers = qsl2.filter("\"Name\",\"Type; q:\"");
         if ( qsl2_headers.size() == 0 )
         {
            // QMessageBox mb(us_tr("UltraScan Warning"),
            //                us_tr("The csv file ") + f2.fileName() + us_tr(" does not appear to contain a correct header.\nPlease manually correct the csv file."),
            //                QMessageBox::Critical,
            //                QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
            // mb.exec();
            QMessageBox::critical( this,
                                   us_tr("UltraScan Warning"),
                                   us_tr("The csv file ") + f2.fileName() + us_tr(" does not appear to contain a correct header.\nPlease manually correct the csv file."),
                                   QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton );
            return;
         }
         if ( qsl2_headers.size() > 1 ) 
         {
            QString ref = qsl2_headers[0];
            for ( unsigned int i = 1; i < (unsigned int)qsl2_headers.size(); i++ )
            {
               if ( ref != qsl2_headers[i] )
               {
                  // QMessageBox mb(us_tr("UltraScan Warning"),
                  //                us_tr("The csv file ") + filename + us_tr(" contains multiple different headers\nPlease manually correct the csv file."),
                  //                QMessageBox::Critical,
                  //                QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
                  // mb.exec();
                  QMessageBox::critical( this,
                                         us_tr("UltraScan Warning"),
                                         us_tr("The csv file ") + filename + us_tr(" contains multiple different headers\nPlease manually correct the csv file."),
                                         QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton );
                  return;
               }
            }
         }
         // get the data
         map < QString, QString > name_to_errors_map;
         QStringList qsl_data = qsl2.filter(",\"I(q)\",");
         if ( qsl_data.size() == 0 )
         {
            // QMessageBox mb(us_tr("UltraScan Warning"),
            //                us_tr("The csv file ") + filename + us_tr(" does not appear to contain any data rows.\n"),
            //                QMessageBox::Critical,
            //                QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
            // mb.exec();
            QMessageBox::critical( this,
                                   us_tr("UltraScan Warning"),
                                   us_tr("The csv file ") + filename + us_tr(" does not appear to contain any data rows.\n"),
                                   QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton );
            return;
         }

         // get possible errors data
         QStringList qsl_errors = qsl2.filter(",\"I(q) sd\",");
         for ( QStringList::iterator it = qsl_errors.begin();
               it != qsl_errors.end();
               it++ )
         {
            QStringList qsl_iq_errors = (*it).split( "," );
            name_to_errors_map[ qsl_iq_errors[ 0 ] ] = *it;
         }

         // QTextStream(stdout) << US_Vector::qs_mapqsqs( "name_to_errors_map", name_to_errors_map ) << "\n";
         
         // get the q values
         QStringList qsl_q;
         QString header2_tag;
         vector < double > q2;
         
         qsl_q = (qsl2_headers[0]).split( "," );
         if ( qsl_q.size() < 3 )
         {
            // QMessageBox mb(us_tr("UltraScan Warning"),
            //                us_tr("The csv file ") + f2.fileName() + us_tr(" does not appear to contain any q values in the header rows.\n"),
            //                QMessageBox::Critical,
            //                QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
            // mb.exec();
            QMessageBox::critical( this,
                                   us_tr("UltraScan Warning"),
                                   us_tr("The csv file ") + f2.fileName() + us_tr(" does not appear to contain any q values in the header rows.\n"),
                                   QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton );
            return;
         }
         q2.push_back(qsl_q[2].toDouble());
         QStringList::iterator it = qsl_q.begin();
         it += 3;
         for ( ; it != qsl_q.end(); it++ )
         {
            if ( (*it).toDouble() > q2[q2.size() - 1] )
            {
               q2.push_back((*it).toDouble());
            } else {
               break;
            }
         }

         // possibly append more q values ?
         unsigned int max_size = q2.size();
         double max_value = q2[max_size - 1];
         if ( q.size() > 1 && q[q.size() - 1] < max_value )
         {
            double q_delta = q[1] - q[0];
            if ( q_delta > 0e0 )
            {
               for ( double value = q[q.size() -1] + q_delta;
                     value <= max_value;
                     value += q_delta )
               {
                  q.push_back(value);
               }
            }
         }
         
         if ( max_size > q.size() )
         {
            max_size = q.size();
         }                  
         // TSO << QString("max size %1\n").arg(max_size);
         bool all_match = true;
         for ( unsigned int i = 0; i < max_size; i++ )
         {
            if ( q[i] != q2[i] )
            {
               all_match = false;
               break;
            }
         }
         // TSO << QString("check all match %1\n").arg(all_match ? "true" : false);
         
         for ( QStringList::iterator it = qsl_data.begin();
               it != qsl_data.end();
               it++ )
         {
            QStringList qsl_iq = (*it).split( "," );
            if ( qsl_iq.size() < 3 )
            {
               QString msg = us_tr("The csv file ") + f2.fileName() + us_tr(" does not appear to contain sufficient I(q) values in data row " + qsl_iq[0] + ", skipping\n");
               editor_msg( "red", msg );
            } else {
               // build up new row to append to qsl
               if ( all_match )
               {
                  QString qs_tmp = *it;
                  // optionally? 
                  if ( qs_tmp.contains(QRegExp("(Average|Standard deviation)")) )
                  {
                     qs_tmp.replace(QRegExp("^\""),QString("\"%1: ").arg(QFileInfo(f2).baseName()));
                  }
                  qsl << qs_tmp;
                  // TSO << "yes, all match\nadding:" << qs_tmp << endl;
               } else {
                  // TSO << "not all match, interpolate\n";

                  if ( !interp_msg_done.count(f2.fileName()) )
                  {
                     interp_msg_done[f2.fileName()] = true;
                     QString msg = us_tr("The csv file ") + f2.fileName() + us_tr(" will be interpolated\n");
                     editor_msg( "dark red", msg );
                  }
                  // the new iq:
                  QStringList new_iq_fields;
                  // pull the data values
                  vector < double > this_iq;
                  for ( unsigned int i = 0; i < 2; i++ )
                  {
                     new_iq_fields.push_back(qsl_iq[i]);
                  }
                  {
                     QStringList::iterator it2 = qsl_iq.begin();
                     it2 += 2;
                     for ( ; it2 != qsl_iq.end(); it2++ )
                     {
                        this_iq.push_back((*it2).toDouble());
                     }
                  }
                  // interpolate q2, iq to r, reappend to new_iq_fields
                  vector < double > niq = interpolate(q, q2, this_iq);
                  QString line = QString("%1,%2\n")
                     .arg(new_iq_fields.join(","))
                     .arg(vector_double_to_csv(niq));
                  if ( line.contains(QRegExp("(Average|Standard deviation)")) )
                  {
                     line.replace(QRegExp("^\""),QString("\"%1: ").arg(QFileInfo(f2).baseName()));
                  }
                  qsl << line;
                  // also check to see if errors need interpolation
                  if ( name_to_errors_map.count( qsl_iq[ 0 ] ) != 0 )
                  {
                     // the new iq:
                     QStringList qsl_iq_errors = (name_to_errors_map[ qsl_iq[ 0 ] ]).split( "," );
                     QStringList new_iq_errors_fields;
                     // pull the data values
                     vector < double > this_iq_errors;
                     for ( unsigned int i = 0; i < 2; i++ )
                     {
                        new_iq_errors_fields.push_back(qsl_iq_errors[i]);
                     }
                     {
                        QStringList::iterator it2 = qsl_iq_errors.begin();
                        it2 += 2;
                        for ( ; it2 != qsl_iq_errors.end(); it2++ )
                        {
                           this_iq_errors.push_back((*it2).toDouble());
                        }
                     }
                     // interpolate q2, iq to r, reappend to new_iq_fields
                     vector < double > niq_errors = interpolate(q, q2, this_iq_errors);
                     QString line = QString("%1,%2\n")
                        .arg(new_iq_errors_fields.join(","))
                        .arg(vector_double_to_csv(niq_errors));
                     if ( line.contains(QRegExp("(Average|Standard deviation)")) )
                     {
                        line.replace(QRegExp("^\""),QString("\"%1: ").arg(QFileInfo(f2).baseName()));
                     }
                     qsl << line;
                  }
               }
            }
         }
      }            
   }
   
   // find cropping for all plotted curves
   double crop_min = 1e99;
   double crop_max = -1e99;
   bool found_cropping = false;
   
   for ( unsigned int i = 0; i < (unsigned int)qsl_plotted_iq_names.size(); i++ )
   {
      if ( plotted_q.size() )
      {
         if ( !found_cropping )
         {
            crop_min = plotted_q[ i ][ 0 ];
            crop_max = plotted_q[ i ][ plotted_q[ i ].size() - 1 ];
            found_cropping = true;
         } else {
            if ( crop_min < plotted_q[ i ][ 0 ] )
            {
               crop_min = plotted_q[ i ][ 0 ];
            }
            if ( crop_max > plotted_q[ i ][ plotted_q[ i ].size() - 1 ] )
            {
               crop_max = plotted_q[ i ][ plotted_q[ i ].size() - 1 ];
            }
         }
      }
   }

   if ( found_cropping )
   {
      // is this right? should't we be comparing across all curves?
      if ( crop_min > q[ 0 ] ||
           crop_max < q[ q.size() - 1 ] )
      {
         editor_msg( "dark red", 
                     QString("cropping loaded curves to plotted range intersection q(%1:%2)")
                     .arg( crop_min )
                     .arg( crop_max ) );
         
         US_Saxs_Util usu;

         vector < double > new_q;
         vector < double > new_I;
         vector < double > new_I_errors;
         vector < double > zero;

         if ( !usu.crop( q, q, zero, crop_min, crop_max, new_q, new_I_errors ) )
         {
            editor_msg( "red", usu.errormsg );
            return;
         }
         original_q = q;
         q = new_q;

         // reset header
         QStringList qsl_q = (qsl_headers[0]).split( "," );
         QString msg = qsl_q[ qsl_q.size() - 1 ];
         msg.replace("\"", "");
         qsl[0] = QString("\"Name\",\"Type; q:\",%1,\"%2 cropped to q(%3:%4)\"")
            .arg( vector_double_to_csv( q ) )
            .arg( msg )
            .arg( q.size() ? q[ 0 ] : 0 )
            .arg( q.size() ? q[ q.size() - 1 ] : 0 );
         qsl_headers[ 0 ] = qsl[ 0 ];
         TSO << qsl[ 0 ] << endl;
         QStringList new_qsl;
         QStringList new_qsl_errors;
         new_qsl << qsl[ 0 ];

         // also crop all the data lines
         QStringList qsl_data = qsl.filter(",\"I(q)\",");
         QStringList qsl_sd   = qsl.filter(",\"I(q) sd\",");
         map < QString, QString > sd_map;
         for ( unsigned int i = 0; i < (unsigned int) qsl_sd.size(); i++ )
         {
            QStringList qsl_s = (qsl_sd[i]).split( "," );
            sd_map[ qsl_s[ 0 ] ] = qsl_sd[ i ];
         }

         for ( unsigned int i = 0; i < (unsigned int) qsl_data.size(); i++ )
         {
            QStringList qsl_d = (qsl_data[i]).split( "," );
            vector < double > original_i;
            vector < double > original_i_error;
            for ( int j = 2; j < (int) qsl_d.size() - 1; j++ )
            {
               original_i.push_back( qsl_d[j].toDouble() );
            }
            if ( sd_map.count( qsl_d[ 0 ] ) )
            {
               QStringList qsl_s = (sd_map[ qsl_d[ 0 ] ]).split( "," );
               for ( int j = 2; j < (int) qsl_s.size() - 1; j++ )
               {
                  original_i_error.push_back( qsl_s[j].toDouble() );
               }
            }
            // now crop
            vector < double > ni;
            vector < double > ni_error;
            if ( !usu.crop( original_q, original_i, original_i_error, crop_min, crop_max, ni, ni_error ) )
            {
               editor_msg("red", usu.errormsg );
               return;
            }
            
            new_qsl << QString( "%1,\"I(q)\",%2" ).arg( qsl_d[ 0 ] ).arg( vector_double_to_csv( ni ) );
            if ( sd_map.count( qsl_d[ 0 ] ) )
            {
               new_qsl_errors << QString( "%1,\"I(q) sd\",%2" ).arg( qsl_d[ 0 ] ).arg( vector_double_to_csv( ni_error ) );
            }
         }
         for ( unsigned int i = 0; i < (unsigned int) new_qsl_errors.size(); i++ )
         {
            new_qsl << new_qsl_errors[ i ];
         }
         qsl = new_qsl;
      }
   }         

   // append all currently plotted I(q)s to qsl
   bool added_interpolate_msg = false;
   QString bin_msg = "";
   for ( unsigned int i = 0; i < (unsigned int)qsl_plotted_iq_names.size(); i++ )
   {
      // vector < double > nic = interpolate(q, plotted_q[i], plotted_I[i]);
      // vector < double > nic_errors = interpolate(q, plotted_q[i], plotted_I_error[i]);
      US_Saxs_Util usu;
      vector < double > nic;
      vector < double > nic_errors;

      // QTextStream(stdout)
      //    << "append all currently plotted I(q)s to qsl\n"
      //    << US_Vector::qs_vector3( "plotted q, I, I_error", plotted_q[i], plotted_I[i], plotted_I_error[i] )
      //    ;
      if ( !usu.interpolate_iqq_by_case( plotted_q[i], plotted_I[i], plotted_I_error[i], q, nic, nic_errors ) )
      {
         editor_msg("red", usu.errormsg );
         QMessageBox::warning( this, "US-SOMO",
                               QString( us_tr( "There was an error attempting to interpolate\n"
                                            "%1 q(%2:%3) to the common grid of q(%4:%5)\n"
                                            "Error: \"%6\"" ) )
                               .arg( qsl_plotted_iq_names[ i ] )
                               .arg( plotted_q[ i ].size() ? plotted_q[ i ][ 0 ] : 0 )
                               .arg( plotted_q[ i ].size() ? plotted_q[ i ][ plotted_q[ i ].size() - 1 ] : 0 )
                               .arg( q.size() ? q[ 0 ] : 0 )
                               .arg( q.size() ? q.back() : 0 )
                               .arg( usu.errormsg )
                               );
         return;
      }

      if ( !added_interpolate_msg && q.size() > 1 )
      {
         bin_msg = QString(us_tr("Plotted I(q) interpolated to delta q of %1")).arg(q[1] - q[0]);
         if ( plotted_q[i].size() > 1 && 
              q[1] - q[0] != plotted_q[i][1] - plotted_q[i][0] )
         {
            bin_msg +=
               QString(us_tr(" which is DIFFERENT from the plotted delta q of %1"))
               .arg(plotted_q[i][1] - plotted_q[i][0]);
         }
         // editor->append(bin_msg + "\n");
         added_interpolate_msg = true;
      }
      QString line = QString("\"%1\",\"I(q)\",%2\n")
         .arg(qsl_plotted_iq_names[i].replace( QRegExp( "(^[^\\\"]*\\\"|\\\"[^\\\"]*$)" ), "" ) )
         .arg(vector_double_to_csv(nic));
      qsl << line;
      if ( is_nonzero_vector( nic_errors ) )
      {
         QString line = QString("\"%1\",\"I(q) sd\",%2\n")
            .arg(qsl_plotted_iq_names[i].replace( QRegExp( "(^[^\\\"]*\\\"|\\\"[^\\\"]*$)" ), "" ) )
            .arg(vector_double_to_csv(nic_errors));
         qsl << line;
      }
   }
   
   if ( filenames.size() > 1 )
   {
      QStringList new_qsl;
      for ( unsigned int i = 0; i < (unsigned int)qsl.size(); i++ )
      {
         QString qs_tmp = qsl[i];
         if ( !qs_tmp.contains(QRegExp("(Average|Standard deviation)")) )
         {
            new_qsl << qsl[i];
         }
      }
      if ( new_qsl.size() != qsl.size() )
      {
         switch( QMessageBox::information( this, 
                                           us_tr("UltraScan"),
                                           us_tr("There are multiple average and/or standard deviation lines\n") +
                                           us_tr("What do you want to do?"),
                                           us_tr("&Skip"), 
                                           us_tr("&Just averages"),
                                           us_tr("&Include"),
                                           0,      // Enter == button 0
                                           1 ) ) { // Escape == button 2
         case 0: // skip them
            qsl = new_qsl;
            break;
         case 1: // just averages
            qsl = qsl.filter(QRegExp("(Average|Standard deviation)"));
            break;
         case 2: // Cancel clicked or Escape pressed
            break;
         }
      }
   }
   
   QStringList qsl_data = qsl.filter(",\"I(q)\",");

   if ( qsl_data.size() == 0 )
   {
      // QMessageBox mb(us_tr("UltraScan Warning"),
      //                us_tr("The csv file ") + filename + us_tr(" does not appear to contain any data rows.\n"),
      //                QMessageBox::Critical,
      //                QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
      // mb.exec();
      QMessageBox::critical( this,
                             us_tr("UltraScan Warning"),
                             us_tr("The csv file ") + filename + us_tr(" does not appear to contain any data rows.\n"),
                             QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton );
      return;
   }
   if ( !just_plotted_curves )
   {
      header_tag = qsl_q.last();
   }
   
   // get possible errors data
   map < QString, QString > name_to_errors_map;
   QStringList qsl_errors = qsl.filter(",\"I(q) sd\",");
   for ( QStringList::iterator it = qsl_errors.begin();
         it != qsl_errors.end();
         it++ )
   {
      QStringList qsl_iq_errors = (*it).split( "," );
      name_to_errors_map[ qsl_iq_errors[ 0 ] ] = *it;
   }

   // build a list of names
   QStringList qsl_names;
   for ( QStringList::iterator it = qsl_data.begin();
         it != qsl_data.end();
         it++ )
   {
      QStringList qsl_tmp = (*it).split( "," );
      qsl_names << qsl_tmp[0];
   }
   
   // ask for the names to load if more than one present (cb list? )
   QStringList qsl_sel_names;
   bool create_avg          = false;
   bool create_std_dev      = false;
   bool only_plot_stats     = true;
   save_to_csv              = false;
   csv_filename             = "summary";
   bool save_original_data  = false;
   bool run_nnls            = false;
   bool nnls_csv            = false;
   bool run_best_fit        = false;
   bool run_ift             = false;
   use_SDs_for_fitting_iqq  = true;
   nnls_plot_contrib        = false;
   QString nnls_target      = "";
   if ( !grid_target.isEmpty() )
   {
      nnls_target = "\"" + grid_target + "\"";
   }
   bool clear_plot_first    = true;

   US_Hydrodyn_Saxs_Iqq_Load_Csv *hslc =
      new US_Hydrodyn_Saxs_Iqq_Load_Csv(
                                        "Select models to load\n" + header_tag + 
                                        (bin_msg.isEmpty() ? "" : "\n" + bin_msg),
                                        &qsl_names,
                                        &qsl_sel_names,
                                        &qsl,
                                        filename,
                                        &create_avg,
                                        &create_std_dev,
                                        &only_plot_stats,
                                        &save_to_csv,
                                        &csv_filename,
                                        &save_original_data,
                                        &run_nnls,
                                        &nnls_plot_contrib,
                                        &nnls_csv,
                                        &run_best_fit,
                                        &run_ift,
                                        &use_SDs_for_fitting_iqq,
                                        &nnls_target,
                                        &clear_plot_first,
                                        1 || U_EXPT,
                                        us_hydrodyn
                                        );
   US_Hydrodyn::fixWinButtons( hslc );
   hslc->exec();
   
   delete hslc;
   
   this->isVisible() ? this->raise() : this->show();
   
   if ( qsl_sel_names.size() && clear_plot_first ) {
      clear_plot_saxs_data();
   }

   // make sure target is selected
   
   if ( ( run_nnls || run_best_fit ) &&
        !qsl_sel_names.filter(nnls_target).size() )
   {
      // TSO << "had to add target back\n";
      qsl_sel_names << nnls_target;
   }
   
   // ok, now qsl_sel_names should have the load list
   // loop through qsl_data and match up 
   // create a map to avoid a double loop
   
   map < QString, bool > map_sel_names;
   for ( QStringList::iterator it = qsl_sel_names.begin();
         it != qsl_sel_names.end();
         it++ )
   {
      map_sel_names[*it] = true;
   }

   // check for scaling target

   QString scaling_target = "";
   if ( qsl_sel_names.size() && !run_ift )
   {
      set_scaling_target( scaling_target );
   }         
   
   // setup for average & stdev
   vector < double > sum_iq(q.size());
   vector < double > sum_iq2(q.size());
   
   unsigned int sum_count = 0;
   for ( unsigned int i = 0; i < q.size(); i++ )
   {
      sum_iq[i] = sum_iq2[i] = 0e0;
   }
   
   // setup for ift
   if ( run_ift ) {
      ift_to_process = qsl_sel_names;
      ift_to_process.replaceInStrings( QRegExp( "^\""), "" ).replaceInStrings( QRegExp( "\"$"), "" );
      QTextStream( stdout ) << "ift_to_process:\n" << ift_to_process.join( "\n" ) << Qt::endl;
      call_ift( true );
      return;
   }

   QString nnls_csv_filename;
   nnls_csv_data.clear();
   nnls_csv_footer.clear();
      
   // setup for nnls
   if ( run_nnls || run_best_fit )
   {
      if ( run_nnls )
      {
         editor->append("NNLS target: " + nnls_target + "\n");
         nnls_csv_footer << "\"NNLS target:\"," + nnls_target;
         
         if ( nnls_csv ) {
            QString use_dir = USglobal->config_list.root_dir + "/" + "somo" + "/" + "saxs";

            // ((US_Hydrodyn *)us_hydrodyn)->select_from_directory_history( use_dir, this );

            nnls_csv_filename =
               QFileDialog::getSaveFileName(
                                            this
                                            ,us_tr("Choose a filename to save the NNLS fit")
                                            ,use_dir
                                            + "/"
                                            + QFileInfo( QString( "%1" ).arg( nnls_target ).replace( "\"", "" ) ).baseName()
                                            + "_NNLS_results.csv"
                                            ,"*.csv" );

            if ( nnls_csv_filename.isEmpty() ) {
               editor_msg( "red", us_tr( "Canceling NNLS CSV save" ) );
               nnls_csv = false;
            }
            if ( !nnls_csv_filename.contains(QRegExp(".csv", Qt::CaseInsensitive )) )
            {
               nnls_csv_filename += ".csv";
            }

         }
      }
      if ( run_best_fit )
      {
         editor->append("Best fit target: " + nnls_target + "\n");
      }
      nnls_A.clear( );
      nnls_x.clear( );
      nnls_mw.clear( );
      nnls_B.clear( );
      nnls_B_name = nnls_target;
      nnls_rmsd = 0e0;
   }
   
   bool found_nnls_target = false;
   bool found_nnls_model  = false;

   double avg_rescaling_multiplier = 0e0;
   double avg_rescaling_offset     = 0e0;
   bool   rescaling_warning_shown  = false;
   
   // now go through qsl_data and load up any that map_sel_names contains
   plotted = false;
   for ( QStringList::iterator it = qsl_data.begin();
         it != qsl_data.end();
         it++ )
   {
      QStringList qsl_tmp = (*it).split( "," );
      if ( map_sel_names.count(qsl_tmp[0]) )
      {
         // QTextStream(stdout) << "loading: " << qsl_tmp[0] << "\n" << "-----data: " << (*it) << "\n";
         
         I.clear( );
         I_errors.clear( );
         
         // get the Iq values
         
         QStringList qsl_iq = (*it).split( "," );
         if ( qsl_iq.size() < 3 )
         {
            QString msg = us_tr("The csv file ") + filename + us_tr(" does not appear to contain sufficient I(q) values in data row " + qsl_tmp[0] + "\n");
            editor_msg( "red", msg );
            // QMessageBox mb(us_tr("UltraScan Warning"),
            //                msg,
            //                QMessageBox::Critical,
            //                QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
            // mb.exec();
            QMessageBox::critical( this,
                                   us_tr("UltraScan Warning"),
                                   msg,
                                   QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton );
            break;
         }
         qsl_data_lines_plotted << *it;

         QStringList::iterator it = qsl_iq.begin();
         it += 2;
         for ( ; it != qsl_iq.end(); it++ )
         {
            I.push_back((*it).toDouble());
         }
         I.pop_back();

         if ( name_to_errors_map.count( qsl_iq[ 0 ] ) != 0 )
         {
            QStringList qsl_iq_errors = (name_to_errors_map[ qsl_iq[ 0 ] ]).split( "," );
            QStringList::iterator it = qsl_iq_errors.begin();
            it += 2;
            for ( ; it != qsl_iq_errors.end(); it++ )
            {
               I_errors.push_back((*it).toDouble());
            }
            I_errors.pop_back();
         }            

         if ( run_nnls || run_best_fit )
         {
            // TSO << QString("US_Hydrodyn_Saxs::load_iqq_csv %1 size %2\n").arg(qsl_tmp[0]).arg(I.size());
            if ( qsl_tmp[0] == nnls_target )
            {
               found_nnls_target = true;
               nnls_B = range_crop( q, I );
               nnls_q = range_crop( q, q );
               nnls_errors = range_crop( q, I_errors );
            } else {
               found_nnls_model = true;
               nnls_A[qsl_tmp[0]] = range_crop( q, I );
               nnls_x[qsl_tmp[0]] = 0;
            }
         }

         // plot it
         vector < double > this_q = q;
         // q has the ordinates for the longest data, some will likely be shorter
         if ( q.size() > I.size() )
         {
            this_q.resize(I.size());
         }
         // occasionally one may have a zero in the last p(r) position (?) not for I(q)?
         if ( I.size() > q.size() )
         {
            I.resize(q.size());
         }
         
         for ( unsigned int i = 0; i < I.size(); i++ )
         {
            sum_iq[i] += I[i];
            sum_iq2[i] += I[i] * I[i];
            if ( us_isnan(I[i]) ) 
            {
               TSO << QString("WARNING: isnan I[%1] for %2\n").arg(i).arg(qsl_tmp[0]);
            }
            if ( us_isnan(sum_iq[i]) ) 
            {
               TSO << QString("WARNING: isnan sum_iq[%1] for %2\n").arg(i).arg(qsl_tmp[0]);
            }
            if ( us_isnan(sum_iq2[i]) ) 
            {
               TSO << QString("WARNING: isnan sum_iq2[%1] for %2\n").arg(i).arg(qsl_tmp[0]);
            }
         }
         sum_count++;
         
         if ( !(create_avg && only_plot_stats) && !run_nnls && !run_best_fit )
         {
            if ( !scaling_target.isEmpty() && 
                 plotted_iq_names_to_pos.count(scaling_target) )
            {
               if ( ( qsl_tmp[0].contains( "Average minus 1 standard deviation\"" ) ||
                      qsl_tmp[0].contains( "Average plus 1 standard deviation\"" ) ) )
               {
                  puts("a 1 sd");
                  if ( avg_rescaling_multiplier != 0e0 )
                  {
                     last_rescaling_multiplier = avg_rescaling_multiplier;
                     last_rescaling_offset     = avg_rescaling_offset;
                     rescale_iqq_curve_using_last_rescaling( I );
                  } else {
                     if ( !rescaling_warning_shown )
                     {
                        QMessageBox::warning( this, "UltraScan",
                                              QString( us_tr(
                                                          "Standard deviation curves require an Average curve to be plotted first for correct rescaling\n"
                                                          "Since an Average curve was not plotted first, they will simply be scaled as a best fit to the selected target\n"
                                                          ) ) );
                        rescaling_warning_shown = true;
                     }
                     rescale_iqq_curve( scaling_target, this_q, I );
                  }
               } else {
                  rescale_iqq_curve( scaling_target, this_q, I );
               }
               if ( I_errors.size() )
               {
                  rescale_iqq_curve_using_last_rescaling( I_errors );
               }
               if ( qsl_tmp[0].contains( "Average\"" ) )
               {
                  puts("avg save");
                  avg_rescaling_multiplier = last_rescaling_multiplier;
                  avg_rescaling_offset     = last_rescaling_offset;
               }
            }
                  
            if ( I_errors.size() )
            {
               plot_one_iqq(this_q, I, I_errors, QFileInfo(filename).fileName() + " " + qsl_tmp[0]);
            } else {
               plot_one_iqq(this_q, I, QFileInfo(filename).fileName() + " " + qsl_tmp[0]);
            }
         }
      }
   }
   
   if ( create_avg && sum_count && !run_nnls && !run_best_fit )
   {
      I = sum_iq;
      for ( unsigned int i = 0; i < sum_iq.size(); i++ )
      {
         I[i] /= (double)sum_count;
      }
      vector < double > this_q = q;
      if ( q.size() > I.size() )
      {
         this_q.resize(I.size());
      }
      if ( I.size() > q.size() )
      {
         I.resize(q.size());
      }
      vector < double > iq_avg = I;

      if ( !scaling_target.isEmpty() && 
           plotted_iq_names_to_pos.count(scaling_target) )
      {
         rescale_iqq_curve( scaling_target, this_q, I );
      }
      
      vector < double > iq_std_dev;
      vector < double > iq_avg_minus_std_dev;
      vector < double > iq_avg_plus_std_dev;
      
      if ( create_std_dev && sum_count > 2 )
      {
         vector < double > std_dev(sum_iq.size());
         for ( unsigned int i = 0; i < sum_iq.size(); i++ )
         {
            double tmp_std_dev = 
               sum_iq2[i] - ((sum_iq[i] * sum_iq[i]) / (double)sum_count);
            std_dev[i] = 
               tmp_std_dev > 0e0 ?
               sqrt( ( 1e0 / ((double)sum_count - 1e0) ) * tmp_std_dev ) : 0e0;
            if ( us_isnan(sum_iq[i]) ) 
            {
               TSO << QString("WARNING when calc'ing std dev: isnan sum_iq[%1]\n").arg(i);
            }
            if ( us_isnan(sum_iq2[i]) ) 
            {
               TSO << QString("WARNING when calc'ing std dev: isnan sum_iq2[%1]\n").arg(i);
            }
            if ( us_isnan(std_dev[i]) ) 
            {
               TSO << 
                  QString("WARNING when calc'ing std dev: isnan std_dev[%1]:\n"
                          " sum_iq[%2]  == %3\n"
                          " sum_iq2[%4] == %5\n"
                          " sum_count   == %6\n"
                          " sum_iq2 - ((sum_iq * sum_iq) / sum_count)   == %7\n"
                          )
                  .arg(i)
                  .arg(i).arg(sum_iq[i])
                  .arg(i).arg(sum_iq2[i])
                  .arg(sum_count)
                  .arg(sum_iq2[i] - ((sum_iq[i] * sum_iq[i]) / (double)sum_count) )
                  ;
            }
         }

         iq_std_dev = std_dev;
         
         if ( !scaling_target.isEmpty() && 
              plotted_iq_names_to_pos.count(scaling_target) )
         {
            rescale_iqq_curve_using_last_rescaling( iq_avg );
            rescale_iqq_curve_using_last_rescaling( iq_std_dev );
         }

         plot_one_iqq(this_q, iq_avg, iq_std_dev, QFileInfo(filename).fileName() + " Average");
         
         I = sum_iq;

         for ( unsigned int i = 0; i < sum_iq.size(); i++ )
         {
            I[i] /= (double)sum_count;
            I[i] -= std_dev[i];
         }
         vector < double > this_q = q;
         if ( q.size() > I.size() )
         {
            this_q.resize(I.size());
         }
         if ( I.size() > q.size() )
         {
            I.resize(q.size());
         }
         iq_avg_minus_std_dev = I;
         
         if ( !scaling_target.isEmpty() && 
              plotted_iq_names_to_pos.count(scaling_target) )
         {
            rescale_iqq_curve_using_last_rescaling( I );
         }

         plot_one_iqq(this_q, I, QFileInfo(filename).fileName() + " Average minus 1 standard deviation");
         
         I = sum_iq;
         for ( unsigned int i = 0; i < sum_iq.size(); i++ )
         {
            I[i] /= (double)sum_count;
            I[i] += std_dev[i];
         }
         this_q = q;
         if ( q.size() > I.size() )
         {
            this_q.resize(I.size());
         }
         if ( I.size() > q.size() )
         {
            I.resize(q.size());
         }
         iq_avg_plus_std_dev = I;
         
         if ( !scaling_target.isEmpty() && 
              plotted_iq_names_to_pos.count(scaling_target) )
         {
            rescale_iqq_curve_using_last_rescaling( I );
         }
         plot_one_iqq(this_q, I, QFileInfo(filename).fileName() + " Average plus 1 standard deviation");
      } else {
         plot_one_iqq(this_q, I, QFileInfo(filename).fileName() + " Average");
      }
      if ( plotted )
      {
         editor_msg( "black", "I(q) plot done\n");
         plotted = false;
      }
      if ( save_to_csv )
      {
         // TSO << "save_to_csv\n";
         QString fname = 
            ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "saxs" + SLASH + 
            csv_filename + "_iqq.csv";
         if ( QFile::exists(fname) )
            // && !((US_Hydrodyn *)us_hydrodyn)->overwrite ) 
         {
            fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fname);
         }         
         FILE *of = us_fopen(fname, "wb");
         if ( of )
         {
            //  header: "name","type",r1,r2,...,rn, header info
            fprintf(of, "\"Name\",\"Type; q:\",%s,%s\n", 
                    vector_double_to_csv(q).toLatin1().data(),
                    header_tag.toLatin1().data());
            if ( save_original_data )
            {
               fprintf(of, "%s\n", qsl_data_lines_plotted.join("\n").toLatin1().data());
            }
            fprintf(of, "\"%s\",\"%s\",%s\n", 
                    "Average",
                    "I(q)",
                    vector_double_to_csv(iq_avg).toLatin1().data());
            if ( iq_std_dev.size() )
            {
               fprintf(of, "\"%s\",\"%s\",%s\n", 
                       "Average",
                       "I(q) sd",
                       vector_double_to_csv(iq_std_dev).toLatin1().data());
               fprintf(of, "\"%s\",\"%s\",%s\n", 
                       "Standard deviation",
                       "I(q)",
                       vector_double_to_csv(iq_std_dev).toLatin1().data());
               fprintf(of, "\"%s\",\"%s\",%s\n", 
                       "Average minus 1 standard deviation",
                       "I(q)",
                       vector_double_to_csv(iq_avg_minus_std_dev).toLatin1().data());
               fprintf(of, "\"%s\",\"%s\",%s\n", 
                       "Average plus 1 standard deviation",
                       "I(q)",
                       vector_double_to_csv(iq_avg_plus_std_dev).toLatin1().data());
            }
            if ( !save_original_data )
            {
               fprintf(of, "\n\n\"%s\"\n", 
                       QString(" Average of : " + qsl_sel_names.join(";").replace("\"","")).toLatin1().data()
                       );
            }
            fclose(of);
            editor->append(us_tr("Created file: " + fname + "\n"));
         } else {
            editor_msg( "red", us_tr("ERROR creating file: " + fname + "\n"));
         }
      }
   } else {
      if ( run_nnls || run_best_fit )
      {
         nnls_r = range_crop( q, q );
         if ( found_nnls_model && found_nnls_target )
         {
            nnls_header_tag = header_tag;
            QString use_csv_filename = save_to_csv ? csv_filename : "";
            if ( run_nnls )
            {
               if ( our_saxs_options->iqq_scale_minq ||
                    our_saxs_options->iqq_scale_maxq )
               {
                  editor_msg( "dark red", 
                              QString( us_tr( "Note: the NNLS fit will be performed over a cropped range q(%1:%2)" ) )
                              .arg( our_saxs_options->iqq_scale_minq ? QString( "%1" ).arg( our_saxs_options->iqq_scale_minq ) : "" )
                              .arg( our_saxs_options->iqq_scale_maxq ? QString( "%1" ).arg( our_saxs_options->iqq_scale_maxq ) : "" ) );
                  nnls_csv_footer <<
                     QString( us_tr( "\"Note: the NNLS fit will be performed over a cropped range q\",%1,%2" ) )
                     .arg( our_saxs_options->iqq_scale_minq ? QString( "%1" ).arg( our_saxs_options->iqq_scale_minq ) : "" )
                     .arg( our_saxs_options->iqq_scale_maxq ? QString( "%1" ).arg( our_saxs_options->iqq_scale_maxq ) : "" );
                     
               }

               calc_iqq_nnls_fit( nnls_target, use_csv_filename );

               if ( nnls_csv ) {
                  if ( QFile::exists(nnls_csv_filename) )
                  {
                     nnls_csv_filename = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( nnls_csv_filename, 0, this );
                  }

                  QFile f( nnls_csv_filename );
                  if ( f.open(QIODevice::WriteOnly ) ) {
                     QTextStream tso(&f);
                     tso << "\"File\",\"Model\",\"Contribution weight\"\n";
                     tso << nnls_csv_data.join("\n") << "\n";
                     tso << "\n\"Messages:\"\n";
                     tso << nnls_csv_footer.join("\n") << "\n";
                     f.close();
                     editor_msg( "darkblue", QString( us_tr( "Created file: %1\n" ) ).arg( nnls_csv_filename ) );
                  } else {
                     editor_msg( "red", QString( us_tr( "Error attempting to create file: %1\n" ) ).arg( nnls_csv_filename ) );
                  }
               }
            }
            if ( run_best_fit )
            {
               if ( our_saxs_options->iqq_scale_minq ||
                    our_saxs_options->iqq_scale_maxq )
               {
                  editor_msg( "dark red", 
                              QString( us_tr( "Note: the fit will be performed over a cropped range q(%1:%2)" ) )
                              .arg( our_saxs_options->iqq_scale_minq ? QString( "%1" ).arg( our_saxs_options->iqq_scale_minq ) : "" )
                              .arg( our_saxs_options->iqq_scale_maxq ? QString( "%1" ).arg( our_saxs_options->iqq_scale_maxq ) : "" ) );
               }

               calc_iqq_best_fit( nnls_target, use_csv_filename );
            }
         } else {
            editor->append("NNLS error: could not find target and models in loaded data\n");
         }
      }
      if ( plotted )
      {
         editor_msg( "black", "I(q) plot done\n");
         plotted = false;
      }
   }
   rescale_plot();
   return;
}

bool US_Hydrodyn_Saxs::load_check_csvs_compatible( QStringList filenames ) {
   // check header line for grid and make sure existing grid elements exist

   QStringList refgrid;

   for ( int i = 0; i < filenames.size(); ++i ) {
      QString f = filenames[i];
      QString contents;
      QString error;
      if ( !US_File_Util::getcontents( f, contents, error ) ) {
         qDebug() << "load_check_csvs_compatible failed to get contents " << error;
         return false;
      }
      QStringList lines = contents.split( "\n" );
      if ( !lines.size() ) {
         qDebug() << "load_check_csvs_compatible failed no lines " << f;
         return false;
      }
         
      if ( !lines[0].contains( "\"Type; r:\"" ) ) {
         qDebug() << "found I(q), terminating\n";
         return false;
      }

      QStringList grid  = lines[0].split( "," );
      if ( !grid.size() ) {
         qDebug() << "load_check_csvs_compatible failed first line empty grid " << f;
         return false;
      }      
      grid.removeLast();
      grid.removeLast();
      // qDebug() << QString( "file: %1 gridsize: %2" ).arg( f ).arg( grid.size() );
      if ( !refgrid.size() ) {
         refgrid = grid;
      } else {
         int mingrid = refgrid.size() > grid.size() ? grid.size() : refgrid.size();
         for ( int j = 0; j < mingrid; ++j ) {
            if ( grid[j] != refgrid[j] ) {
               qDebug() << "load_check_csvs_compatible failed incompatible grids " << f;
               return false;
            }
         }
         if ( refgrid.size() < grid.size() ) {
            refgrid = grid;
         }
      }
   }
   // qDebug() << QString( "ref gridsize: %1" ).arg( refgrid.size() );

   return true;
}

QString US_Hydrodyn_Saxs::unify_csv_files( QStringList filenames ) {
   // find max grid length in csv && join files

   // changes:
   // if all i(q) what then ?
   // pad zeros on loading instead of here?

   QStringList refgrid;

   for ( int i = 0; i < filenames.size(); ++i ) {
      QString f = filenames[i];
      QString contents;
      QString error;
      if ( !US_File_Util::getcontents( f, contents, error ) ) {
         qDebug() << "unify_csv_files failed to get contents " << error;
         return "error";
      }
      QStringList lines = contents.split( "\n" );
      if ( !lines.size() ) {
         qDebug() << "unify_csv_files failed no lines " << f;
         return "error";
      }
         
      if ( !lines[0].contains( "\"Type; r:\"" ) ) {
         qDebug() << "found I(q), terminating\n";
         return "error";
      }
         
      QStringList grid  = lines[0].split( "," );
      if ( !grid.size() ) {
         qDebug() << "unify_csv_files failed first line empty grid " << f;
         return "error";
      }      
      grid.removeLast();
      grid.removeLast();
      // qDebug() << QString( "file: %1 gridsize: %2" ).arg( f ).arg( grid.size() );
      if ( !refgrid.size() ) {
         refgrid = grid;
      } else {
         int mingrid = refgrid.size() > grid.size() ? grid.size() : refgrid.size();
         for ( int j = 0; j < mingrid; ++j ) {
            if ( grid[j] != refgrid[j] ) {
               qDebug() << "unify_csv_files failed incompatible grids " << f;
               return "error";
            }
         }
         if ( refgrid.size() < grid.size() ) {
            refgrid = grid;
         }
      }
   }
   
   // qDebug() << QString( "ref gridsize: %1" ).arg( refgrid.size() );

   // now we have an appropriate refgrid, we can create a temp file & pad as in checkjscv.pl

   QString header = refgrid.join(",") + ",,\"SOMO SAXS P(r) vs r data generated by unification of multiple csv files";

   QStringList out;
   out << header;

   set < QString > names_used;

   for ( int i = 0; i < filenames.size(); ++i ) {
      QString f = filenames[i];
      qDebug() << "processing filename " << f;
      QString contents;
      QString error;
      if ( !US_File_Util::getcontents( f, contents, error ) ) {
         qDebug() << "unify_csv_files failed to get contents " << error;
         return "error";
      }
      QStringList lines = contents.split( "\n" );
      if ( !lines.size() ) {
         qDebug() << "unify_csv_files failed no lines " << f;
         return "error";
      }

      lines.removeFirst();
      
      if ( !lines.size() ) {
         qDebug() << "unify_csv_files failed not more than one line " << f;
         return "error";
      }

      for ( int j = 0; j < lines.size(); ++j ) {
         QString     line   = lines[j];
         QStringList fields = line.split( "," );
         fields.removeLast();
         if ( !fields.size() ) {
            out << line;
            continue;
         }

         // qDebug() << "name " << fields[0];

         if ( line.contains( "P(r) normed" ) ) {
            // sync with ::csv_p2iq()
            if( names_used.count( fields[0] ) ) {
               editor_msg( "red", QString( "unified csv load - duplicate name exists, %1, dropping this data\n" ).arg( fields[0] ) );
               continue;
            }
            names_used.insert( fields[0] );
         }

         if ( fields.size() > refgrid.size() ) {
            qDebug() << "unify_csv_files failed line " << j << " has more fields than header " << f;
            QTextStream( stdout )
               << "header: " << refgrid.size() << "\n"
               << refgrid.join( "," ) << "\n"
               << "line  : " << fields.size() << "\n"
               << fields.join( "," ) << "\n"
               ;
               
            return "error";
         }
         // if( fields.size() < refgrid.size() ) {
         //    qDebug() << "padding line " << j;
         // }
         while( fields.size() < refgrid.size() ) {
            fields.append( "0" );
         }
         // clear out negatives
         for ( int k = 4; k < fields.size(); ++k ) {
            if ( fields[k].left(1) == "-" ) {
               fields[k] = "0";
            }
         }
         out << fields.join( "," );
      }
   }

   {
      // write results
      QString outname;
      QString msg;
      QString basename = "unified_saxs_csv";
      QString ext      = "csv";
      QString data     = out.join( "\n" );
      if ( !US_File_Util::writeuniq( outname, msg, ((US_Hydrodyn *)us_hydrodyn)->somo_tmp_dir + QDir::separator() + basename, ext, data ) ) {
         qDebug() << msg;
         return "error";
      }
      editor_msg( "darkblue", QString( "Wrote %1" ).arg( outname ) ); 
      return outname;
   }
}


void US_Hydrodyn_Saxs::load_saxs( QString filename, bool just_plotted_curves, QString scaleto, bool no_scaling )
{
   if ( just_plotted_curves )
   {
      load_iqq_csv( "", true );
      return;
   }

   if ( filename.isEmpty() )
   {
      QString use_dir = 
         our_saxs_options->path_load_saxs_curve.isEmpty() ?
         USglobal->config_list.root_dir + SLASH + "somo" + SLASH + "saxs" :
         our_saxs_options->path_load_saxs_curve;
      select_from_directory_history( use_dir, this );

      
      QStringList filenames =
         QFileDialog::getOpenFileNames( this , "Open" , use_dir , "All files (*);;"
                                       "ssaxs files (*.ssaxs);;"
                                       "csv files (*.csv);;"
                                       "int files [crysol] (*.int);;"
                                       "dat files [foxs / other] (*.dat);;"
                                       "fit files [crysol] (*.fit)" , &load_saxs_sans_selected_filter );
      if ( filenames.size() == 0 ) {
         return;
      }

      if ( filenames.size() == 1 ) {
         filename = filenames[0];
      } else {
         // multiple files loaded
         set_scaling_target( scaleto );

         if ( filenames.size() == filenames.filter( QRegExp( ".csv$", Qt::CaseInsensitive ) ).size() ) {
            qDebug() << "all csv\n";
            if ( load_check_csvs_compatible( filenames ) ) {
               qDebug() << "all csv compatible\n";
               QString outname = unify_csv_files( filenames );
               if ( outname != "error" ) {
                  return load_saxs( outname );
               }
            } else {
               qDebug() << "all csv but not compatible\n";
            }
         } else {
            qDebug() << "not all csv\n";
         }
            
         for ( int i = 0; i < (int) filenames.size(); ++i ) {
            add_to_directory_history( filenames[i] );
            load_saxs( filenames[i], false, scaleto, scaleto.isEmpty() );
         }
         return;
      }
      
      if ( filename.isEmpty() )
      {
         return;
      }
      add_to_directory_history( filename );
   }

   plotted = false;
   QFile f(filename);
   our_saxs_options->path_load_saxs_curve = QFileInfo(filename).absolutePath();
   QString ext = QFileInfo(filename).suffix().toLower();

   if ( ext == "pdb" || ext == "PDB" )
   {
      QMessageBox::warning( this, "UltraScan",
                            QString(us_tr("Can not load a PDB file as a curve: ")
                                    + filename ) );
      return;
   }

   if ( ext == "csv" )
   {
      load_iqq_csv( filename );
      return;
   }

   vector < double > I;
   vector < double > I_error;
   vector < double > I2;
   vector < double > q;
   vector < double > q2;
   double new_I;
   double new_I_error;
   double new_I2;
   double new_q;
   unsigned int Icolumn = 1;
   unsigned int I_errorcolumn = 0;
   bool dolog10 = false;
   QString res = "";
   unsigned int Icolumn2 = 0;
   QString tag1;
   QString tag2;

   // scaling fields
   QString scaling_target = "";

   // bool do_crop = false;

   if ( f.open(QIODevice::ReadOnly) )
   {
      QTextStream ts(&f);
      vector < QString > qv;
      QStringList qsl;
      while ( !ts.atEnd() )
      {
         QString qs = ts.readLine();
         qv.push_back(qs);
         qsl << qs;
      }
      f.close();
      if ( !qv.size() )
      {
         QMessageBox::warning( this, "UltraScan",
                               QString(us_tr("The file ")
                                       + filename + us_tr(" is empty.")) );
         return;
      }

      unsigned int number_of_fields = 0;
      if ( qv.size() > 3 )
      {
         QString test_line = qv[2];
         test_line.replace(QRegExp("^\\s+"),"");
         test_line.replace(QRegExp("\\s+$"),"");
         QStringList test_list = (test_line).split( QRegExp("\\s+") , Qt::SkipEmptyParts );
         number_of_fields = test_list.size();
         TSO << "number of fields: " << number_of_fields << endl;
      }

      if ( scaleto.isEmpty() && !no_scaling ) {
         set_scaling_target( scaling_target );
      } else {
         scaling_target = scaleto;
      }

      // us_qdebug( QString( "scaling target is %1" ).arg( scaling_target ) );

      if ( ext == "int" ) 
      {
         //         dolog10 = true;
         QStringList lst;
         lst << "I(q)   Difference intensity"
             << "Ia(q)  Atomic scattering"
             << "Ic(q)  Shape scattering"
             << "Ib(q)  Border layer scattering Ib(q)";
         bool ok;
         
         if ( !our_saxs_options->crysol_default_load_difference_intensity )
         {
            res = US_Static::getItem(
                                        "Crysol's .int format has four available datasets", 
                                        "Select the set you wish to plot::", lst, 0, false, &ok,
                                        this );
            if ( ok ) {
               // user selected an item and pressed OK
               Icolumn = 0;
               if ( res.contains(QRegExp("^I.q. ")) ) 
               {
                  Icolumn = 1;
               } 
               if ( res.contains(QRegExp("^Ia.q. ")) ) 
               {
                  Icolumn = 2;
               } 
               if ( res.contains(QRegExp("^Ic.q. ")) ) 
               {
                  Icolumn = 3;
               } 
               if ( res.contains(QRegExp("^Ib.q. ")) ) 
               {
                  Icolumn = 4;
               } 
               if ( !Icolumn ) 
               {
                  cerr << "US_Hydrodyn_Saxs::load_saxs : unknown type error" << endl;
                  return;
               }
               TSO << " column " << Icolumn << endl;
            } 
            else
            {
               return;
            }
         } else {
            Icolumn = 1;
         }
      }
      if ( ext == "dat" || ext == "txt" || ext == "sprr" ) 
      {
         // foxs?
         // do_crop = true;

         Icolumn = 1;
         I_errorcolumn = 2;
         if ( qsl.filter("exp_intensity").size() )
         {
            I_errorcolumn = 0;
            
            switch ( QMessageBox::question(this, 
                                           us_tr("UltraScan Notice"),
                                           QString(us_tr("Please note:\n\n"
                                                      "The file appears to have both experiment and model data\n"
                                                      "What would you like to do?\n"))
                                           ,
                                           us_tr("&Load only experimental"),
                                           us_tr("&Load only the model"),
                                           us_tr("&Load both"),
                                           2, // Default
                                           0 // Escape == button 0
                                           ) )
            {
            case 0 : 
               Icolumn = 1;
               tag1 = " Experimental";
               break;
            case 1 : 
               Icolumn = 2;
               tag1 = " Model";
               break;
            case 2 : 
               Icolumn = 1;
               Icolumn2 = 2;
               tag1 = " Experimental";
               tag2 = " Model";
               break;
            default :
               // what happended here?
               return;
               break;
            }
         }             
      }
      if ( ext == "fit" ) 
      {
         // do_crop = true;

         Icolumn = 2;
         I_errorcolumn = 0;
      }
      if ( ext == "ssaxs" ) 
      {
         //         dolog10 = true;
         if ( number_of_fields >= 4 )
         {
            QStringList lst;
            lst << "I(q)   Difference intensity"
                << "Ia(q)  Atomic scattering"
                << "Ic(q)  Shape scattering";
            bool ok;
            res = US_Static::getItem(
                                        "There are three available datasets", 
                                        "Select the set you wish to plot::", lst, 0, false, &ok,
                                        this );
            if ( ok ) {
               // user selected an item and pressed OK
               Icolumn = 0;
               if ( res.contains(QRegExp("^I.q. ")) ) 
               {
                  Icolumn = 1;
               } 
               if ( res.contains(QRegExp("^Ia.q. ")) ) 
               {
                  Icolumn = 2;
               } 
               if ( res.contains(QRegExp("^Ic.q. ")) ) 
               {
                  Icolumn = 3;
               } 
               if ( !Icolumn ) 
               {
                  cerr << "US_Hydrodyn_Saxs::load_saxs : unknown type error" << endl;
                  return;
               }
               TSO << " column " << Icolumn << endl;
            } 
            else
            {
               return;
            }
         } else {
            Icolumn = 1;
         }
      }
      editor->append(QString("Loading SAXS data from %1 %2\n").arg(filename).arg(res));

      int startline;
      if ( QRegExp( "^\\s*(\\d|.|e|E|+|-)+\\s*(\\d|.|e|E|+|-)+" ).indexIn( qv[0] ) ) {
         startline = 0;
      } else {
         startline = 1;
         editor->append(qv[0]);
      }

      double units = 1.0;
      if ( our_saxs_options->iq_scale_ask )
      {
         switch( QMessageBox::information( this, 
                                           us_tr("UltraScan"),
                                           us_tr("Is this file in 1/Angstrom or 1/nm units?"),
                                           "1/&Angstrom", 
                                           "1/&nm", 0,
                                           0,      // Enter == button 0
                                           1 ) ) { // Escape == button 2
         case 0: // load it as is
            units = 1.0;
            break;
         case 1: // rescale
            units = 0.1;
            break;
         } 
      } else {
         if ( our_saxs_options->iq_scale_angstrom ) 
         {
            units = 1.0;
         } else {
            units = 0.1;
         }
      }

      QRegExp rx_ok_line("^(\\s+|-\\d+|\\d+|\\.|\\d(E|e)(\\+|-|\\d))+$");
      rx_ok_line.setMinimal( true );
      for ( unsigned int i = startline; i < (unsigned int) qv.size(); i++ )
      {
         // QTextStream(stdout) << QString( "load line %1 : %2\n" ).arg( i ).arg( qv[i] );
         if ( qv[i].contains(QRegExp("^#")) ||
              rx_ok_line.indexIn( qv[i] ) == -1 )
         {
            TSO << "not ok: " << qv[i] << endl; 
            continue;
         }
         
         // QStringList tokens = (qv[i].replace(QRegExp("^\\s+").split( QRegExp("\\s+") , Qt::SkipEmptyParts ),""));
         QStringList tokens;
         {
            QString qs = qv[i].replace(QRegExp("^\\s+"),"");
            tokens = (qs ).split( QRegExp("\\s+") , Qt::SkipEmptyParts );
         }
         if ( (unsigned int) tokens.size() > Icolumn )
         {
            new_q = tokens[0].toDouble();
            new_I = tokens[Icolumn].toDouble();
            if ( I_errorcolumn && (unsigned int) tokens.size() > I_errorcolumn )
            {
               new_I_error = tokens[I_errorcolumn].toDouble();
               if ( our_saxs_options->iqq_expt_data_contains_variances )
               {
                  new_I_error = sqrt( new_I_error );
               }
            }
            
            if ( Icolumn2 && (unsigned int) tokens.size() > Icolumn2 )
            {
               new_I2 = tokens[Icolumn2].toDouble();
               if ( dolog10 )
               {
                  new_I2 = log10(new_I2);
               }
            }

            if ( dolog10 )
            {
               new_I = log10(new_I);
            }
            I.push_back(new_I);
            q.push_back(QString( "%1" ).arg(new_q * units).toDouble());
            if ( I_errorcolumn && (unsigned int) tokens.size() > I_errorcolumn )
            {
               I_error.push_back(new_I_error);
            }
            if ( Icolumn2 && (unsigned int) tokens.size() > Icolumn2 )
            {
               I2.push_back(new_I2);
            }
         }
      }

      TSO << "q_range after load: " << q[0] << " , " << q[q.size() - 1] << endl;

      TSO << QFileInfo(filename).fileName() << endl;
      // turn off cropping
      //
      // if ( do_crop )
      // {
      //    if ( Icolumn2 )
      //    {
      //       q2 = q;
      //       crop_iq_data(q2, I2);
      //    }
      //    if ( I_error.size() )
      //    {
      //       crop_iq_data(q, I, I_error );
      //    } else {
      //       crop_iq_data(q, I);
      //    }
      // }
         
      // TSO << "q_range after crop: " << q[0] << " , " << q[q.size() - 1] << endl;

      if ( q.size() &&
           !scaling_target.isEmpty() && 
           plotted_iq_names_to_pos.count(scaling_target) )
      {
         if ( Icolumn2 )
         {
            rescale_iqq_curve( scaling_target, q, I, I2 );
         } else {
            rescale_iqq_curve( scaling_target, q, I );
         }
         if ( I_error.size() )
         {
            rescale_iqq_curve_using_last_rescaling( I_error );
         }
      }

      if ( q.size() )
      {
         if ( I_error.size() )
         {
            editor_msg(
                       our_saxs_options->iqq_expt_data_contains_variances ?
                       "red" :
                       "dark blue"
                       , 
                       our_saxs_options->iqq_expt_data_contains_variances ?
                       "Loaded variance data\n" :
                       "Loaded standard deviation data\n"
                       );
            plot_one_iqq(q, I, I_error, QFileInfo(filename).fileName() + tag1);
         } else {
            plot_one_iqq(q, I, QFileInfo(filename).fileName() + tag1);
         }
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header =
            QString("")
            .sprintf(
                     "Saxs curves from %s"
                     , filename.toLatin1().data()
                     );
         for ( unsigned int i = 0; i < q.size(); i++ )
         {
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.push_back(q[i]);
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.push_back(I[i]);
         }
      }
      if ( q2.size() )
      {
         plot_one_iqq(q2, I2, QFileInfo(filename).fileName() + tag2);
      }
      if ( plotted )
      {
         editor_msg( "black", "I(q) plot done\n");
         plotted = false;
      }

      cb_guinier   ->setChecked( false );
      cb_user_range->setChecked( false );
      set_guinier();
   }
}

static QString assoc_sd( QString & qs, const QStringList & qsl_sd ) {
   // get sd qstring if available, return empty string otherwise
   // qDebug() << QString( "assoc_sd( %1, qsl_sd )" ).arg( qs );
   QString name;
   {
      QStringList qsl = qs.split( "," );
      if ( !qsl.size() ) {
         // qDebug() << "no commas in qs assoc_sd()";
         return "";
      }
      name = qsl[0];
   }
   // qDebug() << QString( "qs assoc_sd() name found %1" ).arg( name );

   QStringList qsl =
      qsl_sd.filter(
                    QRegularExpression(
                                       QString( "^%1," )
                                       .arg( QRegularExpression::escape( name ) )
                                       )
                    );
   if ( !qsl.size() ) {
      // qDebug() << "no sd match for " << name;
      return "";
   }
                     
   if ( qsl.size() > 1) {
      qDebug() << "multiple sd match for " << name;
      return "";
   }
   return qsl[0].replace( QRegularExpression( ",\\s*$" ), "" );
}

void US_Hydrodyn_Saxs::load_pr( bool just_plotted_curves, QString load_this, bool skip_mw )
{
   if ( just_plotted_curves &&
        !qsl_plotted_pr_names.size() )
   {
         QMessageBox::warning( this, "UltraScan",
                               QString(us_tr("There is nothing plotted!")) );
         return;
   }      

   QString use_dir = 
      our_saxs_options->path_load_prr.isEmpty() ?
      USglobal->config_list.root_dir + SLASH + "somo" + SLASH + "saxs" :
      our_saxs_options->path_load_prr;

   QStringList filenames;
   QString filename;
   if ( load_this.isEmpty() ) {
      if ( !just_plotted_curves ) {
         select_from_directory_history( use_dir, this );
      }

      if ( !just_plotted_curves ) {
         filenames = QFileDialog::getOpenFileNames( this , "Open" , use_dir , "All files (*);;"
                                                   "sprr files (*.sprr_?);;"
                                                   "csv files (*.csv)" , &load_pr_selected_filter );
      }
   } else {
      filenames << load_this;
   }

   if ( filenames.empty() && !just_plotted_curves )
   {
      return;
   }

   if ( !filenames.empty() )
   {
      add_to_directory_history( filenames[0] );
   }

   if ( filenames.size() > 1 &&
        filenames.filter(QRegExp(".csv$", Qt::CaseInsensitive )).size() != filenames.size() )
   {
      QMessageBox::information( this, "UltraScan",
                                us_tr("Multiple file load is currently only supported for csv files") );
      return;
   }

   if ( filenames.size() > 1 &&
        filenames.filter(QRegExp("_t(|-\\d+).csv$", Qt::CaseInsensitive )).size() ) 
   {
      QMessageBox::information( this, "UltraScan",
                                us_tr("Can not load transposed format csv files:\n") +
                                filenames.filter(QRegExp("_t(|-\\d+).csv$", Qt::CaseInsensitive )).join("\n")
                                ) ;
      return;
   }

   // QTextStream(stdout) << "files to load:\n---\n" << filenames.join("\n") << "\n---\n";

   if ( !just_plotted_curves )
   {
      filename = filenames[0];
      
      if ( filename.contains(QRegExp("_t(|-\\d+).csv$", Qt::CaseInsensitive )) )
      {
         QMessageBox::information( this, "UltraScan",
                                   us_tr("Can not load transposed format csv files") );
         return;
      }
   }

   QFile f(filename);
   if ( !just_plotted_curves )
   {
      our_saxs_options->path_load_prr = QFileInfo(filename).absolutePath();
   }
   QString ext = QFileInfo(filename).suffix().toLower();
   vector < double > r;
   vector < double > pr;
   vector < double > pr_error;
   // double new_r, new_pr, new_pr_error;
   QString res = "";
   unsigned int startline = 1;
   unsigned int pop_last = 0;

   pad_pr_plotted();
   
   if ( just_plotted_curves || f.open(QIODevice::ReadOnly) )
   {
      if ( file_curve_type(f.fileName()) != -1 &&
           file_curve_type(f.fileName()) != our_saxs_options->curve )
      {
         switch ( QMessageBox::question(this, 
                                        us_tr("UltraScan Notice"),
                                        QString(us_tr("Please note:\n\n"
                                                   "The file appears to be in %1 mode and you"
                                                   " are currently set in %2 mode.\n"
                                                   "What would you like to do?\n"))
                                        .arg(curve_type_string(file_curve_type(f.fileName())))
                                        .arg(curve_type_string(our_saxs_options->curve))
                                        ,
                                        us_tr("&Change mode now and load"), 
                                        us_tr("&Load anyway without changing the mode"),
                                        us_tr("&Stop loading"),
                                        0, // Stop == button 0
                                        0 // Escape == button 0
                                        ) )
               {
               case 0 : 
                  our_saxs_options->curve = file_curve_type(f.fileName());
                  rb_curve_raw->setChecked(our_saxs_options->curve == 0);
                  rb_curve_saxs->setChecked(our_saxs_options->curve == 1);
                  rb_curve_sans->setChecked(our_saxs_options->curve == 2);
                  break;
               case 2 : 
                  break;
               case 1 : 
               default :
                  f.close();
                  return;
                  break;
               }
      }
   
      if ( ext == "csv" || just_plotted_curves )
      {
         // attempt to read a csv file
         QStringList qsl;
         QStringList qsl_data_lines_plotted;  // for potential later average save
         if ( !just_plotted_curves )
         {
            QTextStream ts(&f);
            while ( !ts.atEnd() )
            {
               qsl << ts.readLine();
            }
            f.close();
         }

         QStringList qsl_headers = qsl.filter("\"Name\",\"Type; r:\"");
         if ( qsl_headers.size() != 0 ) 
         {
            TSO << "found old csv format, upgrading\n";
            // upgrade the csv format
            QRegExp rx("^\"(Type; r:|P\\(r\\)|P\\(r\\) normed)\"$");
            QStringList new_qsl;
            double delta_r = 0e0;
            for ( unsigned int i = 0; i < (unsigned int)qsl.size(); i++ )
            {
               QStringList tmp2_qsl;
               QStringList tmp_qsl = (qsl[i]).split( "," );
               // if ( tmp_qsl.size() > 1 )
               // {
               //      TSO << QString("line %1 field 1 is <%2>\n").arg(i).arg(tmp_qsl[1]);
               // } else {
               // TSO << QString("line %1 size not greater than 1 value <%2>\n").arg(i).arg(qsl[i]);
               // }
               if ( tmp_qsl.size() > 1 &&
                    rx.indexIn(tmp_qsl[1]) != -1 )
               {
                  // TSO << "trying to fix\n";
                  QStringList tmp2_qsl;
                  tmp2_qsl.push_back(tmp_qsl[0]);
                  if ( tmp_qsl[0] == "\"Name\"" )
                  {
                     tmp2_qsl.push_back("\"MW (Daltons)\"");
                     tmp2_qsl.push_back("\"Area\"");
                     delta_r = tmp_qsl[3].toDouble() - tmp_qsl[2].toDouble();
                     TSO << "delta_r found: " << delta_r << endl;
                  } else {
                     tmp2_qsl.push_back(QString("%1").arg(get_mw(tmp_qsl[0], false)));
                     double tmp_area = 0e0;
                     for ( unsigned int j = 1; j < (unsigned int)tmp_qsl.size(); j++ )
                     {
                        tmp_area += tmp_qsl[j].toDouble();
                     }
                     tmp2_qsl.push_back(QString("%1").arg(tmp_area * delta_r));
                  }
                  for ( unsigned int j = 1; j < (unsigned int)tmp_qsl.size(); j++ )
                  {
                     tmp2_qsl.push_back(tmp_qsl[j]);
                  }
                  new_qsl.push_back(tmp2_qsl.join(","));
               } else {
                  // TSO << "skipped this line, regexp or length\n";
                  // simply push back blank lines or lines with only one entry
                  new_qsl.push_back(qsl[i]);
               }
            }
            // TSO << "orginal csv:\n" << qsl.join("\n") << endl;
            // TSO << "new csv:\n" << new_qsl.join("\n") << endl;
            qsl = new_qsl;
         }

         // qDebug() << "pr_csv_pad_holes() called on qsl";
         qsl = pr_csv_pad_holes( qsl );

         qsl_headers = qsl.filter("\"Name\",\"MW (Daltons)\",\"Area\",\"Type; r:\"");
         if ( qsl_headers.size() == 0 && !just_plotted_curves ) 
         {
            // QMessageBox mb(us_tr("UltraScan Warning"),
            //                us_tr("The csv file ") + filename + us_tr(" does not appear to contain a correct header.\nPlease manually correct the csv file."),
            //                QMessageBox::Critical,
            //                QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
            // mb.exec();
            QMessageBox::critical( this,
                                   us_tr("UltraScan Warning"),
                                   us_tr("The csv file ") + filename + us_tr(" does not appear to contain a correct header.\nPlease manually correct the csv file."),
                                   QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton );
            return;
         }

         if ( qsl_headers.size() > 1 ) 
         {
            QString ref = qsl_headers[0];
            for ( unsigned int i = 1; i < (unsigned int)qsl_headers.size(); i++ )
            {
               if ( ref != qsl_headers[i] )
               {
                  // QMessageBox mb(us_tr("UltraScan Warning"),
                  //                us_tr("The csv file ") + filename + us_tr(" contains multiple different headers\nPlease manually correct the csv file."),
                  //                QMessageBox::Critical,
                  //                QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
                  // mb.exec();
                  QMessageBox::critical( this,
                                         us_tr("UltraScan Warning"),
                                         us_tr("The csv file ") + filename + us_tr(" contains multiple different headers\nPlease manually correct the csv file."),
                                         QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton );
                  return;
               }
            }
         }

         // get the r values
         QStringList qsl_r;
         QString header_tag;

         if ( !just_plotted_curves )
         {
            qsl_r = (qsl_headers[0]).split( "," );
            if ( qsl_r.size() < 6 )
            {
               // QMessageBox mb(us_tr("UltraScan Warning"),
               //                us_tr("The csv file ") + filename + us_tr(" does not appear to contain any r values in the header rows.\n"),
               //                QMessageBox::Critical,
               //             QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
               // mb.exec();
               QMessageBox::critical( this,
                                      us_tr("UltraScan Warning"),
                                      us_tr("The csv file ") + filename + us_tr(" does not appear to contain any r values in the header rows.\n"),
                                      QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton );
               return;
            }
            r.push_back(qsl_r[4].toDouble());
            {
               QStringList::iterator it = qsl_r.begin();
               it += 5;
               for ( ; it != qsl_r.end(); it++ )
               {
                  if ( (*it).toDouble() > r[r.size() - 1] )
                  {
                     r.push_back((*it).toDouble());
                  } else {
                     break;
                  }
               }
            }
         } else {
            r = plotted_r[0];
            header_tag = "Plotted P(r) curves";
            QString header = 
               QString("\"Name\",\"MW (Daltons)\",\"Area\",\"Type; r:\",%1,%2\n")
               .arg(vector_double_to_csv(r))
               .arg(header_tag);
            qsl << header;
         }

         // ok, we have a header line

         // do we have multiple additonal files to plot ?

         map < QString, QString > source_filenames;

         if ( filenames.size() > 1 )
         {
            map < QString, bool > interp_msg_done;
            
            for ( unsigned int i = 1; i < (unsigned int)filenames.size(); i++ )
            {
               TSO << QString("trying file %1 %2\n").arg(i).arg(filenames[i]);
               QFile f2(filenames[i]);
               if ( !f2.open(QIODevice::ReadOnly) )
               {
                  QMessageBox::information( this, "UltraScan",
                                            us_tr("Can not open the file for reading:\n") +
                                            f2.fileName()
                                            );
                  return;
               }

               // append (and possibly interpolate) all the other files
               QStringList qsl2;
               QTextStream ts(&f2);
               while ( !ts.atEnd() )
               {
                  qsl2 << ts.readLine();
               }
               f2.close();
               QStringList qsl2_headers = qsl2.filter("\"Name\",\"Type; r:\"");
               
               // upgrade old format
               if ( qsl2_headers.size() != 0 ) 
               {
                  TSO << "found old csv format, upgrading\n";
                  // upgrade the csv format
                  QRegExp rx("^\"(Type; r:|P\\(r\\)|P\\(r\\) normed|P\\(r\\) sd|P\\(r\\) normed sd)\"$");
                  QStringList new_qsl;
                  double delta_r = 0e0;
                  for ( unsigned int i = 0; i < (unsigned int)qsl2.size(); i++ )
                  {
                     QStringList tmp2_qsl;
                     QStringList tmp_qsl = (qsl2[i]).split( "," );
                     if ( tmp_qsl.size() > 1 &&
                          rx.indexIn(tmp_qsl[1]) != -1 )
                     {
                        // TSO << "trying to fix\n";
                        QStringList tmp2_qsl;
                        tmp2_qsl.push_back(tmp_qsl[0]);
                        if ( tmp_qsl[0] == "\"Name\"" )
                        {
                           tmp2_qsl.push_back("\"MW (Daltons)\"");
                           tmp2_qsl.push_back("\"Area\"");
                           delta_r = tmp_qsl[3].toDouble() - tmp_qsl[2].toDouble();
                           TSO << "delta_r found: " << delta_r << endl;
                        } else {
                           tmp2_qsl.push_back(QString("%1").arg(get_mw(tmp_qsl[0], false)));
                           double tmp_area = 0e0;
                           for ( unsigned int j = 1; j < (unsigned int)tmp_qsl.size(); j++ )
                           {
                              tmp_area += tmp_qsl[j].toDouble();
                           }
                           tmp2_qsl.push_back(QString("%1").arg(tmp_area * delta_r));
                        }
                        for ( unsigned int j = 1; j < (unsigned int)tmp_qsl.size(); j++ )
                        {
                           tmp2_qsl.push_back(tmp_qsl[j]);
                        }
                        new_qsl.push_back(tmp2_qsl.join(","));
                     } else {
                        // TSO << "skipped this line, regexp or length\n";
                        // simply push back blank lines or lines with only one entry
                        new_qsl.push_back(qsl[i]);
                     }
                  }
                  // TSO << "orginal csv:\n" << qsl.join("\n") << endl;
                  // TSO << "new csv:\n" << new_qsl.join("\n") << endl;
                  qsl2 = new_qsl;
               }

               // qDebug() << "pr_csv_pad_holes() called on qsl2";
               qsl2 = pr_csv_pad_holes( qsl2 );

               qsl2_headers = qsl2.filter("\"Name\",\"MW (Daltons)\",\"Area\",\"Type; r:\"");
               if ( qsl2_headers.size() == 0 )
               {
                  // QMessageBox mb(us_tr("UltraScan Warning"),
                  //                us_tr("The csv file ") + f2.fileName() + us_tr(" does not appear to contain a correct header.\nPlease manually correct the csv file."),
                  //                QMessageBox::Critical,
                  //                QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
                  // mb.exec();
                  QMessageBox::critical( this,
                                         us_tr("UltraScan Warning"),
                                         us_tr("The csv file ") + f2.fileName() + us_tr(" does not appear to contain a correct header.\nPlease manually correct the csv file."),
                                         QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton );
                  return;
               }
               if ( qsl2_headers.size() > 1 ) 
               {
                  QString ref = qsl2_headers[0];
                  for ( unsigned int i = 1; i < (unsigned int)qsl2_headers.size(); i++ )
                  {
                     if ( ref != qsl2_headers[i] )
                     {
                        // QMessageBox mb(us_tr("UltraScan Warning"),
                        //                us_tr("The csv file ") + filename + us_tr(" contains multiple different headers\nPlease manually correct the csv file."),
                        //                QMessageBox::Critical,
                        //                QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
                        // mb.exec();
                        QMessageBox::critical( this,
                                               us_tr("UltraScan Warning"),
                                               us_tr("The csv file ") + filename + us_tr(" contains multiple different headers\nPlease manually correct the csv file."),
                                               QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton );
                        return;
                     }
                  }
               }
               // get the data
               QStringList qsl_data = qsl2.filter(",\"P(r)\",");
               if ( qsl_data.size() == 0 )
               {
                  // QMessageBox mb(us_tr("UltraScan Warning"),
                  //                us_tr("The csv file ") + filename + us_tr(" does not appear to contain any data rows.\n"),
                  //                QMessageBox::Critical,
                  //                QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
                  // mb.exec();
                  QMessageBox::critical( this,
                                         us_tr("UltraScan Warning"),
                                         us_tr("The csv file ") + filename + us_tr(" does not appear to contain any data rows.\n"),
                                         QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton );
                  return;
               }

               QStringList qsl_sd = qsl2.filter(",\"P(r) sd\",");
               // qDebug() << QString( "qsl_sd.size() %1" ).arg( qsl_sd.size() );

               // get the r values
               QStringList qsl_r;
               QString header2_tag;
               vector < double > r2;

               qsl_r = (qsl2_headers[0]).split( "," );
               if ( qsl_r.size() < 6 )
               {
                  // QMessageBox mb(us_tr("UltraScan Warning"),
                  //                us_tr("The csv file ") + f2.fileName() + us_tr(" does not appear to contain any r values in the header rows.\n"),
                  //                QMessageBox::Critical,
                  //                QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
                  // mb.exec();
                  QMessageBox::critical( this,
                                         us_tr("UltraScan Warning"),
                                         us_tr("The csv file ") + f2.fileName() + us_tr(" does not appear to contain any r values in the header rows.\n"),
                                         QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton );
                  return;
               }
               r2.push_back(qsl_r[4].toDouble());
            
               {
                  QStringList::iterator it = qsl_r.begin();
                  it += 5;
                  for ( ; it != qsl_r.end(); it++ )
                  {
                     if ( (*it).toDouble() > r2[r2.size() - 1] )
                     {
                        r2.push_back((*it).toDouble());
                     } else {
                        break;
                     }
                  }
               }
               unsigned int max_size = r2.size();
               double max_value = r2[max_size - 1];
               if ( r.size() > 1 && r[r.size() - 1] < max_value )
               {
                  double r_delta = r[1] - r[0];
                  if ( r_delta > 1e0 )
                  {
                     for ( double value = r[r.size() -1] + r_delta;
                           value <= max_value;
                           value += r_delta )
                     {
                        r.push_back(value);
                     }
                  }
               }

               if ( max_size > r.size() )
               {
                  max_size = r.size();
               }                  
               // TSO << QString("max size %1\n").arg(max_size);
               bool all_match = true;
               for ( unsigned int i = 0; i < max_size; i++ )
               {
                  if ( r[i] != r2[i] )
                  {
                     all_match = false;
                     break;
                  }
               }

               // TSO << QString("check all match %1\n").arg(all_match ? "true" : "false");

               for ( QStringList::iterator it = qsl_data.begin();
                     it != qsl_data.end();
                     it++ )
               {
                  QStringList qsl_pr = (*it).split( "," );
                  if ( qsl_pr.size() < 6 )
                  {
                     QString msg = us_tr("The csv file ") + f2.fileName() + us_tr(" does not appear to contain sufficient p(r) values in data row " + qsl_pr[0] + ", skipping\n");
                     editor_msg( "red", msg );
                  } else {
                     // build up new row to append to qsl
                     assoc_sd( qsl_pr[0], qsl_sd );
                     if ( all_match )
                     {
                        QString qs_tmp = *it;
                        // optionally? 
                        if ( qs_tmp.contains(QRegExp("(Average|Standard deviation)")) )
                        {
                           qs_tmp.replace(QRegExp("^\""),QString("\"%1: ").arg(QFileInfo(f2).baseName()));
                        }
                        qsl << qs_tmp;
                        // TSO << "yes, all match\nadding:" << qs_tmp << endl;
                        {
                           QStringList qsl_pr_8;
                           QStringList new_qsl = qs_tmp.split( "," );
                           for ( int i = 0; i < 8 && i < (int) new_qsl.size(); ++i ) {
                              qsl_pr_8 << new_qsl[i];
                           }
                           source_filenames[ qsl_pr_8.join(",") ] = QFileInfo(f2).fileName();
                        }
                     } else {
                        // TSO << "not all match, interpolate\n";
                        if ( !interp_msg_done.count(f2.fileName()) )
                        {
                           interp_msg_done[f2.fileName()] = true;
                           QString msg = us_tr("The csv file ") + f2.fileName() + us_tr(" will be interpolated\n");
                           editor_msg( "dark red", msg );
                        }
                        // the new pr:
                        QStringList new_pr_fields;
                        // pull the data values
                        vector < double > this_pr;
                        for ( unsigned int i = 0; i < 4; i++ )
                        {
                           new_pr_fields.push_back(qsl_pr[i]);
                        }
                        {
                           QStringList::iterator it2 = qsl_pr.begin();
                           it2 += 4;
                           for ( ; it2 != qsl_pr.end(); it2++ )
                           {
                              this_pr.push_back((*it2).toDouble());
                           }
                        }
                        // interpolate r2, pr to r, reappend to new_pr_fields
                        vector < double > npr;
                        bool ok = true;
                        if ( !interpolate(r, r2, this_pr, npr ) ) {
                           editor_msg( "red",  QString( us_tr("Interpolation error on curve in %1 ") ).arg( f2.fileName() ) );
                           ok = false;
                        }

                        if ( ok ) {
                           QString qs_assoc_sd = assoc_sd( new_pr_fields[0], qsl_sd );
                           // qDebug() << "spec qs_assoc_sd " << qs_assoc_sd;
                           if ( !qs_assoc_sd.isEmpty() ) {
                              // TSO << "--> Found SD for " << new_pr_fields[0] << " in " << QFileInfo(f2).baseName() << "\n";

                              // the new pr_error:
                              QStringList new_pr_error_fields;
                              QStringList qsl_pr_error = qs_assoc_sd.split( "," );

                              if ( qsl_pr_error.size() > 4 ) {
                                 // pull the error values
                                 vector < double > this_pr_error;
                                 for ( unsigned int i = 0; i < 4; i++ ) {
                                    new_pr_error_fields.push_back(qsl_pr_error[i]);
                                 }
                                 {
                                    QStringList::iterator it2 = qsl_pr_error.begin();
                                    it2 += 4;
                                    for ( ; it2 != qsl_pr_error.end(); it2++ )
                                    {
                                       this_pr_error.push_back((*it2).toDouble());
                                    }
                                 }

                                 vector < double > new_pr_error;
                                 if ( !interpolate(r, r2, this_pr_error, new_pr_error ) ) {
                                    editor_msg( "red",  QString( us_tr("Interpolation error on curve in %1 ") ).arg( f2.fileName() ) );
                                 } else {
                                    QString line = QString("%1,%2\n")
                                       .arg(new_pr_error_fields.join(","))
                                       .arg(vector_double_to_csv(new_pr_error));
                                    // TSO << QString("r:\n%1\n").arg(vector_double_to_csv(r));
                                    // TSO << QString("r2:\n%1\n").arg(vector_double_to_csv(r2));
                                    // TSO << QString("org line:\n%1\n").arg(qs_assoc_sd);
                                    // TSO << QString("new interpolated line:\n%1\n").arg(line);
                                    if ( line.contains(QRegExp("(Average|Standard deviation)")) )
                                    {
                                       line.replace(QRegExp("^\""),QString("\"%1: ").arg(QFileInfo(f2).baseName()));
                                    }
                                    qsl << line;
                                 }
                              }
                           }

                           QString line = QString("%1,%2\n")
                              .arg(new_pr_fields.join(","))
                              .arg(vector_double_to_csv(npr));
                           // TSO << QString("r:\n%1\n").arg(vector_double_to_csv(r));
                           // TSO << QString("r2:\n%1\n").arg(vector_double_to_csv(r2));
                           // TSO << QString("org line:\n%1\n").arg(*it);
                           // TSO << QString("new interpolated line:\n%1\n").arg(line);
                           if ( line.contains(QRegExp("(Average|Standard deviation)")) )
                           {
                              line.replace(QRegExp("^\""),QString("\"%1: ").arg(QFileInfo(f2).baseName()));
                           }
                           qsl << line;
                           {
                              QStringList qsl_pr_8;
                              QStringList new_qsl = line.split( "," );
                              for ( int i = 0; i < 8 && i < (int) new_qsl.size(); ++i ) {
                                 qsl_pr_8 << new_qsl[i];
                              }
                              source_filenames[ qsl_pr_8.join(",") ] = QFileInfo(f2).fileName();
                           }
                        }
                     }
                  }
               }
            }            
         }

         // TSO << US_Vector::qs_mapqsqs( "source_filenames", source_filenames );

         // append all currently plotted p(r)s to qsl
         bool added_interpolate_msg = false;
         QString bin_msg = "";
         for ( unsigned int i = 0; i < (unsigned int)qsl_plotted_pr_names.size(); i++ )
         {
            vector < double > npr       = interpolate(r, plotted_r[i], plotted_pr_not_normalized[i]);
            vector < double > npr_error;
            if ( plotted_pr_not_normalized_error[i].size() ) {
               // US_Vector::printvector3( "errors exist r, plotted_r[i], plotted_pr_not_nomalized_error[i]", r, plotted_pr[i], plotted_pr_not_normalized_error[i] );                  
               npr_error = interpolate(r, plotted_r[i], plotted_pr_not_normalized_error[i]);
            }
            if ( !added_interpolate_msg && r.size() > 1 )
            {
               bin_msg = QString(us_tr("Plotted P(r) interpolated to bin size of %1")).arg(r[1] - r[0]);
               if ( plotted_r[i].size() > 1 && 
                    r[1] - r[0] != plotted_r[i][1] - plotted_r[i][0] )
               {
                  bin_msg +=
                     QString(us_tr(" which is DIFFERENT from the plotted bin size of %1"))
                     .arg(plotted_r[i][1] - plotted_r[i][0]);
               }
               // editor->append(bin_msg + "\n");
               added_interpolate_msg = true;
            }
            QString line = QString("\"%1\",%2,%3,\"P(r)\",%4\n")
               .arg(qsl_plotted_pr_names[i].replace( QRegExp( "(^[^\\\"]*\\\"|\\\"[^\\\"]*$)" ), "" ) )
               .arg(plotted_pr_mw[i])
               .arg(compute_pr_area(npr, r))
               .arg(vector_double_to_csv(npr));
            qsl << line;
            if ( npr_error.size() ) {
               QString line = QString("\"%1\",%2,%3,\"P(r) sd\",%4\n")
                  .arg(qsl_plotted_pr_names[i].replace( QRegExp( "(^[^\\\"]*\\\"|\\\"[^\\\"]*$)" ), "" ) )
                  .arg(plotted_pr_mw[i])
                  .arg(compute_pr_area(npr, r))
                  .arg(vector_double_to_csv(npr_error));
               qsl << line;
            }
         }
            
         if ( filenames.size() > 1 )
         {
            QStringList new_qsl;
            for ( unsigned int i = 0; i < (unsigned int)qsl.size(); i++ )
            {
               QString qs_tmp = qsl[i];
               if ( !qs_tmp.contains(QRegExp("(Average|Standard deviation)")) )
               {
                  new_qsl << qsl[i];
               }
            }
            if ( new_qsl.size() != qsl.size() )
            {
               switch( QMessageBox::information( this, 
                                                 us_tr("UltraScan"),
                                                 us_tr("There are multiple average and/or standard deviation lines\n") +
                                                 us_tr("What do you want to do?"),
                                                 us_tr("&Skip"), 
                                                 us_tr("&Just averages"),
                                                 us_tr("&Include"),
                                                 0,      // Enter == button 0
                                                 1 ) ) { // Escape == button 2
               case 0: // skip them
                  qsl = new_qsl;
                  break;
               case 1: // just averages
                  qsl = qsl.filter(QRegExp("(Average|Standard deviation)"));
                  break;
               case 2: // Cancel clicked or Escape pressed
                  break;
               }
            }
         }

         QStringList qsl_data = qsl.filter(",\"P(r)\",");
         if ( qsl_data.size() == 0 )
         {
            // QMessageBox mb(us_tr("UltraScan Warning"),
            //                us_tr("The csv file ") + filename + us_tr(" does not appear to contain any data rows.\n"),
            //                QMessageBox::Critical,
            //                QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
            // mb.exec();
            QMessageBox::critical( this,
                                   us_tr("UltraScan Warning"),
                                   us_tr("The csv file ") + filename + us_tr(" does not appear to contain any data rows.\n"),
                                   QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton );
            return;
         }

         QStringList qsl_sd = qsl.filter(",\"P(r) sd\",");
         // qDebug() << QString( "qsl_sd.size() %1" ).arg( qsl_sd.size() );

         if ( !just_plotted_curves )
         {
            header_tag = qsl_r.last();
         }

#if defined(DEBUG_PR)
         TSO << "r values (" << r.size() << "): ";
         
         for ( unsigned int i = 0; i < r.size(); i++ )
         {
            TSO << r[i] << ",";
         }
         TSO << endl;
#endif
      
         // build a list of names
         QStringList qsl_names;
         for ( QStringList::iterator it = qsl_data.begin();
               it != qsl_data.end();
               it++ )
         {
            QStringList qsl_tmp = (*it).split( "," );
            qsl_names << qsl_tmp[0];
         }

         // ask for the names to load if more than one present (cb list? )
         QStringList qsl_sel_names;
         bool create_avg          = false;
         bool create_std_dev      = false;
         bool only_plot_stats     = true;
         save_to_csv              = false;
         csv_filename             = "summary";
         bool save_original_data  = false;
         bool run_nnls            = false;
         bool nnls_csv            = false;
         bool run_best_fit        = false;
         use_SDs_for_fitting_prr  = false;
         nnls_plot_contrib        = false;
         QString nnls_target      = "";
         bool clear_plot_first    = true;
         
         US_Hydrodyn_Saxs_Load_Csv *hslc =
            new US_Hydrodyn_Saxs_Load_Csv(
                                          "Select models to load\n" + header_tag + 
                                          (bin_msg.isEmpty() ? "" : "\n" + bin_msg),
                                          &qsl_names,
                                          &qsl_sel_names,
                                          &qsl,
                                          filename,
                                          &create_avg,
                                          &create_std_dev,
                                          &only_plot_stats,
                                          &save_to_csv,
                                          &csv_filename,
                                          &save_original_data,
                                          &run_nnls,
                                          &nnls_plot_contrib,
                                          &nnls_csv,
                                          &run_best_fit,
                                          &use_SDs_for_fitting_prr,
                                          &nnls_target,
                                          &clear_plot_first,
                                          1 || U_EXPT,
                                          us_hydrodyn
                                          );
         US_Hydrodyn::fixWinButtons( hslc );
         hslc->exec();
            
         delete hslc;
         
         this->isVisible() ? this->raise() : this->show();

         if ( qsl_sel_names.size() && clear_plot_first ) {
            clear_plot_pr( true );
         }

         // make sure target is selected

         if ( ( run_nnls || run_best_fit ) &&
              !qsl_sel_names.filter(nnls_target).size() )
         {
            // TSO << "had to add target back\n";
            qsl_sel_names << nnls_target;
         }

         // ok, now qsl_sel_names should have the load list
         // loop through qsl_data and match up 
         // create a map to avoid a double loop

         map < QString, bool > map_sel_names;
         for ( QStringList::iterator it = qsl_sel_names.begin();
               it != qsl_sel_names.end();
               it++ )
         {
            map_sel_names[*it] = true;
         }

         // setup for average & stdev
         vector < double > sum_pr(r.size());
         vector < double > sum_pr2(r.size());

         unsigned int sum_count = 0;
         for ( unsigned int i = 0; i < r.size(); i++ )
         {
            sum_pr[i] = sum_pr2[i] = 0e0;
         }

         QString nnls_csv_filename;
         nnls_csv_data.clear();
         nnls_csv_footer.clear();

         QString qs_target_sd;

         // setup for nnls
         if ( run_nnls || run_best_fit )
         {
            qs_target_sd = assoc_sd( nnls_target, qsl_sd );
            if ( run_nnls )
            {
               editor->append("NNLS target: " + nnls_target + "\n");

               // {
               //    nnls_csv_footer << "\"NNLS target:\"," + nnls_target;
               // }
            }
            if ( nnls_csv ) {
               QString use_dir = USglobal->config_list.root_dir + "/" + "somo" + "/" + "saxs";

               // ((US_Hydrodyn *)us_hydrodyn)->select_from_directory_history( use_dir, this );

               nnls_csv_filename =
                  QFileDialog::getSaveFileName(
                                               this
                                               ,us_tr("Choose a filename to save the NNLS fit")
                                               ,use_dir
                                               + "/"
                                               + QFileInfo( QString( "%1" ).arg( nnls_target ).replace( "\"", "" ) ).baseName()
                                               + "_NNLS_results_pr.csv"
                                               ,"*.csv" );

               if ( nnls_csv_filename.isEmpty() ) {
                  editor_msg( "red", us_tr( "Canceling NNLS CSV save" ) );
                  nnls_csv = false;
               }
               if ( !nnls_csv_filename.contains(QRegExp(".csv", Qt::CaseInsensitive )) )
               {
                  nnls_csv_filename += ".csv";
               }

            }

            if ( run_best_fit )
            {
               editor->append("Best fit target: " + nnls_target + "\n");
            }
            nnls_A.clear( );
            nnls_x.clear( );
            nnls_mw.clear( );
            nnls_B.clear( );
            nnls_B_name = nnls_target;
            nnls_errors.clear();
            nnls_rmsd = 0e0;
         }

         bool found_nnls_target = false;
         bool found_nnls_model  = false;

         float pr_mw_avg = 0.0;
         vector < float > pr_mws;
         vector < double > pr_error;

         // now go through qsl_data and load up any that map_sel_names contains
         plotted = false;
         for ( QStringList::iterator it = qsl_data.begin();
               it != qsl_data.end();
               it++ )
         {
            QStringList qsl_tmp = (*it).split( "," );
            QString use_filename = QFileInfo(filename).fileName();
            {
               QStringList qsl_tmp_8;
               for ( int i = 0; i < 8 && i < (int) qsl_tmp.size(); ++i ) {
                  qsl_tmp_8 << qsl_tmp[i];
               }
               if ( source_filenames.count( qsl_tmp_8.join(",") ) ) {
                  use_filename = source_filenames[ qsl_tmp_8.join(",") ];
               }
            }
            
            if ( map_sel_names.count(qsl_tmp[0]) )
            {
               // TSO << "loading: " << qsl_tmp[0] << endl;

               pr      .clear();
               pr_error.clear();

               // setup for avg mw's
               // 1st check the line to see if we have a mw
               {
                  if ( qsl_tmp[1].toFloat() > 0e0 )
                  {
                     (*remember_mw)[QFileInfo(qsl_tmp[0]).fileName()] = qsl_tmp[1].toFloat();
                     (*remember_mw_source)[QFileInfo(qsl_tmp[0]).fileName()] = "loaded from csv file";
                  } else {
                     if ( (*remember_mw).count(QFileInfo(qsl_tmp[0]).fileName()) )
                     {
                        (*remember_mw).erase(QFileInfo(qsl_tmp[0]).fileName());
                        (*remember_mw_source).erase(QFileInfo(qsl_tmp[0]).fileName());
                     }
                  }
               }

               float tmp_mw = get_mw(qsl_tmp[0], false);
               pr_mw_avg += tmp_mw;
               pr_mws.push_back(tmp_mw);

               // get the pr values

               QStringList qsl_pr = QString(*it).replace( QRegularExpression( ",\\s*$" ), "" ).split( "," );
               if ( qsl_pr.size() < 6 )
               {
                  QString msg = us_tr("The csv file ") + filename + us_tr(" does not appear to contain sufficient p(r) values in data row " + qsl_tmp[0] + "\n");
                  editor_msg( "red", msg );
                  // QMessageBox mb(us_tr("UltraScan Warning"),
                  //                msg,
                  //                QMessageBox::Critical,
                  //                QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
                  // mb.exec();
                  QMessageBox::critical( this,
                                         us_tr("UltraScan Warning"),
                                         msg,
                                         QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton );
                  break;
               }
               qsl_data_lines_plotted << *it;
               pr.push_back(qsl_pr[4].toDouble());
               QString qs_sd      = assoc_sd( qsl_pr[0], qsl_sd );

               {
                  QStringList::iterator it = qsl_pr.begin();
                  it += 5;
                  for ( ; it != qsl_pr.end(); ++it ) {
                     pr.push_back((*it).toDouble());
                  }
               }

               if ( !qs_sd.isEmpty() ) {
                  QStringList qsl_sd = qs_sd.split(",");
                  QStringList::iterator it = qsl_sd.begin();
                  it += 4;
                  for ( ; it != qsl_sd.end(); ++it ) {
                     pr_error.push_back((*it).toDouble());
                  }
                  if ( pr_error.size() != pr.size() ) {
                     editor_msg( "red"
                                 ,QString(
                                          us_tr(
                                                "S.D.s found in %1, but they are not of the same length (%2) as the P(r) data points (%3), dropping S.D.s"
                                                )
                                          )
                                 .arg( qsl_pr[0] )
                                 .arg( pr_error.size() )
                                 .arg( pr.size() )
                                 );
                     // US_Vector::printvector3( "pr sd mismatch, r, pr, pr_error:", r, pr, pr_error );
                     pr_error.clear();
                  }
               }

               set_pr_sd( r, pr, pr_error );

               if ( run_nnls || run_best_fit )
               {
                  if ( qsl_tmp[0] == nnls_target )
                  {
                     found_nnls_target = true;
                     nnls_B            = pr;
                     nnls_errors       = pr_error;
                  } else {
                     found_nnls_model    = true;
                     nnls_A[qsl_tmp[0]]  = pr;
                     nnls_x[qsl_tmp[0]]  = 0;
                     nnls_mw[qsl_tmp[0]] = qsl_tmp[1].toDouble();
                  }
               }
#if defined(DEBUG_PR)
               TSO << "pr values (" << pr.size() << "): ";
               
               for ( unsigned int i = 0; i < pr.size(); i++ )
               {
                  TSO << pr[i] << ",";
               }
               TSO << endl;
#endif
               // plot it
               vector < double > this_r = r;
               // r has the ordinates for the longest data, some will likely be shorter
               if ( r.size() > pr.size() ) {
                  this_r.resize(pr.size());
               }
               // occasionally one may have a zero in the last p(r) position
               if ( pr.size() > r.size() ) {
                  pr.resize(r.size());
               }
               if ( pr_error.size() && pr_error.size() != pr.size() ) {
                  editor_msg( "red"
                              ,QString(
                                       us_tr(
                                             "S.D.s found in %1, but they are not of the same length (%2) as the P(r) data points (%3), dropping S.D.s"
                                             )
                                       )
                              .arg( qsl_pr[0] )
                              .arg( pr_error.size() )
                              .arg( pr.size() )
                              );
                  // US_Vector::printvector3( "pr sd mismatch, r, pr, pr_error:", r, pr, pr_error );
                  pr_error.clear();
               }

               for ( unsigned int i = 0; i < pr.size(); i++ ) {
                  sum_pr[i] += pr[i];
                  sum_pr2[i] += pr[i] * pr[i];
                  if ( us_isnan(pr[i]) ) {
                     TSO << QString("WARNING: isnan pr[%1] for %2\n").arg(i).arg(qsl_tmp[0]);
                  }
                  if ( us_isnan(sum_pr[i]) ) {
                     TSO << QString("WARNING: isnan sum_pr[%1] for %2\n").arg(i).arg(qsl_tmp[0]);
                  }
                  if ( us_isnan(sum_pr2[i]) ) {
                     TSO << QString("WARNING: isnan sum_pr2[%1] for %2\n").arg(i).arg(qsl_tmp[0]);
                  }
               }
               sum_count++;

               if ( !(create_avg && only_plot_stats) && !run_nnls && !run_best_fit ){
                  (*remember_mw)[use_filename + " " + qsl_tmp[0]] = tmp_mw;
                  (*remember_mw_source)[use_filename + " " + qsl_tmp[0]] = "loaded from csv";
                  plot_one_pr( this_r, pr, pr_error, use_filename + " " + qsl_tmp[0], skip_mw );
                  compute_rg_to_progress( this_r, pr, use_filename + " " + qsl_tmp[0] );
               }
            }
         }
         if ( pr_mws.size() )
         {
            pr_mw_avg /= pr_mws.size();
         }
         (*remember_mw)[QFileInfo(filename).fileName() + " Average"] = pr_mw_avg;
         (*remember_mw_source)[QFileInfo(filename).fileName() + " Average"] = "computed average from selected csv models";
         (*remember_mw)["Standard deviation"] = pr_mw_avg;
         (*remember_mw_source)["Standard deviation"] = "computed average from selected csv models";
         (*remember_mw)[QFileInfo(filename).fileName() + " Average minus 1 std dev"] = pr_mw_avg;
         (*remember_mw_source)[QFileInfo(filename).fileName() + " Average minus 1 std dev"] = "computed average from selected csv models";
         (*remember_mw)[QFileInfo(filename).fileName() + " Average plus 1 std dev"] = pr_mw_avg;
         (*remember_mw_source)[QFileInfo(filename).fileName() + " Average plus 1 std dev"] = "computed average from selected csv models";

         (*remember_mw)["Average"] = pr_mw_avg;
         (*remember_mw_source)["Average"] = "computed average from selected csv models";
         (*remember_mw)["Standard deviation"] = pr_mw_avg;
         (*remember_mw_source)["Standard deviation"] = "computed average from selected csv models";
         (*remember_mw)["Average minus 1 standard deviation"] = pr_mw_avg;
         (*remember_mw_source)["Average minus 1 standard deviation"] = "computed average from selected csv models";
         (*remember_mw)["Average plus 1 standard deviation"] = pr_mw_avg;
         (*remember_mw_source)["Average plus 1 standard deviation"] = "computed average from selected csv models";

         (*remember_mw)[csv_filename + " Model"] = pr_mw_avg;
         (*remember_mw_source)[csv_filename + " Model"] = "computed average from selected csv models";
         (*remember_mw)[csv_filename + " Residual"] = pr_mw_avg;
         (*remember_mw_source)[csv_filename + " Residual"] = "computed average from selected csv models";
         (*remember_mw)[csv_filename + " Target"] = pr_mw_avg;
         (*remember_mw_source)[csv_filename + " Target"] = "computed average from selected csv models";

         if ( create_avg && sum_count && !run_nnls && !run_best_fit )
         {
            vector < double > no_error;
            pr = sum_pr;
            for ( unsigned int i = 0; i < sum_pr.size(); i++ )
            {
               pr[i] /= (double)sum_count;
            }
            vector < double > this_r = r;
            if ( r.size() > pr.size() )
            {
               this_r.resize(pr.size());
            }
            if ( pr.size() > r.size() )
            {
               pr.resize(r.size());
            }
            vector < double > pr_avg = pr;

            double pr_avg_area = compute_pr_area(pr_avg, r);

            plot_one_pr( this_r, pr, no_error, QFileInfo(filename).fileName() + " Average", skip_mw );
            compute_rg_to_progress( this_r, pr, QFileInfo(filename).fileName() + " Average" );

            vector < double > pr_std_dev;
            vector < double > pr_avg_minus_std_dev;
            vector < double > pr_avg_plus_std_dev;

            double pr_mw_std_dev = 0e0;

            if ( create_std_dev && sum_count > 2 )
            {
               vector < double > std_dev(sum_pr.size());
               for ( unsigned int i = 0; i < sum_pr.size(); i++ )
               {
                  double tmp_std_dev = 
                     sum_pr2[i] - ((sum_pr[i] * sum_pr[i]) / (double)sum_count);
                  std_dev[i] = 
                     tmp_std_dev > 0e0 ?
                     sqrt( ( 1e0 / ((double)sum_count - 1e0) ) * tmp_std_dev ) : 0e0;
                  if ( us_isnan(sum_pr[i]) ) 
                  {
                     TSO << QString("WARNING when calc'ing std dev: isnan sum_pr[%1]\n").arg(i);
                  }
                  if ( us_isnan(sum_pr2[i]) ) 
                  {
                     TSO << QString("WARNING when calc'ing std dev: isnan sum_pr2[%1]\n").arg(i);
                  }
                  if ( us_isnan(std_dev[i]) ) 
                  {
                     TSO << 
                        QString("WARNING when calc'ing std dev: isnan std_dev[%1]:\n"
                                " sum_pr[%2]  == %3\n"
                                " sum_pr2[%4] == %5\n"
                                " sum_count   == %6\n"
                                " sum_pr2 - ((sum_pr * sum_pr) / sum_count)   == %7\n"
                                )
                        .arg(i)
                        .arg(i).arg(sum_pr[i])
                        .arg(i).arg(sum_pr2[i])
                        .arg(sum_count)
                        .arg(sum_pr2[i] - ((sum_pr[i] * sum_pr[i]) / (double)sum_count) )
                        ;
                  }
               }

               pr_std_dev = std_dev;

               double pr_std_dev_area = compute_pr_area(pr_std_dev, r);

               double area_sd_avg = 
                  ( pr_avg_area > 0e0 && pr_std_dev_area > 0e0 ) ? 
                  pr_std_dev_area / pr_avg_area :
                  0e0 ;

               if ( area_sd_avg >= 1e0 )
               {
                  area_sd_avg = 1e0;
               }
               
               pr_mw_std_dev = pr_mw_avg * area_sd_avg;
               (*remember_mw)["Standard deviation"] = pr_mw_std_dev;
               (*remember_mw)[QFileInfo(filename).fileName() + " Average minus 1 std dev"] = pr_mw_avg - pr_mw_std_dev;
               (*remember_mw)[QFileInfo(filename).fileName() + " Average plus 1 std dev"] = pr_mw_avg + pr_mw_std_dev;

               // TSO << QString("sd mw calcs:\n"
               // "area_sd_avg mult: %1\n"
               // "pr_mw_avg:        %2\n"
               // "pr_mw_sd_minus_1: %3\n"
               // "pr_mw_sd_plus_1:  %4\n"
               // )
               // .arg(area_sd_avg)
               // .arg(pr_mw_avg)
               // .arg(pr_mw_avg - pr_mw_std_dev)
               // .arg(pr_mw_avg + pr_mw_std_dev)
               // ;

#if defined(DEBUG_STD_DEV)
               TSO << "sum pr (" << sum_pr.size() << "): ";
               
               for ( unsigned int i = 0; i < sum_pr.size(); i++ )
               {
                  TSO << sum_pr[i] << ",";
               }
               TSO << endl;
               
               TSO << "sum pr2 (" << sum_pr2.size() << "): ";
               
               for ( unsigned int i = 0; i < sum_pr2.size(); i++ )
               {
                  TSO << sum_pr2[i] << ",";
               }
               TSO << endl;
               
               TSO << "std values (" << std_dev.size() << "): ";
               
               for ( unsigned int i = 0; i < std_dev.size(); i++ )
               {
                  TSO << std_dev[i] << ",";
               }
               TSO << endl;
#endif
               pr = sum_pr;
               for ( unsigned int i = 0; i < sum_pr.size(); i++ )
               {
                  pr[i] /= (double)sum_count;
                  pr[i] -= std_dev[i];
               }
               vector < double > this_r = r;
               if ( r.size() > pr.size() )
               {
                  this_r.resize(pr.size());
               }
               if ( pr.size() > r.size() )
               {
                  pr.resize(r.size());
               }
               pr_avg_minus_std_dev = pr;

               plot_one_pr( this_r, pr, no_error, QFileInfo(filename).fileName() + " Average minus 1 std dev", skip_mw );
               
               pr = sum_pr;
               for ( unsigned int i = 0; i < sum_pr.size(); i++ )
               {
                  pr[i] /= (double)sum_count;
                  pr[i] += std_dev[i];
               }
               this_r = r;
               if ( r.size() > pr.size() )
               {
                  this_r.resize(pr.size());
               }
               if ( pr.size() > r.size() )
               {
                  pr.resize(r.size());
               }
               pr_avg_plus_std_dev = pr;

               plot_one_pr(this_r, pr, no_error, QFileInfo(filename).fileName() + " Average plus 1 std dev", skip_mw );
            }
            if ( plotted )
            {
               editor_msg( "black", "P(r) plot done\n");
               plotted = false;
            }
            if ( save_to_csv )
            {
               // TSO << "save_to_csv\n";
               QString fname = 
                  ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "saxs" + SLASH + 
                  csv_filename + "_sprr_" + ((US_Hydrodyn *)us_hydrodyn)->saxs_sans_ext() + ".csv";
               if ( QFile::exists(fname) )
                  // && !((US_Hydrodyn *)us_hydrodyn)->overwrite ) 
               {
                  fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fname);
               }         
               FILE *of = us_fopen(fname, "wb");
               if ( of )
               {
                  //  header: "name","type",r1,r2,...,rn, header info
                  fprintf(of, "\"Name\",\"MW (Daltons)\",\"Area\",\"Type; r:\",%s,%s\n", 
                          vector_double_to_csv(r).toLatin1().data(),
                          header_tag.toLatin1().data());
                  if ( save_original_data )
                  {
                     fprintf(of, "%s\n", qsl_data_lines_plotted.join("\n").toLatin1().data());
                  }
                  fprintf(of, "\"%s\",%.2f,%.2f,\"%s\",%s\n", 
                          "Average",
                          pr_mw_avg,
                          compute_pr_area(pr_avg, r),
                          "P(r)",
                          vector_double_to_csv(pr_avg).toLatin1().data());
                  if ( pr_std_dev.size() )
                  {
                     fprintf(of, "\"%s\",%.2f,%.2f,\"%s\",%s\n", 
                             "Standard deviation",
                             pr_mw_std_dev,
                             compute_pr_area(pr_std_dev, r),
                             "P(r)",
                             vector_double_to_csv(pr_std_dev).toLatin1().data());
                     fprintf(of, "\"%s\",%.2f,%.2f,\"%s\",%s\n", 
                             "Average minus 1 standard deviation",
                             pr_mw_avg - pr_mw_std_dev,
                             compute_pr_area(pr_avg_minus_std_dev, r),
                             "P(r)",
                             vector_double_to_csv(pr_avg_minus_std_dev).toLatin1().data());
                     fprintf(of, "\"%s\",%.2f,%.2f,\"%s\",%s\n", 
                             "Average plus 1 standard deviation",
                             pr_mw_avg + pr_mw_std_dev,
                             compute_pr_area(pr_avg_plus_std_dev, r),
                             "P(r)",
                             vector_double_to_csv(pr_avg_plus_std_dev).toLatin1().data());
                  }
                  if ( !save_original_data )
                  {
                     fprintf(of, "\n\n\"%s\"\n", 
                             QString(" Average of : " + qsl_sel_names.join(";").replace("\"","")).toLatin1().data()
                             );
                  }
                  fclose(of);
                  editor->append(us_tr("Created file: " + fname + "\n"));
               } else {
                  editor_msg( "red", us_tr("ERROR creating file: " + fname + "\n" ) );
               }
            }
         } else {
            if ( run_nnls || run_best_fit )
            {
               nnls_r = r;
               if ( found_nnls_model && found_nnls_target )
               {
                  nnls_header_tag = header_tag;
                  QString use_csv_filename = save_to_csv ? csv_filename : "";
                  (*remember_mw)[use_csv_filename + " Target"] =  (*remember_mw)[nnls_target];
                  (*remember_mw_source)[use_csv_filename + " Target"] =  "copied from target curve";
                  (*remember_mw)[use_csv_filename + " Model"] =  (*remember_mw)[nnls_target];
                  (*remember_mw_source)[use_csv_filename + " Model"] =  "copied from target curve";
                  if ( run_nnls )
                  {

                     {
                        QString rg_msg;
                        {
                           double Rg;
                           QString errormsg;
                           if ( US_Saxs_Util::compute_rg_from_pr( nnls_r, nnls_B, Rg, errormsg ) ) {
                              rg_msg = QString( "%1" ).arg( Rg, 0, 'f', 2 );
                           } else {
                              rg_msg = errormsg;
                           }
                        }
                        double dmax = nnls_r.back();
                        {
                           int i = (int) nnls_r.size() - 1;
                           if ( i > (int) nnls_B.size() - 1 ) {
                              i = (int) nnls_B.size() - 1;
                           }
                           while ( --i >= 0 && nnls_B[i] == 0 ) {
                              dmax = nnls_r[i];
                           }
                        }               
                        
                        nnls_csv_footer << QString( "\"NNLS target:\",\"%1\",,%2,%3" )
                           .arg( QString( "%1" ).arg( nnls_target ).replace( QRegularExpression( "(\"| )" ) , "_" ) )
                           .arg( rg_msg )
                           .arg( dmax )
                           ;
                     }

                     calc_nnls_fit( nnls_target, use_csv_filename );
                     if ( nnls_csv ) {
                        if ( QFile::exists(nnls_csv_filename) )
                        {
                           nnls_csv_filename = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( nnls_csv_filename, 0, this );
                        }

                        QFile f( nnls_csv_filename );
                        if ( f.open(QIODevice::WriteOnly ) ) {
                           QTextStream tso(&f);
                           tso << "\"File\",\"Model\",\"Contribution weight\",\"Rg [A]\",\"Dmax [A]\"\n";
                           tso << nnls_csv_data.join("\n") << "\n";
                           tso << "\n\"Messages:\"\n";
                           tso << nnls_csv_footer.join("\n") << "\n";
                           f.close();
                           editor_msg( "darkblue", QString( us_tr( "Created file: %1\n" ) ).arg( nnls_csv_filename ) );
                        } else {
                           editor_msg( "red", QString( us_tr( "Error attempting to create file: %1\n" ) ).arg( nnls_csv_filename ) );
                        }
                     }
                  }
                  if ( run_best_fit )
                  {
                     calc_best_fit( nnls_target, use_csv_filename );
                  }
               } else {
                  editor->append("NNLS error: could not find target and models in loaded data\n");
               }
            }
            if ( plotted )
            {
               editor_msg( "black", "P(r) plot done\n");
               plotted = false;
            }
         }
         return;
      }

      float mw = 0.0;

      if ( !ext.contains(QRegExp("^sprr(|_(x|n|r))")) )
      {
         // check for gnom output
         QTextStream ts(&f);
         QString tmp;
         unsigned int pos = 0;
         QRegExp qx_mw("molecular weight (\\d+(|\\.\\d+))", Qt::CaseInsensitive );

         while ( !ts.atEnd() )
         {
            tmp = ts.readLine();
            if ( qx_mw.indexIn( tmp ) != -1 ) {
               mw = qx_mw.cap(1).toFloat();
               TSO <<
                  QString(
                          "load pr found gnom mw %1\n"
                          )
                  .arg( mw )
                  ;
               (*remember_mw)[QFileInfo(filename).fileName()] = mw;
               (*remember_mw_source)[QFileInfo(filename).fileName()] = "Found in gnom.out file";
            }
            pos++;
            if ( tmp.contains("Distance distribution  function of particle") ) 
            {
               editor->append("\nRecognized GNOM output.\n");
               startline = pos + 3;
               pop_last = 1;
               break;
            }
         }
         f.close();
         f.open(QIODevice::ReadOnly);
      }

      QTextStream ts(&f);
      //      editor->append(QString("\nLoading pr(r) data from %1 %2\n").arg(filename).arg(res));
      QString firstLine = ts.readLine();
      QRegExp sprr_mw_line("mw\\s+(\\S+)\\s+Daltons");
      if ( sprr_mw_line.indexIn(firstLine) != -1 )
      {
         mw = sprr_mw_line.cap(1).toFloat();
         (*remember_mw)[QFileInfo(filename).fileName()] = mw;
         (*remember_mw_source)[QFileInfo(filename).fileName()] = "loaded from sprr file";
      }
      
      {
         QRegularExpression rx( "^\\s*([0-9.+-eE])+\\s+([0-9.+-eE])+" );
         if ( rx.match( firstLine ).hasMatch() ) {
            // QTextStream(stdout) << "found numeric 1st line in P(r)\n" << firstLine << "\n";
            startline = 0;
            QStringList qsl = firstLine.trimmed().split( QRegExp( "\\s+" ) );
            if ( qsl.size() >= 2 ) {
               r.push_back( qsl[0].toDouble() );
               pr.push_back( qsl[1].toDouble() );
            }
         } else {
            editor->append(firstLine);
         }
      }

      bool normed_pr_present = false;
      while ( startline > 0 )
      {
         if ( ts.readLine().contains( "norm. p(r)", Qt::CaseInsensitive ) ) {
            normed_pr_present = true;
            // qDebug() << "--> normed pr found, will ignore 3rd column";
         }
         startline--;
      }

      // qDebug() << "load pr normal load"; 

      while ( !ts.atEnd() ) {
         // QString     qs  = ts.readLine();
         QStringList qsl = ts.readLine().trimmed().split( QRegularExpression( "\\s+" ) );


         if ( qsl.size() < 2 ) {
            editor_msg( "black"
                        ,QString( us_tr( "Notice: File %1 : improper format line found after r = %2, further data trimmed\n" ) )
                        .arg( QFileInfo(filename).fileName() )
                        .arg( r.back() )
                        );
            break;
         }
         
         // qDebug() << QString( "load_pr() qsl size %1 %2" ).arg( qsl.size() ).arg( qsl.join( "," ) );

         r .push_back( qsl[0].toDouble() );
         pr.push_back( qsl[1].toDouble() );

         if ( !normed_pr_present && qsl.size() > 2 ) {
            pr_error.push_back( qsl[2].toDouble() );
         }
      }
      f.close();
      // US_Vector::printvector3( "pr load 0 r, pr, pr_error", r, pr, pr_error, 6 );

      if ( pr_error.size() && pr.size() != pr_error.size() ) {
         pr_error.clear();
         editor_msg( "darkRed"
                     ,QString( us_tr( "File %1 : some, but not all, data had apparent associated error values, all error values removed" ) )
                     .arg( QFileInfo(filename).fileName() )
                     );
      }
      
      while ( pop_last > 0 && r.size() )
      {
         r.pop_back();
         pr.pop_back();
         pop_last--;
      }

      if ( pr_error.size() ) {
         pr_error.resize( r.size() );
      }

      QString use_filename = QFileInfo(filename).fileName() + " P(r)";
      if ( mw )
      {
         (*remember_mw)[use_filename] = mw;
         (*remember_mw_source)[use_filename] = (*remember_mw_source)[QFileInfo(filename).fileName()];
      }         
      check_pr_grid( r, pr, pr_error );
      plot_one_pr(r, pr, pr_error, use_filename, skip_mw );
      compute_rg_to_progress( r, pr, use_filename );
   }
}

void US_Hydrodyn_Saxs::rescale_iqq_curve( QString scaling_target,
                                          vector < double > &q,
                                          vector < double > &I,
                                          bool do_plot_residuals
                                          )
{
   vector < double > I2;
   QColor plot_color = plot_colors[ plotted_q.size() % plot_colors.size() ];
   rescale_iqq_curve( scaling_target, q, I, I2, plot_color, do_plot_residuals );
}

void US_Hydrodyn_Saxs::rescale_iqq_curve( QString scaling_target,
                                          vector < double > &q,
                                          vector < double > &I,
                                          QColor plot_color,
                                          bool do_plot_residuals
                                          )
{
   vector < double > I2;
   rescale_iqq_curve( scaling_target, q, I, I2, plot_color, do_plot_residuals );
}

void US_Hydrodyn_Saxs::rescale_iqq_curve( QString scaling_target,
                                          vector < double > &q,
                                          vector < double > &I,
                                          vector < double > &I2,
                                          bool do_plot_residuals)
{
   QColor plot_color = plot_colors[ plotted_q.size() % plot_colors.size() ];
   rescale_iqq_curve( scaling_target, q, I, I2, plot_color, do_plot_residuals );
}

void US_Hydrodyn_Saxs::rescale_iqq_curve( QString scaling_target,
                                          vector < double > &q,
                                          vector < double > &I,
                                          vector < double > &/* I2 */,
                                          QColor plot_color,
                                          bool do_plot_residuals
                                          )
{
   if ( !q.size() ||
        scaling_target.isEmpty() ||
        !plotted_iq_names_to_pos.count(scaling_target) )
   {
      TSO << "rescale_iqq_curves() return 1\n";
      return;
   }

   unsigned int iq_pos = plotted_iq_names_to_pos[scaling_target];
   TSO << "scaling target pos is " << iq_pos << endl;
   double target_q_min = plotted_q[iq_pos][0];
   double target_q_max = plotted_q[iq_pos][plotted_q[iq_pos].size() - 1];
   double source_q_min = q[0];
   double source_q_max = q[q.size() - 1];
   double q_min = target_q_min;
   if ( q_min < our_saxs_options->iqq_scale_minq )
   {
      q_min = our_saxs_options->iqq_scale_minq;
   }

   if ( q_min < source_q_min )
   {
      q_min = source_q_min;
   }
   double q_max = target_q_max;

   if ( our_saxs_options->iqq_scale_maxq > 0.0f )
   {
      q_max = our_saxs_options->iqq_scale_maxq;
   }

   if ( q_max > source_q_max )
   {
      q_max = source_q_max;
   }
   
   TSO << QString(
                   "target q_min %1 max %2\n"
                   "source q_min %3 max %4\n"
                   "select q_min %5 max %6\n"
                   )
      .arg(target_q_min)
      .arg(target_q_max)
      .arg(source_q_min)
      .arg(source_q_max)
      .arg(q_min)
      .arg(q_max)
      ;
   
   vector < double > use_q;
   vector < double > use_I;
   vector < double > use_I_error;

   double       avg_std_dev_frac        = 0e0;
   unsigned int avg_std_dev_point_count = 0;

   for ( unsigned int i = 0; i < plotted_q[iq_pos].size(); i++ )
   {
      if ( plotted_q[iq_pos][i] >= q_min &&
           plotted_q[iq_pos][i] <= q_max )
      {
         use_q.push_back(plotted_q[iq_pos][i]);
         use_I.push_back(plotted_I[iq_pos][i]);
         use_I_error.push_back(plotted_I_error[iq_pos][i]);
         if ( plotted_I[ iq_pos ][ i ] &&
              plotted_I_error[ iq_pos ][ i ] <= plotted_I[ iq_pos ][ i ] )
         {
            avg_std_dev_frac += 
               ( plotted_I_error[ iq_pos ][ i ] * plotted_I_error[ iq_pos ][ i ] ) / 
               ( plotted_I[ iq_pos ][ i ] * plotted_I[ iq_pos ][ i ] );
            avg_std_dev_point_count++;
         }
      }
   }
   
   if ( !avg_std_dev_point_count ) {
      avg_std_dev_point_count = 1;
   }

   avg_std_dev_frac = sqrt( avg_std_dev_frac / ( double ) avg_std_dev_point_count );

   TSO << QString("After cropping q to overlap region:\n"
                   "use_q.size == %1\n").arg(use_q.size());
   
   if ( !use_q.size() )
   {
      QMessageBox::warning( this, "UltraScan",
                            QString(us_tr("Could not find sufficient q range overlap\n"
                                       "to scale the loaded data to the selected target")) );
      TSO << "rescale_iqq_curves() return 2\n";
      return;
   }

   vector < double > use_source_I = interpolate(use_q, q, I);
   
   US_Saxs_Util usu;

   double k;
   double chi2;

   bool do_chi2_fitting        = use_SDs_for_fitting_iqq || ( is_nonzero_vector( use_I_error ) && use_I_error.size() == use_I.size() );
   bool do_scale_linear_offset = our_saxs_options->iqq_scale_linear_offset;
   bool do_kratky              = our_saxs_options->iqq_kratky_fit;

   if ( do_chi2_fitting &&
        !is_nonzero_vector( use_I_error ) )
   {
      editor_msg( "dark red", us_tr( "Ignoring experimental errors" ) );
      do_chi2_fitting = false;
   }

   if ( do_chi2_fitting && is_zero_vector( use_I_error ) )
   {
      if ( use_SDs_for_fitting_iqq ) {
         editor_msg( "red", us_tr("Chi^2 fitting requested, but target data has no standard deviation data\n"
                                  "Chi^2 fitting not used\n") );
      }
      do_chi2_fitting = false;
   }

   if ( do_kratky )
   {
      editor_msg( "blue", us_tr( "Kratky fit (q^2*I)\n" ) );
      for ( unsigned int i = 0; i < use_source_I.size(); i++ )
      {
         use_source_I[ i ] *= use_q[ i ] * use_q[ i ];
         use_I       [ i ] *= use_q[ i ] * use_q[ i ];
         if ( do_chi2_fitting )
         {
            use_I_error[ i ] *= use_q[ i ] * use_q[ i ];
         }
      }
   }

   // for ( unsigned int i = 0; i < use_source_I.size(); i++ )
   // {
   //    TSO << QString( "q %1 use_source_I %2 use_I %3\n" )
   //       .arg( q[ i ] )
   //       .arg( use_source_I[ i ] )
   //       .arg( use_I[ i ] );
   // }

   if ( our_saxs_options->iqq_scale_nnls ) {
      // iqq_scale_nnls is an *experimental" option, not enabled for normal usage
      if ( !use_SDs_for_fitting_iqq )
      {
         if ( do_plot_residuals ) {
            editor_msg( "red", us_tr("Chi^2 fitting is currently not compatable with NNLS scaling\n") );
         }
         do_chi2_fitting = false;
      }
      if ( our_saxs_options->iqq_scale_linear_offset )
      {
         if ( do_plot_residuals ) {
            editor_msg( "red", us_tr("Scale with linear offset is not compatable with NNLS scaling\n") );
         }
         do_scale_linear_offset = false;
      }

      usu.nnls_fit( 
                   use_source_I, 
                   use_I, 
                   k, 
                   chi2
                   );
   } else {
      if ( do_plot_residuals ) {
         editor_msg( "dark blue", do_chi2_fitting ? "Chi^2 fitting\n" : "" );
      }
      if ( our_saxs_options->iqq_scale_linear_offset )
      {
         if ( do_plot_residuals ) {
            editor_msg( "red", us_tr("Scale with linear offset is not currently implemented\n") );
         }
         do_scale_linear_offset = false;
      }
      if ( do_chi2_fitting && !is_nonzero_vector( use_I_error ) )
      {
         if ( do_plot_residuals ) {
            editor_msg( "red", us_tr("Chi^2 fitting disabled, zeros in target standard deviation\n") );
         }
         do_chi2_fitting = false;
      }
      if ( do_plot_residuals ) {
         editor_msg( "dark blue",
                     QString( us_tr( "fitting range: %1 to %2 with %3 points\n" ) )
                     .arg( use_q[ 0 ] )
                     .arg( use_q.back() )
                     .arg( use_q.size() ) );

         nnls_csv_footer <<
            QString( us_tr( "\"fitting range start, end, points:\",%1,%2,%3" ) )
            .arg( use_q[ 0 ] )
            .arg( use_q.back() )
            .arg( use_q.size() )
            ;
      }

      if ( do_chi2_fitting )
      {
         usu.scaling_fit( 
                         use_source_I, 
                         use_I, 
                         use_I_error,
                         k, 
                         chi2
                         );
      } else {
         usu.scaling_fit( 
                         use_source_I, 
                         use_I, 
                         k, 
                         chi2
                         );
      }
   }
   
   if ( our_saxs_options->iqq_scale_play ) 
   {
      bool ok = true;
      double ournchi = do_chi2_fitting ?
         sqrt( chi2 / ( use_I.size() - ( do_scale_linear_offset ? 2 : 1 ) ) ) 
         :
         chi2
         ;

      do {
         vector < double > mult_source_I = use_source_I;
         double res = US_Static::getDouble(
                                              "Scaling value:",
                                              QString(
                                                      "Current scaling of %1 gives a %2 of %3\n"
                                                      "Enter a scaling value or press Cancel to finish\n")
                                              .arg( k )
                                              .arg( do_chi2_fitting ? "nchi" : "RMSD" )
                                              .arg( ournchi, 5 ),
                                              k, 
                                              1e-50,
                                              1e99, 
                                              6, 
                                              &ok, 
                                              this );
         if ( ok ) {
            k = res;
            for ( unsigned int i = 0; i < mult_source_I.size(); i++ )
            {
               mult_source_I[ i ] *= k;
            }
            // compute chi2 or rmsd
            if ( do_chi2_fitting )
            {
               US_Saxs_Util::calc_mychi2( mult_source_I, use_I, use_I_error, chi2 );
               ournchi = sqrt( chi2 / ( use_I.size() - ( do_scale_linear_offset ? 2 : 1 ) ) );
            } else {
               US_Saxs_Util::calc_myrmsd( mult_source_I, use_I, chi2 );
            }
         } 
      } while( ok );
   }

   QString results = "Scaling ";

   if ( our_saxs_options->iqq_scale_minq > 0.0f )
   {
      results += QString("minq: %1 ").arg( our_saxs_options->iqq_scale_minq );
   }

   if ( our_saxs_options->iqq_scale_maxq > 0.0f )
   {
      results += QString("maxq: %1 ").arg( our_saxs_options->iqq_scale_maxq );
   }

   // double chi2_prob = 0e0;
   QString fit_msg = "";

   if ( do_plot_residuals ) {
      if ( do_chi2_fitting )
      {
         // usu.calc_chisq_prob( 0.5 * use_I.size() - ( do_scale_linear_offset ? 2 : 1 ),
         // 0.5 * chi2,
         // chi2_prob );
         fit_msg = 
            QString("chi^2=%1 df=%2 nchi=%3 nchi^2=%4")
            .arg(chi2, 6)
            .arg(use_I.size() - ( do_scale_linear_offset ? 2 : 1 ) )
            .arg(sqrt( chi2 / ( use_I.size() - ( do_scale_linear_offset ? 2 : 1 ) ) ), 5 )
            .arg(chi2 / ( use_I.size() - ( do_scale_linear_offset ? 2 : 1 ) ), 5 )
            ;

         nnls_csv_footer
            << QString( "\"chi^2\",%1" ) .arg( chi2, 6 )
            << QString( "\"df\",%1" )    .arg(use_I.size() - ( do_scale_linear_offset ? 2 : 1 ) )
            << QString( "\"nchi^2\",%1" ).arg(chi2 / ( use_I.size() - ( do_scale_linear_offset ? 2 : 1 ) ), 5 )
            << QString( "\"nchi\",%1" )  .arg(sqrt( chi2 / ( use_I.size() - ( do_scale_linear_offset ? 2 : 1 ) ) ), 5 )
            ;

         if ( avg_std_dev_frac )
         {
            fit_msg += QString( " r_sigma=%1 nchi*r_sigma=%2 " )
               .arg( avg_std_dev_frac ) 
               .arg( avg_std_dev_frac * sqrt( chi2 / ( use_I.size() - ( do_scale_linear_offset ? 2 : 1 ) ) ), 5 );

            nnls_csv_footer
               << QString( "\"r_sigma\",%1" )     .arg( avg_std_dev_frac ) 
               << QString( "\"nchi*r_sigma\",%1" ).arg( avg_std_dev_frac * sqrt( chi2 / ( use_I.size() - ( do_scale_linear_offset ? 2 : 1 ) ) ), 5 )
               ;
         }
      } else {
         fit_msg = QString("RMSD=%1").arg(chi2, 5);
         nnls_csv_footer
            << QString( "\"RMSD\",%1" ).arg(chi2, 5)
            ;
      }

      // compute p value
      {
         vector < double > I1 = use_source_I;
         vector < double > I2 = use_I;
         double p;
         QString emsg;

         if ( pvalue( q, I1, I2, p, emsg ) ) {
            QString p_status = "bad";
            if ( p >= 0.05 ) {
               p_status = "good";
            } else if ( p >= 0.01 ) {
               p_status = "fair";
            }
         
            fit_msg += QString( " P-value=%1 (%2)" ).arg( p ).arg( p_status );
            nnls_csv_footer
               << QString( "\"P value\",%1,\"%2\"" ).arg( p ).arg( p_status );
            ;
         } else {
            qDebug() << emsg;
         }
      }
      
      results += 
         QString("factor: %1 %2\n")
         .arg( k )
         .arg( fit_msg );

      nnls_csv_footer
         << QString("\"Scaling factor\",%1" ).arg( k )
         ;

      editor->append(results);
   }

   last_rescaling_multiplier = k;
   last_rescaling_offset     = 0e0;
   last_rescaling_chi2       = chi2;
   if ( do_chi2_fitting )
   {
      last_rescaling_chi2    = sqrt( chi2 / ( use_I.size() - ( do_scale_linear_offset ? 2 : 1 ) ) );
   }

   for ( unsigned int i = 0; i < I.size(); i++ )
   {
      I[i] = k * I[i];
   }

   for ( unsigned int i = 0; i < use_source_I.size(); i++ )
   {
      use_source_I[i] = k * use_source_I[i];
   }
   
   if ( do_plot_residuals ) {
      display_iqq_residuals( scaling_target, 
                             use_q,
                             use_I,
                             use_source_I,
                             plot_color,
                             use_I_error
                             );
   }
   
   // check this, as I2 may need to be interpolated
   // if ( I2.size() )
   //   {
   //      for ( unsigned int i = 0; i < I2.size(); i++ )
   //      {
   //         I2[i] = k * I2[i];
   //      }
   //      display_iqq_residuals( scaling_target, 
   //                             use_q,
   //                             I2,
   //                             save_use_I );
   //   }

}

void US_Hydrodyn_Saxs::rescale_iqq_curve_using_last_rescaling( vector < double > &I, bool use_offset )
{
   for ( unsigned int i = 0; i < I.size(); i++ )
   {
      I[ i ] *= last_rescaling_multiplier;
      if ( use_offset )
      {
         I[ i ] += last_rescaling_offset;
      }
   }
}

bool US_Hydrodyn_Saxs::select_from_directory_history( QString &dir, QWidget *parent )
{
   return ((US_Hydrodyn *)us_hydrodyn)->select_from_directory_history( dir, parent );
}

   
void US_Hydrodyn_Saxs::add_to_directory_history( QString filename, bool accessed )
{
   ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( filename, accessed );
}

vector < double > US_Hydrodyn_Saxs::range_crop( vector < double > &q, vector < double > &I )
{
   if ( !our_saxs_options->iqq_scale_minq &&
        !our_saxs_options->iqq_scale_maxq )
   {
      return I;
   }

   vector < double > result;
   for ( unsigned int i = 0; i < q.size(); i++ )
   {
      if ( ( !our_saxs_options->iqq_scale_minq || q[ i ] >= our_saxs_options->iqq_scale_minq ) &&
           ( !our_saxs_options->iqq_scale_maxq || q[ i ] <= our_saxs_options->iqq_scale_maxq ) )
      {
         result.push_back( I[ i ] );
      }
   }
   return result;
}


void US_Hydrodyn_Saxs::load_sans( QString filename, bool just_plotted_curves )
{
   if ( just_plotted_curves )
   {
      load_iqq_csv( "", true );
      return;
   }

   if ( filename.isEmpty() )
   {
      QString use_dir = 
         our_saxs_options->path_load_saxs_curve.isEmpty() ?
         USglobal->config_list.root_dir + SLASH + "somo" + SLASH + "saxs" :
         our_saxs_options->path_load_saxs_curve;
      select_from_directory_history( use_dir, this );
      filename = QFileDialog::getOpenFileName( this , "Open" , use_dir , "All files (*);;"
                                              "ssans files (*.ssans);;"
                                              "csv files (*.csv);;"
                                              "int files [cryson] (*.int);;"
                                              "fit files [cryson] (*.fit)" , &load_saxs_sans_selected_filter );

      if ( filename.isEmpty() )
      {
         return;
      }
      add_to_directory_history( filename );
   }

   plotted = false;
   QFile f(filename);
   our_saxs_options->path_load_saxs_curve = QFileInfo(filename).absolutePath();
   QString ext = QFileInfo(filename).suffix().toLower();

   if ( ext == "pdb" || ext == "PDB" )
   {
      QMessageBox::warning( this, "UltraScan",
                            QString(us_tr("Can not load a PDB file as a curve: ")
                                    + filename ) );
      return;
   }

   if ( ext == "csv" )
   {
      load_iqq_csv( filename );
      return;
   }

   vector < double > I;
   vector < double > I_error;
   vector < double > I2;
   vector < double > q;
   vector < double > q2;
   double new_I;
   double new_I_error;
   double new_I2;
   double new_q;
   unsigned int Icolumn = 1;
   unsigned int I_errorcolumn = 0;
   bool dolog10 = false;
   QString res = "";
   unsigned int Icolumn2 = 0;
   QString tag1;
   QString tag2;

   // scaling fields
   QString scaling_target = "";

   // bool do_crop = false;

   if ( f.open(QIODevice::ReadOnly) )
   {
      QTextStream ts(&f);
      vector < QString > qv;
      QStringList qsl;
      while ( !ts.atEnd() )
      {
         QString qs = ts.readLine();
         qv.push_back(qs);
         qsl << qs;
      }
      f.close();
      if ( !qv.size() )
      {
         QMessageBox::warning( this, "UltraScan",
                               QString(us_tr("The file ")
                                       + filename + us_tr(" is empty.")) );
         return;
      }

      unsigned int number_of_fields = 0;
      if ( qv.size() > 3 )
      {
         QString test_line = qv[2];
         test_line.replace(QRegExp("^\\s+"),"");
         test_line.replace(QRegExp("\\s+$"),"");
         QStringList test_list = (test_line).split( QRegExp("\\s+") , Qt::SkipEmptyParts );
         number_of_fields = test_list.size();
         TSO << "number of fields: " << number_of_fields << endl;
      }

      set_scaling_target( scaling_target );

      if ( ext == "int" ) 
      {
         //         dolog10 = true;
         QStringList lst;
         lst << "I(q)   Difference intensity"
             << "Ia(q)  Atomic scattering"
             << "Ic(q)  Shape scattering"
             << "Ib(q)  Border layer scattering Ib(q)";
         bool ok;
         
         if ( !our_saxs_options->crysol_default_load_difference_intensity )
         {
            res = US_Static::getItem(
                                        "Crysol's .int format has four available datasets", 
                                        "Select the set you wish to plot::", lst, 0, false, &ok,
                                        this );
            if ( ok ) {
               // user selected an item and pressed OK
               Icolumn = 0;
               if ( res.contains(QRegExp("^I.q. ")) ) 
               {
                  Icolumn = 1;
               } 
               if ( res.contains(QRegExp("^Ia.q. ")) ) 
               {
                  Icolumn = 2;
               } 
               if ( res.contains(QRegExp("^Ic.q. ")) ) 
               {
                  Icolumn = 3;
               } 
               if ( res.contains(QRegExp("^Ib.q. ")) ) 
               {
                  Icolumn = 4;
               } 
               if ( !Icolumn ) 
               {
                  cerr << "US_Hydrodyn_Saxs::load_saxs : unknown type error" << endl;
                  return;
               }
               TSO << " column " << Icolumn << endl;
            } 
            else
            {
               return;
            }
         } else {
            Icolumn = 1;
         }
      }
      if ( ext == "dat" ) 
      {
         // foxs?
         // do_crop = true;

         Icolumn = 1;
         I_errorcolumn = 2;
         if ( qsl.filter("exp_intensity").size() )
         {
            I_errorcolumn = 0;
            
            switch ( QMessageBox::question(this, 
                                           us_tr("UltraScan Notice"),
                                           QString(us_tr("Please note:\n\n"
                                                      "The file appears to have both experiment and model data\n"
                                                      "What would you like to do?\n"))
                                           ,
                                           us_tr("&Load only experimental"),
                                           us_tr("&Load only the model"),
                                           us_tr("&Load both"),
                                           2, // Default
                                           0 // Escape == button 0
                                           ) )
            {
            case 0 : 
               Icolumn = 1;
               tag1 = " Experimental";
               break;
            case 1 : 
               Icolumn = 2;
               tag1 = " Model";
               break;
            case 2 : 
               Icolumn = 1;
               Icolumn2 = 2;
               tag1 = " Experimental";
               tag2 = " Model";
               break;
            default :
               // what happended here?
               return;
               break;
            }
         }             
      }
      if ( ext == "fit" ) 
      {
         // do_crop = true;

         Icolumn = 2;
         I_errorcolumn = 0;
      }
      if ( ext == "ssaxs" ) 
      {
         //         dolog10 = true;
         if ( number_of_fields >= 4 )
         {
            QStringList lst;
            lst << "I(q)   Difference intensity"
                << "Ia(q)  Atomic scattering"
                << "Ic(q)  Shape scattering";
            bool ok;
            res = US_Static::getItem(
                                        "There are three available datasets", 
                                        "Select the set you wish to plot::", lst, 0, false, &ok,
                                        this );
            if ( ok ) {
               // user selected an item and pressed OK
               Icolumn = 0;
               if ( res.contains(QRegExp("^I.q. ")) ) 
               {
                  Icolumn = 1;
               } 
               if ( res.contains(QRegExp("^Ia.q. ")) ) 
               {
                  Icolumn = 2;
               } 
               if ( res.contains(QRegExp("^Ic.q. ")) ) 
               {
                  Icolumn = 3;
               } 
               if ( !Icolumn ) 
               {
                  cerr << "US_Hydrodyn_Saxs::load_saxs : unknown type error" << endl;
                  return;
               }
               TSO << " column " << Icolumn << endl;
            } 
            else
            {
               return;
            }
         } else {
            Icolumn = 1;
         }
      }
      editor->append(QString("Loading SAXS data from %1 %2\n").arg(filename).arg(res));
      editor->append(qv[0]);
      double units = 1.0;
      if ( our_saxs_options->iq_scale_ask )
      {
         switch( QMessageBox::information( this, 
                                           us_tr("UltraScan"),
                                           us_tr("Is this file in 1/Angstrom or 1/nm units?"),
                                           "1/&Angstrom", 
                                           "1/&nm", 0,
                                           0,      // Enter == button 0
                                           1 ) ) { // Escape == button 2
         case 0: // load it as is
            units = 1.0;
            break;
         case 1: // rescale
            units = 0.1;
            break;
         } 
      } else {
         if ( our_saxs_options->iq_scale_angstrom ) 
         {
            units = 1.0;
         } else {
            units = 0.1;
         }
      }

      QRegExp rx_ok_line("^(\\s+|-\\d+|\\d+|\\.|\\d(E|e)(\\+|-|\\d))+$");
      rx_ok_line.setMinimal( true );
      for ( unsigned int i = 1; i < (unsigned int) qv.size(); i++ )
      {
         if ( qv[i].contains(QRegExp("^#")) ||
              rx_ok_line.indexIn( qv[i] ) == -1 )
         {
            TSO << "not ok: " << qv[i] << endl; 
            continue;
         }
         
         // QStringList tokens = (qv[i].replace(QRegExp("^\\s+").split( QRegExp("\\s+") , Qt::SkipEmptyParts ),""));
         QStringList tokens;
         {
            QString qs = qv[i].replace(QRegExp("^\\s+"),"");
            tokens = (qs ).split( QRegExp("\\s+") , Qt::SkipEmptyParts );
         }
         if ( (unsigned int) tokens.size() > Icolumn )
         {
            new_q = tokens[0].toDouble();
            new_I = tokens[Icolumn].toDouble();
            if ( I_errorcolumn && (unsigned int) tokens.size() > I_errorcolumn )
            {
               new_I_error = tokens[I_errorcolumn].toDouble();
               if ( our_saxs_options->iqq_expt_data_contains_variances )
               {
                  new_I_error = sqrt( new_I_error );
               }
            }
            
            if ( Icolumn2 && (unsigned int) tokens.size() > Icolumn2 )
            {
               new_I2 = tokens[Icolumn2].toDouble();
               if ( dolog10 )
               {
                  new_I2 = log10(new_I2);
               }
            }

            if ( dolog10 )
            {
               new_I = log10(new_I);
            }
            I.push_back(new_I);
            q.push_back(new_q * units);
            if ( I_errorcolumn && (unsigned int) tokens.size() > I_errorcolumn )
            {
               I_error.push_back(new_I_error);
            }
            if ( Icolumn2 && (unsigned int) tokens.size() > Icolumn2 )
            {
               I2.push_back(new_I2);
            }
         }
      }

      TSO << "q_range after load: " << q[0] << " , " << q[q.size() - 1] << endl;

      // turn off cropping
      // TSO << QFileInfo(filename).fileName() << endl;
      // if ( 0 &&  do_crop )
      // {
      //    if ( Icolumn2 )
      //    {
      //       q2 = q;
      //       crop_iq_data(q2, I2);
      //    }
      //    if ( I_error.size() )
      //    {
      //       crop_iq_data(q, I, I_error );
      //    } else {
      //       crop_iq_data(q, I);
      //    }
      // }
         
      // TSO << "q_range after crop: " << q[0] << " , " << q[q.size() - 1] << endl;

      if ( q.size() &&
           !scaling_target.isEmpty() && 
           plotted_iq_names_to_pos.count(scaling_target) )
      {
         if ( Icolumn2 )
         {
            rescale_iqq_curve( scaling_target, q, I, I2 );
         } else {
            rescale_iqq_curve( scaling_target, q, I );
         }
         if ( I_error.size() )
         {
            rescale_iqq_curve_using_last_rescaling( I_error );
         }
      }

      if ( q.size() )
      {
         if ( I_error.size() )
         {
            editor_msg(
                       our_saxs_options->iqq_expt_data_contains_variances ?
                       "red" :
                       "dark blue"
                       , 
                       our_saxs_options->iqq_expt_data_contains_variances ?
                       "Loaded variance data\n" :
                       "Loaded standard deviation data\n"
                       );
            plot_one_iqq(q, I, I_error, QFileInfo(filename).fileName() + tag1);
         } else {
            plot_one_iqq(q, I, QFileInfo(filename).fileName() + tag1);
         }
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header =
            QString("")
            .sprintf(
                     "Saxs curves from %s"
                     , filename.toLatin1().data()
                     );
         for ( unsigned int i = 0; i < q.size(); i++ )
         {
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.push_back(q[i]);
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.push_back(I[i]);
         }
      }
      if ( q2.size() )
      {
         plot_one_iqq(q2, I2, QFileInfo(filename).fileName() + tag2);
      }
      if ( plotted )
      {
         editor_msg( "black", "I(q) plot done\n");
         plotted = false;
      }

      cb_guinier   ->setChecked( false );
      cb_user_range->setChecked( false );
      set_guinier();
   }
}

void US_Hydrodyn_Saxs::load_gnom()
{
   cb_guinier->setChecked( false );
   set_guinier();
   // map < QString, QString > params;
   // params[ "wild" ] = "10.7";
   // US_SAS_Dammin * usd = new US_SAS_Dammin( (US_Hydrodyn *)us_hydrodyn, 41e0, params );
   // usd->show();
   // return;

   plotted = false;
   QString use_dir = 
      our_saxs_options->path_load_gnom.isEmpty() ?
      USglobal->config_list.root_dir + SLASH + "somo" + SLASH + "saxs" :
      our_saxs_options->path_load_gnom;

   select_from_directory_history( use_dir, this );

   QString filename = QFileDialog::getOpenFileName( this , windowTitle() , use_dir , "*.out" );
   if (filename.isEmpty())
   {
      return;
   }
   add_to_directory_history( filename );

   QFile f(filename);
   our_saxs_options->path_load_gnom = QFileInfo(filename).absolutePath();
   QString ext = QFileInfo(filename).suffix().toLower();
   bool plot_gnom = false;
   vector < double > gnom_Iq_reg;
   vector < double > gnom_Iq_exp;
   vector < double > gnom_q;

   if ( f.open(QIODevice::ReadOnly) )
   {
      QStringList qsl_gnom;
      {
         QTextStream ts(&f);
         while ( !ts.atEnd() ) {
            qsl_gnom << ts.readLine();
         }
      }
      f.close();
      f.open(QIODevice::ReadOnly);
      bool ask_save_mw_to_gnom;
      double gnom_mw = 0e0;

      {
         QRegExp qx_mw("molecular weight (\\d+(|\\.\\d+))", Qt::CaseInsensitive );
         QStringList mwline = qsl_gnom.filter(qx_mw);
         ask_save_mw_to_gnom = !mwline.size();

         if ( mwline.size() ) {

            if ( qx_mw.indexIn(mwline[0]) == -1 )
            {
               TSO << QString("qx_mw.search of <%1> for molecular weight failed!\n").arg(mwline[0]);
               gnom_mw = 0e0;
            } else {
               TSO << QString("mwline cap 0 <%1> cap 1 <%2>\n").arg(qx_mw.cap(0)).arg(qx_mw.cap(1));
               gnom_mw = qx_mw.cap(1).toDouble();
            }
            if ( mwline.size() > 1 )
            {
               if ( !((US_Hydrodyn *)us_hydrodyn)->gui_script ) {
                  US_Static::us_message(us_tr("Please note:"), 
                                        QString(us_tr("There are multiple molecular weight lines in the gnom file\n"
                                                      "Using the first one found (%1 Daltons)")).arg(gnom_mw));
               }
            }
            if ( gnom_mw > 0e0 )
            {
               (*remember_mw)[QFileInfo(filename).fileName()] = gnom_mw;
               (*remember_mw_source)[QFileInfo(filename).fileName()] = "Found in gnom.out file";
            }
         } else {
            TSO << "mwline empty\n";
         }
      }

      double units = 1;
      if ( our_saxs_options->iq_scale_ask )
      {
         switch( QMessageBox::information( this, 
                                           us_tr("UltraScan"),
                                           us_tr("Is this file in 1/Angstrom or 1/nm units?"),
                                           "1/&Angstrom", 
                                           "1/&nm", 0,
                                           0,      // Enter == button 0
                                           1 ) ) { // Escape == button 2
         case 0: // load it as is
            units = 1.0;
            break;
         case 1: // rescale
            units = 0.1;
            break;
         } 
      } else {
         if ( our_saxs_options->iq_scale_angstrom ) 
         {
            units = 1.0;
         } else {
            units = 0.1;
         }
      }
         
      QTextStream ts(&f);
      QRegExp iqqh("^\\s*S\\s+J EXP\\s+ERROR\\s+J REG\\s+I REG\\s*$");
      QRegExp prrh("^\\s*R\\s+P\\(R\\)\\s+ERROR\\s*$");
      QRegExp rx2("^\\s*(\\S*)\\s+(\\S*)\\s*$");
      QRegExp rx3("^\\s*(\\S*)\\s+(\\S*)\\s+(\\S*)\\s*$");
      QRegExp rx5("^\\s*(\\S*)\\s+(\\S*)\\s+(\\S*)\\s+(\\S*)\\s+(\\S*)\\s*$");
      QRegExp rxinputfile("Input file.s. : (\\S+)\\s*$");
      QString tmp;
      vector < QString > datafiles;
      bool sprr_saved = false;
      while ( !ts.atEnd() )
      {
         tmp = ts.readLine();
         if ( rxinputfile.indexIn(tmp) != -1 ) 
         {
            // datafiles.push_back(rxinputfile.cap(1).trimmed());
            continue;
         }
         if ( iqqh.indexIn(tmp) != -1 )
         {
            vector < double > I_exp;
            vector < double > I_reg;
            vector < double > q;
            // TSO << "load_gnom() start of iqq\n";
            ts.readLine(); // blank line
            while ( !ts.atEnd() )
            {
               tmp = ts.readLine();
               if ( rx5.indexIn(tmp) != -1 )
               {
                  q.push_back(rx5.cap(1).toDouble() * units );
                  I_exp.push_back(rx5.cap(2).toDouble());
                  I_reg.push_back(rx5.cap(5).toDouble());
                  // cout << "iqq point: " << rx5.cap(1).toDouble() << " " << rx5.cap(5).toDouble() << endl;
               } else {
                  // end of iqq?
                  if ( rx2.indexIn(tmp) == -1 )
                  {
                     plot_gnom = true;
                     gnom_Iq_reg = I_reg;
                     gnom_Iq_exp = I_exp;
                     gnom_q = q;
                     // plot_one_iqq(q, I, QFileInfo(filename).fileName());
                     // if ( plotted )
                     // {
                     //   editor->setParagraphBackgroundColor ( editor->paragraphs() - 1, QColor("white") );
                     //   editor->append("I(q) vs q plot done\n");
                     //   plotted = false;
                     // }
                     break;
                  } else {
                     // cout << "iqq 2 fielder ignored\n";
                  }
               }
            }
            if ( ts.atEnd() &&
                 q.size() )
            {
               plot_gnom = true;
               gnom_Iq_reg = I_reg;
               gnom_Iq_exp = I_exp;
               gnom_q = q;
            }
            continue;
         }
         if ( prrh.indexIn(tmp) != -1 )
         {
            vector < double > r;
            vector < double > pr;
            vector < double > pre;
            // TSO << "load_gnom() start of prr\n";
            ts.readLine(); // blank line
            while ( !ts.atEnd() ) {
               tmp = ts.readLine();
               if ( rx3.indexIn(tmp) != -1 )
               {
                  r  .push_back(rx3.cap(1).toDouble() / units);
                  pr .push_back(rx3.cap(2).toDouble());
                  pre.push_back(rx3.cap(3).toDouble());
                  // cout << "prr point: " << rx3.cap(1).toDouble() << " " << rx3.cap(2).toDouble() << endl;
               } else {
                  // end of prr
                  TSO << "end of prr\n";
                  // QRegExp qx_mw("molecular weight (\\d+(|\\.\\d+))", Qt::CaseInsensitive );
                  // QStringList mwline = qsl_gnom.filter(qx_mw);
                  // if ( mwline.size() )
                  // {
                  //    if ( qx_mw.indexIn(mwline[0]) == -1 )
                  //    {
                  //       TSO << QString("qx_mw.search of <%1> for molecular weight failed!\n").arg(mwline[0]);
                  //       gnom_mw = 0e0;
                  //    } else {
                  //       TSO << QString("mwline cap 0 <%1> cap 1 <%2>\n").arg(qx_mw.cap(0)).arg(qx_mw.cap(1));
                  //       gnom_mw = qx_mw.cap(1).toDouble();
                  //    }
                  //    if ( mwline.size() > 1 )
                  //    {
                  //       if ( !((US_Hydrodyn *)us_hydrodyn)->gui_script ) {
                  //          US_Static::us_message(us_tr("Please note:"), 
                  //                                QString(us_tr("There are multiple molecular weight lines in the gnom file\n"
                  //                                              "Using the first one found (%1 Daltons)")).arg(gnom_mw));
                  //       }
                  //    }
                  //    if ( gnom_mw > 0e0 )
                  //    {
                  //       (*remember_mw)[QFileInfo(filename).fileName()] = gnom_mw;
                  //       (*remember_mw_source)[QFileInfo(filename).fileName()] = "Found in gnom.out file";
                  //    }
                  // } else {
                  //    TSO << "mwline empty\n";
                  // }
                  gnom_mw = get_mw(filename,false);
                  // if ( !mwline.size() )
                  // {
                  //    ask_save_mw_to_gnom = true;
                  // }
                  if ( cb_normalize->isChecked() )
                  {
                     normalize_pr(r, &pr, &pre, get_mw(filename, false));
                  }

                  // TSO << "load_gnom() plot_one_pr 1\n";
                  plot_one_pr(r, pr, pre, QFileInfo(filename).fileName());
                  compute_rg_to_progress( r, pr, QFileInfo(filename).fileName());

                  // save sprr
                  if ( !sprr_saved ) {
                     sprr_saved = true;
                     QString rxstr = "\\..*$";
                     QString dest = USglobal->config_list.root_dir + "/somo/saxs/" + QString( "%1" ).arg( QFileInfo(filename).fileName() ).replace( QRegExp( rxstr ), "_gnom.sprr" );
                     if ( !((US_Hydrodyn *) us_hydrodyn )->overwrite ) {
                        dest = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( dest, 0, this );
                     }

                     double mw = get_mw(filename, false);
                     QStringList sprrcontents;
                     
                     sprrcontents << 
                        QString( "# GNOM P(r) from " + filename + "%1\nR\tP(r)\tSD\n" )
                        .arg(
                             mw == -1e0
                             ? QString( "" )
                             : QString( " mw %1 Daltons" ).arg( mw )
                             )
                        ;

                     for ( int i = 0; i < (int)r.size(); ++i ) {
                        sprrcontents << QString( "%1 %2 %3\n" ).arg( r[i], 0, 'g', 9 ).arg( pr[i], 0, 'g', 9 ).arg( pre[i], 0, 'g', 9 );
                     }
                     QString error;
                     QString putcontents = sprrcontents.join("");
                     if ( !US_File_Util::putcontents( dest, putcontents, error ) ) {
                        editor_msg( "red", error );
                     } else {
                        editor_msg( "blue", QString( "created %1" ).arg( dest ) );
                     }            
                     // save reinterpolated to current bin size sprr
                     {
                        bool ok = true;
                        vector < double > to_grid;
                        // build to grid
                     
                        for ( double x = 0; x < r.back(); x += our_saxs_options->bin_size ) {
                           to_grid.push_back(x);
                        }

                        // interpolate pr as ipr
                        vector < double > ipr;
                     
                        if ( !interpolate( to_grid, r, pr, ipr ) ) {
                           editor_msg( "red", QString( "error interpolating gnom p(r) grid to current binsize %1\n" ).arg( our_saxs_options->bin_size ) );
                           ok = false;
                        }

                        if ( pr.size() != pre.size() ) {
                           qDebug() << "gnom load sprr create, data error length mismatch";
                           ok = false;
                        }
                        
                        // interpolate pr + pre as iprppre
                        vector < double > iprppre;
                        if ( ok ) {
                           vector < double > prppre = pr;
                           for ( int i = 0; i < (int)prppre.size(); ++i ) {
                              prppre[ i ] += pre[ i ];
                           }
                        
                           if ( !interpolate( to_grid, r, prppre, iprppre ) ) {
                              editor_msg( "red", QString( "error interpolating gnom p(r) grid to current binsize %1\n" ).arg( our_saxs_options->bin_size ) );
                              ok = false;
                           }
                        }
                     
                        // interpolate pr - pre as iprmpre
                        vector < double > iprmpre;
                        if ( ok ) {
                           vector < double > prmpre = pr;
                           for ( int i = 0; i < (int)prmpre.size(); ++i ) {
                              prmpre[ i ] -= pre[ i ];
                           }
                        
                           if ( !interpolate( to_grid, r, prmpre, iprmpre ) ) {
                              editor_msg( "red", QString( "error interpolating gnom p(r) grid to current binsize %1\n" ).arg( our_saxs_options->bin_size ) );
                              ok = false;
                           }
                        }

                        if ( ok ) {
                           // take ( iprppre - iprmpre ) / 2 as ipre
                           vector < double > ipre(iprppre.size());
                           for ( int i = 0; i < (int)iprmpre.size(); ++i ) {
                              ipre[ i ] = 0.5 * ( iprppre[ i ] - iprmpre[ i ] );
                           }
                     
                           // // is this different from iprppre - ipr or ipr - iprmpre ?
                           // answer is no, we could get away with one less interpolation
                           // {
                           //    vector < double > iprppremipr = iprppre;
                           //    vector < double > iprmiprmpre = ipr;

                           //    for ( int i = 0; i < (int)ipr.size(); ++i ) {
                           //       iprppremipr[ i ] -= ipr[i];
                           //       iprmiprmpre[ i ] -= iprmpre[i];
                           //    }
                           //    QTextStream(stdout) << US_Vector::qs_vector4( "to_grid, ipre, iprppre-ipr, ipr-iprmpre", to_grid, ipre, iprppremipr, iprmiprmpre );
                           // }
                           // now create reinterpolated version
                           QString dest = USglobal->config_list.root_dir + "/somo/saxs/" + QString( "%1" ).arg( QFileInfo(filename).fileName() ).replace( QRegExp( rxstr ), QString( "_gnom_bin%1.sprr" ).arg( our_saxs_options->bin_size ) );
                           if ( !((US_Hydrodyn *) us_hydrodyn )->overwrite ) {
                              dest = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( dest, 0, this );
                           }

                           double mw = get_mw(filename, false);
                           QStringList sprrcontents;
                     
                           sprrcontents << 
                              QString( "# GNOM P(r) from " + filename + "%1\nR\tP(r)\tSD\n" )
                              .arg(
                                   mw == -1e0
                                   ? QString( "" )
                                   : QString( " mw %1 Daltons" ).arg( mw )
                                   )
                              ;

                           for ( int i = 0; i < (int)to_grid.size(); ++i ) {
                              sprrcontents << QString( "%1 %2 %3\n" ).arg( to_grid[i], 0, 'g', 9 ).arg( ipr[i], 0, 'g', 9 ).arg( ipre[i], 0, 'g', 9 );
                           }
                           // add trailing zero
                           sprrcontents << QString( "%1 0 0\n" ).arg( to_grid.back() + our_saxs_options->bin_size, 0, 'g', 9 );
                           QString error;
                           QString putcontents = sprrcontents.join("");
                           if ( !US_File_Util::putcontents( dest, putcontents, error ) ) {
                              editor_msg( "red", error );
                           } else {
                              editor_msg( "blue", QString( "created %1" ).arg( dest ) );
                           }
                        }
                     }
                  }
                  r  .clear();
                  pr .clear();
                  pre.clear();
                  if ( plotted )
                  {
                     editor_msg( "black", "P(r) plot done\n" );
                     plotted = false;
                  }
                  ts.readAll();
                  break;
               }
            }
            if ( r.size() ) {
               gnom_mw = get_mw(filename,false);
               if ( cb_normalize->isChecked() )
               {
                  normalize_pr(r, &pr, &pre, get_mw(filename, false));
               }
               // TSO << "load_gnom() plot_one_pr 2\n";
               plot_one_pr(r, pr, pre, QFileInfo(filename).fileName());
               compute_rg_to_progress( r, pr, QFileInfo(filename).fileName());

               // save sprr
               if ( !sprr_saved ) {
                  QString rxstr = "\\..*$";
                  QString dest = USglobal->config_list.root_dir + "/somo/saxs/" + QString( "%1" ).arg( QFileInfo(filename).fileName() ).replace( QRegExp( rxstr ), "_gnom.sprr" );
                  if ( !((US_Hydrodyn *) us_hydrodyn )->overwrite ) {
                     dest = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( dest, 0, this );
                  }

                  double mw = get_mw(filename, false);
                  QStringList sprrcontents;
                     
                  sprrcontents << 
                     QString( "# GNOM P(r) from " + filename + "%1\nR\tP(r)\tSD\n" )
                     .arg(
                          mw == -1e0
                          ? QString( "" )
                          : QString( " mw %1 Daltons" ).arg( mw )
                          )
                     ;

                  for ( int i = 0; i < (int)r.size(); ++i ) {
                     sprrcontents << QString( "%1 %2 %3\n" ).arg( r[i], 0, 'g', 9 ).arg( pr[i], 0, 'g', 9 ).arg( pre[i], 0, 'g', 9 );
                  }
                  QString error;
                  QString putcontents = sprrcontents.join("");
                  if ( !US_File_Util::putcontents( dest, putcontents, error ) ) {
                     editor_msg( "red", error );
                  } else {
                     editor_msg( "blue", QString( "created %1" ).arg( dest ) );
                  }
                  // save reinterpolated to current bin size sprr
                  {
                     bool ok = true;
                     vector < double > to_grid;
                     // build to grid
                     
                     for ( double x = 0; x < r.back(); x += our_saxs_options->bin_size ) {
                        to_grid.push_back(x);
                     }

                     // interpolate pr as ipr
                     vector < double > ipr;
                     
                     if ( !natural_spline_interpolate( to_grid, r, pr, ipr ) ) {
                        editor_msg( "red", QString( "error interpolating gnom p(r) grid to current binsize %1\n" ).arg( our_saxs_options->bin_size ) );
                        ok = false;
                     }

                     if ( pr.size() != pre.size() ) {
                        qDebug() << "gnom load sprr create, data error length mismatch";
                        ok = false;
                     }
                        
                     // interpolate pr + pre as iprppre
                     vector < double > iprppre;
                     if ( ok ) {
                        vector < double > prppre = pr;
                        for ( int i = 0; i < (int)prppre.size(); ++i ) {
                           prppre[ i ] += pre[ i ];
                        }
                        
                        if ( !natural_spline_interpolate( to_grid, r, prppre, iprppre ) ) {
                           editor_msg( "red", QString( "error interpolating gnom p(r) grid to current binsize %1\n" ).arg( our_saxs_options->bin_size ) );
                           ok = false;
                        }
                     }
                     
                     // interpolate pr - pre as iprmpre
                     vector < double > iprmpre;
                     if ( ok ) {
                        vector < double > prmpre = pr;
                        for ( int i = 0; i < (int)prmpre.size(); ++i ) {
                           prmpre[ i ] -= pre[ i ];
                        }
                        
                        if ( !natural_spline_interpolate( to_grid, r, prmpre, iprmpre ) ) {
                           editor_msg( "red", QString( "error interpolating gnom p(r) grid to current binsize %1\n" ).arg( our_saxs_options->bin_size ) );
                           ok = false;
                        }
                     }

                     if ( ok ) {
                        // take ( iprppre - iprmpre ) / 2 as ipre
                        vector < double > ipre(iprppre.size());
                        for ( int i = 0; i < (int)iprmpre.size(); ++i ) {
                           ipre[ i ] = 0.5 * ( iprppre[ i ] - iprmpre[ i ] );
                        }
                     
                        // // is this different from iprppre - ipr or ipr - iprmpre ?
                        // answer is no, we could get away with one less interpolation
                        // {
                        //    vector < double > iprppremipr = iprppre;
                        //    vector < double > iprmiprmpre = ipr;

                        //    for ( int i = 0; i < (int)ipr.size(); ++i ) {
                        //       iprppremipr[ i ] -= ipr[i];
                        //       iprmiprmpre[ i ] -= iprmpre[i];
                        //    }
                        //    QTextStream(stdout) << US_Vector::qs_vector4( "to_grid, ipre, iprppre-ipr, ipr-iprmpre", to_grid, ipre, iprppremipr, iprmiprmpre );
                        // }
                        // now create reinterpolated version

                        QString dest = USglobal->config_list.root_dir + "/somo/saxs/" + QString( "%1" ).arg( QFileInfo(filename).fileName() ).replace( QRegExp( rxstr ), QString( "_gnom_bin%1.sprr" ).arg( our_saxs_options->bin_size ) );
                        if ( !((US_Hydrodyn *) us_hydrodyn )->overwrite ) {
                           dest = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( dest, 0, this );
                        }

                        double mw = get_mw(filename, false);
                        QStringList sprrcontents;
                     
                        sprrcontents << 
                           QString( "# GNOM P(r) from " + filename + "%1\nR\tP(r)\tSD\n" )
                           .arg(
                                mw == -1e0
                                ? QString( "" )
                                : QString( " mw %1 Daltons" ).arg( mw )
                                )
                           ;

                        for ( int i = 0; i < (int)to_grid.size(); ++i ) {
                           sprrcontents << QString( "%1 %2 %3\n" ).arg( to_grid[i], 0, 'g', 9 ).arg( ipr[i], 0, 'g', 9 ).arg( ipre[i], 0, 'g', 9 );
                        }
                        // add trailing zero
                        sprrcontents << QString( "%1 0 0\n" ).arg( to_grid.back() + our_saxs_options->bin_size, 0, 'g', 9 );
                        QString error;
                        QString putcontents = sprrcontents.join("");
                        if ( !US_File_Util::putcontents( dest, putcontents, error ) ) {
                           editor_msg( "red", error );
                        } else {
                           editor_msg( "blue", QString( "created %1" ).arg( dest ) );
                        }            
                     }
                  }
               }

               r  .clear();
               pr .clear();
               pre.clear();
               if ( plotted )
               {
                  editor_msg( "black", "P(r) plot done\n" );
                  plotted = false;
               }
            }
         }
      }
      f.close();
      TSO << QString( "ask_save_mw_to_gnom %1 gnom_mw %2\n" ).arg( ask_save_mw_to_gnom ).arg( gnom_mw );
      if ( ask_save_mw_to_gnom && gnom_mw )
      {
         switch( QMessageBox::information( this, 
                                           us_tr("Save GNOM with Molecular Weight"),
                                           QString(us_tr("Do you want to save the molecular weight entered (%1 Daltons) into the gnom.out file?"))
                                           .arg(gnom_mw),
                                           "&Ok",  
                                           "&Cancel", 
                                           0,
                                           0,      // Enter == button 0
                                           1 ) ) { // Escape == button 2
         case 0: // write the file
            {
               QString fname = filename;
               if ( QFile::exists(fname) )
               {
                  fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fname, 0, this);
               }
               QFile f(fname);
               if ( !f.open( QIODevice::WriteOnly ) )
               {
                  QMessageBox::warning( this, "UltraScan",
                                        QString(us_tr("Could not open %1 for writing!")).arg(fname) );
               } else {
                  QTextStream t( &f );
                  t << QString("Molecular weight %1 Daltons\n\n").arg(gnom_mw);
                  t << qsl_gnom.join("\n");
                  t << "\n";
                  f.close();
                  editor->append(QString(us_tr("Created file %1\n")).arg(f.fileName()));
               }
            }                  
            break;
         case 1: // Cancel clicked or Escape pressed
            break;
         }
      }

      if ( datafiles.size() )
      {
         for ( unsigned int i = 0; i < datafiles.size(); i++ )
         {
            QString datafile = our_saxs_options->path_load_gnom + QDir::separator() + datafiles[i];
            if ( QFileInfo(datafile).exists() )
            {
               switch( QMessageBox::information( this, 
                                                 us_tr("UltraScan"),
                                                 us_tr("Found the GNOM associated data file\n") + QFileInfo(datafile).fileName() + "\n" +
                                                 us_tr("Do you want to load it?"),
                                                 "&Ok", 
                                                 "&No", 0,
                                                 0,      // Enter == button 0
                                                 1 ) ) { // Escape == button 2
               case 0: // load it
                  load_saxs(datafile);
               break;
               case 1: // Cancel clicked or Escape pressed
                  break;
               }
            }
         }
      }
      if ( plot_gnom )
      {
         vector < double > gnom_q_reg = gnom_q;

         crop_iq_data(gnom_q, gnom_Iq_exp);
         plot_one_iqq(gnom_q, gnom_Iq_exp, QFileInfo(filename).fileName() + " Experimental");
         crop_iq_data(gnom_q_reg, gnom_Iq_reg);
         plot_one_iqq(gnom_q_reg, gnom_Iq_reg, QFileInfo(filename).fileName() + " From P(r)");
         if ( plotted )
         {
            editor_msg( "black", "I(q) vs q plot done\n" );
            plotted = false;
         }
         cb_guinier->setChecked(false);
         cb_user_range->setChecked(false);
         set_guinier();
      }
   }
}
