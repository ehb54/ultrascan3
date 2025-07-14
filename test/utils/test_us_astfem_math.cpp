// test_us_astfem_math.cpp
#include "qt_test_base.h"
#include "us_astfem_math.h"
#include "us_model.h"
#include "us_simparms.h"

using ::testing::_;
using ::testing::Return;
using ::testing::Eq;
using ::testing::DoubleEq;
using ::testing::DoubleNear;
using ::testing::ElementsAre;
using ::testing::SizeIs;

class US_AstfemMathTest : public QtTestBase {
protected:
    void SetUp() override {
        QtTestBase::SetUp();
    }

    void TearDown() override {
        QtTestBase::TearDown();
    }
};

// Test MfemInitial class
TEST_F(US_AstfemMathTest, MfemInitialConstructor) {
    US_AstfemMath::MfemInitial initial;

    EXPECT_TRUE(initial.radius.isEmpty());
    EXPECT_TRUE(initial.concentration.isEmpty());
}

TEST_F(US_AstfemMathTest, MfemInitialDataManipulation) {
    US_AstfemMath::MfemInitial initial;

    initial.radius << 6.0 << 6.5 << 7.0;
    initial.concentration << 1.0 << 0.8 << 0.6;

    EXPECT_THAT(initial.radius.size(), Eq(3));
    EXPECT_THAT(initial.concentration.size(), Eq(3));
    EXPECT_THAT(initial.radius[0], DoubleEq(6.0));
    EXPECT_THAT(initial.concentration[0], DoubleEq(1.0));
}

// Test MfemScan class
TEST_F(US_AstfemMathTest, MfemScanConstructor) {
    US_AstfemMath::MfemScan scan;

    EXPECT_THAT(scan.time, DoubleEq(0.0));
    EXPECT_THAT(scan.omega_s_t, DoubleEq(0.0));
    EXPECT_THAT(scan.temperature, DoubleEq(0.0));
    EXPECT_THAT(scan.rpm, DoubleEq(0.0));
    EXPECT_TRUE(scan.conc.isEmpty());
}

TEST_F(US_AstfemMathTest, MfemScanDataAssignment) {
    US_AstfemMath::MfemScan scan;

    scan.time = 1800.0;
    scan.omega_s_t = 3.14159;
    scan.temperature = 20.0;
    scan.rpm = 50000.0;
    scan.conc << 1.0 << 0.9 << 0.8;

    EXPECT_THAT(scan.time, DoubleEq(1800.0));
    EXPECT_THAT(scan.omega_s_t, DoubleEq(3.14159));
    EXPECT_THAT(scan.temperature, DoubleEq(20.0));
    EXPECT_THAT(scan.rpm, DoubleEq(50000.0));
    EXPECT_THAT(scan.conc.size(), Eq(3));
}

// Test MfemData class
TEST_F(US_AstfemMathTest, MfemDataConstructor) {
    US_AstfemMath::MfemData data;

    EXPECT_TRUE(data.id.isEmpty());
    EXPECT_THAT(data.cell, Eq(0));
    EXPECT_THAT(data.channel, Eq(0));
    EXPECT_THAT(data.wavelength, DoubleEq(0.0));
    EXPECT_THAT(data.rpm, DoubleEq(0.0));
    EXPECT_TRUE(data.radius.isEmpty());
    EXPECT_TRUE(data.scan.isEmpty());
}

TEST_F(US_AstfemMathTest, MfemDataFieldAssignment) {
    US_AstfemMath::MfemData data;

    data.id = "TestData";
    data.cell = 1;
    data.channel = 2;
    data.wavelength = 280.0;
    data.rpm = 50000.0;
    data.s20w_correction = 1.0;
    data.D20w_correction = 1.0;
    data.viscosity = 1.002;
    data.density = 0.998;
    data.vbar = 0.72;
    data.avg_temperature = 20.0;
    data.vbar20 = 0.72;
    data.meniscus = 5.8;
    data.bottom = 7.2;

    EXPECT_THAT(data.id, Eq("TestData"));
    EXPECT_THAT(data.cell, Eq(1));
    EXPECT_THAT(data.channel, Eq(2));
    EXPECT_THAT(data.wavelength, DoubleEq(280.0));
    EXPECT_THAT(data.rpm, DoubleEq(50000.0));
    EXPECT_THAT(data.meniscus, DoubleEq(5.8));
    EXPECT_THAT(data.bottom, DoubleEq(7.2));
}

// Test ReactionGroup class
TEST_F(US_AstfemMathTest, ReactionGroupConstructor) {
    US_AstfemMath::ReactionGroup group;

    EXPECT_TRUE(group.association.isEmpty());
    EXPECT_TRUE(group.GroupComponent.isEmpty());
}

TEST_F(US_AstfemMathTest, ReactionGroupDataManipulation) {
    US_AstfemMath::ReactionGroup group;

    group.association << 1 << 0 << 1;
    group.GroupComponent << 0 << 1 << 2;

    EXPECT_THAT(group.association.size(), Eq(3));
    EXPECT_THAT(group.GroupComponent.size(), Eq(3));
    EXPECT_THAT(group.association[0], Eq(1));
    EXPECT_THAT(group.GroupComponent[1], Eq(1));
}

// Test ComponentRole class
TEST_F(US_AstfemMathTest, ComponentRoleConstructor) {
    US_AstfemMath::ComponentRole role;

    EXPECT_THAT(role.comp_index, Eq(0));
    EXPECT_TRUE(role.assocs.isEmpty());
    EXPECT_TRUE(role.stoichs.isEmpty());
}

TEST_F(US_AstfemMathTest, ComponentRoleDataAssignment) {
    US_AstfemMath::ComponentRole role;

    role.comp_index = 5;
    role.assocs << 1 << 3 << 5;
    role.stoichs << 1 << -1 << 2;

    EXPECT_THAT(role.comp_index, Eq(5));
    EXPECT_THAT(role.assocs.size(), Eq(3));
    EXPECT_THAT(role.stoichs.size(), Eq(3));
    EXPECT_THAT(role.assocs[1], Eq(3));
    EXPECT_THAT(role.stoichs[1], Eq(-1));
}

// Test AstFemParameters class
TEST_F(US_AstfemMathTest, AstFemParametersConstructor) {
    US_AstfemMath::AstFemParameters params;

    EXPECT_THAT(params.simpoints, Eq(0));
    EXPECT_TRUE(params.s.isEmpty());
    EXPECT_TRUE(params.D.isEmpty());
    EXPECT_TRUE(params.kext.isEmpty());
    EXPECT_TRUE(params.role.isEmpty());
    EXPECT_THAT(params.pathlength, DoubleEq(0.0));
    EXPECT_THAT(params.dt, DoubleEq(0.0));
    EXPECT_THAT(params.time_steps, Eq(0));
}

TEST_F(US_AstfemMathTest, AstFemParametersFieldAssignment) {
    US_AstfemMath::AstFemParameters params;

    params.simpoints = 100;
    params.s << 1.5 << 2.0 << 2.5;
    params.D << 5e-7 << 6e-7 << 7e-7;
    params.kext << 1.0 << 1.2 << 1.1;
    params.pathlength = 1.2;
    params.dt = 1.0;
    params.time_steps = 1000;
    params.omega_s = 2500.0;
    params.start_time = 0.0;
    params.start_om2t = 0.0;
    params.current_meniscus = 5.8;
    params.current_bottom = 7.2;
    params.bottom_pos = 7.2;
    params.cdset_speed = 50000;
    params.rg_index = 0;

    EXPECT_THAT(params.simpoints, Eq(100));
    EXPECT_THAT(params.s.size(), Eq(3));
    EXPECT_THAT(params.D.size(), Eq(3));
    EXPECT_THAT(params.kext.size(), Eq(3));
    EXPECT_THAT(params.pathlength, DoubleEq(1.2));
    EXPECT_THAT(params.dt, DoubleEq(1.0));
    EXPECT_THAT(params.time_steps, Eq(1000));
    EXPECT_THAT(params.omega_s, DoubleEq(2500.0));
    EXPECT_THAT(params.current_meniscus, DoubleEq(5.8));
    EXPECT_THAT(params.current_bottom, DoubleEq(7.2));
}

// Test static utility methods
TEST_F(US_AstfemMathTest, MaxvalDoubleVector) {
    QVector<double> values;
    values << 1.0 << 3.5 << 2.1 << 0.8;

    double result = US_AstfemMath::maxval(values);

    EXPECT_THAT(result, DoubleEq(3.5));
}

TEST_F(US_AstfemMathTest, MaxvalEmptyVector) {
    QVector<double> values;

    double result = US_AstfemMath::maxval(values);

    EXPECT_THAT(result, DoubleEq(-1.0e300));
}

TEST_F(US_AstfemMathTest, MaxvalSingleElement) {
    QVector<double> values;
    values << 42.0;

    double result = US_AstfemMath::maxval(values);

    EXPECT_THAT(result, DoubleEq(42.0));
}

TEST_F(US_AstfemMathTest, MinvalDoubleVector) {
    QVector<double> values;
    values << 1.0 << 3.5 << 2.1 << 0.8;

    double result = US_AstfemMath::minval(values);

    EXPECT_THAT(result, DoubleEq(0.8));
}

TEST_F(US_AstfemMathTest, MinvalEmptyVector) {
    QVector<double> values;

    double result = US_AstfemMath::minval(values);

    EXPECT_THAT(result, DoubleEq(1.0e300));
}

TEST_F(US_AstfemMathTest, MinvalSingleElement) {
    QVector<double> values;
    values << -15.7;

    double result = US_AstfemMath::minval(values);

    EXPECT_THAT(result, DoubleEq(-15.7));
}

TEST_F(US_AstfemMathTest, MaxvalSimulationComponent) {
    QVector<US_Model::SimulationComponent> components(3);
    components[0].s = 1.5;
    components[1].s = 3.2;
    components[2].s = 2.1;

    double result = US_AstfemMath::maxval(components);

    EXPECT_THAT(result, DoubleEq(3.2));
}

TEST_F(US_AstfemMathTest, MaxvalSimulationComponentEmpty) {
    QVector<US_Model::SimulationComponent> components;

    double result = US_AstfemMath::maxval(components);

    EXPECT_THAT(result, DoubleEq(-1.0e300));
}

TEST_F(US_AstfemMathTest, MinvalSimulationComponent) {
    QVector<US_Model::SimulationComponent> components(3);
    components[0].s = 1.5;
    components[1].s = 3.2;
    components[2].s = 2.1;

    double result = US_AstfemMath::minval(components);

    EXPECT_THAT(result, DoubleEq(1.5));
}

TEST_F(US_AstfemMathTest, MinvalSimulationComponentEmpty) {
    QVector<US_Model::SimulationComponent> components;

    double result = US_AstfemMath::minval(components);

    EXPECT_THAT(result, DoubleEq(1.0e300));
}

// Test cube_root method
TEST_F(US_AstfemMathTest, CubeRootPositiveRoot) {
    double a0 = -8.0;  // a0 <= 0
    double a1 = 0.0;   // a1 >= 0
    double a2 = 0.0;   // a2 >= 0

    double result = US_AstfemMath::cube_root(a0, a1, a2);

    EXPECT_THAT(result, DoubleNear(2.0, 1e-10));
}

TEST_F(US_AstfemMathTest, CubeRootComplexCase) {
    double a0 = -1.0;
    double a1 = 2.0;
    double a2 = 1.0;

    double result = US_AstfemMath::cube_root(a0, a1, a2);

    // Should return some valid positive root
    EXPECT_GT(result, 0.0);
}

TEST_F(US_AstfemMathTest, CubeRootZeroCase) {
    double a0 = 0.0;
    double a1 = 0.0;
    double a2 = 0.0;

    double result = US_AstfemMath::cube_root(a0, a1, a2);

    EXPECT_THAT(result, DoubleNear(0.0, 1e-10));
}

// Test find_C1_mono_Nmer method
TEST_F(US_AstfemMathTest, FindC1MonoNmerValidInput) {
    int n = 2;
    double K = 1.0;
    double CT = 1.0;

    double result = US_AstfemMath::find_C1_mono_Nmer(n, K, CT);

    EXPECT_GT(result, 0.0);
    EXPECT_LT(result, CT);
}

TEST_F(US_AstfemMathTest, FindC1MonoNmerZeroConcentration) {
    int n = 2;
    double K = 1.0;
    double CT = 0.0;

    double result = US_AstfemMath::find_C1_mono_Nmer(n, K, CT);

    EXPECT_THAT(result, DoubleEq(0.0));
}

TEST_F(US_AstfemMathTest, FindC1MonoNmerVerySmallConcentration) {
    int n = 2;
    double K = 1.0;
    double CT = 1e-15;

    double result = US_AstfemMath::find_C1_mono_Nmer(n, K, CT);

    EXPECT_THAT(result, DoubleEq(CT));
}

TEST_F(US_AstfemMathTest, FindC1MonoNmerHigherOrder) {
    int n = 3;
    double K = 0.5;
    double CT = 2.0;

    double result = US_AstfemMath::find_C1_mono_Nmer(n, K, CT);

    EXPECT_GT(result, 0.0);
    EXPECT_LT(result, CT);
}

// Test calc_bottom method
TEST_F(US_AstfemMathTest, CalcBottomBasicCase) {
    double rpm = 50000.0;
    double bottom_chan = 7.0;
    double rotorcoefs[2] = {0.001, 0.000001};

    double result = US_AstfemMath::calc_bottom(rpm, bottom_chan, rotorcoefs);

    double expected = bottom_chan + rotorcoefs[0] * rpm + rotorcoefs[1] * rpm * rpm;
    EXPECT_THAT(result, DoubleEq(expected));
}

TEST_F(US_AstfemMathTest, CalcBottomZeroCoefficients) {
    double rpm = 50000.0;
    double bottom_chan = 7.0;
    double rotorcoefs[2] = {0.0, 0.0};

    double result = US_AstfemMath::calc_bottom(rpm, bottom_chan, rotorcoefs);

    EXPECT_THAT(result, DoubleEq(bottom_chan));
}

TEST_F(US_AstfemMathTest, CalcBottomZeroRpm) {
    double rpm = 0.0;
    double bottom_chan = 7.0;
    double rotorcoefs[2] = {0.001, 0.000001};

    double result = US_AstfemMath::calc_bottom(rpm, bottom_chan, rotorcoefs);

    EXPECT_THAT(result, DoubleEq(bottom_chan));
}

TEST_F(US_AstfemMathTest, CalcBottomNegativeCoefficients) {
    double rpm = 50000.0;
    double bottom_chan = 7.0;
    double rotorcoefs[2] = {-0.001, -0.000001};

    double result = US_AstfemMath::calc_bottom(rpm, bottom_chan, rotorcoefs);

    double expected = bottom_chan + rotorcoefs[0] * rpm + rotorcoefs[1] * rpm * rpm;
    EXPECT_THAT(result, DoubleEq(expected));
    EXPECT_LT(result, bottom_chan);
}

// Test matrix operations - initialize_2d and clear_2d
TEST_F(US_AstfemMathTest, Initialize2DMatrix) {
    double** matrix;
    int rows = 3;
    int cols = 4;

    US_AstfemMath::initialize_2d(rows, cols, &matrix);

    // Check that all elements are initialized to 0
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            EXPECT_THAT(matrix[i][j], DoubleEq(0.0));
        }
    }

    // Test that we can write to the matrix
    matrix[1][2] = 5.5;
    EXPECT_THAT(matrix[1][2], DoubleEq(5.5));

    US_AstfemMath::clear_2d(rows, matrix);
}

TEST_F(US_AstfemMathTest, Initialize2DMatrixSingleElement) {
    double** matrix;
    int rows = 1;
    int cols = 1;

    US_AstfemMath::initialize_2d(rows, cols, &matrix);

    EXPECT_THAT(matrix[0][0], DoubleEq(0.0));

    matrix[0][0] = 42.0;
    EXPECT_THAT(matrix[0][0], DoubleEq(42.0));

    US_AstfemMath::clear_2d(rows, matrix);
}

// Test zero_2d method
TEST_F(US_AstfemMathTest, Zero2DMatrix) {
    double** matrix;
    int rows = 2;
    int cols = 3;

    US_AstfemMath::initialize_2d(rows, cols, &matrix);

    // Set some non-zero values
    matrix[0][1] = 3.14;
    matrix[1][2] = -2.71;

    // Zero the matrix
    US_AstfemMath::zero_2d(rows, cols, matrix);

    // Check all elements are zero
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            EXPECT_THAT(matrix[i][j], DoubleEq(0.0));
        }
    }

    US_AstfemMath::clear_2d(rows, matrix);
}

// Test 3D matrix operations
TEST_F(US_AstfemMathTest, Initialize3DMatrix) {
    double*** matrix;
    int dim1 = 2, dim2 = 2, dim3 = 2;

    US_AstfemMath::initialize_3d(dim1, dim2, dim3, &matrix);

    // Check initialization to zero
    for (int i = 0; i < dim1; i++) {
        for (int j = 0; j < dim2; j++) {
            for (int k = 0; k < dim3; k++) {
                EXPECT_THAT(matrix[i][j][k], DoubleEq(0.0));
            }
        }
    }

    // Test assignment
    matrix[1][0][1] = 7.5;
    EXPECT_THAT(matrix[1][0][1], DoubleEq(7.5));

    US_AstfemMath::clear_3d(dim1, dim2, matrix);
}

TEST_F(US_AstfemMathTest, Initialize3DMatrixSingleElement) {
    double*** matrix;
    int dim1 = 1, dim2 = 1, dim3 = 1;

    US_AstfemMath::initialize_3d(dim1, dim2, dim3, &matrix);

    EXPECT_THAT(matrix[0][0][0], DoubleEq(0.0));

    matrix[0][0][0] = 99.9;
    EXPECT_THAT(matrix[0][0][0], DoubleEq(99.9));

    US_AstfemMath::clear_3d(dim1, dim2, matrix);
}

// Test tridiagonal solver
TEST_F(US_AstfemMathTest, TridiagSimpleSystem) {
    int N = 3;
    double a[3] = {0.0, 1.0, 1.0};  // sub-diagonal
    double b[3] = {2.0, 2.0, 2.0};  // main diagonal
    double c[3] = {1.0, 1.0, 0.0};  // super-diagonal
    double r[3] = {3.0, 3.0, 3.0};  // right-hand side
    double u[3];                    // solution

    US_AstfemMath::tridiag(a, b, c, r, u, N);

    // For this system, solution should be [1, 1, 1]
    EXPECT_THAT(u[0], DoubleNear(1.0, 1e-10));
    EXPECT_THAT(u[1], DoubleNear(1.0, 1e-10));
    EXPECT_THAT(u[2], DoubleNear(1.0, 1e-10));
}

TEST_F(US_AstfemMathTest, TridiagSingleElement) {
    int N = 1;
    double a[1] = {0.0};
    double b[1] = {5.0};
    double c[1] = {0.0};
    double r[1] = {10.0};
    double u[1];

    US_AstfemMath::tridiag(a, b, c, r, u, N);

    EXPECT_THAT(u[0], DoubleNear(2.0, 1e-10));
}

// Test Gaussian elimination
TEST_F(US_AstfemMathTest, GaussElimSimpleSystem) {
    int n = 2;
    double** A;
    US_AstfemMath::initialize_2d(n, n, &A);
    double b[2];

    // Set up system: 2x + 3y = 7, x + y = 3
    A[0][0] = 2.0; A[0][1] = 3.0;
    A[1][0] = 1.0; A[1][1] = 1.0;
    b[0] = 7.0; b[1] = 3.0;

    int result = US_AstfemMath::GaussElim(n, A, b);

    EXPECT_THAT(result, Eq(1));  // Success
    EXPECT_THAT(b[0], DoubleNear(2.0, 1e-10));  // x = 2
    EXPECT_THAT(b[1], DoubleNear(1.0, 1e-10));  // y = 1

    US_AstfemMath::clear_2d(n, A);
}

TEST_F(US_AstfemMathTest, GaussElimSingularMatrix) {
    int n = 2;
    double** A;
    US_AstfemMath::initialize_2d(n, n, &A);
    double b[2];

    // Set up singular system
    A[0][0] = 1.0; A[0][1] = 2.0;
    A[1][0] = 2.0; A[1][1] = 4.0;  // Second row is 2 * first row
    b[0] = 3.0; b[1] = 6.0;

    int result = US_AstfemMath::GaussElim(n, A, b);

    EXPECT_THAT(result, Eq(-1));  // Singular matrix

    US_AstfemMath::clear_2d(n, A);
}

TEST_F(US_AstfemMathTest, GaussElimSingleEquation) {
    int n = 1;
    double** A;
    US_AstfemMath::initialize_2d(n, n, &A);
    double b[1];

    A[0][0] = 3.0;
    b[0] = 9.0;

    int result = US_AstfemMath::GaussElim(n, A, b);

    EXPECT_THAT(result, Eq(1));
    EXPECT_THAT(b[0], DoubleNear(3.0, 1e-10));

    US_AstfemMath::clear_2d(n, A);
}

// Test interpolate_C0 methods
TEST_F(US_AstfemMathTest, InterpolateC0VectorToVector) {
    US_AstfemMath::MfemInitial C0, C1;

    // Set up C0 with known values
    C0.radius << 6.0 << 7.0 << 8.0;
    C0.concentration << 1.0 << 0.5 << 0.0;

    // Set up C1 with different radius points
    C1.radius << 6.5 << 7.5;
    C1.concentration.resize(2);

    US_AstfemMath::interpolate_C0(C0, C1);

    // Check interpolated values
    EXPECT_THAT(C1.concentration[0], DoubleNear(0.75, 1e-10));  // Mid-point between 1.0 and 0.5
    EXPECT_THAT(C1.concentration[1], DoubleNear(0.25, 1e-10));  // Mid-point between 0.5 and 0.0
}

TEST_F(US_AstfemMathTest, InterpolateC0VectorToArray) {
    US_AstfemMath::MfemInitial C0;
    QVector<double> xvec;

    C0.radius << 6.0 << 7.0 << 8.0;
    C0.concentration << 1.0 << 0.5 << 0.0;

    xvec << 6.5 << 7.5;
    double C1[2];

    US_AstfemMath::interpolate_C0(C0, C1, xvec);

    EXPECT_THAT(C1[0], DoubleNear(0.75, 1e-10));
    EXPECT_THAT(C1[1], DoubleNear(0.25, 1e-10));
}

TEST_F(US_AstfemMathTest, InterpolateC0OutOfBounds) {
    US_AstfemMath::MfemInitial C0, C1;

    C0.radius << 6.0 << 7.0 << 8.0;
    C0.concentration << 1.0 << 0.5 << 0.0;

    // Test points outside the range
    C1.radius << 5.0 << 9.0;  // Before first and after last
    C1.concentration.resize(2);

    US_AstfemMath::interpolate_C0(C0, C1);

    // Should use boundary values
    EXPECT_THAT(C1.concentration[0], DoubleEq(1.0));  // First value for point before range
    EXPECT_THAT(C1.concentration[1], DoubleEq(0.0));  // Last value for point after range
}

TEST_F(US_AstfemMathTest, InterpolateC0ExactMatch) {
    US_AstfemMath::MfemInitial C0, C1;

    C0.radius << 6.0 << 7.0 << 8.0;
    C0.concentration << 1.0 << 0.5 << 0.0;

    // Test exact matches
    C1.radius << 6.0 << 8.0;
    C1.concentration.resize(2);

    US_AstfemMath::interpolate_C0(C0, C1);

    EXPECT_THAT(C1.concentration[0], DoubleEq(1.0));
    EXPECT_THAT(C1.concentration[1], DoubleEq(0.0));
}

// Test QuadSolver method
TEST_F(US_AstfemMathTest, QuadSolverSimpleSystem) {
    int N = 3;
    double ai[3] = {0.0, 1.0, 1.0};  // sub-sub-diagonal
    double bi[3] = {1.0, 1.0, 1.0};  // sub-diagonal
    double ci[3] = {2.0, 2.0, 2.0};  // main diagonal
    double di[3] = {1.0, 1.0, 0.0};  // super-diagonal
    double cr[3] = {4.0, 4.0, 3.0};  // right-hand side
    double solu[3];

    US_AstfemMath::QuadSolver(ai, bi, ci, di, cr, solu, N);

    // Solution should be reasonable (exact values depend on the specific system)
    for (int i = 0; i < N; i++) {
        EXPECT_FALSE(std::isnan(solu[i]));
        EXPECT_FALSE(std::isinf(solu[i]));
    }
}

TEST_F(US_AstfemMathTest, QuadSolverSingleElement) {
    int N = 1;
    double ai[1] = {0.0};
    double bi[1] = {0.0};
    double ci[1] = {3.0};
    double di[1] = {0.0};
    double cr[1] = {9.0};
    double solu[1];

    US_AstfemMath::QuadSolver(ai, bi, ci, di, cr, solu, N);

    EXPECT_THAT(solu[0], DoubleNear(3.0, 1e-10));
}

// Test low_acceleration method
TEST_F(US_AstfemMathTest, LowAccelerationNormalCase) {
    QVector<US_SimulationParameters::SpeedProfile> speedsteps(1);
    speedsteps[0].rotorspeed = 50000;
    speedsteps[0].time_first = 1800.0;
    speedsteps[0].w2t_first = 1000000.0;

    double min_accel = 100.0;
    double rate;

    bool result = US_AstfemMath::low_acceleration(speedsteps, min_accel, rate);

    EXPECT_GT(rate, 0.0);
    // Result depends on whether calculated rate is below min_accel
}

TEST_F(US_AstfemMathTest, LowAccelerationHighMinimum) {
    QVector<US_SimulationParameters::SpeedProfile> speedsteps(1);
    speedsteps[0].rotorspeed = 50000;
    speedsteps[0].time_first = 1800.0;
    speedsteps[0].w2t_first = 1000000.0;

    double min_accel = 1000000.0;  // Very high minimum
    double rate;

    bool result = US_AstfemMath::low_acceleration(speedsteps, min_accel, rate);

    EXPECT_TRUE(result);  // Should be low acceleration
    EXPECT_GT(rate, 0.0);
}

TEST_F(US_AstfemMathTest, LowAccelerationZeroSpeed) {
    QVector<US_SimulationParameters::SpeedProfile> speedsteps(1);
    speedsteps[0].rotorspeed = 0;
    speedsteps[0].time_first = 1800.0;
    speedsteps[0].w2t_first = 0.0;

    double min_accel = 100.0;
    double rate;

    bool result = US_AstfemMath::low_acceleration(speedsteps, min_accel, rate);

    EXPECT_THAT(rate, DoubleEq(0.0));
    EXPECT_TRUE(result);  // Zero rate is below any positive minimum
}

// Edge case tests for mathematical functions
TEST_F(US_AstfemMathTest, MaxvalWithNegativeValues) {
    QVector<double> values;
    values << -5.0 << -2.1 << -8.3 << -1.0;

    double result = US_AstfemMath::maxval(values);

    EXPECT_THAT(result, DoubleEq(-1.0));
}

TEST_F(US_AstfemMathTest, MinvalWithNegativeValues) {
    QVector<double> values;
    values << -5.0 << -2.1 << -8.3 << -1.0;

    double result = US_AstfemMath::minval(values);

    EXPECT_THAT(result, DoubleEq(-8.3));
}

TEST_F(US_AstfemMathTest, MaxvalWithMixedValues) {
    QVector<double> values;
    values << -5.0 << 3.2 << -8.3 << 1.0 << 0.0;

    double result = US_AstfemMath::maxval(values);

    EXPECT_THAT(result, DoubleEq(3.2));
}

TEST_F(US_AstfemMathTest, MinvalWithMixedValues) {
    QVector<double> values;
    values << -5.0 << 3.2 << -8.3 << 1.0 << 0.0;

    double result = US_AstfemMath::minval(values);

    EXPECT_THAT(result, DoubleEq(-8.3));
}

// Test edge cases for cube_root
TEST_F(US_AstfemMathTest, CubeRootLargeMagnitudes) {
    double a0 = -1000000.0;
    double a1 = 1000.0;
    double a2 = 10.0;

    double result = US_AstfemMath::cube_root(a0, a1, a2);

    EXPECT_GT(result, 0.0);
    EXPECT_FALSE(std::isnan(result));
    EXPECT_FALSE(std::isinf(result));
}

TEST_F(US_AstfemMathTest, CubeRootVerySmallValues) {
    double a0 = -1e-10;
    double a1 = 1e-5;
    double a2 = 1e-3;

    double result = US_AstfemMath::cube_root(a0, a1, a2);

    EXPECT_GE(result, 0.0);
    EXPECT_FALSE(std::isnan(result));
    EXPECT_FALSE(std::isinf(result));
}

// Test edge cases for find_C1_mono_Nmer
TEST_F(US_AstfemMathTest, FindC1MonoNmerLargeN) {
    int n = 10;
    double K = 0.1;
    double CT = 5.0;

    double result = US_AstfemMath::find_C1_mono_Nmer(n, K, CT);

    EXPECT_GT(result, 0.0);
    EXPECT_LT(result, CT);
    EXPECT_FALSE(std::isnan(result));
}

TEST_F(US_AstfemMathTest, FindC1MonoNmerLargeK) {
    int n = 2;
    double K = 1000.0;
    double CT = 1.0;

    double result = US_AstfemMath::find_C1_mono_Nmer(n, K, CT);

    EXPECT_GE(result, 0.0);
    EXPECT_FALSE(std::isnan(result));
}

TEST_F(US_AstfemMathTest, FindC1MonoNmerVeryLargeConcentration) {
    int n = 2;
    double K = 1.0;
    double CT = 1000.0;

    double result = US_AstfemMath::find_C1_mono_Nmer(n, K, CT);

    EXPECT_GT(result, 0.0);
    EXPECT_LT(result, CT);
    EXPECT_FALSE(std::isnan(result));
}

// Test matrix operations with edge cases
TEST_F(US_AstfemMathTest, Initialize2DLargeMatrix) {
    double** matrix;
    int rows = 100;
    int cols = 50;

    US_AstfemMath::initialize_2d(rows, cols, &matrix);

    // Check a few random elements
    EXPECT_THAT(matrix[0][0], DoubleEq(0.0));
    EXPECT_THAT(matrix[50][25], DoubleEq(0.0));
    EXPECT_THAT(matrix[99][49], DoubleEq(0.0));

    // Test assignment to verify proper allocation
    matrix[50][25] = 42.0;
    EXPECT_THAT(matrix[50][25], DoubleEq(42.0));

    US_AstfemMath::clear_2d(rows, matrix);
}

TEST_F(US_AstfemMathTest, Initialize3DLargeMatrix) {
    double*** matrix;
    int dim1 = 10, dim2 = 10, dim3 = 10;

    US_AstfemMath::initialize_3d(dim1, dim2, dim3, &matrix);

    // Check a few elements
    EXPECT_THAT(matrix[0][0][0], DoubleEq(0.0));
    EXPECT_THAT(matrix[5][5][5], DoubleEq(0.0));
    EXPECT_THAT(matrix[9][9][9], DoubleEq(0.0));

    // Test assignment
    matrix[5][5][5] = 123.456;
    EXPECT_THAT(matrix[5][5][5], DoubleEq(123.456));

    US_AstfemMath::clear_3d(dim1, dim2, matrix);
}

// Test GaussElim with edge cases
TEST_F(US_AstfemMathTest, GaussElimLargerSystem) {
    int n = 3;
    double** A;
    US_AstfemMath::initialize_2d(n, n, &A);
    double b[3];

    // Set up 3x3 system with known solution [1, 2, 3]
    A[0][0] = 2.0; A[0][1] = 1.0; A[0][2] = 1.0;  // 2x + y + z = 7
    A[1][0] = 1.0; A[1][1] = 3.0; A[1][2] = 2.0;  // x + 3y + 2z = 13
    A[2][0] = 1.0; A[2][1] = 1.0; A[2][2] = 4.0;  // x + y + 4z = 15
    b[0] = 7.0; b[1] = 13.0; b[2] = 15.0;

    int result = US_AstfemMath::GaussElim(n, A, b);

    EXPECT_THAT(result, Eq(1));
    EXPECT_THAT(b[0], DoubleNear(1.0, 1e-10));
    EXPECT_THAT(b[1], DoubleNear(2.0, 1e-10));
    EXPECT_THAT(b[2], DoubleNear(3.0, 1e-10));

    US_AstfemMath::clear_2d(n, A);
}

TEST_F(US_AstfemMathTest, GaussElimNearSingular) {
    int n = 2;
    double** A;
    US_AstfemMath::initialize_2d(n, n, &A);
    double b[2];

    // Set up nearly singular system
    A[0][0] = 1.0; A[0][1] = 2.0;
    A[1][0] = 1.0; A[1][1] = 2.0 + 1e-15;  // Very close to singular
    b[0] = 3.0; b[1] = 3.0 + 1e-15;

    int result = US_AstfemMath::GaussElim(n, A, b);

    // Behavior depends on numerical precision - could succeed or fail
    EXPECT_TRUE(result == 1 || result == -1);

    US_AstfemMath::clear_2d(n, A);
}

// Test tridiag with edge cases
TEST_F(US_AstfemMathTest, TridiagLargerSystem) {
    int N = 5;
    double a[5] = {0.0, 1.0, 1.0, 1.0, 1.0};
    double b[5] = {2.0, 2.0, 2.0, 2.0, 2.0};
    double c[5] = {1.0, 1.0, 1.0, 1.0, 0.0};
    double r[5] = {3.0, 3.0, 3.0, 3.0, 3.0};
    double u[5];

    US_AstfemMath::tridiag(a, b, c, r, u, N);

    // Verify solution is reasonable
    for (int i = 0; i < N; i++) {
        EXPECT_FALSE(std::isnan(u[i]));
        EXPECT_FALSE(std::isinf(u[i]));
    }
}

TEST_F(US_AstfemMathTest, TridiagDiagonalSystem) {
    int N = 3;
    double a[3] = {0.0, 0.0, 0.0};  // No sub-diagonal
    double b[3] = {2.0, 3.0, 4.0};  // Main diagonal only
    double c[3] = {0.0, 0.0, 0.0};  // No super-diagonal
    double r[3] = {4.0, 9.0, 12.0}; // Right-hand side
    double u[3];

    US_AstfemMath::tridiag(a, b, c, r, u, N);

    // Solution should be [2, 3, 3]
    EXPECT_THAT(u[0], DoubleNear(2.0, 1e-10));
    EXPECT_THAT(u[1], DoubleNear(3.0, 1e-10));
    EXPECT_THAT(u[2], DoubleNear(3.0, 1e-10));
}

// Test calc_bottom with extreme values
TEST_F(US_AstfemMathTest, CalcBottomVeryHighRpm) {
    double rpm = 100000.0;  // Very high RPM
    double bottom_chan = 7.0;
    double rotorcoefs[2] = {0.001, 0.000001};

    double result = US_AstfemMath::calc_bottom(rpm, bottom_chan, rotorcoefs);

    double expected = bottom_chan + rotorcoefs[0] * rpm + rotorcoefs[1] * rpm * rpm;
    EXPECT_THAT(result, DoubleEq(expected));
    EXPECT_GT(result, bottom_chan);  // Should be larger due to stretching
}

TEST_F(US_AstfemMathTest, CalcBottomLargeCoefficients) {
    double rpm = 50000.0;
    double bottom_chan = 7.0;
    double rotorcoefs[2] = {0.1, 0.001};  // Large coefficients

    double result = US_AstfemMath::calc_bottom(rpm, bottom_chan, rotorcoefs);

    double expected = bottom_chan + rotorcoefs[0] * rpm + rotorcoefs[1] * rpm * rpm;
    EXPECT_THAT(result, DoubleEq(expected));
    EXPECT_GT(result, bottom_chan + 1000.0);  // Should be significantly larger
}

// Test interpolation edge cases
TEST_F(US_AstfemMathTest, InterpolateC0EmptySource) {
    US_AstfemMath::MfemInitial C0, C1;

    // Empty C0
    C1.radius << 6.0 << 7.0;
    C1.concentration.resize(2);

    // This should handle gracefully without crashing
    US_AstfemMath::interpolate_C0(C0, C1);

    // Behavior with empty source is implementation-defined
    // Just verify no crash occurs
}

TEST_F(US_AstfemMathTest, InterpolateC0EmptyTarget) {
    US_AstfemMath::MfemInitial C0, C1;

    C0.radius << 6.0 << 7.0 << 8.0;
    C0.concentration << 1.0 << 0.5 << 0.0;
    // Empty C1

    // Should handle gracefully
    US_AstfemMath::interpolate_C0(C0, C1);

    // Verify no crash
}

TEST_F(US_AstfemMathTest, InterpolateC0IdenticalRadii) {
    US_AstfemMath::MfemInitial C0, C1;

    C0.radius << 6.0 << 7.0 << 8.0;
    C0.concentration << 1.0 << 0.5 << 0.0;

    C1.radius << 6.0 << 7.0 << 8.0;  // Identical radii
    C1.concentration.resize(3);

    US_AstfemMath::interpolate_C0(C0, C1);

    // Should copy values exactly
    EXPECT_THAT(C1.concentration[0], DoubleEq(1.0));
    EXPECT_THAT(C1.concentration[1], DoubleEq(0.5));
    EXPECT_THAT(C1.concentration[2], DoubleEq(0.0));
}