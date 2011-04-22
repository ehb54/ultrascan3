<?php
   $username = "limsconfig";
   $password = "lookn4a$";
   $host = "bcf.uthscsa.edu";
   $link = mysql_connect($host, $username, $password) or die("Could not connect");
   mysql_select_db("lims2_config") or die("Could not select database");

	$query = "SELECT * FROM config";
	$result = mysql_query($query) or die("Query failed");

	$header = "<?php\n\n"
			  . "/*  Database Information - Required!!\n"  
		     . "*  -- Configure the Variables Below --\n"
			  . "*  Other files that may need to be edited:\n"
			  . "*\n"
			  . "*  1. newaccount.html: Edit user account affiliation, Account name\n"
			  . "*  2. login_success.php: edit the calendar link and other customizations\n"
			  . "*  3. introduction.php: change verbage to match your facility\n" 
			  . "*  4. privacy.php: edit privacy statement\n"
			  . "*  5. images/UltraScan-banner.png: approx 89 pixels h x 560 pixels w\n" 
			  . "*     change the file name below\n"
			  . "*  6. create data and user/data\n"
			  . "*  7. chown -R apache for data and user/data\n"
			  . "*/\n\n";

	$footer = "//////////// End of user specific configuration\n\n"
			  . "/* Define our file paths */\n"
			  . "if ( ! defined('BASE_DIR') ) \n"
			  . "{\n"
			  . "    // ensure a trailing slash\n"
			  . "	if ( \$full_path[strlen(\$full_path) - 1] != '/' )\n"
			  . "		\$full_path .= '/';\n"
			  . "	define('BASE_DIR', \$full_path . 'user/' );\n"
			  . "	define('HOME_DIR', \$full_path );\n"
			  . "}\n\n"
			  . "/* Define our library directory */\n"
			  . "if ( ! defined('LIB_DIR') )\n"
			  . "{\n"
			  . "	define('LIB_DIR', BASE_DIR.\"lib\/\" );\n"
			  . "}\n\n"
			  . "if ( !defined('DATA_DIR') )\n"
			  . "{\n"
			  . "	// ensure a trailing slash\n"
			  . "	if ( \$data_dir[strlen(\$data_dir) - 1] != '/' )\n"
			  . "		\$data_dir .= '/';\n\n"
			  . "	define('DATA_DIR', HOME_DIR . \$data_dir );\n"
			  . "}\n\n"
			  . "if ( ! defined('DEBUG') )\n"
			  . "{\n"
			  . "	define('DEBUG', false );\n"
			  . "}\n\n"
			  . "?>\n";
					
	while ($line = mysql_fetch_array($result, MYSQL_ASSOC))
	{
			  /*
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
				*/
		$filename = $line["database_name"] . "/user/config.php";
		$file = fopen($filename, 'w');
		fwrite($file, $header);
		fwrite($file, "\$topTitle				= '" . $line["topTitle"] . "'\n");
		fwrite($file, "\$topImage				= '" . $line["topImage"] . "'\n");
		fwrite($file, "\$facilityName			= '" . $line["facilityName"] . "'\n");
		fwrite($file, "\$facilityAdmin			= '" . $line["facilityAdmin"] . "'\n");
		fwrite($file, "\$facilityAdminPhone	= '" . $line["facilityAdminPhone"] . "'\n");
		fwrite($file, "\$facilityAdminEmail	= '" . $line["facilityAdminEmail"] . "'\n");
		fwrite($file, "\$dbusername				= '" . $line["dbusername"] . "'\n");
		fwrite($file, "\$dbpasswd				= '" . $line["dbpasswd"] . "'\n");
		fwrite($file, "\$database_name			= '" . $line["database_name"] . "'\n");
		fwrite($file, "\$db_host					= '" . $line["db_host"] . "'\n");
		fwrite($file, "\$data_dir				= '" . $line["data_dir"] . "'\n");
		fwrite($file, "\$full_path				= '" . $line["full_path"] . "'\n\n");
		fwrite($file, $footer);
		fclose($file);
	}
?>
