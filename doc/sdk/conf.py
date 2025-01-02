# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

from pathlib import Path
import shutil
import sys


# -- Project information -----------------------------------------------------

project = 'SDK documentation'
copyright = '2024, Alif Semiconductor'
author = 'Alif Semiconductor'
release = '1.1'

SDK_BASE = Path(__file__).absolute().parents[2]


# -- General configuration ---------------------------------------------------

extensions = [
    'sphinx_copybutton'
]

rst_epilog = """
.. include:: /links.txt
.. include:: /substitutions.txt
"""

# -- Options for HTML output -------------------------------------------------

html_theme = 'sphinx_rtd_theme'
html_static_path = [str(SDK_BASE / "doc" / "_static")]
html_logo = "../_static/logo.png"
html_favicon = "../_static/favicon.png"
html_theme_options = {
    "logo_only": True,
    "navigation_depth": 4,
    "prev_next_buttons_location": None,
    "style_nav_header_background": "#2980b9",
}
html_title = f"Alif SDK Documentation - v{release}"
html_last_updated_fmt = "%b %d, %Y"
html_show_sphinx = False
html_additional_pages = {
    "search": "search.html"
}
html_context = {
    "current_version": release,
    "versions": [
        ("latest", "/"),
        (release, f"/{release}/"),
    ]
}


# -- Options for LaTeX output -------------------------------------------------

latex_elements = {
    'papersize': 'a4paper',
    'extraclassoptions': 'openany,oneside',
    'preamble': r'''
        \usepackage{graphicx}
        \usepackage{charter}
        \usepackage[defaultsans]{lato}
        \usepackage[T1]{fontenc}
        \usepackage{inconsolata}
        \usepackage{hyperref}
        \usepackage{float}
        \usepackage{titlesec}
        \titlespacing*{\section}{0pt}{*0}{*0}
        \titlespacing*{\subsection}{0pt}{*0}{*0}
        \setlength{\parskip}{0pt}
        \setlength{\parindent}{0pt}
        \usepackage{multicol}
        \usepackage{eso-pic}
        \usepackage{tikz}
        \usepackage{geometry}
        \geometry{
            a4paper,
            left=20mm,
            top=20mm,
            right=20mm,
            bottom=20mm
        }
        \usepackage{etoolbox}
        \pretocmd{\tableofcontents}{\clearpage}{}{}
        \AddToShipoutPictureFG*{
            \AtPageUpperLeft{
                \hspace*{0.1\textwidth} % Adjust horizontal offset here
                \vspace*{0.1\textheight} % Adjust vertical offset here
                \includegraphics[width=2cm]{_static/logo.png} % Adjust size here
            }
        }
        \addto\captionsenglish{\renewcommand{\contentsname}{Table of Contents}}
        \setcounter{tocdepth}{3} % TOC depth
    ''',
}

latex_documents = [
    ('index', 'AlifSDK.tex', project,
     author, 'manual'),
]

latex_logo = str(SDK_BASE / "doc" / "_static" / "logo.png")
latex_show_pagerefs = True
latex_show_urls = 'footnote'
latex_appendices = []
latex_domain_indices = False
latex_theme = 'manual'

# Image numbering settings
numfig = True
numfig_format = {
    'figure': 'Figure %s',
    'table': 'Table %s',
    'code-block': 'Listing %s'
}

def setup(app):
    # Adding custom CSS and JS files
    app.add_css_file("css/custom.css")
    app.add_js_file("js/custom.js")
    # Function to copy static files to LaTeX build directory
    def copy_static_files(app, exception):
        if not exception:
            static_dir = Path(str(SDK_BASE / "doc" / '_static'))
            latex_dir = Path(app.builder.outdir)
            if static_dir.exists():
                shutil.copytree(str(static_dir), str(latex_dir / '_static'), dirs_exist_ok=True)
    # Connect the callback function to the 'build-finished' event
    app.connect('build-finished', copy_static_files)
