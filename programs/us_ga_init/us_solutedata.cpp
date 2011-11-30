//! \file us_solutedata.cpp

#include "us_solutedata.h"
#include "us_defines.h"
#include "us_math2.h"
#include "us_settings.h"

// bucket vertex LessThan routine
bool buck_vx_lessthan( const bucket &buck1, const bucket &buck2 )
{  // TRUE iff  (sx1<sx2) ||  (sx1==sx2 && sy1<sy2)
   return ( buck1.s_min < buck2.s_min ) ||
      ( ( buck1.s_min == buck2.s_min ) && ( buck1.ff0_min < buck2.ff0_min ) );
}

// Holds Solute data for US_GA_Initialize
US_SoluteData::US_SoluteData( const QString& title ) : QObject()
{
   bndx   = -1;
   btitle = title;
   allbucks.clear();
   dbg_level = US_Settings::us_debug();
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

int US_SoluteData::sortBuckets( QList< bucket >* buks )
{
   int rc = 0;
   qSort( buks->begin(), buks->end() );
   return rc;
}

int US_SoluteData::sortBuckets()
{
   int rc = 0;
   qSort( allbucks.begin(), allbucks.end() );
   return rc;
}


int US_SoluteData::appendBucket( bucket& buk )
{
   int rc = 0;

   limitBucket( buk );
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

   limitBucket( buk );

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

QPointF US_SoluteData::bucketPoint( int ix, bool nearest )
{
   bucket buk  = bucketAt( ix );    // get specified bucket
   qreal x1    = buk.s_min;         // get vertices
   qreal x2    = buk.s_max;
   qreal y1    = buk.ff0_min;
   qreal y2    = buk.ff0_max;

   QPointF mpt( ( x1 + x2 ) / 2.0, ( y1 + y2 ) / 2.0 );
   QPointF spt = mpt;               // get mid-point
   QPointF& pt = spt;

   if ( nearest )                   // get nearby solute point
      findNearestPoint( pt );

   return pt;
}

QPointF US_SoluteData::bucketPoint( int ix )
{
   return bucketPoint( ix, true );   // solute point near bucket 
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

   limitBucket( buk );

   return QString(
      "Solute Bin %1: s_min=%2, s_max=%3, f/f0_min=%4, f/f0_max=%5" )
      .arg( kx ).arg( buk.s_min ).arg( buk.s_max )
      .arg( buk.ff0_min ).arg( buk.ff0_max );
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

// do automatic calculation of bins
int US_SoluteData::autoCalcBins( int mxsols, qreal wsbuck, qreal hfbuck )
{
#define _MIN_VHR_ 0.01
#define FMIN(a,b) ((a<b)?a:b)
#define FMAX(a,b) ((a>b)?a:b)
   QList< bucket >  tbuk1;
   QList< bucket >  tbuk2;
   QList< bucket >* buks1 = &tbuk1;
   QList< bucket >* buks2 = &tbuk2;
   QList< qreal  >  cvals;
   bucket           buk;
   int              bukflip = 0;
   int              nisols  = distro->size();
   int              nssols  = nisols;
   int              ntsols  = mxsols;
   qreal            cval;
   qreal            cutlo;
   qreal            wbuckh  = wsbuck / 2.0;
   qreal            hbuckh  = hfbuck / 2.0;

   if ( ntsols < 2  ||  ntsols > nisols )
   {
      ntsols      = nisols;
   }


   // initialize work list from solute distribution and get prelim stats
   for ( int jj = 0; jj < nisols; jj++ )
   {
      qreal sval  = distro->at( jj ).s;
      qreal fval  = distro->at( jj ).k;
      cval        = distro->at( jj ).c;
      buk.s       = sval;
      buk.s_min   = sval - wbuckh;
      buk.s_max   = sval + wbuckh;
      buk.ff0     = fval;
      buk.ff0_min = fval - hbuckh;
      buk.ff0_max = fval + hbuckh;
      buk.conc    = cval;
      buk.status  = 2;

      limitBucket( buk );

      buks1->append( buk );
      cvals.append( cval );
   }

   // sort the concentrate values to find cut-off values
   qSort( cvals.begin(), cvals.end() );

   // (possibly) pare down the list based on cut-off C values
   cutlo       = cvals.at( nisols - ntsols ); // low val in sorted concen vals

   for ( int jj = 0; jj < nssols; jj++ )
   {
      buk         = buks1->at( jj );
      cval        = buk.conc;
      if ( cval >= cutlo )
      {
         buks2->append( buk );
      }
   }

   nssols      = buks2->size();
   buks1->clear();
   buks1       = bukflip ? &tbuk1 : &tbuk2;
   buks2       = bukflip ? &tbuk2 : &tbuk1;
   bukflip     = 1 - bukflip;

   // now perform as many passes as needed to handle all overlaps
   bool overs  = true;
   int npass   = 0;
   int tstat   = 1;      // set up to skip reduced bins during 1st few passes

   while ( overs )
   {
      int novls   = 0;
      npass++;
      buks2->append( buks1->at( 0 ) );

      for ( int jj = 1; jj < nssols; jj++ )
      {  // examine each of the current buckets
         qreal horzr;
         qreal vertr;
         buk          = buks1->at( jj );

         // don't break up bins that are already reduced for 1st few passes
         if ( buk.status == tstat )
         {
            buks2->append( buk );
            continue;
         }

         qreal cx1    = buk.s_min;
         qreal cx2    = buk.s_max;
         qreal cy1    = buk.ff0_min;
         qreal cy2    = buk.ff0_max;

         for ( int kk = 0; kk < jj; kk++ )
         {  // compare current to each previous bucket
            bucket buk2  = buks1->at( kk );
            qreal px1    = buk2.s_min;
            qreal px2    = buk2.s_max;
            qreal py1    = buk2.ff0_min;
            qreal py2    = buk2.ff0_max;

            if ( cx1 < px2 )
            {  // possible horizontal overlap

               if ( cy1 < py2  &&  cy2 > py2 )
               {  // current overlaps in its lower left
                  novls++;
                  buk.status   = 1;
                  buk2         = buk;
                  horzr        = ( px2 - cx1 ) / wsbuck;
                  vertr        = ( py2 - cy1 ) / hfbuck;
                  if ( vertr < horzr )
                  {  // split vertically
                     buk2.s_min   = FMIN(cx1,px2);
                     buk2.s_max   = FMAX(px2,cx1);
                     buk2.ff0_min = FMIN(py2,cy2);
                     buk2.ff0_max = FMAX(cy2,py2);
                     horzr        = ( buk2.s_max   - buk2.s_min   ) / wsbuck;
                     vertr        = ( buk2.ff0_max - buk2.ff0_min ) / hfbuck;
                     if ( horzr > _MIN_VHR_  &&  vertr > _MIN_VHR_ )
                        buks2->append( buk2 );
else DbgLv(2) << "BUCKET TOO THIN H,V " << horzr << "," << vertr;

                     buk.s_min    = FMIN(px2,cx2);
                     buk.s_max    = FMAX(cx2,px2);
                     buk.ff0_min  = cy1;
                     buk.ff0_max  = cy2;
                  }

                  else
                  {  // split horizontally
                     buk2.s_min   = cx1;
                     buk2.s_max   = cx2;
                     buk2.ff0_min = FMIN(py2,cy2);
                     buk2.ff0_max = FMAX(cy2,py2);
                     horzr        = ( buk2.s_max   - buk2.s_min   ) / wsbuck;
                     vertr        = ( buk2.ff0_max - buk2.ff0_min ) / hfbuck;
                     if ( horzr > _MIN_VHR_  &&  vertr > _MIN_VHR_ )
                        buks2->append( buk2 );
else DbgLv(2) << "BUCKET TOO THIN H,V " << horzr << "," << vertr;

                     buk.s_min    = FMIN(px2,cx2);
                     buk.s_max    = FMAX(cx2,px2);
                     buk.ff0_min  = FMIN(cy1,py2);
                     buk.ff0_max  = FMAX(py2,cy1);
                  }
DbgLv(2) << "  LL OVL: novls " << novls;
                  break;
               }

               else if ( cy2 > py1  &&  cy1 < py1 )
               {  // current overlaps in its upper left
                  novls++;
                  buk.status   = 1;
                  buk2         = buk;
                  horzr        = ( px2 - cx1 ) / wsbuck;
                  vertr        = ( cy2 - py1 ) / hfbuck;
                  if ( vertr < horzr )
                  {  // split vertically
                     buk2.s_min   = FMIN(cx1,px2);
                     buk2.s_max   = FMAX(px2,cx1);
                     buk2.ff0_min = FMIN(cy1,py1);
                     buk2.ff0_max = FMAX(py1,cy1);
                     horzr        = ( buk2.s_max   - buk2.s_min   ) / wsbuck;
                     vertr        = ( buk2.ff0_max - buk2.ff0_min ) / hfbuck;
                     if ( horzr > _MIN_VHR_  &&  vertr > _MIN_VHR_ )
                        buks2->append( buk2 );
else DbgLv(2) << "BUCKET TOO THIN H,V " << horzr << "," << vertr;

                     buk.s_min    = FMIN(px2,cx2);
                     buk.s_max    = FMAX(cx2,px2);
                     buk.ff0_min  = cy1;
                     buk.ff0_max  = cy2;
                  }

                  else
                  {  // split horizontally
                     buk2.s_min   = FMIN(px2,cx2);
                     buk2.s_max   = FMAX(cx2,px2);
                     buk2.ff0_min = FMIN(py1,cy2);
                     buk2.ff0_max = FMAX(cy2,py1);
                     horzr        = ( buk2.s_max   - buk2.s_min   ) / wsbuck;
                     vertr        = ( buk2.ff0_max - buk2.ff0_min ) / hfbuck;
                     if ( horzr > _MIN_VHR_  &&  vertr > _MIN_VHR_ )
                        buks2->append( buk2 );
else DbgLv(2) << "BUCKET TOO THIN H,V " << horzr << "," << vertr;

                     buk.s_min    = FMIN(cx1,cx2);
                     buk.s_max    = FMAX(cx2,cx1);
                     buk.ff0_min  = FMIN(cy1,py1);
                     buk.ff0_max  = FMAX(py1,cy1);
                  }
DbgLv(2) << "  UL OVL: novls " << novls;
                  break;
               }

               else if ( cy1 >= py1  &&  cy2 <= py2 )
               {  // current overlaps in its middle left
                  novls++;
                  buk.status   = 1;
                  buk.s_min    = FMIN(px2,cx2);
                  buk.s_max    = FMAX(cx2,px2);
DbgLv(2) << "  UL OVL: novls " << novls;
                  break;
               }

               else if ( cy2 > py2  &&  cy1 < py1 )
               {  // current overlaps in both upper and lower left
                  novls++;
                  buk.status   = 1;
                  buk2         = buk;
                  horzr        = ( px2 - cx1 ) / wsbuck;
                  vertr        = ( py2 - py1 ) / hfbuck;
                  if ( vertr < horzr )
                  {  // split vertically (into 3 total parts)
                     buk2.s_min   = FMIN(cx1,px2);  // top
                     buk2.s_max   = FMAX(px2,cx1);
                     buk2.ff0_min = FMIN(py1,cy2);
                     buk2.ff0_max = FMAX(cy2,py1);
                     horzr        = ( buk2.s_max   - buk2.s_min   ) / wsbuck;
                     vertr        = ( buk2.ff0_max - buk2.ff0_min ) / hfbuck;
                     if ( horzr > _MIN_VHR_  &&  vertr > _MIN_VHR_ )
                        buks2->append( buk2 );
else DbgLv(2) << "BUCKET TOO THIN H,V " << horzr << "," << vertr;

                     buk2.ff0_min = FMIN(cy1,py1);  // bottom
                     buk2.ff0_max = FMAX(py1,cy1);
                     horzr        = ( buk2.s_max   - buk2.s_min   ) / wsbuck;
                     vertr        = ( buk2.ff0_max - buk2.ff0_min ) / hfbuck;
                     if ( horzr > _MIN_VHR_  &&  vertr > _MIN_VHR_ )
                        buks2->append( buk2 );
else DbgLv(2) << "BUCKET TOO THIN H,V " << horzr << "," << vertr;

                     buk.s_min    = FMIN(px2,cx2);  // right
                     buk.s_max    = FMAX(cx2,px2);
                     buk.ff0_min  = cy1;
                     buk.ff0_max  = cy2;
                  }

                  else
                  {  // split horizontally (into 3 total parts)
                     buk2.s_min   = FMIN(cx1,cx2);  // top
                     buk2.s_max   = FMAX(cx2,cx1);
                     buk2.ff0_min = FMIN(py2,cy2);
                     buk2.ff0_max = FMAX(cy2,py2);
                     horzr        = ( buk2.s_max   - buk2.s_min   ) / wsbuck;
                     vertr        = ( buk2.ff0_max - buk2.ff0_min ) / hfbuck;
                     if ( horzr > _MIN_VHR_  &&  vertr > _MIN_VHR_ )
                        buks2->append( buk2 );
else DbgLv(2) << "BUCKET TOO THIN H,V " << horzr << "," << vertr;

                     buk2.ff0_min = FMIN(cy1,py1);  // bottom
                     buk2.ff0_max = FMAX(py1,cy1);
                     horzr        = ( buk2.s_max   - buk2.s_min   ) / wsbuck;
                     vertr        = ( buk2.ff0_max - buk2.ff0_min ) / hfbuck;
                     if ( horzr > _MIN_VHR_  &&  vertr > _MIN_VHR_ )
                        buks2->append( buk2 );
else DbgLv(2) << "BUCKET TOO THIN H,V " << horzr << "," << vertr;

                     buk.s_min    = FMIN(px2,cx2);  // middle
                     buk.s_max    = FMAX(cx2,px2);
                     buk.ff0_min  = FMIN(py1,py2);
                     buk.ff0_max  = FMAX(py2,py1);
                  }
DbgLv(2) << "  UL OVL: novls " << novls;
                  break;
               }

            }

            else if ( cx1 == px1 )
            {  // possible pure vertical overlap

               if ( cy1 < py1  &&  cy2 > py1 )
               {  // current overlaps in its upper part
                  novls++;
                  buk.status   = 1;
                  buk.ff0_min  = FMIN(cy1,py1);
                  buk.ff0_max  = FMAX(py1,cy1);
DbgLv(2) << "   UV OVL: novls " << novls;
                  break;
               }
            }
         }
         horzr        = ( buk.s_max   - buk.s_min   ) / wsbuck;
         vertr        = ( buk.ff0_max - buk.ff0_min ) / hfbuck;
         if ( horzr > _MIN_VHR_  &&  vertr > _MIN_VHR_ )
            buks2->append( buk );
else DbgLv(2) << "BUCKET TOO THIN H,V " << horzr << "," << vertr;
      }
      // get new bucket count; flip-flop input,output lists
      nssols      = buks2->size();
      buks1->clear();
      buks1       = bukflip ? &tbuk1 : &tbuk2;
      buks2       = bukflip ? &tbuk2 : &tbuk1;
      bukflip     = 1 - bukflip;

      if ( novls > 0 )
      {  // if there were overlaps, re-sort buckets by vertex for next pass
         qSort( buks1->begin(), buks1->end(), buck_vx_lessthan );
      }

      else if ( tstat == 1 )
      {  // start new set of passes where reduced buckets can be reduced more
         tstat       = 4;
      }

      else
      {  // otherwise, we must be done!
         overs       = false;
      }
   }

   // do a re-sort based on center point
   qSort( buks1->begin(), buks1->end() );

   // copy the final bucket list to the master

   allbucks.clear();

   for ( int jj = 0; jj < nssols; jj++ )
   {
      allbucks.append( buks1->at( jj ) );
   }

   tbuk1.clear();
   tbuk2.clear();
   cvals.clear();
   return nssols;
}

// save bucket information to file for use by GA
int US_SoluteData::saveGAdata( QString& fname )
{
   int     rc   = 0;
   int     nsol = allbucks.size();
   QString line;
   bucket  buk;

   QFile fileo( fname );

   if ( fileo.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      QTextStream ts( &fileo );
      ts << nsol << endl;

      for ( int jj = 0; jj < nsol; jj++ )
      {
         buk      = allbucks.at( jj );

         limitBucket( buk );

         line.sprintf(
            "%6.4f, %6.4f, %6.4f, %6.4f",
             buk.s_min, buk.s_max, buk.ff0_min, buk.ff0_max );
         ts << line << endl;
      }
      fileo.close();
   }
   else
      rc    = 1;

   return rc;
}


// build the data lists for Monte Carlo analysis
int US_SoluteData::buildDataMC( bool plot_s )
{
   int         rc   = 0;
   int         nsol = distro->size();
   int         nbuk = allbucks.size();
   bucket      buk;          // bucket record
   Solute      d_sol;        // solute record
   SimComp     simc;         // simulation component record
   SimCompList bcomp;        // sim component list
   qreal       bsmin;        // bucket vertices
   qreal       bsmax;
   qreal       bfmin;
   qreal       bfmax;
   qreal       ssval;        // component s,f_f0 values
   qreal       sfval;

   // build component list from solute lists
   component.clear();

   if ( plot_s )      // solute distro is s_distro
   {
      for ( int jj = 0; jj < nsol; jj++ )
      {
         d_sol    = distro->at( jj );    // get solute record ("x" is "s")

         simc.s   = d_sol.s * 1e-13;     // compose simulation component
         simc.w   = d_sol.w;
         simc.f   = d_sol.k;
         simc.c   = d_sol.c;
         simc.d   = d_sol.d;

         component.append( simc );       // and add it to list
      }
   }

   else               // solute distro is mw_distro
   {
      for ( int jj = 0; jj < nsol; jj++ )
      {
         d_sol    = distro->at( jj );    // get solute record ("x" is "mw")

         simc.s   = d_sol.w * 1e-13;     // compose simulation component
         simc.w   = d_sol.s;
         simc.f   = d_sol.k;
         simc.c   = d_sol.c;
         simc.d   = d_sol.d;

         component.append( simc );       // and add it to list
      }
   }
   // build list of component solute data for each bucket
   MC_solute.clear();

   for ( int jj = 0; jj < nbuk; jj++ )
   {
      // get bucket dimensions
      buk      = allbucks.at( jj );      // get bucket and its vertices
      bsmin    = buk.s_min;
      bsmax    = buk.s_max;
      bfmin    = buk.ff0_min;
      bfmax    = buk.ff0_max;
      bcomp.clear();

      for ( int kk = 0; kk < nsol; kk++ )
      {  // add solute points that fall within bin dimensions
         ssval    = distro->at( kk ).s;  // "x,y" of solute point
         sfval    = distro->at( kk ).k;
         if ( ssval >= bsmin  &&  ssval <= bsmax   &&
              sfval >= bfmin  &&  sfval <= bfmax )
         {  // solute is in this bin:  save component for bin
            bcomp.append( component.at( kk ) );
         }
      }

      MC_solute.append( bcomp );
   }

   return rc;
}

// complete analysis and report Monte Carlo statistics
int US_SoluteData::reportDataMC( QString& fname, int mc_iters )
{
   int         rc = 0;
   int         nbuk = MC_solute.size();
   bucket      buk;          // bucket record
   SimCompList bcomp;        // sim component list
   QList< double > vals;

   QFile fileo( fname );

   if ( fileo.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {  // output Monte Carlo statistics to a report file
      QList< qreal > valus;
      QList< qreal > concs;
      QList< qreal > csums;
      QTextStream ts( &fileo );
      QString str1;

      ts << "*****************************************"
         "**********************************\n\n";
      ts << "Monte Carlo Analysis Statistical Results "
         "(from Genetic Algorithm Analysis):\n\n";
      ts << "*****************************************"
         "**********************************\n\n";
      ts << "Summary:\n";

      if ( nbuk < 1 )
      {
         return 2;
      }
      qreal concsum = 0.0;
      qreal vsum    = 0.0;
      qreal vsiz    = 0.0;

      for ( int kk = 0; kk < nbuk; kk++ )
      {  // accumulate statistics for each bin
         bcomp    = MC_solute.at( kk );
         ts << "\nSolute " << ( kk + 1 ) << ":\n";
         int ksol = bcomp.size();
         vsiz     = (double)ksol;

         if ( ksol < 3 )
         {  // Summary prints for a bin that has only a point or two
            ts << "\nThis solute bin does not have sufficient points to"
               "\ncalculate meaningful statistics.\n";

            ts << "Average Molecular Weight: ";
            vsum     = 0.0;
            for ( int jj = 0; jj < ksol; jj++ )
               vsum    += bcomp.at( jj ).w;
            ts << ( vsum / vsiz ) << endl;

            ts << "Average Concentration:    ";
            vsum     = 0.0;
            for ( int jj = 0; jj < ksol; jj++ )
               vsum    += bcomp.at( jj ).c;
            ts << ( vsum / vsiz ) << endl;
            csums.append( vsum );
            concsum += vsum;

            ts << "Average Frictional Ratio: ";
            vsum     = 0.0;

            for ( int jj = 0; jj < ksol; jj++ )
               vsum    += bcomp.at( jj ).f;
            ts << ( vsum / vsiz ) << endl;
         }

         else
         {  // Summary prints for the typical bin with many points
            qreal vtotal  = 0.0;
            qreal sclmci  = (qreal)mc_iters;
            valus.clear();
            concs.clear();

            for ( int jj = 0; jj < ksol; jj++ )
               concs.append( bcomp.at( jj ).c );

            for ( int jj = 0; jj < ksol; jj++ )
               valus.append( bcomp.at( jj ).w );
            outputStats( ts, valus, concs, false,
                  tr( "Molecular weight:        " ) );
            valus.clear();

            for ( int jj = 0; jj < ksol; jj++ )
            {
               qreal cval = bcomp.at( jj ).c;
               valus.append( cval * sclmci );
               vtotal    += cval;
            }

            csums.append( vtotal );
            concsum   += vtotal;
            outputStats( ts, concs, concs, false,
                  tr( "Concentration:           " ) );
            valus.clear();

            ts << tr( "Total Concentration:     " ) <<
               str1.sprintf( " %6.4e\n", vtotal );

            for ( int jj = 0; jj < ksol; jj++ )
               valus.append( bcomp.at( jj ).s );
            outputStats( ts, valus, concs, false,
                  tr( "Sedimentation Coeff.:    " ) );
            valus.clear();

            for ( int jj = 0; jj < ksol; jj++ )
               valus.append( bcomp.at( jj ).d );
            outputStats( ts, valus, concs, false,
                  tr( "Diffusion Coeff.:        " ) );
            valus.clear();

            for ( int jj = 0; jj < ksol; jj++ )
               valus.append( bcomp.at( jj ).f );
            outputStats( ts, valus, concs, false,
                  tr( "Frictional Ratio, f/f0:  " ) );
            valus.clear();
         }
      }

      ts << tr( "\n\nRelative Concentrations:\n\n" );
      ts << tr( "Total concentration: " ) << concsum << " OD\n";

      for ( int jj = 0; jj < csums.size(); jj++ )
      {
         qreal pcconc = 100.0 * csums.at( jj ) / concsum;
         ts << tr( "Relative percentage of Solute " ) << ( jj + 1 )
            << ": " << QString().sprintf( "%7.3f", pcconc ) << " %\n";
      }

      ts << tr( "\n\nDetailed Results:\n" );

      for ( int kk = 0; kk < nbuk; kk++ )
      {  // output detailed statistics for all the bins
         bcomp    = MC_solute.at( kk );
         ts << "\n*****************************************"
            "**********************************";
         ts << "\nSolute " << ( kk + 1 ) << ":";
         int ksol = bcomp.size();
         vsiz     = (double)ksol;

         if ( ksol < 3 )
         {  // just print the values for a sparse bin
            ts << tr( "\nThis solute bin does not have sufficient points to"
                  "\ncalculate a meaningful distribution\n" );

            ts << tr( "\nMolecular Weight:\n" );
            for ( int jj = 0; jj < ksol; jj++ )
               ts << bcomp.at( jj ).w << endl;
 
            ts << tr( "\nConcentration:\n" );
            for ( int jj = 0; jj < ksol; jj++ )
               ts << bcomp.at( jj ).c << endl;

            ts << tr( "\nSedimentation Coefficient:\n" );
            for ( int jj = 0; jj < ksol; jj++ )
               ts << bcomp.at( jj ).s << endl;

            ts << tr( "\nDiffusion Coefficient:\n" );
            for ( int jj = 0; jj < ksol; jj++ )
               ts << bcomp.at( jj ).d << endl;

            ts << tr( "\nFrictional Ratio:\n" );
            for ( int jj = 0; jj < ksol; jj++ )
               ts << bcomp.at( jj ).f << endl;
         }

         else
         {  // calculate and output detailed statistics for the bin
            qreal sclmci  = (qreal)mc_iters;
            concs.clear();
            valus.clear();

            for ( int jj = 0; jj < ksol; jj++ )
               concs.append( bcomp.at( jj ).c );

            for ( int jj = 0; jj < ksol; jj++ )
               valus.append( bcomp.at( jj ).w );
            outputStats( ts, valus, concs, true,
                  tr( "Molecular Weight" ) );
            valus.clear();

            for ( int jj = 0; jj < ksol; jj++ )
               valus.append( bcomp.at( jj ).c * sclmci );
            outputStats( ts, valus, concs, true,
                  tr( "Concentration" ) );
            valus.clear();

            for ( int jj = 0; jj < ksol; jj++ )
               valus.append( bcomp.at( jj ).s );
            outputStats( ts, valus, concs, true,
                  tr( "Sedimentation Coefficient" ) );
            valus.clear();

            for ( int jj = 0; jj < ksol; jj++ )
               valus.append( bcomp.at( jj ).d );
            outputStats( ts, valus, concs, true,
                  tr( "Diffusion Coefficient" ) );
            valus.clear();

            for ( int jj = 0; jj < ksol; jj++ )
               valus.append( bcomp.at( jj ).f );
            outputStats( ts, valus, concs, true,
                  tr( "Frictional Ratio" ) );
            valus.clear();

         }
      }

      fileo.close();
   }
   else
      rc    = 1;
   return rc;
}

// output statistics for an array of values of a given type
void US_SoluteData::outputStats( QTextStream& ts, QList< qreal >& vals,
      QList< qreal >& concs, bool details, QString title )
{
   QString str1;
   QString str2;
   int     nvals      = vals.size();
   int     nbins      = 50;
   qreal   vsiz       = (qreal)nvals;
   qreal   binsz      = 50.0;
   qreal   vlo        = 9.9e30;
   qreal   vhi        = -9.9e30;

   QVector< qreal > xpvec( qMax( nvals, nbins ) );
   QVector< qreal > ypvec( qMax( nvals, nbins ) );

   qreal   *xplot     = xpvec.data();
   qreal   *yplot     = ypvec.data();
   qreal   vsum       = 0.0;
   qreal   vm2        = 0.0;
   qreal   vm3        = 0.0;
   qreal   vm4        = 0.0;
   qreal   vmean;
   qreal   mode_cen;
   qreal   mode_lo;
   qreal   mode_hi;
   qreal   conf99lo;
   qreal   conf99hi;
   qreal   conf95lo;
   qreal   conf95hi;
   qreal   skew;
   qreal   kurto;
   qreal   vmedi;
   qreal   slope;
   qreal   vicep;
   qreal   sigma;
   qreal   corr;
   qreal   sdevi;
   qreal   sderr;
   qreal   vari;
   qreal   area;
   qreal   bininc;
   qreal   val;
   qreal   conc;
   qreal   vctot = 0.0;

   // get basic min,max,mean information

   for ( int jj = 0; jj < nvals; jj++ )
   {
      val       = vals.at( jj );
      conc      = concs.at( jj );
      vsum     += ( val * conc );
      vctot    += conc;
      vlo       = ( vlo < val ) ? vlo : val;
      vhi       = ( vhi > val ) ? vhi : val;
      xplot[jj] = (qreal)jj;
      yplot[jj] = val;
   }

   vmean     = vsum / vctot;

   // get difference information

   for ( int jj = 0; jj < nvals; jj++ )
   {
      val       = vals.at( jj );
      qreal dif = val - vmean;
      qreal dsq = dif * dif;
      vm2      += dsq;           // diff squared
      vm3      += ( dsq * dif ); // cubed
      vm4      += ( dsq * dsq ); // to the 4th
   }

   vm2      /= vsiz;
   vm3      /= vsiz;
   vm4      /= vsiz;
   skew      = vm3 / pow( vm2, 1.5 );
   kurto     = vm4 / pow( vm2, 2.0 ) - 3.0;
   vmedi     = ( vlo + vhi ) / 2.0;

   // do line fit (mainly for corr value)

   US_Math2::linefit( &xplot, &yplot, &slope, &vicep, &sigma, &corr, nvals );

   // standard deviation and error

   sdevi     = pow( vm2, 0.5 );
   sderr     = sdevi / pow( vsiz, 0.5 );
   vari      = vm2;
   area      = 0.0;

   bininc    = ( vhi - vlo ) / binsz;

   // mode and confidence

   for ( int ii = 0; ii < nbins; ii++ )
   {
      xplot[ii] = vlo + bininc * (qreal)ii;
      yplot[ii] = 0.0;

      for ( int jj = 0; jj < nvals; jj++ )
      {
         val       = vals.at( jj );

         if ( val >= xplot[ ii ]  &&  val < ( xplot[ ii ] + bininc ) )
         {
            yplot[ii] += ( concs.at( jj ) );
         }
      }

      area     += yplot[ ii ] * bininc;
   }

   val       = -1.0;
   int thisb = 0;

   for ( int ii = 0; ii < nbins; ii++ )
   {
      if ( yplot[ii] > val )
      {
         val       = yplot[ ii ];
         thisb     = ii;
      }
   }

   mode_lo   = xplot[ thisb ];
   mode_hi   = mode_lo + bininc;
   mode_cen  = ( mode_lo + mode_hi ) / 2.0;
   conf99lo  = vmean - 2.576 * sdevi;
   conf99hi  = vmean + 2.576 * sdevi;
   conf95lo  = vmean - 1.960 * sdevi;
   conf95hi  = vmean + 1.960 * sdevi;

   if ( details )
   {  // Details
      ts << "\n\n" << tr( "Results for the " ) << title << ":\n\n";
      ts << tr( "Maximum Value:             " ) 
         << str1.sprintf( "%6.4e\n", vhi   );
      ts << tr( "Minimum Value:             " ) 
         << str1.sprintf( "%6.4e\n", vlo   );
      ts << tr( "Mean Value:                " ) 
         << str1.sprintf( "%6.4e\n", vmean );
      ts << tr( "Median Value:              " ) 
         << str1.sprintf( "%6.4e\n", vmedi );
      ts << tr( "Skew Value:                " ) 
         << str1.sprintf( "%6.4e\n", skew  );
      ts << tr( "Kurtosis Value:            " ) 
         << str1.sprintf( "%6.4e\n", kurto );
      ts << tr( "Lower Mode Value:          " ) 
         << str1.sprintf( "%6.4e\n", mode_lo );
      ts << tr( "Upper Mode Value:          " ) 
         << str1.sprintf( "%6.4e\n", mode_hi );
      ts << tr( "Mode Center:               " ) 
         << str1.sprintf( "%6.4e\n", mode_cen );
      ts << tr( "95% Confidence Limits:     " ) 
         << str1.sprintf( "%6.4e, -%6.4e\n",
         ( conf95hi - mode_cen ), ( mode_cen - conf95lo ) );
      ts << tr( "99% Confidence Limits:     " ) 
         << str1.sprintf( "%6.4e, -%6.4e\n",
         ( conf99hi - mode_cen ), ( mode_cen - conf99lo ) );
      ts << tr( "Standard Deviation:        " ) 
         << str1.sprintf( "%6.4e\n", sdevi );
      ts << tr( "Standard Error:            " ) 
         << str1.sprintf( "%6.4e\n", sderr );
      ts << tr( "Variance:                  " ) 
         << str1.sprintf( "%6.4e\n", vari );
      ts << tr( "Correlation Coefficent:    " ) 
         << str1.sprintf( "%6.4e\n", corr );
      ts << tr( "Number of Bins:            " ) 
         << qRound( binsz ) << "\n";
      ts << tr( "Distribution Area:         " ) 
         << str1.sprintf( "%6.4e\n", area );

      str1.sprintf( "%e", conf95lo ).append( tr( " (low), " ) );
      str2.sprintf( "%e", conf95hi ).append( tr( " (high)\n" ) );
      ts << tr( "95% Confidence Interval:   " ) << str1 << str2;

      str1.sprintf( "%e", conf99lo ).append( tr( " (low), " ) );
      str2.sprintf( "%e", conf99hi ).append( tr( " (high)\n" ) );
      ts << tr( "99% Confidence Interval:   " ) << str1 << str2;
   }

   else
   {  // Summary
      ts << title << str1.sprintf( " %6.4e (%6.4e, %6.4e)\n",
            mode_cen, conf95lo, conf95hi );
   }
}

// Insure vertexes of a bucket do not exceed physically possible limits
void US_SoluteData::limitBucket( bucket& buk )
{
   buk.s_min   = buk.s_min >= 0.0 ?
                 max(  0.1, buk.s_min ) :
                 min( -0.1, buk.s_min );
   buk.s_max   = buk.s_max >= 0.0 ?
                 max(  0.1, buk.s_max ) :
                 min( -0.1, buk.s_max );
   buk.ff0_min = max(  1.0, buk.ff0_min );
}

// Count the number of overlaps in the current list of buckets
int US_SoluteData::countOverlaps()
{
   int nbuks  = allbucks.size();
   int novlps = 0;
   QList< QRectF > bucket_rects;

   // Create the list of bucket rectangles
   for ( int ii = 0; ii < nbuks; ii++ )
      bucket_rects << bucketRect( ii );

   // Count the number of overlaps
   for ( int ii = 0; ii < nbuks; ii++ )
   {
      QRectF bukrect = bucket_rects[ ii ];

      // Compare this bucket to all buckets that follow it
      for ( int jj = ii + 1; jj < nbuks; jj++ )
      {
         if ( bukrect.intersects( bucket_rects[ jj ] ) )
            novlps++;   // It intersects, so bump overlaps count
      }
   }

   return novlps;
}

