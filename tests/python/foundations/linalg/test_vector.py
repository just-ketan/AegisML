import math

import pytest

from aegisml.foundations.linalg import Vector


def test_vector_construction():
    vector = Vector([1, 2, 3])

    assert len(vector) == 3
    assert vector[0] == 1.0
    assert vector[1] == 2.0
    assert vector[2] == 3.0


def test_vector_addition():
    x = Vector([1, 2, 3])
    y = Vector([4, 5, 6])

    result = x + y

    assert list(result) == [5.0, 7.0, 9.0]


def test_vector_subtraction():
    x = Vector([4, 5, 6])
    y = Vector([1, 2, 3])

    result = x - y

    assert list(result) == [3.0, 3.0, 3.0]


def test_scalar_multiplication():
    x = Vector([1, 2, 3])

    assert list(x * 2) == [2.0, 4.0, 6.0]
    assert list(2 * x) == [2.0, 4.0, 6.0]


def test_dot_product():
    x = Vector([1, 2, 3])
    y = Vector([4, 5, 6])

    assert x.dot(y) == 32.0


def test_norm():
    x = Vector([3, 4])

    assert x.norm() == 5.0


def test_distance():
    x = Vector([0, 0])
    y = Vector([3, 4])

    assert x.distance(y) == 5.0


def test_cosine_similarity():
    x = Vector([1, 0])
    y = Vector([1, 0])

    assert x.cosine_similarity(y) == 1.0


def test_orthogonal_vectors_have_zero_cosine_similarity():
    x = Vector([1, 0])
    y = Vector([0, 1])

    assert math.isclose(
        x.cosine_similarity(y),
        0.0,
        abs_tol=1e-12,
    )


def test_dimension_mismatch():
    x = Vector([1, 2])
    y = Vector([1, 2, 3])

    with pytest.raises(ValueError):
        x + y


def test_cosine_similarity_zero_vector():
    x = Vector([0, 0])
    y = Vector([1, 2])

    with pytest.raises(ValueError):
        x.cosine_similarity(y)