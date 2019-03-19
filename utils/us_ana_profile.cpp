//! \file us_ana_profile.cpp

#include "us_ana_profile.h"
#include "us_constants.h"
#include "us_settings.h"
#include "us_util.h"

#ifndef _TR_
#define _TR_(a) QObject::tr(a)
#endif

// AnaProfile constructor
US_AnaProfile::US_AnaProfile()
{
   aprofname       = "";
   aprofGUID       = QString( "00000000-0000-0000-0000-000000000000" );
   protoname       = "";
   protoGUID       = QString( "00000000-0000-0000-0000-000000000000" );

   pchans    << "1A";
   chndescs  << "1A:UV/vis.:(unspecified)";
   lc_ratios << 1.0;
   lc_tolers << 5.0;
   l_volumes << 460.0;
   lv_tolers << 10.0;
   data_ends << 7.0;
}

// AnaProfile Equality operator
bool US_AnaProfile::operator== ( const US_AnaProfile& ap ) const
{
   if ( aprofname    != ap.aprofname    )  return false;
   if ( aprofGUID    != ap.aprofGUID    )  return false;
   if ( protoname    != ap.protoname    )  return false;
   if ( protoGUID    != ap.protoGUID    )  return false;

   if ( ap2DSA       != ap.ap2DSA  )  return false;
   if ( apPCSA       != ap.apPCSA  )  return false;

   return true;
}

// Write all current controls to an XML stream
bool US_AnaProfile::toXml( QXmlStreamWriter& xmlo )
{
   xmlo.writeStartDocument();
   xmlo.writeDTD          ( "<!DOCTYPE US_AnalysisProfile>" );
   xmlo.writeStartElement ( "AnalysisProfileData" );
   xmlo.writeAttribute    ( "version", "1.0" );

   xmlo.writeStartElement ( "analysis_profile" );
   xmlo.writeAttribute    ( "name", aprofname );
   xmlo.writeAttribute    ( "guid", aprofGUID );

   for ( int ii = 0; ii < pchans.count(); ii++ )
   {
      xmlo.writeStartElement ( "channel_parms" );
      xmlo.writeAttribute    ( "channel",  pchans  [ ii ] );
      xmlo.writeAttribute    ( "chandesc", chndescs[ ii ] );
      xmlo.writeAttribute    ( "load_concen_ratio",
                               QString::number( lc_ratios[ ii ] ) );
      xmlo.writeAttribute    ( "lcr_tolerance",
                               QString::number( lc_tolers[ ii ] ) );
      xmlo.writeAttribute    ( "load_volume",
                               QString::number( l_volumes[ ii ] ) );
      xmlo.writeAttribute    ( "lv_tolerance",
                               QString::number( lv_tolers[ ii ] ) );
      xmlo.writeAttribute    ( "data_end",
                               QString::number( data_ends[ ii ] ) );
      xmlo.writeEndElement();    // channel_parms
   }

   ap2DSA.toXml( xmlo );
   apPCSA.toXml( xmlo );

   xmlo.writeEndElement();    // analysis_profile

   xmlo.writeEndElement();    // AnalysisProfileData
   xmlo.writeEndDocument();

   return ( ! xmlo.hasError() );
}

// Read all current controls from an XML stream
bool US_AnaProfile::fromXml( QXmlStreamReader& xmli )
{
   int chx            = 0;
   pchans   .clear();
   chndescs .clear();
   lc_ratios.clear();
   lc_tolers.clear();
   l_volumes.clear();
   lv_tolers.clear();
   data_ends.clear();

   while( ! xmli.atEnd() )
   {
      xmli.readNext();

      if ( xmli.isStartElement() )
      {
         QString ename   = xmli.name().toString();

         if ( ename == "analysis_profile" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
	    aprofname       = attr.value( "name" ).toString();
            aprofGUID       = attr.value( "guid" ).toString();
         }

         else if ( ename == "channel_parms" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
            pchans    << attr.value( "channel" ).toString();
            chndescs  << attr.value( "chandesc" ).toString();
            lc_ratios << attr.value( "load_concen_ratio" ).toString().toDouble();
            lc_tolers << attr.value( "lcr_tolerance" ).toString().toDouble();
            l_volumes << attr.value( "load_volume" ).toString().toDouble();
            lv_tolers << attr.value( "lv_tolerance" ).toString().toDouble();
            data_ends << attr.value( "data_end" ).toString().toDouble();
//3-------------------------------------------------------------------------->80
            chx++;
         }

         else if ( ename == "2DSA" )      { ap2DSA.fromXml( xmli ); }
         else if ( ename == "PCSA" )      { apPCSA.fromXml( xmli ); }
      }
   }

   return ( ! xmli.hasError() );
}


// AnaProf2DSA subclass constructor
US_AnaProfile::AnaProf2DSA::AnaProf2DSA()
{
   fitrng      = 0.3;
   nchan       = 1;
   grpoints    = 64;
   rfiters     = 0;
   mciters     = 0;
   job1run     = false;
   job2run     = false;
   job3run     = false;
   job4run     = false;
   job5run     = false;
   job3auto    = true;
   job1nois    = "ti";
   job2nois    = "both";
   job4nois    = "both";

   parms.clear();
   Parm2DSA parm1;
   parms << parm1;
}

// AnaProf2DSA subclass Equality operator
bool US_AnaProfile::AnaProf2DSA::operator== 
                  ( const AnaProf2DSA& ap ) const
{
   if ( nchan  != ap.nchan  ) return false;

   return true;
}

// Read all current 2DSA controls from an XML stream
bool US_AnaProfile::AnaProf2DSA::fromXml( QXmlStreamReader& xmli )
{
   nchan           = 0;
   parms.clear();
   Parm2DSA parm1;

   while( ! xmli.atEnd() )
   {
      QString ename   = xmli.name().toString();

      if ( xmli.isStartElement() )
      {

         if ( ename == "channel_parms" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
            QString chan   = attr.value( "channel" ).toString();
            QString vvflag = attr.value( "vary_vbar" ).toString();
            QString cgrid  = attr.value( "custom_grid_guid" ).toString();
            double smin    = attr.value( "s_min" ).toString().toDouble();
            double smax    = attr.value( "s_max" ).toString().toDouble();
            int sgpts      = attr.value( "s_gridpoints" ).toString().toInt();
            double kmin    = attr.value( "k_min" ).toString().toDouble();
            double kmax    = attr.value( "k_max" ).toString().toDouble();
            int kgpts      = attr.value( "k_gridpoints" ).toString().toInt();
            int grreps     = attr.value( "grid_repetitions" ).toString().toInt();
            parm1.channel    = chan;
            parm1.s_min      = smin;
            parm1.s_max      = smax;
            parm1.s_grpts    = sgpts;
            parm1.k_min      = kmin;
            parm1.k_max      = kmax;
            parm1.k_grpts    = kgpts;
            parm1.gridreps   = grreps;
            parm1.cust_grid  = cgrid;
            parm1.cgrid_name = "";
            parm1.varyvbar   = US_Util::bool_flag( vvflag );
            parm1.have_custg = ( ! cgrid.isEmpty()  &&
                                          cgrid.length() != 36 );

            parms << parm1;
            nchan++;
         }
         else if ( ename == "job_2dsa" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
            job1run        = US_Util::bool_flag( attr.value( "run" ).toString() );
            job1nois       = attr.value( "noise" ).toString();
         }
         else if ( ename == "job_2dsa_fm" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
            job1run        = US_Util::bool_flag( attr.value( "run" ).toString() );
            job2nois       = attr.value( "noise" ).toString();
            fitrng         = attr.value( "fit_range" ).toString().toDouble();
            grpoints       = attr.value( "grid_points" ).toString().toInt();
         }
         else if ( ename == "job_fitmen" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
            job3run        = US_Util::bool_flag( attr.value( "run" ).toString() );
            job3auto       = attr.value( "interactive" ).toString().toInt() == 0;
         }
         else if ( ename == "job_2dsa_it" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
            job4run        = US_Util::bool_flag( attr.value( "run" ).toString() );
            job4nois       = attr.value( "noise" ).toString();
            rfiters        = attr.value( "max_iterations" ).toString().toInt();
         }
         else if ( ename == "job_2dsa_mc" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
            job5run        = US_Util::bool_flag( attr.value( "run" ).toString() );
            mciters        = attr.value( "mc_iterations" ).toString().toInt();
         }
      }

      bool was_end    = xmli.isEndElement();  // Just read was End of element?
      xmli.readNext();                        // Read the next element

      if ( was_end  &&  ename == "2dsa" )     // Break after "</2dsa>"
         break;
   }

   return ( ! xmli.hasError() );
}

// Write the current 2DSA portion of controls to an XML stream
bool US_AnaProfile::AnaProf2DSA::toXml( QXmlStreamWriter& xmlo )
{
   xmlo.writeStartElement( "2dsa" );
  
   for ( int ii = 0; ii < nchan; ii++ )
   {
      xmlo.writeStartElement( "channel_parms" );
      xmlo.writeAttribute   ( "channel",            parms[ ii ].channel );
      xmlo.writeAttribute   ( "s_min",              QString::number(
                                                    parms[ ii ].s_min ) );
      xmlo.writeAttribute   ( "s_max",              QString::number(
                                                    parms[ ii ].s_max ) );
      xmlo.writeAttribute   ( "s_gridpoints",       QString::number(
                                                    parms[ ii ].s_grpts ) );
      xmlo.writeAttribute   ( "k_min",              QString::number(
                                                    parms[ ii ].k_min ) );
      xmlo.writeAttribute   ( "k_max",              QString::number(
                                                    parms[ ii ].k_max ) );
      xmlo.writeAttribute   ( "k_gridpoints",       QString::number(
                                                    parms[ ii ].k_grpts ) );
      xmlo.writeAttribute   ( "vary_vbar",          US_Util::bool_string(
                                                    parms[ ii ].varyvbar ) );
      xmlo.writeAttribute   ( "constant_ff0",       QString::number(
                                                    parms[ ii ].ff0_const ) );
      xmlo.writeAttribute   ( "custom_grid_guid",   parms[ ii ].cust_grid );

      xmlo.writeEndElement  (); // channel_parms
   }
   xmlo.writeStartElement( "job_2dsa" );
   xmlo.writeAttribute   ( "run",            US_Util::bool_string(
                                             job1run ) );
   xmlo.writeAttribute   ( "noise",          job1nois );
   xmlo.writeEndElement  (); // job_2dsa

   xmlo.writeStartElement( "job_2dsa_fm" );
   xmlo.writeAttribute   ( "run",            US_Util::bool_string(
                                             job2run ) );
   xmlo.writeAttribute   ( "noise",          job2nois );
   xmlo.writeAttribute   ( "fit_range",      QString::number(
                                             fitrng ) );
   xmlo.writeAttribute   ( "grid_points",    QString::number(
                                             grpoints ) );
   xmlo.writeEndElement  (); // job_2dsa_fm

   xmlo.writeStartElement( "job_fitmen" );
   xmlo.writeAttribute   ( "run",            US_Util::bool_string(
                                             job3run ) );
   xmlo.writeAttribute   ( "interactive",  ( job2run ? "1" : "0" ) );
   xmlo.writeEndElement  (); // job_fitmen

   xmlo.writeStartElement( "job_2dsa_it" );
   xmlo.writeAttribute   ( "run",            US_Util::bool_string(
                                             job4run ) );
   xmlo.writeAttribute   ( "noise",          job2nois );
   xmlo.writeAttribute   ( "max_iterations", QString::number(
                                             rfiters ) );
   xmlo.writeEndElement  (); // job_2dsa_it

   xmlo.writeStartElement( "job_2dsa_mc" );
   xmlo.writeAttribute   ( "run",            US_Util::bool_string(
                                             job5run ) );
   xmlo.writeAttribute   ( "mc_iterations",  QString::number(
                                             mciters ) );
   xmlo.writeEndElement  (); // job_2dsa_mc
//3-------------------------------------------------------------------------->80

   xmlo.writeEndElement(); // 2dsa
/*
        <2dsa>
            <channel_parms channel="1A" s_min="1" s_max="10" s_gridpoints="64"
                k_min="1" k_max="4" k_gridpoints="64"
                vary_vbar="false" constant_ff0="2" grid_repetitions="8"
                custom_grid_guid="" />
            <channel_parms channel="1B" s_min="1" s_max="10" s_gridpoints="64"
                k_min="1" k_max="4" k_gridpoints="64"
                vary_vbar="false" constant_ff0="2" grid_repetitions="8"
                custom_grid_guid="" />
            <job_2dsa run="1" noise="TI" />
            <job_2dsa_fm run="1" noise="both" fit_range="0.03" grid_points="10" />
            <job_fitmen run="1" interactive="true" />
            <job_2dsa_it run="1" noise="both" max_iterations="3" />
            <job_2dsa_mc run="1" mc_iterations="100" />
        </2dsa>
 */

   return ( ! xmlo.hasError() );
}

// AnaProf2DSA::Parm2DSA subclass constructor
US_AnaProfile::AnaProf2DSA::Parm2DSA::Parm2DSA()
{
   s_min      = 1.0;
   s_max      = 10.0;
   k_min      = 1.0;
   k_max      = 5.0;
   ff0_const  = 0.72;
   s_grpts    = 64;
   k_grpts    = 64;
   gridreps   = 8;
   varyvbar   = false;
   have_custg = false;
   channel    = "1A";
   cust_grid  = "";
   cgrid_name = "";
}

// AnaProf2DSA::Parm2DSA subclass Equality operator
bool US_AnaProfile::AnaProf2DSA::Parm2DSA::operator==
                 ( const Parm2DSA& p ) const
{
   bool same      = true;
   same           = ( channel != p.channel ) ? false : same;

   return same;
}

// AnaProfPCSA subclass constructor
US_AnaProfile::AnaProfPCSA::AnaProfPCSA()
{
   nchan                = 0;
   parms.clear();
}

// AnaProfPCSA subclass Equality operator
bool US_AnaProfile::AnaProfPCSA::operator== 
                  ( const AnaProfPCSA& ap ) const
{
   if ( nchan  != ap.nchan ) return false;

   return true;
}

// Read all current Cells controls from an XML stream
bool US_AnaProfile::AnaProfPCSA::fromXml( QXmlStreamReader& xmli )
{
   parms.clear();
   nchan           = 0;
   ParmPCSA parm1;

   while( ! xmli.atEnd() )
   {
      QString ename   = xmli.name().toString();

      if ( xmli.isStartElement() )
      {

         if ( ename == "channel_parms" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
            QString chan   = attr.value( "channel" ).toString();
            QString ctype  = attr.value( "curve_type" ).toString();
            QString xtype  = attr.value( "x_type" ).toString();
            QString ytype  = attr.value( "y_type" ).toString();
            QString ztype  = attr.value( "z_type" ).toString();
            QString ntype  = attr.value( "noise" ).toString();
            QString ttype  = attr.value( "regularization" ).toString();
            QString jobrun = attr.value( "job_run" ).toString();
            double xmin    = attr.value( "x_min" ).toString().toDouble();
            double xmax    = attr.value( "x_max" ).toString().toDouble();
            double ymin    = attr.value( "y_min" ).toString().toDouble();
            double ymax    = attr.value( "y_max" ).toString().toDouble();
            double zval    = attr.value( "z_value" ).toString().toDouble();
            double alph    = attr.value( "reg_alpha" ).toString().toDouble();
            int varcnt     = attr.value( "variations_count" ).toString().toInt();
            int griter     = attr.value( "gridfit_iterations" ).toString().toInt();
            int respts     = attr.value( "curve_reso_points" ).toString().toInt();
            int mciter     = attr.value( "mc_iterations" ).toString().toInt();
            int noisflg    = 0;
            noisflg        = ( ntype == "ti"   ) ? 1 : noisflg;
            noisflg        = ( ntype == "ri"   ) ? 2 : noisflg;
            noisflg        = ( ntype == "both" ) ? 3 : noisflg;
            int tregflg    = 0;
            tregflg        = ( ttype == "spec" ) ? 1 : tregflg;
            tregflg        = ( ttype == "auto" ) ? 2 : tregflg;
            bool jobrunf   = US_Util::bool_flag( jobrun );

            parm1.channel    = chan;
            parm1.curv_type  = ctype;
            parm1.x_type     = xtype;
            parm1.y_type     = ytype;
            parm1.z_type     = ztype;
            parm1.treg_type  = ttype;
            parm1.noise_type = ntype;
            parm1.x_min      = xmin;
            parm1.x_max      = xmax;
            parm1.y_min      = ymin;
            parm1.y_max      = ymax;
            parm1.z_value    = zval;
            parm1.tr_alpha   = alph;
            parm1.varcount   = varcnt;
            parm1.grf_iters  = griter;
            parm1.creso_pts  = respts;

            parm1.noise_flag = noisflg;
            parm1.treg_flag  = tregflg;
            parm1.mc_iters   = mciter;
            parm1.job_run    = jobrunf;

            parms << parm1;
            nchan++;
         }
      }

      bool was_end    = xmli.isEndElement();  // Just read was End of element?
      xmli.readNext();                        // Read the next element

      if ( was_end  &&  ename == "pcsa" )     // Break after "</pcsa>"
         break;
   }

   return ( ! xmli.hasError() );
}

// Write the current PCSA portion of controls to an XML stream
bool US_AnaProfile::AnaProfPCSA::toXml( QXmlStreamWriter& xmlo )
{
   xmlo.writeStartElement( "pcsa" );
  
   for ( int ii = 0; ii < nchan; ii++ )
   {
      xmlo.writeStartElement( "channel_parms" );
      xmlo.writeAttribute   ( "channel",            parms[ ii ].channel );
      xmlo.writeAttribute   ( "curve_type",         parms[ ii ].curv_type );
      xmlo.writeAttribute   ( "x_type",             parms[ ii ].x_type );
      xmlo.writeAttribute   ( "y_type",             parms[ ii ].y_type );
      xmlo.writeAttribute   ( "z_type",             parms[ ii ].z_type );
      xmlo.writeAttribute   ( "x_min",              QString::number(
                                                    parms[ ii ].x_min ) );
      xmlo.writeAttribute   ( "x_max",              QString::number(
                                                    parms[ ii ].x_max ) );
      xmlo.writeAttribute   ( "y_min",              QString::number(
                                                    parms[ ii ].y_min ) );
      xmlo.writeAttribute   ( "y_max",              QString::number(
                                                    parms[ ii ].y_max ) );
      xmlo.writeAttribute   ( "z_value",            QString::number(
                                                    parms[ ii ].z_value ) );
      xmlo.writeAttribute   ( "variations_count",   QString::number(
                                                    parms[ ii ].varcount ) );
      xmlo.writeAttribute   ( "gridfit_iterations", QString::number(
                                                    parms[ ii ].grf_iters ) );
      xmlo.writeAttribute   ( "curve_reso_points",  QString::number(
                                                    parms[ ii ].creso_pts ) );
      xmlo.writeAttribute   ( "noise",              parms[ ii ].noise_type );
      xmlo.writeAttribute   ( "regularization",     parms[ ii ].treg_type );
      xmlo.writeAttribute   ( "reg_alpha",          QString::number(
                                                    parms[ ii ].tr_alpha ) );
      xmlo.writeAttribute   ( "mc_iterations",      QString::number(
                                                    parms[ ii ].mc_iters ) );
      xmlo.writeAttribute   ( "job_run",            US_Util::bool_string(
                                                    parms[ ii ].job_run ) );
      xmlo.writeEndElement(); // channel_parms
   }
//3-------------------------------------------------------------------------->80

   xmlo.writeEndElement(); // pcsa

   return ( ! xmlo.hasError() );
}

// AnaProfPCSA::ParmPCSA subclass constructor
US_AnaProfile::AnaProfPCSA::ParmPCSA::ParmPCSA()
{
   x_min      = 1.0;
   x_max      = 10.0;
   y_min      = 1.0;
   y_max      = 4.0;
   z_value    = 0.72;
   tr_alpha   = 0.0;
   varcount   = 3;
   grf_iters  = 6;
   creso_pts  = 100;
   noise_flag = 0;
   treg_flag  = 0;
   mc_iters   = 0;
   job_run    = false;
   channel    = "1A";
   curv_type  = "All";
   x_type     = "s";
   y_type     = "f/f0";
   z_type     = "vbar";
   noise_type = "none";
   treg_type  = "none";
}

// AnaProfPCSA::ParmPCSA subclass Equality operator
bool US_AnaProfile::AnaProfPCSA::ParmPCSA::operator==
                 ( const ParmPCSA& p ) const
{
   bool same      = true;
   same           = ( channel != p.channel ) ? false : same;

   return same;
}


