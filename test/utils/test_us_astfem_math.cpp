// test_us_astfem_math.cpp - Simple working unit tests
#include "qt_test_base.h"
#include "us_astfem_math.h"
#include "us_model.h"
#include "us_simparms.h"
#include <vector>

using ::testing::DoubleEq;
using ::testing::DoubleNear;
using ::testing::Eq;

class US_AstfemMathTest : public QtTestBase {
protected:
    void SetUp() override {
        QtTestBase::SetUp();
    }

    void TearDown() override {
        QtTestBase::TearDown();
    }
};

// Test basic utility functions
TEST_F(US_AstfemMathTest, MaxvalBasic) {
QVector<double> values;
values << 1.0 << 3.0 << 2.0;
EXPECT_THAT(US_AstfemMath::maxval(values), DoubleEq(3.0));
}

TEST_F(US_AstfemMathTest, MinvalBasic) {
QVector<double> values;
values << 1.0 << 3.0 << 2.0;
EXPECT_THAT(US_AstfemMath::minval(values), DoubleEq(1.0));
}

TEST_F(US_AstfemMathTest, CalcBottom) {
double rpm = 50000.0;
double bottom_chan = 7.0;
double rotorcoefs[2] = {0.001, 0.000001};

double result = US_AstfemMath::calc_bottom(rpm, bottom_chan, rotorcoefs);
double expected = bottom_chan + rotorcoefs[0] * rpm + rotorcoefs[1] * rpm * rpm;
EXPECT_THAT(result, DoubleEq(expected));
}

// Test data structures can be created and used
TEST_F(US_AstfemMathTest, MfemInitialUsage) {
US_AstfemMath::MfemInitial initial;
initial.radius << 6.0 << 7.0;
initial.concentration << 1.0 << 0.5;

EXPECT_THAT(initial.radius.size(), Eq(2));
EXPECT_THAT(initial.concentration.size(), Eq(2));
}

TEST_F(US_AstfemMathTest, MfemScanUsage) {
US_AstfemMath::MfemScan scan;
scan.time = 1800.0;
scan.rpm = 50000.0;

EXPECT_THAT(scan.time, DoubleEq(1800.0));
EXPECT_THAT(scan.rpm, DoubleEq(50000.0));
}

// Test matrix operations
TEST_F(US_AstfemMathTest, Matrix2DBasic) {
double** matrix = nullptr;

US_AstfemMath::initialize_2d(2, 2, &matrix);

ASSERT_NE(matrix, nullptr);
matrix[0][0] = 5.0;
EXPECT_THAT(matrix[0][0], DoubleEq(5.0));

US_AstfemMath::clear_2d(2, matrix);
}

// Test tridiagonal solver with simple case
TEST_F(US_AstfemMathTest, TridiagSimple) {
int N = 3;
std::vector<double> a_vec(N, 0.0);  // sub-diagonal
std::vector<double> b_vec(N, 1.0);  // main diagonal
std::vector<double> c_vec(N, 0.0);  // super-diagonal
std::vector<double> r_vec{1.0, 2.0, 3.0};  // RHS
std::vector<double> u_vec(N);

US_AstfemMath::tridiag(a_vec.data(), b_vec.data(), c_vec.data(),
        r_vec.data(), u_vec.data(), N);

// Identity system should give u = r
EXPECT_THAT(u_vec[0], DoubleNear(1.0, 1e-10));
EXPECT_THAT(u_vec[1], DoubleNear(2.0, 1e-10));
EXPECT_THAT(u_vec[2], DoubleNear(3.0, 1e-10));
}

// Test Gaussian elimination with simple 2x2 system
TEST_F(US_AstfemMathTest, GaussElimSimple) {
int n = 2;
double** A = nullptr;
US_AstfemMath::initialize_2d(n, n, &A);
std::vector<double> b_vec{7.0, 3.0};

// 2x + 3y = 7, x + y = 3 -> solution: x=2, y=1
A[0][0] = 2.0; A[0][1] = 3.0;
A[1][0] = 1.0; A[1][1] = 1.0;

int result = US_AstfemMath::GaussElim(n, A, b_vec.data());

EXPECT_THAT(result, Eq(1));
EXPECT_THAT(b_vec[0], DoubleNear(2.0, 1e-10));
EXPECT_THAT(b_vec[1], DoubleNear(1.0, 1e-10));

US_AstfemMath::clear_2d(n, A);
}

// Test cube root with known case
TEST_F(US_AstfemMathTest, CubeRootSimple) {
// x^3 - 8 = 0, root = 2
double result = US_AstfemMath::cube_root(-8.0, 0.0, 0.0);
EXPECT_THAT(result, DoubleNear(2.0, 1e-10));
}

// Test interpolation
TEST_F(US_AstfemMathTest, InterpolateSimple) {
US_AstfemMath::MfemInitial C0, C1;

C0.radius << 6.0 << 8.0;
C0.concentration << 1.0 << 0.0;

C1.radius << 7.0;  // midpoint
C1.concentration.resize(1);

US_AstfemMath::interpolate_C0(C0, C1);
EXPECT_THAT(C1.concentration[0], DoubleNear(0.5, 1e-10));
}

// Test low acceleration with realistic values
TEST_F(US_AstfemMathTest, LowAccelerationRealistic) {
QVector<US_SimulationParameters::SpeedProfile> speedsteps(1);

speedsteps[0].rotorspeed = 30000;
speedsteps[0].time_first = 1800.0;
speedsteps[0].w2t_first = 450000.0;

double rate;
bool result = US_AstfemMath::low_acceleration(speedsteps, 250.0, rate);

// Just verify function completes and sets rate
EXPECT_GE(rate, 0.0);
EXPECT_FALSE(std::isnan(rate));
}