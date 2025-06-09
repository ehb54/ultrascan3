# PAM Authentication Helper for PHP

This project provides a secure, setuid PAM authentication helper binary callable from PHP (e.g., within an Apache web application) using environment variables. It avoids the limitations of `pam_auth()` in Apache environments and enables robust integration with system authentication mechanisms (e.g., `/etc/shadow`, LDAP, etc.).

## 🔐 Features

- Simple C helper compiled to `/usr/local/bin/pam_auth_helper`
- Uses PAM to authenticate a username/password pair passed via environment variables
- Can be safely invoked from PHP using `proc_open()`
- Isolated PAM stack (`/etc/pam.d/php-helper`) avoids recursive invocation
- Includes test script for CLI verification

---

## 🧰 File Overview

```
pam_auth_helper/
├── Makefile             # Build, install, and clean targets
├── pam_auth_helper.c    # Main helper source file
├── pam_auth_test.php    # PHP test script using proc_open
└── pam.d/
    └── php-helper       # PAM stack used by the helper binary
    ```

---

## ⚙️ Installation

Build and install the helper and PAM config:

```bash
make
sudo make install
```

This will:

- Compile `pam_auth_helper`
- Install it as a setuid root binary at `/usr/local/bin/pam_auth_helper`
- Install the PAM configuration file at `/etc/pam.d/php-helper`

---

## 🧪 Usage

### From PHP

```php
$user = 'testuser';
$pass = 'correctpassword';

$env = [
    'PAM_USER'    => $user,
        'PAM_AUTHTOK' => $pass,
        ];

$descriptorspec = [
    0 => ["pipe", "r"],
        1 => ["pipe", "w"],
            2 => ["pipe", "w"]
            ];

$process = proc_open('/usr/local/bin/pam_auth_helper', $descriptorspec, $pipes, null, $env);

if (is_resource($process)) {
    fclose($pipes[0]);
        $stdout = stream_get_contents($pipes[1]); fclose($pipes[1]);
            $stderr = stream_get_contents($pipes[2]); fclose($pipes[2]);

    $status = proc_close($process);
        echo ($status === 0) ? "✅ Success!\n" : "❌ Failed\n";
        } else {
            echo "❌ Could not start helper\n";
            }
            ```

### From CLI (test mode)

```bash
PAM_USER=testuser PAM_AUTHTOK=correctpassword /usr/local/bin/pam_auth_helper
echo $?
```

Exit status `0` indicates success.

---

## 🔧 PAM Configuration

The helper uses a dedicated PAM stack located at `/etc/pam.d/php-helper`. Example:

```
#%PAM-1.0
auth       sufficient   pam_unix.so
account    sufficient   pam_unix.so
```

This can be extended to use LDAP, 2FA, or other PAM modules as needed.

---

## 🔒 Security Notes

- `pam_auth_helper` must be owned by `root` and have mode `4755`:
  ```bash
    sudo chown root:root /usr/local/bin/pam_auth_helper
      sudo chmod 4755 /usr/local/bin/pam_auth_helper
        ```

- Avoid exposing sensitive environment variables in logs or external tools.

---

## 🧹 Clean Up

To remove installed files:

```bash
sudo make uninstall
```

---
