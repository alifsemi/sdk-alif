# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'Getting started with Zephyr Alif SDK'
copyright = '2024, Alif Semiconductor'
author = 'Alif Semiconductor'
release = '1.00'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    'sphinx_copybutton'
]

templates_path = ['_templates']
exclude_patterns = []

rst_epilog = """
.. include:: /links.txt
"""

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'sphinx_rtd_theme'
#html_theme = 'alabaster'

latex_elements = {
  'extraclassoptions': 'openany,oneside'
}

latex_show_urls = 'footnote'

