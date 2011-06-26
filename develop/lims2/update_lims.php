<?php
   $username = "limsconfig";
   $password = "lookn4a$";
   $host = "bcf.uthscsa.edu";
   $link = mysql_connect($host, $username, $password) or die("Could not connect");
   mysql_select_db("lims2_config") or die("Could not select database");

	$query = "SELECT * FROM config";
	$result = mysql_query($query) or die("Query failed");
	while ($line = mysql_fetch_array($result, MYSQL_ASSOC))
	{
		print
		$line["topImage"] . " " . 
		$line["facilityName"] . " " . 
		$line["facilityAdmin"] . " " . 
		$line["facilityAdminPhone"] . " " . 
		$line["facilityAdminEmail"] . " " . 
		$line["dbusername"] . " " . 
		$line["dbpasswd"] . " " . 
		$line["db_host"] . " " . 
		$line["data_dir"] . " " . 
		$line["full_path"] . " " . 
		"\n";


	}
?>
