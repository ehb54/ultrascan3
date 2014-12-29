//! \file us_pcsa_modelrec.cpp

#include "us_pcsa_modelrec.h"
#include "us_solute.h"

// Construct model record
US_ModelRecord::US_ModelRecord( void )
{
   taskx      = -1;
   ctype      = CTYPE_NONE;
   v_ctype    = CTYPE_NONE;
   mrecID     = 0;
   editID     = 0;
   modelID    = 0;
   str_k      = 0.0;
   end_k      = 0.0;
   par1       = 0.0;
   par2       = 0.0;
   variance   = 9999.9;
   rmsd       = 9999.9;
   smin       = 0.0;
   smax       = 0.0;
   kmin       = 0.0;
   kmax       = 0.0;
   mrecGUID   = "";
   editGUID   = "";
   modelGUID  = "";
   isolutes.clear();
   csolutes.clear();
   clear_data();
}

// Model record destructor
US_ModelRecord::~US_ModelRecord()
{
   isolutes.clear();
   csolutes.clear();

   clear_data();
}

// Public slot to clear data vectors
void US_ModelRecord::clear_data( void )
{
   sim_data .scanData.clear();
   sim_data .xvalues .clear();
   residuals.scanData.clear();
   residuals.xvalues .clear();
   ti_noise          .clear();
   ri_noise          .clear();
}

// Static public function to compute straight lines
int US_ModelRecord::compute_slines( double& smin, double& smax,
      double& fmin, double& fmax, int& nkpts, int& nlpts,
      double* parlims, QVector< US_ModelRecord >& mrecs )
{
   US_ModelRecord mrec;
   mrec.ctype    = CTYPE_SL;

   double  prng  = (double)( nlpts - 1 );
   double  krng  = (double)( nkpts - 1 );
   double  xrng  = smax - smin;
   double  xinc  = xrng / prng;
   if ( parlims[ 0 ] < 0.0 )
   {
      parlims[ 0 ] = fmin;
      parlims[ 1 ] = fmax;
      parlims[ 2 ] = fmin;
      parlims[ 3 ] = fmax;
   }
   double  yslo  = parlims[ 0 ];
   double  yshi  = parlims[ 1 ];
   double  yelo  = parlims[ 2 ];
   double  yehi  = parlims[ 3 ];
   double  vbar  = parlims[ 4 ];
   double  ysinc = ( yshi - yslo ) / krng;
   double  yeinc = ( yehi - yelo ) / krng;
   double  ystr  = yslo;

   int     mndx  = mrecs.size();
   int     nmodl = nkpts * nkpts;

   // Generate straight lines
   for ( int ii = 0; ii < nkpts; ii++ )
   { // Loop over k start values
      double yend = yelo;

      for ( int jj = 0; jj < nkpts; jj++ )
      { // Loop over k end values
         double xval = smin;
         double yval = ystr;
         double yinc = ( yend - ystr ) / prng; 

         mrec.isolutes.clear();
         US_Solute isol( 0.0, 0.0, 0.0, vbar );

         for ( int kk = 0; kk < nlpts; kk++ )
         { // Loop over points on a line
            isol.s      = xval * 1.e-13;
            isol.k      = yval;
            mrec.isolutes << isol;
            xval       += xinc;
            yval       += yinc;
         } // END: points-per-line loop

         mrec.taskx     = mndx;
         mrec.str_k     = ystr;
         mrec.end_k     = yend;
         mrec.par1      = ystr;
         mrec.par2      = ( yend - ystr ) / xrng;
//qDebug() << "MR: ii jj" << ii << jj << "ys ye p1 p2" << ystr << yend
// << mrec.par1 << mrec.par2;
         mrecs << mrec;

         yend   += yeinc;
         mndx++;
      } // END: k-end loop

      ystr   += ysinc;
   } // END: k-start loop

   return nmodl;
}

// Static public function to compute sigmoid curves
int US_ModelRecord::compute_sigmoids( int& ctype, double& smin, double& smax,
      double& fmin, double& fmax, int& nkpts, int& nlpts,
      double* parlims, QVector< US_ModelRecord >& mrecs )
{
   if ( parlims[ 0 ] < 0.0 )
   {
      parlims[ 0 ] = 0.001;
      parlims[ 1 ] = 0.5;
      parlims[ 2 ] = 0.0;
      parlims[ 3 ] = 1.0;
   }
   double  p1lo = parlims[ 0 ];
   double  p1up = parlims[ 1 ];
   double  p2lo = parlims[ 2 ];
   double  p2up = parlims[ 3 ];
   double  vbar = parlims[ 4 ];

   US_ModelRecord mrec;
   mrec.ctype   = ctype;

   double srng  = smax - smin;
   double p1llg = log( p1lo );
   double p1ulg = log( p1up );
   double prng  = (double)( nlpts - 1 );
   double krng  = (double)( nkpts - 1 );
   double p1inc = ( p1ulg - p1llg ) / krng;
   double p2inc = ( p2up  - p2lo  ) / krng;
   double xinc  = 1.0 / prng;
   double kstr  = fmin;               // Start,Diff of 'IS'
   double kdif  = fmax - fmin;
   if ( ctype == CTYPE_DS )
   {
      kstr         = fmax;            // Start,Diff of 'DS'
      kdif         = -kdif;
   }
   double p1vlg = p1llg;
   int    mndx  = mrecs.size();
   int    nmodl = nkpts * nkpts;

   for ( int ii = 0; ii < nkpts; ii++ )
   { // Loop over par1 values (logarithmic progression)
      double p1val = exp( p1vlg );
      double p1fac = sqrt( 2.0 * p1val );
      double p2val = p2lo;

      for ( int jj = 0; jj < nkpts; jj++ )
      { // Loop over par2 value (linear progression)
         double xval  = 0.0;
         double kval  = kstr;
         double kval0 = kval;
         double sval  = smin;

         mrec.isolutes.clear();
         US_Solute isol( 0.0, 0.0, 0.0, vbar );

         for ( int kk = 0; kk < nlpts; kk++ )
         { // Loop over points on a curve
            double efac  = 0.5 * erf( ( xval - p2val ) / p1fac ) + 0.5;
            kval         = kstr + kdif * efac;
            sval         = smin + xval * srng;
            xval        += xinc;
            if ( kk == 0 )
               kval0        = kval;

            isol.s       = sval * 1.e-13;
            isol.k       = kval;
            mrec.isolutes << isol;
         } // END: points-on-curve loop

         mrec.taskx   = mndx;
         mrec.str_k   = kval0;
         mrec.end_k   = kval;
         mrec.par1    = p1val;
         mrec.par2    = p2val;
         mrecs << mrec;

         mndx++;
         p2val    += p2inc;
      } // END: par2 values loop

      p1vlg    += p1inc;
   } // END: par1 values loop

   return nmodl;
}

// Static public function to compute horizontal lines [ C(s) ]
int US_ModelRecord::compute_hlines( double& smin, double& smax,
      double& fmin, double& fmax, int& nkpts, int& nlpts,
      double* parlims, QVector< US_ModelRecord >& mrecs )
{
   US_ModelRecord mrec;
   mrec.ctype    = CTYPE_HL;

   double  krng  = (double)( nkpts - 1 );
   double  prng  = (double)( nlpts - 1 );
   double  xinc  = ( smax - smin ) / prng;

   if ( parlims[ 0 ] < 0.0 )
   {
      parlims[ 0 ] = fmin;
      parlims[ 1 ] = fmax;
      parlims[ 2 ] = fmin;
      parlims[ 3 ] = fmax;
   }

   double  yval  = parlims[ 0 ];
   double  yinc  = ( parlims[ 1 ] - yval ) / krng;
   double  vbar  = parlims[ 4 ];
   int     mndx  = mrecs.size();

   // Generate horizontal lines
   for ( int ii = 0; ii < nkpts; ii++ )
   { // Loop over k start and end values
      double xval = smin;

      mrec.isolutes.clear();
      US_Solute isol( 0.0, 0.0, 0.0, vbar );

      for ( int kk = 0; kk < nlpts; kk++ )
      { // Loop over points on a line
         isol.s      = xval * 1.e-13;
         isol.k      = yval;
         mrec.isolutes << isol;
         xval       += xinc;
      } // END: points-per-line loop

      mrec.taskx  = mndx;
      mrec.str_k  = yval;
      mrec.end_k  = yval;
      mrec.par1   = yval;
      mrec.par2   = 0.0;
      yval       += yinc;
      mndx++;

      mrecs << mrec;
   } // END: k-value loop

   return nkpts;
}

// Static public function to load model records from an XML stream
int US_ModelRecord::load_modelrecs( QXmlStreamReader& xml,
   QVector< US_ModelRecord >& mrecs, QString& descr,
   int& ctype, double& smin, double& smax, double& kmin, double& kmax )
{
   int nmrecs      = 0;
   int nisols      = 0;
   int ncsols      = 0;
   bool old_vers   = false;
   const double ov = 1.0;
   US_ModelRecord mrec;
   QStringList sctypes;
   sctypes << "SL" << "IS" << "DS" << "All" << "HL";
   mrecs.clear();

   while ( ! xml.atEnd() )
   {
      xml.readNext();
      QString              xmlname  = xml.name().toString();

      if ( xml.isStartElement() )
      {
         QXmlStreamAttributes attrs    = xml.attributes();

         if ( xmlname == "modelrecords" )
         {
            double vers      = attrs.value( "version" ).toString().toDouble();
            old_vers         = ( vers <= ov );
            QString s_type   = attrs.value( "type" ).toString();
            ctype            = sctypes.contains( s_type ) ?
                               ctype_flag( s_type ) : s_type.toInt();
            if ( old_vers )
            {
               ctype            = ( ctype == 2 ) ? CTYPE_DS : ctype;
               ctype            = ( ctype == 1 ) ? CTYPE_IS : ctype;
               ctype            = ( ctype == 0 ) ? CTYPE_SL : ctype;
            }
//qDebug() << "STYPE" << s_type << "CTYPE" << ctype;
            smin             = attrs.value( "smin" ).toString().toDouble();
            smax             = attrs.value( "smax" ).toString().toDouble();
            kmin             = attrs.value( "kmin" ).toString().toDouble();
            kmax             = attrs.value( "kmax" ).toString().toDouble();
            nisols           = attrs.value( "curve_points" ).toString().toInt();
            mrec.v_ctype     = ctype;
            mrec.ctype       = ctype;
            mrec.smin        = smin;
            mrec.smax        = smax;
            mrec.kmin        = kmin;
            mrec.kmax        = kmax;
            descr            = attrs.value( "description" ).toString();
            QString s_eID    = attrs.value( "editID"      ).toString();
            QString s_mID    = attrs.value( "modelID"     ).toString();
            mrec.mrecGUID    = attrs.value( "mrecGUID"    ).toString();
            mrec.editGUID    = attrs.value( "editGUID"    ).toString();
            mrec.modelGUID   = attrs.value( "modelGUID"   ).toString();
            mrec.editID      = s_eID.isEmpty() ? 0 : s_eID.toInt();
            mrec.modelID     = s_mID.isEmpty() ? 0 : s_mID.toInt();
         }

         else if ( xmlname == "modelrecord" )
         {
            QString s_type   = attrs.value( "type" ).toString();
            mrec.ctype       = sctypes.contains( s_type ) ?
                               ctype_flag( s_type ) : s_type.toInt();
            if ( old_vers )
            {
               mrec.ctype       = ( mrec.ctype == 2 ) ? CTYPE_DS : mrec.ctype;
               mrec.ctype       = ( mrec.ctype == 1 ) ? CTYPE_IS : mrec.ctype;
               mrec.ctype       = ( mrec.ctype == 0 ) ? CTYPE_SL : mrec.ctype;
            }
            mrec.taskx       = attrs.value( "taskx"   ).toString().toInt();
            mrec.str_k       = attrs.value( "start_k" ).toString().toDouble();
            mrec.end_k       = attrs.value( "end_k"   ).toString().toDouble();
            mrec.par1        = attrs.value( "par1"    ).toString().toDouble();
            mrec.par2        = attrs.value( "par2"    ).toString().toDouble();
            mrec.rmsd        = attrs.value( "rmsd"    ).toString().toDouble();
            QString s_cvpt   = attrs.value( "curve_points" ).toString();
            int kisols       = s_cvpt.isEmpty() ? nisols : s_cvpt.toInt();

            if ( nmrecs > 0 )
            {
               mrec.mrecID      = 0;
               mrec.mrecGUID    = "";
               mrec.editID      = 0;
               mrec.editGUID    = "";
               QString s_mID    = attrs.value( "modelID"   ).toString();
               mrec.modelID     = s_mID.isEmpty() ? 0 : s_mID.toInt();
               mrec.modelGUID   = attrs.value( "modelGUID" ).toString();
            }

            ncsols           = 0;
            mrec.isolutes.resize( kisols );
            mrec.csolutes.clear();
            kmin             = qMin( kmin, mrec.str_k );
            kmax             = qMax( kmax, mrec.end_k );
         }

         else if ( xmlname == "c_solute" )
         {
            US_Solute csolute;
            csolute.s        = attrs.value( "s" ).toString().toDouble();
            csolute.k        = attrs.value( "k" ).toString().toDouble();
            csolute.c        = attrs.value( "c" ).toString().toDouble();
            smin             = qMin( smin, csolute.s );
            smax             = qMax( smax, csolute.s );
            kmin             = qMin( kmin, csolute.k );
            kmax             = qMax( kmax, csolute.k );
            csolute.s       *= 1.e-13;

            mrec.csolutes << csolute;
            ncsols++;
         }
      }

      else if ( xml.isEndElement()  &&  xmlname == "modelrecord" )
      {
         nmrecs++;
         mrecs << mrec;
         mrec.csolutes.clear();
      }
   }
   return nmrecs;
}

// Static public function to write model records to an XML stream
int US_ModelRecord::write_modelrecs( QXmlStreamWriter& xml,
   QVector< US_ModelRecord >& mrecs, QString& descr,
   int& ctype, double& smin, double& smax, double& kmin, double& kmax )
{
   int nmrecs      = mrecs.size();
   US_ModelRecord mrec;
   mrec            = mrecs[ 0 ];
   ctype           = ( ctype == CTYPE_NONE ) ? mrec.ctype : ctype;
   int v_ctype     = ctype;
   smin            = mrec.smin;
   smax            = mrec.smax;
   kmin            = mrec.kmin;
   kmax            = mrec.kmax;
   int nisols      = mrec.isolutes.size();
   xml.setAutoFormatting( true );
   xml.writeStartDocument( "1.0" );
   xml.writeComment( "DOCTYPE PcsaModelRecords" );
   xml.writeCharacters( "\n" );
   xml.writeStartElement( "modelrecords" );
   xml.writeAttribute( "version",      "1.2" );
   xml.writeAttribute( "description",  descr );
   xml.writeAttribute( "type",         ctype_text( v_ctype )     );
   xml.writeAttribute( "smin",         QString::number( smin )   );
   xml.writeAttribute( "smax",         QString::number( smax )   );
   xml.writeAttribute( "kmin",         QString::number( kmin )   );
   xml.writeAttribute( "kmax",         QString::number( kmax )   );
   xml.writeAttribute( "curve_points", QString::number( nisols ) );
   if ( mrec.editID > 0 )
      xml.writeAttribute( "editID",    QString::number( mrec.editID ) );
   if ( mrec.modelID > 0  )
      xml.writeAttribute( "modelID",   QString::number( mrec.modelID )  );
   if ( ! mrec.mrecGUID.isEmpty() )
      xml.writeAttribute( "mrecGUID",  mrec.mrecGUID  );
   if ( ! mrec.editGUID.isEmpty() )
      xml.writeAttribute( "editGUID",  mrec.editGUID  );
   if ( ! mrec.modelGUID.isEmpty() )
      xml.writeAttribute( "modelGUID", mrec.modelGUID );

   for ( int mr = 0; mr < nmrecs; mr++ )
   {
      mrec            = mrecs[ mr ];
      int kisols      = mrec.isolutes.size();
      int ncsols      = mrec.csolutes.size();
      xml.writeStartElement( "modelrecord" );
      xml.writeAttribute( "taskx",        QString::number( mrec.taskx )  );
      xml.writeAttribute( "type",         ctype_text( mrec.ctype )       );
      xml.writeAttribute( "start_k",      QString::number( mrec.str_k )  );
      xml.writeAttribute( "end_k",        QString::number( mrec.end_k )  );
      xml.writeAttribute( "par1",         QString::number( mrec.par1  )  );
      xml.writeAttribute( "par2",         QString::number( mrec.par2  )  );
      xml.writeAttribute( "rmsd",         QString::number( mrec.rmsd  )  );
      if ( kisols != nisols )
         xml.writeAttribute( "curve_points", QString::number( kisols )   );

      if ( mr == 0 )
      {
         if ( mrec.editID > 0 )
            xml.writeAttribute( "editID",    QString::number( mrec.editID ) );
         if ( ! mrec.mrecGUID.isEmpty() )
            xml.writeAttribute( "mrecGUID",  mrec.mrecGUID  );
         if ( ! mrec.editGUID.isEmpty() )
            xml.writeAttribute( "editGUID",  mrec.editGUID  );
      }

      if ( mrec.modelID > 0  )
         xml.writeAttribute( "modelID",   QString::number( mrec.modelID )  );
      if ( ! mrec.modelGUID.isEmpty() )
         xml.writeAttribute( "modelGUID", mrec.modelGUID );

      for ( int cc = 0; cc < ncsols; cc++ )
      {
         xml.writeStartElement( "c_solute" );
         double sval     = mrec.csolutes[ cc ].s * 1.e13;
         xml.writeAttribute( "s", QString::number( sval ) );
         xml.writeAttribute( "k", QString::number( mrec.csolutes[ cc ].k ) );
         xml.writeAttribute( "c", QString::number( mrec.csolutes[ cc ].c ) );
         xml.writeEndElement();
      }

      xml.writeEndElement();
   }

   xml.writeEndElement();
   xml.writeEndDocument();

   return nmrecs;
}


// Static public function to determine model records elite (top 10%) limits
void US_ModelRecord::elite_limits( QVector< US_ModelRecord >& mrecs,
      int& ctype,    double& minkv, double& maxkv,
      double& minp1, double& maxp1, double& minp2, double& maxp2 )
{
   const double efrac = 0.1;
   // Set up variables that help insure that the par1,par2 extents of elites
   // extend at least one step on either side of record 0's par1,par2
   double m0p1    = mrecs[ 0 ].par1;
   double m0p2    = mrecs[ 0 ].par2;
   double m0p1l   = m0p1;
   double m0p1h   = m0p1;
   double m0p2l   = m0p2;
   double m0p2h   = m0p2;
   bool ln_type   = false;

   if ( ctype == CTYPE_SL  ||  ctype == CTYPE_HL )
   {  // Possibly adjust initial par1,par2 limits for lines
      ln_type        = true;
      m0p1           = mrecs[ 0 ].str_k;
      m0p2           = mrecs[ 0 ].end_k;
      m0p1l          = ( m0p1 > maxkv ) ? m0p1 : ( m0p1 * 1.0001 );
      m0p1h          = ( m0p1 < minkv ) ? m0p1 : ( m0p1 * 0.9991 );
      m0p2l          = ( m0p2 > maxkv ) ? m0p2 : ( m0p2 * 1.0001 );
      m0p2h          = ( m0p2 < minkv ) ? m0p2 : ( m0p2 * 0.9991 );
   }

   if ( ctype == CTYPE_IS  ||  ctype == CTYPE_DS )
   {  // Possibly adjust initial par1,par2 limits for sigmoids
      double dif1    = m0p1 - 0.001;
      double dif2    = m0p1 - 0.500;
      double dif3    = m0p2 - 0.000;
      double dif4    = m0p2 - 1.000;
      m0p1l          = ( dif1 > 1.e-8 ) ? m0p1 : 0.002;
      m0p1h          = ( dif2 < -1e-8 ) ? m0p1 : 0.499;
      m0p2l          = ( dif3 > 1.e-8 ) ? m0p2 : 0.001;
      m0p2h          = ( dif4 < -1e-8 ) ? m0p2 : 0.999;
//qDebug() << " ElLim: ADJUST SIGM: m0p1 m0p1h" << m0p1 << m0p1h
//   << "m0p1<0.500" << (m0p1<0.500) << 0.500 << "(m0p1-0.5)" << (m0p1-0.5);
   }

   int nmrec      = mrecs.size();
   int nelite     = qRound( efrac * nmrec );        // Elite is top 10%
   int maxel      = nmrec / 2;
   int minel      = qMin( maxel, 4 );
   nelite         = qMin( nelite, maxel );          // At most half of all
   nelite         = qMax( nelite, minel );          // At least 4
   nelite        -= 2;                              // Less 2 for compare
//qDebug() << " ElLim: nmrec nelite" << nmrec << nelite;
//qDebug() << " ElLim: in minkv maxkv" << minkv << maxkv;
//qDebug() << " ElLim: in min/max p1/p2" << minp1 << maxp1 << minp2 << maxp2;
//qDebug() << " ElLim: in m0p1 m0p2" << m0p1 << m0p2;
//qDebug() << " ElLim: in m0p1l,m0p1h,m0p2l,m0p2h" << m0p1l << m0p1h
// << m0p2l << m0p2h;

   for ( int ii = 0; ii < nmrec; ii++ )
   {
      double str_k   = mrecs[ ii ].str_k;
      double end_k   = mrecs[ ii ].end_k;
      double par1    = ln_type ? str_k : mrecs[ ii ].par1;
      double par2    = ln_type ? end_k : mrecs[ ii ].par2;
//if(ii<3||(ii+4)>nelite)
//qDebug() << " ElLim:   ii" << ii << "par1 par2" << par1 << par2
// << "str_k end_k" << str_k << end_k << "rmsd" << mrecs[ii].rmsd;
      minkv          = qMin( minkv, str_k );
      maxkv          = qMax( maxkv, str_k );
      minkv          = qMin( minkv, end_k );
      maxkv          = qMax( maxkv, end_k );
      minp1          = qMin( minp1, par1  );
      maxp1          = qMax( maxp1, par1  );
      minp2          = qMin( minp2, par2  );
      maxp2          = qMax( maxp2, par2  );

      // We want to break out of the min,max scan loop if the sorted index
      // exceeds the elite count. But we continue in the loop if we have not
      // yet found min,max par1,par2 values that are at least a step
      // on either side of the par1,par2 values for the best model (m0).
//if(ii>nelite)
//qDebug() << " ElLim:    minp1 maxp1 m0p1" << minp1 << maxp1 << m0p1
// << "minp2 maxp2 m0p2" << minp2 << maxp2 << m0p2;
      if ( ii > nelite  &&
           minp1 < m0p1l  &&  maxp1 > m0p1h  &&
           minp2 < m0p2l  &&  maxp2 > m0p2h )
         break;
   }

//qDebug() << " ElLim: out minkv maxkv" << minkv << maxkv;
//qDebug() << " ElLim: out min/max p1/p2" << minp1 << maxp1 << minp2 << maxp2;
}

// Static public function to recompute model records vector for new iteration
int US_ModelRecord::recompute_mrecs( int& ctype, double& smin, double& smax,
      double& kmin, double& kmax, int& nkpts, int& nlpts, double* parlims,
      QVector< US_ModelRecord >&mrecs )
{
   int    nmrec  = mrecs.size();
   bool   LnType = ( ctype == CTYPE_SL  ||  ctype == CTYPE_HL );
   bool   SgType = ( ctype == CTYPE_IS  ||  ctype == CTYPE_DS );

   double minkv  = kmax;
   double maxkv  = kmin;
   double minp1  = LnType ? minkv : 0.500;
   double maxp1  = LnType ? maxkv : 0.001;
   double minp2  = LnType ? minkv : 1.0;
   double maxp2  = LnType ? maxkv : 0.0;
//qDebug() << "RF: 2)nmrec" << mrecs.size();

   elite_limits( mrecs, ctype, minkv, maxkv, minp1, maxp1, minp2, maxp2 );

   // Recompute the new min,max so that the old best is a point
   //  on the new grid to be tested
   double p1best = LnType ? mrecs[ 0 ].str_k : mrecs[ 0 ].par1;
   double p2best = LnType ? mrecs[ 0 ].end_k : mrecs[ 0 ].par2;
   double krng   = (double)( nkpts - 1 );
   double p1rng  = maxp1 - minp1;
   double p2rng  = maxp2 - minp2;
   double p1inc  = p1rng / krng;
   double p2inc  = p2rng / krng;
   double p1dif  = qRound( ( p1best - minp1 ) / p1inc ) * p1inc;
   double p2dif  = qRound( ( p2best - minp2 ) / p2inc ) * p2inc;
//qDebug() << "RF: rcomp: p12 rng" << p1rng << p2rng << "p12 inc"
// << p1inc << p2inc << "p12 dif" << p1dif << p2dif;
//qDebug() << "RF: rcomp: mmp1 mmp2" << minp1 << maxp1 << minp2 << maxp2;
   minp1         = p1best - p1dif;
   minp2         = p2best - p2dif;
   maxp1         = minp1 + p1inc * krng;
   maxp2         = minp2 + p2inc * krng;
//qDebug() << "RF: rcomp: nkpts" << nkpts;

   mrecs    .clear();

   if ( LnType )
   { // Determine models for straight-line or horizontal-line curves
      parlims[ 0 ]  = qMax( kmin, minp1 );
      parlims[ 1 ]  = qMin( kmax, maxp1 );
      parlims[ 2 ]  = qMax( kmin, minp2 );
      parlims[ 3 ]  = qMin( kmax, maxp2 );
//qDebug() << "RF: slin: mmp1 mmp2" << minp1 << maxp1 << minp2 << maxp2;
//qDebug() << "RF: slin:  p1,p2 best" << p1best << p2best;
//qDebug() << "RF: slin:    mnmx p1 p2" << parlims[0] << parlims[1]
// << parlims[2] << parlims[3] << "kmax" << kmax;

      if ( ctype == CTYPE_SL )
      {
         nmrec         = compute_slines( smin, smax, kmin, kmax,
                                         nkpts, nlpts, parlims, mrecs );
      }

      else if ( ctype == CTYPE_HL )
      {
         nmrec         = compute_hlines( smin, smax, kmin, kmax,
                                         nkpts, nlpts, parlims, mrecs );
      }
   }

   else if ( SgType )
   { // Determine models for sigmoid curves
//qDebug() << "RF: sigm:  mnmx p1 p2" << minp1 << maxp1 << minp2 << maxp2;
      parlims[ 0 ]  = qMax( 0.001, minp1 );
      parlims[ 1 ]  = qMin( 0.500, maxp1 );
      parlims[ 2 ]  = qMax( 0.000, minp2 );
      parlims[ 3 ]  = qMin( 1.000, maxp2 );
//qDebug() << "RF: sigm:  p1,p2 best" << p1best << p2best;
//qDebug() << "RF: sigm:    mnmx p1 p2" << parlims[0] << parlims[1]
// << parlims[2] << parlims[3];

      nmrec         = compute_sigmoids( ctype, smin, smax, kmin, kmax,
                                        nkpts, nlpts, parlims, mrecs );
//qDebug() << "RF: sigm: nmrec" << nmrec;
   }

   else if ( ctype == CTYPE_ALL )
   {
   }

   else
      nmrec       = 0;

   return nmrec;
}

// Static public function to return integer curve-type flag for given string
int US_ModelRecord::ctype_flag( const QString s_ctype )
{
   int i_ctype     = CTYPE_NONE;
   i_ctype         = ( s_ctype == "SL"  ) ? CTYPE_SL  : i_ctype;
   i_ctype         = ( s_ctype == "IS"  ) ? CTYPE_IS  : i_ctype;
   i_ctype         = ( s_ctype == "DS"  ) ? CTYPE_DS  : i_ctype;
   i_ctype         = ( s_ctype == "HL"  ) ? CTYPE_HL  : i_ctype;
   i_ctype         = ( s_ctype == "All" ) ? CTYPE_ALL : i_ctype;

   return i_ctype;
}

// Static public function to return curve-type text for given integer flag
QString US_ModelRecord::ctype_text( const int i_ctype )
{
   QString s_ctype = "None";
   s_ctype         = ( i_ctype == CTYPE_SL  ) ? "SL"  : s_ctype;
   s_ctype         = ( i_ctype == CTYPE_IS  ) ? "IS"  : s_ctype;
   s_ctype         = ( i_ctype == CTYPE_DS  ) ? "DS"  : s_ctype;
   s_ctype         = ( i_ctype == CTYPE_HL  ) ? "HL"  : s_ctype;
   s_ctype         = ( i_ctype == CTYPE_ALL ) ? "All" : s_ctype;

   return s_ctype;
}

