# Build Qt Assistant files for UltraScan

INSTALL_DIR = ../../bin
STAGING_DIR = .staging
BODY_FILES  = $(shell find . -type d -name "$(STAGING_DIR)" -prune -o -type f -name "*.body" -print)

# Main targets
all: $(INSTALL_DIR)/manual.qch $(INSTALL_DIR)/manual.qhc

# Prepare staging directory with subdirectories
prepare_staging:
	@echo "Preparing staging directory..."
	@rm -rf $(STAGING_DIR)
	@mkdir -p $(STAGING_DIR)
	@cp header.us3 footer.us3 mainstyle.css $(STAGING_DIR)/
	@cp manual.qhp manual.qhcp $(STAGING_DIR)/
	@cp -r images $(STAGING_DIR)/
	@cp ultrascan3.txt $(STAGING_DIR)/ 2>/dev/null || true
	@find . -type d -name "$(STAGING_DIR)" -prune -o -type f -name "*.body" -print | xargs -I{} cp {} $(STAGING_DIR)/
	@cp make-html.sh $(STAGING_DIR)/
	@chmod +x $(STAGING_DIR)/make-html.sh

# Process all HTML files at once
process_html: prepare_staging
	@echo "Processing HTML files..."
	@cd $(STAGING_DIR) && \
	for file in *.body; do \
		basename=$${file%.body}; \
		./make-html.sh $$basename "UltraScan Documentation" -f -q; \
	done

# Generate and install Qt help files
$(INSTALL_DIR)/manual.qch $(INSTALL_DIR)/manual.qhc: process_html
	@echo "Generating Qt help files..."
	@cd $(STAGING_DIR) && \
	qhelpgenerator manual.qhp -o manual.qch && \
	qhelpgenerator manual.qhcp -o manual.qhc
	@echo "Installing Qt help files to $(INSTALL_DIR)..."
	@mkdir -p $(INSTALL_DIR)
	@cp $(STAGING_DIR)/manual.qch $(STAGING_DIR)/manual.qhc $(INSTALL_DIR)

# Clean everything
clean:
	rm -rf $(STAGING_DIR)
	rm -f $(INSTALL_DIR)/manual.qch $(INSTALL_DIR)/manual.qhc

.PHONY: all prepare_staging process_html clean