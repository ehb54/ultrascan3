# Directory containing the actual Sphinx Makefile
SUBDIR = source

# Where to copy final help artifacts
OUTDIR = ../../bin

# Name of the qthelp_basename from conf.py
PROJECT = manual

.PHONY: all html qthelp qch qhc install clean

all: html qthelp qch qhc install

html:
	@cd $(SUBDIR) && make html

qthelp:
	@cd $(SUBDIR) && make qthelp

qch: qthelp
	@cd $(SUBDIR) && make qch

qhc: qthelp
	@cd $(SUBDIR) && make qhc

install: qthelp qch qhc
	@cd $(SUBDIR) && make qthelpinstall

clean:
	@cd $(SUBDIR) && make clean
	@rm -f $(OUTDIR)/$(PROJECT).qch $(OUTDIR)/$(PROJECT).qhc
