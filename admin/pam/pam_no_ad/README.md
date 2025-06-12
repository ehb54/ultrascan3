# PAM no AD/LDAP

This project provides a minimal, secure PAM + SSSD configuration for systems using **local Linux users only** — no LDAP, no Active Directory. It enables authentication via `pam_sss.so` using SSSD's proxy support, avoiding direct access to `/etc/shadow` (which is critical for applications like PHP or MariaDB that cannot or should not run as root).

---

## 🔐 Why Use This?

- Secure PAM-based authentication without requiring root access to `/etc/shadow`
- Ideal for web apps or services (e.g. PHP, MariaDB) running as non-root users
- Uses only local accounts defined in `/etc/passwd`, with password verification proxied via SSSD
- No LDAP or external identity provider required

---

## 📁 Files

```
pam_no_ad/
├── files/
│   ├── php          # PAM stack for PHP
│   ├── sssd.conf    # SSSD configuration for proxying local auth
├── Makefile         # Install/uninstall automation
├── README.md        # This file
```

---

## ⚙️ Requirements

- Linux system with:
  - `sssd` installed and enabled
    - `pam_sss.so` and `pam_unix.so` available
    - Local users in `/etc/passwd` with valid passwords in `/etc/shadow`
    - `systemd` and root access to manage `sssd`

---

## 🚀 Usage

### 🔹 Install

This will:

- Back up existing `/etc/pam.d/php`, `/etc/pam.d/mariadb`, and `/etc/sssd/sssd.conf`
- Install the PAM stack for `php` and `mariadb`
- Replace `/etc/sssd/sssd.conf` with the proxy-enabled version
- Restart the `sssd` service

```bash
make install
```

### 🔹 Uninstall

This will:

- Restore previous backups if available
- Otherwise, remove the installed PAM and SSSD files
- Restart the `sssd` service

```bash
make uninstall
```

### 🔹 Manual Backup Only

```bash
make backup
```

---

## ✅ Test

Try authenticating via your PHP or MariaDB application using a local Linux user. Check that:

- The user is listed in `getent passwd`
- The process runs as a non-root user
- Authentication works without direct access to `/etc/shadow`

---

## 📎 Notes

- The SSSD config uses:
  ```
  id_provider = files
  auth_provider = proxy
  proxy_pam_target = system-auth
  ```
  This delegates password checking to your existing system PAM stack (which includes `pam_unix.so`), all handled internally by the root-owned SSSD daemon.

- The PAM stacks (`php`, `mariadb`) only use `pam_sss.so`, completely avoiding `pam_unix.so` from the application’s perspective.

---

## 🤝 Acknowledgments

Designed for secure, lightweight environments where local auth is preferred and `pam_unix.so` access is restricted.
