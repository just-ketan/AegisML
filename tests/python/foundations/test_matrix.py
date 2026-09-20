import pytest

from aegisml.foundations.linalg import Matrix


def test_matrix_construction():
    matrix = Matrix([
        [1, 2, 3],
        [4, 5, 6],
    ])

    assert matrix.shape == (2, 3)
    assert matrix[0][0] == 1.0
    assert matrix[1][2] == 6.0


def test_matrix_iteration():
    matrix = Matrix([
        [1, 2],
        [3, 4],
    ])

    assert list(matrix) == [
        [1.0, 2.0],
        [3.0, 4.0],
    ]


def test_matrix_addition():
    a = Matrix([
        [1, 2],
        [3, 4],
    ])

    b = Matrix([
        [5, 6],
        [7, 8],
    ])

    result = a + b

    assert result.shape == (2, 2)
    assert list(result) == [
        [6.0, 8.0],
        [10.0, 12.0],
    ]


def test_matrix_subtraction():
    a = Matrix([
        [5, 6],
        [7, 8],
    ])

    b = Matrix([
        [1, 2],
        [3, 4],
    ])

    result = a - b

    assert list(result) == [
        [4.0, 4.0],
        [4.0, 4.0],
    ]


def test_scalar_multiplication():
    matrix = Matrix([
        [1, 2],
        [3, 4],
    ])

    assert list(matrix * 2) == [
        [2.0, 4.0],
        [6.0, 8.0],
    ]

    assert list(2 * matrix) == [
        [2.0, 4.0],
        [6.0, 8.0],
    ]


def test_shape_mismatch():
    a = Matrix([
        [1, 2],
        [3, 4],
    ])

    b = Matrix([
        [1, 2, 3],
        [4, 5, 6],
    ])

    with pytest.raises(ValueError):
        a + b


def test_ragged_matrix_rejected():
    with pytest.raises(ValueError):
        Matrix([
            [1, 2, 3],
            [4, 5],
        ])


def test_empty_matrix():
    matrix = Matrix([])

    assert matrix.shape == (0, 0)