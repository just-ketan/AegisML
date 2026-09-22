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
