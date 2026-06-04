import shutil
from pathlib import Path
from typing import Any

# -- Path setup --------------------------------------------------------------
# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use Path(__file__).resolve().parent.joinpath('relative_path').

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'Application Notes for Zephyr Alif SDK'
copyright = '2025-2026, Alif Semiconductor'
author = 'Alif Semiconductor'
release = '2.3.0'

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
html_title = f"Application Notes for Zephyr Alif SDK - v{release}"
html_last_updated_fmt = "%b %d, %Y"
html_show_sphinx = False

# -- Options for LaTeX output ------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-latex-output

latex_elements = {
    'papersize': 'a4paper',
    'extraclassoptions': 'openany,oneside',
    'pointsize': '11pt',
    'preamble': r'''
        \usepackage{float}
        \usepackage{graphicx}
        \usepackage{_static/latex/alif_semiconductor}
    ''',
    'figure_align': 'H',  # Force figures to appear exactly where placed
}

latex_table_style = ['booktabs', 'colorrows']

latex_use_latex_multicolumn = False

latex_documents = [
    ('index', 'Appnotes.tex', 'Application Notes for Zephyr Alif SDK',
     'Alif Semiconductor', 'manual', False),
]

latex_logo = "_static/logo.png"
latex_show_urls = 'no'
latex_domain_indices = False

# -- Custom setup ------------------------------------------------------------

def setup(sphinx_app: Any) -> None:
    sphinx_app.add_css_file("css/custom.css")

    def copy_static_files(build_app: Any, exception: Exception | None) -> None:
        if exception is None:
            static_dir = Path(build_app.builder.srcdir) / '_static'
            latex_dir = Path(build_app.builder.outdir)
            if static_dir.exists():
                shutil.copytree(static_dir, latex_dir / '_static', dirs_exist_ok=True)

    sphinx_app.connect('build-finished', copy_static_files)
