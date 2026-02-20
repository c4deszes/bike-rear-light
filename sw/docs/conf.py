import sys, os

project = "rear-light"
copyright = "Balazs Eszes, 2026"
author = "Balazs Eszes"
version = '0.1'
sys.path.append(os.path.abspath("./_ext"))

extensions = [
    'sphinx.ext.intersphinx',
    'sphinx.ext.autodoc',
    'sphinx.ext.autosectionlabel',
    'sphinx.ext.doctest',
    "sphinx_rtd_theme",
    "breathe",
    'jupyter_sphinx',
    'sphinx.ext.mathjax',
    'matplotlib.sphinxext.mathmpl',
    'matplotlib.sphinxext.plot_directive',
    'sphinxcontrib.kroki',
    'sphinxcontrib.drawio',
]

autosectionlabel_prefix_document = True

# Drawio configuration
drawio_no_sandbox=True

# Breathe configuration
breathe_default_project = "rear-light"
breathe_projects = {}
breathe_show_define_initializer = False

# Sphinx configuration
html_static_path = ['_static']
html_style = 'css/custom.css'
html_theme = "sphinx_rtd_theme"
html_theme_options = {
    "collapse_navigation" : False
}
html_js_files = ["https://cdn.jsdelivr.net/npm/mermaid/dist/mermaid.min.js"]

source_suffix = '.rst'
master_doc = 'index'
