import unittest
import stator

class Testfit(unittest.TestCase):
    def test_interface(self):
        self.assertEqual(repr(stator.Expr("1+1")),"Expr('2')")
        self.assertEqual(repr(stator.Expr("1=1")),"Expr('1=1')")

if __name__ == "__main__":
    unittest.main()
