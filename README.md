Stator is a header-only compile-time and runtime C++17 Computer
Algebra System. Its designed to be fast, and uses metaprogramming to
achieve much of this speed. 

Aside from the math, it has some tools for application in event-driven
dynamics/ray-tracing/collision-detection and is being extended towards
thermodynamics applications.

For full details, see the Documentation:
https://toastedcrumpets.github.io/stator

# C++ interface

Example to come!

# Python bindings

Stator has python bindings to allow "fast" math in python. Its built
around the parser

```python
import stator
# Use the built-in expression parser to make objects like variables
x = Expr("x")
# You can then create more complex expressions
f = x*x+2
# And print them
print(f) # x*x+2
# They're still Expr objects
print(repr(f)) # Expr('x*x+2')
# You can evaluate them by substitution
result = sub(f, {x:2})
# And the result becomes a python float
print(result) # 6.0

# We can parse more complex expressions, like lists
funcs = Expr('[1, x, x^2/2, x^3/6, sin(x), sin(y)]')
# And take derivatives
print(derivative(funcs, x)) # [0, 1, 4*x/4, 18*x^2/36, cos x, 0]
# You'll note simplification is not very advanced (yet)

# Substitution can use dictionaries to provide computation 'contexts'
print(sub(funcs, {x:1, Expr('y'): 2})) # [1.0, 1.0, 0.5, 0.16666666666666666, 0.8414709848078965, 0.9092974268256817]
```

# Requirements

You need the Google test library to build the unit tests. The Boost
interval library is needed for interval arithmetic support.

# Alternatives/Similar software 

For the compile-time C++ library:
- [ViennaMath](http://viennamath.sourceforge.net/): A nice
  compile-time math library, stator has most of this functionality
  except the extensions to FEM.
- [SymbolicC++](https://issc.uj.ac.za/symbolic/symbolic.html): A nice
  library, but behind in adopting modern compiler extensions (last
  released in 2010). Also you seem to need to buy the book to get the
  docs, but at least it has them!

For the python bindings:
- [sympy](https://www.sympy.org/): The full-featured CAS for python, but also quite slow.
- [symengine](https://symengine.org/index.html): Just like
  stator. It's actually a C++ CAS engine with python wrappers.

For the interval arithmetic
- [Kodiak](https://github.com/nasa/Kodiak) This is a full library for interval arithmatic solving.
- [Julia](https://github.com/JuliaIntervals/IntervalArithmetic.jl) A brilliant implementation of interval solving.
