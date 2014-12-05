//! \file us_pcsa_modelrec.cpp

#include "us_pcsa_modelrec.h"
#include "us_solute.h"

// Construct model record
US_ModelRecord::US_ModelRecord( void )
{
   taskx      = -1;
   ctype      = 0;
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
   mrec.ctype    = 1;

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
   if ( ctype == 4 )
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
   mrec.ctype    = 8;

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
   US_ModelRecord mrec;
   QMap< QString, int > ctypes;
   ctypes[ "IS"  ] = 1;
   ctypes[ "IS"  ] = 2;
   ctypes[ "DS"  ] = 4;
   ctypes[ "All" ] = 7;
   ctypes[ "HL"  ] = 8;
   QStringList sctypes = ctypes.keys();
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
            QString s_type   = attrs.value( "type" ).toString();
            ctype            = sctypes.contains( s_type ) ?
                               ctypes[ s_type ] : 0;
            smin             = attrs.value( "smin" ).toString().toDouble();
            smax             = attrs.value( "smax" ).toString().toDouble();
            kmin             = attrs.value( "kmin" ).toString().toDouble();
            kmax             = attrs.value( "kmax" ).toString().toDouble();
            nisols           = attrs.value( "curve_points" ).toString().toInt();
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
                               ctypes[ s_type ] : 0;
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
   ctype           = ( ctype == 0 ) ? mrec.ctype : ctype;
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
   xml.writeAttribute( "version",      "1.1" );
   xml.writeAttribute( "description",  descr );
   xml.writeAttribute( "type",         QString::number( ctype )  );
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
      xml.writeAttribute( "type",         QString::number( mrec.ctype )  );
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

