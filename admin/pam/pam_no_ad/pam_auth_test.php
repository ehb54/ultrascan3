#!/usr/bin/env php
<?php

{};

## Interactive PAM authentication test against multiple PAM services
echo "Username: ";
$username = trim(fgets(STDIN));

echo "Password: ";
system('stty -echo');
$password = trim(fgets(STDIN));
system('stty echo');
echo "\n";

## Try authenticating against multiple PAM service definitions
foreach ([ 'php', 'mariadb' ] as $service) {
    if (pam_auth($username, $password, $error, $service)) {
        echo "✅ Authentication successful for user '$username' using service '$service'\n";
    } else {
        echo "❌ Authentication failed for user '$username' using service '$service': $error\n";
    }
}

    
