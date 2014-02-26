#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_saxs_util.h"
#include "../include/us_hydrodyn_saxs_iqq_load_csv.h"
#include "../include/us_hydrodyn_saxs_load_csv.h"

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

#define SLASH QDir::separator()

#ifdef WIN32
# include <float.h>
//Added by qt3to4:
#include <Q3TextStream>
#endif

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
                               QString(tr("Can not open file ")
                                    + filename ) );
         return;
      }
      Q3TextStream ts(&f);
      while ( !ts.atEnd() )
      {
         qsl << ts.readLine();
      }
      f.close();
   }
   
   QStringList qsl_headers = qsl.grep("\"Name\",\"Type; q:\"");
   if ( qsl_headers.size() == 0 && !just_plotted_curves ) 
   {
      QMessageBox mb(tr("UltraScan Warning"),
                     tr("The csv file ") + filename + tr(" does not appear to contain a correct header.\n"
                                                         "Please manually correct the csv file."),
                     QMessageBox::Critical,
                     Qt::NoButton, Qt::NoButton, Qt::NoButton, 0, 0, 1);
      mb.exec();
      return;
   }
   
   if ( qsl_headers.size() > 1 ) 
   {
      QString ref = qsl_headers[0];
      for ( unsigned int i = 1; i < (unsigned int)qsl_headers.size(); i++ )
      {
         if ( ref != qsl_headers[i] )
         {
            QMessageBox mb(tr("UltraScan Warning"),
                           tr("The csv file ") + filename + tr(" contains multiple different headers\n"
                                                               "Please manually correct the csv file."),
                           QMessageBox::Critical,
                           Qt::NoButton, Qt::NoButton, Qt::NoButton, 0, 0, 1);
            mb.exec();
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
                               QString(tr("There is nothing plotted!")) );
         return;
   }      

   QString grid_target = "";
   vector < double > original_q;

   if ( !just_plotted_curves )
   {
      if ( qsl_plotted_iq_names.size() )
      {
         bool ok;
         grid_target = QInputDialog::getItem(
                                                tr("Set I(q) Grid"),
                                                tr("Select the target plotted data set for the loaded data grid:\n"
                                                   "or Cancel if you wish to interpolate the plotted to the loaded data")
                                                , 
                                                qsl_plotted_iq_names, 
                                                0, 
                                                FALSE, 
                                                &ok,
                                                this );
         if ( ok ) {
            // user selected an item and pressed OK
         } else {
            grid_target = "";
         }
      }

      qsl_q = QStringList::split(",",qsl_headers[0],true);
      if ( qsl_q.size() < 3 )
      {
         QMessageBox mb(tr("UltraScan Warning"),
                        tr("The csv file ") + filename + tr(" does not appear to contain any q values in the header rows.\n"),
                        QMessageBox::Critical,
                        Qt::NoButton, Qt::NoButton, Qt::NoButton, 0, 0, 1);
         mb.exec();
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
      if ( !grid_target.isEmpty() )
      {
         if ( !plotted_iq_names_to_pos.count( grid_target ) )
         {
            editor_msg( "red", QString( tr("Internal error: could not find %1 in plotted data" ) ).arg( grid_target ) );
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
            QStringList qsl_q = QStringList::split( ",", qsl_headers[0], true );
            QString msg = qsl_q[ qsl_q.size() - 1 ];
            msg.replace("\"", "");
            qsl[0] = QString( "\"Name\",\"Type; q:\",%1,\"%2 reinterpolated to q(%3:%4) step %5\"" )
               .arg( vector_double_to_csv( q ) )
               .arg( msg )
               .arg( q.size() ? q[ 0 ] : 0 )
               .arg( q.size() ? q[ q.size() - 1 ] : 0 )
               .arg( q.size() > 1 ? q[ 1 ] - q[ 0 ] : 0 );
            qsl_headers[ 0 ] = qsl[ 0 ];
            cout << qsl[ 0 ] << endl;
            QStringList new_qsl;
            QStringList new_qsl_errors;
            new_qsl << qsl[ 0 ];

            // also also reinterpolate all the data lines
            QStringList qsl_data = qsl.grep(",\"I(q)\",");
            QStringList qsl_sd   = qsl.grep(",\"I(q) sd\",");
            map < QString, QString > sd_map;
            for ( unsigned int i = 0; i < (unsigned int) qsl_sd.size(); i++ )
            {
               QStringList qsl_s = QStringList::split( ",", qsl_sd[i], true );
               sd_map[ qsl_s[ 0 ] ] = qsl_sd[ i ];
            }
               
            US_Saxs_Util usu;

            for ( unsigned int i = 0; i < (unsigned int) qsl_data.size(); i++ )
            {
               QStringList qsl_d = QStringList::split( ",", qsl_data[i], true );
               vector < double > original_i;
               vector < double > original_i_error;
               for ( int j = 2; j < (int) qsl_d.size() - 1; j++ )
               {
                  original_i.push_back( qsl_d[j].toDouble() );
               }
               if ( sd_map.count( qsl_d[ 0 ] ) )
               {
                  QStringList qsl_s = QStringList::split( ",", sd_map[ qsl_d[ 0 ] ], true );
                  for ( int j = 2; j < (int) qsl_s.size() - 1; j++ )
                  {
                     original_i_error.push_back( qsl_s[j].toDouble() );
                  }
               }
               // now interpolate
               vector < double > ni;
               vector < double > ni_error;
               if ( !usu.interpolate_iqq_by_case( original_q, original_i, original_i_error, q, ni, ni_error ) )
               {
                  editor_msg("red", usu.errormsg );
                  QMessageBox::warning( this, "US-SOMO",
                                        QString( tr( "There was an error attempting to interpolate\n"
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
         cout << QString("trying file %1 %2\n").arg(i).arg(filenames[i]);
         QFile f2(filenames[i]);
         if ( !f2.open(QIODevice::ReadOnly) )
         {
            QMessageBox::information( this, "UltraScan",
                                      tr("Can not open the file for reading:\n") +
                                      f2.name()
                                      );
            return;
         }
         
         // append (and possibly interpolate) all the other files
         QStringList qsl2;
         Q3TextStream ts(&f2);
         while ( !ts.atEnd() )
         {
            qsl2 << ts.readLine();
         }
         f2.close();
         QStringList qsl2_headers = qsl2.grep("\"Name\",\"Type; q:\"");
         
         qsl2_headers = qsl2.grep("\"Name\",\"Type; q:\"");
         if ( qsl2_headers.size() == 0 )
         {
            QMessageBox mb(tr("UltraScan Warning"),
                           tr("The csv file ") + f2.name() + tr(" does not appear to contain a correct header.\n"
                                                                "Please manually correct the csv file."),
                           QMessageBox::Critical,
                           Qt::NoButton, Qt::NoButton, Qt::NoButton, 0, 0, 1);
            mb.exec();
            return;
         }
         if ( qsl2_headers.size() > 1 ) 
         {
            QString ref = qsl2_headers[0];
            for ( unsigned int i = 1; i < (unsigned int)qsl2_headers.size(); i++ )
            {
               if ( ref != qsl2_headers[i] )
               {
                  QMessageBox mb(tr("UltraScan Warning"),
                                 tr("The csv file ") + filename + tr(" contains multiple different headers\n"
                                                                     "Please manually correct the csv file."),
                                 QMessageBox::Critical,
                                 Qt::NoButton, Qt::NoButton, Qt::NoButton, 0, 0, 1);
                  mb.exec();
                  return;
               }
            }
         }
         // get the data
         map < QString, QString > name_to_errors_map;
         QStringList qsl_data = qsl2.grep(",\"I(q)\",");
         if ( qsl_data.size() == 0 )
         {
            QMessageBox mb(tr("UltraScan Warning"),
                           tr("The csv file ") + filename + tr(" does not appear to contain any data rows.\n"),
                           QMessageBox::Critical,
                           Qt::NoButton, Qt::NoButton, Qt::NoButton, 0, 0, 1);
            mb.exec();
            return;
         }

         // get possible errors data
         QStringList qsl_errors = qsl2.grep(",\"I(q) sd\",");
         for ( QStringList::iterator it = qsl_errors.begin();
               it != qsl_errors.end();
               it++ )
         {
            QStringList qsl_iq_errors = QStringList::split(",",*it,true);
            name_to_errors_map[ qsl_iq_errors[ 0 ] ] = *it;
         }
         
         // get the q values
         QStringList qsl_q;
         QString header2_tag;
         vector < double > q2;
         
         qsl_q = QStringList::split(",",qsl2_headers[0],true);
         if ( qsl_q.size() < 3 )
         {
            QMessageBox mb(tr("UltraScan Warning"),
                           tr("The csv file ") + f2.name() + tr(" does not appear to contain any q values in the header rows.\n"),
                           QMessageBox::Critical,
                           Qt::NoButton, Qt::NoButton, Qt::NoButton, 0, 0, 1);
            mb.exec();
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
         // cout << QString("max size %1\n").arg(max_size);
         bool all_match = true;
         for ( unsigned int i = 0; i < max_size; i++ )
         {
            if ( q[i] != q2[i] )
            {
               all_match = false;
               break;
            }
         }
         // cout << QString("check all match %1\n").arg(all_match ? "true" : false);
         
         for ( QStringList::iterator it = qsl_data.begin();
               it != qsl_data.end();
               it++ )
         {
            QStringList qsl_iq = QStringList::split(",",*it,true);
            if ( qsl_iq.size() < 3 )
            {
               QString msg = tr("The csv file ") + f2.name() + tr(" does not appear to contain sufficient I(q) values in data row " + qsl_iq[0] + ", skipping\n");
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
                  // cout << "yes, all match\nadding:" << qs_tmp << endl;
               } else {
                  // cout << "not all match, interpolate\n";

                  if ( !interp_msg_done.count(f2.name()) )
                  {
                     interp_msg_done[f2.name()] = true;
                     QString msg = tr("The csv file ") + f2.name() + tr(" will be interpolated\n");
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
                     QStringList qsl_iq_errors = QStringList::split( ",", name_to_errors_map[ qsl_iq[ 0 ] ], true );
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
         QStringList qsl_q = QStringList::split(",",qsl_headers[0],true);
         QString msg = qsl_q[ qsl_q.size() - 1 ];
         msg.replace("\"", "");
         qsl[0] = QString("\"Name\",\"Type; q:\",%1,\"%2 cropped to q(%3:%4)\"")
            .arg( vector_double_to_csv( q ) )
            .arg( msg )
            .arg( q.size() ? q[ 0 ] : 0 )
            .arg( q.size() ? q[ q.size() - 1 ] : 0 );
         qsl_headers[ 0 ] = qsl[ 0 ];
         cout << qsl[ 0 ] << endl;
         QStringList new_qsl;
         QStringList new_qsl_errors;
         new_qsl << qsl[ 0 ];

         // also crop all the data lines
         QStringList qsl_data = qsl.grep(",\"I(q)\",");
         QStringList qsl_sd   = qsl.grep(",\"I(q) sd\",");
         map < QString, QString > sd_map;
         for ( unsigned int i = 0; i < (unsigned int) qsl_sd.size(); i++ )
         {
            QStringList qsl_s = QStringList::split( ",", qsl_sd[i], true );
            sd_map[ qsl_s[ 0 ] ] = qsl_sd[ i ];
         }

         for ( unsigned int i = 0; i < (unsigned int) qsl_data.size(); i++ )
         {
            QStringList qsl_d = QStringList::split( ",", qsl_data[i], true );
            vector < double > original_i;
            vector < double > original_i_error;
            for ( int j = 2; j < (int) qsl_d.size() - 1; j++ )
            {
               original_i.push_back( qsl_d[j].toDouble() );
            }
            if ( sd_map.count( qsl_d[ 0 ] ) )
            {
               QStringList qsl_s = QStringList::split( ",", sd_map[ qsl_d[ 0 ] ], true );
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

      if ( !usu.interpolate_iqq_by_case( plotted_q[i], plotted_I[i], plotted_I_error[i], q, nic, nic_errors ) )
      {
         editor_msg("red", usu.errormsg );
         QMessageBox::warning( this, "US-SOMO",
                               QString( tr( "There was an error attempting to interpolate\n"
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
         bin_msg = QString(tr("Plotted I(q) interpolated to delta q of %1")).arg(q[1] - q[0]);
         if ( plotted_q[i].size() > 1 && 
              q[1] - q[0] != plotted_q[i][1] - plotted_q[i][0] )
         {
            bin_msg +=
               QString(tr(" which is DIFFERENT from the plotted delta q of %1"))
               .arg(plotted_q[i][1] - plotted_q[i][0]);
         }
         // editor->append(bin_msg + "\n");
         added_interpolate_msg = true;
      }
      QString line = QString("\"%1\",\"I(q)\",%2\n")
         .arg(qsl_plotted_iq_names[i])
         .arg(vector_double_to_csv(nic));
      qsl << line;
      if ( is_nonzero_vector( nic_errors ) )
      {
         QString line = QString("\"%1\",\"I(q) sd\",%2\n")
            .arg(qsl_plotted_iq_names[i])
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
                                           tr("UltraScan"),
                                           tr("There are multiple average and/or standard deviation lines\n") +
                                           tr("What do you want to do?"),
                                           tr("&Skip"), 
                                           tr("&Just averages"),
                                           tr("&Include"),
                                           0,      // Enter == button 0
                                           1 ) ) { // Escape == button 2
         case 0: // skip them
            qsl = new_qsl;
            break;
         case 1: // just averages
            qsl = qsl.grep(QRegExp("(Average|Standard deviation)"));
            break;
         case 2: // Cancel clicked or Escape pressed
            break;
         }
      }
   }
   
   QStringList qsl_data = qsl.grep(",\"I(q)\",");

   if ( qsl_data.size() == 0 )
   {
      QMessageBox mb(tr("UltraScan Warning"),
                     tr("The csv file ") + filename + tr(" does not appear to contain any data rows.\n"),
                     QMessageBox::Critical,
                     Qt::NoButton, Qt::NoButton, Qt::NoButton, 0, 0, 1);
      mb.exec();
      return;
   }
   if ( !just_plotted_curves )
   {
      header_tag = qsl_q.last();
   }
   
   // get possible errors data
   map < QString, QString > name_to_errors_map;
   QStringList qsl_errors = qsl.grep(",\"I(q) sd\",");
   for ( QStringList::iterator it = qsl_errors.begin();
         it != qsl_errors.end();
         it++ )
   {
      QStringList qsl_iq_errors = QStringList::split(",",*it,true);
      name_to_errors_map[ qsl_iq_errors[ 0 ] ] = *it;
   }

   // build a list of names
   QStringList qsl_names;
   for ( QStringList::iterator it = qsl_data.begin();
         it != qsl_data.end();
         it++ )
   {
      QStringList qsl_tmp = QStringList::split(",",*it,true);
      qsl_names << qsl_tmp[0];
   }
   
   // ask for the names to load if more than one present (cb list? )
   QStringList qsl_sel_names;
   bool create_avg = false;
   bool create_std_dev = false;
   bool only_plot_stats = true;
   save_to_csv = false;
   csv_filename = "summary";
   bool save_original_data = false;
   bool run_nnls = false;
   bool run_best_fit = false;
   QString nnls_target = "";
   if ( !grid_target.isEmpty() )
   {
      nnls_target = "\"" + grid_target + "\"";
   }

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
                                        &run_best_fit,
                                        &nnls_target,
                                        1 || U_EXPT,
                                        us_hydrodyn
                                        );
   US_Hydrodyn::fixWinButtons( hslc );
   hslc->exec();
   
   delete hslc;
   
   this->isVisible() ? this->raise() : this->show();
   
   // make sure target is selected
   
   if ( ( run_nnls || run_best_fit ) &&
        !qsl_sel_names.grep(nnls_target).size() )
   {
      // cout << "had to add target back\n";
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
   if ( qsl_sel_names.size() )
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
   
   // setup for nnls
   if ( run_nnls || run_best_fit )
   {
      if ( run_nnls )
      {
         editor->append("NNLS target: " + nnls_target + "\n");
      }
      if ( run_best_fit )
      {
         editor->append("Best fit target: " + nnls_target + "\n");
      }
      nnls_A.clear();
      nnls_x.clear();
      nnls_mw.clear();
      nnls_B.clear();
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
      QStringList qsl_tmp = QStringList::split(",",*it,true);
      if ( map_sel_names.count(qsl_tmp[0]) )
      {
         // cout << "loading: " << qsl_tmp[0] << endl;
         
         I.clear();
         I_errors.clear();
         
         // get the Iq values
         
         QStringList qsl_iq = QStringList::split(",",*it,true);
         if ( qsl_iq.size() < 3 )
         {
            QString msg = tr("The csv file ") + filename + tr(" does not appear to contain sufficient I(q) values in data row " + qsl_tmp[0] + "\n");
            editor_msg( "red", msg );
            QMessageBox mb(tr("UltraScan Warning"),
                           msg,
                           QMessageBox::Critical,
                           Qt::NoButton, Qt::NoButton, Qt::NoButton, 0, 0, 1);
            mb.exec();
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
            QStringList qsl_iq_errors = QStringList::split( ",", name_to_errors_map[ qsl_iq[ 0 ] ], true );
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
            // cout << QString("US_Hydrodyn_Saxs::load_iqq_csv %1 size %2\n").arg(qsl_tmp[0]).arg(I.size());
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
            if ( isnan(I[i]) ) 
            {
               cout << QString("WARNING: isnan I[%1] for %2\n").arg(i).arg(qsl_tmp[0]);
            }
            if ( isnan(sum_iq[i]) ) 
            {
               cout << QString("WARNING: isnan sum_iq[%1] for %2\n").arg(i).arg(qsl_tmp[0]);
            }
            if ( isnan(sum_iq2[i]) ) 
            {
               cout << QString("WARNING: isnan sum_iq2[%1] for %2\n").arg(i).arg(qsl_tmp[0]);
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
                                              QString( tr(
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
            if ( isnan(sum_iq[i]) ) 
            {
               cout << QString("WARNING when calc'ing std dev: isnan sum_iq[%1]\n").arg(i);
            }
            if ( isnan(sum_iq2[i]) ) 
            {
               cout << QString("WARNING when calc'ing std dev: isnan sum_iq2[%1]\n").arg(i);
            }
            if ( isnan(std_dev[i]) ) 
            {
               cout << 
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
         // cout << "save_to_csv\n";
         QString fname = 
            ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "saxs" + SLASH + 
            csv_filename + "_iqq.csv";
         if ( QFile::exists(fname) )
            // && !((US_Hydrodyn *)us_hydrodyn)->overwrite ) 
         {
            fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fname);
         }         
         FILE *of = fopen(fname, "wb");
         if ( of )
         {
            //  header: "name","type",r1,r2,...,rn, header info
            fprintf(of, "\"Name\",\"Type; q:\",%s,%s\n", 
                    vector_double_to_csv(q).ascii(),
                    header_tag.ascii());
            if ( save_original_data )
            {
               fprintf(of, "%s\n", qsl_data_lines_plotted.join("\n").ascii());
            }
            fprintf(of, "\"%s\",\"%s\",%s\n", 
                    "Average",
                    "I(q)",
                    vector_double_to_csv(iq_avg).ascii());
            if ( iq_std_dev.size() )
            {
               fprintf(of, "\"%s\",\"%s\",%s\n", 
                       "Average",
                       "I(q) sd",
                       vector_double_to_csv(iq_std_dev).ascii());
               fprintf(of, "\"%s\",\"%s\",%s\n", 
                       "Standard deviation",
                       "I(q)",
                       vector_double_to_csv(iq_std_dev).ascii());
               fprintf(of, "\"%s\",\"%s\",%s\n", 
                       "Average minus 1 standard deviation",
                       "I(q)",
                       vector_double_to_csv(iq_avg_minus_std_dev).ascii());
               fprintf(of, "\"%s\",\"%s\",%s\n", 
                       "Average plus 1 standard deviation",
                       "I(q)",
                       vector_double_to_csv(iq_avg_plus_std_dev).ascii());
            }
            if ( !save_original_data )
            {
               fprintf(of, "\n\n\"%s\"\n", 
                       QString(" Average of : " + qsl_sel_names.join(";").replace("\"","")).ascii()
                       );
            }
            fclose(of);
            editor->append(tr("Created file: " + fname + "\n"));
         } else {
            editor_msg( "red", tr("ERROR creating file: " + fname + "\n"));
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
                              QString( tr( "Note: the NNLS fit will be performed over a cropped range q(%1:%2)" ) )
                              .arg( our_saxs_options->iqq_scale_minq ? QString( "%1" ).arg( our_saxs_options->iqq_scale_minq ) : "" )
                              .arg( our_saxs_options->iqq_scale_maxq ? QString( "%1" ).arg( our_saxs_options->iqq_scale_maxq ) : "" ) );
               }

               calc_iqq_nnls_fit( nnls_target, use_csv_filename );
            }
            if ( run_best_fit )
            {
               if ( our_saxs_options->iqq_scale_minq ||
                    our_saxs_options->iqq_scale_maxq )
               {
                  editor_msg( "dark red", 
                              QString( tr( "Note: the fit will be performed over a cropped range q(%1:%2)" ) )
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


void US_Hydrodyn_Saxs::load_saxs( QString filename, bool just_plotted_curves )
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
      filename = Q3FileDialog::getOpenFileName(use_dir, 
                                              "All files (*);;"
                                              "ssaxs files (*.ssaxs);;"
                                              "csv files (*.csv);;"
                                              "int files [crysol] (*.int);;"
                                              "dat files [foxs / other] (*.dat);;"
                                              "fit files [crysol] (*.fit);;"
                                              , this
                                              , "open file dialog"
                                              , "Open"
                                              , &load_saxs_sans_selected_filter
                                              );
      if ( filename.isEmpty() )
      {
         return;
      }
      add_to_directory_history( filename );
   }

   plotted = false;
   QFile f(filename);
   our_saxs_options->path_load_saxs_curve = QFileInfo(filename).dirPath(true);
   QString ext = QFileInfo(filename).extension(FALSE).lower();

   if ( ext == "pdb" || ext == "PDB" )
   {
      QMessageBox::warning( this, "UltraScan",
                            QString(tr("Can not load a PDB file as a curve: ")
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

   bool do_crop = false;

   if ( f.open(QIODevice::ReadOnly) )
   {
      Q3TextStream ts(&f);
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
                               QString(tr("The file ")
                                       + filename + tr(" is empty.")) );
         return;
      }

      unsigned int number_of_fields = 0;
      if ( qv.size() > 3 )
      {
         QString test_line = qv[2];
         test_line.replace(QRegExp("^\\s+"),"");
         test_line.replace(QRegExp("\\s+$"),"");
         QStringList test_list = QStringList::split(QRegExp("\\s+"), test_line);
         number_of_fields = test_list.size();
         cout << "number of fields: " << number_of_fields << endl;
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
            res = QInputDialog::getItem(
                                        "Crysol's .int format has four available datasets", 
                                        "Select the set you wish to plot::", lst, 0, FALSE, &ok,
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
               cout << " column " << Icolumn << endl;
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
         do_crop = true;

         Icolumn = 1;
         I_errorcolumn = 2;
         if ( qsl.grep("exp_intensity").size() )
         {
            I_errorcolumn = 0;
            
            switch ( QMessageBox::question(this, 
                                           tr("UltraScan Notice"),
                                           QString(tr("Please note:\n\n"
                                                      "The file appears to have both experiment and model data\n"
                                                      "What would you like to do?\n"))
                                           ,
                                           tr("&Load only experimental"),
                                           tr("&Load only the model"),
                                           tr("&Load both"),
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
         do_crop = true;

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
            res = QInputDialog::getItem(
                                        "There are three available datasets", 
                                        "Select the set you wish to plot::", lst, 0, FALSE, &ok,
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
               cout << " column " << Icolumn << endl;
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
                                           tr("UltraScan"),
                                           tr("Is this file in 1/Angstrom or 1/nm units?"),
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

      QRegExp rx_ok_line("^(\\s+|\\d+|\\.|\\d(E|e)(\\+|-|\\d))+$");
      rx_ok_line.setMinimal( true );
      for ( unsigned int i = 1; i < (unsigned int) qv.size(); i++ )
      {
         if ( qv[i].contains(QRegExp("^#")) ||
              rx_ok_line.search( qv[i] ) == -1 )
         {
            cout << "not ok: " << qv[i] << endl; 
            continue;
         }
         
         QStringList tokens = QStringList::split(QRegExp("\\s+"), qv[i].replace(QRegExp("^\\s+"),""));
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

      cout << "q_range after load: " << q[0] << " , " << q[q.size() - 1] << endl;

      cout << QFileInfo(filename).fileName() << endl;
      if ( do_crop )
      {
         if ( Icolumn2 )
         {
            q2 = q;
            crop_iq_data(q2, I2);
         }
         if ( I_error.size() )
         {
            crop_iq_data(q, I, I_error );
         } else {
            crop_iq_data(q, I);
         }
      }
         
      cout << "q_range after crop: " << q[0] << " , " << q[q.size() - 1] << endl;

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
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header =
            QString("")
            .sprintf(
                     "Saxs curves from %s"
                     , filename.ascii()
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

void US_Hydrodyn_Saxs::load_pr( bool just_plotted_curves )
{
   if ( just_plotted_curves &&
        !qsl_plotted_pr_names.size() )
   {
         QMessageBox::warning( this, "UltraScan",
                               QString(tr("There is nothing plotted!")) );
         return;
   }      

   QString use_dir = 
      our_saxs_options->path_load_prr.isEmpty() ?
      USglobal->config_list.root_dir + SLASH + "somo" + SLASH + "saxs" :
      our_saxs_options->path_load_prr;

   select_from_directory_history( use_dir, this );

   QStringList filenames;
   QString filename;
   if ( !just_plotted_curves )
   {
      filenames = Q3FileDialog::getOpenFileNames(
                                                "All files (*);;"
                                                "sprr files (*.sprr_?);;"
                                                "csv files (*.csv)"
                                                , use_dir
                                                , this
                                                , "open file dialog"
                                                , "Open"
                                                , &load_pr_selected_filter
                                                );
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
        filenames.grep(QRegExp(".csv$", false)).size() != filenames.size() )
   {
      QMessageBox::information( this, "UltraScan",
                                tr("Multiple file load is currently only supported for csv files") );
      return;
   }

   if ( filenames.size() > 1 &&
        filenames.grep(QRegExp("_t(|-\\d+).csv$", false)).size() ) 
   {
      QMessageBox::information( this, "UltraScan",
                                tr("Can not load transposed format csv files:\n") +
                                filenames.grep(QRegExp("_t(|-\\d+).csv$", false)).join("\n")
                                ) ;
      return;
   }

   if ( !just_plotted_curves )
   {
      filename = filenames[0];
      
      if ( filename.contains(QRegExp("_t(|-\\d+).csv$", false)) )
      {
         QMessageBox::information( this, "UltraScan",
                                   tr("Can not load transposed format csv files") );
         return;
      }
   }

   QFile f(filename);
   if ( !just_plotted_curves )
   {
      our_saxs_options->path_load_prr = QFileInfo(filename).dirPath(true);
   }
   QString ext = QFileInfo(filename).extension(FALSE).lower();
   vector < double > r;
   vector < double > pr;
   double new_r, new_pr;
   QString res = "";
   unsigned int startline = 1;
   unsigned int pop_last = 0;
   if ( just_plotted_curves || f.open(QIODevice::ReadOnly) )
   {
      if ( file_curve_type(f.name()) != -1 &&
           file_curve_type(f.name()) != our_saxs_options->curve )
      {
         switch ( QMessageBox::question(this, 
                                        tr("UltraScan Notice"),
                                        QString(tr("Please note:\n\n"
                                                   "The file appears to be in %1 mode and you"
                                                   " are currently set in %2 mode.\n"
                                                   "What would you like to do?\n"))
                                        .arg(curve_type_string(file_curve_type(f.name())))
                                        .arg(curve_type_string(our_saxs_options->curve))
                                        ,
                                        tr("&Change mode now and load"), 
                                        tr("&Load anyway without changing the mode"),
                                        tr("&Stop loading"),
                                        0, // Stop == button 0
                                        0 // Escape == button 0
                                        ) )
               {
               case 0 : 
                  our_saxs_options->curve = file_curve_type(f.name());
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
            Q3TextStream ts(&f);
            while ( !ts.atEnd() )
            {
               qsl << ts.readLine();
            }
            f.close();
         }

         QStringList qsl_headers = qsl.grep("\"Name\",\"Type; r:\"");
         if ( qsl_headers.size() != 0 ) 
         {
            cout << "found old csv format, upgrading\n";
            // upgrade the csv format
            QRegExp rx("^\"(Type; r:|P\\(r\\)|P\\(r\\) normed)\"$");
            QStringList new_qsl;
            double delta_r = 0e0;
            for ( unsigned int i = 0; i < (unsigned int)qsl.size(); i++ )
            {
               QStringList tmp2_qsl;
               QStringList tmp_qsl = QStringList::split(",",qsl[i],true);
               // if ( tmp_qsl.size() > 1 )
               // {
               //      cout << QString("line %1 field 1 is <%2>\n").arg(i).arg(tmp_qsl[1]);
               // } else {
               // cout << QString("line %1 size not greater than 1 value <%2>\n").arg(i).arg(qsl[i]);
               // }
               if ( tmp_qsl.size() > 1 &&
                    rx.search(tmp_qsl[1]) != -1 )
               {
                  // cout << "trying to fix\n";
                  QStringList tmp2_qsl;
                  tmp2_qsl.push_back(tmp_qsl[0]);
                  if ( tmp_qsl[0] == "\"Name\"" )
                  {
                     tmp2_qsl.push_back("\"MW (Daltons)\"");
                     tmp2_qsl.push_back("\"Area\"");
                     delta_r = tmp_qsl[3].toDouble() - tmp_qsl[2].toDouble();
                     cout << "delta_r found: " << delta_r << endl;
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
                  // cout << "skipped this line, regexp or length\n";
                  // simply push back blank lines or lines with only one entry
                  new_qsl.push_back(qsl[i]);
               }
            }
            // cout << "orginal csv:\n" << qsl.join("\n") << endl;
            // cout << "new csv:\n" << new_qsl.join("\n") << endl;
            qsl = new_qsl;
         }

         qsl_headers = qsl.grep("\"Name\",\"MW (Daltons)\",\"Area\",\"Type; r:\"");
         if ( qsl_headers.size() == 0 && !just_plotted_curves ) 
         {
            QMessageBox mb(tr("UltraScan Warning"),
                           tr("The csv file ") + filename + tr(" does not appear to contain a correct header.\n"
                                                               "Please manually correct the csv file."),
                           QMessageBox::Critical,
                           Qt::NoButton, Qt::NoButton, Qt::NoButton, 0, 0, 1);
            mb.exec();
            return;
         }

         if ( qsl_headers.size() > 1 ) 
         {
            QString ref = qsl_headers[0];
            for ( unsigned int i = 1; i < (unsigned int)qsl_headers.size(); i++ )
            {
               if ( ref != qsl_headers[i] )
               {
                  QMessageBox mb(tr("UltraScan Warning"),
                                 tr("The csv file ") + filename + tr(" contains multiple different headers\n"
                                                                     "Please manually correct the csv file."),
                                 QMessageBox::Critical,
                                 Qt::NoButton, Qt::NoButton, Qt::NoButton, 0, 0, 1);
                  mb.exec();
                  return;
               }
            }
         }

         // get the r values
         QStringList qsl_r;
         QString header_tag;

         if ( !just_plotted_curves )
         {
            qsl_r = QStringList::split(",",qsl_headers[0],true);
            if ( qsl_r.size() < 6 )
            {
               QMessageBox mb(tr("UltraScan Warning"),
                              tr("The csv file ") + filename + tr(" does not appear to contain any r values in the header rows.\n"),
                              QMessageBox::Critical,
                           Qt::NoButton, Qt::NoButton, Qt::NoButton, 0, 0, 1);
               mb.exec();
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

         if ( filenames.size() > 1 )
         {
            map < QString, bool > interp_msg_done;
            for ( unsigned int i = 1; i < (unsigned int)filenames.size(); i++ )
            {
               cout << QString("trying file %1 %2\n").arg(i).arg(filenames[i]);
               QFile f2(filenames[i]);
               if ( !f2.open(QIODevice::ReadOnly) )
               {
                  QMessageBox::information( this, "UltraScan",
                                            tr("Can not open the file for reading:\n") +
                                            f2.name()
                                            );
                  return;
               }

               // append (and possibly interpolate) all the other files
               QStringList qsl2;
               Q3TextStream ts(&f2);
               while ( !ts.atEnd() )
               {
                  qsl2 << ts.readLine();
               }
               f2.close();
               QStringList qsl2_headers = qsl2.grep("\"Name\",\"Type; r:\"");
               
               // upgrade old format
               if ( qsl2_headers.size() != 0 ) 
               {
                  cout << "found old csv format, upgrading\n";
                  // upgrade the csv format
                  QRegExp rx("^\"(Type; r:|P\\(r\\)|P\\(r\\) normed)\"$");
                  QStringList new_qsl;
                  double delta_r = 0e0;
                  for ( unsigned int i = 0; i < (unsigned int)qsl2.size(); i++ )
                  {
                     QStringList tmp2_qsl;
                     QStringList tmp_qsl = QStringList::split(",",qsl2[i],true);
                     if ( tmp_qsl.size() > 1 &&
                          rx.search(tmp_qsl[1]) != -1 )
                     {
                        // cout << "trying to fix\n";
                        QStringList tmp2_qsl;
                        tmp2_qsl.push_back(tmp_qsl[0]);
                        if ( tmp_qsl[0] == "\"Name\"" )
                        {
                           tmp2_qsl.push_back("\"MW (Daltons)\"");
                           tmp2_qsl.push_back("\"Area\"");
                           delta_r = tmp_qsl[3].toDouble() - tmp_qsl[2].toDouble();
                           cout << "delta_r found: " << delta_r << endl;
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
                        // cout << "skipped this line, regexp or length\n";
                        // simply push back blank lines or lines with only one entry
                        new_qsl.push_back(qsl[i]);
                     }
                  }
                  // cout << "orginal csv:\n" << qsl.join("\n") << endl;
                  // cout << "new csv:\n" << new_qsl.join("\n") << endl;
                  qsl2 = new_qsl;
               }
               qsl2_headers = qsl2.grep("\"Name\",\"MW (Daltons)\",\"Area\",\"Type; r:\"");
               if ( qsl2_headers.size() == 0 )
               {
                  QMessageBox mb(tr("UltraScan Warning"),
                                 tr("The csv file ") + f2.name() + tr(" does not appear to contain a correct header.\n"
                                                                      "Please manually correct the csv file."),
                                 QMessageBox::Critical,
                                 Qt::NoButton, Qt::NoButton, Qt::NoButton, 0, 0, 1);
                  mb.exec();
                  return;
               }
               if ( qsl2_headers.size() > 1 ) 
               {
                  QString ref = qsl2_headers[0];
                  for ( unsigned int i = 1; i < (unsigned int)qsl2_headers.size(); i++ )
                  {
                     if ( ref != qsl2_headers[i] )
                     {
                        QMessageBox mb(tr("UltraScan Warning"),
                                       tr("The csv file ") + filename + tr(" contains multiple different headers\n"
                                                                           "Please manually correct the csv file."),
                                       QMessageBox::Critical,
                                       Qt::NoButton, Qt::NoButton, Qt::NoButton, 0, 0, 1);
                        mb.exec();
                        return;
                     }
                  }
               }
               // get the data
               QStringList qsl_data = qsl2.grep(",\"P(r)\",");
               if ( qsl_data.size() == 0 )
               {
                  QMessageBox mb(tr("UltraScan Warning"),
                                 tr("The csv file ") + filename + tr(" does not appear to contain any data rows.\n"),
                                 QMessageBox::Critical,
                                 Qt::NoButton, Qt::NoButton, Qt::NoButton, 0, 0, 1);
                  mb.exec();
                  return;
               }

               // get the r values
               QStringList qsl_r;
               QString header2_tag;
               vector < double > r2;

               qsl_r = QStringList::split(",",qsl2_headers[0],true);
               if ( qsl_r.size() < 6 )
               {
                  QMessageBox mb(tr("UltraScan Warning"),
                                 tr("The csv file ") + f2.name() + tr(" does not appear to contain any r values in the header rows.\n"),
                                 QMessageBox::Critical,
                                 Qt::NoButton, Qt::NoButton, Qt::NoButton, 0, 0, 1);
                  mb.exec();
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
               // cout << QString("max size %1\n").arg(max_size);
               bool all_match = true;
               for ( unsigned int i = 0; i < max_size; i++ )
               {
                  if ( r[i] != r2[i] )
                  {
                     all_match = false;
                     break;
                  }
               }
               // cout << QString("check all match %1\n").arg(all_match ? "true" : false);

               for ( QStringList::iterator it = qsl_data.begin();
                     it != qsl_data.end();
                     it++ )
               {
                  QStringList qsl_pr = QStringList::split(",",*it,true);
                  if ( qsl_pr.size() < 6 )
                  {
                     QString msg = tr("The csv file ") + f2.name() + tr(" does not appear to contain sufficient p(r) values in data row " + qsl_pr[0] + ", skipping\n");
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
                        // cout << "yes, all match\nadding:" << qs_tmp << endl;
                     } else {
                        // cout << "not all match, interpolate\n";
                        if ( !interp_msg_done.count(f2.name()) )
                        {
                           interp_msg_done[f2.name()] = true;
                           QString msg = tr("The csv file ") + f2.name() + tr(" will be interpolated\n");
                           editor_msg( "dark red", msg );
                        }
                        // the new pr:
                        QStringList new_pr_fields;
                        // pull the data values
                        vector < double > this_pr;
                        for ( unsigned int i = 0; i < 5; i++ )
                        {
                           new_pr_fields.push_back(qsl_pr[i]);
                        }
                        {
                           QStringList::iterator it2 = qsl_pr.begin();
                           it2 += 5;
                           for ( ; it2 != qsl_pr.end(); it2++ )
                           {
                              this_pr.push_back((*it2).toDouble());
                           }
                        }
                        // interpolate r2, pr to r, reappend to new_pr_fields
                        vector < double > npr = interpolate(r, r2, this_pr);
                        QString line = QString("%1,%2\n")
                           .arg(new_pr_fields.join(","))
                           .arg(vector_double_to_csv(npr));
                        // cout << QString("r:\n%1\n").arg(vector_double_to_csv(r));
                        // cout << QString("r2:\n%1\n").arg(vector_double_to_csv(r2));
                        // cout << QString("org line:\n%1\n").arg(*it);
                        // cout << QString("new interpolated line:\n%1\n").arg(line);
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

         // append all currently plotted p(r)s to qsl
         bool added_interpolate_msg = false;
         QString bin_msg = "";
         for ( unsigned int i = 0; i < (unsigned int)qsl_plotted_pr_names.size(); i++ )
         {
            vector < double > npr = interpolate(r, plotted_r[i], plotted_pr_not_normalized[i]);
            if ( !added_interpolate_msg && r.size() > 1 )
            {
               bin_msg = QString(tr("Plotted P(r) interpolated to bin size of %1")).arg(r[1] - r[0]);
               if ( plotted_r[i].size() > 1 && 
                    r[1] - r[0] != plotted_r[i][1] - plotted_r[i][0] )
               {
                  bin_msg +=
                     QString(tr(" which is DIFFERENT from the plotted bin size of %1"))
                     .arg(plotted_r[i][1] - plotted_r[i][0]);
               }
               // editor->append(bin_msg + "\n");
               added_interpolate_msg = true;
            }
            QString line = QString("\"%1\",%2,%3,\"P(r)\",%4\n")
               .arg(qsl_plotted_pr_names[i])
               .arg(plotted_pr_mw[i])
               .arg(compute_pr_area(npr, r))
               .arg(vector_double_to_csv(npr));
            qsl << line;
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
                                                 tr("UltraScan"),
                                                 tr("There are multiple average and/or standard deviation lines\n") +
                                                 tr("What do you want to do?"),
                                                 tr("&Skip"), 
                                                 tr("&Just averages"),
                                                 tr("&Include"),
                                                 0,      // Enter == button 0
                                                 1 ) ) { // Escape == button 2
               case 0: // skip them
                  qsl = new_qsl;
                  break;
               case 1: // just averages
                  qsl = qsl.grep(QRegExp("(Average|Standard deviation)"));
                  break;
               case 2: // Cancel clicked or Escape pressed
                  break;
               }
            }
         }

         QStringList qsl_data = qsl.grep(",\"P(r)\",");
         if ( qsl_data.size() == 0 )
         {
            QMessageBox mb(tr("UltraScan Warning"),
                           tr("The csv file ") + filename + tr(" does not appear to contain any data rows.\n"),
                           QMessageBox::Critical,
                           Qt::NoButton, Qt::NoButton, Qt::NoButton, 0, 0, 1);
            mb.exec();
            return;
         }
         if ( !just_plotted_curves )
         {
            header_tag = qsl_r.last();
         }

#if defined(DEBUG_PR)
         cout << "r values (" << r.size() << "): ";
         
         for ( unsigned int i = 0; i < r.size(); i++ )
         {
            cout << r[i] << ",";
         }
         cout << endl;
#endif
      
         // build a list of names
         QStringList qsl_names;
         for ( QStringList::iterator it = qsl_data.begin();
               it != qsl_data.end();
               it++ )
         {
            QStringList qsl_tmp = QStringList::split(",",*it,true);
            qsl_names << qsl_tmp[0];
         }

         // ask for the names to load if more than one present (cb list? )
         QStringList qsl_sel_names;
         bool create_avg = false;
         bool create_std_dev = false;
         bool only_plot_stats = true;
         save_to_csv = false;
         csv_filename = "summary";
         bool save_original_data = false;
         bool run_nnls = false;
         bool run_best_fit = false;
         QString nnls_target = "";
         
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
                                          &run_best_fit,
                                          &nnls_target,
                                          1 || U_EXPT,
                                          us_hydrodyn
                                          );
         US_Hydrodyn::fixWinButtons( hslc );
         hslc->exec();
            
         delete hslc;
         
         this->isVisible() ? this->raise() : this->show();

         // make sure target is selected

         if ( ( run_nnls || run_best_fit ) &&
              !qsl_sel_names.grep(nnls_target).size() )
         {
            // cout << "had to add target back\n";
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

         // setup for nnls
         if ( run_nnls || run_best_fit )
         {
            if ( run_nnls )
            {
               editor->append("NNLS target: " + nnls_target + "\n");
            }
            if ( run_best_fit )
            {
               editor->append("Best fit target: " + nnls_target + "\n");
            }
            nnls_A.clear();
            nnls_x.clear();
            nnls_mw.clear();
            nnls_B.clear();
            nnls_B_name = nnls_target;
            nnls_rmsd = 0e0;
         }

         bool found_nnls_target = false;
         bool found_nnls_model = false;

         float pr_mw_avg = 0.0;
         vector < float > pr_mws;

         // now go through qsl_data and load up any that map_sel_names contains
         plotted = false;
         for ( QStringList::iterator it = qsl_data.begin();
               it != qsl_data.end();
               it++ )
         {
            QStringList qsl_tmp = QStringList::split(",",*it,true);
            if ( map_sel_names.count(qsl_tmp[0]) )
            {
               // cout << "loading: " << qsl_tmp[0] << endl;

               pr.clear();

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

               QStringList qsl_pr = QStringList::split(",",*it,true);
               if ( qsl_pr.size() < 6 )
               {
                  QString msg = tr("The csv file ") + filename + tr(" does not appear to contain sufficient p(r) values in data row " + qsl_tmp[0] + "\n");
                  editor_msg( "red", msg );
                  QMessageBox mb(tr("UltraScan Warning"),
                                 msg,
                                 QMessageBox::Critical,
                                 Qt::NoButton, Qt::NoButton, Qt::NoButton, 0, 0, 1);
                  mb.exec();
                  break;
               }
               qsl_data_lines_plotted << *it;
               pr.push_back(qsl_pr[4].toDouble());
            
               {
                  QStringList::iterator it = qsl_pr.begin();
                  it += 5;
                  for ( ; it != qsl_pr.end(); it++ )
                  {
                     pr.push_back((*it).toDouble());
                  }
               }

               if ( run_nnls || run_best_fit )
               {
                  if ( qsl_tmp[0] == nnls_target )
                  {
                     found_nnls_target = true;
                     nnls_B = pr;
                  } else {
                     found_nnls_model = true;
                     nnls_A[qsl_tmp[0]] = pr;
                     nnls_x[qsl_tmp[0]] = 0;
                     nnls_mw[qsl_tmp[0]] = qsl_tmp[1].toDouble();
                  }
               }
#if defined(DEBUG_PR)
               cout << "pr values (" << pr.size() << "): ";
               
               for ( unsigned int i = 0; i < pr.size(); i++ )
               {
                  cout << pr[i] << ",";
               }
               cout << endl;
#endif
               // plot it
               vector < double > this_r = r;
               // r has the ordinates for the longest data, some will likely be shorter
               if ( r.size() > pr.size() )
               {
                  this_r.resize(pr.size());
               }
               // occasionally one may have a zero in the last p(r) position
               if ( pr.size() > r.size() )
               {
                  pr.resize(r.size());
               }

               for ( unsigned int i = 0; i < pr.size(); i++ )
               {
                  sum_pr[i] += pr[i];
                  sum_pr2[i] += pr[i] * pr[i];
                  if ( isnan(pr[i]) ) 
                  {
                     cout << QString("WARNING: isnan pr[%1] for %2\n").arg(i).arg(qsl_tmp[0]);
                  }
                  if ( isnan(sum_pr[i]) ) 
                  {
                     cout << QString("WARNING: isnan sum_pr[%1] for %2\n").arg(i).arg(qsl_tmp[0]);
                  }
                  if ( isnan(sum_pr2[i]) ) 
                  {
                     cout << QString("WARNING: isnan sum_pr2[%1] for %2\n").arg(i).arg(qsl_tmp[0]);
                  }
               }
               sum_count++;

               if ( !(create_avg && only_plot_stats) && !run_nnls && !run_best_fit )
               {
                  (*remember_mw)[QFileInfo(filename).fileName() + " " + qsl_tmp[0]] = tmp_mw;
                  (*remember_mw_source)[QFileInfo(filename).fileName() + " " + qsl_tmp[0]] = "loaded from csv";
                  plot_one_pr(this_r, pr, QFileInfo(filename).fileName() + " " + qsl_tmp[0]);
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

            plot_one_pr(this_r, pr, QFileInfo(filename).fileName() + " Average");

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
                  if ( isnan(sum_pr[i]) ) 
                  {
                     cout << QString("WARNING when calc'ing std dev: isnan sum_pr[%1]\n").arg(i);
                  }
                  if ( isnan(sum_pr2[i]) ) 
                  {
                     cout << QString("WARNING when calc'ing std dev: isnan sum_pr2[%1]\n").arg(i);
                  }
                  if ( isnan(std_dev[i]) ) 
                  {
                     cout << 
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

               // cout << QString("sd mw calcs:\n"
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
               cout << "sum pr (" << sum_pr.size() << "): ";
               
               for ( unsigned int i = 0; i < sum_pr.size(); i++ )
               {
                  cout << sum_pr[i] << ",";
               }
               cout << endl;
               
               cout << "sum pr2 (" << sum_pr2.size() << "): ";
               
               for ( unsigned int i = 0; i < sum_pr2.size(); i++ )
               {
                  cout << sum_pr2[i] << ",";
               }
               cout << endl;
               
               cout << "std values (" << std_dev.size() << "): ";
               
               for ( unsigned int i = 0; i < std_dev.size(); i++ )
               {
                  cout << std_dev[i] << ",";
               }
               cout << endl;
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

               plot_one_pr(this_r, pr, QFileInfo(filename).fileName() + " Average minus 1 std dev");
               
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

               plot_one_pr(this_r, pr, QFileInfo(filename).fileName() + " Average plus 1 std dev");
            }
            if ( plotted )
            {
               editor_msg( "black", "P(r) plot done\n");
               plotted = false;
            }
            if ( save_to_csv )
            {
               // cout << "save_to_csv\n";
               QString fname = 
                  ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "saxs" + SLASH + 
                  csv_filename + "_sprr_" + ((US_Hydrodyn *)us_hydrodyn)->saxs_sans_ext() + ".csv";
               if ( QFile::exists(fname) )
                  // && !((US_Hydrodyn *)us_hydrodyn)->overwrite ) 
               {
                  fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fname);
               }         
               FILE *of = fopen(fname, "wb");
               if ( of )
               {
                  //  header: "name","type",r1,r2,...,rn, header info
                  fprintf(of, "\"Name\",\"MW (Daltons)\",\"Area\",\"Type; r:\",%s,%s\n", 
                          vector_double_to_csv(r).ascii(),
                          header_tag.ascii());
                  if ( save_original_data )
                  {
                     fprintf(of, "%s\n", qsl_data_lines_plotted.join("\n").ascii());
                  }
                  fprintf(of, "\"%s\",%.2f,%.2f,\"%s\",%s\n", 
                          "Average",
                          pr_mw_avg,
                          compute_pr_area(pr_avg, r),
                          "P(r)",
                          vector_double_to_csv(pr_avg).ascii());
                  if ( pr_std_dev.size() )
                  {
                     fprintf(of, "\"%s\",%.2f,%.2f,\"%s\",%s\n", 
                             "Standard deviation",
                             pr_mw_std_dev,
                             compute_pr_area(pr_std_dev, r),
                             "P(r)",
                             vector_double_to_csv(pr_std_dev).ascii());
                     fprintf(of, "\"%s\",%.2f,%.2f,\"%s\",%s\n", 
                             "Average minus 1 standard deviation",
                             pr_mw_avg - pr_mw_std_dev,
                             compute_pr_area(pr_avg_minus_std_dev, r),
                             "P(r)",
                             vector_double_to_csv(pr_avg_minus_std_dev).ascii());
                     fprintf(of, "\"%s\",%.2f,%.2f,\"%s\",%s\n", 
                             "Average plus 1 standard deviation",
                             pr_mw_avg + pr_mw_std_dev,
                             compute_pr_area(pr_avg_plus_std_dev, r),
                             "P(r)",
                             vector_double_to_csv(pr_avg_plus_std_dev).ascii());
                  }
                  if ( !save_original_data )
                  {
                     fprintf(of, "\n\n\"%s\"\n", 
                             QString(" Average of : " + qsl_sel_names.join(";").replace("\"","")).ascii()
                             );
                  }
                  fclose(of);
                  editor->append(tr("Created file: " + fname + "\n"));
               } else {
                  editor_msg( "red", tr("ERROR creating file: " + fname + "\n" ) );
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
                     calc_nnls_fit( nnls_target, use_csv_filename );
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

      if ( !ext.contains(QRegExp("^sprr(|_(x|n|r))")) )
      {
         // check for gnom output
         Q3TextStream ts(&f);
         QString tmp;
         unsigned int pos = 0;
         while ( !ts.atEnd() )
         {
            tmp = ts.readLine();
            pos++;
            if ( tmp.contains("Distance distribution  function of particle") ) 
            {
               editor->append("\nRecognized GNOM output.\n");
               startline = pos + 4;
               pop_last = 2;
               break;
            }
         }
         f.close();
         f.open(QIODevice::ReadOnly);
      }

      Q3TextStream ts(&f);
      //      editor->append(QString("\nLoading pr(r) data from %1 %2\n").arg(filename).arg(res));
      QString firstLine = ts.readLine();
      QRegExp sprr_mw_line("mw\\s+(\\S+)\\s+Daltons");
      float mw = 0.0;
      if ( sprr_mw_line.search(firstLine) != -1 )
      {
         mw = sprr_mw_line.cap(1).toFloat();
         (*remember_mw)[QFileInfo(filename).fileName()] = mw;
         (*remember_mw_source)[QFileInfo(filename).fileName()] = "loaded from sprr file";
      }
      editor->append(firstLine);
      while ( startline > 0 )
      {
         ts.readLine();
         startline--;
      }
         
      while ( !ts.atEnd() )
      {
         ts >> new_r;
         ts >> new_pr;
         ts.readLine();
         r.push_back(new_r);
         pr.push_back(new_pr);
      }
      f.close();
      while ( pop_last > 0 && r.size() )
      {
         r.pop_back();
         pr.pop_back();
         pop_last--;
      }
      QString use_filename = QFileInfo(filename).fileName() + " P(r)";
      if ( mw )
      {
         (*remember_mw)[use_filename] = mw;
         (*remember_mw_source)[use_filename] = "loaded from sprr file";
      }         
      check_pr_grid( r, pr );
      plot_one_pr(r, pr, use_filename);
   }
}

void US_Hydrodyn_Saxs::rescale_iqq_curve( QString scaling_target,
                                          vector < double > &q,
                                          vector < double > &I )
{
   vector < double > I2;
   QColor plot_color = plot_colors[ plotted_q.size() % plot_colors.size() ];
   rescale_iqq_curve( scaling_target, q, I, I2, plot_color );
}

void US_Hydrodyn_Saxs::rescale_iqq_curve( QString scaling_target,
                                          vector < double > &q,
                                          vector < double > &I,
                                          QColor plot_color )
{
   vector < double > I2;
   rescale_iqq_curve( scaling_target, q, I, I2, plot_color );
}

void US_Hydrodyn_Saxs::rescale_iqq_curve( QString scaling_target,
                                          vector < double > &q,
                                          vector < double > &I,
                                          vector < double > &I2 )
{
   QColor plot_color = plot_colors[ plotted_q.size() % plot_colors.size() ];
   rescale_iqq_curve( scaling_target, q, I, I2, plot_color );
}

void US_Hydrodyn_Saxs::rescale_iqq_curve( QString scaling_target,
                                          vector < double > &q,
                                          vector < double > &I,
                                          vector < double > &/* I2 */,
                                          QColor plot_color
                                          )
{
   if ( !q.size() ||
        scaling_target.isEmpty() ||
        !plotted_iq_names_to_pos.count(scaling_target) )
   {
      return;
   }

   unsigned int iq_pos = plotted_iq_names_to_pos[scaling_target];
   cout << "scaling target pos is " << iq_pos << endl;
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
   
   cout << QString(
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
         if ( plotted_I[ iq_pos ][ i ] )
         {
            avg_std_dev_frac += plotted_I_error[ iq_pos ][ i ] / plotted_I[ iq_pos ][ i ];
            avg_std_dev_point_count++;
         }
      }
   }
   
   if ( avg_std_dev_point_count )
   {
      avg_std_dev_frac /= ( double ) avg_std_dev_point_count;
   }

   cout << QString("After cropping q to overlap region:\n"
                   "use_q.size == %1\n").arg(use_q.size());
   
   if ( !use_q.size() )
   {
      QMessageBox::warning( this, "UltraScan",
                            QString(tr("Could not find sufficient q range overlap\n"
                                       "to scale the loaded data to the selected target")) );
      return;
   }

   vector < double > use_source_I = interpolate(use_q, q, I);
   
   US_Saxs_Util usu;

   double k;
   double chi2;

   bool do_chi2_fitting        = our_saxs_options->iqq_scale_chi2_fitting;
   bool do_scale_linear_offset = our_saxs_options->iqq_scale_linear_offset;
   bool do_kratky              = our_saxs_options->iqq_kratky_fit;

   if ( our_saxs_options->ignore_errors &&
        !is_zero_vector( use_I_error ) )
   {
      editor_msg( "dark red", tr( "Ignoring experimental errors" ) );
      do_chi2_fitting = false;
   }

   if ( is_zero_vector( use_I_error ) )
   {
      editor_msg( "red", tr("Chi^2 fitting requested, but target data has no standard deviation data\n"
                            "Chi^2 fitting not used\n") );
      do_chi2_fitting = false;
   }

   if ( do_kratky )
   {
      editor_msg( "blue", tr( "Kratky fit (q^2*I)\n" ) );
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

   for ( unsigned int i = 0; i < use_source_I.size(); i++ )
   {
      cout << QString( "q %1 use_source_I %2 use_I %3\n" )
         .arg( q[ i ] )
         .arg( use_source_I[ i ] )
         .arg( use_I[ i ] );
   }

   if ( our_saxs_options->iqq_scale_nnls )
   {
      if ( our_saxs_options->iqq_scale_chi2_fitting )
      {
         editor_msg( "red", tr("Chi^2 fitting is currently not compatable with NNLS scaling\n") );
         do_chi2_fitting = false;
      }
      if ( our_saxs_options->iqq_scale_linear_offset )
      {
         editor_msg( "red", tr("Scale with linear offset is not compatable with NNLS scaling\n") );
         do_scale_linear_offset = false;
      }

      usu.nnls_fit( 
                   use_source_I, 
                   use_I, 
                   k, 
                   chi2
                   );
   } else {
      editor_msg( "dark blue", do_chi2_fitting ? "Chi^2 fitting\n" : "" );
      if ( our_saxs_options->iqq_scale_linear_offset )
      {
         editor_msg( "red", tr("Scale with linear offset is not currently implemented\n") );
         do_scale_linear_offset = false;
      }
      if ( do_chi2_fitting && !is_nonzero_vector( use_I_error ) )
      {
         editor_msg( "red", tr("Chi^2 fitting disabled, zeros in target standard deviation\n") );
         do_chi2_fitting = false;
      }
      editor_msg( "dark blue",
                  QString( tr( "fitting range: %1 to %2 with %3 points\n" ) )
                  .arg( use_q[ 0 ] )
                  .arg( use_q.back() )
                  .arg( use_q.size() ) );

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
         double res = QInputDialog::getDouble(
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

   if ( do_chi2_fitting )
   {
      // usu.calc_chisq_prob( 0.5 * use_I.size() - ( do_scale_linear_offset ? 2 : 1 ),
      // 0.5 * chi2,
      // chi2_prob );
      fit_msg = 
         QString("chi^2=%1 df=%2 nchi=%3")
         .arg(chi2, 6)
         .arg(use_I.size() - ( do_scale_linear_offset ? 2 : 1 ) )
         .arg(sqrt( chi2 / ( use_I.size() - ( do_scale_linear_offset ? 2 : 1 ) ) ), 5 )
         ;
      if ( avg_std_dev_frac )
      {
         fit_msg += QString( " sdf=%1 nchi*sdf=%2 " )
            .arg( avg_std_dev_frac ) 
            .arg( avg_std_dev_frac * sqrt( chi2 / ( use_I.size() - ( do_scale_linear_offset ? 2 : 1 ) ) ), 5 );
      }
   } else {
      fit_msg = QString("RMSD=%1").arg(chi2, 5);
   }

   results += 
      QString("factor: %1 %2\n")
      .arg( k )
      .arg( fit_msg );
   editor->append(results);

         
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
   
   display_iqq_residuals( scaling_target, 
                          use_q,
                          use_I,
                          use_source_I,
                          plot_color,
                          use_I_error
                          );
   
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
      filename = Q3FileDialog::getOpenFileName(use_dir, 
                                              "All files (*);;"
                                              "ssans files (*.ssans);;"
                                              "csv files (*.csv);;"
                                              "int files [cryson] (*.int);;"
                                              "fit files [cryson] (*.fit);;"
                                              , this
                                              , "open file dialog"
                                              , "Open"
                                              , &load_saxs_sans_selected_filter
                                              );
      if ( filename.isEmpty() )
      {
         return;
      }
      add_to_directory_history( filename );
   }

   plotted = false;
   QFile f(filename);
   our_saxs_options->path_load_saxs_curve = QFileInfo(filename).dirPath(true);
   QString ext = QFileInfo(filename).extension(FALSE).lower();

   if ( ext == "pdb" || ext == "PDB" )
   {
      QMessageBox::warning( this, "UltraScan",
                            QString(tr("Can not load a PDB file as a curve: ")
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

   bool do_crop = false;

   if ( f.open(QIODevice::ReadOnly) )
   {
      Q3TextStream ts(&f);
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
                               QString(tr("The file ")
                                       + filename + tr(" is empty.")) );
         return;
      }

      unsigned int number_of_fields = 0;
      if ( qv.size() > 3 )
      {
         QString test_line = qv[2];
         test_line.replace(QRegExp("^\\s+"),"");
         test_line.replace(QRegExp("\\s+$"),"");
         QStringList test_list = QStringList::split(QRegExp("\\s+"), test_line);
         number_of_fields = test_list.size();
         cout << "number of fields: " << number_of_fields << endl;
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
            res = QInputDialog::getItem(
                                        "Crysol's .int format has four available datasets", 
                                        "Select the set you wish to plot::", lst, 0, FALSE, &ok,
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
               cout << " column " << Icolumn << endl;
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
         do_crop = true;

         Icolumn = 1;
         I_errorcolumn = 2;
         if ( qsl.grep("exp_intensity").size() )
         {
            I_errorcolumn = 0;
            
            switch ( QMessageBox::question(this, 
                                           tr("UltraScan Notice"),
                                           QString(tr("Please note:\n\n"
                                                      "The file appears to have both experiment and model data\n"
                                                      "What would you like to do?\n"))
                                           ,
                                           tr("&Load only experimental"),
                                           tr("&Load only the model"),
                                           tr("&Load both"),
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
         do_crop = true;

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
            res = QInputDialog::getItem(
                                        "There are three available datasets", 
                                        "Select the set you wish to plot::", lst, 0, FALSE, &ok,
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
               cout << " column " << Icolumn << endl;
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
                                           tr("UltraScan"),
                                           tr("Is this file in 1/Angstrom or 1/nm units?"),
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

      QRegExp rx_ok_line("^(\\s+|\\d+|\\.|\\d(E|e)(\\+|-|\\d))+$");
      rx_ok_line.setMinimal( true );
      for ( unsigned int i = 1; i < (unsigned int) qv.size(); i++ )
      {
         if ( qv[i].contains(QRegExp("^#")) ||
              rx_ok_line.search( qv[i] ) == -1 )
         {
            cout << "not ok: " << qv[i] << endl; 
            continue;
         }
         
         QStringList tokens = QStringList::split(QRegExp("\\s+"), qv[i].replace(QRegExp("^\\s+"),""));
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

      cout << "q_range after load: " << q[0] << " , " << q[q.size() - 1] << endl;

      cout << QFileInfo(filename).fileName() << endl;
      if ( do_crop )
      {
         if ( Icolumn2 )
         {
            q2 = q;
            crop_iq_data(q2, I2);
         }
         if ( I_error.size() )
         {
            crop_iq_data(q, I, I_error );
         } else {
            crop_iq_data(q, I);
         }
      }
         
      cout << "q_range after crop: " << q[0] << " , " << q[q.size() - 1] << endl;

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
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header =
            QString("")
            .sprintf(
                     "Saxs curves from %s"
                     , filename.ascii()
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
