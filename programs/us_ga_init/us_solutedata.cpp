//! \file us_solutedata.cpp

#include "us_solutedata.h"
#include "us_defines.h"

// Holds Solute data for US_GA_Initialize
US_SoluteData::US_SoluteData( const QString& title ) : QObject()
{
   bndx   = -1;
   btitle = title;
   allbucks.clear();
}

US_SoluteData::US_SoluteData( ) : QObject()
{
   US_SoluteData( QString( "SoluteData#1" ) );
}

int US_SoluteData::clearBuckets()
{
   int rc = 0;
   bndx   = -1;
   allbucks.clear();
   return rc;
}

int US_SoluteData::sortBuckets()
{
   int rc = 0;
   return rc;
}

int US_SoluteData::appendBucket( bucket& buk )
{
   int rc = 0;
   bndx   = allbucks.size();
   allbucks.append( buk );
   return rc;
}

int US_SoluteData::appendBucket( QRectF brect, QPointF bpnt,
      qreal bconc, int bstat )
{
   bucket buk = createBucket( brect, bpnt, bconc, bstat );

   return appendBucket( buk );
}

int US_SoluteData::appendBucket( QRectF brect, QPointF bpnt, qreal bconc )
{
   return appendBucket( brect, bpnt, bconc, 2 );
}

int US_SoluteData::setBucket( int ix, QRectF brect, QPointF bpnt,
      qreal bconc, int bstat )
{
   bucket buk = createBucket( brect, bpnt, bconc, bstat );
   int rc     = 0;
   int bsiz   = allbucks.size();
   int lx     = bsiz - 1;
   int jx     = bndx + 1;
   jx         = ( jx < lx ) ? jx : lx;

   if ( ix > (-1)  &&  ix < bsiz )
   {
      bndx    = ix;
   }
   else if ( ix == (-1) )
   {
      bndx    = lx;
   }
   else if ( ix == (-2) )
   {
      bndx    = jx;
   }
   else if ( ix > lx )
   {
      bndx    = bsiz;
      allbucks.append( buk );
      rc      = 1;
   }

   allbucks.replace( bndx, buk );
   return rc;
}

int US_SoluteData::bucketsCount()
{
   return allbucks.size();
}

bucket US_SoluteData::createBucket( QRectF brect, QPointF bpnt,
      qreal bconc, int bstat )
{
   bucket buk;

   buk.s       = bpnt.x();
   buk.ff0     = bpnt.y();
   buk.s_min   = brect.left();
   buk.s_max   = brect.right();
   buk.ff0_min = brect.bottom();
   buk.ff0_max = brect.top();
   buk.conc    = bconc;
   buk.status  = bstat;

   return buk;
}
bucket US_SoluteData::minBucket()
{
   bucket buk;
   return buk;
}
bucket US_SoluteData::maxBucket()
{
   bucket buk;
   return buk;
}

bucket US_SoluteData::bucketAt( int ix )
{
   bucket buk;
   int bsiz   = allbucks.size();
   int lx     = bsiz - 1;
   int jx     = bndx + 1;
   jx         = ( jx < lx ) ? jx : lx;

   if ( ix > (-1)  &&  ix < bsiz )
   {  // get bucket at specified index and save the index
      buk   = allbucks.at( ix );
      bndx    = ix;
   }
   else if ( ix == (-1) )
   {  // get last bucket and set its index
      buk   = allbucks.at( lx );
      bndx    = lx;
   }
   else if ( ix == (-2) )
   {  // get next bucket and set its index
      buk   = allbucks.at( jx );
      bndx    = jx;
   }
   else if ( ix == (-3) )
   {  // get current bucket
      buk   = allbucks.at( bndx );
   }

   return buk;
}

bucket US_SoluteData::firstBucket()
{
   return bucketAt( 0 );
}

bucket US_SoluteData::lastBucket()
{
   return bucketAt( -1 );
}

bucket US_SoluteData::nextBucket()
{
   return bucketAt( -2 );
}

bucket US_SoluteData::currentBucket()
{
   return bucketAt( -3 );
}

QRectF US_SoluteData::bucketRect(  int ix )
{
   bucket buk = bucketAt( ix );
   qreal x1   = buk.s_min;
   qreal x2   = buk.s_max;
   qreal y1   = buk.ff0_min;
   qreal y2   = buk.ff0_max;

   if ( x1 > x2 )
   {
      x1         = x2;
      x2         = buk.s_min;
   }

   if ( y1 > y2 )
   {
      y1         = y2;
      y2         = buk.ff0_min;
   }

   return QRectF( QPointF( x1, y1 ), QPointF( x2, y2 ) );
}

QPointF US_SoluteData::bucketPoint( int ix )
{
   bucket buk  = bucketAt( ix );
   qreal x1    = buk.s_min;
   qreal x2    = buk.s_max;
   qreal y1    = buk.ff0_min;
   qreal y2    = buk.ff0_max;

   QPointF mpt( ( x1 + x2 ) / 2.0, ( y1 + y2 ) / 2.0 );
   QPointF spt = mpt;
   QPointF& pt = spt;

   findNearestPoint( pt );

   return pt;
}

QSizeF US_SoluteData::bucketSize(  int ix )
{
   bucket buk = bucketAt( ix );
   qreal xext = buk.s_max - buk.s_min;
   xext       = ( xext > 0.0 ) ? xext : -xext;
   qreal yext = buk.ff0_max - buk.ff0_min;
   yext       = ( yext > 0.0 ) ? yext : -yext;
   QSizeF siz( xext, yext );
   return siz;
}

QString US_SoluteData::bucketLine(  int ix )
{
   bucket buk = bucketAt( ix );
   int kx     = ( ix < 0 ) ? bucketsCount() : ( ix + 1 );
#if 0
   QString line;
   line.sprintf(
      "Solute Bin %d: s_min=%4.2e, s_max=%4.2e, f/f0_min=%4.2f, f/f0_max=%4.2f",
      kx, buk.s_min, buk.s_max, buk.ff0_min, buk.ff0_max );
   return line;
#endif
#if 1
   return QString(
      "Solute Bin %1: s_min=%2, s_max=%3, f/f0_min=%4, f/f0_max=%5" )
      .arg( kx ).arg( buk.s_min ).arg( buk.s_max )
      .arg( buk.ff0_min ).arg( buk.ff0_max );
#endif
}

void US_SoluteData::setDistro( SoluteList* a_distro )
{
   distro  = a_distro;
}

int US_SoluteData::findNearestPoint( QPointF& midpt )
{
   qreal xm = midpt.x();
   qreal ym = midpt.y();
   qreal xn = xm;
   qreal yn = ym;
   qreal dl = xm * xm + ym * ym;
   qreal dd = dl * 10.0;
   int kx   = -1;

   for ( int ii = 0; ii < distro->size(); ii++ )
   {
      qreal xs = distro->at( ii ).s;
      qreal ys = distro->at( ii ).k;
      qreal xd = xs - xm;
      qreal yd = ys - ym;
      dd       = xd * xd + yd * yd;

      if ( dd < dl )
      {
         kx       = ii;
         dl       = dd;
         xn       = xs;
         yn       = ys;
      }
   }

   if ( kx >= 0 )
   {
      midpt.setX( xn );
      midpt.setY( yn );
   }

   return kx;
}

int US_SoluteData::removeBucketAt( int ix )
{
   allbucks.removeAt( ix );
   return allbucks.size();
}

