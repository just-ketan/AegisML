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

