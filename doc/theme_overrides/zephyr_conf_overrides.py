# Zephyr doc build overrides to use the SDK theme/assets
# Usage:
#   sphinx-build -b html -c /absolute/path/to/alif/doc/theme_overrides \
#       /absolute/path/to/zephyr/doc /absolute/path/to/zephyr/doc/_build/html

from pathlib import Path

# Resolve SDK doc root (alif/doc)
SDK_DOC_ROOT = Path(__file__).resolve().parents[1]

# Theme and assets — keep aligned with SDK docs
html_theme = 'sphinx_rtd_theme'
html_logo = str(SDK_DOC_ROOT / '_static' / 'logo.png')
html_favicon = str(SDK_DOC_ROOT / '_static' / 'favicon.png')

# Ensure our static path and custom CSS are applied
html_static_path = [str(SDK_DOC_ROOT / '_static')]
html_css_files = [
    'alif.css',
]
