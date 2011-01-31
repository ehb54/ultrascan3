#ifndef US_EQMATH_H
#define US_EQMATH_H

#include <QtCore>

#include "us_extern.h"
#include "us_dataIO2.h"
#include "us_matrix.h"
#include "us_math2.h"
#include "us_globeq_data.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

class US_EXTERN US_EqMath : public QObject
{
	Q_OBJECT
	
	public:
		US_EqMath( QVector< US_DataIO2::EditedData >&, 
            QVector< ScanEdit >&, QVector< EqScanFit >&, EqRunFit& );

      void   init_params        ( int, bool,
                                  QList< double >&, QList< double >& );
      double calc_testParameter ( double    );
      double minimum_residual   ( void      );
      void   genLeastSquaresOrd2( double**, int, double*, double** );

	private:
      QVector< US_DataIO2::EditedData >&  dataList;
      QVector< ScanEdit >&                scedits;
      QVector< EqScanFit >&               scanfits;
      EqRunFit&                           runfit;

      int      dbg_level;

   private slots:
      bool    Cholesky_DecompOrd2(   double** );
      bool    Cholesky_SolveSysOrd2( double**, double* );
      bool    isNan( double );
};
#endif

