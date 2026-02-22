#pragma once

#include <array>
#include <cassert>
#include <cmath>
#include <utility>

namespace engine {
namespace math {

/** Fixed-size matrix for EKF. Row-major storage. */
template <int Rows, int Cols>
class Matrix {
public:
    static constexpr int R = Rows;
    static constexpr int C = Cols;

    std::array<double, Rows * Cols> data{};

    double& operator()(int i, int j) {
        assert(i >= 0 && i < Rows && j >= 0 && j < Cols);
        return data[i * Cols + j];
    }
    double operator()(int i, int j) const {
        assert(i >= 0 && i < Rows && j >= 0 && j < Cols);
        return data[i * Cols + j];
    }
    /** Single-index access for column vectors (Cols==1) */
    double& operator()(int i) {
        static_assert(Cols == 1, "single-index only for column vectors");
        return (*this)(i, 0);
    }
    double operator()(int i) const {
        static_assert(Cols == 1, "single-index only for column vectors");
        return (*this)(i, 0);
    }

    void setZero() { data.fill(0); }
    void setIdentity() {
        setZero();
        for (int i = 0; i < std::min(Rows, Cols); ++i) (*this)(i, i) = 1.0;
    }

    Matrix<Cols, Rows> transposed() const {
        Matrix<Cols, Rows> T;
        for (int i = 0; i < Rows; ++i)
            for (int j = 0; j < Cols; ++j)
                T(j, i) = (*this)(i, j);
        return T;
    }
};

/** Matrix multiply: A * B */
template <int M, int N, int P>
Matrix<M, P> operator*(const Matrix<M, N>& A, const Matrix<N, P>& B) {
    Matrix<M, P> C;
    C.setZero();
    for (int i = 0; i < M; ++i)
        for (int k = 0; k < N; ++k)
            for (int j = 0; j < P; ++j)
                C(i, j) += A(i, k) * B(k, j);
    return C;
}

/** Matrix + Matrix */
template <int R, int C>
Matrix<R, C> operator+(const Matrix<R, C>& A, const Matrix<R, C>& B) {
    Matrix<R, C> C_out;
    for (int i = 0; i < R * C; ++i) C_out.data[i] = A.data[i] + B.data[i];
    return C_out;
}

/** Matrix - Matrix */
template <int R, int C>
Matrix<R, C> operator-(const Matrix<R, C>& A, const Matrix<R, C>& B) {
    Matrix<R, C> C_out;
    for (int i = 0; i < R * C; ++i) C_out.data[i] = A.data[i] - B.data[i];
    return C_out;
}

/** Invert square matrix (Gauss-Jordan). Returns false if singular. */
template <int N>
bool invert(const Matrix<N, N>& A, Matrix<N, N>& inv) {
    Matrix<N, 2*N> aug;
    aug.setZero();
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) aug(i, j) = A(i, j);
        aug(i, N + i) = 1.0;
    }

    for (int k = 0; k < N; ++k) {
        int pivot_row = k;
        double maxv = std::abs(aug(k, k));
        for (int i = k + 1; i < N; ++i) {
            const double v = std::abs(aug(i, k));
            if (v > maxv) { maxv = v; pivot_row = i; }
        }
        if (maxv < 1e-15) return false;
        if (pivot_row != k) {
            for (int j = 0; j < 2*N; ++j) std::swap(aug(k, j), aug(pivot_row, j));
        }
        const double pivot = aug(k, k);
        for (int j = 0; j < 2*N; ++j) aug(k, j) /= pivot;
        for (int i = 0; i < N; ++i) {
            if (i == k) continue;
            const double f = aug(i, k);
            for (int j = 0; j < 2*N; ++j) aug(i, j) -= f * aug(k, j);
        }
    }
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            inv(i, j) = aug(i, N + j);
    return true;
}

/** Cholesky decomposition: A = L*L^T, A must be SPD. L lower triangular. */
template <int N>
bool cholesky(const Matrix<N, N>& A, Matrix<N, N>& L) {
    L.setZero();
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j <= i; ++j) {
            double s = A(i, j);
            for (int k = 0; k < j; ++k) s -= L(i, k) * L(j, k);
            if (i == j) {
                if (s <= 0) return false;
                L(i, i) = std::sqrt(s);
            } else {
                L(i, j) = s / L(j, j);
            }
        }
    }
    return true;
}

}  // namespace math
}  // namespace engine
