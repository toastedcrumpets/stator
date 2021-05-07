import unittest
from stator import *



class Testfit(unittest.TestCase):
    def test_simplify(self):
        x = Expr("x")
        g = Expr("2*x^2")
        dg = derivative(g, x)
        self.assertEqual(repr(simplify(dg)), "Expr('4*x')")
        
    def test_interface(self):
        self.assertEqual(repr(Expr("1+1")),"Expr('2')")
        self.assertEqual(repr(Expr("1=1")),"Expr('1=1')")
        self.assertEqual(repr(Expr("x[1]")),"Expr('x[1]')")

    def test_array(self):
        self.assertEqual(repr(Expr("[]")),"Expr('[]')")
        self.assertEqual(repr(Expr("[1]")),"Expr('[1]')")
        self.assertEqual(repr(Expr("[1, 2]")),"Expr('[1, 2]')")
        f = Expr("[1, x, 2*x^2]")
        x = Expr("x")
        df = derivative(f, x)
        df = simplify(df)
        self.assertEqual(repr(df), "Expr('[0, 1, 4*x]')")
        #self.assertEqual(repr(subs(df, x = 2)), "Expr('[0, 1, 8]')")
        

    def test_dict(self):
        self.assertEqual(repr(Expr("{}")),"Expr('{}')")
        self.assertEqual(repr(Expr("{x:1}")),"Expr('{x:1}')")

        
        
if __name__ == "__main__":
    unittest.main()
