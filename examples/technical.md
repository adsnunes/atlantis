---
title: The Elegance of Proof
author: Notes on mathematical typography
date: 2025
---

# On Notation

Mathematics is a language written in symbols older than most nations. When Euler first wrote $e^{i\pi} + 1 = 0$, he compressed infinite complexity into five symbols. This is the art of notation: saying more with less.

---

## The Quadratic Formula

Every student learns it. Few appreciate it.

$$
x = \frac{-b \pm \sqrt{b^2 - 4ac}}{2a}
$$

This formula contains a story. The $\pm$ tells us there may be two answers, or one, or none that live in the real numbers. The discriminant $b^2 - 4ac$ is the oracle: positive means two roots, zero means one, negative means we must venture into the complex plane.

---

## Summation and Product

The sigma and pi notations are compression algorithms for the mind:

$$
\sum_{k=1}^{n} k = \frac{n(n+1)}{2}
$$

Gauss allegedly discovered this as a schoolboy, asked to sum the integers from 1 to 100. He paired them: $1 + 100$, $2 + 99$, $3 + 98$... Each pair sums to 101. Fifty pairs. The answer: 5050.

The product notation tells similar stories:

$$
n! = \prod_{k=1}^{n} k
$$

Factorials grow faster than intuition can follow. $20!$ exceeds the number of seconds since the Big Bang.

---

## Code as Proof

Programs are proofs made executable. Consider the Euclidean algorithm, unchanged since 300 BCE:

```python
def gcd(a: int, b: int) -> int:
    """Greatest common divisor via Euclid's algorithm."""
    while b:
        a, b = b, a % b
    return a
```

Or its recursive form, which mirrors the mathematical definition:

```haskell
gcd :: Int -> Int -> Int
gcd a 0 = a
gcd a b = gcd b (a `mod` b)
```

The Haskell version reads almost like the theorem itself. This is not coincidence. Curry-Howard correspondence tells us that types are propositions and programs are proofs.

---

## Limits and Infinity

Calculus begins with a question: what happens as we approach, but never reach?

$$
\lim_{x \to 0} \frac{\sin x}{x} = 1
$$

This limit is foundational. From it, we derive that $\frac{d}{dx}\sin x = \cos x$, and from that, all of trigonometric calculus unfolds.

The integral of the Gaussian:

$$
\int_{-\infty}^{\infty} e^{-x^2} dx = \sqrt{\pi}
$$

Here, $\pi$ appears unbidden. The circle constant emerges from the exponential function and the integers. Mathematics is full of such unexpected connections.

---

## Matrices

Linear algebra speaks in arrays:

$$
\begin{pmatrix} a & b \\ c & d \end{pmatrix} \begin{pmatrix} x \\ y \end{pmatrix} = \begin{pmatrix} ax + by \\ cx + dy \end{pmatrix}
$$

A matrix is a function. It takes vectors and returns vectors, stretching, rotating, reflecting. The eigenvalues tell us which directions survive unchanged.

---

## Implementation Notes

When rendering mathematics in a terminal, we face constraints:

```c
// Unicode provides mathematical symbols
const char *GREEK[] = {
    "α", "β", "γ", "δ", "ε",  // alpha through epsilon
    "ζ", "η", "θ", "ι", "κ",  // zeta through kappa
    "λ", "μ", "ν", "ξ", "ο",  // lambda through omicron
    "π", "ρ", "σ", "τ", "υ",  // pi through upsilon
    "φ", "χ", "ψ", "ω"        // phi through omega
};

// Superscripts for exponents
const char *SUPER = "⁰¹²³⁴⁵⁶⁷⁸⁹";

// Box-drawing for fractions
void render_fraction(const char *num, const char *den) {
    int w = max(strlen(num), strlen(den));
    printf("%*s\n", (w + (int)strlen(num)) / 2, num);
    for (int i = 0; i < w; i++) printf("─");
    printf("\n%*s\n", (w + (int)strlen(den)) / 2, den);
}
```

The result: $\frac{a}{b}$ becomes ASCII art that conveys the same meaning.

---

## On Precision

> *"A mathematician is a device for turning coffee into theorems."*
> -- Alfréd Rényi (often misattributed to Erdős)

The coffee-to-theorem pipeline requires precision. Consider floating-point:

```rust
fn nearly_equal(a: f64, b: f64) -> bool {
    let epsilon = 1e-10;
    (a - b).abs() < epsilon
}

// Why not just a == b?
// Because 0.1 + 0.2 != 0.3 in floating-point
```

The number $0.1$ cannot be represented exactly in binary floating-point. This is not a bug; it is a fundamental limitation of finite representations meeting infinite decimals.

---

## The Beauty of Recursion

Fibonacci, rendered in mathematics:

$$
F_n = F_{n-1} + F_{n-2}, \quad F_0 = 0, \quad F_1 = 1
$$

And in code:

```python
from functools import cache

@cache
def fib(n: int) -> int:
    if n < 2:
        return n
    return fib(n - 1) + fib(n - 2)
```

The `@cache` decorator transforms exponential time into linear. Memoization is mathematical induction made practical.

The closed form exists too:

$$
F_n = \frac{\phi^n - \psi^n}{\sqrt{5}}
$$

where $\phi = \frac{1 + \sqrt{5}}{2}$ (the golden ratio) and $\psi = \frac{1 - \sqrt{5}}{2}$.

---

## Closing Thoughts

- [ ] Read Knuth's *Concrete Mathematics*
- [ ] Implement a symbolic differentiator
- [ ] Prove something new
- [x] Appreciate that notation is technology

Mathematics and programming share a common ancestor: the desire to express complex ideas in precise, reproducible form. A proof is a program for convincing minds. A program is a proof that runs on silicon.

The symbols we use shape the thoughts we can think.[^1]

[^1]: This is the Sapir-Whorf hypothesis applied to formal languages. Whether it holds for natural language is debated; for mathematics, it is demonstrably true.
