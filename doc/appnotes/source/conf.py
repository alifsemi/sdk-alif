# -*- coding: utf-8 -*-
#
# Configuration file for the Sphinx documentation builder.
#

import os
import sys
from pathlib import Path
import re
import shutil

# Path setup
ZEPHYR_BASE = Path(__file__).resolve().parents[2]
ZEPHYR_BUILD = Path(os.environ.get("OUTPUT_DIR", "_build")).resolve()

sys.path.insert(0, str(ZEPHYR_BASE / "doc" / "_extensions"))
sys.path.insert(0, str(ZEPHYR_BASE / "doc" / "_scripts"))
sys.path.insert(0, str(ZEPHYR_BASE / "scripts" / "west_commands"))

# Version info
version = "2.0"
release = "2.0"
if (ZEPHYR_BASE / "VERSION").exists():
    with open(ZEPHYR_BASE / "VERSION", "r") as file:
        content = file.read()
        major_match = re.search(r"VERSION_MAJOR\s*=\s*(\d+)", content)
        minor_match = re.search(r"VERSION_MINOR\s*=\s*(\d+)", content)
        patch_match = re.search(r"PATCHLEVEL\s*=\s*(\d+)", content)
        if major_match and minor_match:
            major, minor = major_match.group(1), minor_match.group(1)
            version = f"{major}.{minor}"
            release = f"{major}.{minor}.{patch_match.group(1)}" if patch_match else version

# Project info
project = "Zephyr Alif SDK Application Notes"
copyright = "2024-2025, Alif Semiconductor"
author = " "

# Extensions
extensions = [
    'sphinx_copybutton',
    'sphinx.ext.todo',
    'sphinx.ext.extlinks',
    'sphinx.ext.autodoc',
    'sphinx.ext.graphviz',
    'sphinx_rtd_theme',
    'sphinx.ext.imgconverter',
    'sphinx_tabs.tabs',
    'sphinx.ext.autosectionlabel',
]

templates_path = ['_templates']
exclude_patterns = []

# HTML theme settings
html_theme = 'sphinx_rtd_theme'
html_static_path = ['_static']
html_logo = "_static/logo.png"
html_favicon = "_static/favicon.png"

html_theme_options = {
    "logo_only": True,
    "collapse_navigation": True,       # ✅ Collapse other sections when one is opened
    "navigation_depth": 4,             # ✅ Keep deep nesting
    "titles_only": False,
    "prev_next_buttons_location": None,
    "style_nav_header_background": "#2980b9",
}

html_title = f"Zephyr Alif SDK Appnotes - v{release}"
html_last_updated_fmt = "%b %d, %Y"
html_show_sphinx = False
html_additional_pages = {"search": "search.html"}

html_context = {
    "current_version": release,
    "versions": [("latest", "/"), (release, f"/{release}/")],
}

# Autosectionlabel settings
autosectionlabel_prefix_document = True
autosectionlabel_maxdepth = 2

# Substitution for version in RST files
rst_epilog = f"""
.. include:: /links.txt
.. |version| replace:: {version}
.. |release| replace:: {release}
"""

# LaTeX settings (unchanged)
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
        \geometry{a4paper,left=20mm,top=20mm,right=20mm,bottom=20mm}
        \usepackage{etoolbox}
        \pretocmd{\tableofcontents}{\clearpage}{}{}
        \AddToShipoutPictureFG*{\AtPageUpperLeft{\hspace*{0.1\textwidth}\vspace*{0.1\textheight}\includegraphics[width=2cm]{_static/logo.png}}}
        \addto\captionsenglish{\renewcommand{\contentsname}{Table of Contents}}
        \usepackage{textcomp}
        \DeclareUnicodeCharacter{03BC}{\textmu}
        \DeclareUnicodeCharacter{2212}{-}
        \usepackage{url}
        \usepackage{seqsplit}
        \floatplacement{figure}{H}
        \hyphenation{Unencrypted Encrypted}
        \makeatletter
        \def\FV@ObeyVerbFont{\ifx\FancyVerbFont\FV@Inconsolata\fontshape{n}\selectfont\else\FancyVerbFont\fi}
        \makeatother
        \usepackage{longtable}
        \usepackage{needspace}
    ''',
}

latex_documents = [('index', 'Alif_SDK_Appnotes.tex', project, author, 'manual')]
latex_logo = "_static/logo.png"
latex_show_pagerefs = True
latex_show_urls = 'footnote'
latex_appendices = []
latex_domain_indices = False
latex_theme = 'manual'

numfig = True
numfig_format = {'figure': 'Figure %s', 'table': 'Table %s', 'code-block': 'Listing %s'}

def setup(app):
    app.add_css_file("css/custom.css")
    app.add_js_file("js/custom.js")
    def copy_static_files(app, exception):
        if not exception:
            static_dir = Path(app.builder.srcdir) / '_static'
            latex_dir = Path(app.builder.outdir)
            if static_dir.exists():
                shutil.copytree(str(static_dir), str(latex_dir / '_static'), dirs_exist_ok=True)
    app.connect('build-finished', copy_static_files)