//! \file us_solutedata.h
#ifndef US_SOLUTEDATA_H
#define US_SOLUTEDATA_H

#include "us_spectrodata.h"

typedef struct short_sim_comp_s
{
   double s;    // sedimentation coefficient
   double w;    // molecular weight
   double f;    // frictional ratio
   double c;    // concentration
   double d;    // diffusion coefficient
   double v;    // vbar20
} SimComp;

//! \brief Store and retrieve solution data

class bucket
{
   public:
      double x;
      double x_min;
      double x_max;
      double y;
      double y_max;
      double y_min;
      double conc;
      int    status; // 0 = full-sized bucket, 
                     // 1 = this bucket is reduced from overlap
                     // 2 = newly added bucket
      bucket() {};
      ~bucket() {};
      bool operator==(const bucket& objIn) 
      {
         return ( x == objIn.x && x_min == objIn.x_min && x_max == objIn.x_max 
               && y == objIn.y && y_min == objIn.y_min && y_max == objIn.y_max 
               && conc == objIn.conc && status == objIn.status );
      }
      bool operator!=(const bucket& objIn)
      {
         return ( x != objIn.x || x_min != objIn.x_min || x_max != objIn.x_max 
               || y != objIn.y || y_min != objIn.y_min || y_max != objIn.y_max 
               || conc != objIn.conc || status != objIn.status );
      }
      bool operator < (const bucket& objIn) const
      {
         if ( x < objIn.x )
         {
            return ( true );
         }
         else if ( x == objIn.x && y < objIn.y )
         {
            return ( true );
         }
         else
         {
            return ( false );
         }
      }
};

//! \brief Bucket vertex less than routine.
bool buck_vx_lessthan( const bucket&, const bucket& );

/*! \class US_SoluteData

  Provides functions to launch a web browser for external links or
  Qt Assistant for local help files.
*/

class US_SoluteData : public QObject
{
  Q_OBJECT

  public:
    /*! \brief Create data object
    */
    US_SoluteData( void );

  public slots:
    int      clearBuckets();
    int      sortBuckets();
    int      sortBuckets( QList< bucket >* );
    int      appendBucket( bucket& );
    int      appendBucket( QRectF, QPointF, qreal, int );
    int      appendBucket( QRectF, QPointF, qreal );
    int      setBucket(    int, QRectF, QPointF, qreal, int );
    int      bucketsCount();
    bucket   createBucket( QRectF, QPointF, qreal, int );
    bucket   minBucket();
    bucket   maxBucket();
    bucket   bucketAt(    int );
    bucket   firstBucket();
    bucket   lastBucket();
    bucket   nextBucket();
    bucket   currentBucket();
    QRectF   bucketRect(  int );
    QPointF  bucketPoint( int, bool );
    QPointF  bucketPoint( int );
    QSizeF   bucketSize(  int );
    QString  bucketLine(  int );
    void     setDistro( QList< S_Solute >*, int, int, int );
    int      findNearestPoint( QPointF& );
    int      removeBucketAt( int );
    int      autoCalcBins( int, qreal, qreal );
    int      saveGAdata  ( QString&, int = 0, int = 1, int = 3, double = 0.0 );
    int      loadGAdata  ( QString&, int* = NULL, int* = NULL, int* = NULL,
                           double* = NULL );
    int      buildDataMC ( void );
    int      reportDataMC( QString&, int );
    void     outputStats( QTextStream&, QList< qreal >&, QList< qreal >&,
                          bool, QString ); 
    void     limitBucket( bucket& );
    int      countOverlaps();
    int      countFullestBucket();
    void     bucketSeparate( int, int, QList< QRectF >& );

  private:
    QList< bucket >            allbucks;
    QList< SimComp >           component;
    QList< QList< SimComp > >  MC_solute;
    QList< S_Solute >*         distro;

    int      bndx;
    int      attr_x;
    int      attr_y;
    int      attr_z;
    int      dbg_level;

    QRectF   brecmin;
    QRectF   brecmax;

    QPointF  bpntmin; 
    QPointF  bpntmax; 

    QSizeF   bsizmin; 
    QSizeF   bsizmax; 

    qreal    bconmin;
    qreal    bconmax;
};

#endif
