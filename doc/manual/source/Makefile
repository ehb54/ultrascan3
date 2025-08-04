#Makefile for Sphinx documentation

SPHINXBUILD = sphinx-build
SOURCEDIR   = .
BUILDDIR    = ../../../build/doc/manual
INSTALLDIR  = ../../../bin
PROJECT    	= manual

.PHONY: help html qthelp qch qhc qthelpinstall latex pdf

help:
	@echo "Sphinx Makefile targets:"
	@echo "  make html             to build HTML documentation"
	@echo "  make qthelp           to build Qt Help (.qhp/.qhcp)"
	@echo "  make qch              to build Qt Compressed Help (.qch)"
	@echo "  make qhc              to build Qt Help Collection (.qhc)"
	@echo "  make qthelpinstall    to copy .qch and .qhc to bin"
	@echo "  make latex            to generate LaTeX source files"
	@echo "  make pdf              to generate PDF (requires LaTeX tools)"
	@echo "  make help             to show this message"

html:
	$(SPHINXBUILD) -b html $(SOURCEDIR) $(BUILDDIR)/html

qthelp:
	$(SPHINXBUILD) -b qthelp $(SOURCEDIR) $(BUILDDIR)/qthelp

qch: qthelp
	qhelpgenerator $(BUILDDIR)/qthelp/$(PROJECT).qhp -o $(BUILDDIR)/qthelp/$(PROJECT).qch

qhc: qthelp
	qhelpgenerator $(BUILDDIR)/qthelp/$(PROJECT).qhcp -o $(BUILDDIR)/qthelp/$(PROJECT).qhc

qthelpinstall: qch qhc
	@mkdir -p $(INSTALLDIR)
	@cp -v $(BUILDDIR)/qthelp/$(PROJECT).qch $(INSTALLDIR)/
	@cp -v $(BUILDDIR)/qthelp/$(PROJECT).qhc $(INSTALLDIR)/

latex:
	$(SPHINXBUILD) -b latex $(SOURCEDIR) $(BUILDDIR)/latex

pdf: latex
	$(MAKE) -C $(BUILDDIR)/latex all-pdf
