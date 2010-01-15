# Build Qt Assistant files for UltraScan


INSTALL_DIR = ../../bin
BODY_Files  = $(wildcard *.body)
HTML_Files  = $(BODY_Files:%.body=%.html)
TARGETS     = $(INSTALL_DIR)/manual.qch $(INSTALL_DIR)/manual.qhc


# images/*.png is a little overkill.  We probably just want to grep
# manual.qhp for .png files

OTHER_Files = ultrascan3.txt images/ultra.xpm $(wildcard images/*.png)$

$(TARGETS): manual.qhp manual.qhcp $(HTML_Files) $(OTHER_Files)
	@echo "Making Qt Assistant files"
	qhelpgenerator manual.qhp -o $(INSTALL_DIR)/manual.qch
	qcollectiongenerator manual.qhcp -o $(INSTALL_DIR)/manual.qhc

%.html: %.body header.us3 footer.us3
	tpage $< > $@


clean:
	rm -f *.html manual.qch manual.qhc
	rm -f $(TARGETS)
	rm -f *~


.PHONY: clean
