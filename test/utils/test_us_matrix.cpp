#include <QtTest/QtTest>
#include "us_matrix.h"

class TestUSMatrix : public QObject
{

    private slots:
    void testLsfit();
    void testCholeskyDecomposition();
    void testCholeskySolveSystem();
    void testConstruct();
};

void TestUSMatrix::testLsfit()
{
    // Test data: y = 2x + 1
    double x[] = {1, 2, 3, 4, 5};
    double y[] = {3, 5, 7, 9, 11};
    int N = 5;
    int order = 2; // Linear fit (y = c[1]*x + c[0])

    double c[2] = {0};

    bool status = US_Matrix::lsfit(c, x, y, N, order);

    QVERIFY(status == true);

    double tolerance = 0.0001;

    // Expected coefficients: c[1] (slope) = 2, c[0] (intercept) = 1
    QVERIFY2(qAbs(c[0] - 1.0) < tolerance, qPrintable(QString("Expected intercept: 1.0, but got: %1").arg(c[0])));
    QVERIFY2(qAbs(c[1] - 2.0) < tolerance, qPrintable(QString("Expected slope: 2.0, but got: %1").arg(c[1])));
}

void TestUSMatrix::testCholeskyDecomposition()
{
    int n = 3;
    double matrixData[3][3] = {
            {4, 12, -16},
            {12, 37, -43},
            {-16, -43, 98}
    };

    QVector<double*> vecA;
    QVector<double> datA;
    double** matrix = US_Matrix::construct(vecA, datA, n, n);

    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            matrix[i][j] = matrixData[i][j];

    bool result = US_Matrix::Cholesky_Decomposition(matrix, n);

    QVERIFY(result);
}

void TestUSMatrix::testCholeskySolveSystem()
{
    int n = 3;
    double matrixData[3][3] = {
            {4, 12, -16},
            {12, 37, -43},
            {-16, -43, 98}
    };
    double b[3] = {1, 1, 1};

    QVector<double*> vecA;
    QVector<double> datA;
    double** matrix = US_Matrix::construct(vecA, datA, n, n);

    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            matrix[i][j] = matrixData[i][j];

    bool decompResult = US_Matrix::Cholesky_Decomposition(matrix, n);
    QVERIFY(decompResult);

    bool solveResult = US_Matrix::Cholesky_SolveSystem(matrix, b, n);
    QVERIFY(solveResult);

    QVERIFY(b[0] != 0.0);
    QVERIFY(b[1] != 0.0);
    QVERIFY(b[2] != 0.0);
}

void TestUSMatrix::testConstruct()
{
    int rows = 3;
    int columns = 3;
    QVector<double*> vecA;
    QVector<double> datA;

    double** matrix = US_Matrix::construct(vecA, datA, rows, columns);

    QVERIFY(matrix != nullptr);
    QCOMPARE(vecA.size(), rows);
    QCOMPARE(datA.size(), rows * columns);
}

