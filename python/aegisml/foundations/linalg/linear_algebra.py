# import Data Structures
from .vector import Vector
from .matrix import Matrix
import cmath

def vectors_to_matrix(vectors: list[Vector]) -> Matrix:
    # from given list of vectors construct a matrix
    if not vectors:
        raise ValueError("No vectors to convert to matrix")

    dim = len(vectors[0])       # dimension
    for vec in vectors:
        if len(vec) != dim:
            raise ValueError("all vectors must be of same dimension")

    rows = [[ vectors[col][row] for col in range(len(vectors))] for row in range(dim)]
    return Matrix(rows)

# 1. linear indepedence: c1v1 + c2v2 + ... cNxN = 0 iff c1 = c2 = ... = cN = 0
# number of linearly independent columns = rank(matrix)
def are_linearly_independent(vectors: list[Vector]) -> bool:
    # check if collection of vectors are LI
    matrix = vectors_to_matrix(vectors)
    return matrix.rank() == len(vectors)    # all cols unqiue then rank will be same as len else LD

#2. Span: can a target vector be constructed from these vectors
def vector_is_in_span(vector: Vector, spanning_vectors: list[Vector]) -> bool:
    if not spanning_vectors:
        raise ValueError("No spanning vectors to check against")

    matrix = vectors_to_matrix(spanning_vectors)
    if len(vector) != matrix.shape[0]:
        raise ValueError("vector dimensions are not same as span")

    augmented_cols = spanning_vectors + [vector]
    augmented = vectors_to_matrix(augmented_cols)

    return augmented.rank() == matrix.rank()

#3. Basis: both independent and spanning
def is_basis(vectors: list[Vector], dim: int) -> bool:
    if len(vectors) != dim:
        return False
    if not vectors:
        return False

    if any(len(vector) != dim for vector in vectors):
        return False

    return are_linearly_independent(vectors)    # this captures the standard Rn case

# gaussian elimination helper, will help answer Ax=b
def _rref(matrix: Matrix) -> tuple[Matrix, list[int]]:
    # compute Reduced Row Echelon Form (RREF) 
    values = [row.copy() for row in matrix._values]

    rows, cols = matrix.shape

    pivot_row=0
    pivot_cols=[]

    for pivot_col in range(cols):
        # we have exhausted all available rows
        if pivot_row >= rows:
            break

        # otw, find a row with non-zero value in this column
        candidate_row = None
        for row in range(pivot_row, rows):
            if abs(values[row][pivot_col]) > 1e-12:
                candidate_row = row
                break

        # no pivot in this col, move to next column
        if candidate_row is None:
            continue

        # tw, move pivot row into position
        values[pivot_row], values[candidate_row] = (values[candidate_row], values[pivot_row],)

        # normalize pivot to make it 1
        pivot = values[pivot_row][pivot_col]
        for col in range(cols):
            values[pivot_row][col] /= pivot

        # eliminate this column from every other row
        for row in range(rows):
            if row == pivot_row:
                continue
            factor = values[row][pivot_col]
            if abs(factor) <= 1e-12:
                continue
            for col in range(cols):
                values[row][col] -= (factor*values[pivot_row][col])

        pivot_cols.append(pivot_col)
        pivot_row += 1

    return Matrix(values), pivot_cols

#4. Column Space Basis: actual independent cols
def column_space_basis(matrix:Matrix) -> list[Vector]:
    if matrix.shape[1] == 0:
        return []   # no column entries

    _, pivot_cols = _rref(matrix)
    basis = []
    for col in pivot_cols:
        values = [matrix[row][col] for row in range(matrix.shape[0])]
        basis.append(Vector(values))
    return basis

#5. Null Space Basis: want solution to Ax=0 for matrix A and variables vector x
def null_space_basis(matrix:Matrix) -> list[Vector]:
    # returns a basis for null space of matrix
    
    rref, pivot_cols = _rref(matrix)
    rows, cols = matrix.shape

    pivot_set = set(pivot_cols)
    free_cols = [col for col in range(cols) if col not in pivot_set]
    basis = []

    for free_col in free_cols:
        # start with every variable equal to zero
        solution = [0.0]*cols
        solution[free_col] = 1.0    #mark curr as 1

        # each pivot gives us an equation for corresponding pivot variable
        for pivot_row, pivot_col in enumerate(pivot_cols):
            coeff = rref[pivot_row][free_col]
            solution[pivot_col] = -coeff
        basis.append(Vector(solution))
    return basis

#6. orthogonality: are the two vectors perpendicular to each other
def are_orthogonal(u: Vector,v: Vector,tolerance: float = 1e-12,) -> bool:
    if len(u) != len(v):
        raise ValueError("Vectors must have the same dimension.")
    return abs(u.dot(v)) <= tolerance   # if vectors are orthogonal their '.' product is 0

#7. Projection: component of V that lies along the direction of U
def project_onto(vector: Vector,direction: Vector,) -> Vector:
    '''
    proj_u(v) = (u^T v / u^T u) u
    v = projection + orthogonal_residual
    residual is perpendicular to u
    '''
    if len(vector) != len(direction):
        raise ValueError("Vectors must have the same dimension.")

    denominator = direction.dot(direction)

    if abs(denominator) <= 1e-12:
        raise ValueError("Cannot project onto the zero vector.")

    coefficient = direction.dot(vector) / denominator
    return direction * coefficient

#8. gram-schmidt: given independent vectors v1, v2....vk. we construct an orthogonal basis spanning same subspace
def gram_schmidt(vectors:list[Vector], tolerance: float = 1e-12,) -> list[Vector]:
    '''
    for each vk, we remove its component in all prev constructed orthogonal directions
    uk = vk - proj_u1(vk) - proj_u2(vk) ....
    ans then normalize: qk = uk / ||uk||
    resulting vectors qualify qi.qj=0 for i!=j and ||qi||=1
    '''
    if not vectors:
        return []

    dim = len(vectors[0])
    for vector in vectors:
        if len(vector) != dim:
            raise ValueError("all vectors must have same dimension")

    orthogonal = []
    for vector in vectors:
        # start with original vector
        u = vector

        # remove component along every previouisly constructed orthogonal direction
        for prev in orthogonal:
            projection = project_onto(u, prev)
            u = u - projection

        # if resulting vector has zero mag then input vectors are LD
        if u.norm() <= tolerance:
            raise ValueError("Vectors are Linearly Dependent")

        orthogonal.append(u)

    #normalize
    return [ vector * (1.0/vector.norm()) for vector in orthogonal ]

#9. Eigenvalues & Eigenvectors
def characteristic_polynomial(matrix:Matrix) -> tuple[float,...]:
    '''
    return coeff of charachteristic poly of 2x2 matrix
    for A = [[a,b],[c,d]]
    charchteristic poly is: det(A - λI) = λ² - (a + d)λ + (ad - bc)
    thus the returned tuple is (1, -(a + d), det(A))

    Example:    A = [2 1][0 3]
    gives:  λ² - 5λ + 6
    and therefore:  (1, -5, 6)
    '''

    rows, cols = matrix.shape
    if rows != 2 or cols != 2:
        raise ValueError("only supporting 2x2 as of now")

    a = matrix[0][0]
    d = matrix[1][1]
    det = matrix.determinant()

    return (1.0, -(a+d), det,)

def eigenvalues(matrix:Matrix) -> tuple[complex, complex]:
    '''
    Eigenvalues satisfy: Av = λv
    which requires: det(A - λI) = 0.
    For a 2x2 matrix, CE -> λ² + bλ + c = 0
    and the quadratic formula gives: λ = (-b ± sqrt(b² - 4c)) / 2

    The discriminant determines the type of eigenvalues:
        Δ > 0  -> two distinct real eigenvalues
        Δ = 0  -> repeated real eigenvalue
        Δ < 0  -> complex-conjugate eigenvalues
    '''

    coeff = characteristic_polynomial(matrix)
    a,b,c = coeff

    discriminant = complex(b*b - 4*a*c)
    sqrt_disc = cmath.sqrt(discriminant)

    lambda_1 = (-b + sqrt_disc) / (2*a)
    lambda_2 = (-b - sqrt_disc) / (2*a)

    return lambda_1, lambda_2

def eigenvectors(matrix:Matrix, tolerance:float = 1e-10,) -> dict[complex, list[Vector]]:
    # get eigenvectors
    values = eigenvalues(matrix)
    res = {}

    for eigenvalue in values:
        # numerical eigenvalues that are theoritically reala, may have extremely small imaginary component
        if abs(eigenvalue.imag) > tolerance:
            raise ValueError("Complex eigenvectors are not currently supported")

        lam = eigenvalue.real
        rows, cols = matrix.shape

        shifted_values = [[matrix[row][col] - (lam if row == col else 0.0) for col in range(cols)] for row in range(rows)]
        shifted = Matrix(shifted_values)
        basis = null_space_basis(shifted)

        res[eigenvalue] = basis

    return res

#9. Diagonalization
def diagonalize(matrix: Matrix,tolerance: float = 1e-10,) -> tuple[Matrix, Matrix, Matrix]:
    """
    Diagonalize a real 2x2 matrix.
    A matrix is diagonalizable when it has enough linearly independent eigenvectors to form a basis.

    For a 2x2 matrix: A = P D P^-1
    where: P = [v1 v2]
    contains two linearly independent eigenvectors and:D = [λ1  0][ 0 λ2]
    Returns: P, D, P_inverse
    such that: A = P @ D @ P_inverse
    """
    values = eigenvalues(matrix)
    eigenspaces = eigenvectors(matrix)
    independent_vectors = []
    corresponding_values = []

    for eigenvalue in values:
        basis = eigenspaces[eigenvalue]
        for vector in basis:
            independent_vectors.append(vector)
            corresponding_values.append(eigenvalue.real)

    if len(independent_vectors) != 2:
        raise ValueError("Matrix is not diagonalizable.")

    if not are_linearly_independent(independent_vectors):
        raise ValueError("Matrix does not have enough linearly independent eigenvectors.")

    # Eigenvectors become the columns of P.
    P = vectors_to_matrix(independent_vectors)
    # Eigenvalues become the diagonal of D.
    D = Matrix([[corresponding_values[0], 0.0],[0.0, corresponding_values[1]],])

    P_inverse = P.inverse()
    return P, D, P_inverse

def quadratic_form(matrix: Matrix,vector: Vector,) -> float:
    if matrix.shape[1] != len(vector):
        raise ValueError("Matrix columns must match vector dimension.")
    transformed = matrix @ vector
    return vector.dot(transformed)

def is_symmetric(matrix: Matrix,tolerance: float = 1e-12,) -> bool:
    rows, columns = matrix.shape
    if rows != columns:
        return False
    for i in range(rows):
        for j in range(columns):
            if abs(matrix[i][j] - matrix[j][i]) > tolerance:
                return False
    return True

def is_positive_definite(matrix: Matrix,tolerance: float = 1e-10,) -> bool:
    if not is_symmetric(matrix):
        return False
    values = eigenvalues(matrix)
    for value in values:
        if abs(value.imag) > tolerance:
            return False
        if value.real <= tolerance:
            return False
    return True

def covariance_matrix(data: Matrix,) -> Matrix:
    """
    Compute the sample covariance matrix.
        Rows represent observations.
        Columns represent features.

    Given centered data X:  Σ = (1 / (n - 1)) X^T X
    The function first centers every feature by subtractingits sample mean.

    The resulting covariance matrix has:
        diagonal:variances
        off-diagonal: covariances

    The covariance matrix is symmetric and positivesemidefinite.
    """
    observations, features = data.shape
    if observations < 2:
        raise ValueError("At least two observations are required.")

    # Compute the mean of each feature.
    means = []

    for column in range(features):
        total = sum( data[row][column] for row in range(observations))
        means.append(total / observations)

    # Center the data.
    centered = []

    for row in range(observations):
        centered_row = [ data[row][column] - means[column] for column in range(features) ]
        centered.append(centered_row)

    centered_matrix = Matrix(centered)

    # X^T X
    covariance = (centered_matrix.T @ centered_matrix)

    # Divide by n - 1.
    return (1.0 / (observations - 1)) * covariance