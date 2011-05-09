<?php
// UltraScan III only

// If there isn't an id, do nothing
if ( ! isset( $_GET[ 'id' ] ) ) exit;

$id = $_GET[ 'id' ];

$db   = "us3_register";
$user = "us3_register";
$pw   = "out2lunch";
$host = "biochem.uthscsa.edu";

$connection = mysql_pconnect ( $host, $user, $pw ) or die( "Could not connect to server" );
$db         = mysql_select_db( $db, $connection  ) or die( "Could not select database" );

$query  = "SELECT fname, lname, validated FROM people WHERE random=$id";
$result = mysql_query( $query ) or die( "Bad query: $query" );;

if ( mysql_num_rows( $result) == 0 )
{
   $msg = "Sorry.  We could not find your registration.";
}
else
{
   list ( $fname, $lname, $validated ) = mysql_fetch_row( $result );

   if ( $validated == 'n' )
   {
      $query = "UPDATE people SET validated='y' WHERE random=$id";
      mysql_query( $query ) or die( "Could not update database" );
      $status = "Your Your UltraScan III license has been activated.";
   }
   else
   {
      $status = "Your Your UltraScan III license has been previousely activated.";
   }
   
   $msg = "Thank you $fname $lname. $status\n" .
          "Please restart UltraScan to retrieve your license.";
}
// Send response.

echo <<< EOF
<html>
<head>
  <title>UltraScan III License Activation</title>
</head>
<body>

   <p>$msg</p>

</body>
</html>
EOF

?>
