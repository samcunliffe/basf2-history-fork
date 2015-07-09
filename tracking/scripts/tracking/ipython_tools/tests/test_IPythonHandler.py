from unittest import TestCase
from tracking.ipython_tools import handler
import basf2


class TestIPythonHandler(TestCase):

    def create_path(self):
        path = basf2.create_path()
        path.add_module("EventInfoSetter")
        path.add_module("Gearbox")
        path.add_module("Geometry")

        return path

    def test_empty_path(self):
        empty_path = None
        self.assertRaises(ValueError, handler.process, empty_path)

    def test_process_no_start(self):
        path = self.create_path()

        calculation = handler.process(path)
        # status should be set correctly
        self.assertFalse(calculation.is_running())
        self.assertRaises(AssertionError, calculation.has_failed)
        self.assertFalse(calculation.is_finished())
        self.assertRaises(AssertionError, calculation.wait_for_end)
        self.assertRaises(AssertionError, calculation.stop)
        self.assertEqual(calculation.get_status(), "not started")

        # All the output should lead to nothing
        self.assertRaises(AssertionError, calculation.get_statistics)
        self.assertRaises(AssertionError, calculation.get_log)

        self.assertEqual(len(calculation.get_keys()), 0)

        # The modules
        modules = calculation.get_modules()
        self.assertEqual(len(modules), 5)

    def test_process_with_start(self):
        path = self.create_path()

        calculation = handler.process(path)
        calculation.start()

        self.assertTrue(calculation.is_running())
        self.assertEqual(calculation.get_status(), "running")

        calculation.wait_for_end(display_bar=False)

        self.assertFalse(calculation.is_running())
        self.assertEqual(calculation.get_status(), "finished")

        self.assertFalse(calculation.has_failed())
        self.assertTrue(calculation.is_finished())
        self.assertRaises(AssertionError, calculation.stop)

        # All the output should lead to something
        self.assertGreater(len(calculation.get_statistics()), 0)
        self.assertGreater(len(calculation.get_log()), 0)

        self.assertEqual(len(calculation.get_keys()), 2)

        self.assertEqual(calculation.get("basf2.statistics"), calculation.get_statistics())
        self.assertEqual(calculation.get("basf2.store_content"), [])

        # The modules
        modules = calculation.get_modules()
        self.assertEqual(len(modules), 5)
