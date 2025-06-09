<?php
{};

$user = 'XXXX - replace with user name';
$pass = 'XXXX - replace with user password';

$env = [
    'PAM_USER'    => $user,
    'PAM_AUTHTOK' => $pass,
    ];

$descriptorspec = [
    0 => ["pipe", "r"],
    1 => ["pipe", "w"],
    2 => ["pipe", "w"]
    ];

$process = proc_open( '/usr/local/bin/pam_auth_helper', $descriptorspec, $pipes, null, $env );

if (is_resource($process)) {
    fclose($pipes[0]);  // stdin not used
                            $stdout = stream_get_contents($pipes[1]);
    fclose($pipes[1]);
    $stderr = stream_get_contents($pipes[2]);
    fclose($pipes[2]);

    $status = proc_close($process);

    if ($status === 0) {
        echo "✅ Success!\n";
    } else {
        echo "❌ Failed\n";
        if ($stderr) {
            echo "stderr: $stderr\n";
        }
    }
} else {
    echo "❌ Could not start helper\n";
}
