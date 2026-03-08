# UltraScan III Documentation

Documentation is written in [reStructuredText](https://www.sphinx-doc.org/en/master/usage/restructuredtext/index.html)
and built with [Sphinx](https://www.sphinx-doc.org/). The build produces two outputs:

- **HTML** — for local browsing and review
- **Qt help files** (`manual.qch` / `manual.qhc`) — for Qt Assistant, installed to `bin/`

## Table of Contents

- [Prerequisites](#prerequisites)
- [Building the Documentation](#building-the-documentation)
- [File Organization](#file-organization)
- [Adding New Pages](#adding-new-pages)
- [Documentation Style Guidelines](#documentation-style-guidelines)
- [Testing with Qt Assistant](#testing-with-qt-assistant)

---

## Prerequisites

Install Sphinx and the required extensions:

```bash
pip3 install -r source/requirements.txt
```

For Qt help output you also need `qhelpgenerator`. Whether it is provided
automatically or requires a manual install depends on your build system and
Qt version:

| Build system | Qt version | `qhelpgenerator` source |
|---|---|---|
| CMake + vcpkg | Qt6 | **Automatic** via `qttools[assistant]` in `vcpkg.json` |
| CMake + vcpkg | Qt5 | Manual system install (see below) |
| Legacy qmake  | Qt6 | Manual system install (see below) |
| Legacy qmake  | Qt5 | Manual system install (see below) |

For all cases requiring a manual install:

```bash
# macOS - Qt6
brew install qt

# macOS - Qt5
brew install qt@5

# Ubuntu / Debian - Qt6
sudo apt install qt6-tools-dev

# Ubuntu / Debian - Qt5
sudo apt install qttools5-dev-tools
```

If `qhelpgenerator` is not found the build will still succeed but Qt help
files (`manual.qch` / `manual.qhc`) will not be produced.

### macOS: making sphinx-build available

On macOS, `pip3 install` places scripts under `~/Library/Python/<version>/bin/`,
which is not on the default PATH. After installing, check which version was used:

```bash
python3 --version
```

Then add the matching bin directory to your PATH. For Python 3.9 (the macOS
system default as of Ventura/Sonoma):

```bash
export PATH="$HOME/Library/Python/3.9/bin:$PATH"
```

Add that line to `~/.zshrc` to make it permanent:

```bash
echo 'export PATH="$HOME/Library/Python/3.9/bin:$PATH"' >> ~/.zshrc
source ~/.zshrc
```

Verify it worked:

```bash
which sphinx-build
```

---

## Building the Documentation

All commands are run from the `ultrascan3/doc/manual` directory.

### Full build (HTML + Qt help files)

```bash
make all
```

This builds HTML output into `build/html/` and compiles `manual.qch` / `manual.qhc`
into `../../bin/`.

### HTML only (no qhelpgenerator required)

```bash
make html
```

Output is written to `build/html/index.html`.

### Qt help files only

```bash
make qch
```

Runs the `qthelp` Sphinx builder first, then compiles with `qhelpgenerator`.
Installed files: `../../bin/manual.qch` and `../../bin/manual.qhc`.

### Clean

```bash
make clean
```

Removes `build/` and the installed `.qch` / `.qhc` files.

### CMake

The documentation target is included in the normal CMake build when `sphinx-build`
is found on your PATH:

```bash
cmake --build <build-dir> --target documentation
```

Pass `-DBUILD_DOCUMENTATION=OFF` to skip it entirely.

---

## File Organization

```
doc/manual/
├── CMakeLists.txt          # CMake documentation target
├── Makefile                # Convenience make targets
├── README.md               # This file
└── source/                 # All documentation source lives here
    ├── conf.py             # Sphinx configuration
    ├── requirements.txt    # Python dependencies
    ├── index.rst           # Root table of contents
    ├── _static/            # CSS, images, and other static assets
    │   └── custom.css
    ├── <topic>.rst         # Top-level pages
    └── <module>/           # Sub-directories for related page groups
        └── index.rst
```

All new content goes under `source/`. The Sphinx build preserves subdirectory
structure, so cross-references between pages use the RST path relative to
`source/` (e.g. `gmp/index`).

---

## Adding New Pages

### 1. Create the RST file

Add a new `.rst` file in `source/` or an appropriate subdirectory:

```rst
.. _my_new_page:

My New Page
===========

Description of the topic.

.. figure:: /_static/images/my_screenshot.png
   :align: center
   :alt: My screenshot

   Caption text here.
```

### 2. Add it to the table of contents

Open `source/index.rst` (or the relevant subdirectory `index.rst`) and add
the new page to the appropriate `.. toctree::` directive:

```rst
.. toctree::
   :maxdepth: 1
   :caption: My Section:

   my_new_page
```

The entry is the filename without the `.rst` extension. For pages in a
subdirectory, include the relative path: `gmp/my_new_page`.

### 3. Rebuild

```bash
make html
```

Open `build/html/index.html` to review, then run `make all` to produce the
Qt help files.

---

## Documentation Style Guidelines

### Images

Use the `.. figure::` directive with `/_static/images/` as the root:

```rst
.. figure:: /_static/images/module/screenshot.png
   :align: center
   :alt: Descriptive alt text

   Optional caption.
```

For side-by-side figures, the `sphinx_subfigure` extension is available — see
its [documentation](https://sphinx-subfigure.readthedocs.io/).

### Cross-references

Link to another page using its label or document path:

```rst
See :doc:`us_edit` for details.

See :ref:`us_edit:Editing Options` for a specific section.
```

### Notes, warnings, and tips

```rst
.. note::

   This is a note.

.. warning::

   This is a warning.

.. tip::

   This is a tip.
```

### Code blocks

```rst
.. code-block:: bash

   make html
```

---

## Testing with Qt Assistant

After running `make all`, launch Qt Assistant with the compiled collection:

```bash
assistant -collectionFile ../../bin/manual.qhc
```

On macOS, if the `assistant` symlink was created in `bin/` by the CMake build:

```bash
../../bin/assistant -collectionFile ../../bin/manual.qhc
```

Check that:
- All table of contents entries are present and link correctly
- Images display at the right size and position
- Search returns relevant results
- No broken cross-references (Sphinx will warn about these at build time)

Sphinx itself reports broken references during the build. Treat any
`WARNING:` lines in the build output as errors to fix before committing.
