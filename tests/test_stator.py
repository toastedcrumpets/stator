import unittest
import stator
import stator.core

class Testfit(unittest.TestCase):
    def test_interface(self):
        e = stator.core.Expr("1+1")

if __name__ == "__main__":
    unittest.main()
