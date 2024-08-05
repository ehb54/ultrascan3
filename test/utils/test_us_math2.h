#ifndef US_MATRIX_H
#define US_MATRIX_H

#include <QVector>
#include <cmath>

class US_Matrix
{
public:
    static bool lsfit(double* c, double* x, double* y, int N, int order);
    static bool Cholesky_Decomposition(double** a, int n);
    static bool Cholesky_SolveSystem(double** L, double* b, int n);
    static bool Cholesky_Invert(double** AA, double** AI, int nn);
    static double** construct(QVector<double*>& QVm, QVector<double>& QVd, int rows, int columns);
    static void tmm(double** AA, double** CC, int rows, int columns);
    static void tmm(double** AA, double** CC, int rows, int columns, bool fill);
    static void mvv(double** AA, double* bb, double* cc, int rows, int columns);
    static void tvv(double** AA, double* bb, double* cc, int rows, int columns);
    static void mmm(double** AA, double** BB, double** CC, int rows, int size, int columns);
    static void msum(double** AA, double** BB, double** CC, int rows, int columns);
    static void vsum(double* aa, double* bb, double* cc, int size);
    static void mident(double** CC, int size);
    static void mcopy(double** AA, double** CC, int rows, int columns);
    static void vcopy(double* aa, double* cc, int size);
    static void add_diag(double** CC, double ss, int size);
    static void add(double** CC, double ss, int rows, int columns);
    static void scale(double** CC, double ss, int rows, int columns);
    static double dotproduct(double* aa, double* bb, int size);
    static double dotproduct(double* aa, int size);
    static void print_vector(double* v, int n);
    static void print_matrix(double** A, int rows, int columns);
    static void LU_Decomposition(double** matrix, int* index, bool parity, int n);
    static void LU_BackSubstitute(double** A, double*& b, int* index, int n);
    static void LU_SolveSystem(double** A, double*& b, int n);
    static void mmv(float** result, double** vector, float*** matrix, int row, int column);
    static void mmv(float** result, double** vector, double*** matrix, int row, int column);
    static void mmv(float** result, float** vector, double*** matrix, int row, int column);
    static void mmv(float** result, float** vector, float*** matrix, int row, int column);
    static float dotproduct(float** v1, float** v2, int size);
    static void vvt(float*** result, float** v1, float** v2, int size);
    static void calc_A_transpose_A(double*** A, double*** product, unsigned int rows, unsigned int columns, unsigned int threads);
    static void calc_A_transpose_A(float*** A, float*** product, unsigned int rows, unsigned int columns);
};

#endif // US_MATRIX_H
