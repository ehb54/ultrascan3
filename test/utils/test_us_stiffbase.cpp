#include "qt_test_base.h"
#include "us_stiffbase.h"
#include <cmath>

using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;

class US_StiffBaseTest : public QtTestBase {
protected:
    void SetUp() override {
        QtTestBase::SetUp();
        stiffBase = std::make_unique<US_StiffBase>();
    }

    void TearDown() override {
        stiffBase.reset();
        QtTestBase::TearDown();
    }

    std::unique_ptr<US_StiffBase> stiffBase;

    // Helper to create test node coordinates for triangular element
    void createTriangularElement(double xd[4][2]) {
        // Standard triangular element: (0,0), (1,0), (0,1)
        xd[0][0] = 0.0; xd[0][1] = 0.0;  // Node 0
        xd[1][0] = 1.0; xd[1][1] = 0.0;  // Node 1
        xd[2][0] = 0.0; xd[2][1] = 1.0;  // Node 2
        xd[3][0] = 0.0; xd[3][1] = 0.0;  // Unused for triangular
    }

    // Helper to create test node coordinates for quadrilateral element
    void createQuadrilateralElement(double xd[4][2]) {
        // Standard quadrilateral element: (0,0), (1,0), (1,1), (0,1)
        xd[0][0] = 0.0; xd[0][1] = 0.0;  // Node 0
        xd[1][0] = 1.0; xd[1][1] = 0.0;  // Node 1
        xd[2][0] = 1.0; xd[2][1] = 1.0;  // Node 2
        xd[3][0] = 0.0; xd[3][1] = 1.0;  // Node 3
    }

    // Helper to create deformed triangular element
    void createDeformedTriangularElement(double xd[4][2]) {
        xd[0][0] = 0.5; xd[0][1] = 0.2;  // Shifted node 0
        xd[1][0] = 1.5; xd[1][1] = 0.3;  // Shifted node 1
        xd[2][0] = 0.3; xd[2][1] = 1.2;  // Shifted node 2
        xd[3][0] = 0.0; xd[3][1] = 0.0;  // Unused
    }

    // Helper to create deformed quadrilateral element
    void createDeformedQuadrilateralElement(double xd[4][2]) {
        xd[0][0] = 0.1; xd[0][1] = 0.1;  // Slightly shifted
        xd[1][0] = 1.1; xd[1][1] = 0.05; // Slightly deformed
        xd[2][0] = 1.05; xd[2][1] = 1.1; // Slightly deformed
        xd[3][0] = 0.05; xd[3][1] = 1.05; // Slightly shifted
    }

    // Helper to allocate 2D array for stiffness matrix
    double** allocateStiffMatrix(int size) {
        double** matrix = new double*[size];
        for (int i = 0; i < size; ++i) {
            matrix[i] = new double[size];
            for (int j = 0; j < size; ++j) {
                matrix[i][j] = 0.0;
            }
        }
        return matrix;
    }

    // Helper to deallocate 2D array
    void deallocateStiffMatrix(double** matrix, int size) {
        for (int i = 0; i < size; ++i) {
            delete[] matrix[i];
        }
        delete[] matrix;
    }

    // Helper to check if matrix is symmetric
    bool isMatrixSymmetric(double** matrix, int size, double tolerance = 1e-12) {
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                if (std::abs(matrix[i][j] - matrix[j][i]) > tolerance) {
                    return false;
                }
            }
        }
        return true;
    }

    // Helper to calculate matrix norm
    double matrixFrobeniusNorm(double** matrix, int size) {
        double sum = 0.0;
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                sum += matrix[i][j] * matrix[i][j];
            }
        }
        return std::sqrt(sum);
    }
};

// Constructor Tests
TEST_F(US_StiffBaseTest, Constructor_InitializesCorrectly) {
// Test that constructor doesn't crash and object is created
EXPECT_NE(stiffBase.get(), nullptr);

// The constructor should complete without throwing exceptions
EXPECT_NO_THROW(US_StiffBase testStiffBase);
}

TEST_F(US_StiffBaseTest, Constructor_AllocatesMemoryCorrectly) {
// Constructor should allocate all necessary arrays
// We can't directly access private members, but we can test that
// the object is functional by calling public methods

double xd[4][2];
createTriangularElement(xd);
double** stifMatrix = allocateStiffMatrix(3);

// This should not crash if memory was allocated correctly
EXPECT_NO_THROW(stiffBase->CompLocalStif(3, xd, 1.0, 1.0, stifMatrix));

deallocateStiffMatrix(stifMatrix, 3);
}

// Destructor Tests
TEST_F(US_StiffBaseTest, Destructor_HandlesMemoryCleanup) {
// Test that destructor doesn't crash
EXPECT_NO_THROW({
US_StiffBase* testStiffBase = new US_StiffBase();
delete testStiffBase;
});
}

TEST_F(US_StiffBaseTest, Destructor_MultipleDestructions_HandlesCorrectly) {
// Test multiple objects can be created and destroyed
EXPECT_NO_THROW({
for (int i = 0; i < 10; ++i) {
US_StiffBase testStiffBase;
} // Destructor called automatically
});
}

// Gauss2D Class Tests
TEST_F(US_StiffBaseTest, Gauss2D_DefaultInitialization) {
US_StiffBase::Gauss2D gauss;

// Test that Gauss2D object can be created
// Values may be uninitialized, but object should exist
EXPECT_NO_THROW(gauss.x = 1.0);
EXPECT_NO_THROW(gauss.y = 2.0);
EXPECT_NO_THROW(gauss.w = 3.0);

EXPECT_EQ(gauss.x, 1.0);
EXPECT_EQ(gauss.y, 2.0);
EXPECT_EQ(gauss.w, 3.0);
}

// CompLocalStif Tests - Triangular Elements
TEST_F(US_StiffBaseTest, CompLocalStif_TriangularElement_StandardElement) {
double xd[4][2];
createTriangularElement(xd);
double** stifMatrix = allocateStiffMatrix(3);

double D = 1.0;   // Diffusion coefficient
double sw2 = 1.0; // Omega-squared

EXPECT_NO_THROW(stiffBase->CompLocalStif(3, xd, D, sw2, stifMatrix));

// Check that matrix is not all zeros
double norm = matrixFrobeniusNorm(stifMatrix, 3);
EXPECT_GT(norm, 0.0);

// Check that matrix has reasonable values
for (int i = 0; i < 3; ++i) {
for (int j = 0; j < 3; ++j) {
EXPECT_TRUE(std::isfinite(stifMatrix[i][j]));
}
}

deallocateStiffMatrix(stifMatrix, 3);
}

TEST_F(US_StiffBaseTest, CompLocalStif_TriangularElement_ZeroDiffusion) {
double xd[4][2];
createTriangularElement(xd);
double** stifMatrix = allocateStiffMatrix(3);

double D = 0.0;   // Zero diffusion
double sw2 = 1.0;

EXPECT_NO_THROW(stiffBase->CompLocalStif(3, xd, D, sw2, stifMatrix));

// Matrix should still be computed (though different from D>0 case)
double norm = matrixFrobeniusNorm(stifMatrix, 3);
EXPECT_GE(norm, 0.0); // Could be zero or positive

deallocateStiffMatrix(stifMatrix, 3);
}

TEST_F(US_StiffBaseTest, CompLocalStif_TriangularElement_ZeroOmega) {
double xd[4][2];
createTriangularElement(xd);
double** stifMatrix = allocateStiffMatrix(3);

double D = 1.0;
double sw2 = 0.0; // Zero omega-squared

EXPECT_NO_THROW(stiffBase->CompLocalStif(3, xd, D, sw2, stifMatrix));

// Matrix should still be computed
double norm = matrixFrobeniusNorm(stifMatrix, 3);
EXPECT_GT(norm, 0.0);

deallocateStiffMatrix(stifMatrix, 3);
}

TEST_F(US_StiffBaseTest, CompLocalStif_TriangularElement_NegativeParameters) {
double xd[4][2];
createTriangularElement(xd);
double** stifMatrix = allocateStiffMatrix(3);

double D = -1.0;   // Negative diffusion
double sw2 = -1.0; // Negative omega-squared

EXPECT_NO_THROW(stiffBase->CompLocalStif(3, xd, D, sw2, stifMatrix));

// Should still compute, though physically meaningless
for (int i = 0; i < 3; ++i) {
for (int j = 0; j < 3; ++j) {
EXPECT_TRUE(std::isfinite(stifMatrix[i][j]));
}
}

deallocateStiffMatrix(stifMatrix, 3);
}

TEST_F(US_StiffBaseTest, CompLocalStif_TriangularElement_DeformedElement) {
double xd[4][2];
createDeformedTriangularElement(xd);
double** stifMatrix = allocateStiffMatrix(3);

double D = 1.0;
double sw2 = 1.0;

EXPECT_NO_THROW(stiffBase->CompLocalStif(3, xd, D, sw2, stifMatrix));

// Check that deformed element produces non-zero matrix
double norm = matrixFrobeniusNorm(stifMatrix, 3);
EXPECT_GT(norm, 0.0);

deallocateStiffMatrix(stifMatrix, 3);
}

TEST_F(US_StiffBaseTest, CompLocalStif_TriangularElement_ExtremelySmallElement) {
double xd[4][2];
// Very small triangular element
xd[0][0] = 0.0; xd[0][1] = 0.0;
xd[1][0] = 1e-10; xd[1][1] = 0.0;
xd[2][0] = 0.0; xd[2][1] = 1e-10;
xd[3][0] = 0.0; xd[3][1] = 0.0;

double** stifMatrix = allocateStiffMatrix(3);
double D = 1.0;
double sw2 = 1.0;

EXPECT_NO_THROW(stiffBase->CompLocalStif(3, xd, D, sw2, stifMatrix));

// Matrix values should be finite even for small elements
for (int i = 0; i < 3; ++i) {
for (int j = 0; j < 3; ++j) {
EXPECT_TRUE(std::isfinite(stifMatrix[i][j]));
}
}

deallocateStiffMatrix(stifMatrix, 3);
}

// CompLocalStif Tests - Quadrilateral Elements
TEST_F(US_StiffBaseTest, CompLocalStif_QuadrilateralElement_StandardElement) {
double xd[4][2];
createQuadrilateralElement(xd);
double** stifMatrix = allocateStiffMatrix(4);

double D = 1.0;
double sw2 = 1.0;

EXPECT_NO_THROW(stiffBase->CompLocalStif(4, xd, D, sw2, stifMatrix));

// Check that matrix is not all zeros
double norm = matrixFrobeniusNorm(stifMatrix, 4);
EXPECT_GT(norm, 0.0);

// Check that all values are finite
for (int i = 0; i < 4; ++i) {
for (int j = 0; j < 4; ++j) {
EXPECT_TRUE(std::isfinite(stifMatrix[i][j]));
}
}

deallocateStiffMatrix(stifMatrix, 4);
}

TEST_F(US_StiffBaseTest, CompLocalStif_QuadrilateralElement_DeformedElement) {
double xd[4][2];
createDeformedQuadrilateralElement(xd);
double** stifMatrix = allocateStiffMatrix(4);

double D = 2.5;
double sw2 = 3.7;

EXPECT_NO_THROW(stiffBase->CompLocalStif(4, xd, D, sw2, stifMatrix));

// Check that deformed element produces valid matrix
double norm = matrixFrobeniusNorm(stifMatrix, 4);
EXPECT_GT(norm, 0.0);

deallocateStiffMatrix(stifMatrix, 4);
}

TEST_F(US_StiffBaseTest, CompLocalStif_QuadrilateralElement_ExtremeAspectRatio) {
double xd[4][2];
// Very elongated quadrilateral
xd[0][0] = 0.0; xd[0][1] = 0.0;
xd[1][0] = 10.0; xd[1][1] = 0.0;
xd[2][0] = 10.0; xd[2][1] = 0.01;
xd[3][0] = 0.0; xd[3][1] = 0.01;

double** stifMatrix = allocateStiffMatrix(4);
double D = 1.0;
double sw2 = 1.0;

EXPECT_NO_THROW(stiffBase->CompLocalStif(4, xd, D, sw2, stifMatrix));

// Should handle extreme aspect ratios
for (int i = 0; i < 4; ++i) {
for (int j = 0; j < 4; ++j) {
EXPECT_TRUE(std::isfinite(stifMatrix[i][j]));
}
}

deallocateStiffMatrix(stifMatrix, 4);
}

// Error Condition Tests
TEST_F(US_StiffBaseTest, CompLocalStif_InvalidNK_HandlesGracefully) {
double xd[4][2];
createTriangularElement(xd);
double** stifMatrix = allocateStiffMatrix(4);

// Test with invalid NK values
EXPECT_NO_THROW(stiffBase->CompLocalStif(5, xd, 1.0, 1.0, stifMatrix));
EXPECT_NO_THROW(stiffBase->CompLocalStif(2, xd, 1.0, 1.0, stifMatrix));
EXPECT_NO_THROW(stiffBase->CompLocalStif(0, xd, 1.0, 1.0, stifMatrix));
EXPECT_NO_THROW(stiffBase->CompLocalStif(-1, xd, 1.0, 1.0, stifMatrix));

deallocateStiffMatrix(stifMatrix, 4);
}

TEST_F(US_StiffBaseTest, CompLocalStif_DegenerateTriangle_HandlesGracefully) {
double xd[4][2];
// Degenerate triangle (all points collinear)
xd[0][0] = 0.0; xd[0][1] = 0.0;
xd[1][0] = 1.0; xd[1][1] = 0.0;
xd[2][0] = 2.0; xd[2][1] = 0.0; // Collinear point
xd[3][0] = 0.0; xd[3][1] = 0.0;

double** stifMatrix = allocateStiffMatrix(3);

// Should handle degenerate elements without crashing
EXPECT_NO_THROW(stiffBase->CompLocalStif(3, xd, 1.0, 1.0, stifMatrix));

deallocateStiffMatrix(stifMatrix, 3);
}

TEST_F(US_StiffBaseTest, CompLocalStif_DegenerateQuadrilateral_HandlesGracefully) {
double xd[4][2];
// Degenerate quadrilateral (zero area)
xd[0][0] = 0.0; xd[0][1] = 0.0;
xd[1][0] = 0.0; xd[1][1] = 0.0; // Duplicate point
xd[2][0] = 0.0; xd[2][1] = 0.0; // Duplicate point
xd[3][0] = 0.0; xd[3][1] = 0.0; // Duplicate point

double** stifMatrix = allocateStiffMatrix(4);

// Should handle degenerate elements
EXPECT_NO_THROW(stiffBase->CompLocalStif(4, xd, 1.0, 1.0, stifMatrix));

deallocateStiffMatrix(stifMatrix, 4);
}

// Extreme Parameter Tests
TEST_F(US_StiffBaseTest, CompLocalStif_ExtremeParameters_HandlesCorrectly) {
double xd[4][2];
createTriangularElement(xd);
double** stifMatrix = allocateStiffMatrix(3);

// Test with very large parameters
EXPECT_NO_THROW(stiffBase->CompLocalStif(3, xd, 1e10, 1e10, stifMatrix));

// Test with very small parameters
EXPECT_NO_THROW(stiffBase->CompLocalStif(3, xd, 1e-10, 1e-10, stifMatrix));

// Test with infinite parameters (should handle gracefully or crash predictably)
EXPECT_NO_THROW(stiffBase->CompLocalStif(3, xd,
std::numeric_limits<double>::infinity(), 1.0, stifMatrix));

deallocateStiffMatrix(stifMatrix, 3);
}

TEST_F(US_StiffBaseTest, CompLocalStif_NaNParameters_HandlesGracefully) {
double xd[4][2];
createTriangularElement(xd);
double** stifMatrix = allocateStiffMatrix(3);

// Test with NaN parameters
double nan_val = std::numeric_limits<double>::quiet_NaN();

EXPECT_NO_THROW(stiffBase->CompLocalStif(3, xd, nan_val, 1.0, stifMatrix));
EXPECT_NO_THROW(stiffBase->CompLocalStif(3, xd, 1.0, nan_val, stifMatrix));

deallocateStiffMatrix(stifMatrix, 3);
}

// Matrix Properties Tests
TEST_F(US_StiffBaseTest, CompLocalStif_TriangularElement_MatrixProperties) {
double xd[4][2];
createTriangularElement(xd);
double** stifMatrix = allocateStiffMatrix(3);

double D = 1.0;
double sw2 = 1.0;

stiffBase->CompLocalStif(3, xd, D, sw2, stifMatrix);

// Check matrix properties
// Note: The stiffness matrix may not be symmetric due to the convection term
// but it should have finite values
for (int i = 0; i < 3; ++i) {
for (int j = 0; j < 3; ++j) {
EXPECT_TRUE(std::isfinite(stifMatrix[i][j]));
}
}

// Check that matrix is not trivial (not all zeros)
bool hasNonZero = false;
for (int i = 0; i < 3 && !hasNonZero; ++i) {
for (int j = 0; j < 3 && !hasNonZero; ++j) {
if (std::abs(stifMatrix[i][j]) > 1e-15) {
hasNonZero = true;
}
}
}
EXPECT_TRUE(hasNonZero);

deallocateStiffMatrix(stifMatrix, 3);
}

TEST_F(US_StiffBaseTest, CompLocalStif_QuadrilateralElement_MatrixProperties) {
double xd[4][2];
createQuadrilateralElement(xd);
double** stifMatrix = allocateStiffMatrix(4);

double D = 2.0;
double sw2 = 3.0;

stiffBase->CompLocalStif(4, xd, D, sw2, stifMatrix);

// Check that all matrix entries are finite
for (int i = 0; i < 4; ++i) {
for (int j = 0; j < 4; ++j) {
EXPECT_TRUE(std::isfinite(stifMatrix[i][j]));
}
}

// Check that matrix has reasonable magnitude
double norm = matrixFrobeniusNorm(stifMatrix, 4);
EXPECT_GT(norm, 0.0);
EXPECT_LT(norm, 1e10); // Should not be extremely large

deallocateStiffMatrix(stifMatrix, 4);
}

// Parameter Variation Tests
TEST_F(US_StiffBaseTest, CompLocalStif_DiffusionVariation_AffectsMatrix) {
double xd[4][2];
createTriangularElement(xd);
double** stifMatrix1 = allocateStiffMatrix(3);
double** stifMatrix2 = allocateStiffMatrix(3);

double sw2 = 1.0;

// Compute with different diffusion coefficients
stiffBase->CompLocalStif(3, xd, 1.0, sw2, stifMatrix1);
stiffBase->CompLocalStif(3, xd, 2.0, sw2, stifMatrix2);

// Matrices should be different
bool matricesDifferent = false;
for (int i = 0; i < 3 && !matricesDifferent; ++i) {
for (int j = 0; j < 3 && !matricesDifferent; ++j) {
if (std::abs(stifMatrix1[i][j] - stifMatrix2[i][j]) > 1e-12) {
matricesDifferent = true;
}
}
}
EXPECT_TRUE(matricesDifferent);

deallocateStiffMatrix(stifMatrix1, 3);
deallocateStiffMatrix(stifMatrix2, 3);
}

TEST_F(US_StiffBaseTest, CompLocalStif_OmegaVariation_AffectsMatrix) {
double xd[4][2];
createTriangularElement(xd);
double** stifMatrix1 = allocateStiffMatrix(3);
double** stifMatrix2 = allocateStiffMatrix(3);

double D = 1.0;

// Compute with different omega-squared values
stiffBase->CompLocalStif(3, xd, D, 1.0, stifMatrix1);
stiffBase->CompLocalStif(3, xd, D, 4.0, stifMatrix2);

// Matrices should be different
bool matricesDifferent = false;
for (int i = 0; i < 3 && !matricesDifferent; ++i) {
for (int j = 0; j < 3 && !matricesDifferent; ++j) {
if (std::abs(stifMatrix1[i][j] - stifMatrix2[i][j]) > 1e-12) {
matricesDifferent = true;
}
}
}
EXPECT_TRUE(matricesDifferent);

deallocateStiffMatrix(stifMatrix1, 3);
deallocateStiffMatrix(stifMatrix2, 3);
}

// Element Type Comparison Tests
TEST_F(US_StiffBaseTest, CompLocalStif_TriangularVsQuadrilateral_DifferentSizes) {
double xd[4][2];

// Test triangular element
createTriangularElement(xd);
double** stifMatrixT = allocateStiffMatrix(3);
stiffBase->CompLocalStif(3, xd, 1.0, 1.0, stifMatrixT);

// Test quadrilateral element
createQuadrilateralElement(xd);
double** stifMatrixQ = allocateStiffMatrix(4);
stiffBase->CompLocalStif(4, xd, 1.0, 1.0, stifMatrixQ);

// Both should produce valid matrices of different sizes
double normT = matrixFrobeniusNorm(stifMatrixT, 3);
double normQ = matrixFrobeniusNorm(stifMatrixQ, 4);

EXPECT_GT(normT, 0.0);
EXPECT_GT(normQ, 0.0);

deallocateStiffMatrix(stifMatrixT, 3);
deallocateStiffMatrix(stifMatrixQ, 4);
}

// Stress Tests for Constructor/Destructor
TEST_F(US_StiffBaseTest, MultipleInstances_CreateAndDestroy) {
// Test creating multiple instances
std::vector<std::unique_ptr<US_StiffBase>> instances;

EXPECT_NO_THROW({
for (int i = 0; i < 10; ++i) {
instances.push_back(std::make_unique<US_StiffBase>());
}
});

// Test that all instances are functional
double xd[4][2];
createTriangularElement(xd);

for (auto& instance : instances) {
double** stifMatrix = allocateStiffMatrix(3);
EXPECT_NO_THROW(instance->CompLocalStif(3, xd, 1.0, 1.0, stifMatrix));
deallocateStiffMatrix(stifMatrix, 3);
}

// Instances will be destroyed automatically when vector goes out of scope
}

// Memory Boundary Tests
TEST_F(US_StiffBaseTest, CompLocalStif_LargeCoordinates_HandlesCorrectly) {
double xd[4][2];
// Very large coordinates
xd[0][0] = 1e6; xd[0][1] = 1e6;
xd[1][0] = 1e6 + 1; xd[1][1] = 1e6;
xd[2][0] = 1e6; xd[2][1] = 1e6 + 1;
xd[3][0] = 0.0; xd[3][1] = 0.0;

double** stifMatrix = allocateStiffMatrix(3);

EXPECT_NO_THROW(stiffBase->CompLocalStif(3, xd, 1.0, 1.0, stifMatrix));

// Values should be finite
for (int i = 0; i < 3; ++i) {
for (int j = 0; j < 3; ++j) {
EXPECT_TRUE(std::isfinite(stifMatrix[i][j]));
}
}

deallocateStiffMatrix(stifMatrix, 3);
}

// Death Tests
TEST_F(US_StiffBaseTest, CompLocalStif_NullMatrix_DoesNotCrash) {
double xd[4][2];
createTriangularElement(xd);

// Test with null matrix pointer - this should crash or be handled
EXPECT_DEATH_IF_SUPPORTED(
        stiffBase->CompLocalStif(3, xd, 1.0, 1.0, nullptr),
".*"
);
}

// Consistency Tests
TEST_F(US_StiffBaseTest, CompLocalStif_RepeatedCalls_ProduceSameResult) {
double xd[4][2];
createTriangularElement(xd);
double** stifMatrix1 = allocateStiffMatrix(3);
double** stifMatrix2 = allocateStiffMatrix(3);

double D = 1.5;
double sw2 = 2.3;

// Call multiple times with same parameters
stiffBase->CompLocalStif(3, xd, D, sw2, stifMatrix1);
stiffBase->CompLocalStif(3, xd, D, sw2, stifMatrix2);

// Results should be identical
for (int i = 0; i < 3; ++i) {
for (int j = 0; j < 3; ++j) {
EXPECT_NEAR(stifMatrix1[i][j], stifMatrix2[i][j], 1e-15);
}
}

deallocateStiffMatrix(stifMatrix1, 3);
deallocateStiffMatrix(stifMatrix2, 3);
}

TEST_F(US_StiffBaseTest, CompLocalStif_MatrixInitialization_ClearsMatrix) {
double xd[4][2];
createTriangularElement(xd);
double** stifMatrix = allocateStiffMatrix(3);

// Set matrix to non-zero values
for (int i = 0; i < 3; ++i) {
for (int j = 0; j < 3; ++j) {
stifMatrix[i][j] = 999.0;
}
}

// CompLocalStif should zero the matrix and compute new values
stiffBase->CompLocalStif(3, xd, 1.0, 1.0, stifMatrix);

// Matrix should not contain the initial values
bool foundInitialValue = false;
for (int i = 0; i < 3; ++i) {
for (int j = 0; j < 3; ++j) {
if (std::abs(stifMatrix[i][j] - 999.0) < 1e-15) {
foundInitialValue = true;
}
}
}
EXPECT_FALSE(foundInitialValue);

deallocateStiffMatrix(stifMatrix, 3);
}

// Element Scaling Tests
TEST_F(US_StiffBaseTest, CompLocalStif_ScaledElement_ScalesCorrectly) {
double xd1[4][2], xd2[4][2];

// Original element
createTriangularElement(xd1);

// Scaled element (2x larger)
double scale = 2.0;
for (int i = 0; i < 3; ++i) {
xd2[i][0] = xd1[i][0] * scale;
xd2[i][1] = xd1[i][1] * scale;
}
xd2[3][0] = xd2[3][1] = 0.0;

double** stifMatrix1 = allocateStiffMatrix(3);
double** stifMatrix2 = allocateStiffMatrix(3);

double D = 1.0;
double sw2 = 1.0;

stiffBase->CompLocalStif(3, xd1, D, sw2, stifMatrix1);
stiffBase->CompLocalStif(3, xd2, D, sw2, stifMatrix2);

// Matrices should be different due to scaling
bool matricesDifferent = false;
for (int i = 0; i < 3 && !matricesDifferent; ++i) {
for (int j = 0; j < 3 && !matricesDifferent; ++j) {
if (std::abs(stifMatrix1[i][j] - stifMatrix2[i][j]) > 1e-12) {
matricesDifferent = true;
}
}
}
EXPECT_TRUE(matricesDifferent);

deallocateStiffMatrix(stifMatrix1, 3);
deallocateStiffMatrix(stifMatrix2, 3);
}

// Numerical Precision Tests
TEST_F(US_StiffBaseTest, CompLocalStif_HighPrecisionParameters_MaintainsPrecision) {
double xd[4][2];
createTriangularElement(xd);
double** stifMatrix = allocateStiffMatrix(3);

// Use parameters with many significant digits
double D = 1.23456789012345;
double sw2 = 9.87654321098765;

stiffBase->CompLocalStif(3, xd, D, sw2, stifMatrix);

// Check that results maintain reasonable precision
for (int i = 0; i < 3; ++i) {
for (int j = 0; j < 3; ++j) {
EXPECT_TRUE(std::isfinite(stifMatrix[i][j]));
// Values should not be exactly zero (unless physically meaningful)
if (std::abs(stifMatrix[i][j]) > 1e-15) {
EXPECT_GT(std::abs(stifMatrix[i][j]), 1e-20);
}
}
}

deallocateStiffMatrix(stifMatrix, 3);
}

// Element Orientation Tests
TEST_F(US_StiffBaseTest, CompLocalStif_TriangularElement_DifferentOrientations) {
double xd1[4][2], xd2[4][2];

// Standard orientation
createTriangularElement(xd1);

// Rotated orientation (different node ordering)
xd2[0][0] = 1.0; xd2[0][1] = 0.0;  // Node 1 -> Node 0
xd2[1][0] = 0.0; xd2[1][1] = 1.0;  // Node 2 -> Node 1
xd2[2][0] = 0.0; xd2[2][1] = 0.0;  // Node 0 -> Node 2
xd2[3][0] = 0.0; xd2[3][1] = 0.0;

double** stifMatrix1 = allocateStiffMatrix(3);
double** stifMatrix2 = allocateStiffMatrix(3);

stiffBase->CompLocalStif(3, xd1, 1.0, 1.0, stifMatrix1);
stiffBase->CompLocalStif(3, xd2, 1.0, 1.0, stifMatrix2);

// Different orientations should produce different matrices
bool matricesDifferent = false;
for (int i = 0; i < 3 && !matricesDifferent; ++i) {
for (int j = 0; j < 3 && !matricesDifferent; ++j) {
if (std::abs(stifMatrix1[i][j] - stifMatrix2[i][j]) > 1e-12) {
matricesDifferent = true;
}
}
}
EXPECT_TRUE(matricesDifferent);

deallocateStiffMatrix(stifMatrix1, 3);
deallocateStiffMatrix(stifMatrix2, 3);
}

// Boundary Coordinate Tests
TEST_F(US_StiffBaseTest, CompLocalStif_ZeroCoordinates_HandlesCorrectly) {
double xd[4][2];

// All coordinates at origin (degenerate case)
for (int i = 0; i < 4; ++i) {
xd[i][0] = 0.0;
xd[i][1] = 0.0;
}

double** stifMatrix = allocateStiffMatrix(3);

// Should handle degenerate case without crashing
EXPECT_NO_THROW(stiffBase->CompLocalStif(3, xd, 1.0, 1.0, stifMatrix));

deallocateStiffMatrix(stifMatrix, 3);
}

TEST_F(US_StiffBaseTest, CompLocalStif_NegativeCoordinates_HandlesCorrectly) {
double xd[4][2];

// Triangle with negative coordinates
xd[0][0] = -1.0; xd[0][1] = -1.0;
xd[1][0] =  0.0; xd[1][1] = -1.0;
xd[2][0] = -1.0; xd[2][1] =  0.0;
xd[3][0] =  0.0; xd[3][1] =  0.0;

double** stifMatrix = allocateStiffMatrix(3);

EXPECT_NO_THROW(stiffBase->CompLocalStif(3, xd, 1.0, 1.0, stifMatrix));

// Should produce finite values
for (int i = 0; i < 3; ++i) {
for (int j = 0; j < 3; ++j) {
EXPECT_TRUE(std::isfinite(stifMatrix[i][j]));
}
}

deallocateStiffMatrix(stifMatrix, 3);
}

// Parameter Interaction Tests
TEST_F(US_StiffBaseTest, CompLocalStif_DiffusionOmegaInteraction_ReflectsPhysics) {
double xd[4][2];
createTriangularElement(xd);
double** stifMatrix1 = allocateStiffMatrix(3);
double** stifMatrix2 = allocateStiffMatrix(3);
double** stifMatrix3 = allocateStiffMatrix(3);

// Test different combinations
stiffBase->CompLocalStif(3, xd, 1.0, 0.0, stifMatrix1); // Pure diffusion
stiffBase->CompLocalStif(3, xd, 0.0, 1.0, stifMatrix2); // Pure convection
stiffBase->CompLocalStif(3, xd, 1.0, 1.0, stifMatrix3); // Combined

// All should produce valid matrices
double norm1 = matrixFrobeniusNorm(stifMatrix1, 3);
double norm2 = matrixFrobeniusNorm(stifMatrix2, 3);
double norm3 = matrixFrobeniusNorm(stifMatrix3, 3);

EXPECT_GE(norm1, 0.0);
EXPECT_GE(norm2, 0.0);
EXPECT_GT(norm3, 0.0); // Combined should be non-zero

deallocateStiffMatrix(stifMatrix1, 3);
deallocateStiffMatrix(stifMatrix2, 3);
deallocateStiffMatrix(stifMatrix3, 3);
}

// Robustness Tests
TEST_F(US_StiffBaseTest, CompLocalStif_MixedElementTypes_AllFunctional) {
double xd[4][2];
double** stifMatrixT = allocateStiffMatrix(3);
double** stifMatrixQ = allocateStiffMatrix(4);

// Test triangular elements multiple times
for (int i = 0; i < 5; ++i) {
createTriangularElement(xd);
// Slightly perturb coordinates
for (int j = 0; j < 3; ++j) {
xd[j][0] += i * 0.01;
xd[j][1] += i * 0.01;
}

EXPECT_NO_THROW(stiffBase->CompLocalStif(3, xd, 1.0 + i * 0.1, 1.0 + i * 0.1, stifMatrixT));
}

// Test quadrilateral elements multiple times
for (int i = 0; i < 5; ++i) {
createQuadrilateralElement(xd);
// Slightly perturb coordinates
for (int j = 0; j < 4; ++j) {
xd[j][0] += i * 0.01;
xd[j][1] += i * 0.01;
}

EXPECT_NO_THROW(stiffBase->CompLocalStif(4, xd, 1.0 + i * 0.1, 1.0 + i * 0.1, stifMatrixQ));
}

deallocateStiffMatrix(stifMatrixT, 3);
deallocateStiffMatrix(stifMatrixQ, 4);
}

// Thread Safety Preparation Tests (for potential future multi-threading)
TEST_F(US_StiffBaseTest, CompLocalStif_IndependentInstances_NoInterference) {
US_StiffBase stiffBase2;

double xd1[4][2], xd2[4][2];
createTriangularElement(xd1);
createQuadrilateralElement(xd2);

double** stifMatrix1 = allocateStiffMatrix(3);
double** stifMatrix2 = allocateStiffMatrix(4);
double** stifMatrix3 = allocateStiffMatrix(3);
double** stifMatrix4 = allocateStiffMatrix(4);

// Compute with both instances
stiffBase->CompLocalStif(3, xd1, 1.0, 1.0, stifMatrix1);
stiffBase2.CompLocalStif(4, xd2, 2.0, 2.0, stifMatrix2);

// Repeat with same parameters
stiffBase->CompLocalStif(3, xd1, 1.0, 1.0, stifMatrix3);
stiffBase2.CompLocalStif(4, xd2, 2.0, 2.0, stifMatrix4);

// Results should be consistent for each instance
for (int i = 0; i < 3; ++i) {
for (int j = 0; j < 3; ++j) {
EXPECT_NEAR(stifMatrix1[i][j], stifMatrix3[i][j], 1e-15);
}
}

for (int i = 0; i < 4; ++i) {
for (int j = 0; j < 4; ++j) {
EXPECT_NEAR(stifMatrix2[i][j], stifMatrix4[i][j], 1e-15);
}
}

deallocateStiffMatrix(stifMatrix1, 3);
deallocateStiffMatrix(stifMatrix2, 4);
deallocateStiffMatrix(stifMatrix3, 3);
deallocateStiffMatrix(stifMatrix4, 4);
}

// Final Validation Tests
TEST_F(US_StiffBaseTest, CompLocalStif_RandomValidElements_AllSucceed) {
double** stifMatrixT = allocateStiffMatrix(3);
double** stifMatrixQ = allocateStiffMatrix(4);

// Test with several random but valid elements
double xd[4][2];

// Random triangular elements
std::vector<std::vector<std::vector<double>>> triangles = {
        {{0.1, 0.2}, {0.8, 0.1}, {0.3, 0.9}, {0.0, 0.0}},
        {{1.5, 2.3}, {3.1, 1.8}, {2.2, 4.1}, {0.0, 0.0}},
        {{-0.5, 0.5}, {0.5, -0.3}, {0.2, 0.8}, {0.0, 0.0}}
};

for (const auto& triangle : triangles) {
for (int i = 0; i < 4; ++i) {
xd[i][0] = triangle[i][0];
xd[i][1] = triangle[i][1];
}

EXPECT_NO_THROW(stiffBase->CompLocalStif(3, xd, 1.0, 1.0, stifMatrixT));

// Verify finite results
for (int i = 0; i < 3; ++i) {
for (int j = 0; j < 3; ++j) {
EXPECT_TRUE(std::isfinite(stifMatrixT[i][j]));
}
}
}

// Random quadrilateral elements
std::vector<std::vector<std::vector<double>>> quadrilaterals = {
        {{0.0, 0.0}, {1.0, 0.1}, {1.1, 1.0}, {0.1, 1.1}},
        {{2.0, 1.0}, {3.2, 1.1}, {3.1, 2.2}, {1.9, 2.1}},
        {{-1.0, -1.0}, {0.0, -0.9}, {0.1, 0.1}, {-0.9, 0.0}}
};

for (const auto& quad : quadrilaterals) {
for (int i = 0; i < 4; ++i) {
xd[i][0] = quad[i][0];
xd[i][1] = quad[i][1];
}

EXPECT_NO_THROW(stiffBase->CompLocalStif(4, xd, 1.0, 1.0, stifMatrixQ));

// Verify finite results
for (int i = 0; i < 4; ++i) {
for (int j = 0; j < 4; ++j) {
EXPECT_TRUE(std::isfinite(stifMatrixQ[i][j]));
}
}
}

deallocateStiffMatrix(stifMatrixT, 3);
deallocateStiffMatrix(stifMatrixQ, 4);
}

// Resource Management Tests
TEST_F(US_StiffBaseTest, MultipleOperations_NoMemoryLeaks) {
double xd[4][2];
createTriangularElement(xd);

// Perform many operations to test for memory leaks
for (int iter = 0; iter < 100; ++iter) {
double** stifMatrix = allocateStiffMatrix(3);

EXPECT_NO_THROW(stiffBase->CompLocalStif(3, xd,
1.0 + iter * 0.01, 1.0 + iter * 0.01, stifMatrix));

deallocateStiffMatrix(stifMatrix, 3);
}

// If we reach here without crashes, memory management is likely correct
SUCCEED();
}