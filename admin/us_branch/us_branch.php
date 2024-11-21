#!/usr/bin/php

<?php

{};

# user defines

$modulename         = "ultrascan/home-gui-build";
$repo_url           = "https://github.com/ehb54/ultrascan3";

# developer defines
$logging_level = 2;
$debug = 1;

# end of developer defines

$self = __FILE__;
    
$notes = <<<__EOD
usage: $self {options}

utility checkout, build and run ultrascan3 from a specified branch
leverages module $modulename

Options

--help                : print this information and exit

--branch branchname   : checkout branch
--pullrequest id      : checkout from pull request
--build               : build 
--run                 : run


__EOD;

$scriptdir = dirname(__FILE__);

require "$scriptdir/utility.php";

## check if module file exists

{
    $cmd = "module is-avail $modulename";
    run_cmd( $cmd, false );
    if ( $run_cmd_last_error ) {
        error_exit( "Please ask your system administrators to verify\nthat environment module '$modulename' is available\n" );
    }
}


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
      default:
        error_exit( "\nUnknown option '$u_argv[0]'\n\n$notes" );
    }        
}

if ( $branch && $pullrequest ) {
    error_exit( "--branch & --pullrequest are mutually exclusive" );
}

if ( count( $u_argv ) ) {
    echo $notes;
    exit;
}

$nprocs  = trim( run_cmd( "grep -c processor /proc/cpuinfo" ) );

## is a clone needed

{
    $cmd          = "module swap $modulename && echo \$ULTRASCAN";
    $uspath       = trim( run_cmd( $cmd ) );
    $uspathparent = preg_replace( '/\/ultrascan3$/', "", $uspath );

    echo "uspath       is '$uspath'\n";
    echo "uspathparent is '$uspathparent'\n";

    if ( !is_dir( $uspath ) ) {
        echo "cloning ultrascan into $uspath\n";
        $cmd = "cd $uspathparent && git clone $repo_url";
        run_cmd( $cmd );

        # setup local.pri
        $qwtpath = trim( run_cmd( "module swap $modulename && echo \$QWTDIR" ) );
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
__EOD;
        if ( file_put_contents( "$uspath/local.pri", $local_pri_template ) === FALSE ) {
            error_exit( "Error creating file '$uspath/local.pri'" );
        }
    } else {
        # check if directory is a git repo
        echo "verifying $uspath is a git repo\n";
        $cmd = "cd $uspath && git status";
        run_cmd( $cmd );
        echo "verified $uspath is a git repo\n";

        # check for local changes
        echo "checking $uspath for uncommitted local changes\n";
        $cmd = "cd $uspath && git diff --name-only && git diff --staged --name-only";
        $res = trim( run_cmd( $cmd ) );
        if ( strlen( $res ) ) {
            error_exit( "Uncommitted changes are present in $uspath, please correct this before proceeding\n" );
        }
        echo "no local changes detected in $uspath\n";

    }
}

## ok, now do we have a branch or pullreq info

if ( $pull ) {
    # pull latest

    echo "pulling latest code from $repo_url\n";
    $cmd = "cd $uspath && git pull";
    run_cmd( $cmd );
    echo "latest code $repo_url pulled\n";
}

if ( strlen( $branch ) ) {
    echo "checking out branch $branch\n";
    $cmd = "cd $uspath && git checkout $branch && git pull";
    print run_cmd( $cmd );
    echo "branch $branch checked out\n";
}

if ( strlen( $pullrequest ) ) {
    $pullrequest = preg_replace( '/^#/', '', $pullrequest );
    $tmpbranch = getenv('USER') . sprintf('-tmp-%04X%04X-%04X-%04X-%04X-%04X%04X%04X', mt_rand(0, 65535), mt_rand(0, 65535), mt_rand(0, 65535), mt_rand(16384, 20479), mt_rand(32768, 49151), mt_rand(0, 65535), mt_rand(0, 65535), mt_rand(0, 65535));
    echo "checking out pull request #$pullrequest as branch $tmpbranch\n";
    $cmd = "cd $uspath && git fetch origin pull/$pullrequest/head:$tmpbranch && git checkout $tmpbranch";
    print run_cmd( $cmd );
    echo "pull request #$pullrequest checked out as branch $tmpbranch\n";
}
    
if ( $build ) {
    echo "building ultrascan, this can take awhile\n";
    $cmd = "module swap $modulename && cd $uspath && qmake && make -j $nprocs && ./makeall.sh -j $nprocs";
    print run_cmd( $cmd );
    echo "building ultrascan complete\n";
}

if ( $run ) {
    echo "running ultrascan3 from $uspath/bin\n";
    $cmd = "module swap $modulename && $uspath/bin/us";
    print run_cmd( $cmd );
}

    
