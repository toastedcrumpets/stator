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
        self.assertEqual(repr(simplify(sub(df, Expr('x=2')))), "Expr('[0, 1, 8]')")
        

    def test_dict(self):
        self.assertEqual(repr(Expr("{}")),"Expr('{}')")
        self.assertEqual(repr(Expr("{x:1}")),"Expr('{x:1}')")

    def test_sub_generic(self):
        self.assertEqual(repr(sub(Expr("x"), Expr('{x:2}'))),"Expr('2')")

    def test_type_conversions(self):
        self.assertEqual(Expr("2+2").to_python(), 4)
        self.assertEqual(Expr("[1,2,3]").to_python(), [1,2,3])
        #self.assertEqual(Expr("{x:1, y:(z-2)}").to_python(), {Expr('x'):1, Expr('y'):Expr('z-2'),})
        
        
if __name__ == "__main__":
    unittest.main()
