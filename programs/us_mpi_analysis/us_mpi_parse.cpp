#include "us_mpi_analysis.h"
#include "us_math2.h"
#include "us_settings.h"
#include "mpi.h"

//  Parse XML routines
void US_MPI_Analysis::parse( const QString& xmlfile )
{
   QFile file ( xmlfile );

   if ( ! file.open( QIODevice::ReadOnly | QIODevice::Text) )
   {
      // Fail quietly - can't use udp yet
      if ( my_rank == 0 ) DbgLv(0) << "Cannot open file " << xmlfile;
      printf( "Cannot open file %s\n", xmlfile.toAscii().data() );
      printf( "Aborted" );
      MPI_Finalize();
      qApp->exit();
   }

   QXmlStreamReader xml( &file );

   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isStartElement() )
      {
         if ( xml.name() == "US_JobSubmit" )
         {
            QXmlStreamAttributes a = xml.attributes();
            analysis_type = a.value( "method" ).toString();
         }

         if ( xml.name() == "job" )
            parse_job( xml );

         if ( xml.name() == "dataset" )
         {
            US_SolveSim::DataSet* d = new US_SolveSim::DataSet;
            parse_dataset( xml, d );

            if ( parameters.contains( "CG_model" ) )
            {  // Flag Custom Grid model input
               d->model_file  = parameters[ "CG_model" ];
               d->solute_type = 2;
            }

            else if ( ! parameters[ "ztype" ].isEmpty() )
            {  // Flag generic x,y model with fixed "z"
               QStringList s_attrs;
               s_attrs << "s" << "ff0" << "MW" << "vbar" << "D" << "f";
               attr_x         = s_attrs.indexOf( parameters[ "xtype" ] );
               attr_y         = s_attrs.indexOf( parameters[ "ytype" ] );
               attr_z         = s_attrs.indexOf( parameters[ "ztype" ] );
               attr_x         = ( attr_x < 0 ) ? ATTR_S : attr_x;
               attr_y         = ( attr_y < 0 ) ? ATTR_K : attr_y;
               attr_z         = ( attr_z < 0 ) ? ATTR_V : attr_z;
               // The solute type flag is a number which when viewed as
               //  octal is in the form "01xyz".
               d->solute_type = ( attr_x << 6 ) + ( attr_y << 3 ) + attr_z;
            }

            else if ( parameters[ "ytype" ] == "vbar" )
            {  // Flag s,vbar model with fixed f_f0
               d->solute_type = 1;
               attr_x         = ATTR_S;
               attr_y         = ATTR_V;
               attr_z         = ATTR_K;
            }

            else
            {  // Flag standard s,k model with fixed vbar
               d->solute_type = 0;
               attr_x         = ATTR_S;
               attr_y         = ATTR_K;
               attr_z         = ATTR_V;
//d->solute_type = (0 << 6) + (1 << 3) + 3;
            }

if (my_rank==0) DbgLv(0) << "PF: solute_type" << d->solute_type;
            data_sets << d;
         }
      }
   }

   file.close();
}

void US_MPI_Analysis::parse_job( QXmlStreamReader& xml )
{
   QXmlStreamAttributes a;
   QString ytype;

   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isEndElement()  &&  xml.name() == "job" ) break;

      if ( xml.isStartElement()  &&  xml.name() == "cluster" )
      {
         a       = xml.attributes();
         cluster = a.value( "shortname" ).toString();
      }

      if ( xml.isStartElement()  &&  xml.name() == "name" )
      {
         a       = xml.attributes();
         db_name = a.value( "value" ).toString();
      }

      if ( xml.isStartElement()  &&  xml.name() == "udp" )
      {
         a      = xml.attributes();
         server = QHostAddress( a.value( "server" ).toString() );
         port   = (quint16)a.value( "port" ).toString().toInt();
      }

      if ( xml.isStartElement()  &&  xml.name() == "request" )
      {
         a       = xml.attributes();
         QString s;
         requestID   = s.sprintf( "%06d", a.value( "id" ).toString().toInt() );
         requestGUID = a.value( "guid" ).toString();
      }

      if ( xml.isStartElement()  &&  xml.name() == "jobParameters" )
      {
         while ( ! xml.atEnd() )
         {
            xml.readNext();
            QString name = xml.name().toString();

            if ( xml.isEndElement()  &&  name == "jobParameters" ) break;

            if ( xml.isStartElement() )
            {
               a       = xml.attributes();

               if ( name == "bucket" )
               { // Get bucket coordinates; try to forestall round-off problems
                  QString ytyp0 = QString( "ff0" );
                  Bucket b;
                  QString stxmn = a.value( "x_min"    ).toString();
                  QString stxmx = a.value( "x_max"    ).toString();
                  QString stymn = a.value( "y_min"    ).toString();
                  QString stymx = a.value( "y_max"    ).toString();
                  QString stfmn = a.value( "ff0_min"  ).toString();
                  QString stfmx = a.value( "ff0_max"  ).toString();
                  QString stvmn = a.value( "vbar_min" ).toString();
                  QString stvmx = a.value( "vbar_max" ).toString();
                  double xmin   = stxmn.toDouble();
                  double xmax   = stxmx.toDouble();
                  double ymin   = stymn.toDouble();
                  double ymax   = stymx.toDouble();

                  if ( stxmx.isEmpty() )
                  {
                     xmin       = a.value( "s_min"   ).toString().toDouble();
                     xmax       = a.value( "s_max"   ).toString().toDouble();
                     if ( !stfmx.isEmpty() )
                     {
                        ymin    = stfmn.toDouble();
                        ymax    = stfmx.toDouble();
                        ytyp0   = QString( "ff0" );
                     }
                     else if ( !stvmx.isEmpty() )
                     {
                        ymin    = stvmn.toDouble();
                        ymax    = stvmx.toDouble();
                        ytyp0   = QString( "vbar" );
                     }
                  }

                  if ( ytype.isEmpty() )
                  {
                     ytype      = ytyp0;
                     parameters[ "ytype" ]     = ytype;
                  }

                  // Try to forestall round-off problems
if (my_rank==0) DbgLv(0) << "PF: xymnmx" << xmin << xmax << ymin << ymax;
                  int xpwr      = (int)qFloor( log10( xmax ) ) - 6;
                  int ypwr      = (int)qFloor( log10( ymax ) ) - 6;
                  double xinc   = pow( 10.0, xpwr );
                  double yinc   = pow( 10.0, ypwr );
if (my_rank==0) DbgLv(0) << "PF:  xp yp xi yi" << xpwr << ypwr << xinc << yinc;
                  b.x_min       = qRound64( xmin / xinc ) * xinc;
                  b.x_max       = qRound64( xmax / xinc ) * xinc;
                  b.y_min       = qRound64( ymin / yinc ) * yinc;
                  b.y_max       = qRound64( ymax / yinc ) * yinc;
if (my_rank==0) DbgLv(0) << "PF:   rnd xymnmx" << b.x_min << b.x_max << b.y_min << b.y_max;

                  buckets << b;
               }
               else if ( name == "CG_model" )
               {
                  parameters[ name ]        = a.value( "filename" ).toString();
               }
               else if ( name == "bucket_fixed" )
               {
                  parameters[ name ]        = a.value( "value" ).toString();
                  parameters[ "xtype" ]     = a.value( "xtype" ).toString();
                  parameters[ "ytype" ]     = a.value( "ytype" ).toString();
                  parameters[ "ztype" ]     = a.value( "fixedtype" ).toString();
                  ytype                     = parameters[ "ytype" ];
               }
               else
               {
                  parameters[ name ]        = a.value( "value" ).toString();
               }
            }
         }
      }
   }

   if ( parameters.contains( "debug_level" ) )
      dbg_level  = parameters[ "debug_level" ].toInt();

   else
      dbg_level  = 0;

   US_Settings::set_us_debug( dbg_level );
   dbg_timing = ( parameters.contains( "debug_timings" )
              &&  parameters[ "debug_timings" ].toInt() != 0 );
}

void US_MPI_Analysis::parse_dataset( QXmlStreamReader& xml,
      US_SolveSim::DataSet* dataset )
{
   dataset->simparams.speed_step.clear();

   QXmlStreamAttributes a;

   while ( ! xml.atEnd() )
   {
      xml.readNext();
      QString              name = xml.name().toString();

      if ( xml.isEndElement()  &&  xml.name() == "dataset" ) return;

      if ( ! xml.isStartElement() )  continue;

      if ( xml.name() == "files" )
         parse_files( xml, dataset ); 

      if ( xml.name() == "solution" )
         parse_solution( xml, dataset ); 

      if ( xml.name() == "simpoints" )
      {
         a                  = xml.attributes();
         dataset->simparams.simpoints
                            = a.value( "value" ).toString().toInt();
      }

      if ( xml.name() == "band_volume" )
      {
         a                  = xml.attributes();
         dataset->simparams.band_volume
                            = a.value( "value" ).toString().toDouble();
      }

      if ( xml.name() == "radial_grid" )
      {
         a                  = xml.attributes();
         dataset->simparams.meshType = (US_SimulationParameters::MeshType)
                              a.value( "value" ).toString().toInt();
      }

      if ( xml.name() == "time_grid" )
      {
         a                  = xml.attributes();
         dataset->simparams.gridType = (US_SimulationParameters::GridType)
                              a.value( "value" ).toString().toInt();
      }

      if ( xml.name() == "density" )
      {
         a                    = xml.attributes();
         dataset->density     = a.value( "value" ).toString().toDouble();
      }

      if ( xml.name() == "viscosity" )
      {
         a                    = xml.attributes();
         dataset->viscosity   = a.value( "value" ).toString().toDouble();
      }

      if ( xml.name() == "rotor_stretch" )
      {
         a                    = xml.attributes();
         QStringList stretch  = 
            a.value( "value" ).toString().split( " ", QString::SkipEmptyParts );

         dataset->rotor_stretch[ 0 ] = stretch[ 0 ].toDouble();
         dataset->rotor_stretch[ 1 ] = stretch[ 1 ].toDouble();
      }

      if ( xml.name() == "centerpiece_bottom" )
      {
         a                      = xml.attributes();
         dataset->centerpiece_bottom
                                = a.value( "value" ).toString().toDouble();
      }

      if ( xml.name() == "centerpiece_shape" )
      {
         a                      = xml.attributes();
         QString shape          = a.value( "value" ).toString();
         QStringList shapes;
         shapes << "sector"
                << "standard"
                << "rectangular"
                << "band-forming"
                << "meniscus-matching"
                << "circular"
                << "synthetic";
         dataset->simparams.cp_sector
                                = qMax( 0, shapes.indexOf( shape ) );
      }

      if ( xml.name() == "centerpiece_angle" )
      {
         a                      = xml.attributes();
         dataset->simparams.cp_angle
                                = a.value( "value" ).toString().toDouble();
      }

      if ( xml.name() == "centerpiece_pathlength" )
      {
         a                      = xml.attributes();
         dataset->simparams.cp_pathlen
                                = a.value( "value" ).toString().toDouble();
      }

      if ( xml.name() == "centerpiece_width" )
      {
         a                      = xml.attributes();
         dataset->simparams.cp_width
                                = a.value( "value" ).toString().toDouble();
      }

      if ( xml.name() == "speedstep" )
      {
         US_SimulationParameters::SpeedProfile sp;

         US_SimulationParameters::speedstepFromXml( xml, sp );

         dataset->simparams.speed_step << sp;
      }
   }
}

void US_MPI_Analysis::parse_files( QXmlStreamReader& xml,
      US_SolveSim::DataSet* dataset )
{
   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isEndElement()  &&  xml.name() == "files" )  break;

      if ( xml.isStartElement() )
      {
         QXmlStreamAttributes a        = xml.attributes();
         QString              type     = xml.name().toString();
         QString              filename = a.value( "filename" ).toString();

         if ( type == "auc"   ) dataset->auc_file   = filename;
         if ( type == "edit"  ) dataset->edit_file  = filename;
         if ( type == "noise" ) dataset->noise_files << filename;
      }
   }

   QString clambda = dataset->edit_file.section( ".", -2, -2 );
   if ( clambda.contains( "-" ) )
   {  // Modify edit file name for MWL case
      clambda         = clambda + "@" + dataset->auc_file.section( ".", -2, -2 );
      QString fpart1  = dataset->edit_file.section( ".",  0, -3 );
      QString fpart2  = dataset->edit_file.section( ".", -1, -1 );
      dataset->edit_file = fpart1 + "." + clambda + "." + fpart2;
if (my_rank==0) DbgLv(0) << "PF: MWL edit_file" << dataset->edit_file;
   }
}

void US_MPI_Analysis::parse_solution( QXmlStreamReader& xml,
      US_SolveSim::DataSet* dataset )
{
   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isEndElement()  &&  xml.name() == "solution" ) break;

      if ( xml.isStartElement() && xml.name() == "buffer" )
      {
         QXmlStreamAttributes a        = xml.attributes();
         dataset->density   = a.value( "density"   ).toString().toDouble();
         dataset->viscosity = a.value( "viscosity" ).toString().toDouble();
         dataset->manual    = a.value( "manual"    ).toString().toInt();
      }

      if ( xml.isStartElement() && xml.name() == "analyte" )
      {
         US_Solution::AnalyteInfo aninfo;
         QXmlStreamAttributes a        = xml.attributes();

         aninfo.analyte.mw     = a.value( "mw"     ).toString().toDouble();
         aninfo.analyte.vbar20 = a.value( "vbar20" ).toString().toDouble();
         aninfo.amount         = a.value( "amount" ).toString().toDouble();
         QString atype         = a.value( "type"   ).toString();
         aninfo.analyte.type   = US_Analyte::PROTEIN;
         if ( atype == "DNA" )
            aninfo.analyte.type   = US_Analyte::DNA;
         if ( atype == "RNA" )
            aninfo.analyte.type   = US_Analyte::RNA;
         if ( atype == "Other" )
            aninfo.analyte.type   = US_Analyte::CARBOHYDRATE;

         dataset->solution_rec.analyteInfo << aninfo;
      }
   }

   dataset->vbar20  = US_Math2::calcCommonVbar( dataset->solution_rec, 20.0 );
}

