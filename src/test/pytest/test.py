from rmtest import ModuleTestCase
import redis
import unittest

class MLTestCase(ModuleTestCase('../../redis-ml.so')):
    
    def testForest(self):
        with self.redis() as r:
            
            self.assertOk(r.execute_command('ml.forest.add','f_1', 0 ,'.', 'numeric', 1, 0.5, '.l', 'leaf', 1, '.r', 'leaf' ,0))
            self.assertOk(r.execute_command('ml.forest.add','f_1', 1 ,'.', 'numeric', 1, 0.5, '.l', 'leaf', 1, '.r', 'leaf' ,0))
            self.assertTrue(r.exists('f_1'))
            self.assertEqual(r.execute_command('ml.forest.run', 'f_1', '1:0'), '1')
    
            self.assertOk(r.execute_command('ml.forest.add','f_2', 0 ,'.', 'numeric', 1, 0.5, '.l', 'leaf', 1, 'stats', '5:0', '.r', 'leaf' ,0, 'stats', '0:1'))
            self.assertTrue(r.exists('f_2'))
            self.assertEqual(r.execute_command('ml.forest.run', 'f_2', '1:1'), '1')
            self.assertOk(r.execute_command('ml.forest.add','f_2', 1 ,'.', 'numeric', 1, 0.5, '.l', 'leaf', 1, 'stats', '0:8', '.r', 'leaf' ,0, 'stats', '5:1'))
            self.assertEqual(r.execute_command('ml.forest.run', 'f_2', '1:1'), '0')
            with self.assertResponseError():
                r.execute_command('ml.forest.add','tree-4', '0', '.', 'CATEGORIC', 'country', '1', '.l', 'CATEGORIC', 'us_state', '4', '.ll', 'LEAF', '0', '.lr', 'NUMERIC', 'height_m', '1.6')
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
