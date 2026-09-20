from __future__ import annotations
from typing import Iterable

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