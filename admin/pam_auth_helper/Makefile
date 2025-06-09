# Makefile for pam_auth_helper and PAM configuration

CC := gcc
CFLAGS := -Wall -Wextra -O2
LDFLAGS := -lpam

TARGET := /usr/local/bin/pam_auth_helper
SRC := pam_auth_helper.c
PAM_SRC := pam.d
PAM_FILE := php-helper
PAM_DEST := /etc/pam.d

.PHONY: all install uninstall clean

all: pam_auth_helper

pam_auth_helper: $(SRC)
	$(CC) $(CFLAGS) -o pam_auth_helper $(SRC) $(LDFLAGS)

install: pam_auth_helper
	install -m 4755 -o root -g root pam_auth_helper $(TARGET)
	@echo "Installing PAM config file..."
	install -m 0644 $(PAM_SRC)/$(PAM_FILE) $(PAM_DEST)/$(PAM_FILE)
	@echo "✅ Install complete."

uninstall:
	@echo "Removing installed files..."
	@rm -f $(TARGET)
	@rm -f $(PAM_DEST)/$(PAM_FILE)
	@echo "🗑️ Uninstall complete."

clean:
	rm -f pam_auth_helper
