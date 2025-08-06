# Directory containing the actual Sphinx Makefile
SUBDIR = source

# Where to copy final help artifacts
OUTDIR = ../../bin

# Name of the qthelp_basename from conf.py
PROJECT = manual

.PHONY: all html qthelp qch qhc install clean

all: html install pdf

help:
	@echo "Sphinx Makefile targets:"
	@echo "  all      - Build HTML, install Qt Help, and generate PDF (default)"
	@echo "  html     - Build HTML documentation"
	@echo "  qthelp   - Build Qt Help source files"
	@echo "  install  - Install Qt Help files using qthelpinstall in bin"
	@echo "  pdf      - Build PDF documentation (requires LaTeX)"
	@echo "  clean    - Remove all build artifacts"
	@echo "  help     - Show this help message"

html:
	@cd $(SUBDIR) && make html

qthelp:
	@cd $(SUBDIR) && make qthelp

install:
	@cd $(SUBDIR) && make qthelpinstall

pdf:
	@cd $(SUBDIR) && make pdf

clean:
	@cd $(SUBDIR) && make clean
