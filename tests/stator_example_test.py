from stator import *

import math
import unittest
class TestExample(unittest.TestCase):
    def test_basic(self):
        #Example start
        
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

        #Example end

if __name__ == "__main__":
    unittest.main()
