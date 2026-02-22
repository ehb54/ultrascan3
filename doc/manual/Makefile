# Sphinx Makefile for UltraScan III documentation
#
# Targets:
#   make html       - build HTML output
#   make qthelp     - build qthelp output (.qhp/.qhcp)
#   make qch        - compile qthelp output into .qch/.qhc (requires qhelpgenerator)
#   make all        - html + qch
#   make clean      - remove build directory

SPHINXBUILD  ?= sphinx-build
SOURCEDIR     = source
BUILDDIR      = build
SPHINXOPTS   ?=
SPHINXPROJ    = manual

INSTALL_DIR   = ../../bin
QHELPGEN     ?= qhelpgenerator

.PHONY: help html qthelp qch all clean Makefile

help:
	@$(SPHINXBUILD) -M help $(SOURCEDIR) $(BUILDDIR) $(SPHINXOPTS)

html:
	@$(SPHINXBUILD) -b html $(SOURCEDIR) $(BUILDDIR)/html $(SPHINXOPTS)
	@echo "HTML documentation written to $(BUILDDIR)/html"

qthelp:
	@$(SPHINXBUILD) -b qthelp $(SOURCEDIR) $(BUILDDIR)/qthelp $(SPHINXOPTS)
	@echo "Qt help source written to $(BUILDDIR)/qthelp"

qch: qthelp
	@mkdir -p $(INSTALL_DIR)
	$(QHELPGEN) $(BUILDDIR)/qthelp/$(SPHINXPROJ).qhp  -o $(INSTALL_DIR)/manual.qch
	$(QHELPGEN) $(BUILDDIR)/qthelp/$(SPHINXPROJ).qhcp -o $(INSTALL_DIR)/manual.qhc
	@echo "Installed manual.qch and manual.qhc to $(INSTALL_DIR)"

all: html qch

clean:
	@rm -rf $(BUILDDIR)
	@rm -f $(INSTALL_DIR)/manual.qch $(INSTALL_DIR)/manual.qhc
	@echo "Cleaned build directory and installed files"

# Catch-all: forward anything else to sphinx-build
%: Makefile
	@$(SPHINXBUILD) -M $@ $(SOURCEDIR) $(BUILDDIR) $(SPHINXOPTS)
