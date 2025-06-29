<?php

# utility

$STDERR = STDERR;

function write_logl( $msg, $this_level = 0 ) {
    global $logging_level;
    global $self;
    if ( $logging_level >= $this_level ) {
        echo "${self}: $msg\n";
    }
}

function timestamp( $msg = "" ) {
    return date( "Y-m-d H:i:s " ) . $msg;
}

function write_logld( $msg, $this_level = 0 ) {
    global $logging_level;
    global $self;
    if ( $logging_level >= $this_level ) {
        echo timestamp() . "${self}: $msg\n";
    }
}

function db_obj_result( $db_handle, $query, $expectedMultiResult = false, $emptyok = false ) {
    $result = mysqli_query( $db_handle, $query );

    if ( !$result || ( is_object( $result ) && !$result->num_rows ) ) {
        if ( $result ) {
            # $result->free_result();
        }
        if ( $emptyok ) {
            return false;
        }
        write_logl( "db query failed : $query\ndb query error: " . mysqli_error($db_handle) . "\n" );
        if ( $result ) {
            debug_json( "query result", $result );
        }
        exit;
    }

    if ( is_object( $result ) && $result->num_rows > 1 && !$expectedMultiResult ) {
        write_logl( "WARNING: db query returned " . $result->num_rows . " rows : $query" );
    }    

    if ( $expectedMultiResult ) {
        return $result;
    } else {
        if ( is_object( $result ) ) {
            return mysqli_fetch_object( $result );
        } else {
            return $result;
        }
    }
}

function debug_json( $msg, $json, $debuglevel = 0 ) {
    global $debug;
    global $STDERR;
    if ( $debuglevel > 0 && $debug < $debuglevel ) {
        return;
    }
    fwrite( $STDERR,  "$msg\n" );
    fwrite( $STDERR, json_encode( $json, JSON_PRETTY_PRINT ) );
    fwrite( $STDERR, "\n" );
}

function echo_json( $msg, $json, $debuglevel = 0 ) {
    echo  "$msg\n";
    echo json_encode( $json, JSON_PRETTY_PRINT );
    echo "\n";
}

function run_cmd( $cmd, $die_if_exit = true, $array_result = false ) {
    global $debug;
    global $run_cmd_last_error;

    if ( isset( $debug ) && $debug ) {
        echo "$cmd\n";
    }
    exec( "$cmd 2>&1", $res, $run_cmd_last_error );
    if ( $die_if_exit && $run_cmd_last_error ) {
        error_exit( "shell command '$cmd' returned result:\n" . implode( "\n", $res ) . "\nand with exit status '$run_cmd_last_error'" );
    }
    if ( !$array_result ) {
        return implode( "\n", $res ) . "\n";
    }
    return $res;
}

function error_exit( $msg ) {
    global $STDERR;
    fwrite( $STDERR, "$msg\nTerminating due to errors.\n" );
    exit(-1);
}

function echoline( $str = "-", $count = 80, $print = true ) {
    $out = "";
    for ( $i = 0; $i < $count; ++$i ) {
       $out .= $str;
    }
    if ( $print ) {
        echo "$out\n";
    }
    return "$out\n";
}

function headerline( $msg, $str = "=", $count = 80, $print = true ) {
    $out =
        echoline( $str, $count, false )
        . "$msg\n"
        . echoline( $str, $count, false )
        ;
    if ( $print ) {
        echo $out;
    }
    return $out;
}

$warnings       = '';
$warnings_count = 0;
function flush_warnings( $msg = NULL ) {
    global $warnings;
    global $warnings_count;
    if ( strlen( $warnings ) ) {
        echo $warnings;
        $warnings_count += count( explode( "\n", trim( $warnings ) ) );
        echoline();
        $warnings = '';
        return true;
    } else {
        if ( $msg ) {
            echo "$msg\n";
        }
        return false;
    }
}

function warnings_summary( $msg = NULL ) {
    global $warnings_count;
    return $warnings_count ? "Warnings generated $warnings_count\n" : "";
}

$errors = '';
function flush_errors_exit() {
    global $errors;
    if ( strlen( $errors ) ) {
        error_exit( $errors );
    }
}

function get_yn_answer( $question, $quit_if_no = false ) {
    global $STDERR;
    echoline( '=' );
    do {
        $answer = readline( "$question (y or n) : " );
    } while ( $answer != "y" && $answer != "n" );
    if ( $quit_if_no && $answer == "n" ) {
        fwrite( $STDERR, "Terminated by user response.\n" );
        exit(-1);
    }
    return $answer == "y";
}

$util_backup_dir = "";

function backup_dir_init( $dir = "backup" ) {
    global $util_backup_dir;
    $util_backup_dir = "$dir-" . trim( run_cmd( 'date +"%Y%m%d%H%M%S"' ) );
    mkdir( $util_backup_dir );
    if ( !is_dir( $util_backup_dir ) ) {
        error_exit( "Could not make backup directory $util_backup_dir" );
    }
}

function backup_file( $filename ) {
    global $util_backup_dir;
    if ( !file_exists( $filename ) ) {
        error_exit( "backup_file : $filename does not exist!" );
    }
    if ( !strlen( $util_backup_dir ) ) {
        backup_dir_init();
    }
    run_cmd( "cp $filename $util_backup_dir" );
    echo "Original $filename backed up in to $util_backup_dir\n";
}

$newfile_dir = "";

function newfile_dir_init( $dir = "newfile" ) {
    global $newfile_dir;
    $newfile_dir = "$dir-" . trim( run_cmd( 'date +"%Y%m%d%H%M%S"' ) );
    mkdir( $newfile_dir );
    if ( !is_dir( $newfile_dir ) ) {
        error_exit( "Could not make newfile directory $newfile_dir" );
    }
    return $newfile_dir;
}

function newfile_file( $filename, $contents ) {
    global $newfile_dir;
    if ( !strlen( $newfile_dir ) ) {
        newfile_dir_init();
    }
    $outfile = "$newfile_dir/$filename";
    if ( false === file_put_contents( $outfile, $contents ) ) {
        error_exit( "Could not write $outfile" );
    }
    echo "CREATED: New file $outfile\n";
    return $outfile;
}

function is_admin( $must_be_root = true, $as_user = "" ) {
    $user = posix_getpwuid(posix_geteuid())['name'];
    if ( strlen( $as_user ) ) {
        return $user == $as_user;
    }
        
    if ( $user == 'root' ) {
        return true;
    }
    if ( $must_be_root ) {
        return false;
    }

    $groupInfo = posix_getgrnam('wheel');
    if ($groupInfo === false) {
        return false;
    }

    return in_array( $user, $groupInfo['members'] );
}

$db_handle = NULL;

function check_db( $sleep_seconds = 30 ) {
    global $db_handle;
        
    while ( !mysqli_ping( $db_handle ) ) {
        write_logld( "mysql server has gone away" );
        sleep( $sleep_seconds );
        write_logl( "attempting to reconnect" );
        open_db();
        if ( mysqli_ping( $db_handle ) ) {
            write_logl( "reconnected - success" );
        }            
    }
}

function open_db() {
    global $db_handle;
    global $dbhost;
    global $user;
    global $passwd;
    $db_handle = mysqli_connect( $dbhost, $user, $passwd );
    if ( !$db_handle ) {
        write_logl( "could not connect to mysql: $dbhost, $user. exiting\n" );
        exit(-1);
    }
}    
    
function existing_dbs() {
    global $db_handle;
    if ( $db_handle === NULL ) {
        open_db();
    }
    $res = db_obj_result( $db_handle, "show databases like 'uslims3_%'", True );
    $existing_dbs = [];
    while( $row = mysqli_fetch_array($res) ) {
        $this_db = (string)$row[0];
        if ( $this_db != "uslims3_global" ) {
            $existing_dbs[] = $this_db;
        }
    }
    return $existing_dbs;
}

function existing_stash_dbs() {
    global $db_handle;
    if ( $db_handle === NULL ) {
        open_db();
    }
    $res = db_obj_result( $db_handle, "show databases like 'stash_%'", true, true );
    $existing_dbs = [];
    if ( $res ) {
        while( $row = mysqli_fetch_array($res) ) {
            $this_db = (string)$row[0];
            $existing_dbs[] = $this_db;
        }
    }
    return $existing_dbs;
}

function boolstr( $val, $truestr = "True", $falsestr = "" ) {
    return $val ? $truestr : $falsestr;
}

function tempdir( $dir = NULL, $prefix = NULL ) {
    $template = "{$prefix}XXXXXX";
    if ( $dir && is_dir($dir) ) {
        $tmpdir = "--tmpdir=$dir";
    } else {
        $tmpdir = '--tmpdir=' . sys_get_temp_dir();
    }
    return exec( "mktemp -d $tmpdir $template" );
}

function file_perms_must_be( $file, $least_restrictive = "600" ) {
    $least_restrictive = octdec( $least_restrictive );
    if ( !file_exists( $file ) ) {
        error_exit( "file permissions check: file '$file' does not exist" );
    }
    $perms = fileperms( $file ) & octdec( "777" );
    $remainder = ( $perms | $least_restrictive ) - $least_restrictive;
    if ( $remainder ) {
        error_exit( sprintf( "Permissions on '$file' are too lenient, fix with:\nchmod %o $file", $least_restrictive ) );
    }
    return;
}

function get_slurm_cores( $cores = 4, $slurmconf = "/etc/slurm/slurm.conf" ) {
    if ( !file_exists( $slurmconf ) ) {
        return $cores;
    }
    $res = run_cmd( 'grep -e "^\s*NodeName\s*=\\s*localhost" ' . $slurmconf, false );
    preg_match( '/Procs=(\d+)\s/', $res, $matches );
    if ( count( $matches ) != 2 ) {
        return $cores;
    }
    
    return intval( $matches[1] * 2 );
}

/**
 * Generates a Universally Unique IDentifier, version 4.
 *
 * RFC 4122 (http://www.ietf.org/rfc/rfc4122.txt) defines a special type of Globally
 * Unique IDentifiers (GUID), as well as several methods for producing them. One
 * such method, described in section 4.4, is based on truly random or pseudo-random
 * number generators, and is therefore implementable in a language like PHP.
 *
 * We choose to produce pseudo-random numbers with the Mersenne Twister, and to always
 * limit single generated numbers to 16 bits (ie. the decimal value 65535). That is
 * because, even on 32-bit systems, PHP's RAND_MAX will often be the maximum *signed*
 * value, with only the equivalent of 31 significant bits. Producing two 16-bit random
 * numbers to make up a 32-bit one is less efficient, but guarantees that all 32 bits
 * are random.
 *
 * The algorithm for version 4 UUIDs (ie. those based on random number generators)
 * states that all 128 bits separated into the various fields (32 bits, 16 bits, 16 bits,
 * 8 bits and 8 bits, 48 bits) should be random, except : (a) the version number should
 * be the last 4 bits in the 3rd field, and (b) bits 6 and 7 of the 4th field should
 * be 01. We try to conform to that definition as efficiently as possible, generating
 * smaller values where possible, and minimizing the number of base conversions.
 *
 * @copyright   Copyright (c) CFD Labs, 2006. This function may be used freely for
 *              any purpose ; it is distributed without any form of warranty whatsoever.
 * @author      David Holmes <dholmes@cfdsoftware.net>
 *
 * @return  string  A UUID, made up of 32 hex digits and 4 hyphens.
 */

function uuid() {
   
    // The field names refer to RFC 4122 section 4.1.2

    return sprintf('%04x%04x-%04x-%03x4-%04x-%04x%04x%04x',
        mt_rand(0, 65535), mt_rand(0, 65535), // 32 bits for "time_low"
        mt_rand(0, 65535), // 16 bits for "time_mid"
        mt_rand(0, 4095),  // 12 bits before the 0100 of (version) 4 for "time_hi_and_version"
        bindec(substr_replace(sprintf('%016b', mt_rand(0, 65535)), '01', 6, 2)),
            // 8 bits, the last two of which (positions 6 and 7) are 01, for "clk_seq_hi_res"
            // (hence, the 2nd hex digit after the 3rd hyphen can only be 1, 5, 9 or d)
            // 8 bits for "clk_seq_low"
        mt_rand(0, 65535), mt_rand(0, 65535), mt_rand(0, 65535) // 48 bits for "node" 
    ); 
}

function is_locked( $php ) {
    global $lock_dir;
    if ( !isset( $lock_dir ) ) {
        error_exit( "is_locked(): \$lock_dir is not set" );
    }
    $lock_file        = "$lock_dir/" . basename( $php ) . ".lock";
    $expected_cmdline = basename( $php );
    $isstale = false;

    if ( !file_exists($lock_file) ) {
        # echo "file $lock_file does not exist\n";
        return false;
    }

    if ( is_link($lock_file) ) {
        # echo "is_link(" . $lock_file . ") true\n";
        if ( ( $link = readlink( $lock_file ) ) === FALSE ) {
            $isstale = true;
            # echo "is stale 1\n";
        }
    } else {
        $isstale = true;
        # echo "is stale 2\n";
    }
    #    echo "tryLock() 2\n";

    if ( !$isstale && is_dir( $link ) ) {
        # make sure the cmdline exists & matches expected
        $cmdline_file = $link . "/cmdline";
        echo "cmdline_file = $cmdline_file\n";
        if ( ($cmdline = file_get_contents( $cmdline_file )) === FALSE ) {
            # echo "could not get contents of $cmdline_file\n";
            $isstale = true;
            # echo "is stale 3\n";
        } else {
            # remove nulls
            $cmdline = str_replace("\0", "", $cmdline);
            if ( strpos( $cmdline, $expected_cmdline ) === false ) {
                # echo "unexpected contents of $cmdline_file\n";
                $isstale = true;
                # echo "is stale 4 \n";
            }
        }
    }
    #    echo "tryLock() 3\n";

    if (is_link($lock_file) && !is_dir($lock_file)) {
        $isstale = true;
    }
    
    return !$isstale;
}

$dt_store_array = [];

function dt_duration_minutes ( $datetime_start, $datetime_end ) {
    return ($datetime_end->getTimestamp() - $datetime_start->getTimestamp()) / 60;
}

function dt_now () {
    return new DateTime( "now" );
}

function dt_store_now( $name ) {
    global $dt_store_array;
    $dt_store_array[ $name ] = dt_now();
}

function dt_store_get( $name ) {
    global $dt_store_array;
    if ( !array_key_exists( $name, $dt_store_array ) ) {
        error_exit( "dt_store_get() : \$dt_array does not contain key '$name'" );
    }
    return $dt_store_array[ $name ];
}

function dt_store_get_printable( $name ) {
    $dt = dt_store_get( $name );
    return $dt->format( DATE_ATOM );
}

function dt_store_duration( $name_start, $name_end ) {
    return sprintf( "%.2f", dt_duration_minutes( dt_store_get( $name_start ), dt_store_get( $name_end ) ) );
}

function dhms_from_minutes( $time ) {
    $res = '';
    $days  =  floor( $time / (24 * 60) );
    $time  -= $days * 24 * 60;
    $hours =  floor( $time / 60 );
    $time  -= $hours * 60;

    if ( $days ) {
        return sprintf( "%sd %sh %.2fm", $days, $hours, $time );
    }
    if ( $hours ) {
        return sprintf( "%sh %.2fm", $hours, $time );
    }
    return sprintf( "%.2fm", $time );
}

function backup_rsync_email_headers() {
    global $backup_host;
    global $backup_user;
    global $backup_email_address;

    $headers  = 
        "From: backups $backup_host<$backup_user@$backup_host>\r\n"
        . "Reply-To: $backup_email_address\r\n"
        ;

    return $headers;
}

function backup_rsync_failure( $msg ) {
    global $backup_email_reports;
    global $backup_email_address;
    global $backup_host;
    global $backup_logs;
    global $date;
    if ( !$backup_email_reports ) {
        error_exit( $msg );
    }
    $emaillog = "$backup_logs/summary-ERRORS-$date.txt";
    file_put_contents( $emaillog, $msg );
    if ( !mail( 
               $backup_email_address
               ,"BACKUP ERRORS for $backup_host"
               ,$msg
               ,backup_rsync_email_headers()
              )
       ) {
        error_exit( $msg . "\nemail notification also failed" );
    }
    error_exit( $msg );
}
    
function backup_rsync_run_cmd( $cmd, $die_if_exit = true ) {
    global $debug;
    if ( isset( $debug ) && $debug ) {
        echo "$cmd\n";
    }
    exec( "$cmd 2>&1", $res, $res_code );
    if ( $die_if_exit && $res_code ) {
        backup_rsync_failure( "shell command '$cmd' returned result:\n" . implode( "\n", $res ) . "\nand with exit status '$res_code'" );
    }
    return implode( "\n", $res ) . "\n";
}

function debug_echo ( $s, $debuglevel = 1 ) {
    global $debug;
    if ( !$debug || $debug < $debuglevel ) {
        return;
    }
    echo "$s\n";
}
    
function fix_single_quote( $str, $rplc = "" ) {
    return str_replace( "'", $rplc, $str );
}

## squash an object
## credit https://gist.github.com/woganmay/9a98dda059246bca664c

function squash($array, $prefix = '') {
    $flat = array();
    $sep = ".";
    
    if (!is_array($array)) $array = (array)$array;
    
    foreach($array as $key => $value)
    {
        $_key = ltrim($prefix.$sep.$key, ".");
        
        if (is_array($value) || is_object($value))
        {
            // Iterate this one too
            $flat = array_merge($flat, squash($value, $_key));
        }
        else
        {
            $flat[$_key] = $value;
        }
    }
    
    return $flat;
}
