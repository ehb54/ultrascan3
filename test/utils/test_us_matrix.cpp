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
    int N = 5;
    int order = 2;
    double x[5] = {1, 2, 3, 4, 5};
    double y[5] = {1, 4, 9, 16, 25};
    double c[2] = {0, 0};

    bool result = US_Matrix::lsfit(c, x, y, N, order);

    QVERIFY(result);
    QVERIFY(c[0] > 0.0);
    QVERIFY(c[1] > 0.0);
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

