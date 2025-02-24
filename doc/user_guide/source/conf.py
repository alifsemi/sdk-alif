# -*- coding: utf-8 -*-
#
# **Configuration file for the Sphinx documentation builder.**
#
# Reference: https://www.sphinx-doc.org/en/master/usage/configuration.html

import os
import sys
from pathlib import Path
import re
import shutil

# Path setup
ZEPHYR_BASE = Path(__file__).resolve().parents[2]
ZEPHYR_BUILD = Path(os.environ.get("OUTPUT_DIR", "_build")).resolve()

# Add paths for Sphinx extensions and utility modules
sys.path.insert(0, str(ZEPHYR_BASE / "doc" / "_extensions"))
sys.path.insert(0, str(ZEPHYR_BASE / "doc" / "_scripts"))
sys.path.insert(0, str(ZEPHYR_BASE / "scripts" / "west_commands"))

# Initialize version and release with default values
version = "1.2"
release = "1.2"

# Read the version from VERSION file if it exists
if (ZEPHYR_BASE / "VERSION").exists():
    with open(ZEPHYR_BASE / "VERSION", "r") as file:
        content = file.read()
        major_match = re.search(r"VERSION_MAJOR\s*=\s*(\d+)", content)
        minor_match = re.search(r"VERSION_MINOR\s*=\s*(\d+)", content)
        patch_match = re.search(r"PATCHLEVEL\s*=\s*(\d+)", content)
        if major_match and minor_match:
            major, minor = major_match.group(1), minor_match.group(1)
            version = f"{major}.{minor}"
            if patch_match:
                patch = patch_match.group(1)
                release = f"{major}.{minor}.{patch}"
            else:
                release = version

# Project information
project = f"Getting Started with Zephyr Alif SDK"
copyright = f"2024, Alif Semiconductor"
author = " "

# General configuration
extensions = [
    'sphinx_copybutton',
    'sphinx.ext.todo',
    'sphinx.ext.extlinks',
    'sphinx.ext.autodoc',
    'sphinx.ext.graphviz',
    'sphinx_rtd_theme',
    'sphinx.ext.imgconverter',
    'sphinx_tabs.tabs',
]

templates_path = ['_templates']
exclude_patterns = []

# Options for HTML output
html_theme = 'sphinx_rtd_theme'
html_static_path = ['_static']
html_logo = "_static/logo.png"
html_favicon = "_static/favicon.png"
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

# Substitution for version in RST files and links.txt inclusion
rst_epilog = f"""
.. include:: /links.txt
.. |version| replace:: {version}
.. |release| replace:: {release}
"""

# **Options for LaTeX output**
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
    ''',
}

latex_documents = [
    ('index', 'AlifSDK.tex', project,
     author, 'manual'),
]

latex_logo = "_static/logo.png"
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
            static_dir = Path(app.builder.srcdir) / '_static'
            latex_dir = Path(app.builder.outdir)
            if static_dir.exists():
                shutil.copytree(str(static_dir), str(latex_dir / '_static'), dirs_exist_ok=True)
    # Connect the callback function to the 'build-finished' event
    app.connect('build-finished', copy_static_files)
