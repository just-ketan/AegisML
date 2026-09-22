from aegisml.foundations.linalg.linear_algebra import (
    vectors_to_matrix, 
    are_linearly_independent, 
    vector_is_in_span, 
    is_basis, 
    column_space_basis,
    _rref,
    null_space_basis,
    are_orthogonal,
    project_onto,
    gram_schmidt,
    characteristic_polynomial,
    eigenvalues,
    eigenvectors,
    diagonalize,
    quadratic_form,
    is_positive_definite,
    is_symmetric,
    covariance_matrix,
    )
from aegisml.foundations.linalg.matrix import Matrix
from aegisml.foundations.linalg.vector import Vector

import pytest


def test_rref():
    matrix = Matrix([
        [1, 2, 3],
        [2, 4, 6],
        [1, 1, 1],
    ])
    rref, pivots = _rref(matrix)
    assert pivots == [0, 1]
    assert rref[0][0] == 1
    assert rref[0][1] == 0
    assert rref[1][0] == 0
    assert rref[1][1] == 1
    assert rref[2][0] == 0
    assert rref[2][1] == 0

def test_rref_does_not_modify_original():
    matrix = Matrix([
        [1, 2],
        [2, 4],
    ])

    original = matrix._values.copy()
    _rref(matrix)
    assert matrix._values == original

def test_column_space_basis():
    matrix = Matrix([
        [1, 2, 3],
        [2, 4, 6],
        [1, 1, 1],
    ])
    basis = column_space_basis(matrix)
    assert len(basis) == 2
    assert basis[0] == Vector([1, 2, 1])
    assert basis[1] == Vector([2, 4, 1])

def test_column_space_basis_is_independent():
    matrix = Matrix([
        [1, 2, 3],
        [2, 4, 6],
        [1, 1, 1],
    ])
    basis = column_space_basis(matrix)
    assert are_linearly_independent(basis)

def test_vector_equality():
    assert Vector([1, 2, 3]) == Vector([1.0, 2.0, 3.0])
    assert Vector([1, 2, 3]) != Vector([1, 2, 4])

def test_vector_equality_tolerates_floating_point_error():
    assert Vector([1.0,2.0,3.0,]) == Vector([1.0 + 1e-13,2.0 - 1e-13,3.0 + 1e-13,])

def test_vector_equality_different_dimensions():
    assert Vector([1, 2]) != Vector([1, 2, 3])

def test_null_space_basis():
    matrix = Matrix([[1, 2, 3],[2, 4, 6],])
    basis = null_space_basis(matrix)
    assert len(basis) == 2

    assert basis[0] == Vector([-2.0,1.0,0.0,])
    assert basis[1] == Vector([-3.0,0.0,1.0,])

def test_null_space_vectors_are_actually_null():
    matrix = Matrix([[1, 2, 3],[2, 4, 6],])

    basis = null_space_basis(matrix)

    for vector in basis:
        result = matrix @ vector
        assert result == Vector([0.0,0.0,])

def test_null_space_basis_is_independent():
    matrix = Matrix([[1, 2, 3],[2, 4, 6],])
    basis = null_space_basis(matrix)
    assert are_linearly_independent(basis)

def test_are_orthogonal():
    u = Vector([1, 2])
    v = Vector([2, -1])
    assert are_orthogonal(u, v)

def test_are_not_orthogonal():
    u = Vector([1, 2])
    v = Vector([3, 4])
    assert not are_orthogonal(u, v)

def test_orthogonality_requires_same_dimension():
    u = Vector([1, 2])
    v = Vector([1, 2, 3])
    with pytest.raises(ValueError):
        are_orthogonal(u, v)

def test_projection():
    vector = Vector([3, 4])
    direction = Vector([1, 0])
    projection = project_onto(vector,direction,)
    assert projection == Vector([3.0,0.0,])

def test_projection_onto_diagonal():
    vector = Vector([2, 0])
    direction = Vector([1, 1])
    projection = project_onto(vector,direction,)
    assert projection == Vector([1.0,1.0,])

def test_projection_onto_zero_vector():
    vector = Vector([1, 2])
    direction = Vector([0, 0])
    with pytest.raises(ValueError):
        project_onto(vector,direction,)

def test_projection_residual_is_orthogonal():
    vector = Vector([3, 4])
    direction = Vector([1, 2])

    projection = project_onto(vector,direction,)
    residual = vector - projection

    assert are_orthogonal(residual,direction,)

def test_gram_schmidt():
    vectors = [Vector([1, 0]),Vector([1, 1]),]
    basis = gram_schmidt(vectors)

    assert len(basis) == 2
    assert basis[0] == Vector([1.0,0.0,])
    assert basis[1] == Vector([0.0,1.0,])

def test_gram_schmidt_produces_unit_vectors():
    vectors = [Vector([1, 2]),Vector([2, 1]),]
    basis = gram_schmidt(vectors)
    for vector in basis:
        assert abs(vector.norm() - 1.0) <= 1e-12

def test_gram_schmidt_produces_orthogonal_vectors():
    vectors = [Vector([1, 2]),Vector([2, 1]),]
    basis = gram_schmidt(vectors)
    assert are_orthogonal(basis[0],basis[1],)

def test_gram_schmidt_rejects_dependent_vectors():
    vectors = [Vector([1, 2]),Vector([2, 4]),]
    with pytest.raises(ValueError):
        gram_schmidt(vectors)

def test_gram_schmidt_requires_same_dimension():
    vectors = [Vector([1, 2]),Vector([1, 2, 3]),]
    with pytest.raises(ValueError):
        gram_schmidt(vectors)

def test_characteristic_polynomial():
    matrix = Matrix([[2, 1],[0, 3],])
    coefficients = characteristic_polynomial(matrix)
    assert coefficients == (1.0,-5.0,6.0,)

def test_characteristic_polynomial_general_matrix():
    matrix = Matrix([[4, 2],[1, 3],])
    coefficients = characteristic_polynomial(matrix)
    assert coefficients == (1.0,-7.0,10.0,)

def test_eigenvalues_distinct_real():
    matrix = Matrix([[2, 1],[0, 3],])
    values = eigenvalues(matrix)
    assert set(values) == {complex(2, 0),complex(3, 0),}

def test_eigenvalues_repeated():
    matrix = Matrix([[2, 1],[0, 2],])
    values = eigenvalues(matrix)
    assert values == (complex(2, 0),complex(2, 0),)

def test_eigenvalues_complex():
    matrix = Matrix([[0, -1],[1, 0],])
    values = eigenvalues(matrix)
    assert set(values) == {complex(0, 1),complex(0, -1),}

def test_eigenvectors():
    matrix = Matrix([[2, 1],[0, 3],])
    vectors = eigenvectors(matrix)
    assert vectors[complex(2, 0)][0] == Vector([1.0,0.0,])
    assert vectors[complex(3, 0)][0] == Vector([1.0,1.0,])

def test_eigenvectors_satisfy_eigenvalue_equation():
    matrix = Matrix([[2, 1],[0, 3],])
    vectors = eigenvectors(matrix)
    for eigenvalue, basis in vectors.items():
        for vector in basis:
            lhs = matrix @ vector
            rhs = vector * eigenvalue.real
            assert lhs == rhs

def test_eigenvectors_are_nonzero():
    matrix = Matrix([[2, 1],[0, 3],])
    vectors = eigenvectors(matrix)
    for basis in vectors.values():
        for vector in basis:
            assert vector.norm() > 1e-12

def test_repeated_eigenvalue_eigenspace():
    matrix = Matrix([[2, 1],[0, 2],])
    vectors = eigenvectors(matrix)
    basis = vectors[complex(2, 0)]
    assert len(basis) == 1
    assert basis[0] == Vector([1.0,0.0,])

def test_diagonalize():
    matrix = Matrix([[2, 1],[0, 3],])
    P, D, P_inverse = diagonalize(matrix)
    reconstructed = P @ D @ P_inverse
    assert reconstructed == matrix

def test_diagonalize():
    matrix = Matrix([[2, 1],[0, 3],])
    P, D, P_inverse = diagonalize(matrix)
    reconstructed = P @ D @ P_inverse
    assert reconstructed == matrix

def test_diagonalization_inverse():
    matrix = Matrix([[2, 1],[0, 3],])
    P, D, P_inverse = diagonalize(matrix)
    identity = P_inverse @ P
    assert identity == Matrix.identity(2)

def test_quadratic_form():
    matrix = Matrix([[2, 1],[1, 3],])
    vector = Vector([2,1,])
    # A x = [5, 5]
    #
    # x^T A x = [2,1] · [5,5]
    #          = 15

    assert quadratic_form(matrix,vector,) == 15.0

def test_quadratic_form():
    matrix = Matrix([[2, 1],[1, 3],])
    vector = Vector([2,1,])

    # A x = [5, 5]
    #
    # x^T A x = [2,1] · [5,5]
    #          = 15

    assert quadratic_form(matrix,vector,) == 15.0

def test_is_positive_definite():
    matrix = Matrix([[2, 1],[1, 3],])
    assert is_positive_definite(matrix)

def test_is_symmetric():
    matrix = Matrix([[2, 1],[1, 3],])
    assert is_symmetric(matrix)

def test_is_symmetric():
    matrix = Matrix([[2, 1],[1, 3],])
    assert is_symmetric(matrix)

def test_non_symmetric_matrix_is_not_positive_definite():
    matrix = Matrix([[1, 2],[0, 1],])
    assert not is_positive_definite(matrix)

def test_covariance_matrix():
    data = Matrix([[1, 2],[2, 4],[3, 6],])
    covariance = covariance_matrix(data)
    assert covariance == Matrix([[1.0, 2.0],[2.0, 4.0],])

def test_covariance_matrix():
    data = Matrix([[1, 2],[2, 4],[3, 6],])
    covariance = covariance_matrix(data)
    assert covariance == Matrix([[1.0, 2.0],[2.0, 4.0],])

def test_covariance_matrix_is_positive_semidefinite():
    data = Matrix([[1, 2],[2, 4],[3, 6],])
    covariance = covariance_matrix(data)
    vector = Vector([1,-1,])
    assert quadratic_form(covariance,vector,) >= 0