#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import glob
import re
import sys
import subprocess
import basf2
from time import time

"""
Check that no light-release-breaking dependencies have been added.

If you are failing this test you have managed to break light builds, please
check with the light release manager (Frank Meier) for more information.
"""


def get_sconscripts(package):
    """glob to get the path to all sconscript files

    Parameters:
        package (str): the name of the package directory

    Returns:
        list(str): a list of all SConscript files
    """
    return glob.glob(package + "/**/SConscript", recursive=True)


def get_dependencies(sconscript_filename):
    """grab all of the LIBS in the sconscript file

    Parameters:
        sconscript_filename (str): the path to the sconscript file

    Returns:
        set(str): a set of packages in the env['LIBS'] in this file
    """
    # get the " env['LIBS'] " line in the file
    # (if no LIBS are in the SConscript file then ignore)
    with open(sconscript_filename) as fi:
        file_data = fi.read()
        dependencies = re.search(
            r"env\['LIBS'\] *= *\[\n*((?:.*?|\n)*?)\n*\]", file_data
        )
        if dependencies:
            dependencies = dependencies.group(1)
        else:
            dependencies = ""

    # clean up the list retuned from the regex: only want the names of the
    # basf2 libs (i.e. strings of the form: packagename_module)
    dependencies = [
        lb.replace("'", "").replace('"', "").strip()
        for lb in re.split(",|\n", dependencies)
        if not lb.count("#") and not lb == ""
    ]
    # now just trim only the packagename's from the library names
    # and make a python set of them
    package_dependencies = set([lb.split("_")[0] for lb in dependencies])
    return package_dependencies


def check_dependencies(forbidden, sconscript_files, exceptions, error=""):
    """Check that there are no dependencies that are forbidden.

    Parameters:
        forbidden set(str): a set of packages we don't want to depend on
        sconscript_files list(str): a list of paths to sconscript files
        exceptions list(str): if a sconscript file contains one of the
          strings listed here, don't check it
        error str: optionally specify the error message

    Returns:
        int: count of the number of forbidden dependencies found
    """
    n_forbidden = 0
    for sconscript_filename in sconscript_files:
        if any(exception in sconscript_filename for exception in exceptions):
            continue
        # get the dependencies from this SConscript file
        this_dependencies = get_dependencies(sconscript_filename)

        # using the nice and cool and mathsy python set syntax: figure out if
        # we depend on anything in the forbidden packages list
        forbidden_packages_dependencies = forbidden.intersection(this_dependencies)

        # if so then throw the error
        if len(forbidden_packages_dependencies) != 0:
            print(
                f"The sconscript file {sconscript_filename} depends on",
                forbidden_packages_dependencies,
                error,
            )
            n_forbidden += 1
    return n_forbidden


# grab all of the light release packages
# (defined by the .light file for the sparse checkout)
with open(basf2.find_file(".light")) as fi:
    light_packages = [li.strip().replace("/", "") for li in fi if li.strip().endswith("/")]

# we also need all packages (to compare), for this use b2code-package-list
all_packages = subprocess.Popen(
    ["b2code-package-list"], stdout=subprocess.PIPE
).communicate()[0]
all_packages = all_packages.decode("utf-8").strip().split(" ")

# the forbidden packages
non_light_packages = set(all_packages).difference(set(light_packages))

# if a sconscript contains one of this strings, don't check it
test_exceptions = ['online_book/awesome']

# sum up the return codes
return_code_sum = 0

# run the check over all packages - just a dumb nested loop
start = time()
for package in light_packages:
    sconscript_files = get_sconscripts(package)
    return_code_sum += check_dependencies(
        non_light_packages,
        sconscript_files,
        test_exceptions,
        "This breaks light releases! Sorry."
    )

# test finished, now report
print("Test of light dependencies, the loop took:", time() - start, "seconds to run")
print("There were", return_code_sum, "forbidden dependencies")

if return_code_sum != 0:
    sys.exit(1)  # fails the test
