import pytest

from aegisml.foundations.linalg import Matrix, Vector


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

def test_matrix_vector_mux():
    matrix=  Matrix([[1,2,3],[4,5,6],])
    vector = Vector([10,20,30])

    res = matrix @ vector
    assert list(res) == [140.0, 320.0]

def test_matrix_vector_dimension_mismatch():
    matrix = Matrix([
        [1, 2, 3],
        [4, 5, 6],
    ])

    vector = Vector([1, 2])

    with pytest.raises(ValueError):
        matrix @ vector

def test_matrix_vector_multiplication_identity():
    matrix = Matrix([
        [1, 0],
        [0, 1],
    ])

    vector = Vector([7, 11])

    result = matrix @ vector

    assert list(result) == [7.0, 11.0]

def test_matrix_matrix_multiplication():
    a = Matrix([
        [1, 2, 3],
        [4, 5, 6],
    ])

    b = Matrix([
        [7, 8],
        [9, 10],
        [11, 12],
    ])

    result = a @ b

    assert result.shape == (2, 2)

    assert list(result) == [
        [58.0, 64.0],
        [139.0, 154.0],
    ]

def test_matrix_matrix_dimension_mismatch():
    a = Matrix([
        [1, 2, 3],
        [4, 5, 6],
    ])

    b = Matrix([
        [1, 2],
        [3, 4],
    ])

    with pytest.raises(ValueError):
        a @ b

def test_matrix_multiplication_identity():
    matrix = Matrix([
        [1, 2],
        [3, 4],
    ])

    identity = Matrix([
        [1, 0],
        [0, 1],
    ])

    result = matrix @ identity

    assert list(result) == [
        [1.0, 2.0],
        [3.0, 4.0],
    ]

def test_matrix_multiplication_associativity():
    a = Matrix([
        [1, 2],
        [3, 4],
    ])

    b = Matrix([
        [5, 6],
        [7, 8],
    ])

    c = Matrix([
        [9, 10],
        [11, 12],
    ])

    left = (a @ b) @ c
    right = a @ (b @ c)

    assert list(left) == list(right)

def test_matrix_transpose():
    matrix = Matrix([
        [1, 2, 3],
        [4, 5, 6],
    ])

    result = matrix.transpose()

    assert result.shape == (3, 2)

    assert list(result) == [
        [1.0, 4.0],
        [2.0, 5.0],
        [3.0, 6.0],
    ]


def test_matrix_transpose_property():
    matrix = Matrix([
        [1, 2],
        [3, 4],
        [5, 6],
    ])

    assert list(matrix.T) == [
        [1.0, 3.0, 5.0],
        [2.0, 4.0, 6.0],
    ]


def test_double_transpose():
    matrix = Matrix([
        [1, 2, 3],
        [4, 5, 6],
    ])

    assert list(matrix.T.T) == list(matrix)

def test_identity_matrix():
    identity = Matrix.identity(3)

    assert identity.shape == (3, 3)

    assert list(identity) == [
        [1.0, 0.0, 0.0],
        [0.0, 1.0, 0.0],
        [0.0, 0.0, 1.0],
    ]


def test_identity_multiplication():
    matrix = Matrix([
        [1, 2],
        [3, 4],
    ])

    identity = Matrix.identity(2)

    assert list(matrix @ identity) == list(matrix)
    assert list(identity @ matrix) == list(matrix)

def test_determinant_2x2():
    matrix = Matrix([
        [2, 3],
        [1, 4],
    ])

    assert matrix.determinant() == 5.0


def test_determinant_3x3():
    matrix = Matrix([
        [1, 2, 3],
        [0, 4, 5],
        [1, 0, 6],
    ])

    assert matrix.determinant() == 22.0


def test_determinant_singular_matrix():
    matrix = Matrix([
        [1, 2],
        [2, 4],
    ])

    assert matrix.determinant() == 0.0


def test_determinant_non_square_rejected():
    matrix = Matrix([
        [1, 2, 3],
        [4, 5, 6],
    ])

    with pytest.raises(ValueError):
        matrix.determinant()

def test_inverse_2x2():
    matrix = Matrix([
        [4, 7],
        [2, 6],
    ])

    inverse = matrix.inverse()

    assert inverse.shape == (2, 2)

    assert inverse[0][0] == pytest.approx(0.6)
    assert inverse[0][1] == pytest.approx(-0.7)
    assert inverse[1][0] == pytest.approx(-0.2)
    assert inverse[1][1] == pytest.approx(0.4)

def test_inverse_identity_property():
    matrix = Matrix([
        [4, 7],
        [2, 6],
    ])

    identity = Matrix.identity(2)

    result = matrix @ matrix.inverse()

    for i in range(2):
        for j in range(2):
            assert result[i][j] == pytest.approx(
                identity[i][j]
            )

def test_inverse_singular_matrix_rejected():
    matrix = Matrix([
        [1, 2],
        [2, 4],
    ])

    with pytest.raises(ValueError):
        matrix.inverse()

def test_rank_full_rank():
    matrix = Matrix([
        [1, 0],
        [0, 1],
    ])

    assert matrix.rank() == 2


def test_rank_dependent_rows():
    matrix = Matrix([
        [1, 2],
        [2, 4],
    ])

    assert matrix.rank() == 1


def test_rank_zero_matrix():
    matrix = Matrix([
        [0, 0],
        [0, 0],
    ])

    assert matrix.rank() == 0


def test_rank_rectangular_matrix():
    matrix = Matrix([
        [1, 2, 3],
        [4, 5, 6],
    ])

    assert matrix.rank() == 2

def test_invertible_matrix_full_rank():
    matrix = Matrix([
        [4, 7],
        [2, 6],
    ])

    assert matrix.determinant() != 0
    assert matrix.rank() == 2

def test_singular_matrix_rank_deficient():
    matrix = Matrix([
        [1, 2],
        [2, 4],
    ])

    assert matrix.determinant() == 0
    assert matrix.rank() == 1


def test_matrix_equality():
    a = Matrix([
        [1, 2],
        [3, 4],
    ])

    b = Matrix([
        [1, 2],
        [3, 4],
    ])

    assert a == b


def test_matrix_equality_tolerates_floating_point_error():
    a = Matrix([
        [1.0, 2.0],
        [3.0, 4.0],
    ])

    b = Matrix([
        [1.0 + 1e-13, 2.0],
        [3.0, 4.0 - 1e-13],
    ])

    assert a == b


def test_matrix_equality_different_shapes():
    a = Matrix([
        [1, 2],
        [3, 4],
    ])

    b = Matrix([
        [1, 2, 3],
        [4, 5, 6],
    ])

    assert a != b