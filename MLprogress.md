# Phase1

## First implementation: Vector


```yaml
                    PHASE 1
                       │
                       ▼
              foundations/linalg
                       │
             ┌─────────┴─────────┐
             ▼                   ▼
          Vector              Matrix
             │                   │
             └─────────┬─────────┘
                       ▼
              foundations/
              probability
                       │
                       ▼
                  statistics
                       │
                       ▼
                    MLE
                       │
                       ▼
                    MAP
                       │
                       ▼
             foundations/
             optimization
                       │
                       ▼
               GradientDescent
                       │
                       ▼
              LinearRegression
                       │
                       ▼
             LogisticRegression
                       │
                       ▼
               Regularization
                       │
                       ▼
                 Evaluation
                       │
                       ▼
                  AEGISML
```
first we implementation `vector` it should represent `x belongs R`


Before I give you code, let's establish the **contract**.

Our `Vector` should represent:

$$
x \in \mathbb{R}^n
$$

So:

```python
x = Vector([1.0, 2.0, 3.0])
```

means:

$$
x =
\begin{bmatrix}
1\\
2\\
3
\end{bmatrix}
$$

### Operations

#### Addition

```python
x + y
```

$$
(x+y)_i=x_i+y_i
$$

#### Subtraction

```python
x - y
```

#### Scalar multiplication

```python
x * 3
```

#### Dot product

```python
x.dot(y)
```

$$
x^Ty=\sum_i x_i y_i
$$

#### Norm

```python
x.norm()
```

$$
\|x\|_2=\sqrt{x^Tx}
$$

#### Distance

```python
x.distance(y)
```

$$
\|x-y\|_2
$$

#### Cosine similarity

```python
x.cosine_similarity(y)
```

$$
\frac{x^Ty}{\|x\|_2\|y\|_2}
$$

---

Let's hit the **important Matrix milestone** now: multiplication. 🔥

# Milestone 1B.2 — Matrix × Vector

This is where the stuff we've built starts looking like actual ML.

## 1. Start with the dimensions

Suppose:

$$
A =
\begin{bmatrix}
1&2&3\\
4&5&6
\end{bmatrix}
$$

and

$$
x =
\begin{bmatrix}
10\\
20\\
30
\end{bmatrix}
$$

Their shapes are:

$$
A:(2\times3)
$$

$$
x:(3\times1)
$$

The inner dimensions match:

$$
\boxed{(2\times3)(3\times1)}
$$

so multiplication is valid.

The result has the **outer dimensions**:

$$
\boxed{2\times1}
$$

---

## 2. Why is multiplication defined this way?

Take the first row of \(A\):

$$
[1,2,3]
$$

and the vector:

$$
x=
\begin{bmatrix}
10\\20\\30
\end{bmatrix}
$$

Their dot product is:

$$
1(10)+2(20)+3(30)=140
$$

Now take the second row:

$$
[4,5,6]
$$

$$
4(10)+5(20)+6(30)=320
$$

Therefore:

$$
Ax=
\begin{bmatrix}
140\\
320
\end{bmatrix}
$$

So the fundamental definition is:

$$
\boxed{
(Ax)_i=\sum_{j=1}^{n}A_{ij}x_j
}
$$

Every output element is a **row of \(A\) dotted with \(x\)**.

That's why our `Vector.dot()` implementation wasn't some isolated exercise.

## The ML connection — remember this one

Suppose:

$$
X=
\begin{bmatrix}
x_{11}&x_{12}&x_{13}\\
x_{21}&x_{22}&x_{23}\\
x_{31}&x_{32}&x_{33}
\end{bmatrix}
$$

and:

$$
w=
\begin{bmatrix}
w_1\\w_2\\w_3
\end{bmatrix}
$$

Then:

$$
Xw=
\begin{bmatrix}
x_{11}w_1+x_{12}w_2+x_{13}w_3\\
x_{21}w_1+x_{22}w_2+x_{23}w_3\\
x_{31}w_1+x_{32}w_2+x_{33}w_3
\end{bmatrix}
$$

Each row represents **one sample**.

Each dot product combines that sample's features with the model's weights.

That's why later:

$$
\boxed{\hat y=Xw+b}
$$

isn't magic.

It's just **a bunch of dot products + a bias**.

Get these tests passing, and then we'll do **Matrix × Matrix**, where the exact same row-column idea generalizes one level further.



## Matrix × Matrix

We'll derive:

$$
\boxed{(AB)_{ij}=\sum_k A_{ik}B_{kj}}
$$

and see that it is really just **row × column dot products**.

Then we'll have:

$$
\boxed{(m\times n)(n\times p)=(m\times p)}
$$

After that we'll tackle **transpose**, which sets us up for:

$$
X^TX
$$

and eventually the normal equation:

$$
\boxed{w=(X^TX)^{-1}X^Ty}
$$

That's where our from-scratch linear algebra starts turning directly into the machinery of linear regression.

```yaml
                 LINEAR ALGEBRA
                       │
          ┌────────────┴────────────┐
          │                         │
     Vector Spaces             Matrices
          │                         │
    ┌─────┼─────┐           ┌──────┼──────┐
    │     │     │           │      │      │
  Span  Basis  Indep.     Rank  Null  Column
    │                       │      │    Space
    └──────────┬────────────┘      │
               │                   │
          Orthogonality ───────────┘
               │
          Projection
               │
        Least Squares
               │
         ┌─────┴─────┐
         │           │
    Eigenvalues   Quadratic
    Eigenvectors    Forms
         │           │
         └─────┬─────┘
               │
       Positive Definite
               │
        Covariance Matrix
               │
          Eigenvectors
               │
              PCA
```

Absolutely. Let's close the remaining **12 linear-algebra foundations in one pass**, but still do them properly: **intuition → mathematics → ML relevance → implementation target**.

The key is that these 12 aren't independent topics. They form a chain:

> **Independence → Span → Basis → Spaces → Orthogonality → Projection → Eigenstructure → Positive definiteness → Quadratic forms → Covariance**

---

# 1. Linear Independence

Suppose we have vectors

$$
v_1,v_2,\ldots,v_k
$$

They are **linearly independent** if the only solution to

$$
c_1v_1+c_2v_2+\cdots+c_kv_k=0
$$

is

$$
c_1=c_2=\cdots=c_k=0.
$$

### Intuition

No vector can be constructed from the others.

For example:

$$
v_1=
\begin{bmatrix}
1\\0
\end{bmatrix},
\qquad
v_2=
\begin{bmatrix}
0\\1
\end{bmatrix}
$$

are independent.

But

$$
v_1=
\begin{bmatrix}
1\\2
\end{bmatrix},
\qquad
v_2=
\begin{bmatrix}
2\\4
\end{bmatrix}
$$

are dependent because

$$
v_2=2v_1.
$$

### Matrix connection

Put vectors into columns:

$$
A=
\begin{bmatrix}
|&|\\
v_1&v_2\\
|&|
\end{bmatrix}
$$

Then:

$$
\boxed{\text{columns independent} \iff \operatorname{rank}(A)=\text{number of columns}}
$$

This is exactly why we implemented `rank()`.

### ML relevance

Feature redundancy.

If:

$$
x_3=2x_1+x_2
$$

then feature \(x_3\) doesn't provide independent information.

This becomes important for:

* multicollinearity
* regression
* PCA
* dimensionality reduction

---

# 2. Span

The **span** of vectors is every vector that can be produced using linear combinations of them.

For vectors \(v_1,\ldots,v_k\):

$$
\operatorname{span}(v_1,\ldots,v_k)
=
\left\{
\sum_{i=1}^{k}c_iv_i
\right\}.
$$

Example:

$$
v_1=
\begin{bmatrix}
1\\0
\end{bmatrix},
\quad
v_2=
\begin{bmatrix}
0\\1
\end{bmatrix}
$$

Then:

$$
c_1v_1+c_2v_2
=
\begin{bmatrix}
c_1\\c_2
\end{bmatrix}
$$

so they span all of:

$$
\mathbb R^2.
$$

But a single vector

$$
\begin{bmatrix}
1\\2
\end{bmatrix}
$$

only spans a line:

$$
\left\{
c
\begin{bmatrix}
1\\2
\end{bmatrix}
:c\in\mathbb R
\right\}.
$$

### ML connection

Your dataset lives in some feature space.

If your features only span a lower-dimensional subspace, then the apparent dimensionality is misleading.

---

# 3. Basis

A **basis** is a set of vectors that is simultaneously:

1. linearly independent
2. spanning the space

For \(\mathbb R^2\):

$$
e_1=
\begin{bmatrix}
1\\0
\end{bmatrix},
\qquad
e_2=
\begin{bmatrix}
0\\1
\end{bmatrix}
$$

form the standard basis.

Every vector can uniquely be written as:

$$
x=x_1e_1+x_2e_2.
$$

### Important theorem

Every vector space has infinitely many possible bases.

For example:

$$
\begin{bmatrix}
1\\0
\end{bmatrix},
\begin{bmatrix}
0\\1
\end{bmatrix}
$$

and

$$
\begin{bmatrix}
1\\1
\end{bmatrix},
\begin{bmatrix}
1\\-1
\end{bmatrix}
$$

are both bases of \(\mathbb R^2\).

### Dimension

The number of vectors in a basis is the **dimension**.

$$
\boxed{\dim(\mathbb R^n)=n}
$$

More generally:

$$
\boxed{\dim(V)=\text{number of vectors in any basis of }V}
$$

---

# 4. Column Space

Given

$$
A=
\begin{bmatrix}
|&|&|\\
a_1&a_2&a_3\\
|&|&|
\end{bmatrix}
$$

the **column space** is:

$$
\operatorname{Col}(A)
=
\operatorname{span}(a_1,a_2,a_3).
$$

In other words:

> Every vector that can be produced by \(Ax\).

Because:

$$
Ax=
x_1a_1+x_2a_2+\cdots+x_na_n
$$

therefore:

$$
\boxed{\operatorname{Col}(A)=\{Ax:x\in\mathbb R^n\}}
$$

### Why this matters

Consider:

$$
A=
\begin{bmatrix}
1&2\\
2&4
\end{bmatrix}
$$

The second column is twice the first.

Therefore:

$$
\operatorname{rank}(A)=1.
$$

The column space is only a line.

### ML connection

Linear regression:

$$
\hat y=Xw
$$

means:

$$
\hat y\in\operatorname{Col}(X).
$$

This is **huge**.

Linear regression isn't capable of producing an arbitrary \(y\).

It can only produce predictions lying inside the column space of \(X\).

That leads directly to **projection**.

---

# 5. Null Space

The null space of \(A\) is:

$$
\boxed{
\operatorname{Null}(A)
=
\{x:Ax=0\}
}
$$

Example:

$$
A=
\begin{bmatrix}
1&2\\
2&4
\end{bmatrix}
$$

Solve:

$$
Ax=0.
$$

Therefore:

$$
x_1+2x_2=0
$$

so:

$$
x_1=-2x_2.
$$

Let \(x_2=t\):

$$
x=
t
\begin{bmatrix}
-2\\1
\end{bmatrix}.
$$

Thus:

$$
\operatorname{Null}(A)
=
\operatorname{span}
\left(
\begin{bmatrix}
-2\\1
\end{bmatrix}
\right).
$$

### Fundamental relationship

For an \(m\times n\) matrix:

$$
\boxed{
\operatorname{rank}(A)+\operatorname{nullity}(A)=n
}
$$

This is the **Rank-Nullity Theorem**.

If \(A\) has 5 columns and rank 3:

$$
\text{nullity}=5-3=2.
$$

---

# 6. Orthogonality

Two vectors are orthogonal when their dot product is zero:

$$
\boxed{u^Tv=0}
$$

Example:

$$
u=
\begin{bmatrix}
1\\2
\end{bmatrix},
\qquad
v=
\begin{bmatrix}
2\\-1
\end{bmatrix}
$$

Then:

$$
u^Tv
=
1(2)+2(-1)
=0.
$$

Therefore:

$$
u\perp v.
$$

Geometrically, they're at \(90^\circ\).

### Orthogonal set

A set of non-zero vectors is orthogonal if every pair satisfies:

$$
v_i^Tv_j=0
\qquad i\neq j.
$$

If additionally:

$$
\|v_i\|=1
$$

they're **orthonormal**.

---

# 7. Projection

This one is extremely important for ML.

Suppose we want to project vector \(y\) onto vector \(x\).

The projection is:

$$
\boxed{
\operatorname{proj}_x(y)
=
\frac{x^Ty}{x^Tx}x
}
$$

Why?

We want:

$$
y_{\text{projection}}=cx.
$$

The residual:

$$
r=y-cx
$$

should be perpendicular to \(x\):

$$
x^Tr=0.
$$

Therefore:

$$
x^T(y-cx)=0
$$

$$
x^Ty-cx^Tx=0
$$

and:

$$
c=\frac{x^Ty}{x^Tx}.
$$

Hence:

$$
\boxed{
\operatorname{proj}_x(y)
=
\frac{x^Ty}{x^Tx}x
}
$$

### Projection onto a subspace

If the columns of \(A\) form a full-rank basis for a subspace:

$$
\boxed{
P=A(A^TA)^{-1}A^T
}
$$

and:

$$
\boxed{
\hat y=Py
}
$$

This is the mathematical heart of ordinary least squares.

---

# 8. Eigenvalues and Eigenvectors

Now we're entering the really interesting stuff.

For a matrix \(A\), an eigenvector \(v\) satisfies:

$$
\boxed{
Av=\lambda v
}
$$

where:

* \(v\) = eigenvector
* \(\lambda\) = eigenvalue

The matrix transforms \(v\), but **doesn't change its direction**.

It only scales it.

### Example

$$
A=
\begin{bmatrix}
2&0\\
0&3
\end{bmatrix}
$$

Take:

$$
v=
\begin{bmatrix}
1\\0
\end{bmatrix}.
$$

Then:

$$
Av=
\begin{bmatrix}
2\\0
\end{bmatrix}
=
2v.
$$

Therefore:

$$
\lambda=2.
$$

Similarly:

$$
\begin{bmatrix}
0\\1
\end{bmatrix}
$$

has eigenvalue \(3\).

---

## Finding eigenvalues

Starting with:

$$
Av=\lambda v
$$

we get:

$$
Av-\lambda v=0
$$

$$
(A-\lambda I)v=0.
$$

For a non-zero \(v\), \(A-\lambda I\) must be singular:

$$
\boxed{
\det(A-\lambda I)=0
}
$$

This is the **characteristic equation**.

---

# 9. Diagonalization

If a matrix has enough independent eigenvectors, we can write:

$$
\boxed{
A=PDP^{-1}
}
$$

where:

* \(P\) contains eigenvectors
* \(D\) contains eigenvalues on the diagonal

For example:

$$
D=
\begin{bmatrix}
\lambda_1&0\\
0&\lambda_2
\end{bmatrix}.
$$

Then:

$$
A=PDP^{-1}.
$$

### Why is this useful?

Because powers become easy:

$$
A^k=PD^kP^{-1}.
$$

And:

$$
D^k=
\begin{bmatrix}
\lambda_1^k&0\\
0&\lambda_2^k
\end{bmatrix}.
$$

### ML connection

Eigenvectors/eigenvalues appear everywhere:

* PCA
* covariance analysis
* spectral clustering
* dimensionality reduction
* stability analysis
* optimization

PCA in particular is basically:

> Find the directions of maximum variance.

Those directions are eigenvectors of the covariance matrix.

---

# 10. Positive-Definite Matrices

A symmetric matrix \(A\) is **positive definite** if:

$$
\boxed{
x^TAx>0
\qquad\forall x\neq0
}
$$

Example:

$$
A=
\begin{bmatrix}
2&0\\
0&3
\end{bmatrix}.
$$

For:

$$
x=
\begin{bmatrix}
x_1\\x_2
\end{bmatrix}
$$

we get:

$$
x^TAx
=
2x_1^2+3x_2^2.
$$

For every non-zero \(x\):

$$
2x_1^2+3x_2^2>0.
$$

Therefore \(A\) is positive definite.

### Why do we care?

Positive-definite matrices behave nicely in optimization.

For a quadratic function:

$$
f(x)=\frac12x^TAx-b^Tx
$$

if \(A\) is positive definite, the function is **strictly convex**.

Therefore it has a unique global minimum.

That's directly relevant to regression and optimization.

---

# 11. Quadratic Forms

A quadratic form is:

$$
\boxed{
x^TAx
}
$$

where \(A\) is typically symmetric.

Example:

$$
A=
\begin{bmatrix}
2&1\\
1&3
\end{bmatrix}
$$

and:

$$
x=
\begin{bmatrix}
x\\y
\end{bmatrix}.
$$

Then:

$$
x^TAx
=
2x^2+2xy+3y^2.
$$

Notice that the matrix compactly represents the quadratic expression.

### Why ML cares

Quadratic forms appear in:

* optimization
* covariance
* Gaussian distributions
* Mahalanobis distance
* least squares
* second-order optimization
* Hessians

For example, Mahalanobis distance:

$$
\boxed{
d_M(x,\mu)
=
\sqrt{
(x-\mu)^T
\Sigma^{-1}
(x-\mu)
}
}
$$

That's literally a quadratic form.

---

# 12. Covariance Matrices

Now we bring everything together.

Suppose we have \(n\) observations and \(d\) features.

Represent centered data as:

$$
X\in\mathbb R^{n\times d}.
$$

The covariance matrix is:

$$
\boxed{
\Sigma=
\frac{1}{n-1}X^TX
}
$$

assuming columns have been centered.

For two features \(X_1,X_2\):

$$
\Sigma=
\begin{bmatrix}
\operatorname{Var}(X_1)&\operatorname{Cov}(X_1,X_2)\\
\operatorname{Cov}(X_2,X_1)&\operatorname{Var}(X_2)
\end{bmatrix}.
$$

So:

### Diagonal

$$
\Sigma_{ii}=\operatorname{Var}(X_i)
$$

### Off-diagonal

$$
\Sigma_{ij}
=
\operatorname{Cov}(X_i,X_j)
$$

---

## Why covariance matrices are special

They are symmetric:

$$
\Sigma^T=\Sigma.
$$

They are positive semidefinite:

$$
\boxed{
x^T\Sigma x\ge0
}
$$

because:

$$
x^T\Sigma x
=
\frac{1}{n-1}x^TX^TXx
$$

$$
=
\frac{1}{n-1}(Xx)^T(Xx)
$$

$$
=
\frac{1}{n-1}\|Xx\|^2
\ge0.
$$

🔥 **This is a beautiful connection.**

Covariance → quadratic form → positive semidefinite matrix.

And because covariance matrices are symmetric, their eigenvectors have very useful properties.


## Null Space Basis

This is the next proper brick.

We want all solutions to:

$$
Ax=0.
$$

For example:

$$
A=
\begin{bmatrix}
1&2&3\\
2&4&6
\end{bmatrix}
$$

RREF:

$$
\begin{bmatrix}
1&2&3\\
0&0&0
\end{bmatrix}
$$

Pivot:

$$
x_1
$$

Free variables:

$$
x_2,x_3
$$

Set:

$$
x_2=s,\qquad x_3=t
$$

Then:

$$
x_1+2s+3t=0
$$

so:

$$
x_1=-2s-3t.
$$

Therefore:

$$
x=
s
\begin{bmatrix}
-2\\1\\0
\end{bmatrix}
+
t
\begin{bmatrix}
-3\\0\\1
\end{bmatrix}.
$$

Hence:

$$
\boxed{
\operatorname{Null}(A)
=
\operatorname{span}
\left\{
\begin{bmatrix}-2\\1\\0\end{bmatrix},
\begin{bmatrix}-3\\0\\1\end{bmatrix}
\right\}
}
$$

And notice the beautiful check:

$$
\operatorname{rank}(A)=1
$$

$$
\operatorname{nullity}(A)=2
$$

$$
1+2=3
$$

which is exactly **rank-nullity**.

So this next implementation will teach us something important rather than just adding another method.

**Next brick: `null_space_basis()` + tests.** 🔨
