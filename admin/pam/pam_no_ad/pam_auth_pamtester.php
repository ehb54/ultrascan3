#!/usr/bin/env php
<?php

function prompt($prompt) {
    echo $prompt;
    return rtrim(fgets(STDIN), "\n");
}

function read_password($prompt) {
    echo $prompt;
    system('stty -echo');
    $password = rtrim(fgets(STDIN), "\n");
    system('stty echo');
    echo "\n";
    return $password;
}

function run_pamtester($service, $username, $password, $action) {
    $start = microtime(true);
    $cmd = sprintf(
        "echo %s | pamtester %s %s %s 2>&1",
        escapeshellarg($password),
        escapeshellarg($service),
        escapeshellarg($username),
        escapeshellarg($action)
    );
    exec($cmd, $output, $status);
    $end = microtime(true);
    $elapsed_ms = round(($end - $start) * 1000, 2);

    return [$status, implode("\n", $output), $elapsed_ms];
}

function log_line($fh, $line) {
    if ($fh) {
        fwrite($fh, $line . "\n");
    }
}

// Check pamtester availability
if (!shell_exec('which pamtester')) {
    fwrite(STDERR, "❌ Error: pamtester is not installed or not in PATH.\n");
    exit(1);
}

$services = ['php', 'mariadb'];
$actions = [
    'authenticate'   => 'Authentication',
    'acct_mgmt'      => 'Account Management',
    'setcred'        => 'Establish Credentials',
    'open_session'   => 'Open Session',
    'close_session'  => 'Close Session',
    'reinitcred'     => 'Reinitialize Credentials',
    'refreshcred'    => 'Refresh Credentials',
    'chauthtok'      => 'Change Authentication Token'
];

// Optional logging
$logfile = 'pam_test.log';
$logfh = fopen($logfile, 'w');
if ($logfh) {
    fwrite($logfh, "PAM Tester Log - " . date('Y-m-d H:i:s') . "\n\n");
}

echo "🔐 PAM Multi-Action Test via pamtester\n";
echo "======================================\n";

$username = prompt("Username: ");
$password = read_password("Password: ");

foreach ($services as $service) {
    echo "\n🧪 Testing service: $service\n";
    log_line($logfh, "\nService: $service");

    foreach ($actions as $action => $desc) {
        [$status, $output, $elapsed] = run_pamtester($service, $username, $password, $action);
        $status_str = $status === 0 ? "✅ Success" : "❌ Failed";
        $line = sprintf(" - %-30s (%s): %-10s [%5.1f ms]", $desc, $action, $status_str, $elapsed);
        echo $line . "\n";
        log_line($logfh, $line);
    }
}

if ($logfh) {
    fclose($logfh);
    echo "\n📄 Results saved to: $logfile\n";
}
