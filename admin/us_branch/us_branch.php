#!/usr/bin/php
<?php

{};

## =================== user defines ===================
$modulename         = "ultrascan/home-gui-build";
$repo_url           = "https://github.com/ehb54/ultrascan3";
$qmake_configs      = ""; 
## ====================================================

## ================= developer defines ================
$logging_level = 2;
$debug = 1;
## ====================================================

$self = __FILE__;

$notes = <<<__EOD
usage: $self {options}

utility checkout, build and run ultrascan3 from a specified branch

Options

--help                : print this information and exit

--branch branchname   : checkout branch
--pullrequest id      : checkout from pull request
--build               : build 
--run                 : run
--qt6                 : enable qt6
--assume-yes          : non-interactive; assume YES when asked to discard uncommitted changes


__EOD;

$scriptdir = dirname(__FILE__);

require "$scriptdir/utility.php";

$u_argv = $argv;
array_shift( $u_argv ); # first element is program name

if ( !count( $u_argv ) ) {
    echo $notes;
    exit;
}

$branch         = "";
$pullrequest    = "";
$build          = false;
$run            = false;
$pull           = false;   ## reserved for future use
$qt6            = false;
$assume_yes     = false;

while( count( $u_argv ) && substr( $u_argv[ 0 ], 0, 1 ) == "-" ) {
    switch( $arg = $u_argv[ 0 ] ) {
        case "--help": {
            echo $notes;
            exit;
        }
        case "--branch": {
            array_shift( $u_argv );
            if ( !count( $u_argv ) ) {
                error_exit( "ERROR: option '$arg' requires an argument\n$notes" );
            }
            if ( strlen( $branch ) ) {
                error_exit( "ERROR: option '$arg' can only be specified once" );
            }
            $branch = array_shift( $u_argv );
            break;
        }
        case "--pullrequest": {
            array_shift( $u_argv );
            if ( !count( $u_argv ) ) {
                error_exit( "ERROR: option '$arg' requires an argument\n$notes" );
            }
            if ( strlen( $pullrequest ) ) {
                error_exit( "ERROR: option '$arg' can only be specified once" );
            }
            $pullrequest = array_shift( $u_argv );
            break;
        }
        case "--build": {
            array_shift( $u_argv );
            $build = true;
            break;
        }
        case "--run": {
            array_shift( $u_argv );
            $run = true;
            break;
        }
        case "--qt6": {
            array_shift( $u_argv );
            $qt6 = true;
            $modulename = "ultrascan/home-gui-qt6-build";
            $qmake_configs .= "'CONFIG+=c++17' ";
            $cmd = "module is-avail $modulename";
            run_cmd( $cmd, false );
            if ( $run_cmd_last_error ) {
                error_exit( "Please ask your system administrators to verify\nthat environment module '$modulename' is available\n" );
            }
            break;
        }
        case "--assume-yes": {
            array_shift( $u_argv );
            $assume_yes = true;
            break;
        }
      default:
        error_exit( "\nUnknown option '$u_argv[0]'\n\n$notes" );
    }
}

## check if module file exists
{
    $cmd = "module is-avail $modulename";
    run_cmd( $cmd, false );
    if ( $run_cmd_last_error ) {
        error_exit( "Please ask your system administrators to verify\nthat environment module '$modulename' is available\n" );
    }
}

if ( $branch && $pullrequest ) {
    error_exit( "--branch & --pullrequest are mutually exclusive" );
}

if ( count( $u_argv ) ) {
    echo $notes;
    exit;
}

$nprocs = trim( run_cmd( "grep -c processor /proc/cpuinfo" ) );

## ---------- helpers ---------- 

function write_local_pri( $uspath, $modulename, $nprocs ) {

    ## pull QWTDIR from the active module
    $qwtpath = trim( run_cmd( "module swap $modulename && echo \$QWTDIR" ) );
    if ( !strlen($qwtpath) ) {
        error_exit("Unable to determine QWTDIR from module '$modulename'.\n");
    }

    $local_pri_template = <<<__EOD

DEBUGORRELEASE  = release

unix {
  QMAKE_CXXFLAGS_RELEASE = -O3 -march=native
  QMAKE_CFLAGS_RELEASE   = -O3 -march=native
  QMAKE_CXXFLAGS += -Wno-unused-local-typedefs

  MAKEFLAGS   = " -j $nprocs -O3"

  QWTPATH     = $qwtpath
  QWTLIB      = -L../../lib -L../lib -L$qwtpath/lib -lqwtplot3d -lqwt -lGLU
  MYSQLDIR    = /usr/lib64/mysql
  MYSQLPATH   = /usr/include/mysql
  QWT3D       = ../qwtplot3d
  X11LIB      = -L/usr/lib/X11 -lXau -lX11
  INCLUDEPATH += $qwtpath/src
}
CONFIG += c++17
#CONFIG -= warn_on
#CONFIG += warn_off
        
__EOD;

    if ( file_put_contents( "$uspath/local.pri", $local_pri_template ) === FALSE ) {
        error_exit( "Error creating file '$uspath/local.pri'\n" );
    }
    echo "local.pri created at $uspath/local.pri\n";
}

function git_fetch_prune ( $path ) {
    run_cmd( "cd $path && git fetch --all --prune --tags" );
}

function git_upstream_or_empty ( $path ) {
    $res = trim( run_cmd( "cd $path && git rev-parse --abbrev-ref --symbolic-full-name \\@\\{u\\} 2>/dev/null", false ) );
    global $run_cmd_last_error;
    if ( $run_cmd_last_error ) {
        return "";
    }
    return $res;
}

function is_tty_stdin() {
    if ( function_exists('stream_isatty') ) {
        return @stream_isatty(STDIN);
    }
    if ( function_exists('posix_isatty') ) {
        return @posix_isatty(STDIN);
    }
    return true; ## assume interactive if unsure
}

function prompt_confirm_discard( $file_list, $count_total ) {
    $shown = 0;
    if ( strlen($file_list) ) {
        $lines = explode("\n", trim($file_list));
        $max_show = 30;
        echo "The following files are modified/staged or untracked and will be removed/reset:\n";
        foreach ( $lines as $ln ) {
            if ( $ln === '' ) continue;
            if ( $shown >= $max_show ) break;
            echo "  - $ln\n";
            $shown++;
        }
        if ( $count_total > $shown ) {
            echo "  ... and " . ($count_total - $shown) . " more files\n";
        }
    }
    echo "\nType YES to discard ALL uncommitted/staged changes and untracked files: ";
    $answer = trim(fgets(STDIN));
    return $answer === 'YES';
}

## ---------- main flow ---------- 

## is a clone needed
{
    $cmd          = "module swap $modulename && echo \$ULTRASCAN";
    $uspath       = trim( run_cmd( $cmd ) );
    $uspathparent = preg_replace( '/\/[^\/]+$/', '', $uspath );
    $uspathname   = preg_replace( '/^.*\//', '', $uspath );

    echo "uspath       is '$uspath'\n";
    echo "uspathparent is '$uspathparent'\n";
    echo "uspathname   is '$uspathname'\n";

    if ( !is_dir( $uspath ) ) {
        echo "cloning ultrascan into $uspath\n";
        $cmd = "cd $uspathparent && git clone $repo_url $uspathname";
        run_cmd( $cmd );

        # setup local.pri
        write_local_pri( $uspath, $modulename, $nprocs );
    } else {
        # check if directory is a git repo
        echo "verifying $uspath is a git repo\n";
        $cmd = "cd $uspath && git status";
        run_cmd( $cmd );
        echo "verified $uspath is a git repo\n";

        if ( $build || strlen( $pullrequest ) || strlen( $branch ) ) {
            # always fetch/prune so branch/PR checkout is deterministic
            echo "updating remote refs (fetch/prune)\n";
            git_fetch_prune( $uspath );
            echo "remote refs updated\n";
        }
    }
}

## ok, now do we have a branch or pullreq info

if ( strlen( $branch ) ) {
    echo "checking out branch $branch\n";
    $cmd = "cd $uspath && git checkout $branch";
    print run_cmd( $cmd );
    echo "branch $branch checked out\n";
}

if ( strlen( $pullrequest ) ) {
    $pullrequest = preg_replace( '/^#/', '', $pullrequest );
    $tmpbranch = getenv('USER') . sprintf('-tmp-%04X%04X-%04X-%04X-%04X-%04X%04X%04X',
        mt_rand(0, 65535), mt_rand(0, 65535),
        mt_rand(0, 65535), mt_rand(16384, 20479),
        mt_rand(32768, 49151), mt_rand(0, 65535),
        mt_rand(0, 65535), mt_rand(0, 65535)
    );
    echo "checking out pull request #$pullrequest as branch $tmpbranch\n";
    $cmd = "cd $uspath && git fetch origin pull/$pullrequest/head:$tmpbranch && git checkout $tmpbranch";
    print run_cmd( $cmd );
    echo "pull request #$pullrequest checked out as branch $tmpbranch\n";
}

## make worktree clean and at the correct ref (ask strongly before removing changes)
if ( $build || strlen( $pullrequest ) || strlen( $branch ) ) {
    echo "checking for local modifications prior to cleaning\n";
    $modified  = trim( run_cmd( "cd $uspath && ( git diff --name-only; git diff --staged --name-only )" ) );
    $untracked = trim( run_cmd( "cd $uspath && git ls-files --others --exclude-standard" ) );
    $has_changes = strlen($modified) || strlen($untracked);

    if ( $has_changes ) {
        echo "WARNING: uncommitted or untracked changes detected in $uspath.\n";
        $combined = trim( implode("\n", array_filter([$modified, $untracked])) );
        $combined_count = $combined === '' ? 0 : count( preg_split('/\r?\n/', $combined) );

        if ( !$assume_yes ) {
            if ( !is_tty_stdin() ) {
                error_exit("Interactive confirmation required but no TTY detected. Re-run in a terminal or use --assume-yes to proceed.\n");
            }
            $ok = prompt_confirm_discard( $combined, $combined_count );
            if ( !$ok ) {
                error_exit("Aborted by user. No changes were discarded.\n");
            }
        } else {
            echo "--assume-yes provided: proceeding without prompt.\n";
        }
    }

    $target_ref = "";
    if ( strlen( $pullrequest ) ) {
        # after PR fetch+checkout, FETCH_HEAD is the PR tip
        $target_ref = "FETCH_HEAD";
    } else if ( strlen( $branch ) ) {
        $target_ref = "origin/$branch";
    } else {
        $up = git_upstream_or_empty( $uspath );
        if ( strlen( $up ) ) {
            $target_ref = $up;
        } else {
            $target_ref = "HEAD";
        }
    }

    echo "syncing worktree to $target_ref and removing untracked files\n";
    run_cmd( "cd $uspath && git reset --hard $target_ref" );
    run_cmd( "cd $uspath && git clean -xfd" );
    echo "worktree is clean and synchronized\n";

    ## git clean -xfd will remove local.pri; recreate it now
    if ( !file_exists( "$uspath/local.pri" ) ) {
        write_local_pri( $uspath, $modulename, $nprocs );
    }
}

if ( $build ) {
    echo "building ultrascan, this can take awhile\n";
    $cmd = "module swap $modulename && cd $uspath && qmake $qmake_configs && make -j $nprocs && ./makeall.sh -j $nprocs";
    print run_cmd( $cmd );

    if ( !file_exists( "$uspath/local.pri" ) ) {
        write_local_pri( $uspath, $modulename, $nprocs );
    }

    if ( !file_exists( "$uspath/bin/assistant" ) ) {
        $cmd = "module swap $modulename && cp \$QTDIR/bin/assistant $uspath/bin";
        print run_cmd( $cmd );
    }
    echo "building ultrascan complete\n";
}

if ( $run ) {
    echo "running ultrascan3 from $uspath/bin\n";
    $cmd = "module swap $modulename && stdbuf -oL $uspath/bin/us";
    print "command is: $cmd\n";

    $descriptorspec = array(
        0 => array("pipe", "r"),  ## stdin is a pipe
        1 => array("pipe", "w"),  ## stdout is a pipe
        2 => array("pipe", "w")   ## stderr is a pipe
        );

    $pipes = [];
    $process = proc_open($cmd, $descriptorspec, $pipes);

    if (is_resource($process)) {
        ## Make stdout pipe non-blocking
        stream_set_blocking($pipes[1], 0);

        while (!feof($pipes[1])) {
            ## Read the output chunk-by-chunk
            $chunk = fread($pipes[1], 8192);

            if ( $chunk ) {
                ## Process the chunk immediately (e.g., echo to browser or log)
                echo $chunk;

                ## This is crucial: flush PHP and web server output buffers
                if (ob_get_level() > 0) {
                    ob_flush();
                }
                flush();
            }

            ## Short sleep to prevent CPU hogging
            usleep(500000); ## 500 milliseconds
        }

        fclose($pipes[1]);
        fclose($pipes[2]);
        proc_close($process);
    }
}
