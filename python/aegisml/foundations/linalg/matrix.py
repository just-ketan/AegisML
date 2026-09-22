from __future__ import annotations
from typing import Iterable
from .vector import Vector

class Matrix:
    '''
    small from-scratch implementation of mathematical matrix
    A = [[1,2,3],[4,5,6]] has shape = (2,3) --> 2 rows and 3 cols
    '''
    def __init__(self, values: Iterable[Iterable[float]]):
        # construct matrix from an iterable of rows, matrix must be mathematically rectangular
        self._values = [[float(value) for value in row] for row in values]
        self._validate_rectangular()

    def __len__(self) -> int:
        # return number of rows in matrix
        return len(self._values)

    @property
    def shape(self) -> tuple[int, int]:
        rows = len(self._values)
        if rows == 0:
            return (0,0)
        cols = len(self._values[0])
        return (rows, cols)

    def __getitem__(self, index:int) -> list[float]:
        # return a row identified by index
        return self._values[index]

    def __iter__(self):
        return iter(self._values)

    def __repr__(self) -> str:
        # represent the matrix
        return f"Matrix({self._values!r})"

    def __add__(self, other:Matrix) -> Matrix:
        # matrix addition implementation
        self._check_same_shape(other)
        return Matrix([
            [a+b for a,b in zip(row_a, row_b)] for row_a, row_b in zip(self._values, other._values)
        ])

    def __sub__(self, other:Matrix) -> Matrix:
        # matrix substraction implementation
        self._check_same_shape(other)
        return Matrix([
            [a-b for a,b in zip(row_a, row_b)] for row_a, row_b in zip(self._values, other._values)
        ])

    def __mul__(self, scalar:float) -> Matrix:
        return Matrix([
            [value*scalar for value in row] for row in self._values
        ])
    def __rmul__(self, scalar:float) -> Matrix:
        return self*scalar

    def _check_same_shape(self, other:Matrix) -> None:
        if self.shape != other.shape:
            raise ValueError(f"Shape Mismatch: {self.shape} != {other.shape}")

    def _validate_rectangular(self) -> None:
        if not self._values:
            return
        expected_cols = len(self._values[0])

        for row in self._values:
            if len(row) != expected_cols:
                raise ValueError("Matrix must be rectangular")

    def __matmul__(self, other):
        '''
        # MxN * Nx1 will give a vector of Mx1
            A = [1 2 3]
                [4 5 6]

            x = [10]
                [20]
                [30]

            Ax =[1(10) + 2(20) + 3(30)]
                [4(10) + 5(20) + 6(30)]

            = [140]
              [320]
        '''
        if isinstance(other, Vector):
            if self.shape[1] != len(other):
                raise ValueError(f"Shape mismatch: {self.shape} cannot be multiplied by vector of dimension {len(other)}")

            return Vector(sum(value * other[j] for j, value in enumerate(row)) for row in self._values)

        if isinstance(other, Matrix):
            if self.shape[1] != other.shape[0]:
                raise ValueError(f"Shape mismatch {self.shape}")
            res = []
            for row in self._values:
                res_ = []
                for col in range(other.shape[1]):
                    val = sum(row[k]*other._values[k][col] for k in range (self.shape[1]))
                    res_.append(val)
                res.append(res_)
            return Matrix(res)

        raise TypeError("Unsupported type for @")

    def transpose(self) -> Matrix:
        # (Aᵀ)_ij = A_ji
        if not self._values:
            return Matrix([])

        return Matrix([[self._values[row][col] for row in range(self.shape[0])] for col in range(self.shape[1])])

    @property
    def T(self) -> Matrix:
        # return transpose using convenient property syntax
        return self.transpose()

    @classmethod
    def identity(cls, size:int) -> Matrix:
        # construct a square identity matrix
        if size <= 0:
            raise ValueError("Identity matrix size must be positive")

        return cls([[1.0 if row == col else 0.0 for col in range(size)] for row in range(size)])

    # determinant det(A) = ad - bc for 2x2 square matrix, for larger, we need recursive cofactor expansion along first row
    # det(A) != 0 -> A is invertible, else det(A) == 0, A is singular and dont have ordinary inverse

    def determinant(self) -> float:
        rows, cols = self.shape
        if rows != cols:
            raise ValueError("invoked by non square matrix")

        if rows == 0:
            raise ValueError("determinant dont exist for empty matrices")

        if rows == 1:
            return self._values[0][0]

        if rows == 2:
            a,b = self._values[0]
            c,d = self._values[1]
            return a*d - b*c

        # non size 2 matrix
        det = 0.0
        for col in range(cols):
            sign = 1.0 if col%2==0 else -1.0
            minor = self._minor(0, col)
            det += (sign * self._values[0][col] * minor.determinant())

        return det

    # helper funciton "minor": removes one row and one column from current matrix and returns reduced matrix
    def _minor(self, excl_row:int, excl_col:int) -> Matrix:
        '''
        For example, removing row 0 and column 1 from:

            [1 2 3]
            [4 5 6]
            [7 8 9]

        produces:

            [4 6]
            [7 9]
        '''
        return Matrix([[val for col,val in enumerate(row) if col != excl_col] for row_idx, row in enumerate(self._values) if row_idx != excl_row])

    def inverse(self) -> Matrix:
        # AA⁻¹ = A⁻¹A = I
        # A⁻¹ = adj(A) / det(A)
        rows, cols = self.shape
        if rows != cols:
            raise ValueError("Inverse function invoked by non-square matrix")

        det = self.determinant()
        if abs(det) < 1e-12:
            raise ValueError("Matrix is singular and cannot be inverted")

        if rows == 1:
            return Matrix([1.0/self._values[0][0]])

        cofactors = []
        for row in range(rows):
            cof_row = []
            for col in range(cols):
                sign = 1.0 if (row+col)%2 == 0 else -1.0
                minor = self._minor(row, col)
                cof_row.append(sign*minor.determinant())
            cofactors.append(cof_row)
        cofactor_matrix = Matrix(cofactors)
        adjugate = cofactor_matrix.transpose()

        return adjugate * (1.0/det)

    def rank(self) -> int:
        """
        Compute the rank of the matrix.

        Rank is the number of linearly independent rows or,
        equivalently, columns.

        This implementation uses Gaussian elimination and counts
        the number of pivot rows.

        Example:

            [1 2]
            [2 4]

        becomes:

            [1 2]
            [0 0]

        Therefore:

            rank = 1

        Rank is important in machine learning because it tells us
        how much independent information is contained in a matrix.
        """

        matrix = [
            row.copy()
            for row in self._values
        ]

        rows, columns = self.shape

        rank = 0
        pivot_column = 0

        while rank < rows and pivot_column < columns:

            # Search for a row containing a non-zero pivot
            # in the current column.
            pivot_row = None

            for row in range(rank, rows):
                if abs(matrix[row][pivot_column]) > 1e-12:
                    pivot_row = row
                    break

            # No pivot exists in this column.
            # Move to the next column and continue searching.
            if pivot_row is None:
                pivot_column += 1
                continue

            # Move the pivot row into the current rank position.
            matrix[rank], matrix[pivot_row] = (
                matrix[pivot_row],
                matrix[rank],
            )

            # Normalize the pivot row so that the pivot becomes 1.
            pivot = matrix[rank][pivot_column]

            for column in range(pivot_column, columns):
                matrix[rank][column] /= pivot

            # Eliminate this pivot from every other row.
            for row in range(rows):
                if row == rank:
                    continue

                factor = matrix[row][pivot_column]

                for column in range(pivot_column, columns):
                    matrix[row][column] -= (
                        factor * matrix[rank][column]
                    )

            # We successfully found one independent pivot.
            rank += 1
            pivot_column += 1

        return rank