# ---------------------------------------------------------------------------
# Sphinx Makefile for UltraScan III documentation
#
# Purpose
# -------
# This Makefile allows the documentation to be built independently of the
# main UltraScan CMake build.  It is primarily intended for:
#
#   • Documentation authors editing the manual
#   • Legacy workflows that do not run the full CMake build
#   • Quickly previewing documentation changes
#
# Documentation Build Pipeline
# ----------------------------
#
#   Sphinx source (.rst / images)
#        ↓
#   sphinx-build -b qthelp
#        ↓
#   manual.qhp / manual.qhcp
#        ↓
#   qhelpgenerator
#        ↓
#   manual.qch / manual.qhc
#
# Where:
#
#   .qhp   = Qt Help Project file (table of contents + HTML references)
#   .qhcp  = Qt Help Collection Project (defines the help collection)
#   .qch   = Compiled help file containing the documentation
#   .qhc   = Qt help collection database used by Qt Assistant
#
# In the full UltraScan build, the generated .qch and .qhc files are copied
# into the application's runtime directory (bin/) and loaded by Qt Assistant
# when the user opens the help system.
#
# Targets
# -------
#
#   make html
#       Build the HTML documentation using Sphinx.
#       Output: build/html/
#
#   make qthelp
#       Build the QtHelp intermediate files (.qhp and .qhcp).
#       Output: build/qthelp/
#
#   make qch
#       Compile the QtHelp project into runtime help files (.qch/.qhc)
#       using qhelpgenerator.
#
#	make latexpdf
#   	Build a printable PDF version of the manual using LaTeX.
#
#   make all
#       Build both HTML documentation and Qt help files.
#
#   make clean
#       Remove the generated documentation build directory.
#
# Typical usage
# -------------
#
#   make html
#       Quickly preview documentation changes in a browser.
#
#   make qch
#       Rebuild the help files used by Qt Assistant.
#
# ---------------------------------------------------------------------------
# Qt Tools
#
# The Qt help files (.qch/.qhc) require the Qt tool `qhelpgenerator`.
#
# By default the Makefile takes qhelpgenerator from the Qt reported by
# $(QMAKE) (default: qmake on PATH; make_all.pro passes the qmake it was
# run with), and uses PATH only when that qmake cannot be queried. Qt
# Assistant is looked up only to print a viewing hint.
#
# You can override the tool locations when invoking make.
#
# Example using Qt tools installed by vcpkg:
#
#   make qch \
#       QHELPGEN=$HOME/vcpkg/installed/arm64-osx/tools/qt6/bin/qhelpgenerator \
#       ASSISTANT=$HOME/vcpkg/installed/arm64-osx/tools/qttools/bin/Assistant.app/Contents/MacOS/Assistant
#
# Note:
#   qhelpgenerator is part of QtBase and typically located in:
#
#       $HOME/vcpkg/installed/<triplet>/tools/qt6/bin/
#
#   Qt Assistant is part of QtTools and located in:
#
#       $HOME/vcpkg/installed/<triplet>/tools/qttools/bin/Assistant.app
#
# On macOS the executable is inside the app bundle:
#
#       Assistant.app/Contents/MacOS/Assistant
#
# ---------------------------------------------------------------------------

SPHINXBUILD  ?= python3 -m sphinx
SOURCEDIR     = source
BUILDDIR      = build
SPHINXOPTS   ?=
SPHINXPROJ    = manual

INSTALL_DIR   = ../../bin
# qhelpgenerator must come from the Qt being built against: an explicit
# QHELPGEN wins, then the selected qmake's tool directories (Qt 6 installs
# it in libexec). PATH is used only when that qmake cannot be queried.
QMAKE ?= qmake
ifeq ($(origin QHELPGEN),undefined)
QT_TOOL_BINS := $(shell $(QMAKE) -query QT_INSTALL_BINS 2>/dev/null)
ifneq ($(QT_TOOL_BINS),)
QHELPGEN := $(firstword $(wildcard \
    $(shell $(QMAKE) -query QT_HOST_LIBEXECS 2>/dev/null)/qhelpgenerator \
    $(shell $(QMAKE) -query QT_INSTALL_LIBEXECS 2>/dev/null)/qhelpgenerator \
    $(shell $(QMAKE) -query QT_HOST_BINS 2>/dev/null)/qhelpgenerator \
    $(QT_TOOL_BINS)/qhelpgenerator))
else
QHELPGEN := $(shell command -v qhelpgenerator 2>/dev/null)
endif
endif

# Locate Qt Assistant if available: PATH, then the active qmake's Qt.
ASSISTANT := $(shell command -v assistant 2>/dev/null || command -v Assistant 2>/dev/null)
ifeq ($(ASSISTANT),)
ASSISTANT := $(firstword $(wildcard \
    $(shell $(QMAKE) -query QT_INSTALL_BINS 2>/dev/null)/assistant \
    $(shell $(QMAKE) -query QT_INSTALL_BINS 2>/dev/null)/Assistant.app/Contents/MacOS/Assistant))
endif

# Locate Latex
LATEXMK ?= $(shell command -v latexmk 2>/dev/null)

.PHONY: help html qthelp qch qch-tools all clean Makefile

all: html qch

help:
	@$(SPHINXBUILD) -M help $(SOURCEDIR) $(BUILDDIR) $(SPHINXOPTS)

html: 
	@$(SPHINXBUILD) -b html $(SOURCEDIR) $(BUILDDIR)/html $(SPHINXOPTS)
	@echo "HTML documentation written to $(BUILDDIR)/html"

# Sphinx's stdout goes through a log so its exit status is kept; a failed
# build must stop qch from packaging older generated files.
qthelp:
	@mkdir -p $(BUILDDIR)
	@$(SPHINXBUILD) -b qthelp $(SOURCEDIR) $(BUILDDIR)/qthelp $(SPHINXOPTS) > $(BUILDDIR)/qthelp.log; \
	status=$$?; \
	sed '/You can now run "qcollectiongenerator"/,/assistant -collectionFile/d' $(BUILDDIR)/qthelp.log; \
	exit $$status
	@echo "Qt help project files written to $(BUILDDIR)/qthelp"

# Succeeds only when Sphinx and qhelpgenerator are both available, so an
# optional caller (make_all.pro) can skip the manual with a warning.
qch-tools:
	@$(SPHINXBUILD) --version >/dev/null 2>&1 || \
		{ echo "Sphinx is not available to $(SPHINXBUILD)."; exit 1; }
	@if [ -z "$(QHELPGEN)" ]; then \
		echo "qhelpgenerator was not found in $(QMAKE)'s Qt (or on PATH if $(QMAKE) is unusable); set QHELPGEN to override."; \
		exit 1; \
	fi

qch: qthelp
	@if [ -z "$(QHELPGEN)" ]; then \
		echo "Qt help files require qhelpgenerator."; \
		echo "Install Qt tools for $(QMAKE)'s Qt or set QHELPGEN."; \
		exit 1; \
	fi

	@mkdir -p $(INSTALL_DIR)
	(cd $(BUILDDIR)/qthelp && $(QHELPGEN) $(SPHINXPROJ).qhp  -o $(SPHINXPROJ).qch)
	(cd $(BUILDDIR)/qthelp && $(QHELPGEN) -c $(SPHINXPROJ).qhcp -o $(SPHINXPROJ).qhc)
	cp $(BUILDDIR)/qthelp/$(SPHINXPROJ).qch $(INSTALL_DIR)
	cp $(BUILDDIR)/qthelp/$(SPHINXPROJ).qhc $(INSTALL_DIR)
	@echo ""
	@echo "Qt help source written to $(BUILDDIR)/qthelp"
	@echo ""

	@if [ -n "$(ASSISTANT)" ]; then \
        echo "View the help manual with:"; \
        echo "  $(ASSISTANT) -collectionFile $(INSTALL_DIR)/$(SPHINXPROJ).qhc"; \
    else \
        echo "To view the help file run Qt Assistant:"; \
        echo "  assistant -collectionFile $(INSTALL_DIR)/$(SPHINXPROJ).qhc"; \
        echo "(Qt Assistant ships with Qt's tools; it was not found on PATH or in qmake's Qt)"; \
    fi

latexpdf:
	@if [ -z "$(LATEXMK)" ]; then \
		echo ""; \
		echo "Error: LaTeX toolchain not found."; \
		echo "Install a TeX distribution that provides 'latexmk'."; \
		echo ""; \
		echo "Common options:"; \
		echo "  macOS  : MacTeX or BasicTeX"; \
		echo "  Linux  : texlive-latex-extra"; \
		echo "  Windows: MiKTeX or TeX Live"; \
		echo ""; \
		exit 1; \
	fi
	@$(SPHINXBUILD) -b latex $(SOURCEDIR) $(BUILDDIR)/latex $(SPHINXOPTS)
	@$(MAKE) -C $(BUILDDIR)/latex
	@cp $(BUILDDIR)/latex/UltraScanIII.pdf $(SOURCEDIR)/_static/UltraScanIII.pdf
	@echo "PDF manual written to $(BUILDDIR)/latex"

clean:
	@rm -rf $(BUILDDIR)
	@rm -f $(INSTALL_DIR)/manual.qch $(INSTALL_DIR)/manual.qhc
	@rm -f $(SOURCEDIR)/_static/UltraScanIII.pdf
	@echo "Cleaned build directory and installed files"

# Catch-all: forward anything else to sphinx-build
%: Makefile
	@$(SPHINXBUILD) -M $@ $(SOURCEDIR) $(BUILDDIR) $(SPHINXOPTS)
