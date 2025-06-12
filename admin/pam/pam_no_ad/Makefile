PREFIX ?= /etc
SSSD_DIR := $(PREFIX)/sssd
PAM_DIR := $(PREFIX)/pam.d

SRC_DIR := files
PAM_TARGETS := php mariadb
SSSD_CONF := sssd.conf

.PHONY: all install uninstall backup

all: install

backup:
	@echo "Backing up existing PAM and SSSD config files..."
	for target in $(PAM_TARGETS); do \
		[ -f $(PAM_DIR)/$$target ] && cp -n $(PAM_DIR)/$$target $(PAM_DIR)/$$target.bak || true; \
	done
	[ -f $(SSSD_DIR)/sssd.conf ] && cp -n $(SSSD_DIR)/sssd.conf $(SSSD_DIR)/sssd.conf.bak || true
	@echo "Backups created where applicable."

install: backup
	@echo "Installing PAM config for: $(PAM_TARGETS)"
	for target in $(PAM_TARGETS); do \
		install -m 644 $(SRC_DIR)/php $(PAM_DIR)/$$target; \
	done
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
	if [ -f $(SSSD_DIR)/sssd.conf.bak ]; then \
		mv $(SSSD_DIR)/sssd.conf.bak $(SSSD_DIR)/sssd.conf; \
	else \
		rm -f $(SSSD_DIR)/sssd.conf; \
	fi
	@echo "Restarting SSSD..."
	systemctl restart sssd
	@echo "Uninstall complete."
