from rmtest import ModuleTestCase
import redis
import unittest

class MLTestCase(ModuleTestCase('../../redis-ml.so')):
    
    def testMatrix(self):
        with self.redis() as r:
            
            self.assertOk(r.execute_command('ml.matrix.set', 'a', 2, 3, 1, 2, 5, 3, 4, 6))
            self.assertTrue(r.exists('a'))
            res = r.execute_command('ml.matrix.get', 'a')
            
            self.assertTrue(len(res) == 8)
            self.assertEqual(res[0], 2L)
            self.assertEqual(res[1], 3L)
            self.assertEqual(res[2], '1')
            self.assertEqual(res[3], '2')
            self.assertEqual(res[4], '5')
            self.assertEqual(res[5], '3')
            self.assertEqual(res[6], '4')
            self.assertEqual(res[7], '6')

            self.assertOk(r.execute_command('ml.matrix.set', 'b', 3, 2, 1, 2, 3, 4, 7, 1))
            self.assertOk(r.execute_command('ml.matrix.multiply', 'a', 'b', 'c'))

            res = r.execute_command('ml.matrix.get', 'c')
            self.assertTrue(len(res) == 6)
            self.assertEqual(res[0], 2L)
            self.assertEqual(res[1], 2L)
            self.assertEqual(res[2], '42')
            self.assertEqual(res[3], '15')
            self.assertEqual(res[4], '57')
            self.assertEqual(res[5], '28')

if __name__ == '__main__':

    unittest.main()