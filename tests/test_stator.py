import unittest
import stator

class Testfit(unittest.TestCase):
    def test_interface(self):
        self.assertEqual(repr(stator.Expr("1+1")),"Expr('2')")
        self.assertEqual(repr(stator.Expr("1=1")),"Expr('1=1')")
        self.assertEqual(repr(stator.Expr("x[1]")),"Expr('x[1]')")

    def test_array(self):
        self.assertEqual(repr(stator.Expr("[]")),"Expr('[]')")
        self.assertEqual(repr(stator.Expr("[1]")),"Expr('[1]')")
        self.assertEqual(repr(stator.Expr("[1, 2]")),"Expr('[1, 2]')")


    def test_dict(self):
        self.assertEqual(repr(stator.Expr("{}")),"Expr('{}')")
        self.assertEqual(repr(stator.Expr("{x:1}")),"Expr('{x:1}')")
        
if __name__ == "__main__":
    unittest.main()
