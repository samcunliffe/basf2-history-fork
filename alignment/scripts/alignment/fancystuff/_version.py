#! -*- coding: utf-8 -*-

"""
    Retrieval of version number

    This file helps to compute a version number in source trees obtained from
    git-archive tarball (such as those provided by githubs download-from-tag
    feature). Distribution tarballs (built by setup.py sdist) and build
    directories (produced by setup.py build) will contain a much shorter file
    that just contains the computed version number.
    This file was generated by PyScaffold.
"""

import inspect
import os
import re
import subprocess
import sys

__location__ = os.path.join(os.getcwd(), os.path.dirname(
    inspect.getfile(inspect.currentframe())))

# these strings will be replaced by git during git-archive
git_refnames = "$Format:%d$"
git_full = "$Format:%H$"

# general settings
tag_prefix = 'v'  # tags are like v1.2.0
package = "fancystuff"
namespace = []
root_pkg = namespace[0] if namespace else package
if namespace:
    pkg_path = os.path.join(*namespace[-1].split('.') + [package])
else:
    pkg_path = package


class ShellCommand(object):

    def __init__(self, command, shell=True, cwd=None):
        self._command = command
        self._shell = shell
        self._cwd = cwd

    def __call__(self, *args):
        command = "{cmd} {args}".format(cmd=self._command,
                                        args=subprocess.list2cmdline(args))
        output = subprocess.check_output(command,
                                         shell=self._shell,
                                         cwd=self._cwd,
                                         stderr=subprocess.STDOUT,
                                         universal_newlines=True)
        return self._yield_output(output)

    def _yield_output(self, msg):
        for line in msg.splitlines():
            yield line


def get_git_cmd(**args):
    if sys.platform == "win32":
        for cmd in ["git.cmd", "git.exe"]:
            git = ShellCommand(cmd, **args)
            try:
                git("--version")
            except (subprocess.CalledProcessError, OSError):
                continue
            return git
        return None
    else:
        git = ShellCommand("git", **args)
        try:
            git("--version")
        except (subprocess.CalledProcessError, OSError):
            return None
        return git


def version_from_git(tag_prefix, root, verbose=False):
    # this runs 'git' from the root of the source tree. This only gets called
    # if the git-archive 'subst' keywords were *not* expanded, and
    # _version.py hasn't already been rewritten with a short version string,
    # meaning we're inside a checked out source tree.
    git = get_git_cmd(cwd=root)
    if not git:
        print("no git found")
        return None
    try:
        tag = next(git("describe", "--tags", "--dirty", "--always"))
    except subprocess.CalledProcessError:
        return None
    if not tag.startswith(tag_prefix):
        if verbose:
            print("tag '{}' doesn't start with prefix '{}'".format(tag,
                                                                   tag_prefix))
        return None
    tag = tag[len(tag_prefix):]
    sha1 = next(git("rev-parse", "HEAD"))
    full = sha1.strip()
    if tag.endswith("-dirty"):
        full += "-dirty"
    return {"version": tag, "full": full}


def get_keywords(versionfile_abs):
    # the code embedded in _version.py can just fetch the value of these
    # keywords. When used from setup.py, we don't want to import _version.py,
    # so we do it with a regexp instead. This function is not used from
    # _version.py.
    keywords = dict()
    try:
        with open(versionfile_abs, "r") as fh:
            for line in fh.readlines():
                if line.strip().startswith("git_refnames ="):
                    mo = re.search(r'=\s*"(.*)"', line)
                    if mo:
                        keywords["refnames"] = mo.group(1)
                if line.strip().startswith("git_full ="):
                    mo = re.search(r'=\s*"(.*)"', line)
                    if mo:
                        keywords["full"] = mo.group(1)
    except EnvironmentError:
        return None
    return keywords


def version_from_keywords(keywords, tag_prefix, verbose=False):
    if not keywords:
        return None  # keyword-finding function failed to find keywords
    refnames = keywords["refnames"].strip()
    if refnames.startswith("$Format"):
        if verbose:
            print("keywords are unexpanded, not using")
        return None  # unexpanded, so not in an unpacked git-archive tarball
    refs = set([r.strip() for r in refnames.strip("()").split(",")])
    # starting in git-1.8.3, tags are listed as "tag: foo-1.0" instead of
    # just "foo-1.0". If we see a "tag: " prefix, prefer those.
    TAG = "tag: "
    tags = set([r[len(TAG):] for r in refs if r.startswith(TAG)])
    if not tags:
        # Either we're using git < 1.8.3, or there really are no tags. We use
        # a heuristic: assume all version tags have a digit. The old git %d
        # expansion behaves like git log --decorate=short and strips out the
        # refs/heads/ and refs/tags/ prefixes that would let us distinguish
        # between branches and tags. By ignoring refnames without digits, we
        # filter out many common branch names like "release" and
        # "stabilization", as well as "HEAD" and "master".
        tags = set([r for r in refs if re.search(r'\d', r)])
        if verbose:
            print("discarding '{}', no digits".format(",".join(refs - tags)))
    if verbose:
        print("likely tags: {}".format(",".join(sorted(tags))))
    for ref in sorted(tags):
        # sorting will prefer e.g. "2.0" over "2.0rc1"
        if ref.startswith(tag_prefix):
            r = ref[len(tag_prefix):]
            if verbose:
                print("picking {}".format(r))
            return {"version": r,
                    "full": keywords["full"].strip()}
    else:
        if verbose:
            print("no suitable tags, using full revision id")
        return {"version": keywords["full"].strip(),
                "full": keywords["full"].strip()}


def version_from_parentdir(parentdir_prefix, root, verbose=False):
    # Source tarballs conventionally unpack into a directory that includes
    # both the project name and a version string.
    dirname = os.path.basename(root)
    if not dirname.startswith(parentdir_prefix):
        if verbose:
            print("guessing rootdir is '{}', but '{}' doesn't start with "
                  "prefix '{}'".format(root, dirname, parentdir_prefix))
        return None
    version = dirname[len(parentdir_prefix):].split('-')[0]
    return {"version": version, "full": ""}


def git2pep440(ver_str):
    dash_count = ver_str.count('-')
    if dash_count == 0:
        return ver_str
    elif dash_count == 1:
        return ver_str.split('-')[0] + "+dirty"
    elif dash_count == 2:
        tag, commits, sha1 = ver_str.split('-')
        return "{}.post0.dev{}+{}".format(tag, commits, sha1)
    elif dash_count == 3:
        tag, commits, sha1, _ = ver_str.split('-')
        return "{}.post0.dev{}+{}.dirty".format(tag, commits, sha1)
    else:
        raise RuntimeError("Invalid version string")


def get_versions(verbose=False):
    vcs_kwds = {"refnames": git_refnames, "full": git_full}
    parentdir = package + '-'
    root = __location__
    # pkg_path is the relative path from the top of the source
    # tree (where the .git directory might live) to this file.
    # Invert this to find the root of our package.
    for _ in pkg_path.split(os.sep):
        root = os.path.dirname(root)

    # different version retrieval methods as (method, args, comment)
    ver_retrieval = [
        (version_from_keywords, (vcs_kwds, tag_prefix, verbose),
         'expanded keywords'),
        (version_from_parentdir, (parentdir, root, verbose), 'parentdir'),
        (version_from_git, (tag_prefix, root, verbose), 'git')
    ]
    for method, args, comment in ver_retrieval:
        ver = method(*args)
        if ver:
            if verbose:
                print("got version from {}".format(comment))
            break
    else:
        ver = {"version": "unknown", "full": ""}
    ver['version'] = git2pep440(ver['version'])
    return ver
