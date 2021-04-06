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

   analysis_run << 1;

   wvl_edit << 280;

   wvl_not_run << "";
   
   QList< double > wvl = { 280 };
   ch_wvls[ chndescs[0] ] = wvl;
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

   /** //OLD -- incorrect
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
     //ALEXEY
     xmlo.writeAttribute    ( "run",
   			QString::number( analysis_run[ ii ] ) );
     xmlo.writeEndElement();    // channel_parms
    
   }
   */

   //DEBUG
   qDebug() << "Size: pchans() -- " << pchans.count();
   qDebug() << "Sizes of lc_ratios, lc_tolers, l_volumes, lv_tolers, data_ends, analysis_run -- "
	    << lc_ratios.count() << lc_tolers.count() << l_volumes.count() <<  lv_tolers.count() <<  data_ends.count() <<  analysis_run.count();

   // for (int i=0; i<data_ends.count(); i++)
   //   {
   //     qDebug() << "lc_ratios: " << i << lc_ratios[ i ];
   //     qDebug() << "lc_tolers: " << i << lc_tolers[ i ];
   //     qDebug() << "lc_volumes: " << i << l_volumes[ i ];
   //     qDebug() << "lv_tolers: " << i << lv_tolers[ i ];
   //     qDebug() << "data_ends: " << i << data_ends[ i ];
   //     qDebug() << "analysis_run: " << i << analysis_run[ i ];
   //   }
   //END of DEBUG

   int kk = 0;
   for ( int ii = 0; ii < pchans.count(); ii++ )
   {
     //ALEXEY: do not write Interference B
     if (  chndescs[ii].contains( "B:Interf" ) )
       continue;
       
     qDebug() << "WRITE: Ch description: " << chndescs[ ii ];

     qDebug() << "Index of affected PARMS: " << kk;
     xmlo.writeStartElement ( "channel_parms" );
     xmlo.writeAttribute    ( "channel",  pchans  [ ii ] );
     xmlo.writeAttribute    ( "chandesc", chndescs[ ii ] );

     //Affected parms
     xmlo.writeAttribute    ( "load_concen_ratio",
   			QString::number( lc_ratios[ kk ] ) );
     xmlo.writeAttribute    ( "lcr_tolerance",
   			QString::number( lc_tolers[ kk ] ) );
     xmlo.writeAttribute    ( "load_volume",
   			QString::number( l_volumes[ kk ] ) );
     xmlo.writeAttribute    ( "lv_tolerance",
   			QString::number( lv_tolers[ kk ] ) );
     xmlo.writeAttribute    ( "data_end",
   			QString::number( data_ends[ kk ] ) );
     //ALEXEY
     xmlo.writeAttribute    ( "run",
   			QString::number( analysis_run[ kk ] ) );
     
     //ALEXEY: wvl to edit
     xmlo.writeAttribute    ( "wvl_edit",
   			QString::number( wvl_edit[ kk ] ) );
     
     //ALEXEY: wvl not to be analyzed
     xmlo.writeAttribute    ( "wvl_not_run", wvl_not_run[ kk ]  );

     
     xmlo.writeEndElement();
     // channel_parms

     kk++;

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

   analysis_run.clear();
   wvl_edit    .clear();
   wvl_not_run .clear();
   //ch_wvls     .clear();
   
   while( ! xmli.atEnd() )
   {
      xmli.readNext();
//qDebug() << "AP:fX: next hasErr Err" << xmli.hasError() << xmli.errorString()
// << xmli.tokenString();

      if ( xmli.isStartElement() )
      {
         QString ename   = xmli.name().toString();
//qDebug() << "AP:fX: ename" << ename;

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
	    //ALEXEY: for now -- put all checked; later will be 'run="1"' OR 'run="0"' field
	    //analysis_run << 1;
	    analysis_run << attr.value( "run" ).toString().toInt();
	    wvl_edit     << attr.value( "wvl_edit" ).toString().toInt();
	    wvl_not_run  << attr.value( "wvl_not_run" ).toString();
//3-------------------------------------------------------------------------->80
            chx++;
//qDebug() << "AP:fX:  chx" << chx << pchans.count();
         }

         else if ( ename == "p_2dsa" )      { ap2DSA.fromXml( xmli ); }
         else if ( ename == "p_pcsa" )      { apPCSA.fromXml( xmli ); }
      }
//else
 //qDebug() << "AP:fX: non-start ename" << xmli.name().toString();
   }

   return ( ! xmli.hasError() );
}


// AnaProf2DSA subclass constructor
US_AnaProfile::AnaProf2DSA::AnaProf2DSA()
{
   fitrng      = 0.03;
   nchan       = 1;
   grpoints    = 11;
   rfiters     = 5;
   mciters     = 100;
   job1run     = true;
   job2run     = true;
   job3run     = true;
   job4run     = true;
   job5run     = true;
   job3auto    = true;
   job1nois    = "ti";
   job2nois    = "both";
   job4nois    = "both";

   fmb         = "fm";

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
//qDebug() << "AP:2fX: IN" << xmli.name().toString();

   while( ! xmli.atEnd() )
   {
      QString ename   = xmli.name().toString();
//qDebug() << "AP:2fX: ename" << ename;

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
                                 cgrid.length() == 36 );

            parms << parm1;
            nchan++;
//qDebug() << "AP:2fX:  nchan" << nchan << parms.count();
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
	    QStringRef attr_ref;
	    
            //job1run        = US_Util::bool_flag( attr.value( "run" ).toString() );
	    job2run        = US_Util::bool_flag( attr.value( "run" ).toString() );      // ALEXEY; shouldn't it be job2run ?
            job2nois       = attr.value( "noise" ).toString();

	    //fitrng         = attr.value( "fit_range" ).toString().toDouble();
	    //grpoints       = attr.value( "grid_points" ).toString().toInt();

	    //ALEXEY -- check if the following 2 attributes exist (for older ana_profiles):
	    attr_ref       = attr.value("meniscus_range");
	    if ( !attr_ref.isNull() )
	      {
		fitrng         = attr.value( "meniscus_range" ).toString().toDouble();
		if ( fitrng == 0 )
		  fitrng = 0.03; //DEFAULT if 0 stored in xml (due old ana_profiles)
	      }
	    else
	      fitrng         = 0.03; //DEFAULT if no attribute found

	    attr_ref       = attr.value("meniscus_points");
	    if ( !attr_ref.isNull() )
	      {
		grpoints       = attr.value( "meniscus_points" ).toString().toInt();
		if ( !grpoints )
		  grpoints = 11; //DEFAULT if 0 stored in xml (due old ana_profiles)
	      }
	    else
	      grpoints       = 11; //DEFAULT if no attribute found
	    
	    //fit m|b
	    int fmb_n      = attr.value( "fit_mb_select" ).toString().toInt();
	    if ( fmb_n == 1 )
	      fmb = "fm";
	    if ( fmb_n == 2 )
	      fmb = "fb";
	    if ( fmb_n == 3 )
	      fmb = "fmb";
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

      if ( was_end  &&  ename == "p_2dsa" )   // Break after "</p_2dsa>"
         break;
   }

   return ( ! xmli.hasError() );
}

// Write the current 2DSA portion of controls to an XML stream
bool US_AnaProfile::AnaProf2DSA::toXml( QXmlStreamWriter& xmlo )
{
   xmlo.writeStartElement( "p_2dsa" );
   nchan           = parms.count();
  
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

   //<job_2dsa_fm run="1" noise="(TI+RI Noise)" fit_mb_select="1" meniscus_range="0.03" meniscus_points="11"/> 
   xmlo.writeStartElement( "job_2dsa_fm" );
   xmlo.writeAttribute   ( "run",            US_Util::bool_string(
                                             job2run ) );
   xmlo.writeAttribute   ( "noise",          job2nois );

   //ALEXEY: not used in autoflow
   //xmlo.writeAttribute   ( "fit_range",      QString::number(
   //                                          fitrng ) );

   //xmlo.writeAttribute   ( "grid_points",    QString::number(
   //                                          grpoints ) );
   //-------------------------------
   int fmb_n=1;
   if ( fmb == "fm" )
     fmb_n = 1;
   if ( fmb == "fb" )
     fmb_n = 2;
   if ( fmb == "fmb" )
     fmb_n = 3;
   
   xmlo.writeAttribute   ( "fit_mb_select",    QString::number( fmb_n ) );
   xmlo.writeAttribute   ( "meniscus_range",   QString::number( fitrng ) );
   xmlo.writeAttribute   ( "meniscus_points",  QString::number( grpoints ) );

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

   xmlo.writeEndElement(); // p_2dsa
/*
        <p_2dsa>
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
        </p_2dsa>
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
   nchan          = 0;
   job_run        = true;
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
         if ( ename == "p_pcsa" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
            QString jobrun = attr.value( "job_run" ).toString();
            job_run        = US_Util::bool_flag( jobrun );
         }

         else if ( ename == "channel_parms" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
            QString chan   = attr.value( "channel" ).toString();
            QString ctype  = attr.value( "curve_type" ).toString();
            QString xtype  = attr.value( "x_type" ).toString();
            QString ytype  = attr.value( "y_type" ).toString();
            QString ztype  = attr.value( "z_type" ).toString();
            QString ntype  = attr.value( "noise" ).toString();
            QString ttype  = attr.value( "regularization" ).toString();
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

            parms << parm1;
            nchan++;
//qDebug() << "AP:pfX: nchan" << nchan << parms.count();
         }
      }

      bool was_end    = xmli.isEndElement();  // Just read was End of element?
      xmli.readNext();                        // Read the next element

      if ( was_end  &&  ename == "p_pcsa" )   // Break after "</pcsa>"
         break;
   }

   return ( ! xmli.hasError() );
}

// Write the current PCSA portion of controls to an XML stream
bool US_AnaProfile::AnaProfPCSA::toXml( QXmlStreamWriter& xmlo )
{
   xmlo.writeStartElement( "p_pcsa" );
   xmlo.writeAttribute   ( "job_run", US_Util::bool_string( job_run ) );
   nchan           = parms.count();
  
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
      xmlo.writeEndElement(); // channel_parms
   }
//3-------------------------------------------------------------------------->80

   xmlo.writeEndElement(); // p_pcsa

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


