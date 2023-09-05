#include "us_mpi_analysis.h"
#include "us_math2.h"
#include "us_settings.h"
#include "mpi.h"

//  Parse XML routines
void US_MPI_Analysis::parse( const QString& xmlfile )
{
   DbgLv(0) << "parse: started" << my_rank;
   QFile file ( xmlfile );

   if ( ! file.open( QIODevice::ReadOnly | QIODevice::Text) )
   {
      // Fail quietly - can't use udp yet
      if ( my_rank == 0 ) DbgLv(0) << "Cannot open file " << xmlfile;
      printf( "Cannot open file %s\n", xmlfile.toLatin1().data() );
      printf( "Aborted" );
      MPI_Finalize();
      qApp->exit();
   }

   QString tmst_fn;
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
            d->tmst_file   = tmst_fn;       // Initialize to blank or previous
            parse_dataset( xml, d );
            tmst_fn        = d->tmst_file;  // Init for subsequent dataset

            if ( parameters.contains( "CG_model" ) )
            {  // Flag Custom Grid model input
               d->model_file  = parameters[ "CG_model" ];
               d->solute_type = 2;
               attr_x         = ATTR_S;
               attr_y         = ATTR_V;
               attr_z         = ATTR_K;
            }

            else if ( parameters.contains( "DC_model" ) )
            {  // Flag DMGA_Constraints model input
               d->model_file  = parameters[ "DC_model" ];
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

            else if ( ! parameters[ "solute_type" ].isEmpty() )
            {  // Flag explicit solute type
               QString s_styp = parameters[ "solute_type" ];
               d->solute_type = US_ModelRecord::stype_flag( s_styp );
               attr_x         = ( d->solute_type >> 6 ) & 7;
               attr_y         = ( d->solute_type >> 3 ) & 7;
               attr_z         =   d->solute_type        & 7;
            }

            else
            {  // Flag standard s,k model with fixed vbar
               d->solute_type = 0;
               attr_x         = ATTR_S;
               attr_y         = ATTR_K;
               attr_z         = ATTR_V;
//d->solute_type = (0 << 6) + (1 << 3) + 3;
            }

if (my_rank==0) DbgLv(0) << "PF: solute_type" << d->solute_type
 << "attr_x _y _z" << attr_x << attr_y << attr_z;
            data_sets << d;
         }
      }
   }

   file.close();

   if ( parameters.contains( "debug_text" ) )
   {
      QString dbgtxt_str     = parameters[ "debug_text" ];
      QStringList dbgtxt_sls = dbgtxt_str.split( ',' );
      int ndtln              = dbgtxt_sls.count();

      for ( int ii = 0; ii < ndtln; ii++ )
      {
         dbgtxt_sls[ ii ] = dbgtxt_sls[ ii ].simplified();
      }

      US_Settings::set_debug_text( dbgtxt_sls );
//*DEBUG*
if (my_rank==0) {
DbgLv(0) << "DBGTXT: dt string: " << dbgtxt_str;
DbgLv(0) << "DBGTXT: dt sl_count" << ndtln;
DbgLv(0) << "DBGTXT: dt strlst: " << dbgtxt_sls;
dbgtxt_sls = US_Settings::debug_text();
ndtln = dbgtxt_sls.count();
DbgLv(0) << "DBGTXT: dt sl_count(2)" << ndtln;
DbgLv(0) << "DBGTXT: dt strlst(2): " << dbgtxt_sls;
}
//*DEBUG*
   }
   DbgLv(0) << "parse: finished" << my_rank;
}

void US_MPI_Analysis::parse_job( QXmlStreamReader& xml )
{
   DbgLv(0) << "parse_job: started" << my_rank;
   QXmlStreamAttributes attr;
   QString xname;
   QString ytype;

   while ( ! xml.atEnd() )
   {
      xml.readNext();
      xname       = xml.name().toString();

      if ( xml.isEndElement()  &&  xname == "job" ) break;

      if ( xml.isStartElement() )
      {
         attr        = xml.attributes();

         if ( xname == "cluster" )
         {
            cluster     = attr.value( "shortname" ).toString();
         }

         else if ( xname == "name" )
         {
            db_name     = attr.value( "value" ).toString();
         }

         else if ( xname == "udp" )
         {
            server     = QHostAddress( attr.value( "server" ).toString() );
            port       = (quint16)attr.value( "port" ).toString().toInt();
         }

         else if ( xname == "global_fit" )
         {
            parameters[ xname ] = attr.value( "value" ).toString();
         }

         else if ( xname == "request" )
         {
            QString ss;
            requestID   = ss.sprintf( "%06d",
                             attr.value( "id" ).toString().toInt() );
            requestGUID = attr.value( "guid" ).toString();
         }

         else if ( xname == "jobParameters" )
         {
            while ( ! xml.atEnd() )
            {
               xml.readNext();
               QString name = xml.name().toString();

               if ( xml.isEndElement()  &&  name == "jobParameters" ) break;

               if ( xml.isStartElement() )
               {
                  attr    = xml.attributes();

                  if ( name == "bucket" )
                  { // Get bucket coords; try to forestall round-off problems
                     QString ytyp0 = QString( "ff0" );
                     Bucket b;
                     QString stxmn = attr.value( "x_min"    ).toString();
                     QString stxmx = attr.value( "x_max"    ).toString();
                     QString stymn = attr.value( "y_min"    ).toString();
                     QString stymx = attr.value( "y_max"    ).toString();
                     QString stfmn = attr.value( "ff0_min"  ).toString();
                     QString stfmx = attr.value( "ff0_max"  ).toString();
                     QString stvmn = attr.value( "vbar_min" ).toString();
                     QString stvmx = attr.value( "vbar_max" ).toString();
                     double xmin   = stxmn.toDouble();
                     double xmax   = stxmx.toDouble();
                     double ymin   = stymn.toDouble();
                     double ymax   = stymx.toDouble();

                     if ( stxmx.isEmpty() )
                     {
                        xmin       = attr.value( "s_min").toString().toDouble();
                        xmax       = attr.value( "s_max").toString().toDouble();
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
                  else if ( name == "CG_model"  ||  name == "DC_model" )
                  {
                     parameters[ name ]    = attr.value( "filename").toString();
if (my_rank==0) DbgLv(0) << "PF:   DC_model" << parameters[name] << name;
                  }
                  else if ( name == "bucket_fixed" )
                  {
                     parameters[ name ]    = attr.value( "value" ).toString();
                     parameters[ "xtype" ] = attr.value( "xtype" ).toString();
                     parameters[ "ytype" ] = attr.value( "ytype" ).toString();
                     parameters[ "ztype" ] = attr.value("fixedtype").toString();
                     ytype                 = parameters[ "ytype" ];
                  }

                  else if ( name.startsWith( "s_m" )  ||
                            name.startsWith( "ff0_m" ) )
                  {
                     QString svalu = attr.value( "value" ).toString();
                     parameters[ name ]    = svalu;
                     name          = name.replace( "s_m",   "x_m" );
                     name          = name.replace( "ff0_m", "y_m" );
                     parameters[ name ]    = svalu;
                  }

                  else
                  {
                     parameters[ name ]    = attr.value( "value" ).toString();
                  }
               }
            }
         }
      }

      if ( ! parameters.contains( "z_value" ) )
         parameters[ "z_value" ]     = "0.0";

      if ( parameters.contains( "debug_level" ) )
         dbg_level  = parameters[ "debug_level" ].toInt() + 1;

      else
         dbg_level  = 0;

//      US_Settings::set_us_debug( dbg_level );
      int dbglv = ( my_rank < 2 || my_rank == 15 ) ? dbg_level : 0;
      US_Settings::set_us_debug( dbglv );
      dbg_timing = ( parameters.contains( "debug_timings" )
                 &&  parameters[ "debug_timings" ].toInt() != 0 );
   }
   DbgLv(-1) << "parse_job: finished" << my_rank;
}

void US_MPI_Analysis::parse_dataset( QXmlStreamReader& xml, DATASET* dataset )
{
   DbgLv(0) << "parse_dataset: started" << my_rank;
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

      a                  = xml.attributes();
      if ( xml.name() == "simpoints" )
      {
         dataset->simparams.simpoints
                            = a.value( "value" ).toString().toInt();
      }

      if ( xml.name() == "band_volume" )
      {
         dataset->simparams.band_volume
                            = a.value( "value" ).toString().toDouble();
      }

      if ( xml.name() == "radial_grid" )
      {
         dataset->simparams.meshType = (US_SimulationParameters::MeshType)
                              a.value( "value" ).toString().toInt();
      }

      if ( xml.name() == "time_grid" )
      {
         dataset->simparams.gridType = (US_SimulationParameters::GridType)
                              a.value( "value" ).toString().toInt();
      }

      if ( xml.name() == "density" )
      {
         dataset->density     = a.value( "value" ).toString().toDouble();
      }

      if ( xml.name() == "viscosity" )
      {
         dataset->viscosity   = a.value( "value" ).toString().toDouble();
      }

      if ( xml.name() == "compress" )
      {
         dataset->compress    = a.value( "value" ).toString().toDouble();
      }

      if ( xml.name() == "rotor_stretch" )
      {
         QStringList stretch  = 
            a.value( "value" ).toString().split( " ", QString::SkipEmptyParts );

         dataset->rotor_stretch[ 0 ] = stretch[ 0 ].toDouble();
         dataset->rotor_stretch[ 1 ] = stretch[ 1 ].toDouble();
      }

      if ( xml.name() == "centerpiece_bottom" )
      {
         dataset->centerpiece_bottom
                                = a.value( "value" ).toString().toDouble();
      }

      if ( xml.name() == "centerpiece_shape" )
      {
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
         dataset->simparams.cp_angle
                                = a.value( "value" ).toString().toDouble();
      }

      if ( xml.name() == "centerpiece_pathlength" )
      {
         dataset->simparams.cp_pathlen
                                = a.value( "value" ).toString().toDouble();
      }

      if ( xml.name() == "centerpiece_width" )
      {
         dataset->simparams.cp_width
                                = a.value( "value" ).toString().toDouble();
      }

      if ( xml.name() == "speedstep" )
      {
         US_SimulationParameters::SpeedProfile sp;

         US_SimulationParameters::speedstepFromXml( xml, sp );

         dataset->simparams.speed_step << sp;
      }

      if ( xml.name() == "total_concentration" )
      {
         concentrations << a.value( "value" ).toString().toDouble();
      }
   }
   DbgLv(-1) << "parse_dataset: finished" << my_rank;
}

void US_MPI_Analysis::parse_files( QXmlStreamReader& xml, DATASET* dataset )
{
   DbgLv(0) << "parse_files: started" << my_rank;
   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isEndElement()  &&  xml.name() == "files" )  break;

      if ( xml.isStartElement() )
      {
         QXmlStreamAttributes a        = xml.attributes();
         QString              type     = xml.name().toString();
         QString              filename = a.value( "filename" ).toString();

         if ( type == "auc"   )      dataset->auc_file   = filename;
         if ( type == "edit"  )      dataset->edit_file  = filename;
         if ( type == "noise" )      dataset->noise_files << filename;
         if ( type == "timestate" )  dataset->tmst_file  = filename;
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
   DbgLv(0) << "parse_files: finished" << my_rank;
}

void US_MPI_Analysis::parse_solution( QXmlStreamReader& xml, DATASET* dataset )
{
   DbgLv(0) << "parse_solution: started"  << my_rank;
   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isEndElement()  &&  xml.name() == "solution" ) break;

      if ( xml.isStartElement() && xml.name() == "buffer" )
      {
         QXmlStreamAttributes a        = xml.attributes();
         dataset->density   = a.value( "density"   ).toString().toDouble();
         dataset->viscosity = a.value( "viscosity" ).toString().toDouble();
         dataset->compress  = a.value( "compress"  ).toString().toDouble();
         dataset->manual    = a.value( "manual"    ).toString().toInt();
         dataset->solution_rec.buffer.viscosity = dataset->viscosity;
         dataset->solution_rec.buffer.density = dataset->density;
         dataset->solution_rec.buffer.cosed_componentIDs.clear();
         dataset->solution_rec.buffer.cosed_component.clear();
         while (!xml.atEnd()){
            xml.readNext();
            if (xml.isEndElement() && xml.name() == "buffer") break;
            if (xml.isStartElement() && xml.name() == "cosedcomponent") {
               US_CosedComponent bc;

               QXmlStreamAttributes ab = xml.attributes();
               bc.id = ab.value("id").toInt();
               bc.componentID = ab.value("id").toString();
               bc.name = ab.value("name").toString();
               bc.overlaying = US_Util::bool_flag(ab.value("overlay").toString());
               bc.conc = ab.value("conc").toString().toDouble();
               bc.s_coeff = ab.value("s").toString().toDouble()*1E-13;
               bc.d_coeff = ab.value("D").toString().toDouble(); // *1e-6
               bc.vbar = ab.value("vbar").toString().toDouble();
               QStringList dens = ab.value("dens").toString().split(" ");
               QStringList visc = ab.value("visc").toString().split(" ");
               for(int i = 0; i < dens.length(); i++ ){
                  bc.dens_coeff[i] = dens[i].toDouble();
               }
               for(int i = 0; i < visc.length(); i++ ){
                  bc.visc_coeff[i] = visc[i].toDouble();
               }
               DbgLv(0)<< bc.name << bc.visc_coeff;
               dataset->solution_rec.buffer.cosed_component << bc;
               dataset->solution_rec.buffer.cosed_componentIDs << bc.componentID;
            }
         }
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
   DbgLv(0) << "parse_solution: finished"  << my_rank;
}

