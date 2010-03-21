//! \file us_solutedata.h
#ifndef US_SOLDAT_H
#define US_SOLDAT_H

#include "us_femglobal.h"
#include "us_spectrodata.h"

typedef struct SimulationComponent SimComp;
typedef QList< SimComp >           SimCompList;
typedef QList< SimCompList >       SimCompLL;
typedef QList< Solute >            SoluteList;

//! \brief Store and retrieve solution data

class bucket
{
   public:
      double s;
      double s_min;
      double s_max;
      double ff0;
      double ff0_max;
      double ff0_min;
      double conc;
      int    status; // 0 = full-sized bucket, 
                     // 1 = this bucket is reduced from overlap
                     // 2 = newly added bucket
      bucket() {};
      ~bucket() {};
      bool operator==(const bucket& objIn) 
      {
         return (s == objIn.s && s_min == objIn.s_min && s_max == objIn.s_max 
            && ff0 == objIn.ff0 && ff0_min == objIn.ff0_min && ff0_max == objIn.ff0_max 
            && conc == objIn.conc && status == objIn.status);
      }
      bool operator!=(const bucket& objIn)
      {
         return (s != objIn.s || s_min != objIn.s_min || s_max != objIn.s_max 
            || ff0 != objIn.ff0 || ff0_min != objIn.ff0_min || ff0_max != objIn.ff0_max 
            || conc != objIn.conc || status != objIn.status);
      }
      bool operator < (const bucket& objIn) const
      {
         if (s < objIn.s)
         {
            return (true);
         }
         else if (s == objIn.s && ff0 < objIn.ff0)
         {
            return(true);
         }
         else
         {
            return(false);
         }
      }
};

/*! \class US_SoluteData

  Provides functions to launch a web browser for external links or
  Qt Assistant for local help files.
*/

class US_SoluteData : public QObject
{
  Q_OBJECT

  public:
    /*! \brief Create data object and set a title for it.
        \param title A title to associate with the data object.
    */
    US_SoluteData( const QString& );
    US_SoluteData( void );

  public slots:
    int      clearBuckets();
    int      sortBuckets();
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
    QPointF  bucketPoint( int );
    QSizeF   bucketSize(  int );
    QString  bucketLine(  int );
    void     setDistro( SoluteList* );
    int      findNearestPoint( QPointF& );
    int      removeBucketAt( int );

  private:
    QList< bucket >  allbucks;
    SimCompList      component;
    SimCompLL        MC_solute;
    SoluteList*      distro;

    QString  btitle;

    int      bndx;

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
