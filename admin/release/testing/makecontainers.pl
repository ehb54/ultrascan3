#!/usr/bin/perl

use strict;
use warnings;
use English; # Use descriptive variable names like $CHILD_ERROR

# --- Configuration ---
my $MAX_CONCURRENT_JOBS = 2;

# --- Master List of Commands ---
# Group commands logically by the 'cd' step.
my @jobs = (
    # --- Ubuntu Commands ---
    {
        dir => '/home/exouser/ultrascan3/admin/release/ubuntu24_qt6_src',
        commands => [
            "env BUILDKIT_PROGRESS=plain /usr/bin/docker build -t ehb1/usbuildtest:ubuntu24.04-qt6.8.3-qwt6.3.0 --build-arg image=rockylinux/ubuntu:24.04 --build-arg apt_python_version=python3 --build-arg parallel_compile=60 --build-arg ultrascan_branch=main --build-arg qt_major_version=6.8 --build-arg qt_version=6.8.3 .",
            "env BUILDKIT_PROGRESS=plain /usr/bin/docker build -t ehb1/usbuildtest:ubuntu24.04-qt6.9.3-qwt6.3.0 --build-arg image=rockylinux/ubuntu:24.04 --build-arg apt_python_version=python3 --build-arg parallel_compile=60 --build-arg ultrascan_branch=main --build-arg qt_major_version=6.9 --build-arg qt_version=6.9.3 .",
            "env BUILDKIT_PROGRESS=plain /usr/bin/docker build -t ehb1/usbuildtest:ubuntu24.04-qt6.10.0-qwt6.3.0 --build-arg image=rockylinux/ubuntu:24.04 --build-arg apt_python_version=python3 --build-arg parallel_compile=60 --build-arg ultrascan_branch=main --build-arg qt_major_version=6.10 --build-arg qt_version=6.10.0 .",
        ]
    },
    # --- Ubuntu qt5 Commands ---
    {
        dir => '/home/exouser/ultrascan3/admin/release/ubuntu22_qt5_src',
        commands => [
            "env BUILDKIT_PROGRESS=plain /usr/bin/docker build -t ehb1/usbuildtest:ubuntu22.04-qt5.15.18-qwt6.1.6 --build-arg image=ubuntu:22.04 --build-arg apt_python_version=python3 --build-arg parallel_compile=60 --build-arg ultrascan_branch=main --build-arg qt_major_version=5.15 --build-arg qt_version=5.15.18 --build-arg qwt_version=6.1.6 .",
            "env BUILDKIT_PROGRESS=plain /usr/bin/docker build -t ehb1/usbuildtest:ubuntu22.04-qt5.15.18-qwt6.3.0 --build-arg image=ubuntu:22.04 --build-arg apt_python_version=python3 --build-arg parallel_compile=60 --build-arg ultrascan_branch=main --build-arg qt_major_version=5.15 --build-arg qt_version=5.15.18 --build-arg qwt_version=6.3.0 .",
            "env BUILDKIT_PROGRESS=plain /usr/bin/docker build -t ehb1/usbuildtest:ubuntu22.04-qt5.15.10-qwt6.1.6 --build-arg image=ubuntu:22.04 --build-arg apt_python_version=python3 --build-arg parallel_compile=60 --build-arg ultrascan_branch=main --build-arg qt_major_version=5.15 --build-arg qt_version=5.15.10 --build-arg qwt_version=6.1.6 .",
            "env BUILDKIT_PROGRESS=plain /usr/bin/docker build -t ehb1/usbuildtest:ubuntu22.04-qt5.15.10-qwt6.3.0 --build-arg image=ubuntu:22.04 --build-arg apt_python_version=python3 --build-arg parallel_compile=60 --build-arg ultrascan_branch=main --build-arg qt_major_version=5.15 --build-arg qt_version=5.15.10 --build-arg qwt_version=6.3.0 .",
        ]
    },
    # --- RedHat Qt6 Commands ---
    {
        dir => '/home/exouser/ultrascan3/admin/release/redhat_qt6_src',
        commands => [
            "env BUILDKIT_PROGRESS=plain /usr/bin/docker build -t ehb1/usbuildtest:rh9.6-qt6.8.3-qwt6.3.0 --build-arg image=rockylinux/rockylinux:9.6 --build-arg apt_python_version=python3 --build-arg parallel_compile=60 --build-arg ultrascan_branch=main --build-arg qt_major_version=6.8 --build-arg qt_version=6.8.3 .",
            "env BUILDKIT_PROGRESS=plain /usr/bin/docker build -t ehb1/usbuildtest:rh9.6-qt6.9.3-qwt6.3.0 --build-arg image=rockylinux/rockylinux:9.6 --build-arg apt_python_version=python3 --build-arg parallel_compile=60 --build-arg ultrascan_branch=main --build-arg qt_major_version=6.9 --build-arg qt_version=6.9.3 .",
            "env BUILDKIT_PROGRESS=plain /usr/bin/docker build -t ehb1/usbuildtest:rh9.6-qt6.10.0-qwt6.3.0 --build-arg image=rockylinux/rockylinux:9.6 --build-arg apt_python_version=python3 --build-arg parallel_compile=60 --build-arg ultrascan_branch=main --build-arg qt_major_version=6.10 --build-arg qt_version=6.10.0 .",
        ]
    },
    # --- RedHat8 Qt6 Commands ---
    {
        dir => '/home/exouser/ultrascan3/admin/release/redhat8_qt6_src',
        commands => [
            "env BUILDKIT_PROGRESS=plain /usr/bin/docker build -t ehb1/usbuildtest:rh8.10-qt6.8.3-qwt6.3.0 --build-arg image=rockylinux/rockylinux:8.10 --build-arg apt_python_version=python3 --build-arg parallel_compile=60 --build-arg ultrascan_branch=main --build-arg qt_major_version=6.8 --build-arg qt_version=6.8.3 .",
            "env BUILDKIT_PROGRESS=plain /usr/bin/docker build -t ehb1/usbuildtest:rh8.10-qt6.9.3-qwt6.3.0 --build-arg image=rockylinux/rockylinux:8.10 --build-arg apt_python_version=python3 --build-arg parallel_compile=60 --build-arg ultrascan_branch=main --build-arg qt_major_version=6.9 --build-arg qt_version=6.9.3 .",
            "env BUILDKIT_PROGRESS=plain /usr/bin/docker build -t ehb1/usbuildtest:rh8.10-qt6.10.0-qwt6.3.0 --build-arg image=rockylinux/rockylinux:8.10 --build-arg apt_python_version=python3 --build-arg parallel_compile=60 --build-arg ultrascan_branch=main --build-arg qt_major_version=6.10 --build-arg qt_version=6.10.0 .",
        ]
    },
    # --- RedHat Qt5 Commands (4 jobs in this directory) ---
    {
        dir => '/home/exouser/ultrascan3/admin/release/redhat_qt5_qwt630',
        commands => [
            "env BUILDKIT_PROGRESS=plain /usr/bin/docker build -t ehb1/usbuildtest:rh8.10-qt5.15.18-qwt6.1.6 --build-arg image=rockylinux/rockylinux:8.10 --build-arg apt_python_version=python3 --build-arg parallel_compile=60 --build-arg ultrascan_branch=main --build-arg qt_major_version=5.15 --build-arg qt_version=5.15.18 --build-arg qwt_version=6.1.6 .",
            "env BUILDKIT_PROGRESS=plain /usr/bin/docker build -t ehb1/usbuildtest:rh8.10-qt5.15.18-qwt6.3.0 --build-arg image=rockylinux/rockylinux:8.10 --build-arg apt_python_version=python3 --build-arg parallel_compile=60 --build-arg ultrascan_branch=main --build-arg qt_major_version=5.15 --build-arg qt_version=5.15.18 --build-arg qwt_version=6.3.0 .",
            "env BUILDKIT_PROGRESS=plain /usr/bin/docker build -t ehb1/usbuildtest:rh8.10-qt5.15.10-qwt6.1.6 --build-arg image=rockylinux/rockylinux:8.10 --build-arg apt_python_version=python3 --build-arg parallel_compile=60 --build-arg ultrascan_branch=main --build-arg qt_major_version=5.15 --build-arg qt_version=5.15.10 --build-arg qwt_version=6.1.6 .",
            "env BUILDKIT_PROGRESS=plain /usr/bin/docker build -t ehb1/usbuildtest:rh8.10-qt5.15.10-qwt6.3.0 --build-arg image=rockylinux/rockylinux:8.10 --build-arg apt_python_version=python3 --build-arg parallel_compile=60 --build-arg ultrascan_branch=main --build-arg qt_major_version=5.15 --build-arg qt_version=5.15.10 --build-arg qwt_version=6.3.0 .",
        ]
    },
);

# --- Main Logic ---

foreach my $job_block (@jobs) {
    my $dir = $job_block->{dir};
    my @commands = @{$job_block->{commands}};
    my %pids; # Hash to track active PIDs and their commands for the current block

    print "\n🚀 Switching directory to: **$dir**\n";

    # Execute 'cd' command sequentially and check its status immediately
    unless (chdir $dir) {
        die "ERROR: Failed to change directory to '$dir': $!\n";
    }

    # Queue and manage parallel commands
    while (@commands || keys %pids) {
        # 1. Start new jobs if capacity allows
        while (keys %pids < $MAX_CONCURRENT_JOBS && @commands) {
            my $command = shift @commands;
            my $pid = fork();

            die "ERROR: fork failed: $!\n" unless defined $pid;

            if ($pid == 0) {
                # --- CHILD PROCESS ---
                print "  [Starting] PID $PID: $command\n";
                # Execute the command. The 'exec' call replaces the Perl child process
                # with the docker build process, so no return to this point.
                # Use /bin/sh -c to correctly interpret the shell command string.
                exec '/bin/sh', '-c', $command or die "ERROR: exec failed for PID $PID: $!\n";
            } else {
                # --- PARENT PROCESS ---
                $pids{$pid} = $command;
            }
        }

        # 2. Wait for *any* child process to finish
        if (keys %pids > 0) {
            # WNOHANG makes waitpid non-blocking, but we want it to block until *a* child finishes.
            # Using 0 as the second argument blocks until a child changes state (exits).
            my $finished_pid = waitpid -1, 0;

            if ($finished_pid > 0) {
                my $command_info = delete $pids{$finished_pid};
                my $exit_value = $CHILD_ERROR >> 8; # Get the actual exit code
                my $signal_num = $CHILD_ERROR & 127; # Get the signal number

                if ($exit_value != 0) {
                    # --- FAIL-FAST CHECK ---
                    print "\n❌ **FATAL ERROR**: Build job failed!\n";
                    print "  PID: $finished_pid\n";
                    print "  Command: $command_info\n";
                    print "  Exit Code: $exit_value\n";

                    # Kill all other remaining child processes before exiting
                    foreach my $pid_to_kill (keys %pids) {
                        kill 'KILL', $pid_to_kill;
                        print "  Killing remaining job PID $pid_to_kill...\n";
                    }
                    exit $exit_value;
                } elsif ($signal_num != 0) {
                    # Handle process terminated by a signal (e.g., OOM killer)
                    print "\n❌ **FATAL ERROR**: Build job terminated by signal $signal_num!\n";
                    print "  PID: $finished_pid\n";
                    print "  Command: $command_info\n";

                    foreach my $pid_to_kill (keys %pids) {
                        kill 'KILL', $pid_to_kill;
                    }
                    exit 128 + $signal_num;
                } else {
                    print "  [Finished] PID $finished_pid succeeded.\n";
                }
            }
        }
    }

    print "✅ **Success**: All commands in $dir completed.\n";
}

print "\n🎉 **All build blocks completed successfully!**\n";
exit 0;
