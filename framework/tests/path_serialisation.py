import os
import shutil
import tempfile
from unittest import TestCase, main

import basf2


class PathSerialisationTestCase(TestCase):

    def setUp(self):
        # Create a temporary directory
        self.test_dir = tempfile.mkdtemp()
        self.pickle_file = os.path.join(self.test_dir, "output.pkl")

    def tearDown(self):
        # Remove the directory after the test
        shutil.rmtree(self.test_dir)

    def check_if_paths_equal(self, path1, path2):
        self.assertEqual(len(path1.modules()), len(path2.modules()))

        for module1, module2 in zip(path1.modules(), path2.modules()):
            self.assertEqual(module1.name(), module2.name())
            self.assertEqual(module1.type(), module2.type())
            self.assertEqual(module1.description(), module2.description())
            self.assertEqual(module1.package(), module2.package())

            self.assertEqual(len(module1.available_params()), len(module2.available_params()))
            for param1, param2 in zip(module1.available_params(), module2.available_params()):
                self.assertEqual(param1.name, param2.name)
                self.assertEqual(param1.description, param2.description)
                self.assertEqual(param1.values, param2.values)

            self.assertEqual(len(module1.get_all_conditions()), len(module2.get_all_conditions()))
            for cond1, cond2 in zip(module1.get_all_conditions(), module2.get_all_conditions()):
                self.assertEqual(cond1.get_value(), cond2.get_value())
                self.assertEqual(cond1.get_operator(), cond2.get_operator())
                self.assertEqual(cond1.get_after_path(), cond2.get_after_path())

                self.check_if_paths_equal(cond1.get_path(), cond2.get_path())

    def pickle_and_check(self, path):
        basf2.write_path_to_file(path, self.pickle_file)
        pickled_path = basf2.get_path_from_file(self.pickle_file)

        self.check_if_paths_equal(pickled_path, path)

    def test_simple_path(self):
        path = basf2.create_path()
        path.add_module("EventInfoSetter", evtNumList=[42])

        self.pickle_and_check(path)

    def test_condition_path(self):
        path = basf2.create_path()
        module = path.add_module("EventInfoSetter", evtNumList=[42])

        condition_path_1 = basf2.create_path()
        condition_path_1.add_module("Geometry", components=["SVD"])
        condition_path_2 = basf2.create_path()
        condition_path_2.add_module("Geometry", components=["CDC"])

        module.if_value("<3", condition_path_1)
        module.if_value(">5", condition_path_2)

        self.pickle_and_check(condition_path_1)
        self.pickle_and_check(condition_path_2)
        self.pickle_and_check(path)

if __name__ == '__main__':
    main()
