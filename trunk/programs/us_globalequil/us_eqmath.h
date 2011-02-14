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

      void   init_params          ( int, bool,
                                    QList< double >&, QList< double >& );
      void   init_fit             ( int, int, int&, int&, int& );
      int    calc_jacobian        ( void   );
      double calc_testParameter   ( double );
      double linesearch           ( void   );
      void   calc_B               ( void   );
      double calc_residuals       ( void   );
      void   genLeastSquaresOrd2  ( double**, int, double*, double** );
      void   guess_mapForward     ( QVector< double >& );
      void   parameter_mapBackward( QVector< double >& );
      bool   Cholesky_Invert      ( double**, double**, int );
      void   calc_A_transpose_A   ( double**, double**, int, int );
      void   calc_A_transpose_A   ( double**, double**, int, int, bool );
      void   column_array         ( double**, int, int, double* );

   private:
      QVector< US_DataIO2::EditedData >&  dataList;
      QVector< ScanEdit >&                scedits;
      QVector< EqScanFit >&               scanfits;
      EqRunFit&                           runfit;

      QVector< int >      v_setpts;   // Set points vector
      QVector< int >      v_setlpts;  // Set log points vector

      QVector< double >   v_yraw;     // Y raw values vector
      QVector< double >   v_yguess;   // Y guesses vector
      QVector< double >   v_ydelta;   // Y deltas vector
      QVector< double >   v_BB;       // B vector
      QVector< double >   v_guess;    // Guesses vector
      QVector< double >   v_tguess;   // Test guesses vector
      QVector< double >   v_jacobi;   // Jacobian matrix values vector
      QVector< double >   v_info;     // Information matrix values vector
      QVector< double >   v_LLtrns;   // LL transpose matrix values vector
      QVector< double >   v_dcr2;     // dcr2 matrix values vector
      QVector< double >   v_dlncr2;   // dlncr2 matrix values vector
      QVector< double >   v_lncr2;    // lncr2 matrix values vector

      QVector< double* >  m_jacobi;   // Jacobian matrix
      QVector< double* >  m_info;     // Information matrix
      QVector< double* >  m_LLtrns;   // LL transpose matrix
      QVector< double* >  m_dcr2;     // dcr2 matrix
      QVector< double* >  m_dlncr2;   // dlncr2 matrix
      QVector< double* >  m_lncr2;    // lncr2 matrix

      double*             setpts;     // Set points array
      double*             setlpts;    // Set log points array
      double*             y_raw;      // Y raw values array
      double*             y_guess;    // Y guesses array
      double*             y_delta;    // Y deltas array
      double*             BB;         // B array
      double*             guess;      // Guesses array
      double*             d_jacobi;   // Jacobian data array
      double*             d_info;     // Information data array
      double*             d_LLtrns;   // LL transpose data array
      double*             d_dcr2;     // dcr2 data array
      double*             d_dlncr2;   // dlncr2 data array
      double*             d_lncr2;    // lncr2 data array

      double**            jacobian;   // Jacobian matrix array
      double**            info;       // Information matrix array
      double**            LLtr;       // LL transpose matrix array
      double**            dcr2;       // dcr2 matrix array
      double**            dlncr2;     // dlncr2 matrix array
      double**            lncr2;      // lncr2 matrix array

      int      dbg_level;
      int      modelx;                // Model type index
      int      nlsmeth;               // NLS method index
      int      ntpts;                 // Number of total points
      int      ndsets;                // Number of data sets
      int      nfpars;                // Number of data sets
      int      ffitx;                 // First fit scan index
      int      nspts;                 // Number of set points
      int      nslpts;                // Number of set log points

   private slots:
      bool    Cholesky_DecompOrd2  ( double** );
      bool    Cholesky_SolveSysOrd2( double**, double* );
      bool    isNan( double );
};
#endif

