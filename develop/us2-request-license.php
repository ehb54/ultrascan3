<html>
<head>
  <title>UltraScan III License Registration</title>
  </head>
  <body>

<?php
// UltraScan 2 only

$error      = 0;
$reason     = "OK";
$expiration = "";
$maillist   = "y";

// Check all data items have been sent
check_values();
if ( $error != 0 ) handle_error();

$license = strtoupper( generate_license() );

update_db();
if ( $error != 0 ) handle_error();

send_email();
if ( $error != 0 ) handle_error();

finish();
exit();
//////////////////////
function finish()
{
  echo "<p>Thank you for registering your UltraScan copy!</p>\n" .
       "<p>A license key will be sent to you shortly by e-mail to:<br/>\n";

  echo $_POST[ 'email' ] . "</p>\n</body></html>";
}


function handle_error()
{
    global $error;
    global $reason;
 
    echo "<p>There was an error in the current registration:</p>\n" .
         "<p>$reason</p>\n</body></html>";

    exit();
}

function check_values()
{
    global $error;
    global $reason;
    global $maillist;

    $a = array( "lastname", "firstname", "org"         , "address", "city",
                "state"    ,"zip"      , "phone"       , "email"  , "os"  , 
                "platform", "version"  , "affiliation" );

    for ( $i = 0; $i < count( $a ); $i++ )
    {
      $item = $a[ $i ];

      if ( strlen( $_POST[ $item ] ) == 0 )
      {
        $error = 1;
        $reason = "Missing data item.";
        break;
      }
    }

    // Don't require this, but set a default.
    $maillist = ( $_POST[ 'maillist' ] ) ? 'y' : 'n';

    // Check for valid version

    if ( $error == 0 )
    {
      $v = $_POST[ 'version' ];

      if ( $v > 10.0 )
      {
        $error = 2; 
        $reason = "Invalid version.";
      }
    }

    // Check for valid opsys
    if ( $error == 0 )
    {
      $opsys = Array( "linux", "win32", "freebsd", "openbsd", "netbsd", "osx",
                      "irix" , "solaris" );
      $ok    = false;
      
      for ( $i = 0; $i < count( $opsys ); $i++ )
      {
        if ( $opsys[ $i ] == strtolower( $_POST[ 'os' ] ) ) 
        {
          $ok = true;
          break;
        }
      }

      if ( ! $ok )
      {
        $error = 3;
        $reason = "Invalid operating system";
      }
    }

    // Check for valid platform
    if ( $error == 0 )
    {
      $platform = Array( "generic", "sparc", "intel", "mac", "opteron", "sgi" );
      $ok       = false;
      
      for ( $i = 0; $i < count( $platform ); $i++ )
      {
        if ( $platform[ $i ] == strtolower( $_POST[ 'platform' ] ) )
        {
          $ok = true;
        }
      }

      if ( ! $ok )
      {
        $error = 4;
        $reason = "Invalid platform.";
      }
    }
}

function generate_license()
{
    global $expiration;

    $code1 = encode( $_POST[ 'firstname'    ], $_POST[ 'lastname' ] );
    $code2 = encode( $_POST[ 'org'          ], $_POST[ 'address'  ] );
    $code3 = encode( $_POST[ 'city'         ], $_POST[ 'zip'      ] );
    $code4 = encode( $_POST[ 'phone'        ], $_POST[ 'email'    ] );

    date_default_timezone_set( 'America/Chicago' );
    $t = time();

    switch( $_POST[ 'affiliation' ] )
    {
        case 'academic':
        case 'commercial':
            $timestamp = $t + 365 * 86400;
            break;

        default: //case 'trial':
            $timestamp = $t + 30 * 86400;
            break;
    }

    $expiration = date( "D M j Y", $timestamp );
    $item       = $_POST[ 'platform' ] . $_POST[ 'os' ] . $_POST[ 'version' ];
    $code5      = encode( $item , $expiration );

    return $code1 . "-" . $code2 . "-"  . $code3 . "-" . $code4 . "-" . $code5;
}

function encode( $s1, $s2 )
{
    $S1 = strtoupper( $s1 );
    $S2 = strtoupper( $s2 );

    $sum1 = 0;
    for ( $i = 0; $i < strlen( $S1 ); $i++ )
      $sum1 += ord( substr( $S1, $i , 1 ) );

    $sum2 = 0;
    for ( $i = 0; $i < strlen( $S2 ); $i++ )
        $sum2 += ord( substr( $S2, $i , 1 ) );

    $sum = $sum1 . $sum2;

    $x = (int)( abs( sin( $sum ) * 65535 ) );

    return sprintf( "%04x", $x );
}

function update_db()
{
  global $error;
  global $reason;
  global $maillist;
  global $random;
  global $license;
  global $expiration;

  // Update Database
  $db   = "us_register";
  $user = "us_register";
  $pw   = "lookn4a$";
  $host = "bcf.uthscsa.edu";
  
  $connection = mysql_pconnect ( $host, $user, $pw ) or $error = 6;
  $db         = mysql_select_db( $db, $connection  ) or $error = 6;

  $email  = $_POST[ 'email' ];
  $query  = "SELECT count(*) FROM registration WHERE email='$email'";
  $result = mysql_query( $query ) or $error = 6;
  if ( $error == 6 ) $reason = "$query";

  if ( $error == 0 ) 
  {
    // If the person doesn't exist, insert into DB
    list( $count ) = mysql_fetch_array( $result );

    if ( $count == 0 )
    {  
      $query = "INSERT INTO registration ("                                .
               "first_name, last_name, organization, address, city, "      .
               "state, phone, email, mailing_list, zipcode ) "             .
               "VALUES ("                                                  .
               "'${_POST[ 'firstname'   ]}', '${_POST[ 'lastname' ]}', "   .
               "'${_POST[ 'org'         ]}', '${_POST[ 'address'  ]}', "   .
               "'${_POST[ 'city'        ]}', '${_POST[ 'state'    ]}', "   .
               "'${_POST[ 'phone'       ]}', '${_POST[ 'email'    ]}', "   .
               "'$maillist'                , '${_POST[ 'zip'      ]}')"    ;
               
      mysql_query( $query ) or $error = 6;
      if ( $error == 6 ) $reason = $query;
    }
  }

  if ( $error == 0 ) 
  {
    $email = $_POST[ 'email' ];
    $query = "SELECT id FROM registration WHERE email='$email'";
    $result = mysql_query( $query ) or $error = 6;
    if ( $error == 0 )
    {
      // If the license base info is not in the DB, insert it

      list( $id ) = mysql_fetch_array( $result );
    
      $query = "SELECT id FROM license WHERE "                     .
               "owner_id        = $id AND "                        . 
               "license_type    ='${_POST[ 'affiliation' ]}' AND " .
               "platform        ='${_POST[ 'platform'    ]}' AND " .
               "operating_system='${_POST[ 'os'          ]}' AND " .
               "version         ='${_POST[ 'version'     ]}'";

      $result = mysql_query( $query ) or $error = 6;
      if ( $error == 6 ) $reason = "$query";

      if ( $error == 0 )
      {
        if ( mysql_num_Rows( $result ) == 0 )
        {
          $query = "INSERT INTO license ("                           .
                   "owner_id, license_type, platform, operating_system, version," .
                   "license_key, expiration ) "         .
                   "VALUES ("                           .
                   "$id, '${_POST[ 'affiliation' ]}', " .
                        "'${_POST[ 'platform'    ]}', " .
                        "'${_POST[ 'os'          ]}', " .
                        "'${_POST[ 'version'     ]}', " .
                        "'$license', "                  .
                        "'$expiration')";        
    
          mysql_query( $query ) or $error = 6;
          if ( $error == 6 ) $reason = "$query"; 
        }
        else
        {
            // License exists - just update
            list( $licID ) = mysql_fetch_array( $result );

            $query = "UPDATE license SET " .
                     "license_key='$license', "  .
                     "expiration='$expiration' " .
                     "WHERE id=$licID";

            mysql_query( $query ) or $error = 6;
            if ( $error == 6 ) $reason = "$query"; 
        }
      }
    }
  }
}

function send_email()
{
    global $license;
    global $expiration;
    global $error;
    global $reason;

    // Send email
    if ( $error == 0 )
    {
      $file  = tempnam( "/tmp", "us-req" ); 
      //$url   = "http://ultrascan.uthscsa.edu/us-register.php?id=$id";
      $email = $_POST[ 'email' ];

      $message = 
        "To: $email\n"                                                  .
        "Bcc: us@biochem.uthscsa.edu\n"                                 .
        "Subject: UltraScan 2 Registration\n\n"                       .
        "Thank you for registering your copy of UltraScan 2.\n"       .
        "Attached is your UltraScan license.\n\n" .
        "Please save this e-mail as a text file to the computer where you plan to\n".
        "run UltraScan. Then import the file using the registration dialogue of\n".
        "UltraScan by clicking on the \"E-mail Import\" button.\n\n".
        "Thanks,\n\n"                                                   .
        "   The UltraScan Team\n\n"                                     .
        "For more information, please visit:\n"                         .
        "http://www.ultrascan.uthscsa.edu\n\n"                          .
        "_____________________CUT HERE_____________________\n".
        "${_POST[ 'firstname'   ]}\n".
        "${_POST[ 'lastname'    ]}\n".
        "${_POST[ 'org'         ]}\n".
        "${_POST[ 'address'     ]}\n".
        "${_POST[ 'city'        ]}\n".
        "${_POST[ 'state'       ]}\n".
        "${_POST[ 'zip'         ]}\n".
        "${_POST[ 'phone'       ]}\n".
        "${_POST[ 'email'       ]}\n".
        "${_POST[ 'platform'    ]}\n".
        "${_POST[ 'os'          ]}\n".
        "${_POST[ 'version'     ]}\n".
        "${_POST[ 'affiliation' ]}\n".
        "$license\n"                 .
        "$expiration\n"
        ;
      
      $handle = fopen( $file, "w" );
      fwrite( $handle, $message );
      fclose( $handle );

      $mail = "sendmail -f ultrascan@biochem.uthscsa.edu -t < $file ";

      system( $mail, $retval );
      
      if ( $retval != 0 )
      {
        $error = 7;
        $reason = "$mail\n$message";
      } 

      unlink( $file );
    }
}
?>
