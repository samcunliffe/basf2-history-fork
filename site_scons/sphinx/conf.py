#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# basf2 documentation build configuration file, created by
# sphinx-quickstart on Mon May 23 17:26:16 2016.
#
# This file is execfile()d with the current directory set to its
# containing dir.
#
# Note that not all possible configuration values are present in this
# autogenerated file.
#
# All configuration values have a default; values that are commented out
# serve to show the default.

import sys
import os
import re
import subprocess
import jupytext

sys.path.insert(0, os.path.abspath("extensions"))

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
# sys.path.insert(0, os.path.abspath('....'))

# -- General configuration ------------------------------------------------

# If your documentation needs a minimal Sphinx version, state it here.
# needs_sphinx = '1.0'

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    'sphinx.ext.intersphinx',
    'sphinx.ext.autodoc',
    # 'sphinx.ext.autosummary',
    'sphinx.ext.mathjax',
    'sphinx.ext.napoleon',
    'sphinx.ext.viewcode',
    'sphinx.ext.autosectionlabel',
    'sphinxarg.ext',
    'basf2ext',
    'nbsphinx',
    'IPython.sphinxext.ipython_console_highlighting',
]

nbsphinx_allow_errors = True
# Anything that ends with .jupy.py will be understood as a jupyter
# notebook converted to a plain python file with jupytext. During the sphinx
# build, jupytext will converted it back to a .ipynb file and nbsphinx will
# build the HTML
nbsphinx_custom_formats = {
    '.doc.jupy.py': lambda s: jupytext.reads(s, '.py'),
}

# autosummary_generate = True

# prefix each section with the name of the document it is in followed by a
# colon
autosectionlabel_prefix_document = True

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_sphinxtemplates']

# The suffix(es) of source filenames.
# You can specify multiple suffix as a list of string:
# source_suffix = ['.rst', '.md']
source_suffix = '.rst'

# The encoding of source files.
# source_encoding = 'utf-8-sig'

# The master toctree document.
master_doc = 'index'

# General information about the project.
project = 'basf2'
copyright = '2010-2019, Belle II Collaboration'
author = 'Belle2 Software Group'

# The version info for the project you're documenting, acts as replacement for
# |version| and |release|, also used in various other places throughout the
# built documents.
#
# The short X.Y version.
version = subprocess.check_output(["git", "rev-parse", "--short", "HEAD"]).decode().strip()
# Used for links to the repository
basf2_repository = "https://stash.desy.de/projects/B2/repos/software"
basf2_commitid = subprocess.check_output(["git", "rev-parse", "HEAD"]).decode().strip()
basf2_jira = "https://agira.desy.de"

# The full version, including alpha/beta/rc tags.
release = os.environ.get('BELLE2_RELEASE', 'development')
if release == 'head':
    release = 'development'

# Add warnings in the code if this is not a release
keep_warnings = release == "development"
nitpicky = keep_warnings

# The language for content autogenerated by Sphinx. Refer to documentation
# for a list of supported languages.
#
# This is also used if you do content translation via gettext catalogs.
# Usually you set "language" from the command line for these cases.
language = None

# There are two options for replacing |today|: either, you set today to some
# non-false value, then it is used:
# today = ''
# Else, today_fmt is used as the format for a strftime call.
# today_fmt = '%B %d, %Y'

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This patterns also effect to html_static_path and html_extra_path
exclude_patterns = ['.*', '_sphinxbuild', 'Thumbs.db', 'build', 'include', 'lib', 'bin', 'modules', 'data', 'site_scons']
# If we want to create the light release documentation then we need t exclude anything not in the light release.
if tags.has('light'):  # noqa
    light_packages = set([entry.strip('/') for entry in open('../../.light').read().split() if entry.endswith('/')])
    for entry in os.listdir("../../"):
        if entry.find('.') > -1 or os.path.isfile(entry) or entry in exclude_patterns or entry in light_packages:
            continue
        exclude_patterns.append(entry)
    del light_packages

# now we need to exclude everything in the build dir except for the tools_doc
# sub dir but there's no negative exclusion pattern so do it manually
exclude_patterns.remove("build")
exclude_patterns += ['build/html', 'build/latex', 'build/json', 'build/Linux*']
# Ignore jupyter notebooks by default, we only want the ones meant for documentation
exclude_patterns += ['**/*.ipynb', '*.ipynb']

# The reST default role (used for this markup: `text`) to use for all
# documents. :any: allows easy linking to functions/classes/modules
default_role = 'any'

# If true, '()' will be appended to :func: etc. cross-reference text.
# add_function_parentheses = True

# If true, the current module name will be prepended to all description
# unit titles (such as .. function::).
# add_module_names = True

# If true, sectionauthor and moduleauthor directives will be shown in the
# output. They are ignored by default.
# show_authors = False

# The name of the Pygments (syntax highlighting) style to use.
pygments_style = 'sphinx'

# A list of ignored prefixes for module index sorting.
# modindex_common_prefix = []

# If true, keep warnings as "system message" paragraphs in the built documents.
# keep_warnings = False

# If true, `todo` and `todoList` produce output, else they produce nothing.
todo_include_todos = False

# If true, figures with captions automatically get figure numbers included at the start of the caption.
# Also the :numref: role is allowed for figures. Referencing figures/sections etc in this way
# will insert a reference to the reference number instead of the reference text/caption.
numfig = True

# -- Options for HTML output ----------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
html_theme = 'sphinx_rtd_theme'

# Theme options are theme-specific and customize the look and feel of a theme
# further.  For a list of options available for each theme, see the
# documentation.
# html_theme_options = {'stickysidebar': True}

# Add any paths that contain custom themes here, relative to this directory.
html_theme_path = ["_themes", ]

# The name for this set of Sphinx documents.
# "<project> v<release> documentation" by default.
# html_title = 'basf2 vhead'

# A shorter title for the navigation bar.  Default is the same as html_title.
# html_short_title = None

# The name of an image file (relative to this directory) to place at the top
# of the sidebar.
html_logo = "b2logo.svg"

# The name of an image file (relative to this directory) to use as a favicon of
# the docs.  This file should be a Windows icon file (.ico) being 16x16 or 32x32
# pixels large.
# html_favicon = None

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_sphinxstatic']

# Add any extra paths that contain custom files (such as robots.txt or
# .htaccess) here, relative to this directory. These files are copied
# directly to the root of the documentation.
# html_extra_path = []

# If not None, a 'Last updated on:' timestamp is inserted at every page
# bottom, using the given strftime format.
# The empty string is equivalent to '%b %d, %Y'.
# html_last_updated_fmt = None

# If true, SmartyPants will be used to convert quotes and dashes to
# typographically correct entities.
# html_use_smartypants = True

# Custom sidebar templates, maps document names to template names.
# html_sidebars = {}

# Additional templates that should be rendered to pages, maps page names to
# template names.
# html_additional_pages = {}

# If false, no module index is generated.
# html_domain_indices = True

# If false, no index is generated.
# html_use_index = True

# If true, the index is split into individual pages for each letter.
# html_split_index = False

# If true, links to the reST sources are added to the pages.
# html_show_sourcelink = True

# If true, "Created using Sphinx" is shown in the HTML footer. Default is True.
# html_show_sphinx = True

# If true, "(C) Copyright ..." is shown in the HTML footer. Default is True.
# html_show_copyright = True

# If true, an OpenSearch description file will be output, and all pages will
# contain a <link> tag referring to it.  The value of this option must be the
# base URL from which the finished HTML is served.
# html_use_opensearch = ''

# This is the file name suffix for HTML files (e.g. ".xhtml").
# html_file_suffix = None

# Language to be used for generating the HTML full-text search index.
# Sphinx supports the following languages:
#   'da', 'de', 'en', 'es', 'fi', 'fr', 'h', 'it', 'ja'
#   'nl', 'no', 'pt', 'ro', 'r', 'sv', 'tr', 'zh'
# html_search_language = 'en'

# A dictionary with options for the search language support, empty by default.
# 'ja' uses this config value.
# 'zh' user can custom change `jieba` dictionary path.
# html_search_options = {'type': 'default'}

# The name of a javascript file (relative to the configuration directory) that
# implements a search results scorer. If empty, the default will be used.
# html_search_scorer = 'scorer.js'

# Output file base name for HTML help builder.
htmlhelp_basename = 'basf2doc'

# -- Options for LaTeX output ---------------------------------------------

latex_elements = {
    # The paper size ('letterpaper' or 'a4paper').
    'papersize': 'a4paper',

    # The font size ('10pt', '11pt' or '12pt').
    'pointsize': '10pt',

    # Additional stuff for the LaTeX preamble.
    'preamble': '\\setcounter{tocdepth}{2}',

    # Latex figure (float) alignment
    # 'figure_align': 'htbp',
}

# Grouping the document tree into LaTeX files. List of tuples
# (source start file, target name, title,
#  author, documentclass [howto, manual, or own class]).
latex_documents = [
    (master_doc, 'basf2.tex', 'Belle 2 Software Documentation',
     author, 'manual'),
]

# The name of an image file (relative to this directory) to place at the top of
# the title page.
latex_logo = "belle2-logo.pdf"

# For "manual" documents, if this is true, then toplevel headings are parts,
# not chapters.
latex_use_parts = True
latex_show_urls = 'footnote'
latex_show_pagerefs = True

# If true, show page references after internal links.
# latex_show_pagerefs = False

# If true, show URL addresses after external links.
# latex_show_urls = False

# Documents to append as an appendix to all manuals.
# latex_appendices = []

# If false, no module index is generated.
# latex_domain_indices = True


# -- Options for manual page output ---------------------------------------

# One entry per manual page. List of tuples
# (source start file, name, description, authors, manual section).
man_pages = [
    (master_doc, 'basf2', 'basf2 Documentation',
     [author], 1)
]

# If true, show URL addresses after external links.
# man_show_urls = False


# -- Options for Texinfo output -------------------------------------------

# Grouping the document tree into Texinfo files. List of tuples
# (source start file, target name, title, author,
#  dir menu entry, description, category)
texinfo_documents = [
    (master_doc, 'basf2', 'basf2 Documentation',
     author, 'basf2', 'One line description of project.',
     'Miscellaneous'),
]

# Documents to append as an appendix to all manuals.
# texinfo_appendices = []

# If false, no module index is generated.
# texinfo_domain_indices = True

# How to display URL addresses: 'footnote', 'no', or 'inline'.
# texinfo_show_urls = 'footnote'

# If true, do not generate a @detailmenu in the "Top" node's menu.
# texinfo_no_detailmenu = False

# allow to have links to python documentation
intersphinx_mapping = {'python': ('https://docs.python.org/3.6', None)}


def process_sig(app, what, name, obj, options, signature, return_annotation):
    """
    remove unhelpful 'self' arguments from methods.
    """
    if what == 'method' and signature:
        reg = re.compile('^\\( \\(.*\\)arg1')
        signature = reg.sub('(', signature)
        return (signature, return_annotation)


def improve_docstring(obj):
    """
    Enhances docstrings of PyROOT objects/classes.

    >>> improve_docstring(Belle2.Variable.Manager)

    or

    >>> variables = ROOT.Belle2.Variable.Manager
    >>> improve_docstring(variables)
    """
    try:
        # is this a ..._meta object?
        classname = obj.__name__
        pyclass = obj
    except AttributeError:
        classname = obj.__class__.__name__
        pyclass = obj.__class__

    if '::' not in classname:
        return  # not a ROOT class?
    pos = classname.find('Belle2::')
    classname = classname[pos:]
    if pyclass.__doc__ is None:
        pyclass.__doc__ = ''

    pyclass.__name__ = 'Belle2.' + classname

    from ROOT import TClass
    tclass = TClass(classname)
    # if tclass:
    #    pyclass.__doc__ += '\n' + tclass.GetTitle()

    doxygen_url = 'https://b2-master.belle2.org/software/development/class'
    doxygen_url += '_1_1'.join(classname.split('::'))
    doxygen_url += '.html'
    pyclass.__doc__ += '\n`Doxygen page for %s <%s>`_' % (classname, doxygen_url)

    # TODO put this into the member docstrings directly? (sadly, readonly)
    members = tclass.GetListOfMethods()
    if members.GetEntries() > 0:
        pyclass.__doc__ += '\n\nMember functions:'
    for f in members:
        # getattr(pyclass, f.GetName()).__doc__ = "test"
        pyclass.__doc__ += '\n * %s %s%s' % (f.GetReturnTypeName(), f.GetName(), f.GetSignature())
        title = f.GetTitle()
        if title:
            pyclass.__doc__ += ' (%s)' % (title)

    members = tclass.GetListOfAllPublicDataMembers()
    if members.GetEntries() > 0:
        pyclass.__doc__ += '\n\nPublic data members'
    for f in members:
        pyclass.__doc__ += '\n * %s' % (f.GetName())
        title = f.GetTitle()
        if title:
            pyclass.__doc__ += ' (%s)' % (title)


def skipmember(app, what, name, obj, skip, options):
    """
    This is executed before docstring processing,
    so try improving them a bit.
    """
    try:
        improve_docstring(obj)
    except AttributeError:
        pass
    return skip


def process_docstring(app, what, name, obj, options, lines):
    """
    convert doxygen syntax to sphinx
    """
    substitutions = {
        re.compile(r'^( *)@param (.*?):? '): r':param \2: ',
        re.compile(r'^( *)@returns? '): r':return: ',
    }
    newlines = []
    for line in lines:
        new = line
        for reg, sub in substitutions.items():
            new = reg.sub(sub, new)
        if new != line:
            # Sphinx wants a new paragraph before these, so let's add one
            newlines += ['']
        newlines += [new]
    lines[:] = newlines


def setup(app):
    """
    Install some event handlers to improve output.
    """
    app.connect('autodoc-process-signature', process_sig)
    app.connect('autodoc-process-docstring', process_docstring)
    app.connect('autodoc-skip-member', skipmember)
    app.add_css_file('css/custom.css')


# Work around https://github.com/sphinx-doc/sphinx/issues/9189 by monkey patching the member function in question
# FIXME: Not needed with sphinx>=4.0.1

from sphinx.ext import autodoc  # noqa
# remember the old function
old_directive_header = autodoc.PropertyDocumenter.add_directive_header


# make a new one that ignores the value error
def _fixed_directive_header(*args, **argk):
    """Catch the ValueError and ignore it as done in https://github.com/sphinx-doc/sphinx/pull/9190"""
    try:
        old_directive_header(*args, **argk)
    except ValueError:
        return None


# and override the existing function with our new and improved version.
autodoc.PropertyDocumenter.add_directive_header = _fixed_directive_header
