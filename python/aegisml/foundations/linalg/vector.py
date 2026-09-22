from __future__ import annotations

import math
from typing import Iterable

class Vector:
    """
    a small from scratch implementation of mathematical vector
    """

    def __init__(self, values:Iterable[float]):
        self._values = [float(value) for value in values]

    def __len__(self) -> int:
        return len(self._values)

    # return component of a vector at given index x[0] -> x1, x[1] -> x2 ....
    def __getitem__(self, index:int) -> float:
        return self._values[index]

    # run iterator over vector's components, allows vector to be used in loops and converted to toher iterable structures
    def __iter__(self):
        return iter(self._values)

    # returns unambiguous string representing the vector
    def __repr__(self) -> str:
        return f"Vector({self._values!r})"

    # adds two vectors x = {x1, x2, x3}T + y = {y1, y2, y3}T => {x1+y1, x2+y2, x3+y3}
    def __add__(self, other:Vector) -> Vector:
        # check whether the dimensions are same for operation
        self._check_same_dimension(other)
        return Vector(a+b for a,b in zip(self._values, other._values))

    # subtracts two vectors x = {x1, x2, x3}T - y = {y1, y2, y3}T => {x1-y1, x2-y2, x3-y3}
    def __sub__(self, other:Vector) -> Vector:
        self._check_same_dimension(other)
        return Vector(a-b for a,b in zip(self._values, other._values))

    # multiply scalar by vector
    def __mul__(self, scalar:float) -> Vector:
        return Vector(value*scalar for value in self._values)

    # allow multiplication when scalar appears on left hand side, [2*vector]
    def __rmul__(self, scalar:float) -> Vector:
        return self*scalar

    # returns dot product,  x · y = Σᵢ xᵢyᵢ
    def dot(self, other:Vector) -> float:
        self._check_same_dimension(other)
        return sum(a*b for a,b in zip(self._values, other._values))

    # compute Euclidean (L2) norm of vector, ||x||₂ = sqrt(x · x) = sqrt(x₁² + x₂² + ... + xₙ²)
    def norm(self) -> float:
        return math.sqrt(self.dot(self))

    # compute Euclidean distance between two vectors: d(x, y) = ||x - y||₂
    def distance(self, other:Vector) -> float:
        return (self-other).norm()

    # measures angle between two vectors, tells us the similarity between two vectors.
    # cos(θ) = x · y / ||x|| ||y||
    def cosine_similarity(self, other:Vector) -> float:
        self._check_same_dimension(other)
        denominator = self.norm() * other.norm()
        if denominator == 0:
            raise ValueError("Cosine Similarity is undefined for zero vector")

        return self.dot(other) / denominator

    def _check_same_dimension(self, other:Vector) -> None:
        if(len(self) != len(other)):
            raise ValueError(f"Dimension mismatch: {len(self)} != {len(other)}")
        
    # helper to compare two column spaces
    def __eq__(self, other: object) -> bool:
        if not isinstance(other, Vector):
            return NotImplemented
        if len(self) != len(other):
            return False

        tolerance = 1e-12
        return all(abs(a-b) <= tolerance for a,b in zip(self._values, other._values))
        