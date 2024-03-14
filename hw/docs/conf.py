import sys, os

version = '0.1'
sys.path.append(os.path.abspath("./_ext"))

extensions = [
    "sphinx_needs",
    "sphinx_rtd_theme",
    'jupyter_sphinx',
    'sphinx.ext.mathjax',
    'matplotlib.sphinxext.mathmpl',
    'matplotlib.sphinxext.plot_directive',
    'sphinx.ext.intersphinx',
    'sphinx.ext.autodoc',
    'sphinx.ext.autosectionlabel',
    'sphinx.ext.doctest',
    'sphinxcontrib.mermaid',
    'sphinxcontrib.kroki',
    'linuxdoc.rstFlatTable',
    'sphinxcontrib.drawio',
]

autosectionlabel_prefix_document = True

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

needs_build_json = True
needs_autoexport = True
needs_builder_filter = "True"
needs_external_needs = []
needs_id_required = True
needs_id_regex = "^[a-zA-Z0-9_]{3,}"
needs_template_folder = "_templates"
needs_types = [
    dict(directive="req-hw", title="Hardware Requirement", prefix="HW_REQ_", color="#BFD8D2", style="node"),
    dict(directive="hw-drs", title="Hardware Design Requirement", prefix="HW_DRS_", color="#BFD8D2", style="node"),
    dict(directive="fmea", title="Failure mode", prefix="FMEA_", color="#DF744A", style="node"),
    dict(directive="test", title="Test Case", prefix="TC_HW_", color="#DCB239", style="node"),
]
needs_statuses = [
    dict(name="draft", description="Still under work or waiting for review"),
    dict(name="reviewed", description="Requirement is valid"),
    dict(name="accepted", description="Requirement will be taken into account")
]
needs_tags = [
    dict(name="security", description="tag for security requirements"),
    dict(name="safety", description="tag for safety requirements")
]
needs_extra_options = [
    "probability",      # valid values: none, remote, occasional, frequent
    "severity",         # valid values: none, minor, critical, catastrophic
    "detection",        # valid values: always, high, low, undetected,
    "mitigation",       # free text
]
needs_extra_links = [
    {
        "option": "satisfies",
        "incoming": "is satisfied by",
        "outgoing": "satisfies"
    },
    {
        "option": "implements",
        "incoming": "is implemented by",
        "outgoing": "implements"
    },
    {
        "option": "tests",
        "incoming": "is tested by",
        "outgoing": "tests"
    },
    {
        "option": "relates",
        "incoming": "is related to",
        "outgoing": "relates"
    },
    {
        "option": "affects",
        "incoming": "is affected by",
        "outgoing": "affects"
    }
]
