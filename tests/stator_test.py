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
        #self.assertEqual(repr(Expr("x[1]")),"Expr('x[1]')")

    def test_list(self):
        self.assertEqual(repr(Expr("[]")),"Expr('[]')")
        self.assertEqual(repr(Expr("[1]")),"Expr('[1]')")
        self.assertEqual(repr(Expr("[1, 2]")),"Expr('[1, 2]')")
        f = Expr("[1, x, 2*x^2]")
        x = Expr("x")
        df = derivative(f, x)
        self.assertEqual(repr(simplify(df)), "Expr('[0, 1, 4*x]')")
        self.assertEqual(sub(df, Expr('x=2')), [0, 1, 8])
        self.assertEqual(Expr("[1,2,3,4]") + Expr("[0,1,2,3]"), Expr('[1, 3, 5, 7]').to_python())
        self.assertEqual(Expr("[1,2,3,4]") - Expr("[0,1,2,3]"), Expr('[1, 1, 1, 1]').to_python())
        self.assertEqual(Expr("[1,2,3,4]") * Expr("[0,1,2,3]"), Expr('[0, 2, 6, 12]').to_python())
        self.assertEqual(Expr("[1,2,3,4]") / Expr("[1,2,3,2]"), Expr('[1, 1, 1, 2]').to_python())
        
    def test_dict(self):
        pass
        #self.assertEqual(repr(Expr("{}")),"Expr('{}')")
        #self.assertEqual(repr(Expr("{x:1}")),"Expr('{x:1}')")
        #self.assertEqual(Expr("{x:1}") + Expr("{x:1, y:2}"), Expr('{x:2.0, y:2.0}').to_python())
        #self.assertEqual(Expr("{x:1}") - Expr("{x:1, y:2}"), Expr('{x:0, y:-2.0}').to_python())
        #self.assertEqual(Expr("{x:3}") * Expr("{x:2, y:2}"), Expr('{x:6}').to_python())

    def test_sub_generic(self):
        self.assertEqual(sub(Expr("x"), Expr('{x:2}')), 2)

    def test_to_python_conversions(self):
        self.assertEqual(Expr("2+2"), 4)
        self.assertEqual(Expr("[1,2,3]").to_python(), [1,2,3])
        self.assertEqual(Expr("{x:1+2, y:(1+x)}").to_python(), {Expr('x'):3, Expr('y'):Expr('1+x')})

    def test_from_python_conversions(self):
        self.assertEqual(Expr({Expr('x'): 1, Expr('y'):Expr('1-x')}), Expr('{y:1-x, x:1}'))
        self.assertEqual(Expr([1, Expr('1-x')]), Expr('[1, 1-x]'))

    def test_units(self):
        print(simplify(Expr('2.2{m}')+Expr('2.2{m}')))
        #self.assertTrue(False
        
if __name__ == "__main__":
    unittest.main()
