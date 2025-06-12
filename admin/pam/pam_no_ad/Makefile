PREFIX ?= /etc
SSSD_DIR := $(PREFIX)/sssd
PAM_DIR := $(PREFIX)/pam.d

SRC_DIR := files
PAM_TARGETS := php mariadb
SSSD_CONF := sssd.conf
NOSSS_FILE := system-auth-nosss
NOSSS_TARGET := system-auth-nosss

.PHONY: all install uninstall backup

all: install

backup:
	@echo "Backing up existing PAM and SSSD config files..."
	for target in $(PAM_TARGETS); do \
		[ -f $(PAM_DIR)/$$target ] && cp -n $(PAM_DIR)/$$target $(PAM_DIR)/$$target.bak || true; \
	done
	[ -f $(PAM_DIR)/$(NOSSS_TARGET) ] && cp -n $(PAM_DIR)/$(NOSSS_TARGET) $(PAM_DIR)/$(NOSSS_TARGET).bak || true
	[ -f $(SSSD_DIR)/$(SSSD_CONF) ] && cp -n $(SSSD_DIR)/$(SSSD_CONF) $(SSSD_DIR)/$(SSSD_CONF).bak || true
	@echo "Backups created where applicable."

install: backup
	@echo "Installing PAM configs for: $(PAM_TARGETS)"
	for target in $(PAM_TARGETS); do \
		install -m 644 $(SRC_DIR)/$$target $(PAM_DIR)/$$target; \
	done
	@echo "Installing system-auth-nosss..."
	install -m 644 $(SRC_DIR)/$(NOSSS_FILE) $(PAM_DIR)/$(NOSSS_TARGET)
	@echo "Installing SSSD config..."
	install -m 600 $(SRC_DIR)/$(SSSD_CONF) $(SSSD_DIR)/$(SSSD_CONF)
	@echo "Restarting SSSD..."
	systemctl restart sssd
	@echo "Installation complete."

uninstall:
	@echo "Restoring or removing PAM and SSSD configs..."
	for target in $(PAM_TARGETS); do \
		if [ -f $(PAM_DIR)/$$target.bak ]; then \
			mv $(PAM_DIR)/$$target.bak $(PAM_DIR)/$$target; \
		else \
			rm -f $(PAM_DIR)/$$target; \
		fi \
	done
	if [ -f $(PAM_DIR)/$(NOSSS_TARGET).bak ]; then \
		mv $(PAM_DIR)/$(NOSSS_TARGET).bak $(PAM_DIR)/$(NOSSS_TARGET); \
	else \
		rm -f $(PAM_DIR)/$(NOSSS_TARGET); \
	fi
	if [ -f $(SSSD_DIR)/$(SSSD_CONF).bak ]; then \
		mv $(SSSD_DIR)/$(SSSD_CONF).bak $(SSSD_DIR)/$(SSSD_CONF); \
	else \
		rm -f $(SSSD_DIR)/$(SSSD_CONF); \
	fi
	@echo "Restarting SSSD..."
	systemctl restart sssd
	@echo "Uninstall complete."
