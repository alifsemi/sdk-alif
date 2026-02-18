import os
import sys
from pathlib import Path
import shutil

# -- Path setup --------------------------------------------------------------
# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use Path(__file__).resolve().parent.joinpath('relative_path').

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'Release Notes of Zephyr Alif SDK'
copyright = '2024-2025, Alif Semiconductor'
author = 'Alif Semiconductor'
release = '2.2.0'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    'sphinx_copybutton',
    'sphinx.ext.todo',
    'sphinx.ext.autodoc',
    'sphinx.ext.viewcode',
    'sphinx_rtd_theme',
]

templates_path = ['_templates']
exclude_patterns = []

rst_epilog = """
.. include:: /links.txt
.. |release| replace:: {release}
""".format(release=release)

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'sphinx_rtd_theme'
html_static_path = ['_static']
html_logo = "_static/logo.png"
html_favicon = "_static/favicon.png"
html_theme_options = {
    "logo_only": True,
    "navigation_depth": 4,
    "collapse_navigation": False,
    "sticky_navigation": True,
}
html_title = f"Zephyr Alif SDK Release Notes - v{release}"
html_last_updated_fmt = "%b %d, %Y"
html_show_sphinx = False

# -- Options for LaTeX output ------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-latex-output

latex_elements = {
    'papersize': 'a4paper',
    'extraclassoptions': 'openany,oneside',
    'pointsize': '11pt',
    'preamble': r'''
        \usepackage{_static/latex/alif_semiconductor}
    ''',
}

latex_documents = [
    ('index', 'release_notes.tex', project,
     author, 'manual', False),
]

latex_logo = "_static/logo.png"
latex_show_urls = 'footnote'
latex_domain_indices = False

# -- Custom setup ------------------------------------------------------------

def setup(app):
    app.add_css_file("css/custom.css")
    def copy_static_files(app, exception):
        if not exception:
            static_dir = Path(app.builder.srcdir) / '_static'
            latex_dir = Path(app.builder.outdir)
            if static_dir.exists():
                shutil.copytree(str(static_dir), str(latex_dir / '_static'), dirs_exist_ok=True)
    app.connect('build-finished', copy_static_files)
