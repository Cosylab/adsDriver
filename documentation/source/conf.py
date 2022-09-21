# SPDX-FileCopyrightText: 2022 Cosylab d.d.
#
# SPDX-License-Identifier: MIT-0

# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
# import os
# import sys
# sys.path.insert(0, os.path.abspath('.'))


# -- Project information -----------------------------------------------------

project = 'EPICS ADS Device Support'
copyright = '2022, Cosylab d.d'
author = 'EPICS ADS Team'


# -- General configuration ---------------------------------------------------
master_doc = 'index'
source_suffix = '.rst'

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = ['sphinx_rtd_theme']

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = []


# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'sphinx_rtd_theme'

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']

html_theme_options = {
    'collapse_navigation': False,
#    'analytics_id': UA-XXXXX or G-XXX,
    'display_version': False,
    'style_nav_header_background': '#00002C'
}

html_css_files = [
    'custom.css',
]

html_title = "EPICS ADS Documentation"
html_baseurl = "documentation/ads"

html_logo = "_static/Cosylab_Logo.png"
html_favicon = "_static/Cosylab_Favicon.png"
