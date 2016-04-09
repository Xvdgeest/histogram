#!/usr/bin/env python
import unittest
from math import pi
from histogram import histogram, regular_axis, polar_axis, \
                      variable_axis, category_axis, integer_axis
import cPickle
import StringIO
try: import numpy # optional, for tests that need numpy
except ImportError: pass

class test_regular_axis(unittest.TestCase):

    def test_init(self):
        regular_axis(1, 1.0, 2.0)
        regular_axis(1, 1.0, 2.0, label="ra")
        regular_axis(1, 1.0, 2.0, uoflow=False)
        regular_axis(1, 1.0, 2.0, label="ra", uoflow=False)
        with self.assertRaises(TypeError):
            regular_axis()
        with self.assertRaises(TypeError):
            regular_axis(1)
        with self.assertRaises(TypeError):
            regular_axis(1, 1.0)
        with self.assertRaises(RuntimeError):
            regular_axis(0, 1.0, 2.0)
        with self.assertRaises(TypeError):
            regular_axis("1", 1.0, 2.0)
        with self.assertRaises(StandardError):
            regular_axis(-1, 1.0, 2.0)
        with self.assertRaises(RuntimeError):
            regular_axis(1, 2.0, 1.0)
        with self.assertRaises(TypeError):
            regular_axis(1, 1.0, 2.0, label=0)
        with self.assertRaises(TypeError):
            regular_axis(1, 1.0, 2.0, label="ra", uoflow="True")
        with self.assertRaises(TypeError):
            regular_axis(1, 1.0, 2.0, bad_keyword="ra")
        a = regular_axis(4, 1.0, 2.0)
        self.assertEqual(a, regular_axis(4, 1.0, 2.0))
        self.assertNotEqual(a, regular_axis(3, 1.0, 2.0))
        self.assertNotEqual(a, regular_axis(4, 1.1, 2.0))
        self.assertNotEqual(a, regular_axis(4, 1.0, 2.1))

    def test_len(self):
        a = regular_axis(4, 1.0, 2.0)
        self.assertEqual(len(a), 5)

    def test_repr(self):
        for s in ("regular_axis(4, 1.1, 2.2)",
                  "regular_axis(4, 1.1, 2.2, label='ra')",
                  "regular_axis(4, 1.1, 2.2, uoflow=False)",
                  "regular_axis(4, 1.1, 2.2, label='ra', uoflow=False)"):
            self.assertEqual(str(eval(s)), s)

    def test_getitem(self):
        v = [1.0, 1.25, 1.5, 1.75, 2.0]
        a = regular_axis(4, 1.0, 2.0)
        for i in xrange(5):
            self.assertEqual(a[i], v[i])

    def test_iter(self):
        v = [1.0, 1.25, 1.5, 1.75, 2.0]
        a = regular_axis(4, 1.0, 2.0)
        self.assertEqual([x for x in a], v)

    def test_index(self):
        a = regular_axis(4, 1.0, 2.0)
        self.assertEqual(a.index(-1), -1)
        self.assertEqual(a.index(0.99), -1)
        self.assertEqual(a.index(1.0), 0)
        self.assertEqual(a.index(1.1), 0)
        self.assertEqual(a.index(1.249), 0)
        self.assertEqual(a.index(1.250), 1)
        self.assertEqual(a.index(1.499), 1)
        self.assertEqual(a.index(1.500), 2)
        self.assertEqual(a.index(1.749), 2)
        self.assertEqual(a.index(1.750), 3)
        self.assertEqual(a.index(1.999), 3)
        self.assertEqual(a.index(2.000), 4)
        self.assertEqual(a.index(2.1), 4)
        self.assertEqual(a.index(20), 4)

class test_polar_axis(unittest.TestCase):

    def test_init(self):
        polar_axis(1)
        polar_axis(4, 1.0)
        polar_axis(4, 1.0, label="pa")
        with self.assertRaises(TypeError):
            polar_axis()
        with self.assertRaises(StandardError):
            polar_axis(-1)
        with self.assertRaises(TypeError):
            polar_axis(4, 1.0, uoflow=True)
        with self.assertRaises(TypeError):
            polar_axis(1, 1.0, 2.0)
        with self.assertRaises(TypeError):
            polar_axis(1, 1.0, label=1)
        with self.assertRaises(TypeError):
            polar_axis("1")
        a = polar_axis(4, 1.0)
        self.assertEqual(a, polar_axis(4, 1.0))
        self.assertNotEqual(a, polar_axis(2, 1.0))
        self.assertNotEqual(a, polar_axis(4, 0.0))

    def test_len(self):
        self.assertEqual(len(polar_axis(4)), 5)
        self.assertEqual(len(polar_axis(4, 1.0)), 5)

    def test_repr(self):
        for s in ("polar_axis(4)",
                  "polar_axis(4, 1)",
                  "polar_axis(4, 1, label='x')",
                  "polar_axis(4, label='x')"):
            self.assertEqual(str(eval(s)), s)

    def test_getitem(self):
        v = [1.0, 1.0 + 0.5 * pi, 1.0 + pi, 1.0 + 1.5 *pi, 1.0 + 2.0 * pi]
        a = polar_axis(4, 1.0)
        for i in xrange(5):
            self.assertEqual(a[i], v[i])

    def test_iter(self):
        a = polar_axis(4, 1.0)
        v = [1.0, 1.0 + 0.5 * pi, 1.0 + pi, 1.0 + 1.5 *pi, 1.0 + 2.0 * pi]
        self.assertEqual([x for x in a], v)

    def test_index(self):
        a = polar_axis(4, 1.0)
        d = 0.5 * pi
        self.assertEqual(a.index(0.99 - 4*d), 3)
        self.assertEqual(a.index(0.99 - 3*d), 0)
        self.assertEqual(a.index(0.99 - 2*d), 1)
        self.assertEqual(a.index(0.99 - d), 2)
        self.assertEqual(a.index(0.99), 3)
        self.assertEqual(a.index(1.0), 0)
        self.assertEqual(a.index(1.01), 0)
        self.assertEqual(a.index(0.99 + d), 0)
        self.assertEqual(a.index(1.0 + d), 1)
        self.assertEqual(a.index(1.0 + 2*d), 2)
        self.assertEqual(a.index(1.0 + 3*d), 3)
        self.assertEqual(a.index(1.0 + 4*d), 0)
        self.assertEqual(a.index(1.0 + 5*d), 1)

class test_variable_axis(unittest.TestCase):

    def test_init(self):
        variable_axis(0, 1)
        variable_axis(1, -1)
        variable_axis(0, 1, 2, 3, 4)
        variable_axis(0, 1, label="va")
        variable_axis(0, 1, uoflow=True)
        variable_axis(0, 1, label="va", uoflow=True)
        with self.assertRaises(TypeError):
            variable_axis()
        with self.assertRaises(RuntimeError):
            variable_axis(1.0)
        with self.assertRaises(TypeError):
            variable_axis("1", 2)
        with self.assertRaises(TypeError):
            regular_axis(1, 1.0, 2.0, bad_keyword="ra")
        a = variable_axis(-0.1, 0.2, 0.3)
        self.assertEqual(a, variable_axis(-0.1, 0.2, 0.3))
        self.assertNotEqual(a, variable_axis(0, 0.2, 0.3))
        self.assertNotEqual(a, variable_axis(-0.1, 0.1, 0.3))
        self.assertNotEqual(a, variable_axis(-0.1, 0.1))

    def test_len(self):
        self.assertEqual(len(variable_axis(-0.1, 0.2, 0.3)), 3)

    def test_repr(self):
        for s in ("variable_axis(-0.1, 0.2)",
                  "variable_axis(-0.1, 0.2, 0.3)",
                  "variable_axis(-0.1, 0.2, 0.3, label='va')",
                  "variable_axis(-0.1, 0.2, 0.3, uoflow=False)",
                  "variable_axis(-0.1, 0.2, 0.3, label='va', uoflow=False)"):
            self.assertEqual(str(eval(s)), s)

    def test_getitem(self):
        v = [-0.1, 0.2, 0.3]
        a = variable_axis(*v)
        for i in xrange(3):
            self.assertEqual(a[i], v[i])

    def test_iter(self):
        v = [-0.1, 0.2, 0.3]
        a = variable_axis(*v)
        self.assertEqual([x for x in a], v)

    def test_index(self):
        a = variable_axis(-0.1, 0.2, 0.3)
        self.assertEqual(a.index(-10.0), -1)
        self.assertEqual(a.index(-0.11), -1)
        self.assertEqual(a.index(-0.1), 0)
        self.assertEqual(a.index(0.0), 0)
        self.assertEqual(a.index(0.19), 0)        
        self.assertEqual(a.index(0.2), 1)
        self.assertEqual(a.index(0.21), 1)
        self.assertEqual(a.index(0.29), 1)
        self.assertEqual(a.index(0.3), 2)
        self.assertEqual(a.index(0.31), 2)
        self.assertEqual(a.index(10), 2)

class test_category_axis(unittest.TestCase):

    def test_init(self):
        category_axis("A", "B", "C")
        category_axis("A;B;C")
        with self.assertRaises(TypeError):
            category_axis()
        with self.assertRaises(TypeError):
            category_axis(1)
        with self.assertRaises(TypeError):
            category_axis("1", 2)
        with self.assertRaises(TypeError):
            category_axis("A", "B", "C", label="ca")
        with self.assertRaises(TypeError):
            category_axis("A", "B", "C", uoflow=True)        
        self.assertEqual(category_axis("A", "B", "C"),
                         category_axis("A", "B", "C"))
        self.assertEqual(category_axis("A", "B", "C"),
                         category_axis("A;B;C"))
        self.assertEqual(category_axis(";"), category_axis("", ""))
        self.assertEqual(category_axis("abc;d"), category_axis("abc", "d"))
        self.assertEqual(category_axis("a;bcd"), category_axis("a", "bcd"))
        self.assertNotEqual(category_axis("A;B"), category_axis("A;C"))

    def test_len(self):
        a = category_axis("A", "B", "C")
        self.assertEqual(len(a), 3)
        a = category_axis("A;B;C")
        self.assertEqual(len(a), 3)

    def test_repr(self):
        for s in ("category_axis('A')",
                  "category_axis('A', 'B')",
                  "category_axis('A', 'B', 'C')"):
            self.assertEqual(str(eval(s)), s)

    def test_getitem(self):
        c = "A", "B", "C"
        a = category_axis(*c)
        for i in xrange(3):
            self.assertEqual(a[i], c[i])

    def test_iter(self):
        c = ["A", "B", "C"]
        self.assertEqual([x for x in category_axis(*c)], c)

class test_integer_axis(unittest.TestCase):

    def test_init(self):
        integer_axis(-1, 2)
        with self.assertRaises(TypeError):
            integer_axis()
        with self.assertRaises(TypeError):
            integer_axis(1)
        with self.assertRaises(TypeError):
            integer_axis("1", 2)
        with self.assertRaises(RuntimeError):
            integer_axis(2, -1)
        with self.assertRaises(TypeError):
            integer_axis(1, 2, 3)
        self.assertEqual(integer_axis(-1, 2), integer_axis(-1, 2))
        self.assertNotEqual(integer_axis(-1, 2), integer_axis(-1, 2, label="ia"))
        self.assertNotEqual(integer_axis(-1, 2), integer_axis(-1, 2, uoflow=True))

    def test_len(self):
        self.assertEqual(len(integer_axis(-1, 2)), 4)

    def test_repr(self):
        for s in ("integer_axis(-1, 1)",
                  "integer_axis(-1, 1, label='ia')",
                  "integer_axis(-1, 1, uoflow=False)",
                  "integer_axis(-1, 1, label='ia', uoflow=False)"):
            self.assertEqual(str(eval(s)), s)

    def test_label(self):
        self.assertEqual(integer_axis(-1, 2, label="ia").label, "ia")

    def test_getitem(self):
        v = [-1, 0, 1, 2]
        a = integer_axis(-1, 2)
        for i in xrange(4):
            self.assertEqual(a[i], v[i])

    def test_iter(self):
        v = [x for x in integer_axis(-1, 2)]
        self.assertEqual(v, [-1, 0, 1, 2])

    def test_index(self):
        a = integer_axis(-1, 2)
        self.assertEqual(a.index(-3), -1)
        self.assertEqual(a.index(-2), -1)
        self.assertEqual(a.index(-1), 0)
        self.assertEqual(a.index(0), 1)
        self.assertEqual(a.index(1), 2)
        self.assertEqual(a.index(2), 3)
        self.assertEqual(a.index(3), 4)
        self.assertEqual(a.index(4), 4)

class histogram_test(unittest.TestCase):

    def test_init(self):
        histogram(integer_axis(-1, 1))
        with self.assertRaises(TypeError):
            histogram(1)
        with self.assertRaises(TypeError):
            histogram("bla")
        with self.assertRaises(TypeError):
            histogram([])
        with self.assertRaises(TypeError):
            histogram(regular_axis)
        with self.assertRaises(TypeError):
            histogram(regular_axis())
        with self.assertRaises(TypeError):
            histogram([integer_axis(-1, 1)])
        with self.assertRaises(TypeError):
            histogram(integer_axis(-1, 1), unknown_keyword="nh")

        h = histogram(integer_axis(-1, 1))
        self.assertEqual(h.dim, 1)
        self.assertEqual(h.axis(0), integer_axis(-1, 1))
        self.assertEqual(h.shape(0), 5)
        self.assertEqual(h.depth, 1)
        self.assertEqual(histogram(integer_axis(-1, 1, uoflow=False)).shape(0), 3)
        self.assertNotEqual(h, histogram(regular_axis(1, -1, 1)))
        self.assertNotEqual(h, histogram(integer_axis(-1, 2)))
        self.assertNotEqual(h, histogram(integer_axis(-1, 1, label="ia")))
        self.assertNotEqual(h, histogram(integer_axis(-1, 1, uoflow=True)))

    def test_fill_1d(self):
        h0 = histogram(integer_axis(-1, 1, uoflow=False))
        h1 = histogram(integer_axis(-1, 1, uoflow=True))
        for h in (h0, h1):
            with self.assertRaises(StandardError):
                h.fill()
            with self.assertRaises(StandardError):
                h.fill(1, 2)
            h.fill(-10)
            h.fill(-1)
            h.fill(-1)
            h.fill(0)
            h.fill(1)
            h.fill(1)
            h.fill(1)
            h.fill(10)
        self.assertEqual(h0.sum, 6)
        self.assertEqual(h0.depth, 1)
        self.assertEqual(h0.shape(0), 3)
        self.assertEqual(h1.sum, 8)
        self.assertEqual(h1.depth, 1)
        self.assertEqual(h1.shape(0), 5)

        for h in (h0, h1):
            self.assertEqual(h[0], 2)
            self.assertEqual(h[1], 1)
            self.assertEqual(h[2], 3)
            with self.assertRaises(TypeError):
                h[0, 1]

        self.assertEqual(h1[-1], 1)
        self.assertEqual(h1[3], 1)

    def test_growth(self):
        h = histogram(integer_axis(-1, 1))
        h.fill(-1)
        h.fill(1)
        h.fill(1)
        for i in xrange(255):
            h.fill(0)
        self.assertEqual(h.depth, 1)
        h.fill(0)
        self.assertEqual(h.depth, 2)
        for i in xrange(1000-256):
            h.fill(0)
        self.assertEqual(h[-1], 0)
        self.assertEqual(h[0], 1)
        self.assertEqual(h[1], 1000)
        self.assertEqual(h[2], 2)
        self.assertEqual(h[3], 0)

    def test_fill_2d(self):
        for uoflow in (False, True):
            h = histogram(integer_axis(-1, 1, uoflow=uoflow),
                           regular_axis(4, -2, 2, uoflow=uoflow))
            h.fill(-1, -2)
            h.fill(-1, -1)
            h.fill(0, 0)
            h.fill(0, 1)
            h.fill(1, 0)
            h.fill(3, -1)
            h.fill(0, -3)
            with self.assertRaises(StandardError):
                h.fill(1)
            with self.assertRaises(StandardError):
                h.fill(1, 2, 3)

            m = [[1, 1, 0, 0, 0, 0],
                 [0, 0, 1, 1, 0, 1],
                 [0, 0, 1, 0, 0, 0],
                 [0, 1, 0, 0, 0, 0],
                 [0, 0, 0, 0, 0, 0]]
            for i in xrange(h.axis(0).bins + 2*uoflow):
                for j in xrange(h.axis(1).bins + 2*uoflow):
                    self.assertEqual(h[i, j], m[i][j])

    def test_add_2d(self):
        for uoflow in (False, True):
            h = histogram(integer_axis(-1, 1, uoflow=uoflow),
                           regular_axis(4, -2, 2, uoflow=uoflow))
            h.fill(-1, -2)
            h.fill(-1, -1)
            h.fill(0, 0)
            h.fill(0, 1)
            h.fill(1, 0)
            h.fill(3, -1)
            h.fill(0, -3)

            m = [[1, 1, 0, 0, 0, 0],
                 [0, 0, 1, 1, 0, 1],
                 [0, 0, 1, 0, 0, 0],
                 [0, 1, 0, 0, 0, 0],
                 [0, 0, 0, 0, 0, 0]]

            h2 = h + h
            h += h
            self.assertEqual(h, h2)

            for i in xrange(h.axis(0).bins + 2*uoflow):
                for j in xrange(h.axis(1).bins + 2*uoflow):
                    self.assertEqual(h[i, j], 2 * m[i][j])

    def test_pickle(self):
        a = histogram(category_axis('A', 'B', 'C'),
                       integer_axis(0, 3, label='ia'),
                       regular_axis(4, 0.0, 4.0, uoflow=False),
                       variable_axis(0.0, 1.0, 2.0))
        for i in xrange(a.axis(0).bins):
            a.fill(i, 0, 0, 0)
            for j in xrange(a.axis(1).bins):
                a.fill(i, j, 0, 0)
                for k in xrange(a.axis(2).bins):
                    a.fill(i, j, k, 0)
                    for l in xrange(a.axis(3).bins):
                        a.fill(i, j, k, l)

        io = StringIO.StringIO()
        cPickle.dump(a, io)
        io.seek(0)
        b = cPickle.load(io)
        self.assertNotEqual(id(a), id(b))
        self.assertEqual(a.dim, b.dim)
        self.assertEqual(a.axis(0), b.axis(0))
        self.assertEqual(a.axis(1), b.axis(1))
        self.assertEqual(a.axis(2), b.axis(2))
        self.assertEqual(a.axis(3), b.axis(3))
        self.assertEqual(a.sum, b.sum)
        self.assertEqual(a, b)

    @unittest.skipUnless("numpy" in globals(), "requires numpy")
    def test_numpy_conversion(self):
        a = histogram(integer_axis(0, 2, uoflow=False))
        for i in xrange(100):
            a.fill(1)
        b = numpy.array(a) # a copy
        v = numpy.asarray(a) # a view
        self.assertEqual(b.dtype, numpy.uint8)
        self.assertTrue(numpy.all(b == numpy.array((0, 100, 0))))
        for i in xrange(100):
            a.fill(1)
        self.assertTrue(numpy.all(b == numpy.array((0, 100, 0))))
        self.assertTrue(numpy.all(v == numpy.array((0, 200, 0))))
        for i in xrange(100):
            a.fill(1)
        b = numpy.array(a)
        self.assertEqual(b.dtype, numpy.uint16)
        self.assertTrue(numpy.all(b == numpy.array((0, 300, 0))))
        # view does not follow underlying switch in word size
        self.assertFalse(numpy.all(v == b))

    @unittest.skipUnless("numpy" in globals(), "requires numpy")
    def test_fill_with_numpy_array(self):
        a = histogram(integer_axis(0, 2, uoflow=False))
        a.fill(numpy.array([-1, 0, 1, 2, 1, 4]))
        self.assertEqual(a[0], 1)
        self.assertEqual(a[1], 2)
        self.assertEqual(a[2], 1)
        a = histogram(integer_axis(0, 1, uoflow=False),
                       regular_axis(2, 0, 2, uoflow=False))
        a.fill(numpy.array([[-1., -1.], [0., 1.], [1., 0.1]]))
        self.assertEqual(a[0, 0], 0)
        self.assertEqual(a[0, 1], 1)
        self.assertEqual(a[1, 0], 1)
        self.assertEqual(a[1, 1], 0)
        a = histogram(integer_axis(0, 2, uoflow=False))
        a.fill([0, 0, 1, 2])
        a.fill((1, 0, 2, 2))
        self.assertEqual(a[0], 3)
        self.assertEqual(a[1], 2)
        self.assertEqual(a[2], 3)

if __name__ == "__main__":
    unittest.main()
