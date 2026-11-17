#include "qt_test_base.h"
#include "us_stiffbase.h"
#include <algorithm>
#include <cmath>

class US_StiffBaseTest : public QtTestBase
{
   protected:
      void SetUp() override
      {
         ASSERT_NO_FATAL_FAILURE( QtTestBase::SetUp() );
         stiffBase = std::make_unique<US_StiffBase>();
      }

      void TearDown() override
      {
         stiffBase.reset();
         QtTestBase::TearDown();
      }

      std::unique_ptr<US_StiffBase> stiffBase;

      // Helper function to create a simple test matrix
      double**                      createMatrix( int size )
      {
         double** matrix = new double*[size];
         for ( int i = 0; i < size; i++ )
         {
            matrix[i] = new double[size];
            for ( int j = 0; j < size; j++ )
            {
               matrix[i][j] = 0.0;
            }
         }
         return matrix;
      }

      // Helper function to clean up matrix
      void deleteMatrix( double** matrix, int size )
      {
         for ( int i = 0; i < size; i++ )
         {
            delete[] matrix[i];
         }
         delete[] matrix;
      }

      // Helper to check if matrix is symmetric within tolerance
      bool isMatrixSymmetric( double** matrix, int size, double tolerance = 1e-10 )
      {
         for ( int i = 0; i < size; i++ )
         {
            for ( int j = 0; j < size; j++ )
            {
               if ( std::abs( matrix[i][j] - matrix[j][i] ) > tolerance )
               {
                  return false;
               }
            }
         }
         return true;
      }
};

TEST_F( US_StiffBaseTest, TriangleWithoutSedimentationMatchesTheClosedForm )
{
   // Reference triangle with basis 1-x-y, x, y and sw2 = 0:
   // Stif[j][i] = d(phi_j)/dy * integral(x phi_i) + D d(phi_j)/dx d(phi_i)/dx integral(x),
   // with integral(x) = 1/6 and integral(x phi_i) = 1/24, 1/12, 1/24.
   double       xd[4][2] = { { 0.0, 0.0 }, { 1.0, 0.0 }, { 0.0, 1.0 }, { 0.0, 0.0 } };
   const double D        = 1.5;
   double**     stif     = createMatrix( 4 );
   for ( int i = 0; i < 4; i++ )
   {
      for ( int j = 0; j < 4; j++ )
      {
         stif[i][j] = 99.0;
      }
   }

   stiffBase->CompLocalStif( 3, xd, D, 0.0, stif );

   const double expected[3][3] = { { -1.0 / 24 + D / 6, -1.0 / 12 - D / 6, -1.0 / 24 },
                                   { -D / 6, D / 6, 0.0 },
                                   { 1.0 / 24, 1.0 / 12, 1.0 / 24 } };
   for ( int j = 0; j < 4; j++ )
   {
      for ( int i = 0; i < 4; i++ )
      {
         SCOPED_TRACE( ::testing::Message() << "Stif[" << j << "][" << i << "]" );
         // The triangle quadrature points and weights are tabulated to ten digits.
         EXPECT_NEAR( ( j < 3 && i < 3 ) ? expected[j][i] : 0.0, stif[j][i], 1e-9 );
      }
   }
   deleteMatrix( stif, 4 );
}

TEST_F( US_StiffBaseTest, ColumnsSumToZeroWithoutSedimentation )
{
   // The basis functions sum to one, so their derivatives sum to zero at every quadrature point.
   struct Element
   {
         const char* name;
         int         nk;
         double      xd[4][2];
   };
   const Element elements[] = { { "triangle", 3, { { 6.0, 0.0 }, { 6.01, 0.0 }, { 6.0, 2.0 }, { 0.0, 0.0 } } },
                                { "unit square", 4, { { 0.0, 0.0 }, { 1.0, 0.0 }, { 1.0, 1.0 }, { 0.0, 1.0 } } },
                                { "sector step", 4, { { 6.5, 0.0 }, { 6.505, 0.0 }, { 6.51, 30.0 }, { 6.502, 30.0 } } },
                                { "large square", 4, { { 0.0, 0.0 }, { 100.0, 0.0 }, { 100.0, 100.0 }, { 0.0, 100.0 } } } };
   double**      stif       = createMatrix( 4 );
   for ( const Element& e : elements )
   {
      SCOPED_TRACE( e.name );
      double xd[4][2];
      std::copy( &e.xd[0][0], &e.xd[0][0] + 8, &xd[0][0] );
      stiffBase->CompLocalStif( e.nk, xd, 6.0e-7, 0.0, stif );
      double scale = 0.0;
      for ( int j = 0; j < e.nk; j++ )
      {
         for ( int i = 0; i < e.nk; i++ )
         {
            ASSERT_TRUE( std::isfinite( stif[j][i] ) );
            scale = std::max( scale, std::abs( stif[j][i] ) );
         }
      }
      ASSERT_GT( scale, 0.0 );
      for ( int i = 0; i < e.nk; i++ )
      {
         double sum = 0.0;
         for ( int j = 0; j < e.nk; j++ )
         {
            sum += stif[j][i];
         }
         EXPECT_LT( std::abs( sum ), 1e-12 * scale ) << "column " << i;
      }
   }
   deleteMatrix( stif, 4 );
}

TEST_F( US_StiffBaseTest, StiffnessIsLinearInDiffusionAndSedimentation )
{
   double   xdT[4][2] = { { 6.0, 0.0 }, { 6.01, 0.0 }, { 6.0, 2.0 }, { 0.0, 0.0 } };
   double   xdQ[4][2] = { { 6.5, 0.0 }, { 6.505, 0.0 }, { 6.51, 30.0 }, { 6.502, 30.0 } };
   double** s0        = createMatrix( 4 );
   double** s1        = createMatrix( 4 );
   double** s2        = createMatrix( 4 );
   for ( int nk : { 3, 4 } )
   {
      auto& xd = ( nk == 3 ) ? xdT : xdQ;
      // Step D with sw2 fixed, then sw2 with D fixed: equal steps must change the matrix equally.
      for ( bool stepD : { true, false } )
      {
         SCOPED_TRACE( ::testing::Message() << "nk " << nk << ( stepD ? " D" : " sw2" ) );
         const double D = 6.0e-7, sw2 = 1.4e-5;
         stiffBase->CompLocalStif( nk, xd, stepD ? 0.0 : D, stepD ? sw2 : 0.0, s0 );
         stiffBase->CompLocalStif( nk, xd, D, sw2, s1 );
         stiffBase->CompLocalStif( nk, xd, stepD ? 2 * D : D, stepD ? sw2 : 2 * sw2, s2 );
         double step = 0.0, scale = 0.0;
         for ( int j = 0; j < nk; j++ )
         {
            for ( int i = 0; i < nk; i++ )
            {
               step  = std::max( step, std::abs( s1[j][i] - s0[j][i] ) );
               scale = std::max( scale, std::abs( s2[j][i] ) );
            }
         }
         ASSERT_GT( step, 0.0 ) << "the coefficient must change the matrix";
         for ( int j = 0; j < nk; j++ )
         {
            for ( int i = 0; i < nk; i++ )
            {
               EXPECT_NEAR( s2[j][i] - s1[j][i], s1[j][i] - s0[j][i], 1e-12 * scale );
            }
         }
      }
   }
   deleteMatrix( s0, 4 );
   deleteMatrix( s1, 4 );
   deleteMatrix( s2, 4 );
}

TEST_F( US_StiffBaseTest, EachCallReplacesThePreviousMatrix )
{
   double   xd[4][2] = { { 0.0, 0.0 }, { 1.0, 0.0 }, { 0.0, 1.0 }, { 0.0, 0.0 } };
   double** fresh    = createMatrix( 4 );
   double** reused   = createMatrix( 4 );

   stiffBase->CompLocalStif( 3, xd, 1.5, 0.5, fresh );
   for ( int j = 0; j < 4; j++ )
   {
      for ( int i = 0; i < 4; i++ )
      {
         reused[j][i] = -7.0;
      }
   }
   stiffBase->CompLocalStif( 3, xd, 1.5, 0.5, reused );

   for ( int j = 0; j < 4; j++ )
   {
      for ( int i = 0; i < 4; i++ )
      {
         EXPECT_EQ( fresh[j][i], reused[j][i] ) << "Stif[" << j << "][" << i << "]";
      }
   }
   deleteMatrix( fresh, 4 );
   deleteMatrix( reused, 4 );
}

TEST_F( US_StiffBaseTest, RectangleElementMatchesQuadrilateralQuadrature )
{
   // Fixed-mesh elements [x0,x1] x [0,dt]:  the closed form from reference
   // moments must match the Gauss quadrature of CompLocalStif( 4, ... ), with
   // the vertices ordered as in US_Astfem_RSA::ComputeCoefMatrixFixedMesh
   double** stifQ  = createMatrix( 4 );
   double** stifR  = createMatrix( 4 );
   double   maxerr = 0.0;

   for ( const double x0 : { 5.9, 6.5, 7.1 } )
   {
      for ( const double h : { 0.001, 0.0065, 0.05 } )
      {
         for ( const double dt : { 0.1, 2.0, 30.0, 300.0 } )
         {
            for ( const double D : { 1.0e-7, 6.0e-7 } )
            {
               for ( const double sw2 : { 0.0, 1.4e-5, 5.5e-5 } )
               {
                  double xd[4][2] = { { x0, 0.0 }, { x0 + h, 0.0 }, { x0 + h, dt }, { x0, dt } };
                  stiffBase->CompLocalStif( 4, xd, D, sw2, stifQ );
                  stiffBase->CompLocalStifRect( x0, x0 + h, dt, D, sw2, stifR );

                  double scale = 0.0;
                  for ( int i = 0; i < 4; i++ )
                  {
                     for ( int j = 0; j < 4; j++ )
                     {
                        scale = std::max( scale, std::abs( stifQ[i][j] ) );
                     }
                  }

                  for ( int i = 0; i < 4; i++ )
                  {
                     for ( int j = 0; j < 4; j++ )
                     {
                        maxerr = std::max( maxerr, std::abs( stifR[i][j] - stifQ[i][j] ) / scale );
                     }
                  }
               }
            }
         }
      }
   }

   EXPECT_LT( maxerr, 1e-10 ) << "Rectangle stiffness differs from quadrature";

   deleteMatrix( stifQ, 4 );
   deleteMatrix( stifR, 4 );
}
