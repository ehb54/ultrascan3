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
   chndescs_alt  << "1A:UV/vis.:(unspecified)";
   lc_ratios << 1.0;
   lc_tolers << 5.0;
   l_volumes << 460.0;
   lv_tolers << 10.0;
   data_ends << 7.0;

   //abde
   ld_dens_0s << 1.42;
   gm_vbars   << 0.2661;
   gm_mws     << 168.36;
   ref_channels << 0;
   ref_use_channels << 0;

   analysis_run << 1;
   report_run   << 1;

   scan_excl_begin << 0;
   scan_excl_end   << 0;

   replicates  << 0;

   replicates_to_channdesc_main.clear();
   channdesc_to_overlapping_wvls_main.clear();
   
   wvl_edit << 180;
      
   wvl_not_run << "";

   //QList< double > wvl = { 280 };
   QList< double > wvl = { 180 };
   ch_wvls[ chndescs[0] ] = wvl;

   //Reports
   //US_ReportGMP report; 
   US_ReportGMP report = US_ReportGMP();
   //ch_reports[ chndescs_alt[0] ] = report;


   //ALEXEY_NEW_REPORT:
   ch_reports[ chndescs_alt[0] ] [ QString::number( wvl[ 0 ] ) ] = report;

   //default JSON for report profile: general settings 
   report_mask =
   "{"
      "\"A. General Settings\":        \"1\","
      "\"B. Lab/Rotor Parameters\":    \"1\","
      "\"C. Optima Machine Operator\": \"1\","
      "\"D. Speed Parameters\":        \"1\","
      "\"E. Cell Centerpiece Usage\":  \"1\","
      "\"F. Solutions for Channels\":"
               "[{"
                  "\"Solution Information\":  \"1\","
                  "\"Analyte Information\":   \"1\","
                  "\"Buffer Information\":    \"1\""
               "}],"
      "\"G. Optical Systems\":         \"1\","
      "\"H. Ranges\":                  \"1\","
      "\"I. Scan Counts and Intervals for Optics\": \"1\","
      "\"J. Analysis Profile\":"
               "[{"
                  "\"General Settings\":"
                          "{"
                             "\"Channel General Settings\":            \"1\","
                             "\"Report Parameters (per-triple)\":      \"1\","
                             "\"Report Item Parameters (per-triple)\": \"1\""
                           "},"
                   "\"2DSA Controls\":" 
                          "{"
                             "\"Job Flow Summary\":     \"1\","
                             "\"Per-Channel Profiles\": \"1\""
                          "},"
                   "\"PCSA Controls\":"
                          "{"
                              "\"Job Flow Summary\":     \"1\","
                              "\"Per-Channel Profiles\": \"1\""
                          "}"
  
               "}]"
   "}"
     ;

   //default JSON for combined plots global settings:
   combPlots_parms =
   "{"
      "\"s,2DSA-IT\":"
               "[{"
                  "\"Envelope Gaussian Sigma\":  \"0.01\","
                  "\"Plot X Minimum\":   \"1\","
                  "\"Plot X Maximum\":    \"10\""
                "}],"
      "\"s,2DSA-MC\":"
               "[{"
                  "\"Envelope Gaussian Sigma\":  \"0.01\","
                  "\"Plot X Minimum\":   \"1\","
                  "\"Plot X Maximum\":    \"10\""
                "}],"
      "\"s,PCSA\":"
               "[{"
                  "\"Envelope Gaussian Sigma\":  \"0.01\","
                  "\"Plot X Minimum\":   \"1\","
                  "\"Plot X Maximum\":    \"10\""
                "}],"
      "\"D,2DSA-IT\":"
               "[{"
                  "\"Envelope Gaussian Sigma\":  \"0.01\","
                  "\"Plot X Minimum\":   \"0\","
                  "\"Plot X Maximum\":    \"0.000002\""
                "}],"
      "\"D,2DSA-MC\":"
               "[{"
                  "\"Envelope Gaussian Sigma\":  \"0.01\","
                  "\"Plot X Minimum\":   \"0\","
                  "\"Plot X Maximum\":    \"0.000002\""
                "}],"
      "\"D,PCSA\":"
               "[{"
                  "\"Envelope Gaussian Sigma\":  \"0.01\","
                  "\"Plot X Minimum\":   \"0\","
                  "\"Plot X Maximum\":    \"0.000002\""
                "}],"
      "\"f/f0,2DSA-IT\":"
               "[{"
                  "\"Envelope Gaussian Sigma\":  \"0.01\","
                  "\"Plot X Minimum\":   \"1\","
                  "\"Plot X Maximum\":    \"4\""
                "}],"
      "\"f/f0,2DSA-MC\":"
               "[{"
                  "\"Envelope Gaussian Sigma\":  \"0.01\","
                  "\"Plot X Minimum\":   \"1\","
                  "\"Plot X Maximum\":    \"4\""
                "}],"
      "\"f/f0,PCSA\":"
               "[{"
                  "\"Envelope Gaussian Sigma\":  \"0.01\","
                  "\"Plot X Minimum\":   \"1\","
                  "\"Plot X Maximum\":    \"4\""
                "}],"
      "\"MW,2DSA-IT\":"
               "[{"
                  "\"Envelope Gaussian Sigma\":  \"0.01\","
                  "\"Plot X Minimum\":   \"0\","
                  "\"Plot X Maximum\":    \"100000\""
                "}],"
      "\"MW,2DSA-MC\":"
               "[{"
                  "\"Envelope Gaussian Sigma\":  \"0.01\","
                  "\"Plot X Minimum\":   \"1\","
                  "\"Plot X Maximum\":    \"100000\""
                "}],"
      "\"MW,PCSA\":"
               "[{"
                  "\"Envelope Gaussian Sigma\":  \"0.01\","
                  "\"Plot X Minimum\":   \"1\","
                  "\"Plot X Maximum\":    \"100000\""
                "}],"
      "\"Radius,raw\":"
               "[{"
                  "\"Envelope Gaussian Sigma\":  \"0.01\","
                  "\"Plot X Minimum\":   \"5.8\","
                  "\"Plot X Maximum\":    \"7.2\""
                "}]"     
   "}" 
     ;
   
}

// AnaProfile Equality operator
bool US_AnaProfile::operator== ( const US_AnaProfile& ap ) const
{
  qDebug() << "RP:AnaProfile COMPARISON!! ";

  //General
  if ( aprofname    != ap.aprofname    )  return false;
  if ( aprofGUID    != ap.aprofGUID    )  return false;
  if ( protoname    != ap.protoname    )  return false;
  if ( protoGUID    != ap.protoGUID    )  return false;

  qDebug() << "[ANA comp] Gen1 OK";

  qDebug() << "nchan: " << nchan << ap.nchan;
  //if ( nchan        != ap.nchan        )  return false;
							 
  qDebug() << "[ANA comp] Gen2 OK";
  
  if ( lc_ratios    != ap.lc_ratios    )  return false;
  if ( lc_tolers    != ap.lc_tolers    )  return false;
  if ( l_volumes    != ap.l_volumes    )  return false;
  if ( lv_tolers    != ap.lv_tolers    )  return false;
  if ( data_ends    != ap.data_ends    )  return false;

  qDebug() << "[ANA comp] Gen3 OK";

  //abde
  if ( ld_dens_0s   != ap.ld_dens_0s )  return false;
  if ( gm_vbars     != ap.gm_vbars   )  return false;
  if ( gm_mws       != ap.gm_mws     )  return false;
  if ( ref_channels != ap.ref_channels  )  return false;
  if ( ref_use_channels != ap.ref_use_channels  )  return false;

  if ( scan_excl_begin  != ap.scan_excl_begin  )  return false;
  if ( scan_excl_end    != ap.scan_excl_end    )  return false;
  
  if ( analysis_run    != ap.analysis_run  )  return false;
  if ( report_run      != ap.report_run    )  return false;
  if ( wvl_edit        != ap.wvl_edit      )  return false;
  if ( replicates      != ap.replicates    )  return false;
  if ( wvl_not_run     != ap.wvl_not_run   )  return false;
  
  if ( report_mask     != ap.report_mask   )  return false;
  if ( combPlots_parms != ap.combPlots_parms ) return false;
  
  //2DSA controls
  if ( ap2DSA       != ap.ap2DSA  )  return false;
  //PCSA controls
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

   qDebug() << "Size: pchans(), chndescs.size(),  scan_excl_begin.size(), scan_excl_end.size()  -- "
	    << pchans.count()
	    << chndescs.size()
	    << scan_excl_begin.size()
	    << scan_excl_end.size()
     ;
   
   for ( int ii = 0; ii < pchans.count(); ii++ )
     {
        qDebug() << "In toXML AProfile: Ch_desc, scan_beg, scan_end -- "
		 << chndescs[ ii ]
		 << scan_excl_begin[ ii ]
		 << scan_excl_end[ ii ];
     }
   //END of DEBUG

   int kk = 0;
   for ( int ii = 0; ii < pchans.count(); ii++ )
   {
     //ALEXEY: do not write Interference B
     if (  chndescs[ii].contains( "B:Interf" ) )
       continue;
       
     qDebug() << "WRITE: Ch description vs. chndescs_alt: " << chndescs[ ii ] << " vs. " << chndescs_alt[ kk ];

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

     
     //ABDE parms
     xmlo.writeAttribute    ( "load_dens",
   			QString::number( ld_dens_0s[ kk ] ) );
     xmlo.writeAttribute    ( "grad_vbar",
   			QString::number( gm_vbars[ kk ] ) );
     xmlo.writeAttribute    ( "grad_mw",
   			QString::number( gm_mws[ kk ] ) );
     
     
     //ALEXEY: analyse?
     xmlo.writeAttribute    ( "run",
   			QString::number( analysis_run[ kk ] ) );

     //ALEXEY: run report ?
     xmlo.writeAttribute    ( "run_report",
   			QString::number( report_run[ kk ] ) );

     //ABDE: reference, use_ref.
     xmlo.writeAttribute    ( "abde_reference",
   			QString::number( ref_channels[ kk ] ) );
     xmlo.writeAttribute    ( "abde_use_reference",
   			QString::number( ref_use_channels[ kk ] ) );

     
     //ALEXEY: wvl to edit
     xmlo.writeAttribute    ( "wvl_edit",
   			QString::number( wvl_edit[ kk ] ) );
     
     //ALEXEY: wvl not to be analyzed
     xmlo.writeAttribute    ( "wvl_not_run", wvl_not_run[ kk ]  );

     
     //ALEXEY: Id or guid of the channel's report
     xmlo.writeAttribute    ( "chandesc_alt", chndescs_alt[ kk ]  );

     //ALEXEY_NEW_REPORT:
     // iterate over all channel's reportIDs [i]:
     // "report_id" && "report_guid" will now be strings like "100,101,102,.."
     // ch_report_ids[ chndescs_alt[ kk ] ]   [ i ], where i is for QList < int >, a list of reportIDs...
     // ch_report_guids[ chndescs_alt[ kk ] ] [ i ]

     QList < int > reportIDs        =  ch_report_ids[ chndescs_alt[ kk ] ];
     QStringList   reportIDs_list;
     QStringList   reportGUIDs_list =  ch_report_guids[ chndescs_alt[ kk ] ];

     for ( int ri = 0; ri < reportIDs.size(); ++ri )
       reportIDs_list   << QString::number( reportIDs[ ri ] );

     QString reportIDs_string   = reportIDs_list.join(",");
     QString reportGUIDs_string = reportGUIDs_list.join(",");
     
     xmlo.writeAttribute    ( "report_id",   reportIDs_string   );
     xmlo.writeAttribute    ( "report_guid", reportGUIDs_string );

     // //scan exclusions: ATTENTION -- use ii index, NOT kk!!!
     // xmlo.writeAttribute    ( "scan_excl_begin", QString::number( scan_excl_begin[ ii ] )    );
     // xmlo.writeAttribute    ( "scan_excl_end",   QString::number( scan_excl_end[ ii ] ) );

     //scan exclusions: ATTENTION -- Now use kk index, as for all other attributes
     xmlo.writeAttribute    ( "scan_excl_begin", QString::number( scan_excl_begin[ kk ] )    );
     xmlo.writeAttribute    ( "scan_excl_end",   QString::number( scan_excl_end[ kk ] ) );

     xmlo.writeAttribute    ( "replicate_group",       QString::number( replicates[ kk ] )    );

     //overlapping wvls:
     //Debug:
     QMap< QString, QStringList >::iterator wvl_overlap;
     for ( wvl_overlap = channdesc_to_overlapping_wvls_main.begin(); wvl_overlap != channdesc_to_overlapping_wvls_main.end(); ++wvl_overlap )
	{
	  qDebug() << "In AProfile's toXML:  channdesc_to_overlapping_wvls_main QMap: " << wvl_overlap.key() << ", list of overlapping wvls: " << wvl_overlap.value();
	}
     qDebug() << "Current channel_alt name: " << chndescs_alt[ kk ];
     //end debug
     
     
     QStringList repl_wvl_overlap = channdesc_to_overlapping_wvls_main [ chndescs_alt[ kk ] ];
     if ( !repl_wvl_overlap.isEmpty() )
       {
	 QString repl_wvl_overlap_str = repl_wvl_overlap.join(","); 
	 xmlo.writeAttribute    ( "replicate_wvl_overlap", repl_wvl_overlap_str  );
       }
     
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

   //abde
   ld_dens_0s. clear();
   gm_vbars.   clear();
   gm_mws .    clear();
   ref_channels. clear();
   ref_use_channels. clear();

   analysis_run.clear();
   report_run  .clear();
   wvl_edit    .clear();
   wvl_not_run .clear();

   chndescs_alt   .clear();
   ch_report_ids  .clear();
   ch_report_guids.clear();

   scan_excl_begin.clear(); 
   scan_excl_end.  clear();

   replicates. clear();
   replicates_to_channdesc_main. clear();
   channdesc_to_overlapping_wvls_main. clear();
   
   
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

	    //abde: for backward compatibility
	    if ( attr.hasAttribute("load_dens") ) 
	      ld_dens_0s << attr.value( "load_dens" ).toString().toDouble();
	    else
	      ld_dens_0s << 1.42;
	    if ( attr.hasAttribute("grad_vbar") ) 
	      gm_vbars   << attr.value( "grad_vbar" ).toString().toDouble();
	    else
	      gm_vbars << 0.2661;
	    if ( attr.hasAttribute("grad_mw") ) 
	      gm_mws     << attr.value( "grad_mw" ).toString().toDouble();
	    else
	      gm_mws  <<  168.36;

	    //ALEXEY: for now -- put all checked; later will be 'run="1"' OR 'run="0"' field
	    //analysis_run << 1;
	    analysis_run << attr.value( "run" ).toString().toInt();

	    if ( attr.hasAttribute("run_report") ) 
	      report_run   << attr.value( "run_report" ).toString().toInt();
	    else  // This will not be needed for newer analysis profiles; logic is for backward compatibility..
	      {
		if ( attr.value( "run" ).toString().toInt() ) 
		  report_run   << 1;
		else
		  report_run   << 0;
	      }

	    //abde: for backward compatibility
	    if ( attr.hasAttribute("abde_reference") )
	      ref_channels << attr.value( "abde_reference" )  .toString().toInt();
	    else
	      ref_channels << 0;
	      
	    if ( attr.hasAttribute("abde_use_reference") )
	      ref_use_channels << attr.value( "abde_use_reference" )  .toString().toInt();
	    else
	      ref_use_channels << 0;
	    
	    wvl_edit     << attr.value( "wvl_edit" ).toString().toInt();
	    wvl_not_run  << attr.value( "wvl_not_run" ).toString();

	    QString channel_alt_desc = attr.value( "chandesc_alt" ).toString();
	    chndescs_alt  << channel_alt_desc;

	    //ALEXEY_NEW_REPORT:
	    // attr.value( "report_id" ).toString() will be something like "101,102,103,.."
	    // transform this string into QList < int >
	    // insert this QList into ch_report_ids[ channel_alt_desc ]

	    QString reportIDs_string   = attr.value( "report_id" ).toString();
	    QString reportGIUDs_string = attr.value( "report_guid" ).toString();

	    QStringList reportIDs_list   = reportIDs_string.split(",");
	    QStringList reportGIUDs_list = reportGIUDs_string.split(",");

	    QList < int > reportIDs_qlist;
	    for( int ri=0; ri<reportIDs_list.size(); ++ri )
	      reportIDs_qlist.push_back ( reportIDs_list[ ri ].toInt() );
	    
	    ch_report_ids[ channel_alt_desc ]   = reportIDs_qlist;
	    ch_report_guids[ channel_alt_desc ] = reportGIUDs_list;

	    if ( attr.hasAttribute("scan_excl_begin") ) 
	      scan_excl_begin <<  attr.value( "scan_excl_begin" ).toString().toInt();
	    else
	      scan_excl_begin << 0;
	    if ( attr.hasAttribute("scan_excl_end") )
	      scan_excl_end   <<  attr.value( "scan_excl_end" )  .toString().toInt();
	    else
	      scan_excl_end   << 0;

	    if (  attr.hasAttribute ("replicate_group") )
	      replicates << attr.value( "replicate_group" )  .toString().toInt();
	    else
	      replicates << 0;

	    if (  attr.hasAttribute ("replicate_wvl_overlap") )
	      {
		channdesc_to_overlapping_wvls_main[ channel_alt_desc ] = attr.value( "replicate_wvl_overlap" )  .toString().split(",");
		qDebug() << "In AProfile's fromXML: channel_alt_desc:  " << channel_alt_desc
			 << ", replicate_wvl_overlap -- " << attr.value( "replicate_wvl_overlap" )  .toString().split(",");
	      }
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
   j2rfiters   = 1;
   //rfiters     = 5;
   rfiters     = 10;
   mciters     = 100;
   job1run     = true;
   job2run     = true;
   job3run     = true;
   job4run     = true;
   job5run     = true;
   //job3auto    = true;
   job3auto    =false;
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
   //ALEXEY: add the rest of Job flow params to compare!
   if ( fitrng    != ap.fitrng  )   return false;
   if ( grpoints  != ap.grpoints  ) return false;
   if ( j2rfiters != ap.j2rfiters ) return false;   
   if ( rfiters   != ap.rfiters )   return false;  
   if ( mciters   != ap.mciters  )  return false;   
   if ( fmb       != ap.fmb  )      return false;

   if ( job1run      != ap.job1run  ) return false;
   if ( job2run      != ap.job2run  ) return false;
   if ( job3run      != ap.job3run  ) return false;
   if ( job4run      != ap.job4run  ) return false;
   if ( job5run      != ap.job5run  ) return false;
   if ( job3auto     != ap.job3auto  ) return false;

   //& array of 2DSA controls per channel..
   if ( parms     != ap.parms )     return false;
   
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

	    //ALEXEY -- check if the following 3 attributes exist (for older ana_profiles):
	    //Meniscus Range
	    attr_ref       = attr.value("meniscus_range");
	    if ( !attr_ref.isNull() )
	      {
		fitrng         = attr.value( "meniscus_range" ).toString().toDouble();
		if ( fitrng == 0 )
		  fitrng = 0.03; //DEFAULT if 0 stored in xml (due old ana_profiles)
	      }
	    else
	      fitrng         = 0.03; //DEFAULT if no attribute found

	    //Meniscus Points
	    attr_ref       = attr.value("meniscus_points");
	    if ( !attr_ref.isNull() )
	      {
		grpoints       = attr.value( "meniscus_points" ).toString().toInt();
		if ( !grpoints )
		  grpoints = 11; //DEFAULT if 0 stored in xml (due old ana_profiles)
	      }
	    else
	      grpoints       = 11; //DEFAULT if no attribute found

	    //Refinement Iterations
	    attr_ref       = attr.value("max_iterations");
	    if ( !attr_ref.isNull() )
	      {
		j2rfiters       = attr.value( "max_iterations" ).toString().toInt();
		if ( !j2rfiters )
		  j2rfiters = 1; //DEFAULT if 0 stored in xml (due old ana_profiles)
	      }
	    else
	      j2rfiters       = 1; //DEFAULT if no attribute found
	    
	    
	    //fit m|b
	    int fmb_n      = attr.value( "fit_mb_select" ).toString().toInt();
	    if ( fmb_n == 1 )
	      fmb = "fm";
	    if ( fmb_n == 2 )
	      fmb = "fb";
	    if ( fmb_n == 3 )
	      fmb = "fmb";
         }
         else if ( ename == "job_fitmen" || ename == "job_fitmen_auto" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
            job3run        = US_Util::bool_flag( attr.value( "run" ).toString() );
            job3auto       = attr.value( "interactive" ).toString().toInt() == 0;

	    qDebug() << "In 2DSA's fromXml(): job3auto -- " << job3auto;
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
   xmlo.writeAttribute   ( "max_iterations",   QString::number( j2rfiters ) );

   xmlo.writeEndElement  (); // job_2dsa_fm

   //job_fitmen || job_fitmen_auto
   if ( !job3auto ) //manual, so interactive
     {
       xmlo.writeStartElement( "job_fitmen" );
       xmlo.writeAttribute   ( "run",   US_Util::bool_string( job3run ) );
       xmlo.writeAttribute   ( "interactive",  "1" );
       //xmlo.writeAttribute   ( "interactive",  ( job3auto ? "0" : "1" ) );
     }
   else  //auto-pick, so non-interactive
     {
       xmlo.writeStartElement( "job_fitmen_auto" );
       xmlo.writeAttribute   ( "run",   US_Util::bool_string( job3run ) );
       xmlo.writeAttribute   ( "interactive",  "0" );
     }
   xmlo.writeEndElement  (); // job_fitmen || job_fitmen_auto


   
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
   k_max      = 4.0;
   //ff0_const  = 0.72;
   ff0_const  = 1.0;
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
  //bool same      = true;
  //same           = ( channel != p.channel ) ? false : same;
  
  //return same;

  if ( s_min    != p.s_min    ) return false;
  if ( s_max    != p.s_max    ) return false;
  if ( k_min    != p.k_min    ) return false;
  if ( k_max    != p.k_max    ) return false;
  if ( ff0_const  != p.ff0_const    ) return false;
  if ( s_grpts    != p.s_grpts    ) return false;
  if ( k_grpts    != p.k_grpts    ) return false;
  if ( gridreps   != p.gridreps   ) return false;

  if ( varyvbar  != p.varyvbar )  return false;
  
  if ( channel  != p.channel    ) return false;
    
  return true;
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
  qDebug() << "RP:AnaProfPCSA: nchan, ap.nchan -- "     << nchan << ", " << ap.nchan;
  qDebug() << "RP:AnaProfPCSA: job_run, ap.job_run -- " << job_run << ", " << ap.job_run;
  
  if ( nchan   != ap.nchan ) return false;
  if ( job_run != ap.job_run ) return false;
  
  //& PCSA controls per channel
  if ( parms  != ap.parms ) return false;
  
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
	    
	    //ALEXEY: get (modified)  curve type form XML & transform into appropriate internal varialble 
	    //QString ctype  = attr.value( "curve_type" ).toString();
	    QString ctype_t  = attr.value( "curve_type" ).toString();
	    QString ctype = curve_type_fromXml( ctype_t );
	    
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
            // tregflg        = ( ttype == "spec" ) ? 1 : tregflg;
            // tregflg        = ( ttype == "auto" ) ? 2 : tregflg;

	    //ALEXEY: change "regularization" to: {"none","specified_alpha","auto_computed_alpha"}
	    tregflg        = ( ttype == "specified_alpha" )     ? 1 : tregflg;
	    tregflg        = ( ttype == "auto_computed_alpha" ) ? 2 : tregflg;

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

// get curve type from AProfile in a format suitable for internal use
QString US_AnaProfile::AnaProfPCSA::curve_type_fromXml( QString curve_param )
{
  QString curve_type = QString("");

  if ( curve_param.contains( "All") )
    curve_type = "All (IS + DS + SL)";
  if ( curve_param.contains( "IS") )
    curve_type = "Increasing Sigmoid";
  if ( curve_param.contains( "DS") )
    curve_type = "Decreasing Sigmoid";
  if ( curve_param.contains( "SL") )
    curve_type = "Straight Line";
  if ( curve_param.contains( "HL") )
    curve_type = "Horizontal Line";
  if ( curve_param.contains( "2O") )
    curve_type = "Second Order Power";  
    
  return curve_type;
  
}

// get curve type in a format suitable for Analysis submission
QString US_AnaProfile::AnaProfPCSA::curve_type_toXml( QString curve_param )
{
  QString curve_type = QString("");

  if ( curve_param.contains( "All") )
    curve_type = "All";
  if ( curve_param.contains( "Increasing Sigmoid") )
    curve_type = "IS";
  if ( curve_param.contains( "Decreasing Sigmoid") )
    curve_type = "DS";
  if ( curve_param.contains( "Straight Line") )
    curve_type = "SL";
  if ( curve_param.contains( "Horizontal Line") )
    curve_type = "HL";
  if ( curve_param.contains( "Second Order Power") )
    curve_type = "2O";  
    
  return curve_type;
  
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

      //ALEXEY: transform into "SL,IS,DS,All,HL,2O"
      //xmlo.writeAttribute   ( "curve_type",         parms[ ii ].curv_type );
      QString curve_type = curve_type_toXml( parms[ ii ].curv_type );
      xmlo.writeAttribute   ( "curve_type",         curve_type );
      
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
   
   //z_value    = 0.72;
   z_value    = 0;
   
   tr_alpha   = 0.0;
   varcount   = 10;
   grf_iters  = 3;
   creso_pts  = 200;
   noise_flag = 0;
   treg_flag  = 0;
   mc_iters   = 1;
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
  // bool same      = true;
  // same           = ( channel != p.channel ) ? false : same;
  
  // return same;

  if ( x_min   != p.x_min    ) return false;
  if ( x_max   != p.x_max    ) return false;
  if ( y_min   != p.y_min    ) return false;
  if ( y_max   != p.y_max    ) return false;
  if ( z_value   != p.z_value    ) return false;
  if ( tr_alpha   != p.tr_alpha   ) return false;
  if ( varcount   != p.varcount  ) return false;
  if ( grf_iters   != p.grf_iters ) return false;
  if ( creso_pts   != p.creso_pts ) return false;
  if ( noise_flag   != p.noise_flag ) return false;
  if ( treg_flag   != p.treg_flag ) return false;
  if ( mc_iters   != p.mc_iters ) return false;
  
  if ( channel   != p.channel    ) return false;
  if ( curv_type   != p.curv_type    ) return false;
  if ( x_type   != p.x_type    ) return false;
  if ( y_type   != p.y_type    ) return false;
  if ( z_type   != p.z_type    ) return false;
  if ( noise_type   != p.noise_type   ) return false;
  if ( treg_type   != p.treg_type   ) return false;
  
  return true;
}


