#include "us_mpi_analysis.h"
#include "mpi.h"

//  Parse XML routines
void US_MPI_Analysis::parse( const QString& xmlfile )
{
   QFile file ( xmlfile );

   if ( ! file.open( QIODevice::ReadOnly | QIODevice::Text) )
   {
      // Fail quietly - can't use udp yet
      if ( my_rank == 0 ) qDebug() << "Cannot open file " << xmlfile;
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
            DataSet* d = new DataSet;
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

      if ( xml.isEndElement()  &&  xml.name() == "job" ) return;

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
         port   = a.value( "port" ).toString().toInt();
      }

      if ( xml.isStartElement()  &&  xml.name() == "requestID" )
      {
         a       = xml.attributes();
         QString s;
         requestID = s.sprintf( "%06d", a.value( "id" ).toString().toInt() );
      }

      if ( xml.isStartElement()  &&  xml.name() == "jobParameters" )
      {
         while ( ! xml.atEnd() )
         {
            xml.readNext();

            if ( xml.isEndElement()  &&  xml.name() == "jobParameters" ) break;

            if ( xml.isStartElement() )
            {
               QXmlStreamAttributes a    = xml.attributes();
               QString              name = xml.name().toString();
               parameters[ name ]        = a.value( "value" ).toString();
            }
         }
      }
   }
}

void US_MPI_Analysis::parse_dataset( QXmlStreamReader& xml, DataSet* dataset )
{
   QXmlStreamAttributes a;

   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isEndElement()  &&  xml.name() == "dataset" ) return;

      if ( xml.isStartElement()  &&  xml.name() == "files" )
         parse_files( xml, dataset ); 

      if ( xml.isStartElement()  &&  xml.name() == "simpoints" )
      {
         a                  = xml.attributes();
         dataset->simpoints = a.value( "value" ).toString().toInt();
      }

      if ( xml.isStartElement()  &&  xml.name() == "band_volume" )
      {
         a                    = xml.attributes();
         dataset->band_volume = a.value( "value" ).toString().toDouble();
      }

      if ( xml.isStartElement()  &&  xml.name() == "radial_grid" )
      {
         a                    = xml.attributes();
         dataset->radial_grid = a.value( "value" ).toString().toInt();
      }

      if ( xml.isStartElement()  &&  xml.name() == "time_grid" )
      {
         a                    = xml.attributes();
         dataset->time_grid   = a.value( "value" ).toString().toInt();
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
      
      if ( xml.isStartElement()  &&  xml.name() == "centerpiece_bottom" )
      {
         a                      = xml.attributes();
         dataset->centerpiece_bottom = a.value( "value" ).toString().toDouble();
      }
      
      if ( xml.isStartElement()  &&  xml.name() == "vbar20" )
      {
         a                      = xml.attributes();
         dataset->vbar20        = a.value( "value" ).toString().toDouble();
      }
      
      if ( xml.isStartElement()  &&  xml.name() == "rotor_stretch" )
      {
         a                    = xml.attributes();
         QStringList stretch  = 
            a.value( "value" ).toString().split( " ", QString::SkipEmptyParts );

         dataset->rotor_stretch[ 0 ] = stretch[ 0 ].toDouble();
         dataset->rotor_stretch[ 1 ] = stretch[ 1 ].toDouble();
      }
   }
}

void US_MPI_Analysis::parse_files( QXmlStreamReader& xml, DataSet* dataset )
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
