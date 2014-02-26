<?php
	$username = "";
	$password = "";
	$host = "";
   $link = mysql_connect($host, $username, $password) or die("Could not connect");
   mysql_select_db("us_register") or die("Could not select database\n");
   $query = "SELECT * FROM license";
   $result = mysql_query($query) or die("Query failed");
	while ($line = mysql_fetch_array($result, MYSQL_ASSOC))
	{
		print $line["operating_system"] . " " . $line["expiration"] . "\n";
	}
?>	
