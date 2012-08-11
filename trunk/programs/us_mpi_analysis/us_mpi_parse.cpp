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
            data_sets << d;
         }
      }
   }

   file.close();
}

void US_MPI_Analysis::parse_job( QXmlStreamReader& xml )
{
   QXmlStreamAttributes a;

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
               if ( name == "bucket" )
               {
                  QXmlStreamAttributes a    = xml.attributes();
                  Bucket               b;
                  b.s_min       = a.value( "s_min"   ).toString().toDouble();
                  b.s_max       = a.value( "s_max"   ).toString().toDouble();
                  b.ff0_min     = a.value( "ff0_min" ).toString().toDouble();
                  b.ff0_max     = a.value( "ff0_max" ).toString().toDouble();

                  buckets << b;
               }
               else
               {
                  QXmlStreamAttributes a    = xml.attributes();
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
   QXmlStreamAttributes a;

   while ( ! xml.atEnd() )
   {
      xml.readNext();
      QString              name = xml.name().toString();

      if ( xml.isEndElement()  &&  xml.name() == "dataset" ) return;

      if ( xml.isStartElement()  &&  xml.name() == "files" )
         parse_files( xml, dataset ); 

      if ( xml.isStartElement()  &&  xml.name() == "solution" )
         parse_solution( xml, dataset ); 

      if ( xml.isStartElement()  &&  xml.name() == "simpoints" )
      {
         a                  = xml.attributes();
         dataset->simparams.simpoints
                            = a.value( "value" ).toString().toInt();
      }

      if ( xml.isStartElement()  &&  xml.name() == "band_volume" )
      {
         a                  = xml.attributes();
         dataset->simparams.band_volume
                            = a.value( "value" ).toString().toDouble();
      }

      if ( xml.isStartElement()  &&  xml.name() == "radial_grid" )
      {
         a                  = xml.attributes();
         dataset->simparams.meshType = (US_SimulationParameters::MeshType)
                              a.value( "value" ).toString().toInt();
      }

      if ( xml.isStartElement()  &&  xml.name() == "time_grid" )
      {
         a                  = xml.attributes();
         dataset->simparams.gridType = (US_SimulationParameters::GridType)
                              a.value( "value" ).toString().toInt();
      }

      if ( xml.isStartElement()  &&  xml.name() == "density" )
      {
         a                    = xml.attributes();
         dataset->density     = a.value( "value" ).toString().toDouble();
      }

      if ( xml.isStartElement()  &&  xml.name() == "viscosity" )
      {
         a                    = xml.attributes();
         dataset->viscosity   = a.value( "value" ).toString().toDouble();
      }

      if ( xml.isStartElement()  &&  xml.name() == "rotor_stretch" )
      {
         a                    = xml.attributes();
         QStringList stretch  = 
            a.value( "value" ).toString().split( " ", QString::SkipEmptyParts );

         dataset->rotor_stretch[ 0 ] = stretch[ 0 ].toDouble();
         dataset->rotor_stretch[ 1 ] = stretch[ 1 ].toDouble();
      }

      if ( xml.isStartElement()  &&  xml.name() == "centerpiece_bottom" )
      {
         a                      = xml.attributes();
         dataset->centerpiece_bottom
                                = a.value( "value" ).toString().toDouble();
      }

      if ( xml.isStartElement()  &&  xml.name() == "centerpiece_shape" )
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
                                = qMax( 0, shapes.indexOf( shape );
      }

      if ( xml.isStartElement()  &&  xml.name() == "centerpiece_angle" )
      {
         a                      = xml.attributes();
         dataset->simparams.cp_angle
                                = a.value( "value" ).toString().toDouble();
      }

      if ( xml.isStartElement()  &&  xml.name() == "centerpiece_pathlength" )
      {
         a                      = xml.attributes();
         dataset->simparams.cp_pathlen
                                = a.value( "value" ).toString().toDouble();
      }

      if ( xml.isStartElement()  &&  xml.name() == "centerpiece_width" )
      {
         a                      = xml.attributes();
         dataset->simparams.cp_width
                                = a.value( "value" ).toString().toDouble();
      }
   }
}

void US_MPI_Analysis::parse_files( QXmlStreamReader& xml,
      US_SolveSim::DataSet* dataset )
{
   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isEndElement()  &&  xml.name() == "files" ) return;

      if ( xml.isStartElement() )
      {
         QXmlStreamAttributes a        = xml.attributes();
         QString              type     = xml.name().toString();
         QString              filename = a.value( "filename" ).toString();

         if ( type == "auc"   ) dataset->auc_file   = filename;
         if ( type == "edit"  ) dataset->edit_file  = filename;
         if ( type == "model" ) dataset->model_file = filename;
         if ( type == "noise" ) dataset->noise_files << filename;
      }
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

