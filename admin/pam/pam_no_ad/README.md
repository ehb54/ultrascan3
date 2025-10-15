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
│   ├── mariadb           # PAM stack for mariadb
│   ├── php               # PAM stack for PHP
│   ├── sssd.conf         # SSSD configuration for proxying local auth
│   ├── system-auth-nosss # PAM stack used by SSSD to avoid recursion
├── Makefile              # Install/uninstall automation
├── README.md             # This file
```

---

## ⚙️ Requirements

- Linux system with:
  - `sssd` installed and enabled
  - `pam_sss.so` and `pam_unix.so` available
  - Local users in `/etc/passwd` with valid `/etc/shadow` entries
  - `systemd` and root access to manage `sssd`
  - `make` utility for installation

---

## 🚀 Usage

### 🔹 Install

This will:

- Back up:
  - `/etc/pam.d/php`
  - `/etc/pam.d/mariadb`
  - `/etc/pam.d/system-auth-nosss`
  - `/etc/sssd/sssd.conf`
- Install:
  - PAM stack for `php` and `mariadb`
  - A clean, recursion-free `system-auth-nosss` file
  - Updated `sssd.conf` to proxy via `system-auth-nosss`
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

## ✅ Test (basic)

You can test your PAM+SSSD setup using the included `pam_auth_test.php` script. This interactive script allows you to verify authentication using both the `php` and `mariadb` PAM service stacks.

### 🔹 Usage

Make sure the script is executable:

```bash
chmod +x pam_auth_test.php
```

Then run:

```bash
./pam_auth_test.php
```

You will be prompted for a username and password. The script will attempt to authenticate using both PAM services:

```
Username: testuser
Password:
✅ Authentication successful for user 'testuser' using service 'php'
✅ Authentication successful for user 'testuser' using service 'mariadb'
```

### ✅ What to Expect

- You should see success messages if the user exists in `/etc/passwd` and has a valid local password.
- The authentication is performed via `pam_sss.so` and proxied through `sssd`, so no access to `/etc/shadow` is required.
- If authentication fails, the error message from PAM will be printed.

### 🛠 Troubleshooting

- Ensure SSSD is running:  
  ```bash
  sudo systemctl status sssd
  ```
- Check that the user is available to SSSD:  
  ```bash
  getent passwd testuser
  ```
- Check `/var/log/sssd/` for debugging info (e.g., `sssd_pam.log` or `sssd_local.log`)

---
## ✅ Testing PAM Services

This project includes a comprehensive test script, `pam_auth_pamtester.php`, which uses the `pamtester` utility to validate authentication and session-related PAM actions for configured services such as `php` and `mariadb`.

### 🔹 What It Does

The script:

- Prompts for a username and password
- Tests multiple PAM actions (authentication, session handling, credential setup, etc.)
- Displays result and timing for each operation
- Logs the test results to a file (`pam_test.log`) for review or auditing

Supported actions:

- `authenticate` – test password verification  
- `acct_mgmt` – check account validity (expiration, lockout, etc.)  
- `setcred` – establish user credentials  
- `open_session` / `close_session` – simulate login/logout session  
- `reinitcred` / `refreshcred` – simulate long-running session revalidation  
- `chauthtok` – test password change capabilities (if applicable)

### 🔹 Requirements

Ensure `pamtester` is installed:

```bash
# RHEL / CentOS / Oracle Linux
sudo dnf install pamtester

# Debian / Ubuntu
sudo apt install pamtester
```

### 🔹 Running the Script

Make it executable:

```bash
chmod +x pam_auth_pamtester.php
```

Run it interactively:

```bash
./pam_auth_pamtester.php
```

Example output:

```
🔐 PAM Multi-Action Test via pamtester
======================================
Username: testuser
Password: 

🧪 Testing service: php
 - Authentication                 (authenticate): ✅ Success   [ 25.3 ms]
 - Account Management             (acct_mgmt):    ✅ Success   [  3.1 ms]
 - Establish Credentials          (setcred):      ✅ Success   [  2.7 ms]
 - Open Session                   (open_session): ✅ Success   [  3.5 ms]
 - Close Session                  (close_session):✅ Success   [  2.4 ms]
 - Reinitialize Credentials       (reinitcred):   ❌ Failed    [  2.6 ms]
 - Refresh Credentials            (refreshcred):  ❌ Failed    [  2.7 ms]
 - Change Authentication Token    (chauthtok):    ❌ Failed    [  5.0 ms]

📄 Results saved to: pam_test.log
```

### 🔹 Troubleshooting

- Ensure the specified PAM services (`php`, `mariadb`) exist in `/etc/pam.d/`
- Verify that SSSD is running and correctly configured for local authentication:

  ```bash
  sudo systemctl status sssd
  ```

- Check logs in `/var/log/sssd/` for authentication issues
- Some actions (e.g., `chauthtok`) may fail unless explicitly supported or configured in the PAM stack

---

## 📎 Notes

- The SSSD config uses:
  ```
  id_provider = files
  auth_provider = proxy
  proxy_pam_target = system-auth-nosss
  ```
  This delegates password verification to a custom PAM target (system-auth-nosss) that only includes pam_unix.so — avoiding recursion by excluding pam_sss.so.

- The PAM stacks (`php`, `mariadb`) only use `pam_sss.so`, completely avoiding `pam_unix.so` from the application’s perspective.

---

## 🤝 Acknowledgments

Designed for secure, lightweight environments where local auth is preferred and `pam_unix.so` access is restricted.
