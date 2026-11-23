# Builds the Qt help manual (manual.qch, manual.qhc in bin/) as part of
# make_all.pro by running this directory's hand-written Makefile.
# make_all.pro writes this project's Makefile as Makefile.qmake so that
# the hand-written Makefile is not overwritten.

TEMPLATE = aux

# Sphinx fails with parallel jobs, so the manual always builds with -j1.
# Without Sphinx or qhelpgenerator the manual is skipped with a warning,
# as makeall.sh does.
manual.target   = manual
manual.commands = cd $$shell_quote($$PWD) && \
                  if $(MAKE) -s -f Makefile QMAKE=$$shell_quote($$QMAKE_QMAKE) qch-tools; then \
                     $(MAKE) -j1 -f Makefile QMAKE=$$shell_quote($$QMAKE_QMAKE) qch; \
                  else \
                     echo "WARNING: documentation tools not found - the help manual will not be built."; \
                     echo "  Install them: python3 -m pip install -r doc/manual/source/requirements.txt"; \
                  fi

QMAKE_EXTRA_TARGETS += manual
PRE_TARGETDEPS      += manual
