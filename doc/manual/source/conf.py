# UltraScan III Documentation Configuration
#
# Sphinx configuration file for building the UltraScan documentation.
# See the Sphinx documentation for the full list of configuration options:
# https://www.sphinx-doc.org/

# ---------------------------------------------------------------------------
# Path setup
#
# If Python modules need to be imported for autodoc, add them to sys.path
# here.
#
# Example:
#   import os
#   import sys
#   sys.path.insert(0, os.path.abspath('../src'))
# ---------------------------------------------------------------------------
from __future__ import annotations
import datetime
import re
from pathlib import Path
from sphinxcontrib.qthelp import QtHelpBuilder
import os
import subprocess
import sys
# add current directory to path
sys.path.insert(0, os.path.abspath('.'))


def _run_git(args: list[str], cwd: Path) -> str:
    try:
        result = subprocess.run(
            ["git", *args],
            cwd=str(cwd),
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            text=True,
            check=False,
        )
        return result.stdout.strip()
    except OSError:
        return ""


def generate_version_metadata(source_dir: str | os.PathLike[str]) -> dict[str, str]:
    source_dir: Path = Path(source_dir)

    build_number = _run_git(["rev-list", "--count", "HEAD"], source_dir) or "0"
    git_revision = _run_git(["rev-parse", "--short=7", "HEAD"], source_dir) or "unknown"
    git_branch = _run_git(["rev-parse", "--abbrev-ref", "HEAD"], source_dir) or "unknown"

    # Tracked changes only: ignore untracked files
    dirty = False
    try:
        worktree_rc = subprocess.run(
            ["git", "diff", "--quiet", "--exit-code"],
            cwd=str(source_dir),
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            check=False,
        ).returncode

        index_rc = subprocess.run(
            ["git", "diff", "--quiet", "--cached", "--exit-code"],
            cwd=str(source_dir),
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            check=False,
        ).returncode

        dirty = (worktree_rc != 0) or (index_rc != 0)
    except OSError:
        dirty = False

    local_changes = "Δ" if dirty else ""
    git_dirty_flag = "-dirty" if dirty else ""

    try:
        revision_date = subprocess.run(
            [
                "git",
                "log",
                "-1",
                "--date=format-local:%Y-%m-%d %H:%M:%S UTC",
                "--format=%cd",
            ],
            cwd=str(source_dir),
            env={**os.environ, "TZ": "UTC0"},
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            text=True,
            check=False,
        ).stdout.strip()
    except OSError:
        revision_date = ""

    revision_date = revision_date or "unknown"

    version_full = f"{build_number}{git_dirty_flag}({git_revision})"  # keep/extend as needed, e.g. f"{version_string}+{build_number}"

    return {
        "BUILD_NUMBER": build_number,
        "GIT_REVISION": git_revision,
        "GIT_BRANCH": git_branch,
        "GIT_DIRTY_FLAG": git_dirty_flag,
        "LOCAL_CHANGES": local_changes,
        "REVISION_DATE": revision_date,
        "VERSION_FULL": version_full,
    }

# Read root VERSION file at ../../../VERSION
version_file = Path(__file__).parent.parent.parent / "VERSION"
version = "unknown"
if version_file.exists():
    with open(version_file, "r") as f:
        version = 'v' + f.read().strip()

meta = generate_version_metadata(os.path.dirname(__file__))
release = version + '-' + meta["VERSION_FULL"]

# ---------------------------------------------------------------------------
# Fix: sphinxcontrib-qthelp hardcodes "doc" as the virtualFolder in .qhp/.qhcp.
# Subclass the builder to rewrite it to "manual" after generation.
# ---------------------------------------------------------------------------
class UltraScanQtHelpBuilder(QtHelpBuilder):
    name = 'qthelp'

    def build_qhp(self, outdir, outname):
        super().build_qhp(outdir, outname)
        for suffix in ('.qhp', '.qhcp'):
            qthelp_file = Path(outdir) / f'{outname}{suffix}'
            if qthelp_file.exists():
                qthelp_file.write_text(
                    re.sub(r'\bdoc\b', 'manual', qthelp_file.read_text(encoding='utf-8'), count=2),
                    encoding='utf-8'
                )


def setup(app):
    app.add_builder(UltraScanQtHelpBuilder, override=True)
    app.add_config_value('qthelp_css_files', [], 'html', types=frozenset({list, tuple}))

# -- Project information -----------------------------------------------------

project = 'UltraScan III'
author = (
    "Borries Demeler, Emre Brookes, Lukas Dobler, "
    "Alexey Savelyev, Saeed Mortezazadeh, Gary Gorbet, "
    "and Haben Gabir"
)

copyright_str = f"{datetime.datetime.now(tz=datetime.timezone.utc).year}, AUC Solutions LLC"


# -- General configuration ---------------------------------------------------

# If your documentation needs a minimal Sphinx version, state it here.
#
# needs_sphinx = '1.0'

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    'sphinx.ext.autodoc',
    'sphinx.ext.coverage',
    'sphinx.ext.mathjax',
    'sphinx.ext.autosectionlabel',
    'sphinxcontrib.qthelp',
    'sphinx.ext.graphviz',
    'sphinx_design',
    'auto_index'
]

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# The suffix(es) of source filenames.
# You can specify multiple suffix as a list of string:
#
# source_suffix = ['.rst', '.md']
source_suffix = '.rst'

# The master toctree document.
master_doc = 'index'

# The language for content autogenerated by Sphinx. Refer to documentation
# for a list of supported languages.
#
# This is also used if you do content translation via gettext catalogs.
# Usually you set "language" from the command line for these cases.
language = 'en'

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path .
exclude_patterns = []

# The name of the Pygments (syntax highlighting) style to use.
pygments_style = 'sphinx'


# -- Options for HTML output -------------------------------------------------
# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'haiku'

html_title = f"{project} Documentation {release}"

# Theme options are theme-specific and customize the look and feel of a theme
# further.  For a list of options available for each theme, see the
# documentation.
#
# html_theme_options = {}

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']

html_css_files = [
    'custom.css',
]
qthelp_css_files = [
    ('custom.css', {}),
    ('qthelp.css', {})
]
# def setup(app):

#   app.add_stylesheet('.custom.css')

# Custom sidebar templates, must be a dictionary that maps document names
# to template names.
#
# The default sidebars (for documents that don't match any pattern) are
# defined by theme itself.  Builtin themes are using these templates by
# default: ``['localtoc.html', 'relations.html', 'sourcelink.html',
# 'searchbox.html']``.
#
# html_sidebars = {}


# -- Options for HTMLHelp output ---------------------------------------------

# Output file base name for HTML help builder.
htmlhelp_basename = 'UltraScanIIIdoc'

# Namespace used by
qthelp_basename = 'manual'
qthelp_namespace = f'org.sphinx.{qthelp_basename}.{release}'
# -- Options for LaTeX output ------------------------------------------------
# Use XeLaTeX for proper Unicode support
latex_engine = "xelatex"
latex_use_xindy = False
latex_elements = {
    # Paper size for the generated PDF
    'papersize': 'letterpaper',

    # Font size
    'pointsize': '11pt',

    # Improve figure placement for technical documentation
    'figure_align': 'htbp',

    # LaTeX preamble additions
    'preamble': r"""
\usepackage{graphicx}
\setlength{\headheight}{13.6pt}
""",
    # Overwrite fontpkg to prevent FreeSerif fonts from not being found by xelatex
    # https://github.com/sphinx-doc/sphinx/issues/10347#issuecomment-1647984313
    'fontpkg': r'''
\setmainfont{FreeSerif}[
  UprightFont    = *,
  ItalicFont     = *Italic,
  BoldFont       = *Bold,
  BoldItalicFont = *BoldItalic
]
\setsansfont{FreeSans}[
  UprightFont    = *,
  ItalicFont     = *Oblique,
  BoldFont       = *Bold,
  BoldItalicFont = *BoldOblique,
]
\setmonofont{FreeMono}[
  UprightFont    = *,
  ItalicFont     = *Oblique,
  BoldFont       = *Bold,
  BoldItalicFont = *BoldOblique,
]
''',
}

# Grouping the document tree into LaTeX files. List of tuples
# (source start file, target name, title,
#  author, documentclass [howto, manual, or own class]).
latex_documents = [
    (master_doc, 'UltraScanIII.tex', f"{project} Documentation",
     'Borries Demeler, Haben Gabir', 'manual'),
]


# -- Options for manual page output ------------------------------------------

# One entry per manual page. List of tuples
# (source start file, name, description, authors, manual section).
man_pages = [
    (master_doc, project, f"{project} Documentation",
     [author], 1)
]


# -- Options for Texinfo output ----------------------------------------------

# Grouping the document tree into Texinfo files. List of tuples
# (source start file, target name, title, author,
#  dir menu entry, description, category)
texinfo_documents = [
    (master_doc, project, f"{project} Documentation",
     author, project, 'Comprehensive data analysis software for hydrodynamic data from analytical ultracentrifugation experiments.',
     'Science',
     'Miscellaneous'),
]


# -- Options for Epub output -------------------------------------------------

# Bibliographic Dublin Core info.
epub_title = project
epub_author = author
epub_publisher = author
epub_copyright = copyright_str

# The unique identifier of the text. This can be a ISBN number
# or the project homepage.
#
# epub_identifier = ''

# A unique identification for the text.
#
# epub_uid = ''

# A list of files that should not be packed into the epub file.
epub_exclude_files = ['search.html']


# -- Extension configuration -------------------------------------------------

autosectionlabel_prefix_document = True


# -- Options for intersphinx extension ---------------------------------------

# Example configuration for intersphinx: refer to the Python standard library.

intersphinx_mapping = {
    'python': ('https://docs.python.org/3/', None),
    'sphinx': ('https://www.sphinx-doc.org/en/master/', None),
}


# -- Options for todo extension ----------------------------------------------

# If true, `todo` and `todoList` produce output, else they produce nothing.
todo_include_todos = True
